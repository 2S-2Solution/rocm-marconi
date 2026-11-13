/*
 * api7_misc.inc - gruppo 2f: completamento. Registrazioni mancanti, logging/profiler, mem pool per
 * posizione, advise/prefetch v2, copie a lotti, LUID. 2 Solution, 13 nov 2026. Dopo api7_green.inc.
 *
 * Restano non implementate (NONSUP o assenti) solo le funzioni che la 5.7 non puo' reggere: i nodi di
 * grafo avanzati (semafori esterni, batch mem-op, API driver dei grafi, cattura verso grafo,
 * istanziazione con parametri) e hipMemcpy3DBatchAsync / mipmap da memoria esterna.
 */

/* ---- registrazioni mancanti (stub: le variabili managed/texture/surface statiche sono rare) ----
 * __hipRegisterManagedVar la registra come variabile normale cosi' hipGetSymbolAddress la trova; le
 * texture/surface reference sono deprecate e qui ignorate; __hipOnError installa un callback non usato. */
__declspec(dllexport) void __hipRegisterManagedVar(void** moduli, void** puntatore, void* init, char* nome,
                                                   size_t dim, unsigned int align) {
    (void)puntatore; (void)init; (void)align;
    __hipRegisterVar(moduli, puntatore ? *puntatore : NULL, nome, nome, 0, dim, 0, 1);
}
__declspec(dllexport) void __hipRegisterSurface(void** m, void* a, char* b, char* c, int d, int e) {
    (void)m; (void)a; (void)b; (void)c; (void)d; (void)e;   /* surface reference: deprecata */
}
__declspec(dllexport) void __hipRegisterTexture(void** m, void* a, char* b, char* c, int d, int e, int f) {
    (void)m; (void)a; (void)b; (void)c; (void)d; (void)e; (void)f;   /* texture reference: deprecata */
}
__declspec(dllexport) int __hipOnError(void* callback, void* userData) {
    (void)callback; (void)userData;
    return hipSuccess;
}

/* ---- logging e profiler (diagnostica): rispondono senza errore ----
 * AMD_LOG_LEVEL (variabile d'ambiente) resta il vero controllo del log del driver. */
__declspec(dllexport) int hipExtEnableLogging(void) { return hipSuccess; }
__declspec(dllexport) int hipExtDisableLogging(void) { return hipSuccess; }
__declspec(dllexport) int hipExtSetLoggingParams(unsigned long long lvl, unsigned long long size, unsigned long long mask) {
    (void)lvl; (void)size; (void)mask; return hipSuccess;
}
__declspec(dllexport) int hipProfilerEnableExt(void* a) { (void)a; return hipSuccess; }
__declspec(dllexport) int hipProfilerDisableExt(void* a) { (void)a; return hipSuccess; }
__declspec(dllexport) int hipProfilerGetRecordsExt(void* a, void* b, void* c) { (void)a; (void)c; if (b) *(size_t*)b = 0; return hipSuccess; }
__declspec(dllexport) int hipProfilerRegisterChunkCallbackExt(void* a, void* b) { (void)a; (void)b; return hipSuccess; }

/* ---- mem pool per posizione (hipMemLocation: type a 0, id a 4) ---- */
typedef int (*api7_f_devpool)(void** pool, int device);
typedef int (*api7_f_setpool)(int device, void* pool);

__declspec(dllexport) int hipMemGetDefaultMemPool(void** memPool, const void* location, int type) {
    int dev = location ? *(const int*)((const unsigned char*)location + 4) : 0;
    (void)type;
    ABI57_REALE(f, api7_f_devpool, "hipDeviceGetDefaultMemPool");
    if (!memPool) return hipErrorInvalidValue;
    return f(memPool, dev);
}
__declspec(dllexport) int hipMemGetMemPool(void** pool, const void* location, int type) {
    int dev = location ? *(const int*)((const unsigned char*)location + 4) : 0;
    (void)type;
    ABI57_REALE(f, api7_f_devpool, "hipDeviceGetMemPool");
    if (!pool) return hipErrorInvalidValue;
    return f(pool, dev);
}
__declspec(dllexport) int hipMemSetMemPool(const void* location, int type, void* pool) {
    int dev = location ? *(const int*)((const unsigned char*)location + 4) : 0;
    (void)type;
    ABI57_REALE(f, api7_f_setpool, "hipDeviceSetMemPool");
    return f(dev, pool);
}

/* ---- advise e prefetch, varianti v2 (hipMemLocation invece di device) ----
 * hipMemLocation {int type; int id;} e' 8 byte e viaggia PER VALORE in un registro, non per puntatore. */
typedef struct { int type; int id; } api7_memloc;
typedef int (*api7_f_advise)(const void* ptr, size_t count, int advice, int device);
typedef int (*api7_f_prefetch)(const void* ptr, size_t count, int device, void* stream);

__declspec(dllexport) int hipMemAdvise_v2(const void* ptr, size_t count, int advice, api7_memloc location) {
    ABI57_REALE(f, api7_f_advise, "hipMemAdvise");
    return f(ptr, count, advice, location.id);
}
__declspec(dllexport) int hipMemPrefetchAsync_v2(const void* ptr, size_t count, api7_memloc location,
                                                 unsigned int flags, void* stream) {
    (void)flags;
    ABI57_REALE(f, api7_f_prefetch, "hipMemPrefetchAsync");
    return f(ptr, count, location.id, stream);
}

/* ---- copie a lotti ---- */
typedef int (*api7_f_cpasync)(void* dst, const void* src, size_t n, int kind, void* stream);
__declspec(dllexport) int hipMemcpyBatchAsync(void** dsts, void** srcs, size_t* sizes, unsigned long long count,
                                              void* attrs, size_t* attrsIdxs, unsigned long long numAttrs,
                                              size_t* failIdx, void* stream) {
    unsigned long long i;
    (void)attrs; (void)attrsIdxs; (void)numAttrs;
    ABI57_REALE(cp, api7_f_cpasync, "hipMemcpyAsync");
    if (count && (!dsts || !srcs || !sizes)) return hipErrorInvalidValue;
    for (i = 0; i < count; i++) {
        int rc = cp(dsts[i], srcs[i], sizes[i], REG_DEFAULT, stream);
        if (rc != hipSuccess) { if (failIdx) *failIdx = (size_t)i; return rc; }
    }
    return hipSuccess;
}

/* ---- LUID del dispositivo (Windows): deriva un identificatore stabile dall'indice del dispositivo.
 * La 5.7 non lo espone; un LUID a 8 byte col solo indice basta per distinguere i dispositivi. ---- */
__declspec(dllexport) int hipDeviceGetLuid(char* luid, unsigned int* deviceNodeMask, int device) {
    if (!luid || !deviceNodeMask) return hipErrorInvalidValue;
    memset(luid, 0, 8);
    memcpy(luid, &device, sizeof(int) <= 8 ? sizeof(int) : 8);
    *deviceNodeMask = 1u;
    return hipSuccess;
}
