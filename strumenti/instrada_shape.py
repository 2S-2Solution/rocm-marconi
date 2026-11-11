"""
Marconi 5.7 v2 - instrada le shape reali ai kernel migliori misurati.

PERCHE' SERVE. rocBLAS sceglie il kernel con una libreria "Matching":
  properties = [FreeSizeA[0]=M, FreeSizeB[0]=N, BoundSize[0]=K]
  table      = voci {key:[M,N,K], index:<soluzione>, speed:<misura>}
  distance   = Euclidea
Cerca la chiave piu' vicina e usa la sua soluzione. Il difetto e' la metrica:
su valori grezzi, con M e K nell'ordine delle migliaia e N nelle centinaia, il
contributo di N alla distanza e' trascurabile -> il dispatcher IGNORA di fatto
N, che e' proprio la dimensione che distingue una GEMM magra da una quadrata.

Misurato sulle shape di un carico reale (GEMM magre): per N=330 sceglieva una voce con N=32,
per N=243 una con N=4096. Nessuna corrispondenza esatta, distanze 795-5838.

RIMEDIO. Aggiungere voci con chiave ESATTA: distanza 0 batte qualunque altra,
quindi la selezione e' garantita. E' il meccanismo nativo, per shape, senza
variabili d'ambiente, valido per ogni processo.

INDICI. misura_soluzioni_gemm.py riporta il solution_index di rocBLAS, che e'
l'indice Tensile + 1. Nella tabella va l'indice Tensile: questo script toglie 1.
(La versione di agosto scriveva l'indice rocBLAS: ogni voce puntava alla
soluzione successiva. Corretto il 20 ott 2026; la libreria del pacchetto
Marconi ha gia' gli indici giusti, per esempio HH_HPA TN 9728x330x2560 -> 23255.)

Uso:  python instrada_shape.py [--libreria <cartella>] [--applica]
      (senza --applica: solo simulazione). Cartella predefinita: ROCBLAS_TENSILE_LIBPATH,
      altrimenti quella dell'HIP SDK 5.7. Scrivere in Program Files richiede l'amministratore.
"""
import msgpack, os, sys, shutil, math


def _libreria():
    if "--libreria" in sys.argv:
        return sys.argv[sys.argv.index("--libreria") + 1]
    return os.environ.get("ROCBLAS_TENSILE_LIBPATH", r"C:\Program Files\AMD\ROCm\5.7\bin\rocblas\library")


LIB = _libreria()

# trasposizione rocBLAS -> nome Tensile
TRASP = {"NN": "Ailk_Bljk", "NT": "Ailk_Bjlk", "TN": "Alik_Bljk", "TT": "Alik_Bjlk"}

# shape REALI catturate da un carico reale con ROCBLAS_LAYER=2, con la soluzione
# migliore misurata da misura_soluzioni_gemm.py (tutte validate per correttezza).
#  (tipo_tensile, trasposizione, M, N, K, indice_rocblas, tflops_misurati, peso_sul_calcolo)
VOCI = [
    ("HH_HPA", "TN", 9728, 330, 2560, 23256, 16.99, "23.0%"),
    ("HH_HPA", "NN", 2560, 243, 9728, 20471, 12.69, "16.9%"),
    ("HH_HPA", "TN", 2560, 330, 9728, 23256, 13.55, "11.5%"),
    ("HH_HPA", "NN", 9728, 243, 2560, 20479, 17.42, "8.5%"),
    ("HH_HPA", "TN", 4096, 330, 2560, 23252, 13.57, "4.8%"),
    ("HH_HPA", "TN", 2560, 330, 4096, 23332, 13.06, "4.8%"),
    ("SS",     "TN", 9728, 330, 2560, 23983, 10.53, "23.0%"),
    ("SS",     "NN", 2560, 243, 9728, 21945, 10.46, "16.9%"),
    ("SS",     "TN", 2560, 330, 9728, 24673, 11.02, "11.5%"),
    ("SS",     "NN", 9728, 243, 2560, 22206, 11.34, "8.5%"),
    ("SS",     "TN", 4096, 330, 2560, 24404, 9.38, "4.8%"),
    ("SS",     "TN", 2560, 330, 4096, 24673, 10.72, "4.8%"),
]


