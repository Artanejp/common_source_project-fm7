/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ config ]
*/
#if defined(_USE_QT)
#include <string>
#include <vector>
#include "fileio.h"
#include "csp_logger.h"
#include "qt_main.h"
# if defined(Q_OS_WIN)
# include <windows.h>
# endif

extern CSP_Logger *csp_logger;
#endif

#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "config.h"
#include "fileio.h"
#if defined(_USE_AGAR)
#include "agar_main.h"
#endif

config_t DLL_PREFIX config;

#ifndef CONFIG_NAME
#define CONFIG_NAME "conf"
#endif

BOOL MyWritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int Value, LPCTSTR lpFileName)
{
	return MyWritePrivateProfileString(lpAppName, lpKeyName, create_string(_T("%d"), Value), lpFileName);
}
 
BOOL MyWritePrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool Value, LPCTSTR lpFileName)
{
	return MyWritePrivateProfileString(lpAppName, lpKeyName, create_string(_T("%d"), Value ? 1 : 0), lpFileName);
}
 
bool MyGetPrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool bDefault, LPCTSTR lpFileName)
{
	return (MyGetPrivateProfileInt(lpAppName, lpKeyName, bDefault ? 1 : 0, lpFileName) != 0);
}

void DLL_PREFIX initialize_config()
{
	// initial settings
	memset(&config, 0, sizeof(config_t));
	config.window_mode = 1;	
	// memo: set only non zero value
	config.full_speed = false;	
	
	// control
	#if defined(USE_BOOT_MODE) && defined(BOOT_MODE_DEFAULT)
		config.boot_mode = BOOT_MODE_DEFAULT;
	#endif
	#if defined(USE_CPU_TYPE) && defined(CPU_TYPE_DEFAULT)
		config.cpu_type = CPU_TYPE_DEFAULT;
	#endif
	#if defined(USE_DIPSWITCH) && defined(DIPSWITCH_DEFAULT)
		config.dipswitch = DIPSWITCH_DEFAULT;
	#endif
	#if defined(USE_DEVICE_TYPE) && defined(DEVICE_TYPE_DEFAULT)
		config.device_type = DEVICE_TYPE_DEFAULT;
	#endif
	#if defined(USE_DRIVE_TYPE) && defined(DRIVE_TYPE_DEFAULT)
		config.drive_type = DRIVE_TYPE_DEFAULT;
	#endif
	#if defined(USE_KEYBOARD_TYPE) && defined(KEYBOARD_TYPE_DEFAULT)
		config.keyboard_type = KEYBOARD_TYPE_DEFAULT;
	#endif
	#if defined(USE_MOUSE_TYPE) && defined(MOUSE_TYPE_DEFAULT)
		config.mouse_type = MOUSE_TYPE_DEFAULT;
	#endif
	#if defined(USE_JOYSTICK_TYPE) && defined(JOYSTICK_TYPE_DEFAULT)
		config.joystick_type = JOYSTICK_TYPE_DEFAULT;
	#endif
	#if defined(USE_SOUND_TYPE) && defined(SOUND_TYPE_DEFAULT)
		config.sound_type = SOUND_TYPE_DEFAULT;
	#endif
	#if defined(USE_MONITOR_TYPE) && defined(MONITOR_TYPE_DEFAULT)
		config.monitor_type = MONITOR_TYPE_DEFAULT;
	#endif
	#if defined(USE_PRINTER_TYPE) && defined(PRINTER_TYPE_DEFAULT)
		config.printer_type = PRINTER_TYPE_DEFAULT;
	#endif
	#if defined(USE_FLOPPY_DISK)
		for(int drv = 0; drv < USE_FLOPPY_DISK; drv++) {
			#if defined(CORRECT_DISK_TIMING_DEFAULT)
				config.correct_disk_timing[drv] = CORRECT_DISK_TIMING_DEFAULT;
			#else
				config.correct_disk_timing[drv] = false;
			#endif
			#if defined(IGNORE_DISK_CRC_DEFAULT)
				config.ignore_disk_crc[drv] = IGNORE_DISK_CRC_DEFAULT;
			#else
				config.ignore_disk_crc[drv] = false;
			#endif
		}
	#endif
	#if defined(USE_TAPE)
		for(int drv = 0; drv < USE_TAPE; drv++) {
			config.wave_shaper[drv] = true;
			config.direct_load_mzt[drv] = true;
			config.baud_high[drv] = true;
		}
	#endif
	#ifdef USE_COMPACT_DISC
		for(int drv = 0; drv < USE_COMPACT_DISC_TMP; drv++) {
			config.swap_audio_byteorder[drv] = false;
		}
	#endif
		
	config.compress_state = true;
	
	// screen
	#ifndef ONE_BOARD_MICRO_COMPUTER
		config.fullscreen_stretch_type = 1;	// Stretch (Aspect)
	#endif

	// sound
	#ifdef SOUND_RATE_DEFAULT
		config.sound_frequency = SOUND_RATE_DEFAULT;
	#else
		config.sound_frequency = 6;	// 48KHz
	#endif
		config.sound_latency = 1;	// 100msec
		config.sound_strict_rendering = true;
		config.sound_device_num = 0; // Add 20190221
	#ifdef USE_FLOPPY_DISK
		config.sound_noise_fdd = true;
	#endif
	#ifdef USE_TAPE
		config.sound_noise_cmt = true;
		config.sound_play_tape = true;
	#endif
	// input
	#ifdef USE_JOYSTICK
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 16; j++) {
				config.joy_buttons[i][j] = (i << 5) | j;
			}
		}
		config.use_joy_to_key = false;
		config.joy_to_key_type = 2;
		config.joy_to_key_numpad5 = false;
		config.joy_to_key_buttons[0] = -('Z');
		config.joy_to_key_buttons[1] = -('X');
	#endif
	#if defined(USE_VARIABLE_MEMORY)
		config.current_ram_size = USE_VARIABLE_MEMORY;
	#endif
	// debug
	config.special_debug_fdc = false;
	config.print_statistics = false;
	
	// win32
	#if defined(_WIN32) && !defined(_USE_QT)
		#ifndef ONE_BOARD_MICRO_COMPUTER
			config.use_d3d9 = true;
		#endif
		config.use_dinput = true;
		config.show_status_bar = true;
	#endif
	
	// qt
	#ifdef _USE_QT
		config.use_separate_thread_draw = true;
		config.use_osd_virtual_media = true;		
		config.render_platform = CONFIG_RENDER_PLATFORM_OPENGL_ES;
		config.render_major_version = 2; // For crash with some devices.
		config.render_minor_version = 0;
		config.rendering_type = CONFIG_RENDER_TYPE_STD;
		
		config.use_opengl_scanline = false;
		config.opengl_scanline_vert = false;
		config.opengl_scanline_horiz = false;
		config.use_opengl_filters = false;
		config.opengl_filter_num = 0;
		config.focus_with_click = false;
		
		config.video_width = 640;
		config.video_height = 480;
		config.video_codec_type = 0; // MPEG4
	
		config.video_h264_bitrate = 512;
		config.video_h264_bframes = 4;
		config.video_h264_b_adapt = 2;
		config.video_h264_minq = 14;
		config.video_h264_maxq = 25;
		config.video_h264_subme = 8;
		
		config.video_mpeg4_bitrate = 512;
		config.video_mpeg4_bframes = 4;
		config.video_mpeg4_minq = 1;
		config.video_mpeg4_maxq = 20;
		
		config.audio_codec_type = 0;
		config.video_threads = 0;
		config.audio_bitrate = 160;
		config.video_frame_rate = 30;
		config.log_to_syslog = false;
		config.log_to_console = true;
		for(int ii = 0; ii < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1) ; ii++) {
			for(int jj = 0; jj < 8; jj++) {
				config.dev_log_to_syslog[ii][jj] = true;
				config.dev_log_to_console[ii][jj] = true;
				config.dev_log_recording[ii][jj] = true;
			}
		}
		config.state_log_to_console = false;
		config.state_log_to_syslog = false;
		config.state_log_to_recording = false; 
		
		config.rendering_type = CONFIG_RENDER_TYPE_STD;
		config.virtual_media_position = 2; // Down.
		for(int drv = 0; drv < 16; drv++) {
			config.disk_count_immediate[drv] = false;
		}
		// Extra UI
		config.cursor_as_ten_key = CONFIG_CURSOR_AS_CURSOR;
		memset(config.debugwindow_font, 0x00, sizeof(config.debugwindow_font));
		memset(config.logwindow_font, 0x00, sizeof(config.logwindow_font));
		
	#if defined(_FM7) || defined(_FMNEW7) || defined(_FM8) \
	    || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)		
		config.numpad_enter_as_fullkey = false;
	#else
		config.numpad_enter_as_fullkey = true;
	#endif
		config.host_keyboard_type = CONFIG_HOST_KEYBOARD_AT_109JP;
