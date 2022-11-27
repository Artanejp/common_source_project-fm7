#pragma once

#include "common.h"
#include <string>

class EMU_TEMPLATE;
class EVENT;
class DEVICE;
class FILEIO;
class DLL_PREFIX VM_TEMPLATE {
private:
	uint32_t m_state_version;
	const double template_default_framerate = 59.94;
protected:
	EMU_TEMPLATE* emu;
	// devices
	EVENT* event;
	std::string m_git_revision;
	
	// These are temporally functiion(s) to make backward compatibilities.
	inline void set_state_version(const uint32_t ver)
	{
		m_state_version = ver;
	}
	inline uint32_t get_state_version()
	{
		return m_state_version;
	}
	inline void set_git_repo_version(_TCHAR* p)
	{
		if(p != nullptr) {
			m_git_revision = std::string(p);
		}
	}
	void initialize_devices();
public:
	VM_TEMPLATE(EMU_TEMPLATE* parent_emu);
	virtual ~VM_TEMPLATE() {} // OK?
	// drive virtual machine
	virtual void reset();
	virtual void special_reset(int num);
	virtual void run();
	virtual void notify_power_off();
	
	virtual double get_frame_rate();
	virtual void get_screen_resolution(int *w, int *h);

	virtual void set_cpu_clock(DEVICE *cpu, uint32_t clocks);

	// debugger
	virtual DEVICE *get_cpu(int num);
	virtual void initialize(void);
	virtual void update_dipswitch(void);
	
	// draw screen
	virtual void draw_screen();

	// multimedia
	virtual void movie_sound_callback(uint8_t *buffer, long size);
	// sound generation
	virtual void initialize_sound(int rate, int samples);
	virtual uint16_t* create_sound(int* extra_frames);
	virtual int get_sound_buffer_ptr();
	virtual void set_sound_device_volume(int ch, int decibel_l, int decibel_r);

	// network
	virtual void notify_socket_connected(int ch);
	virtual void notify_socket_disconnected(int ch);
	virtual uint8_t* get_socket_send_buffer(int ch, int* size);
	virtual void inc_socket_send_buffer_ptr(int ch, int size);
	virtual uint8_t* get_socket_recv_buffer0(int ch, int* size0, int* size1);
	virtual uint8_t* get_socket_recv_buffer1(int ch);
	virtual void inc_socket_recv_buffer_ptr(int ch, int size);

	// notify key
	virtual void key_down(int code, bool repeat);
	virtual void key_up(int code);
	virtual bool get_caps_locked();
	virtual bool get_kana_locked();
	virtual uint32_t get_led_status();
	
	// user interface
	virtual void open_floppy_disk(int drv, const _TCHAR *file_path, int bank);
	virtual void open_quick_disk(int drv, const _TCHAR *file_path);
	virtual void open_hard_disk(int drv, const _TCHAR *file_path);
	virtual void open_compact_disc(int drv, const _TCHAR *file_path);
	virtual void open_laser_disc(int drv, const _TCHAR *file_path);
	virtual void open_bubble_casette(int drv, const _TCHAR *file_path, int bank);
	virtual void open_cart(int drv, const _TCHAR *file_path);
	virtual void play_tape(int drv, const _TCHAR *file_path);
	virtual void rec_tape(int drv, const _TCHAR *file_path);
	virtual void load_binary(int drv, const _TCHAR *file_path);
	virtual void save_binary(int drv, const _TCHAR *file_path);
	
	virtual void close_floppy_disk(int drv);
	virtual void close_quick_disk(int drv);
	virtual void close_hard_disk(int drv);
	virtual void close_compact_disc(int drv);
	virtual void close_laser_disc(int drv);
	virtual void close_bubble_casette(int drv);
	virtual void close_cart(int drv);
	virtual void close_tape(int drv);
	
	virtual uint32_t is_floppy_disk_accessed();
	virtual uint32_t floppy_disk_indicator_color();
	virtual uint32_t is_quick_disk_accessed();
	virtual uint32_t is_hard_disk_accessed();
	virtual uint32_t is_compact_disc_accessed();
	virtual uint32_t is_laser_disc_accessed();

	virtual bool is_floppy_disk_connected(int drv);
	virtual bool is_quick_disk_connected(int drv);
	
	virtual bool is_floppy_disk_inserted(int drv);
	virtual bool is_quick_disk_inserted(int drv);
	virtual bool is_hard_disk_inserted(int drv);
	virtual bool is_compact_disc_inserted(int drv);
	virtual bool is_cart_inserted(int drv);
	virtual bool is_laser_disc_inserted(int drv);
	virtual bool is_tape_inserted(int drv);
	
	virtual void is_floppy_disk_protected(int drv, bool value);
	virtual bool is_floppy_disk_protected(int drv);
	virtual void is_bubble_casette_protected(int drv, bool flag);
	virtual bool is_bubble_casette_protected(int drv);
	
	virtual bool is_tape_playing(int drv);
	virtual bool is_tape_recording(int drv);
	virtual int get_tape_position(int drv);
	virtual const _TCHAR* get_tape_message(int drv);
	
	virtual void push_play(int drv);
	virtual void push_stop(int drv);
	virtual void push_fast_forward(int drv);
	virtual void push_fast_rewind(int drv);
	virtual void push_apss_forward(int drv);
	virtual void push_apss_rewind(int drv);

	virtual void update_config();
	// !< This is commonly state processing.
	virtual bool process_state_core(FILEIO* state_fio, bool loading, uint32_t version = 0);
	
	// devices
	virtual void set_vm_frame_rate(double fps);
	virtual bool is_frame_skippable();
	virtual bool is_screen_changed();
	virtual int max_draw_ranges();
	virtual DEVICE* get_device(int id);
	//	misc
	virtual const _TCHAR *get_vm_git_version(void);
	virtual int get_key_name_table_size(void);
	virtual const _TCHAR *get_phy_key_name_by_scancode(uint32_t scancode);
	virtual const _TCHAR *get_phy_key_name_by_vk(uint32_t vk);
	virtual uint32_t get_scancode_by_vk(uint32_t vk);
	virtual uint32_t get_vk_by_scancode(uint32_t scancode);
	virtual double get_current_usec();
	virtual uint64_t get_current_clock_uint64();

	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

