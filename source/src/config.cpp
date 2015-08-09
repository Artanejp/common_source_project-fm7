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

#ifdef _USE_QT
//# include <SDL/SDL.h>
#include <string>
#include <vector>
#include "fileio.h"
#include "agar_logger.h"
#include "qt_main.h"
#else
#include <windows.h>
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

#if defined(_USE_AGAR) || defined(_USE_QT)
bool WritePrivateProfileString(char *lpAppName, char *lpKeyName, char *Value, FILEIO *lpFileName)
{
   std::string s;
  
   s = lpAppName;
   s = s + ".";
   s = s + lpKeyName + "=";
   s = s + Value + "\n";
   
   lpFileName->Fwrite((void *)s.c_str(), s.length(), 1);
   return true;
}

bool WritePrivateProfileInt(char *lpAppName, char *lpKeyName, int Value, FILEIO *lpFileName)
{
   std::string s;
   int l;
   char valuebuf[256];
   memset(valuebuf, 0x00, 256);
   
   l = snprintf(valuebuf, 254, "%d", Value);
   if((l <= 0) || (l >= 253)) return false;
   s = lpAppName;
   s = s + ".";
   s = s + lpKeyName + "=";
   s = s + valuebuf + "\n";
   lpFileName->Fwrite((void *)s.c_str(), s.length(), 1);

   return true;
}

BOOL WritePrivateProfileBool(char *lpAppName, char *lpKeyName, bool Value, FILEIO *lpFileName)
{
   int v = 0;
   if(Value) v = 1; 
   return WritePrivateProfileInt(lpAppName, lpKeyName, v, lpFileName);
}
 


std::string GetPrivateProfileStr(char *lpAppName, char *lpKeyName, FILEIO *lpFileName)
{
   std::string key;
   char ibuf[4096 + 102];
   uint64_t i;
   int l_len;
   int c = '\0';
   std::string::size_type  pos;
   std::string key_str;
   std::string got_str;
  
   key = lpAppName;
   key = key + ".";
   key = key + lpKeyName;
   AGAR_DebugLog(AGAR_LOG_DEBUG, "Try App: %s Key: %s", lpAppName, lpKeyName);
   lpFileName->Fseek(0, FILEIO_SEEK_SET);
   do {
      key_str = key;
      ibuf[0] = '\0';
      i = 0;
      l_len = 0;
      while(1) {
	if(l_len > (4096 + 100)) { // Too long, read dummy.
	   c = (char)lpFileName->Fgetc();
	   if((c != EOF) && (c != '\n') && (c != '\0')) continue;
	   break;
	}
	c = (char)lpFileName->Fgetc();
	if((c == EOF) || (c == '\n') || (c == '\0')) break;
	ibuf[i] = (char)c;
	i++;
	l_len++;
      }
      l_len = 0;
      ibuf[i] = '\0';
      got_str = ibuf;
      //AGAR_DebugLog(AGAR_LOG_DEBUG, "Got: %s %d chars.\n", got_str.c_str(), i);
      key_str = key_str + "=";
      pos = got_str.find(key_str);
      if(pos != std::string::npos) break;
      if(c == EOF) return "";
   } while(c != EOF);

   got_str.erase(0, pos + key_str.length());
   //AGAR_DebugLog(AGAR_LOG_DEBUG, "Ok. Got %s = %s.\n", key, got_str.c_str());
   return got_str;
}

void GetPrivateProfileString(char *section, char *key, char *defaultstr, char *str, int max_len, FILEIO *p)
{
   std::string sp = GetPrivateProfileStr(section, key, p);
//   printf("Got: %s\n", sp.c_str());
   
   if((!sp.empty()) && (max_len > 1)){
	strncpy(str, sp.c_str(), max_len);
   } else {
	strncpy(str, defaultstr, max_len);
   }
   printf("Got: %s\n", str);
 
}