#endif	
}

void DLL_PREFIX load_config(const _TCHAR *config_path)
{
	int drv, i;
	// initial settings
	initialize_config();
	
	// control
	#ifdef USE_BOOT_MODE
		config.boot_mode = MyGetPrivateProfileInt(_T("Control"), _T("BootMode"), config.boot_mode, config_path);
	#endif
	#ifdef USE_CPU_TYPE
		config.cpu_type = MyGetPrivateProfileInt(_T("Control"), _T("CPUType"), config.cpu_type, config_path);
	#endif
	#ifdef USE_DIPSWITCH
		config.dipswitch = MyGetPrivateProfileInt(_T("Control"), _T("DipSwitch"), config.dipswitch, config_path);
	#endif
	#ifdef USE_DEVICE_TYPE
		config.device_type = MyGetPrivateProfileInt(_T("Control"), _T("DeviceType"), config.device_type, config_path);
	#endif
	#ifdef USE_DRIVE_TYPE
		config.drive_type = MyGetPrivateProfileInt(_T("Control"), _T("DriveType"), config.drive_type, config_path);
	#endif
	#ifdef USE_KEYBOARD_TYPE
		config.keyboard_type = MyGetPrivateProfileInt(_T("Control"), _T("KeyboardType"), config.keyboard_type, config_path);
	#endif
	#ifdef USE_MOUSE_TYPE
		config.mouse_type = MyGetPrivateProfileInt(_T("Control"), _T("MouseType"), config.mouse_type, config_path);
	#endif
	#ifdef USE_JOYSTICK_TYPE
		config.joystick_type = MyGetPrivateProfileInt(_T("Control"), _T("JoystickType"), config.joystick_type, config_path);
	#endif
	#ifdef USE_SOUND_TYPE
		config.sound_type = MyGetPrivateProfileInt(_T("Control"), _T("SoundType"), config.sound_type, config_path);
	#endif
	#ifdef USE_MONITOR_TYPE
		config.monitor_type = MyGetPrivateProfileInt(_T("Control"), _T("MonitorType"), config.monitor_type, config_path);
	#endif
	#ifdef USE_SCANLINE
		config.scan_line = MyGetPrivateProfileBool(_T("Control"), _T("ScanLine"), config.scan_line, config_path);
	#endif
	#ifdef USE_PRINTER
		config.printer_type = MyGetPrivateProfileInt(_T("Control"), _T("PrinterType"), config.printer_type, config_path);
	#endif
	#if defined(USE_VARIABLE_MEMORY)
		config.current_ram_size = MyGetPrivateProfileInt(_T("Control"), _T("CurrentRAMSize"), config.current_ram_size, config_path);
	#endif
	#ifdef USE_FLOPPY_DISK
		for(int drv = 0; drv < USE_FLOPPY_DISK; drv++) {
			config.correct_disk_timing[drv] = MyGetPrivateProfileBool(_T("Control"), create_string(_T("CorrectDiskTiming%d"), drv + 1), config.correct_disk_timing[drv], config_path);
			config.ignore_disk_crc[drv] = MyGetPrivateProfileBool(_T("Control"), create_string(_T("IgnoreDiskCRC%d"), drv + 1), config.ignore_disk_crc[drv], config_path);
		#ifdef _USE_QT
			config.disk_count_immediate[drv] = MyGetPrivateProfileBool(_T("Control"), create_string(_T("DiskIncrementImmediate%d"), drv + 1), config.disk_count_immediate[drv], config_path);
		#endif
		}
	#endif
	#ifdef USE_TAPE
		for(int drv = 0; drv < USE_TAPE; drv++) {
			config.wave_shaper[drv] = MyGetPrivateProfileBool(_T("Control"), create_string(_T("WaveShaper%d"), drv + 1), config.wave_shaper[drv], config_path);
			config.direct_load_mzt[drv] = MyGetPrivateProfileBool(_T("Control"), create_string(_T("DirectLoadMZT%d"), drv + 1), config.direct_load_mzt[drv], config_path);
			config.baud_high[drv] = MyGetPrivateProfileBool(_T("Control"), create_string(_T("BaudHigh%d"), drv + 1), config.baud_high[drv], config_path);
		}
	#endif
	config.compress_state = MyGetPrivateProfileBool(_T("Control"), _T("CompressState"), config.compress_state, config_path);		
		// recent files
	#ifdef USE_CART
		MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialCartDir"), _T(""), config.initial_cart_dir, _MAX_PATH, config_path);
		for(int drv = 0; drv < USE_CART; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentCartPath%d_%d"), drv + 1, i + 1), _T(""), config.recent_cart_path[drv][i], _MAX_PATH, config_path);
			}
		}
	#endif
	#ifdef USE_FLOPPY_DISK
		MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), _T(""), config.initial_floppy_disk_dir, _MAX_PATH, config_path);
		for(int drv = 0; drv < USE_FLOPPY_DISK; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentDiskPath%d_%d"), drv + 1, i + 1), _T(""), config.recent_floppy_disk_path[drv][i], _MAX_PATH, config_path);
			}
		}
	#endif
	#ifdef USE_QUICK_DISK
		MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"), _T(""), config.initial_quick_disk_dir, _MAX_PATH, config_path);
		for(int drv = 0; drv < USE_QUICK_DISK; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1), _T(""), config.recent_quick_disk_path[drv][i], _MAX_PATH, config_path);
			}
		}
	#endif
	#ifdef USE_HARD_DISK
		MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialHardDiskDir"), _T(""), config.initial_hard_disk_dir, _MAX_PATH, config_path);
		for(int drv = 0; drv < USE_HARD_DISK; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentHardDiskPath%d_%d"), drv + 1, i + 1), _T(""), config.recent_hard_disk_path[drv][i], _MAX_PATH, config_path);
			}
			MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("LastHardDiskPath%d"), drv + 1), _T(""), config.last_hard_disk_path[drv], _MAX_PATH, config_path);
		}
	#endif
	#ifdef USE_TAPE
		MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), _T(""), config.initial_tape_dir, _MAX_PATH, config_path);
		for(int drv = 0; drv < USE_TAPE; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentTapePath%d_%d"), drv + 1, i + 1), _T(""), config.recent_tape_path[drv][i], _MAX_PATH, config_path);
			}
		}
	#endif
	#ifdef USE_COMPACT_DISC
		MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialCompactDiscDir"), _T(""), config.initial_compact_disc_dir, _MAX_PATH, config_path);
		for(int drv = 0; drv < USE_COMPACT_DISC; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentCompactDiscPath%d_%d"), drv + 1, i + 1), _T(""), config.recent_compact_disc_path[drv][i], _MAX_PATH, config_path);
			}
		}
	#endif
	#ifdef USE_LASER_DISC
		MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), _T(""), config.initial_laser_disc_dir, _MAX_PATH, config_path);
		for(int drv = 0; drv < USE_LASER_DISC; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentLaserDiscPath%d_%d"), drv + 1, i + 1), _T(""), config.recent_laser_disc_path[drv][i], _MAX_PATH, config_path);
			}
		}
	#endif
	#ifdef USE_BINARY_FILE
		MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), _T(""), config.initial_binary_dir, _MAX_PATH, config_path);
		for(int drv = 0; drv < USE_BINARY_FILE; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentBinaryPath%d_%d"), drv + 1, i + 1), _T(""), config.recent_binary_path[drv][i], _MAX_PATH, config_path);
			}
		}
	#endif
	#ifdef USE_BUBBLE
		MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialBubbleDir"), _T(""), config.initial_bubble_casette_dir, _MAX_PATH, config_path);
		for(int drv = 0; drv < USE_BUBBLE; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyGetPrivateProfileString(_T("RecentFiles"), create_string(_T("RecentBubblePath%d_%d"), drv + 1, i + 1), _T(""), config.recent_bubble_casette_path[drv][i], _MAX_PATH, config_path);
			}
		}
	#endif
	
	// screen
	#ifndef ONE_BOARD_MICRO_COMPUTER
		config.window_mode = MyGetPrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
		config.window_stretch_type = MyGetPrivateProfileInt(_T("Screen"), _T("WindowStretchType"), config.window_stretch_type, config_path);
		config.fullscreen_stretch_type = MyGetPrivateProfileInt(_T("Screen"), _T("FullScreenStretchType"), config.fullscreen_stretch_type, config_path);
