/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ config ]
*/
#if defined(_USE_AGAR)
#include <SDL/SDL.h>
#include <agar/core.h>
#include <string>
#include <vector>
#include "fileio.h"
#include "agar_logger.h"
#endif

#if defined(_USE_QT)
#include <string>
#include <vector>
#include "fileio.h"
#include "agar_logger.h"
#include "qt_main.h"
# if defined(Q_OS_WIN)
# include <windows.h>
# endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "config.h"
#include "fileio.h"
#if defined(_USE_AGAR)
#include "agar_main.h"
#endif

config_t config;

#ifndef CONFIG_NAME
#define CONFIG_NAME "conf"
#endif

//extern _TCHAR* get_parent_dir(_TCHAR* file);
BOOL MyWritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int Value, LPCTSTR lpFileName)
{
 	_TCHAR String[32];
	my_stprintf_s(String, 32, _T("%d"), Value);
	return MyWritePrivateProfileString(lpAppName, lpKeyName, String, lpFileName);
}
 
BOOL MyWritePrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool Value, LPCTSTR lpFileName)
{
 	_TCHAR String[32];
	my_stprintf_s(String, 32, _T("%d"), Value ? 1 : 0);
	return MyWritePrivateProfileString(lpAppName, lpKeyName, String, lpFileName);
}
 
bool MyGetPrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool bDefault, LPCTSTR lpFileName)
{
	return (MyGetPrivateProfileInt(lpAppName, lpKeyName, bDefault ? 1 : 0, lpFileName) != 0);
}

void init_config()
{
	int i;
	// initial settings
	memset(&config, 0, sizeof(config_t));
	config.window_mode = 1;	
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
#if defined(USE_FD1)
	for(int drv = 0; drv < MAX_FD; drv++) {
#if defined(CORRECT_DISK_TIMING_DEFAULT)
		config.correct_disk_timing[drv] = CORRECT_DISK_TIMING_DEFAULT;
#else
		config.correct_disk_timing[drv] = true;
#endif
#if defined(IGNORE_DISK_CRC_DEFAULT)
		config.ignore_disk_crc[drv] = IGNORE_DISK_CRC_DEFAULT;
#endif
	}
#elif defined(USE_FD1)
	for(int drv = 0; drv < MAX_FD; drv++) {
		config.ignore_disk_crc[drv] = false;
	}
#endif	
#if defined(USE_TAPE)
	config.wave_shaper = true;
	config.direct_load_mzt = true;
	config.baud_high = true;
#endif

	// sound
	config.sound_frequency = 6;	// 48KHz
	config.sound_latency = 1;	// 100msec
	config.multiple_speakers = false;
	config.general_sound_level = 0;
#if defined(USE_SOUND_DEVICE_TYPE) && defined(SOUND_DEVICE_TYPE_DEFAULT)
	config.sound_device_type = SOUND_DEVICE_TYPE_DEFAULT;
#elif defined(USE_SOUND_DEVICE_TYPE)
	config.sound_device_type = 0;
#endif
	
	// input
#ifdef _WIN32
	config.use_direct_input = true;
	config.disable_dwm = false;
#endif
	config.keyboard_type = 0;
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 16; j++) {
			config.joy_buttons[i][j] = (i << 4) | j;
		}
	}
	
	// printer
#if defined(USE_PRINTER) && defined(PRINTER_DEVICE_TYPE_DEFAULT)
	config.printer_device_type = PRINTER_DEVICE_TYPE_DEFAULT;
#elif defined(USE_PRINTER)
	config.printer_device_type = 0;
#endif
	
	// screen
#ifndef ONE_BOARD_MICRO_COMPUTER
#ifdef _WIN32
	config.use_d3d9 = true;
#endif
	config.stretch_type = 1;	// Stretch (Aspect)
#endif
	
#if defined(_USE_QT)
	config.use_opengl_scanline = false;
	config.opengl_scanline_vert = false;
	config.opengl_scanline_horiz = false;
	config.use_opengl_filters = false;
	config.opengl_filter_num = 0;
#endif	
#ifdef USE_MULTIPLE_SOUNDCARDS
	{
		int ii;
		for(ii = 0; ii < USE_MULTIPLE_SOUNDCARDS; ii++) {
			config.sound_device_level[ii] = 0;
		}
	}
