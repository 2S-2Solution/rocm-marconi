"""
Marconi 5.7 v2 - confronto delle API HIP fra le versioni ROCm installate.

Domanda a cui risponde: passando dalla 5.7 alle versioni successive, quali
funzioni sono state aggiunte, e quali di queste NON sono raggiungibili dal
nostro stack (runtime System32 v5.7 + shim Marconi)?

Distingue tre casi, che hanno rimedi diversi:
  A) funzione presente nella runtime System32 ma non nello shim
     -> si aggiunge un forwarder allo shim: costo minuti
  B) funzione assente dalla runtime System32
     -> non esiste nel runtime che esegue: serve implementazione user-space
  C) funzione presente ovunque -> nessun problema

Nessuna GPU coinvolta: legge solo le tabelle di export dei PE.
"""
import re, subprocess, os, sys

OBJDUMP = r"C:\Program Files\AMD\ROCm\7.1\bin\llvm-objdump.exe"

FONTI = [
    ("ROCm 6.1", r"C:\Program Files\AMD\ROCm\6.1\bin\amdhip64_6.dll"),
    ("ROCm 6.2", r"C:\Program Files\AMD\ROCm\6.2\bin\amdhip64_6.dll"),
    ("ROCm 6.4", r"C:\Program Files\AMD\ROCm\6.4\bin\amdhip64_6.dll"),
    ("ROCm 7.1", r"C:\Program Files\AMD\ROCm\7.1\bin\amdhip64_7.dll"),
]
RUNTIME = ("System32 (v5.7, ESEGUE)", r"C:\Windows\System32\amdhip64.dll")
# Kit Marconi installato: MARCONI_ROOT (pacchetto delle Releases), altrimenti la cartella storica.
MARCONI = os.environ.get("MARCONI_ROOT", r"C:\rocm_5_7_expansion_pack")
SHIM = ("shim Marconi", os.path.join(MARCONI, "bin", "amdhip64_7_shim.dll"))


def esporta(percorso):
    """nomi delle funzioni hip* esportate da un PE"""
    if not os.path.exists(percorso):
        return None
    out = subprocess.run([OBJDUMP, "--private-headers", percorso],
                         capture_output=True, text=True, errors="ignore").stdout
    # nella tabella Export Directory i nomi compaiono come voci isolate
    return set(re.findall(r"\bhip[A-Za-z0-9_]+", out))


def main():
    print("Estrazione tabelle di export...\n")
    runtime = esporta(RUNTIME[1])
    shim = esporta(SHIM[1])
    if runtime is None:
        print("runtime System32 non leggibile"); return 1

    print(f"{'sorgente':<28}{'funzioni hip*':>14}")
    print("-" * 44)
    print(f"{RUNTIME[0]:<28}{len(runtime):>14}")
    print(f"{SHIM[0]:<28}{len(shim) if shim else 0:>14}")
    versioni = []
    for nome, p in FONTI:
        s = esporta(p)
        if s is None:
            print(f"{nome:<28}{'assente':>14}")
            continue
        versioni.append((nome, s))
        print(f"{nome:<28}{len(s):>14}")

    print("\n" + "=" * 78)
    print("NOVITA' PER VERSIONE, e loro raggiungibilita' dal nostro stack")
    print("=" * 78)

    precedente = None
    for nome, s in versioni:
        if precedente is None:
            nuove = s - runtime
            eti = f"{nome} rispetto alla runtime 5.7"
        else:
            nuove = s - precedente[1]
            eti = f"{nome} rispetto a {precedente[0]}"
        mancanti_runtime = nuove - runtime
        coperte_runtime = nuove & runtime
        print(f"\n### {eti}: {len(nuove)} funzioni nuove")
        if coperte_runtime:
            print(f"  gia' nella runtime System32 (caso A, forwarder): {len(coperte_runtime)}")
        if mancanti_runtime:
            print(f"  ASSENTI dalla runtime System32 (caso B): {len(mancanti_runtime)}")
            for f in sorted(mancanti_runtime)[:24]:
                print(f"      {f}")
            if len(mancanti_runtime) > 24:
                print(f"      ... e altre {len(mancanti_runtime)-24}")
        precedente = (nome, s)

    print("\n" + "=" * 78)
    print("VERDETTO COMPLESSIVO")
    print("=" * 78)
    unione = set()
    for _, s in versioni:
        unione |= s
    perse = unione - runtime
    print(f"funzioni hip* esistenti in 6.1/6.2/6.4/7.1 (unione) : {len(unione)}")
    print(f"  di cui presenti nella runtime System32 v5.7        : {len(unione & runtime)}")
    print(f"  di cui ASSENTI dalla runtime che esegue            : {len(perse)}")
    if shim:
        colmabili = (runtime - shim)
        print(f"\npresenti nella runtime ma non ancora nello shim      : {len(colmabili)}")
        print("  (caso A: si colmano con un forwarder, non serve implementare nulla)")
    print(f"\nfunzioni esclusive della runtime System32 v5.7        : {len(runtime - unione)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
