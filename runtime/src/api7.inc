/*
 * api7.inc - API HIP 6.x/7.x che la runtime 5.7 del driver non ha, costruite sui suoi mattoni.
 * 2 Solution, 13 nov 2026. Incluso da shim_main.c dopo abi57.inc e registrazione.inc.
 *
 * Disposizioni e valori misurati sugli header HIP 7.16 di ROCm 10.1 (clang -fdump-record-layouts e
 * programma che stampa gli enumeratori): vedi attrezzi\censimento_rocm10 nella macchina di sviluppo.
 *
 * Gruppo 2a: lanci e ricerche.
 *   hipLaunchKernelExC, hipGetFuncBySymbol, hipGetDriverEntryPoint(_spt), hipGetProcAddress_spt,
 *   hipStreamGetId, hipStreamGetAttribute/SetAttribute/CopyAttributes (con hipStreamDestroy che li
 *   dimentica), hipMemsetD2D8/16/32(Async), hipGetDevicePropertiesR0000, hipChooseDeviceR0000/R0600,
 *   hipDeviceGetTexture1DLinearMaxWidth, hipOccupancyAvailableDynamicSMemPerBlock,
 *   hipOccupancyMaxPotentialClusterSize, hipOccupancyMaxActiveClusters.
 *
 * Cluster di blocchi: gfx906 non li ha in hardware, e per lo stesso motivo il compilatore non emette
 * per gfx906 nessuna istruzione di cluster (sincronizzazione di cluster, memoria condivisa distribuita).
 * Un kernel gfx906 non puo' quindi osservare il raggruppamento: un lancio con dimensione di cluster si
 * esegue come lancio normale, con lo stesso risultato. Si controlla solo che la griglia sia multipla
 * della dimensione del cluster, come fa CUDA.
 */

/* ---------- lancio con configurazione (runtime) ---------- */

/* hipLaunchConfig_t, 56 byte: griglia (dim3) a 0, blocco a 12, memoria dinamica (size_t) a 24,
 * stream a 32, attributi a 40, numero di attributi a 48. */
typedef struct {
    reg_dim3 griglia, blocco;
    size_t shm;
    void* stream;
    const unsigned char* attr;
    unsigned int nattr;
} api7_launch_config;

__declspec(dllexport) int hipLaunchKernelExC(const api7_launch_config* c, const void* host, void** arg) {
    int coop, rc;
    if (!c || !host) return reg_errore(hipErrorInvalidValue);
    rc = reg_leggi_attributi(c->attr, c->nattr, c->griglia.x, c->griglia.y, c->griglia.z, &coop);
    if (rc != hipSuccess) return reg_errore(rc);
    if (coop) return hipLaunchCooperativeKernel(host, c->griglia, c->blocco, arg, (unsigned int)c->shm, c->stream);
    return hipLaunchKernel(host, c->griglia, c->blocco, arg, c->shm, c->stream);
}

/* hipGetFuncBySymbol: la funzione di dispositivo di un kernel registrato dallo shim. */
__declspec(dllexport) int hipGetFuncBySymbol(void** f, const void* host) {
    int rc;
    if (!f || !host) return hipErrorInvalidValue;
    rc = reg_funzione(host, f);
    if (rc < 0) return hipErrorInvalidDeviceFunction;
    return rc;
}

/* ---------- ricerca dei simboli ---------- */

__declspec(dllexport) int hipGetProcAddress(const char* simbolo, void** pfn, int versione, unsigned long long flags,
                                            int* stato_simbolo);

/* hipGetDriverEntryPoint: come hipGetProcAddress (lo shim prima, poi la runtime del driver).
 * Stato: 0 trovato, 1 simbolo assente. */
__declspec(dllexport) int hipGetDriverEntryPoint(const char* simbolo, void** pfn, unsigned long long flags, int* stato) {
    return hipGetProcAddress(simbolo, pfn, 0, flags, stato);
}
__declspec(dllexport) int hipGetDriverEntryPoint_spt(const char* simbolo, void** pfn, unsigned long long flags, int* stato) {
    return hipGetProcAddress(simbolo, pfn, 0, flags, stato);
}
__declspec(dllexport) int hipGetProcAddress_spt(const char* simbolo, void** pfn, int versione, unsigned long long flags,
                                                int* stato) {
    return hipGetProcAddress(simbolo, pfn, versione, flags, stato);
}

