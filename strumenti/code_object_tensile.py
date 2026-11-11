"""
Marconi 5.7 - versione e modalita' dei code object AMDGPU in una libreria Tensile/rocBLAS.

PERCHE' SERVE. Il driver Adrenalin per Windows carica i code object v4 e v5, non i v6
(e non i v2/v3). Un code object v5 con sramecc incompatibile con la scheda viene rifiutato
anche lui. Una libreria rocBLAS con kernel nella versione sbagliata fa cadere la prima GEMM
che li usa, senza un messaggio chiaro. Questo script legge le intestazioni ELF di ogni
code object (anche quelli impacchettati nei .co/.hsaco/.dat) e riassume:
  - la versione (EI_ABIVERSION: 1=v3, 2=v4, 3=v5, 4=v6);
  - l'architettura (e_flags & 0xff: 0x2f = gfx906);
  - la modalita' xnack e sramecc (e_flags, dalla v4 in poi: any / off / on).

Uso:  python code_object_tensile.py [--arch gfx906] [cartella ...]
      senza cartelle: %MARCONI_ROOT%\\bin\\rocblas\\library. L'esito guarda solo i code object
      dell'architettura indicata (predefinita gfx906): gli altri si elencano e basta.
Nessuna GPU coinvolta: legge solo i file.
"""
import os
import struct
import sys
from collections import Counter

VERSIONE = {0: "v2", 1: "v3", 2: "v4", 3: "v5", 4: "v6"}
ACCETTATE = {"v4", "v5"}                      # dal driver Adrenalin per Windows
MACH = {0x2c: "gfx900", 0x2d: "gfx902", 0x2f: "gfx906", 0x30: "gfx908", 0x3f: "gfx90a",
        0x36: "gfx1030", 0x41: "gfx1100", 0x46: "gfx1101", 0x47: "gfx1102"}
MODO = {0: "non gestito", 1: "any", 2: "off", 3: "on"}  # code object v4+


def code_object(data):
    """(versione, arch, xnack, sramecc) di ogni ELF AMDGPU HSA contenuto in data"""
    i = 0
    while True:
        i = data.find(b"\x7fELF", i)
        if i < 0 or i + 52 > len(data):
            return
        # ELF64 (EI_CLASS=2), OSABI HSA (64), e_machine AMDGPU (224)
        if data[i + 4] == 2 and data[i + 7] == 64 and struct.unpack_from("<H", data, i + 18)[0] == 224:
            ver = VERSIONE.get(data[i + 8], "abi%d" % data[i + 8])
            flags = struct.unpack_from("<I", data, i + 48)[0]
            arch = MACH.get(flags & 0xFF, "mach 0x%02x" % (flags & 0xFF))
            if data[i + 8] >= 2:
                xnack, sramecc = MODO[(flags >> 8) & 3], MODO[(flags >> 10) & 3]
            else:
                xnack = "on" if flags & 0x100 else "off"
                sramecc = "on" if flags & 0x200 else "off"
            yield ver, arch, xnack, sramecc
        i += 4


def esamina(cartella, arch_scheda):
    conta, esempio, file_letti = Counter(), {}, 0
    for radice, _, nomi in os.walk(cartella):
        for n in nomi:
            if not n.endswith((".co", ".hsaco", ".dat")):
                continue
            p = os.path.join(radice, n)
            with open(p, "rb") as f:
                data = f.read()
            file_letti += 1
            for chiave in code_object(data):
                conta[chiave] += 1
                esempio.setdefault(chiave, os.path.relpath(p, cartella))
    print(f"\n{cartella}")
    print(f"  file letti (.co .hsaco .dat): {file_letti}")
    if not conta:
        print("  nessun code object AMDGPU trovato")
        return True
    buona = True
    for (ver, arch, xnack, sramecc), c in sorted(conta.items()):
        if arch != arch_scheda:
            nota = "   (altra architettura)"
        else:
            nota = "" if ver in ACCETTATE else "   <- il driver Adrenalin NON lo carica"
            buona &= ver in ACCETTATE
        print(f"  {ver:4s} {arch:8s} xnack {xnack:11s} sramecc {sramecc:11s} {c:6d} oggetti"
              f"   (es. {esempio[(ver, arch, xnack, sramecc)]}){nota}")
    return buona


def main():
    argomenti = sys.argv[1:]
    arch_scheda = "gfx906"
    if "--arch" in argomenti:
        i = argomenti.index("--arch")
        arch_scheda = argomenti[i + 1]
        del argomenti[i:i + 2]
    cartelle = argomenti
    if not cartelle:
        m = os.environ.get("MARCONI_ROOT")
        if not m:
            print("Indicare una o piu' cartelle, oppure impostare MARCONI_ROOT (kit Marconi installato).")
            return 2
        cartelle = [os.path.join(m, "bin", "rocblas", "library")]
    tutte = True
    for c in cartelle:
        if not os.path.isdir(c):
            print(f"\n{c}: cartella non trovata")
            tutte = False
            continue
        tutte &= esamina(c, arch_scheda)
    print(f"\nESITO ({arch_scheda}):", "tutti i code object sono in una versione che il driver carica" if tutte
          else "ci sono code object che il driver non carica (o cartelle mancanti)")
    return 0 if tutte else 1


if __name__ == "__main__":
    sys.exit(main())
