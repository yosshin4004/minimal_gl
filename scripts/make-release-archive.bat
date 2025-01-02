@echo off && setlocal EnableDelayedExpansion

:
set /a _E =0 &&   set "_P=Build"
:
set /a _E+=1 &&   call "%~dp0build.bat" || goto :ERROR


:
set /a _E =0 &&   set "_P=Retrieve date, time and build-suffix"
:
set /a _E+=1 &&   cd /d "%~dp0"
set /a _E+=1 &&   cd ..
set /a _E+=1 &&   set /a errorno=1
set /a _E+=1 &&   for /F "delims=#" %%E in ('"prompt #$E# & for %%E in (1) do rem"') do set "_ESC=%%E"
set /a _E+=1 &&   for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set /a _E+=1 &&   set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set /a _E+=1 &&   set "Hour=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"
set /a _E+=1 &&   set "build_suffix=build_!YYYY!_!MM!_!DD!"
set /a _E+=1 &&   set "arcname=minimal_gl.!build_suffix!.zip"


:
set /a _E =0 &&   set "_P=Setup staging directory for release archive"
:
set /a _E+=1 &&   if exist staging ( rmdir /S /Q staging 2>nul )                      || goto :ERROR
set /a _E+=1 &&   mkdir staging\minimal_gl                                            || goto :ERROR
set /a _E+=1 &&   mkdir staging\minimal_gl\examples                                   || goto :ERROR
set /a _E+=1 &&   copy x64\Release\minimal_gl.exe staging\minimal_gl\                 || goto :ERROR
set /a _E+=1 &&   copy examples\*.*               staging\minimal_gl\examples         || goto :ERROR
set /a _E+=1 &&   ren staging\minimal_gl\minimal_gl.exe minimal_gl.!build_suffix!.exe || goto :ERROR


:
set /a _E =0 &&   set "_P=Create release archive with %windir%\tar.exe"
:
set /a _E+=1 &&   tar.exe -a -cf !arcname! -C staging minimal_gl || goto :ERROR


:
set /a _E =0 &&   set "_P=Delete staging directory"
:
set /a _E+=1 &&   rmdir /S /Q staging 2>nul || goto :ERROR


:
set /a _E =0 &&   set "_P=Clean"
:
set /a _E+=1 &&   call "%~dp0clean.bat" || goto :ERROR


echo !_ESC![2K %~n0 : Status =!_ESC![92m OK !_ESC![0m
set /a errorno=0
goto :END

:ERROR
echo !_ESC![2K %~n0 : Status =!_ESC![92m ERROR !_ESC![0m, _P=!_P!, _E=!_E!

:END
exit /B %errorno%
