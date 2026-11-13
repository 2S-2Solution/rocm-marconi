/*
 * api7_green.inc - gruppo 2e: contesti "green" e risorse CU (SM) di HIP 7, sulla maschera di CU della 5.7.
 * 2 Solution, 13 nov 2026. Incluso da shim_main.c dopo api7_graph.inc.
 *
 * Un green context raggruppa un sottoinsieme delle unita' di calcolo (CU, le "SM"). La 5.7 non ha i green
 * context, ma ha hipExtStreamCreateWithCUMask: uno stream con una maschera di CU. Qui un green context e'
 * quindi una maschera di CU, e uno stream creato dal contesto e' uno stream con quella maschera, che su
 * gfx906 limita davvero le CU usate. La divisione (SplitByCount) assegna CU consecutive.
 *
 * hipDevResource_st (144 byte): type a 0, l'unione a 96; per Sm i campi sono smCount a 96,
 * minSmPartitionSize a 100 (qui riusato come offset della prima CU del gruppo), nextResource a 136.
 */

#define API7_RES_SMCOUNT  96
#define API7_RES_SMOFF   100
#define API7_MASK_WORDS    8     /* fino a 256 CU */

typedef struct { unsigned int nmask; unsigned int mask[API7_MASK_WORDS]; int device; unsigned long long id; } api7_green;
typedef struct { unsigned int nmask; unsigned int mask[API7_MASK_WORDS]; } api7_descr;
static unsigned long long api7_green_prossimo = 1;

static int api7_num_cu(int dev) {
    int n = 0;
    api7_f_attr attr = (api7_f_attr)abi57_reale("hipDeviceGetAttribute");
    if (!attr || attr(&n, 63 /*MultiprocessorCount*/, dev) != hipSuccess || n <= 0) n = 60;   /* gfx906 */
    if (n > API7_MASK_WORDS * 32) n = API7_MASK_WORDS * 32;
    return n;
}
static unsigned int api7_popcount(const unsigned int* m, unsigned int nw) {
    unsigned int i, c = 0, v;
    for (i = 0; i < nw; i++) for (v = m[i]; v; v &= v - 1) c++;
    return c;
}

/* ---- risorse CU ---- */

__declspec(dllexport) int hipDeviceGetDevResource(int device, void* resource, int type) {
    unsigned char* r = (unsigned char*)resource;
    if (!resource) return hipErrorInvalidValue;
    if (type != 1 /*Sm*/) return hipErrorNotSupported;
    *(unsigned int*)(r + API7_RES_SMCOUNT) = (unsigned int)api7_num_cu(device);
    *(unsigned int*)(r + API7_RES_SMOFF) = 0;    /* prima CU */
    return hipSuccess;
}

static int api7_split(unsigned char* result, unsigned int ngruppi, const unsigned char* input,
                      unsigned char* remainder, unsigned int minCount) {
    unsigned int tot = *(const unsigned int*)(input + API7_RES_SMCOUNT);
    unsigned int off = *(const unsigned int*)(input + API7_RES_SMOFF);
    unsigned int i;
    if (!minCount || minCount > tot) return hipErrorInvalidValue;
    for (i = 0; i < ngruppi; i++) {
        unsigned char* g = result + (size_t)i * 144;
        memset(g, 0, 144);
        *(unsigned int*)(g + API7_RES_SMCOUNT) = minCount;
        *(unsigned int*)(g + API7_RES_SMOFF) = off + i * minCount;
    }
    if (remainder) {
        memset(remainder, 0, 144);
        *(unsigned int*)(remainder + API7_RES_SMCOUNT) = tot - ngruppi * minCount;
        *(unsigned int*)(remainder + API7_RES_SMOFF) = off + ngruppi * minCount;
    }
    return hipSuccess;
}

