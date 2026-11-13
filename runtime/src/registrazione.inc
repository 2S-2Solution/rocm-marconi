/*
 * registrazione.inc - kernel incorporati dei programmi HIP 6.x/7.x sopra la runtime 5.7 del driver.
 * 2 Solution, 12 nov 2026.
 *
 * Un programma compilato con hipcc registra all'avvio i suoi kernel (__hipRegisterFatBinary,
 * __hipRegisterFunction, __hipRegisterVar) e poi li lancia per puntatore host (hipLaunchKernel,
 * <<<...>>>). La runtime 5.7 del driver non sa caricare il codice di ROCm 7/10: kpack (HIPK), code
 * object v6, bundle nuovi. Qui la registrazione la tiene lo shim e il codice si carica con le API
 * a moduli della 5.7 (hipModuleLoadData, hipModuleGetFunction, hipModuleLaunchKernel).
 *
 * - Registrare non tocca la GPU: il codice si carica al primo lancio (o alla prima richiesta di
 *   attributi/simboli) di quel fat binary. Chi registra senza mai lanciare non cambia nulla.
 * - HIPK (kpack di ROCm 10): il code object arriva da rocm_kpack.dll (kpack_load_code_object).
 * - HIPF: bundle clang classico; si sceglie la voce gfx906. Il bundle compresso (CCOB) non e'
 *   supportato e restituisce hipErrorNoBinaryForGpu.
 * - Code object v6 (ROCm 7+): il driver carica v4 e v5. Per gfx906 il v6 differisce solo nel
 *   campo EI_ABIVERSION; lo si riporta a 3 (v5). Vale anche per hipModuleLoad*.
 */

#define REG_HIPF 0x48495046u   /* "HIPF" */
#define REG_HIPK 0x4B504948u   /* "HIPK" */
#define hipErrorInvalidDeviceFunction 98
#define hipErrorNoBinaryForGpu        209
#define hipErrorInvalidSymbol         13   /* era 701 (hipErrorLaunchOutOfResources) fino al 13 nov 2026 */
#define hipErrorInvalidImage          200

typedef struct { unsigned int x, y, z; } reg_dim3;
typedef struct { unsigned int magic, version; const void* binary; void* reserved1; } reg_involucro;

typedef struct reg_fatbin {
    const reg_involucro* inv;
    SRWLOCK lock;
    int stato;          /* 0 da caricare, 1 caricato, <0 errore (codice HIP negato) */
    void* modulo;       /* hipModule_t della 5.7 */
} reg_fatbin;

typedef struct { const void* host; reg_fatbin* fb; char* nome; void* volatile funzione; } reg_funz;
typedef struct { const void* host; reg_fatbin* fb; char* nome; size_t dim; } reg_var;

/* ---- tabelle host -> voce, indirizzamento aperto, crescita a potenze di 2 ---- */
typedef struct { void** voci; size_t cap, n; SRWLOCK lock; } reg_tabella;
static reg_tabella reg_funzioni = { NULL, 0, 0, SRWLOCK_INIT };
static reg_tabella reg_variabili = { NULL, 0, 0, SRWLOCK_INIT };

static size_t reg_hash(const void* p) {
    unsigned long long x = (unsigned long long)(size_t)p;
    x ^= x >> 33; x *= 0xff51afd7ed558ccdULL; x ^= x >> 33;
    return (size_t)x;
}

/* le voci iniziano tutte con il puntatore host */
static void reg_metti_senza_lock(reg_tabella* t, void* voce) {
    const void* chiave = *(const void**)voce;
    size_t i;
    if ((t->n + 1) * 2 > t->cap) {
        size_t nc = t->cap ? t->cap * 2 : 1024, k;
        void** nv = (void**)calloc(nc, sizeof(void*));
        if (!nv) return;
        for (k = 0; k < t->cap; k++) {
            if (t->voci[k]) {
                size_t j = reg_hash(*(const void**)t->voci[k]) & (nc - 1);
                while (nv[j]) j = (j + 1) & (nc - 1);
                nv[j] = t->voci[k];
            }
        }
        free(t->voci); t->voci = nv; t->cap = nc;
    }
    i = reg_hash(chiave) & (t->cap - 1);
    while (t->voci[i]) {
        if (*(const void**)t->voci[i] == chiave) { t->voci[i] = voce; return; }   /* ri-registrazione */
        i = (i + 1) & (t->cap - 1);
    }
    t->voci[i] = voce; t->n++;
}

static void reg_metti(reg_tabella* t, void* voce) {
    AcquireSRWLockExclusive(&t->lock); reg_metti_senza_lock(t, voce); ReleaseSRWLockExclusive(&t->lock);
}

static void* reg_trova(reg_tabella* t, const void* chiave) {
    void* r = NULL;
    size_t i;
    AcquireSRWLockShared(&t->lock);
    if (t->cap) {
        i = reg_hash(chiave) & (t->cap - 1);
        while (t->voci[i]) {
            if (*(const void**)t->voci[i] == chiave) { r = t->voci[i]; break; }
            i = (i + 1) & (t->cap - 1);
        }
    }
    ReleaseSRWLockShared(&t->lock);
    return r;
}

static char* reg_copia(const char* s) {
    size_t n = s ? strlen(s) : 0;
    char* c = (char*)malloc(n + 1);
    if (c) { if (n) memcpy(c, s, n); c[n] = 0; }
    return c;
}

/* ---- code object: dimensione, v6 -> v5, bundle ---- */

static size_t reg_dim_elf(const unsigned char* e) {
    unsigned long long shoff = *(const unsigned long long*)(e + 0x28);
    unsigned short shentsize = *(const unsigned short*)(e + 0x3A), shnum = *(const unsigned short*)(e + 0x3C);
    unsigned long long phoff = *(const unsigned long long*)(e + 0x20);
    unsigned short phentsize = *(const unsigned short*)(e + 0x36), phnum = *(const unsigned short*)(e + 0x38);
    unsigned long long fine = shoff + (unsigned long long)shentsize * shnum, k;
    for (k = 0; k < shnum; k++) {           /* sezioni con contenuto nel file */
        const unsigned char* s = e + shoff + k * shentsize;
        unsigned int tipo = *(const unsigned int*)(s + 4);
        unsigned long long off = *(const unsigned long long*)(s + 0x18), sz = *(const unsigned long long*)(s + 0x20);
        if (tipo != 8 && off + sz > fine) fine = off + sz;    /* 8 = SHT_NOBITS */
    }
    for (k = 0; k < phnum; k++) {
        const unsigned char* p = e + phoff + k * phentsize;
        unsigned long long off = *(const unsigned long long*)(p + 8), fsz = *(const unsigned long long*)(p + 0x20);
        if (off + fsz > fine) fine = off + fsz;
    }
    return (size_t)fine;
}

static int reg_e_elf_amdgpu(const unsigned char* e) {
    return e[0] == 0x7f && e[1] == 'E' && e[2] == 'L' && e[3] == 'F' && e[4] == 2 &&
           e[7] == 64 && *(const unsigned short*)(e + 18) == 224;
}

/* Copia il code object adattandolo al caricatore della 5.7. Restituisce NULL se non serve.
 * - v6 -> v5: EI_ABIVERSION 4 -> 3 (per gfx906 le due versioni differiscono solo li').
 * - gfx906 con sramecc on -> any (e_flags bit 10-11: 3 -> 1). La Radeon VII non ha la ECC sulla SRAM e
 *   il driver rifiuta "sramecc on" (rc 303); hiprtc compila cosi' quando l'architettura dichiarata e'
 *   gfx906:sramecc+. Il codice per sramecc on non conta sulla conservazione dei bit alti delle letture
 *   d16, quindi e' valido anche senza ECC. */
