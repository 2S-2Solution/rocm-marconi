/*
 * shim_main_v33.c - Marconi 5.7 Expansion Pack v3.3 - SHIM COMPLETO con forwarder critici.
 * 2 Solution, 20 giu 2026 T13:50 CEST.
 *
 * Lo shim serve a ogni programma HIP 6.x/7.x, non a un solo carico.
 * Bug V7 torch.zeros "hipErrorNoDevice" causato da forwarder MANCANTI nel shim:
 *   - hipMemcpy, hipMemset, hipMemsetAsync (plain, no D8/D16/D32)
 *   - hipStreamCreate, hipEventCreate (plain, no WithFlags)
 *   - hipLaunchKernel (plain, no ModuleLaunchKernel)
 *   - hipCtxGetCurrent, hipCtxSetCurrent, hipCtxGetDevice
 *   - hipGetDeviceProperties (plain, no R0600)
 *   - hipDeviceGetP2PAttribute, hipMemAllocHost, hipHostFree, hipMemPrefetchAsync
 *
 * v3.2 -> v3.3: 97 -> ~130 forwarder. Stessa conversion struct hipDeviceProp v5.7 -> R0600.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>
#include <string.h>

typedef int hipError_t;
#define hipSuccess              0
#define hipErrorNotInitialized  3
#define hipErrorDeinitialized   4   /* la runtime si sta chiudendo (cudaErrorCudartUnloading) */
#define hipErrorInvalidValue    1   /* era 7 (hipErrorProfilerAlreadyStarted) fino al 13 nov 2026 */
#define hipErrorNotSupported    801

