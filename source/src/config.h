/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ config ]
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "common.h"
#include "vm/vm_template.h"
#include "fileio.h"
#if defined(_USE_QT)
#define USE_FIXED_CONFIG 1
#include "qt/gui/csp_logger.h"
#else
  #if defined(USE_SHARED_DLL)
  #define USE_FIXED_CONFIG 1
  #endif
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

/*
 * ToDo: Apply keyboard types for emulation.
 */
enum {
	CONFIG_HOST_KEYBOARD_TYPE_AT_JP   = 0x00000000,
	CONFIG_HOST_KEYBOARD_TYPE_AT_EN   = 0x00010000,
	CONFIG_HOST_KEYBOARD_TYPE_AT_83   = 0x00030000,
	CONFIG_HOST_KEYBOARD_TYPE_AT_84   = 0x00040000,
	CONFIG_HOST_KEYBOARD_TYPE_AT_MISC = 0x00ff0000,
	CONFIG_HOST_KEYBOARD_TYPE_PC98    = 0x00980000,
	CONFIG_HOST_KEYBOARD_TYPE_MAC     = 0x00fe0000,
};	

enum {
	CONFIG_HOST_KEYBOARD_AT_106JP    = CONFIG_HOST_KEYBOARD_TYPE_AT_JP + 106,
	CONFIG_HOST_KEYBOARD_AT_101      = CONFIG_HOST_KEYBOARD_TYPE_AT_EN + 101,
	CONFIG_HOST_KEYBOARD_AT_102      = CONFIG_HOST_KEYBOARD_TYPE_AT_EN + 102,
	CONFIG_HOST_KEYBOARD_AT_104      = CONFIG_HOST_KEYBOARD_TYPE_AT_EN + 104,
	CONFIG_HOST_KEYBOARD_AT_109JP    = CONFIG_HOST_KEYBOARD_TYPE_AT_JP + 109,
	CONFIG_HOST_KEYBOARD_AT_83       = CONFIG_HOST_KEYBOARD_TYPE_AT_83 + 83,
	CONFIG_HOST_KEYBOARD_AT_84       = CONFIG_HOST_KEYBOARD_TYPE_AT_84 + 84,
	CONFIG_HOST_KEYBOARD_AT_MISC     = CONFIG_HOST_KEYBOARD_TYPE_AT_MISC,
	CONFIG_HOST_KEYBOARD_MAC_US      = CONFIG_HOST_KEYBOARD_TYPE_MAC + 0,
	CONFIG_HOST_KEYBOARD_MAC_JP      = CONFIG_HOST_KEYBOARD_TYPE_MAC + 1,
	CONFIG_HOST_KEYBOARD_MAC_ANOTHER = CONFIG_HOST_KEYBOARD_TYPE_MAC + 0xff,
};	

enum {
	CONFIG_CURSOR_AS_CURSOR = 0,
	CONFIG_CURSOR_AS_2468 = 1,
	CONFIG_CURSOR_AS_1235 = 2,
};

#define MAX_HISTORY	8

#ifdef USE_FIXED_CONFIG
	#define USE_CART_TMP		8
	#define USE_FLOPPY_DISK_TMP	16
	#define USE_QUICK_DISK_TMP	8
	#define USE_HARD_DISK_TMP	8
	#define USE_TAPE_TMP		8
	#define USE_COMPACT_DISC_TMP	8
	#define USE_LASER_DISC_TMP	8
	#define USE_BINARY_FILE_TMP	8
	#define USE_BUBBLE_TMP		16
	#define MAX_VOLUME_TMP		32
#else
	#define USE_CART_TMP		USE_CART
	#define USE_FLOPPY_DISK_TMP	USE_FLOPPY_DISK
	#define USE_QUICK_DISK_TMP	USE_QUICK_DISK
	#define USE_HARD_DISK_TMP	USE_HARD_DISK
	#define USE_TAPE_TMP		USE_TAPE
	#define USE_COMPACT_DISC_TMP	USE_COMPACT_DISC
	#define USE_LASER_DISC_TMP	USE_LASER_DISC
	#define USE_BINARY_FILE_TMP	USE_BINARY_FILE
	#define USE_BUBBLE_TMP		USE_BUBBLE
	#ifdef USE_SOUND_VOLUME
		#define MAX_VOLUME_TMP	USE_SOUND_VOLUME
	#endif