/* ---------- attributi e identificativo degli stream ----------
 * La 5.7 non ha attributi di stream: lo shim li conserva per stream. La priorita' si legge dal driver
 * (fissata alla creazione); impostarla vale come suggerimento conservato, come la finestra di accesso
 * e il dominio di sincronizzazione, che su gfx906 non hanno effetto. hipStreamGetId da' un numero unico
 * per la vita del processo (0 lo stream nullo, 1 quello per thread). */
#define API7_ATTR_VAL 64
#define API7_STREAM_MAX 4096
typedef struct {
    void* stream;
    unsigned long long id;
    unsigned int impostati;            /* bit per id 1, 3, 8, 9, 10 */
    unsigned char val[5][API7_ATTR_VAL];
} api7_stream;
static api7_stream api7_stream_tab[API7_STREAM_MAX];
static SRWLOCK api7_stream_lock = SRWLOCK_INIT;
static unsigned long long api7_prossimo_id = 16;

static int api7_slot_attr(int id) {
    switch (id) { case 1: return 0; case 3: return 1; case 8: return 2; case 9: return 3; case 10: return 4; }
    return -1;
}

/* voce dello stream, creata se crea != 0; da chiamare col lock esclusivo */
static api7_stream* api7_voce(void* s, int crea) {
    int k, libero = -1;
    for (k = 0; k < API7_STREAM_MAX; k++) {
        if (api7_stream_tab[k].stream == s && api7_stream_tab[k].id) return &api7_stream_tab[k];
        if (libero < 0 && !api7_stream_tab[k].id) libero = k;
    }
    if (!crea || libero < 0) return NULL;
    memset(&api7_stream_tab[libero], 0, sizeof(api7_stream));
    api7_stream_tab[libero].stream = s;
    api7_stream_tab[libero].id = api7_prossimo_id++;
    return &api7_stream_tab[libero];
}

__declspec(dllexport) int hipStreamGetId(void* s, unsigned long long* id) {
    api7_stream* v;
    if (!id) return hipErrorInvalidValue;
    if (s == NULL) { *id = 0; return hipSuccess; }
    if (s == (void*)2) { *id = 1; return hipSuccess; }   /* hipStreamPerThread */
    AcquireSRWLockExclusive(&api7_stream_lock);
    v = api7_voce(s, 1);
    if (v) *id = v->id;
    ReleaseSRWLockExclusive(&api7_stream_lock);
    return v ? hipSuccess : hipErrorOutOfMemory;
}

typedef int (*api7_f_prio)(void* s, int* p);
__declspec(dllexport) int hipStreamGetAttribute(void* s, int id, void* valore) {
    int slot = api7_slot_attr(id), trovato = 0;
    api7_stream* v;
    if (!valore) return hipErrorInvalidValue;
    if (slot < 0) return hipErrorInvalidValue;
    memset(valore, 0, API7_ATTR_VAL);
    AcquireSRWLockShared(&api7_stream_lock);
    v = api7_voce(s, 0);
    if (v && (v->impostati & (1u << slot))) { memcpy(valore, v->val[slot], API7_ATTR_VAL); trovato = 1; }
    ReleaseSRWLockShared(&api7_stream_lock);
    if (trovato) return hipSuccess;
    if (id == 8) {                                  /* priorita': quella vera dello stream */
        ABI57_REALE(prio, api7_f_prio, "hipStreamGetPriority");
        return prio(s, (int*)valore);
    }
    if (id == 3) *(int*)valore = 1;                 /* hipSyncPolicyAuto */
    return hipSuccess;
}

__declspec(dllexport) int hipStreamSetAttribute(void* s, int id, const void* valore) {
    int slot = api7_slot_attr(id);
    api7_stream* v;
    if (!valore || slot < 0) return hipErrorInvalidValue;
    AcquireSRWLockExclusive(&api7_stream_lock);
    v = api7_voce(s, 1);
    if (v) { memcpy(v->val[slot], valore, API7_ATTR_VAL); v->impostati |= 1u << slot; }
    ReleaseSRWLockExclusive(&api7_stream_lock);
    return v ? hipSuccess : hipErrorOutOfMemory;
}

