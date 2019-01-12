echo off

if exist "%ProgramFiles(x86)%" goto is_x64
set path="%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE";%PATH%
set path="%ProgramFiles%\Windows Kits\8.1\bin\x86";%PATH%
goto start

:is_x64
set path="%ProgramFiles(x86)%\Microsoft Visual Studio 9.0\Common7\IDE";%PATH%
set path="%ProgramFiles(x86)%\Windows Kits\8.1\bin\x86";%PATH%

:start
rmdir /s /q build_xp
mkdir build_xp
ren Release Release_tmp
rmdir /s /q Release_tmp

devenv.com babbage2nd.vcproj /Rebuild Release
mkdir build_xp\babbage2nd
copy Release\babbage2nd.exe build_xp\babbage2nd\.

devenv.com bmjr.vcproj /Rebuild Release
mkdir build_xp\bmjr
copy Release\bmjr.exe build_xp\bmjr\.

devenv.com bubcom80.vcproj /Rebuild Release
mkdir build_xp\bubcom80
copy Release\bubcom80.exe build_xp\bubcom80\.

devenv.com colecovision.vcproj /Rebuild Release
mkdir build_xp\colecovision
copy Release\colecovision.exe build_xp\colecovision\.

devenv.com ex80.vcproj /Rebuild Release
mkdir build_xp\ex80
copy Release\ex80.exe build_xp\ex80\.

devenv.com familybasic.vcproj /Rebuild Release
mkdir build_xp\familybasic
copy Release\familybasic.exe build_xp\familybasic\.

devenv.com fm8.vcproj /Rebuild Release
mkdir build_xp\fm8
copy Release\fm8.exe build_xp\fm8\.

devenv.com fm7.vcproj /Rebuild Release
devenv.com fm77.vcproj /Rebuild Release
devenv.com fm77l4.vcproj /Rebuild Release
devenv.com fm77av.vcproj /Rebuild Release
mkdir build_xp\fm7
copy Release\fm7.exe build_xp\fm7\.
copy Release\fm77.exe build_xp\fm7\.
copy Release\fm77l4.exe build_xp\fm7\.
copy Release\fm77av.exe build_xp\fm7\.

devenv.com fm77av40.vcproj /Rebuild Release
devenv.com fm77av40ex.vcproj /Rebuild Release
mkdir build_xp\fm77av40
copy Release\fm77av40.exe build_xp\fm77av40\.
copy Release\fm77av40ex.exe build_xp\fm77av40\.

devenv.com fm16beta_i186.vcproj /Rebuild Release
mkdir build_xp\fm16beta
copy Release\fm16beta_i186.exe build_xp\fm16beta\.

devenv.com fm16pi.vcproj /Rebuild Release
mkdir build_xp\fm16pi
copy Release\fm16pi.exe build_xp\fm16pi\.

devenv.com fmr30_i86.vcproj /Rebuild Release
devenv.com fmr30_i286.vcproj /Rebuild Release
mkdir build_xp\fmr30
copy Release\fmr30_i86.exe build_xp\fmr30\.
copy Release\fmr30_i286.exe build_xp\fmr30\.

devenv.com fmr50_i286.vcproj /Rebuild Release
devenv.com fmr50_i386.vcproj /Rebuild Release
devenv.com fmr50_i486.vcproj /Rebuild Release
mkdir build_xp\fmr50
copy Release\fmr50_i286.exe build_xp\fmr50\.
copy Release\fmr50_i386.exe build_xp\fmr50\.
copy Release\fmr50_i486.exe build_xp\fmr50\.

devenv.com fmr60.vcproj /Rebuild Release
devenv.com fmr70.vcproj /Rebuild Release
devenv.com fmr80.vcproj /Rebuild Release
mkdir build_xp\fmr60
copy Release\fmr60.exe build_xp\fmr60\.
copy Release\fmr70.exe build_xp\fmr60\.
copy Release\fmr80.exe build_xp\fmr60\.

devenv.com fp200.vcproj /Rebuild Release
mkdir build_xp\fp200
copy Release\fp200.exe build_xp\fp200\.

devenv.com fp1100.vcproj /Rebuild Release
mkdir build_xp\fp1100
copy Release\fp1100.exe build_xp\fp1100\.

devenv.com gamegear.vcproj /Rebuild Release
mkdir build_xp\gamegear
copy Release\gamegear.exe build_xp\gamegear\.

devenv.com hc20.vcproj /Rebuild Release
mkdir build_xp\hc20
copy Release\hc20.exe build_xp\hc20\.

