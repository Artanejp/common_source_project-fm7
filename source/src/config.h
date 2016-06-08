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

void DLL_PREFIX initialize_config();
void DLL_PREFIX load_config(const _TCHAR* config_path);
void DLL_PREFIX save_config(const _TCHAR* config_path);
void DLL_PREFIX save_config_state(void *f);
bool DLL_PREFIX load_config_state(void *f);


/*
 * 20160407 Ohta:
 * Qt:
 *  To reduce time to build, compiling common blocks of GUI at once.
 *  So, you should not separate items with #ifdef.
 */ 
typedef struct {
	// control
	int boot_mode;
	int cpu_type;
	int cpu_power;
	uint32_t dipswitch;
	int device_type;
	int drive_type;
	bool correct_disk_timing[16];
	bool ignore_disk_crc[16];
	bool tape_sound;
	bool wave_shaper;
	bool direct_load_mzt;
	bool baud_high;
	// recent files
	_TCHAR initial_cart_dir[_MAX_PATH];
	_TCHAR recent_cart_path[8][MAX_HISTORY][_MAX_PATH];
	_TCHAR initial_floppy_disk_dir[_MAX_PATH];
	_TCHAR recent_floppy_disk_path[16][MAX_HISTORY][_MAX_PATH];
	_TCHAR initial_quick_disk_dir[_MAX_PATH];
	_TCHAR recent_quick_disk_path[8][MAX_HISTORY][_MAX_PATH];
	_TCHAR initial_tape_dir[_MAX_PATH];
	_TCHAR recent_tape_path[MAX_HISTORY][_MAX_PATH];
	_TCHAR initial_compact_disc_dir[_MAX_PATH];
	_TCHAR recent_compact_disc_path[MAX_HISTORY][_MAX_PATH];
	_TCHAR initial_laser_disc_dir[_MAX_PATH];
	_TCHAR recent_laser_disc_path[MAX_HISTORY][_MAX_PATH];
	_TCHAR initial_binary_dir[_MAX_PATH];
	_TCHAR recent_binary_path[8][MAX_HISTORY][_MAX_PATH];
	_TCHAR initial_bubble_casette_dir[_MAX_PATH];
	_TCHAR recent_bubble_casette_path[16][MAX_HISTORY][_MAX_PATH];
	// screen
	int window_mode;
#ifdef _WIN32
	bool use_d3d9;
	bool wait_vsync;
#endif
	int window_stretch_type;
	int fullscreen_stretch_type;
	int monitor_type;
	bool crt_filter;
	bool scan_line;
	int rotate_type;
#ifdef _USE_QT
	bool use_opengl_scanline;
	bool opengl_scanline_vert;
	bool opengl_scanline_horiz;
	bool use_opengl_filters;
	int opengl_filter_num;

	bool swap_kanji_pause;
#endif	
	
	// sound
	int sound_frequency;
	int sound_latency;
	
	int general_sound_level;
	int sound_device_type;
	int sound_volume_l[32];
	int sound_volume_r[32];
	_TCHAR fmgen_dll_path[_MAX_PATH];
	
	// input
#ifdef _WIN32
	bool use_direct_input;
	bool disable_dwm;
#endif
	int keyboard_type;
	int joy_buttons[4][16];
#ifdef _USE_QT
	_TCHAR assigned_joystick_name[16][256];

	int video_width;
	int video_height;
	int video_bitrate;
	int video_bframes;
	int video_b_adapt;
	int video_minq;
	int video_maxq;
	int video_subme;
	int video_threads;
	int audio_bitrate;
	int video_frame_rate; // FPS * 1000.0
#endif	
	// printer
	int printer_device_type;
	_TCHAR printer_dll_path[_MAX_PATH];
} config_t;

extern config_t config;

#if defined(_USE_AGAR) || defined(_USE_QT)
# include <string>
#endif

#endif

