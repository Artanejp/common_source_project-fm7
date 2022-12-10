/*
 *
 * This is some functions of VM_TEMPLATE...
 * Author: Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
 * Date  : Nov 27, 2022 -
 */
#include "vm_template.h"
#include "../fileio.h"
#include "../emu_template.h"
#include "./device.h"

VM_TEMPLATE::VM_TEMPLATE(EMU_TEMPLATE* parent_emu) :
	emu(parent_emu),
	event(nullptr),
	first_device(nullptr),
	last_device(nullptr),
	dummy(nullptr), /* Q: OK? 20221127 K.O */
	m_state_version(1)
{
	m_git_revision.clear();
}

// drive virtual machine
void VM_TEMPLATE::reset()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
}

void VM_TEMPLATE::special_reset(int num)
{
}

void VM_TEMPLATE::run()
{
}

void VM_TEMPLATE::notify_power_off()
{
}

double VM_TEMPLATE::get_frame_rate()
{
	return template_default_framerate;
}

void VM_TEMPLATE::get_screen_resolution(int *w, int *h)
{
	if(w != nullptr) *w = 0;
	if(h != nullptr) *h = 0;
}

void VM_TEMPLATE::initialize()
{
}

void VM_TEMPLATE::initialize_devices()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
}

void VM_TEMPLATE::release_devices()
{
	for(DEVICE* device = first_device; device;) {
		DEVICE *next_device = device->next_device;
		device->release();
		delete device;
		device = next_device;
	}
}

void VM_TEMPLATE::update_dipswitch()
{
}

// debugger
DEVICE *VM_TEMPLATE::get_cpu(int num)
{
	return nullptr;
}

// draw screen
void VM_TEMPLATE::draw_screen()
{
}

// multimedia
void VM_TEMPLATE::movie_sound_callback(uint8_t *buffer, long size)
{
}

// sound generation
void VM_TEMPLATE::initialize_sound(int rate, int samples)
{
}

uint16_t* VM_TEMPLATE::create_sound(int* extra_frames)
{
	return nullptr;
}

int VM_TEMPLATE::get_sound_buffer_ptr()
{
	return 0;
}

void VM_TEMPLATE::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
}


// network
void VM_TEMPLATE::notify_socket_connected(int ch)
{
}

void VM_TEMPLATE::notify_socket_disconnected(int ch)
{
}

uint8_t* VM_TEMPLATE::get_socket_send_buffer(int ch, int* size)
{
	if(size != nullptr) *size = 0;
	return nullptr;
}
void VM_TEMPLATE::inc_socket_send_buffer_ptr(int ch, int size)
{
}

uint8_t* VM_TEMPLATE::get_socket_recv_buffer0(int ch, int* size0, int* size1)
{
	if(size0 != nullptr) *size0 = 0;
	if(size1 != nullptr) *size1 = 0;
	return nullptr;
}

uint8_t* VM_TEMPLATE::get_socket_recv_buffer1(int ch)
{
	return nullptr;
}

void VM_TEMPLATE::inc_socket_recv_buffer_ptr(int ch, int size)
{
}

// notify key
void VM_TEMPLATE::key_down(int code, bool repeat)
{
}

void VM_TEMPLATE::key_up(int code)
{
}

bool VM_TEMPLATE::get_caps_locked()
{
	return false;
}

bool VM_TEMPLATE::get_kana_locked()
{
	return false;
}

uint32_t VM_TEMPLATE::get_led_status()
{
	return 0;
}

// user interface
void VM_TEMPLATE::open_floppy_disk(int drv, const _TCHAR *file_path, int bank)
{
}

void VM_TEMPLATE::open_quick_disk(int drv, const _TCHAR *file_path)
{
}

void VM_TEMPLATE::open_hard_disk(int drv, const _TCHAR *file_path)
{
}

void VM_TEMPLATE::open_compact_disc(int drv, const _TCHAR *file_path)
{
}

void VM_TEMPLATE::open_laser_disc(int drv, const _TCHAR *file_path)
{
}

void VM_TEMPLATE::open_bubble_casette(int drv, const _TCHAR *file_path, int bank)
{
}

void VM_TEMPLATE::open_cart(int drv, const _TCHAR *file_path)
{
}

void VM_TEMPLATE::play_tape(int drv, const _TCHAR *file_path)
{
}

void VM_TEMPLATE::rec_tape(int drv, const _TCHAR *file_path)
{
}

void VM_TEMPLATE::load_binary(int drv, const _TCHAR *file_path)
{
}

void VM_TEMPLATE::save_binary(int drv, const _TCHAR *file_path)
{
}


void VM_TEMPLATE::close_floppy_disk(int drv)
{
}

void VM_TEMPLATE::close_quick_disk(int drv)
{
}

void VM_TEMPLATE::close_hard_disk(int drv)
{
}

void VM_TEMPLATE::close_compact_disc(int drv)
{
}

void VM_TEMPLATE::close_laser_disc(int drv)
{
}

void VM_TEMPLATE::close_bubble_casette(int drv)
{
}

void VM_TEMPLATE::close_cart(int drv)
{
}

void VM_TEMPLATE::close_tape(int drv)
{
}
	
uint32_t VM_TEMPLATE::is_floppy_disk_accessed()
{
	return 0;
}