static unsigned char* reg_v6_a_v5(const unsigned char* e, size_t dim) {
    unsigned char* c;
    unsigned int ef;
    int abi, ecc;
    if (!reg_e_elf_amdgpu(e)) return NULL;
    ef = *(const unsigned int*)(e + 48);
    abi = e[8] == 4;
    ecc = (ef & 0xff) == 0x2f && ((ef >> 10) & 3) == 3;
    if (!abi && !ecc) return NULL;
    c = (unsigned char*)malloc(dim);
    if (!c) return NULL;
    memcpy(c, e, dim);
    if (abi) c[8] = 3;
    if (ecc) *(unsigned int*)(c + 48) = (ef & ~0xc00u) | 0x400u;
    return c;
}

static const char REG_BUNDLE[] = "__CLANG_OFFLOAD_BUNDLE__";

/* Voce gfx906 di un bundle clang non compresso; *dim riceve la dimensione. */
static const unsigned char* reg_da_bundle(const unsigned char* b, size_t* dim) {
    unsigned long long n, k;
    const unsigned char* p;
    if (memcmp(b, REG_BUNDLE, 24) != 0) return NULL;
    n = *(const unsigned long long*)(b + 24);
    p = b + 32;
    for (k = 0; k < n; k++) {
        unsigned long long off = *(const unsigned long long*)p, sz = *(const unsigned long long*)(p + 8);
        unsigned long long tl = *(const unsigned long long*)(p + 16);
        const char* tripla = (const char*)(p + 24);
        char t[256];
        size_t m = tl < sizeof(t) - 1 ? (size_t)tl : sizeof(t) - 1;
        memcpy(t, tripla, m); t[m] = 0;
        if (strstr(t, "amdgcn") && strstr(t, "gfx906") && sz) { *dim = (size_t)sz; return b + off; }
        p += 24 + tl;
    }
    return NULL;
}

/* ---- argomenti nascosti che la runtime 5.7 non conosce ----
 * I compilatori di ROCm 6+ danno ai kernel che usano memoria condivisa dinamica l'argomento nascosto
 * hidden_dynamic_lds_size. La 5.7 non lo conosce e lo tratta come un argomento esplicito: nei lanci
 * con kernelParams legge una voce oltre la fine dell'array (crash o dati sbagliati). Il code object
 * non si tocca: dopo il caricamento si leggono gli argomenti dei kernel con l'API pubblica di comgr
 * (la amd_comgr.dll della runtime del driver) e al lancio si passa alla 5.7 un array con la voce in
 * piu', che punta alla memoria condivisa dinamica del lancio (il valore che il kernel si aspetta). */

typedef struct { unsigned long long h; } reg_cg;
typedef int (*cg_f_create)(int kind, reg_cg* d);
typedef int (*cg_f_set)(reg_cg d, size_t n, const char* b);
typedef int (*cg_f_getmd)(reg_cg d, reg_cg* md);
typedef int (*cg_f_lookup)(reg_cg n, const char* chiave, reg_cg* v);
typedef int (*cg_f_listsize)(reg_cg n, size_t* dim);
typedef int (*cg_f_index)(reg_cg n, size_t i, reg_cg* v);
typedef int (*cg_f_string)(reg_cg n, size_t* dim, char* s);
typedef int (*cg_f_destroy)(reg_cg n);
typedef int (*cg_f_release)(reg_cg d);
#define REG_CG_EXECUTABLE 8

/* Per ogni kernel con argomenti da completare: tipo per argomento, nell'ordine dei metadati.
 * 'e' esplicito (dal chiamante), 'l' hidden_dynamic_lds_size (lo aggiunge lo shim), 'h' altro nascosto. */
/* Un kernel di un modulo caricato: tipi degli argomenti (per il completamento al lancio), e offset e
 * dimensione di ciascuno (per hipKernelGetParamInfo). ha_l != 0 se c'e' hidden_dynamic_lds_size. Dal
 * 13 nov 2026 si registra OGNI kernel del modulo, non solo quelli con 'l', cosi' le API a libreria
 * (hipModuleGetFunctionCount, hipLibrary e hipKernel) hanno l'elenco completo. */
typedef struct reg_kinfo { struct reg_kinfo* succ; void* modulo; char* nome; int n; char* tipi; int espliciti;
                           int ha_l; size_t* off; size_t* sz; } reg_kinfo;
static reg_kinfo* reg_kinfo_lista = NULL;
static SRWLOCK reg_kinfo_lock = SRWLOCK_INIT;

typedef struct { const void* f; reg_kinfo* info; } reg_fun_info;
static reg_tabella reg_fun_infos = { NULL, 0, 0, SRWLOCK_INIT };


static int reg_cg_stringa(cg_f_string str, reg_cg n, char* buf, size_t dim) {
    size_t d = 0;
    if (str(n, &d, NULL) != 0 || d == 0 || d > dim) return 0;
    return str(n, &d, buf) == 0;
}

/* Legge i kernel del modulo appena caricato e li registra tutti (nome, tipi degli argomenti, offset e
 * dimensione di ciascuno). Serve sia al completamento di hidden_dynamic_lds_size al lancio, sia alle API
 * a libreria (hipModuleGetFunctionCount, hipLibrary e hipKernel). Legge sempre i metadati, non solo
 * quando c'e' LDS dinamica (prima era un'ottimizzazione): i moduli si caricano poche volte. */
static void reg_leggi_argomenti(void* modulo, const unsigned char* elf, size_t dim) {
    static const char DYN[] = "hidden_dynamic_lds_size";
    cg_f_create crea; cg_f_set imposta; cg_f_getmd getmd; cg_f_lookup cerca; cg_f_listsize quanti;
    cg_f_index indice; cg_f_string str; cg_f_destroy distruggi; cg_f_release rilascia;
    reg_cg dati = { 0 }, md = { 0 }, kernels = { 0 };
    size_t nk = 0, k;
    int trovati = 0;
    (void)dim;
    if (!g_comgr_driver) return;
    crea = (cg_f_create)GetProcAddress(g_comgr_driver, "amd_comgr_create_data");
    imposta = (cg_f_set)GetProcAddress(g_comgr_driver, "amd_comgr_set_data");
    getmd = (cg_f_getmd)GetProcAddress(g_comgr_driver, "amd_comgr_get_data_metadata");
    cerca = (cg_f_lookup)GetProcAddress(g_comgr_driver, "amd_comgr_metadata_lookup");
    quanti = (cg_f_listsize)GetProcAddress(g_comgr_driver, "amd_comgr_get_metadata_list_size");
    indice = (cg_f_index)GetProcAddress(g_comgr_driver, "amd_comgr_index_list_metadata");
    str = (cg_f_string)GetProcAddress(g_comgr_driver, "amd_comgr_get_metadata_string");
    distruggi = (cg_f_destroy)GetProcAddress(g_comgr_driver, "amd_comgr_destroy_metadata");
    rilascia = (cg_f_release)GetProcAddress(g_comgr_driver, "amd_comgr_release_data");
    if (!crea || !imposta || !getmd || !cerca || !quanti || !indice || !str || !distruggi || !rilascia) return;
    if (crea(REG_CG_EXECUTABLE, &dati) != 0) return;
    if (imposta(dati, dim, (const char*)elf) == 0 && getmd(dati, &md) == 0) {
        if (cerca(md, "amdhsa.kernels", &kernels) == 0) {
            if (quanti(kernels, &nk) != 0) nk = 0;
            for (k = 0; k < nk; k++) {
                reg_cg kn = { 0 }, nome = { 0 }, args = { 0 };
                char nm[1024], vk[64];
                size_t na = 0, a;
                if (indice(kernels, k, &kn) != 0) continue;
                if (cerca(kn, ".name", &nome) == 0 && reg_cg_stringa(str, nome, nm, sizeof(nm)) &&
                    cerca(kn, ".args", &args) == 0 && quanti(args, &na) == 0 && na > 0 && na < 4096) {
                    char* tipi = (char*)malloc(na);
                    size_t* off = (size_t*)calloc(na, sizeof(size_t));
                    size_t* sz = (size_t*)calloc(na, sizeof(size_t));
                    int conta_l = 0, espliciti = 0;
                    for (a = 0; tipi && off && sz && a < na; a++) {
                        reg_cg ar = { 0 }, v = { 0 };
                        char num[32];
                        tipi[a] = 'e';
                        if (indice(args, a, &ar) != 0) continue;
                        if (cerca(ar, ".value_kind", &v) == 0) {
                            if (reg_cg_stringa(str, v, vk, sizeof(vk))) {
                                if (strcmp(vk, DYN) == 0) { tipi[a] = 'l'; conta_l++; }
                                else if (strncmp(vk, "hidden_", 7) == 0) tipi[a] = 'h';
                            }
                            distruggi(v);
                        }
                        v.h = 0;
                        if (cerca(ar, ".offset", &v) == 0) { if (reg_cg_stringa(str, v, num, sizeof(num))) off[a] = (size_t)_strtoui64(num, NULL, 10); distruggi(v); }
                        v.h = 0;
                        if (cerca(ar, ".size", &v) == 0) { if (reg_cg_stringa(str, v, num, sizeof(num))) sz[a] = (size_t)_strtoui64(num, NULL, 10); distruggi(v); }
                        if (tipi[a] == 'e') espliciti++;
                        distruggi(ar);
                    }
                    if (tipi && off && sz) {
                        reg_kinfo* ki = (reg_kinfo*)calloc(1, sizeof(reg_kinfo));
                        if (ki) {
                            ki->modulo = modulo; ki->nome = reg_copia(nm); ki->n = (int)na; ki->tipi = tipi;
                            ki->espliciti = espliciti; ki->ha_l = conta_l; ki->off = off; ki->sz = sz;
                            AcquireSRWLockExclusive(&reg_kinfo_lock);
                            ki->succ = reg_kinfo_lista; reg_kinfo_lista = ki;
                            ReleaseSRWLockExclusive(&reg_kinfo_lock);
                            tipi = NULL; off = NULL; sz = NULL; trovati++;
                        }
                    }
                    free(tipi); free(off); free(sz);
                }
                if (args.h) distruggi(args);
                if (nome.h) distruggi(nome);
                distruggi(kn);
            }
            distruggi(kernels);
        }
        distruggi(md);
    }
    rilascia(dati);
    REG_T("[reg] modulo %p: %d kernel su %zu con hidden_dynamic_lds_size", modulo, trovati, nk);
}

