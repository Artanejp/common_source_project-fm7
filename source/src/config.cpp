/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ config ]
*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "fileio.h"

config_t config;

BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int Value, LPCTSTR lpFileName)
{
	_TCHAR String[32];
	_stprintf(String, _T("%d"), Value);
	return WritePrivateProfileString(lpAppName, lpKeyName, String, lpFileName);
}

BOOL WritePrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool Value, LPCTSTR lpFileName)
{
	_TCHAR String[32];
	_stprintf(String, _T("%d"), Value ? 1 : 0);
	return WritePrivateProfileString(lpAppName, lpKeyName, String, lpFileName);
}

bool GetPrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool bDefault, LPCTSTR lpFileName)
{
	return (GetPrivateProfileInt(lpAppName, lpKeyName, bDefault ? 1 : 0, lpFileName) != 0);
}

void init_config()
{
	// initial settings
	memset(&config, 0, sizeof(config_t));
	
#if !(defined(USE_BITMAP) || defined(USE_LED))
	config.use_d3d9 = true;
	config.stretch_type = 1;	// Stretch (Aspect)
#endif
	config.sound_frequency = 6;	// 48KHz
	config.sound_latency = 1;	// 100msec
	
#if defined(USE_TAPE)
	config.wave_shaper = true;
	config.direct_load_mzt = true;
#endif
#if defined(USE_DIPSWITCH) && defined(DIPSWITCH_DEFAULT)
	config.dipswitch = DIPSWITCH_DEFAULT;
#endif
#if defined(_HC80)
	config.device_type = 2;		// Nonintelligent ram disk
#endif
#if defined(_PC8801MA)
	config.boot_mode = 2;		// V2 mode, 4MHz
	config.cpu_type = 1;
#endif
#if defined(_X1TURBO) || defined(_X1TURBOZ)
	config.device_type = 1;		// Keyboard mode B
#endif
#if defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
	config.sound_device_type = 1;	// CZ-8BS1
#endif
}

void load_config()
{
	// initial settings
	init_config();
	
	// get config path
	_TCHAR app_path[_MAX_PATH], config_path[_MAX_PATH], *ptr;
	GetModuleFileName(NULL, config_path, _MAX_PATH);
	GetFullPathName(config_path, _MAX_PATH, app_path, &ptr);
	*ptr = _T('\0');
	_stprintf(config_path, _T("%s%s.ini"), app_path, _T(CONFIG_NAME));
	
	// control
#ifdef USE_BOOT_MODE
	config.boot_mode = GetPrivateProfileInt(_T("Control"), _T("BootMode"), config.boot_mode, config_path);
#endif
#ifdef USE_CPU_TYPE
	config.cpu_type = GetPrivateProfileInt(_T("Control"), _T("CPUType"), config.cpu_type, config_path);
#endif
#ifdef USE_DIPSWITCH
	config.dipswitch = GetPrivateProfileInt(_T("Control"), _T("DipSwitch"), config.dipswitch, config_path);
#endif
#ifdef USE_DEVICE_TYPE
	config.device_type = GetPrivateProfileInt(_T("Control"), _T("DeviceType"), config.device_type, config_path);
#endif
#ifdef USE_FD1
	config.ignore_crc = GetPrivateProfileBool(_T("Control"), _T("IgnoreCRC"), config.ignore_crc, config_path);
#endif
#ifdef USE_TAPE
	config.wave_shaper = GetPrivateProfileBool(_T("Control"), _T("WaveShaper"), config.wave_shaper, config_path);
	config.direct_load_mzt = GetPrivateProfileBool(_T("Control"), _T("DirectLoadMZT"), config.direct_load_mzt, config_path);
#endif
	
	// recent files
#ifdef USE_CART1
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialCartDir"), _T(""), config.initial_cart_dir, _MAX_PATH, config_path);
	for(int drv = 0; drv < MAX_CART; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf(name, _T("RecentCartPath%d_%d"), drv + 1, i + 1);
			GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_cart_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_FD1
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), _T(""), config.initial_disk_dir, _MAX_PATH, config_path);
	for(int drv = 0; drv < MAX_FD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf(name, _T("RecentDiskPath%d_%d"), drv + 1, i + 1);
			GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_disk_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_QD1
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"), _T(""), config.initial_quickdisk_dir, _MAX_PATH, config_path);
	for(int drv = 0; drv < MAX_QD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf(name, _T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1);
			GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_quickdisk_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_TAPE
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), _T(""), config.initial_tape_dir, _MAX_PATH, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		_stprintf(name, _T("RecentTapePath1_%d"), i + 1);
		GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_tape_path[i], _MAX_PATH, config_path);
	}
