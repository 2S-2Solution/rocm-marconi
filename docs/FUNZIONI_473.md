# Marconi shim v5.2 - stato delle 474 funzioni esportate

(Il nome del file resta FUNZIONI_473.md per non rompere i collegamenti.)

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
| 93 | `hipEventRecordWithFlags` | **OK propria** | HIP 7: flag 0/1 (contano solo nei grafi) -> hipEventRecord della 5.7 |
| 94 | `hipEventRecord_spt` | OK | inoltrata alla runtime v5.7 |
| 95 | `hipEventSynchronize` | OK | inoltrata alla runtime v5.7 |
| 96 | `hipExtGetLastError` | **OK propria** | come hipGetLastError (la 5.7 non la ha) |
| 97 | `hipExtGetLinkTypeAndHopCount` | OK | inoltrata alla runtime v5.7 |
| 98 | `hipExtLaunchKernel` | **OK propria** | kernel registrati dallo shim: eventi registrati prima e dopo sullo stesso stream, lancio con hipModuleLaunchKernel; gli altri inoltrati |
| 99 | `hipExtLaunchMultiKernelMultiDevice` | OK | inoltrata alla runtime v5.7 |
| 100 | `hipExtMallocWithFlags` | OK | inoltrata alla runtime v5.7 |
| 101 | `hipExtModuleLaunchKernel` | OK | inoltrata alla runtime v5.7 |
| 102 | `hipExtStreamCreateWithCUMask` | OK | inoltrata alla runtime v5.7 |
| 103 | `hipExtStreamGetCUMask` | OK | inoltrata alla runtime v5.7 |
| 104 | `hipExternalMemoryGetMappedBuffer` | OK | inoltrata alla runtime v5.7 |
| 105 | `hipExternalMemoryGetMappedMipmappedArray` | **NO** | primitiva assente nella runtime 5.7 |
| 106 | `hipFree` | OK | inoltrata alla runtime v5.7 |
| 107 | `hipFreeArray` | OK | inoltrata alla runtime v5.7 |
| 108 | `hipFreeAsync` | OK | inoltrata alla runtime v5.7 |
| 109 | `hipFreeHost` | OK | inoltrata alla runtime v5.7 |
| 110 | `hipFreeMipmappedArray` | OK | inoltrata alla runtime v5.7 |
| 111 | `hipFuncGetAttribute` | OK | inoltrata alla runtime v5.7 |
| 112 | `hipFuncGetAttributes` | **OK propria** | per i kernel registrati dallo shim: attributi letti con hipFuncGetAttribute della 5.7 |
| 113 | `hipFuncSetAttribute` | **OK propria** | kernel registrati dallo shim: accettata senza effetto (gfx906 non ha ripartizione cache/LDS) |
| 114 | `hipFuncSetCacheConfig` | **OK propria** | kernel registrati dallo shim: accettata senza effetto |
| 115 | `hipFuncSetSharedMemConfig` | OK | inoltrata alla runtime v5.7 |
| 116 | `hipGLGetDevices` | OK | inoltrata alla runtime v5.7 |
| 117 | `hipGetChannelDesc` | OK | inoltrata alla runtime v5.7 |
| 118 | `hipGetCmdName` | OK | inoltrata alla runtime v5.7 |
| 119 | `hipGetDevice` | **OK propria** | inoltrata; alla chiusura del processo risponde lo shim |
| 120 | `hipGetDeviceCount` | OK | inoltrata alla runtime v5.7 |
| 121 | `hipGetDeviceFlags` | OK | inoltrata alla runtime v5.7 |
| 122 | `hipGetDeviceProperties` | OK | inoltrata alla runtime v5.7 |
| 123 | `hipGetDevicePropertiesR0000` | **NO** | servirebbe il layout esatto della struct della versione chiamante: inventarlo corromperebbe la memoria |
| 124 | `hipGetDevicePropertiesR0600` | **OK propria** | struttura R0600 (1640 B) riempita con valori FISSI della Radeon VII (nome, gfx906:sramecc+:xnack-, 16 GB, 60 CU, compute capability 7.0), senza chiamare il driver e per qualunque indice di dispositivo |
| 125 | `hipGetErrorName` | OK | inoltrata alla runtime v5.7 |
| 126 | `hipGetErrorString` | OK | inoltrata alla runtime v5.7 |
| 127 | `hipGetFuncBySymbol` | **NO** | primitiva assente nella runtime 5.7 |
| 128 | `hipGetLastError` | **OK propria** | errore della 5.7 piu' quello dei lanci falliti dentro lo shim |
| 129 | `hipGetMipmappedArrayLevel` | OK | inoltrata alla runtime v5.7 |
| 130 | `hipGetProcAddress` | **OK propria** | implementata: cerca nello shim, poi nella runtime |
| 131 | `hipGetStreamDeviceId` | OK | inoltrata alla runtime v5.7 |
| 132 | `hipGetSymbolAddress` | **OK propria** | variabili __device__/__constant__ registrate dallo shim: hipModuleGetGlobal |
| 133 | `hipGetSymbolSize` | **OK propria** | variabili registrate dallo shim: hipModuleGetGlobal |
| 134 | `hipGetTextureAlignmentOffset` | OK | inoltrata alla runtime v5.7 |
| 135 | `hipGetTextureObjectResourceDesc` | OK | inoltrata alla runtime v5.7 |
| 136 | `hipGetTextureObjectResourceViewDesc` | OK | inoltrata alla runtime v5.7 |
| 137 | `hipGetTextureObjectTextureDesc` | OK | inoltrata alla runtime v5.7 |
| 138 | `hipGetTextureReference` | OK | inoltrata alla runtime v5.7 |
| 139 | `hipGraphAddBatchMemOpNode` | **NO** | la 5.7 non ha la primitiva batch mem-op |
| 140 | `hipGraphAddChildGraphNode` | OK | inoltrata alla runtime v5.7 |
| 141 | `hipGraphAddDependencies` | OK | inoltrata alla runtime v5.7 |
| 142 | `hipGraphAddEmptyNode` | OK | inoltrata alla runtime v5.7 |
| 143 | `hipGraphAddEventRecordNode` | OK | inoltrata alla runtime v5.7 |
| 144 | `hipGraphAddEventWaitNode` | OK | inoltrata alla runtime v5.7 |
| 145 | `hipGraphAddExternalSemaphoresSignalNode` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 146 | `hipGraphAddExternalSemaphoresWaitNode` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 147 | `hipGraphAddHostNode` | OK | inoltrata alla runtime v5.7 |
| 148 | `hipGraphAddKernelNode` | OK | inoltrata alla runtime v5.7 |
| 149 | `hipGraphAddMemAllocNode` | OK | inoltrata alla runtime v5.7 |
| 150 | `hipGraphAddMemFreeNode` | OK | inoltrata alla runtime v5.7 |
| 151 | `hipGraphAddMemcpyNode` | OK | inoltrata alla runtime v5.7 |
| 152 | `hipGraphAddMemcpyNode1D` | OK | inoltrata alla runtime v5.7 |
| 153 | `hipGraphAddMemcpyNodeFromSymbol` | OK | inoltrata alla runtime v5.7 |
| 154 | `hipGraphAddMemcpyNodeToSymbol` | OK | inoltrata alla runtime v5.7 |
| 155 | `hipGraphAddMemsetNode` | OK | inoltrata alla runtime v5.7 |
| 156 | `hipGraphAddNode` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 157 | `hipGraphAddNode_v2` | **OK propria** | implementazione custom nello shim |
| 158 | `hipGraphBatchMemOpNodeGetParams` | **NO** | la 5.7 non ha la primitiva batch mem-op |
| 159 | `hipGraphBatchMemOpNodeSetParams` | **NO** | la 5.7 non ha la primitiva batch mem-op |
| 160 | `hipGraphChildGraphNodeGetGraph` | OK | inoltrata alla runtime v5.7 |
| 161 | `hipGraphClone` | OK | inoltrata alla runtime v5.7 |
| 162 | `hipGraphCreate` | OK | inoltrata alla runtime v5.7 |
| 163 | `hipGraphDebugDotPrint` | OK | inoltrata alla runtime v5.7 |
| 164 | `hipGraphDestroy` | OK | inoltrata alla runtime v5.7 |
| 165 | `hipGraphDestroyNode` | OK | inoltrata alla runtime v5.7 |
| 166 | `hipGraphEventRecordNodeGetEvent` | OK | inoltrata alla runtime v5.7 |
| 167 | `hipGraphEventRecordNodeSetEvent` | OK | inoltrata alla runtime v5.7 |
| 168 | `hipGraphEventWaitNodeGetEvent` | OK | inoltrata alla runtime v5.7 |
| 169 | `hipGraphEventWaitNodeSetEvent` | OK | inoltrata alla runtime v5.7 |
| 170 | `hipGraphExecBatchMemOpNodeSetParams` | **NO** | la 5.7 non ha la primitiva batch mem-op |
| 171 | `hipGraphExecChildGraphNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 172 | `hipGraphExecDestroy` | OK | inoltrata alla runtime v5.7 |
| 173 | `hipGraphExecEventRecordNodeSetEvent` | OK | inoltrata alla runtime v5.7 |
| 174 | `hipGraphExecEventWaitNodeSetEvent` | OK | inoltrata alla runtime v5.7 |
| 175 | `hipGraphExecExternalSemaphoresSignalNodeSetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 176 | `hipGraphExecExternalSemaphoresWaitNodeSetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 177 | `hipGraphExecGetFlags` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 178 | `hipGraphExecHostNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 179 | `hipGraphExecKernelNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 180 | `hipGraphExecMemcpyNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 181 | `hipGraphExecMemcpyNodeSetParams1D` | OK | inoltrata alla runtime v5.7 |
| 182 | `hipGraphExecMemcpyNodeSetParamsFromSymbol` | OK | inoltrata alla runtime v5.7 |
| 183 | `hipGraphExecMemcpyNodeSetParamsToSymbol` | OK | inoltrata alla runtime v5.7 |
| 184 | `hipGraphExecMemsetNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 185 | `hipGraphExecNodeSetParams` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 186 | `hipGraphExecUpdate` | OK | inoltrata alla runtime v5.7 |
| 187 | `hipGraphExternalSemaphoresSignalNodeGetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 188 | `hipGraphExternalSemaphoresSignalNodeSetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 189 | `hipGraphExternalSemaphoresWaitNodeGetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 190 | `hipGraphExternalSemaphoresWaitNodeSetParams` | **NO** | la 5.7 non espone i semafori esterni COME NODI di grafo |
| 191 | `hipGraphGetEdges` | OK | inoltrata alla runtime v5.7 |
| 192 | `hipGraphGetNodes` | OK | inoltrata alla runtime v5.7 |
| 193 | `hipGraphGetRootNodes` | OK | inoltrata alla runtime v5.7 |
| 194 | `hipGraphHostNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 195 | `hipGraphHostNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 196 | `hipGraphInstantiate` | OK | inoltrata alla runtime v5.7 |
| 197 | `hipGraphInstantiateWithFlags` | OK | inoltrata alla runtime v5.7 |
| 198 | `hipGraphInstantiateWithParams` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 199 | `hipGraphKernelNodeCopyAttributes` | OK | inoltrata alla runtime v5.7 |
| 200 | `hipGraphKernelNodeGetAttribute` | OK | inoltrata alla runtime v5.7 |
| 201 | `hipGraphKernelNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 202 | `hipGraphKernelNodeSetAttribute` | OK | inoltrata alla runtime v5.7 |
| 203 | `hipGraphKernelNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 204 | `hipGraphLaunch` | OK | inoltrata alla runtime v5.7 |
| 205 | `hipGraphLaunch_spt` | OK | inoltrata alla runtime v5.7 |
| 206 | `hipGraphMemAllocNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 207 | `hipGraphMemFreeNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 208 | `hipGraphMemcpyNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 209 | `hipGraphMemcpyNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 210 | `hipGraphMemcpyNodeSetParams1D` | OK | inoltrata alla runtime v5.7 |
| 211 | `hipGraphMemcpyNodeSetParamsFromSymbol` | OK | inoltrata alla runtime v5.7 |
| 212 | `hipGraphMemcpyNodeSetParamsToSymbol` | OK | inoltrata alla runtime v5.7 |
| 213 | `hipGraphMemsetNodeGetParams` | OK | inoltrata alla runtime v5.7 |
| 214 | `hipGraphMemsetNodeSetParams` | OK | inoltrata alla runtime v5.7 |
| 215 | `hipGraphNodeFindInClone` | OK | inoltrata alla runtime v5.7 |
| 216 | `hipGraphNodeGetDependencies` | OK | inoltrata alla runtime v5.7 |
| 217 | `hipGraphNodeGetDependentNodes` | OK | inoltrata alla runtime v5.7 |
| 218 | `hipGraphNodeGetEnabled` | OK | inoltrata alla runtime v5.7 |
| 219 | `hipGraphNodeGetType` | OK | inoltrata alla runtime v5.7 |
| 220 | `hipGraphNodeSetEnabled` | OK | inoltrata alla runtime v5.7 |
| 221 | `hipGraphNodeSetParams` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 222 | `hipGraphReleaseUserObject` | OK | inoltrata alla runtime v5.7 |
| 223 | `hipGraphRemoveDependencies` | OK | inoltrata alla runtime v5.7 |
| 224 | `hipGraphRetainUserObject` | OK | inoltrata alla runtime v5.7 |
| 225 | `hipGraphUpload` | OK | inoltrata alla runtime v5.7 |
| 226 | `hipGraphicsGLRegisterBuffer` | OK | inoltrata alla runtime v5.7 |
| 227 | `hipGraphicsGLRegisterImage` | OK | inoltrata alla runtime v5.7 |
| 228 | `hipGraphicsMapResources` | OK | inoltrata alla runtime v5.7 |
| 229 | `hipGraphicsResourceGetMappedPointer` | OK | inoltrata alla runtime v5.7 |
| 230 | `hipGraphicsSubResourceGetMappedArray` | OK | inoltrata alla runtime v5.7 |
| 231 | `hipGraphicsUnmapResources` | OK | inoltrata alla runtime v5.7 |
| 232 | `hipGraphicsUnregisterResource` | OK | inoltrata alla runtime v5.7 |
| 233 | `hipHccModuleLaunchKernel` | OK | inoltrata alla runtime v5.7 |
| 234 | `hipHostAlloc` | OK | inoltrata alla runtime v5.7 |
| 235 | `hipHostFree` | OK | inoltrata alla runtime v5.7 |
| 236 | `hipHostGetDevicePointer` | OK | inoltrata alla runtime v5.7 |
| 237 | `hipHostGetFlags` | OK | inoltrata alla runtime v5.7 |
| 238 | `hipHostMalloc` | OK | inoltrata alla runtime v5.7 |
| 239 | `hipHostRegister` | OK | inoltrata alla runtime v5.7 |
| 240 | `hipHostUnregister` | OK | inoltrata alla runtime v5.7 |
| 241 | `hipImportExternalMemory` | OK | inoltrata alla runtime v5.7 |
| 242 | `hipImportExternalSemaphore` | OK | inoltrata alla runtime v5.7 |
| 243 | `hipInit` | OK | inoltrata alla runtime v5.7 |
| 244 | `hipIpcCloseMemHandle` | OK | inoltrata alla runtime v5.7 |
| 245 | `hipIpcGetEventHandle` | OK | inoltrata alla runtime v5.7 |
| 246 | `hipIpcGetMemHandle` | OK | inoltrata alla runtime v5.7 |
| 247 | `hipIpcOpenEventHandle` | OK | inoltrata alla runtime v5.7 |
| 248 | `hipIpcOpenMemHandle` | OK | inoltrata alla runtime v5.7 |
| 249 | `hipKernelNameRef` | OK | inoltrata alla runtime v5.7 |
| 250 | `hipLaunchByPtr` | OK | inoltrata alla runtime v5.7 |
| 251 | `hipLaunchCooperativeKernel` | **OK propria** | kernel registrati dallo shim: hipModuleLaunchCooperativeKernel; il driver rifiuta comunque i lanci cooperativi (719, anche con codice 5.7) |
| 252 | `hipLaunchCooperativeKernelMultiDevice` | OK | inoltrata alla runtime v5.7 |
| 253 | `hipLaunchCooperativeKernel_spt` | OK | inoltrata alla runtime v5.7 |
| 254 | `hipLaunchHostFunc` | OK | inoltrata alla runtime v5.7 |
| 255 | `hipLaunchHostFunc_spt` | OK | inoltrata alla runtime v5.7 |
| 256 | `hipLaunchKernel` | **OK propria** | kernel registrati dallo shim: codice caricato al primo lancio (bundle, CCOB, kpack), lancio con hipModuleLaunchKernel; gli altri inoltrati |
| 257 | `hipLaunchKernel_spt` | OK | inoltrata alla runtime v5.7 |
| 258 | `hipMalloc` | OK | inoltrata alla runtime v5.7 |
| 259 | `hipMalloc3D` | OK | inoltrata alla runtime v5.7 |
| 260 | `hipMalloc3DArray` | OK | inoltrata alla runtime v5.7 |
| 261 | `hipMallocArray` | OK | inoltrata alla runtime v5.7 |
| 262 | `hipMallocAsync` | OK | inoltrata alla runtime v5.7 |
| 263 | `hipMallocFromPoolAsync` | OK | inoltrata alla runtime v5.7 |
| 264 | `hipMallocHost` | OK | inoltrata alla runtime v5.7 |
| 265 | `hipMallocManaged` | OK | inoltrata alla runtime v5.7 |
| 266 | `hipMallocMipmappedArray` | OK | inoltrata alla runtime v5.7 |
| 267 | `hipMallocPitch` | OK | inoltrata alla runtime v5.7 |
| 268 | `hipMemAddressFree` | OK | inoltrata alla runtime v5.7 |
| 269 | `hipMemAddressReserve` | OK | inoltrata alla runtime v5.7 |
| 270 | `hipMemAdvise` | OK | inoltrata alla runtime v5.7 |
| 271 | `hipMemAllocHost` | OK | inoltrata alla runtime v5.7 |
| 272 | `hipMemAllocPitch` | OK | inoltrata alla runtime v5.7 |
| 273 | `hipMemCreate` | OK | inoltrata alla runtime v5.7 |
| 274 | `hipMemExportToShareableHandle` | OK | inoltrata alla runtime v5.7 |
| 275 | `hipMemGetAccess` | OK | inoltrata alla runtime v5.7 |
| 276 | `hipMemGetAddressRange` | OK | inoltrata alla runtime v5.7 |
| 277 | `hipMemGetAllocationGranularity` | OK | inoltrata alla runtime v5.7 |
| 278 | `hipMemGetAllocationPropertiesFromHandle` | OK | inoltrata alla runtime v5.7 |
| 279 | `hipMemGetInfo` | OK | inoltrata alla runtime v5.7 |
| 280 | `hipMemImportFromShareableHandle` | OK | inoltrata alla runtime v5.7 |
| 281 | `hipMemMap` | OK | inoltrata alla runtime v5.7 |
| 282 | `hipMemMapArrayAsync` | OK | inoltrata alla runtime v5.7 |
| 283 | `hipMemPoolCreate` | OK | inoltrata alla runtime v5.7 |
| 284 | `hipMemPoolDestroy` | OK | inoltrata alla runtime v5.7 |
| 285 | `hipMemPoolExportPointer` | OK | inoltrata alla runtime v5.7 |
| 286 | `hipMemPoolExportToShareableHandle` | OK | inoltrata alla runtime v5.7 |
| 287 | `hipMemPoolGetAccess` | OK | inoltrata alla runtime v5.7 |
| 288 | `hipMemPoolGetAttribute` | OK | inoltrata alla runtime v5.7 |
| 289 | `hipMemPoolImportFromShareableHandle` | OK | inoltrata alla runtime v5.7 |
| 290 | `hipMemPoolImportPointer` | OK | inoltrata alla runtime v5.7 |
| 291 | `hipMemPoolSetAccess` | OK | inoltrata alla runtime v5.7 |
| 292 | `hipMemPoolSetAttribute` | OK | inoltrata alla runtime v5.7 |
| 293 | `hipMemPoolTrimTo` | OK | inoltrata alla runtime v5.7 |
| 294 | `hipMemPrefetchAsync` | OK | inoltrata alla runtime v5.7 |
| 295 | `hipMemPtrGetInfo` | OK | inoltrata alla runtime v5.7 |
| 296 | `hipMemRangeGetAttribute` | OK | inoltrata alla runtime v5.7 |
| 297 | `hipMemRangeGetAttributes` | OK | inoltrata alla runtime v5.7 |
| 298 | `hipMemRelease` | OK | inoltrata alla runtime v5.7 |
| 299 | `hipMemRetainAllocationHandle` | OK | inoltrata alla runtime v5.7 |
| 300 | `hipMemSetAccess` | OK | inoltrata alla runtime v5.7 |
| 301 | `hipMemUnmap` | OK | inoltrata alla runtime v5.7 |
| 302 | `hipMemcpy` | OK | inoltrata alla runtime v5.7 |
| 303 | `hipMemcpy2D` | OK | inoltrata alla runtime v5.7 |
| 304 | `hipMemcpy2DArrayToArray` | **NO** | servono i descrittori di array che la 5.7 non espone |
| 305 | `hipMemcpy2DAsync` | OK | inoltrata alla runtime v5.7 |
| 306 | `hipMemcpy2DAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 307 | `hipMemcpy2DFromArray` | OK | inoltrata alla runtime v5.7 |
| 308 | `hipMemcpy2DFromArrayAsync` | OK | inoltrata alla runtime v5.7 |
| 309 | `hipMemcpy2DFromArrayAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 310 | `hipMemcpy2DFromArray_spt` | OK | inoltrata alla runtime v5.7 |
| 311 | `hipMemcpy2DToArray` | OK | inoltrata alla runtime v5.7 |
| 312 | `hipMemcpy2DToArrayAsync` | OK | inoltrata alla runtime v5.7 |
| 313 | `hipMemcpy2DToArrayAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 314 | `hipMemcpy2DToArray_spt` | OK | inoltrata alla runtime v5.7 |
| 315 | `hipMemcpy2D_spt` | OK | inoltrata alla runtime v5.7 |
| 316 | `hipMemcpy3D` | OK | inoltrata alla runtime v5.7 |
| 317 | `hipMemcpy3DAsync` | OK | inoltrata alla runtime v5.7 |
| 318 | `hipMemcpy3DAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 319 | `hipMemcpy3D_spt` | OK | inoltrata alla runtime v5.7 |
| 320 | `hipMemcpyAsync` | OK | inoltrata alla runtime v5.7 |
| 321 | `hipMemcpyAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 322 | `hipMemcpyAtoA` | **NO** | servono i descrittori di array che la 5.7 non espone |
| 323 | `hipMemcpyAtoD` | **NO** | servono i descrittori di array che la 5.7 non espone |
| 324 | `hipMemcpyAtoH` | OK | inoltrata alla runtime v5.7 |
| 325 | `hipMemcpyAtoHAsync` | **OK propria** | implementata: delega alla sincrona hipMemcpyAtoH |
| 326 | `hipMemcpyDtoA` | **NO** | servono i descrittori di array che la 5.7 non espone |
| 327 | `hipMemcpyDtoD` | OK | inoltrata alla runtime v5.7 |
| 328 | `hipMemcpyDtoDAsync` | OK | inoltrata alla runtime v5.7 |
| 329 | `hipMemcpyDtoH` | OK | inoltrata alla runtime v5.7 |
| 330 | `hipMemcpyDtoHAsync` | OK | inoltrata alla runtime v5.7 |
| 331 | `hipMemcpyFromArray` | OK | inoltrata alla runtime v5.7 |
| 332 | `hipMemcpyFromArray_spt` | OK | inoltrata alla runtime v5.7 |
| 333 | `hipMemcpyFromSymbol` | **OK propria** | variabili registrate dallo shim: hipMemcpy sull'indirizzo del simbolo |
| 334 | `hipMemcpyFromSymbolAsync` | **OK propria** | variabili registrate dallo shim: hipMemcpyAsync sull'indirizzo del simbolo |
| 335 | `hipMemcpyFromSymbolAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 336 | `hipMemcpyFromSymbol_spt` | OK | inoltrata alla runtime v5.7 |
| 337 | `hipMemcpyHtoA` | OK | inoltrata alla runtime v5.7 |
| 338 | `hipMemcpyHtoAAsync` | **OK propria** | implementata: delega alla sincrona hipMemcpyHtoA |
| 339 | `hipMemcpyHtoD` | OK | inoltrata alla runtime v5.7 |
| 340 | `hipMemcpyHtoDAsync` | OK | inoltrata alla runtime v5.7 |
| 341 | `hipMemcpyParam2D` | **OK tradotta** | tradotta: hipMemoryType 7.x -> 5.7 in hip_Memcpy2D |
| 342 | `hipMemcpyParam2DAsync` | **OK tradotta** | tradotta: hipMemoryType 7.x -> 5.7 in hip_Memcpy2D |
| 343 | `hipMemcpyPeer` | OK | inoltrata alla runtime v5.7 |
| 344 | `hipMemcpyPeerAsync` | OK | inoltrata alla runtime v5.7 |
| 345 | `hipMemcpyToArray` | OK | inoltrata alla runtime v5.7 |
| 346 | `hipMemcpyToSymbol` | **OK propria** | variabili registrate dallo shim: hipMemcpy sull'indirizzo del simbolo |
| 347 | `hipMemcpyToSymbolAsync` | **OK propria** | variabili registrate dallo shim: hipMemcpyAsync sull'indirizzo del simbolo |
| 348 | `hipMemcpyToSymbolAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 349 | `hipMemcpyToSymbol_spt` | OK | inoltrata alla runtime v5.7 |
| 350 | `hipMemcpyWithStream` | OK | inoltrata alla runtime v5.7 |
| 351 | `hipMemcpy_spt` | OK | inoltrata alla runtime v5.7 |
| 352 | `hipMemset` | OK | inoltrata alla runtime v5.7 |
| 353 | `hipMemset2D` | OK | inoltrata alla runtime v5.7 |
| 354 | `hipMemset2DAsync` | OK | inoltrata alla runtime v5.7 |
| 355 | `hipMemset2DAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 356 | `hipMemset2D_spt` | OK | inoltrata alla runtime v5.7 |
| 357 | `hipMemset3D` | OK | inoltrata alla runtime v5.7 |
| 358 | `hipMemset3DAsync` | OK | inoltrata alla runtime v5.7 |
| 359 | `hipMemset3DAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 360 | `hipMemset3D_spt` | OK | inoltrata alla runtime v5.7 |
| 361 | `hipMemsetAsync` | OK | inoltrata alla runtime v5.7 |
| 362 | `hipMemsetAsync_spt` | OK | inoltrata alla runtime v5.7 |
| 363 | `hipMemsetD16` | OK | inoltrata alla runtime v5.7 |
| 364 | `hipMemsetD16Async` | OK | inoltrata alla runtime v5.7 |
| 365 | `hipMemsetD32` | OK | inoltrata alla runtime v5.7 |
| 366 | `hipMemsetD32Async` | OK | inoltrata alla runtime v5.7 |
| 367 | `hipMemsetD8` | OK | inoltrata alla runtime v5.7 |
| 368 | `hipMemsetD8Async` | OK | inoltrata alla runtime v5.7 |
| 369 | `hipMemset_spt` | OK | inoltrata alla runtime v5.7 |
| 370 | `hipMipmappedArrayCreate` | OK | inoltrata alla runtime v5.7 |
| 371 | `hipMipmappedArrayDestroy` | OK | inoltrata alla runtime v5.7 |
| 372 | `hipMipmappedArrayGetLevel` | OK | inoltrata alla runtime v5.7 |
| 373 | `hipModuleGetFunction` | **OK propria** | inoltrata; ricorda i kernel con hidden_dynamic_lds_size |
| 374 | `hipModuleGetGlobal` | OK | inoltrata alla runtime v5.7 |
| 375 | `hipModuleGetTexRef` | OK | inoltrata alla runtime v5.7 |
| 376 | `hipModuleLaunchCooperativeKernel` | **OK propria** | inoltrata, con kernelParams completato come hipModuleLaunchKernel; il driver risponde 719 |
| 377 | `hipModuleLaunchCooperativeKernelMultiDevice` | OK | inoltrata alla runtime v5.7 |
| 378 | `hipModuleLaunchKernel` | **OK propria** | inoltrata; per i kernel con hidden_dynamic_lds_size completa kernelParams |
| 379 | `hipModuleLaunchKernelExt` | OK | inoltrata alla runtime v5.7 |
| 380 | `hipModuleLoad` | **OK propria** | legge il file e prosegue come hipModuleLoadData |
| 381 | `hipModuleLoadData` | **OK propria** | bundle (anche compresso CCOB zstd) -> voce gfx906; code object v6 -> v5; sramecc on -> any |
| 382 | `hipModuleLoadDataEx` | **OK propria** | come hipModuleLoadData (le opzioni JIT sono ignorate, come nella 5.7) |
| 383 | `hipModuleOccupancyMaxActiveBlocksPerMultiprocessor` | OK | inoltrata alla runtime v5.7 |
| 384 | `hipModuleOccupancyMaxActiveBlocksPerMultiprocessorWithFlags` | OK | inoltrata alla runtime v5.7 |
| 385 | `hipModuleOccupancyMaxPotentialBlockSize` | OK | inoltrata alla runtime v5.7 |
| 386 | `hipModuleOccupancyMaxPotentialBlockSizeWithFlags` | OK | inoltrata alla runtime v5.7 |
| 387 | `hipModuleUnload` | OK | inoltrata alla runtime v5.7 |
| 388 | `hipOccupancyMaxActiveBlocksPerMultiprocessor` | **OK propria** | kernel registrati dallo shim: hipModuleOccupancy* della 5.7 |
| 389 | `hipOccupancyMaxActiveBlocksPerMultiprocessorWithFlags` | OK | inoltrata alla runtime v5.7 |
| 390 | `hipOccupancyMaxPotentialBlockSize` | **OK propria** | kernel registrati dallo shim: hipModuleOccupancy* della 5.7 |
| 391 | `hipPeekAtLastError` | **OK propria** | come hipGetLastError, senza azzerare |
| 392 | `hipPointerGetAttribute` | **OK tradotta** | tradotta: hipMemoryType 5.7 -> 7.x; tipo di un puntatore non HIP: hipErrorInvalidValue (la 5.7 cadeva) |
| 393 | `hipPointerGetAttributes` | **OK tradotta** | tradotta: hipMemoryType 5.7 -> 7.x |
| 394 | `hipProfilerStart` | OK | inoltrata alla runtime v5.7 |
| 395 | `hipProfilerStop` | OK | inoltrata alla runtime v5.7 |
| 396 | `hipRegisterTracerCallback` | OK | inoltrata alla runtime v5.7 |
| 397 | `hipRuntimeGetVersion` | OK | inoltrata alla runtime v5.7 |
| 398 | `hipSetDevice` | **OK propria** | inoltrata; alla chiusura del processo risponde lo shim |
| 399 | `hipSetDeviceFlags` | OK | inoltrata alla runtime v5.7 |
| 400 | `hipSetValidDevices` | **OK propria** | implementata: valida l'elenco e attiva il primo device buono |
| 401 | `hipSetupArgument` | OK | inoltrata alla runtime v5.7 |
| 402 | `hipSignalExternalSemaphoresAsync` | **OK tradotta** | tradotta: parametri 7.x (144 B) riportati al layout 5.7 (136 B) |
| 403 | `hipStreamAddCallback` | OK | inoltrata alla runtime v5.7 |
| 404 | `hipStreamAddCallback_spt` | OK | inoltrata alla runtime v5.7 |
| 405 | `hipStreamAttachMemAsync` | OK | inoltrata alla runtime v5.7 |
| 406 | `hipStreamBatchMemOp` | **NO** | la 5.7 non ha la primitiva batch mem-op |
| 407 | `hipStreamBeginCapture` | OK | inoltrata alla runtime v5.7 |
| 408 | `hipStreamBeginCaptureToGraph` | **NO** | la runtime 5.7 non ha i nodi grafo generici ne' hipGraphNodeParams |
| 409 | `hipStreamBeginCapture_spt` | OK | inoltrata alla runtime v5.7 |
| 410 | `hipStreamCreate` | OK | inoltrata alla runtime v5.7 |
| 411 | `hipStreamCreateWithFlags` | OK | inoltrata alla runtime v5.7 |
| 412 | `hipStreamCreateWithPriority` | OK | inoltrata alla runtime v5.7 |
| 413 | `hipStreamDestroy` | OK | inoltrata alla runtime v5.7 |
| 414 | `hipStreamEndCapture` | OK | inoltrata alla runtime v5.7 |
| 415 | `hipStreamEndCapture_spt` | OK | inoltrata alla runtime v5.7 |
| 416 | `hipStreamGetCaptureInfo` | OK | inoltrata alla runtime v5.7 |
| 417 | `hipStreamGetCaptureInfo_spt` | OK | inoltrata alla runtime v5.7 |
| 418 | `hipStreamGetCaptureInfo_v2` | OK | inoltrata alla runtime v5.7 |
| 419 | `hipStreamGetCaptureInfo_v2_spt` | OK | inoltrata alla runtime v5.7 |
| 420 | `hipStreamGetDevice` | OK | inoltrata alla runtime v5.7 |
| 421 | `hipStreamGetFlags` | OK | inoltrata alla runtime v5.7 |
| 422 | `hipStreamGetFlags_spt` | OK | inoltrata alla runtime v5.7 |
| 423 | `hipStreamGetPriority` | OK | inoltrata alla runtime v5.7 |
| 424 | `hipStreamGetPriority_spt` | OK | inoltrata alla runtime v5.7 |
| 425 | `hipStreamIsCapturing` | OK | inoltrata alla runtime v5.7 |
| 426 | `hipStreamIsCapturing_spt` | OK | inoltrata alla runtime v5.7 |
| 427 | `hipStreamQuery` | OK | inoltrata alla runtime v5.7 |
| 428 | `hipStreamQuery_spt` | OK | inoltrata alla runtime v5.7 |
| 429 | `hipStreamSynchronize` | OK | inoltrata alla runtime v5.7 |
| 430 | `hipStreamSynchronize_spt` | OK | inoltrata alla runtime v5.7 |
| 431 | `hipStreamUpdateCaptureDependencies` | OK | inoltrata alla runtime v5.7 |
| 432 | `hipStreamWaitEvent` | OK | inoltrata alla runtime v5.7 |
| 433 | `hipStreamWaitEvent_spt` | OK | inoltrata alla runtime v5.7 |
| 434 | `hipStreamWaitValue32` | OK | inoltrata alla runtime v5.7 |
| 435 | `hipStreamWaitValue64` | OK | inoltrata alla runtime v5.7 |
| 436 | `hipStreamWriteValue32` | OK | inoltrata alla runtime v5.7 |
| 437 | `hipStreamWriteValue64` | OK | inoltrata alla runtime v5.7 |
| 438 | `hipTexObjectCreate` | OK | inoltrata alla runtime v5.7 |
| 439 | `hipTexObjectDestroy` | OK | inoltrata alla runtime v5.7 |
| 440 | `hipTexObjectGetResourceDesc` | OK | inoltrata alla runtime v5.7 |
| 441 | `hipTexObjectGetResourceViewDesc` | OK | inoltrata alla runtime v5.7 |
| 442 | `hipTexObjectGetTextureDesc` | OK | inoltrata alla runtime v5.7 |
| 443 | `hipTexRefGetAddress` | OK | inoltrata alla runtime v5.7 |
| 444 | `hipTexRefGetAddressMode` | OK | inoltrata alla runtime v5.7 |
| 445 | `hipTexRefGetArray` | OK | inoltrata alla runtime v5.7 |
| 446 | `hipTexRefGetBorderColor` | OK | inoltrata alla runtime v5.7 |
| 447 | `hipTexRefGetFilterMode` | OK | inoltrata alla runtime v5.7 |
| 448 | `hipTexRefGetFlags` | OK | inoltrata alla runtime v5.7 |
| 449 | `hipTexRefGetFormat` | OK | inoltrata alla runtime v5.7 |
| 450 | `hipTexRefGetMaxAnisotropy` | OK | inoltrata alla runtime v5.7 |
| 451 | `hipTexRefGetMipMappedArray` | **OK propria** | alias di hipTexRefGetMipmappedArray della 5.7 (sola differenza di maiuscola) |
| 452 | `hipTexRefGetMipmapFilterMode` | OK | inoltrata alla runtime v5.7 |
| 453 | `hipTexRefGetMipmapLevelBias` | OK | inoltrata alla runtime v5.7 |
| 454 | `hipTexRefGetMipmapLevelClamp` | OK | inoltrata alla runtime v5.7 |
| 455 | `hipTexRefGetMipmappedArray` | OK | inoltrata alla runtime v5.7 |
| 456 | `hipTexRefSetAddress` | OK | inoltrata alla runtime v5.7 |
| 457 | `hipTexRefSetAddress2D` | OK | inoltrata alla runtime v5.7 |
| 458 | `hipTexRefSetAddressMode` | OK | inoltrata alla runtime v5.7 |
| 459 | `hipTexRefSetArray` | OK | inoltrata alla runtime v5.7 |
| 460 | `hipTexRefSetBorderColor` | OK | inoltrata alla runtime v5.7 |
| 461 | `hipTexRefSetFilterMode` | OK | inoltrata alla runtime v5.7 |
| 462 | `hipTexRefSetFlags` | OK | inoltrata alla runtime v5.7 |
| 463 | `hipTexRefSetFormat` | OK | inoltrata alla runtime v5.7 |
| 464 | `hipTexRefSetMaxAnisotropy` | OK | inoltrata alla runtime v5.7 |
| 465 | `hipTexRefSetMipmapFilterMode` | OK | inoltrata alla runtime v5.7 |
| 466 | `hipTexRefSetMipmapLevelBias` | OK | inoltrata alla runtime v5.7 |
| 467 | `hipTexRefSetMipmapLevelClamp` | OK | inoltrata alla runtime v5.7 |
| 468 | `hipTexRefSetMipmappedArray` | OK | inoltrata alla runtime v5.7 |
| 469 | `hipThreadExchangeStreamCaptureMode` | OK | inoltrata alla runtime v5.7 |
| 470 | `hipUnbindTexture` | OK | inoltrata alla runtime v5.7 |
| 471 | `hipUserObjectCreate` | OK | inoltrata alla runtime v5.7 |
| 472 | `hipUserObjectRelease` | OK | inoltrata alla runtime v5.7 |
| 473 | `hipUserObjectRetain` | OK | inoltrata alla runtime v5.7 |
| 474 | `hipWaitExternalSemaphoresAsync` | **OK tradotta** | tradotta: parametri 7.x (144 B) riportati al layout 5.7 (136 B) |

## Riepilogo

- **OK (inoltrate)**: 394
- **OK tradotte**: 10 (inoltrate traducendo l'ABI, vedi sotto)
- **OK propria**: 35
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

Oltre alle API `hip*`, lo shim esporta le funzioni interne del compilatore
`__hipRegisterFatBinary`, `__hipRegisterFunction`, `__hipRegisterVar`, `__hipRegisterManagedVar`,
`__hipRegisterSurface`, `__hipRegisterTexture`, `__hipOnError`, `__hipUnregisterFatBinary`
(implementate nello shim), `__hipPushCallConfiguration`, `__hipPopCallConfiguration`
(inoltrate alla runtime v5.7) e 4 funzioni diagnostiche `marconi_57exp_*`.

## Famiglie HIP 7 aggiunte il 13 novembre 2026

La runtime 5.7 non ha queste famiglie; lo shim le costruisce sui suoi mattoni (file
`runtime/src/api7*.inc`). Export della DLL salito da 484 a **557**.

| famiglia | funzioni | come |
|---|---|---|
| lanci corretti | `hipExtModuleLaunchKernel` (14 arg), `hipHccModuleLaunchKernel` (13), `hipTexRefSetMipmapLevelBias/Clamp` (float) | prototipo esplicito invece della macro a 9 argomenti; `hipDrvLaunchKernelEx` nuova |
| ricerche e stream | `hipLaunchKernelExC`, `hipGetFuncBySymbol`, `hipGetDriverEntryPoint(_spt)`, `hipStreamGetId`, `hipStreamGet/Set/CopyAttributes`, `hipMemsetD2D8/16/32(Async)`, `hipGetDevicePropertiesR0000`, `hipChooseDeviceR0000/R0600`, `hipDeviceGetTexture1DLinearMaxWidth`, occupazione | sulle primitive 5.7 |
| moduli e librerie | `hipModuleGetFunctionCount`, `hipModuleLoadFatBinary`, `hipLibrary*`, `hipKernel*`, `hipLink*` | involucro sull'API a moduli 5.7; link JIT su hipRTC 5.7 |
| copie | `hipMemcpyAtoD/DtoA/AtoA`, `hipMemcpy2DArrayToArray`, `hipMemcpy3DPeer(Async)` | buffer host per gli array; copia 3D per il «peer» su una GPU |
| grafi | `hipGraphAddNode`, `hipGraphNodeSetParams`, `hipGraphExecNodeSetParams`, `hipStreamBatchMemOp` | nodi generici tradotti nei nodi tipizzati 5.7; i nodi-kernel tradotti come i lanci |
| green context | `hipGreenCtxCreate`, `hipExecutionCtx*`, `hipDeviceGetDevResource`, `hipDevSmResourceSplit(ByCount)`, `hipDevResourceGenerateDesc`, `hipStreamGetDevResource` | maschera di CU (`hipExtStreamCreateWithCUMask`) |
| completamento | `hipMemGet/SetMemPool`, `hipMemAdvise_v2`, `hipMemPrefetchAsync_v2`, `hipMemcpyBatchAsync`, `hipDeviceGetLuid`, logging/profiler | mappate o stub diagnostici |

**Scaricamento dei moduli (correzione della revisione, 13 nov 2026):** il driver riusa gli indirizzi delle
funzioni quando si scarica e ricarica un modulo (misurato: 27 indirizzi distinti su 30 cicli). Lo shim non
libera piu' le descrizioni dei kernel di un modulo scaricato (le stacca soltanto) e aggiorna sempre
l'associazione funzione -> argomenti da completare, cosi' un kernel nuovo a un indirizzo riusato riceve il
completamento giusto di `hidden_dynamic_lds_size`.

**`__hipRegisterManagedVar`:** esportata perche' il codice compilato la chiama, ma le variabili `__managed__`
statiche non vengono allocate: il loro puntatore resta vuoto. Sono rare nel codice di calcolo.

**Limiti noti (restano `hipErrorNotSupported`):** nodi di grafo avanzati (semafori esterni, batch-mem-op,
API driver dei grafi, cattura verso grafo, istanziazione con parametri), `hipMemcpy3DBatchAsync` e mipmap
da memoria esterna, perché la 5.7 non ha le primitive corrispondenti. Le operazioni di memoria sugli
stream (`hipStreamWaitValue`) non sono supportate dal driver su gfx906/Windows: la scrittura usa
`hipMemsetD32Async`, l'attesa di un valore è vuota su stream in ordine.

## Codice compilato con ROCm 7 e 10 (12 nov 2026)

`runtime/src/registrazione.inc`. Lo scopo: far girare sulla runtime 5.7 del driver programmi e
librerie compilati con HIP 7.x, comprese le wheel AMD di PyTorch 2.15 (ROCm 10.1, HIP 7.16).
Le librerie della wheel restano quelle di AMD, con i loro difetti: la MIOpen 10.1 sbaglia ancora il
gradiente dei pesi con `ConvAsmBwdWrW3x3` (conv 3x3 a 3 canali, scarto relativo 3,5 contro la CPU),
come la MIOpen non corretta. Con `MIOPEN_DEBUG_CONV_DIRECT_ASM_WRW3X3=0` e
`MIOPEN_DEBUG_CONV_IMPLICIT_GEMM_ASM_WRW_V4R1=0` lo scarto torna a 2,6e-7 (misurato il 12 nov 2026).

- **Registrazione dei kernel.** `__hipRegisterFatBinary/Function/Var` tengono la registrazione nello
  shim; il codice si carica al primo lancio (o alla prima richiesta di attributi o simboli) di quel
  fat binary, con le API a moduli della 5.7. Registrare non tocca la GPU. `hipLaunchKernel` e le
  funzioni per puntatore host (attributi, occupazione, simboli) usano quella registrazione; per
  ciò che lo shim non ha registrato inoltrano alla 5.7.
- **Formati del codice.** Involucro `HIPF` (bundle clang, anche compresso `CCOB` con zstd: i file
  `.co` di Tensile in ROCm 10) e `HIPK` (kpack di ROCm 10, letti con `rocm_kpack.dll` delle wheel).
  Il decompressore è zstd 1.5.7 (BSD), in `runtime/src/zstd`.
- **Adattamenti per il caricatore della 5.7**, sulla copia passata alla runtime: code object v6 ->
  v5 (campo `EI_ABIVERSION`); gfx906 con sramecc on -> any (la Radeon VII non ha ECC e il driver
  rifiuta "on"; hiprtc compila così perché lo shim dichiara `gfx906:sramecc+:xnack-`).
- **Argomento nascosto `hidden_dynamic_lds_size`** (kernel con memoria condivisa dinamica, ROCm 6+):
  la 5.7 non lo conosce e lo conta come argomento esplicito, per cui legge una voce di `kernelParams`
  oltre la fine dell'array. Lo shim legge gli argomenti con l'API pubblica di comgr dopo il
  caricamento e al lancio passa l'array completo, con la dimensione della memoria condivisa del
  lancio. I code object non vengono modificati.
- **Errori**: `hipGetLastError`/`hipPeekAtLastError`/`hipExtGetLastError` riportano anche i lanci
  falliti dentro lo shim. PyTorch controlla ogni lancio così.
- **Due amd_comgr.dll.** Le wheel AMD precaricano la `amd_comgr.dll` di ROCm 10; la runtime del driver
  la importa per nome e si legherebbe a quella (crash al primo caricamento di codice). Lo shim carica
  per percorso completo quella del driver e ci ripunta gli import della runtime del driver.
- **Chiusura del processo**: la runtime del driver, caricata per ultima, viene staccata per prima.
  Durante `ExitProcess` lo shim non le inoltra più nulla: pulizia (Free, Destroy, Unload, Unregister,
  Release) data per riuscita, `hipGetDevice`/`hipSetDevice` con l'ultimo dispositivo, il resto
  `hipErrorDeinitialized`.

Limiti: bundle compressi con zlib non supportati (solo zstd); i lanci cooperativi (`hipLaunchCooperativeKernel`,
`hipModuleLaunchCooperativeKernel`) il driver li rifiuta con 719 anche con codice compilato con HIP 5.7:
lo shim li inoltra correttamente, e dal 13 nov 2026 `hipGetDevicePropertiesR0600` dichiara
`cooperativeLaunch = 0` come il driver (prima 1); dei tipi di argomento nascosti posteriori alla 5.7 è gestito solo
`hidden_dynamic_lds_size`, l'unico trovato.

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