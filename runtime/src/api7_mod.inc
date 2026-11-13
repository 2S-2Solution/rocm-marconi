/*
 * api7_mod.inc - gruppo 2b: moduli, librerie, kernel e link JIT di HIP 6.x/7.x sulla runtime 5.7.
 * 2 Solution, 13 nov 2026. Incluso da shim_main.c dopo api7.inc.
 *
 * Le API a libreria di HIP 7 (hipLibrary_t, hipKernel_t) sono un involucro sulle API a moduli della 5.7:
 * una libreria e' un modulo caricato con hipModuleLoadData, un kernel e' una funzione ottenuta con
 * hipModuleGetFunction, con in piu' l'elenco dei suoi kernel (gia' letto dai metadati in reg_kinfo).
 * Il link JIT usa hiprtcLink* di hipRTC 5.7, che la 5.7 del driver non espone come hipLink*.
 */

#ifndef hipErrorNotFound
#define hipErrorNotFound      500
#endif
#ifndef hipErrorFileNotFound
#define hipErrorFileNotFound  301
#endif

/* ---- elenco dei kernel di un modulo (da reg_kinfo_lista) ---- */

static unsigned int api7_conta_kernel(void* modulo) {
    reg_kinfo* k;
    unsigned int n = 0;
    AcquireSRWLockShared(&reg_kinfo_lock);
    for (k = reg_kinfo_lista; k; k = k->succ)
        if (k->modulo == modulo) n++;
    ReleaseSRWLockShared(&reg_kinfo_lock);
    return n;
}

__declspec(dllexport) int hipModuleGetFunctionCount(unsigned int* count, void* modulo) {
    if (!count || !modulo) return hipErrorInvalidValue;
    *count = api7_conta_kernel(modulo);
    return hipSuccess;
}

/* hipModuleLoadFatBinary: carica da un bundle/fat binary, come hipModuleLoadData (reg_carica_immagine
 * accetta ELF, bundle clang e CCOB compressi). */
__declspec(dllexport) int hipModuleLoadFatBinary(void** modulo, const void* fatbin) {
    return reg_carica_immagine(modulo, fatbin);
}

/* hipModuleUnload: toglie dalla lista i kernel del modulo (gli handle hipKernel_t che vi puntano non
 * vanno piu' usati) e inoltra alla 5.7. */
typedef int (*api7_f_unload)(void* modulo);
__declspec(dllexport) int hipModuleUnload(void* modulo) {
    reg_kinfo **pp, *k;
    if (marconi_in_chiusura()) return marconi_esito_in_chiusura("hipModuleUnload");
    AcquireSRWLockExclusive(&reg_kinfo_lock);
    pp = &reg_kinfo_lista;
    while ((k = *pp) != NULL) {
        if (k->modulo == modulo) { *pp = k->succ; free(k->nome); free(k->tipi); free(k->off); free(k->sz); free(k); }
        else pp = &k->succ;
    }
    ReleaseSRWLockExclusive(&reg_kinfo_lock);
    { ABI57_REALE(reale, api7_f_unload, "hipModuleUnload"); return reale(modulo); }
}

/* ---- librerie e kernel (HIP 7) ---- */

typedef struct { void* modulo; int proprio; } api7_library;   /* proprio: il buffer dell'immagine e' nostro */
typedef struct { api7_library* lib; void* func; reg_kinfo* info; } api7_kernel;

typedef int (*api7_f_getfunc)(void** f, void* modulo, const char* nome);
typedef int (*api7_f_global)(void** dptr, size_t* dim, void* modulo, const char* nome);

static int api7_crea_kernel(api7_library* lib, const char* nome, api7_kernel** out) {
    void* f = NULL;
    api7_kernel* k;
    ABI57_REALE(getf, api7_f_getfunc, "hipModuleGetFunction");
    int rc = getf(&f, lib->modulo, nome);
    if (rc != hipSuccess) return rc == hipErrorNotSupported ? rc : hipErrorInvalidDeviceFunction;
    reg_associa(f, lib->modulo, nome);
    k = (api7_kernel*)calloc(1, sizeof(api7_kernel));
    if (!k) return hipErrorOutOfMemory;
    k->lib = lib; k->func = f; k->info = reg_kinfo_trova(lib->modulo, nome);
    *out = k;
    return hipSuccess;
}

__declspec(dllexport) int hipLibraryLoadData(void** library, const void* code, void* jitOpt, void** jitVal,
                                             unsigned int nJit, void* libOpt, void** libVal, unsigned int nLib) {
    api7_library* lib;
    int rc;
    (void)jitOpt; (void)jitVal; (void)nJit; (void)libOpt; (void)libVal; (void)nLib;   /* la 5.7 ignora le opzioni */
    if (!library || !code) return hipErrorInvalidValue;
    lib = (api7_library*)calloc(1, sizeof(api7_library));
    if (!lib) return hipErrorOutOfMemory;
    rc = reg_carica_immagine(&lib->modulo, code);
    if (rc != hipSuccess) { free(lib); return rc; }
    *library = lib;
    return hipSuccess;
}

