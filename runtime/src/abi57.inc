/*
 * abi57.inc - traduzione ABI fra le API HIP 6.x/7.x (quelle che lo shim espone) e la runtime
 * HIP 5.7 del driver (quella che esegue). 2 Solution, 12 nov 2026.
 *
 * Misurato confrontando gli header di ROCm 5.7 e 7.1 con clang (-fdump-record-layouts e
 * valori di tutti i 470 enumeratori comuni). Fra 5.7 e 7.x sono cambiati:
 *   1. hipMemoryType: rinumerato.
 *        5.7: Host=0 Device=1 Array=2 Unified=3 Managed=4
 *        7.x: Unregistered=0 Host=1 Device=2 Managed=3 Array=10 Unified=11
 *      Riguarda hipPointerGetAttribute(s), hipDrvPointerGetAttributes (in uscita) e le copie
 *      2D/3D con hip_Memcpy2D / HIP_MEMCPY3D (in ingresso).
 *   2. HIP_MEMCPY3D: in 5.7 i campi di dimensione sono unsigned int (128 byte), in 7.x size_t
 *      (184 byte). Va ricostruita campo per campo.
 *   3. hipExternalSemaphoreSignalParams / WaitParams: 7.x inserisce l'unione nvSciSync
 *      all'offset 8, quindi keyedMutex.key, timeoutMs e flags si spostano di 8 byte.
 * Le altre strutture cambiate (descrittori di memoria e semafori esterni, hipMemPoolProps,
 * hipMemAllocNodeParams, hipMemAllocationProp) aggiungono campi solo dove la 5.7 ha spazio
 * riservato o padding: passano invariate.
 * Senza questa traduzione un programma compilato con gli header 6.x/7.x leggeva la memoria
 * GPU come "host" e le copie 3D partivano con i campi sbagliati.
 */

#include <stdlib.h>
#include <limits.h>

static void* abi57_reale(const char* nome) {
    if (!marconi_shim_bootstrap()) return NULL;
    return (void*)GetProcAddress(g_amdhip64, nome);
}

typedef int (*abi57_f0)(void*, int, void*);
typedef int (*abi57_f1)(unsigned int, int*, void**, void*);
typedef int (*abi57_f2)(void*, const void*);
typedef int (*abi57_f3)(const void*);
typedef int (*abi57_f4)(const void*, void*);
typedef int (*abi57_f5)(const void*, const void*, unsigned int, void*);

#define ABI57_REALE(var, tipo, nome) \
    static tipo var = NULL; \
    if (!var) { var = (tipo)abi57_reale(nome); if (!var) return hipErrorNotSupported; }

/* hipMemoryType: 5.7 -> 7.x (uscita) e 7.x -> 5.7 (ingresso) */
static int abi57_tipo_da_57(int t) {
    switch (t) {
        case 0: return 1;   /* Host */
        case 1: return 2;   /* Device */
        case 2: return 10;  /* Array */
        case 3: return 11;  /* Unified */
        case 4: return 3;   /* Managed */
        default: return t;
    }
}
static int abi57_tipo_a_57(int t) {
    switch (t) {
        case 0: return 0;   /* Unregistered: per una copia e' memoria host */
        case 1: return 0;   /* Host */
        case 2: return 1;   /* Device */
        case 3: return 4;   /* Managed */
        case 10: return 2;  /* Array */
        case 11: return 3;  /* Unified */
        default: return t;
    }
}

#define ABI57_ATTR_MEMORY_TYPE 2   /* HIP_POINTER_ATTRIBUTE_MEMORY_TYPE, uguale in 5.7 e 7.x */

/* ---- uscita: tipo di memoria di un puntatore ---- */