__declspec(dllexport) int hipStreamCopyAttributes(void* dst, void* src) {
    api7_stream *a, *b;
    int rc = hipSuccess;
    if (dst == src) return hipSuccess;
    AcquireSRWLockExclusive(&api7_stream_lock);
    a = api7_voce(src, 0);
    if (a && a->impostati) {
        unsigned long long id;
        b = api7_voce(dst, 1);
        if (!b) rc = hipErrorOutOfMemory;
        else { id = b->id; a = api7_voce(src, 0); memcpy(b->val, a->val, sizeof(b->val)); b->impostati = a->impostati; b->id = id; }
    } else if ((b = api7_voce(dst, 0)) != NULL) {
        b->impostati = 0;
    }
    ReleaseSRWLockExclusive(&api7_stream_lock);
    return rc;
}

/* hipStreamDestroy: prima di inoltrare, lo shim dimentica attributi e identificativo dello stream
 * (un nuovo stream puo' riavere lo stesso indirizzo). */
typedef int (*api7_f_sdestroy)(void* s);
__declspec(dllexport) int hipStreamDestroy(void* s) {
    api7_stream* v;
    if (marconi_in_chiusura()) return marconi_esito_in_chiusura("hipStreamDestroy");
    {
        ABI57_REALE(reale, api7_f_sdestroy, "hipStreamDestroy");
        AcquireSRWLockExclusive(&api7_stream_lock);
        v = api7_voce(s, 0);
        if (v) memset(v, 0, sizeof(api7_stream));
        ReleaseSRWLockExclusive(&api7_stream_lock);
        return reale(s);
    }
}

/* ---------- memset 2D a 8, 16 e 32 bit ----------
 * 8 bit: hipMemset2D(Async) della 5.7. 16 e 32: una chiamata hipMemsetD16/D32(Async) se le righe sono
 * contigue, altrimenti una per riga. */
typedef int (*api7_f_ms2d)(void* d, size_t pitch, int v, size_t w, size_t h);
typedef int (*api7_f_ms2da)(void* d, size_t pitch, int v, size_t w, size_t h, void* s);
typedef int (*api7_f_msd16)(void* d, unsigned short v, size_t n);
typedef int (*api7_f_msd16a)(void* d, unsigned short v, size_t n, void* s);
typedef int (*api7_f_msd32)(void* d, int v, size_t n);
typedef int (*api7_f_msd32a)(void* d, int v, size_t n, void* s);

__declspec(dllexport) int hipMemsetD2D8(void* d, size_t pitch, unsigned char v, size_t w, size_t h) {
    ABI57_REALE(reale, api7_f_ms2d, "hipMemset2D");
    return reale(d, pitch, v, w, h);
}
__declspec(dllexport) int hipMemsetD2D8Async(void* d, size_t pitch, unsigned char v, size_t w, size_t h, void* s) {
    ABI57_REALE(reale, api7_f_ms2da, "hipMemset2DAsync");
    return reale(d, pitch, v, w, h, s);
}

static int api7_controlla_2d(void* d, size_t pitch, size_t w, size_t h, size_t el) {
    if (!d && w && h) return hipErrorInvalidValue;
    if (h > 1 && w * el > pitch) return hipErrorInvalidValue;
    return hipSuccess;
}

