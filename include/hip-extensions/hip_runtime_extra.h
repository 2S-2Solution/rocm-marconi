/*
 * hip_runtime_extra.h — HIP runtime API estese (Marconi 5.7 Expansion Pack).
 * 2 Solution, 19 giu 2026 T07:10 CEST.
 *
 * Nuove API HIP runtime portate da ROCm 7.1 implementate user-space sopra
 * amdhip64.dll v5.7 System32. Linka contro amdhip64_7_shim.dll del pack.
 *
 * Categoria: async memory management + executor primitives.
 */

#ifndef HIP_RUNTIME_EXTRA_MARCONI_57EXP_H
#define HIP_RUNTIME_EXTRA_MARCONI_57EXP_H

#include "hip_compat_macros.h"

/* Tipi base HIP 5.7 (forward decl per non dipendere da hip_runtime.h) */
typedef struct ihipStream_t* hipStream_t;
typedef int hipError_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * hipMallocAsync — allocazione asincrona su stream.
 *
 * Implementazione shim:
 *   - sincronizza lo stream
 *   - chiama hipMalloc() base 5.7
 *   - registra il puntatore in mappa stream→ptr per free coordinato
 *
 * Limitazione: non è veramente asincrona perché 5.7 non espone memory pool kernel.
 * Funziona correttamente come API semantica (PyTorch CachingAllocator ok).
 */
hipError_t hipMallocAsync(void** ptr, size_t size, hipStream_t stream);

/*
 * hipFreeAsync — deallocazione asincrona su stream.
 *
 * Implementazione shim:
 *   - sincronizza lo stream
 *   - chiama hipFree() base 5.7
 *   - rimuove dalla mappa stream→ptr
 */
hipError_t hipFreeAsync(void* ptr, hipStream_t stream);

/*
 * hipMallocFromPoolAsync — placeholder.
 * NOTA: memory pool non supportati su 5.7. Funzione torna hipErrorNotSupported.
 * Inserita per compat header con PyTorch CUDA caching allocator chiamate.
 */
typedef void* hipMemPool_t;
hipError_t hipMallocFromPoolAsync(void** ptr, size_t size, hipMemPool_t pool, hipStream_t stream);

#ifdef __cplusplus
}
#endif

#endif /* HIP_RUNTIME_EXTRA_MARCONI_57EXP_H */
