/*
 * hip_graph_extra.h — HIP Graph Capture API estese (Marconi 5.7 Expansion Pack).
 * 2 Solution, 19 giu 2026 T07:10 CEST.
 *
 * Implementazione user-space sopra amdhip64.dll v5.7 della Capture API + Graph node
 * derivata dal pattern ZLUDA Fix #1 cuGraphAdd*Node FromCuda (sediment ZLUDA.md T16:05 17 giu).
 *
 * Sblocca PyTorch torch.cuda.graph() + make_graphed_callables() per fusion eager-mode.
 */

#ifndef HIP_GRAPH_EXTRA_MARCONI_57EXP_H
#define HIP_GRAPH_EXTRA_MARCONI_57EXP_H

#include "hip_compat_macros.h"

/* Forward decl */
typedef struct ihipStream_t* hipStream_t;
typedef struct ihipEvent_t* hipEvent_t;
typedef int hipError_t;

/* Opaque graph types (implementati nel shim) */
typedef struct hipGraph_st* hipGraph_t;
typedef struct hipGraphNode_st* hipGraphNode_t;
typedef struct hipGraphExec_st* hipGraphExec_t;

/* Stream capture status enum (= CUDA api) */
typedef enum hipStreamCaptureStatus {
    hipStreamCaptureStatusNone        = 0,
    hipStreamCaptureStatusActive      = 1,
    hipStreamCaptureStatusInvalidated = 2
} hipStreamCaptureStatus;

typedef enum hipStreamCaptureMode {
    hipStreamCaptureModeGlobal       = 0,
    hipStreamCaptureModeThreadLocal  = 1,
    hipStreamCaptureModeRelaxed      = 2
} hipStreamCaptureMode;

/* Kernel node params (subset Drvr API) */
typedef struct hipKernelNodeParams {
    void*        func;            /* hipFunction_t */
    unsigned int gridDim[3];      /* x, y, z */
    unsigned int blockDim[3];     /* x, y, z */
    unsigned int sharedMemBytes;
    void**       kernelParams;    /* array di puntatori */
    void**       extra;           /* opaque buffer driver */
} hipKernelNodeParams;

/* Memcpy node params (subset) */
typedef struct hipMemcpyNodeParams {
    void*       dst;
    const void* src;
    size_t      sizeBytes;
    int         kind;             /* hipMemcpyKind */
} hipMemcpyNodeParams;

/* Memset node params (subset) */
typedef struct hipMemsetNodeParams {
    void*       dst;
    unsigned int value;
    size_t      elementSize;
    size_t      width;
    size_t      height;
    size_t      pitch;
} hipMemsetNodeParams;

#ifdef __cplusplus
extern "C" {
#endif

/* ----- Stream Capture API (lifecycle) ----- */

hipError_t hipStreamBeginCapture(hipStream_t stream, hipStreamCaptureMode mode);
hipError_t hipStreamEndCapture(hipStream_t stream, hipGraph_t* pGraph);
hipError_t hipStreamGetCaptureInfo(hipStream_t stream,
                                   hipStreamCaptureStatus* pCaptureStatus,
                                   unsigned long long* pId);
hipError_t hipStreamIsCapturing(hipStream_t stream, hipStreamCaptureStatus* pCaptureStatus);

/* ----- Graph lifecycle ----- */

hipError_t hipGraphCreate(hipGraph_t* pGraph, unsigned int flags);
hipError_t hipGraphDestroy(hipGraph_t graph);
hipError_t hipGraphInstantiate(hipGraphExec_t* pGraphExec, hipGraph_t graph,
                                hipGraphNode_t* pErrorNode, char* pLogBuffer, size_t bufferSize);
hipError_t hipGraphExecDestroy(hipGraphExec_t graphExec);
hipError_t hipGraphLaunch(hipGraphExec_t graphExec, hipStream_t stream);

/* ----- Graph node Add* (5 entries) ----- */

hipError_t hipGraphAddKernelNode(hipGraphNode_t* pGraphNode, hipGraph_t graph,
                                  const hipGraphNode_t* pDependencies, size_t numDependencies,
                                  const hipKernelNodeParams* pNodeParams);

hipError_t hipGraphAddMemcpyNode(hipGraphNode_t* pGraphNode, hipGraph_t graph,
                                  const hipGraphNode_t* pDependencies, size_t numDependencies,
                                  const hipMemcpyNodeParams* pNodeParams);

hipError_t hipGraphAddMemsetNode(hipGraphNode_t* pGraphNode, hipGraph_t graph,
                                  const hipGraphNode_t* pDependencies, size_t numDependencies,
                                  const hipMemsetNodeParams* pNodeParams);

hipError_t hipGraphAddEventRecordNode(hipGraphNode_t* pGraphNode, hipGraph_t graph,
                                       const hipGraphNode_t* pDependencies, size_t numDependencies,
                                       hipEvent_t event);

hipError_t hipGraphAddDependencies(hipGraph_t graph,
                                    const hipGraphNode_t* pFrom,
                                    const hipGraphNode_t* pTo,
                                    size_t numDependencies);

#ifdef __cplusplus
}
#endif

#endif /* HIP_GRAPH_EXTRA_MARCONI_57EXP_H */
