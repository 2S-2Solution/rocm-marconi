# Marconi shim v5.1 - stato delle 473 funzioni esportate

Legenda: **OK** = funziona (inoltrata alla runtime v5.7 che la implementa) ·
**OK propria** = implementata dentro lo shim · **OK tradotta** = inoltrata alla 5.7 traducendo i
tipi cambiati fra 5.7 e 7.x (`runtime/src/abi57.inc`) · **NO** = esportata ma la runtime non ha i mattoni.

| # | funzione | stato | note |
|---|---|---|---|
| 1 | `hipApiName` | OK | inoltrata alla runtime v5.7 |
| 2 | `hipArray3DCreate` | OK | inoltrata alla runtime v5.7 |
| 3 | `hipArray3DGetDescriptor` | OK | inoltrata alla runtime v5.7 |
| 4 | `hipArrayCreate` | OK | inoltrata alla runtime v5.7 |
| 5 | `hipArrayDestroy` | OK | inoltrata alla runtime v5.7 |
| 6 | `hipArrayGetDescriptor` | OK | inoltrata alla runtime v5.7 |
| 7 | `hipArrayGetInfo` | OK | inoltrata alla runtime v5.7 |
| 8 | `hipBindTexture` | OK | inoltrata alla runtime v5.7 |
| 9 | `hipBindTexture2D` | OK | inoltrata alla runtime v5.7 |
| 10 | `hipBindTextureToArray` | OK | inoltrata alla runtime v5.7 |
| 11 | `hipBindTextureToMipmappedArray` | OK | inoltrata alla runtime v5.7 |
| 12 | `hipChooseDevice` | OK | inoltrata alla runtime v5.7 |
| 13 | `hipChooseDeviceR0000` | **NO** | servirebbe il layout esatto della struct della versione chiamante: inventarlo corromperebbe la memoria |
| 14 | `hipChooseDeviceR0600` | **NO** | servirebbe il layout esatto della struct della versione chiamante: inventarlo corromperebbe la memoria |
| 15 | `hipConfigureCall` | OK | inoltrata alla runtime v5.7 |
| 16 | `hipCreateChannelDesc` | OK | inoltrata alla runtime v5.7 |
| 17 | `hipCreateSurfaceObject` | OK | inoltrata alla runtime v5.7 |
| 18 | `hipCreateTextureObject` | OK | inoltrata alla runtime v5.7 |
| 19 | `hipCtxCreate` | OK | inoltrata alla runtime v5.7 |
| 20 | `hipCtxDestroy` | OK | inoltrata alla runtime v5.7 |
| 21 | `hipCtxDisablePeerAccess` | OK | inoltrata alla runtime v5.7 |
| 22 | `hipCtxEnablePeerAccess` | OK | inoltrata alla runtime v5.7 |
| 23 | `hipCtxGetApiVersion` | OK | inoltrata alla runtime v5.7 |
| 24 | `hipCtxGetCacheConfig` | OK | inoltrata alla runtime v5.7 |
| 25 | `hipCtxGetCurrent` | OK | inoltrata alla runtime v5.7 |
| 26 | `hipCtxGetDevice` | OK | inoltrata alla runtime v5.7 |
| 27 | `hipCtxGetFlags` | OK | inoltrata alla runtime v5.7 |
| 28 | `hipCtxGetSharedMemConfig` | OK | inoltrata alla runtime v5.7 |
| 29 | `hipCtxPopCurrent` | OK | inoltrata alla runtime v5.7 |
| 30 | `hipCtxPushCurrent` | OK | inoltrata alla runtime v5.7 |
| 31 | `hipCtxSetCacheConfig` | OK | inoltrata alla runtime v5.7 |
| 32 | `hipCtxSetCurrent` | OK | inoltrata alla runtime v5.7 |
| 33 | `hipCtxSetSharedMemConfig` | OK | inoltrata alla runtime v5.7 |
| 34 | `hipCtxSynchronize` | OK | inoltrata alla runtime v5.7 |
| 35 | `hipDestroyExternalMemory` | OK | inoltrata alla runtime v5.7 |
| 36 | `hipDestroyExternalSemaphore` | OK | inoltrata alla runtime v5.7 |
| 37 | `hipDestroySurfaceObject` | OK | inoltrata alla runtime v5.7 |
| 38 | `hipDestroyTextureObject` | OK | inoltrata alla runtime v5.7 |
| 39 | `hipDeviceCanAccessPeer` | OK | inoltrata alla runtime v5.7 |
| 40 | `hipDeviceComputeCapability` | OK | inoltrata alla runtime v5.7 |
| 41 | `hipDeviceDisablePeerAccess` | OK | inoltrata alla runtime v5.7 |
| 42 | `hipDeviceEnablePeerAccess` | OK | inoltrata alla runtime v5.7 |
| 43 | `hipDeviceGet` | OK | inoltrata alla runtime v5.7 |
| 44 | `hipDeviceGetAttribute` | OK | inoltrata alla runtime v5.7 |
| 45 | `hipDeviceGetByPCIBusId` | OK | inoltrata alla runtime v5.7 |
| 46 | `hipDeviceGetCacheConfig` | OK | inoltrata alla runtime v5.7 |
| 47 | `hipDeviceGetDefaultMemPool` | OK | inoltrata alla runtime v5.7 |
| 48 | `hipDeviceGetGraphMemAttribute` | OK | inoltrata alla runtime v5.7 |
| 49 | `hipDeviceGetLimit` | OK | inoltrata alla runtime v5.7 |
| 50 | `hipDeviceGetMemPool` | OK | inoltrata alla runtime v5.7 |
| 51 | `hipDeviceGetName` | OK | inoltrata alla runtime v5.7 |
| 52 | `hipDeviceGetP2PAttribute` | OK | inoltrata alla runtime v5.7 |
| 53 | `hipDeviceGetPCIBusId` | OK | inoltrata alla runtime v5.7 |
| 54 | `hipDeviceGetSharedMemConfig` | OK | inoltrata alla runtime v5.7 |
| 55 | `hipDeviceGetStreamPriorityRange` | OK | inoltrata alla runtime v5.7 |
| 56 | `hipDeviceGetTexture1DLinearMaxWidth` | **NO** | primitiva assente nella runtime 5.7 |
| 57 | `hipDeviceGetUuid` | OK | inoltrata alla runtime v5.7 |
| 58 | `hipDeviceGraphMemTrim` | OK | inoltrata alla runtime v5.7 |
| 59 | `hipDevicePrimaryCtxGetState` | OK | inoltrata alla runtime v5.7 |
| 60 | `hipDevicePrimaryCtxRelease` | OK | inoltrata alla runtime v5.7 |
| 61 | `hipDevicePrimaryCtxReset` | OK | inoltrata alla runtime v5.7 |
| 62 | `hipDevicePrimaryCtxRetain` | OK | inoltrata alla runtime v5.7 |
| 63 | `hipDevicePrimaryCtxSetFlags` | OK | inoltrata alla runtime v5.7 |
| 64 | `hipDeviceReset` | OK | inoltrata alla runtime v5.7 |
| 65 | `hipDeviceSetCacheConfig` | OK | inoltrata alla runtime v5.7 |
| 66 | `hipDeviceSetGraphMemAttribute` | OK | inoltrata alla runtime v5.7 |
| 67 | `hipDeviceSetLimit` | OK | inoltrata alla runtime v5.7 |
| 68 | `hipDeviceSetMemPool` | OK | inoltrata alla runtime v5.7 |
| 69 | `hipDeviceSetSharedMemConfig` | OK | inoltrata alla runtime v5.7 |
| 70 | `hipDeviceSynchronize` | OK | inoltrata alla runtime v5.7 |
| 71 | `hipDeviceTotalMem` | OK | inoltrata alla runtime v5.7 |
| 72 | `hipDriverGetVersion` | OK | inoltrata alla runtime v5.7 |
| 73 | `hipDrvGetErrorName` | OK | inoltrata alla runtime v5.7 |
| 74 | `hipDrvGetErrorString` | OK | inoltrata alla runtime v5.7 |
| 75 | `hipDrvGraphAddMemFreeNode` | **NO** | API driver dei grafi: usano HIP_MEMCPY3D, struct non definita nella 5.7 |
| 76 | `hipDrvGraphAddMemcpyNode` | **OK propria** | implementazione custom nello shim |
| 77 | `hipDrvGraphAddMemsetNode` | **NO** | API driver dei grafi: usano HIP_MEMCPY3D, struct non definita nella 5.7 |
| 78 | `hipDrvGraphExecMemcpyNodeSetParams` | **NO** | API driver dei grafi: usano HIP_MEMCPY3D, struct non definita nella 5.7 |
| 79 | `hipDrvGraphExecMemsetNodeSetParams` | **NO** | API driver dei grafi: usano HIP_MEMCPY3D, struct non definita nella 5.7 |
| 80 | `hipDrvGraphMemcpyNodeGetParams` | **NO** | API driver dei grafi: usano HIP_MEMCPY3D, struct non definita nella 5.7 |
| 81 | `hipDrvGraphMemcpyNodeSetParams` | **NO** | API driver dei grafi: usano HIP_MEMCPY3D, struct non definita nella 5.7 |
| 82 | `hipDrvLaunchKernelEx` | **OK propria** | implementazione custom nello shim |
| 83 | `hipDrvMemcpy2DUnaligned` | **OK tradotta** | tradotta: hipMemoryType 7.x -> 5.7 in hip_Memcpy2D |
| 84 | `hipDrvMemcpy3D` | **OK tradotta** | tradotta: HIP_MEMCPY3D 7.x (184 B, size_t) ricostruita in quella 5.7 (128 B, unsigned int) |
| 85 | `hipDrvMemcpy3DAsync` | **OK tradotta** | tradotta: HIP_MEMCPY3D 7.x (184 B, size_t) ricostruita in quella 5.7 (128 B, unsigned int) |
| 86 | `hipDrvPointerGetAttributes` | **OK tradotta** | tradotta: hipMemoryType 5.7 -> 7.x; per un puntatore non HIP il tipo vale 0 senza chiederlo alla 5.7 (cadeva) |
| 87 | `hipEventCreate` | OK | inoltrata alla runtime v5.7 |
| 88 | `hipEventCreateWithFlags` | OK | inoltrata alla runtime v5.7 |
| 89 | `hipEventDestroy` | OK | inoltrata alla runtime v5.7 |
| 90 | `hipEventElapsedTime` | OK | inoltrata alla runtime v5.7 |
| 91 | `hipEventQuery` | OK | inoltrata alla runtime v5.7 |
| 92 | `hipEventRecord` | OK | inoltrata alla runtime v5.7 |
| 93 | `hipEventRecord_spt` | OK | inoltrata alla runtime v5.7 |
| 94 | `hipEventSynchronize` | OK | inoltrata alla runtime v5.7 |
| 95 | `hipExtGetLastError` | **OK propria** | implementazione custom nello shim |
| 96 | `hipExtGetLinkTypeAndHopCount` | OK | inoltrata alla runtime v5.7 |
| 97 | `hipExtLaunchKernel` | OK | inoltrata alla runtime v5.7 |
| 98 | `hipExtLaunchMultiKernelMultiDevice` | OK | inoltrata alla runtime v5.7 |
| 99 | `hipExtMallocWithFlags` | OK | inoltrata alla runtime v5.7 |
| 100 | `hipExtModuleLaunchKernel` | OK | inoltrata alla runtime v5.7 |
| 101 | `hipExtStreamCreateWithCUMask` | OK | inoltrata alla runtime v5.7 |
| 102 | `hipExtStreamGetCUMask` | OK | inoltrata alla runtime v5.7 |
| 103 | `hipExternalMemoryGetMappedBuffer` | OK | inoltrata alla runtime v5.7 |
| 104 | `hipExternalMemoryGetMappedMipmappedArray` | **NO** | primitiva assente nella runtime 5.7 |
| 105 | `hipFree` | OK | inoltrata alla runtime v5.7 |
| 106 | `hipFreeArray` | OK | inoltrata alla runtime v5.7 |
| 107 | `hipFreeAsync` | OK | inoltrata alla runtime v5.7 |
| 108 | `hipFreeHost` | OK | inoltrata alla runtime v5.7 |
| 109 | `hipFreeMipmappedArray` | OK | inoltrata alla runtime v5.7 |
| 110 | `hipFuncGetAttribute` | OK | inoltrata alla runtime v5.7 |
| 111 | `hipFuncGetAttributes` | OK | inoltrata alla runtime v5.7 |
| 112 | `hipFuncSetAttribute` | OK | inoltrata alla runtime v5.7 |
| 113 | `hipFuncSetCacheConfig` | OK | inoltrata alla runtime v5.7 |
| 114 | `hipFuncSetSharedMemConfig` | OK | inoltrata alla runtime v5.7 |
| 115 | `hipGLGetDevices` | OK | inoltrata alla runtime v5.7 |
| 116 | `hipGetChannelDesc` | OK | inoltrata alla runtime v5.7 |
| 117 | `hipGetCmdName` | OK | inoltrata alla runtime v5.7 |
| 118 | `hipGetDevice` | OK | inoltrata alla runtime v5.7 |
| 119 | `hipGetDeviceCount` | OK | inoltrata alla runtime v5.7 |
| 120 | `hipGetDeviceFlags` | OK | inoltrata alla runtime v5.7 |
| 121 | `hipGetDeviceProperties` | OK | inoltrata alla runtime v5.7 |
| 122 | `hipGetDevicePropertiesR0000` | **NO** | servirebbe il layout esatto della struct della versione chiamante: inventarlo corromperebbe la memoria |
| 123 | `hipGetDevicePropertiesR0600` | **OK propria** | struttura R0600 (1640 B) riempita con valori FISSI della Radeon VII (nome, gfx906:sramecc+:xnack-, 16 GB, 60 CU, compute capability 7.0), senza chiamare il driver e per qualunque indice di dispositivo |
| 124 | `hipGetErrorName` | OK | inoltrata alla runtime v5.7 |
| 125 | `hipGetErrorString` | OK | inoltrata alla runtime v5.7 |
| 126 | `hipGetFuncBySymbol` | **NO** | primitiva assente nella runtime 5.7 |
| 127 | `hipGetLastError` | OK | inoltrata alla runtime v5.7 |
| 128 | `hipGetMipmappedArrayLevel` | OK | inoltrata alla runtime v5.7 |
| 129 | `hipGetProcAddress` | **OK propria** | implementata: cerca nello shim, poi nella runtime |
| 130 | `hipGetStreamDeviceId` | OK | inoltrata alla runtime v5.7 |
| 131 | `hipGetSymbolAddress` | OK | inoltrata alla runtime v5.7 |
| 132 | `hipGetSymbolSize` | OK | inoltrata alla runtime v5.7 |
| 133 | `hipGetTextureAlignmentOffset` | OK | inoltrata alla runtime v5.7 |
| 134 | `hipGetTextureObjectResourceDesc` | OK | inoltrata alla runtime v5.7 |
| 135 | `hipGetTextureObjectResourceViewDesc` | OK | inoltrata alla runtime v5.7 |
| 136 | `hipGetTextureObjectTextureDesc` | OK | inoltrata alla runtime v5.7 |
| 137 | `hipGetTextureReference` | OK | inoltrata alla runtime v5.7 |
| 138 | `hipGraphAddBatchMemOpNode` | **NO** | la 5.7 non ha la primitiva batch mem-op |
| 139 | `hipGraphAddChildGraphNode` | OK | inoltrata alla runtime v5.7 |
| 140 | `hipGraphAddDependencies` | OK | inoltrata alla runtime v5.7 |
| 141 | `hipGraphAddEmptyNode` | OK | inoltrata alla runtime v5.7 |
| 142 | `hipGraphAddEventRecordNode` | OK | inoltrata alla runtime v5.7 |
| 143 | `hipGraphAddEventWaitNode` | OK | inoltrata alla runtime v5.7 |
| 144 | `hipGraphAddExternalSemaphoresSignalNode` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 145 | `hipGraphAddExternalSemaphoresWaitNode` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 146 | `hipGraphAddHostNode` | OK | inoltrata alla runtime v5.7 |
| 147 | `hipGraphAddKernelNode` | OK | inoltrata alla runtime v5.7 |
| 148 | `hipGraphAddMemAllocNode` | OK | inoltrata alla runtime v5.7 |
| 149 | `hipGraphAddMemFreeNode` | OK | inoltrata alla runtime v5.7 |
| 150 | `hipGraphAddMemcpyNode` | OK | inoltrata alla runtime v5.7 |
| 151 | `hipGraphAddMemcpyNode1D` | OK | inoltrata alla runtime v5.7 |
| 152 | `hipGraphAddMemcpyNodeFromSymbol` | OK | inoltrata alla runtime v5.7 |
| 153 | `hipGraphAddMemcpyNodeToSymbol` | OK | inoltrata alla runtime v5.7 |
| 154 | `hipGraphAddMemsetNode` | OK | inoltrata alla runtime v5.7 |
| 155 | `hipGraphAddNode` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 156 | `hipGraphAddNode_v2` | **OK propria** | implementazione custom nello shim |
| 157 | `hipGraphBatchMemOpNodeGetParams` | **NO** | la 5.7 non ha la primitiva batch mem-op |
| 158 | `hipGraphBatchMemOpNodeSetParams` | **NO** | la 5.7 non ha la primitiva batch mem-op |
| 159 | `hipGraphChildGraphNodeGetGraph` | OK | inoltrata alla runtime v5.7 |
| 160 | `hipGraphClone` | OK | inoltrata alla runtime v5.7 |
| 161 | `hipGraphCreate` | OK | inoltrata alla runtime v5.7 |
| 162 | `hipGraphDebugDotPrint` | OK | inoltrata alla runtime v5.7 |
| 163 | `hipGraphDestroy` | OK | inoltrata alla runtime v5.7 |
| 164 | `hipGraphDestroyNode` | OK | inoltrata alla runtime v5.7 |
| 165 | `hipGraphEventRecordNodeGetEvent` | OK | inoltrata alla runtime v5.7 |
| 166 | `hipGraphEventRecordNodeSetEvent` | OK | inoltrata alla runtime v5.7 |
| 167 | `hipGraphEventWaitNodeGetEvent` | OK | inoltrata alla runtime v5.7 |
| 168 | `hipGraphEventWaitNodeSetEvent` | OK | inoltrata alla runtime v5.7 |
| 169 | `hipGraphExecBatchMemOpNodeSetParams` | **NO** | la 5.7 non ha la primitiva batch mem-op |
| 170 | `hipGraphExecChildGraphNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 171 | `hipGraphExecDestroy` | OK | inoltrata alla runtime v5.7 |
| 172 | `hipGraphExecEventRecordNodeSetEvent` | OK | inoltrata alla runtime v5.7 |
| 173 | `hipGraphExecEventWaitNodeSetEvent` | OK | inoltrata alla runtime v5.7 |
| 174 | `hipGraphExecExternalSemaphoresSignalNodeSetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 175 | `hipGraphExecExternalSemaphoresWaitNodeSetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 176 | `hipGraphExecGetFlags` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 177 | `hipGraphExecHostNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 178 | `hipGraphExecKernelNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 179 | `hipGraphExecMemcpyNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 180 | `hipGraphExecMemcpyNodeSetParams1D` | OK | inoltrata alla runtime v5.7 |
| 181 | `hipGraphExecMemcpyNodeSetParamsFromSymbol` | OK | inoltrata alla runtime v5.7 |
| 182 | `hipGraphExecMemcpyNodeSetParamsToSymbol` | OK | inoltrata alla runtime v5.7 |
| 183 | `hipGraphExecMemsetNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 184 | `hipGraphExecNodeSetParams` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 185 | `hipGraphExecUpdate` | OK | inoltrata alla runtime v5.7 |
| 186 | `hipGraphExternalSemaphoresSignalNodeGetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 187 | `hipGraphExternalSemaphoresSignalNodeSetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 188 | `hipGraphExternalSemaphoresWaitNodeGetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 189 | `hipGraphExternalSemaphoresWaitNodeSetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 190 | `hipGraphGetEdges` | OK | inoltrata alla runtime v5.7 |
| 191 | `hipGraphGetNodes` | OK | inoltrata alla runtime v5.7 |
| 192 | `hipGraphGetRootNodes` | OK | inoltrata alla runtime v5.7 |
| 193 | `hipGraphHostNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 194 | `hipGraphHostNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 195 | `hipGraphInstantiate` | OK | inoltrata alla runtime v5.7 |
| 196 | `hipGraphInstantiateWithFlags` | OK | inoltrata alla runtime v5.7 |
| 197 | `hipGraphInstantiateWithParams` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 198 | `hipGraphKernelNodeCopyAttributes` | OK | inoltrata alla runtime v5.7 |
| 199 | `hipGraphKernelNodeGetAttribute` | OK | inoltrata alla runtime v5.7 |
| 200 | `hipGraphKernelNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 201 | `hipGraphKernelNodeSetAttribute` | OK | inoltrata alla runtime v5.7 |
| 202 | `hipGraphKernelNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 203 | `hipGraphLaunch` | OK | inoltrata alla runtime v5.7 |
| 204 | `hipGraphLaunch_spt` | OK | inoltrata alla runtime v5.7 |
| 205 | `hipGraphMemAllocNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 206 | `hipGraphMemFreeNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 207 | `hipGraphMemcpyNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 208 | `hipGraphMemcpyNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 209 | `hipGraphMemcpyNodeSetParams1D` | OK | inoltrata alla runtime v5.7 |
| 210 | `hipGraphMemcpyNodeSetParamsFromSymbol` | OK | inoltrata alla runtime v5.7 |
| 211 | `hipGraphMemcpyNodeSetParamsToSymbol` | OK | inoltrata alla runtime v5.7 |
| 212 | `hipGraphMemsetNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 213 | `hipGraphMemsetNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 214 | `hipGraphNodeFindInClone` | OK | inoltrata alla runtime v5.7 |
| 215 | `hipGraphNodeGetDependencies` | OK | inoltrata alla runtime v5.7 |
| 216 | `hipGraphNodeGetDependentNodes` | OK | inoltrata alla runtime v5.7 |
| 217 | `hipGraphNodeGetEnabled` | OK | inoltrata alla runtime v5.7 |
| 218 | `hipGraphNodeGetType` | OK | inoltrata alla runtime v5.7 |
| 219 | `hipGraphNodeSetEnabled` | OK | inoltrata alla runtime v5.7 |
| 220 | `hipGraphNodeSetParams` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 221 | `hipGraphReleaseUserObject` | OK | inoltrata alla runtime v5.7 |
| 222 | `hipGraphRemoveDependencies` | OK | inoltrata alla runtime v5.7 |
| 223 | `hipGraphRetainUserObject` | OK | inoltrata alla runtime v5.7 |
| 224 | `hipGraphUpload` | OK | inoltrata alla runtime v5.7 |
| 225 | `hipGraphicsGLRegisterBuffer` | OK | inoltrata alla runtime v5.7 |
| 226 | `hipGraphicsGLRegisterImage` | OK | inoltrata alla runtime v5.7 |
| 227 | `hipGraphicsMapResources` | OK | inoltrata alla runtime v5.7 |
| 228 | `hipGraphicsResourceGetMappedPointer` | OK | inoltrata alla runtime v5.7 |
| 229 | `hipGraphicsSubResourceGetMappedArray` | OK | inoltrata alla runtime v5.7 |
| 230 | `hipGraphicsUnmapResources` | OK | inoltrata alla runtime v5.7 |
| 231 | `hipGraphicsUnregisterResource` | OK | inoltrata alla runtime v5.7 |
| 232 | `hipHccModuleLaunchKernel` | OK | inoltrata alla runtime v5.7 |
| 233 | `hipHostAlloc` | OK | inoltrata alla runtime v5.7 |
| 234 | `hipHostFree` | OK | inoltrata alla runtime v5.7 |
| 235 | `hipHostGetDevicePointer` | OK | inoltrata alla runtime v5.7 |
| 236 | `hipHostGetFlags` | OK | inoltrata alla runtime v5.7 |
| 237 | `hipHostMalloc` | OK | inoltrata alla runtime v5.7 |
| 238 | `hipHostRegister` | OK | inoltrata alla runtime v5.7 |
| 239 | `hipHostUnregister` | OK | inoltrata alla runtime v5.7 |
| 240 | `hipImportExternalMemory` | OK | inoltrata alla runtime v5.7 |
| 241 | `hipImportExternalSemaphore` | OK | inoltrata alla runtime v5.7 |
| 242 | `hipInit` | OK | inoltrata alla runtime v5.7 |
| 243 | `hipIpcCloseMemHandle` | OK | inoltrata alla runtime v5.7 |
| 244 | `hipIpcGetEventHandle` | OK | inoltrata alla runtime v5.7 |
| 245 | `hipIpcGetMemHandle` | OK | inoltrata alla runtime v5.7 |
| 246 | `hipIpcOpenEventHandle` | OK | inoltrata alla runtime v5.7 |
| 247 | `hipIpcOpenMemHandle` | OK | inoltrata alla runtime v5.7 |
| 248 | `hipKernelNameRef` | OK | inoltrata alla runtime v5.7 |
| 249 | `hipLaunchByPtr` | OK | inoltrata alla runtime v5.7 |
| 250 | `hipLaunchCooperativeKernel` | OK | inoltrata alla runtime v5.7 |
| 251 | `hipLaunchCooperativeKernelMultiDevice` | OK | inoltrata alla runtime v5.7 |
| 252 | `hipLaunchCooperativeKernel_spt` | OK | inoltrata alla runtime v5.7 |
| 253 | `hipLaunchHostFunc` | OK | inoltrata alla runtime v5.7 |
| 254 | `hipLaunchHostFunc_spt` | OK | inoltrata alla runtime v5.7 |
| 255 | `hipLaunchKernel` | OK | inoltrata alla runtime v5.7 |
| 256 | `hipLaunchKernel_spt` | OK | inoltrata alla runtime v5.7 |
| 257 | `hipMalloc` | OK | inoltrata alla runtime v5.7 |
| 258 | `hipMalloc3D` | OK | inoltrata alla runtime v5.7 |
| 259 | `hipMalloc3DArray` | OK | inoltrata alla runtime v5.7 |
| 260 | `hipMallocArray` | OK | inoltrata alla runtime v5.7 |
| 261 | `hipMallocAsync` | OK | inoltrata alla runtime v5.7 |
| 262 | `hipMallocFromPoolAsync` | OK | inoltrata alla runtime v5.7 |
| 263 | `hipMallocHost` | OK | inoltrata alla runtime v5.7 |
| 264 | `hipMallocManaged` | OK | inoltrata alla runtime v5.7 |
| 265 | `hipMallocMipmappedArray` | OK | inoltrata alla runtime v5.7 |
| 266 | `hipMallocPitch` | OK | inoltrata alla runtime v5.7 |
| 267 | `hipMemAddressFree` | OK | inoltrata alla runtime v5.7 |
| 268 | `hipMemAddressReserve` | OK | inoltrata alla runtime v5.7 |
| 269 | `hipMemAdvise` | OK | inoltrata alla runtime v5.7 |
| 270 | `hipMemAllocHost` | OK | inoltrata alla runtime v5.7 |
| 271 | `hipMemAllocPitch` | OK | inoltrata alla runtime v5.7 |
| 272 | `hipMemCreate` | OK | inoltrata alla runtime v5.7 |
| 273 | `hipMemExportToShareableHandle` | OK | inoltrata alla runtime v5.7 |
| 274 | `hipMemGetAccess` | OK | inoltrata alla runtime v5.7 |
| 275 | `hipMemGetAddressRange` | OK | inoltrata alla runtime v5.7 |
| 276 | `hipMemGetAllocationGranularity` | OK | inoltrata alla runtime v5.7 |
| 277 | `hipMemGetAllocationPropertiesFromHandle` | OK | inoltrata alla runtime v5.7 |
| 278 | `hipMemGetInfo` | OK | inoltrata alla runtime v5.7 |
| 279 | `hipMemImportFromShareableHandle` | OK | inoltrata alla runtime v5.7 |
| 280 | `hipMemMap` | OK | inoltrata alla runtime v5.7 |
| 281 | `hipMemMapArrayAsync` | OK | inoltrata alla runtime v5.7 |
| 282 | `hipMemPoolCreate` | OK | inoltrata alla runtime v5.7 |
| 283 | `hipMemPoolDestroy` | OK | inoltrata alla runtime v5.7 |
| 284 | `hipMemPoolExportPointer` | OK | inoltrata alla runtime v5.7 |
| 285 | `hipMemPoolExportToShareableHandle` | OK | inoltrata alla runtime v5.7 |
| 286 | `hipMemPoolGetAccess` | OK | inoltrata alla runtime v5.7 |
| 287 | `hipMemPoolGetAttribute` | OK | inoltrata alla runtime v5.7 |
| 288 | `hipMemPoolImportFromShareableHandle` | OK | inoltrata alla runtime v5.7 |
| 289 | `hipMemPoolImportPointer` | OK | inoltrata alla runtime v5.7 |
| 290 | `hipMemPoolSetAccess` | OK | inoltrata alla runtime v5.7 |
| 291 | `hipMemPoolSetAttribute` | OK | inoltrata alla runtime v5.7 |
| 292 | `hipMemPoolTrimTo` | OK | inoltrata alla runtime v5.7 |
| 293 | `hipMemPrefetchAsync` | OK | inoltrata alla runtime v5.7 |
| 294 | `hipMemPtrGetInfo` | OK | inoltrata alla runtime v5.7 |
| 295 | `hipMemRangeGetAttribute` | OK | inoltrata alla runtime v5.7 |
| 296 | `hipMemRangeGetAttributes` | OK | inoltrata alla runtime v5.7 |
| 297 | `hipMemRelease` | OK | inoltrata alla runtime v5.7 |
| 298 | `hipMemRetainAllocationHandle` | OK | inoltrata alla runtime v5.7 |
| 299 | `hipMemSetAccess` | OK | inoltrata alla runtime v5.7 |
| 300 | `hipMemUnmap` | OK | inoltrata alla runtime v5.7 |
| 301 | `hipMemcpy` | OK | inoltrata alla runtime v5.7 |
| 302 | `hipMemcpy2D` | OK | inoltrata alla runtime v5.7 |
| 303 | `hipMemcpy2DArrayToArray` | **NO** | servono i descrittori di array che la 5.7 non espone |
| 304 | `hipMemcpy2DAsync` | OK | inoltrata alla runtime v5.7 |
| 305 | `hipMemcpy2DAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 306 | `hipMemcpy2DFromArray` | OK | inoltrata alla runtime v5.7 |
| 307 | `hipMemcpy2DFromArrayAsync` | OK | inoltrata alla runtime v5.7 |
| 308 | `hipMemcpy2DFromArrayAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 309 | `hipMemcpy2DFromArray_spt` | OK | inoltrata alla runtime v5.7 |
| 310 | `hipMemcpy2DToArray` | OK | inoltrata alla runtime v5.7 |
| 311 | `hipMemcpy2DToArrayAsync` | OK | inoltrata alla runtime v5.7 |
| 312 | `hipMemcpy2DToArrayAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 313 | `hipMemcpy2DToArray_spt` | OK | inoltrata alla runtime v5.7 |
| 314 | `hipMemcpy2D_spt` | OK | inoltrata alla runtime v5.7 |
| 315 | `hipMemcpy3D` | OK | inoltrata alla runtime v5.7 |
| 316 | `hipMemcpy3DAsync` | OK | inoltrata alla runtime v5.7 |
| 317 | `hipMemcpy3DAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 318 | `hipMemcpy3D_spt` | OK | inoltrata alla runtime v5.7 |
| 319 | `hipMemcpyAsync` | OK | inoltrata alla runtime v5.7 |
| 320 | `hipMemcpyAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 321 | `hipMemcpyAtoA` | **NO** | servono i descrittori di array che la 5.7 non espone |
| 322 | `hipMemcpyAtoD` | **NO** | servono i descrittori di array che la 5.7 non espone |
| 323 | `hipMemcpyAtoH` | OK | inoltrata alla runtime v5.7 |
| 324 | `hipMemcpyAtoHAsync` | **OK propria** | implementata: delega alla sincrona hipMemcpyAtoH |
| 325 | `hipMemcpyDtoA` | **NO** | servono i descrittori di array che la 5.7 non espone |
| 326 | `hipMemcpyDtoD` | OK | inoltrata alla runtime v5.7 |
| 327 | `hipMemcpyDtoDAsync` | OK | inoltrata alla runtime v5.7 |
| 328 | `hipMemcpyDtoH` | OK | inoltrata alla runtime v5.7 |
| 329 | `hipMemcpyDtoHAsync` | OK | inoltrata alla runtime v5.7 |
| 330 | `hipMemcpyFromArray` | OK | inoltrata alla runtime v5.7 |
| 331 | `hipMemcpyFromArray_spt` | OK | inoltrata alla runtime v5.7 |
| 332 | `hipMemcpyFromSymbol` | OK | inoltrata alla runtime v5.7 |
| 333 | `hipMemcpyFromSymbolAsync` | OK | inoltrata alla runtime v5.7 |
| 334 | `hipMemcpyFromSymbolAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 335 | `hipMemcpyFromSymbol_spt` | OK | inoltrata alla runtime v5.7 |
| 336 | `hipMemcpyHtoA` | OK | inoltrata alla runtime v5.7 |
| 337 | `hipMemcpyHtoAAsync` | **OK propria** | implementata: delega alla sincrona hipMemcpyHtoA |
| 338 | `hipMemcpyHtoD` | OK | inoltrata alla runtime v5.7 |
| 339 | `hipMemcpyHtoDAsync` | OK | inoltrata alla runtime v5.7 |
| 340 | `hipMemcpyParam2D` | **OK tradotta** | tradotta: hipMemoryType 7.x -> 5.7 in hip_Memcpy2D |
| 341 | `hipMemcpyParam2DAsync` | **OK tradotta** | tradotta: hipMemoryType 7.x -> 5.7 in hip_Memcpy2D |
| 342 | `hipMemcpyPeer` | OK | inoltrata alla runtime v5.7 |
| 343 | `hipMemcpyPeerAsync` | OK | inoltrata alla runtime v5.7 |
| 344 | `hipMemcpyToArray` | OK | inoltrata alla runtime v5.7 |
| 345 | `hipMemcpyToSymbol` | OK | inoltrata alla runtime v5.7 |
| 346 | `hipMemcpyToSymbolAsync` | OK | inoltrata alla runtime v5.7 |
| 347 | `hipMemcpyToSymbolAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 348 | `hipMemcpyToSymbol_spt` | OK | inoltrata alla runtime v5.7 |
| 349 | `hipMemcpyWithStream` | OK | inoltrata alla runtime v5.7 |
| 350 | `hipMemcpy_spt` | OK | inoltrata alla runtime v5.7 |
| 351 | `hipMemset` | OK | inoltrata alla runtime v5.7 |
| 352 | `hipMemset2D` | OK | inoltrata alla runtime v5.7 |
| 353 | `hipMemset2DAsync` | OK | inoltrata alla runtime v5.7 |
| 354 | `hipMemset2DAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 355 | `hipMemset2D_spt` | OK | inoltrata alla runtime v5.7 |
| 356 | `hipMemset3D` | OK | inoltrata alla runtime v5.7 |
| 357 | `hipMemset3DAsync` | OK | inoltrata alla runtime v5.7 |
| 358 | `hipMemset3DAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 359 | `hipMemset3D_spt` | OK | inoltrata alla runtime v5.7 |
| 360 | `hipMemsetAsync` | OK | inoltrata alla runtime v5.7 |
| 361 | `hipMemsetAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 362 | `hipMemsetD16` | OK | inoltrata alla runtime v5.7 |
| 363 | `hipMemsetD16Async` | OK | inoltrata alla runtime v5.7 |
| 364 | `hipMemsetD32` | OK | inoltrata alla runtime v5.7 |
| 365 | `hipMemsetD32Async` | OK | inoltrata alla runtime v5.7 |
| 366 | `hipMemsetD8` | OK | inoltrata alla runtime v5.7 |
| 367 | `hipMemsetD8Async` | OK | inoltrata alla runtime v5.7 |
| 368 | `hipMemset_spt` | OK | inoltrata alla runtime v5.7 |
| 369 | `hipMipmappedArrayCreate` | OK | inoltrata alla runtime v5.7 |
| 370 | `hipMipmappedArrayDestroy` | OK | inoltrata alla runtime v5.7 |
| 371 | `hipMipmappedArrayGetLevel` | OK | inoltrata alla runtime v5.7 |
| 372 | `hipModuleGetFunction` | OK | inoltrata alla runtime v5.7 |
| 373 | `hipModuleGetGlobal` | OK | inoltrata alla runtime v5.7 |
| 374 | `hipModuleGetTexRef` | OK | inoltrata alla runtime v5.7 |
| 375 | `hipModuleLaunchCooperativeKernel` | OK | inoltrata alla runtime v5.7 |
| 376 | `hipModuleLaunchCooperativeKernelMultiDevice` | OK | inoltrata alla runtime v5.7 |
| 377 | `hipModuleLaunchKernel` | OK | inoltrata alla runtime v5.7 |
| 378 | `hipModuleLaunchKernelExt` | OK | inoltrata alla runtime v5.7 |
| 379 | `hipModuleLoad` | OK | inoltrata alla runtime v5.7 |
| 380 | `hipModuleLoadData` | OK | inoltrata alla runtime v5.7 |
| 381 | `hipModuleLoadDataEx` | OK | inoltrata alla runtime v5.7 |
| 382 | `hipModuleOccupancyMaxActiveBlocksPerMultiprocessor` | OK | inoltrata alla runtime v5.7 |
| 383 | `hipModuleOccupancyMaxActiveBlocksPerMultiprocessorWithFlags` | OK | inoltrata alla runtime v5.7 |
| 384 | `hipModuleOccupancyMaxPotentialBlockSize` | OK | inoltrata alla runtime v5.7 |
| 385 | `hipModuleOccupancyMaxPotentialBlockSizeWithFlags` | OK | inoltrata alla runtime v5.7 |
| 386 | `hipModuleUnload` | OK | inoltrata alla runtime v5.7 |
| 387 | `hipOccupancyMaxActiveBlocksPerMultiprocessor` | OK | inoltrata alla runtime v5.7 |
| 388 | `hipOccupancyMaxActiveBlocksPerMultiprocessorWithFlags` | OK | inoltrata alla runtime v5.7 |
| 389 | `hipOccupancyMaxPotentialBlockSize` | OK | inoltrata alla runtime v5.7 |
| 390 | `hipPeekAtLastError` | OK | inoltrata alla runtime v5.7 |
| 391 | `hipPointerGetAttribute` | **OK tradotta** | tradotta: hipMemoryType 5.7 -> 7.x; tipo di un puntatore non HIP: hipErrorInvalidValue (la 5.7 cadeva) |
| 392 | `hipPointerGetAttributes` | **OK tradotta** | tradotta: hipMemoryType 5.7 -> 7.x |
| 393 | `hipProfilerStart` | OK | inoltrata alla runtime v5.7 |
| 394 | `hipProfilerStop` | OK | inoltrata alla runtime v5.7 |
| 395 | `hipRegisterTracerCallback` | OK | inoltrata alla runtime v5.7 |
| 396 | `hipRuntimeGetVersion` | OK | inoltrata alla runtime v5.7 |
| 397 | `hipSetDevice` | OK | inoltrata alla runtime v5.7 |
| 398 | `hipSetDeviceFlags` | OK | inoltrata alla runtime v5.7 |
| 399 | `hipSetValidDevices` | **OK propria** | implementata: valida l'elenco e attiva il primo device buono |
| 400 | `hipSetupArgument` | OK | inoltrata alla runtime v5.7 |
| 401 | `hipSignalExternalSemaphoresAsync` | **OK tradotta** | tradotta: parametri 7.x (144 B) riportati al layout 5.7 (136 B) |
| 402 | `hipStreamAddCallback` | OK | inoltrata alla runtime v5.7 |
| 403 | `hipStreamAddCallback_spt` | OK | inoltrata alla runtime v5.7 |
| 404 | `hipStreamAttachMemAsync` | OK | inoltrata alla runtime v5.7 |
| 405 | `hipStreamBatchMemOp` | **NO** | la 5.7 non ha la primitiva batch mem-op |
| 406 | `hipStreamBeginCapture` | OK | inoltrata alla runtime v5.7 |
| 407 | `hipStreamBeginCaptureToGraph` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 408 | `hipStreamBeginCapture_spt` | OK | inoltrata alla runtime v5.7 |
| 409 | `hipStreamCreate` | OK | inoltrata alla runtime v5.7 |
| 410 | `hipStreamCreateWithFlags` | OK | inoltrata alla runtime v5.7 |
| 411 | `hipStreamCreateWithPriority` | OK | inoltrata alla runtime v5.7 |
| 412 | `hipStreamDestroy` | OK | inoltrata alla runtime v5.7 |
| 413 | `hipStreamEndCapture` | OK | inoltrata alla runtime v5.7 |
| 414 | `hipStreamEndCapture_spt` | OK | inoltrata alla runtime v5.7 |
| 415 | `hipStreamGetCaptureInfo` | OK | inoltrata alla runtime v5.7 |
| 416 | `hipStreamGetCaptureInfo_spt` | OK | inoltrata alla runtime v5.7 |
| 417 | `hipStreamGetCaptureInfo_v2` | OK | inoltrata alla runtime v5.7 |
| 418 | `hipStreamGetCaptureInfo_v2_spt` | OK | inoltrata alla runtime v5.7 |
| 419 | `hipStreamGetDevice` | OK | inoltrata alla runtime v5.7 |
| 420 | `hipStreamGetFlags` | OK | inoltrata alla runtime v5.7 |
| 421 | `hipStreamGetFlags_spt` | OK | inoltrata alla runtime v5.7 |
| 422 | `hipStreamGetPriority` | OK | inoltrata alla runtime v5.7 |
| 423 | `hipStreamGetPriority_spt` | OK | inoltrata alla runtime v5.7 |
| 424 | `hipStreamIsCapturing` | OK | inoltrata alla runtime v5.7 |
| 425 | `hipStreamIsCapturing_spt` | OK | inoltrata alla runtime v5.7 |
| 426 | `hipStreamQuery` | OK | inoltrata alla runtime v5.7 |
| 427 | `hipStreamQuery_spt` | OK | inoltrata alla runtime v5.7 |
| 428 | `hipStreamSynchronize` | OK | inoltrata alla runtime v5.7 |
| 429 | `hipStreamSynchronize_spt` | OK | inoltrata alla runtime v5.7 |
| 430 | `hipStreamUpdateCaptureDependencies` | OK | inoltrata alla runtime v5.7 |
| 431 | `hipStreamWaitEvent` | OK | inoltrata alla runtime v5.7 |
| 432 | `hipStreamWaitEvent_spt` | OK | inoltrata alla runtime v5.7 |
| 433 | `hipStreamWaitValue32` | OK | inoltrata alla runtime v5.7 |
| 434 | `hipStreamWaitValue64` | OK | inoltrata alla runtime v5.7 |
| 435 | `hipStreamWriteValue32` | OK | inoltrata alla runtime v5.7 |
| 436 | `hipStreamWriteValue64` | OK | inoltrata alla runtime v5.7 |
| 437 | `hipTexObjectCreate` | OK | inoltrata alla runtime v5.7 |
| 438 | `hipTexObjectDestroy` | OK | inoltrata alla runtime v5.7 |
| 439 | `hipTexObjectGetResourceDesc` | OK | inoltrata alla runtime v5.7 |
| 440 | `hipTexObjectGetResourceViewDesc` | OK | inoltrata alla runtime v5.7 |
| 441 | `hipTexObjectGetTextureDesc` | OK | inoltrata alla runtime v5.7 |
| 442 | `hipTexRefGetAddress` | OK | inoltrata alla runtime v5.7 |
| 443 | `hipTexRefGetAddressMode` | OK | inoltrata alla runtime v5.7 |
| 444 | `hipTexRefGetArray` | OK | inoltrata alla runtime v5.7 |
| 445 | `hipTexRefGetBorderColor` | OK | inoltrata alla runtime v5.7 |
| 446 | `hipTexRefGetFilterMode` | OK | inoltrata alla runtime v5.7 |
| 447 | `hipTexRefGetFlags` | OK | inoltrata alla runtime v5.7 |
| 448 | `hipTexRefGetFormat` | OK | inoltrata alla runtime v5.7 |
| 449 | `hipTexRefGetMaxAnisotropy` | OK | inoltrata alla runtime v5.7 |
| 450 | `hipTexRefGetMipMappedArray` | **OK propria** | alias di hipTexRefGetMipmappedArray della 5.7 (sola differenza di maiuscola) |
| 451 | `hipTexRefGetMipmapFilterMode` | OK | inoltrata alla runtime v5.7 |
| 452 | `hipTexRefGetMipmapLevelBias` | OK | inoltrata alla runtime v5.7 |
| 453 | `hipTexRefGetMipmapLevelClamp` | OK | inoltrata alla runtime v5.7 |
| 454 | `hipTexRefGetMipmappedArray` | OK | inoltrata alla runtime v5.7 |
| 455 | `hipTexRefSetAddress` | OK | inoltrata alla runtime v5.7 |
| 456 | `hipTexRefSetAddress2D` | OK | inoltrata alla runtime v5.7 |
| 457 | `hipTexRefSetAddressMode` | OK | inoltrata alla runtime v5.7 |
| 458 | `hipTexRefSetArray` | OK | inoltrata alla runtime v5.7 |
| 459 | `hipTexRefSetBorderColor` | OK | inoltrata alla runtime v5.7 |
| 460 | `hipTexRefSetFilterMode` | OK | inoltrata alla runtime v5.7 |
| 461 | `hipTexRefSetFlags` | OK | inoltrata alla runtime v5.7 |
| 462 | `hipTexRefSetFormat` | OK | inoltrata alla runtime v5.7 |
| 463 | `hipTexRefSetMaxAnisotropy` | OK | inoltrata alla runtime v5.7 |
| 464 | `hipTexRefSetMipmapFilterMode` | OK | inoltrata alla runtime v5.7 |
| 465 | `hipTexRefSetMipmapLevelBias` | OK | inoltrata alla runtime v5.7 |
| 466 | `hipTexRefSetMipmapLevelClamp` | OK | inoltrata alla runtime v5.7 |
| 467 | `hipTexRefSetMipmappedArray` | OK | inoltrata alla runtime v5.7 |
| 468 | `hipThreadExchangeStreamCaptureMode` | OK | inoltrata alla runtime v5.7 |
| 469 | `hipUnbindTexture` | OK | inoltrata alla runtime v5.7 |
| 470 | `hipUserObjectCreate` | OK | inoltrata alla runtime v5.7 |
| 471 | `hipUserObjectRelease` | OK | inoltrata alla runtime v5.7 |
| 472 | `hipUserObjectRetain` | OK | inoltrata alla runtime v5.7 |
| 473 | `hipWaitExternalSemaphoresAsync` | **OK tradotta** | tradotta: parametri 7.x (144 B) riportati al layout 5.7 (136 B) |

## Riepilogo

- **OK (inoltrate)**: 418
- **OK tradotte**: 10 (inoltrate traducendo l'ABI, vedi sotto)
- **OK propria**: 10
- **NO (esportate, non supportate)**: 35

## Traduzione ABI 5.7 ↔ 7.x (12 nov 2026)

Confrontando con clang gli header di ROCm 5.7 e 7.1 (layout di tutte le 50 strutture comuni e valori
di tutti i 470 enumeratori comuni), fra le due versioni sono cambiati solo:
- `hipMemoryType`: rinumerato. 5.7: Host 0, Device 1, Array 2, Unified 3, Managed 4.
  7.x: Unregistered 0, Host 1, Device 2, Managed 3, Array 10, Unified 11;
- `HIP_MEMCPY3D`: campi di dimensione da `unsigned int` (128 byte) a `size_t` (184 byte);
- `hipExternalSemaphoreSignalParams` / `WaitParams`: l'unione `nvSciSync` inserita all'offset 8
  sposta `keyedMutex` e `flags` di 8 byte.

Gli altri cambiamenti (descrittori di memoria e semafori esterni, `hipMemPoolProps`,
`hipMemAllocNodeParams`, `hipMemAllocationProp`) aggiungono campi solo dove la 5.7 ha spazio riservato.
Le 10 funzioni che passano quei tipi ora traducono. Prima un programma compilato con gli header
6.x/7.x leggeva la memoria GPU come «host», e le copie 3D restituivano successo con i dati sbagliati
(55 278 byte diversi su 55 500 nella prova). Lo shim aggira anche un difetto della 5.7:
`hipDrvPointerGetAttributes(MEMORY_TYPE)` cade su alcuni puntatori host non registrati, per esempio
la memoria di un tensore CPU di PyTorch.

Oltre alle 473 API, lo shim esporta 9 simboli: le 5 funzioni interne del compilatore
`__hipRegisterFatBinary`, `__hipRegisterFunction`, `__hipUnregisterFatBinary`,
`__hipPushCallConfiguration`, `__hipPopCallConfiguration` (le ultime due inoltrate alla runtime
v5.7; le prime tre sono **vuote**: la registrazione dei fat binary non avviene, vedi README) e
4 funzioni diagnostiche `marconi_57exp_get_version`, `marconi_57exp_get_device_count`,
`marconi_57exp_count_exported_symbols`, `marconi_57exp_count_resolved_pfns`.
Totale export della DLL: **482**.

## Cosa servirebbe per attivare le NO

Tutte e 35 richiedono **primitive che la runtime v5.7 non espone**, non lavoro nostro.
Le vie possibili, in ordine di costo:

1. **Sorgenti ROCm 6.4 + suo interop PAL** (gfx906 e' ancora nella tabella PAL fino alla 6.4.4,
   sparisce nella 7.0.0): darebbe la runtime nativa con tutte le funzioni. Richiede un albero
   sorgenti che qui non c'e'. **E' l'unica via che le attiva davvero tutte.**
2. **Reverse-engineering dei layout di struct** (`hipGraphNodeParams`, `HIP_MEMCPY3D`,
   `hipDeviceProp` R0000): permetterebbe di implementare i dispatcher grafo e le copie array
   sopra le primitive esistenti. Fattibile per ~10 delle 35, ma ogni struct sbagliata
   corrompe memoria: serve validazione contro gli header ufficiali 6.4.
3. **Le restanti ~25** (semafori esterni nei grafi, batch mem-op, API driver dei grafi)
   richiedono primitive kernel-mode che il driver Adrenalin non espone: fuori portata
   senza la via 1.