#endif
}

void load_config(const _TCHAR *config_path)
{
	int drv, i;
	// initial settings
	init_config();

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
#ifdef USE_FD1
	{
		_TCHAR name[64];
		for(drv = 0; drv < MAX_FD; drv++) {
			memset(name, 0x00, sizeof(name));
			my_stprintf_s(name, 64, _T("CorrectDiskTiming%d"), drv + 1);
			config.correct_disk_timing[drv] = MyGetPrivateProfileBool(_T("Control"), (const _TCHAR *)name,
																	config.correct_disk_timing[drv], config_path);
			my_stprintf_s(name, 64, _T("IgnoreDiskCRC%d"), drv + 1);
			config.ignore_disk_crc[drv] = MyGetPrivateProfileBool(_T("Control"), (const _TCHAR *)name,
																config.ignore_disk_crc[drv], config_path);
		}
	}
#endif

#ifdef USE_TAPE
	config.tape_sound = MyGetPrivateProfileBool(_T("Control"), _T("TapeSound"), config.tape_sound, config_path);
	config.wave_shaper = MyGetPrivateProfileBool(_T("Control"), _T("WaveShaper"), config.wave_shaper, config_path);
	config.direct_load_mzt = MyGetPrivateProfileBool(_T("Control"), _T("DirectLoadMZT"), config.direct_load_mzt, config_path);
	config.baud_high = MyGetPrivateProfileBool(_T("Control"), _T("BaudHigh"), config.baud_high, config_path);
#endif
	
	// recent files
#ifdef USE_CART1
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialCartDir"), _T(""), config.initial_cart_dir, _MAX_PATH, config_path);
	for(drv = 0; drv < MAX_CART; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
		        my_stprintf_s(name, 64, _T("RecentCartPath%d_%d"), drv + 1, i + 1);
			MyGetPrivateProfileString(_T("RecentFiles"), name, _T(""),
									config.recent_cart_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_FD1
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), _T(""),
							config.initial_disk_dir, _MAX_PATH, config_path);
    //    get_parent_dir(config.initial_disk_dir);
	for(drv = 0; drv < MAX_FD; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentDiskPath%d_%d"), drv + 1, i + 1);
			MyGetPrivateProfileString(_T("RecentFiles"), (const _TCHAR *)name,
									_T(""), config.recent_disk_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_QD1
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"),
							_T(""), config.initial_quickdisk_dir, _MAX_PATH, config_path);
	for(drv = 0; drv < MAX_QD; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1);
			MyGetPrivateProfileString(_T("RecentFiles"), name, _T(""),
									config.recent_quickdisk_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif

#ifdef USE_TAPE
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), _T(""),
							config.initial_tape_dir, _MAX_PATH, config_path);
	for(i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("RecentTapePath1_%d"), i + 1);
		MyGetPrivateProfileString(_T("RecentFiles"), (const _TCHAR *)name, _T(""),
								config.recent_tape_path[i], _MAX_PATH, config_path);
	}
#endif

#ifdef USE_LASER_DISC
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), _T(""),
							config.initial_laser_disc_dir, _MAX_PATH, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("RecentLaserDiscPath1_%d"), i + 1);
		MyGetPrivateProfileString(_T("RecentFiles"), (const _TCHAR *)name, _T(""),
								config.recent_laser_disc_path[i], _MAX_PATH, config_path);
	}