static reg_kinfo* reg_kinfo_trova(void* modulo, const char* nome) {
    reg_kinfo* k;
    AcquireSRWLockShared(&reg_kinfo_lock);
    for (k = reg_kinfo_lista; k; k = k->succ)
        if (k->modulo == modulo && strcmp(k->nome, nome) == 0) break;
    ReleaseSRWLockShared(&reg_kinfo_lock);
    return k;
}

/* Dopo hipModuleGetFunction: ricorda quali funzioni vanno completate al lancio. */
static void reg_associa(void* f, void* modulo, const char* nome) {
    reg_kinfo* ki;
    reg_fun_info* fi;
    if (!f || !nome || !reg_kinfo_lista) return;
    ki = reg_kinfo_trova(modulo, nome);
    if (!ki || reg_trova(&reg_fun_infos, f)) return;
    fi = (reg_fun_info*)calloc(1, sizeof(reg_fun_info));
    if (!fi) return;
    fi->f = f; fi->info = ki;
    reg_metti(&reg_fun_infos, fi);
}

typedef int (*reg_f_lancio_mod)(void* f, unsigned int gx, unsigned int gy, unsigned int gz, unsigned int bx,
                                unsigned int by, unsigned int bz, unsigned int shm, void* stream, void** arg, void** extra);

typedef int (*reg_f_lancio_coop)(void* f, unsigned int gx, unsigned int gy, unsigned int gz, unsigned int bx,
                                 unsigned int by, unsigned int bz, unsigned int shm, void* stream, void** arg);

/* Array degli argomenti completato per la 5.7: agli espliciti del chiamante si aggiunge, dove il kernel
 * lo dichiara, il puntatore a hidden_dynamic_lds_size (*lds). Restituisce arg se non serve nulla, locale
 * (64 voci) o un blocco allocato (da liberare con reg_completo_libera); NULL se manca memoria. */
static void** reg_completa(void* f, void** arg, unsigned int* lds, void** locale) {
    reg_fun_info* fi = arg && reg_fun_infos.n ? (reg_fun_info*)reg_trova(&reg_fun_infos, f) : NULL;
    void** completo;
    int a, i = 0, j = 0;
    if (!fi || !fi->info->ha_l) return arg;   /* nessun hidden_dynamic_lds_size: l'array del chiamante va bene */
    completo = fi->info->n <= 64 ? locale : (void**)malloc(sizeof(void*) * (size_t)fi->info->n);
    if (!completo) return NULL;
    for (a = 0; a < fi->info->n; a++) {
        if (fi->info->tipi[a] == 'e') completo[j++] = arg[i++];
        else if (fi->info->tipi[a] == 'l') completo[j++] = lds;
    }
    return completo;
}
static void reg_completo_libera(void** completo, void** arg, void** locale) {
    if (completo != arg && completo != locale) free(completo);
}

/* hipModuleLaunchKernel (o hipModuleLaunchCooperativeKernel, coop = 1) della 5.7, con l'array degli
 * argomenti completato se il kernel lo richiede. */
static int reg_lancia_x(int coop, void* f, unsigned int gx, unsigned int gy, unsigned int gz, unsigned int bx,
                        unsigned int by, unsigned int bz, unsigned int shm, void* stream, void** arg, void** extra) {
    static reg_f_lancio_mod lancio = NULL;
    static reg_f_lancio_coop lancio_coop = NULL;
    void* locale[64];
    void** completo;
    unsigned int lds = shm;
    int rc;
    if (coop) {
        if (!lancio_coop) { lancio_coop = (reg_f_lancio_coop)abi57_reale("hipModuleLaunchCooperativeKernel"); if (!lancio_coop) return hipErrorNotSupported; }
    } else {
        if (!lancio) { lancio = (reg_f_lancio_mod)abi57_reale("hipModuleLaunchKernel"); if (!lancio) return hipErrorNotSupported; }
    }
    completo = reg_completa(f, arg, &lds, locale);
    if (arg && !completo) return hipErrorOutOfMemory;
    rc = coop ? lancio_coop(f, gx, gy, gz, bx, by, bz, shm, stream, completo)
              : lancio(f, gx, gy, gz, bx, by, bz, shm, stream, completo, extra);
    reg_completo_libera(completo, arg, locale);
    return rc;
}

static int reg_lancia(void* f, unsigned int gx, unsigned int gy, unsigned int gz, unsigned int bx, unsigned int by,
                      unsigned int bz, unsigned int shm, void* stream, void** arg, void** extra) {
    return reg_lancia_x(0, f, gx, gy, gz, bx, by, bz, shm, stream, arg, extra);
}

/* ---- bundle compressi (CCOB) ----
 * ROCm 7+ distribuisce i code object anche come bundle clang compressi: "CCOB", versione (u16), metodo
 * (u16: 0 zlib, 1 zstd), poi v1: dimensione decompressa u32 e hash u64; v2: dimensione totale u32,
 * decompressa u32, hash u64; v3: totale u64, decompressa u64, hash u64. Segue il flusso compresso, che
 * contiene un bundle normale. Si decomprime (zstd) e si prosegue come per un bundle normale. */
#include "zstd/zstd.h"

