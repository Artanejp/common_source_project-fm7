#include "common.h"
#include "menu_flags.h"

USING_FLAGS::USING_FLAGS(config_t *cfg)
{
	p_osd = NULL;
	use_alt_f10_key = false;
	use_auto_key = use_auto_key_us = use_auto_key_caps = false;
	use_auto_key_no_caps = use_auto_key_release =
	use_auto_key_shift = use_binary_file = false;
	use_roma_kana_conversion = false;
	
	max_binary = 0;
	use_bitmap = false;
	use_boot_mode = 0;

	use_bubble = false;
	max_bubble =  max_b77_banks = 0;

	use_cart = false;
	max_cart = 0;

	use_cpu_type = 0;

	use_compact_disc = use_crt_filter = use_debugger = false;
	use_device_type = 0;
	use_dipswitch = false;

	use_drive_type = 0;

	use_fd = false;
	max_drive = max_d88_banks = 0;

	max_draw_ranges = 0;
	
	use_joystick = use_joy_button_captions = false;
	num_joy_button_captions = 0;

	use_laser_disc = false;
	use_led_device = 0;

	max_memcard = 0;
	use_minimum_rendering = use_dig_resolution = false;
	use_monitor_type = 0;
	use_mouse = false;
	use_movie_player = false;
	use_notify_power_off = false;

	use_one_board_computer = false;
	use_printer = false;
	use_printer_type = 0;

	use_qd = false;
	max_qd = 0;
	max_tape = 0;
	use_scanline = use_screen_rotate = false;
	use_shift_numpad_key = false;
	screen_mode_num = 1;


	use_sound_device_type = 0;
	use_sound_volume = 0;
	without_sound = false;
	use_sound_files_fdd = false;
	use_sound_files_relay = false;
	
	use_special_reset = false;

	use_state = false;

	use_tape = use_tape_baud = use_tape_button = use_tape_ptr = false;
	use_vm_auto_key_table = false;
	support_tv_render = false;
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 400
	real_screen_width  = SCREEN_WIDTH;
	real_screen_height = SCREEN_HEIGHT;
	
	screen_width = SCREEN_WIDTH;
	screen_height = SCREEN_HEIGHT;
	
	screen_width_aspect = WINDOW_WIDTH_ASPECT;
	screen_height_aspect = WINDOW_HEIGHT_ASPECT;
	max_button = 0;
	vm_buttons_d = NULL;
	max_ranges = 0;
	vm_ranges_d = NULL;
	
	use_vertical_pixel_lines = false;
	notify_key_down_lr_shift = false;
	tape_binary_only = false;
	device_name = QString::fromUtf8("");
	config_name = QString::fromUtf8("");
	
	machine_pasopia_variants = false;
	p_config = cfg;
}

USING_FLAGS::~USING_FLAGS()
{
}

const _TCHAR *USING_FLAGS::get_joy_button_captions(int num)
{
	return "";
}

const _TCHAR *USING_FLAGS::get_sound_device_caption(int num)
{
	return "";
}


void USING_FLAGS::set_osd(OSD *p)
{
	p_osd = p;
}

OSD *USING_FLAGS::get_osd(void)
{
	return p_osd;
}

config_t *USING_FLAGS::get_config_ptr(void)
{
	return p_config;
}

int USING_FLAGS::get_s_freq_table(int num)
{
	return 48000;
}
																		
int USING_FLAGS::get_vm_node_size(void)
{
	return 0;
}

void USING_FLAGS::set_vm_node_name(int id, const _TCHAR *name)
{
}

_TCHAR *USING_FLAGS::get_vm_node_name(int id)
{
	return (_TCHAR *)"NODE";
}

void USING_FLAGS::set_emu(EMU *p)
{
	p_emu = p;
}

EMU *USING_FLAGS::get_emu(void)
{
	return p_emu;
}
