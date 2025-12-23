@echo off && setlocal EnableDelayedExpansion
cd /d "%~dp0"
cd ..

for /F "delims=#" %%E in ('"prompt #$E# & for %%E in (1) do rem"') do set "_ESC=%%E"

rmdir /s /q x64 2>nul
rmdir /s /q .vs 2>nul
rmdir /s /q minimal_gl 2>nul
rmdir /s /q artifacts 2>nul

del *.zip 2>nul
del *.vcxproj.user 2>nul

echo %_ESC%[2K %~n0 : Status =%_ESC%[92m OK %_ESC%[0m
