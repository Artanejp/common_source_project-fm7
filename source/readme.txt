retro pc emulator common source code
								12/29/2016

--- What's this ?

This archive includes the all source codes of emulators listed below:

	ASCII
		yaMSX1		MSX1 (by Mr.tanam and Mr.umaiboux)
		yaMSX2		MSX2 (by Mr.tanam and Mr.umaiboux)
	BANDAI
		eRX-78		RX-78
	CANON
		eX-07		X-07
	CASIO
		eFP-200		FP-200
		eFP-1100	FP-1100
		ePV-1000	PV-1000
		EmuGaki		PV-2000
	EPOCH
		eSCV		Super Cassette Vision
	EPSON
		eHC-20		HC-20/HX-20
		eHC-40		HC-40/PX-4
		eHC-80		HC-80/PX-8/Geneva
		eQC-10		QC-10 / QX-10
	FUJITSU
		eFM8		FM-8 (by Mr.Artane.)
		eFM7		FM-7 (by Mr.Artane.)
		eFM77		FM-77 (by Mr.Artane.)
		eFM77AV		FM77AV (by Mr.Artane.)
		eFM77AV40	FM77AV40 (by Mr.Artane.)
		eFM77AV40EX	FM77AV40EX (by Mr.Artane.)
		eFM16pi		FM16pi
		eFMR-30		FMR-30
		eFMR-50		FMR-50
		eFMR-60		FMR-60
		eFMR-70		FMR-70
		eFMR-80		FMR-80
	Gijutsu Hyoron Sha
		eBabbage-2nd	Babbage-2nd
	HITACHI
		eBASICMasterJr	BASIC Master Jr
	Homebrew
		eZ80TVGAME	Homebrew Z80 TV GAME SYSTEM
	IBM Japan Ltd
		eJX		PC/JX
	Japan Electronics College
		eMYCOMZ-80A	MYCOMZ-80A
	MITSUBISHI Electric
		EmuLTI8		MULTI8
	National
		eJR-100		JR-100
	NEC
		ePC-2001	PC-2001
		yaPC-6001	PC-6001 (by Mr.tanam)
		yaPC-6201	PC-6001mkII (by Mr.tanam)
		yaPC-6401	PC-6001mkIISR (by Mr.tanam)
		yaPC-6601	PC-6601 (by Mr.tanam)
		yaPC-6801	PC-6601SR (by Mr.tanam)
		ePC-8001mkIISR	PC-8001mkIISR
		ePC-8201	PC-8201/PC-8201A
		ePC-8801MA	PC-8801MA
		ePC-9801	PC-9801
		ePC-9801E	PC-9801E/F/M
		ePC-9801U	PC-9801U
		ePC-9801VF	PC-9801VF
		ePC-9801VM	PC-9801VM
		ePC-98DO	PC-98DO
		ePC-98LT	PC-98LT
		eHANDY98	PC-98HA
		ePC-100		PC-100
		eTK-80BS	TK-80BS / COMPO BS/80
		eN5200		N5200 (work in progress)
	NEC-HE
		ePCEngine	PC Engine / SuperGrafx + CD-ROM^2
	Nintendo
		eFamilyBASIC	Family BASIC
	Pioneer
		ePX-7		PX-7 (MSX1 + LaserDisc)
	SANYO
		ePHC-20		PHC-20
		ePHC-25		PHC-25
	SEGA
		eSC-3000	SC-3000
		yaGAME GEAR	GAME GEAR (by Mr.tanam)
		yaMASTER SYSTEM	MASTER SYSTEM (by Mr.tanam)
	SEIKO
		eMAP-1010	MAP-1010
	SHARP
		EmuZ-80A	MZ-80A (by Mr.Suga)
		EmuZ-80B	MZ-80B
		EmuZ-80K	MZ-80K/C
		EmuZ-700	MZ-700
		EmuZ-800	MZ-800
		EmuZ-1200	MZ-1200
		EmuZ-1500	MZ-1500
		EmuZ-2200	MZ-2200
		EmuZ-2500	MZ-2500
		EmuZ-2800	MZ-2800
		EmuZ-3500	MZ-3500
		EmuZ-5500	MZ-5500
		EmuZ-6500	MZ-6500
		EmuZ-6550	MZ-6550 (work in progress)
		eSM-B-80TE	SM-B-80TE
		eX1		X1
		eX1twin		X1twin
		eX1turbo	X1turbo
		eX1turboZ	X1turboZ (work in progress)
	Shinko Sangyo
		eYS-6464A	YS-6464A
	SONY
		eSMC-70		SMC-70
		eSMC-777	SMC-777
	SORD
		Emu5		m5
	TOMY
		ePyuTa		PyuTa/PyuTa Jr.
	TOSHIBA
		eEX-80		EX-80
		EmuPIA		PASOPIA/PASOPIA5
		EmuPIA7		PASOPIA7
		eJ-3100GT	J-3100GT (work in progress)
		eJ-3100SL	J-3100SL (work in progress)
	Yuasa Kyouiku System
		eYALKY		YALKY


