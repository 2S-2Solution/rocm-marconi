/*
 * api7_graph.inc - gruppo 2d: nodi di grafo generici e operazioni a lotti sugli stream.
 * 2 Solution, 13 nov 2026. Incluso da shim_main.c dopo api7_mem.inc.
 *
 * HIP 7 aggiunge i nodi generici (hipGraphAddNode con hipGraphNodeParams, un'unione discriminata da
 * .type) e li traduce nelle funzioni tipizzate della 5.7 (hipGraphAddKernelNode, ...AddMemcpyNode, ...).
 * Le disposizioni di hipKernelNodeParams, hipMemsetParams e hipHostNodeParams sono identiche fra 5.7 e
 * 7.16 (misurato), quindi i sotto-parametri si passano per puntatore senza conversione.
 *
 * hipGraphNodeParams (7.16): type a 0, poi padding, l'unione dei sotto-parametri comincia a 16.
 *   kernel  -> +16  hipKernelNodeParams
 *   memcpy  -> +16  hipMemcpyNodeParams { flags, reserved[3], copyParams } -> copyParams a +32
 *   memset  -> +16  hipMemsetParams
 *   host    -> +16  hipHostNodeParams
 *   graph   -> +16  hipGraph_t (figlio)
 *   event   -> +16  hipEvent_t
 *   memFree -> +16  void* (dptr)
 *
 * I nodi batch-mem-op nei grafi restano non supportati: la 5.7 non ha hipGraphAddBatchMemOpNode e un
 * grafo e' opaco, non vi si puo' iniettare un nodo che la runtime non conosce. hipStreamBatchMemOp su
 * stream invece si costruisce con hipStreamWaitValue32/64 e hipStreamWriteValue32/64 della 5.7.
 */

enum { API7_NODO_KERNEL = 0, API7_NODO_MEMCPY = 1, API7_NODO_MEMSET = 2, API7_NODO_HOST = 3,
       API7_NODO_GRAPH = 4, API7_NODO_EMPTY = 5, API7_NODO_WAITEVENT = 6, API7_NODO_EVENTREC = 7,
       API7_NODO_EXTSEMSIG = 8, API7_NODO_EXTSEMWAIT = 9, API7_NODO_MEMALLOC = 10, API7_NODO_MEMFREE = 11 };

typedef int (*api7_g_params)(void** node, void* graph, const void** deps, size_t nDeps, const void* params);
typedef int (*api7_g_handle)(void** node, void* graph, const void** deps, size_t nDeps, void* handle);
typedef int (*api7_g_empty)(void** node, void* graph, const void** deps, size_t nDeps);

/* Un nodo kernel ha .func (un host stub registrato dallo shim) e .kernelParams. La 5.7 non conosce gli
 * host stub dello shim, quindi come nei lanci si traduce func -> funzione di dispositivo e si completa
 * l'array degli argomenti (hidden_dynamic_lds_size). hipKernelNodeParams (64 byte, identica fra 5.7 e
 * 7.16): func a 24, kernelParams a 48, sharedMemBytes a 56. La copia tradotta sta in 'copia'; il valore
 * restituito va liberato con reg_completo_libera(ret, kparams_orig, locale) dopo la chiamata alla 5.7. */
static void** api7_kernel_tradotto(const unsigned char* kp, unsigned char* copia, void** locale) {
    void* func = *(void* const*)(kp + 24);
    void** kparams = *(void** const*)(kp + 48);
    void* devf = NULL;
    memcpy(copia, kp, 64);
    if (reg_funzione(func, &devf) >= 0 && devf) {
        void** completo = reg_completa(devf, kparams, (unsigned int*)(copia + 56), locale);
        *(void**)(copia + 24) = devf;
        *(void***)(copia + 48) = completo ? completo : kparams;
        return completo ? completo : kparams;
    }
    return kparams;   /* non registrato dallo shim: host stub noto alla 5.7, passa invariato */
}

