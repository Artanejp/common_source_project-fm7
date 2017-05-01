/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

//#include "emu.h"
#include <string>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QString>
#include <QObject>
#include <QThread>

#include "qt_gldraw.h"
#include "osd.h"

OSD::OSD(USING_FLAGS *p, CSP_Logger *logger) : OSD_BASE(p, logger)
{
	set_features();
}

OSD::~OSD()
{
}

void OSD::set_features_machine(void)
{
#ifdef _MASTERSYSTEM
	add_feature(_T("_MASTERSYSTEM"), 1);
#endif
#ifdef _SC3000
	add_feature(_T("_SC3000"), 1);
#endif

#ifdef _MZ80A
	add_feature(_T("_MZ80A"), 1);
#endif
#ifdef _MZ80K
	add_feature(_T("_MZ80K"), 1);
#endif
#ifdef _MZ80B
	add_feature(_T("_MZ80B"), 1);
#endif
#ifdef _MZ2000
	add_feature(_T("_MZ2000"), 1);
#endif
#ifdef _MZ2200
	add_feature(_T("_MZ2200"), 1);
#endif
#ifdef _MZ2500
	add_feature(_T("_MZ2500"), 1);
#endif
#ifdef _MZ1200
	add_feature(_T("_MZ1200"), 1);
#endif
#ifdef _MZ1500
	add_feature(_T("_MZ1500"), 1);
#endif
#ifdef _MZ700
	add_feature(_T("_MZ700"), 1);
#endif
#ifdef _FM7
	add_feature(_T("_FM7"), 1);
#endif
#ifdef _FM8
	add_feature(_T("_FM8"), 1);
#endif
#ifdef _FM77_VARIANTS
	add_feature(_T("_FM77_VARIANTS"), 1);
#endif
#ifdef _FM77AV_VARIANTS
	add_feature(_T("_FM77AV_VARIANTS"), 1);
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
#ifdef _FMR30
	add_feature(_T("_FMR30"), 1);
#endif
#ifdef _FMR50
	add_feature(_T("_FMR50"), 1);
#endif
#ifdef _FMR60
	add_feature(_T("_FMR60"), 1);
#endif
#ifdef _FP200
	add_feature(_T("_FP200"), 1);
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
#ifdef _QC10
	add_feature(_T("_QC10"), 1);
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
#ifdef HAS_I486
	add_feature(_T("HAS_I486"), 1);
#endif
#ifdef HAS_PENTIUM
	add_feature(_T("HAS_PENTIUM"), 1);
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
#ifdef HD46505_CHAR_CLOCK
	add_feature(_T("HD46505_CHAR_CLOCK"), (float)HD46505_CHAR_CLOCK);
#endif
#ifdef _315_5124_LIMIT_SPRITES
	add_feature(_T("_315_5124_LIMIT_SPRITES"), 1);
#endif
#ifdef HAS_AY_3_8910
	add_feature(_T("HAS_AY_3_8910"));
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
	add_feature(_T("DATAREC_FAST_FWD_SPEED"), (int)DATAREC_FAST_FWD_SPEED);
#endif
#ifdef DATAREC_FAST_REW_SPEED
	add_feature(_T("DATAREC_FAST_REW_SPEED"), (int)DATAREC_FAST_REW_SPEED);
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

#ifdef TMS9918A_SUPER_IMPOSE
	add_feature(_T("TMS9918A_SUPER_IMPOSE"), 1);
#endif
#ifdef TMS9918A_LIMIT_SPRITES
	add_feature(_T("TMS9918A_LIMIT_SPRITES"), 1);
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
#ifdef UPD7220_FIXED_PITCH
	add_feature(_T("UPD7220_FIXED_PITCH"), 1);
#endif
#ifdef UPD7220_MSB_FIRST
	add_feature(_T("UPD7220_MSB_FIRST"), 1);
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
	add_feature(T("HAS_UPD7810"), 1);
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
	add_feature(_T("Z80CTC_CLOCKS"), (int)Z80CTC_CLOCKS);
#endif
#ifdef HAS_UPD7201
	add_feature(_T("HAS_UPD7201"), 1);
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
#ifdef _DEBUG_PC80S31K
	add_feature(_T("_DEBUG_PC80S31K"), 1);
#endif
#ifdef DMA_DEBUG
	add_feature(_T("DMA_DEBUG"), 1);
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
	add_feature(_T("MEMORY_ADDR_MAX"), (uint32_t)MEMORY_ADDR_MAX);
#endif
#ifdef MEMORY_BANK_SIZE
	add_feature(_T("MEMORY_BANK_SIZE"), (uint32_t)MEMORY_BANK_SIZE);
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
#ifdef SUPPORT_VARIABLE_TIMING
	add_feature(_T("SUPPORT_VARIABLE_TIMING"), 1);
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
}

void OSD::set_features(void)
{
	set_features_machine();
	set_features_cpu();
	set_features_vm();
	set_features_misc();
	set_features_debug();
}

extern std::string cpp_homedir;
extern std::string my_procname;

void OSD::initialize(int rate, int samples)
{
	// get module path
	QString tmp_path;
	tmp_path = QString::fromStdString(cpp_homedir);
	tmp_path = tmp_path + QString::fromStdString(my_procname);
#if defined(Q_OS_WIN)
	const char *delim = "\\";
#else
	const char *delim = "/";
#endif
	tmp_path = tmp_path + QString::fromUtf8(delim);
	memset(app_path, 0x00, sizeof(app_path));
	strncpy(app_path, tmp_path.toUtf8().constData(), _MAX_PATH);
	
	console_cmd_str.clear();
	osd_console_opened = false;
	osd_timer.start();

	initialize_input();
	initialize_printer();
	initialize_screen();
	initialize_sound(rate, samples);
#if defined(USE_SOUND_FILES)
	init_sound_files();
#endif
	if(get_use_movie_player() || get_use_video_capture()) initialize_video();
	if(get_use_socket()) initialize_socket();

	connect(this, SIGNAL(sig_enable_mouse()), glv, SLOT(do_enable_mouse()));
	connect(this, SIGNAL(sig_disable_mouse()), glv, SLOT(do_disable_mouse()));
}

void OSD::release()
{
	release_input();
	release_printer();
	release_screen();
	release_sound();
	if(get_use_movie_player() || get_use_video_capture()) release_video();
	if(get_use_socket()) release_socket();
#if defined(USE_SOUND_FILES)
	release_sound_files();
#endif
}

void OSD::power_off()
{
	emit sig_close_window();
}

