@echo off && setlocal EnableDelayedExpansion
for /F "delims=#" %%E in ('"prompt #$E# & for %%E in (1) do rem"') do set "_ESC=%%E"

for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "Hour=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"
set _=set /a _LINE_NUMBER+=1
set /a _LINE_NUMBER =9
!_!
!_!&& call "%~dp0build.bat" || goto :ERROR
!_!
!_!&& if exist "artifacts" ( rmdir /S /Q artifacts 2>nul )
!_!&& mkdir     artifacts                                     || goto :ERROR
!_!&& mkdir     artifacts\examples                            || goto :ERROR
!_!&& copy x64\Release\minimal_gl.exe artifacts\              || goto :ERROR
!_!&& copy examples\*.*               artifacts\examples      || goto :ERROR
!_!
!_!&& set "TAG_NAME=%~1"
!_!&& if "!TAG_NAME!" == "" ( set "TAG_NAME=build_!YYYY!_!MM!_!DD!" )
!_!&& set "arcname=minimal_gl.!TAG_NAME!.zip"
!_!
!_!&& tar.exe -a -cf !arcname! -C artifacts minimal_gl.exe examples || goto :ERROR

echo !_ESC![2K %~n0 : Status =!_ESC![92m OK !_ESC![0m
set /a errorno=0
goto :END

:ERROR
echo !_ESC![2K %~n0 : Status =!_ESC![92m ERROR !_ESC![0m, Line Number=!_LINE_NUMBER!

:END
exit /B %errorno%