static unsigned char* reg_da_ccob(const unsigned char* c, size_t* dim) {
    unsigned short ver = *(const unsigned short*)(c + 4), metodo = *(const unsigned short*)(c + 6);
    unsigned long long totale = 0, piena = 0;
    size_t testa;
    unsigned char* out;
    size_t r;
    if (ver == 1) { piena = *(const unsigned int*)(c + 8); testa = 20; }
    else if (ver == 2) { totale = *(const unsigned int*)(c + 8); piena = *(const unsigned int*)(c + 12); testa = 24; }
    else if (ver == 3) { totale = *(const unsigned long long*)(c + 8); piena = *(const unsigned long long*)(c + 16); testa = 32; }
    else return NULL;
    if (metodo != 1 || piena == 0 || piena > (1ull << 32)) return NULL;          /* solo zstd */
    if (!totale) {
        size_t z = ZSTD_findFrameCompressedSize(c + testa, (size_t)1 << 31);
        if (ZSTD_isError(z)) return NULL;
        totale = testa + z;
    }
    if (totale <= testa) return NULL;
    out = (unsigned char*)malloc((size_t)piena);
    if (!out) return NULL;
    r = ZSTD_decompress(out, (size_t)piena, c + testa, (size_t)(totale - testa));
    if (ZSTD_isError(r) || r != piena) {
        REG_T("[reg] CCOB v%u: decompressione fallita (%s)", ver, ZSTD_isError(r) ? ZSTD_getErrorName(r) : "dimensione");
        free(out);
        return NULL;
    }
    *dim = (size_t)piena;
    return out;
}

/* Carica un'immagine (ELF o bundle) con la hipModuleLoadData della 5.7, convertendo se serve. */
typedef int (*reg_f_load)(void** modulo, const void* immagine);
static int reg_carica_immagine(void** modulo, const void* immagine) {
    static reg_f_load reale = NULL;
    const unsigned char* img = (const unsigned char*)immagine;
    const unsigned char* elf = img;
    unsigned char* conv = NULL;
    unsigned char* aperto = NULL;     /* bundle decompresso da un CCOB */
    size_t dim = 0, dim_aperto = 0;
    int rc;
    if (!reale) { reale = (reg_f_load)abi57_reale("hipModuleLoadData"); if (!reale) return hipErrorNotSupported; }
    if (!img) return reale(modulo, immagine);
    if (memcmp(img, "CCOB", 4) == 0) {
        aperto = reg_da_ccob(img, &dim_aperto);
        if (!aperto || dim_aperto < 32 || memcmp(aperto, REG_BUNDLE, 24) != 0) { free(aperto); return hipErrorNoBinaryForGpu; }
        img = aperto;
    }
    if (memcmp(img, REG_BUNDLE, 24) == 0) {
        elf = reg_da_bundle(img, &dim);
        if (!elf) { free(aperto); return hipErrorNoBinaryForGpu; }
    } else if (reg_e_elf_amdgpu(img)) {
        dim = reg_dim_elf(img);
    } else {
        REG_T("[reg] immagine %p formato sconosciuto: %02x %02x %02x %02x", immagine, img[0], img[1], img[2], img[3]);
        return reale(modulo, immagine);       /* formato che non conosciamo: decide la 5.7 */
    }
    conv = reg_v6_a_v5(elf, dim);
    if (aperto) {                     /* si tiene solo l'ELF, non tutto il bundle decompresso */
        if (!conv && (conv = (unsigned char*)malloc(dim)) != NULL) memcpy(conv, elf, dim);
        free(aperto);
        if (!conv) return hipErrorOutOfMemory;
        elf = conv;
    }
    rc = reale(modulo, conv ? conv : elf);
    REG_T("[reg] immagine %p dim %zu abiv %d->%d e_flags %08x bundle %d -> rc %d", immagine, dim, elf[8],
          conv ? conv[8] : elf[8], *(const unsigned int*)(elf + 48), elf != (const unsigned char*)immagine, rc);
    /* Il buffer convertito resta vivo: non e' garantito che la runtime 5.7 copi l'immagine subito. */
    if (rc != hipSuccess) free(conv);
    else reg_leggi_argomenti(*modulo, conv ? conv : elf, dim);
    return rc;
}

/* ---- kpack (HIPK) attraverso rocm_kpack.dll ---- */
typedef int (*kp_f_cache)(void** cache);
typedef int (*kp_f_percorso)(const void* ind, char* out, size_t dim, size_t* off);
typedef int (*kp_f_carica)(void* cache, const void* meta, const char* percorso, unsigned int indice,
                           const char* const* archi, size_t n, void** co, size_t* dim);
typedef void (*kp_f_libera)(void* co);
static struct { int pronto; HMODULE dll; void* cache; kp_f_percorso percorso; kp_f_carica carica; kp_f_libera libera; } reg_kp;
static SRWLOCK reg_kp_lock = SRWLOCK_INIT;

static int reg_kpack_pronto(void) {
    AcquireSRWLockExclusive(&reg_kp_lock);
    if (!reg_kp.pronto) {
        reg_kp.pronto = -1;
        /* gia' caricata; poi le cartelle registrate (os.add_dll_directory, AddDllDirectory); poi accanto
         * allo shim (le wheel AMD la tengono in _rocm_sdk_core\bin con amdhip64_7.dll); poi il PATH */
        reg_kp.dll = GetModuleHandleA("rocm_kpack.dll");
        if (!reg_kp.dll) reg_kp.dll = LoadLibraryExA("rocm_kpack.dll", NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!reg_kp.dll) {
            HMODULE io = NULL;
            char p[MAX_PATH + 32];
            DWORD n;
            if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                   (LPCSTR)(void*)&reg_kpack_pronto, &io) &&
                (n = GetModuleFileNameA(io, p, MAX_PATH)) > 0 && n < MAX_PATH) {
                char* s = strrchr(p, '\\');
                if (s) { lstrcpyA(s + 1, "rocm_kpack.dll"); reg_kp.dll = LoadLibraryExA(p, NULL, LOAD_WITH_ALTERED_SEARCH_PATH); }
            }
        }
        if (!reg_kp.dll) reg_kp.dll = LoadLibraryA("rocm_kpack.dll");
        REG_T("[reg] rocm_kpack.dll %p", (void*)reg_kp.dll);
        if (reg_kp.dll) {
            kp_f_cache crea = (kp_f_cache)GetProcAddress(reg_kp.dll, "kpack_cache_create");
            reg_kp.percorso = (kp_f_percorso)GetProcAddress(reg_kp.dll, "kpack_discover_binary_path");
            reg_kp.carica = (kp_f_carica)GetProcAddress(reg_kp.dll, "kpack_load_code_object");
            reg_kp.libera = (kp_f_libera)GetProcAddress(reg_kp.dll, "kpack_free_code_object");
            if (crea && reg_kp.percorso && reg_kp.carica && reg_kp.libera && crea(&reg_kp.cache) == 0) reg_kp.pronto = 1;
        }
    }
    ReleaseSRWLockExclusive(&reg_kp_lock);
    return reg_kp.pronto == 1;
}