#endif
#ifdef USE_BINARY_FILE1
	MyGetPrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), _T(""),
							config.initial_binary_dir, _MAX_PATH, config_path);
	for(drv = 0; drv < MAX_BINARY; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentBinaryPath%d_%d"), drv + 1, i + 1);
			MyGetPrivateProfileString(_T("RecentFiles"), (const _TCHAR *)name, _T(""),
									config.recent_binary_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
	
	// screen
#ifndef ONE_BOARD_MICRO_COMPUTER
	config.window_mode = MyGetPrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
#ifdef _WIN32
	config.use_d3d9 = MyGetPrivateProfileBool(_T("Screen"), _T("UseD3D9"), config.use_d3d9, config_path);
	config.wait_vsync = MyGetPrivateProfileBool(_T("Screen"), _T("WaitVSync"), config.wait_vsync, config_path);
#endif
	config.stretch_type = MyGetPrivateProfileInt(_T("Screen"), _T("StretchType"), config.stretch_type, config_path);
#else
	config.window_mode = MyGetPrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
#endif
#ifdef USE_MONITOR_TYPE
	config.monitor_type = MyGetPrivateProfileInt(_T("Screen"), _T("MonitorType"), config.monitor_type, config_path);
#endif
#ifdef USE_CRT_FILTER
	config.crt_filter = MyGetPrivateProfileBool(_T("Screen"), _T("CRTFilter"), config.crt_filter, config_path);
#endif
#ifdef USE_SCANLINE
	config.scan_line = MyGetPrivateProfileBool(_T("Screen"), _T("ScanLine"), config.scan_line, config_path);
#endif

#ifdef USE_SCREEN_ROTATE
	config.rotate_type = MyGetPrivateProfileInt(_T("Screen"), _T("RotateType"), config.rotate_type, config_path);
#endif
#if defined(_USE_QT)
	config.use_opengl_scanline = MyGetPrivateProfileBool(_T("Screen"), _T("UseOpenGLScanLine"),
													   config.use_opengl_scanline, config_path);
	config.opengl_scanline_vert = MyGetPrivateProfileBool(_T("Screen"), _T("OpenGLScanLineVert"),
													   config.opengl_scanline_vert, config_path);;
	config.opengl_scanline_horiz = MyGetPrivateProfileBool(_T("Screen"), _T("OpenGLScanLineHoriz"),
													   config.opengl_scanline_horiz, config_path);;
	config.use_opengl_filters = MyGetPrivateProfileBool(_T("Screen"), _T("UseOpenGLFilters"),
													   config.use_opengl_filters, config_path);
	config.opengl_filter_num = 	MyGetPrivateProfileInt(_T("Screen"), _T("OpenGLFilterNum"),
													 config.opengl_filter_num, config_path);
#endif	
	// sound
	config.sound_frequency = MyGetPrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
	config.sound_latency = MyGetPrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
#ifdef USE_SOUND_DEVICE_TYPE
	config.sound_device_type = MyGetPrivateProfileInt(_T("Sound"), _T("DeviceType"), config.sound_device_type, config_path);
#endif
#ifdef USE_SOUND_VOLUME
	for(int i = 0; i < USE_SOUND_VOLUME; i++) {
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("VolumeLeft%d"), i + 1);
		int tmp_l = MyGetPrivateProfileInt(_T("Sound"), name, config.sound_volume_l[i], config_path);
		my_stprintf_s(name, 64, _T("VolumeRight%d"), i + 1);
		int tmp_r = MyGetPrivateProfileInt(_T("Sound"), name, config.sound_volume_r[i], config_path);
		config.sound_volume_l[i] = max(-40, min(0, tmp_l));
		config.sound_volume_r[i] = max(-40, min(0, tmp_r));
	}
#endif
#if !defined(_USE_QT)
 	MyGetPrivateProfileString(_T("Sound"), _T("FMGenDll"), _T("mamefm.dll"), config.fmgen_dll_path, _MAX_PATH, config_path);
#endif	
	// input
	config.multiple_speakers = MyGetPrivateProfileBool(_T("Sound"), _T("MultipleSpeakers"),
													 config.multiple_speakers, config_path);
	config.general_sound_level = MyGetPrivateProfileInt(_T("Sound"), _T("GeneralSoundLevel"),
													  config.general_sound_level, config_path);
#ifdef USE_MULTIPLE_SOUNDCARDS
	{
		_TCHAR _tag[128];
		int ii;
		for(ii = 0; ii < USE_MULTIPLE_SOUNDCARDS; ii++) {
			memset(_tag, 0x00, sizeof(_tag));
			my_stprintf_s(_tag, 64, _T("DeviceVolumeLevel_%d"), ii + 1);
			config.sound_device_level[ii] = MyGetPrivateProfileInt(_T("Sound"), (const _TCHAR *)_tag, config.sound_device_level[ii], config_path);
		}
	}
#endif
#ifdef _WIN32
	config.use_direct_input = MyGetPrivateProfileBool(_T("Input"), _T("UseDirectInput"), config.use_direct_input, config_path);
	config.disable_dwm = MyGetPrivateProfileBool(_T("Input"), _T("DisableDwm"), config.disable_dwm, config_path);
#endif
	config.keyboard_type = MyGetPrivateProfileInt(_T("Input"), _T("KeyboardType"), config.keyboard_type, config_path);
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 16; j++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("JoyButtons%d_%d"), i + 1, j + 1);
			config.joy_buttons[i][j] = MyGetPrivateProfileInt(_T("Input"), name, config.joy_buttons[i][j], config_path);
		}
	}
