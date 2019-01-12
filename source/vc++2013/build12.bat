echo off

if exist "%ProgramFiles(x86)%" goto is_x64
set path="%ProgramFiles%\Microsoft Visual Studio 12.0\Common7\IDE";%PATH%
goto start

:is_x64
set path="%ProgramFiles(x86)%\Microsoft Visual Studio 12.0\Common7\IDE";%PATH%

:start
rmdir /s /q binary_vc12
mkdir binary_vc12
rmdir /s /q build_vc12
mkdir build_vc12
ren Release Release_tmp
rmdir /s /q Release_tmp

devenv.com babbage2nd.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\babbage2nd
copy binary_vc12\babbage2nd.exe build_vc12\babbage2nd\.

devenv.com bmjr.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\bmjr
copy binary_vc12\bmjr.exe build_vc12\bmjr\.

devenv.com bubcom80.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\bubcom80
copy binary_vc12\bubcom80.exe build_vc12\bubcom80\.

devenv.com colecovision.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\colecovision
copy binary_vc12\colecovision.exe build_vc12\colecovision\.

devenv.com ex80.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\ex80
copy binary_vc12\ex80.exe build_vc12\ex80\.

devenv.com familybasic.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\familybasic
copy binary_vc12\familybasic.exe build_vc12\familybasic\.

devenv.com fm8.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\fm8
copy binary_vc12\fm8.exe build_vc12\fm8\.

devenv.com fm7.vcxproj /Rebuild Release
call :clean
devenv.com fm77.vcxproj /Rebuild Release
call :clean
devenv.com fm77l4.vcxproj /Rebuild Release
call :clean
devenv.com fm77av.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\fm7
copy binary_vc12\fm7.exe build_vc12\fm7\.
copy binary_vc12\fm77.exe build_vc12\fm7\.
copy binary_vc12\fm77l4.exe build_vc12\fm7\.
copy binary_vc12\fm77av.exe build_vc12\fm7\.

devenv.com fm77av40.vcxproj /Rebuild Release
call :clean
devenv.com fm77av40ex.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\fm77av40
copy binary_vc12\fm77av40.exe build_vc12\fm77av40\.
copy binary_vc12\fm77av40ex.exe build_vc12\fm77av40\.

devenv.com fm16beta_i186.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\fm16beta
copy binary_vc12\fm16beta_i186.exe build_vc12\fm16beta\.

devenv.com fm16pi.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\fm16pi
copy binary_vc12\fm16pi.exe build_vc12\fm16pi\.

devenv.com fmr30_i86.vcxproj /Rebuild Release
call :clean
devenv.com fmr30_i286.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\fmr30
copy binary_vc12\fmr30_i86.exe build_vc12\fmr30\.
copy binary_vc12\fmr30_i286.exe build_vc12\fmr30\.

devenv.com fmr50_i286.vcxproj /Rebuild Release
call :clean
devenv.com fmr50_i386.vcxproj /Rebuild Release
call :clean
devenv.com fmr50_i486.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\fmr50
copy binary_vc12\fmr50_i286.exe build_vc12\fmr50\.
copy binary_vc12\fmr50_i386.exe build_vc12\fmr50\.
copy binary_vc12\fmr50_i486.exe build_vc12\fmr50\.

devenv.com fmr60.vcxproj /Rebuild Release
call :clean
devenv.com fmr70.vcxproj /Rebuild Release
call :clean
devenv.com fmr80.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\fmr60
copy binary_vc12\fmr60.exe build_vc12\fmr60\.
copy binary_vc12\fmr70.exe build_vc12\fmr60\.
copy binary_vc12\fmr80.exe build_vc12\fmr60\.

devenv.com fp200.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\fp200
copy binary_vc12\fp200.exe build_vc12\fp200\.

devenv.com fp1100.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\fp1100
copy binary_vc12\fp1100.exe build_vc12\fp1100\.

devenv.com gamegear.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\gamegear
copy binary_vc12\gamegear.exe build_vc12\gamegear\.

devenv.com hc20.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\hc20
copy binary_vc12\hc20.exe build_vc12\hc20\.

devenv.com hc40.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\hc40
copy binary_vc12\hc40.exe build_vc12\hc40\.

devenv.com hc80.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\hc80
copy binary_vc12\hc80.exe build_vc12\hc80\.

devenv.com jr100.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\jr100
copy binary_vc12\jr100.exe build_vc12\jr100\.

devenv.com jr800.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\jr800
copy binary_vc12\jr800.exe build_vc12\jr800\.

devenv.com jx.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\jx
copy binary_vc12\jx.exe build_vc12\jx\.
mkdir build_vc12\jx_hires
copy binary_vc12\jx.exe build_vc12\jx_hires\.

devenv.com m5.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\m5
copy binary_vc12\m5.exe build_vc12\m5\.

devenv.com map1010.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\map1010
copy binary_vc12\map1010.exe build_vc12\map1010\.

devenv.com mastersystem.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mastersystem
copy binary_vc12\mastersystem.exe build_vc12\mastersystem\.