//		#ifdef USE_SCREEN_ROTATE
			config.rotate_type = MyGetPrivateProfileInt(_T("Screen"), _T("RotateType"), config.rotate_type, config_path);
//		#endif
	#endif

	
	// filter
	#ifdef USE_SCREEN_FILTER
		config.filter_type = MyGetPrivateProfileInt(_T("Screen"), _T("FilterType"), config.filter_type, config_path);
	#endif

	// sound
	config.sound_frequency = MyGetPrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
	config.sound_latency = MyGetPrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
	config.sound_strict_rendering = MyGetPrivateProfileInt(_T("Sound"), _T("StrictRendering"), config.sound_strict_rendering, config_path);
	config.sound_device_num = MyGetPrivateProfileBool(_T("Sound"), _T("DeviceNumber"), config.sound_device_num, config_path);
	#ifdef USE_FLOPPY_DISK
		config.sound_noise_fdd = MyGetPrivateProfileBool(_T("Sound"), _T("NoiseFDD"), config.sound_noise_fdd, config_path);;
	#endif
	#ifdef USE_TAPE
		config.sound_noise_cmt = MyGetPrivateProfileBool(_T("Sound"), _T("NoiseCMT"), config.sound_noise_cmt, config_path);;
		config.sound_play_tape = MyGetPrivateProfileBool(_T("Sound"), _T("PlayTape"), config.sound_play_tape, config_path);
	#endif
	#ifdef USE_COMPACT_DISC
		for(int drv = 0; drv < USE_COMPACT_DISC; drv++) {
			config.swap_audio_byteorder[drv] = MyGetPrivateProfileBool(_T("Sound"), create_string(_T("SwapCDByteOrder%d"), drv + 1), config.swap_audio_byteorder[drv], config_path);
		}
	#endif
	#ifdef USE_SOUND_VOLUME
		for(int i = 0; i < USE_SOUND_VOLUME; i++) {
			int tmp_l = MyGetPrivateProfileInt(_T("Sound"), create_string(_T("VolumeLeft%d"), i + 1), config.sound_volume_l[i], config_path);
			int tmp_r = MyGetPrivateProfileInt(_T("Sound"), create_string(_T("VolumeRight%d"), i + 1), config.sound_volume_r[i], config_path);
			#ifdef _USE_QT
				// Note: when using balance , levels are -40±20db to 0±20db.
				config.sound_volume_l[i] = max(-60, min(20, tmp_l));
				config.sound_volume_r[i] = max(-60, min(20, tmp_r));
			#else
				config.sound_volume_l[i] = max(-40, min(0, tmp_l));
				config.sound_volume_r[i] = max(-40, min(0, tmp_r));
			#endif
		}
	#endif
	
	#if defined(_WIN32) && !defined(_USE_QT)
		// for compatibilities
		#ifdef _X1_H_
			MyGetPrivateProfileString(_T("Sound"), _T("FMGenDll"), _T("mame2151.dll"), config.mame2151_dll_path, _MAX_PATH, config_path);
			my_tcscpy_s(config.mame2608_dll_path, _MAX_PATH, _T("mamefm.dll"));
		#else
			MyGetPrivateProfileString(_T("Sound"), _T("FMGenDll"), _T("mamefm.dll"), config.mame2608_dll_path, _MAX_PATH, config_path);
			my_tcscpy_s(config.mame2151_dll_path, _MAX_PATH, _T("mame2151.dll"));
		#endif
		MyGetPrivateProfileString(_T("Sound"), _T("YM2151GenDll"), config.mame2151_dll_path, config.mame2151_dll_path, _MAX_PATH, config_path);
		MyGetPrivateProfileString(_T("Sound"), _T("YM2608GenDll"), config.mame2608_dll_path, config.mame2608_dll_path, _MAX_PATH, config_path);
 	#endif
	// input
	#ifdef USE_JOYSTICK
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 16; j++) {
				int old = (i << 4) | j;
				old = MyGetPrivateProfileInt(_T("Input"), create_string(_T("JoyButtons%d_%d"), i + 1, j + 1), old, config_path);
				old = ((old >> 4) << 5) | (old & 0x0f);
				config.joy_buttons[i][j] = MyGetPrivateProfileInt(_T("Input"), create_string(_T("JoyButtonsEx%d_%d"), i + 1, j + 1), old, config_path);
 			}
 		}
		config.use_joy_to_key = MyGetPrivateProfileBool(_T("Input"), _T("UseJoyToKey"), config.use_joy_to_key, config_path);
		config.joy_to_key_type = MyGetPrivateProfileInt(_T("Input"), _T("JoyToKeyType"), config.joy_to_key_type, config_path);
		config.joy_to_key_numpad5 = MyGetPrivateProfileBool(_T("Input"), _T("JoyToKeyNumPad5"), config.joy_to_key_numpad5, config_path);
		for(int i = 0; i < 16; i++) {
			config.joy_to_key_buttons[i] = MyGetPrivateProfileInt(_T("Input"), create_string(_T("JoyToKeyButtons%d"), i + 1), config.joy_to_key_buttons[i], config_path);
		}
	#endif
	// debug
	#ifdef USE_FLOPPY_DISK
		config.special_debug_fdc =	MyGetPrivateProfileInt(_T("Debug"), _T("SpecialDebugFDC"), config.special_debug_fdc, config_path);
	#endif
		config.print_statistics = MyGetPrivateProfileBool(_T("Debug"), _T("PrintCPUStatistics"), config.print_statistics, config_path);
	// printer
	#ifdef USE_PRINTER
		MyGetPrivateProfileString(_T("Printer"), _T("PrinterDll"), _T("printer.dll"), config.printer_dll_path, _MAX_PATH, config_path);
	#endif

	// win32
	#if defined(_WIN32) && !defined(_USE_QT)
		#ifndef ONE_BOARD_MICRO_COMPUTER
			config.use_d3d9 = MyGetPrivateProfileBool(_T("Win32"), _T("UseDirect3D9"), config.use_d3d9, config_path);
			config.wait_vsync = MyGetPrivateProfileBool(_T("Win32"), _T("WaitVSync"), config.wait_vsync, config_path);
		#endif
		config.use_dinput = MyGetPrivateProfileBool(_T("Win32"), _T("UseDirectInput"), config.use_dinput, config_path);
		config.disable_dwm = MyGetPrivateProfileBool(_T("Win32"), _T("DisableDwm"), config.disable_dwm, config_path);
		config.show_status_bar = MyGetPrivateProfileBool(_T("Win32"), _T("ShowStatusBar"), config.show_status_bar, config_path);
	#endif

	// qt
	#ifdef _USE_QT
		config.use_separate_thread_draw = MyGetPrivateProfileBool(_T("Qt"), _T("UseSeparateThreadDraw"), config.use_separate_thread_draw, config_path);
		config.use_osd_virtual_media = MyGetPrivateProfileBool(_T("Qt"), _T("UseOSDVirtualMedia"), config.use_osd_virtual_media, config_path); 
		config.use_opengl_scanline = MyGetPrivateProfileBool(_T("Qt"), _T("UseOpenGLScanLine"), config.use_opengl_scanline, config_path);
		config.opengl_scanline_vert = MyGetPrivateProfileBool(_T("Qt"), _T("OpenGLScanLineVert"), config.opengl_scanline_vert, config_path);;
		config.opengl_scanline_horiz = MyGetPrivateProfileBool(_T("Qt"), _T("OpenGLScanLineHoriz"), config.opengl_scanline_horiz, config_path);;
		config.use_opengl_filters = MyGetPrivateProfileBool(_T("Qt"), _T("UseOpenGLFilters"), config.use_opengl_filters, config_path);
		config.opengl_filter_num = MyGetPrivateProfileInt(_T("Qt"), _T("OpenGLFilterNum"), config.opengl_filter_num, config_path);
		config.render_platform = MyGetPrivateProfileInt(_T("Qt"), _T("RenderPlatform"), config.render_platform, config_path);
		config.render_major_version = MyGetPrivateProfileInt(_T("Qt"), _T("RenderMajorVersion"), config.render_major_version, config_path);
		config.render_minor_version = MyGetPrivateProfileInt(_T("Qt"), _T("RenderMinorVersion"), config.render_minor_version, config_path);
		config.rendering_type = MyGetPrivateProfileInt(_T("Qt"), _T("RenderType"), config.rendering_type, config_path);

		config.general_sound_level = MyGetPrivateProfileInt(_T("Qt"), _T("GeneralSoundLevel"), config.general_sound_level, config_path);
		config.focus_with_click = MyGetPrivateProfileBool(_T("Qt"), _T("FocusWithClick"), config.focus_with_click, config_path);
		
		if(config.rendering_type < 0) config.rendering_type = 0;
		if(config.rendering_type >= CONFIG_RENDER_TYPE_END) config.rendering_type = CONFIG_RENDER_TYPE_END - 1;

		// Extra UI
		config.swap_kanji_pause = MyGetPrivateProfileBool(_T("Qt"), _T("SwapKanjiPause"), config.swap_kanji_pause, config_path);
		config.cursor_as_ten_key = MyGetPrivateProfileInt(_T("Qt"), _T("CursorAsTenKey"), config.cursor_as_ten_key, config_path);
		config.numpad_enter_as_fullkey = MyGetPrivateProfileBool(_T("Qt"), _T("NumpadEnterAsFullKey"), config.numpad_enter_as_fullkey, config_path);
		config.host_keyboard_type = MyGetPrivateProfileInt(_T("Qt"), _T("HostKeyboardType"), config.host_keyboard_type, config_path);
		MyGetPrivateProfileString(_T("Qt"), _T("DebugWindowFont"), _T("Sans"), config.debugwindow_font, sizeof(config.debugwindow_font) - 1, config_path);
		config.debugwindow_width = MyGetPrivateProfileInt(_T("Qt"), _T("DebugWindowWidth"), 800, config_path);
		config.debugwindow_height = MyGetPrivateProfileInt(_T("Qt"), _T("DebugWindowHeight"), 500, config_path);
		MyGetPrivateProfileString(_T("Qt"), _T("LogWindowFont"), _T("Sans"), config.logwindow_font, sizeof(config.logwindow_font) - 1, config_path);
		config.logwindow_width = MyGetPrivateProfileInt(_T("Qt"), _T("LogWindowWidth"), 800, config_path);
		config.logwindow_height = MyGetPrivateProfileInt(_T("Qt"), _T("LogWindowHeight"), 500, config_path);
		// Assigning joysticks.
		for(i = 0; i < 16; i++) {
			_TCHAR name[256];
			my_stprintf_s(name, 255, _T("AssignedJoystick%d"), i + 1);
			MyGetPrivateProfileString(_T("Qt"), (const _TCHAR *)name, _T(""),
									  config.assigned_joystick_name[i], 255, config_path);
//			printf("%d->%s\n", i, config.assigned_joystick_name[i]);
		}
		// Movie load/save.
		config.video_width   = MyGetPrivateProfileInt(_T("Qt"), _T("VideoWidth"), 640, config_path);
		if(config.video_width < 128) config.video_width = 128;
		config.video_height  = MyGetPrivateProfileInt(_T("Qt"), _T("VideoHeight"), 480, config_path);
		if(config.video_height < 80) config.video_height = 80;
		
		config.video_codec_type = MyGetPrivateProfileInt(_T("Qt"), _T("VideoCodecType"), 1, config_path);
		if(config.video_codec_type > 1) config.video_codec_type = 1;
		if(config.video_codec_type < 0) config.video_codec_type = 0;
		
		config.audio_codec_type = MyGetPrivateProfileInt(_T("Qt"), _T("AudioCodecType"), 0, config_path);
		if(config.video_codec_type > 2) config.audio_codec_type = 2;
		if(config.video_codec_type < 0) config.audio_codec_type = 0;
		
		config.video_h264_bitrate = MyGetPrivateProfileInt(_T("Qt"), _T("H264Bitrate"), 3500, config_path);
		if(config.video_h264_bitrate < 64) config.video_h264_bitrate = 64;
		
		config.video_h264_bframes = MyGetPrivateProfileInt(_T("Qt"), _T("H264BFrames"), 4, config_path);
		if(config.video_h264_bframes < 0) config.video_h264_bframes = 0;
		if(config.video_h264_bframes > 10) config.video_h264_bframes = 10;
		
		config.video_h264_b_adapt = MyGetPrivateProfileInt(_T("Qt"), _T("H264BAdapt"), 2, config_path);
		if(config.video_h264_b_adapt < 0) config.video_h264_b_adapt = 0;
		if(config.video_h264_b_adapt > 2) config.video_h264_b_adapt = 2;
		
		config.video_h264_subme   = MyGetPrivateProfileInt(_T("Qt"), _T("H264Subme"), 7, config_path);
		if(config.video_h264_subme < 0) config.video_h264_subme = 0;
		if(config.video_h264_subme > 11) config.video_h264_subme = 11;
		
		config.video_h264_minq   = MyGetPrivateProfileInt(_T("Qt"), _T("H264MinQ"), 15, config_path);
		if(config.video_h264_minq < 0) config.video_h264_minq = 0;
		if(config.video_h264_minq > 63) config.video_h264_minq = 63;
		
		config.video_h264_maxq   = MyGetPrivateProfileInt(_T("Qt"), _T("H264MaxQ"), 28, config_path);
		if(config.video_h264_maxq < 0) config.video_h264_maxq = 0;
		if(config.video_h264_maxq > 63) config.video_h264_maxq = 63;
		
		config.video_mpeg4_bitrate = MyGetPrivateProfileInt(_T("Qt"), _T("MPEG4Bitrate"), 1500, config_path);
		if(config.video_mpeg4_bitrate < 64) config.video_mpeg4_bitrate = 64;
		
		config.video_mpeg4_bframes = MyGetPrivateProfileInt(_T("Qt"), _T("MPEG4BFrames"), 2, config_path);
		if(config.video_mpeg4_bframes < 0) config.video_mpeg4_bframes = 0;
		if(config.video_mpeg4_bframes > 10) config.video_mpeg4_bframes = 10;
		
		config.video_mpeg4_minq   = MyGetPrivateProfileInt(_T("Qt"), _T("MPEG4MinQ"), 1, config_path);
		if(config.video_mpeg4_minq < 1) config.video_mpeg4_minq = 1;
		if(config.video_mpeg4_minq > 31) config.video_mpeg4_minq = 31;
		
		config.video_mpeg4_maxq   = MyGetPrivateProfileInt(_T("Qt"), _T("MPEG4MaxQ"), 15, config_path);
		if(config.video_mpeg4_maxq < 1) config.video_mpeg4_maxq = 1;
		if(config.video_mpeg4_maxq > 31) config.video_mpeg4_maxq = 31;
		if(config.video_mpeg4_maxq < config.video_mpeg4_minq) {
			int n;
			n = config.video_mpeg4_maxq;
			config.video_mpeg4_maxq  = config.video_mpeg4_minq;
			config.video_mpeg4_minq = n;
		}
		
		config.video_threads = MyGetPrivateProfileInt(_T("Qt"), _T("VideoThreads"), 0, config_path);
		if(config.video_threads < 0) config.video_threads = 0;
		if(config.video_threads > 16) config.video_threads = 16;
		
		config.audio_bitrate = MyGetPrivateProfileInt(_T("Qt"), _T("AudioBitrate"), 224, config_path);
		if(config.audio_bitrate < 16) config.audio_bitrate = 16;
		if(config.audio_bitrate > 448) config.audio_bitrate = 448;
		
		config.video_frame_rate = MyGetPrivateProfileInt(_T("Qt"), _T("VideoFramerate"), 60, config_path);
		if(config.video_frame_rate < 15) config.video_frame_rate = 15;
		if(config.video_frame_rate > 75) config.video_frame_rate = 75;
		// Logging
		config.log_to_syslog = MyGetPrivateProfileBool(_T("Qt"), _T("WriteToSyslog"), config.log_to_syslog, config_path);
		config.log_to_console = MyGetPrivateProfileBool(_T("Qt"), _T("WriteToConsole"), config.log_to_console, config_path);
		
		for(int ii = 0; ii < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1) ; ii++) {
			uint32_t flags = 0;
			flags = MyGetPrivateProfileInt(_T("Qt"), create_string(_T("SyslogEnabled%d"), ii), 0xffff, config_path);
			for(int jj = 0; jj < 8; jj++) {
				config.dev_log_to_syslog[ii][jj] = ((flags & 0x0080) != 0) ? true : false;
				flags <<= 1;
			}
			flags = 0;
			flags = MyGetPrivateProfileInt(_T("Qt"), create_string(_T("ConsoleLogEnabled%d"), ii), 0xffff, config_path);
			for(int jj = 0; jj < 8; jj++) {
				config.dev_log_to_console[ii][jj] = ((flags & 0x0080) != 0) ? true : false;
				flags <<= 1;
			}
			flags = MyGetPrivateProfileInt(_T("Qt"), create_string(_T("RecordLogEnabled%d"), ii), 0xffff, config_path);
			for(int jj = 0; jj < 8; jj++) {
				config.dev_log_recording[ii][jj] = ((flags & 0x0080) != 0) ? true : false;
				flags <<= 1;
			}
		}
		config.state_log_to_console = MyGetPrivateProfileBool(_T("Qt"), _T("StateLogToConsole"), config.state_log_to_console, config_path);;
		config.state_log_to_syslog = MyGetPrivateProfileBool(_T("Qt"), _T("StateLogToSyslog"), config.state_log_to_syslog, config_path);;
		config.state_log_to_recording = MyGetPrivateProfileBool(_T("Qt"), _T("StateLogToRecording"), config.state_log_to_recording, config_path);;

		config.virtual_media_position = MyGetPrivateProfileInt(_T("Qt"), _T("UiVirtualMediaPosition"), config.virtual_media_position, config_path);
		//csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Read config done.");
	#endif
}
		
