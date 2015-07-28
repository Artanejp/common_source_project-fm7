/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ config ]
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

# if defined(_USE_AGAR) || defined(_USE_SDL)
#include <sys/param.h>
#include <agar/core.h>
#ifndef _MAX_PATH
 #define _MAX_PATH AG_PATHNAME_MAX
#endif
# elif defined(_USE_QT)
#include <sys/param.h>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#ifndef _MAX_PATH
 #define _MAX_PATH 2048
#endif
#endif
#include "vm/vm.h"
#include "fileio.h"

// LOG COMMAND
#define EMU_LOG_CPU1        0x00000001
#define EMU_LOG_CPU2        0x00000002
#define EMU_LOG_CPU3        0x00000004
#define EMU_LOG_CPU4        0x00000008
#define EMU_LOG_FLOPPY      0x00000010
#define EMU_LOG_CMT         0x00000020
#define EMU_LOG_QD          0x00000040
#define EMU_LOG_CART        0x00000080
#define EMU_LOG_BINARY      0x00000100
#define EMU_LOG_LASERDISC   0x00000200
#define EMU_LOG_DISPLAY     0x00001000
#define EMU_LOG_SOUND       0x00002000
#define EMU_LOG_KEYBOARD    0x00004000
#define EMU_LOG_IO          0x00008000
#define EMU_LOG_MEMORY      0x00010000
#define EMU_LOG_USR1        0x00020000
#define EMU_LOG_USR2        0x00040000
#define EMU_LOG_USR3        0x00080000
#define EMU_LOG_USR4        0x00100000
#define EMU_LOG_USR5        0x00200000
#define EMU_LOG_USR6        0x00400000
#define EMU_LOG_USR7        0x00800000
#define EMU_LOG_USR8        0x01000000
#define EMU_LOG_USR9        0x02000000
#define EMU_LOG_USR10       0x04000000
#define EMU_LOG_GUI         0x08000000
#define EMU_LOG_DEBUG       0x10000000
#define EMU_LOG_INFO        0x20000000
#define EMU_LOG_WARNING     0x40000000
#define EMU_LOG_GENERAL     0x80000000

#define MAX_HISTORY	8

#if defined(USE_CART2)
#define MAX_CART	2
#elif defined(USE_CART1)
#define MAX_CART	1
#endif

#if defined(USE_FD8)
#define MAX_FD		8
#elif defined(USE_FD7)
#define MAX_FD		7
#elif defined(USE_FD6)
#define MAX_FD		6
#elif defined(USE_FD5)
#define MAX_FD		5
#elif defined(USE_FD4)
#define MAX_FD		4
#elif defined(USE_FD3)
#define MAX_FD		3
#elif defined(USE_FD2)
#define MAX_FD		2
#elif defined(USE_FD1)
#define MAX_FD		1
#endif

#if defined(USE_QD2)
#define MAX_QD		2
#elif defined(USE_QD1)
#define MAX_QD		1
#endif

#if defined(USE_BINARY_FILE2)
#define MAX_BINARY	2
#elif defined(USE_BINARY_FILE1)
#define MAX_BINARY	1
#endif

#ifdef __cplusplus
void init_config();
void load_config();
void save_config();
void save_config_state(void *f);
bool load_config_state(void *f);
#endif

typedef struct {
	// control
	bool use_direct_input;
	bool disable_dwm;
	
#ifdef USE_BOOT_MODE
	int boot_mode;
#endif
#ifdef USE_CPU_TYPE
	int cpu_type;
#endif
	int cpu_power;
#ifdef USE_DIPSWITCH
	uint32 dipswitch;
#endif
#ifdef USE_DEVICE_TYPE
	int device_type;
#endif
#ifdef USE_DRIVE_TYPE
	int drive_type;
#endif
#ifdef USE_FD1
	bool ignore_crc[8];
#endif
#ifdef USE_TAPE
	bool tape_sound;
	bool wave_shaper;
	bool direct_load_mzt;
	bool baud_high;
#endif
	
	// recent files
#ifdef USE_CART1
	_TCHAR initial_cart_dir[_MAX_PATH];
	_TCHAR recent_cart_path[MAX_CART][MAX_HISTORY][_MAX_PATH];
#endif
#ifdef USE_FD1
	_TCHAR initial_disk_dir[_MAX_PATH];
	_TCHAR recent_disk_path[MAX_FD][MAX_HISTORY][_MAX_PATH];
#endif
#ifdef USE_QD1
	_TCHAR initial_quickdisk_dir[_MAX_PATH];
	_TCHAR recent_quickdisk_path[MAX_QD][MAX_HISTORY][_MAX_PATH];
#endif
#ifdef USE_TAPE
	_TCHAR initial_tape_dir[_MAX_PATH];
	_TCHAR recent_tape_path[MAX_HISTORY][_MAX_PATH];
#endif
#ifdef USE_LASER_DISC
	_TCHAR initial_laser_disc_dir[_MAX_PATH];
	_TCHAR recent_laser_disc_path[MAX_HISTORY][_MAX_PATH];
#endif
#ifdef USE_BINARY_FILE1
	_TCHAR initial_binary_dir[_MAX_PATH];
	_TCHAR recent_binary_path[MAX_BINARY][MAX_HISTORY][_MAX_PATH];
#endif
#ifdef USE_SCREEN_ROTATE
	bool rotate_type; // FIXME: wanna support +90deg, +180deg and +270deg
#endif
	
	// screen
	int window_mode;
	bool use_d3d9;
	bool wait_vsync;
	int stretch_type;
#ifdef USE_MONITOR_TYPE
	int monitor_type;
#endif
#ifdef USE_CRT_FILTER
	BOOL crt_filter;
#endif
#ifdef USE_SCANLINE
	bool scan_line;
#endif
	// sound
	int sound_frequency;
	int sound_latency;
#ifdef USE_SOUND_DEVICE_TYPE
	int sound_device_type;
#endif
	_TCHAR fmgen_dll_path[_MAX_PATH];
} config_t;

extern config_t config;

#if defined(_USE_AGAR) || defined(_USE_QT)

# include <string>

extern bool WritePrivateProfileString(char *lpAppName, char *lpKeyName, char *Value, FILEIO *lpFileName);
extern bool WritePrivateProfileInt(char *lpAppName, char *lpKeyName, int Value, FILEIO *lpFileName);
extern BOOL WritePrivateProfileBool(char *lpAppName, char *lpKeyName, bool Value, FILEIO *lpFileName);
extern std::string GetPrivateProfileStr(char *lpAppName, char *lpKeyName, FILEIO *lpFileName);
extern void GetPrivateProfileString(char *section, char *key, char *defaultstr, char *str, int max_len, FILEIO *p);
extern int GetPrivateProfileInt(char *lpAppName, char *lpKeyName, int nDefault, FILEIO *lpFileName);
extern bool GetPrivateProfileBool(char *lpAppName, char *lpKeyName, bool bDefault, FILEIO *lpFileName);

#endif

#endif

