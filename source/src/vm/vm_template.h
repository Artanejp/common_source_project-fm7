#ifndef __CSP_VM_TEMPLATE_H
#define __CSP_VM_TEMPLATE_H

#include "common.h"

class EMU;
class EVENT;
class DEVICE;
class csp_state_utils;
class VM_TEMPLATE {
protected:
	EMU* emu;
	// devices
	EVENT* event;
	csp_state_utils *state_entry;
	
public:
	VM_TEMPLATE(EMU* parent_emu) : emu(parent_emu)
	{
		emu = parent_emu;
	}
	~VM_TEMPLATE() {}
	// drive virtual machine
	virtual void reset() { }
	virtual void special_reset() { }
	virtual void run() { }
	virtual void notify_power_off() { }
	
	virtual double get_frame_rate() { return 59.94; }
	virtual void get_screen_resolution(int *w, int *h) {
		if(w != NULL) *w = 0;
		if(h != NULL) *h = 0;
	}

	// debugger
	virtual DEVICE *get_cpu(int num) { return NULL; }
	virtual uint32_t get_cpu_pc() { return 0; }
	virtual void initialize(void) { }
	virtual void update_dipswitch(void) { }
	
	// draw screen
	virtual void draw_screen() { }

	// multimedia
	virtual void movie_sound_callback(uint8_t *buffer, long size) { }
	// sound generation
	virtual void initialize_sound(int rate, int samples) { }
	virtual uint16_t* create_sound(int* extra_frames) { return NULL; }
	virtual int get_sound_buffer_ptr() { return 0; }
	virtual void set_sound_device_volume(int ch, int decibel_l, int decibel_r) { }

	// network
	virtual void notify_socket_connected(int ch) { }
	virtual void notify_socket_disconnected(int ch) { }
	virtual uint8_t* get_socket_send_buffer(int ch, int* size)
	{
		if(size != NULL) *size = 0;
		return NULL;
	}
	virtual void inc_socket_send_buffer_ptr(int ch, int size) { }
	virtual uint8_t* get_socket_recv_buffer0(int ch, int* size0, int* size1) {
		if(size0 != NULL) *size0 = 0;
		if(size1 != NULL) *size1 = 0;
		return NULL;
	}
	virtual uint8_t* get_socket_recv_buffer1(int ch) { return NULL; }
	virtual void inc_socket_recv_buffer_ptr(int ch, int size) { }

	// notify key
	virtual void key_down(int code, bool repeat) { }
	virtual void key_up(int code) { }
	virtual bool get_caps_locked() { return false; }
	virtual bool get_kana_locked() { return false; }
	virtual uint32_t get_led_status() { return 0; }

	
	// user interface
	virtual void open_floppy_disk(int drv, _TCHAR *file_path, int bank) { }
	virtual void open_quick_disk(int drv, _TCHAR *file_path) { }
	virtual void open_hard_disk(int drv, _TCHAR *file_path) { }
	virtual void open_compact_disc(int drv, _TCHAR *file_path) { }
	virtual void open_laser_disc(int drv, _TCHAR *file_path) { }
	virtual void open_bubble_casette(int drv, _TCHAR *file_path, int bank) { }
	virtual void open_cart(int drv, _TCHAR *file_path) { }
	virtual void play_tape(int drv, _TCHAR *file_path) { }
	virtual void rec_tape(int drv, _TCHAR *file_path) { }
	virtual void load_binary(int drv, _TCHAR *file_path) { }
	virtual void save_binary(int drv, _TCHAR *file_path) { }
	
	virtual void close_floppy_disk(int drv) { }
	virtual void close_quick_disk(int drv) { }
	virtual void close_hard_disk(int drv) { }
	virtual void close_compact_disc() { }
	virtual void close_laser_disc(int drv) { }
	virtual void close_bubble_casette(int drv) { }
	virtual void close_cart(int drv) { }
	virtual void close_tape(int drv) { }
	
	virtual uint32_t is_floppy_disk_accessed() { return 0; }
	virtual uint32_t is_quick_disk_accessed() { return 0; }
	virtual uint32_t is_hard_disk_accessed() { return 0; }
	virtual uint32_t is_compact_disc_accessed() { return 0; }
	virtual uint32_t is_laser_disc_accessed() { return 0; }

	virtual bool is_floppy_disk_inserted(int drv) { return false; }
	virtual bool is_quick_disk_inserted(int drv) { return false; }
	virtual bool is_hard_disk_inserted(int drv) { return false; }
	virtual bool is_compact_disc_inserted(int drv) { return false; }
	virtual bool is_laser_disc_inserted(int drv) { return false; }
	virtual bool is_tape_inserted(int drv) { return false; }
	
	virtual void is_floppy_disk_protected(int drv, bool value) { }
	virtual bool is_floppy_disk_protected(int drv) { return false; }
	virtual void is_bubble_casette_protected(int drv, bool flag) { }
	virtual bool is_bubble_casette_protected(int drv) { return false; }
	
	virtual bool is_tape_playing(int drv) { return false; }
	virtual bool is_tape_recording(int drv) { return false; }
	virtual int get_tape_position(int drv) { return 0; }
	virtual const _TCHAR* get_tape_message(int drv) { return (const _TCHAR *) ""; }
	
	virtual void push_play(int drv) { }
	virtual void push_stop(int drv) { }
	virtual void push_fast_forward(int drv) { }
	virtual void push_fast_rewind(int drv) { }
	virtual void push_apss_forward(int drv) { }
	virtual void push_apss_rewind(int drv) { }

	virtual void update_config() { }
	virtual void save_state(FILEIO* state_fio) { }
	virtual bool load_state(FILEIO* state_fio) { }
	virtual void decl_state(void) { }
	
	// devices
	virtual void set_cpu_clock(DEVICE *cpu, uint32_t clocks) { }
	virtual void set_vm_frame_rate(double fps) { }
	virtual double get_vm_frame_rate() { return 59.94; }
	virtual bool is_frame_skippable() { return false; }
	virtual bool is_screen_changed() { return true; }
	virtual int max_draw_ranges() { return 0; }
	virtual DEVICE* get_device(int id) { return first_device; }
	
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};
#endif /* __CSP_VM_TEMPLATE_H */