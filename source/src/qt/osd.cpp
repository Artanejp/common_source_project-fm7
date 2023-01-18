/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

#include "emu.h"

#include <memory>
#include <string>

#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QString>
#include <QObject>
#include <QThread>

#include "qt_gldraw.h"
#include "osd.h"
#include "config.h"

OSD::OSD(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger) : OSD_BASE(p, logger)
{
	//p_config = using_flags->get_config_ptr();
	p_config = &config;
	p_glv = NULL;
	set_features();
}

OSD::~OSD()
{
}

void OSD::set_features_machine(void)
{
// GIJUTSU-HYORON-SHA Babbase-2nd
#ifdef _BABBAGE2ND
	add_feature(_T("_BABBAGE2ND"), 1);
#endif

// HITACHI BASIC Master Jr
#ifdef _BMJR
	add_feature(_T("_BMJR"), 1);
#endif

// COLECO ColecoVision
#ifdef _COLECOVISION
	add_feature(_T("_COLECOVISION"), 1);
#endif

// TOSHIBA EX-80
#ifdef _EX80
	add_feature(_T("_EX80"), 1);
#endif

// Nintendo Family BASIC
#ifdef _FAMILYBASIC
	add_feature(_T("_FAMILYBASIC"), 1);
#endif

// FUJITSU FM-8
#ifdef _FM8
	add_feature(_T("_FM8"), 1);
#endif

// FUJITSU FM-7
#ifdef _FM7
	add_feature(_T("_FM7"), 1);
#endif

// FUJITSU FM-NEW7
#ifdef _FMNEW7
	add_feature(_T("_FM7"), 1);
	add_feature(_T("_FMNEW7"), 1);
#endif

// FUJITSU FM-77 SERIES
#ifdef _FM77
	add_feature(_T("_FM77"), 1);
#endif

#ifdef _FM77L2
	add_feature(_T("_FM77L2"), 1);
#endif
	
#ifdef _FM77L4
	add_feature(_T("_FM77L4"), 1);
#endif
	
#ifdef _FM77_VARIANTS
	add_feature(_T("_FM77_VARIANTS"), 1);
#endif

// FUJITSU FM77AV SERIES
#ifdef _FM77AV
	add_feature(_T("_FM77AV"), 1);
#endif

#ifdef _FM77AV20
	add_feature(_T("_FM77AV20"), 1);
#endif

#ifdef _FM77AV20EX
	add_feature(_T("_FM77AV20EX"), 1);
#endif

#ifdef _FM77AV40
	add_feature(_T("_FM77AV40"), 1);
#endif

#ifdef _FM77AV40SX
	add_feature(_T("_FM77AV40SX"), 1);
#endif

#ifdef _FM77AV40EX
	add_feature(_T("_FM77AV40EX"), 1);
#endif

#ifdef _FM77AV_VARIANTS
	add_feature(_T("_FM77AV_VARIANTS"), 1);
#endif

// FUJITSU FM16pi
#ifdef _FM16PI
	add_feature(_T("_FM16PI"), 1);
#endif


// FUJITSU FMR-30
#ifdef _FMR30
	add_feature(_T("_FMR30"), 1);
#endif

// FUJITSU FMR-50
#ifdef _FMR50
	add_feature(_T("_FMR50"), 1);
#endif

// FUJITSU FMR-60
#ifdef _FMR60
	add_feature(_T("_FMR60"), 1);
#endif

// CASIO FP-200
#ifdef _FP200
	add_feature(_T("_FP200"), 1);
#endif

// CASIO FP-1100
#ifdef _FP1100
	add_feature(_T("_FP1100"), 1);
#endif

// Panasonic FS-A1
#ifdef _FSA1
	add_feature(_T("_FSA1"), 1);
#endif

// SEGA GAME GEAR
#ifdef _GAMEGEAR
	add_feature(_T("_GAMEGEAR"), 1);
#endif

// EPSON HC-20
#ifdef _HC20
	add_feature(_T("_HC20"), 1);
#endif

// EPSON HC-40
#ifdef _HC40
	add_feature(_T("_HC40"), 1);
#endif

// EPSON HC-80
#ifdef _HC80
	add_feature(_T("_HC80"), 1);
#endif

// TOSHIBA HX-20 + FDD
#ifdef _HX20
	add_feature(_T("_HX20"), 1);
#endif

// TOSHIBA J-3100GT
#ifdef _J3100GT
	add_feature(_T("_J3100GT"), 1);
#endif

// TOSHIBA J-3100SL
#ifdef _J3100SL
	add_feature(_T("_J3100SL"), 1);
#endif

// National JR-100
#ifdef _JR100
	add_feature(_T("_JR100"), 1);
#endif

// National JR-200
#ifdef _JR200
	add_feature(_T("_JR200"), 1);
#endif

// National JR-800
#ifdef _JR800
	add_feature(_T("_JR800"), 1);
#endif

// IBM Japan Ltd PC/JX
#ifdef _JX
	add_feature(_T("_JX"), 1);
#endif

// SORD m5
#ifdef _M5
	add_feature(_T("_M5"), 1);
#endif

// SEIKO MAP-1010
#ifdef _MAP1010
	add_feature(_T("_MAP1010"), 1);
#endif

// SEGA MASTER SYSTEM
#ifdef _MASTERSYSTEM
	add_feature(_T("_MASTERSYSTEM"), 1);
#endif


// ASCII MSX
#ifdef _MSX1
	add_feature(_T("_MSX1"), 1);
#endif

// ASCII MSX2
#ifdef _MSX2
	add_feature(_T("_MSX2"), 1);
#endif

// ASCII MSX2+
#ifdef _MSX2P
	add_feature(_T("_MSX2P"), 1);
#endif

#ifdef _MSX1_VARIANTS
	add_feature(_T("_MSX1_VARIANTS"), 1);
#endif
#ifdef _MSX2_VARIANTS
	add_feature(_T("_MSX2_VARIANTS"), 1);
#endif
#ifdef _MSX2P_VARIANTS
	add_feature(_T("_MSX2P_VARIANTS"), 1);
#endif

// MITSUBISHI Elec. MULTI8
#ifdef _MULTI8
	add_feature(_T("_MULTI8"), 1);
#endif

// Japan Electronics College MYCOMZ-80A
#ifdef _MYCOMZ80A
	add_feature(_T("_MYCOMZ80A"), 1);
#endif

// SHARP MZ-80A
#ifdef _MZ80A
	add_feature(_T("_MZ80A"), 1);
#endif

// SHARP MZ-80B
#ifdef _MZ80B
	add_feature(_T("_MZ80B"), 1);
#endif

// SHARP MZ-80K
#ifdef _MZ80K
	add_feature(_T("_MZ80K"), 1);
#endif
	
// SHARP MZ-700
#ifdef _MZ700
	add_feature(_T("_MZ700"), 1);
#endif

// SHARP MZ-800
#ifdef _MZ800
	add_feature(_T("_MZ800"), 1);
#endif

// SHARP MZ-1200
#ifdef _MZ1200
	add_feature(_T("_MZ1200"), 1);
#endif

// SHARP MZ-1500
#ifdef _MZ1500
	add_feature(_T("_MZ1500"), 1);
#endif
	
// SHARP MZ-2000
#ifdef _MZ2000
	add_feature(_T("_MZ2000"), 1);
#endif

// SHARP MZ-2200
#ifdef _MZ2200
	add_feature(_T("_MZ2200"), 1);
#endif

// SHARP MZ-2500
#ifdef _MZ2500
	add_feature(_T("_MZ2500"), 1);
#endif

// SHARP MZ-2800
#ifdef _MZ2800
	add_feature(_T("_MZ2800"), 1);
#endif

// SHARP MZ-3500
#ifdef _MZ3500
	add_feature(_T("_MZ3500"), 1);
#endif

// SHARP MZ-5500
#ifdef _MZ5500
	add_feature(_T("_MZ5500"), 1);
#endif

// SHARP MZ-6500
#ifdef _MZ6500
	add_feature(_T("_MZ6500"), 1);
#endif
	
// SHARP MZ-6550
#ifdef _MZ6550
	add_feature(_T("_MZ6550"), 1);
#endif

// NEC N5200
#ifdef _N5200
	add_feature(_T("_N5200"), 1);
#endif

// TOSHIBA PASOPIA
#ifdef _PASOPIA
	add_feature(_T("_PASOPIA"), 1);
#endif

// TOSHIBA PASOPIA 7
#ifdef _PASOPIA7
	add_feature(_T("_PASOPIA7"), 1);
#endif

// NEC PC-2001
#ifdef _PC2001
	add_feature(_T("_PC2001"), 1);
#endif

// NEC PC-6001
#ifdef _PC6001
	add_feature(_T("_PC6001"), 1);
#endif

// NEC PC-6001mkII
#ifdef _PC6001MK2
	add_feature(_T("_PC6001MK2"), 1);
#endif

// NEC PC-6001mkIISR
#ifdef _PC6001MK2SR
	add_feature(_T("_PC6001MK2SR"), 1);
#endif

// NEC PC-6601
#ifdef _PC6601
	add_feature(_T("_PC6601"), 1);
#endif

// NEC PC-6601SR
#ifdef _PC6601SR
	add_feature(_T("_PC6601SR"), 1);
#endif

// NEC PC-8001mkIISR
#ifdef _PC8001SR
	add_feature(_T("_PC8001SR"), 1);
#endif

// NEC PC-8201
#ifdef _PC8201
	add_feature(_T("_PC8201"), 1);
#endif

// NEC PC-8201A
#ifdef _PC8201A
	add_feature(_T("_PC8201A"), 1);
#endif

// NEC PC-8801MA
#ifdef _PC8801MA
	add_feature(_T("_PC8801MA"), 1);
#endif

// NEC PC-9801
#ifdef _PC9801
	add_feature(_T("_PC9801"), 1);
#endif

// NEC PC-9801E/F/M
#ifdef _PC9801E
	add_feature(_T("_PC9801E"), 1);
#endif

// NEC PC-9801U
#ifdef _PC9801U
	add_feature(_T("_PC9801U"), 1);
#endif

// NEC PC-9801VF
#ifdef _PC9801VF
	add_feature(_T("_PC9801VF"), 1);
#endif

// NEC PC-9801VM
#ifdef _PC9801VM
	add_feature(_T("_PC9801VM"), 1);
#endif

// NEC PC-98DO
#ifdef _PC98DO
	add_feature(_T("_PC98DO"), 1);
#endif

// NEC PC-98HA
#ifdef _PC98HA
	add_feature(_T("_PC98HA"), 1);
#endif

// NEC PC-98LT
#ifdef _PC98LT
	add_feature(_T("_PC98LT"), 1);
#endif

// NEC PC-100
#ifdef _PC100
	add_feature(_T("_PC100"), 1);
#endif

// NEC-HE PC Engine
#ifdef _PCENGINE
	add_feature(_T("_PCENGINE"), 1);
#endif

// SANYO PHC-20
#ifdef _PHC20
	add_feature(_T("_PHC20"), 1);
#endif

// SANYO PHC-25
#ifdef _PHC25
	add_feature(_T("_PHC25"), 1);
#endif

// CASIO PV-1000
#ifdef _PV1000
	add_feature(_T("_PV1000"), 1);
#endif

// CASIO PV-2000
#ifdef _PV2000
	add_feature(_T("_PV2000"), 1);
#endif

// PIONEER PX-7
#ifdef _PX7
	add_feature(_T("_PX7"), 1);
#endif

// TOMY PYUTA
#ifdef _PYUTA
	add_feature(_T("_PYUTA"), 1);
#endif

// EPSON QC-10
#ifdef _QC10
	add_feature(_T("_QC10"), 1);
#endif

// BANDAI RX-78
#ifdef _RX78
	add_feature(_T("_RX78"), 1);
#endif

// SEGA SC-3000
#ifdef _SC3000
	add_feature(_T("_SC3000"), 1);
#endif

// EPOCH Super Cassette Vision
#ifdef _SCV
	add_feature(_T("_SCV"), 1);
#endif

// SHARP SM-B-80TE
#ifdef _SMB80TE
	add_feature(_T("_SMB80TE"), 1);
#endif

// SONY SMC-70
#ifdef _SMC70
	add_feature(_T("_SMC70"), 1);
#endif

// SONY SMC-777
#ifdef _SMC777
	add_feature(_T("_SMC777"), 1);
#endif


// NEC TK-80BS (COMPO BS/80)
#ifdef _TK80BS
	add_feature(_T("_TK80BS"), 1);
#endif

// NEC TK-80
#ifdef _TK80
	add_feature(_T("_TK80"), 1);
#endif

// NEC TK-85
#ifdef _TK85
	add_feature(_T("_TK85"), 1);
#endif

// CANON X-07
#ifdef _X07
	add_feature(_T("_X07"), 1);
#endif

// SHARP X1
#ifdef _X1
	add_feature(_T("_X1"), 1);
#endif

// SHARP X1twin
#ifdef _X1TWIN
	add_feature(_T("_X1TWIN"), 1);
#endif

// SHARP X1turbo
#ifdef _X1TURBO
	add_feature(_T("_X1TURBO"), 1);
#endif
	
// SHARP X1turboz
#ifdef _X1TURBOZ
	add_feature(_T("_X1TURBOZ"), 1);
#endif
	
// Yuasa Kyouiku System YALKY
#ifdef _YALKY
	add_feature(_T("_YALKY"), 1);
#endif

// YAMAHA YIS
#ifdef _YIS
	add_feature(_T("_YIS"), 1);
#endif

// SHINKO SANGYO YS-6464A
#ifdef _YS6464A
	add_feature(_T("_YS6464A"), 1);
#endif

// Homebrew Z80 TV GAME SYSTEM
#ifdef _Z80TVGAME
	add_feature(_T("_Z80TVGAME"), 1);
#endif
}

