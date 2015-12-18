/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ config ]
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "vm/vm.h"
#include "fileio.h"

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

void init_config();
void load_config(const _TCHAR* config_path);
void save_config(const _TCHAR* config_path);
void save_config_state(void *f);
bool load_config_state(void *f);

typedef struct {
	// control
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
	bool correct_disk_timing[16];
	bool ignore_disk_crc[16];
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
	
	// screen
	int window_mode;
#ifdef _WIN32
	bool use_d3d9;
	bool wait_vsync;
#endif
	int stretch_type;
#ifdef USE_MONITOR_TYPE
	int monitor_type;
#endif
#ifdef USE_CRT_FILTER
	bool crt_filter;
#endif
#ifdef USE_SCANLINE
	bool scan_line;
#endif
#ifdef USE_SCREEN_ROTATE
	int rotate_type;
#endif
	
	// sound
	int sound_frequency;
	int sound_latency;
	
	int general_sound_level;
#ifdef USE_MULTIPLE_SOUNDCARDS
	int sound_device_level[USE_MULTIPLE_SOUNDCARDS];
#endif
	bool multiple_speakers; // Effext if multiple sound boards : TRUE = not monoral.
#if defined(USE_QT)
	bool use_opengl_scanline;
	bool opengl_scanline_vert;
	bool opengl_scanline_horiz;
	
	bool use_opengl_filters;
	int opengl_filter_num;
#endif	
#ifdef USE_SOUND_DEVICE_TYPE
	int sound_device_type;
#endif
	_TCHAR fmgen_dll_path[_MAX_PATH];
	
	// input
#ifdef _WIN32
	bool use_direct_input;
	bool disable_dwm;
#endif
	bool swap_joy_buttons;
	
	// printer
#ifdef USE_PRINTER
	int printer_device_type;
	_TCHAR printer_dll_path[_MAX_PATH];
#endif
} config_t;

extern config_t config;

#if defined(_USE_AGAR) || defined(_USE_QT)
# include <string>
#endif

#endif