devenv.com msx1.vcxproj /Rebuild Release
call :clean
devenv.com msx2.vcxproj /Rebuild Release
call :clean
devenv.com msx2p.vcxproj /Rebuild Release
call :clean
devenv.com fsa1.vcxproj /Rebuild Release
call :clean
devenv.com hx20.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\msx
copy binary_vc12\msx1.exe build_vc12\msx\.
copy binary_vc12\msx2.exe build_vc12\msx\.
copy binary_vc12\msx2p.exe build_vc12\msx\.
copy binary_vc12\fsa1.exe build_vc12\msx\.
copy binary_vc12\hx20.exe build_vc12\msx\.

devenv.com multi8.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\multi8
copy binary_vc12\multi8.exe build_vc12\multi8\.

devenv.com mycomz80a.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mycomz80a
copy binary_vc12\mycomz80a.exe build_vc12\mycomz80a\.

devenv.com mz80a.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz80a
copy binary_vc12\mz80a.exe build_vc12\mz80a\.

devenv.com mz80b.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz80b
copy binary_vc12\mz80b.exe build_vc12\mz80b\.

devenv.com mz80k.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz80k
copy binary_vc12\mz80k.exe build_vc12\mz80k\.

devenv.com mz700.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz700
copy binary_vc12\mz700.exe build_vc12\mz700\.

devenv.com mz800.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz800
copy binary_vc12\mz800.exe build_vc12\mz800\.

devenv.com mz1200.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz1200
copy binary_vc12\mz1200.exe build_vc12\mz1200\.

devenv.com mz1500.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz1500
copy binary_vc12\mz1500.exe build_vc12\mz1500\.

devenv.com mz2200.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz2200
copy binary_vc12\mz2200.exe build_vc12\mz2200\.

devenv.com mz2500.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz2500
copy binary_vc12\mz2500.exe build_vc12\mz2500\.

devenv.com mz2800.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz2800
copy binary_vc12\mz2800.exe build_vc12\mz2800\.

devenv.com mz3500.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz3500
copy binary_vc12\mz3500.exe build_vc12\mz3500\.

devenv.com mz5500.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz5500
copy binary_vc12\mz5500.exe build_vc12\mz5500\.

devenv.com mz6500.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\mz6500
copy binary_vc12\mz6500.exe build_vc12\mz6500\.

devenv.com pasopia.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pasopia
copy binary_vc12\pasopia.exe build_vc12\pasopia\.

devenv.com pasopia7.vcxproj /Rebuild Release
call :clean
devenv.com pasopia7lcd.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pasopia7
copy binary_vc12\pasopia7.exe build_vc12\pasopia7\.
copy binary_vc12\pasopia7lcd.exe build_vc12\pasopia7\.

devenv.com pc2001.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc2001
copy binary_vc12\pc2001.exe build_vc12\pc2001\.

devenv.com pc6001.vcxproj /Rebuild Release
call :clean
devenv.com pc6001mk2.vcxproj /Rebuild Release
call :clean
devenv.com pc6001mk2sr.vcxproj /Rebuild Release
call :clean
devenv.com pc6601.vcxproj /Rebuild Release
call :clean
devenv.com pc6601sr.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc6001
copy binary_vc12\pc6001.exe build_vc12\pc6001\.
copy binary_vc12\pc6001mk2.exe build_vc12\pc6001\.
copy binary_vc12\pc6001mk2sr.exe build_vc12\pc6001\.
copy binary_vc12\pc6601.exe build_vc12\pc6001\.
copy binary_vc12\pc6601sr.exe build_vc12\pc6001\.

devenv.com pc8001.vcxproj /Rebuild Release
call :clean
devenv.com pc8001mk2.vcxproj /Rebuild Release
call :clean
devenv.com pc8001mk2sr.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc8001
copy binary_vc12\pc8001.exe build_vc12\pc8001\.
copy binary_vc12\pc8001mk2.exe build_vc12\pc8001\.
copy binary_vc12\pc8001mk2sr.exe build_vc12\pc8001\.

devenv.com pc8201.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc8201
copy binary_vc12\pc8201.exe build_vc12\pc8201\.

devenv.com pc8201a.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc8201a
copy binary_vc12\pc8201a.exe build_vc12\pc8201a\.

devenv.com pc8801.vcxproj /Rebuild Release
call :clean
devenv.com pc8801mk2.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc8801
copy binary_vc12\pc8801.exe build_vc12\pc8801\.
copy binary_vc12\pc8801mk2.exe build_vc12\pc8801\.

devenv.com pc8801ma.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc8801ma
copy binary_vc12\pc8801ma.exe build_vc12\pc8801ma\.

devenv.com pc9801.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc9801
copy binary_vc12\pc9801.exe build_vc12\pc9801\.

devenv.com pc9801e.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc9801e
copy binary_vc12\pc9801e.exe build_vc12\pc9801e\.