void DLL_PREFIX save_config(const _TCHAR *config_path)
{
	int drv, i;
#if !defined(_MSC_VER)
	{
		FILEIO *pt = new FILEIO;
		if(pt->Fopen(config_path, FILEIO_WRITE_ASCII) != true) {
			delete pt;
			return;
		}
		pt->Fclose();
		delete pt;
	}
	
#endif	
	// control
	#ifdef USE_BOOT_MODE
		MyWritePrivateProfileInt(_T("Control"), _T("BootMode"), config.boot_mode, config_path);
	#endif
	#ifdef USE_CPU_TYPE
		MyWritePrivateProfileInt(_T("Control"), _T("CPUType"), config.cpu_type, config_path);
	#endif
	#ifdef USE_DIPSWITCH
		MyWritePrivateProfileInt(_T("Control"), _T("DipSwitch"), config.dipswitch, config_path);
	#endif
	#ifdef USE_DEVICE_TYPE
		MyWritePrivateProfileInt(_T("Control"), _T("DeviceType"), config.device_type, config_path);
	#endif
	#ifdef USE_DRIVE_TYPE
		MyWritePrivateProfileInt(_T("Control"), _T("DriveType"), config.drive_type, config_path);
	#endif
	#ifdef USE_KEYBOARD_TYPE
		MyWritePrivateProfileInt(_T("Control"), _T("KeyboardType"), config.keyboard_type, config_path);
	#endif
	#ifdef USE_MOUSE_TYPE
		MyWritePrivateProfileInt(_T("Control"), _T("MouseType"), config.mouse_type, config_path);
	#endif
	#ifdef USE_JOYSTICK_TYPE
		MyWritePrivateProfileInt(_T("Control"), _T("JoystickType"), config.joystick_type, config_path);
	#endif
	#ifdef USE_SOUND_TYPE
		MyWritePrivateProfileInt(_T("Control"), _T("SoundType"), config.sound_type, config_path);
	#endif
	#ifdef USE_MONITOR_TYPE
		MyWritePrivateProfileInt(_T("Control"), _T("MonitorType"), config.monitor_type, config_path);
	#endif
	#ifdef USE_SCANLINE
		MyWritePrivateProfileBool(_T("Control"), _T("ScanLine"), config.scan_line, config_path);
	#endif
	#ifdef USE_PRINTER
		MyWritePrivateProfileInt(_T("Control"), _T("PrinterType"), config.printer_type, config_path);
	#endif
	#if defined(USE_VARIABLE_MEMORY)
		MyWritePrivateProfileInt(_T("Control"), _T("CurrentRAMSize"), config.current_ram_size, config_path);
	#endif
	#ifdef USE_FLOPPY_DISK
		for(int drv = 0; drv < USE_FLOPPY_DISK; drv++) {
			MyWritePrivateProfileBool(_T("Control"), create_string(_T("CorrectDiskTiming%d"), drv + 1), config.correct_disk_timing[drv], config_path);
			MyWritePrivateProfileBool(_T("Control"), create_string(_T("IgnoreDiskCRC%d"), drv + 1), config.ignore_disk_crc[drv], config_path);
		#ifdef _USE_QT
			MyWritePrivateProfileBool(_T("Control"), create_string(_T("DiskIncrementImmediate%d"), drv + 1), config.disk_count_immediate[drv], config_path);
		#endif
		}
	#endif
	#ifdef USE_TAPE
		for(int drv = 0; drv < USE_TAPE; drv++) {
			MyWritePrivateProfileBool(_T("Control"), create_string(_T("WaveShaper%d"), drv + 1), config.wave_shaper[drv], config_path);
			MyWritePrivateProfileBool(_T("Control"), create_string(_T("DirectLoadMZT%d"), drv + 1), config.direct_load_mzt[drv], config_path);
			MyWritePrivateProfileBool(_T("Control"), create_string(_T("BaudHigh%d"), drv + 1), config.baud_high[drv], config_path);
		}
	#endif
	MyWritePrivateProfileBool(_T("Control"), _T("CompressState"), config.compress_state, config_path);
	
 	// recent files
	
	// recent files
	#ifdef USE_CART
		MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialCartDir"), config.initial_cart_dir, config_path);
		for(int drv = 0; drv < USE_CART; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentCartPath%d_%d"), drv + 1, i + 1), config.recent_cart_path[drv][i], config_path);
			}
		}
	#endif
	#ifdef USE_FLOPPY_DISK
		MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), config.initial_floppy_disk_dir, config_path);
		for(int drv = 0; drv < USE_FLOPPY_DISK; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentDiskPath%d_%d"), drv + 1, i + 1), config.recent_floppy_disk_path[drv][i], config_path);
			}
		}
	#endif
	#ifdef USE_QUICK_DISK
		MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"), config.initial_quick_disk_dir, config_path);
		for(int drv = 0; drv < USE_QUICK_DISK; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1), config.recent_quick_disk_path[drv][i], config_path);
			}
		}
	#endif
	#ifdef USE_HARD_DISK
		MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialHardDiskDir"), config.initial_hard_disk_dir, config_path);
		for(int drv = 0; drv < USE_HARD_DISK; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentHardDiskPath%d_%d"), drv + 1, i + 1), config.recent_hard_disk_path[drv][i], config_path);
			}
			MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("LastHardDiskPath%d"), drv + 1), config.last_hard_disk_path[drv], config_path);
		}
	#endif
	#ifdef USE_TAPE
 		MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), config.initial_tape_dir, config_path);
		for(int drv = 0; drv < USE_TAPE; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentTapePath%d_%d"), drv + 1, i + 1), config.recent_tape_path[drv][i], config_path);
			}
		}
	#endif
	#ifdef USE_COMPACT_DISC
		MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialCompactDiscDir"), config.initial_compact_disc_dir, config_path);
		for(int drv = 0; drv < USE_COMPACT_DISC; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentCompactDiscPath%d_%d"), drv + 1, i + 1), config.recent_compact_disc_path[drv][i], config_path);
			}

 		}
		
 	#endif
	#ifdef USE_LASER_DISC
		MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), config.initial_laser_disc_dir, config_path);
		for(int drv = 0; drv < USE_LASER_DISC; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentLaserDiscPath%d_%d"), drv + 1, i + 1), config.recent_laser_disc_path[drv][i], config_path);
			}
		}
	#endif
	#ifdef USE_BINARY_FILE
		MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), config.initial_binary_dir, config_path);
		for(int drv = 0; drv < USE_BINARY_FILE; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentBinaryPath%d_%d"), drv + 1, i + 1), config.recent_binary_path[drv][i], config_path);
			}
		}
	#endif
	#ifdef USE_BUBBLE
		MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialBubbleDir"), config.initial_bubble_casette_dir, config_path);
		for(int drv = 0; drv < USE_BUBBLE; drv++) {
			for(int i = 0; i < MAX_HISTORY; i++) {
				MyWritePrivateProfileString(_T("RecentFiles"), create_string(_T("RecentBubblePath%d_%d"), drv + 1, i + 1), config.recent_bubble_casette_path[drv][i], config_path);
			}
 		}
	#endif
	
	// screen
	#ifndef ONE_BOARD_MICRO_COMPUTER
		MyWritePrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
		MyWritePrivateProfileInt(_T("Screen"), _T("WindowStretchType"), config.window_stretch_type, config_path);
		MyWritePrivateProfileInt(_T("Screen"), _T("FullScreenStretchType"), config.fullscreen_stretch_type, config_path);
