
#include "common.h"
#include "vm.h"
typedef struct {
	const _TCHAR* caption;
	int x, y;
	int width, height;
	int font_size;
	int code;
} button_desc_t;

class USING_FLAGS {
private:
	// USE_* flags
	bool use_access_lamp;
	bool use_alt_f10_key;
	
	bool use_auto_key;
	bool use_auto_key_us;
	bool use_auto_key_caps;
	bool use_auto_key_no_caps;
	bool use_auto_key_release;
	bool use_auto_key_shift;

	bool use_binary_file;
	int max_binary;
	bool use_bitmap;
	int use_boot_mode;

	bool use_bubble;
	int max_bubble;
	int max_b77_banks;

	bool use_cart;
	int max_cart;

	int use_cpu_type;

	bool use_compact_disc;

	bool use_crt_filter;

	bool use_debugger;

	int use_device_type;
	bool use_dipswitch;

	int use_drive_type;

	bool use_fd;
	int max_drive;
	int max_d88_banks;

	int max_draw_ranges;
	
	bool use_joystick;
	bool use_joy_button_captions;

	bool use_laser_disc;
	int use_led_device;

	int max_memcard;
	
	bool use_minimum_rendaring;
	bool use_dig_resolution;

	int use_monitor_type;
	
	bool use_mouse;
	
	bool use_movie_player;

	bool use_notify_power_off;

	bool use_one_board_computer;
	bool use_printer;
	int use_printer_type;

	bool use_qd;
	int max_qd;

	bool use_scanline;
	bool use_screen_rotate;
	bool use_shift_numpad_key;

	int max_scsi;

	int use_sound_device_type;
	int use_sound_volume;
	bool without_sound;

	bool use_special_reset;

	bool use_state;

	bool use_tape;
	bool use_tape_baud;
	bool use_tape_button;
	bool use_tape_ptr;

	bool use_vm_auto_key_table;

	button_desc_t vm_buttons_d[];
public:
	USING_FLAGS();
	~USING_FLAGS();
	
    bool is_use_access_lamp() { return use_access_lamp; }
	bool is_use_alt_f10_key() { return use_alt_f10_key; }
	
	bool is_use_auto_key() { return use_auto_key; }
	bool is_use_auto_key_us() { return use_auto_key_us; }
	bool is_use_auto_key_caps() { return use_auto_key_caps; }
	bool is_use_auto_key_no_caps() { return use_auto_key_no_caps; }
	bool is_use_auto_key_release() { return use_auto_key_release; }
	bool is_use_auto_key_shift() { return use_auto_key_shift; }

	bool is_use_binary_file() { return use_binary_file; }
	int get_max_binary() { return max_binary; }
	bool is_use_bitmap() { return use_bitmap; }

	int get_use_boot_mode() { return use_boot_mode; }

	bool is_use_bubble() { return use_bubble; }
	int get_max_bubble() { return max_bubble; }
	int get_max_b77_banks() { return max_b77_banks; }

	bool is_use_cart() { return use_cart; }
	int get_max_cart() { return max_cart; }

	int get_use_cpu_type() { return use_cpu_type; }

	bool is_use_compact_disc() { return use_compact_disc; }

	bool is_use_crt_filter() { return use_crt_filter; }

	bool is_use_debugger() { return use_debugger; }

	int get_use_device_type() { return use_device_type; }
	bool is_use_dipswitch() { return use_dipswitch; }

	int get_use_drive_type() { return use_drive_type; }

	bool is_use_fd() { return use_fd; }
	int get_max_drive() { return max_drive; }
	int get_max_d88_banks { return max_d88_banks; }

	int get_max_draw_ranges() { return max_draw_ranges; }
	
	bool is_use_joystick() { return use_joystick; }
	bool is_use_joy_button_captions{} { return use_joy_button_caption };

	bool is_use_laser_disc() {return use_laser_disc; }
	int get_use_led_device() { return use_led_device; }

	int get_max_memcard() { return max_memcard; }
	
	bool is_use_minimum_rendaring() { return use_minimum_rendering; }
	bool is_use_dig_resolution() { return use_dig_resolution; }

	int get_use_monitor_type() { use_monitor_type; }
	
	bool is_use_mouse() { return use_mouse; }
	
	bool is_use_movie_player() { return use_movie_player; }

	bool is_use_notify_power_off() { return use_notify_power_off; }

	bool is_use_one_board_computer() { return use_one_board_computer; }
	bool is_use_printer() { return use_printer; }
	int get_use_printer_type() { return use_printer_type; }

	bool is_use_qd() { return use_qd; }
	int get_max_qd() { return max_qd; }

	bool is_use_scanline() { return use_scanline; }
	bool is_use_screen_rotate() { return use_screen_rotate; }
	bool is_use_shift_numpad_key() { return use_shift_numpad_key; }

	int get_max_scsi() { return max_scsi; }

	int get_use_sound_device_type() { return use_sound_device_type; }
	int get_use_sound_volume() { return use_sound_volume; }
	bool is_without_sound() { return without_sound; }

	bool is_use_special_reset() { return use_special_reset; }

	bool is_use_state() { return use_state; }

	bool is_use_tape() { return use_tape; }
	bool is_use_tape_baud() { return use_tape_baud; }
	bool is_use_tape_button() { return use_tape_button; }
	bool is_use_tape_ptr() { return use_tape_ptr; }

	bool is_use_vm_auto_key_table() { return use_vm_auto_key_table; }
};
	
#ifndef USE_SOUND_VOLUME
static const _TCHAR sound_device_caption[] = {""};
#endif
#ifndef USE_JOY_BUTTON_CAPTIONS
static const _TCHAR *joy_button_captions[] = {""};
#endif