#if defined(_USE_QT)
	for(i = 0; i < 16; i++) {
		_TCHAR name[256];
		my_stprintf_s(name, 256, _T("AssignedJoystick"), i + 1);
		MyGetPrivateProfileString(_T("Input"), (const _TCHAR *)name, _T(""),
								  config.assigned_joystick_name[i], 256, config_path);
	}
#endif	
	// printer
#ifdef USE_PRINTER
	config.printer_device_type = MyGetPrivateProfileInt(_T("Printer"), _T("DeviceType"), config.printer_device_type, config_path);
	MyGetPrivateProfileString(_T("Printer"), _T("PrinterDll"), _T("printer.dll"), config.printer_dll_path, _MAX_PATH, config_path);
#endif
#if defined(_USE_QT) && !defined(Q_OS_WIN)
	AGAR_DebugLog(AGAR_LOG_INFO, "Read Done.");
#endif
}

void save_config(const _TCHAR *config_path)
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
# ifdef USE_BOOT_MODE
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
#ifdef USE_FD1
	{
		_TCHAR name[64];
		for(drv = 0; drv < MAX_FD; drv++) {
			memset(name, 0x00, sizeof(name));
			my_stprintf_s(name, 64, _T("CorrectDiskTiming%d"), drv + 1);
			MyWritePrivateProfileBool(_T("Control"), (const _TCHAR *)name, config.correct_disk_timing[drv], config_path);
			my_stprintf_s(name, 64, _T("IgnoreDiskCRC%d"), drv + 1);
			MyWritePrivateProfileBool(_T("Control"), (const _TCHAR *)name, config.ignore_disk_crc[drv], config_path);
		}
	}

#endif
#ifdef USE_TAPE
	MyWritePrivateProfileBool(_T("Control"), _T("TapeSound"), config.tape_sound, config_path);
	MyWritePrivateProfileBool(_T("Control"), _T("WaveShaper"), config.wave_shaper, config_path);
	MyWritePrivateProfileBool(_T("Control"), _T("DirectLoadMZT"), config.direct_load_mzt, config_path);
	MyWritePrivateProfileBool(_T("Control"), _T("BaudHigh"), config.baud_high, config_path);
#endif
	
	// recent files
#ifdef USE_CART1
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialCartDir"), config.initial_cart_dir, config_path);
	for(drv = 0; drv < MAX_CART; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentCartPath%d_%d"), drv + 1, i + 1);
			MyWritePrivateProfileString(_T("RecentFiles"), (const _TCHAR *)name, config.recent_cart_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_FD1
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), config.initial_disk_dir, config_path);
	for(drv = 0; drv < MAX_FD; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentDiskPath%d_%d"), drv + 1, i + 1);
			MyWritePrivateProfileString(_T("RecentFiles"), (const _TCHAR *)name,
									  config.recent_disk_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_QD1
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"),
							  config.initial_quickdisk_dir, config_path);
	for(drv = 0; drv < MAX_QD; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1);
			MyWritePrivateProfileString(_T("RecentFiles"), (const _TCHAR *)name,
									  config.recent_quickdisk_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_TAPE
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), config.initial_tape_dir, config_path);
	for(i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("RecentTapePath1_%d"), i + 1);
		MyWritePrivateProfileString(_T("RecentFiles"), (const _TCHAR *)name,
								  config.recent_tape_path[i], config_path);
	}
#endif
#ifdef USE_LASER_DISC
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), config.initial_laser_disc_dir, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("RecentLaserDiscPath1_%d"), i + 1);
		MyWritePrivateProfileString(_T("RecentFiles"), (const _TCHAR *)name,
								  config.recent_laser_disc_path[i], config_path);
	}
