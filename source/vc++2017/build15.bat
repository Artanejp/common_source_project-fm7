echo off

if exist "%ProgramFiles(x86)%" goto is_x64
set path="%ProgramFiles%\Microsoft Visual Studio\2017\WDExpress\MSBuild\15.0\Bin";%PATH%
goto start

:is_x64
set path="%ProgramFiles(x86)%\Microsoft Visual Studio\2017\WDExpress\MSBuild\15.0\Bin";%PATH%

:start
rmdir /s /q build_vc15
mkdir build_vc15

msbuild.exe babbage2nd.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\babbage2nd
copy bin\x86\Release\babbage2nd.exe build_vc15\babbage2nd\.

msbuild.exe bmjr.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\bmjr
copy bin\x86\Release\bmjr.exe build_vc15\bmjr\.

msbuild.exe bubcom80.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\bubcom80
copy bin\x86\Release\bubcom80.exe build_vc15\bubcom80\.

msbuild.exe bx1.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\bx1
copy bin\x86\Release\bx1.exe build_vc15\bx1\.

msbuild.exe colecovision.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\colecovision
copy bin\x86\Release\colecovision.exe build_vc15\colecovision\.

msbuild.exe ex80bs.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\ex80bs
copy bin\x86\Release\ex80bs.exe build_vc15\ex80bs\.

msbuild.exe familybasic.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\familybasic
copy bin\x86\Release\familybasic.exe build_vc15\familybasic\.

msbuild.exe fm8.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\fm8
copy bin\x86\Release\fm8.exe build_vc15\fm8\.

msbuild.exe fm7.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe fm77.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe fm77l4.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe fm77av.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\fm7
copy bin\x86\Release\fm7.exe build_vc15\fm7\.
copy bin\x86\Release\fm77.exe build_vc15\fm7\.
copy bin\x86\Release\fm77l4.exe build_vc15\fm7\.
copy bin\x86\Release\fm77av.exe build_vc15\fm7\.

msbuild.exe fm77av40.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe fm77av40ex.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\fm77av40
copy bin\x86\Release\fm77av40.exe build_vc15\fm77av40\.
copy bin\x86\Release\fm77av40ex.exe build_vc15\fm77av40\.

msbuild.exe fm16beta_i186.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe fm16beta_i286.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\fm16beta
copy bin\x86\Release\fm16beta_i186.exe build_vc15\fm16beta\.
copy bin\x86\Release\fm16beta_i286.exe build_vc15\fm16beta\.

msbuild.exe fm16pi.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\fm16pi
copy bin\x86\Release\fm16pi.exe build_vc15\fm16pi\.

msbuild.exe fmr30_i86.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe fmr30_i286.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\fmr30
copy bin\x86\Release\fmr30_i86.exe build_vc15\fmr30\.
copy bin\x86\Release\fmr30_i286.exe build_vc15\fmr30\.

msbuild.exe fmr50_i286.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe fmr50_i386.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe fmr50_i486.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\fmr50
copy bin\x86\Release\fmr50_i286.exe build_vc15\fmr50\.
copy bin\x86\Release\fmr50_i386.exe build_vc15\fmr50\.
copy bin\x86\Release\fmr50_i486.exe build_vc15\fmr50\.

msbuild.exe fmr60.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe fmr70.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe fmr80.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\fmr60
copy bin\x86\Release\fmr60.exe build_vc15\fmr60\.
copy bin\x86\Release\fmr70.exe build_vc15\fmr60\.
copy bin\x86\Release\fmr80.exe build_vc15\fmr60\.

msbuild.exe fp200.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\fp200
copy bin\x86\Release\fp200.exe build_vc15\fp200\.

msbuild.exe fp1100.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\fp1100
copy bin\x86\Release\fp1100.exe build_vc15\fp1100\.

msbuild.exe fx9000p.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\fx9000p
copy bin\x86\Release\fx9000p.exe build_vc15\fx9000p\.

msbuild.exe gamegear.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\gamegear
copy bin\x86\Release\gamegear.exe build_vc15\gamegear\.

msbuild.exe hc20.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\hc20
copy bin\x86\Release\hc20.exe build_vc15\hc20\.

msbuild.exe hc40.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\hc40
copy bin\x86\Release\hc40.exe build_vc15\hc40\.

msbuild.exe hc80.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\hc80
copy bin\x86\Release\hc80.exe build_vc15\hc80\.

msbuild.exe jr100.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\jr100
copy bin\x86\Release\jr100.exe build_vc15\jr100\.

msbuild.exe jr800.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\jr800
copy bin\x86\Release\jr800.exe build_vc15\jr800\.

msbuild.exe jx.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\jx
copy bin\x86\Release\jx.exe build_vc15\jx\.
mkdir build_vc15\jx_hires
copy bin\x86\Release\jx.exe build_vc15\jx_hires\.

msbuild.exe m23.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\m23
copy bin\x86\Release\m23.exe build_vc15\m23\.

msbuild.exe m5.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\m5
copy bin\x86\Release\m5.exe build_vc15\m5\.

