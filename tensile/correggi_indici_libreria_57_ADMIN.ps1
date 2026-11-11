# Correzione degli indici dell'instradamento Tensile di agosto nella libreria rocBLAS di ROCm 5.7.
# 2 Solution, 20 ott 2026. DA LANCIARE COME AMMINISTRATORE (Program Files non e' scrivibile).
#
# instrada_shape.py di agosto scriveva in tabella l'indice di rocBLAS (indice Tensile + 1):
# ogni voce puntava alla soluzione SUCCESSIVA. Verificato: NN 2560x243x9728 fp16 6.9 -> 12.9 TFLOPS,
# TN 9728x330x2560 15.2 -> 16.7. I file corretti sono in v2\library_57_corretta.
# Per tornare indietro: rimettere i file *.bak_pre_correzione_indici_20ott al loro nome.

# Parametri: -Libreria (destinazione, predefinita la libreria dell'HIP SDK 5.7) e -Corretta (sorgente
# dei file corretti, predefinita la libreria del kit Marconi installato, che ha gia' gli indici giusti).
param(
  [string]$Libreria = "C:\Program Files\AMD\ROCm\5.7\bin\rocblas\library",
  [string]$Corretta = $(if ($env:MARCONI_ROOT) { Join-Path $env:MARCONI_ROOT "bin\rocblas\library" } else { "C:\rocm_5_7_expansion_pack\v2\library_57_corretta" })
)
$L = $Libreria
$N = $Corretta
if (-not (Test-Path $N)) { "ERRORE: cartella dei file corretti non trovata: $N"; exit 1 }
$nomi = @(
  "TensileLibrary_Type_HH_HPA_Contraction_l_Ailk_Bljk_Cijk_Dijk_gfx906.dat",
  "TensileLibrary_Type_HH_HPA_Contraction_l_Alik_Bljk_Cijk_Dijk_gfx906.dat",
  "TensileLibrary_Type_SS_Contraction_l_Ailk_Bljk_Cijk_Dijk_gfx906.dat",
  "TensileLibrary_Type_SS_Contraction_l_Alik_Bljk_Cijk_Dijk_gfx906.dat")
foreach ($f in $nomi) {
  $vivo = Join-Path $L $f
  $bak = "$f.bak_pre_correzione_indici_20ott"
  if (Test-Path (Join-Path $L $bak)) { "SALTATO (backup gia' presente): $f"; continue }
  Rename-Item -Path $vivo -NewName $bak
  Copy-Item -Path (Join-Path $N $f) -Destination $vivo
  $a = (Get-FileHash $vivo -Algorithm MD5).Hash; $b = (Get-FileHash (Join-Path $N $f) -Algorithm MD5).Hash
  if ($a -eq $b) { "OK  $f" } else { "ERRORE  $f" }
}