void OSD::set_features_cpu(void)
{
#ifdef HAS_I86
	add_feature(_T("HAS_I86"), 1);
#endif
#ifdef HAS_I88
	add_feature(_T("HAS_I88"), 1);
#endif
#ifdef HAS_I186
	add_feature(_T("HAS_I186"), 1);
#endif
#ifdef HAS_I286
	add_feature(_T("HAS_I286"), 1);
#endif
#ifdef HAS_I386
	add_feature(_T("HAS_I386"), 1);
#endif
#ifdef HAS_I386DX
	add_feature(_T("HAS_I386DX"), 1);
#endif
#ifdef HAS_I386SX
	add_feature(_T("HAS_I386SX"), 1);
#endif
#ifdef HAS_I486
	add_feature(_T("HAS_I486"), 1);
#endif
#ifdef HAS_I486SX
	add_feature(_T("HAS_I486SX"), 1);
#endif
#ifdef HAS_I486DX
	add_feature(_T("HAS_I486DX"), 1);
#endif
#ifdef HAS_PENTIUM
	add_feature(_T("HAS_PENTIUM"), 1);
#endif
#ifdef HAS_PENTIUM_PRO
	add_feature(_T("HAS_PENTIUM_PRO"), 1);
#endif
#ifdef HAS_PENTIUM_MMX
	add_feature(_T("HAS_PENTIUM_MMX"), 1);
#endif
#ifdef HAS_PENTIUM2
	add_feature(_T("HAS_PENTIUM2"), 1);
#endif
#ifdef HAS_PENTIUM2
	add_feature(_T("HAS_PENTIUM3"), 1);
#endif
#ifdef HAS_PENTIUM2
	add_feature(_T("HAS_PENTIUM4"), 1);
#endif
	
#ifdef HAS_V30
	add_feature(_T("HAS_V30"), 1);
#endif
#ifdef HAS_I8085
	add_feature(_T("HAS_I8085"), 1);
#endif
#ifdef HAS_N2A03
	add_feature(_T("HAS_N2A03"), 1);
#endif
#ifdef HAS_MC6800
	add_feature(_T("HAS_MC6800"), 1);
#endif
#ifdef HAS_MC6801
	add_feature(_T("HAS_MC6801"), 1);
#endif
#ifdef HAS_HD6301
	add_feature(_T("HAS_HD6301"), 1);
#endif
#ifdef HAS_NSC800
	add_feature(_T("HAS_NSC800"), 1);
#endif
#ifdef I80186
	add_feature(_T("I80186"), 1);
#endif
#ifdef I80286
	add_feature(_T("I80286"), 1);
#endif
#ifdef RS6000
	add_feature(_T("RS6000"), 1);
#endif

#ifdef I86_PSEUDO_BIOS
	add_feature(_T("I86_PSEUDO_BIOS"), 1);
#endif
#ifdef I286_PSEUDO_BIOS
	add_feature(_T("I286_PSEUDO_BIOS"), 1);
#endif
#ifdef I386_PSEUDO_BIOS
	add_feature(_T("I386_PSEUDO_BIOS"), 1);
#endif
#ifdef Z80_PSEUDO_BIOS
	add_feature(_T("Z80_PSEUDO_BIOS"), 1);
#endif
	
}

