
@echo off

setlocal

if exist "%VS100COMNTOOLS%vsvars32.bat" (
    call "%VS100COMNTOOLS%vsvars32.bat" 
)else (
    call "%VS90COMNTOOLS%vsvars32.bat"
)

set COMPILER=cl /nologo /MD /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE /I "../zlib"
set LIBER=lib /nologo

%COMPILER% *.c
%LIBER% /out:../libpng.lib *.obj
del *.obj

endlocal