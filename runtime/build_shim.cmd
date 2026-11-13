@echo off
REM build_shim.cmd - compila amdhip64_7_shim.dll da src\shim_main.c con clang in modalita' cl.
REM 2 Solution. Compila la copia del repository in cui si trova questo file (non il kit installato)
REM e scrive in questa cartella amdhip64_7_shim.dll e la libreria d'importazione amdhip64_7_shim.lib.
REM
REM Compilatore, nell'ordine:
REM   1. LLVM_BIN\clang-cl.exe, se LLVM_BIN e' impostata;
REM   2. %MARCONI_ROOT%\compiler\bin\clang.exe --driver-mode=cl: il clang 21 del pacchetto Marconi.
REM      E' quello con cui e' stata compilata la DLL pubblicata: il codice esce identico;
REM   3. C:\rocm_5_7_expansion_pack\compiler\bin\clang-cl.exe (macchina di sviluppo, stesso clang 21);
REM   4. C:\Program Files\LLVM\bin\clang-cl.exe (un LLVM recente da' codice diverso, non validato).
REM VCVARS: percorso di vcvarsall.bat (predefinito Visual Studio 2022 Build Tools; servono
REM kernel32.lib, il Windows SDK e link.exe).
REM Attenzione: con clang-cl serve /O2. Con -O3 l'opzione viene scartata e resta -O0, con un solo avviso.

setlocal
cd /d "%~dp0"

set "CC="
if defined LLVM_BIN set "CC="%LLVM_BIN%\clang-cl.exe""
if not defined CC if defined MARCONI_ROOT if exist "%MARCONI_ROOT%\compiler\bin\clang.exe" set "CC="%MARCONI_ROOT%\compiler\bin\clang.exe" --driver-mode=cl"
if not defined CC if exist "C:\rocm_5_7_expansion_pack\compiler\bin\clang-cl.exe" set "CC="C:\rocm_5_7_expansion_pack\compiler\bin\clang-cl.exe""
if not defined CC if exist "C:\Program Files\LLVM\bin\clang-cl.exe" set "CC="C:\Program Files\LLVM\bin\clang-cl.exe""
if not defined CC (
    echo *** compilatore non trovato: installare il kit Marconi oppure impostare LLVM_BIN ***
    exit /b 1
)
if not defined VCVARS set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

call "%VCVARS%" x64 >nul
if errorlevel 1 (
    echo *** impostazione dell'ambiente MSVC non riuscita: impostare VCVARS ***
    exit /b 1
)

echo === Build amdhip64_7_shim.dll con %CC% ===
%CC% /LD /O2 /MD /W2 ^
    /DZSTD_DISABLE_ASM /DZSTD_LEGACY_SUPPORT=0 /DXXH_NAMESPACE=ZSTD_ ^
    src\shim_main.c ^
    src\zstd\common\debug.c src\zstd\common\entropy_common.c src\zstd\common\error_private.c ^
    src\zstd\common\fse_decompress.c src\zstd\common\xxhash.c src\zstd\common\zstd_common.c ^
    src\zstd\decompress\huf_decompress.c src\zstd\decompress\zstd_ddict.c ^
    src\zstd\decompress\zstd_decompress.c src\zstd\decompress\zstd_decompress_block.c ^
    /Fe:amdhip64_7_shim.dll ^
    /link kernel32.lib /implib:amdhip64_7_shim.lib

set RC=%ERRORLEVEL%
echo.
if %RC% EQU 0 (
    echo *** BUILD OK ***
    dir amdhip64_7_shim.dll amdhip64_7_shim.lib
    echo.
    echo === Export ===
    dumpbin /nologo /exports amdhip64_7_shim.dll | "%SystemRoot%\System32\findstr.exe" /R /C:" hip" /C:" __hip" /C:" marconi_" | "%SystemRoot%\System32\find.exe" /c /v ""
    echo export hip e marconi contati qui sopra: attesi 539
) else (
    echo *** BUILD FAIL RC=%RC% ***
)

exit /b %RC%
