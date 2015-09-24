echo off
if exist "%ProgramFiles(x86)%" goto is_x64
set path="%ProgramFiles%\Windows Kits\8.1\bin\x86";%PATH%
goto start
:is_x64
set path="%ProgramFiles(x86)%\Windows Kits\8.1\bin\x86";%PATH%
:start
rmdir /s /q binary_vista
xcopy /e /y binary_xp binary_vista\
rmdir /s /q build_vista
xcopy /e /y build_xp build_vista\
pause

pushd binary_vista
for /r %%i in (*.exe) do mt.exe /manifest ..\src\res\vista.manifest -outputresource:%%i;1
popd
pushd build_vista
for /r %%i in (*.exe) do mt.exe /manifest ..\src\res\vista.manifest -outputresource:%%i;1
popd

pause
echo on
