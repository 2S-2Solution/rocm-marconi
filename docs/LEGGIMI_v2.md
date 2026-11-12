# Marconi 5.7 Expansion Pack — v2

> **Documento storico** (luglio 2026, macchina di sviluppo), conservato per i fatti tecnici
> che contiene. Cosa è cambiato da allora:
> - il pacchetto delle Releases porta la libreria rocBLAS corretta nel proprio
>   `bin\rocblas\library` e **non modifica** `C:\Program Files\AMD\ROCm\5.7`; sulla macchina
>   di sviluppo, invece, la v2 è stata installata proprio in Program Files (vedi sotto);
> - gli indici dell'instradamento sono stati corretti il 20 ottobre 2026 (erano spostati di
>   uno: vedi `strumenti/instrada_shape.py`);
> - lo shim oggi (12 nov 2026) ha **484 export**: 474 API `hip*`, 6 interne `__hip*`, 4 `marconi_57exp_*`;
>   la registrazione dei kernel (`__hipRegisterFatBinary` e simili) non è più vuota: vedi il README
>   e `docs/FUNZIONI_473.md`;
> - gli script citati in «Installazione» (`installa_v2.cmd`, `ripristina_v2.cmd`) non fanno parte del
>   repository; due voci su moduli PyTorch che il documento originale aveva in «Cosa resta
>   aperto» sono state tolte, perché non riguardano il kit.

**Radeon VII · Vega 20 · gfx906 · Windows**
Preparata il 28 luglio 2026. Stato: **INSTALLATA E VERIFICATA IN PRODUZIONE.**
Libreria rocBLAS: audit **42/42 al 100%** su
`C:\Program Files\AMD\ROCm\5.7\bin\rocblas\library`.
Shim: **v5.1, 473 export**, copertura **428/428** della runtime v5.7 e
**469/469** dell'unione ROCm 6.1+6.2+6.4+7.1.

---

## Cos'è la v2

La v1 del pack aggiungeva alla ROCm 5.7 il compilatore clang 21, le device_libs
moderne, lo shim delle API HIP 7 e le librerie math. Tutto questo resta valido e
non è stato toccato.

La v2 aggiunge la cosa che mancava: **una libreria di kernel rocBLAS gfx906
coerente**, cioè in cui ogni soluzione che la logica di dispatch può scegliere
esiste davvero fra i kernel compilati. Più gli strumenti per dimostrarlo.

---

## Il difetto corretto

Il 16 giugno 2026 un file di kernel è stato sovrascritto nella libreria di
produzione, nella convinzione di installare un asset da 17,84 TFLOPS. L'audit di
oggi ha stabilito che:

- il file installato conteneva **126 kernel che nessuna logica referenzia**: erano
  lo stesso insieme dei kernel già presenti nel code object di architettura,
  rinominati da una versione più recente di Tensile (`DTLA0_DTLB0`, `GLVWA8_GLVWB8`,
  `VWB8` invece di `DTL0`, `GRVW8`, `WSGRA0`). Non aggiungevano alcuna capacità;
- il file sovrascritto conteneva invece i **12 kernel generici** del percorso di
  fallback universale per il tipo `HH_HPA` trasposizione `Alik_Bljk` (fp16 con
  accumulo fp32, trasposizione TT);
- di conseguenza quel percorso di fallback è rimasto con **copertura 0%**: per le
  shape che vi ricadono, rocBLAS cerca un kernel che non esiste.

Su 42 combinazioni tipo-dato/trasposizione, 41 erano integre e **una sola era
scoperta**. La v2 ripristina il file originale.

**Verifica indipendente**: la libreria v2 è risultata **byte per byte identica**
al backup del'11 giugno (`C:\ZLUDA\backup_rocm_5_7_T18_00\library`), cioè allo
stato precedente all'errore. Ripristina esattamente il buono, nulla di più.

| | copertura |
|---|---|
| produzione attuale | **1 combinazione su 42 incompleta** (`HH_HPA Alik_Bljk`, fallback 0%) |
| libreria v2 | **42 su 42 complete al 100%** |

---

## Shim v5.0 — copertura completa della runtime (28 luglio)

**Direttiva:**: *"non si possono aggiungere funzioni alla bisogno, devono
essere tutte funzionanti ed esposte, così si ha un lavoro pulito."*

