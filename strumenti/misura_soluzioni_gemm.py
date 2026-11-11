"""
Marconi 5.7 v2 - misura delle soluzioni rocBLAS per una GEMM.

Enumera con rocblas_gemm_ex_get_solutions tutte le soluzioni che rocBLAS ha per
una data shape, poi le esegue una per una selezionandole con solution_index, e
riporta quale batte la scelta automatica.

SICUREZZA (protocollo anti-TDR del sedimento, incidente 22 giu):
  - i kernarg li costruisce rocBLAS: nessun lancio a mano
  - buffer allocati UNA volta, mai nel ciclo
  - ogni soluzione e' VALIDATA per correttezza prima di essere cronometrata;
    se sbaglia viene scartata e non cronometrata
  - pausa fra le soluzioni, iterazioni poche, un solo processo

Uso:
  python misura_soluzioni_gemm.py M N K [iter] [tipo] [transA] [transB]
    tipo   : f16 = HH_HPA (fp16 in/out, calcolo fp32) - il percorso fp16 dei framework
             f32 = SGEMM (fp32 ovunque)
    transA/transB: N oppure T   (default T N)
"""
import ctypes as C
import os, sys, time

# Kit Marconi installato: MARCONI_ROOT (pacchetto delle Releases), altrimenti la cartella storica.
MARCONI = os.environ.get("MARCONI_ROOT", r"C:\rocm_5_7_expansion_pack")
ROCBLAS_BIN = os.environ.get("ROCBLAS_BIN", r"C:\Program Files\AMD\ROCm\5.7\bin")
os.environ.setdefault("HIP_PATH", MARCONI + "\\")
os.environ.setdefault("ROCBLAS_TENSILE_LIBPATH", os.path.join(ROCBLAS_BIN, "rocblas", "library"))
os.environ.setdefault("GGML_HIP_NO_VMM", "1")     # hang fix gfx906, MAI cambiare
os.environ.setdefault("HSA_ENABLE_SDMA", "0")     # hang fix gfx906, MAI cambiare
os.environ.setdefault("HIP_VISIBLE_DEVICES", "0")
os.environ.setdefault("AMD_LOG_LEVEL", "0")
for d in (ROCBLAS_BIN, os.path.join(MARCONI, "bin")):
    if os.path.isdir(d):
        os.add_dll_directory(d)

hip = C.CDLL(r"C:\Windows\System32\amdhip64.dll")
rb = C.CDLL(os.path.join(ROCBLAS_BIN, "rocblas.dll"))   # ROCBLAS_BIN=%MARCONI_ROOT%\bin per il rocBLAS del kit

OP = {"N": 111, "T": 112}
DT_F16, DT_F32 = 150, 151
ALGO_STANDARD, ALGO_SOLUTION = 0, 1
H2D, D2H = 1, 2
TETTO_SCALARE = 13.44          # rate fp32 di gfx906: tetto del percorso non-packed