/* La runtime 5.7 del driver CADE (access violation, lettura a 0xFFFFFFFFFFFFFFFF) su
 * hipDrvPointerGetAttributes(MEMORY_TYPE) per alcuni puntatori host non registrati, per esempio la
 * memoria di un tensore CPU di PyTorch: torch.Tensor.is_pinned() sotto ZLUDA terminava il processo.
 * hipPointerGetAttributes (la struttura) sugli stessi puntatori risponde invece hipErrorInvalidValue
 * senza cadere. Quindi prima si chiede a lei se il puntatore appartiene a HIP. Se non appartiene, il
 * tipo e' "non registrato" (0, come in CUDA) e la 5.7 non lo vede mai. Le altre informazioni passano. */
static int abi57_registrato(void* ptr) {
    static abi57_f2 pga = NULL;
    unsigned long long a[4];
    if (!pga) { pga = (abi57_f2)abi57_reale("hipPointerGetAttributes"); if (!pga) return 1; }
    return pga(a, ptr) == hipSuccess;
}

__declspec(dllexport) int hipPointerGetAttribute(void* data, int attribute, void* ptr) {
    ABI57_REALE(pfn, abi57_f0, "hipPointerGetAttribute");
    int rc;
    if (attribute == ABI57_ATTR_MEMORY_TYPE && !abi57_registrato(ptr)) return hipErrorInvalidValue;
    rc = pfn(data, attribute, ptr);
    if (rc == hipSuccess && data && attribute == ABI57_ATTR_MEMORY_TYPE) *(int*)data = abi57_tipo_da_57(*(int*)data);
    return rc;
}

__declspec(dllexport) int hipDrvPointerGetAttributes(unsigned int n, int* attributes, void** data, void* ptr) {
    ABI57_REALE(pfn, abi57_f1, "hipDrvPointerGetAttributes");
    int rc, chiede_tipo = 0;
    if (attributes && data)
        for (unsigned int i = 0; i < n; i++) if (attributes[i] == ABI57_ATTR_MEMORY_TYPE) chiede_tipo = 1;
    if (chiede_tipo && !abi57_registrato(ptr)) {
        /* puntatore non HIP: il tipo vale 0 (non registrato), gli altri attributi alla 5.7 */
        int* a2 = (int*)malloc(sizeof(int) * (n ? n : 1));
        void** d2 = (void**)malloc(sizeof(void*) * (n ? n : 1));
        unsigned int m = 0;
        if (!a2 || !d2) { free(a2); free(d2); return hipErrorOutOfMemory; }
        for (unsigned int i = 0; i < n; i++) {
            if (attributes[i] == ABI57_ATTR_MEMORY_TYPE) { if (data[i]) *(int*)data[i] = 0; }
            else { a2[m] = attributes[i]; d2[m] = data[i]; m++; }
        }
        rc = m ? pfn(m, a2, d2, ptr) : hipSuccess;
        free(a2); free(d2);
        return rc;
    }
    rc = pfn(n, attributes, data, ptr);
    if (rc == hipSuccess && attributes && data)
        for (unsigned int i = 0; i < n; i++)
            if (attributes[i] == ABI57_ATTR_MEMORY_TYPE && data[i]) *(int*)data[i] = abi57_tipo_da_57(*(int*)data[i]);
    return rc;
}

/* hipPointerAttribute_t: stesso layout in 5.7 e 7.x (32 byte), il tipo e' il primo campo */
__declspec(dllexport) int hipPointerGetAttributes(void* attributes, const void* ptr) {
    ABI57_REALE(pfn, abi57_f2, "hipPointerGetAttributes");
    int rc = pfn(attributes, ptr);
    if (rc == hipSuccess && attributes) *(int*)attributes = abi57_tipo_da_57(*(int*)attributes);
    return rc;
}

/* ---- ingresso: copie 2D (hip_Memcpy2D, stesso layout in 5.7 e 7.x: 128 byte) ---- */