#endif
#ifdef USE_BINARY_FILE1
	MyWritePrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), config.initial_binary_dir, config_path);
	for(drv = 0; drv < MAX_BINARY; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("RecentBinaryPath%d_%d"), drv + 1, i + 1);
			MyWritePrivateProfileString(_T("RecentFiles"), (const _TCHAR *)name,
									  config.recent_binary_path[drv][i], config_path);
		}
	}
#endif
	
	// screen
#ifndef ONE_BOARD_MICRO_COMPUTER
	MyWritePrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
#ifdef _WIN32
	MyWritePrivateProfileBool(_T("Screen"), _T("UseD3D9"), config.use_d3d9, config_path);
	MyWritePrivateProfileBool(_T("Screen"), _T("WaitVSync"), config.wait_vsync, config_path);
#endif
	MyWritePrivateProfileInt(_T("Screen"), _T("StretchType"), config.stretch_type, config_path);
#else
	MyWritePrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
#endif
#ifdef USE_MONITOR_TYPE
	MyWritePrivateProfileInt(_T("Screen"), _T("MonitorType"), config.monitor_type, config_path);
#endif
#ifdef USE_CRT_FILTER
	MyWritePrivateProfileBool(_T("Screen"), _T("CRTFilter"), config.crt_filter, config_path);
#endif
#ifdef USE_SCANLINE
	MyWritePrivateProfileBool(_T("Screen"), _T("ScanLine"), config.scan_line, config_path);
#endif
#ifdef USE_SCREEN_ROTATE
	MyWritePrivateProfileInt(_T("Screen"), _T("RotateType"), config.rotate_type, config_path);
#endif
#if defined(_USE_QT)
	MyWritePrivateProfileBool(_T("Screen"), _T("UseOpenGLScanLine"),
							config.use_opengl_scanline, config_path);
	MyWritePrivateProfileBool(_T("Screen"), _T("OpenGLScanLineVert"),
							config.opengl_scanline_vert, config_path);;
	MyWritePrivateProfileBool(_T("Screen"), _T("OpenGLScanLineHoriz"),
							config.opengl_scanline_horiz, config_path);;
	MyWritePrivateProfileBool(_T("Screen"), _T("UseOpenGLFilters"),
							config.use_opengl_filters, config_path);
	MyWritePrivateProfileInt(_T("Screen"), _T("OpenGLFilterNum"),
						   config.opengl_filter_num, config_path);
#endif	
	
	// sound
	MyWritePrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
	MyWritePrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
#ifdef USE_SOUND_DEVICE_TYPE
	MyWritePrivateProfileInt(_T("Sound"), _T("DeviceType"), config.sound_device_type, config_path);
#endif
#ifdef USE_SOUND_VOLUME
	for(int i = 0; i < USE_SOUND_VOLUME; i++) {
		_TCHAR name[64];
		my_stprintf_s(name, 64, _T("VolumeLeft%d"), i + 1);
		MyWritePrivateProfileInt(_T("Sound"), name, config.sound_volume_l[i], config_path);
		my_stprintf_s(name, 64, _T("VolumeRight%d"), i + 1);
		MyWritePrivateProfileInt(_T("Sound"), name, config.sound_volume_r[i], config_path);
	}
#endif
#if !defined(_USE_QT)
 	MyWritePrivateProfileString(_T("Sound"), _T("FMGenDll"), config.fmgen_dll_path, config_path);
#endif	
	MyWritePrivateProfileBool(_T("Sound"), _T("MultipleSpeakers"),
							config.multiple_speakers, config_path);
	MyWritePrivateProfileInt(_T("Sound"), _T("GeneralSoundLevel"),
						   config.general_sound_level, config_path);
#ifdef USE_MULTIPLE_SOUNDCARDS
	{
		_TCHAR _tag[128];
		int ii;
		for(ii = 0; ii < USE_MULTIPLE_SOUNDCARDS; ii++) {
			memset(_tag, 0x00, sizeof(_tag));
			my_stprintf_s(_tag, 64, _T("DeviceVolumeLevel_%d"), ii + 1);
			MyWritePrivateProfileInt(_T("Sound"), (const _TCHAR *)_tag, config.sound_device_level[ii], config_path);
		}
	}
#endif
	// input