def percorso(tipo, trasp):
    return os.path.join(LIB, f"TensileLibrary_Type_{tipo}_Contraction_l_{TRASP[trasp]}_Cijk_Dijk_gfx906.dat")


def tabella_matching(d):
    """la libreria Matching, visitando ogni nodo una volta sola"""
    viste = set()
    trovate = []

    def vai(x):
        if id(x) in viste:
            return
        viste.add(id(x))
        if isinstance(x, dict):
            if x.get("type") == "Matching" and isinstance(x.get("table"), list):
                trovate.append(x)
            for v in x.values():
                vai(v)
        elif isinstance(x, list):
            for v in x:
                vai(v)

    vai(d.get("library"))
    return trovate[0] if trovate else None


def indici_validi(d):
    s = {x.get("index") for x in d.get("solutions", []) if isinstance(x, dict)}
    s.discard(None)
    return s or set(range(len(d.get("solutions", []))))


def main():
    applica = "--applica" in sys.argv
    print("MODALITA':", "APPLICA (scrive)" if applica else "SIMULAZIONE (non scrive)")
    print()

    # raggruppa per file
    per_file = {}
    for tipo, tr, M, N, K, idx, tf, peso in VOCI:
        per_file.setdefault((tipo, tr), []).append((M, N, K, idx, tf, peso))

    tot_agg = tot_upd = tot_no = 0
    for (tipo, tr), voci in sorted(per_file.items()):
        p = percorso(tipo, tr)
        nome = os.path.basename(p)
        if not os.path.exists(p):
            print(f"[{tipo} {tr}] file assente: {nome}")
            continue
        d = msgpack.unpack(open(p, "rb"), raw=False, strict_map_key=False)
        m = tabella_matching(d)
        if m is None:
            print(f"[{tipo} {tr}] nessuna tabella Matching: salto")
            continue
        validi = indici_validi(d)
        tab = m["table"]
        print(f"[{tipo} {tr}]  {len(tab)} voci esistenti, {len(validi)} soluzioni")

        for M, N, K, idx_rocblas, tf, peso in voci:
            idx = idx_rocblas - 1   # indice Tensile = solution_index di rocBLAS - 1
            if idx not in validi:
                print(f"    ({M},{N},{K}) -> idx {idx} NON ESISTE in questo file: SCARTATA")
                tot_no += 1
                continue
            # distanza attuale, per documentare il guadagno
            prima = min(tab, key=lambda v: sum((a - b) ** 2 for a, b in zip(v["key"][:3], (M, N, K))))
            dist = math.sqrt(sum((a - b) ** 2 for a, b in zip(prima["key"][:3], (M, N, K))))
            esistente = next((v for v in tab if tuple(v["key"][:3]) == (M, N, K)), None)
            if esistente is not None:
                print(f"    ({M},{N},{K}) chiave gia' presente idx {esistente['index']} -> aggiorno a {idx}")
                esistente["index"] = idx
                esistente["speed"] = tf
                tot_upd += 1
            else:
                tab.append({"key": [M, N, K], "index": idx, "speed": tf})
                print(f"    ({M},{N},{K}) peso {peso:>6}  AGGIUNTA -> idx {idx} ({tf:.2f} TFLOPS)"
                      f"   [prima sceglieva {tuple(prima['key'][:3])}, distanza {dist:.0f}]")
                tot_agg += 1

        if applica:
            bak = p + ".bak_pre_instradamento_28lug"
            if not os.path.exists(bak):
                shutil.copy2(p, bak)
            with open(p, "wb") as f:
                msgpack.pack(d, f, use_bin_type=True)
            print(f"    scritto ({len(tab)} voci). backup: {os.path.basename(bak)}")
        print()

    print("-" * 70)
    print(f"aggiunte {tot_agg} · aggiornate {tot_upd} · scartate {tot_no}")
    if not applica:
        print("\nSimulazione: nulla e' stato scritto. Rilancia con --applica per confermare.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