__declspec(dllexport) int hipLibraryLoadFromFile(void** library, const char* file, void* jitOpt, void** jitVal,
                                                 unsigned int nJit, void* libOpt, void** libVal, unsigned int nLib) {
    api7_library* lib;
    void* modulo = NULL;
    int rc;
    (void)jitOpt; (void)jitVal; (void)nJit; (void)libOpt; (void)libVal; (void)nLib;
    if (!library || !file) return hipErrorInvalidValue;
    rc = hipModuleLoad(&modulo, file);               /* legge il file e carica (resta vivo) */
    if (rc != hipSuccess) return rc;
    lib = (api7_library*)calloc(1, sizeof(api7_library));
    if (!lib) return hipErrorOutOfMemory;
    lib->modulo = modulo;
    *library = lib;
    return hipSuccess;
}

__declspec(dllexport) int hipLibraryUnload(void* library) {
    api7_library* lib = (api7_library*)library;
    if (!lib) return hipErrorInvalidValue;
    hipModuleUnload(lib->modulo);
    free(lib);
    return hipSuccess;
}

__declspec(dllexport) int hipLibraryGetKernel(void** pKernel, void* library, const char* nome) {
    api7_library* lib = (api7_library*)library;
    if (!pKernel || !lib || !nome) return hipErrorInvalidValue;
    return api7_crea_kernel(lib, nome, (api7_kernel**)pKernel);
}

__declspec(dllexport) int hipLibraryGetKernelCount(unsigned int* count, void* library) {
    api7_library* lib = (api7_library*)library;
    if (!count || !lib) return hipErrorInvalidValue;
    *count = api7_conta_kernel(lib->modulo);
    return hipSuccess;
}

__declspec(dllexport) int hipLibraryEnumerateKernels(void** kernels, unsigned int numKernels, void* library) {
    api7_library* lib = (api7_library*)library;
    api7_f_getfunc getf = (api7_f_getfunc)abi57_reale("hipModuleGetFunction");
    reg_kinfo* k;
    unsigned int i = 0;
    if (!kernels || !lib) return hipErrorInvalidValue;
    if (!getf) return hipErrorNotSupported;
    AcquireSRWLockShared(&reg_kinfo_lock);
    for (k = reg_kinfo_lista; k && i < numKernels; k = k->succ) {
        api7_kernel* kk;
        void* f = NULL;
        if (k->modulo != lib->modulo) continue;
        kk = (api7_kernel*)calloc(1, sizeof(api7_kernel));
        if (kk && getf(&f, lib->modulo, k->nome) == hipSuccess) {
            kk->lib = lib; kk->func = f; kk->info = k;
            kernels[i++] = kk;
        } else { free(kk); }
    }
    ReleaseSRWLockShared(&reg_kinfo_lock);
    return hipSuccess;
}

__declspec(dllexport) int hipLibraryGetGlobal(void** dptr, size_t* bytes, void* library, const char* nome) {
    api7_library* lib = (api7_library*)library;
    ABI57_REALE(global, api7_f_global, "hipModuleGetGlobal");
    if (!dptr || !lib || !nome) return hipErrorInvalidValue;
    return global(dptr, bytes, lib->modulo, nome);
}

/* managed e global coincidono su questa piattaforma (nessuna memoria gestita separata nei moduli) */
__declspec(dllexport) int hipLibraryGetManaged(void** dptr, size_t* bytes, void* library, const char* nome) {
    return hipLibraryGetGlobal(dptr, bytes, library, nome);
}

__declspec(dllexport) int hipKernelGetFunction(void** pFunc, void* kernel) {
    api7_kernel* k = (api7_kernel*)kernel;
    if (!pFunc || !k) return hipErrorInvalidValue;
    *pFunc = k->func;
    return hipSuccess;
}

__declspec(dllexport) int hipKernelGetLibrary(void** library, void* kernel) {
    api7_kernel* k = (api7_kernel*)kernel;
    if (!library || !k) return hipErrorInvalidValue;
    *library = k->lib;
    return hipSuccess;
}

__declspec(dllexport) int hipKernelGetName(const char** nome, void* kernel) {
    api7_kernel* k = (api7_kernel*)kernel;
    if (!nome || !k) return hipErrorInvalidValue;
    *nome = k->info ? k->info->nome : NULL;
    return k->info ? hipSuccess : hipErrorNotFound;
}

typedef int (*api7_f_fattr)(int* v, int attr, void* f);
__declspec(dllexport) int hipKernelGetAttribute(int* pi, int attrib, void* kernel, int dev) {
    api7_kernel* k = (api7_kernel*)kernel;
    (void)dev;
    ABI57_REALE(attr, api7_f_fattr, "hipFuncGetAttribute");
    if (!pi || !k) return hipErrorInvalidValue;
    return attr(pi, attrib, k->func);
}

