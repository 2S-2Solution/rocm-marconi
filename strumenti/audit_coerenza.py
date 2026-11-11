"""
Marconi 5.7 v2 — audit di coerenza della libreria Tensile gfx906.

Per ogni combinazione tipo-dato/trasposizione verifica che le soluzioni
referenziate dalla logica di dispatch (.dat) esistano davvero fra i kernel
compilati (.hsaco/.co). Una copertura inferiore al 100% significa che rocBLAS,
per quelle shape, cerca un kernel che non c'e'.

Nessuna GPU coinvolta: legge solo i file.

Uso:  python audit_coerenza.py [percorso_libreria]
      (default: la libreria di produzione ROCm 5.7)
"""
import msgpack, re, os, sys, glob
from collections import defaultdict

PRODUZIONE = r"C:\Program Files\AMD\ROCm\5.7\bin\rocblas\library"


def kernel_nel_file(percorso):
    """Nomi dei kernel Tensile contenuti in un code object."""
    if not os.path.exists(percorso):
        return set()
    with open(percorso, "rb") as f:
        blob = f.read()
    nomi = set()
    for m in re.finditer(rb"Cijk_[A-Za-z0-9_]{20,600}", blob):
        s = m.group(0).decode("ascii", "ignore").replace(".kd", "")
        if s.endswith("_preloaded"):
            s = s[: -len("_preloaded")]
        nomi.add(s)
    return nomi


def soluzioni_nel_dat(percorso):
    """Nomi delle soluzioni referenziate da una logica di dispatch."""
    try:
        with open(percorso, "rb") as f:
            d = msgpack.unpack(f, raw=False, strict_map_key=False)
        return {s["name"] for s in d.get("solutions", [])
                if isinstance(s, dict) and s.get("name")}
    except Exception:
        return set()


def audit(lib):
    tipi = sorted({
        os.path.basename(p).replace("TensileLibrary_Type_", "").rsplit("_gfx906", 1)[0]
        for p in glob.glob(os.path.join(lib, "TensileLibrary_Type_*_gfx906.dat"))
    })
    if not tipi:
        print(f"ERRORE: nessuna logica gfx906 trovata in {lib}")
        return 1

    print(f"Libreria: {lib}")
    print(f"File totali: {len(os.listdir(lib))}   tipi/trasposizioni gfx906: {len(tipi)}\n")
    print(f"{'tipo / trasposizione':<44}{'arch':>8}{'fallback':>10}")
    print("-" * 62)

    scoperti = []
    for t in tipi:
        pool = (kernel_nel_file(os.path.join(lib, f"TensileLibrary_Type_{t}_fallback_gfx906-xnack-.hsaco"))
                | kernel_nel_file(os.path.join(lib, f"TensileLibrary_Type_{t}_gfx906.co"))
                | kernel_nel_file(os.path.join(lib, f"TensileLibrary_Type_{t}_gfx906-xnack-.hsaco")))

        def copertura(dat):
            req = soluzioni_nel_dat(os.path.join(lib, dat))
            if not req:
                return None
            return 100.0 * len(req & pool) / len(req)

        c_arch = copertura(f"TensileLibrary_Type_{t}_gfx906.dat")
        c_fall = copertura(f"TensileLibrary_Type_{t}_fallback.dat")
        fmt = lambda c: "n/d" if c is None else f"{c:.0f}%"
        nome = t.replace("_Contraction_l_", " ").replace("_Cijk_Dijk", "")
        marca = ""
        if (c_arch is not None and c_arch < 100) or (c_fall is not None and c_fall < 100):
            marca = "   <-- INCOMPLETO"
            scoperti.append((nome, fmt(c_arch), fmt(c_fall)))
        print(f"{nome:<44}{fmt(c_arch):>8}{fmt(c_fall):>10}{marca}")

    print("-" * 62)
    if scoperti:
        print(f"ESITO: {len(scoperti)}/{len(tipi)} combinazioni INCOMPLETE\n")
        for n, a, f in scoperti:
            print(f"  - {n}: arch={a} fallback={f}")
        return 1
    print(f"ESITO: {len(tipi)}/{len(tipi)} combinazioni complete al 100%. Libreria coerente.")
    return 0


if __name__ == "__main__":
    sys.exit(audit(sys.argv[1] if len(sys.argv) > 1 else PRODUZIONE))
