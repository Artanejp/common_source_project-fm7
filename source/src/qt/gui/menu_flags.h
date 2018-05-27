
#ifndef __CSP_QT_COMMON_MENU_FLAGS_H
#define __CSP_QT_COMMON_MENU_FLAGS_H

#include <QString>
#include "common.h"
#include "config.h"

#ifndef _SCREEN_MODE_NUM
#define _SCREEN_MODE_NUM 32
#endif

class EMU;
class OSD;


typedef struct {
	int x, y;
	int width, height;
	int code;
} button_desc_t;

typedef struct {
	int x, y;
	int width, height;
} vm_ranges_t;

class DLL_PREFIX USING_FLAGS {
protected:
	QString config_name;
	QString device_name;
	// USE_* flags
	bool use_alt_f10_key;
	
	bool use_auto_key;
	bool use_auto_key_us;
	bool use_auto_key_caps;
	bool use_auto_key_no_caps;
	bool use_auto_key_release;
	bool use_auto_key_shift;

	bool use_binary_file;
	int max_binary;
	int base_binary_num;
	
	bool use_bitmap;
	int use_boot_mode;

	bool use_bubble;
	int max_bubble;
	int base_bubble_num;
	int max_b77_banks;

	bool use_cart;
	int max_cart;

	int use_cpu_type;

	bool use_compact_disc;
	int max_comnpact_disc;
	int base_cd_num;

	bool use_debugger;

	int use_device_type;
	int use_mouse_type;
	bool use_dipswitch;

	int use_drive_type;

	bool use_fd;
	int base_fd_num;
	int max_drive;
	int max_d88_banks;

	int max_draw_ranges;
	
	bool use_joystick;
	int use_joystick_type;
	bool use_joy_button_captions;
	int num_joy_button_captions;

	int use_keyboard_type;

	bool use_hd;
	int max_hd;
	int base_hd_num;
	
	bool use_laser_disc;
	int max_laser_disc;
	int base_ld_num;


	bool use_key_locked;
	bool independent_caps_kana_led;

	int use_extra_leds;

	int max_memcard;
	
	bool use_minimum_rendering;
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
	int base_qd_num;

	bool use_scanline;
	bool use_screen_rotate;
	bool use_shift_numpad_key;

	int max_scsi;

	int use_sound_device_type;
	int use_sound_volume;
	bool without_sound;
	bool use_sound_files_fdd;
	bool use_sound_files_relay;
	
	bool use_special_reset;

	bool use_state;

	bool use_tape;
	bool use_tape_baud;
	bool use_tape_button;
	bool use_tape_ptr;
	int max_tape;
	int base_tape_num;
	
	bool use_vm_auto_key_table;

	int max_button;
	int max_ranges;
	bool use_vertical_pixel_lines;

	int screen_width;
	int screen_height;
	
	int real_screen_width;
	int real_screen_height;

	float screen_x_zoom;
	float screen_y_zoom;
	
	int screen_width_aspect;
	int screen_height_aspect;
	bool notify_key_down_lr_shift;

	bool tape_binary_only;
	int screen_mode_num;

	bool support_tv_render;
	
	bool machine_basicmaster_variants;
	bool machine_pasopia_variants;
	bool machine_tk80_series;
	bool machine_cmt_mz_series;
	bool machine_pc6001;
	bool machine_pc8001_variants;
	bool machine_mz80a_variants;
	bool machine_mz80b_variants;
	bool machine_x1_series;
	bool machine_fm7_series;
	bool machine_gamegear;
	bool machine_mastersystem;
	bool machine_has_pcengine;
	bool machine_sc3000;
	bool machine_z80tvgame;
	
	button_desc_t *vm_buttons_d;
	vm_ranges_t *vm_ranges_d;
	EMU *p_emu;
	OSD *p_osd;
	config_t *p_config;
public:
	USING_FLAGS(config_t *cfg);
	~USING_FLAGS();
	QString get_config_name() { return config_name; }
	QString get_device_name() { return device_name; }
	
	bool is_use_alt_f10_key() { return use_alt_f10_key; }
	bool is_use_auto_key() { return use_auto_key; }
	bool is_use_auto_key_us() { return use_auto_key_us; }
	bool is_use_auto_key_caps() { return use_auto_key_caps; }
	bool is_use_auto_key_no_caps() { return use_auto_key_no_caps; }
	bool is_use_auto_key_release() { return use_auto_key_release; }
	bool is_use_auto_key_shift() { return use_auto_key_shift; }

	bool is_use_binary_file() { return use_binary_file; }
	int get_max_binary() { return max_binary; }
	int get_base_binary_num() { return base_binary_num; }
	
	bool is_use_bitmap() { return use_bitmap; }

	int get_use_boot_mode() { return use_boot_mode; }

	bool is_use_bubble() { return use_bubble; }
	int get_max_bubble() { return max_bubble; }
	int get_max_b77_banks() { return max_b77_banks; }
	int get_base_bubble_num() { return base_bubble_num; }

	bool is_use_cart() { return use_cart; }
	int get_max_cart() { return max_cart; }
	int get_base_cart_num() { return base_cart_num; }

	int get_use_cpu_type() { return use_cpu_type; }

	bool is_use_compact_disc() { return use_compact_disc; }
	int get_max_cd() { return max_compact_disc; }
	int get_base_compact_disc_num() { return base_cd_num; }

	bool is_use_debugger() { return use_debugger; }

	int get_use_device_type() { return use_device_type; }
	int get_use_mouse_type() { return use_mouse_type; }
	bool is_use_dipswitch() { return use_dipswitch; }

	int get_use_drive_type() { return use_drive_type; }

