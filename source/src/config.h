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
#if defined(_USE_QT)
#include "qt/gui/csp_logger.h"
#endif

enum {
	CONFIG_RENDER_TYPE_STD = 0,
	CONFIG_RENDER_TYPE_TV,
	CONFIG_RENDER_TYPE_END
};
enum {
	CONFIG_RENDER_PLATFORM_OPENGL_MAIN = 0,
	CONFIG_RENDER_PLATFORM_OPENGL_CORE = 1,
	CONFIG_RENDER_PLATFORM_OPENGL_ES   = 2,
	CONFIG_RENDER_PLATFORM_QT3D        = 8,
	CONFIG_RENDER_PLATFORM_VULKAN      = 16,
	CONFIG_RENDER_PLATFORM_DIRECTDRAW  = 24,
	CONFIG_RENDER_PLATFORM_SDLFB       = 25,
	CONFIG_RENDER_PLATFORM_DIRECTX     = 32,
};

#define MAX_HISTORY	8

#ifndef MAX_CART
	#if defined(USE_CART2)
		#define MAX_CART	2
	#elif defined(USE_CART1)
		#define MAX_CART	1
	#endif
#endif

#ifndef MAX_FD
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
#endif

#ifndef MAX_QD
	#if defined(USE_QD2)
		#define MAX_QD		2
	#elif defined(USE_QD1)
		#define MAX_QD		1
	#endif
#endif

#ifndef MAX_BINARY
	#if defined(USE_BINARY_FILE2)
		#define MAX_BINARY	2
	#elif defined(USE_BINARY_FILE1)
		#define MAX_BINARY	1
	#endif
#endif

#ifndef MAX_BUBBLE
	#if defined(USE_BUBBLE2)
		#define MAX_BUBBLE	2
	#elif defined(USE_BUBBLE1)
		#define MAX_BUBBLE	1
	#endif
#endif

#ifdef USE_SHARED_DLL
	#define MAX_CART_TMP	8
	#define MAX_FD_TMP	16
	#define MAX_QD_TMP	8
	#define MAX_BINARY_TMP	8
	#define MAX_BUBBLE_TMP	16
	#define MAX_VOLUME_TMP	32
#else
	#define MAX_CART_TMP	MAX_CART
	#define MAX_FD_TMP	MAX_FD
	#define MAX_QD_TMP	MAX_QD
	#define MAX_BINARY_TMP	MAX_BINARY
	#define MAX_BUBBLE_TMP	MAX_BUBBLE
	#ifdef USE_SOUND_VOLUME
		#define MAX_VOLUME_TMP	USE_SOUND_VOLUME
	#endif
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
#if defined(USE_SHARED_DLL) || defined(USE_BOOT_MODE)
	int boot_mode;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_CPU_TYPE)
	int cpu_type;
#endif
	int cpu_power;
#if defined(USE_SHARED_DLL) || defined(USE_DIPSWITCH)
	uint32_t dipswitch;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_DEVICE_TYPE)
	int device_type;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_DRIVE_TYPE)
	int drive_type;
#endif
	
#if defined(USE_SHARED_DLL) || defined(USE_FD1)
 	bool correct_disk_timing[16];
 	bool ignore_disk_crc[16];
#endif
#if defined(USE_SHARED_DLL) || defined(USE_TAPE)
	bool wave_shaper;
	bool direct_load_mzt;
	bool baud_high;
#endif
	// recent files
#if defined(USE_SHARED_DLL) || defined(USE_CART1)
	_TCHAR initial_cart_dir[_MAX_PATH];
	_TCHAR recent_cart_path[MAX_CART_TMP][MAX_HISTORY][_MAX_PATH];
#endif
#if defined(USE_SHARED_DLL) || defined(USE_FD1)
	_TCHAR initial_floppy_disk_dir[_MAX_PATH];
	_TCHAR recent_floppy_disk_path[MAX_FD_TMP][MAX_HISTORY][_MAX_PATH];
#endif
#if defined(USE_SHARED_DLL) || defined(USE_QD1)
	_TCHAR initial_quick_disk_dir[_MAX_PATH];
	_TCHAR recent_quick_disk_path[MAX_QD_TMP][MAX_HISTORY][_MAX_PATH];
#endif
#if defined(USE_SHARED_DLL) || defined(USE_TAPE)
	_TCHAR initial_tape_dir[_MAX_PATH];
	_TCHAR recent_tape_path[MAX_HISTORY][_MAX_PATH];