#endif

void initialize_config();
void load_config(const _TCHAR* config_path);
void save_config(const _TCHAR* config_path);
bool process_config_state(void *f, bool loading);

/*
 * 20160407 Ohta:
 * Qt:
 *  To reduce time to build, compiling common blocks of GUI at once.
 *  So, you should not separate items with #ifdef.
 */ 
typedef struct {
	// control
	#if defined(USE_FIXED_CONFIG) || defined(USE_BOOT_MODE)
		int boot_mode;
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_CPU_TYPE)
		int cpu_type;
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_DIPSWITCH)
		uint32_t dipswitch;
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_DEVICE_TYPE)
		int device_type;
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_DRIVE_TYPE)
		int drive_type;
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_KEYBOARD_TYPE)
		int keyboard_type;
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_MOUSE_TYPE)
		int mouse_type; /*!< Emulated type of mouse by VM */ 
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_JOYSTICK_TYPE)
		int joystick_type; /*!< Emulated type of joystick by VM */ 
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_SOUND_TYPE)
		int sound_type;
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_MONITOR_TYPE)
		int monitor_type;
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_SCANLINE)
		bool scan_line;
		bool scan_line_auto;
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_PRINTER_TYPE)
		int printer_type;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_FLOPPY_DISK)
		bool correct_disk_timing[/*USE_FLOPPY_DISK_TMP*/16];
		bool ignore_disk_crc[/*USE_FLOPPY_DISK_TMP*/16];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_TAPE)
		bool wave_shaper[USE_TAPE_TMP];
		bool direct_load_mzt[USE_TAPE_TMP];
		bool baud_high[USE_TAPE_TMP];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_VARIABLE_MEMORY)
	uint32_t current_ram_size;
	#endif
	bool compress_state;
	int cpu_power;
	bool full_speed;
	
	// recent files
	#if defined(USE_SHARED_DLL) || defined(USE_CART)
		_TCHAR initial_cart_dir[_MAX_PATH];
		_TCHAR recent_cart_path[USE_CART_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_FLOPPY_DISK)
		_TCHAR initial_floppy_disk_dir[_MAX_PATH];
		_TCHAR recent_floppy_disk_path[USE_FLOPPY_DISK_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_QUICK_DISK)
		_TCHAR initial_quick_disk_dir[_MAX_PATH];
		_TCHAR recent_quick_disk_path[USE_QUICK_DISK_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_HARD_DISK)
		_TCHAR initial_hard_disk_dir[_MAX_PATH];
		_TCHAR recent_hard_disk_path[USE_HARD_DISK_TMP][MAX_HISTORY][_MAX_PATH];
		_TCHAR last_hard_disk_path[USE_HARD_DISK_TMP][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_TAPE)
		_TCHAR initial_tape_dir[_MAX_PATH];
		_TCHAR recent_tape_path[USE_TAPE_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_COMPACT_DISC)
		_TCHAR initial_compact_disc_dir[_MAX_PATH];
		_TCHAR recent_compact_disc_path[USE_COMPACT_DISC_TMP][MAX_HISTORY][_MAX_PATH];
		bool swap_audio_byteorder[USE_COMPACT_DISC_TMP];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_LASER_DISC)
		_TCHAR initial_laser_disc_dir[_MAX_PATH];
		_TCHAR recent_laser_disc_path[USE_LASER_DISC_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_BINARY_FILE)
		_TCHAR initial_binary_dir[_MAX_PATH];
		_TCHAR recent_binary_path[USE_BINARY_FILE_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_BUBBLE)
		_TCHAR initial_bubble_casette_dir[_MAX_PATH];
		_TCHAR recent_bubble_casette_path[USE_BUBBLE_TMP][MAX_HISTORY][_MAX_PATH];
	#endif

	// screen
	int window_mode;
	int window_stretch_type;
	int fullscreen_stretch_type;
	//	#if defined(USE_FIXED_CONFIG) || defined(USE_SCREEN_ROTATE)
		int rotate_type;
	//	#endif
	
	// filter
	#if defined(USE_FIXED_CONFIG) || defined(USE_SCREEN_FILTER)
		int filter_type;
	#endif

	uint32_t mouse_sensitivity; /*!< SENSITIVITY of MOUSE , Value * 2^12, limit is 2^16-1 */
	// NOTE: Belows contain STAGED CONFIGURATION.