#define ABI57_M2D_SRC_TIPO 16
#define ABI57_M2D_DST_TIPO 72
static void abi57_m2d(unsigned char* copia, const void* orig) {
    memcpy(copia, orig, 128);
    *(int*)(copia + ABI57_M2D_SRC_TIPO) = abi57_tipo_a_57(*(const int*)((const unsigned char*)orig + ABI57_M2D_SRC_TIPO));
    *(int*)(copia + ABI57_M2D_DST_TIPO) = abi57_tipo_a_57(*(const int*)((const unsigned char*)orig + ABI57_M2D_DST_TIPO));
}

__declspec(dllexport) int hipMemcpyParam2D(const void* p) {
    ABI57_REALE(pfn, abi57_f3, "hipMemcpyParam2D");
    unsigned long long copia[16];
    if (!p) return pfn(p);
    abi57_m2d((unsigned char*)copia, p);
    return pfn(copia);
}

__declspec(dllexport) int hipMemcpyParam2DAsync(const void* p, void* stream) {
    ABI57_REALE(pfn, abi57_f4, "hipMemcpyParam2DAsync");
    unsigned long long copia[16];
    if (!p) return pfn(p, stream);
    abi57_m2d((unsigned char*)copia, p);
    return pfn(copia, stream);
}

__declspec(dllexport) int hipDrvMemcpy2DUnaligned(const void* p) {
    ABI57_REALE(pfn, abi57_f3, "hipDrvMemcpy2DUnaligned");
    unsigned long long copia[16];
    if (!p) return pfn(p);
    abi57_m2d((unsigned char*)copia, p);
    return pfn(copia);
}

/* ---- ingresso: copie 3D (HIP_MEMCPY3D, layout diverso) ---- */

typedef struct {   /* HIP_MEMCPY3D di HIP 6.x/7.x, 184 byte */
    size_t srcXInBytes, srcY, srcZ, srcLOD; int srcMemoryType;
    const void* srcHost; void* srcDevice; void* srcArray; size_t srcPitch, srcHeight;
    size_t dstXInBytes, dstY, dstZ, dstLOD; int dstMemoryType;
    void* dstHost; void* dstDevice; void* dstArray; size_t dstPitch, dstHeight;
    size_t WidthInBytes, Height, Depth;
} abi57_memcpy3d_7;

typedef struct {   /* HIP_MEMCPY3D di HIP 5.7, 128 byte */
    unsigned int srcXInBytes, srcY, srcZ, srcLOD; int srcMemoryType;
    const void* srcHost; void* srcDevice; void* srcArray; unsigned int srcPitch, srcHeight;
    unsigned int dstXInBytes, dstY, dstZ, dstLOD; int dstMemoryType;
    void* dstHost; void* dstDevice; void* dstArray; unsigned int dstPitch, dstHeight;
    unsigned int WidthInBytes, Height, Depth;
} abi57_memcpy3d_57;

typedef char abi57_controllo_7[(sizeof(abi57_memcpy3d_7) == 184) ? 1 : -1];
typedef char abi57_controllo_57[(sizeof(abi57_memcpy3d_57) == 128) ? 1 : -1];

#define ABI57_U32(dst, src) do { if ((src) > UINT_MAX) return hipErrorInvalidValue; (dst) = (unsigned int)(src); } while (0)
static int abi57_m3d(abi57_memcpy3d_57* d, const abi57_memcpy3d_7* s) {
    ABI57_U32(d->srcXInBytes, s->srcXInBytes); ABI57_U32(d->srcY, s->srcY); ABI57_U32(d->srcZ, s->srcZ); ABI57_U32(d->srcLOD, s->srcLOD);
    d->srcMemoryType = abi57_tipo_a_57(s->srcMemoryType);
    d->srcHost = s->srcHost; d->srcDevice = s->srcDevice; d->srcArray = s->srcArray;
    ABI57_U32(d->srcPitch, s->srcPitch); ABI57_U32(d->srcHeight, s->srcHeight);
    ABI57_U32(d->dstXInBytes, s->dstXInBytes); ABI57_U32(d->dstY, s->dstY); ABI57_U32(d->dstZ, s->dstZ); ABI57_U32(d->dstLOD, s->dstLOD);
    d->dstMemoryType = abi57_tipo_a_57(s->dstMemoryType);
    d->dstHost = s->dstHost; d->dstDevice = s->dstDevice; d->dstArray = s->dstArray;
    ABI57_U32(d->dstPitch, s->dstPitch); ABI57_U32(d->dstHeight, s->dstHeight);
    ABI57_U32(d->WidthInBytes, s->WidthInBytes); ABI57_U32(d->Height, s->Height); ABI57_U32(d->Depth, s->Depth);
    return hipSuccess;
}