Lo shim esponeva **176** delle **428** funzioni `hip*` presenti in
`System32\amdhip64.dll`: le altre 252 non erano perdite, ma andavano aggiunte
una alla volta quando qualcuno le chiedeva. Ora sono esposte tutte.

| | prima | dopo |
|---|---|---|
| export `hip*` | 176 | **433** |
| funzioni della runtime non esposte | 252 | **0** |
| dimensione | 48.128 byte | 102.912 byte |

**Difetto latente corretto nel farlo**: `hipGetErrorName` e `hipGetErrorString`
usavano la macro `FWD9`, che dichiara il ritorno come `int` e quindi
**troncava a 32 bit il puntatore `const char*` restituito**. Chi le avesse usate
avrebbe ricevuto un puntatore corrotto. Introdotta la macro `FWDP` che
restituisce `void*` preservando l'intero registro RAX; usata per tutti i nuovi
forwarder e per queste due. Serve anche a `hipApiName`, `hipKernelNameRef`,
`hipGetCmdName` e a `hipCreateChannelDesc`, che restituisce una struct per
valore (su x64 tramite puntatore nascosto). Per chi restituisce `hipError_t`
resta corretta: il chiamante legge EAX.

**Collaudo**: catena di preload documentata, 8 DLL su 8 caricate **inclusa
`MIOpen.dll`** (era la regressione storica temuta con la v3.8) ·
`hipGetDeviceCount` rc=0 count=1 · `hipGetErrorString(801)` ora restituisce
davvero `"operation not supported"`.

Implementazioni custom intatte: `hipGetDevicePropertiesR0600` (conversione
struct v5.7→R0600 con valori Vega 20) e `__hipRegisterFatBinary` (che NON va
forwardato alla v5.7, altrimenti "Cannot Register fat binary cov5").

Backup: `runtime\src\shim_main.c.bak_pre_completo_28lug` ·
`bin\amdhip64_7_shim.dll.bak_v40_176export_28lug` (e gemello per `amdhip64_7.dll`).

## Shim v5.1 — superficie API chiusa (28 luglio)

Le **40 funzioni** che esistono nelle ROCm 6.1→7.1 ma **non nella runtime che
esegue** ora sono tutte esportate. Non si risolvevano con un forwarder — nella
`amdhip64.dll` di sistema non esistono — quindi:

- **5 implementate fedelmente**, dove la runtime ha i mattoni:
  `hipTexRefGetMipMappedArray` (pura differenza di maiuscola rispetto a
  `hipTexRefGetMipmappedArray` della 5.7: stessa funzione) ·
  `hipGetProcAddress` (cerca prima in questo shim, poi nella runtime) ·
  `hipSetValidDevices` · `hipMemcpyAtoHAsync` e `hipMemcpyHtoAAsync`
  (delegano alle sincrone: sincronizzazione più forte del richiesto, lecita).
- **35 esportate con `hipErrorNotSupported` (801) esplicito**: nodi grafo
  generici, semafori esterni nei grafi, batch mem-op, API driver dei grafi,
  copie fra array, varianti di struct `R0000`/`R0600`. Per implementarle
  davvero servirebbe il layout esatto di struct che la 5.7 non definisce:
  **inventarlo corromperebbe la memoria**, quindi si dichiara il limite invece
  di fingere.

**Il criterio adottato**: meglio un errore dichiarato che un simbolo mancante
al link o un forwarder che mente. Ogni simbolo esiste, è chiamabile, e dice la
verità su cosa sa fare.

| copertura | risultato |
|---|---|
| runtime System32 v5.7 | **428 / 428** |
| unione ROCm 6.1 + 6.2 + 6.4 + 7.1 | **469 / 469** |
| export totali | **473** |

**Collaudo v5.1**: catena DLL completa caricata inclusa `MIOpen.dll` ·
`hipGetDeviceCount` rc=0 count=1 · `hipGetProcAddress("hipMalloc")` rc=0
stato=0 con puntatore valido · `hipGetProcAddress("hipNonEsiste")` rc=801
stato=1 · `hipSetValidDevices([5,0])` rc=0 (scarta il 5 inesistente, sceglie
lo 0) · `hipGraphAddNode(...)` → 801 esplicito.

Corretto anche `marconi_57exp_count_resolved_pfns`, che restituiva la costante
433 e avrebbe mentito al primo cambio di superficie; aggiunta
`marconi_57exp_count_exported_symbols` col nome onesto.

