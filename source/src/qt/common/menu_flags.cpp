#include "common.h"
#include "vm.h"
#include "emu.h"
#include "osd.h"
#include "menu_flags_ext.h"

#ifndef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {""};
#endif
#ifndef USE_JOY_BUTTON_CAPTIONS
static const _TCHAR *joy_button_captions[] = {""};
#endif


USING_FLAGS_EXT::USING_FLAGS_EXT(config_t *cfg, QSettings* settings) : USING_FLAGS(cfg, settings)
{
	#if defined(USE_FLOPPY_TYPE_BIT)
	floppy_type_bit = USE_FLOPPY_TYPE_BIT;
	#endif
	#if defined(OVERRIDE_SOUND_FREQ_48000HZ)
	override_sound_frequency_48000hz = OVERRIDE_SOUND_FREQ_48000HZ;
	#endif
	#if defined(USE_VARIABLE_MEMORY)
	use_ram_size = true;
	max_ram_size = USE_VARIABLE_MEMORY;
		#if defined(MIN_RAM_SIZE)
		min_ram_size = MIN_RAM_SIZE;
		#endif
		#if defined(RAM_SIZE_ORDER)
		ram_size_order = RAM_SIZE_ORDER;
		#endif
	#endif

	real_screen_width  = SCREEN_WIDTH;
	real_screen_height = SCREEN_HEIGHT;
#if defined(USE_CUSTOM_SCREEN_ZOOM_FACTOR)
	custom_screen_zoom_factor = USE_CUSTOM_SCREEN_ZOOM_FACTOR;
#endif
#if defined(SCREEN_FAKE_WIDTH)
	screen_width = SCREEN_FAKE_WIDTH;
#else
	screen_width = SCREEN_WIDTH;
#endif
#if defined(SCREEN_FAKE_HEIGHT)
	screen_height = SCREEN_FAKE_HEIGHT;
#else
	screen_height = SCREEN_HEIGHT;
#endif
#if defined(SCREEN_X_ZOOM)
	screen_x_zoom = SCREEN_X_ZOOM;
#endif
#if defined(SCREEN_Y_ZOOM)
	screen_y_zoom = SCREEN_Y_ZOOM;
#endif
	screen_width_aspect = WINDOW_WIDTH_ASPECT;
	screen_height_aspect = WINDOW_HEIGHT_ASPECT;

#if defined(DEVICE_NAME)
	device_name = QString::fromUtf8(DEVICE_NAME);
#else
	device_name = QString::fromUtf8("");
#endif
#if defined(CONFIG_NAME)
	config_name = QString::fromUtf8(CONFIG_NAME);
#else
	config_name = QString::fromUtf8("");
#endif

	// Machine definitions
	machine_pasopia_variants = false;
#if defined(_PASOPIA7) || defined(_PASOPIA)
	machine_pasopia_variants = true;
#endif
	machine_tk80_series = false;
#if defined(_TK80BS) || defined(_TK80)
	machine_tk80_series = true;
#endif
	machine_cmt_mz_series = false;
#if defined(_MZ80A) || defined(_MZ80K)  || \
	defined(_MZ1200) || defined(_MZ700) || \
	defined(_MZ800) || defined(_MZ1500) || \
	defined(_MZ80B) || defined(_MZ2000) || \
	defined(_MZ2200) || defined(_MZ2500)
	machine_cmt_mz_series = true;
#endif

#if defined(_PC6001) || defined(_PC6001MK2) || \
	defined(_PC6001MK2SR) || \
	defined(_PC6601) || defined(_PC6601SR)
	machine_pc6001_variants = true;
#endif
#if defined(PC8001_VARIANT)
	machine_pc8001_variants = true;
#endif
#if defined(PC8801_VARIANT)
	machine_pc8801_variants = true;
	#if defined(PC8801SR_VARIANT)
	machine_pc8801sr_variants = true;
	#endif
#endif

#if defined(_MZ80A) || defined(_MZ80K)  || \
	defined(_MZ1200) || defined(_MZ700) || \
	defined(_MZ800) || defined(_MZ1500)
	machine_mz80a_variants = true;
#endif
#if	defined(_MZ80B) || defined(_MZ2000) || \
	defined(_MZ2200) || defined(_MZ2500)
	machine_mz80b_variants = true;
#endif
#if defined(_MZ2500)
	machine_mz2500 = true;
#endif
#if defined(_X1) || defined(_X1TURBO) || \
	defined(_X1TURBOZ) || defined(_X1TWIN)
	machine_x1_series = true;
#endif
#if defined(_FM8) || defined(_FM7) || \
	defined(_FMNEW7) || defined(_FM77) || \
	defined(_FM77L2) || defined(_FM77L4) || \
	defined(_FM77AV) || \
	defined(_FM77AV20) || defined(_FM77AV20EX) || \
	defined(_FM77AV40) || defined(_FM77AV40EX) || \
	defined(_FM77AV40SX)
	machine_fm7_series = true;
#endif
#if defined(_GAMEGEAR)
	machine_gamegear = true;
#endif
#if defined(_MASTERSYSTEM)
	machine_mastersystem = true;
#endif
#if defined(_PCENGINE) || defined(_X1TWIN)
	machine_has_pcengine = true;
#endif
#if defined(_SC3000)
	machine_sc3000 = true;
#endif
#if defined(_Z80TVGAME)
	machine_z80tvgame = true;
#endif
#if defined(_BMJR)
	machine_basicmaster_variants = true;
#endif

#if defined(USE_ALT_F10_KEY)
	use_alt_f10_key = true;
#endif
#if defined(USE_AUTO_KEY)
	use_auto_key = true;
	#if defined(USE_AUTO_KEY_US)
		use_auto_key_us = true;
	#endif
	#if defined(USE_AUTO_KEY_CAPS)
		use_auto_key_caps = true;
	#endif
	#if defined(USE_AUTO_KEY_NO_CAPS)
		use_auto_key_no_caps = true;
	#endif
	#if defined(USE_AUTO_KEY_RELEASE)
		use_auto_key_release = true;
	#endif
	#if defined(USE_AUTO_KEY_SHIFT)
		use_auto_key_shift = true;
	#endif
#endif
#if defined(USE_CART)
	use_cart = true;
	max_cart = USE_CART;
	base_cart_num = BASE_CART_NUM;
#endif
#if defined(USE_FLOPPY_DISK)
	use_fd = true;
	max_drive = USE_FLOPPY_DISK;
	#if defined(MAX_D88_BANKS)
		max_d88_banks = MAX_D88_BANKS;
	#else
		max_d88_banks = 64;
	#endif
	base_fd_num = BASE_FLOPPY_DISK_NUM;
#endif
#if defined(USE_QUICK_DISK)
	use_qd = true;
	max_qd = USE_QUICK_DISK;
	base_qd_num = BASE_QUICK_DISK_NUM;
#endif
#if defined(USE_HARD_DISK)
	use_hd = true;
	max_hd = USE_HARD_DISK;
	base_hd_num = BASE_HARD_DISK_NUM;
#endif
#if defined(USE_TAPE)
	use_tape = true;
	#if defined(USE_TAPE_BAUD)
		use_tape_baud = true;
	#endif
	use_tape_button = true;
	#if defined(USE_TAPE_PTR)
		use_tape_ptr = true;
	#endif
	max_tape = USE_TAPE;
	base_tape_num = BASE_TAPE_NUM;
#endif
#if defined(USE_COMPACT_DISC)
	use_compact_disc = true;
	max_compact_disc = USE_COMPACT_DISC;
	base_cd_num = BASE_COMPACT_DISC_NUM;
#endif
#if defined(USE_LASER_DISC)
	use_laser_disc = true;
	max_laser_disc = USE_LASER_DISC;
	base_ld_num = BASE_LASER_DISC_NUM;
#endif
#if defined(USE_BINARY_FILE)
	use_binary_file = true;
	max_binary = USE_BINARY_FILE;
	base_binary_num = BASE_BINARY_FILE_NUM;
#endif
#if defined(USE_BUBBLE)
	use_bubble = true;
	max_bubble = USE_BUBBLE;
	base_bubble_num = BASE_BUBBLE_NUM;
	#if defined(MAX_B77_BANKS)
		max_b77_banks = MAX_B77_BANKS;
	#else
		max_b77_banks = 16;
	#endif
#endif
#if defined(USE_BITMAP)
	use_bitmap = true;
#endif
#if defined(USE_BOOT_MODE)
	use_boot_mode = USE_BOOT_MODE;
#endif
#if defined(USE_CPU_TYPE)
	use_cpu_type = USE_CPU_TYPE;
#endif
#if defined(SUPPORT_TV_RENDER)
	support_tv_render = true;
#endif
#if defined(USE_DEBUGGER)
	use_debugger = true;
#endif
#if defined(USE_DEVICE_TYPE)
	use_device_type = USE_DEVICE_TYPE;
#endif
#if defined(USE_MOUSE_TYPE)
	use_mouse_type = USE_MOUSE_TYPE;
#endif
#if defined(USE_JOYSTICK_TYPE)
	use_joystick_type = USE_JOYSTICK_TYPE;
#endif
#if defined(USE_KEYBOARD_TYPE)
	use_keyboard_type = USE_KEYBOARD_TYPE;
#endif
#if defined(USE_DIPSWITCH)
	use_dipswitch = true;
#endif
#if defined(USE_MACHINE_FEATURES)
	use_machine_features = USE_MACHINE_FEATURES;
#endif
#if defined(USE_DRIVE_TYPE)
	use_drive_type = USE_DRIVE_TYPE;
#endif
#if defined(MAX_DRAW_RANGES)
	max_draw_ranges = MAX_DRAW_RANGES;
#endif
#if defined(USE_JOYSTICK)
	use_joystick = true;
	#if defined(USE_JOY_BUTTON_CAPTIONS)
		use_joy_button_captions = true;
		num_joy_button_captions = sizeof(joy_button_captions) / sizeof(_TCHAR *);
	#endif
#endif
#if defined(USE_KEY_LOCKED)
	use_key_locked = true;
#endif
#if defined(USE_LED_DEVICE)
	use_led_devices = USE_LED_DEVICE;
#endif
#if defined(INDEPENDENT_CAPS_KANA_LED)
	independent_caps_kana_led = true;
#endif
#if defined(USE_MEMCARD)
	use_memcard = USE_MEMCARD;
#endif
#if defined(USE_MINIMUM_RENDERING)
	use_minimum_rendering = true;
#endif
#if defined(USE_DIG_RESOLUTION)
	use_dig_resolution = true;
#endif
#if defined(USE_MONITOR_TYPE)
	use_monitor_type = USE_MONITOR_TYPE;
#endif

#if defined(USE_SOCKET)
	use_socket = true;
#endif
#if defined(DONT_KEEEP_KEY_PRESSED)
	dont_keeep_key_pressed = true;
#endif
#if defined(USE_MOUSE)
	use_mouse = true;
#endif
#if defined(USE_MOVIE_PLAYER)
	use_movie_player = true;
#endif
#if defined(USE_VIDEO_CAPTURE)
	use_video_capture = false;	
#endif
#if defined(USE_NOTIFY_POWER_OFF)
	use_notify_power_off = true;
#endif
#if defined(ONE_BOARD_MICRO_COMPUTER)
	use_one_board_computer = true;
#endif
#if defined(USE_PRINTER)
	use_printer = true;
	#if defined(USE_PRINTER_TYPE)
		use_printer_type = USE_PRINTER_TYPE;
	#endif
#endif
#if defined(USE_SERIAL)
	use_serial = true;
	#if defined(USE_SERIAL_TYPE)
		use_serial_type = USE_SERIAL_TYPE;
	#endif
#endif
#if defined(USE_SCANLINE)
	use_scanline = true;
	#if defined(PC8801_VARIANT)
	use_scanline_auto = true;
	#endif
#endif
#if defined(USE_SCREEN_ROTATE)
	use_screen_rotate = true;
#endif
#if defined(MAX_SCSI)
	max_scsi = MAX_SCSI;
#endif
#if defined(USE_SOUND_TYPE)
	use_sound_device_type = USE_SOUND_TYPE;
#endif
#if defined(USE_SOUND_VOLUME)
	use_sound_volume = USE_SOUND_VOLUME;
#endif
#if defined(WITHOUT_SOUND)
	without_sound = true;
#endif
#if defined(USE_FLOPPY_DISK)
	use_sound_files_fdd = true;
#endif
#if defined(USE_TAPE)
	use_sound_files_relay = true;
#endif
#if defined(USE_SPECIAL_RESET)
	use_special_reset = true;
	special_reset_num = USE_SPECIAL_RESET;
#endif
#if defined(USE_VM_AUTO_KEY_TABLE)
	use_vm_auto_key_table = true;
#endif
#if defined(MAX_BUTTONS)
	max_button = sizeof(vm_buttons) / sizeof(button_desc_t);
	vm_buttons_d = (button_desc_t *)vm_buttons;
#endif
#if defined(MAX_DRAW_RANGES)
	max_ranges = sizeof(vm_ranges) / sizeof(vm_ranges_t);
	vm_ranges_d = (vm_ranges_t *)vm_ranges;
#endif

#if defined(USE_VERTICAL_PIXEL_LINES)
	use_vertical_pixel_lines = true;
#endif
#if defined(TAPE_BINARY_ONLY)
	tape_binary_only = true;
#endif
#if defined(_SCREEN_MODE_NUM)
	screen_mode_num = _SCREEN_MODE_NUM;
#endif
#if defined(USE_STATE)
	use_state = true;
#endif
	p_config = &config;
}

USING_FLAGS_EXT::~USING_FLAGS_EXT()
{
}

const _TCHAR *USING_FLAGS_EXT::get_joy_button_captions(int num)
{
#ifdef USE_JOY_BUTTON_CAPTIONS
	if((num < 0) || (num >= num_joy_button_captions)) {
		return "";
	} else  {
		return joy_button_captions[num];
	}
#else
	return "";
#endif
}

const _TCHAR *USING_FLAGS_EXT::get_sound_device_caption(int num)
{
#ifdef USE_SOUND_VOLUME
	if((num < 0) || (num >= USE_SOUND_VOLUME)) {
		return "";
	} else  {
		return sound_device_caption[num];
	}
#else
	return "";
#endif
}


