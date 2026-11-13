# rocm-marconi — kit ROCm 5.7 «Marconi» per Radeon VII (gfx906) su Windows

**2S · 2Solution** · [English below](#english)

## Cosa fa

AMD ha tolto le GPU Vega (gfx906/gfx900) dal supporto ROCm per Windows: l'ultima runtime
HIP che le vede è la **5.7** fornita col driver Adrenalin, mentre il software moderno
(PyTorch, MIOpen, ZLUDA, rocBLAS recenti) si aspetta le API HIP 6.x/7.x. **Marconi** è il
ponte: un kit che si affianca a ROCm 5.7 e aggiunge ciò che manca. Il pacchetto delle
Releases si installa in una cartella propria dell'utente, senza diritti di amministratore,
e non modifica né il driver (`System32\amdhip64.dll`) né l'HIP SDK
(`C:\Program Files\AMD\ROCm\5.7`). Fanno eccezione, per scelta esplicita di chi li lancia,
due strumenti di sviluppo in `tensile/` e `strumenti/instrada_shape.py --applica`, che
possono riscrivere la libreria rocBLAS dell'HIP SDK (vedi più sotto).

Questo repository contiene i **sorgenti** del kit:

| cartella | contenuto |
|---|---|
| `runtime/src/shim_main.c`, `runtime/build_shim.cmd` | lo shim **`amdhip64_7_shim.dll`** (installato anche come `amdhip64_7.dll`): **557 export**, cioè **553 API `hip*`/`__hip*`** e 4 funzioni diagnostiche `marconi_57exp_*`. Le API coprono **428/428** funzioni della runtime 5.7, l'unione ROCm 6.1+6.2+6.4+7.1 e, dal 13 novembre 2026, le famiglie di HIP 7 che la runtime 5.7 non ha: moduli e librerie (`hipLibrary*`, `hipKernel*`, `hipModuleGetFunctionCount`), link JIT (`hipLink*`), copie da/verso array e fra dispositivi, nodi di grafo generici (`hipGraphAddNode`), operazioni a lotti sugli stream e contesti "green" (`hipGreenCtxCreate`, `hipExecutionCtx*`). Le funzioni interne del compilatore (`__hipRegisterFatBinary`, `__hipRegisterFunction`, `__hipRegisterVar`, `__hipRegisterManagedVar`, …) sono comprese nel conto |
| `include/hip-extensions/` | intestazioni per le API HIP moderne (`hipMallocAsync`, `hipGraph*`, cattura di stream) e la macro `HAVE_HIP_MARCONI_57EXP` |
| `tensile/` | strumenti della macchina di sviluppo per la libreria Tensile/rocBLAS gfx906 dell'HIP SDK: correzione degli indici e instradamento delle forme reali ai kernel più veloci misurati (`override/*.csv`). Il pacchetto delle Releases porta già la libreria corretta e **non li richiede** |
| `strumenti/` | sei script Python: audit della libreria Tensile (`audit_coerenza.py`, `audit_ldl.py`), versione e modalità sramecc/xnack dei code object rispetto a ciò che il driver carica (`code_object_tensile.py`), instradamento delle forme (`instrada_shape.py`), misura delle soluzioni rocBLAS di una GEMM (`misura_soluzioni_gemm.py`), confronto degli export HIP fra versioni ROCm (`confronto_versioni_hip.py`) |
| `docs/` | [LEGGIMI_v2.md](docs/LEGGIMI_v2.md) (stato e fatti verificati) e [FUNZIONI_473.md](docs/FUNZIONI_473.md) (stato di ogni export) |

Il resto del kit è fatto di **binari di terzi** e non è nel git: rocBLAS 5.7 dell'HIP SDK
con la libreria di kernel gfx906 corretta, hipRTC 5.7 con il suo comgr (`amd_comgr0507.dll`),
clang/lld 21. Si installa dal pacchetto pubblicato nelle Releases, che elenca origine e licenza di
ogni file. MIOpen non fa parte del kit: è nel pacchetto
[miopen-gfx906](https://github.com/2s-2solution/miopen-gfx906).

### Come funziona lo shim

- **428 funzioni** vanno alla runtime 5.7 del driver (`System32\amdhip64.dll`). 418 sono inoltrate
  così come sono. Le altre **10 vengono tradotte**, perché passano tipi cambiati fra 5.7 e 7.x:
  - `hipMemoryType` è stato rinumerato;
  - `HIP_MEMCPY3D` ha campi a 32 bit in 5.7 e a 64 in 7.x;
  - i parametri dei semafori esterni hanno i campi spostati.

  Senza traduzione un programma compilato con gli header 6.x/7.x leggeva la memoria GPU come «host»
  e le copie 3D davano successo con i dati sbagliati. Il confronto fra gli header è sistematico (tutte
  le 50 strutture e i 470 enumeratori comuni): fuori da questi tre casi l'ABI coincide. Dettagli in
  `runtime/src/abi57.inc` e in [FUNZIONI_473.md](docs/FUNZIONI_473.md).
- **Difetto della 5.7 aggirato:** `hipDrvPointerGetAttributes(MEMORY_TYPE)` cade su alcuni puntatori host
  non registrati, per esempio la memoria di un tensore CPU di PyTorch (`is_pinned()` terminava il
  processo). Lo shim chiede prima a `hipPointerGetAttributes`, che su quei puntatori risponde senza
  cadere, e per la memoria non HIP restituisce il tipo «non registrato».
- **Oltre 150 funzioni** sono implementate o tradotte dentro lo shim (elenco in
  [FUNZIONI_473.md](docs/FUNZIONI_473.md), stato «OK propria» o «OK tradotta»): fra queste
  `hipGetProcAddress`, `hipSetValidDevices`, `hipDrvLaunchKernelEx`, `hipGetDevicePropertiesR0600`,
  le registrazioni dei kernel per il codice ROCm 7 e 10, e dal 13 nov 2026 le API HIP 7 di moduli,
  librerie, kernel, link JIT, copie array/peer, nodi di grafo generici, batch-mem-op e green context.
- **Lanci corretti nei prototipi (13 nov 2026):** `hipExtModuleLaunchKernel` (14 argomenti) e
  `hipHccModuleLaunchKernel` (13) hanno ora il prototipo esplicito: prima erano inoltrati con la
  macro a 9 argomenti e funzionavano solo perché il compilatore ne faceva un salto in coda. Corretti
  anche due codici d'errore (`hipErrorInvalidValue` era 7, `hipErrorInvalidSymbol` era 701).
- **Codice compilato con ROCm 7 e 10** (dal 12 nov 2026, `runtime/src/registrazione.inc`): la
  registrazione dei kernel incorporati (`__hipRegisterFatBinary` e simili, `<<<...>>>`) è tenuta
  dallo shim; il codice si carica al primo uso dai bundle clang (anche compressi con zstd) e dai
  kpack di ROCm 10; i code object v6 e quelli "sramecc on" vengono adattati al caricatore del
  driver; l'argomento nascosto `hidden_dynamic_lds_size`, che la 5.7 non conosce, viene
  completato al lancio. Con questo girano le wheel AMD di PyTorch 2.15 (ROCm 10.1) sulla
  Radeon VII. Dettagli in [FUNZIONI_473.md](docs/FUNZIONI_473.md#codice-compilato-con-rocm-7-e-10-12-nov-2026).
- **`hipGetDevicePropertiesR0600` restituisce valori fissi della Radeon VII**, senza
  interrogare il driver e per qualunque indice di dispositivo: nome «AMD Radeon VII»,
  `gcnArchName` `gfx906:sramecc+:xnack-`, 16 GB, 60 CU, 1801 MHz, wave64 e compute capability
  **7.0** (`major=7`, nello stile di ZLUDA), per ogni programma che la chiami. La usano ZLUDA
  (verificato nel sorgente) e i programmi compilati con le intestazioni HIP 6.x/7.x, dove
  `hipGetDeviceProperties` è un alias di `hipGetDevicePropertiesR0600`. Su una scheda diversa
  i valori sarebbero sbagliati: è uno dei motivi per cui il kit è dichiarato solo per la
  Radeon VII.
  `hipGetDeviceProperties` (senza suffisso) invece è inoltrata e riporta i valori veri.
- **35 funzioni** che richiederebbero strutture assenti in 5.7 sono esportate ma rispondono
  **`hipErrorNotSupported` (801)** in modo esplicito: meglio un errore dichiarato che un
  simbolo mancante o un inoltro che corrompe la memoria.
- **Ponte sullo stream nullo** (ottobre 2026): la runtime 5.7 restituisce
  `hipErrorInvalidValue` da `hipStreamWaitEvent` quando a dover attendere è lo stream nullo;
  lo shim fa attendere uno stream bloccante di servizio. Senza, PyTorch con più stream e
  MIOpen fallivano.

## Usato da

Il kit è la base di altri componenti 2S, ciascuno nel proprio repository:
- [zluda-gfx906](https://github.com/2s-2solution/zluda-gfx906): ZLUDA (CUDA su HIP) per gfx906;
- [miopen-gfx906](https://github.com/2s-2solution/miopen-gfx906): MIOpen corretta per gfx906;
- [la-ruota](https://github.com/2s-2solution/la-ruota): PyTorch compilato per ROCm su gfx906.

## Hardware e software

- AMD Radeon VII (Vega 20, gfx906). gfx900 (Vega 10) condivide l'ISA ma non è stata provata
  con questa versione.
- Windows 10 Pro 22H2 (Windows 11 non provato), driver AMD Adrenalin 31.0.21923.11000 con
  runtime HIP 5.7. Il pacchetto delle Releases non richiede l'HIP SDK.
- Per compilare lo shim: Visual Studio Build Tools 2022 e clang-cl (LLVM 17 o successivo;
  provato con clang 21).

## Risultati misurati

| verifica | esito |
|---|---|
| copertura export runtime 5.7 | 428 / 428 |
| copertura unione ROCm 6.1–7.1 | 469 / 469 (553 API `hip*`/`__hip*`, 557 export in tutto) |
| famiglie HIP 7 aggiunte (13 nov 2026) | moduli, librerie, kernel, link JIT, copie array/peer, nodi-grafo generici, batch-mem-op, green context |
| PyTorch 2.15 delle wheel AMD (ROCm 10.1, HIP 7.16) sulla Radeon VII | 12 / 12 prove contro la CPU in fp64: elementari e riduzioni (kpack), rocBLAS fp32/fp16, MIOpen (conv in avanti), addestramento, rocRAND, rocSOLVER, rocFFT. ⚠️ La MIOpen 10.1 della wheel ha ancora il difetto del gradiente dei pesi `ConvAsmBwdWrW3x3` (conv 3x3 a 3 canali: scarto relativo 3,5): impostare `MIOPEN_DEBUG_CONV_DIRECT_ASM_WRW3X3=0` e `MIOPEN_DEBUG_CONV_IMPLICIT_GEMM_ASM_WRW_V4R1=0` (scarto 2,6e-7), oppure usare la MIOpen corretta di [miopen-gfx906](https://github.com/2s-2solution/miopen-gfx906) |
| programma compilato con hipcc 7.16 (`<<<...>>>`, `__constant__`, `__device__`, template, attributi, eventi) | OK |
| catena di caricamento con `MIOpen.dll` | 8 DLL su 8 |
| audit libreria Tensile gfx906 (tipo × trasposizione) | 42 / 42 complete (prima 41 / 42: un percorso fp16 senza kernel di ripiego) |

Fatti accertati disassemblando i binari: il moltiplicatore fp16 dipende da un solo
parametro Tensile, `LocalDotLayout` (LDL=1 → `v_fma_mix_f32`, tetto 13,4 TFLOPS; LDL=2 →
`v_dot2_f32_f16`, ~21 TFLOPS); **gfx906 non ha istruzioni bf16** (il bf16 è emulato). Dettagli
in [docs/LEGGIMI_v2.md](docs/LEGGIMI_v2.md).

## Installazione rapida

Scaricare il pacchetto autoestraente dalla pagina
**[Releases](https://github.com/2s-2solution/rocm-marconi/releases)** e verificarne lo SHA256
pubblicato accanto. Estrarlo in una cartella qualsiasi e lanciare `installa.cmd`: il kit va in
`%LOCALAPPDATA%\2S\marconi` (o in `-Destinazione <cartella>`), si impostano le variabili
dell'utente e parte una verifica sulla GPU. Istruzioni complete nel `LEGGIMI.md` del
pacchetto. I pacchetti binari non sono nel repository git.

## Come si compila lo shim

```bat
runtime\build_shim.cmd
```

`build_shim.cmd` compila `runtime\src\shim_main.c`, con il decompressore zstd di
`runtime\src\zstd`, della copia del repository e scrive in
`runtime\` `amdhip64_7_shim.dll` e `amdhip64_7_shim.lib`. Come compilatore usa, nell'ordine:
`LLVM_BIN\clang-cl.exe` se `LLVM_BIN` è impostata, poi il clang 21 del kit Marconi installato
(`%MARCONI_ROOT%\compiler\bin\clang.exe --driver-mode=cl`), poi `C:\Program Files\LLVM\bin`.
Con il clang del kit il codice esce **identico** a quello della DLL pubblicata (verificato
confrontando il disassemblato); un LLVM più recente (22) dà codice diverso, non validato.
Le opzioni di compilazione sono nello script.

Per usarla, copiare la DLL nel `bin` del kit installato (`%MARCONI_ROOT%\bin`) **sia** come
`amdhip64_7_shim.dll` **sia** come `amdhip64_7.dll`. ⚠️ Con clang-cl usare **`/O2`**:
`-O3` viene scartato in silenzio e la build resta non ottimizzata. Dopo la copia
verificare l'impronta del file: una DLL in uso non si sovrascrive e l'errore è muto.
`amdhip64_7_shim.lib` serve per compilare [zluda-gfx906](https://github.com/2s-2solution/zluda-gfx906).

## Limiti noti

- Le 35 funzioni elencate come **NO** in [FUNZIONI_473.md](docs/FUNZIONI_473.md)
  (nodi grafo generici, semafori esterni nei grafi, varianti di strutture `R0000`/`R0600`…)
  rispondono 801.
- La registrazione dei kernel (`__hipRegisterFatBinary` e simili) è implementata nello shim dal
  12 nov 2026: un programma o una libreria compilati con HIP 7.x che lanciano kernel incorporati
  con `<<<...>>>` girano attraverso lo shim, comprese le wheel AMD di PyTorch 2.15 (kpack di ROCm 10,
  bundle compressi). Dettagli e limiti in [FUNZIONI_473.md](docs/FUNZIONI_473.md#codice-compilato-con-rocm-7-e-10-12-nov-2026):
  bundle compressi con zlib non supportati; i lanci cooperativi li rifiuta il driver stesso (719,
  anche con codice compilato con HIP 5.7), e lo shim dichiara `cooperativeLaunch = 0` come il driver.
- Nella libreria rocBLAS del kit i kernel per gfx900 (Vega 10) sono code object **v6**, che il
  driver Adrenalin non carica (lo mostra `strumenti/code_object_tensile.py --arch gfx900`): su
  una Vega 10 rocBLAS del kit non funzionerebbe. Quelli per gfx906 sono v4 e v5.
- Le macro di inoltro passano 9 argomenti interi: regge per tutte le API HIP attuali, ma non
  per una funzione che prenda `float`/`double` per valore.
- Un solo motore di calcolo per volta: Windows (WDDM) serializza le code di calcolo della
  scheda.
- La via «costruire la runtime HIP nuova per gfx906» è chiusa: lo strato PAL del driver
  Windows non elenca più alcuna GPU GFX9.
- Lo shim carica la runtime del driver da `C:\Windows\System32\amdhip64.dll` (percorso
  fisso: Windows installato altrove non è previsto).
- `tensile/` e `strumenti/instrada_shape.py --applica` sono strumenti della macchina di
  sviluppo: scrivono nella libreria rocBLAS dell'HIP SDK
  (`C:\Program Files\AMD\ROCm\5.7\bin\rocblas\library`, servono i diritti di
  amministratore) e conservano i file originali con un suffisso `.bak_*`. Con il pacchetto
  delle Releases non servono: la sua libreria ha già gli indici corretti.
- Gli script in `strumenti/` cercano il kit in `MARCONI_ROOT`, altrimenti nella cartella
  storica `C:\rocm_5_7_expansion_pack`, e l'HIP SDK nel suo percorso predefinito.
- Le forme in `instrada_shape.py` e in `tensile/override/` sono GEMM magre (N = 243-330)
  catturate con `ROCBLAS_LAYER=2` da un carico reale: per altri carichi vanno rimisurate con
  `misura_soluzioni_gemm.py`.

## Licenza

[MIT](LICENSE). Lo shim riproduce le firme delle API HIP di AMD (MIT, © Advanced Micro
Devices, Inc.); nessun binario AMD è incluso. Vedi [NOTICE](NOTICE).

---

<a name="english"></a>
# English

**rocm-marconi** is the source of the "Marconi" kit that extends the **ROCm/HIP 5.7**
runtime shipped with the AMD Adrenalin driver — the last one that still sees Vega GPUs on
Windows — so that modern software expecting HIP 6.x/7.x APIs runs on a **Radeon VII
(gfx906)**. The release package installs into a per-user folder, without administrator
rights, and does not modify the driver or the HIP SDK; only the opt-in developer tools in
`tensile/` and `strumenti/instrada_shape.py --applica` rewrite the HIP SDK's rocBLAS library.

**ABI translation:** 10 forwarded functions translate the types that changed between 5.7 and 7.x:
`hipMemoryType` was renumbered, `HIP_MEMCPY3D` went from 32- to 64-bit fields, and the
external-semaphore parameters moved. A systematic header comparison (50 structs, 470 enumerators)
shows no other ABI change. The shim also avoids a 5.7 crash in
`hipDrvPointerGetAttributes(MEMORY_TYPE)` on some unregistered host pointers.

**Code built with ROCm 7 and 10** (since 12 Nov 2026): the shim keeps the kernel registration
(`__hipRegisterFatBinary` and friends, `<<<...>>>` launches) itself and loads the code on first use
from clang bundles (also zstd-compressed) and ROCm 10 kpack archives; v6 and "sramecc on" code
objects are adapted for the driver's loader, and the `hidden_dynamic_lds_size` hidden argument,
unknown to 5.7, is completed at launch. AMD's PyTorch 2.15 wheels (ROCm 10.1) pass 12/12 checks
against fp64 CPU results on the Radeon VII (kpack kernels, rocBLAS, MIOpen forward convolution,
training, rocRAND, rocSOLVER, rocFFT). The wheel's MIOpen 10.1 still has the `ConvAsmBwdWrW3x3`
weight-gradient defect: set `MIOPEN_DEBUG_CONV_DIRECT_ASM_WRW3X3=0` and
`MIOPEN_DEBUG_CONV_IMPLICIT_GEMM_ASM_WRW_V4R1=0`, or use the fixed MIOpen of miopen-gfx906.

**Contents:** the `amdhip64_7_shim.dll` shim (557 exports: 553 `hip*`/`__hip*` APIs and 4
diagnostic `marconi_57exp_*` functions; 428/428 runtime 5.7 functions forwarded, the ROCm
6.1–7.1 API union exposed; over 150 implemented or translated in the shim — including, since
13 Nov 2026, the HIP 7 families the 5.7 runtime lacks: modules and libraries, kernels, JIT
linking, array/peer copies, generic graph nodes, stream batch mem-ops and green contexts —
and 23 returning an explicit `hipErrorNotSupported`, plus a null-stream bridge for
`hipStreamWaitEvent`). `hipGetDevicePropertiesR0600` returns fixed Radeon VII values
(including compute capability 7.0) for any device, without querying the driver. Also:
headers for modern HIP APIs, Tensile/rocBLAS library tools (index fixes, shape routing,
coherence audit: 42/42 complete) and documentation (Italian). Third-party binaries of the kit
(rocBLAS 5.7 with the corrected gfx906 library, hipRTC 5.7 with its comgr, clang/lld 21) are not
in git. MIOpen is a separate component (miopen-gfx906).

**Quick install:** download the self-extracting package from the
**[Releases](https://github.com/2s-2solution/rocm-marconi/releases)** page, check its
SHA256, extract it anywhere and run `installa.cmd` (installs to `%LOCALAPPDATA%\2S\marconi`,
sets the user environment, runs a GPU check).

**Build the shim:** `runtime\build_shim.cmd` (set `LLVM_BIN` to the folder with
`clang-cl.exe`); it compiles `src\shim_main.c` and the bundled zstd 1.5.7 decompressor
(`src\zstd`, BSD). Use `/O2`: clang-cl silently drops `-O3`.

**Known limits:** 35 functions return 801; zlib-compressed bundles are not supported (zstd
only); the driver rejects cooperative launches (719, also with code built with HIP 5.7), and the
shim reports `cooperativeLaunch = 0` like the driver; the kit's rocBLAS kernels for gfx900 (Vega 10) are
code object v6, which the Adrenalin driver does not load (`strumenti/code_object_tensile.py`); forwarders assume integer-only arguments; one
compute engine at a time under WDDM; a native HIP runtime for gfx906 cannot be built
because the Windows PAL layer no longer lists GFX9 devices; the tools in `strumenti/` find
the kit via `MARCONI_ROOT`.

**Used by:** [zluda-gfx906](https://github.com/2s-2solution/zluda-gfx906) (ZLUDA),
[miopen-gfx906](https://github.com/2s-2solution/miopen-gfx906) (MIOpen) and
[la-ruota](https://github.com/2s-2solution/la-ruota) (PyTorch for ROCm), each in its own repository.

**License:** MIT. HIP API signatures © Advanced Micro Devices, Inc. (MIT).