uint32_t VM_TEMPLATE::floppy_disk_indicator_color()
{
	return 0;
}

uint32_t VM_TEMPLATE::is_quick_disk_accessed()
{
	return 0;
}

uint32_t VM_TEMPLATE::is_hard_disk_accessed()
{
	return 0;
}

uint32_t VM_TEMPLATE::is_compact_disc_accessed()
{
	return 0;
}

uint32_t VM_TEMPLATE::is_laser_disc_accessed()
{
	return 0;
}

bool VM_TEMPLATE::is_floppy_disk_connected(int drv)
{
	return true;
}

bool VM_TEMPLATE::is_quick_disk_connected(int drv)
{
	return true;
}
	
bool VM_TEMPLATE::is_floppy_disk_inserted(int drv)
{
	return false;
}

bool VM_TEMPLATE::is_quick_disk_inserted(int drv)
{
	return false;
}

bool VM_TEMPLATE::is_hard_disk_inserted(int drv)
{
	return false;
}

bool VM_TEMPLATE::is_compact_disc_inserted(int drv)
{
	return false;
}

bool VM_TEMPLATE::is_cart_inserted(int drv)
{
	return false;
}

bool VM_TEMPLATE::is_laser_disc_inserted(int drv)
{
	return false;
}

bool VM_TEMPLATE::is_tape_inserted(int drv)
{
	return false;
}
	
void VM_TEMPLATE::is_floppy_disk_protected(int drv, bool value)
{
}

bool VM_TEMPLATE::is_floppy_disk_protected(int drv)
{
	return false;
}

bool VM_TEMPLATE::is_bubble_casette_inserted(int drv)
{
	return false;
}

void VM_TEMPLATE::is_bubble_casette_protected(int drv, bool flag)
{
}

bool VM_TEMPLATE::is_bubble_casette_protected(int drv)
{
	return false;
}
	
bool VM_TEMPLATE::is_tape_playing(int drv)
{
	return false;
}

bool VM_TEMPLATE::is_tape_recording(int drv)
{
	return false;
}

int VM_TEMPLATE::get_tape_position(int drv)
{
	return 0;
}

const _TCHAR* VM_TEMPLATE::get_tape_message(int drv)
{
	return _T("");
}
	
void VM_TEMPLATE::push_play(int drv)
{

}

void VM_TEMPLATE::push_stop(int drv)
{

}

void VM_TEMPLATE::push_fast_forward(int drv)
{

}

void VM_TEMPLATE::push_fast_rewind(int drv)
{

}

void VM_TEMPLATE::push_apss_forward(int drv)
{

}

void VM_TEMPLATE::push_apss_rewind(int drv)
{
}


void VM_TEMPLATE::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

// State processing
bool VM_TEMPLATE::process_state_core(FILEIO* state_fio, bool loading, uint32_t version)
{
	if(version == 0) {
		version = get_state_version();
	}
	if(!state_fio->StateCheckUint32(version)) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const _TCHAR *name = char_to_tchar(typeid(*device).name() + 6); // skip "class "
		int len = (int)_tcslen(name);
		
		if(!state_fio->StateCheckInt32(len)) {
			if(loading) {
				printf("Class name len Error: DEVID=%d EXPECT=%s\n", device->this_device_id, name);
			}
			return false;
		}
		if(!state_fio->StateCheckBuffer(name, len, 1)) {
			if(loading) {
				printf("Class name Error: DEVID=%d EXPECT=%s\n", device->this_device_id, name);
			}
			return false;
		}
		if(!device->process_state(state_fio, loading)) {
			if(loading) {
				printf("Data loading Error: DEVID=%d\n", device->this_device_id);
			}
			return false;
		}
	}
	return true;
}

// devices
void VM_TEMPLATE::set_vm_frame_rate(double fps)
{
}

bool VM_TEMPLATE::is_frame_skippable()
{
	return false;
}

bool VM_TEMPLATE::is_screen_changed()
{
	return true;
}

int VM_TEMPLATE::max_draw_ranges()
{
	return 0;
}

DEVICE* VM_TEMPLATE::get_device(int id)
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(device->this_device_id == id) {
			return device;
		}
	}
	return nullptr;
}

void VM_TEMPLATE::set_cpu_clock(DEVICE *cpu, uint32_t clocks)
{
}

//	misc
const _TCHAR *VM_TEMPLATE::get_vm_git_version(void)
{
	return (const _TCHAR *)(m_git_revision.c_str());
}

int VM_TEMPLATE::get_key_name_table_size(void)
{
	return 0;
}

const _TCHAR *VM_TEMPLATE::get_phy_key_name_by_scancode(uint32_t scancode)
{
	return (const _TCHAR *)nullptr;
}

const _TCHAR *VM_TEMPLATE::get_phy_key_name_by_vk(uint32_t vk)
{
	return (const _TCHAR *)nullptr;
}

uint32_t VM_TEMPLATE::get_scancode_by_vk(uint32_t vk)
{
	return 0xffffffff;
}

uint32_t VM_TEMPLATE::get_vk_by_scancode(uint32_t scancode)
{
	return 0xffffffff;
}

double VM_TEMPLATE::get_current_usec()
{
	return 0.0;
}

uint64_t VM_TEMPLATE::get_current_clock_uint64()
{
	return (uint64_t)0;
}