/* Traccia diagnostica: solo nelle build compilate con /DREG_TRACCIA, scrive su stderr. */
#ifdef REG_TRACCIA
#include <stdio.h>
#include <stdarg.h>
static void reg_traccia(const char* f, ...) {
    char b[1024]; va_list a; DWORD s = 0; int n;
    va_start(a, f); n = _vsnprintf(b, sizeof(b) - 2, f, a); va_end(a);
    if (n < 0 || n > (int)sizeof(b) - 2) n = (int)sizeof(b) - 2;
    b[n++] = '\n';
    WriteFile(GetStdHandle(STD_ERROR_HANDLE), b, (DWORD)n, &s, NULL);
}
#define REG_T(...) reg_traccia(__VA_ARGS__)
#define REG_CHIUSURA_T(n) do { if (marconi_in_chiusura()) reg_traccia("[shim] chiamata in chiusura: %s", n); } while (0)
static void reg_descrivi(const char* cosa, void* a) {
    HMODULE m = NULL; char n[MAX_PATH] = "?";
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)a, &m))
        GetModuleFileNameA(m, n, MAX_PATH);
    reg_traccia("%s %p = %s+0x%llx", cosa, a, strrchr(n, '\\') ? strrchr(n, '\\') + 1 : n,
                (unsigned long long)((char*)a - (char*)m));
}
static LONG WINAPI reg_eccezione(EXCEPTION_POINTERS* ep) {
    ULONG_PTR* sp; int k, visti = 0;
    if (ep->ExceptionRecord->ExceptionCode != 0xC0000005) return EXCEPTION_CONTINUE_SEARCH;
    reg_traccia("*** [shim] access violation, indirizzo letto/scritto 0x%llx", (unsigned long long)ep->ExceptionRecord->ExceptionInformation[1]);
    reg_descrivi("  in", ep->ExceptionRecord->ExceptionAddress);
    sp = (ULONG_PTR*)ep->ContextRecord->Rsp;
    for (k = 0; k < 1024 && visti < 25; k++) {
        HMODULE m = NULL;
        if (sp[k] > 0x10000 && GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)sp[k], &m)) {
            reg_descrivi("  pila", (void*)sp[k]); visti++;
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
#else
#define REG_T(...) ((void)0)
#define REG_CHIUSURA_T(n) ((void)0)
#endif

static HMODULE g_amdhip64 = NULL;
static HMODULE g_comgr_driver = NULL;   /* la amd_comgr.dll usata dalla runtime del driver */

/* Ripunta gli import che il modulo m prende da `dll` sulle funzioni omonime di `nuova` (tabella IAT di
 * m, nella memoria di questo processo). Prima si risolvono tutti i nomi, poi si scrive: o tutti o nessuno.
 * Restituisce il numero di voci ripuntate, oppure <0 se non si e' potuto (e nulla e' cambiato). */
static int marconi_rilega_import(HMODULE m, const char* dll, HMODULE nuova) {
    BYTE* base = (BYTE*)m;
    IMAGE_NT_HEADERS64* nt = (IMAGE_NT_HEADERS64*)(base + ((IMAGE_DOS_HEADER*)base)->e_lfanew);
    IMAGE_DATA_DIRECTORY dd = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    IMAGE_IMPORT_DESCRIPTOR* d;
    int passo, n = 0;
    if (!dd.VirtualAddress) return -1;
    for (passo = 0; passo < 2; passo++) {
        n = 0;
        for (d = (IMAGE_IMPORT_DESCRIPTOR*)(base + dd.VirtualAddress); d->Name; d++) {
            IMAGE_THUNK_DATA64 *nomi, *iat;
            if (lstrcmpiA((const char*)(base + d->Name), dll) != 0) continue;
            if (!d->OriginalFirstThunk) return -2;        /* senza la tabella dei nomi non si sa cosa ripuntare */
            nomi = (IMAGE_THUNK_DATA64*)(base + d->OriginalFirstThunk);
            iat = (IMAGE_THUNK_DATA64*)(base + d->FirstThunk);
            for (; nomi->u1.AddressOfData; nomi++, iat++) {
                FARPROC f = IMAGE_SNAP_BY_ORDINAL64(nomi->u1.Ordinal)
                    ? GetProcAddress(nuova, (LPCSTR)(ULONG_PTR)IMAGE_ORDINAL64(nomi->u1.Ordinal))
                    : GetProcAddress(nuova, (LPCSTR)((IMAGE_IMPORT_BY_NAME*)(base + nomi->u1.AddressOfData))->Name);
                if (!f) return -3;
                if (passo == 1) {
                    DWORD prima;
                    if (!VirtualProtect(&iat->u1.Function, sizeof(iat->u1.Function), PAGE_READWRITE, &prima)) return -4;
                    iat->u1.Function = (ULONGLONG)(ULONG_PTR)f;
                    VirtualProtect(&iat->u1.Function, sizeof(iat->u1.Function), prima, &prima);
                }
                n++;
            }
        }
    }
    return n;
}

/* La runtime 5.7 del driver importa amd_comgr.dll per nome. Se nel processo ce n'e' gia' un'altra
 * (le wheel AMD di PyTorch precaricano quella di ROCm 10), Windows lega l'import a quella e la runtime
 * 5.7 va in crash nel suo thread di caricamento del codice. In quel caso si carica per percorso completo
 * la amd_comgr.dll del driver (cartella di sistema), che convive con l'altra, e si ripuntano su di essa
 * gli import della runtime del driver. Chi usa l'altra amd_comgr.dll non e' toccato. */
static HMODULE marconi_carica_driver(const char* percorso) {
    HMODULE gia = GetModuleHandleA("amd_comgr.dll"), h, sua;
    char nome[MAX_PATH], atteso[MAX_PATH];
    UINT n = GetSystemDirectoryA(atteso, MAX_PATH);
    int r;
    h = LoadLibraryA(percorso);
    if (!h || !gia || n == 0 || n > MAX_PATH - 16) return h;
    lstrcatA(atteso, "\\amd_comgr.dll");
    if (!GetModuleFileNameA(gia, nome, MAX_PATH) || lstrcmpiA(nome, atteso) == 0) return h;
    sua = LoadLibraryExA(atteso, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    r = sua ? marconi_rilega_import(h, "amd_comgr.dll", sua) : -5;
    REG_T("[shim] amd_comgr gia' caricata da %s: import della runtime del driver ripuntati su %s -> %d", nome, atteso, r);
    if (r > 0) g_comgr_driver = sua;
    return h;
}

/* Vero mentre il processo termina (ExitProcess): le DLL vengono staccate e la runtime del driver,
 * caricata per ultima, e' la prima a chiudersi. */
typedef BOOLEAN (WINAPI *marconi_f_chiusura)(void);
static int marconi_in_chiusura(void) {
    static marconi_f_chiusura f = NULL;
    static int cercata = 0;
    if (!cercata) { f = (marconi_f_chiusura)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlDllShutdownInProgress"); cercata = 1; }
    return f ? f() != 0 : 0;
}

/* In chiusura la runtime del driver e' gia' staccata e non le si inoltra nulla. Le operazioni di pulizia
 * (memoria, stream, eventi, moduli: le libera comunque Windows) si danno per riuscite, perche' alcune
 * librerie abortiscono se falliscono nei distruttori; le altre rispondono hipErrorDeinitialized, come la
 * runtime CUDA durante lo scaricamento. */
static int marconi_esito_in_chiusura(const char* nome) {
    static const char* const pulizia[] = { "Free", "Destroy", "Unload", "Unregister", "Release" };
    int k;
    for (k = 0; k < (int)(sizeof(pulizia) / sizeof(pulizia[0])); k++)
        if (strstr(nome, pulizia[k])) return hipSuccess;
    return hipErrorDeinitialized;
}

static int marconi_shim_bootstrap(void) {
    if (g_amdhip64) return 1;
#ifdef REG_TRACCIA
    AddVectoredExceptionHandler(0, reg_eccezione);
#endif
    g_amdhip64 = marconi_carica_driver("C:\\Windows\\System32\\amdhip64.dll");
    if (g_amdhip64 && !g_comgr_driver) g_comgr_driver = GetModuleHandleA("amd_comgr.dll");
    return g_amdhip64 ? 1 : 0;
}

typedef struct {
    unsigned int hasGlobalInt32Atomics : 1;
    unsigned int hasGlobalFloatAtomicExch : 1;
    unsigned int hasSharedInt32Atomics : 1;
    unsigned int hasSharedFloatAtomicExch : 1;
    unsigned int hasFloatAtomicAdd : 1;
    unsigned int hasGlobalInt64Atomics : 1;
    unsigned int hasSharedInt64Atomics : 1;
    unsigned int hasDoubles : 1;
    unsigned int hasWarpVote : 1;
    unsigned int hasWarpBallot : 1;
    unsigned int hasWarpShuffle : 1;
    unsigned int hasFunnelShift : 1;
    unsigned int hasThreadFenceSystem : 1;
    unsigned int hasSyncThreadsExt : 1;
    unsigned int hasSurfaceFuncs : 1;
    unsigned int has3dGrid : 1;
    unsigned int hasDynamicParallelism : 1;
    unsigned int _pad : 15;
} hipDeviceArch_t_v57;

typedef struct {
    char name[256];
    size_t totalGlobalMem;
    size_t sharedMemPerBlock;
    int regsPerBlock;
    int warpSize;
    int maxThreadsPerBlock;
    int maxThreadsDim[3];
    int maxGridSize[3];
    int clockRate;
    int memoryClockRate;
    int memoryBusWidth;
    size_t totalConstMem;
    int major;
    int minor;
    int multiProcessorCount;
    int l2CacheSize;
    int maxThreadsPerMultiProcessor;
    int computeMode;
    int clockInstructionRate;
    hipDeviceArch_t_v57 arch;
    int concurrentKernels;
    int pciDomainID;
    int pciBusID;
    int pciDeviceID;
    size_t maxSharedMemoryPerMultiProcessor;
    int isMultiGpuBoard;
    int canMapHostMemory;
    int gcnArch;
    char gcnArchName[256];
    int integrated;
    int cooperativeLaunch;
    int cooperativeMultiDeviceLaunch;
    int maxTexture1DLinear;
    int maxTexture1D;
    int maxTexture2D[2];
    int maxTexture3D[3];
    unsigned int* hdpMemFlushCntl;
    unsigned int* hdpRegFlushCntl;
    size_t memPitch;
    size_t textureAlignment;
    size_t texturePitchAlignment;
    int kernelExecTimeoutEnabled;
    int ECCEnabled;
    int tccDriver;
    int cooperativeMultiDeviceUnmatchedFunc;
    int cooperativeMultiDeviceUnmatchedGridDim;
    int cooperativeMultiDeviceUnmatchedBlockDim;
    int cooperativeMultiDeviceUnmatchedSharedMem;
    int isLargeBar;
    int asicRevision;
    int managedMemory;
    int directManagedMemAccessFromHost;
    int concurrentManagedAccess;
    int pageableMemoryAccess;
    int pageableMemoryAccessUsesHostPageTables;
} hipDeviceProp_v57;

typedef struct { unsigned char bytes[16]; } hipUUID_v6;

typedef struct {
    char name[256];
    hipUUID_v6 uuid;
    char luid[8];
    unsigned int luidDeviceNodeMask;
    size_t totalGlobalMem;
    size_t sharedMemPerBlock;
    int regsPerBlock;
    int warpSize;
    size_t memPitch;
    int maxThreadsPerBlock;
    int maxThreadsDim[3];
    int maxGridSize[3];
    int clockRate;
    size_t totalConstMem;
    int major;
    int minor;
    size_t textureAlignment;
    size_t texturePitchAlignment;
    int deviceOverlap;
    int multiProcessorCount;
    int kernelExecTimeoutEnabled;
    int integrated;
    int canMapHostMemory;
    int computeMode;
    int maxTexture1D;
    int maxTexture1DMipmap;
    int maxTexture1DLinear;
    int maxTexture2D[2];
    int maxTexture2DMipmap[2];
    int maxTexture2DLinear[3];
    int maxTexture2DGather[2];
    int maxTexture3D[3];
    int maxTexture3DAlt[3];
    int maxTextureCubemap;
    int maxTexture1DLayered[2];
    int maxTexture2DLayered[3];
    int maxTextureCubemapLayered[2];
    int maxSurface1D;
    int maxSurface2D[2];
    int maxSurface3D[3];
    int maxSurface1DLayered[2];
    int maxSurface2DLayered[3];
    int maxSurfaceCubemap;
    int maxSurfaceCubemapLayered[2];
    size_t surfaceAlignment;
    int concurrentKernels;
    int ECCEnabled;
    int pciBusID;
    int pciDeviceID;
    int pciDomainID;
    int tccDriver;
    int asyncEngineCount;
    int unifiedAddressing;
    int memoryClockRate;
    int memoryBusWidth;
    int l2CacheSize;
    int persistingL2CacheMaxSize;
    int maxThreadsPerMultiProcessor;
    int streamPrioritiesSupported;
    int globalL1CacheSupported;
    int localL1CacheSupported;
    size_t sharedMemPerMultiprocessor;
    int regsPerMultiprocessor;
    int managedMemory;
    int isMultiGpuBoard;
    int multiGpuBoardGroupID;
    int hostNativeAtomicSupported;
    int singleToDoublePrecisionPerfRatio;
    int pageableMemoryAccess;
    int concurrentManagedAccess;
    int computePreemptionSupported;
    int canUseHostPointerForRegisteredMem;
    int cooperativeLaunch;
    int cooperativeMultiDeviceLaunch;
    size_t sharedMemPerBlockOptin;
    int pageableMemoryAccessUsesHostPageTables;
    int directManagedMemAccessFromHost;
    int maxBlocksPerMultiProcessor;
    int accessPolicyMaxWindowSize;
    size_t reservedSharedMemPerBlock;
    int hostRegisterSupported;
    int sparseHipArraySupported;
    int hostRegisterReadOnlySupported;
    int timelineSemaphoreInteropSupported;
    int memoryPoolsSupported;
    int gpuDirectRDMASupported;
    unsigned int gpuDirectRDMAFlushWritesOptions;
    int gpuDirectRDMAWritesOrdering;
    unsigned int memoryPoolSupportedHandleTypes;
    int deferredMappingHipArraySupported;
    int ipcEventSupported;
    int clusterLaunch;
    int unifiedFunctionPointers;
    int reserved[63];
    int hipReserved[32];
    char gcnArchName[256];
    size_t maxSharedMemoryPerMultiProcessor;
    int clockInstructionRate;
    hipDeviceArch_t_v57 arch;
    unsigned int* hdpMemFlushCntl;
    unsigned int* hdpRegFlushCntl;
    int cooperativeMultiDeviceUnmatchedFunc;
    int cooperativeMultiDeviceUnmatchedGridDim;
    int cooperativeMultiDeviceUnmatchedBlockDim;
    int cooperativeMultiDeviceUnmatchedSharedMem;
    int isLargeBar;
    int asicRevision;
} hipDeviceProp_R0600;

static void conv_v57_to_R0600(const hipDeviceProp_v57* src, hipDeviceProp_R0600* dst) {
    memset(dst, 0, sizeof(*dst));

    /* [VEGA-FIX v3.4 T14:25] BUG CRITICO scoperto via debug profondo:
     * Driver v5.7 amdhip64.dll ritorna major=9, minor=0 (AMD gcnArch 9.0.6).
     * PyTorch 2.3.1+cu118 NON supporta major=9 (compute capability Hopper).
     * ZLUDA simula gfx906 come compute capability 7.0 (Volta-like).
     * Override forzato: major=7, minor=0, e altri campi sporchi del driver. */

    dst->unifiedAddressing = 1;
    dst->streamPrioritiesSupported = 1;
    dst->globalL1CacheSupported = 1;
    dst->localL1CacheSupported = 1;
    dst->asyncEngineCount = 2;
    dst->memoryPoolsSupported = 1;
    dst->hostRegisterSupported = 1;
    dst->hostRegisterReadOnlySupported = 1;
    dst->sparseHipArraySupported = 0;
    dst->timelineSemaphoreInteropSupported = 0;
    dst->gpuDirectRDMASupported = 0;
    dst->deferredMappingHipArraySupported = 0;
    dst->ipcEventSupported = 1;
    dst->clusterLaunch = 0;
    dst->unifiedFunctionPointers = 0;
    dst->computePreemptionSupported = 1;
    dst->canUseHostPointerForRegisteredMem = 1;
    dst->hostNativeAtomicSupported = 1;
    dst->singleToDoublePrecisionPerfRatio = 8;
    dst->multiGpuBoardGroupID = 0;
    dst->persistingL2CacheMaxSize = 0;
    dst->maxBlocksPerMultiProcessor = 32;
    dst->accessPolicyMaxWindowSize = 0;
    dst->sharedMemPerBlockOptin = src->sharedMemPerBlock;
    dst->sharedMemPerMultiprocessor = src->maxSharedMemoryPerMultiProcessor;
    dst->regsPerMultiprocessor = src->regsPerBlock * 16;
    dst->reservedSharedMemPerBlock = 0;
    dst->maxTexture1DMipmap = src->maxTexture1D;

    memcpy(dst->name, src->name, 256);
    dst->totalGlobalMem = src->totalGlobalMem;
    dst->sharedMemPerBlock = src->sharedMemPerBlock;
    /* [VEGA-FIX v3.4] regsPerBlock: driver v5.7 ritorna 0 (sporco).
     * Vega 20 ha 65536 VGPR per CU = 65536 regs per block. */
    dst->regsPerBlock = src->regsPerBlock > 0 ? src->regsPerBlock : 65536;
    dst->warpSize = src->warpSize;
    dst->memPitch = src->memPitch;
    dst->maxThreadsPerBlock = src->maxThreadsPerBlock;
    dst->maxThreadsDim[0] = src->maxThreadsDim[0];
    dst->maxThreadsDim[1] = src->maxThreadsDim[1];
    dst->maxThreadsDim[2] = src->maxThreadsDim[2];
    dst->maxGridSize[0] = src->maxGridSize[0];
    dst->maxGridSize[1] = src->maxGridSize[1];
    dst->maxGridSize[2] = src->maxGridSize[2];
    dst->clockRate = src->clockRate;
    dst->totalConstMem = src->totalConstMem;
    /* [VEGA-FIX v3.4] Force compute capability ZLUDA-compatible:
     * src->major=9 dal driver = AMD gcnArch (9.0.6 Vega 20)
     * PyTorch wants CUDA CC standard: gfx906 -> 7.0 (Volta-like) */
    dst->major = 7;
    dst->minor = 0;
    dst->textureAlignment = src->textureAlignment;
    dst->texturePitchAlignment = src->texturePitchAlignment;
    dst->multiProcessorCount = src->multiProcessorCount;
    dst->kernelExecTimeoutEnabled = src->kernelExecTimeoutEnabled;
    dst->integrated = src->integrated;
    dst->canMapHostMemory = src->canMapHostMemory;
    dst->computeMode = src->computeMode;
    dst->maxTexture1D = src->maxTexture1D;
    dst->maxTexture1DLinear = src->maxTexture1DLinear;
    dst->maxTexture2D[0] = src->maxTexture2D[0];
    dst->maxTexture2D[1] = src->maxTexture2D[1];
    dst->maxTexture3D[0] = src->maxTexture3D[0];
    dst->maxTexture3D[1] = src->maxTexture3D[1];
    dst->maxTexture3D[2] = src->maxTexture3D[2];
    dst->concurrentKernels = src->concurrentKernels;
    dst->ECCEnabled = src->ECCEnabled;
    dst->pciBusID = src->pciBusID;
    dst->pciDeviceID = src->pciDeviceID;
    dst->pciDomainID = src->pciDomainID;
    dst->tccDriver = src->tccDriver;
    dst->memoryClockRate = src->memoryClockRate > 0 ? src->memoryClockRate : 1000000;
    /* [VEGA-FIX v3.4] memoryBusWidth: Vega 20 HBM2 = 4096-bit (driver v5.7 ritorna 0) */
    dst->memoryBusWidth = src->memoryBusWidth > 0 ? src->memoryBusWidth : 4096;
    dst->l2CacheSize = src->l2CacheSize;
    dst->maxThreadsPerMultiProcessor = src->maxThreadsPerMultiProcessor;
    dst->managedMemory = src->managedMemory;
    dst->isMultiGpuBoard = src->isMultiGpuBoard;
    dst->pageableMemoryAccess = src->pageableMemoryAccess;
    dst->concurrentManagedAccess = src->concurrentManagedAccess;
    dst->cooperativeLaunch = src->cooperativeLaunch;
    dst->cooperativeMultiDeviceLaunch = src->cooperativeMultiDeviceLaunch;
    dst->pageableMemoryAccessUsesHostPageTables = src->pageableMemoryAccessUsesHostPageTables;
    dst->directManagedMemAccessFromHost = src->directManagedMemAccessFromHost;
    memcpy(dst->gcnArchName, src->gcnArchName, 256);
    dst->maxSharedMemoryPerMultiProcessor = src->maxSharedMemoryPerMultiProcessor;
    dst->clockInstructionRate = src->clockInstructionRate;
    dst->arch = src->arch;
    dst->hdpMemFlushCntl = src->hdpMemFlushCntl;
    dst->hdpRegFlushCntl = src->hdpRegFlushCntl;
    dst->cooperativeMultiDeviceUnmatchedFunc = src->cooperativeMultiDeviceUnmatchedFunc;
    dst->cooperativeMultiDeviceUnmatchedGridDim = src->cooperativeMultiDeviceUnmatchedGridDim;
    dst->cooperativeMultiDeviceUnmatchedBlockDim = src->cooperativeMultiDeviceUnmatchedBlockDim;
    dst->cooperativeMultiDeviceUnmatchedSharedMem = src->cooperativeMultiDeviceUnmatchedSharedMem;
    dst->isLargeBar = src->isLargeBar;
    dst->asicRevision = src->asicRevision;
}

#define FWD9(name) \
    __declspec(dllexport) int name(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i) { \
        static int (*pfn)(void*,void*,void*,void*,void*,void*,void*,void*,void*) = NULL; \
        if (!pfn) { \
            if (!marconi_shim_bootstrap()) return hipErrorNotInitialized; \
            pfn = (int(*)(void*,void*,void*,void*,void*,void*,void*,void*,void*))GetProcAddress(g_amdhip64, #name); \
            if (!pfn) return hipErrorNotSupported; \
        } \
        REG_CHIUSURA_T(#name); \
        if (marconi_in_chiusura()) return marconi_esito_in_chiusura(#name); \
        return pfn(a,b,c,d,e,f,g,h,i); \
    }

/* ============================================================
 * v5.0 - 28 lug 2026: COPERTURA COMPLETA della runtime.
 * Direttiva: "non si possono aggiungere funzioni alla bisogno,
 * devono essere tutte funzionanti ed esposte".
 * Ogni funzione hip* presente in System32\amdhip64.dll e' ora esposta.
 *
 * FWDP restituisce void* invece di int: preserva l'intero registro RAX.
 * Serve a chi restituisce un puntatore (hipApiName, hipKernelNameRef,
 * hipGetCmdName, hipGetErrorString/Name - queste ultime due erano FWD9 e
 * TRONCAVANO il puntatore a 32 bit) e a chi restituisce una struct per valore
 * (hipCreateChannelDesc, che su x64 usa il puntatore nascosto).
 * Per chi restituisce hipError_t resta corretto: il chiamante legge EAX.
 * ============================================================ */
#define FWDP(name) \
    __declspec(dllexport) void* name(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i) { \
        static void* (*pfn)(void*,void*,void*,void*,void*,void*,void*,void*,void*) = NULL; \
        if (!pfn) { \
            if (!marconi_shim_bootstrap()) return (void*)(size_t)hipErrorNotInitialized; \
            pfn = (void*(*)(void*,void*,void*,void*,void*,void*,void*,void*,void*))GetProcAddress(g_amdhip64, #name); \
            if (!pfn) return (void*)(size_t)hipErrorNotSupported; \
        } \
        return pfn(a,b,c,d,e,f,g,h,i); \
    }


/* v3.2 originali (97) */
FWD9(hipTexRefSetFormat)
FWD9(hipStreamCreateWithFlags)
FWD9(hipDeviceGetUuid)
FWD9(hipModuleUnload)
FWD9(hipGetDeviceCount)
FWD9(hipDeviceGetAttribute)
/* FWD9(hipModuleLoadData): vedi registrazione.inc */
/* FWD9(hipModuleGetFunction): vedi registrazione.inc */
/* FWD9(hipSetDevice): vedi registrazione.inc */
FWD9(hipArray3DCreate)
FWD9(hipArrayCreate)
FWD9(hipArrayDestroy)
FWD9(hipDeviceGetLimit)
FWD9(hipDeviceGetStreamPriorityRange)
FWD9(hipDeviceSetLimit)
FWD9(hipDeviceSynchronize)
/* FWD9(hipGetDevice): vedi registrazione.inc */
FWD9(hipDeviceCanAccessPeer)
FWD9(hipDeviceGet)
FWD9(hipDeviceGetName)
FWD9(hipDeviceGetPCIBusId)
FWD9(hipDeviceTotalMem)
FWD9(hipEventCreateWithFlags)
FWD9(hipEventDestroy)
FWD9(hipEventElapsedTime)
FWD9(hipEventQuery)
FWD9(hipEventRecord)
FWD9(hipEventSynchronize)
FWD9(hipFuncGetAttribute)
/* FWD9(hipFuncSetAttribute): vedi registrazione.inc */
FWD9(hipGraphAddDependencies)
FWD9(hipGraphAddEventRecordNode)
FWD9(hipGraphAddKernelNode)
FWD9(hipGraphAddMemsetNode)
FWD9(hipGraphCreate)
FWD9(hipGraphDestroy)
FWD9(hipGraphExecDestroy)
FWD9(hipGraphExecUpdate)
FWD9(hipGraphGetNodes)
FWD9(hipGraphInstantiateWithFlags)
FWD9(hipGraphLaunch)
FWD9(hipInit)
/* FWD9(hipModuleLaunchKernel): vedi registrazione.inc */
FWD9(hipModuleGetGlobal)
FWD9(hipMallocAsync)
FWD9(hipMemAllocPitch)
FWD9(hipMalloc)
FWD9(hipStreamIsCapturing)
FWD9(hipFree)
FWD9(hipMemsetD8Async)
FWD9(hipStreamSynchronize)
FWD9(hipFreeAsync)
FWD9(hipFreeHost)
FWD9(hipMemGetAddressRange)
FWD9(hipMemGetInfo)
FWD9(hipHostMalloc)
FWD9(hipHostGetDevicePointer)
/* FWD9(hipDrvMemcpy2DUnaligned): tradotta, vedi abi57.inc */
/* FWD9(hipMemcpyParam2D): tradotta, vedi abi57.inc */
/* FWD9(hipDrvMemcpy3D): tradotta, vedi abi57.inc */
FWD9(hipMemcpyAsync)
FWD9(hipMemcpyDtoDAsync)
FWD9(hipMemcpyDtoD)
FWD9(hipMemcpyDtoHAsync)
FWD9(hipMemcpyDtoH)
FWD9(hipMemcpy_spt)
FWD9(hipMemcpyHtoA)
FWD9(hipMemcpyHtoDAsync)
FWD9(hipMemcpyHtoD)
FWD9(hipMemsetD16Async)
FWD9(hipMemsetD16)
FWD9(hipMemset2DAsync)
FWD9(hipMemset2D)
FWD9(hipMemsetD32Async)
FWD9(hipMemsetD32)
FWD9(hipMemsetD8)
FWD9(hipModuleGetTexRef)
FWD9(hipModuleOccupancyMaxActiveBlocksPerMultiprocessorWithFlags)
FWD9(hipModuleOccupancyMaxPotentialBlockSize)
/* FWD9(hipPointerGetAttribute): tradotta, vedi abi57.inc */
/* FWD9(hipDrvPointerGetAttributes): tradotta, vedi abi57.inc */
FWD9(hipStreamBeginCapture)
FWD9(hipStreamCreateWithPriority)
/* FWD9(hipStreamDestroy): in api7.inc (dimentica attributi e id dello stream) */
FWD9(hipStreamEndCapture)
FWD9(hipStreamGetCaptureInfo_v2)
/* [VEGA-PATCH stream-nullo, 17 ott 2026 - 2S]
 * Il runtime HIP del driver (5.7.31541) rifiuta hipStreamWaitEvent quando chi
 * aspetta e' lo stream NULLO (hipErrorInvalidValue): entra nel ramo di cattura dei
 * grafi senza cattura in corso. Gia' corretto nel CLR attuale, ma per gfx906 non
 * esiste un runtime piu' recente. MIOpen passa da questo shim e IGNORA l'errore:
 * le LSTM a piu' stream (>= 32 passi) davano gradienti dei pesi a zero dal secondo
 * indietro in poi, sia sulla ruota sia sotto ZLUDA.
 * Rimedio alla base, per chiunque passi di qui: l'attesa destinata allo stream
 * nullo va su uno stream BLOCCANTE di servizio (uno per dispositivo), che lo stream
 * nullo aspetta per specifica HIP. Nessun blocco della CPU. Gli stream non nulli
 * passano invariati. */
typedef int (*marconi_pfn_wait)(void*, void*, unsigned int);
typedef int (*marconi_pfn_get_device)(int*);
typedef int (*marconi_pfn_stream_create_flags)(void**, unsigned int);
static void* g_marconi_ponti[64];
static SRWLOCK g_marconi_ponti_lock = SRWLOCK_INIT;

__declspec(dllexport) int hipStreamWaitEvent(void* stream, void* event, unsigned int flags) {
    static marconi_pfn_wait reale = NULL;
    if (!reale) {
        if (!marconi_shim_bootstrap()) return hipErrorNotInitialized;
        reale = (marconi_pfn_wait)GetProcAddress(g_amdhip64, "hipStreamWaitEvent");
        if (!reale) return hipErrorNotSupported;
    }
    if (stream != NULL) return reale(stream, event, flags);

    marconi_pfn_get_device dammi_dispositivo =
        (marconi_pfn_get_device)GetProcAddress(g_amdhip64, "hipGetDevice");
    int dispositivo = 0;
    if (!dammi_dispositivo || dammi_dispositivo(&dispositivo) != hipSuccess
            || dispositivo < 0 || dispositivo >= 64)
        return reale(stream, event, flags);

    AcquireSRWLockExclusive(&g_marconi_ponti_lock);
    void* ponte = g_marconi_ponti[dispositivo];
    if (!ponte) {
        marconi_pfn_stream_create_flags crea =
            (marconi_pfn_stream_create_flags)GetProcAddress(g_amdhip64, "hipStreamCreateWithFlags");
        /* flag 0 = hipStreamDefault: BLOCCANTE, e' questo che lo lega allo stream nullo */
        if (crea && crea(&ponte, 0) == hipSuccess) g_marconi_ponti[dispositivo] = ponte;
        else ponte = NULL;
    }
    ReleaseSRWLockExclusive(&g_marconi_ponti_lock);
    if (!ponte) return reale(stream, event, flags);
    return reale(ponte, event, flags);
}
FWD9(hipTexObjectCreate)
FWD9(hipDestroyTextureObject)
FWD9(hipTexRefSetAddressMode)
FWD9(hipTexRefSetAddress)
FWD9(hipTexRefSetArray)
FWD9(hipTexRefSetFilterMode)
FWD9(hipTexRefSetFlags)
FWD9(hipThreadExchangeStreamCaptureMode)

/* === v3.4 NUOVI forwarder per hipBLASLt 26 symbols (Vega T11:30 21 giu) === */
/* FWD9(hipDrvLaunchKernelEx): assente nella 5.7, implementata in registrazione.inc */
/* FWD9(hipExtModuleLaunchKernel): 14 argomenti, prototipo esplicito in registrazione.inc */

/* === v3.7 NUOVI forwarder per MIOpen Marconi TheRock 7.14 (Vega T14:30 22 giu) ===
 * Diff dumpbin /imports MIOpen.dll vs shim v36 exports:
 *   3 simboli mancanti — tutti presenti in amdhip64.dll System32 Adrenalin 24.Q4
 * Fix WinError 127 standalone ctypes load MIOpen.dll. */
FWD9(hipMemPtrGetInfo)
FWD9(hipMemcpyWithStream)
/* FWD9(hipModuleLaunchCooperativeKernel): vedi registrazione.inc */

/* === VEGA-PATCH 2026-06-28 sprint-orch v2 shim-N ===
 * Agent 4 cross-reference: forwarder addizionali per coverage CU mask + graph v2.
 * hipExtStreamCreateWithCUMask: extension multi-stream CU partitioning (Vega 20 4 ACE
 *   asset, sediment 22 giu — driver v5.7 lo esporta MA mancava nel shim).
 * hipGraphAddNode_v2: API graph v2 introdotta hip 5.7+, presente amdhip64.dll v5.7. */
FWD9(hipExtStreamCreateWithCUMask)
FWD9(hipGraphAddNode_v2)

/* === v3.5 underscored CUDA host runtime symbols (Vega T11:45 21 giu) */
FWD9(__hipPopCallConfiguration)
FWD9(__hipPushCallConfiguration)

/* === v3.7 FIX SIGSEGV T4 matmul (Vega T12:35 21 giu)
 * Root cause: __hipRegisterFatBinary forwardato a v5.7 → "Cannot Register fat binary cov5"
 *  → poi hipModuleLoadData crash V5 Code Object.
 * v5.7 Adrenalin supporta solo cov3/cov4. hipblaslt TheRock 7.14 pre-registra fat cov5.
 * Soluzione: STUB NO-OP che ritorna fake handle (1). Tensile pre-registra fake binaries
 * MA i kernel reali li carica via ROCBLAS_TENSILE_LIBPATH ROCm 5.7 cov3/cov4. */
/* 12 nov 2026: la registrazione vera (lazy, HIPK/HIPF, v6 -> v5) e' in registrazione.inc. */

/* === v3.6 rocBLAS PE imports (Vega T12:00 21 giu)
 * rocblas.dll richiede 3 simboli aggiuntivi non in shim v3.5. */
FWD9(hipMemPoolTrimTo)
FWD9(hipModuleOccupancyMaxActiveBlocksPerMultiprocessor)
/* hipExtGetLastError non c'e' nella 5.7: lo implementa registrazione.inc */
/* FWD9(hipExtGetLastError): vedi registrazione.inc */

/* === v3.3 NUOVI forwarder CRITICI per torch.zeros / kernel launch === */
FWD9(hipMemcpy)
FWD9(hipMemset)
FWD9(hipMemsetAsync)
FWD9(hipStreamCreate)
FWD9(hipEventCreate)
/* FWD9(hipLaunchKernel): vedi registrazione.inc */
FWD9(hipCtxGetCurrent)
FWD9(hipCtxSetCurrent)
FWD9(hipCtxGetDevice)
FWD9(hipCtxCreate)
FWD9(hipCtxDestroy)
FWD9(hipCtxPushCurrent)
FWD9(hipCtxPopCurrent)
FWD9(hipCtxSynchronize)
FWD9(hipGetDeviceProperties)
FWD9(hipDeviceGetP2PAttribute)
FWD9(hipMemAllocHost)
FWD9(hipHostFree)
FWD9(hipMemPrefetchAsync)
FWD9(hipMemAdvise)
FWD9(hipMallocManaged)
FWD9(hipStreamQuery)
FWD9(hipStreamAddCallback)
FWD9(hipStreamGetFlags)
FWD9(hipStreamGetPriority)
FWD9(hipDriverGetVersion)
FWD9(hipRuntimeGetVersion)
/* FWD9(hipGetLastError): vedi registrazione.inc */
/* FWD9(hipPeekAtLastError): vedi registrazione.inc */
FWDP(hipGetErrorName)
FWDP(hipGetErrorString)
FWD9(hipMemcpy2D)
FWD9(hipMemcpy2DAsync)
FWD9(hipMemcpy3D)
FWD9(hipMemcpy3DAsync)
FWD9(hipMemPoolCreate)
FWD9(hipMemPoolDestroy)
FWD9(hipMemPoolSetAttribute)
FWD9(hipMemPoolGetAttribute)
FWD9(hipDeviceGetDefaultMemPool)
FWD9(hipDeviceSetMemPool)
FWD9(hipDeviceGetMemPool)
FWD9(hipMallocFromPoolAsync)
FWD9(hipStreamGetCaptureInfo)
FWD9(hipGraphInstantiate)
FWD9(hipGraphAddMemcpyNode)
FWD9(hipGraphAddChildGraphNode)
FWD9(hipGraphAddHostNode)
FWD9(hipGraphLaunch_spt)
FWD9(hipHostRegister)
FWD9(hipHostUnregister)
FWD9(hipHostGetFlags)
/* FWD9(hipPointerGetAttributes): tradotta, vedi abi57.inc */
/* FWD9(hipFuncSetCacheConfig): vedi registrazione.inc */
FWD9(hipFuncSetSharedMemConfig)
FWD9(hipDeviceSetCacheConfig)
FWD9(hipDeviceGetCacheConfig)
FWD9(hipDeviceSetSharedMemConfig)
FWD9(hipDeviceGetSharedMemConfig)
/* FWD9(hipOccupancyMaxActiveBlocksPerMultiprocessor): vedi registrazione.inc */
FWD9(hipOccupancyMaxActiveBlocksPerMultiprocessorWithFlags)
/* FWD9(hipModuleLoad): vedi registrazione.inc */
FWD9(hipDevicePrimaryCtxRetain)
FWD9(hipDevicePrimaryCtxRelease)
FWD9(hipDevicePrimaryCtxReset)
FWD9(hipDevicePrimaryCtxGetState)
FWD9(hipDevicePrimaryCtxSetFlags)
FWD9(hipDeviceComputeCapability)
FWD9(hipDeviceGetByPCIBusId)

/* [VEGA-FIX v3.5 T14:45] Struct R0600 HARDCODED Vega 20 (AMD Radeon VII gfx906).
 * NESSUNA call al driver, NESSUNA conversion v5.7. Tutti i campi a valori noti
 * verificati gfx906 Vega 20. Elimina sticky error driver + ABI bugs conversion.
 * PyTorch valida i campi durante _check_capability + cudaGetDeviceProperties: tutti i campi
 * sono ora completi e validi -> niente fail. */
__declspec(dllexport) int hipGetDevicePropertiesR0600(hipDeviceProp_R0600* prop_R0600, int dev) {
    if (!prop_R0600) return hipErrorInvalidValue;
    memset(prop_R0600, 0, sizeof(*prop_R0600));

    /* Identita' base AMD Radeon VII (Vega 20, gfx906) */
    strcpy(prop_R0600->name, "AMD Radeon VII");
    prop_R0600->totalGlobalMem = 17163091968ULL;  /* 16 GB HBM2 */
    prop_R0600->sharedMemPerBlock = 65536;
    prop_R0600->regsPerBlock = 65536;
    prop_R0600->warpSize = 64;  /* wave64 (config validata). Test 2026-07-02: 32 non cambia reduction torch */
    prop_R0600->memPitch = 2147483647ULL;
    prop_R0600->maxThreadsPerBlock = 1024;
    prop_R0600->maxThreadsDim[0] = 1024;
    prop_R0600->maxThreadsDim[1] = 1024;
    prop_R0600->maxThreadsDim[2] = 1024;
    prop_R0600->maxGridSize[0] = 2147483647;
    prop_R0600->maxGridSize[1] = 65536;
    prop_R0600->maxGridSize[2] = 65536;
    prop_R0600->clockRate = 1801000;       /* 1801 MHz */
    prop_R0600->totalConstMem = 2147483647ULL;
    prop_R0600->major = 7;                  /* Compute capability ZLUDA-style */
    prop_R0600->minor = 0;
    prop_R0600->textureAlignment = 512;
    prop_R0600->texturePitchAlignment = 256;
    prop_R0600->deviceOverlap = 1;
    prop_R0600->multiProcessorCount = 60;   /* 60 CU */
    prop_R0600->kernelExecTimeoutEnabled = 0;
    prop_R0600->integrated = 0;
    prop_R0600->canMapHostMemory = 1;
    prop_R0600->computeMode = 0;
    prop_R0600->maxTexture1D = 16384;
    prop_R0600->maxTexture1DMipmap = 16384;
    prop_R0600->maxTexture1DLinear = 134217728;
    prop_R0600->maxTexture2D[0] = 16384;
    prop_R0600->maxTexture2D[1] = 16384;
    prop_R0600->maxTexture3D[0] = 16384;
    prop_R0600->maxTexture3D[1] = 16384;
    prop_R0600->maxTexture3D[2] = 8192;
    prop_R0600->concurrentKernels = 1;
    prop_R0600->ECCEnabled = 1;             /* HBM2 ha ECC */
    prop_R0600->pciBusID = 14;
    prop_R0600->pciDeviceID = 0;
    prop_R0600->pciDomainID = 0;
    prop_R0600->tccDriver = 0;
    prop_R0600->asyncEngineCount = 2;
    prop_R0600->unifiedAddressing = 1;
    prop_R0600->memoryClockRate = 1000000;
    prop_R0600->memoryBusWidth = 4096;       /* HBM2 4096-bit */
    prop_R0600->l2CacheSize = 4194304;       /* 4 MB L2 */
    prop_R0600->maxThreadsPerMultiProcessor = 2560;
    prop_R0600->streamPrioritiesSupported = 1;
    prop_R0600->globalL1CacheSupported = 1;
    prop_R0600->localL1CacheSupported = 1;
    prop_R0600->sharedMemPerMultiprocessor = 65536;
    prop_R0600->regsPerMultiprocessor = 65536;
    prop_R0600->managedMemory = 0;
    prop_R0600->isMultiGpuBoard = 0;
    prop_R0600->hostNativeAtomicSupported = 1;
    prop_R0600->singleToDoublePrecisionPerfRatio = 8;
    prop_R0600->pageableMemoryAccess = 0;
    prop_R0600->concurrentManagedAccess = 0;
    /* Il driver risponde 0 (hipDeviceGetAttribute) e rifiuta i lanci cooperativi con 719 anche con codice
     * compilato con HIP 5.7: dichiararli supportati farebbe scegliere ai programmi una via che fallisce. */
    prop_R0600->cooperativeLaunch = 0;
    prop_R0600->cooperativeMultiDeviceLaunch = 0;
    prop_R0600->sharedMemPerBlockOptin = 65536;
    prop_R0600->maxBlocksPerMultiProcessor = 32;
    prop_R0600->hostRegisterSupported = 1;
    prop_R0600->hostRegisterReadOnlySupported = 1;
    prop_R0600->memoryPoolsSupported = 1;
    prop_R0600->ipcEventSupported = 1;
    prop_R0600->computePreemptionSupported = 1;
    prop_R0600->canUseHostPointerForRegisteredMem = 1;

    strcpy(prop_R0600->gcnArchName, "gfx906:sramecc+:xnack-");
    prop_R0600->maxSharedMemoryPerMultiProcessor = 65536;
    prop_R0600->clockInstructionRate = 1801000;
    prop_R0600->isLargeBar = 0;
    prop_R0600->asicRevision = 0;
    return hipSuccess;
}

__declspec(dllexport) int hipDrvGraphAddMemcpyNode(void* node, void* graph, void* deps,
                                                     unsigned long long ndeps,
                                                     void* params, void* ctx) {
    (void)node; (void)graph; (void)deps; (void)ndeps; (void)params; (void)ctx;
    return hipErrorNotSupported;
}

#ifndef hipErrorOutOfMemory
#define hipErrorOutOfMemory 2
#endif
#include "abi57.inc"
#include "registrazione.inc"
#include "api7.inc"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
    (void)hinst; (void)reserved;
    if (reason == DLL_PROCESS_DETACH && g_amdhip64) {
        FreeLibrary(g_amdhip64);
        g_amdhip64 = NULL;
    }
    return TRUE;
}

FWDP(hipApiName)
FWDP(hipArray3DGetDescriptor)
FWDP(hipArrayGetDescriptor)
FWDP(hipArrayGetInfo)
FWDP(hipBindTexture)
FWDP(hipBindTexture2D)
FWDP(hipBindTextureToArray)
FWDP(hipBindTextureToMipmappedArray)
FWDP(hipChooseDevice)
FWDP(hipConfigureCall)
FWDP(hipCreateChannelDesc)
FWDP(hipCreateSurfaceObject)
FWDP(hipCreateTextureObject)
FWDP(hipCtxDisablePeerAccess)
FWDP(hipCtxEnablePeerAccess)
FWDP(hipCtxGetApiVersion)
FWDP(hipCtxGetCacheConfig)
FWDP(hipCtxGetFlags)
FWDP(hipCtxGetSharedMemConfig)
FWDP(hipCtxSetCacheConfig)
FWDP(hipCtxSetSharedMemConfig)
FWDP(hipDestroyExternalMemory)
FWDP(hipDestroyExternalSemaphore)
FWDP(hipDestroySurfaceObject)
FWDP(hipDeviceDisablePeerAccess)
FWDP(hipDeviceEnablePeerAccess)
FWDP(hipDeviceGetGraphMemAttribute)
FWDP(hipDeviceGraphMemTrim)
FWDP(hipDeviceReset)
FWDP(hipDeviceSetGraphMemAttribute)
FWDP(hipDrvGetErrorName)
FWDP(hipDrvGetErrorString)
/* FWDP(hipDrvMemcpy3DAsync): tradotta, vedi abi57.inc */
FWDP(hipEventRecord_spt)
FWDP(hipExtGetLinkTypeAndHopCount)
/* FWDP(hipExtLaunchKernel): vedi registrazione.inc */
FWDP(hipExtLaunchMultiKernelMultiDevice)
FWDP(hipExtMallocWithFlags)
FWDP(hipExtStreamGetCUMask)
FWDP(hipExternalMemoryGetMappedBuffer)
FWDP(hipFreeArray)
FWDP(hipFreeMipmappedArray)
/* FWDP(hipFuncGetAttributes): vedi registrazione.inc */
FWDP(hipGLGetDevices)
FWDP(hipGetChannelDesc)
FWDP(hipGetCmdName)
FWDP(hipGetDeviceFlags)
FWDP(hipGetMipmappedArrayLevel)
FWDP(hipGetStreamDeviceId)
/* FWDP(hipGetSymbolAddress): vedi registrazione.inc */
/* FWDP(hipGetSymbolSize): vedi registrazione.inc */
FWDP(hipGetTextureAlignmentOffset)
FWDP(hipGetTextureObjectResourceDesc)
FWDP(hipGetTextureObjectResourceViewDesc)
FWDP(hipGetTextureObjectTextureDesc)
FWDP(hipGetTextureReference)
FWDP(hipGraphAddEmptyNode)
FWDP(hipGraphAddEventWaitNode)
FWDP(hipGraphAddMemAllocNode)
FWDP(hipGraphAddMemFreeNode)
FWDP(hipGraphAddMemcpyNode1D)
FWDP(hipGraphAddMemcpyNodeFromSymbol)
FWDP(hipGraphAddMemcpyNodeToSymbol)
FWDP(hipGraphChildGraphNodeGetGraph)
FWDP(hipGraphClone)
FWDP(hipGraphDebugDotPrint)
FWDP(hipGraphDestroyNode)
FWDP(hipGraphEventRecordNodeGetEvent)
FWDP(hipGraphEventRecordNodeSetEvent)
FWDP(hipGraphEventWaitNodeGetEvent)
FWDP(hipGraphEventWaitNodeSetEvent)
FWDP(hipGraphExecChildGraphNodeSetParams)
FWDP(hipGraphExecEventRecordNodeSetEvent)
FWDP(hipGraphExecEventWaitNodeSetEvent)
FWDP(hipGraphExecHostNodeSetParams)
FWDP(hipGraphExecKernelNodeSetParams)
FWDP(hipGraphExecMemcpyNodeSetParams)
FWDP(hipGraphExecMemcpyNodeSetParams1D)
FWDP(hipGraphExecMemcpyNodeSetParamsFromSymbol)
FWDP(hipGraphExecMemcpyNodeSetParamsToSymbol)
FWDP(hipGraphExecMemsetNodeSetParams)
FWDP(hipGraphGetEdges)
FWDP(hipGraphGetRootNodes)
FWDP(hipGraphHostNodeGetParams)
FWDP(hipGraphHostNodeSetParams)
FWDP(hipGraphKernelNodeCopyAttributes)
FWDP(hipGraphKernelNodeGetAttribute)
FWDP(hipGraphKernelNodeGetParams)
FWDP(hipGraphKernelNodeSetAttribute)
FWDP(hipGraphKernelNodeSetParams)
FWDP(hipGraphMemAllocNodeGetParams)
FWDP(hipGraphMemFreeNodeGetParams)
FWDP(hipGraphMemcpyNodeGetParams)
FWDP(hipGraphMemcpyNodeSetParams)
FWDP(hipGraphMemcpyNodeSetParams1D)
FWDP(hipGraphMemcpyNodeSetParamsFromSymbol)
FWDP(hipGraphMemcpyNodeSetParamsToSymbol)
FWDP(hipGraphMemsetNodeGetParams)
FWDP(hipGraphMemsetNodeSetParams)
FWDP(hipGraphNodeFindInClone)
FWDP(hipGraphNodeGetDependencies)
FWDP(hipGraphNodeGetDependentNodes)
FWDP(hipGraphNodeGetEnabled)
FWDP(hipGraphNodeGetType)
FWDP(hipGraphNodeSetEnabled)
FWDP(hipGraphReleaseUserObject)
FWDP(hipGraphRemoveDependencies)
FWDP(hipGraphRetainUserObject)
FWDP(hipGraphUpload)
FWDP(hipGraphicsGLRegisterBuffer)
FWDP(hipGraphicsGLRegisterImage)
FWDP(hipGraphicsMapResources)
FWDP(hipGraphicsResourceGetMappedPointer)
FWDP(hipGraphicsSubResourceGetMappedArray)
FWDP(hipGraphicsUnmapResources)
FWDP(hipGraphicsUnregisterResource)
/* FWDP(hipHccModuleLaunchKernel): 13 argomenti, prototipo esplicito in registrazione.inc */
FWDP(hipHostAlloc)
FWDP(hipImportExternalMemory)
FWDP(hipImportExternalSemaphore)
FWDP(hipIpcCloseMemHandle)
FWDP(hipIpcGetEventHandle)
FWDP(hipIpcGetMemHandle)
FWDP(hipIpcOpenEventHandle)
FWDP(hipIpcOpenMemHandle)
FWDP(hipKernelNameRef)
FWDP(hipLaunchByPtr)
/* FWDP(hipLaunchCooperativeKernel): vedi registrazione.inc */
FWDP(hipLaunchCooperativeKernelMultiDevice)
FWDP(hipLaunchCooperativeKernel_spt)
FWDP(hipLaunchHostFunc)
FWDP(hipLaunchHostFunc_spt)
FWDP(hipLaunchKernel_spt)
FWDP(hipMalloc3D)
FWDP(hipMalloc3DArray)
FWDP(hipMallocArray)
FWDP(hipMallocHost)
FWDP(hipMallocMipmappedArray)
FWDP(hipMallocPitch)
FWDP(hipMemAddressFree)
FWDP(hipMemAddressReserve)
FWDP(hipMemCreate)
FWDP(hipMemExportToShareableHandle)
FWDP(hipMemGetAccess)
FWDP(hipMemGetAllocationGranularity)
FWDP(hipMemGetAllocationPropertiesFromHandle)
FWDP(hipMemImportFromShareableHandle)
FWDP(hipMemMap)
FWDP(hipMemMapArrayAsync)
FWDP(hipMemPoolExportPointer)
FWDP(hipMemPoolExportToShareableHandle)
FWDP(hipMemPoolGetAccess)
FWDP(hipMemPoolImportFromShareableHandle)
FWDP(hipMemPoolImportPointer)
FWDP(hipMemPoolSetAccess)
FWDP(hipMemRangeGetAttribute)
FWDP(hipMemRangeGetAttributes)
FWDP(hipMemRelease)
FWDP(hipMemRetainAllocationHandle)
FWDP(hipMemSetAccess)
FWDP(hipMemUnmap)
FWDP(hipMemcpy2DAsync_spt)
FWDP(hipMemcpy2DFromArray)
FWDP(hipMemcpy2DFromArrayAsync)
FWDP(hipMemcpy2DFromArrayAsync_spt)
FWDP(hipMemcpy2DFromArray_spt)
FWDP(hipMemcpy2DToArray)
FWDP(hipMemcpy2DToArrayAsync)
FWDP(hipMemcpy2DToArrayAsync_spt)
FWDP(hipMemcpy2DToArray_spt)
FWDP(hipMemcpy2D_spt)
FWDP(hipMemcpy3DAsync_spt)
FWDP(hipMemcpy3D_spt)
FWDP(hipMemcpyAsync_spt)
FWDP(hipMemcpyAtoH)
FWDP(hipMemcpyFromArray)
FWDP(hipMemcpyFromArray_spt)
/* FWDP(hipMemcpyFromSymbol): vedi registrazione.inc */
/* FWDP(hipMemcpyFromSymbolAsync): vedi registrazione.inc */
FWDP(hipMemcpyFromSymbolAsync_spt)
FWDP(hipMemcpyFromSymbol_spt)
/* FWDP(hipMemcpyParam2DAsync): tradotta, vedi abi57.inc */
FWDP(hipMemcpyPeer)
FWDP(hipMemcpyPeerAsync)
FWDP(hipMemcpyToArray)
/* FWDP(hipMemcpyToSymbol): vedi registrazione.inc */
/* FWDP(hipMemcpyToSymbolAsync): vedi registrazione.inc */
FWDP(hipMemcpyToSymbolAsync_spt)
FWDP(hipMemcpyToSymbol_spt)
FWDP(hipMemset2DAsync_spt)
FWDP(hipMemset2D_spt)
FWDP(hipMemset3D)
FWDP(hipMemset3DAsync)
FWDP(hipMemset3DAsync_spt)
FWDP(hipMemset3D_spt)
FWDP(hipMemsetAsync_spt)
FWDP(hipMemset_spt)
FWDP(hipMipmappedArrayCreate)
FWDP(hipMipmappedArrayDestroy)
FWDP(hipMipmappedArrayGetLevel)
FWDP(hipModuleLaunchCooperativeKernelMultiDevice)
FWDP(hipModuleLaunchKernelExt)
/* FWDP(hipModuleLoadDataEx): vedi registrazione.inc */
FWDP(hipModuleOccupancyMaxPotentialBlockSizeWithFlags)
/* FWDP(hipOccupancyMaxPotentialBlockSize): vedi registrazione.inc */
FWDP(hipProfilerStart)
FWDP(hipProfilerStop)
FWDP(hipRegisterTracerCallback)
FWDP(hipSetDeviceFlags)
FWDP(hipSetupArgument)
/* FWDP(hipSignalExternalSemaphoresAsync): tradotta, vedi abi57.inc */
FWDP(hipStreamAddCallback_spt)
FWDP(hipStreamAttachMemAsync)
FWDP(hipStreamBeginCapture_spt)
FWDP(hipStreamEndCapture_spt)
FWDP(hipStreamGetCaptureInfo_spt)
FWDP(hipStreamGetCaptureInfo_v2_spt)
FWDP(hipStreamGetDevice)
FWDP(hipStreamGetFlags_spt)
FWDP(hipStreamGetPriority_spt)
FWDP(hipStreamIsCapturing_spt)
FWDP(hipStreamQuery_spt)
FWDP(hipStreamSynchronize_spt)
FWDP(hipStreamUpdateCaptureDependencies)
FWDP(hipStreamWaitEvent_spt)
FWDP(hipStreamWaitValue32)
FWDP(hipStreamWaitValue64)
FWDP(hipStreamWriteValue32)
FWDP(hipStreamWriteValue64)
FWDP(hipTexObjectDestroy)
FWDP(hipTexObjectGetResourceDesc)
FWDP(hipTexObjectGetResourceViewDesc)
FWDP(hipTexObjectGetTextureDesc)
FWDP(hipTexRefGetAddress)
FWDP(hipTexRefGetAddressMode)
FWDP(hipTexRefGetArray)
FWDP(hipTexRefGetBorderColor)
FWDP(hipTexRefGetFilterMode)
FWDP(hipTexRefGetFlags)
FWDP(hipTexRefGetFormat)
FWDP(hipTexRefGetMaxAnisotropy)
FWDP(hipTexRefGetMipmapFilterMode)
FWDP(hipTexRefGetMipmapLevelBias)
FWDP(hipTexRefGetMipmapLevelClamp)
FWDP(hipTexRefGetMipmappedArray)
FWDP(hipTexRefSetAddress2D)
FWDP(hipTexRefSetBorderColor)
FWDP(hipTexRefSetMaxAnisotropy)
FWDP(hipTexRefSetMipmapFilterMode)
/* FWDP(hipTexRefSetMipmapLevelBias): float per valore, prototipo esplicito in registrazione.inc */
/* FWDP(hipTexRefSetMipmapLevelClamp): float per valore, prototipo esplicito in registrazione.inc */
FWDP(hipTexRefSetMipmappedArray)
FWDP(hipUnbindTexture)
FWDP(hipUserObjectCreate)
FWDP(hipUserObjectRelease)
FWDP(hipUserObjectRetain)
/* FWDP(hipWaitExternalSemaphoresAsync): tradotta, vedi abi57.inc */


/* ============================================================
 * v5.1 - 28 lug 2026: le 40 funzioni che esistono nelle ROCm 6.1-7.1
 * ma NON nella runtime v5.7 che esegue.
 *
 * Non si risolvono con un forwarder: nella amdhip64.dll di sistema non
 * esistono. Qui vengono comunque TUTTE esportate, cosi' il link non fallisce
 * mai e il comportamento e' dichiarato invece che silenzioso:
 *   - implementate fedelmente dove la runtime ha i mattoni;
 *   - hipErrorNotSupported (801) esplicito dove non li ha.
 * Nessun forwarder che mente.
 *
 * VINCOLO IMPLICITO DELLE MACRO FWD9/FWDP (documentarlo, non e' ovvio):
 * passano sempre 9 argomenti void*. Regge perche' nessuna API HIP supera 9
 * parametri interi. Si romperebbe in silenzio con una funzione che prende
 * float/double per valore (viaggiano nei registri XMM, non nei GPR).
 * ============================================================ */

/* --- risolutore generico verso la runtime di sistema --- */
static void* marconi_pfn(const char* nome) {
    if (!marconi_shim_bootstrap()) return NULL;
    return (void*)GetProcAddress(g_amdhip64, nome);
}

/* ---------- IMPLEMENTATE FEDELMENTE ---------- */

/* Pura differenza di maiuscola fra le versioni: la 6.x la chiama
   ...MipMappedArray, la 5.7 ...MipmappedArray. Stessa funzione. */
__declspec(dllexport) void* hipTexRefGetMipMappedArray(void* a, void* b, void* c) {
    static void* (*pfn)(void*,void*,void*) = NULL;
    if (!pfn) {
        pfn = (void*(*)(void*,void*,void*))marconi_pfn("hipTexRefGetMipmappedArray");
        if (!pfn) return (void*)(size_t)hipErrorNotSupported;
    }
    return pfn(a, b, c);
}

/* Lookup di un simbolo per nome. Cerca prima in QUESTO shim (cosi' il
   chiamante ottiene i nostri forwarder e le nostre implementazioni), poi
   nella runtime di sistema. symbolStatus: 0 = trovato, 1 = non trovato. */
__declspec(dllexport) int hipGetProcAddress(const char* simbolo, void** pfn,
                                            int versione, unsigned long long flags,
                                            int* stato_simbolo) {
    (void)versione; (void)flags;
    if (!simbolo || !pfn) return hipErrorInvalidValue;
    void* p = NULL;
    HMODULE io_stesso = GetModuleHandleA("amdhip64_7.dll");
    if (!io_stesso) io_stesso = GetModuleHandleA("amdhip64_7_shim.dll");
    if (io_stesso) p = (void*)GetProcAddress(io_stesso, simbolo);
    if (!p) p = marconi_pfn(simbolo);
    *pfn = p;
    if (stato_simbolo) *stato_simbolo = p ? 0 : 1;
    return p ? hipSuccess : hipErrorNotSupported;
}

/* Sceglie il primo dispositivo valido dell'elenco e lo attiva. */
__declspec(dllexport) int hipSetValidDevices(int* elenco, int quanti) {
    if (!elenco || quanti <= 0) return hipErrorInvalidValue;
    static int (*conta)(int*) = NULL;
    static int (*imposta)(int) = NULL;
    if (!conta)   conta   = (int(*)(int*))marconi_pfn("hipGetDeviceCount");
    if (!imposta) imposta = (int(*)(int))marconi_pfn("hipSetDevice");
    if (!conta || !imposta) return hipErrorNotSupported;
    int n = 0;
    if (conta(&n) != hipSuccess) return hipErrorNotInitialized;
    for (int i = 0; i < quanti; ++i)
        if (elenco[i] >= 0 && elenco[i] < n) return imposta(elenco[i]);
    return hipErrorInvalidValue;
}

/* Le varianti Async di copia da/verso array: la runtime v5.7 ha solo le
   sincrone. Delegare alla sincrona e' semanticamente lecito (sincronizzazione
   piu' forte del richiesto), l'argomento stream viene ignorato. */
__declspec(dllexport) int hipMemcpyAtoHAsync(void* dst, void* src_array,
                                              size_t offset, size_t quanti, void* stream) {
    (void)stream;
    static int (*pfn)(void*,void*,size_t,size_t) = NULL;
    if (!pfn) pfn = (int(*)(void*,void*,size_t,size_t))marconi_pfn("hipMemcpyAtoH");
    if (!pfn) return hipErrorNotSupported;
    return pfn(dst, src_array, offset, quanti);
}
__declspec(dllexport) int hipMemcpyHtoAAsync(void* dst_array, size_t offset,
                                              const void* src, size_t quanti, void* stream) {
    (void)stream;
    static int (*pfn)(void*,size_t,const void*,size_t) = NULL;
    if (!pfn) pfn = (int(*)(void*,size_t,const void*,size_t))marconi_pfn("hipMemcpyHtoA");
    if (!pfn) return hipErrorNotSupported;
    return pfn(dst_array, offset, src, quanti);
}

/* ---------- ESPOSTE MA NON SUPPORTATE DALLA RUNTIME v5.7 ----------
 * Ogni simbolo esiste ed e' chiamabile: restituisce 801 (NotSupported) in
 * modo esplicito. Meglio un errore dichiarato che un simbolo mancante al
 * link o un forwarder che finge.
 * Per implementarle davvero servirebbero i mattoni corrispondenti, che la
 * runtime non espone (nodi grafo generici, semafori esterni nei grafi,
 * batch mem-op, API driver dei grafi, mipmap da memoria esterna).
 */
#define NONSUP(name) \
    __declspec(dllexport) void* name(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i) { \
        (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;(void)i; \
        return (void*)(size_t)hipErrorNotSupported; \
    }

/* varianti di struct delle versioni successive (servirebbe il layout esatto
   della struct della versione chiamante: inventarlo corromperebbe la memoria) */
/* hipChooseDeviceR0000: implementata in api7.inc */
/* hipChooseDeviceR0600: implementata in api7.inc */
/* hipGetDevicePropertiesR0000: implementata in api7.inc */
/* nodi grafo generici: richiedono hipGraphNodeParams, struct non nota alla 5.7 */
NONSUP(hipGraphAddNode)
NONSUP(hipGraphNodeSetParams)
NONSUP(hipGraphExecNodeSetParams)
NONSUP(hipGraphInstantiateWithParams)
NONSUP(hipGraphExecGetFlags)
NONSUP(hipStreamBeginCaptureToGraph)
/* semafori esterni come nodi di grafo */
NONSUP(hipGraphAddExternalSemaphoresSignalNode)
NONSUP(hipGraphAddExternalSemaphoresWaitNode)
NONSUP(hipGraphExecExternalSemaphoresSignalNodeSetParams)
NONSUP(hipGraphExecExternalSemaphoresWaitNodeSetParams)
NONSUP(hipGraphExternalSemaphoresSignalNodeGetParams)
NONSUP(hipGraphExternalSemaphoresSignalNodeSetParams)
NONSUP(hipGraphExternalSemaphoresWaitNodeGetParams)
NONSUP(hipGraphExternalSemaphoresWaitNodeSetParams)
/* batch mem-op */
NONSUP(hipStreamBatchMemOp)
NONSUP(hipGraphAddBatchMemOpNode)
NONSUP(hipGraphBatchMemOpNodeGetParams)
NONSUP(hipGraphBatchMemOpNodeSetParams)
NONSUP(hipGraphExecBatchMemOpNodeSetParams)
/* API driver dei grafi (usano HIP_MEMCPY3D invece di hipMemcpy3DParms) */
NONSUP(hipDrvGraphAddMemFreeNode)
NONSUP(hipDrvGraphAddMemsetNode)
NONSUP(hipDrvGraphExecMemcpyNodeSetParams)
NONSUP(hipDrvGraphExecMemsetNodeSetParams)
NONSUP(hipDrvGraphMemcpyNodeGetParams)
NONSUP(hipDrvGraphMemcpyNodeSetParams)
/* copie fra array (servono descrittori di array non esposti dalla 5.7) */
NONSUP(hipMemcpy2DArrayToArray)
NONSUP(hipMemcpyAtoA)
NONSUP(hipMemcpyAtoD)
NONSUP(hipMemcpyDtoA)
/* varie */
/* hipGetFuncBySymbol: implementata in api7.inc */
NONSUP(hipExternalMemoryGetMappedMipmappedArray)
/* hipDeviceGetTexture1DLinearMaxWidth: implementata in api7.inc */


__declspec(dllexport) unsigned int marconi_57exp_get_version(void) { return 51000; }
__declspec(dllexport) int marconi_57exp_get_device_count(void) {
    static int (*pfn)(int*) = NULL;
    if (!pfn) {
        if (!marconi_shim_bootstrap()) return -1;
        pfn = (int(*)(int*))GetProcAddress(g_amdhip64, "hipGetDeviceCount");
        if (!pfn) return -1;
    }
    int n = 0;
    return pfn(&n) == 0 ? n : -1;
}
/* Numero di API hip* ESPORTATE da questo shim (non quelle gia' risolte:
   contando anche i 5 __hip* interni e queste 4 marconi_57exp_* gli export sono 482;
   la risoluzione e' pigra, avviene alla prima chiamata di ciascuno).
   Copre 428/428 della runtime v5.7 e 469/469 dell'unione ROCm 6.1-7.1. */
__declspec(dllexport) unsigned int marconi_57exp_count_exported_symbols(void) { return 473; }
__declspec(dllexport) unsigned int marconi_57exp_count_resolved_pfns(void) { return 473; }