--- How to build

Build the projects with the Microsoft Visual C++ 2008 with Service Pack 1 or
the Microsoft Visual C++ 2013 with Update 5.

The DirectX 9.0 SDK is required.
I recommend the DirectX 9.0 SDK Update (December 2004),
and dinput.lib included in the DirectX 9.0 SDK Update (October 2004).

If your DirectX 9.0 SDK is newer and does not contain dinput.lib,
pelase modify src/win32/osd.h to change the definition of DIRECTINPUT_VERSION
from 0x500 to 0x800.

When you use the Microsoft Visual C++ 2008 with Service Pack 1,
the Windows SDK for Windows 8.1 is also required to get the mt.exe utility.
The mt.exe is used to merge a manifest file for Windows Vista or later.

https://msdn.microsoft.com/en-us/windows/desktop/bg162891.aspx

When you use the Microsoft Visual C++ 2013 with Update 5, the system
environment variables WindowsSDK_IncludePath, WindowsSDK_LibraryPath_x86,
and DXSDK_DIR shoud be defined and should specifies the install directories
of the Windows SDK and the DirectX 9.0 SDK.


--- License

The copyright belongs to the author, but you can use the source codes
under the GNU GENERAL PUBLIC LICENSE Version 2.

See also COPYING.txt for more details about the license.


--- Thanks

- vm/datarec.*
	MESS formats/fmsx_cas.c for fMSX cas image decoder
- vm/device.h
	XM6 by Mr.PI.