#ifdef _WIN32
	MyWritePrivateProfileBool(_T("Input"), _T("UseDirectInput"), config.use_direct_input, config_path);
	MyWritePrivateProfileBool(_T("Input"), _T("DisableDwm"), config.disable_dwm, config_path);
#endif
	MyWritePrivateProfileInt(_T("Input"), _T("KeyboardType"), config.keyboard_type, config_path);
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 16; j++) {
			_TCHAR name[64];
			my_stprintf_s(name, 64, _T("JoyButtons%d_%d"), i + 1, j + 1);
			MyWritePrivateProfileInt(_T("Input"), name, config.joy_buttons[i][j], config_path);
		}
	}
#if defined(_USE_QT)
	for(i = 0; i < 16; i++) {
		_TCHAR name[256];
		my_stprintf_s(name, 256, _T("AssignedJoystick%d"), i + 1);
		MyWritePrivateProfileString(_T("Input"), (const _TCHAR *)name, 
									config.assigned_joystick_name[i], config_path);
	}
#endif	
	
	// printer
#ifdef USE_PRINTER
	MyWritePrivateProfileInt(_T("Printer"), _T("DeviceType"), config.printer_device_type, config_path);
#endif
#if defined(_USE_QT) && !defined(Q_OS_WIN)
	AGAR_DebugLog(AGAR_LOG_INFO, "Write done.");
#endif
}

#define STATE_VERSION	5

void save_config_state(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;
	int drv;
	
	state_fio->FputUint32(STATE_VERSION);
	
#ifdef USE_BOOT_MODE
	state_fio->FputInt32(config.boot_mode);
#endif
#ifdef USE_CPU_TYPE
	state_fio->FputInt32(config.cpu_type);
#endif
#ifdef USE_DIPSWITCH
	state_fio->FputUint32(config.dipswitch);
#endif
#ifdef USE_DEVICE_TYPE
	state_fio->FputInt32(config.device_type);
#endif
#ifdef USE_DRIVE_TYPE
	state_fio->FputInt32(config.drive_type);
#endif
#ifdef USE_FD1
	for(int drv = 0; drv < MAX_FD; drv++) {
		state_fio->FputBool(config.correct_disk_timing[drv]);
		state_fio->FputBool(config.ignore_disk_crc[drv]);
	}
//	for(int drv = 0; drv < MAX_FD; drv++) {
//		state_fio->FputBool(config.fdd_hack_fast_transfer[drv]);
//	}
#endif
#ifdef USE_MONITOR_TYPE
	state_fio->FputInt32(config.monitor_type);
#endif
#ifdef USE_SOUND_DEVICE_TYPE
	state_fio->FputInt32(config.sound_device_type);
#endif
#ifdef USE_PRINTER
	state_fio->FputInt32(config.printer_device_type);
#endif
	state_fio->FputInt32(config.keyboard_type);
}

bool load_config_state(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
#ifdef USE_BOOT_MODE
	config.boot_mode = state_fio->FgetInt32();
#endif
#ifdef USE_CPU_TYPE
	config.cpu_type = state_fio->FgetInt32();
#endif
#ifdef USE_DIPSWITCH
	config.dipswitch = state_fio->FgetUint32();
#endif
#ifdef USE_DEVICE_TYPE
	config.device_type = state_fio->FgetInt32();
#endif
#ifdef USE_DRIVE_TYPE
	config.drive_type = state_fio->FgetInt32();
#endif
#ifdef USE_FD1
	for(int drv = 0; drv < MAX_FD; drv++) {
		config.correct_disk_timing[drv] = state_fio->FgetBool();
		config.ignore_disk_crc[drv] = state_fio->FgetBool();
	}
//	for(int drv = 0; drv < MAX_FD; drv++) {
//		config.fdd_hack_fast_transfer[drv] = state_fio->FgetBool();
//	}
#endif
#ifdef USE_MONITOR_TYPE
	config.monitor_type = state_fio->FgetInt32();
#endif
#ifdef USE_SOUND_DEVICE_TYPE
	config.sound_device_type = state_fio->FgetInt32();
#endif
#ifdef USE_PRINTER
	config.printer_device_type = state_fio->FgetInt32();
#endif
	config.keyboard_type = state_fio->FgetInt32();
	return true;
}