static int reg_carica_kpack(reg_fatbin* fb) {
    /* la Radeon VII si presenta come gfx906:sramecc+:xnack-; i kpack di ROCm 10 la portano come "any" */
    static const char* const archi[] = { "gfx906:sramecc+:xnack-", "gfx906:sramecc-:xnack-", "gfx906:xnack-",
                                         "gfx906:sramecc+", "gfx906:sramecc-", "gfx906" };
    char percorso[1024];
    void* co = NULL;
    size_t dim = 0;
    int rc;
    if (!reg_kpack_pronto()) return hipErrorNoBinaryForGpu;   /* manca rocm_kpack.dll */
    rc = reg_kp.percorso(fb->inv->binary, percorso, sizeof(percorso), NULL);
    REG_T("[reg] kpack percorso rc %d: %s", rc, rc == 0 ? percorso : "-");
    if (rc != 0) return hipErrorNoBinaryForGpu;
    rc = reg_kp.carica(reg_kp.cache, fb->inv->binary, percorso, (unsigned int)(size_t)fb->inv->reserved1, archi, sizeof(archi) / sizeof(archi[0]), &co, &dim);
    REG_T("[reg] kpack carica rc %d co %p dim %zu abiv %d", rc, co, dim, co ? ((unsigned char*)co)[8] : -1);
    if (rc != 0 || !co) return hipErrorNoBinaryForGpu;
#ifdef REG_TRACCIA
    {   /* MARCONI_CO_DUMP=<cartella>: salva i code object estratti, per leggerne i metadati */
        char cart[512], f[700]; DWORD sc = 0; HANDLE h;
        if (GetEnvironmentVariableA("MARCONI_CO_DUMP", cart, sizeof(cart)) > 0) {
            _snprintf(f, sizeof(f), "%s\\co_%p.elf", cart, (void*)fb);
            h = CreateFileA(f, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (h != INVALID_HANDLE_VALUE) { WriteFile(h, co, (DWORD)dim, &sc, NULL); CloseHandle(h); }
        }
    }
#endif
    rc = reg_carica_immagine(&fb->modulo, co);
    if (rc != hipSuccess) reg_kp.libera(co);    /* se caricato, resta vivo per tutta la vita del modulo */
    return rc;
}

/* Carica il codice di un fat binary alla prima richiesta. Restituisce 0 o un codice HIP. */
static int reg_carica(reg_fatbin* fb) {
    int stato;
    AcquireSRWLockShared(&fb->lock); stato = fb->stato; ReleaseSRWLockShared(&fb->lock);
    if (stato == 1) return hipSuccess;
    if (stato < 0) return -stato;
    AcquireSRWLockExclusive(&fb->lock);
    if (fb->stato == 0) {
        int rc;
        if (!marconi_shim_bootstrap()) rc = hipErrorNotInitialized;
        else if (fb->inv->magic == REG_HIPK) rc = reg_carica_kpack(fb);
        else if (fb->inv->magic == REG_HIPF) rc = reg_carica_immagine(&fb->modulo, fb->inv->binary);
        else rc = hipErrorInvalidImage;
        fb->stato = rc == hipSuccess ? 1 : -rc;
        REG_T("[reg] carica fb %p magic %08x indice %p -> rc %d modulo %p", (void*)fb, fb->inv->magic,
              fb->inv->reserved1, rc, fb->modulo);
    }
    stato = fb->stato;
    ReleaseSRWLockExclusive(&fb->lock);
    return stato == 1 ? hipSuccess : -stato;
}

typedef int (*reg_f_getfunc)(void** f, void* modulo, const char* nome);
static int reg_funzione(const void* host, void** f) {
    static reg_f_getfunc getfunc = NULL;
    reg_funz* v = (reg_funz*)reg_trova(&reg_funzioni, host);
    int rc;
    if (!v) return -1;                                   /* non nostra */
    if (v->funzione) { *f = v->funzione; return hipSuccess; }
    rc = reg_carica(v->fb);
    if (rc != hipSuccess) return rc;
    if (!getfunc) { getfunc = (reg_f_getfunc)abi57_reale("hipModuleGetFunction"); if (!getfunc) return hipErrorNotSupported; }
    {
        void* g = NULL;
        rc = getfunc(&g, v->fb->modulo, v->nome);
        REG_T("[reg] funzione %p = %s (fb %p) -> rc %d f %p", host, v->nome, (void*)v->fb, rc, g);
        if (rc != hipSuccess) return rc == hipErrorNotSupported ? rc : hipErrorInvalidDeviceFunction;
        reg_associa(g, v->fb->modulo, v->nome);
        InterlockedCompareExchangePointer(&v->funzione, g, NULL);
        *f = v->funzione;
    }
    return hipSuccess;
}

typedef int (*reg_f_global)(void** dptr, size_t* dim, void* modulo, const char* nome);
static int reg_simbolo(const void* host, void** dptr, size_t* dim) {
    static reg_f_global global = NULL;
    reg_var* v = (reg_var*)reg_trova(&reg_variabili, host);
    int rc;
    size_t d = 0;
    void* p = NULL;
    if (!v) return -1;
    rc = reg_carica(v->fb);
    if (rc != hipSuccess) return rc;
    if (!global) { global = (reg_f_global)abi57_reale("hipModuleGetGlobal"); if (!global) return hipErrorNotSupported; }
    rc = global(&p, &d, v->fb->modulo, v->nome);
    if (rc != hipSuccess) return hipErrorInvalidSymbol;
    if (dptr) *dptr = p;
    if (dim) *dim = d;
    return hipSuccess;
}

/* ---- registrazione ---- */

__declspec(dllexport) void** __hipRegisterFatBinary(const void* dati) {
    reg_fatbin* fb = (reg_fatbin*)calloc(1, sizeof(reg_fatbin));
    if (!fb) return NULL;
    InitializeSRWLock(&fb->lock);
    fb->inv = (const reg_involucro*)dati;
    if (!dati || (fb->inv->magic != REG_HIPF && fb->inv->magic != REG_HIPK)) fb->stato = -hipErrorInvalidImage;
    REG_T("[reg] fatbin %p magic %08x", (void*)fb, dati ? fb->inv->magic : 0);
    return (void**)fb;
}

__declspec(dllexport) void __hipRegisterFunction(void** moduli, const void* host, char* funzione_device,
                                                 const char* nome_device, unsigned int limite, void* tid,
                                                 void* bid, void* blocco, void* griglia, int* wsize) {
    reg_funz* v;
    (void)funzione_device; (void)limite; (void)tid; (void)bid; (void)blocco; (void)griglia; (void)wsize;
    if (!moduli || !host || !nome_device) return;
    v = (reg_funz*)calloc(1, sizeof(reg_funz));
    if (!v) return;
    v->host = host; v->fb = (reg_fatbin*)moduli; v->nome = reg_copia(nome_device);
    reg_metti(&reg_funzioni, v);
}

__declspec(dllexport) void __hipRegisterVar(void** moduli, void* host, char* nome_host, char* nome_device,
                                            int esterna, size_t dim, int costante, int globale) {
    reg_var* v;
    (void)nome_host; (void)esterna; (void)costante; (void)globale;
    if (!moduli || !host || !nome_device) return;
    v = (reg_var*)calloc(1, sizeof(reg_var));
    if (!v) return;
    v->host = host; v->fb = (reg_fatbin*)moduli; v->nome = reg_copia(nome_device); v->dim = dim;
    reg_metti(&reg_variabili, v);
}

/* Si chiama alla chiusura del programma o allo scaricamento della DLL: le voci restano (gli indirizzi
 * host non vengono riusati finche' il modulo che li contiene e' caricato) e il modulo GPU si scarica. */
typedef int (*reg_f_unload)(void* modulo);
/* Alla chiusura del processo i distruttori dei programmi chiamano __hipUnregisterFatBinary quando la
 * runtime del driver (caricata per ultima, quindi staccata per prima) e' gia' chiusa: allora non si
 * scarica nulla, la memoria la libera Windows. Vedi marconi_in_chiusura in shim_main.c. */

__declspec(dllexport) void __hipUnregisterFatBinary(void** moduli) {
    reg_fatbin* fb = (reg_fatbin*)moduli;
    if (!fb) return;
    if (marconi_in_chiusura()) return;
    AcquireSRWLockExclusive(&fb->lock);
    if (fb->stato == 1 && fb->modulo && g_amdhip64) {
        reg_f_unload scarica = (reg_f_unload)GetProcAddress(g_amdhip64, "hipModuleUnload");
        if (scarica) scarica(fb->modulo);
    }
    fb->modulo = NULL;
    fb->stato = -hipErrorInvalidImage;
    ReleaseSRWLockExclusive(&fb->lock);
}

/* ---- lanci e interrogazioni per puntatore host ---- */

typedef int (*reg_f_lancio)(const void* host, reg_dim3 g, reg_dim3 b, void** arg, size_t shm, void* stream);

static __declspec(thread) int reg_ultimo_errore = hipSuccess;
static int reg_errore(int rc) { if (rc != hipSuccess) reg_ultimo_errore = rc; return rc; }

__declspec(dllexport) int hipLaunchKernel(const void* host, reg_dim3 griglia, reg_dim3 blocco, void** arg,
                                          size_t shm, void* stream) {
    void* f = NULL;
    int rc = reg_funzione(host, &f);
    REG_T("[reg] lancio %p rc %d f %p g %u,%u,%u b %u,%u,%u shm %zu s %p", host, rc, f, griglia.x, griglia.y, griglia.z,
          blocco.x, blocco.y, blocco.z, shm, stream);
    if (rc < 0) {                                         /* non registrata dallo shim */
        ABI57_REALE(reale, reg_f_lancio, "hipLaunchKernel");
        return reale(host, griglia, blocco, arg, shm, stream);
    }
    if (rc != hipSuccess) return reg_errore(rc);
    return reg_errore(reg_lancia(f, griglia.x, griglia.y, griglia.z, blocco.x, blocco.y, blocco.z, (unsigned int)shm,
                                 stream, arg, NULL));
}

/* API a moduli: stesse funzioni della 5.7, piu' l'associazione funzione -> argomenti da completare. */
__declspec(dllexport) int hipModuleGetFunction(void** f, void* modulo, const char* nome) {
    int rc;
    ABI57_REALE(reale, reg_f_getfunc, "hipModuleGetFunction");
    rc = reale(f, modulo, nome);
    if (rc == hipSuccess && f) reg_associa(*f, modulo, nome);
    return rc;
}

__declspec(dllexport) int hipModuleLaunchKernel(void* f, unsigned int gx, unsigned int gy, unsigned int gz,
                                                unsigned int bx, unsigned int by, unsigned int bz, unsigned int shm,
                                                void* stream, void** arg, void** extra) {
    return reg_lancia(f, gx, gy, gz, bx, by, bz, shm, stream, arg, extra);
}

__declspec(dllexport) int hipModuleLaunchCooperativeKernel(void* f, unsigned int gx, unsigned int gy, unsigned int gz,
                                                           unsigned int bx, unsigned int by, unsigned int bz,
                                                           unsigned int shm, void* stream, void** arg) {
    return reg_lancia_x(1, f, gx, gy, gz, bx, by, bz, shm, stream, arg, NULL);
}

/* hipExtModuleLaunchKernel (14 argomenti) e hipHccModuleLaunchKernel (13): prima erano inoltrati con la
 * macro a 9 argomenti e funzionavano solo perche' clang a /O2 ne faceva un salto in coda (jmp), che riusa
 * la pila del chiamante. Ora hanno il prototipo vero e l'array degli argomenti completato come gli altri
 * lanci. Le dimensioni sono in thread (globalWorkSize = griglia * blocco), come vuole la 5.7. */
typedef int (*reg_f_extmod)(void* f, unsigned int gx, unsigned int gy, unsigned int gz, unsigned int lx,
                            unsigned int ly, unsigned int lz, size_t shm, void* stream, void** arg, void** extra,
                            void* ev0, void* ev1, unsigned int flag);
typedef int (*reg_f_hccmod)(void* f, unsigned int gx, unsigned int gy, unsigned int gz, unsigned int lx,
                            unsigned int ly, unsigned int lz, size_t shm, void* stream, void** arg, void** extra,
                            void* ev0, void* ev1);
__declspec(dllexport) int hipExtModuleLaunchKernel(void* f, unsigned int gx, unsigned int gy, unsigned int gz,
                                                   unsigned int lx, unsigned int ly, unsigned int lz, size_t shm,
                                                   void* stream, void** arg, void** extra, void* ev0, void* ev1,
                                                   unsigned int flag) {
    void* locale[64];
    void** completo;
    unsigned int lds = (unsigned int)shm;
    int rc;
    ABI57_REALE(reale, reg_f_extmod, "hipExtModuleLaunchKernel");
    completo = reg_completa(f, arg, &lds, locale);
    if (arg && !completo) return hipErrorOutOfMemory;
    rc = reale(f, gx, gy, gz, lx, ly, lz, shm, stream, completo, extra, ev0, ev1, flag);
    reg_completo_libera(completo, arg, locale);
    return rc;
}

__declspec(dllexport) int hipHccModuleLaunchKernel(void* f, unsigned int gx, unsigned int gy, unsigned int gz,
                                                   unsigned int lx, unsigned int ly, unsigned int lz, size_t shm,
                                                   void* stream, void** arg, void** extra, void* ev0, void* ev1) {
    void* locale[64];
    void** completo;
    unsigned int lds = (unsigned int)shm;
    int rc;
    ABI57_REALE(reale, reg_f_hccmod, "hipHccModuleLaunchKernel");
    completo = reg_completa(f, arg, &lds, locale);
    if (arg && !completo) return hipErrorOutOfMemory;
    rc = reale(f, gx, gy, gz, lx, ly, lz, shm, stream, completo, extra, ev0, ev1);
    reg_completo_libera(completo, arg, locale);
    return rc;
}

/* hipDrvLaunchKernelEx (HIP 7, assente nella 5.7): lancio con una configurazione e una lista di attributi.
 * Disposizioni misurate sugli header 7.16: HIP_LAUNCH_CONFIG 56 byte, hipLaunchAttribute 72 (id a 0,
 * valore a 8). Attributi: Cooperative (2) -> lancio cooperativo; AccessPolicyWindow (1), Priority (8),
 * SynchronizationPolicy (3), MemSyncDomainMap/MemSyncDomain (9, 10), ClusterSchedulingPolicy (5),
 * ExtDynDataPrefetch (1024) e Ignore (0) sono suggerimenti senza effetto su gfx906. ClusterDimension (4):
 * gfx906 non ha cluster in hardware e il compilatore non emette per gfx906 istruzioni di cluster, quindi
 * un kernel gfx906 non puo' osservare il raggruppamento e il lancio si esegue normale; si controlla solo
 * che la griglia sia multipla della dimensione del cluster (come CUDA). */
typedef struct {
    unsigned int gx, gy, gz, bx, by, bz, shm;
    void* stream;
    const unsigned char* attr;
    unsigned int nattr;
} reg_launch_config;
#define REG_ATTR_DIM 72

static int reg_leggi_attributi(const unsigned char* attr, unsigned int n, unsigned int gx, unsigned int gy,
                               unsigned int gz, int* coop) {
    unsigned int k;
    *coop = 0;
    if (n && !attr) return hipErrorInvalidValue;
    for (k = 0; k < n; k++) {
        const unsigned char* a = attr + (size_t)k * REG_ATTR_DIM;
        int id = *(const int*)a;
        const unsigned int* v = (const unsigned int*)(a + 8);
        switch (id) {
            case 0: case 1: case 3: case 5: case 8: case 9: case 10: case 1024: break;
            case 2: *coop = v[0] != 0; break;
            case 4:
                if (!v[0] || !v[1] || !v[2] || gx % v[0] || gy % v[1] || gz % v[2]) return hipErrorInvalidValue;
                break;
            default: return hipErrorInvalidValue;
        }
    }
    return hipSuccess;
}

__declspec(dllexport) int hipDrvLaunchKernelEx(const reg_launch_config* c, void* f, void** arg, void** extra) {
    int coop, rc;
    if (!c || !f) return hipErrorInvalidValue;
    rc = reg_leggi_attributi(c->attr, c->nattr, c->gx, c->gy, c->gz, &coop);
    if (rc != hipSuccess) return rc;
    if (coop && extra) return hipErrorInvalidValue;
    return reg_lancia_x(coop, f, c->gx, c->gy, c->gz, c->bx, c->by, c->bz, c->shm, c->stream, arg, extra);
}

/* hipTexRefSetMipmapLevelBias/Clamp: hanno float per valore, che su x64 viaggiano nei registri XMM; la
 * macro generica (argomenti interi) li passava solo grazie al salto in coda. */
typedef int (*reg_f_tex1f)(void* tex, float a);
typedef int (*reg_f_tex2f)(void* tex, float a, float b);
__declspec(dllexport) int hipTexRefSetMipmapLevelBias(void* tex, float bias) {
    ABI57_REALE(reale, reg_f_tex1f, "hipTexRefSetMipmapLevelBias");
    return reale(tex, bias);
}
__declspec(dllexport) int hipTexRefSetMipmapLevelClamp(void* tex, float minimo, float massimo) {
    ABI57_REALE(reale, reg_f_tex2f, "hipTexRefSetMipmapLevelClamp");
    return reale(tex, minimo, massimo);
}

/* hipExtLaunchKernel: come hipLaunchKernel, con eventi facoltativi registrati prima e dopo sullo stesso
 * stream. I flag (hipExtAnyOrderLaunch) non hanno effetto con un solo motore di calcolo. */
typedef int (*reg_f_extlancio)(const void* host, reg_dim3 g, reg_dim3 b, void** arg, size_t shm, void* stream,
                               void* ev0, void* ev1, int flag);
typedef int (*reg_f_evrec)(void* ev, void* stream);
__declspec(dllexport) int hipExtLaunchKernel(const void* host, reg_dim3 griglia, reg_dim3 blocco, void** arg,
                                             size_t shm, void* stream, void* ev0, void* ev1, int flag) {
    void* f = NULL;
    int rc = reg_funzione(host, &f);
    if (rc < 0) {
        ABI57_REALE(reale, reg_f_extlancio, "hipExtLaunchKernel");
        return reale(host, griglia, blocco, arg, shm, stream, ev0, ev1, flag);
    }
    if (rc != hipSuccess) return reg_errore(rc);
    {
        ABI57_REALE(registra, reg_f_evrec, "hipEventRecord");
        if (ev0 && (rc = registra(ev0, stream)) != hipSuccess) return reg_errore(rc);
        rc = reg_lancia(f, griglia.x, griglia.y, griglia.z, blocco.x, blocco.y, blocco.z, (unsigned int)shm, stream, arg, NULL);
        if (rc == hipSuccess && ev1) rc = registra(ev1, stream);
    }
    return reg_errore(rc);
}

typedef int (*reg_f_cooplancio)(const void* host, reg_dim3 g, reg_dim3 b, void** arg, unsigned int shm, void* stream);
__declspec(dllexport) int hipLaunchCooperativeKernel(const void* host, reg_dim3 griglia, reg_dim3 blocco, void** arg,
                                                     unsigned int shm, void* stream) {
    void* f = NULL;
    int rc = reg_funzione(host, &f);
    if (rc < 0) {
        ABI57_REALE(reale, reg_f_cooplancio, "hipLaunchCooperativeKernel");
        return reale(host, griglia, blocco, arg, shm, stream);
    }
    if (rc != hipSuccess) return reg_errore(rc);
    return reg_errore(reg_lancia_x(1, f, griglia.x, griglia.y, griglia.z, blocco.x, blocco.y, blocco.z, shm, stream, arg, NULL));
}

/* hipGetLastError e simili: all'errore della runtime 5.7 si aggiunge quello dei lanci falliti dentro lo
 * shim (codice non caricabile, funzione assente), che la 5.7 non vede. PyTorch controlla ogni lancio con
 * hipGetLastError: senza questo un kernel mancante passerebbe in silenzio. */
typedef int (*reg_f_errore)(void);
__declspec(dllexport) int hipGetLastError(void) {
    int e;
    ABI57_REALE(reale, reg_f_errore, "hipGetLastError");
    e = reale();
    if (reg_ultimo_errore) { if (e == hipSuccess) e = reg_ultimo_errore; reg_ultimo_errore = hipSuccess; }
    return e;
}

__declspec(dllexport) int hipPeekAtLastError(void) {
    int e;
    ABI57_REALE(reale, reg_f_errore, "hipPeekAtLastError");
    e = reale();
    return e == hipSuccess ? reg_ultimo_errore : e;
}

/* HIP 7: come hipGetLastError (restituisce e azzera l'ultimo errore del thread). */
__declspec(dllexport) int hipExtGetLastError(void) {
    return hipGetLastError();
}

typedef struct {
    int binaryVersion, cacheModeCA;
    size_t constSizeBytes, localSizeBytes;
    int maxDynamicSharedSizeBytes, maxThreadsPerBlock, numRegs, preferredShmemCarveout, ptxVersion;
    size_t sharedSizeBytes;
} reg_attributi;
typedef int (*reg_f_attr)(int* v, int attr, void* f);
typedef int (*reg_f_attrs)(reg_attributi* a, const void* host);

__declspec(dllexport) int hipFuncGetAttributes(reg_attributi* a, const void* host) {
    static reg_f_attr attr = NULL;
    void* f = NULL;
    int rc = reg_funzione(host, &f), v = 0;
    if (rc < 0) {
        ABI57_REALE(reale, reg_f_attrs, "hipFuncGetAttributes");
        return reale(a, host);
    }
    if (rc != hipSuccess) return rc;
    if (!a) return hipErrorInvalidValue;
    if (!attr) { attr = (reg_f_attr)abi57_reale("hipFuncGetAttribute"); if (!attr) return hipErrorNotSupported; }
    memset(a, 0, sizeof(*a));
#define REG_ATTR(campo, codice, tipo) if (attr(&v, codice, f) == hipSuccess) a->campo = (tipo)v
    REG_ATTR(maxThreadsPerBlock, 0, int);
    REG_ATTR(sharedSizeBytes, 1, size_t);
    REG_ATTR(constSizeBytes, 2, size_t);
    REG_ATTR(localSizeBytes, 3, size_t);
    REG_ATTR(numRegs, 4, int);
    REG_ATTR(ptxVersion, 5, int);
    REG_ATTR(binaryVersion, 6, int);
    REG_ATTR(cacheModeCA, 7, int);
    REG_ATTR(maxDynamicSharedSizeBytes, 8, int);
    REG_ATTR(preferredShmemCarveout, 9, int);
#undef REG_ATTR
    return hipSuccess;
}

/* gfx906: niente ripartizione configurabile fra cache e memoria condivisa; per i kernel dello shim
 * gli attributi impostabili si accettano senza effetto (come fa la runtime sulle schede senza carveout). */
typedef int (*reg_f_setattr)(const void* host, int attr, int valore);
__declspec(dllexport) int hipFuncSetAttribute(const void* host, int attr, int valore) {
    void* f = NULL;
    int rc = reg_funzione(host, &f);
    if (rc < 0) {
        ABI57_REALE(reale, reg_f_setattr, "hipFuncSetAttribute");
        return reale(host, attr, valore);
    }
    return rc;
}

typedef int (*reg_f_cache)(const void* host, int config);
__declspec(dllexport) int hipFuncSetCacheConfig(const void* host, int config) {
    void* f = NULL;
    int rc = reg_funzione(host, &f);
    if (rc < 0) {
        ABI57_REALE(reale, reg_f_cache, "hipFuncSetCacheConfig");
        return reale(host, config);
    }
    return rc;
}

typedef int (*reg_f_occ)(int* n, const void* f, int blocco, size_t shm);
__declspec(dllexport) int hipOccupancyMaxActiveBlocksPerMultiprocessor(int* n, const void* host, int blocco, size_t shm) {
    void* f = NULL;
    int rc = reg_funzione(host, &f);
    if (rc < 0) {
        ABI57_REALE(reale, reg_f_occ, "hipOccupancyMaxActiveBlocksPerMultiprocessor");
        return reale(n, host, blocco, shm);
    }
    if (rc != hipSuccess) return rc;
    {
        ABI57_REALE(mod, reg_f_occ, "hipModuleOccupancyMaxActiveBlocksPerMultiprocessor");
        return mod(n, f, blocco, shm);
    }
}

typedef int (*reg_f_occ2)(int* griglia, int* blocco, const void* f, size_t shm, int limite);
__declspec(dllexport) int hipOccupancyMaxPotentialBlockSize(int* griglia, int* blocco, const void* host, size_t shm, int limite) {
    void* f = NULL;
    int rc = reg_funzione(host, &f);
    if (rc < 0) {
        ABI57_REALE(reale, reg_f_occ2, "hipOccupancyMaxPotentialBlockSize");
        return reale(griglia, blocco, host, shm, limite);
    }
    if (rc != hipSuccess) return rc;
    {
        ABI57_REALE(mod, reg_f_occ2, "hipModuleOccupancyMaxPotentialBlockSize");
        return mod(griglia, blocco, f, shm, limite);
    }
}

/* ---- variabili __device__ / __constant__ ---- */

typedef int (*reg_f_symaddr)(void** p, const void* host);
__declspec(dllexport) int hipGetSymbolAddress(void** p, const void* host) {
    int rc = reg_simbolo(host, p, NULL);
    if (rc < 0) {
        ABI57_REALE(reale, reg_f_symaddr, "hipGetSymbolAddress");
        return reale(p, host);
    }
    return rc;
}

typedef int (*reg_f_symsize)(size_t* d, const void* host);
__declspec(dllexport) int hipGetSymbolSize(size_t* d, const void* host) {
    int rc = reg_simbolo(host, NULL, d);
    if (rc < 0) {
        ABI57_REALE(reale, reg_f_symsize, "hipGetSymbolSize");
        return reale(d, host);
    }
    return rc;
}

typedef int (*reg_f_memcpy)(void* dst, const void* src, size_t n, int tipo);
typedef int (*reg_f_memcpya)(void* dst, const void* src, size_t n, int tipo, void* stream);
typedef int (*reg_f_tosym)(const void* host, const void* src, size_t n, size_t off, int tipo);
typedef int (*reg_f_fromsym)(void* dst, const void* host, size_t n, size_t off, int tipo);
typedef int (*reg_f_tosyma)(const void* host, const void* src, size_t n, size_t off, int tipo, void* stream);
typedef int (*reg_f_fromsyma)(void* dst, const void* host, size_t n, size_t off, int tipo, void* stream);
#define REG_DEFAULT 4   /* hipMemcpyDefault */

__declspec(dllexport) int hipMemcpyToSymbol(const void* host, const void* src, size_t n, size_t off, int tipo) {
    void* p = NULL; size_t d = 0;
    int rc = reg_simbolo(host, &p, &d);
    if (rc < 0) { ABI57_REALE(reale, reg_f_tosym, "hipMemcpyToSymbol"); return reale(host, src, n, off, tipo); }
    if (rc != hipSuccess) return rc;
    if (off + n > d) return hipErrorInvalidValue;
    { ABI57_REALE(cp, reg_f_memcpy, "hipMemcpy"); return cp((char*)p + off, src, n, REG_DEFAULT); }
}

__declspec(dllexport) int hipMemcpyFromSymbol(void* dst, const void* host, size_t n, size_t off, int tipo) {
    void* p = NULL; size_t d = 0;
    int rc = reg_simbolo(host, &p, &d);
    if (rc < 0) { ABI57_REALE(reale, reg_f_fromsym, "hipMemcpyFromSymbol"); return reale(dst, host, n, off, tipo); }
    if (rc != hipSuccess) return rc;
    if (off + n > d) return hipErrorInvalidValue;
    { ABI57_REALE(cp, reg_f_memcpy, "hipMemcpy"); return cp(dst, (char*)p + off, n, REG_DEFAULT); }
}

__declspec(dllexport) int hipMemcpyToSymbolAsync(const void* host, const void* src, size_t n, size_t off, int tipo, void* stream) {
    void* p = NULL; size_t d = 0;
    int rc = reg_simbolo(host, &p, &d);
    if (rc < 0) { ABI57_REALE(reale, reg_f_tosyma, "hipMemcpyToSymbolAsync"); return reale(host, src, n, off, tipo, stream); }
    if (rc != hipSuccess) return rc;
    if (off + n > d) return hipErrorInvalidValue;
    { ABI57_REALE(cp, reg_f_memcpya, "hipMemcpyAsync"); return cp((char*)p + off, src, n, REG_DEFAULT, stream); }
}

__declspec(dllexport) int hipMemcpyFromSymbolAsync(void* dst, const void* host, size_t n, size_t off, int tipo, void* stream) {
    void* p = NULL; size_t d = 0;
    int rc = reg_simbolo(host, &p, &d);
    if (rc < 0) { ABI57_REALE(reale, reg_f_fromsyma, "hipMemcpyFromSymbolAsync"); return reale(dst, host, n, off, tipo, stream); }
    if (rc != hipSuccess) return rc;
    if (off + n > d) return hipErrorInvalidValue;
    { ABI57_REALE(cp, reg_f_memcpya, "hipMemcpyAsync"); return cp(dst, (char*)p + off, n, REG_DEFAULT, stream); }
}

/* ---- caricamento esplicito di moduli: stessa conversione v6 -> v5 e bundle ---- */

__declspec(dllexport) int hipModuleLoadData(void** modulo, const void* immagine) {
    return reg_carica_immagine(modulo, immagine);
}

typedef int (*reg_f_loadex)(void** modulo, const void* immagine, unsigned int n, void* opz, void** val);
__declspec(dllexport) int hipModuleLoadDataEx(void** modulo, const void* immagine, unsigned int n, void* opz, void** val) {
    (void)n; (void)opz; (void)val;    /* la 5.7 ignora le opzioni del JIT */
    return reg_carica_immagine(modulo, immagine);
}

__declspec(dllexport) int hipModuleLoad(void** modulo, const char* percorso) {
    HANDLE h;
    LARGE_INTEGER dim;
    unsigned char* buf;
    DWORD letti = 0;
    int rc;
    if (!percorso || !modulo) return hipErrorInvalidValue;
    h = CreateFileA(percorso, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 301;     /* hipErrorFileNotFound */
    if (!GetFileSizeEx(h, &dim) || dim.QuadPart <= 0 || dim.QuadPart > 0x7FFFFFFF) { CloseHandle(h); return hipErrorInvalidImage; }
    buf = (unsigned char*)malloc((size_t)dim.QuadPart);
    if (!buf) { CloseHandle(h); return hipErrorOutOfMemory; }
    if (!ReadFile(h, buf, (DWORD)dim.QuadPart, &letti, NULL) || letti != (DWORD)dim.QuadPart) { free(buf); CloseHandle(h); return hipErrorInvalidImage; }
    CloseHandle(h);
    rc = reg_carica_immagine(modulo, buf);
    if (rc != hipSuccess) free(buf);   /* se caricato resta vivo, come sopra */
    return rc;
}

/* ---- chiusura del processo ----
 * Alcuni distruttori statici (per esempio la cache dei kernel del jiterator di PyTorch) chiedono il
 * dispositivo corrente mentre il processo termina, quando la runtime del driver e' gia' staccata: la
 * chiamata la fa terminare con un fail-fast. In chiusura risponde lo shim, con l'ultimo dispositivo che
 * la runtime ha dato a questo thread (0 se non l'ha mai chiesto). */
static __declspec(thread) int reg_ultimo_dispositivo = 0;
typedef int (*reg_f_getdev)(int* d);
__declspec(dllexport) int hipGetDevice(int* d) {
    int rc;
    if (!d) return hipErrorInvalidValue;
    if (marconi_in_chiusura()) { *d = reg_ultimo_dispositivo; return hipSuccess; }
    {
        ABI57_REALE(reale, reg_f_getdev, "hipGetDevice");
        rc = reale(d);
    }
    if (rc == hipSuccess) reg_ultimo_dispositivo = *d;
    return rc;
}

typedef int (*reg_f_setdev)(int d);
__declspec(dllexport) int hipSetDevice(int d) {
    int rc;
    if (marconi_in_chiusura()) { reg_ultimo_dispositivo = d; return hipSuccess; }
    {
        ABI57_REALE(reale, reg_f_setdev, "hipSetDevice");
        rc = reale(d);
    }
    if (rc == hipSuccess) reg_ultimo_dispositivo = d;
    return rc;
}

/* ---- eventi ---- */

typedef int (*reg_f_record)(void* evento, void* stream);
/* HIP 7: i flag (hipEventRecordDefault 0, hipEventRecordExternal 1) contano solo nella cattura dei
 * grafi; la 5.7 non ha la funzione e si registra l'evento normalmente. */
__declspec(dllexport) int hipEventRecordWithFlags(void* evento, void* stream, unsigned int flag) {
    ABI57_REALE(reale, reg_f_record, "hipEventRecord");
    if (flag > 1) return hipErrorInvalidValue;
    return reale(evento, stream);
}