devenv.com hc40.vcproj /Rebuild Release
mkdir build_xp\hc40
copy Release\hc40.exe build_xp\hc40\.

devenv.com hc80.vcproj /Rebuild Release
mkdir build_xp\hc80
copy Release\hc80.exe build_xp\hc80\.

devenv.com jr100.vcproj /Rebuild Release
mkdir build_xp\jr100
copy Release\jr100.exe build_xp\jr100\.

devenv.com jr800.vcproj /Rebuild Release
mkdir build_xp\jr800
copy Release\jr800.exe build_xp\jr800\.

devenv.com jx.vcproj /Rebuild Release
mkdir build_xp\jx
copy Release\jx.exe build_xp\jx\.
mkdir build_xp\jx_hires
copy Release\jx.exe build_xp\jx_hires\.

devenv.com m5.vcproj /Rebuild Release
mkdir build_xp\m5
copy Release\m5.exe build_xp\m5\.

devenv.com map1010.vcproj /Rebuild Release
mkdir build_xp\map1010
copy Release\map1010.exe build_xp\map1010\.

devenv.com mastersystem.vcproj /Rebuild Release
mkdir build_xp\mastersystem
copy Release\mastersystem.exe build_xp\mastersystem\.

devenv.com msx1.vcproj /Rebuild Release
devenv.com msx2.vcproj /Rebuild Release
devenv.com msx2p.vcproj /Rebuild Release
devenv.com fsa1.vcproj /Rebuild Release
devenv.com hx20.vcproj /Rebuild Release
mkdir build_xp\msx
copy Release\msx1.exe build_xp\msx\.
copy Release\msx2.exe build_xp\msx\.
copy Release\msx2p.exe build_xp\msx\.
copy Release\fsa1.exe build_xp\msx\.
copy Release\hx20.exe build_xp\msx\.

devenv.com multi8.vcproj /Rebuild Release
mkdir build_xp\multi8
copy Release\multi8.exe build_xp\multi8\.

devenv.com mycomz80a.vcproj /Rebuild Release
mkdir build_xp\mycomz80a
copy Release\mycomz80a.exe build_xp\mycomz80a\.

devenv.com mz80a.vcproj /Rebuild Release
mkdir build_xp\mz80a
copy Release\mz80a.exe build_xp\mz80a\.

devenv.com mz80b.vcproj /Rebuild Release
mkdir build_xp\mz80b
copy Release\mz80b.exe build_xp\mz80b\.

devenv.com mz80k.vcproj /Rebuild Release
mkdir build_xp\mz80k
copy Release\mz80k.exe build_xp\mz80k\.

devenv.com mz700.vcproj /Rebuild Release
mkdir build_xp\mz700
copy Release\mz700.exe build_xp\mz700\.

devenv.com mz800.vcproj /Rebuild Release
mkdir build_xp\mz800
copy Release\mz800.exe build_xp\mz800\.

devenv.com mz1200.vcproj /Rebuild Release
mkdir build_xp\mz1200
copy Release\mz1200.exe build_xp\mz1200\.

devenv.com mz1500.vcproj /Rebuild Release
mkdir build_xp\mz1500
copy Release\mz1500.exe build_xp\mz1500\.

devenv.com mz2200.vcproj /Rebuild Release
mkdir build_xp\mz2200
copy Release\mz2200.exe build_xp\mz2200\.

devenv.com mz2500.vcproj /Rebuild Release
mkdir build_xp\mz2500
copy Release\mz2500.exe build_xp\mz2500\.

devenv.com mz2800.vcproj /Rebuild Release
mkdir build_xp\mz2800
copy Release\mz2800.exe build_xp\mz2800\.

devenv.com mz3500.vcproj /Rebuild Release
mkdir build_xp\mz3500
copy Release\mz3500.exe build_xp\mz3500\.

devenv.com mz5500.vcproj /Rebuild Release
mkdir build_xp\mz5500
copy Release\mz5500.exe build_xp\mz5500\.

devenv.com mz6500.vcproj /Rebuild Release
mkdir build_xp\mz6500
copy Release\mz6500.exe build_xp\mz6500\.

devenv.com pasopia.vcproj /Rebuild Release
mkdir build_xp\pasopia
copy Release\pasopia.exe build_xp\pasopia\.

devenv.com pasopia7.vcproj /Rebuild Release
devenv.com pasopia7lcd.vcproj /Rebuild Release
mkdir build_xp\pasopia7
copy Release\pasopia7.exe build_xp\pasopia7\.
copy Release\pasopia7lcd.exe build_xp\pasopia7\.