devenv.com pc9801u.vcxproj /Rebuild Release
call :clean
devenv.com pc9801vf.vcxproj /Rebuild Release
call :clean
devenv.com pc9801vm.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc9801vm
copy binary_vc12\pc9801u.exe build_vc12\pc9801vm\.
copy binary_vc12\pc9801vf.exe build_vc12\pc9801vm\.
copy binary_vc12\pc9801vm.exe build_vc12\pc9801vm\.

devenv.com pc9801vx.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc9801vx
copy binary_vc12\pc9801vx.exe build_vc12\pc9801vx\.

devenv.com pc9801ra.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc9801ra
copy binary_vc12\pc9801ra.exe build_vc12\pc9801ra\.

devenv.com pc98rl.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc98rl
copy binary_vc12\pc98rl.exe build_vc12\pc98rl\.

devenv.com pc98xa.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc98xa
copy binary_vc12\pc98xa.exe build_vc12\pc98xa\.

devenv.com pc98xl.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc98xl
copy binary_vc12\pc98xl.exe build_vc12\pc98xl\.

devenv.com pc98do.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc98do
copy binary_vc12\pc98do.exe build_vc12\pc98do\.

devenv.com pc98ha.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc98ha
copy binary_vc12\pc98ha.exe build_vc12\pc98ha\.

devenv.com pc98lt.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc98lt
copy binary_vc12\pc98lt.exe build_vc12\pc98lt\.

devenv.com pc100.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pc100
copy binary_vc12\pc100.exe build_vc12\pc100\.

devenv.com pcengine.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pcengine
copy binary_vc12\pcengine.exe build_vc12\pcengine\.

devenv.com phc20.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\phc20
copy binary_vc12\phc20.exe build_vc12\phc20\.

devenv.com phc25.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\phc25
copy binary_vc12\phc25.exe build_vc12\phc25\.

devenv.com pv1000.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pv1000
copy binary_vc12\pv1000.exe build_vc12\pv1000\.

devenv.com pv2000.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pv2000
copy binary_vc12\pv2000.exe build_vc12\pv2000\.

devenv.com px7.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\px7
copy binary_vc12\px7.exe build_vc12\px7\.

devenv.com pyuta.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\pyuta
copy binary_vc12\pyuta.exe build_vc12\pyuta\.
mkdir build_vc12\pyuta_jr
copy binary_vc12\pyuta.exe build_vc12\pyuta_jr\.

devenv.com qc10.vcxproj /Rebuild Release
call :clean
devenv.com qc10cms.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\qc10
copy binary_vc12\qc10.exe build_vc12\qc10\.
copy binary_vc12\qc10cms.exe build_vc12\qc10\.

devenv.com rx78.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\rx78
copy binary_vc12\rx78.exe build_vc12\rx78\.

devenv.com sc3000.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\sc3000
copy binary_vc12\sc3000.exe build_vc12\sc3000\.

devenv.com scv.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\scv
copy binary_vc12\scv.exe build_vc12\scv\.

devenv.com smb80te.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\smb80te
copy binary_vc12\smb80te.exe build_vc12\smb80te\.

devenv.com smc70.vcxproj /Rebuild Release
call :clean
devenv.com smc777.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\smc777
copy binary_vc12\smc70.exe build_vc12\smc777\.
copy binary_vc12\smc777.exe build_vc12\smc777\.

devenv.com tk80bs.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\tk80bs
copy binary_vc12\tk80bs.exe build_vc12\tk80bs\.

devenv.com tk85.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\tk85
copy binary_vc12\tk85.exe build_vc12\tk85\.

devenv.com x07.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\x07
copy binary_vc12\x07.exe build_vc12\x07\.

devenv.com x1.vcxproj /Rebuild Release
call :clean
devenv.com x1twin.vcxproj /Rebuild Release
call :clean
devenv.com x1turbo.vcxproj /Rebuild Release
call :clean
devenv.com x1turboz.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\x1
copy binary_vc12\x1.exe build_vc12\x1\.
copy binary_vc12\x1twin.exe build_vc12\x1\.
copy binary_vc12\x1turbo.exe build_vc12\x1\.
copy binary_vc12\x1turboz.exe build_vc12\x1\.

devenv.com yalky.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\yalky
copy binary_vc12\yalky.exe build_vc12\yalky\.

devenv.com yis.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\yis
copy binary_vc12\yis.exe build_vc12\yis\.

devenv.com ys6464a.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\ys6464a
copy binary_vc12\ys6464a.exe build_vc12\ys6464a\.

devenv.com z80tvgame_i8255.vcxproj /Rebuild Release
call :clean
devenv.com z80tvgame_z80pio.vcxproj /Rebuild Release
call :clean
mkdir build_vc12\z80tvgame
copy binary_vc12\z80tvgame_i8255.exe build_vc12\z80tvgame\.
copy binary_vc12\z80tvgame_z80pio.exe build_vc12\z80tvgame\.

pause
echo on
exit /b

:clean
copy Release\*.exe binary_vc12\.
ren Release Release_tmp
rmdir /s /q Release_tmp
exit /b