#endif
#ifdef USE_LASER_DISC
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), _T(""), config.initial_laser_disc_dir, _MAX_PATH, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		_stprintf(name, _T("RecentLaserDiscPath1_%d"), i + 1);
		GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_laser_disc_path[i], _MAX_PATH, config_path);
	}
#endif
#ifdef USE_BINARY_FILE1
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), _T(""), config.initial_binary_dir, _MAX_PATH, config_path);
	for(int drv = 0; drv < MAX_BINARY; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf(name, _T("RecentBinaryPath%d_%d"), drv + 1, i + 1);
			GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_binary_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
	
	// screen
#if !(defined(USE_BITMAP) || defined(USE_LED))
	config.window_mode = GetPrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
	config.use_d3d9 = GetPrivateProfileBool(_T("Screen"), _T("UseD3D9"), config.use_d3d9, config_path);
	config.wait_vsync = GetPrivateProfileBool(_T("Screen"), _T("WaitVSync"), config.wait_vsync, config_path);
	config.stretch_type = GetPrivateProfileInt(_T("Screen"), _T("StretchType"), config.stretch_type, config_path);
#endif
#ifdef USE_MONITOR_TYPE
	config.monitor_type = GetPrivateProfileInt(_T("Screen"), _T("MonitorType"), config.monitor_type, config_path);
#endif
#ifdef USE_CRT_FILTER
	config.crt_filter = GetPrivateProfileBool(_T("Screen"), _T("CRTFilter"), config.crt_filter, config_path);
#endif
#ifdef USE_SCANLINE
	config.scan_line = GetPrivateProfileBool(_T("Screen"), _T("ScanLine"), config.scan_line, config_path);
#endif
	
	// sound
	config.sound_frequency = GetPrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
	config.sound_latency = GetPrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
#ifdef USE_SOUND_DEVICE_TYPE
	config.sound_device_type = GetPrivateProfileInt(_T("Sound"), _T("DeviceType"), config.sound_device_type, config_path);
#endif
	GetPrivateProfileString(_T("Sound"), _T("FMGenDll"), _T("mamefm.dll"), config.fmgen_dll_path, _MAX_PATH, config_path);
}