int GetPrivateProfileInt(char *lpAppName, char *lpKeyName, int nDefault, FILEIO *lpFileName)
{
   int i;
   std::string s = GetPrivateProfileStr(lpAppName,lpKeyName, lpFileName);

   if(s.empty()) {
      i = nDefault;
   } else {
      i = strtol(s.c_str(), NULL, 10);
   }
   printf("Got: %d\n", i);
   return i;
}



bool GetPrivateProfileBool(char *lpAppName, char *lpKeyName, bool bDefault, FILEIO *lpFileName)
{
   
	return (GetPrivateProfileInt(lpAppName, lpKeyName, bDefault ? 1 : 0, lpFileName) != 0);
}
   
#else
extern _TCHAR* get_parent_dir(_TCHAR* file);
BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int Value, LPCTSTR lpFileName)
{
	_TCHAR String[32];
	_stprintf_s(String, 32, _T("%d"), Value);
	return WritePrivateProfileString(lpAppName, lpKeyName, String, lpFileName);
}

BOOL WritePrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool Value, LPCTSTR lpFileName)
{
	_TCHAR String[32];
	_stprintf_s(String, 32, _T("%d"), Value ? 1 : 0);
	return WritePrivateProfileString(lpAppName, lpKeyName, String, lpFileName);
}

BOOL GetPrivateProfileBool(LPCTSTR lpAppName, LPCTSTR lpKeyName, bool bDefault, LPCTSTR lpFileName)
{
	return (GetPrivateProfileInt(lpAppName, lpKeyName, bDefault ? 1 : 0, lpFileName) != 0);
}

