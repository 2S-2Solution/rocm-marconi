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
| `runtime/src/shim_main.c`, `runtime/build_shim.cmd` | lo shim **`amdhip64_7_shim.dll`** (installato anche come `amdhip64_7.dll`): **482 export**, cioè **473 API `hip*`**, 5 funzioni interne del compilatore (`__hipRegisterFatBinary`, `__hipRegisterFunction`, `__hipUnregisterFatBinary`, `__hipPushCallConfiguration`, `__hipPopCallConfiguration`) e 4 funzioni diagnostiche `marconi_57exp_*`. Le 473 API coprono **428/428** funzioni della runtime 5.7 e **469/469** dell'unione ROCm 6.1+6.2+6.4+7.1 |
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

- **428 funzioni** vengono inoltrate alla runtime 5.7 del driver (`System32\amdhip64.dll`).
- **10 funzioni** sono implementate dentro lo shim (elenco in
  [FUNZIONI_473.md](docs/FUNZIONI_473.md), stato «OK propria»): `hipGetProcAddress`,
  `hipSetValidDevices`, `hipTexRefGetMipMappedArray`, `hipMemcpyAtoHAsync`,
  `hipMemcpyHtoAAsync`, `hipExtGetLastError`, `hipDrvLaunchKernelEx`, `hipGraphAddNode_v2`,
  `hipDrvGraphAddMemcpyNode` e `hipGetDevicePropertiesR0600`.
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
| copertura unione ROCm 6.1–7.1 | 469 / 469 (473 API `hip*`, 482 export in tutto) |
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

`build_shim.cmd` compila `runtime\src\shim_main.c` della copia del repository e scrive in
`runtime\` `amdhip64_7_shim.dll` e `amdhip64_7_shim.lib`. Come compilatore usa, nell'ordine:
`LLVM_BIN\clang-cl.exe` se `LLVM_BIN` è impostata, poi il clang 21 del kit Marconi installato
(`%MARCONI_ROOT%\compiler\bin\clang.exe --driver-mode=cl`), poi `C:\Program Files\LLVM\bin`.
Con il clang del kit il codice esce **identico** a quello della DLL pubblicata (verificato
confrontando il disassemblato); un LLVM più recente (22) dà codice diverso, non validato.
Equivale a, da un prompt x64 di Visual Studio Build Tools 2022:

```bat
cd runtime
clang-cl /LD /O2 /MD /W2 src\shim_main.c /Fe:amdhip64_7_shim.dll /link kernel32.lib /implib:amdhip64_7_shim.lib
```

Per usarla, copiare la DLL nel `bin` del kit installato (`%MARCONI_ROOT%\bin`) **sia** come
`amdhip64_7_shim.dll` **sia** come `amdhip64_7.dll`. ⚠️ Con clang-cl usare **`/O2`**:
`-O3` viene scartato in silenzio e la build resta non ottimizzata. Dopo la copia
verificare l'impronta del file: una DLL in uso non si sovrascrive e l'errore è muto.
`amdhip64_7_shim.lib` serve per compilare [zluda-gfx906](https://github.com/2s-2solution/zluda-gfx906).

## Limiti noti

- Le 35 funzioni elencate come **NO** in [FUNZIONI_473.md](docs/FUNZIONI_473.md)
  (nodi grafo generici, semafori esterni nei grafi, varianti di strutture `R0000`/`R0600`…)
  rispondono 801.
- `__hipRegisterFatBinary`, `__hipRegisterFunction` e `__hipUnregisterFatBinary` sono
  **vuote**: restituiscono un handle finto e non registrano nulla. Servono perché le librerie
  costruite con ROCm 7 (per esempio hipBLASLt di TheRock) pre-registrano code object v5 che
  la runtime 5.7 rifiuta, facendo cadere il processo. Di conseguenza un programma che usa lo
  shim come runtime (cioè è collegato ad `amdhip64_7.dll`) e lancia kernel incorporati con
  `<<<...>>>` non li trova registrati: attraverso lo shim i kernel vanno caricati
  esplicitamente con `hipModuleLoad*`/`hipModuleGetFunction`, come fanno MIOpen e ZLUDA.
  I programmi collegati direttamente alla runtime del driver (`amdhip64.dll`) non passano
  dallo shim e non hanno questo limite.
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

**Contents:** the `amdhip64_7_shim.dll` shim (482 exports: 473 `hip*` APIs, 5 internal
`__hip*` compiler entry points and 4 diagnostic `marconi_57exp_*` functions; 428/428 runtime
5.7 functions forwarded, 469/469 of the ROCm 6.1–7.1 API union exposed; 10 implemented in the
shim, 35 returning an explicit `hipErrorNotSupported`, plus a null-stream bridge for
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
`clang-cl.exe`), or `clang-cl /LD /O2 /MD src\shim_main.c /Fe:amdhip64_7_shim.dll /link
kernel32.lib /implib:amdhip64_7_shim.lib` (use `/O2`: clang-cl silently drops `-O3`).

**Known limits:** 35 functions return 801; the kit's rocBLAS kernels for gfx900 (Vega 10) are
code object v6, which the Adrenalin driver does not load (`strumenti/code_object_tensile.py`); forwarders assume integer-only arguments; one
compute engine at a time under WDDM; a native HIP runtime for gfx906 cannot be built
because the Windows PAL layer no longer lists GFX9 devices; the tools in `strumenti/` find
the kit via `MARCONI_ROOT`.

**Used by:** [zluda-gfx906](https://github.com/2s-2solution/zluda-gfx906) (ZLUDA),
[miopen-gfx906](https://github.com/2s-2solution/miopen-gfx906) (MIOpen) and
[la-ruota](https://github.com/2s-2solution/la-ruota) (PyTorch for ROCm), each in its own repository.

**License:** MIT. HIP API signatures © Advanced Micro Devices, Inc. (MIT).