msbuild.exe map1010.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\map1010
copy bin\x86\Release\map1010.exe build_vc15\map1010\.

msbuild.exe mastersystem.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mastersystem
copy bin\x86\Release\mastersystem.exe build_vc15\mastersystem\.

msbuild.exe micom_mahjong.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\micom_mahjong
copy bin\x86\Release\micom_mahjong.exe build_vc15\micom_mahjong\.

msbuild.exe mp85.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mp85
copy bin\x86\Release\mp85.exe build_vc15\mp85\.

msbuild.exe msx1.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe msx2.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe msx2p.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe fsa1.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe hx20.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\msx
copy bin\x86\Release\msx1.exe build_vc15\msx\.
copy bin\x86\Release\msx2.exe build_vc15\msx\.
copy bin\x86\Release\msx2p.exe build_vc15\msx\.
copy bin\x86\Release\fsa1.exe build_vc15\msx\.
copy bin\x86\Release\hx20.exe build_vc15\msx\.

msbuild.exe multi8.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\multi8
copy bin\x86\Release\multi8.exe build_vc15\multi8\.

msbuild.exe mycomz80a.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mycomz80a
copy bin\x86\Release\mycomz80a.exe build_vc15\mycomz80a\.

msbuild.exe mz80a.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz80a
copy bin\x86\Release\mz80a.exe build_vc15\mz80a\.

msbuild.exe mz80b.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz80b
copy bin\x86\Release\mz80b.exe build_vc15\mz80b\.

msbuild.exe mz80k.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz80k
copy bin\x86\Release\mz80k.exe build_vc15\mz80k\.

msbuild.exe mz700.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz700
copy bin\x86\Release\mz700.exe build_vc15\mz700\.

msbuild.exe mz800.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz800
copy bin\x86\Release\mz800.exe build_vc15\mz800\.

msbuild.exe mz1200.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz1200
copy bin\x86\Release\mz1200.exe build_vc15\mz1200\.

msbuild.exe mz1500.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz1500
copy bin\x86\Release\mz1500.exe build_vc15\mz1500\.

msbuild.exe mz2200.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz2200
copy bin\x86\Release\mz2200.exe build_vc15\mz2200\.

msbuild.exe mz2500.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz2500
copy bin\x86\Release\mz2500.exe build_vc15\mz2500\.

msbuild.exe mz2800.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz2800
copy bin\x86\Release\mz2800.exe build_vc15\mz2800\.

msbuild.exe mz3500.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz3500
copy bin\x86\Release\mz3500.exe build_vc15\mz3500\.

msbuild.exe mz5500.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz5500
copy bin\x86\Release\mz5500.exe build_vc15\mz5500\.

msbuild.exe mz6500.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\mz6500
copy bin\x86\Release\mz6500.exe build_vc15\mz6500\.

msbuild.exe pasopia.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pasopia
copy bin\x86\Release\pasopia.exe build_vc15\pasopia\.

msbuild.exe pasopia7.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe pasopia7lcd.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pasopia7
copy bin\x86\Release\pasopia7.exe build_vc15\pasopia7\.
copy bin\x86\Release\pasopia7lcd.exe build_vc15\pasopia7\.

msbuild.exe pc2001.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc2001
copy bin\x86\Release\pc2001.exe build_vc15\pc2001\.

msbuild.exe pc6001.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe pc6001mk2.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe pc6001mk2sr.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe pc6601.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe pc6601sr.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc6001
copy bin\x86\Release\pc6001.exe build_vc15\pc6001\.
copy bin\x86\Release\pc6001mk2.exe build_vc15\pc6001\.
copy bin\x86\Release\pc6001mk2sr.exe build_vc15\pc6001\.
copy bin\x86\Release\pc6601.exe build_vc15\pc6001\.
copy bin\x86\Release\pc6601sr.exe build_vc15\pc6001\.

msbuild.exe pc8001.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe pc8001mk2.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe pc8001mk2sr.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc8001
copy bin\x86\Release\pc8001.exe build_vc15\pc8001\.
copy bin\x86\Release\pc8001mk2.exe build_vc15\pc8001\.
copy bin\x86\Release\pc8001mk2sr.exe build_vc15\pc8001\.

msbuild.exe pc8201.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc8201
copy bin\x86\Release\pc8201.exe build_vc15\pc8201\.

msbuild.exe pc8201a.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc8201a
copy bin\x86\Release\pc8201a.exe build_vc15\pc8201a\.

msbuild.exe pc8801.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe pc8801mk2.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc8801
copy bin\x86\Release\pc8801.exe build_vc15\pc8801\.
copy bin\x86\Release\pc8801mk2.exe build_vc15\pc8801\.

msbuild.exe pc8801ma.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc8801ma
copy bin\x86\Release\pc8801ma.exe build_vc15\pc8801ma\.

msbuild.exe pc9801.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc9801
copy bin\x86\Release\pc9801.exe build_vc15\pc9801\.

msbuild.exe pc9801e.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc9801e
copy bin\x86\Release\pc9801e.exe build_vc15\pc9801e\.