	bool is_use_fd() { return use_fd; }
	int get_max_drive() { return max_drive; }
	int get_max_d88_banks() { return max_d88_banks; }
	int get_base_floppy_disk_num() { return base_fd_num; }

	bool is_use_joystick() { return use_joystick; }
	bool is_use_joy_button_captions() { return use_joy_button_captions; }
	int  get_num_joy_button_captions() { return num_joy_button_captions; }
	int  get_use_joystick_type() { return use_joystick_type; }

	int  get_use_keyboard_type() { return use_keyboard_type; }
	
	bool is_use_hdd() { return use_hd; }
	int get_max_hdd() { return max_hd; }
	int get_base_hdd_num() { return base_hd_num; }
	
	bool is_use_laser_disc() {return use_laser_disc; }
	int get_max_ld() { return max_laser_disc; }
	int get_base_laser_disc_num() { return base_ld_num; }

	bool get_use_key_locked() { return use_key_locked; }
	int get_use_extra_leds() { return use_extra_leds; }
	bool get_independent_caps_kana_led() { return independent_caps_kana_led; }

	int get_max_memcard() { return max_memcard; }
	
	bool is_use_minimum_rendaring() { return use_minimum_rendering; }
	bool is_use_dig_resolution() { return use_dig_resolution; }

	int get_use_monitor_type() { return use_monitor_type; }
	
	bool is_use_mouse() { return use_mouse; }
	
	bool is_use_movie_player() { return use_movie_player; }

	bool is_use_notify_power_off() { return use_notify_power_off; }

	bool is_use_one_board_computer() { return use_one_board_computer; }
	bool is_use_printer() { return use_printer; }
	int get_use_printer_type() { return use_printer_type; }

	bool is_use_qd() { return use_qd; }
	int get_max_qd() { return max_qd; }
	int get_base_quick_disk_num() { return base_qd_num; }

	bool is_use_scanline() { return use_scanline; }
	bool is_use_screen_rotate() { return use_screen_rotate; }
	bool is_use_shift_numpad_key() { return use_shift_numpad_key; }

	int get_max_scsi() { return max_scsi; }

	int get_use_sound_device_type() { return use_sound_device_type; }
	int get_use_sound_volume() { return use_sound_volume; }
	bool is_without_sound() { return without_sound; }
	bool is_use_sound_files_fdd() { return use_sound_files_fdd; }
	bool is_use_sound_files_relay() { return use_sound_files_relay; }
	bool is_use_special_reset() { return use_special_reset; }

	bool is_use_state() { return use_state; }

	bool is_use_tape() { return use_tape; }
	bool is_use_tape_baud() { return use_tape_baud; }
	bool is_use_tape_button() { return use_tape_button; }
	bool is_use_tape_ptr() { return use_tape_ptr; }
	int get_max_tape() { return max_tape; }
	int get_base_tape_num() { return base_tape_num; }

	bool is_use_vm_auto_key_table() { return use_vm_auto_key_table; }

	bool is_use_vertical_pixel_lines() { return use_vertical_pixel_lines; }
	bool is_support_tv_render() { return support_tv_render; }

	int get_screen_width() { return screen_width; }
	int get_screen_height() { return screen_height; }
	int get_screen_width_aspect() { return screen_width_aspect; }
	int get_screen_height_aspect() { return screen_height_aspect; }
	int get_real_screen_width() { return real_screen_width; }
	int get_real_screen_height() { return real_screen_height; }
	float get_screen_x_zoom() { return screen_x_zoom; }
	float get_screen_y_zoom() { return screen_y_zoom; }
	
	int get_screen_mode_num() { return screen_mode_num; }
	int get_max_button() { return max_button; }
	int get_max_draw_ranges() { return max_ranges; }
	button_desc_t *get_vm_buttons() { return vm_buttons_d; }
	vm_ranges_t *get_draw_ranges() { return vm_ranges_d; }

	bool is_notify_key_down_lr_shift() { return notify_key_down_lr_shift; }
	bool is_tape_binary_only() { return tape_binary_only; }

	bool is_machine_basicmaster_variants() { return machine_basicmaster_variants; }
	bool is_machine_pasopia_variants() { return machine_pasopia_variants; }
	bool is_machine_tk80_series() { return machine_tk80_series; }
	bool is_machine_cmt_mz_series() { return machine_cmt_mz_series; }
	bool is_machine_pc6001() { return machine_pc6001; }
	bool is_machine_pc8001_variants() { return machine_pc8001_variants; }
	bool is_machine_mz80a_variants() { return machine_mz80a_variants; }
	bool is_machine_mz80b_variants() { return machine_mz80b_variants; }
	bool is_machine_x1_series() { return machine_x1_series; }
	bool is_machine_fm7_series() { return machine_fm7_series; }
	bool is_machine_gamegear() { return machine_gamegear; }
	bool is_machine_mastersystem() { return machine_mastersystem; }
	bool is_machine_has_pcengine() { return machine_has_pcengine; }
	bool is_machine_sc3000() { return machine_sc3000; }
	bool is_machine_z80tvgame() { return machine_z80tvgame; }
	virtual const _TCHAR *get_joy_button_captions(int num);
	virtual const _TCHAR *get_sound_device_caption(int num);
	virtual int get_s_freq_table(int num);
	void set_emu(EMU *p);
	EMU *get_emu(void);
	void set_osd(OSD *p);
	OSD *get_osd(void);
	
	virtual int get_vm_node_size();
	virtual void set_vm_node_name(int id, const _TCHAR *name);
	virtual _TCHAR *get_vm_node_name(int id);
	
	config_t *get_config_ptr(void);
};
	

#endif //#ifndef __CSP_QT_COMMON_MENU_FLAGS_H