devenv.com pc2001.vcproj /Rebuild Release
mkdir build_xp\pc2001
copy Release\pc2001.exe build_xp\pc2001\.

devenv.com pc6001.vcproj /Rebuild Release
devenv.com pc6001mk2.vcproj /Rebuild Release
devenv.com pc6001mk2sr.vcproj /Rebuild Release
devenv.com pc6601.vcproj /Rebuild Release
devenv.com pc6601sr.vcproj /Rebuild Release
mkdir build_xp\pc6001
copy Release\pc6001.exe build_xp\pc6001\.
copy Release\pc6001mk2.exe build_xp\pc6001\.
copy Release\pc6001mk2sr.exe build_xp\pc6001\.
copy Release\pc6601.exe build_xp\pc6001\.
copy Release\pc6601sr.exe build_xp\pc6001\.

devenv.com pc8001.vcproj /Rebuild Release
devenv.com pc8001mk2.vcproj /Rebuild Release
devenv.com pc8001mk2sr.vcproj /Rebuild Release
mkdir build_xp\pc8001
copy Release\pc8001.exe build_xp\pc8001\.
copy Release\pc8001mk2.exe build_xp\pc8001\.
copy Release\pc8001mk2sr.exe build_xp\pc8001\.

devenv.com pc8201.vcproj /Rebuild Release
mkdir build_xp\pc8201
copy Release\pc8201.exe build_xp\pc8201\.

devenv.com pc8201a.vcproj /Rebuild Release
mkdir build_xp\pc8201a
copy Release\pc8201a.exe build_xp\pc8201a\.

devenv.com pc8801.vcproj /Rebuild Release
devenv.com pc8801mk2.vcproj /Rebuild Release
mkdir build_xp\pc8801
copy Release\pc8801.exe build_xp\pc8801\.
copy Release\pc8801mk2.exe build_xp\pc8801\.

devenv.com pc8801ma.vcproj /Rebuild Release
mkdir build_xp\pc8801ma
copy Release\pc8801ma.exe build_xp\pc8801ma\.

devenv.com pc9801.vcproj /Rebuild Release
mkdir build_xp\pc9801
copy Release\pc9801.exe build_xp\pc9801\.

devenv.com pc9801e.vcproj /Rebuild Release
mkdir build_xp\pc9801e
copy Release\pc9801e.exe build_xp\pc9801e\.

devenv.com pc9801u.vcproj /Rebuild Release
devenv.com pc9801vf.vcproj /Rebuild Release
devenv.com pc9801vm.vcproj /Rebuild Release
mkdir build_xp\pc9801vm
copy Release\pc9801u.exe build_xp\pc9801vm\.
copy Release\pc9801vf.exe build_xp\pc9801vm\.
copy Release\pc9801vm.exe build_xp\pc9801vm\.

devenv.com pc9801vx.vcproj /Rebuild Release
mkdir build_xp\pc9801vx
copy Release\pc9801vx.exe build_xp\pc9801vx\.

devenv.com pc9801ra.vcproj /Rebuild Release
mkdir build_xp\pc9801ra
copy Release\pc9801ra.exe build_xp\pc9801ra\.

devenv.com pc98rl.vcproj /Rebuild Release
mkdir build_xp\pc98rl
copy Release\pc98rl.exe build_xp\pc98rl\.

devenv.com pc98xa.vcproj /Rebuild Release
mkdir build_xp\pc98xa
copy Release\pc98xa.exe build_xp\pc98xa\.

devenv.com pc98xl.vcproj /Rebuild Release
mkdir build_xp\pc98xl
copy Release\pc98xl.exe build_xp\pc98xl\.

devenv.com pc98do.vcproj /Rebuild Release
mkdir build_xp\pc98do
copy Release\pc98do.exe build_xp\pc98do\.

devenv.com pc98ha.vcproj /Rebuild Release
mkdir build_xp\pc98ha
copy Release\pc98ha.exe build_xp\pc98ha\.

devenv.com pc98lt.vcproj /Rebuild Release
mkdir build_xp\pc98lt
copy Release\pc98lt.exe build_xp\pc98lt\.

devenv.com pc100.vcproj /Rebuild Release
mkdir build_xp\pc100
copy Release\pc100.exe build_xp\pc100\.

devenv.com pcengine.vcproj /Rebuild Release
mkdir build_xp\pcengine
copy Release\pcengine.exe build_xp\pcengine\.

