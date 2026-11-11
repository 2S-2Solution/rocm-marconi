@echo off
REM Marconi 5.7 v2 - instrada le shape reali ai kernel migliori misurati (strumento di sviluppo).
REM Scrive nella libreria rocBLAS indicata (predefinita: quella dell'HIP SDK 5.7 in Program Files,
REM per cui servono i diritti di amministratore). Il pacchetto delle Releases non ne ha bisogno.
REM Variabili facoltative: PYTHON (python.exe con msgpack), LIBRERIA (cartella della libreria).
setlocal
if not defined PYTHON set "PYTHON=python"
if not defined LIBRERIA set "LIBRERIA=C:\Program Files\AMD\ROCm\5.7\bin\rocblas\library"
net session >nul 2>&1
if errorlevel 1 (
    echo Attenzione: senza diritti di amministratore la scrittura in Program Files non riesce.
)
"%PYTHON%" "%~dp0..\strumenti\instrada_shape.py" --libreria "%LIBRERIA%" --applica
pause