msbuild.exe pc9801u.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe pc9801vf.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe pc9801vm.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc9801vm
copy bin\x86\Release\pc9801u.exe build_vc15\pc9801vm\.
copy bin\x86\Release\pc9801vf.exe build_vc15\pc9801vm\.
copy bin\x86\Release\pc9801vm.exe build_vc15\pc9801vm\.

msbuild.exe pc9801vx.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc9801vx
copy bin\x86\Release\pc9801vx.exe build_vc15\pc9801vx\.

msbuild.exe pc9801ra.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc9801ra
copy bin\x86\Release\pc9801ra.exe build_vc15\pc9801ra\.

msbuild.exe pc98rl.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc98rl
copy bin\x86\Release\pc98rl.exe build_vc15\pc98rl\.

msbuild.exe pc98xa.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc98xa
copy bin\x86\Release\pc98xa.exe build_vc15\pc98xa\.

msbuild.exe pc98xl.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc98xl
copy bin\x86\Release\pc98xl.exe build_vc15\pc98xl\.

msbuild.exe pc98do.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc98do
copy bin\x86\Release\pc98do.exe build_vc15\pc98do\.

msbuild.exe pc98ha.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc98ha
copy bin\x86\Release\pc98ha.exe build_vc15\pc98ha\.

msbuild.exe pc98lt.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc98lt
copy bin\x86\Release\pc98lt.exe build_vc15\pc98lt\.

msbuild.exe pc100.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pc100
copy bin\x86\Release\pc100.exe build_vc15\pc100\.

msbuild.exe pcengine.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pcengine
copy bin\x86\Release\pcengine.exe build_vc15\pcengine\.

msbuild.exe phc20.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\phc20
copy bin\x86\Release\phc20.exe build_vc15\phc20\.

msbuild.exe phc25.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\phc25
copy bin\x86\Release\phc25.exe build_vc15\phc25\.

msbuild.exe pv1000.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pv1000
copy bin\x86\Release\pv1000.exe build_vc15\pv1000\.

msbuild.exe pv2000.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pv2000
copy bin\x86\Release\pv2000.exe build_vc15\pv2000\.

msbuild.exe px7.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\px7
copy bin\x86\Release\px7.exe build_vc15\px7\.

msbuild.exe pyuta.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\pyuta
copy bin\x86\Release\pyuta.exe build_vc15\pyuta\.
mkdir build_vc15\pyuta_jr
copy bin\x86\Release\pyuta.exe build_vc15\pyuta_jr\.

msbuild.exe qc10.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe qc10cms.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\qc10
copy bin\x86\Release\qc10.exe build_vc15\qc10\.
copy bin\x86\Release\qc10cms.exe build_vc15\qc10\.

msbuild.exe rx78.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\rx78
copy bin\x86\Release\rx78.exe build_vc15\rx78\.

msbuild.exe sc3000.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\sc3000
copy bin\x86\Release\sc3000.exe build_vc15\sc3000\.

msbuild.exe scv.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\scv
copy bin\x86\Release\scv.exe build_vc15\scv\.

msbuild.exe smb80te.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\smb80te
copy bin\x86\Release\smb80te.exe build_vc15\smb80te\.

msbuild.exe smc70.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe smc777.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\smc777
copy bin\x86\Release\smc70.exe build_vc15\smc777\.
copy bin\x86\Release\smc777.exe build_vc15\smc777\.

msbuild.exe svi3x8.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\svi3x8
copy bin\x86\Release\svi3x8.exe build_vc15\svi3x8\.

msbuild.exe tk80bs.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\tk80bs
copy bin\x86\Release\tk80bs.exe build_vc15\tk80bs\.

msbuild.exe tk85.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\tk85
copy bin\x86\Release\tk85.exe build_vc15\tk85\.

msbuild.exe tvboy.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\tvboy
copy bin\x86\Release\tvboy.exe build_vc15\tvboy\.

msbuild.exe x07.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\x07
copy bin\x86\Release\x07.exe build_vc15\x07\.

msbuild.exe x1.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe x1twin.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe x1turbo.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe x1turboz.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\x1
copy bin\x86\Release\x1.exe build_vc15\x1\.
copy bin\x86\Release\x1twin.exe build_vc15\x1\.
copy bin\x86\Release\x1turbo.exe build_vc15\x1\.
copy bin\x86\Release\x1turboz.exe build_vc15\x1\.

msbuild.exe yalky.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\yalky
copy bin\x86\Release\yalky.exe build_vc15\yalky\.

msbuild.exe yis.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\yis
copy bin\x86\Release\yis.exe build_vc15\yis\.

msbuild.exe ys6464a.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\ys6464a
copy bin\x86\Release\ys6464a.exe build_vc15\ys6464a\.

msbuild.exe z80tvgame_i8255.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
msbuild.exe z80tvgame_z80pio.vcxproj /t:clean;rebuild /p:Configuration=Release;Platform="Win32"
mkdir build_vc15\z80tvgame
copy bin\x86\Release\z80tvgame_i8255.exe build_vc15\z80tvgame\.
copy bin\x86\Release\z80tvgame_z80pio.exe build_vc15\z80tvgame\.

pause
echo on
exit /b