__declspec(dllexport) int hipDrvMemcpy3D(const void* p) {
    ABI57_REALE(pfn, abi57_f3, "hipDrvMemcpy3D");
    abi57_memcpy3d_57 d;
    int rc;
    if (!p) return hipErrorInvalidValue;
    memset(&d, 0, sizeof(d));
    rc = abi57_m3d(&d, (const abi57_memcpy3d_7*)p);
    if (rc != hipSuccess) return rc;
    return pfn(&d);
}

__declspec(dllexport) int hipDrvMemcpy3DAsync(const void* p, void* stream) {
    ABI57_REALE(pfn, abi57_f4, "hipDrvMemcpy3DAsync");
    abi57_memcpy3d_57 d;
    int rc;
    if (!p) return hipErrorInvalidValue;
    memset(&d, 0, sizeof(d));
    rc = abi57_m3d(&d, (const abi57_memcpy3d_7*)p);
    if (rc != hipSuccess) return rc;
    return pfn(&d, stream);
}

/* ---- ingresso: semafori esterni (parametri di 144 byte in 7.x, 136 in 5.7) ---- */
/* offset 7.x: valore 0, key 16, timeoutMs 24 (solo wait), flags 72
 * offset 5.7: valore 0, key 8,  timeoutMs 16 (solo wait), flags 64 */
static unsigned char* abi57_sem(const unsigned char* p7, unsigned int n, int wait) {
    unsigned char* p57 = (unsigned char*)calloc(n ? n : 1, 136);
    if (!p57) return NULL;
    for (unsigned int i = 0; i < n; i++) {
        const unsigned char* s = p7 + (size_t)i * 144;
        unsigned char* d = p57 + (size_t)i * 136;
        memcpy(d + 0, s + 0, 8);                 /* fence.value */
        memcpy(d + 8, s + 16, 8);                /* keyedMutex.key */
        if (wait) memcpy(d + 16, s + 24, 4);     /* keyedMutex.timeoutMs */
        memcpy(d + 64, s + 72, 4);               /* flags */
    }
    return p57;
}

__declspec(dllexport) int hipSignalExternalSemaphoresAsync(const void* sem, const void* params, unsigned int n, void* stream) {
    ABI57_REALE(pfn, abi57_f5, "hipSignalExternalSemaphoresAsync");
    unsigned char* p57;
    int rc;
    if (!params) return pfn(sem, params, n, stream);
    p57 = abi57_sem((const unsigned char*)params, n, 0);
    if (!p57) return hipErrorOutOfMemory;
    rc = pfn(sem, p57, n, stream);
    free(p57);
    return rc;
}

__declspec(dllexport) int hipWaitExternalSemaphoresAsync(const void* sem, const void* params, unsigned int n, void* stream) {
    ABI57_REALE(pfn, abi57_f5, "hipWaitExternalSemaphoresAsync");
    unsigned char* p57;
    int rc;
    if (!params) return pfn(sem, params, n, stream);
    p57 = abi57_sem((const unsigned char*)params, n, 1);
    if (!p57) return hipErrorOutOfMemory;
    rc = pfn(sem, p57, n, stream);
    free(p57);
    return rc;
}