void OSD::set_features_vm(void)
{
// Begin vm.h
#ifdef USE_CART
	add_feature(_T("MAX_CART"), (int)USE_CART);
	add_feature(_T("USE_CART"), (int)USE_CART);
#endif
#ifdef BASE_CART_NUM
	add_feature(_T("BASE_CART_NUM"), (int)BASE_CART_NUM);
#endif
	
#if defined(USE_FLOPPY_DISK)
	add_feature(_T("MAX_FD"), (int)USE_FLOPPY_DISK);
	add_feature(_T("USE_FLOPPY_DISK"), (int)USE_FLOPPY_DISK);
#endif
#ifdef BASE_FLOPPY_DISK_NUM
	add_feature(_T("BASE_FLOPPY_DISK_NUM"), (int)BASE_FLOPPY_DISK_NUM);
#endif

#ifdef USE_QUICK_DISK
	add_feature(_T("USE_QUICK_DISK"), (int)USE_QUICK_DISK);
	add_feature(_T("MAX_QD"), (int)USE_QUICK_DISK);
#endif
#ifdef BASE_QUICK_DISK_NUM
	add_feature(_T("BASE_QUICK_DISK_NUM"), (int)BASE_QUICK_DISK_NUM);
#endif

#ifdef USE_HARD_DISK
	add_feature(_T("USE_HARD_DISK"), (int)USE_HARD_DISK);
	add_feature(_T("MAX_HDD"), (int)USE_HARD_DISK);
#endif
#ifdef BASE_HARD_DISK_NUM
	add_feature(_T("BASE_HARD_DISK_NUM"), (int)BASE_HARD_DISK_NUM);
#endif

#ifdef USE_COMPACT_DISC
	add_feature(_T("USE_COMPACT_DISC"), (int)USE_COMPACT_DISC);
	add_feature(_T("MAX_CD"), (int)USE_COMPACT_DISC);
#endif
#ifdef BASE_COMPACT_DISC_NUM
	add_feature(_T("BASE_COMPACT_DISC_NUM"), (int)BASE_COMPACT_DISC_NUM);
#endif

#ifdef USE_LASER_DISC
	add_feature(_T("USE_LASER_DISC"), (int)USE_LASER_DISC);
	add_feature(_T("MAX_LD"), (int)USE_LASER_DISC);
#endif
#ifdef BASE_LASER_DISC_NUM
	add_feature(_T("BASE_LASER_DISC_NUM"), (int)BASE_LASER_DISC_NUM);
#endif

#ifdef USE_TAPE
	add_feature(_T("USE_TAPE"), (int)USE_TAPE);
	add_feature(_T("MAX_TAPE"), (int)USE_TAPE);
#endif
#ifdef BASE_TAPE_NUM
	add_feature(_T("BASE_TAPE_NUM"), (int)BASE_TAPE_NUM);
#endif

#ifdef USE_BINARY_FILE
	add_feature(_T("USE_BINARY_FILE"), (int)USE_BINARY_FILE);
	add_feature(_T("MAX_BINARY"), (int)USE_BINARY_FILE);
#endif
#ifdef BASE_BINARY_FILE_NUM
	add_feature(_T("BASE_BINARY_FILE_NUM"), (int)BASE_BINARY_FILE_NUM);
#endif

#ifdef USE_BUBBLE
	add_feature(_T("USE_BUBBLE"), (int)USE_BUBBLE);
	add_feature(_T("MAX_BUBBLE"), (int)USE_BUBBLE);
#endif
#ifdef BASE_BUBBLE_NUM
	add_feature(_T("BASE_BUBBLE_NUM"), (int)BASE_BUBBLE_NUM);
#endif

#ifdef KEY_KEEP_FRAMES
	add_feature(_T("KEY_KEEP_FRAMES"), (int)KEY_KEEP_FRAMES);
#endif
// End vm.h

#ifdef CPU_CLOCKS
	add_feature(_T("CPU_CLOCKS"), (int64_t)CPU_CLOCKS);
#endif
#ifdef APU_CLOCK
	add_feature(_T("APU_CLOCK"), (int64_t)APU_CLOCK);
#endif

	
#ifdef MAX_DRIVE
	add_feature(_T("MAX_DRIVE"), (int)MAX_DRIVE);
#endif

#ifdef HD46505_CHAR_CLOCK
	add_feature(_T("HD46505_CHAR_CLOCK"), (float)HD46505_CHAR_CLOCK);
#endif
#ifdef HD46505_HORIZ_FREQ
	add_feature(_T("HD46505_HORIZ_FREQ"), (float)HD46505_HORIZ_FREQ);
#endif
#ifdef _315_5124_LIMIT_SPRITES
	add_feature(_T("_315_5124_LIMIT_SPRITES"), 1);
#endif
#ifdef HAS_AY_3_8910
	add_feature(_T("HAS_AY_3_8910"));
#endif
#ifdef HAS_AY_3_8912
	add_feature(_T("HAS_AY_3_8912"));
#endif
#ifdef HAS_AY_3_8913
	add_feature(_T("HAS_AY_3_8913"));
#endif
#ifdef SUPPORT_AY_3_891X_PORT
	add_feature(_T("SUPPORT_AY_3_891X_PORT"), 1);
#endif
#ifdef AY_3_891X_PORT_MODE
	add_feature(_T("AY_3_891X_PORT_MODE"), (uint32_t)AY_3_891X_PORT_MODE);
#endif
#ifdef SUPPORT_AY_3_891X_PORT_A
	add_feature(_T("SUPPORT_AY_3_891X_PORT_A"), 1);
#endif
#ifdef SUPPORT_AY_3_891X_PORT_B
	add_feature(_T("SUPPORT_AY_3_891X_PORT_B"), 1);
#endif
#ifdef DATAREC_FAST_FWD_SPEED
	add_feature(_T("DATAREC_FAST_FWD_SPEED"), (double)DATAREC_FAST_FWD_SPEED);
#endif
#ifdef DATAREC_FAST_REW_SPEED
	add_feature(_T("DATAREC_FAST_REW_SPEED"), (double)DATAREC_FAST_REW_SPEED);
#endif
#ifdef DATAREC_PCM_VOLUME
	add_feature(_T("DATAREC_PCM_VOLUME"), (int)DATAREC_PCM_VOLUME);
#endif

#ifdef HAS_I8254
	add_feature(_T("HAS_I8254"), 1);
#endif
#ifdef I8255_AUTO_HAND_SHAKE
	add_feature(_T("I8255_AUTO_HAND_SHAKE"), 1);
#endif
#ifdef HAS_MB8866
	add_feature(_T("HAS_MB8866"), 1);
#endif
#ifdef HAS_MB8876
	add_feature(_T("HAS_MB8876"), 1);
#endif
#ifdef HAS_MB89311
	add_feature(_T("HAS_MB89311"), 1);
#endif
#ifdef MB8877_NO_BUSY_AFTER_SEEK
	add_feature(_T("MB8877_NO_BUSY_AFTER_SEEK"), 1);
#endif
#ifdef MB8877_DELAY_AFTER_SEEK
	add_feature(_T("MB8877_DELAY_AFTER_SEEK"), (int)MB8877_DELAY_AFTER_SEEK);
#endif
	
#ifdef I8259_MAX_CHIPS
	add_feature(_T("I8259_MAX_CHIPS"), (uint32_t)I8259_MAX_CHIPS);
#endif
#ifdef I8259_PC98_HACK
	add_feature(_T("I8259_PC98_HACK"), 1);
#endif
	
#ifdef I8080_MEMORY_WAIT
	add_feature(_T("I8080_MEMORY_WAIT"), 1);
#endif
#ifdef I8080_IO_WAIT
	add_feature(_T("I8080_IO_WAIT"), 1);
#endif

#ifdef MC6847_VRAM_OFS
	add_feature(_T("MC6847_VRAM_OFS"), (uint32_t)MC6847_VRAM_OFS);
#endif
#ifdef MC6847_VRAM_AS
	add_feature(_T("MC6847_VRAM_AS"), (uint32_t)MC6847_VRAM_AS);
#endif
#ifdef MC6847_VRAM_CSS
	add_feature(_T("MC6847_VRAM_CSS"), (uint32_t)MC6847_VRAM_CSS);
#endif
#ifdef MC6847_VRAM_INV
	add_feature(_T("MC6847_VRAM_INV"), (uint32_t)MC6847_VRAM_INV);
#endif
#ifdef MC6847_VRAM_INTEXT
	add_feature(_T("MC6847_VRAM_INTEXT"), (uint32_t)MC6847_VRAM_INTEXT);
#endif
	
#ifdef MC6847_ATTR_OFS
	add_feature(_T("MC6847_ATTR_OFS"), (uint32_t)MC6847_ATTR_OFS);
#endif
#ifdef MC6847_ATTR_AS
	add_feature(_T("MC6847_ATTR_AS"), (uint32_t)MC6847_ATTR_AS);
#endif
#ifdef MC6847_ATTR_CSS
	add_feature(_T("MC6847_ATTR_CSS"), (uint32_t)MC6847_ATTR_CSS);
#endif
#ifdef MC6847_ATTR_INV
	add_feature(_T("MC6847_ATTR_INV"), (uint32_t)MC6847_ATTR_INV);
#endif
#ifdef MC6847_ATTR_INTEXT
	add_feature(_T("MC6847_ATTR_INTEXT"), (uint32_t)MC6847_ATTR_INTEXT);
#endif

#ifdef MSM58321_START_DAY
	add_feature(_T("MSM58321_START_DAY"), (int)MSM58321_START_DAY);
#endif
#ifdef MSM58321_START_YEAR
	add_feature(_T("MSM58321_START_YEAR"), (int)MSM58321_START_YEAR);
#endif
#ifdef HAS_MSM5832
	add_feature(_T("HAS_MSM5832"), 1);
#endif
#ifdef PRINTER_STROBE_RISING_EDGE
	add_feature(_T("PRINTER_STROBE_RISING_EDGE"), 1);
#endif
#ifdef MZ1P17_SW1_4_ON
	add_feature(_T("MZ1P17_SW1_4_ON"), 1);
#endif
#ifdef DOT_PRINT
	add_feature(_T("DOT_PRINT"), 1);
#endif

#ifdef PC80S31K_NO_WAIT
	add_feature(_T("PC80S31K_NO_WAIT"), 1);
#endif

#ifdef HAS_RP5C15
	add_feature(_T("HAS_RP5C15"), 1);
#endif

#ifdef SCSI_DEV_IMMEDIATE_SELECT
	add_feature(_T("SCSI_DEV_IMMEDIATE_SELECT"), 1);
#endif
#ifdef SCSI_HOST_WIDE
	add_feature(_T("SCSI_HOST_WIDE"), 1);
#endif
#ifdef SCSI_HOST_AUTO_ACK
	add_feature(_T("SCSI_HOST_AUTO_ACK"), 1);
#endif
	
#ifdef HAS_SN76489
	add_feature(_T("HAS_SN76489"), 1);
#endif
#ifdef HAS_T3444M
	add_feature(_T("HAS_T3444M"), 1);
#endif

#ifdef TMS9918A_VRAM_SIZE
	add_feature(_T("TMS9918A_VRAM_SIZE"), (uint32_t)TMS9918A_VRAM_SIZE);
#endif
#ifdef TMS9918A_SUPER_IMPOSE
	add_feature(_T("TMS9918A_SUPER_IMPOSE"), 1);
#endif
#ifdef TMS9918A_LIMIT_SPRITES
	add_feature(_T("TMS9918A_LIMIT_SPRITES"), 1);
#endif
	
#ifdef TIMER_HACK
	add_feature(_T("TIMER_HACK"));
#endif
#ifdef KEYBOARD_HACK
	add_feature(_T("KEYBOARD_HACK"));
#endif
	
#ifdef HAS_UPD7907
	add_feature(_T("HAS_UPD7907"), 1);
#endif
#ifdef HAS_UPD4990A
	add_feature(_T("HAS_UPD4990A"), 1);
#endif
#ifdef UPD7220_HORIZ_FREQ
	add_feature(_T("UPD7220_HORIZ_FREQ"), (int)UPD7220_HORIZ_FREQ);
#endif
#ifdef UPD7220_UGLY_PC98_HACK
	add_feature(_T("UPD7220_UGLY_PC98_HACK"), 1);
#endif
#ifdef UPD7220_FIXED_PITCH
	add_feature(_T("UPD7220_FIXED_PITCH"), 1);
#endif
#ifdef UPD7220_MSB_FIRST
	add_feature(_T("UPD7220_MSB_FIRST"), 1);
#endif
#ifdef UPD7220_A_VERSION	
	add_feature(_T("UPD7220_A_VERSION"), (int)UPD7220_A_VERSION);
#endif
#ifdef UPD765A_DMA_MODE
	add_feature(_T("UPD765A_DMA_MODE"), 1);
#endif
#ifdef UPD765A_EXT_DRVSEL
	add_feature(_T("UPD765A_EXT_DRVSEL"), 1);
#endif
#ifdef UPD765A_SENCE_INTSTAT_RESULT
	add_feature(_T("UPD765A_SENCE_INTSTAT_RESULT"), 1);
#endif
#ifdef UPD765A_DONT_WAIT_SEEK
	add_feature(_T("UPD765A_DONT_WAIT_SEEK"), 1);
#endif
#ifdef UPD765A_NO_ST0_AT_FOR_SEEK
	add_feature(_T("UPD765A_NO_ST0_AT_FOR_SEEK"), 1);
#endif
#ifdef UPD765A_WAIT_RESULT7
	add_feature(_T("UPD765A_WAIT_RESULT7"), 1);
#endif
#ifdef UPD765A_NO_ST1_EN_OR_FOR_RESULT7
	add_feature(_T("UPD765A_NO_ST1_EN_OR_FOR_RESULT7"), 1);
#endif

#ifdef UPD7801_MEMORY_WAIT
	add_feature(_T("UPD7801_MEMORY_WAIT") , 1);
#endif
#ifdef HAS_UPD7810
	add_feature(_T("HAS_UPD7810"), 1);
#endif
#ifdef HAS_UPD7807
	add_feature(_T("HAS_UPD7807"), 1);
#endif
#ifdef HAS_UPD7801
	add_feature(_T("HAS_UPD7801"), 1);
#endif
#ifdef HAS_UPD78C05
	add_feature(_T("HAS_UPD78C05"), 1);
#endif
#ifdef HAS_UPD78C06
	add_feature(_T("HAS_UPD78C06"), 1);
#endif
#ifdef HAS_UPD7907
	add_feature(_T("HAS_UPD7907"), 1);
#endif

#ifdef HAS_YM2608
	add_feature(_T("HAS_YM2608"), 1);
#endif
#ifdef HAS_YM_SERIES
	add_feature(_T("HAS_YM_SERIES"), 1);
#endif
#ifdef SUPPORT_YM2203_PORT
	add_feature(_T("SUPPORT_YM2203_PORT"), 1);
#endif
#ifdef SUPPORT_YM2203_PORT_A
	add_feature(_T("SUPPORT_YM2203_PORT_A"), 1);
#endif
#ifdef SUPPORT_YM2203_PORT_B
	add_feature(_T("SUPPORT_YM2203_PORT_B"), 1);
#endif

#ifdef Z80_MEMORY_WAIT
	add_feature(_T("Z80_MEMORY_WAIT"), 1);
#endif
#ifdef Z80_IO_WAIT
	add_feature(_T("Z80_IO_WAIT"), 1);
#endif
#ifdef HAS_LDAIR_QUIRK
	add_feature(_T("HAS_LDAIR_QUIRK"), 1);
#endif
#ifdef Z80CTC_CLOCKS
	add_feature(_T("Z80CTC_CLOCKS"), (double)Z80CTC_CLOCKS);
#endif
#ifdef HAS_UPD7201
	add_feature(_T("HAS_UPD7201"), 1);
#endif

#ifdef _X1TURBO_FEATURE
	add_feature(_T("_X1TURBO_FEATURE"), 1);
#endif
#ifdef MEMORY_DISABLE_DMA_MMIO
	add_feature(_T("MEMORY_DISABLE_DMA_MMIO"), 1);
#endif
}