devenv.com phc20.vcproj /Rebuild Release
mkdir build_xp\phc20
copy Release\phc20.exe build_xp\phc20\.

devenv.com phc25.vcproj /Rebuild Release
mkdir build_xp\phc25
copy Release\phc25.exe build_xp\phc25\.

devenv.com pv1000.vcproj /Rebuild Release
mkdir build_xp\pv1000
copy Release\pv1000.exe build_xp\pv1000\.

devenv.com pv2000.vcproj /Rebuild Release
mkdir build_xp\pv2000
copy Release\pv2000.exe build_xp\pv2000\.

devenv.com px7.vcproj /Rebuild Release
mkdir build_xp\px7
copy Release\px7.exe build_xp\px7\.

devenv.com pyuta.vcproj /Rebuild Release
mkdir build_xp\pyuta
copy Release\pyuta.exe build_xp\pyuta\.
mkdir build_xp\pyuta_jr
copy Release\pyuta.exe build_xp\pyuta_jr\.

devenv.com qc10.vcproj /Rebuild Release
devenv.com qc10cms.vcproj /Rebuild Release
mkdir build_xp\qc10
copy Release\qc10.exe build_xp\qc10\.
copy Release\qc10cms.exe build_xp\qc10\.

devenv.com rx78.vcproj /Rebuild Release
mkdir build_xp\rx78
copy Release\rx78.exe build_xp\rx78\.

devenv.com sc3000.vcproj /Rebuild Release
mkdir build_xp\sc3000
copy Release\sc3000.exe build_xp\sc3000\.

devenv.com scv.vcproj /Rebuild Release
mkdir build_xp\scv
copy Release\scv.exe build_xp\scv\.

devenv.com smb80te.vcproj /Rebuild Release
mkdir build_xp\smb80te
copy Release\smb80te.exe build_xp\smb80te\.

devenv.com smc70.vcproj /Rebuild Release
devenv.com smc777.vcproj /Rebuild Release
mkdir build_xp\smc777
copy Release\smc70.exe build_xp\smc777\.
copy Release\smc777.exe build_xp\smc777\.

devenv.com tk80bs.vcproj /Rebuild Release
mkdir build_xp\tk80bs
copy Release\tk80bs.exe build_xp\tk80bs\.

devenv.com tk85.vcproj /Rebuild Release
mkdir build_xp\tk85
copy Release\tk85.exe build_xp\tk85\.

devenv.com x07.vcproj /Rebuild Release
mkdir build_xp\x07
copy Release\x07.exe build_xp\x07\.

devenv.com x1.vcproj /Rebuild Release
devenv.com x1twin.vcproj /Rebuild Release
devenv.com x1turbo.vcproj /Rebuild Release
devenv.com x1turboz.vcproj /Rebuild Release
mkdir build_xp\x1
copy Release\x1.exe build_xp\x1\.
copy Release\x1twin.exe build_xp\x1\.
copy Release\x1turbo.exe build_xp\x1\.
copy Release\x1turboz.exe build_xp\x1\.

devenv.com yalky.vcproj /Rebuild Release
mkdir build_xp\yalky
copy Release\yalky.exe build_xp\yalky\.

devenv.com yis.vcproj /Rebuild Release
mkdir build_xp\yis
copy Release\yis.exe build_xp\yis\.

devenv.com ys6464a.vcproj /Rebuild Release
mkdir build_xp\ys6464a
copy Release\ys6464a.exe build_xp\ys6464a\.

devenv.com z80tvgame_i8255.vcproj /Rebuild Release
devenv.com z80tvgame_z80pio.vcproj /Rebuild Release
mkdir build_xp\z80tvgame
copy Release\z80tvgame_i8255.exe build_xp\z80tvgame\.
copy Release\z80tvgame_z80pio.exe build_xp\z80tvgame\.

rmdir /s /q binary_xp
mkdir binary_xp
copy Release\*.exe binary_xp\.
rmdir /s /q binary_vista
mkdir binary_vista
copy Release\*.exe binary_vista\.
rmdir /s /q build_vista
xcopy /e /y build_xp build_vista\
rmdir /s /q Release

pushd binary_vista
for /r %%i in (*.exe) do mt.exe /manifest ..\..\src\res\vista.manifest -outputresource:%%i;1
popd
pushd build_vista
for /r %%i in (*.exe) do mt.exe /manifest ..\..\src\res\vista.manifest -outputresource:%%i;1
popd

pause
echo on