**Vincolo implicito delle macro, ora documentato nel sorgente**: `FWD9`/`FWDP`
passano sempre 9 argomenti `void*`. Regge perché nessuna API HIP supera 9
parametri interi, ma **si romperebbe in silenzio** con una funzione che prenda
`float`/`double` per valore, che viaggiano nei registri XMM e non nei GPR.

Backup: `runtime\src\shim_main.c.bak_pre_v51_28lug`.

---

## Perché la strada TheRock è chiusa (e perché il lavoro veniva ripreso)

Marconi nasce per portare le funzionalità della 6.4 sulla 5.7. La via maestra
sarebbe stata *costruire* la runtime HIP nuova per gfx906 con TheRock, non
shimmarla. Quella strada è **definitivamente chiusa**, e questa è la prova:

1. La `amdhip64_7.dll` per gfx906 **è già stata costruita** (10 giugno,
   20,7 MB, `build_gfx906\core\clr\dist\bin\`): **541 funzioni `hip*`**, più
   della 6.4, e contiene tutto ciò che ci manca.
2. Ma in `clr/rocclr/device/pal/paldevice.cpp` la tabella `supportedPalDevices[]`
   **comincia da `gfx1010`**: l'intera generazione GFX9 non c'è.
3. E non è un controllo da rimuovere: l'enum `Pal::AsicRevision` nelle
   intestazioni dell'interop Windows parte da `Navi10 = 0x1F` e **non contiene
   alcun simbolo Vega**. La libreria binaria PAL ha **zero** occorrenze di
   vega/gfx90x.

Non è un cancello software: è l'hardware rimosso dallo strato di astrazione del
driver Windows, che AMD distribuisce in forma binaria chiusa. Nessun codice
nostro può aggirarlo.

**Questo spiega perché il lavoro è stato ripreso e abbandonato più volte**: la
build riesce, i binari si producono, e poi la GPU "non viene rilevata" senza
causa evidente. Resta teoricamente aperta la sola via dei sorgenti **ROCm 6.4**,
dove gfx906 è ancora nella tabella PAL (verificato tag per tag: presente fino
alla 6.4.4, sparisce nella 7.0.0) — servirebbe un albero sorgenti diverso col
suo interop, che in questo workspace non c'è.

**Conclusione: lo shim non è un ripiego, è l'unica strada — ed è ora completo.**

---

## Fatti verificati sul ferro

Tutto quanto segue è stato ottenuto disassemblando i binari presenti su questa
macchina, senza usare la GPU.

**Il moltiplicatore fp16 dipende da un solo parametro Tensile, `LocalDotLayout`.**
La correlazione è totale, senza eccezioni, su 126 kernel esaminati:

| `LDL` | kernel | istruzioni emesse |
|---|---|---|
| 1 | 99 | `v_fma_mix_f32` = 171.904 · **`v_dot2` = 0** |
| 2 | 27 | **`v_dot2_f32_f16` = 49.744** · `fma_mix` = 2.208 (sola coda) |

A parità di macro-tile e thread-tile il rapporto è due a uno: `MT128x128x16 TT8_8`
con `LDL1` emette 4.288 moltiplicazioni-accumulo scalari, con `LDL2` ne emette
2.112 packed — lo stesso lavoro nella metà delle istruzioni.

**I tetti che ne discendono** (picco della scheda: 26,9 TFLOPS a 1725 MHz,
27,7 a 1801 MHz):

| percorso | istruzione | tetto |
|---|---|---|
| fp16, accumulo fp32, `LDL=1` | `v_fma_mix_f32` | **13,4 TFLOPS** (rate fp32) |
| fp16, accumulo fp32, `LDL=2` | `v_dot2_f32_f16` | **~21 TFLOPS** |
| fp16, accumulo fp16 | `v_pk_fma_f16` | **~27 TFLOPS** |
| bf16 | **emulato**: `v_fmac_f32` + shift/maschera | sotto i 13,4, metà slot VALU spesi in conversione |

**I kernel packed sono già presenti e raggiungibili**: nel code object di
architettura ci sono **14 kernel `LDL=2` con macro-tile ≥ 64 e thread-tile 8×8**,
di cui 11 con `MT128x128`. Sono le due soglie fisiche che contano: sotto
macro-tile 64 una GEMM fp16 diventa memory-bound, sotto thread-tile 8×8 la banda
LDS satura.

**Il bf16 non esiste in hardware su gfx906.** Nei suoi kernel non c'è nessuna
istruzione bf16 nativa: solo `v_fmac_f32` (5.556 occorrenze) più 5.348 fra
`v_lshlrev_b32` e `v_and_b32`, cioè la conversione fatta a mano nel ciclo caldo.
È un'assenza di silicio, arrivata con gfx908; nessuna libreria può colmarla.

---

## Correzioni al sedimento

1. **L'asset "17,84 TFLOPS" non è mai stato in produzione.** I 126 kernel
   installati il 16 giugno sono orfani: nessuna logica li referenzia. La misura
   di 17,5 TFLOPS sulle shape FFN veniva dai kernel stock di AMD, che su quel
   percorso selezionano già una soluzione `LDL=2`.
2. **La strada `V_DOT2` non è un vicolo cieco.** È un parametro documentato e
   supportato. L'ostacolo storico era il default `LocalDotLayout: [1]` di Tensile
   (`Common.py:1511`), che fa generare solo kernel bloccati a 13,4 TFLOPS.
3. **Il vincolo `WGM=1` vale solo per il lancio manuale via ctypes**, dove il
   kernarg costruito a mano non ha i campi `NumFullBlocks`/`WgmRemainder1`.
   Passando per rocBLAS il vincolo non si applica: la logica upstream usa `WGM=8`
   in 93 soluzioni su 152.
4. **Il timore che lo shim v4.0 rompesse il caricamento di MIOpen non si applica
   al binario in uso.** Un processo in esecuzione ha caricato
   `MIOpen.dll` (469 MB) attraverso `amdhip64_7_shim.dll` da 48.128 byte con
   ~176 export: prova diretta che funziona.
5. **Il `README.md` del pack dichiarava ancora la v1.0.0** con "shim 10,7 KB,
   11 exports" e le API a placeholder. Superato di due generazioni.

---

## Installazione

*(Storico: procedura della macchina di sviluppo; gli script non sono nel repository. Per
installare oggi si usa il pacchetto delle Releases.)*

**Prerequisito assoluto: la Radeon VII deve essere libera.** rocBLAS carica i
kernel da questa cartella in modo pigro, durante l'esecuzione: sostituirli
mentre un programma li usa può farlo cadere. Lo script controlla la presenza di
processi `python.exe` e chiede conferma esplicita.

```
installa_v2.cmd        (come amministratore)
```

Fa il backup della libreria attuale in `_backup_originali\library_pre_v2`, copia
i soli file differenti, e rilancia l'audit di coerenza per confermare 42/42.

Per annullare:

```
ripristina_v2.cmd      (come amministratore)
```

Backup alternativo sempre disponibile: `C:\ZLUDA\backup_rocm_5_7_T18_00\library`.

---

## Strumenti

`strumenti\audit_coerenza.py [libreria]` — per ogni combinazione tipo/trasposizione
verifica che le soluzioni referenziate dalla logica esistano fra i kernel
compilati. Esce con codice 1 se trova una combinazione incompleta.

`strumenti\audit_ldl.py [libreria] [tipo]` — disassembla i kernel, li segmenta per
simbolo e correla `LocalDotLayout` alle istruzioni realmente emesse. Riporta
quanti kernel packed con tile utilizzabile sono presenti.

Entrambi girano su CPU, non toccano la GPU, e richiedono `msgpack`
(`pip install msgpack`) e `llvm-objdump` da ROCm 7.1.

---

## Cosa resta aperto

- **Misurare gli 11 candidati `LDL=2` con `MT128x128` sul cubo 4096³**, con
  `rocblas-bench.exe`, direttamente su rocBLAS. Oggi quelle shape ricadono su kernel
  `LDL=1` e prendono il tetto da 13,4: si misurano 9,7 TFLOPS, il 72% di quel
  tetto. La forbice attesa passando a `LDL=2` è 13-17 TFLOPS. Richiede la GPU.
- **Verificare il clock sostenuto sotto carico**: su Windows manca `rocm-smi`,
  e le MI50 sono documentate girare spesso con power limit a 225 W. Se il clock
  reale è 1450 MHz e non 1801, il picco è 22,5 e non 27,7, e ogni percentuale
  va ricalcolata.

---

*Bottega italiana sovrana. Nessuna modifica a `System32\amdhip64.dll` o al driver. La v2
è rimasta in staging finché non è stata installata esplicitamente; installata, ha
sostituito (con backup) i file della libreria rocBLAS in `C:\Program Files\AMD\ROCm\5.7\`.*