#if defined(_USE_QT)
	bool use_separate_thread_draw;
	bool use_opengl_scanline;
	bool use_osd_virtual_media;
	bool opengl_scanline_vert;
	bool opengl_scanline_horiz;
	bool use_opengl_filters;
	bool focus_with_click;
	int opengl_filter_num;
	
	// STAGED CONFIG VALUES
	bool swap_kanji_pause;
	int  cursor_as_ten_key;
	bool numpad_enter_as_fullkey;
	int host_keyboard_type;
	
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
	bool disk_count_immediate[16]; // Hack for MB8877, FLEX for FM-7.
	_TCHAR debugwindow_font[1024];
	_TCHAR logwindow_font[1024];
	int debugwindow_width;
	int debugwindow_height;
	int logwindow_width;
	int logwindow_height;
	
#endif	
	
	// sound
	int sound_frequency;
	int sound_latency;
	bool sound_strict_rendering;
	int sound_device_num;
	
#if defined(_USE_QT)	
	int general_sound_level;
#endif
#if defined(USE_FIXED_CONFIG) || defined(USE_FLOPPY_DISK)
	bool sound_noise_fdd;
#endif
#if defined(USE_FIXED_CONFIG) || defined(USE_TAPE)
	bool sound_noise_cmt;
	bool sound_play_tape;
#endif
#if defined(USE_FIXED_CONFIG) || defined(USE_SOUND_VOLUME)
	int sound_volume_l[MAX_VOLUME_TMP];
	int sound_volume_r[MAX_VOLUME_TMP];
#endif
 	#if defined(USE_SHARED_DLL) || defined(_WIN32) && !defined(_USE_QT)
		_TCHAR mame2151_dll_path[_MAX_PATH];
		_TCHAR mame2608_dll_path[_MAX_PATH];
 	#endif
	// input
	#if defined(USE_FIXED_CONFIG) || defined(USE_JOYSTICK)
		int joy_buttons[4][16];
		bool use_joy_to_key;
		int joy_to_key_type; // Note: ADD "1235" as type 3.20181218 K.O
		bool joy_to_key_numpad5;
		int joy_to_key_buttons[16];
		_TCHAR assigned_joystick_name[16][256];
		int assigned_joystick_num[16];
		bool emulated_joystick_dpad[4];
	#endif
	#if defined(USE_FIXED_CONFIG) || defined(USE_AUTO_KEY)
		bool romaji_to_kana;
	#endif

	// printer
#if defined(USE_FIXED_CONFIG) || defined(USE_PRINTER)
	_TCHAR printer_dll_path[_MAX_PATH];
#endif
	// debug 
	bool special_debug_fdc;	
	bool print_statistics;

#if defined(_WIN32) && !defined(_USE_QT)
	bool use_direct_input;
	bool disable_dwm;

	bool use_d2d1;
	bool use_d3d9;
	bool wait_vsync;
	bool use_dinput;
	bool show_status_bar;
#endif

#ifdef _USE_QT

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
	bool log_to_syslog;
	bool log_to_console;
	bool dev_log_to_syslog[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][8];
	bool dev_log_to_console[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][8];
	bool dev_log_recording[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][8];

	bool state_log_to_console;
	bool state_log_to_syslog;
	bool state_log_to_recording;
	
	int rendering_type;

	int virtual_media_position; // -1 = none, 1, 2, 3, 4 = LRUD
#endif
	
} config_t;

extern config_t DLL_PREFIX_I config;

#if defined(_USE_QT)
# include <string>
#endif

#endif

