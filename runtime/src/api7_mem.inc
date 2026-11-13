/*
 * api7_mem.inc - gruppo 2c: copie di memoria da/verso array e fra dispositivi, non esposte dalla 5.7.
 * 2 Solution, 13 nov 2026. Incluso da shim_main.c dopo api7_mod.inc.
 *
 * La 5.7 espone hipMemcpyHtoA/AtoH (array <-> host), hipMemcpy2DToArray/FromArray e hipMemcpy3D(Async).
 * Le copie array<->device e array<->array mancanti passano per un buffer host temporaneo: gli hipArray
 * servono alle texture, non al percorso di calcolo caldo, quindi la lentezza non conta. hipMemcpy3DPeer
 * su una sola GPU e' una copia 3D normale (src e dst sono lo stesso dispositivo).
 */

typedef int (*api7_f_atoh)(void* dst, void* srcArray, size_t srcOffset, size_t count);
typedef int (*api7_f_htoa)(void* dstArray, size_t dstOffset, const void* srcHost, size_t count);
typedef int (*api7_f_hd)(void* dst, const void* src, size_t n, int kind);

/* array -> device: array -> host temporaneo -> device */
__declspec(dllexport) int hipMemcpyAtoD(void* dstDevice, void* srcArray, size_t srcOffset, size_t count) {
    void* tmp;
    int rc;
    ABI57_REALE(atoh, api7_f_atoh, "hipMemcpyAtoH");
    { ABI57_REALE(cp2, api7_f_hd, "hipMemcpy"); (void)cp2; }
    if (!dstDevice || !srcArray) return hipErrorInvalidValue;
    if (!count) return hipSuccess;
    tmp = malloc(count);
    if (!tmp) return hipErrorOutOfMemory;
    rc = atoh(tmp, srcArray, srcOffset, count);
    if (rc == hipSuccess) { api7_f_hd cp = (api7_f_hd)abi57_reale("hipMemcpy"); rc = cp ? cp(dstDevice, tmp, count, REG_DEFAULT) : hipErrorNotSupported; }
    free(tmp);
    return rc;
}

/* device -> array: device -> host temporaneo -> array */
__declspec(dllexport) int hipMemcpyDtoA(void* dstArray, size_t dstOffset, void* srcDevice, size_t count) {
    void* tmp;
    int rc;
    ABI57_REALE(htoa, api7_f_htoa, "hipMemcpyHtoA");
    if (!dstArray || !srcDevice) return hipErrorInvalidValue;
    if (!count) return hipSuccess;
    tmp = malloc(count);
    if (!tmp) return hipErrorOutOfMemory;
    { api7_f_hd cp = (api7_f_hd)abi57_reale("hipMemcpy"); rc = cp ? cp(tmp, srcDevice, count, REG_DEFAULT) : hipErrorNotSupported; }
    if (rc == hipSuccess) rc = htoa(dstArray, dstOffset, tmp, count);
    free(tmp);
    return rc;
}

/* array -> array: array -> host temporaneo -> array */
__declspec(dllexport) int hipMemcpyAtoA(void* dstArray, size_t dstOffset, void* srcArray, size_t srcOffset, size_t count) {
    void* tmp;
    int rc;
    ABI57_REALE(atoh, api7_f_atoh, "hipMemcpyAtoH");
    { ABI57_REALE(htoa0, api7_f_htoa, "hipMemcpyHtoA"); (void)htoa0; }
    if (!dstArray || !srcArray) return hipErrorInvalidValue;
    if (!count) return hipSuccess;
    tmp = malloc(count);
    if (!tmp) return hipErrorOutOfMemory;
    rc = atoh(tmp, srcArray, srcOffset, count);
    if (rc == hipSuccess) { api7_f_htoa htoa = (api7_f_htoa)abi57_reale("hipMemcpyHtoA"); rc = htoa ? htoa(dstArray, dstOffset, tmp, count) : hipErrorNotSupported; }
    free(tmp);
    return rc;
}

/* 2D array -> array: array -> host (con pitch) -> array */
typedef int (*api7_f_2dfa)(void* dst, size_t dpitch, const void* srcArray, size_t wOff, size_t hOff, size_t w, size_t h, int kind);
typedef int (*api7_f_2dta)(void* dstArray, size_t wOff, size_t hOff, const void* src, size_t spitch, size_t w, size_t h, int kind);
__declspec(dllexport) int hipMemcpy2DArrayToArray(void* dst, size_t wOffDst, size_t hOffDst, const void* src,
                                                  size_t wOffSrc, size_t hOffSrc, size_t width, size_t height, int kind) {
    void* tmp;
    int rc;
    size_t pitch = width;
    ABI57_REALE(fromA, api7_f_2dfa, "hipMemcpy2DFromArray");
    { ABI57_REALE(toA0, api7_f_2dta, "hipMemcpy2DToArray"); (void)toA0; }
    if (!dst || !src) return hipErrorInvalidValue;
    if (!width || !height) return hipSuccess;
    (void)kind;
    tmp = malloc(pitch * height);
    if (!tmp) return hipErrorOutOfMemory;
    rc = fromA(tmp, pitch, src, wOffSrc, hOffSrc, width, height, 2 /*DeviceToHost*/);
    if (rc == hipSuccess) { api7_f_2dta toA = (api7_f_2dta)abi57_reale("hipMemcpy2DToArray"); rc = toA ? toA(dst, wOffDst, hOffDst, tmp, pitch, width, height, 1 /*HostToDevice*/) : hipErrorNotSupported; }
    free(tmp);
    return rc;
}

/* ---- copie 3D fra dispositivi (una sola GPU: copia 3D normale) ----
 * hipMemcpy3DPeerParms (168 byte) = hipMemcpy3DParms (160 byte) con srcDevice a 64 e dstDevice a 136 in
 * piu'. Si ricostruisce la hipMemcpy3DParms copiando srcArray/srcPos/srcPtr, dstArray/dstPos/dstPtr,
 * extent, e kind = Default. Offset misurati sugli header 7.16. */
typedef int (*api7_f_cpy3d)(const void* p);
typedef int (*api7_f_cpy3da)(const void* p, void* stream);

static void api7_peer_a_parms(const unsigned char* peer, unsigned char* parms) {
    memset(parms, 0, 160);
    memcpy(parms + 0,   peer + 0,   64);  /* srcArray(0)+srcPos(8)+srcPtr(32): 0..63, stesso layout */
    memcpy(parms + 64,  peer + 72,  64);  /* dstArray+dstPos+dstPtr: peer 72..135 -> parms 64..127 (salta srcDevice) */
    memcpy(parms + 128, peer + 144, 24);  /* extent: peer 144..167 -> parms 128..151 (salta dstDevice) */
    /* kind a 152: Default (0), gia' azzerato */
}

__declspec(dllexport) int hipMemcpy3DPeer(const void* p) {
    unsigned char parms[160];
    ABI57_REALE(cpy, api7_f_cpy3d, "hipMemcpy3D");
    if (!p) return hipErrorInvalidValue;
    api7_peer_a_parms((const unsigned char*)p, parms);
    return cpy(parms);
}

__declspec(dllexport) int hipMemcpy3DPeerAsync(const void* p, void* stream) {
    unsigned char parms[160];
    ABI57_REALE(cpy, api7_f_cpy3da, "hipMemcpy3DAsync");
    if (!p) return hipErrorInvalidValue;
    api7_peer_a_parms((const unsigned char*)p, parms);
    return cpy(parms, stream);
}