__declspec(dllexport) int hipDevSmResourceSplitByCount(void* result, unsigned int* nbGroups, const void* input,
                                                       void* remainder, unsigned int flags, unsigned int minCount) {
    unsigned int tot, ngruppi;
    (void)flags;
    if (!result || !nbGroups || !input) return hipErrorInvalidValue;
    tot = *(const unsigned int*)((const unsigned char*)input + API7_RES_SMCOUNT);
    if (!minCount) return hipErrorInvalidValue;
    ngruppi = tot / minCount;
    if (ngruppi > *nbGroups) ngruppi = *nbGroups;    /* non oltre la capacita' di result */
    *nbGroups = ngruppi;
    if (!ngruppi) return hipSuccess;
    return api7_split((unsigned char*)result, ngruppi, (const unsigned char*)input, (unsigned char*)remainder, minCount);
}

__declspec(dllexport) int hipDevSmResourceSplit(void* result, unsigned int nbGroups, const void* input,
                                                void* remainder, unsigned int flags, void* groupParams) {
    unsigned int tot, minCount;
    (void)flags; (void)groupParams;
    if (!result || !input || !nbGroups) return hipErrorInvalidValue;
    tot = *(const unsigned int*)((const unsigned char*)input + API7_RES_SMCOUNT);
    minCount = tot / nbGroups;
    if (!minCount) return hipErrorInvalidValue;
    return api7_split((unsigned char*)result, nbGroups, (const unsigned char*)input, (unsigned char*)remainder, minCount);
}

static void api7_bit_imposta(api7_descr* d, unsigned int cu) {
    if (cu < API7_MASK_WORDS * 32) { d->mask[cu >> 5] |= 1u << (cu & 31); if ((cu >> 5) + 1 > d->nmask) d->nmask = (cu >> 5) + 1; }
}

__declspec(dllexport) int hipDevResourceGenerateDesc(void** phDesc, void* resources, unsigned int nbResources) {
    api7_descr* d;
    unsigned int i, c;
    if (!phDesc || (!resources && nbResources)) return hipErrorInvalidValue;
    d = (api7_descr*)calloc(1, sizeof(api7_descr));
    if (!d) return hipErrorOutOfMemory;
    for (i = 0; i < nbResources; i++) {
        const unsigned char* r = (const unsigned char*)resources + (size_t)i * 144;
        unsigned int off = *(const unsigned int*)(r + API7_RES_SMOFF);
        unsigned int cnt = *(const unsigned int*)(r + API7_RES_SMCOUNT);
        for (c = 0; c < cnt; c++) api7_bit_imposta(d, off + c);
    }
    *phDesc = d;
    return hipSuccess;
}

/* ---- green context ---- */

static api7_green* api7_green_crea(const api7_descr* d, int device) {
    api7_green* g = (api7_green*)calloc(1, sizeof(api7_green));
    if (!g) return NULL;
    g->device = device;
    g->id = api7_green_prossimo++;
    if (d && d->nmask) { g->nmask = d->nmask; memcpy(g->mask, d->mask, sizeof(g->mask)); }
    else {   /* tutte le CU del dispositivo */
        int n = api7_num_cu(device), i;
        for (i = 0; i < n; i++) g->mask[i >> 5] |= 1u << (i & 31);
        g->nmask = (unsigned int)((n + 31) / 32);
    }
    return g;
}

__declspec(dllexport) int hipGreenCtxCreate(void** ctx, void* desc, int device, unsigned int flags) {
    api7_green* g;
    (void)flags;
    if (!ctx) return hipErrorInvalidValue;
    g = api7_green_crea((const api7_descr*)desc, device);
    if (!g) return hipErrorOutOfMemory;
    *ctx = g;
    return hipSuccess;
}

__declspec(dllexport) int hipDeviceGetExecutionCtx(void** ctx, int device) {
    return hipGreenCtxCreate(ctx, NULL, device, 0);
}

