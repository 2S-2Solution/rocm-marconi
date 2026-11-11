"""
Marconi 5.7 v2 — audit LocalDotLayout: quali kernel usano la matematica packed.

Su gfx906 il moltiplicatore fp16 dipende da un solo parametro Tensile:

    LDL=1  ->  v_fma_mix_f32     1 MAC per lane per ciclo   tetto ~13,4 TFLOPS
    LDL=2  ->  v_dot2_f32_f16    2 MAC per lane per ciclo   tetto ~21 TFLOPS
    HPA=no ->  v_pk_fma_f16      2 MAC, ma accumulo fp16    tetto ~27 TFLOPS

Questo strumento disassembla i kernel, li segmenta per simbolo, e correla il
parametro dichiarato nel nome con le istruzioni realmente emesse. Serve a
verificare che i kernel packed siano presenti e raggiungibili.

Nessuna GPU coinvolta.

Uso:  python audit_ldl.py [percorso_libreria] [tipo]
"""
import re, subprocess, os, sys
from collections import defaultdict

OBJDUMP = r"C:\Program Files\AMD\ROCm\7.1\bin\llvm-objdump.exe"
PRODUZIONE = r"C:\Program Files\AMD\ROCm\5.7\bin\rocblas\library"
TIPO_DEFAULT = "HH_HPA_Contraction_l_Alik_Bljk_Cijk_Dijk"

# soglie fisiche: sotto queste il kernel non e' utile su GEMM grandi
MT_MINIMO = 64      # macro-tile: sotto 64 la GEMM fp16 diventa memory-bound
TT_UTILE = "8_8"    # thread-tile: sotto 8x8 la banda LDS satura


def per_kernel(percorso):
    if not os.path.exists(percorso):
        return {}
    out = subprocess.run([OBJDUMP, "-d", "--mcpu=gfx906", percorso],
                         capture_output=True, text=True, errors="ignore").stdout
    risultato, corrente = {}, None
    for riga in out.splitlines():
        m = re.match(r"^[0-9a-f]+ <(Cijk_[^>]+)>:", riga)
        if m:
            corrente = m.group(1)
            risultato[corrente] = defaultdict(int)
            continue
        if corrente is None:
            continue
        if re.search(r"\bv_dot2_f32_f16", riga):
            risultato[corrente]["dot2"] += 1
        elif re.search(r"\bv_pk_fma_f16", riga):
            risultato[corrente]["pk_fma"] += 1
        elif re.search(r"\bv_(fma|mad)_mix", riga):
            risultato[corrente]["fma_mix"] += 1
        elif re.search(r"\bv_(fma|fmac|mac)_f32", riga):
            risultato[corrente]["scalare_f32"] += 1
    return risultato


def campo(nome, pattern, default="?"):
    m = re.search(pattern, nome)
    return m.group(1) if m else default


def analizza(lib, tipo):
    file_da_leggere = [
        (f"TensileLibrary_Type_{tipo}_gfx906.co", "code object di architettura"),
        (f"TensileLibrary_Type_{tipo}_gfx906-xnack-.hsaco", "code object xnack-"),
        (f"TensileLibrary_Type_{tipo}_fallback_gfx906-xnack-.hsaco", "fallback universale"),
    ]
    print(f"Libreria: {lib}")
    print(f"Tipo    : {tipo}\n")

    usabili_totali = 0
    for nomefile, etichetta in file_da_leggere:
        kern = per_kernel(os.path.join(lib, nomefile))
        if not kern:
            print(f"### {etichetta}: assente\n")
            continue
        agg, conta = defaultdict(lambda: defaultdict(int)), defaultdict(int)
        usabili = []
        for nome, c in kern.items():
            ldl = campo(nome, r"_LDL(\d+)_")
            conta[ldl] += 1
            for k, v in c.items():
                agg[ldl][k] += v
            mt = campo(nome, r"_MT(\d+)x(\d+)x\d+_", None)
            m = re.search(r"_MT(\d+)x(\d+)x(\d+)_", nome)
            tt = campo(nome, r"_TT(\d+_\d+)_")
            if m and int(m.group(1)) >= MT_MINIMO and int(m.group(2)) >= MT_MINIMO \
               and tt == TT_UTILE and c.get("dot2", 0) > 0:
                usabili.append(f"MT{m.group(1)}x{m.group(2)}x{m.group(3)}")

        print(f"### {etichetta}  ({len(kern)} kernel)")
        for ldl in sorted(agg):
            dettaglio = "  ".join(f"{k}={v}" for k, v in sorted(agg[ldl].items()))
            print(f"    LDL={ldl}  {conta[ldl]:>3} kernel   {dettaglio}")
        if usabili:
            from collections import Counter
            riep = "  ".join(f"{t}x{n}" for t, n in sorted(Counter(usabili).items()))
            print(f"    packed utilizzabili su GEMM grandi (MT>={MT_MINIMO}, TT{TT_UTILE}): "
                  f"{len(usabili)}  ->  {riep}")
            usabili_totali += len(usabili)
        print()

    print("-" * 62)
    if usabili_totali:
        print(f"ESITO: {usabili_totali} kernel packed (v_dot2_f32_f16) con tile grande "
              f"presenti e raggiungibili.")
    else:
        print("ESITO: NESSUN kernel packed con tile grande. Il percorso fp16 e' "
              "limitato al rate fp32 (~13,4 TFLOPS).")
    return 0


if __name__ == "__main__":
    lib = sys.argv[1] if len(sys.argv) > 1 else PRODUZIONE
    tipo = sys.argv[2] if len(sys.argv) > 2 else TIPO_DEFAULT
    sys.exit(analizza(lib, tipo))