//		#ifdef USE_SCREEN_ROTATE
			MyWritePrivateProfileInt(_T("Screen"), _T("RotateType"), config.rotate_type, config_path);
//		#endif
	#endif

	// filter
	#ifdef USE_SCREEN_FILTER
		MyWritePrivateProfileInt(_T("Screen"), _T("FilterType"), config.filter_type, config_path);
	#endif
		
	// sound
		MyWritePrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
		MyWritePrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
		MyWritePrivateProfileBool(_T("Sound"), _T("StrictRendering"), config.sound_strict_rendering, config_path);
		MyWritePrivateProfileInt(_T("Sound"), _T("DeviceNumber"), config.sound_device_num, config_path);
	#ifdef USE_FLOPPY_DISK
		MyWritePrivateProfileBool(_T("Sound"), _T("NoiseFDD"), config.sound_noise_fdd, config_path);
	#endif
	#ifdef USE_TAPE
		MyWritePrivateProfileBool(_T("Sound"), _T("NoiseCMT"), config.sound_noise_cmt, config_path);
		MyWritePrivateProfileBool(_T("Sound"), _T("PlayTape"), config.sound_play_tape, config_path);
	#endif
	#ifdef USE_COMPACT_DISC
		for(int drv = 0; drv < USE_COMPACT_DISC; drv++) {
			MyWritePrivateProfileBool(_T("Sound"), create_string(_T("SwapCDByteOrder%d"), drv + 1), config.swap_audio_byteorder[drv], config_path);
		}
	#endif
	#ifdef USE_SOUND_VOLUME
		for(int i = 0; i < USE_SOUND_VOLUME; i++) {
			MyWritePrivateProfileInt(_T("Sound"), create_string(_T("VolumeLeft%d"), i + 1), config.sound_volume_l[i], config_path);
			MyWritePrivateProfileInt(_T("Sound"), create_string(_T("VolumeRight%d"), i + 1), config.sound_volume_r[i], config_path);
		}
	#endif
	#if defined(_WIN32) && !defined(_USE_QT)
		MyWritePrivateProfileString(_T("Sound"), _T("YM2151GenDll"), config.mame2151_dll_path, config_path);
		MyWritePrivateProfileString(_T("Sound"), _T("YM2608GenDll"), config.mame2608_dll_path, config_path);
	#endif
	
	// input
	#ifdef USE_JOYSTICK
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 16; j++) {
				MyWritePrivateProfileInt(_T("Input"), create_string(_T("JoyButtonsEx%d_%d"), i + 1, j + 1), config.joy_buttons[i][j], config_path);
			}
		}
		MyWritePrivateProfileBool(_T("Input"), _T("UseJoyToKey"), config.use_joy_to_key, config_path);
		MyWritePrivateProfileInt(_T("Input"), _T("JoyToKeyType"), config.joy_to_key_type, config_path);
		MyWritePrivateProfileBool(_T("Input"), _T("JoyToKeyNumPad5"), config.joy_to_key_numpad5, config_path);
		for(int i = 0; i < 16; i++) {
			MyWritePrivateProfileInt(_T("Input"), create_string(_T("JoyToKeyButtons%d"), i + 1), config.joy_to_key_buttons[i], config_path);
		}
	#endif

	// debug
	#ifdef USE_FLOPPY_DISK
		MyWritePrivateProfileInt(_T("Debug"), _T("SpecialDebugFDC"), config.special_debug_fdc, config_path);
	#endif
		MyWritePrivateProfileBool(_T("Debug"), _T("PrintCPUStatistics"), config.print_statistics, config_path);
	// printer
	#ifdef USE_PRINTER
		MyWritePrivateProfileString(_T("Printer"), _T("PrinterDll"), config.printer_dll_path, config_path);
	#endif

	
	// win32
	#if defined(_WIN32) && !defined(_USE_QT)
		#ifndef ONE_BOARD_MICRO_COMPUTER
			MyWritePrivateProfileBool(_T("Win32"), _T("UseDirect3D9"), config.use_d3d9, config_path);
			MyWritePrivateProfileBool(_T("Win32"), _T("WaitVSync"), config.wait_vsync, config_path);
		#endif
		MyWritePrivateProfileBool(_T("Win32"), _T("UseDirectInput"), config.use_dinput, config_path);
		MyWritePrivateProfileBool(_T("Win32"), _T("DisableDwm"), config.disable_dwm, config_path);
		MyWritePrivateProfileBool(_T("Win32"), _T("ShowStatusBar"), config.show_status_bar, config_path);
	#endif
	#ifdef _USE_QT
		MyWritePrivateProfileBool(_T("Qt"), _T("UseSeparateThreadDraw"), config.use_separate_thread_draw, config_path);
		MyWritePrivateProfileBool(_T("Qt"), _T("UseOSDVirtualMedia"), config.use_osd_virtual_media, config_path); 
		MyWritePrivateProfileBool(_T("Qt"), _T("UseOpenGLScanLine"), config.use_opengl_scanline, config_path);
		MyWritePrivateProfileBool(_T("Qt"), _T("OpenGLScanLineVert"), config.opengl_scanline_vert, config_path);;
		MyWritePrivateProfileBool(_T("Qt"), _T("OpenGLScanLineHoriz"), config.opengl_scanline_horiz, config_path);;
		MyWritePrivateProfileBool(_T("Qt"), _T("UseOpenGLFilters"), config.use_opengl_filters, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("OpenGLFilterNum"), config.opengl_filter_num, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("RenderType"), config.rendering_type, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("RenderPlatform"), config.render_platform, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("RenderMajorVersion"), config.render_major_version, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("RenderMinorVersion"), config.render_minor_version, config_path);

		MyWritePrivateProfileInt(_T("Qt"), _T("GeneralSoundLevel"), config.general_sound_level, config_path);
		MyWritePrivateProfileBool(_T("Qt"), _T("FocusWithClick"), config.focus_with_click, config_path);

		// Extra UI
		MyWritePrivateProfileBool(_T("Qt"), _T("SwapKanjiPause"), config.swap_kanji_pause, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("CursorAsTenKey"), config.cursor_as_ten_key, config_path);
		MyWritePrivateProfileBool(_T("Qt"), _T("NumpadEnterAsFullKey"), config.numpad_enter_as_fullkey, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("HostKeyboardType"), config.host_keyboard_type, config_path);
		MyWritePrivateProfileString(_T("Qt"), _T("DebugWindowFont"), config.debugwindow_font, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("DebugWindowWidth"), config.debugwindow_width, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("DebugWindowHeight"), config.debugwindow_height, config_path);
		MyWritePrivateProfileString(_T("Qt"), _T("LogWindowFont"), config.logwindow_font, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("LogWindowWidth"), config.logwindow_width, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("LogWindowHeight"), config.logwindow_height, config_path);

		for(i = 0; i < 16; i++) {
			_TCHAR name[256];
			my_stprintf_s(name, 255, _T("AssignedJoystick%d"), i + 1);
			MyWritePrivateProfileString(_T("Qt"), (const _TCHAR *)name, 
										config.assigned_joystick_name[i], config_path);
		}
		MyWritePrivateProfileInt(_T("Qt"), _T("VideoWidth"), config.video_width, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("VideoHeight"), config.video_height, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("VideoCodecType"), config.video_codec_type, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("AudioCodecType"), config.audio_codec_type, config_path);
		
		MyWritePrivateProfileInt(_T("Qt"), _T("H264Bitrate"), config.video_h264_bitrate, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("H264BFrames"), config.video_h264_bframes, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("H264BAdapt"), config.video_h264_b_adapt, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("H264MinQ"), config.video_h264_minq, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("H264MaxQ"), config.video_h264_maxq, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("H264Subme"), config.video_h264_subme, config_path);
		
		MyWritePrivateProfileInt(_T("Qt"), _T("MPEG4Bitrate"), config.video_mpeg4_bitrate, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("MPEG4BFrames"), config.video_mpeg4_bframes, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("MPEG4MinQ"), config.video_mpeg4_minq, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("MPEG4MaxQ"), config.video_mpeg4_maxq, config_path);
		
		MyWritePrivateProfileInt(_T("Qt"), _T("VideoThreads"), config.video_threads, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("AudioBitrate"), config.audio_bitrate, config_path);
		MyWritePrivateProfileInt(_T("Qt"), _T("VideoFramerate"), config.video_frame_rate, config_path);
		
		MyWritePrivateProfileBool(_T("Qt"), _T("WriteToSyslog"), config.log_to_syslog, config_path);
		MyWritePrivateProfileBool(_T("Qt"), _T("WriteToConsole"), config.log_to_console, config_path);
		
		for(int ii = 0; ii < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1) ; ii++) {
			uint32_t flags = 0;
			flags = 0;
			for(int jj = 0; jj < 8; jj++) {
				flags <<= 1;
				if(config.dev_log_to_syslog[ii][jj]) flags |= 0x0001;
			}
			MyWritePrivateProfileInt(_T("Qt"), create_string(_T("SyslogEnabled%d"), ii), flags, config_path);
			
			flags = 0;
			for(int jj = 0; jj < 8; jj++) {
				flags <<= 1;
				if(config.dev_log_to_console[ii][jj]) flags |= 0x0001;
			}
			MyWritePrivateProfileInt(_T("Qt"), create_string(_T("ConsoleLogEnabled%d"), ii), flags, config_path);
			
			flags = 0;
			for(int jj = 0; jj < 8; jj++) {
				flags <<= 1;
				if(config.dev_log_recording[ii][jj]) flags |= 0x0001;
			}
			MyWritePrivateProfileInt(_T("Qt"), create_string(_T("RecordLogEnabled%d"), ii), flags, config_path);
		}
		MyWritePrivateProfileBool(_T("Qt"), _T("StateLogToConsole"), config.state_log_to_console, config_path);
		MyWritePrivateProfileBool(_T("Qt"), _T("StateLogToSyslog"), config.state_log_to_syslog, config_path);
		MyWritePrivateProfileBool(_T("Qt"), _T("StateLogToRecording"), config.state_log_to_recording, config_path);
		
		MyWritePrivateProfileInt(_T("Qt"), _T("UiVirtualMediaPosition"), config.virtual_media_position, config_path);
		//csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Write config done.");
	#endif	
}