void OSD::set_features_debug(void)
{

#ifdef USE_DEBUGGER
	add_feature(_T("USE_DEBUGGER"), 1);
#endif
#ifdef _DEBUG_LOG
	add_feature(_T("_DEBUG_LOG"), 1);
#endif
#ifdef _FDC_DEBUG_LOG
	add_feature(_T("_FDC_DEBUG_LOG"), 1);
#endif
#ifdef _IO_DEBUG_LOG
	add_feature(_T("_IO_DEBUG_LOG"), 1);
#endif
#ifdef DEBUG_MISSING_OPCODE
	add_feature(_T("DEBUG_MISSING_OPCODE"), 1);
#endif
#ifdef _SCSI_DEBUG_LOG
	add_feature(_T("_SCSI_DEBUG_LOG"), 1);
#endif
#ifdef _CDROM_DEBUG_LOG
	add_feature(_T("_CDROM_DEBUG_LOG"), 1);
#endif
#ifdef _DEBUG_PC80S31K
	add_feature(_T("_DEBUG_PC80S31K"), 1);
#endif
#ifdef DMA_DEBUG_LOG
	add_feature(_T("DMA_DEBUG_LOG"), 1);
#endif
#ifdef SIO_DEBUG
	add_feature(_T("SIO_DEBUG"), 1);
#endif
}