typedef int (*api7_f_cumask)(void** stream, unsigned int nmask, const unsigned int* mask);
__declspec(dllexport) int hipExecutionCtxStreamCreate(void** stream, void* greenctx, unsigned int flags, int priority) {
    api7_green* g = (api7_green*)greenctx;
    (void)flags; (void)priority;    /* la CU mask della 5.7 non porta priorita' */
    ABI57_REALE(crea, api7_f_cumask, "hipExtStreamCreateWithCUMask");
    if (!stream || !g) return hipErrorInvalidValue;
    return crea(stream, g->nmask, g->mask);
}

__declspec(dllexport) int hipExecutionCtxDestroy(void* ctx) {
    if (!ctx) return hipErrorInvalidValue;
    free(ctx);
    return hipSuccess;
}

typedef int (*api7_f_sync)(void);
__declspec(dllexport) int hipExecutionCtxSynchronize(void* ctx) {
    (void)ctx;    /* una sola GPU: sincronizza il dispositivo */
    { ABI57_REALE(sync, api7_f_sync, "hipDeviceSynchronize"); return sync(); }
}

__declspec(dllexport) int hipExecutionCtxGetDevice(int* device, void* ctx) {
    api7_green* g = (api7_green*)ctx;
    if (!device || !g) return hipErrorInvalidValue;
    *device = g->device;
    return hipSuccess;
}

__declspec(dllexport) int hipExecutionCtxGetId(void* ctx, unsigned long long* id) {
    api7_green* g = (api7_green*)ctx;
    if (!id || !g) return hipErrorInvalidValue;
    *id = g->id;
    return hipSuccess;
}

typedef int (*api7_f_evrec)(void* ev, void* stream);
__declspec(dllexport) int hipExecutionCtxRecordEvent(void* ctx, void* event) {
    (void)ctx;    /* nessuno stream proprio del contesto: usa lo stream nullo */
    { ABI57_REALE(rec, api7_f_evrec, "hipEventRecord"); return rec(event, NULL); }
}

/* Senza uno stream persistente del contesto, l'attesa su un evento e' conservativa: sincronizza
 * sull'evento. Non si usa hipStreamWaitEvent(NULL, ...) perche' lo stream nullo che aspetta un evento
 * e' un difetto noto del driver 5.7 (restituisce hipErrorInvalidValue). */
typedef int (*api7_f_evsync)(void* ev);
__declspec(dllexport) int hipExecutionCtxWaitEvent(void* ctx, void* event) {
    (void)ctx;
    if (!event) return hipErrorInvalidValue;
    { ABI57_REALE(sync, api7_f_evsync, "hipEventSynchronize"); return sync(event); }
}

__declspec(dllexport) int hipExecutionCtxGetDevResource(void* ctx, void* resource, int type) {
    api7_green* g = (api7_green*)ctx;
    unsigned char* r = (unsigned char*)resource;
    if (!g || !resource) return hipErrorInvalidValue;
    if (type != 1) return hipErrorNotSupported;
    memset(r, 0, 144);
    *(unsigned int*)(r + API7_RES_SMCOUNT) = api7_popcount(g->mask, g->nmask);
    return hipSuccess;
}

typedef int (*api7_f_getcumask)(void* stream, unsigned int nmask, unsigned int* mask);
__declspec(dllexport) int hipStreamGetDevResource(void* stream, void* resource, int type) {
    unsigned int mask[API7_MASK_WORDS];
    unsigned char* r = (unsigned char*)resource;
    ABI57_REALE(get, api7_f_getcumask, "hipExtStreamGetCUMask");
    if (!resource) return hipErrorInvalidValue;
    if (type != 1) return hipErrorNotSupported;
    memset(mask, 0, sizeof(mask));
    { int rc = get(stream, API7_MASK_WORDS, mask); if (rc != hipSuccess) return rc; }
    memset(r, 0, 144);
    *(unsigned int*)(r + API7_RES_SMCOUNT) = api7_popcount(mask, API7_MASK_WORDS);
    return hipSuccess;
}