typedef int (*api7_f_fsetattr)(void* f, int attr, int v);
__declspec(dllexport) int hipKernelSetAttribute(int attrib, int value, void* kernel, int dev) {
    api7_kernel* k = (api7_kernel*)kernel;
    (void)dev;
    ABI57_REALE(set, api7_f_fsetattr, "hipFuncSetAttribute");
    if (!k) return hipErrorInvalidValue;
    return set(k->func, attrib, value);
}

/* offset e dimensione del parametro esplicito di indice paramIndex (contati fra gli argomenti 'e'). */
__declspec(dllexport) int hipKernelGetParamInfo(void* kernel, size_t paramIndex, size_t* off, size_t* sz) {
    api7_kernel* k = (api7_kernel*)kernel;
    int a, e = 0;
    if (!k || !k->info) return hipErrorInvalidValue;
    for (a = 0; a < k->info->n; a++) {
        if (k->info->tipi[a] != 'e') continue;
        if ((size_t)e == paramIndex) {
            if (off) *off = k->info->off[a];
            if (sz) *sz = k->info->sz[a];
            return hipSuccess;
        }
        e++;
    }
    return hipErrorInvalidValue;
}

/* ---- link JIT (hipLink*) sopra hiprtcLink* di hipRTC 5.7 ----
 * hipRTC 5.7 espone hiprtcLinkCreate/AddData/AddFile/Complete/Destroy, che fanno il collegamento di piu'
 * code object in uno solo. Le opzioni JIT di HIP non si traducono una a una in quelle di hipRTC, quindi
 * si crea un link senza opzioni (comportamento predefinito). hipJitInputType: 0 cubin/co, 1 ptx/llvm-ir;
 * si passano a hiprtc come LLVM_BITCODE (0) o come immagine gia' compilata, che e' l'uso comune. */
static HMODULE api7_hiprtc(void) {
    static HMODULE h = NULL;
    if (!h) {
        h = GetModuleHandleA("hiprtc0507.dll");
        if (!h) h = LoadLibraryA("hiprtc0507.dll");
    }
    return h;
}
typedef int (*api7_f_lcreate)(unsigned int n, void* opt, void** val, void** out);
typedef int (*api7_f_ladd)(void* state, int type, void* data, size_t size, const char* name, unsigned int n, void* opt, void** val);
typedef int (*api7_f_laddf)(void* state, int type, const char* path, unsigned int n, void* opt, void** val);
typedef int (*api7_f_lcomplete)(void* state, void** bin, size_t* size);
typedef int (*api7_f_ldestroy)(void* state);

#define API7_HIPRTC(var, tipo, nome) \
    HMODULE h_ = api7_hiprtc(); tipo var; \
    if (!h_) return hipErrorNotSupported; \
    var = (tipo)GetProcAddress(h_, nome); \
    if (!var) return hipErrorNotSupported;

__declspec(dllexport) int hipLinkCreate(unsigned int n, void* opt, void** val, void** out) {
    (void)n; (void)opt; (void)val;
    { API7_HIPRTC(crea, api7_f_lcreate, "hiprtcLinkCreate");
      if (!out) return hipErrorInvalidValue;
      return crea(0, NULL, NULL, out) == 0 ? hipSuccess : hipErrorNotSupported; }
}
__declspec(dllexport) int hipLinkAddData(void* state, int type, void* data, size_t size, const char* name,
                                         unsigned int n, void* opt, void** val) {
    (void)n; (void)opt; (void)val;
    { API7_HIPRTC(add, api7_f_ladd, "hiprtcLinkAddData");
      if (!state || !data) return hipErrorInvalidValue;
      return add(state, type, data, size, name, 0, NULL, NULL) == 0 ? hipSuccess : hipErrorInvalidImage; }
}
__declspec(dllexport) int hipLinkAddFile(void* state, int type, const char* path, unsigned int n, void* opt, void** val) {
    (void)n; (void)opt; (void)val;
    { API7_HIPRTC(add, api7_f_laddf, "hiprtcLinkAddFile");
      if (!state || !path) return hipErrorInvalidValue;
      return add(state, type, path, 0, NULL, NULL) == 0 ? hipSuccess : hipErrorFileNotFound; }
}
__declspec(dllexport) int hipLinkComplete(void* state, void** bin, size_t* size) {
    API7_HIPRTC(fine, api7_f_lcomplete, "hiprtcLinkComplete");
    if (!state) return hipErrorInvalidValue;
    return fine(state, bin, size) == 0 ? hipSuccess : hipErrorNotSupported;
}
__declspec(dllexport) int hipLinkDestroy(void* state) {
    API7_HIPRTC(distruggi, api7_f_ldestroy, "hiprtcLinkDestroy");
    if (!state) return hipErrorInvalidValue;
    return distruggi(state) == 0 ? hipSuccess : hipErrorInvalidValue;
}
