/*
 * hip_compat_macros.h — macros di rilevamento Marconi 5.7 Expansion Pack.
 * 2 Solution, 19 giu 2026 T07:10 CEST.
 *
 * Permette al codice utente di rilevare la presenza dell'expansion pack
 * e attivare condizionalmente le API estese senza rompere build su 5.7 puro.
 */

#ifndef HIP_COMPAT_MACROS_MARCONI_57EXP_H
#define HIP_COMPAT_MACROS_MARCONI_57EXP_H

/* Versione expansion pack */
#define HIP_MARCONI_57EXP_VERSION_MAJOR 1
#define HIP_MARCONI_57EXP_VERSION_MINOR 0
#define HIP_MARCONI_57EXP_VERSION_PATCH 0

/* Macro di presenza pack — definita quando questo header è incluso */
#define HAVE_HIP_MARCONI_57EXP 1

/* Versioni HIP API estese — usare per #ifdef nel codice utente */
#define HIP_HAS_MALLOC_ASYNC         1  /* hipMallocAsync/hipFreeAsync via shim */
#define HIP_HAS_GRAPH_ADD_NODES      1  /* hipGraphAdd{Kernel,Memcpy,Memset,Event,Dep}Node via shim */
#define HIP_HAS_STREAM_CAPTURE       1  /* hipStreamBeginCapture/EndCapture via shim */
#define HIP_HAS_MODULE_LAUNCH_COV4   1  /* hipModuleLaunchKernel forced cov4 via shim */

/*
 * Stack runtime sottostante:
 *   amdhip64.dll v5.7 System32 (FileVersion 10.0.3584.0, Pro 25.Q3 driver)
 *   + amdhip64_7_shim.dll (user-space ext) — Marconi 57exp Expansion Pack
 *
 * Tutte le API estese implementate sopra l'ABI 5.7 base.
 * Code Object forzato cov4 (driver kernel-mode non popola cov5 hidden_args).
 */

#endif /* HIP_COMPAT_MACROS_MARCONI_57EXP_H */