void save_config()
{
	// get config path
	_TCHAR app_path[_MAX_PATH], config_path[_MAX_PATH], *ptr;
	GetModuleFileName(NULL, config_path, _MAX_PATH);
	GetFullPathName(config_path, _MAX_PATH, app_path, &ptr);
	*ptr = _T('\0');
	_stprintf(config_path, _T("%s%s.ini"), app_path, _T(CONFIG_NAME));
	
	// control
#ifdef USE_BOOT_MODE
	WritePrivateProfileInt(_T("Control"), _T("BootMode"), config.boot_mode, config_path);
#endif
#ifdef USE_CPU_TYPE
	WritePrivateProfileInt(_T("Control"), _T("CPUType"), config.cpu_type, config_path);
#endif
#ifdef USE_DIPSWITCH
	WritePrivateProfileInt(_T("Control"), _T("DipSwitch"), config.dipswitch, config_path);
#endif
#ifdef USE_DEVICE_TYPE
	WritePrivateProfileInt(_T("Control"), _T("DeviceType"), config.device_type, config_path);
#endif
#ifdef USE_FD1
	WritePrivateProfileBool(_T("Control"), _T("IgnoreCRC"), config.ignore_crc, config_path);
#endif
#ifdef USE_TAPE
	WritePrivateProfileBool(_T("Control"), _T("WaveShaper"), config.wave_shaper, config_path);
	WritePrivateProfileBool(_T("Control"), _T("DirectLoadMZT"), config.direct_load_mzt, config_path);
#endif
	
	// recent files
#ifdef USE_CART1
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialCartDir"), config.initial_cart_dir, config_path);
	for(int drv = 0; drv < MAX_CART; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf(name, _T("RecentCartPath%d_%d"), drv + 1, i + 1);
			WritePrivateProfileString(_T("RecentFiles"), name, config.recent_cart_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_FD1
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), config.initial_disk_dir, config_path);
	for(int drv = 0; drv < MAX_FD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf(name, _T("RecentDiskPath%d_%d"), drv + 1, i + 1);
			WritePrivateProfileString(_T("RecentFiles"), name, config.recent_disk_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_QD1
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"), config.initial_quickdisk_dir, config_path);
	for(int drv = 0; drv < MAX_QD; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf(name, _T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1);
			WritePrivateProfileString(_T("RecentFiles"), name, config.recent_quickdisk_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_TAPE
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), config.initial_tape_dir, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		_stprintf(name, _T("RecentTapePath1_%d"), i + 1);
		WritePrivateProfileString(_T("RecentFiles"), name, config.recent_tape_path[i], config_path);
	}
#endif
#ifdef USE_LASER_DISC
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), config.initial_laser_disc_dir, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		_stprintf(name, _T("RecentLaserDiscPath1_%d"), i + 1);
		WritePrivateProfileString(_T("RecentFiles"), name, config.recent_laser_disc_path[i], config_path);
	}
#endif
#ifdef USE_BINARY_FILE1
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), config.initial_binary_dir, config_path);
	for(int drv = 0; drv < MAX_BINARY; drv++) {
		for(int i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf(name, _T("RecentBinaryPath%d_%d"), drv + 1, i + 1);
			WritePrivateProfileString(_T("RecentFiles"), name, config.recent_binary_path[drv][i], config_path);
		}
	}
#endif
	
	// screen
#if !(defined(USE_BITMAP) || defined(USE_LED))
	WritePrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
	WritePrivateProfileBool(_T("Screen"), _T("UseD3D9"), config.use_d3d9, config_path);
	WritePrivateProfileBool(_T("Screen"), _T("WaitVSync"), config.wait_vsync, config_path);
	WritePrivateProfileInt(_T("Screen"), _T("StretchType"), config.stretch_type, config_path);
#endif
#ifdef USE_MONITOR_TYPE
	WritePrivateProfileInt(_T("Screen"), _T("MonitorType"), config.monitor_type, config_path);
#endif
#ifdef USE_CRT_FILTER
	WritePrivateProfileBool(_T("Screen"), _T("CRTFilter"), config.crt_filter, config_path);
#endif
#ifdef USE_SCANLINE
	WritePrivateProfileBool(_T("Screen"), _T("ScanLine"), config.scan_line, config_path);
#endif
	
	// sound
	WritePrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
	WritePrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
#ifdef USE_SOUND_DEVICE_TYPE
	WritePrivateProfileInt(_T("Sound"), _T("DeviceType"), config.sound_device_type, config_path);
#endif
}

#define STATE_VERSION	1

void save_config_state(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;
	
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
#ifdef USE_FD1
	state_fio->FputBool(config.ignore_crc);
#endif
#ifdef USE_MONITOR_TYPE
	state_fio->FputInt32(config.monitor_type);
#endif
#ifdef USE_SOUND_DEVICE_TYPE
	state_fio->FputInt32(config.sound_device_type);
#endif
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
#ifdef USE_FD1
	config.ignore_crc = state_fio->FgetBool();
#endif
#ifdef USE_MONITOR_TYPE
	config.monitor_type = state_fio->FgetInt32();
#endif
#ifdef USE_SOUND_DEVICE_TYPE
	config.sound_device_type = state_fio->FgetInt32();
#endif
	return true;
}