#define STATE_VERSION	6

bool DLL_PREFIX process_config_state(void *f, bool loading)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	#ifdef USE_BOOT_MODE
		state_fio->StateValue(config.boot_mode);
	#endif
	#ifdef USE_CPU_TYPE
		state_fio->StateValue(config.cpu_type);
	#endif
	#ifdef USE_DIPSWITCH
		state_fio->StateValue(config.dipswitch);
	#endif
	#ifdef USE_DEVICE_TYPE
		state_fio->StateValue(config.device_type);
	#endif
	#ifdef USE_DRIVE_TYPE
		state_fio->StateValue(config.drive_type);
	#endif
	#ifdef USE_KEYBOARD_TYPE
		state_fio->StateValue(config.keyboard_type);
	#endif
	#ifdef USE_MOUSE_TYPE
		state_fio->StateValue(config.mouse_type);
	#endif
	#ifdef USE_JOYSTICK_TYPE
		state_fio->StateValue(config.joystick_type);
	#endif
	#ifdef USE_SOUND_TYPE
		state_fio->StateValue(config.sound_type);
	#endif
	#ifdef USE_MONITOR_TYPE
		state_fio->StateValue(config.monitor_type);
	#endif
	#ifdef USE_PRINTER_TYPE
		state_fio->StateValue(config.printer_type);
	#endif
	#ifdef USE_FLOPPY_DISK
		for(int drv = 0; drv < USE_FLOPPY_DISK; drv++) {
			state_fio->StateValue(config.correct_disk_timing[drv]);
			state_fio->StateValue(config.ignore_disk_crc[drv]);
		}
	#endif
	state_fio->StateValue(config.sound_frequency);
	state_fio->StateValue(config.sound_latency);
	return true;
}