- vm/fmgen/*
	M88/fmgen by Mr.CISC
- vm/disk.*
	TDLZHUF for Teledisk floppy disk image decoder
	MESS formats/dsk_dsk.c for CPDRead floppy disk image decorder
	MESS formats/imd_dsk.c for ImageDisk floppy disk image decorder
- vm/hd63484.*
	MAME HD63484 core
- vm/huc6280.*
	MESS huc6280 core
- vm/i86.*
	MAME i86 core
- vm/i286.*
	MAME i286 core
- vm/i386.*
	MAME i386 core
- vm/i8259.*
	Neko Project 2 and MESS 8259 core
- vm/ld700.*
	openMSX LD-700
- vm/m6502.*
	MAME m6502 core
- vm/mb8877.*
	XM7 by Mr.PI.
- vm/mc6800.*
	MAME mc6800 core
- vm/mc6809.*
	MAME mc6809 core and improved by Mr.Artane.
- vm/mc6840.*
	MAME Motorola 6840 (PTM) by Mr.James Wallace
- vm/mc6847.*
	MAME mc6847 core
- vm/msm5205.*
	MAME msm5205 core
- vm/mz1p17.*
	MZ-80P3 / MZ-80P4 mode by Mr.Suga
- vm/pc6031.*
	iP6 by Mr.Nishida
- vm/scsi_dev.*
- vm/scsi_host.*
	MAME SCSI bus codes gives me a good hint to implement SCSI protocols
- vm/scsi_cdrom.*
	NEC CD-ROM^2 features by MAME TG16 CD-ROM^2 driver (pce_cdrom.*)
- vm/sn76489an.*
	MAME SN76496 core
- vm/sy6522.*
	MAME Rockwell 6522 VIA by Mr.Peter Trauner and Mr.Mathis Rosenhauer
- vm/t3444a.*
	Mr.Oh!Ishi for the chip specification info
- vm/tf20.*
	vfloppy 1.4 by Mr.Justin Mitchell and Mr.Fred Jan Kraan
	Mr.Fred Han Kraan for EPSON TF-20 hardware design info
- vm/tms3631.*
	Neko Project 2 by Mr.Yui
- vm/tms9918a.*
	MAME TMS9928 core
- vm/tms9995.*
	MAME TMS99xx core
- vm/upd71071.*
	88VA Eternal Grafx by Mr.Shinra
- vm/upd7220.*
	Neko Project 2 by Mr.Yui
- vm/upd765a.*
	M88 fdc/fdu core by Mr.CISC
- vm/upd7752.*
	iP6 by Mr.Nishida
- vm/upd7801.*
	MAME uPD7810 core
	Mr.Komatsu for the chip specification info
- vm/upd7810.*
	MAME uPD7810 core and PockEmul uPD7907 core
- vm/upd16434.*
	PockEmul uPD16434 core
- vm/v99x8.*
	Zodiac V99x8 core, converted to C++ class by Mr.umaiboux
- vm/w3100a.*
	Mr.Oh!Ishi for the chip specification info
- vm/z80.*
	MAME Z80 core
- vm/z80dma.*
	MAME Z80DMA core and improved by Mr Y.S.
- vm/bmjr/*
	bm2 by Mr.maruhiro
	Mr.Enri for HITACH BASIC Master Jr hardware design info
- vm/familybasic
	nester by Mr.Darren Ranalli
- vm/fm7/*
	eFM7/77/77AV by Mr.Artane.
- vm/fmr50/bios.*
	FM-TOWNS emulator on bochs
	UNZ pseudo BIOS by Mr.Kasanova
- vm/fp200/*
	PockEmul gives much hints about LCD driver
- vm/gamegear/*
	yaGAMEGEAR/yaMASTERSYSTEM by Mr.tanam
- vm/hc20/*
	Mr.Fred Han Kraan for EPSON HC-20/HX-20 hardware design info
- vm/hc40/*
	Mr.Fred Han Kraan for EPSON HC-40/PX-4 hardware design info
- vm/hc80/*
	Mr.Fred Han Kraan for EPSON HC-80/PX-8/Geneva hardware design info
- vm/hc80/io.*
	Mr.Dennis Heynlein for intelligent ram disk unit
- vm/jr100/*
	Mr.Enri for National JR-100 hardware design info
- vm/m5/*
	MESS sord driver
	Mr.Moriya for Sord M5 hardware design info
- vm/msx/*
	yaMSX1 and yaMSX2 by Mr.tanam
- vm/msx/memory.*
	fMSX Disk BIOS
- vm/mycomz80a/mon/mon.c
	Based on MON80 by Mr.Tesuya Suzuki
- vm/mz80k/memory.*
- vm/mz80k/mz80aif.*
	MZ-80A emulator by Mr.Suga
- vm/mz80k/mz80fio.*
	Mr.Enri for SHARP MZ-80FIO and MZ-80FD hardware design info
- vm/mz80k/printer.*
	The printer interface by Mr.Suga
- vm/mz2500/sasi.*
	X millenium by Mr.Punyu
- vm/mz3500/keyboard.*
	The keycode tables are from Martinuv 8-bitovy blog
	http://www.8bity.cz/2013/adapter-pro-pripojeni-ps2-klavesnice-k-sharp-mz-3500/
- vm/pc6001/*
	yaPC-6001/yaPC-6201/yaPC-6601 by Mr.tanam
- vm/pc8801/pc88.*
	M88 by Mr.CISC
	XM8 by Mr.PI.
	MESS PC-8801 driver
- vm/pcengine/pce.*
	Ootake (joypad)
	xpce (psg)
	MESS TG16 driver (vdc/vce/vpc/cdrom)
- vm/phc25/*
	PHC-25 emulator by Mr.Tago
- vm/pv1000/*
	Mr.Enri for CASIO PV-1000 hardware design info
- vm/pv2000/*
	Mr.Enri for CASIO PV-2000 hardware design info
- vm/pyuta/*
	MESS tutor driver
	Mr.Enri for TOMY PyuTa Jr. hardware design info
- vm/qc10/*
	Mr.Fred Han Kraan for EPSON QC-10/QX-10 hardware design info
- vm/scv/*
	Mr.Enri and Mr.333 for Epoch Super Cassette Vision hardware info
- vm/x07/io.*
	x07_emul by Mr.Jacques Brigaud
- vm/x1/*
	Many advices by Mr Y.S.
- vm/x1/psub.*
	X millenium T-tune by Mr.Sato
- vm/x1/display.*
	KANJI ROM support by X1EMU
- vm/z80tvgame/*
	This homebrew cnosole is designed by Mr.Ishizu
	http://w01.tp1.jp/~a571632211/z80tvgame/index.html
- win32/osd_sound.cpp
	XM7 by Mr.PI. for DirectSound implement
- res/*.ico
	Mr.Temmaru, Mr.Marukun, and Mr.Yoshikun
	See also res/icon.txt

- emulation core design
	nester by Mr.Darren Ranalli
	XM6 by Mr.PI.

----------------------------------------
TAKEDA, toshiya
t-takeda@m1.interq.or.jp
http://takeda-toshiya.my.coocan.jp/