void OSD::set_features_misc(void)
{
#ifdef LSB_FIRST
	add_feature(_T("LSB_FIRST"), 1);
#endif
#ifdef SINGLE_MODE_DMA
	add_feature(_T("SINGLE_MODE_DMA"), 1);
#endif
#ifdef MEMORY_ADDR_MAX
	add_feature(_T("MEMORY_ADDR_MAX"), (uint64_t)MEMORY_ADDR_MAX);
#endif
#ifdef MEMORY_BANK_SIZE
	add_feature(_T("MEMORY_BANK_SIZE"), (uint64_t)MEMORY_BANK_SIZE);
#endif
#ifdef IOBUS_RETURN_ADDR
	add_feature(_T("IOBUS_RETURN_ADDR"), 1);
#endif
#ifdef IO_ADDR_MAX
	add_feature(_T("IO_ADDR_MAX"), (uint32_t)IO_ADDR_MAX);
#endif
#ifdef CPU_START_ADDR
	add_feature(_T("CPU_START_ADDR"), (uint32_t)CPU_START_ADDR);
#endif

#ifdef LOW_PASS_FILTER
	add_feature(_T("LOW_PASS_FILTER"), 1);
#endif

#ifdef SUPPORT_MAME_FM_DLL
	add_feature(_T("SUPPORT_MAME_FM_DLL"), 1);
#endif
#ifdef SUPPORT_WIN32_DLL
	add_feature(_T("SUPPORT_WIN32_DLL"), 1);
#endif
	
#ifdef SCREEN_WIDTH
	add_feature(_T("SCREEN_WIDTH"), (int)SCREEN_WIDTH);
#endif
#ifdef SCREEN_HEIGHT
	add_feature(_T("SCREEN_HEIGHT"), (int)SCREEN_HEIGHT);
#endif
#ifdef CHARS_PER_LINE
	add_feature(_T("CHARS_PER_LINE"), (int)CHARS_PER_LINE);
#endif
#ifdef LINES_PER_FRAME
	add_feature(_T("LINES_PER_FRAME"), (int)LINES_PER_FRAME);
#endif
#ifdef USE_ALPHA_BLENDING_TO_IMPOSE
	add_feature(_T("USE_ALPHA_BLENDING_TO_IMPOSE"), 1);
#endif
#ifdef SUPPORT_MEDIA_TYPE_1DD
	add_feature(_T("SUPPORT_MEDIA_TYPE_1DD"), 1);
#endif
#ifdef DATAREC_SOUND
	add_feature(_T("DATAREC_SOUND"), 1);
#endif
#ifdef DATAREC_SOUND_LEFT
	add_feature(_T("DATAREC_SOUND_LEFT"), 1);
#endif
#ifdef DATAREC_SOUND_RIGHT
	add_feature(_T("DATAREC_SOUND_RIGHT"), 1);
#endif
#ifdef SUPPORT_QUERY_PHY_KEY_NAME
	add_feature(_T("SUPPORT_QUERY_PHY_KEY_NAME"), 1);
#endif
}