def hip_ok(rc, dove):
    if rc != 0:
        raise RuntimeError(f"HIP errore {rc} in {dove}")


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 1
    M, N, K = (int(x) for x in sys.argv[1:4])
    ITER = int(sys.argv[4]) if len(sys.argv) > 4 else 5
    tipo = (sys.argv[5] if len(sys.argv) > 5 else "f16").lower()
    tA = (sys.argv[6] if len(sys.argv) > 6 else "T").upper()
    tB = (sys.argv[7] if len(sys.argv) > 7 else "N").upper()

    import numpy as np

    if tipo == "f16":
        dt_io, np_io = DT_F16, np.float16
        famiglia = "HH_HPA (fp16 in/out, calcolo fp32)"
    else:
        dt_io, np_io = DT_F32, np.float32
        famiglia = "SGEMM (fp32 ovunque)"

    lda = M if tA == "N" else K
    ldb = K if tB == "N" else N
    ldc = M
    n_a = lda * (K if tA == "N" else M)
    n_b = ldb * (N if tB == "N" else K)
    n_c = ldc * N

    print(f"GEMM {famiglia}")
    print(f"  M={M} N={N} K={K}  transA={tA} transB={tB}  iter={ITER}")

    val = np_io(0.0625)                 # esatto in fp16
    atteso = K / 256.0                  # somma su K di (1/16)^2
    A = np.full(n_a, val, dtype=np_io)
    B = np.full(n_b, val, dtype=np_io)
    Dh = np.zeros(n_c, dtype=np_io)

    dA, dB, dC, dD = (C.c_void_p() for _ in range(4))
    hip_ok(hip.hipMalloc(C.byref(dA), C.c_size_t(A.nbytes)), "malloc A")
    hip_ok(hip.hipMalloc(C.byref(dB), C.c_size_t(B.nbytes)), "malloc B")
    hip_ok(hip.hipMalloc(C.byref(dC), C.c_size_t(Dh.nbytes)), "malloc C")
    hip_ok(hip.hipMalloc(C.byref(dD), C.c_size_t(Dh.nbytes)), "malloc D")
    hip_ok(hip.hipMemcpy(dA, A.ctypes.data_as(C.c_void_p), C.c_size_t(A.nbytes), H2D), "copy A")
    hip_ok(hip.hipMemcpy(dB, B.ctypes.data_as(C.c_void_p), C.c_size_t(B.nbytes), H2D), "copy B")
    hip_ok(hip.hipMemset(dC, 0, C.c_size_t(Dh.nbytes)), "memset C")

    handle = C.c_void_p()
    if rb.rocblas_create_handle(C.byref(handle)) != 0:
        print("rocblas_create_handle FALLITO")
        return 3

    alpha, beta = C.c_float(1.0), C.c_float(0.0)

    def args():
        return (handle, OP[tA], OP[tB], C.c_int(M), C.c_int(N), C.c_int(K),
                C.byref(alpha),
                dA, C.c_int(dt_io), C.c_int(lda),
                dB, C.c_int(dt_io), C.c_int(ldb),
                C.byref(beta),
                dC, C.c_int(dt_io), C.c_int(ldc),
                dD, C.c_int(dt_io), C.c_int(ldc),
                C.c_int(DT_F32))

    n_sol = C.c_int(0)
    if rb.rocblas_gemm_ex_get_solutions(*args(), C.c_int(ALGO_STANDARD), C.c_uint(0),
                                        None, C.byref(n_sol)) != 0 or n_sol.value <= 0:
        print("  enumerazione soluzioni fallita")
        return 4
    lista = (C.c_int * n_sol.value)()
    rb.rocblas_gemm_ex_get_solutions(*args(), C.c_int(ALGO_STANDARD), C.c_uint(0),
                                     lista, C.byref(n_sol))
    print(f"  soluzioni disponibili: {n_sol.value}")

    flop = 2.0 * M * N * K

    def cronometra(algo, idx):
        hip_ok(hip.hipMemset(dD, 0, C.c_size_t(Dh.nbytes)), "memset D")
        if rb.rocblas_gemm_ex(*args(), C.c_int(algo), C.c_int(idx), C.c_uint(0)) != 0:
            return None
        hip_ok(hip.hipDeviceSynchronize(), "sync")
        hip_ok(hip.hipMemcpy(Dh.ctypes.data_as(C.c_void_p), dD,
                             C.c_size_t(Dh.nbytes), D2H), "copy D")
        camp = np.array([Dh[0], Dh[n_c // 2], Dh[n_c - 1]], dtype=np.float64)
        if not np.all(np.isfinite(camp)) or \
           not np.all(np.abs(camp - atteso) <= max(1e-3, atteso * 2e-3)):
            return None                      # scartata: risultato errato
        t0 = time.perf_counter()
        for _ in range(ITER):
            if rb.rocblas_gemm_ex(*args(), C.c_int(algo), C.c_int(idx), C.c_uint(0)) != 0:
                return None
        hip_ok(hip.hipDeviceSynchronize(), "sync bench")
        return flop / ((time.perf_counter() - t0) / ITER) / 1e12

    base = cronometra(ALGO_STANDARD, 0)
    time.sleep(0.2)
    esiti = []
    for idx in lista:
        tf = cronometra(ALGO_SOLUTION, idx)
        if tf is not None:
            esiti.append((tf, idx))
        time.sleep(0.15)

    esiti.sort(reverse=True)
    print(f"  automatica : {base:6.2f} TFLOPS" if base else "  automatica : fallita")
    if esiti:
        print(f"  migliore   : {esiti[0][0]:6.2f} TFLOPS  (idx={esiti[0][1]})")
        if base:
            marca = "   [packed: sopra il tetto scalare]" if esiti[0][0] > TETTO_SCALARE else ""
            print(f"  GUADAGNO   : {esiti[0][0]/base:5.2f}x{marca}")
        print("  prime 3    : " + "  ".join(f"idx={i}:{t:.2f}" for t, i in esiti[:3]))
        print(f"  valide     : {len(esiti)}/{n_sol.value}")

    rb.rocblas_destroy_handle(handle)
    for p in (dA, dB, dC, dD):
        hip.hipFree(p)
    return 0


if __name__ == "__main__":
    sys.exit(main())