#endif
void init_config()
{
	int i;
	// initial settings
	memset(&config, 0, sizeof(config_t));
	
#ifdef _WIN32
	config.use_direct_input = true;
	config.disable_dwm = false;
#endif
	config.swap_joy_buttons = false;
	
#if !(defined(USE_BITMAP) || defined(USE_LED))
#ifdef _WIN32
	config.use_d3d9 = true;
#endif
	config.stretch_type = 1;	// Stretch (Aspect)
#endif
	config.sound_frequency = 6;	// 48KHz
	config.sound_latency = 1;	// 100msec
	
#if defined(USE_TAPE)
	config.wave_shaper = true;
	config.direct_load_mzt = true;
	config.baud_high = true;
#endif
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
//	for(i = 0; i <8; i++) config.fdd_hack_fast_transfer[i] = false;
#endif	
#if defined(USE_SOUND_DEVICE_TYPE) && defined(SOUND_DEVICE_TYPE_DEFAULT)
	config.sound_device_type = SOUND_DEVICE_TYPE_DEFAULT;
#endif
	// FM7 Series:
	// 0 = PSG or NONE
	// 1 = OPN (+PSG)
	// 2 = WHG (+PSG)
	// 3 = WHG + OPN (+PSG)
	// 4 = THG  (+PSG)
	// 5 = THG + OPN (+PSG)
	// 6 = THG + WHG (+PSG)
	// 7 = THG + WHG + OPN (+PSG)
#if defined(_FM8)
	config.sound_device_type = 0;	// WITHOUT PSG?
#elif  defined(_FM7) || defined(_FMNEW7) || defined(_FM77) || defined(_FM77L4) || defined(_FM77L2)
	config.sound_device_type = 0;   // PSG ONLY      
#elif  defined(_FM77AV) || defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	config.sound_device_type = 1;   // OPN      
#endif	     
	config.multiple_speakers = false;
	config.general_sound_level = 0;

#if defined(USE_QT)
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

void load_config()
{
	int drv, i;
	// initial settings
	init_config();
	
	// get config path

#if defined(_USE_AGAR) || defined(_USE_QT) 
	char app_path2[_MAX_PATH], *ptr;
        FILEIO *config_path = new FILEIO();
   
        memset(app_path2, 0x00, _MAX_PATH);
        cpp_confdir.copy(app_path2, _MAX_PATH, 0);
        
        strncat(app_path2, CONFIG_NAME, _MAX_PATH);
        strncat(app_path2, ".ini", _MAX_PATH);

        AGAR_DebugLog(AGAR_LOG_INFO, "Try to read config: %s", app_path2);
        if(!config_path->Fopen(app_path2, FILEIO_READ_ASCII)) return;
#else
	_TCHAR app_path[_MAX_PATH], config_path[_MAX_PATH], *ptr;
	memset(config_path, 0x00, _MAX_PATH);
	memset(app_path, 0x00, _MAX_PATH);
	GetModuleFileName(NULL, config_path, _MAX_PATH);
	GetFullPathName(config_path, _MAX_PATH, app_path, &ptr);
	if(ptr != NULL) *ptr = _T('\0');
	_stprintf_s(config_path, _MAX_PATH, _T("%s%s.ini"), app_path, _T(CONFIG_NAME));
#endif
   
	
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
#ifdef USE_DRIVE_TYPE
	config.drive_type = GetPrivateProfileInt(_T("Control"), _T("DriveType"), config.drive_type, config_path);
#endif
#ifdef USE_FD1
	{
		_TCHAR name[64];
		for(drv = 0; drv < MAX_FD; drv++) {
			memset(name, 0x00, sizeof(name));
			_stprintf_s(name, 64, _T("CorrectDiskTiming%d"), drv + 1);
			config.correct_disk_timing[drv] = GetPrivateProfileBool(_T("Control"), name, config.correct_disk_timing[drv], config_path);
			_stprintf_s(name, 64, _T("IgnoreDiskCRC%d"), drv + 1);
			config.ignore_disk_crc[drv] = GetPrivateProfileBool(_T("Control"), name, config.ignore_disk_crc[drv], config_path);
		}
	}
#endif

#ifdef USE_TAPE
	config.tape_sound = GetPrivateProfileBool(_T("Control"), _T("TapeSound"), config.tape_sound, config_path);
	config.wave_shaper = GetPrivateProfileBool(_T("Control"), _T("WaveShaper"), config.wave_shaper, config_path);
	config.direct_load_mzt = GetPrivateProfileBool(_T("Control"), _T("DirectLoadMZT"), config.direct_load_mzt, config_path);
	config.baud_high = GetPrivateProfileBool(_T("Control"), _T("BaudHigh"), config.baud_high, config_path);
#endif
	
	// recent files
#ifdef USE_CART1
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialCartDir"), _T(""), config.initial_cart_dir, _MAX_PATH, config_path);
	for(drv = 0; drv < MAX_CART; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
		        _stprintf_s(name, 64, _T("RecentCartPath%d_%d"), drv + 1, i + 1);
			GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_cart_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_FD1
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), _T(""), config.initial_disk_dir, _MAX_PATH, config_path);
    //    get_parent_dir(config.initial_disk_dir);
	for(drv = 0; drv < MAX_FD; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf_s(name, 64, _T("RecentDiskPath%d_%d"), drv + 1, i + 1);
			GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_disk_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_QD1
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"), _T(""), config.initial_quickdisk_dir, _MAX_PATH, config_path);
	for(drv = 0; drv < MAX_QD; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf_s(name, 64, _T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1);
			GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_quickdisk_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
#ifdef USE_TAPE
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), _T(""), config.initial_tape_dir, _MAX_PATH, config_path);
	for(i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		_stprintf_s(name, 64, _T("RecentTapePath1_%d"), i + 1);
		GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_tape_path[i], _MAX_PATH, config_path);
	}
#endif
#ifdef USE_LASER_DISC
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), _T(""), config.initial_laser_disc_dir, _MAX_PATH, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		_stprintf_s(name, 64, _T("RecentLaserDiscPath1_%d"), i + 1);
		GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_laser_disc_path[i], _MAX_PATH, config_path);
	}
#endif
#ifdef USE_BINARY_FILE1
	GetPrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), _T(""), config.initial_binary_dir, _MAX_PATH, config_path);
	for(drv = 0; drv < MAX_BINARY; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf_s(name, 64, _T("RecentBinaryPath%d_%d"), drv + 1, i + 1);
			GetPrivateProfileString(_T("RecentFiles"), name, _T(""), config.recent_binary_path[drv][i], _MAX_PATH, config_path);
		}
	}
#endif
	
	// screen