void OSD::set_features(void)
{
	set_features_machine();
	set_features_cpu();
	set_features_vm();
	set_features_misc();
	set_features_debug();

	__USE_AUTO_KEY = false;
#ifdef USE_AUTO_KEY
	__USE_AUTO_KEY = true;
#endif
}

extern std::string cpp_homedir;
extern std::string my_procname;

void OSD::initialize(int rate, int samples, int* presented_rate, int* presented_samples)
{
	// get module path
	QString tmp_path;
	tmp_path = QString::fromStdString(cpp_homedir);
#if defined(Q_OS_WIN)
	const char *delim = "\\";
#else
	const char *delim = "/";
#endif
	//tmp_path = tmp_path + QString::fromUtf8(delim);
	tmp_path = tmp_path + QString::fromUtf8("CommonSourceCodeProject");
	tmp_path = tmp_path + QString::fromUtf8(delim);
	tmp_path = tmp_path + QString::fromStdString(my_procname);
	tmp_path = tmp_path + QString::fromUtf8(delim);
	memset(app_path, 0x00, sizeof(app_path));
	strncpy(app_path, tmp_path.toUtf8().constData(), _MAX_PATH - 1);
	
	console_cmd_str.clear();

	osd_timer.start();

	initialize_console();
	initialize_input();
	initialize_printer();
	initialize_screen();
	initialize_midi();
	
	initialize_sound(rate, samples, presented_rate, presented_samples);
	if(get_use_movie_player() || get_use_video_capture()) initialize_video();
	if(get_use_socket()) initialize_socket();
}

void OSD::release()
{
	release_console();
	release_input();
	release_printer();
	release_screen();
	release_midi();
	
	release_sound();
	if(get_use_movie_player() || get_use_video_capture()) release_video();
	if(get_use_socket()) release_socket();
}