__declspec(dllexport) int hipMemsetD2D16(void* d, size_t pitch, unsigned short v, size_t w, size_t h) {
    size_t r; int rc;
    ABI57_REALE(reale, api7_f_msd16, "hipMemsetD16");
    if ((rc = api7_controlla_2d(d, pitch, w, h, 2)) != hipSuccess) return rc;
    if (pitch == w * 2 || h <= 1) return reale(d, v, w * h);
    for (r = 0; r < h && rc == hipSuccess; r++) rc = reale((char*)d + r * pitch, v, w);
    return rc;
}
__declspec(dllexport) int hipMemsetD2D16Async(void* d, size_t pitch, unsigned short v, size_t w, size_t h, void* s) {
    size_t r; int rc;
    ABI57_REALE(reale, api7_f_msd16a, "hipMemsetD16Async");
    if ((rc = api7_controlla_2d(d, pitch, w, h, 2)) != hipSuccess) return rc;
    if (pitch == w * 2 || h <= 1) return reale(d, v, w * h, s);
    for (r = 0; r < h && rc == hipSuccess; r++) rc = reale((char*)d + r * pitch, v, w, s);
    return rc;
}
__declspec(dllexport) int hipMemsetD2D32(void* d, size_t pitch, unsigned int v, size_t w, size_t h) {
    size_t r; int rc;
    ABI57_REALE(reale, api7_f_msd32, "hipMemsetD32");
    if ((rc = api7_controlla_2d(d, pitch, w, h, 4)) != hipSuccess) return rc;
    if (pitch == w * 4 || h <= 1) return reale(d, (int)v, w * h);
    for (r = 0; r < h && rc == hipSuccess; r++) rc = reale((char*)d + r * pitch, (int)v, w);
    return rc;
}
__declspec(dllexport) int hipMemsetD2D32Async(void* d, size_t pitch, unsigned int v, size_t w, size_t h, void* s) {
    size_t r; int rc;
    ABI57_REALE(reale, api7_f_msd32a, "hipMemsetD32Async");
    if ((rc = api7_controlla_2d(d, pitch, w, h, 4)) != hipSuccess) return rc;
    if (pitch == w * 4 || h <= 1) return reale(d, (int)v, w * h, s);
    for (r = 0; r < h && rc == hipSuccess; r++) rc = reale((char*)d + r * pitch, (int)v, w, s);
    return rc;
}

/* ---------- proprieta' del dispositivo nelle varianti vecchie ----------
 * hipDeviceProp_tR0000 (hip_deprecated.h di ROCm 10.1) e' identica, campo per campo, alla
 * hipDeviceProp_t della 5.7 (792 byte, misurato): si passa al driver cosi' com'e'. */
typedef int (*api7_f_props)(void* p, int dev);
typedef int (*api7_f_choose)(int* dev, const void* p);
__declspec(dllexport) int hipGetDevicePropertiesR0000(void* p, int dev) {
    ABI57_REALE(reale, api7_f_props, "hipGetDeviceProperties");
    if (!p) return hipErrorInvalidValue;
    return reale(p, dev);
}
__declspec(dllexport) int hipChooseDeviceR0000(int* dev, const void* p) {
    ABI57_REALE(reale, api7_f_choose, "hipChooseDevice");
    if (!dev || !p) return hipErrorInvalidValue;
    return reale(dev, p);
}

/* R0600: il kit supporta soltanto schede gfx906 (la verifica dell'installatore si ferma sulle altre),
 * quindi tutti i dispositivi visibili sono equivalenti e si sceglie il primo, come fa la scelta della
 * runtime quando i requisiti coincidono. */
typedef int (*api7_f_count)(int* n);
__declspec(dllexport) int hipChooseDeviceR0600(int* dev, const void* richiesta) {
    int n = 0;
    ABI57_REALE(conta, api7_f_count, "hipGetDeviceCount");
    if (!dev || !richiesta) return hipErrorInvalidValue;
    if (conta(&n) != hipSuccess || n <= 0) return 100;   /* hipErrorNoDevice */
    *dev = 0;
    return hipSuccess;
}

/* ---------- texture 1D lineari ---------- */
typedef int (*api7_f_attr)(int* v, int attr, int dev);
/* La 5.7 non risponde a hipDeviceAttributeMaxTexture1DLinear (41) con hipDeviceGetAttribute, ma lo
 * scrive nella sua hipDeviceProp_t (campo maxTexture1DLinear, offset 664 su 792 byte, misurato). */
__declspec(dllexport) int hipDeviceGetTexture1DLinearMaxWidth(size_t* w, const void* desc, int dev) {
    unsigned char pr[792 + 64];
    int v = 0, rc;
    ABI57_REALE(attr, api7_f_attr, "hipDeviceGetAttribute");
    if (!w || !desc) return hipErrorInvalidValue;
    rc = attr(&v, 41, dev);
    if (rc != hipSuccess || v <= 0) {
        rc = hipGetDevicePropertiesR0000(pr, dev);
        if (rc != hipSuccess) return rc;
        memcpy(&v, pr + 664, sizeof(int));
        if (v <= 0) return hipErrorNotSupported;
    }
    *w = (size_t)v;
    return hipSuccess;
}