#if !(defined(USE_BITMAP) || defined(USE_LED))
	config.window_mode = GetPrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
#ifdef _WIN32
	config.use_d3d9 = GetPrivateProfileBool(_T("Screen"), _T("UseD3D9"), config.use_d3d9, config_path);
	config.wait_vsync = GetPrivateProfileBool(_T("Screen"), _T("WaitVSync"), config.wait_vsync, config_path);
#endif
	config.stretch_type = GetPrivateProfileInt(_T("Screen"), _T("StretchType"), config.stretch_type, config_path);
#else
	config.window_mode = GetPrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
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

#ifdef USE_SCREEN_ROTATE
	config.rotate_type = GetPrivateProfileBool(_T("Screen"), _T("RotateType"), config.rotate_type, config_path);
#endif
#if defined(USE_QT)
	config.use_opengl_scanline = GetPrivateProfileBool(_T("Screen"), _T("UseOpenGLScanLine"),
													   config.use_opengl_scanline, config_path);
	config.opengl_scanline_vert = GetPrivateProfileBool(_T("Screen"), _T("OpenGLScanLineVert"),
													   config.opengl_scanline_vert, config_path);;
	config.opengl_scanline_horiz = GetPrivateProfileBool(_T("Screen"), _T("OpenGLScanLineHoriz"),
													   config.opengl_scanline_horiz, config_path);;
	config.use_opengl_filters = GetPrivateProfileBool(_T("Screen"), _T("UseOpenGLFilters"),
													   config.use_opengl_filters, config_path);
	config.opengl_filter_num = 	GetPrivateProfileInt(_T("Screen"), _T("OpenGlFilterNum"),
													 config.opengl_filter_num, config_path);
#endif	
	// sound
	config.sound_frequency = GetPrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
	config.sound_latency = GetPrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
#ifdef USE_SOUND_DEVICE_TYPE
	config.sound_device_type = GetPrivateProfileInt(_T("Sound"), _T("DeviceType"), config.sound_device_type, config_path);
#endif
 	GetPrivateProfileString(_T("Sound"), _T("FMGenDll"), _T("mamefm.dll"), config.fmgen_dll_path, _MAX_PATH, config_path);
	
	// input
	config.multiple_speakers = GetPrivateProfileBool(_T("Sound"), _T("MultipleSpeakers"),
													 config.multiple_speakers, config_path);
	config.general_sound_level = GetPrivateProfileInt(_T("Sound"), _T("GeneralSoundLevel"),
													  config.general_sound_level, config_path);
#ifdef USE_MULTIPLE_SOUNDCARDS
	{
		_TCHAR _tag[128];
		int ii;
		for(ii = 0; ii < USE_MULTIPLE_SOUNDCARDS; ii++) {
			memset(_tag, 0x00, sizeof(_tag));
			_stprintf_s(_tag, 64, _T("DeviceVolumeLevel_%d"), ii + 1);
			config.sound_device_level[ii] = GetPrivateProfileInt(_T("Sound"), _tag, config.sound_device_level[ii], config_path);
		}
	}
#endif
#ifdef _WIN32
	config.use_direct_input = GetPrivateProfileBool(_T("Input"), _T("UseDirectInput"), config.use_direct_input, config_path);
	config.disable_dwm = GetPrivateProfileBool(_T("Input"), _T("DisableDwm"), config.disable_dwm, config_path);
#endif
	config.swap_joy_buttons = GetPrivateProfileBool(_T("Input"), _T("SwapJoyButtons"), config.swap_joy_buttons, config_path);
#if defined(_USE_AGAR) || defined(_USE_QT)
     config_path->Fclose();
     delete config_path;
     AGAR_DebugLog(AGAR_LOG_INFO, "Read Done.");
#endif
}