__declspec(dllexport) int hipGraphAddNode(void** pGraphNode, void* graph, const void** deps, size_t nDeps,
                                          void* nodeParams) {
    const unsigned char* np = (const unsigned char*)nodeParams;
    int type;
    if (!pGraphNode || !graph || !nodeParams) return hipErrorInvalidValue;
    type = *(const int*)np;
    switch (type) {
        case API7_NODO_KERNEL:  { unsigned char copia[64]; void* locale[64]; void** c; int rc;
                                  ABI57_REALE(f, api7_g_params, "hipGraphAddKernelNode");
                                  c = api7_kernel_tradotto(np + 16, copia, locale);
                                  rc = f(pGraphNode, graph, deps, nDeps, copia);
                                  reg_completo_libera(c, *(void** const*)(np + 16 + 48), locale); return rc; }
        case API7_NODO_MEMCPY:  { ABI57_REALE(f, api7_g_params, "hipGraphAddMemcpyNode"); return f(pGraphNode, graph, deps, nDeps, np + 32); }
        case API7_NODO_MEMSET:  { ABI57_REALE(f, api7_g_params, "hipGraphAddMemsetNode"); return f(pGraphNode, graph, deps, nDeps, np + 16); }
        case API7_NODO_HOST:    { ABI57_REALE(f, api7_g_params, "hipGraphAddHostNode");   return f(pGraphNode, graph, deps, nDeps, np + 16); }
        case API7_NODO_GRAPH:   { ABI57_REALE(f, api7_g_handle, "hipGraphAddChildGraphNode"); return f(pGraphNode, graph, deps, nDeps, *(void**)(np + 16)); }
        case API7_NODO_EMPTY:   { ABI57_REALE(f, api7_g_empty,  "hipGraphAddEmptyNode");  return f(pGraphNode, graph, deps, nDeps); }
        case API7_NODO_WAITEVENT: { ABI57_REALE(f, api7_g_handle, "hipGraphAddEventWaitNode");   return f(pGraphNode, graph, deps, nDeps, *(void**)(np + 16)); }
        case API7_NODO_EVENTREC:  { ABI57_REALE(f, api7_g_handle, "hipGraphAddEventRecordNode"); return f(pGraphNode, graph, deps, nDeps, *(void**)(np + 16)); }
        case API7_NODO_MEMALLOC:  { ABI57_REALE(f, api7_g_params, "hipGraphAddMemAllocNode"); return f(pGraphNode, graph, deps, nDeps, np + 16); }
        case API7_NODO_MEMFREE:   { ABI57_REALE(f, api7_g_handle, "hipGraphAddMemFreeNode");  return f(pGraphNode, graph, deps, nDeps, *(void**)(np + 16)); }
        default: return hipErrorNotSupported;   /* semafori esterni e batch mem-op nei grafi: non nei grafi 5.7 */
    }
}

/* tipo del nodo (hipGraphNodeType) -> funzione *NodeSetParams della 5.7 */
typedef int (*api7_g_nodetype)(void* node, int* tipo);
typedef int (*api7_g_setp)(void* node, const void* params);
typedef int (*api7_g_setev)(void* node, void* event);
typedef int (*api7_g_execsetp)(void* exec, void* node, const void* params);
typedef int (*api7_g_execsetev)(void* exec, void* node, void* event);

static int api7_tipo_nodo(void* node, int* tipo) {
    ABI57_REALE(f, api7_g_nodetype, "hipGraphNodeGetType");
    return f(node, tipo);
}

__declspec(dllexport) int hipGraphNodeSetParams(void* node, void* nodeParams) {
    const unsigned char* np = (const unsigned char*)nodeParams;
    int tipo, rc;
    if (!node || !nodeParams) return hipErrorInvalidValue;
    if ((rc = api7_tipo_nodo(node, &tipo)) != hipSuccess) return rc;
    switch (tipo) {
        case API7_NODO_KERNEL: { unsigned char copia[64]; void* locale[64]; void** c; int rc;
                                 ABI57_REALE(f, api7_g_setp, "hipGraphKernelNodeSetParams");
                                 c = api7_kernel_tradotto(np + 16, copia, locale);
                                 rc = f(node, copia);
                                 reg_completo_libera(c, *(void** const*)(np + 16 + 48), locale); return rc; }
        case API7_NODO_MEMCPY: { ABI57_REALE(f, api7_g_setp, "hipGraphMemcpyNodeSetParams"); return f(node, np + 32); }
        case API7_NODO_MEMSET: { ABI57_REALE(f, api7_g_setp, "hipGraphMemsetNodeSetParams"); return f(node, np + 16); }
        case API7_NODO_HOST:   { ABI57_REALE(f, api7_g_setp, "hipGraphHostNodeSetParams");   return f(node, np + 16); }
        case API7_NODO_WAITEVENT: { ABI57_REALE(f, api7_g_setev, "hipGraphEventWaitNodeSetEvent");   return f(node, *(void**)(np + 16)); }
        case API7_NODO_EVENTREC:  { ABI57_REALE(f, api7_g_setev, "hipGraphEventRecordNodeSetEvent"); return f(node, *(void**)(np + 16)); }
        default: return hipErrorNotSupported;
    }
}