/* ---------- occupazione ---------- */
typedef struct { int binaryVersion, cacheModeCA; size_t constSizeBytes, localSizeBytes;
                 int maxDynamicSharedSizeBytes, maxThreadsPerBlock, numRegs, preferredShmemCarveout, ptxVersion;
                 size_t sharedSizeBytes; } api7_func_attr;   /* hipFuncAttributes, 56 byte */

static int api7_attr_disp(int attr, int* v) {
    int d = 0, rc;
    ABI57_REALE(leggi, api7_f_attr, "hipDeviceGetAttribute");
    rc = hipGetDevice(&d);
    if (rc != hipSuccess) return rc;
    return leggi(v, attr, d);
}

__declspec(dllexport) int hipOccupancyAvailableDynamicSMemPerBlock(size_t* dim, const void* host, int blocchi, int thread) {
    api7_func_attr fa;
    int per_cu = 0, per_blocco = 0, rc;
    long long disp;
    if (!dim || !host || blocchi <= 0 || thread <= 0) return hipErrorInvalidValue;
    memset(&fa, 0, sizeof(fa));
    rc = hipFuncGetAttributes((reg_attributi*)&fa, host);
    if (rc != hipSuccess) return rc;
    if (fa.maxThreadsPerBlock && thread > fa.maxThreadsPerBlock) { *dim = 0; return hipSuccess; }
    if ((rc = api7_attr_disp(10002, &per_cu)) != hipSuccess) return rc;      /* LDS per CU */
    if ((rc = api7_attr_disp(74, &per_blocco)) != hipSuccess) return rc;     /* LDS per blocco */
    disp = (long long)per_cu / blocchi;
    if (disp > per_blocco) disp = per_blocco;
    disp -= (long long)fa.sharedSizeBytes;
    *dim = disp > 0 ? (size_t)disp : 0;
    return hipSuccess;
}

/* dimensione del cluster negli attributi della configurazione (1 se assente) */
static unsigned int api7_cluster(const api7_launch_config* c) {
    unsigned int k;
    for (k = 0; c->attr && k < c->nattr; k++) {
        const unsigned char* a = c->attr + (size_t)k * REG_ATTR_DIM;
        if (*(const int*)a == 4) {
            const unsigned int* v = (const unsigned int*)(a + 8);
            return (v[0] ? v[0] : 1) * (v[1] ? v[1] : 1) * (v[2] ? v[2] : 1);
        }
    }
    return 1;
}

static int api7_blocchi_attivi(const void* host, const api7_launch_config* c, int* totale) {
    int per_cu = 0, cu = 0, rc;
    unsigned int thread = c->blocco.x * c->blocco.y * c->blocco.z;
    rc = hipOccupancyMaxActiveBlocksPerMultiprocessor(&per_cu, host, (int)thread, c->shm);
    if (rc != hipSuccess) return rc;
    if ((rc = api7_attr_disp(63, &cu)) != hipSuccess) return rc;           /* numero di CU */
    *totale = per_cu * cu;
    return hipSuccess;
}

/* massimo 8 blocchi per cluster (il limite portabile di CUDA), entro i blocchi che stanno sulla GPU */
__declspec(dllexport) int hipOccupancyMaxPotentialClusterSize(int* dim, const void* host, const api7_launch_config* c) {
    int tot = 0, rc;
    if (!dim || !host || !c) return hipErrorInvalidValue;
    rc = api7_blocchi_attivi(host, c, &tot);
    if (rc != hipSuccess) return rc;
    *dim = tot < 8 ? (tot > 0 ? tot : 1) : 8;
    return hipSuccess;
}

__declspec(dllexport) int hipOccupancyMaxActiveClusters(int* n, const void* host, const api7_launch_config* c) {
    int tot = 0, rc;
    if (!n || !host || !c) return hipErrorInvalidValue;
    rc = api7_blocchi_attivi(host, c, &tot);
    if (rc != hipSuccess) return rc;
    *n = tot / (int)api7_cluster(c);
    return hipSuccess;
}