void save_config()
{
	int drv, i;
	// get config path
#if defined(_USE_AGAR) || defined(_USE_QT)
	char app_path2[_MAX_PATH], *ptr;
        FILEIO *config_path = new FILEIO();
   
        app_path2[0] = '\0';
        memset(app_path2, 0x00, _MAX_PATH);
        cpp_confdir.copy(app_path2, _MAX_PATH, 0);
        
        strncat(app_path2, CONFIG_NAME, _MAX_PATH);
        strncat(app_path2, ".ini", _MAX_PATH);

        AGAR_DebugLog(AGAR_LOG_INFO, "Try to write config: %s", app_path2);
        if(config_path->Fopen(app_path2, FILEIO_WRITE_ASCII) != true) return;
#else
        _TCHAR app_path[_MAX_PATH], config_path[_MAX_PATH], *ptr;
	memset(config_path, 0x00, _MAX_PATH);
	memset(app_path, 0x00, _MAX_PATH);
	GetModuleFileName(NULL, config_path, _MAX_PATH);
	GetFullPathName(config_path, _MAX_PATH, app_path, &ptr);
	if(ptr != NULL) *ptr = _T('\0');
	_stprintf_s(config_path, _MAX_PATH, _T("%s%s.ini"), app_path, _T(CONFIG_NAME));
#endif	
	// control
# ifdef USE_BOOT_MODE
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
#ifdef USE_DRIVE_TYPE
	WritePrivateProfileInt(_T("Control"), _T("DriveType"), config.drive_type, config_path);
#endif
#ifdef USE_FD1
	{
		_TCHAR name[64];
		for(drv = 0; drv < MAX_FD; drv++) {
			memset(name, 0x00, sizeof(name));
			_stprintf_s(name, 64, _T("CorrectDiskTiming%d"), drv + 1);
			WritePrivateProfileBool(_T("Control"), name, config.correct_disk_timing[drv], config_path);
			_stprintf_s(name, 64, _T("IgnoreDiskCRC%d"), drv + 1);
			WritePrivateProfileBool(_T("Control"), name, config.ignore_disk_crc[drv], config_path);
		}
	}

#endif
#ifdef USE_TAPE
	WritePrivateProfileBool(_T("Control"), _T("TapeSound"), config.tape_sound, config_path);
	WritePrivateProfileBool(_T("Control"), _T("WaveShaper"), config.wave_shaper, config_path);
	WritePrivateProfileBool(_T("Control"), _T("DirectLoadMZT"), config.direct_load_mzt, config_path);
	WritePrivateProfileBool(_T("Control"), _T("BaudHigh"), config.baud_high, config_path);
#endif
	
	// recent files
#ifdef USE_CART1
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialCartDir"), config.initial_cart_dir, config_path);
	for(drv = 0; drv < MAX_CART; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf_s(name, 64, _T("RecentCartPath%d_%d"), drv + 1, i + 1);
			WritePrivateProfileString(_T("RecentFiles"), name, config.recent_cart_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_FD1
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialDiskDir"), config.initial_disk_dir, config_path);
	for(drv = 0; drv < MAX_FD; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf_s(name, 64, _T("RecentDiskPath%d_%d"), drv + 1, i + 1);
			WritePrivateProfileString(_T("RecentFiles"), name, config.recent_disk_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_QD1
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialQuickDiskDir"), config.initial_quickdisk_dir, config_path);
	for(drv = 0; drv < MAX_QD; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf_s(name, 64, _T("RecentQuickDiskPath%d_%d"), drv + 1, i + 1);
			WritePrivateProfileString(_T("RecentFiles"), name, config.recent_quickdisk_path[drv][i], config_path);
		}
	}
#endif
#ifdef USE_TAPE
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialTapeDir"), config.initial_tape_dir, config_path);
	for(i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		_stprintf_s(name, 64, _T("RecentTapePath1_%d"), i + 1);
		WritePrivateProfileString(_T("RecentFiles"), name, config.recent_tape_path[i], config_path);
	}
#endif
#ifdef USE_LASER_DISC
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialLaserDiscDir"), config.initial_laser_disc_dir, config_path);
	for(int i = 0; i < MAX_HISTORY; i++) {
		_TCHAR name[64];
		_stprintf_s(name, 64, _T("RecentLaserDiscPath1_%d"), i + 1);
		WritePrivateProfileString(_T("RecentFiles"), name, config.recent_laser_disc_path[i], config_path);
	}
#endif
#ifdef USE_BINARY_FILE1
	WritePrivateProfileString(_T("RecentFiles"), _T("InitialBinaryDir"), config.initial_binary_dir, config_path);
	for(drv = 0; drv < MAX_BINARY; drv++) {
		for(i = 0; i < MAX_HISTORY; i++) {
			_TCHAR name[64];
			_stprintf_s(name, 64, _T("RecentBinaryPath%d_%d"), drv + 1, i + 1);
			WritePrivateProfileString(_T("RecentFiles"), name, config.recent_binary_path[drv][i], config_path);
		}
	}
#endif
	
	// screen
#if !(defined(USE_BITMAP) || defined(USE_LED))
	WritePrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
#ifdef _WIN32
	WritePrivateProfileBool(_T("Screen"), _T("UseD3D9"), config.use_d3d9, config_path);
	WritePrivateProfileBool(_T("Screen"), _T("WaitVSync"), config.wait_vsync, config_path);
#endif
	WritePrivateProfileInt(_T("Screen"), _T("StretchType"), config.stretch_type, config_path);
#else
	WritePrivateProfileInt(_T("Screen"), _T("WindowMode"), config.window_mode, config_path);
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
#ifdef USE_SCREEN_ROTATE
	WritePrivateProfileBool(_T("Screen"), _T("RotateType"), config.rotate_type, config_path);
#endif
#if defined(USE_QT)
	WritePrivateProfileBool(_T("Screen"), _T("UseOpenGLScanLine"),
							config.use_opengl_scanline, config_path);
	WritePrivateProfileBool(_T("Screen"), _T("OpenGLScanLineVert"),
							config.opengl_scanline_vert, config_path);;
	WritePrivateProfileBool(_T("Screen"), _T("OpenGLScanLineHoriz"),
							config.opengl_scanline_horiz, config_path);;
	WritePrivateProfileBool(_T("Screen"), _T("UseOpenGLFilters"),
							config.use_opengl_filters, config_path);
	WritePrivateProfileInt(_T("Screen"), _T("OpenGlFilterNum"),
						   config.opengl_filter_num, config_path);
#endif	
	
	// sound
	WritePrivateProfileInt(_T("Sound"), _T("Frequency"), config.sound_frequency, config_path);
	WritePrivateProfileInt(_T("Sound"), _T("Latency"), config.sound_latency, config_path);
#ifdef USE_SOUND_DEVICE_TYPE
	WritePrivateProfileInt(_T("Sound"), _T("DeviceType"), config.sound_device_type, config_path);
#endif
	WritePrivateProfileBool(_T("Sound"), _T("MultipleSpeakers"),
							config.multiple_speakers, config_path);
	WritePrivateProfileInt(_T("Sound"), _T("GeneralSoundLevel"),
						   config.general_sound_level, config_path);
#ifdef USE_MULTIPLE_SOUNDCARDS
	{
		_TCHAR _tag[128];
		int ii;
		for(ii = 0; ii < USE_MULTIPLE_SOUNDCARDS; ii++) {
			memset(_tag, 0x00, sizeof(_tag));
			_stprintf_s(_tag, 64, _T("DeviceVolumeLevel_%d"), ii + 1);
			WritePrivateProfileInt(_T("Sound"), _tag, config.sound_device_level[ii], config_path);
		}
	}
#endif
	// input
#ifdef _WIN32
	WritePrivateProfileBool(_T("Input"), _T("UseDirectInput"), config.use_direct_input, config_path);
	WritePrivateProfileBool(_T("Input"), _T("DisableDwm"), config.disable_dwm, config_path);
#endif
	WritePrivateProfileBool(_T("Input"), _T("SwapJoyButtons"), config.swap_joy_buttons, config_path);
#if defined(_USE_AGAR) || defined(_USE_QT)
        config_path->Fclose();
        delete config_path;
        AGAR_DebugLog(AGAR_LOG_INFO, "Write done.");
#endif

}

#define STATE_VERSION	3

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
	return true;
}