#endif
#if defined(USE_SHARED_DLL) || defined(USE_COMPACT_DISC)
	_TCHAR initial_compact_disc_dir[_MAX_PATH];
	_TCHAR recent_compact_disc_path[MAX_HISTORY][_MAX_PATH];
#endif
#if defined(USE_SHARED_DLL) || defined(USE_LASER_DISC)
	_TCHAR initial_laser_disc_dir[_MAX_PATH];
	_TCHAR recent_laser_disc_path[MAX_HISTORY][_MAX_PATH];
#endif
#if defined(USE_SHARED_DLL) || defined(USE_BINARY_FILE1)
	_TCHAR initial_binary_dir[_MAX_PATH];
	_TCHAR recent_binary_path[MAX_BINARY_TMP][MAX_HISTORY][_MAX_PATH];
#endif
#if defined(USE_SHARED_DLL) || defined(USE_BUBBLE1)
 	_TCHAR initial_bubble_casette_dir[_MAX_PATH];
	_TCHAR recent_bubble_casette_path[MAX_BUBBLE_TMP][MAX_HISTORY][_MAX_PATH];
#endif
	// screen
	int window_mode;
#ifdef _WIN32
	bool use_d3d9;
	bool wait_vsync;
#endif
	int window_stretch_type;
	int fullscreen_stretch_type;
#if defined(USE_SHARED_DLL) || defined(USE_MONITOR_TYPE)
 	int monitor_type;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_CRT_FILTER)
 	bool crt_filter;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_SCANLINE)
	bool scan_line;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_SCREEN_ROTATE)
 	int rotate_type;
#endif
#if defined(_USE_QT)
	bool use_opengl_scanline;
	bool opengl_scanline_vert;
	bool opengl_scanline_horiz;
	bool use_opengl_filters;
	int opengl_filter_num;

	bool swap_kanji_pause;
	/*
	 * TYPE : 
	 *    0 : OpenGL/Main Profile
	 *    1 : OpenGL/Core Profile
	 *    2 : OpenGL ES
	 *    8 : Qt3D(Will not implement)
	 *   16 : Vulkan (Will not implement)
	 *   24 : DirectDraw (Will not implement)
	 *   25 : SDLFB(Will not implement)
     *   32 : DirectX(Will not implement)
	 */ 
	int render_platform;
	int render_major_version;
	int render_minor_version;
#endif	
	
	// sound
	int sound_frequency;
	int sound_latency;
	bool sound_strict_rendering;
#if defined(_USE_QT)	
	int general_sound_level;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_SOUND_DEVICE_TYPE)
	int sound_device_type;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_FD1)
	bool sound_noise_fdd;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_TAPE)
	bool sound_noise_cmt;
	bool sound_play_tape;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_SOUND_VOLUME)
	int sound_volume_l[MAX_VOLUME_TMP];
	int sound_volume_r[MAX_VOLUME_TMP];
#endif
	// input
#ifdef _WIN32
	_TCHAR fmgen_dll_path[_MAX_PATH];
	bool use_direct_input;
	bool disable_dwm;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_KEYBOARD_TYPE)
	int keyboard_type;
#endif
#if defined(USE_SHARED_DLL) || defined(USE_JOYSTICK)
	int joy_buttons[4][16];
#endif

#ifdef _USE_QT
	_TCHAR assigned_joystick_name[16][256];

	int video_width;
	int video_height;
	int video_codec_type;
	int audio_codec_type;
	
	int video_h264_bitrate;
	int video_h264_bframes;
	int video_h264_b_adapt;
	int video_h264_minq;
	int video_h264_maxq;
	int video_h264_subme;

	int video_mpeg4_bitrate;
	int video_mpeg4_bframes;
	int video_mpeg4_minq;
	int video_mpeg4_maxq;
	
	int video_threads;
	int audio_bitrate;
	int video_frame_rate; // FPS * 1000.0
#endif
	
	// printer
#if defined(USE_SHARED_DLL) || defined(USE_PRINTER)
	int printer_device_type;
	_TCHAR printer_dll_path[_MAX_PATH];
#endif

	// General
#ifdef _USE_QT
	bool log_to_syslog;
	bool log_to_console;
	bool dev_log_to_syslog[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][8];
	bool dev_log_to_console[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][8];
	bool dev_log_recording[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][8];

	bool roma_kana_conversion;
	int rendering_type;
#endif
} config_t;

extern DLL_PREFIX config_t config;

#if defined(_USE_QT)
# include <string>
#endif

#endif