__declspec(dllexport) int hipGraphExecNodeSetParams(void* graphExec, void* node, void* nodeParams) {
    const unsigned char* np = (const unsigned char*)nodeParams;
    int tipo, rc;
    if (!graphExec || !node || !nodeParams) return hipErrorInvalidValue;
    if ((rc = api7_tipo_nodo(node, &tipo)) != hipSuccess) return rc;
    switch (tipo) {
        case API7_NODO_KERNEL: { unsigned char copia[64]; void* locale[64]; void** c; int rc;
                                 ABI57_REALE(f, api7_g_execsetp, "hipGraphExecKernelNodeSetParams");
                                 c = api7_kernel_tradotto(np + 16, copia, locale);
                                 rc = f(graphExec, node, copia);
                                 reg_completo_libera(c, *(void** const*)(np + 16 + 48), locale); return rc; }
        case API7_NODO_MEMCPY: { ABI57_REALE(f, api7_g_execsetp, "hipGraphExecMemcpyNodeSetParams"); return f(graphExec, node, np + 32); }
        case API7_NODO_MEMSET: { ABI57_REALE(f, api7_g_execsetp, "hipGraphExecMemsetNodeSetParams"); return f(graphExec, node, np + 16); }
        case API7_NODO_HOST:   { ABI57_REALE(f, api7_g_execsetp, "hipGraphExecHostNodeSetParams");   return f(graphExec, node, np + 16); }
        case API7_NODO_WAITEVENT: { ABI57_REALE(f, api7_g_execsetev, "hipGraphExecEventWaitNodeSetEvent");   return f(graphExec, node, *(void**)(np + 16)); }
        case API7_NODO_EVENTREC:  { ABI57_REALE(f, api7_g_execsetev, "hipGraphExecEventRecordNodeSetEvent"); return f(graphExec, node, *(void**)(np + 16)); }
        default: return hipErrorNotSupported;
    }
}

/* ---- operazioni a lotti sugli stream ----
 * hipStreamBatchMemOpParams: unione di 48 byte; ogni elemento ha operation (int) a 0, indirizzo a 8,
 * valore (uint32 o uint64) a 16, flags a 24. Operazioni: WaitValue32=1, WriteValue32=2,
 * FlushRemoteWrites=3, WaitValue64=4, WriteValue64=5, Barrier=6.
 *
 * Il driver 5.7 non supporta le stream memory operations su gfx906/Windows
 * (hipDeviceAttributeCanUseStreamWaitValue = 0): hipStreamWriteValue/WaitValue rispondono 0 ma non
 * fanno nulla. Qui la scrittura si fa con hipMemsetD32Async, che scrive davvero sullo stream; l'attesa
 * di un valore non e' realizzabile senza supporto hardware, ma su uno stream in ordine il valore scritto
 * prima e' gia' presente quando il consumatore lo legge, quindi e' un'attesa vuota (non si fa busy-wait
 * sul device, che rischierebbe un blocco). Documentato come limite del driver. */
typedef int (*api7_f_ms32a)(void* d, int v, size_t n, void* s);

static int api7_scrivi32(void* stream, void* ptr, unsigned int value) {
    ABI57_REALE(ms, api7_f_ms32a, "hipMemsetD32Async");
    return ms(ptr, (int)value, 1, stream);
}

__declspec(dllexport) int hipStreamBatchMemOp(void* stream, unsigned int count, void* paramArray, unsigned int flags) {
    const unsigned char* p = (const unsigned char*)paramArray;
    unsigned int i;
    (void)flags;
    if (count && !paramArray) return hipErrorInvalidValue;
    for (i = 0; i < count; i++) {
        const unsigned char* e = p + (size_t)i * 48;
        int op = *(const int*)e;
        void* ptr = *(void* const*)(e + 8);
        int rc = hipSuccess;
        switch (op) {
            case 2: rc = api7_scrivi32(stream, ptr, *(const unsigned int*)(e + 16)); break;
            case 5: { unsigned long long v = *(const unsigned long long*)(e + 16);
                      rc = api7_scrivi32(stream, ptr, (unsigned int)v);
                      if (rc == hipSuccess) rc = api7_scrivi32(stream, (char*)ptr + 4, (unsigned int)(v >> 32)); break; }
            case 1: case 4: break;    /* wait value: il driver non lo supporta; attesa vuota su stream in ordine */
            case 3: case 6: break;    /* flush remote writes / barrier: senza effetto su una GPU sola */
            default: return hipErrorInvalidValue;
        }
        if (rc != hipSuccess) return rc;
    }
    return hipSuccess;
}
