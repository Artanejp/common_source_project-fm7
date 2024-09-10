/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 emulation i/f ]
*/

#if defined(_USE_QT)
#include <string>
#include "qt/common/menu_flags_ext.h"
#endif

#include "emu.h"
#if defined(USE_DEBUGGER)
#include "vm/debugger.h"
#endif
#include "vm/vm.h"
#include "fifo.h"
#include "fileio.h"

#include "osdcall_types.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------
#if !defined(_USE_QT)
static const int sound_frequency_table[8] = {
	2000, 4000, 8000, 11025, 22050, 44100,
#ifdef OVERRIDE_SOUND_FREQ_48000HZ
	OVERRIDE_SOUND_FREQ_48000HZ,
#else
	48000,
#endif
	96000,
};
static const double sound_latency_table[5] = {0.05, 0.1, 0.2, 0.3, 0.4};
#endif

#if defined(_USE_QT)
// Please permit at least them m(.. )m
//extern void get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
#include <string>
#include <memory>
#endif

#if defined(_USE_QT)
EMU::EMU(class Ui_MainWindow *hwnd, GLDrawClass *hinst, std::shared_ptr<CSP_Logger> p_logger, std::shared_ptr<USING_FLAGS> p) : EMU_TEMPLATE(hwnd, hinst, p_logger, p)
#elif defined(OSD_WIN32)
EMU::EMU(HWND hwnd, HINSTANCE hinst) : EMU_TEMPLATE(hwnd, hinst)
#else
EMU::EMU() : EMU()
#endif
{
	message_count = 0;
	// store main window handle
#ifdef USE_FLOPPY_DISK
	// initialize d88 file info
	memset(d88_file, 0, sizeof(d88_file));
#endif
#ifdef USE_BUBBLE
	// initialize d88 file info
	memset(b77_file, 0, sizeof(b77_file));
#endif
	// load sound config

	if(!(0 <= config.sound_frequency && config.sound_frequency < 8)) {
		config.sound_frequency = 6;	// default: 48KHz
	}
	if(!(0 <= config.sound_latency && config.sound_latency < 5)) {
		config.sound_latency = 1;	// default: 100msec
	}
	#if !defined(_USE_QT)
	sound_rate = sound_frequency_table[config.sound_frequency];
	sound_samples = (int)(sound_rate * sound_latency_table[config.sound_latency] + 0.5);
	#else
	if(using_flags.get() != nullptr) {
		sound_rate = using_flags->get_sound_sample_rate(config.sound_frequency);
		sound_samples = (int)(sound_rate * using_flags->get_sound_latency(config.sound_latency) + 0.5);
	} else {
		// Fallback
		sound_rate = 48000;
		sound_samples = (int)(sound_rate * 0.1 + 0.5); // 100mSec
	}
	#endif
	
#ifdef USE_CPU_TYPE
	cpu_type = config.cpu_type;
#endif
#ifdef USE_DIPSWITCH
	dipswitch = config.dipswitch;
#endif
#ifdef USE_SOUND_TYPE
	sound_type = config.sound_type;
#endif
#ifdef USE_PRINTER_TYPE
	printer_type = config.printer_type;
#endif
#ifdef USE_SERIAL_TYPE
	serial_type = config.serial_type;
#endif
#ifdef USE_BUBBLE
	// initialize b77 file info
	memset(b77_file, 0, sizeof(b77_file));
#endif

	// initialize osd
#if defined(OSD_QT)
	osd = new OSD(p, csp_logger);
	osd->main_window_handle = hwnd;
	//osd->p_glv = hinst;
	osd->host_cpus = 4;
#elif defined(OSD_WIN32)
	osd = new OSD();
	osd->main_window_handle = hwnd;
	osd->instance_handle = hinst;
#endif
	int presented_rate, presented_samples;
	osd->initialize(sound_rate, sound_samples, &presented_rate, &presented_samples);
	sound_rate = presented_rate;
	sound_samples = presented_samples;
	// initialize vm
   	vm = new VM(this);
	osd->vm = vm;
# if defined(_USE_QT)
	osd->vm_has_set();
	osd->reset_vm_node();
	osd->update_keyname_table();
# endif
#ifdef USE_AUTO_KEY
	initialize_auto_key();
#endif
#ifdef USE_DEBUGGER
	initialize_debugger();
#endif
	now_waiting_in_debugger = false;
	initialize_media();
	__LIKELY_IF(vm != NULL) {
		vm->initialize_sound(sound_rate, sound_samples);
#ifdef USE_SOUND_VOLUME
		for(int i = 0; i < USE_SOUND_VOLUME; i++) {
			vm->set_sound_device_volume(i, config.sound_volume_l[i], config.sound_volume_r[i]);
		}
#endif
#ifdef USE_HARD_DISK
		for(int drv = 0; drv < USE_HARD_DISK; drv++) {
			if(config.last_hard_disk_path[drv][0] != _T('\0') && FILEIO::IsFileExisting(config.last_hard_disk_path[drv])) {
				vm->open_hard_disk(drv, config.last_hard_disk_path[drv]);
				my_tcscpy_s(hard_disk_status[drv].path, _MAX_PATH, config.last_hard_disk_path[drv]);
			}
		}
#endif
		vm->reset();
	}
#if defined(_USE_QT) // Temporally
	osd->sync_some_devices();
#endif
	now_suspended = false;
}

EMU::~EMU()
{
#ifdef USE_AUTO_KEY
	release_auto_key();
#endif
#ifdef USE_DEBUGGER
	release_debugger();
#endif
	if(osd != NULL) {
		osd->lock_vm();
	}
	if(osd != NULL) {
		osd->vm = nullptr;
		osd->force_unlock_vm();
	}
	if(vm != NULL) {
		delete vm;
		vm = NULL;
	}
	if(osd != NULL) {
		osd->release();
		delete osd;
	}
#ifdef _DEBUG_LOG
	release_debug_log();
#endif
}

// messaging proxies.
// These are mostly used for replying mounting virtual media.
// 20230125 K.O
void EMU::osdcall_string(EMU_MEDIA_TYPE::type_t media_type, int drive,EMU_MESSAGE_TYPE::type_t message_type, _TCHAR* message)
{
	__LIKELY_IF(osd != nullptr) {
		osd->string_message_from_emu(media_type, drive, message_type, message);
	}
}

void EMU::osdcall_int(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, int64_t data)
{
	__LIKELY_IF(osd != nullptr) {
		osd->int_message_from_emu(media_type, drive, message_type, data);
	}
}

void EMU::osdcall_bubble_inserted(int drv, _TCHAR* filestr, int bank)
{
#ifdef USE_BUBBLE
	if((vm == NULL) || (drv < 0) || (drv >= USE_BUBBLE) || (drv >= 16)) {
		return;
	}
	if(!(vm->is_bubble_casette_inserted(drv))) {
		return;
	}
	EMU_MESSAGE_TYPE::type_t mess = EMU_MESSAGE_TYPE::MEDIA_MOUNTED;
	mess |= ((is_bubble_casette_protected(drv)) ? EMU_MESSAGE_TYPE::WRITE_PROTECT : 0);

	if((bank < 0) || (b77_file[drv].bank_num <= 0)) {
		bank = 0;
	}
	
	EMU_MEDIA_TYPE::type_t media_type = EMU_MEDIA_TYPE::BUBBLE_CASETTE | (bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK);
	
	osdcall_int(EMU_MEDIA_TYPE::BUBBLE_CASETTE,
				drv,
				EMU_MESSAGE_TYPE::VIRT_MEDIA_CLEAR,
				0);
	if(b77_file[drv].bank_num > 0) {
		for(uint64_t i = 0; i < b77_file[drv].bank_num; i++) {
			if(i >= 16) break;
			EMU_MEDIA_TYPE::type_t submedia_type = EMU_MEDIA_TYPE::BUBBLE_CASETTE | i;
			EMU_MESSAGE_TYPE::type_t submess = EMU_MESSAGE_TYPE::VIRT_MEDIA_UPDATE;
			_TCHAR tmp_name[128] = {0};
			if(strlen(b77_file[drv].bubble_name[i]) > 0) {
				my_tcscpy_s(tmp_name, 128, b77_file[drv].bubble_name[i]);
			} else {
				my_sprintf_s(tmp_name, 128, _T("*Slot %d*"), i + 1);
			}
			osdcall_string(submedia_type, drv, submess, tmp_name);
		}
		osdcall_int(media_type, drv, EMU_MESSAGE_TYPE::VIRT_MEDIA_FINISH, 0);
	}
	osdcall_string(media_type, drv, mess, filestr);

#endif
}

void EMU::osdcall_bubble_ejected(int drv)
{
#ifdef USE_BUBBLE
	if((drv < 0) || (drv >= USE_BUBBLE)) {
		return;
	}
	osdcall_int(EMU_MEDIA_TYPE::BUBBLE_CASETTE,
				drv,
				EMU_MESSAGE_TYPE::MEDIA_REMOVED,
				0);
#endif
}

void EMU::osdcall_floppy_inserted(int drv, _TCHAR* filestr, int bank)
{
#ifdef USE_FLOPPY_DISK
	if((vm == NULL) || (drv < 0) || (drv >= USE_FLOPPY_DISK) || (drv >= 16)) {
		return;
	}
	if(!(vm->is_floppy_disk_inserted(drv))) {
		return;
	}
	EMU_MESSAGE_TYPE::type_t mess = EMU_MESSAGE_TYPE::MEDIA_MOUNTED;
	mess |= ((is_floppy_disk_protected(drv)) ? EMU_MESSAGE_TYPE::WRITE_PROTECT : 0);
	EMU_MEDIA_TYPE::type_t media_type = EMU_MEDIA_TYPE::FLOPPY_DISK | (bank & EMU_MEDIA_TYPE::MULTIPLE_SLOT_MASK);
	
	osdcall_int(EMU_MEDIA_TYPE::FLOPPY_DISK,
				drv,
				EMU_MESSAGE_TYPE::VIRT_MEDIA_CLEAR,
				0);
	if(d88_file[drv].bank_num > 0) {
		for(uint64_t i = 0; i < d88_file[drv].bank_num; i++) {
			if(i >= 64) break;
			EMU_MEDIA_TYPE::type_t submedia_type = EMU_MEDIA_TYPE::FLOPPY_DISK | i;
			EMU_MESSAGE_TYPE::type_t submess = EMU_MESSAGE_TYPE::VIRT_MEDIA_UPDATE;
			_TCHAR tmp_name[128] = {0};
			if(strlen(d88_file[drv].disk_name[i]) > 0) {
				my_tcscpy_s(tmp_name, 128, d88_file[drv].disk_name[i]);
			} else {
				my_sprintf_s(tmp_name, 128, _T("*Slot %d*"), i + 1);
			}				
			osdcall_string(submedia_type, drv, submess, tmp_name);
		}
		osdcall_int(media_type, drv, EMU_MESSAGE_TYPE::VIRT_MEDIA_FINISH, 0);
	}
	
	osdcall_string(media_type, drv, mess, filestr);
#endif
}

void EMU::osdcall_floppy_ejected(int drv)
{
#ifdef USE_FLOPPY_DISK
	if((drv < 0) || (drv >= USE_FLOPPY_DISK)) {
		return;
	}
	osdcall_int(EMU_MEDIA_TYPE::FLOPPY_DISK,
				drv,
				EMU_MESSAGE_TYPE::MEDIA_REMOVED,
				0);
#endif
}

void EMU::osdcall_quickdisk_inserted(int drv, _TCHAR* filestr)
{
#ifdef USE_QUICK_DISK
	if((vm == NULL) || (drv < 0) || (drv >= USE_QUICK_DISK)) {
		return;
	}
	if(!(vm->is_quick_disk_inserted(drv))) {
		return;
	}
	// ToDo: Write Protect
	osdcall_string(EMU_MEDIA_TYPE::QUICK_DISK,
				   drv,
				   EMU_MESSAGE_TYPE::MEDIA_MOUNTED,
				   filestr);
#endif
}

void EMU::osdcall_quickdisk_ejected(int drv)
{
#ifdef USE_QUICK_DISK
	if((vm == NULL) || (drv < 0) || (drv >= USE_QUICK_DISK)) {
		return;
	}
	osdcall_int(EMU_MEDIA_TYPE::QUICK_DISK,
				drv,
				EMU_MESSAGE_TYPE::MEDIA_REMOVED,
				0);
#endif
}
#ifdef OSD_QT
EmuThreadClass *EMU::get_parent_handler()
{
	__LIKELY_IF(osd != nullptr) {
		return osd->get_parent_handler();
	}
	return NULL;
}

void EMU::set_parent_handler(EmuThreadClass *p, std::shared_ptr<DrawThreadClass> q)
{
	__LIKELY_IF(osd != nullptr) {
		osd->set_parent_thread(p);
		osd->set_draw_thread(q);
	}
}

void EMU::set_host_cpus(int v)
{
	osd->host_cpus = (v <= 0) ? 1 : v;
}

int EMU::get_host_cpus()
{
	__LIKELY_IF(osd != nullptr) {
		return osd->host_cpus;
	}
	return 0; // OK?
}
#endif

// ----------------------------------------------------------------------------
// drive machine
// ----------------------------------------------------------------------------

double EMU::get_frame_rate()
{
	__LIKELY_IF(vm != NULL) {
		return vm->get_frame_rate();
	}
	return 30.0; // Dummy
}

int64_t EMU::get_frame_interval()
{
	// Note: Return by uSec.
	static int64_t prev_interval = 0;
	static double prev_fps = -1;
	double fps = 30.0; // Dummy
	__LIKELY_IF(vm != NULL) {
		fps = vm->get_frame_rate();
	}
	if(prev_fps != fps) {
		prev_interval = llrint(1024.0 * 1.0e6 / fps);
		prev_fps = fps;
	}
	return prev_interval;
}

bool EMU::is_frame_skippable()
{
	__LIKELY_IF(vm != NULL) {
		return vm->is_frame_skippable();
	}
	return false;
}

bool EMU::is_half_event()
{
	__LIKELY_IF(vm != NULL) {
		return vm->is_half_event();
	}
	return false;
}

void EMU::request_update_screen()
{
	__LIKELY_IF(vm != NULL) {
		vm->request_update_screen();
	}
}

const bool EMU::is_use_state()
{
#ifdef USE_STATE
	return true;
#else
	return false;
#endif
}
/*!
 * @note Now, run() calles per half of a frame. 20240214 K.O
 */
int EMU::run()
{
	__UNLIKELY_IF(vm == NULL) {
		return 1; // Dummy.
	}

#if defined(USE_DEBUGGER) && defined(USE_STATE)
	if(request_save_state >= 0 || request_load_state >= 0) {
		if(request_save_state >= 0) {
			save_state(state_file_path(request_save_state));
		} else {
			load_state(state_file_path(request_load_state));
		}
		// NOTE: vm instance may be reinitialized in load_state
		// ToDo: Support multiple debuggers. 20230110 K.O
		if(!is_debugger_enabled(debugger_cpu_index)) {
			for(int i = 0; i < 8; i++) {
				if(is_debugger_enabled(i)) {
					debugger_cpu_index = i;
					debugger_target_id = vm->get_cpu(debugger_cpu_index)->this_device_id;
					break;
				}
			}
		}
		if(is_debugger_enabled(debugger_cpu_index)) {
			if(!(vm->get_device(debugger_target_id) != NULL && vm->get_device(debugger_target_id)->get_debugger() != NULL)) {
				debugger_target_id = vm->get_cpu(debugger_cpu_index)->this_device_id;
			}
			DEBUGGER *cpu_debugger = (DEBUGGER *)vm->get_cpu(debugger_cpu_index)->get_debugger();
			cpu_debugger->now_going = false;
			cpu_debugger->now_debugging = true;
			debugger_thread_param.vm = vm;
		} else {
			close_debugger(debugger_cpu_index);
		}
		request_save_state = request_load_state = -1;
	}
#endif
	if(now_suspended) {
		osd->restore();
		now_suspended = false;
	}
	osd->update_input();
#ifdef USE_AUTO_KEY
	update_auto_key();
#endif
//#ifdef USE_JOYSTICK
//	update_joystick();
//#endif

#ifdef USE_SOCKET
#if !defined(_USE_QT) // Temporally
 	osd->update_socket();
#endif
#endif
	update_media();

	// virtual machine may be driven to fill sound buffer
	int extra_frames = 0;

	osd->update_sound(&extra_frames);
	// drive virtual machine
	if(extra_frames == 0) {
		osd->lock_vm();
		vm->run();
		if(!(is_half_event())) {
			extra_frames = 1;
		}
		osd->unlock_vm();
	}
	osd->add_extra_frames(extra_frames);
	return extra_frames;
}

void EMU::reset()
{
#ifdef USE_AUTO_KEY
	stop_auto_key();
	config.romaji_to_kana = false;
#endif

	// check if virtual machine should be reinitialized
	bool reinitialize = false;
#ifdef USE_CPU_TYPE
	reinitialize |= (cpu_type != config.cpu_type);
	cpu_type = config.cpu_type;
#endif
#ifdef USE_DIPSWITCH
	reinitialize |= (dipswitch != config.dipswitch);
	dipswitch = config.dipswitch;
#endif
#ifdef USE_SOUND_TYPE
	reinitialize |= (sound_type != config.sound_type);
	sound_type = config.sound_type;
#endif
#ifdef USE_PRINTER_TYPE
	reinitialize |= (printer_type != config.printer_type);
	printer_type = config.printer_type;
#endif
#ifdef USE_SERIAL_TYPE
	reinitialize |= (serial_type != config.serial_type);
	serial_type = config.serial_type;
#endif
	if(reinitialize) {
		// stop sound
		osd->stop_sound();
		// reinitialize virtual machine
		osd->vm = nullptr;
		osd->force_unlock_vm();
		delete vm;
		vm = nullptr;
		vm = new VM(this);
 		osd->vm = vm;
#if defined(_USE_QT)
		osd->vm_has_set();
		osd->reset_vm_node();
		osd->update_keyname_table();
		osd->reset_screen_buffer();
#endif
		int presented_rate;
		int presented_samples;
		#if !defined(_USE_QT)
		sound_rate = sound_frequency_table[config.sound_frequency];
		sound_samples = (int)(sound_rate * sound_latency_table[config.sound_latency] + 0.5);
		#else
		if(using_flags.get() != nullptr) {
			sound_rate = using_flags->get_sound_sample_rate(config.sound_frequency);
			sound_samples = (int)(sound_rate * using_flags->get_sound_latency(config.sound_latency) + 0.5);
		} else {
			// Fallback
			sound_rate = 48000;
			sound_samples = (int)(sound_rate * 0.1 + 0.5); // 100mSec
		}
		#endif
		osd->initialize_sound(sound_rate, sound_samples, &presented_rate, &presented_samples);
		if((sound_rate != presented_rate) ||
		   (sound_samples != presented_samples)) {
			sound_rate = presented_rate;
			sound_samples = presented_samples;
		}
		vm->initialize_sound(sound_rate, sound_samples);
#ifdef USE_SOUND_VOLUME
		for(int i = 0; i < USE_SOUND_VOLUME; i++) {
			vm->set_sound_device_volume(i, config.sound_volume_l[i], config.sound_volume_r[i]);
		}
#endif
		// restore inserted medias
		restore_media();
		vm->reset();
		osd->unlock_vm();
	} else {
		// reset virtual machine
		osd->lock_vm();
		vm->reset();
		osd->unlock_vm();
	}
#if defined(_USE_QT) // Temporally
	osd->sync_some_devices();
#endif
#if !defined(_USE_QT) // Temporally
	// restart recording
	osd->restart_record_sound();
	osd->restart_record_video();
#endif
}

#ifdef USE_SPECIAL_RESET
void EMU::special_reset(int num)
{
	if(num < 0) return;
	if(num >= USE_SPECIAL_RESET) return;
#ifdef USE_AUTO_KEY
	stop_auto_key();
	config.romaji_to_kana = false;
#endif

	// reset virtual machine
	osd->lock_vm();
	vm->special_reset(num);
	osd->unlock_vm();
	// restart recording
#if !defined(_USE_QT) // Temporally
	restart_record_sound();
	restart_record_video();
#endif
}
#endif

#ifdef USE_NOTIFY_POWER_OFF
void EMU::notify_power_off()
{
	__LIKELY_IF(vm != NULL) {
		vm->notify_power_off();
	}
	osd->notify_power_off(); // To GUI 20230120 K.O
}
#endif

void EMU::power_off()
{
	osd->power_off();
}

void EMU::suspend()
{
	if(!now_suspended) {
		osd->suspend();
		now_suspended = true;
	}
}

void EMU::lock_vm()
{
	osd->lock_vm();
}

void EMU::unlock_vm()
{
	osd->unlock_vm();
}

void EMU::force_unlock_vm()
{
	osd->force_unlock_vm();
}

bool EMU::is_vm_locked()
{
	return osd->is_vm_locked();
}

// ----------------------------------------------------------------------------
// input
// ----------------------------------------------------------------------------


void EMU::key_down(int code, bool extended, bool repeat)
{
#ifdef USE_AUTO_KEY
	if(code == 0x10) {
		shift_pressed = true;
	}
	if(config.romaji_to_kana) {
		if(!repeat) {
			// Page Up, Page Down, End, Home, Left, Up, Right, Down, Ins, Del, Help, and F1-F12
			if((code >= 0x21 && code <= 0x2f) || (code >= 0x70 && code <= 0x7b)) {
				if(shift_pressed) {
					auto_key_buffer->write(code | 0x100);
				} else {
					auto_key_buffer->write(code);
				}
				if(!is_auto_key_running()) {
					start_auto_key();
				}
			}
		}
	} else if(!is_auto_key_running())
#endif
	osd->key_down(code, extended, repeat);
//	printf("KEY DOWN: %04X EXT=%d REPEAT=%d\n", code, extended, repeat);
}

void EMU::key_up(int code, bool extended)
{
#ifdef USE_AUTO_KEY
	if(code == 0x10) {
		shift_pressed = false;
	}
	if(config.romaji_to_kana) {
		// do nothing
	} else if(!is_auto_key_running())
#endif
	osd->key_up(code, extended);
//	printf("KEY UP: %04X EXT=%d\n", code, extended);
}

void EMU::key_char(char code)
{
#ifdef USE_AUTO_KEY
	if(config.romaji_to_kana) {
		set_auto_key_char(code);
	}
#endif
}

#ifdef USE_KEY_LOCKED
bool EMU::get_caps_locked()
{
	__LIKELY_IF(vm != NULL) {
		return vm->get_caps_locked();
	}
	return false;
}

bool EMU::get_kana_locked()
{
	__LIKELY_IF(vm != NULL) {
		return vm->get_kana_locked();
	}
	return false;
}
#endif

void EMU::key_lost_focus()
{
	osd->key_lost_focus();
}

#ifdef ONE_BOARD_MICRO_COMPUTER
void EMU::press_button(int num)
{
	int code = vm_buttons[num].code;

	if(code) {
		osd->key_down_native(code, false);
		osd->get_key_buffer()[code] = KEY_KEEP_FRAMES;
	} else {
		// code=0: reset virtual machine
		__LIKELY_IF(vm != NULL) {
			vm->reset();
		}
	}
}
#endif

#ifdef USE_MOUSE
void EMU::enable_mouse()
{
	osd->enable_mouse();
}

void EMU::disable_mouse()
{
	osd->disable_mouse();
}

void EMU::toggle_mouse()
{
	osd->toggle_mouse();
}

bool EMU::is_mouse_enabled()
{
	return osd->is_mouse_enabled();
}
#endif

#ifdef USE_AUTO_KEY
static const int auto_key_table_base[][2] = {
	// 0x100: shift
	// 0x200: kana
	// 0x400: alphabet
	// 0x800: ALPHABET
	// 0x1000 : LOCK
	// 0x2000 : UNLOCK
	{0x08,	0x000 | 0x08},	// BS
	{0x09,	0x000 | 0x09},	// Tab
	{0x0d,	0x000 | 0x0d},	// Enter
	{0x1b,	0x000 | 0x1b},	// Escape
	{0x20,	0x000 | 0x20},	// ' '
#ifdef AUTO_KEY_US
	{0x21,	0x100 | 0x31},	// '!'
	{0x22,	0x100 | 0xba},	// '"'
	{0x23,	0x100 | 0x33},	// '#'
	{0x24,	0x100 | 0x34},	// '$'
	{0x25,	0x100 | 0x35},	// '%'
	{0x26,	0x100 | 0x37},	// '&'
	{0x27,	0x000 | 0xba},	// '''
	{0x28,	0x100 | 0x39},	// '('
	{0x29,	0x100 | 0x30},	// ')'
	{0x2a,	0x100 | 0x38},	// '*'
	{0x2b,	0x100 | 0xde},	// '+'
	{0x2c,	0x000 | 0xbc},	// ','
	{0x2d,	0x000 | 0xbd},	// '-'
	{0x2e,	0x000 | 0xbe},	// '.'
	{0x2f,	0x000 | 0xbf},	// '/'
#else
	{0x21,	0x100 | 0x31},	// '!'
	{0x22,	0x100 | 0x32},	// '"'
	{0x23,	0x100 | 0x33},	// '#'
	{0x24,	0x100 | 0x34},	// '$'
	{0x25,	0x100 | 0x35},	// '%'
	{0x26,	0x100 | 0x36},	// '&'
	{0x27,	0x100 | 0x37},	// '''
	{0x28,	0x100 | 0x38},	// '('
	{0x29,	0x100 | 0x39},	// ')'
	{0x2a,	0x100 | 0xba},	// '*'
	{0x2b,	0x100 | 0xbb},	// '+'
	{0x2c,	0x000 | 0xbc},	// ','
	{0x2d,	0x000 | 0xbd},	// '-'
	{0x2e,	0x000 | 0xbe},	// '.'
	{0x2f,	0x000 | 0xbf},	// '/'
#endif
	{0x30,	0x000 | 0x30},	// '0'
	{0x31,	0x000 | 0x31},	// '1'
	{0x32,	0x000 | 0x32},	// '2'
	{0x33,	0x000 | 0x33},	// '3'
	{0x34,	0x000 | 0x34},	// '4'
	{0x35,	0x000 | 0x35},	// '5'
	{0x36,	0x000 | 0x36},	// '6'
	{0x37,	0x000 | 0x37},	// '7'
	{0x38,	0x000 | 0x38},	// '8'
	{0x39,	0x000 | 0x39},	// '9'
#ifdef AUTO_KEY_US
	{0x3a,	0x100 | 0xbb},	// ':'
	{0x3b,	0x000 | 0xbb},	// ';'
	{0x3c,	0x100 | 0xbc},	// '<'
	{0x3d,	0x000 | 0xde},	// '='
	{0x3e,	0x100 | 0xbe},	// '>'
	{0x3f,	0x100 | 0xbf},	// '?'
	{0x40,	0x100 | 0x32},	// '@'
#else
	{0x3a,	0x000 | 0xba},	// ':'
	{0x3b,	0x000 | 0xbb},	// ';'
	{0x3c,	0x100 | 0xbc},	// '<'
	{0x3d,	0x100 | 0xbd},	// '='
	{0x3e,	0x100 | 0xbe},	// '>'
	{0x3f,	0x100 | 0xbf},	// '?'
	{0x40,	0x000 | 0xc0},	// '@'
#endif
	{0x41,	0x400 | 0x41},	// 'A'
	{0x42,	0x400 | 0x42},	// 'B'
	{0x43,	0x400 | 0x43},	// 'C'
	{0x44,	0x400 | 0x44},	// 'D'
	{0x45,	0x400 | 0x45},	// 'E'
	{0x46,	0x400 | 0x46},	// 'F'
	{0x47,	0x400 | 0x47},	// 'G'
	{0x48,	0x400 | 0x48},	// 'H'
	{0x49,	0x400 | 0x49},	// 'I'
	{0x4a,	0x400 | 0x4a},	// 'J'
	{0x4b,	0x400 | 0x4b},	// 'K'
	{0x4c,	0x400 | 0x4c},	// 'L'
	{0x4d,	0x400 | 0x4d},	// 'M'
	{0x4e,	0x400 | 0x4e},	// 'N'
	{0x4f,	0x400 | 0x4f},	// 'O'
	{0x50,	0x400 | 0x50},	// 'P'
	{0x51,	0x400 | 0x51},	// 'Q'
	{0x52,	0x400 | 0x52},	// 'R'
	{0x53,	0x400 | 0x53},	// 'S'
	{0x54,	0x400 | 0x54},	// 'T'
	{0x55,	0x400 | 0x55},	// 'U'
	{0x56,	0x400 | 0x56},	// 'V'
	{0x57,	0x400 | 0x57},	// 'W'
	{0x58,	0x400 | 0x58},	// 'X'
	{0x59,	0x400 | 0x59},	// 'Y'
	{0x5a,	0x400 | 0x5a},	// 'Z'
#ifdef AUTO_KEY_US
	{0x5b,	0x000 | 0xc0},	// '['
	{0x5c,	0x000 | 0xe2},	// '\'
	{0x5d,	0x000 | 0xdb},	// ']'
	{0x5e,	0x100 | 0x36},	// '^'
	{0x5f,	0x100 | 0xbd},	// '_'
	{0x60,	0x000 | 0xdd},	// '`'
#else
	{0x5b,	0x000 | 0xdb},	// '['
	{0x5c,	0x000 | 0xdc},	// '\'
	{0x5d,	0x000 | 0xdd},	// ']'
	{0x5e,	0x000 | 0xde},	// '^'
	{0x5f,	0x100 | 0xe2},	// '_'
	{0x60,	0x100 | 0xc0},	// '`'
#endif
	{0x61,	0x800 | 0x41},	// 'a'
	{0x62,	0x800 | 0x42},	// 'b'
	{0x63,	0x800 | 0x43},	// 'c'
	{0x64,	0x800 | 0x44},	// 'd'
	{0x65,	0x800 | 0x45},	// 'e'
	{0x66,	0x800 | 0x46},	// 'f'
	{0x67,	0x800 | 0x47},	// 'g'
	{0x68,	0x800 | 0x48},	// 'h'
	{0x69,	0x800 | 0x49},	// 'i'
	{0x6a,	0x800 | 0x4a},	// 'j'
	{0x6b,	0x800 | 0x4b},	// 'k'
	{0x6c,	0x800 | 0x4c},	// 'l'
	{0x6d,	0x800 | 0x4d},	// 'm'
	{0x6e,	0x800 | 0x4e},	// 'n'
	{0x6f,	0x800 | 0x4f},	// 'o'
	{0x70,	0x800 | 0x50},	// 'p'
	{0x71,	0x800 | 0x51},	// 'q'
	{0x72,	0x800 | 0x52},	// 'r'
	{0x73,	0x800 | 0x53},	// 's'
	{0x74,	0x800 | 0x54},	// 't'
	{0x75,	0x800 | 0x55},	// 'u'
	{0x76,	0x800 | 0x56},	// 'v'
	{0x77,	0x800 | 0x57},	// 'w'
	{0x78,	0x800 | 0x58},	// 'x'
	{0x79,	0x800 | 0x59},	// 'y'
	{0x7a,	0x800 | 0x5a},	// 'z'
#ifdef AUTO_KEY_US
	{0x7b,	0x100 | 0xc0},	// '{'
	{0x7c,	0x100 | 0xe2},	// '|'
	{0x7d,	0x100 | 0xdb},	// '}'
	{0x7e,	0x100 | 0xdd},	// '~'
#else
	{0x7b,	0x100 | 0xdb},	// '{'
	{0x7c,	0x100 | 0xdc},	// '|'
	{0x7d,	0x100 | 0xdd},	// '}'
	{0x7e,	0x100 | 0xde},	// '~'
#endif
	{-1, -1},
};

static const int auto_key_table_kana_base[][2] = {
	{0xa1,	0x300 | 0xbe},	// '｡'
	{0xa2,	0x300 | 0xdb},	// '｢'
	{0xa3,	0x300 | 0xdd},	// '｣'
	{0xa4,	0x300 | 0xbc},	// '､'
	{0xa5,	0x300 | 0xbf},	// '･'
	{0xa6,	0x300 | 0x30},	// 'ｦ'
	{0xa7,	0x300 | 0x33},	// 'ｧ'
	{0xa8,	0x300 | 0x45},	// 'ｨ'
	{0xa9,	0x300 | 0x34},	// 'ｩ'
	{0xaa,	0x300 | 0x35},	// 'ｪ'
	{0xab,	0x300 | 0x36},	// 'ｫ'
	{0xac,	0x300 | 0x37},	// 'ｬ'
	{0xad,	0x300 | 0x38},	// 'ｭ'
	{0xae,	0x300 | 0x39},	// 'ｮ'
	{0xaf,	0x300 | 0x5a},	// 'ｯ'
	{0xb0,	0x200 | 0xdc},	// 'ｰ'
	{0xb1,	0x200 | 0x33},	// 'ｱ'
	{0xb2,	0x200 | 0x45},	// 'ｲ'
	{0xb3,	0x200 | 0x34},	// 'ｳ'
	{0xb4,	0x200 | 0x35},	// 'ｴ'
	{0xb5,	0x200 | 0x36},	// 'ｵ'
	{0xb6,	0x200 | 0x54},	// 'ｶ'
	{0xb7,	0x200 | 0x47},	// 'ｷ'
	{0xb8,	0x200 | 0x48},	// 'ｸ'
	{0xb9,	0x200 | 0xba},	// 'ｹ'
	{0xba,	0x200 | 0x42},	// 'ｺ'
	{0xbb,	0x200 | 0x58},	// 'ｻ'
	{0xbc,	0x200 | 0x44},	// 'ｼ'
	{0xbd,	0x200 | 0x52},	// 'ｽ'
	{0xbe,	0x200 | 0x50},	// 'ｾ'
	{0xbf,	0x200 | 0x43},	// 'ｿ'
	{0xc0,	0x200 | 0x51},	// 'ﾀ'
	{0xc1,	0x200 | 0x41},	// 'ﾁ'
	{0xc2,	0x200 | 0x5a},	// 'ﾂ'
	{0xc3,	0x200 | 0x57},	// 'ﾃ'
	{0xc4,	0x200 | 0x53},	// 'ﾄ'
	{0xc5,	0x200 | 0x55},	// 'ﾅ'
	{0xc6,	0x200 | 0x49},	// 'ﾆ'
	{0xc7,	0x200 | 0x31},	// 'ﾇ'
	{0xc8,	0x200 | 0xbc},	// 'ﾈ'
	{0xc9,	0x200 | 0x4b},	// 'ﾉ'
	{0xca,	0x200 | 0x46},	// 'ﾊ'
	{0xcb,	0x200 | 0x56},	// 'ﾋ'
	{0xcc,	0x200 | 0x32},	// 'ﾌ'
	{0xcd,	0x200 | 0xde},	// 'ﾍ'
	{0xce,	0x200 | 0xbd},	// 'ﾎ'
	{0xcf,	0x200 | 0x4a},	// 'ﾏ'
	{0xd0,	0x200 | 0x4e},	// 'ﾐ'
	{0xd1,	0x200 | 0xdd},	// 'ﾑ'
	{0xd2,	0x200 | 0xbf},	// 'ﾒ'
	{0xd3,	0x200 | 0x4d},	// 'ﾓ'
	{0xd4,	0x200 | 0x37},	// 'ﾔ'
	{0xd5,	0x200 | 0x38},	// 'ﾕ'
	{0xd6,	0x200 | 0x39},	// 'ﾖ'
	{0xd7,	0x200 | 0x4f},	// 'ﾗ'
	{0xd8,	0x200 | 0x4c},	// 'ﾘ'
	{0xd9,	0x200 | 0xbe},	// 'ﾙ'
	{0xda,	0x200 | 0xbb},	// 'ﾚ'
	{0xdb,	0x200 | 0xe2},	// 'ﾛ'
	{0xdc,	0x200 | 0x30},	// 'ﾜ'
	{0xdd,	0x200 | 0x59},	// 'ﾝ'
	{0xde,	0x200 | 0xc0},	// 'ﾞ'
	{0xdf,	0x200 | 0xdb},	// 'ﾟ'
	{-1, -1},
};

static const int auto_key_table_50on_base[][2] = {
	{0xa1,	0x300 | 0xbf},	// '｡'
	{0xa2,	0x300 | 0xdb},	// '｢'
	{0xa3,	0x300 | 0xdd},	// '｣'
	{0xa4,	0x300 | 0xbe},	// '､'
	{0xa5,	0x300 | 0xe2},	// '･'
	{0xa6,	0x200 | 0xbf},	// 'ｦ'
	{0xa7,	0x300 | 0x31},	// 'ｧ'
	{0xa8,	0x300 | 0x32},	// 'ｨ'
	{0xa9,	0x300 | 0x33},	// 'ｩ'
	{0xaa,	0x300 | 0x34},	// 'ｪ'
	{0xab,	0x300 | 0x35},	// 'ｫ'
	{0xac,	0x300 | 0x4e},	// 'ｬ'
	{0xad,	0x300 | 0x4d},	// 'ｭ'
	{0xae,	0x300 | 0xbc},	// 'ｮ'
	{0xaf,	0x300 | 0x43},	// 'ｯ'
	{0xb0,	0x300 | 0xba},	// 'ｰ'
	{0xb1,	0x200 | 0x31},	// 'ｱ'
	{0xb2,	0x200 | 0x32},	// 'ｲ'
	{0xb3,	0x200 | 0x33},	// 'ｳ'
	{0xb4,	0x200 | 0x34},	// 'ｴ'
	{0xb5,	0x200 | 0x35},	// 'ｵ'
	{0xb6,	0x200 | 0x51},	// 'ｶ'
	{0xb7,	0x200 | 0x57},	// 'ｷ'
	{0xb8,	0x200 | 0x45},	// 'ｸ'
	{0xb9,	0x200 | 0x52},	// 'ｹ'
	{0xba,	0x200 | 0x54},	// 'ｺ'
	{0xbb,	0x200 | 0x41},	// 'ｻ'
	{0xbc,	0x200 | 0x53},	// 'ｼ'
	{0xbd,	0x200 | 0x44},	// 'ｽ'
	{0xbe,	0x200 | 0x46},	// 'ｾ'
	{0xbf,	0x200 | 0x47},	// 'ｿ'
	{0xc0,	0x200 | 0x5a},	// 'ﾀ'
	{0xc1,	0x200 | 0x58},	// 'ﾁ'
	{0xc2,	0x200 | 0x43},	// 'ﾂ'
	{0xc3,	0x200 | 0x56},	// 'ﾃ'
	{0xc4,	0x200 | 0x42},	// 'ﾄ'
	{0xc5,	0x200 | 0x36},	// 'ﾅ'
	{0xc6,	0x200 | 0x37},	// 'ﾆ'
	{0xc7,	0x200 | 0x38},	// 'ﾇ'
	{0xc8,	0x200 | 0x39},	// 'ﾈ'
	{0xc9,	0x200 | 0x30},	// 'ﾉ'
	{0xca,	0x200 | 0x59},	// 'ﾊ'
	{0xcb,	0x200 | 0x55},	// 'ﾋ'
	{0xcc,	0x200 | 0x49},	// 'ﾌ'
	{0xcd,	0x200 | 0x4f},	// 'ﾍ'
	{0xce,	0x200 | 0x50},	// 'ﾎ'
	{0xcf,	0x200 | 0x48},	// 'ﾏ'
	{0xd0,	0x200 | 0x4a},	// 'ﾐ'
	{0xd1,	0x200 | 0x4b},	// 'ﾑ'
	{0xd2,	0x200 | 0x4c},	// 'ﾒ'
	{0xd3,	0x200 | 0xbb},	// 'ﾓ'
	{0xd4,	0x200 | 0x4e},	// 'ﾔ'
	{0xd5,	0x200 | 0x4d},	// 'ﾕ'
	{0xd6,	0x200 | 0xbc},	// 'ﾖ'
	{0xd7,	0x200 | 0xbd},	// 'ﾗ'
	{0xd8,	0x200 | 0xde},	// 'ﾘ'
	{0xd9,	0x200 | 0xdc},	// 'ﾙ'
	{0xda,	0x200 | 0xc0},	// 'ﾚ'
	{0xdb,	0x200 | 0xdb},	// 'ﾛ'
	{0xdc,	0x200 | 0xbe},	// 'ﾜ'
	{0xdd,	0x200 | 0xe2},	// 'ﾝ'
	{0xde,	0x200 | 0xba},	// 'ﾞ'
	{0xdf,	0x200 | 0xdd},	// 'ﾟ'
	{-1, -1},
};

static const struct {
	const char *romaji;
	const uint8_t kana[4];
} romaji_table[] = {
	{"ltsu",	{0xaf, 0x00}},
	{"xtsu",	{0xaf, 0x00}},
	{"ltu",		{0xaf, 0x00}},
	{"xtu",		{0xaf, 0x00}},
	{"bya",		{0xcb, 0xde, 0xac, 0x00}},
	{"byi",		{0xcb, 0xde, 0xa8, 0x00}},
	{"byu",		{0xcb, 0xde, 0xad, 0x00}},
	{"bye",		{0xcb, 0xde, 0xaa, 0x00}},
	{"byo",		{0xcb, 0xde, 0xae, 0x00}},
	{"cha",		{0xc1, 0xac, 0x00}},
	{"chi",		{0xc1, 0x00}},
	{"chu",		{0xc1, 0xad, 0x00}},
	{"che",		{0xc1, 0xaa, 0x00}},
	{"cho",		{0xc1, 0xae, 0x00}},
	{"cya",		{0xc1, 0xac, 0x00}},
	{"cyi",		{0xc1, 0xa8, 0x00}},
	{"cyu",		{0xc1, 0xad, 0x00}},
	{"cye",		{0xc1, 0xaa, 0x00}},
	{"cyo",		{0xc1, 0xae, 0x00}},
	{"dha",		{0xc3, 0xde, 0xac, 0x00}},
	{"dhi",		{0xc3, 0xde, 0xa8, 0x00}},
	{"dhu",		{0xc3, 0xde, 0xad, 0x00}},
	{"dhe",		{0xc3, 0xde, 0xaa, 0x00}},
	{"dho",		{0xc3, 0xde, 0xae, 0x00}},
	{"dwa",		{0xc4, 0xde, 0xa7, 0x00}},
	{"dwi",		{0xc4, 0xde, 0xa8, 0x00}},
	{"dwu",		{0xc4, 0xde, 0xa9, 0x00}},
	{"dwe",		{0xc4, 0xde, 0xaa, 0x00}},
	{"dwo",		{0xc4, 0xde, 0xab, 0x00}},
	{"dya",		{0xc1, 0xde, 0xac, 0x00}},
	{"dyi",		{0xc1, 0xde, 0xa8, 0x00}},
	{"dyu",		{0xc1, 0xde, 0xad, 0x00}},
	{"dye",		{0xc1, 0xde, 0xaa, 0x00}},
	{"dyo",		{0xc1, 0xde, 0xae, 0x00}},
	{"fwa",		{0xcc, 0xa7, 0x00}},
	{"fwi",		{0xcc, 0xa8, 0x00}},
	{"fwu",		{0xcc, 0xa9, 0x00}},
	{"fwe",		{0xcc, 0xaa, 0x00}},
	{"fwo",		{0xcc, 0xab, 0x00}},
	{"fya",		{0xcc, 0xac, 0x00}},
	{"fyi",		{0xcc, 0xa8, 0x00}},
	{"fyu",		{0xcc, 0xad, 0x00}},
	{"fye",		{0xcc, 0xaa, 0x00}},
	{"fyo",		{0xcc, 0xae, 0x00}},
	{"gwa",		{0xb8, 0xde, 0xa7, 0x00}},
	{"gwi",		{0xb8, 0xde, 0xa8, 0x00}},
	{"gwu",		{0xb8, 0xde, 0xa9, 0x00}},
	{"gwe",		{0xb8, 0xde, 0xaa, 0x00}},
	{"gwo",		{0xb8, 0xde, 0xab, 0x00}},
	{"gya",		{0xb7, 0xde, 0xac, 0x00}},
	{"gyi",		{0xb7, 0xde, 0xa8, 0x00}},
	{"gyu",		{0xb7, 0xde, 0xad, 0x00}},
	{"gye",		{0xb7, 0xde, 0xaa, 0x00}},
	{"gyo",		{0xb7, 0xde, 0xae, 0x00}},
	{"hya",		{0xcb, 0xac, 0x00}},
	{"hyi",		{0xcb, 0xa8, 0x00}},
	{"hyu",		{0xcb, 0xad, 0x00}},
	{"hye",		{0xcb, 0xaa, 0x00}},
	{"hyo",		{0xcb, 0xae, 0x00}},
	{"jya",		{0xbc, 0xde, 0xac, 0x00}},
	{"jyi",		{0xbc, 0xde, 0xa8, 0x00}},
	{"jyu",		{0xbc, 0xde, 0xad, 0x00}},
	{"jye",		{0xbc, 0xde, 0xaa, 0x00}},
	{"jyo",		{0xbc, 0xde, 0xae, 0x00}},
	{"kya",		{0xb7, 0xac, 0x00}},
	{"kyi",		{0xb7, 0xa8, 0x00}},
	{"kyu",		{0xb7, 0xad, 0x00}},
	{"kye",		{0xb7, 0xaa, 0x00}},
	{"kyo",		{0xb7, 0xae, 0x00}},
	{"lya",		{0xac, 0x00}},
	{"lyi",		{0xa8, 0x00}},
	{"lyu",		{0xad, 0x00}},
	{"lye",		{0xaa, 0x00}},
	{"lyo",		{0xae, 0x00}},
	{"mya",		{0xd0, 0xac, 0x00}},
	{"myi",		{0xd0, 0xa8, 0x00}},
	{"myu",		{0xd0, 0xad, 0x00}},
	{"mye",		{0xd0, 0xaa, 0x00}},
	{"myo",		{0xd0, 0xae, 0x00}},
	{"nya",		{0xc6, 0xac, 0x00}},
	{"nyi",		{0xc6, 0xa8, 0x00}},
	{"nyu",		{0xc6, 0xad, 0x00}},
	{"nye",		{0xc6, 0xaa, 0x00}},
	{"nyo",		{0xc6, 0xae, 0x00}},
	{"pya",		{0xcb, 0xdf, 0xac, 0x00}},
	{"pyi",		{0xcb, 0xdf, 0xa8, 0x00}},
	{"pyu",		{0xcb, 0xdf, 0xad, 0x00}},
	{"pye",		{0xcb, 0xdf, 0xaa, 0x00}},
	{"pyo",		{0xcb, 0xdf, 0xae, 0x00}},
	{"qwa",		{0xb8, 0xa7, 0x00}},
	{"qwi",		{0xb8, 0xa8, 0x00}},
	{"qwu",		{0xb8, 0xa9, 0x00}},
	{"qwe",		{0xb8, 0xaa, 0x00}},
	{"qwo",		{0xb8, 0xab, 0x00}},
	{"qya",		{0xb8, 0xac, 0x00}},
	{"qyi",		{0xb8, 0xa8, 0x00}},
	{"qyu",		{0xb8, 0xad, 0x00}},
	{"qye",		{0xb8, 0xaa, 0x00}},
	{"qyo",		{0xb8, 0xae, 0x00}},
	{"rya",		{0xd8, 0xac, 0x00}},
	{"ryi",		{0xd8, 0xa8, 0x00}},
	{"ryu",		{0xd8, 0xad, 0x00}},
	{"rye",		{0xd8, 0xaa, 0x00}},
	{"ryo",		{0xd8, 0xae, 0x00}},
	{"sha",		{0xbc, 0xac, 0x00}},
	{"shi",		{0xbc, 0x00}},
	{"shu",		{0xbc, 0xad, 0x00}},
	{"she",		{0xbc, 0xaa, 0x00}},
	{"sho",		{0xbc, 0xae, 0x00}},
	{"swa",		{0xbd, 0xa7, 0x00}},
	{"swi",		{0xbd, 0xa8, 0x00}},
	{"swu",		{0xbd, 0xa9, 0x00}},
	{"swe",		{0xbd, 0xaa, 0x00}},
	{"swo",		{0xbd, 0xab, 0x00}},
	{"sya",		{0xbc, 0xac, 0x00}},
	{"syi",		{0xbc, 0xa8, 0x00}},
	{"syu",		{0xbc, 0xad, 0x00}},
	{"sye",		{0xbc, 0xaa, 0x00}},
	{"syo",		{0xbc, 0xae, 0x00}},
	{"tha",		{0xc3, 0xac, 0x00}},
	{"thi",		{0xc3, 0xa8, 0x00}},
	{"thu",		{0xc3, 0xad, 0x00}},
	{"the",		{0xc3, 0xaa, 0x00}},
	{"tho",		{0xc3, 0xae, 0x00}},
	{"tsa",		{0xc2, 0xa7, 0x00}},
	{"tsi",		{0xc2, 0xa8, 0x00}},
	{"tsu",		{0xc2, 0x00}},
	{"tse",		{0xc2, 0xaa, 0x00}},
	{"tso",		{0xc2, 0xab, 0x00}},
	{"twa",		{0xc4, 0xa7, 0x00}},
	{"twi",		{0xc4, 0xa8, 0x00}},
	{"twu",		{0xc4, 0xa9, 0x00}},
	{"twe",		{0xc4, 0xaa, 0x00}},
	{"two",		{0xc4, 0xab, 0x00}},
	{"tya",		{0xc1, 0xac, 0x00}},
	{"tyi",		{0xc1, 0xa8, 0x00}},
	{"tyu",		{0xc1, 0xad, 0x00}},
	{"tye",		{0xc1, 0xaa, 0x00}},
	{"tyo",		{0xc1, 0xae, 0x00}},
	{"vya",		{0xb3, 0xde, 0xac, 0x00}},
	{"vyi",		{0xb3, 0xde, 0xa8, 0x00}},
	{"vyu",		{0xb3, 0xde, 0xad, 0x00}},
	{"vye",		{0xb3, 0xde, 0xaa, 0x00}},
	{"vyo",		{0xb3, 0xde, 0xae, 0x00}},
	{"wha",		{0xb3, 0xa7, 0x00}},
	{"whi",		{0xb3, 0xa8, 0x00}},
	{"whu",		{0xb3, 0x00}},
	{"whe",		{0xb3, 0xaa, 0x00}},
	{"who",		{0xb3, 0xab, 0x00}},
	{"xya",		{0xac, 0x00}},
	{"xyi",		{0xa8, 0x00}},
	{"xyu",		{0xad, 0x00}},
	{"xye",		{0xaa, 0x00}},
	{"xyo",		{0xae, 0x00}},
	{"zya",		{0xbc, 0xde, 0xac, 0x00}},
	{"zyi",		{0xbc, 0xde, 0xa8, 0x00}},
	{"zyu",		{0xbc, 0xde, 0xad, 0x00}},
	{"zye",		{0xbc, 0xde, 0xaa, 0x00}},
	{"zyo",		{0xbc, 0xde, 0xae, 0x00}},
	{"ba",		{0xca, 0xde, 0x00}},
	{"bi",		{0xcb, 0xde, 0x00}},
	{"bu",		{0xcc, 0xde, 0x00}},
	{"be",		{0xcd, 0xde, 0x00}},
	{"bo",		{0xce, 0xde, 0x00}},
	{"ca",		{0xb6, 0x00}},
	{"ci",		{0xbc, 0x00}},
	{"cu",		{0xb8, 0x00}},
	{"ce",		{0xbe, 0x00}},
	{"co",		{0xba, 0x00}},
	{"da",		{0xc0, 0xde, 0x00}},
	{"di",		{0xc1, 0xde, 0x00}},
	{"du",		{0xc2, 0xde, 0x00}},
	{"de",		{0xc3, 0xde, 0x00}},
	{"do",		{0xc4, 0xde, 0x00}},
	{"fa",		{0xcc, 0xa7, 0x00}},
	{"fi",		{0xcc, 0xa8, 0x00}},
	{"fu",		{0xcc, 0x00}},
	{"fe",		{0xcc, 0xaa, 0x00}},
	{"fo",		{0xcc, 0xab, 0x00}},
	{"ga",		{0xb6, 0xde, 0x00}},
	{"gi",		{0xb7, 0xde, 0x00}},
	{"gu",		{0xb8, 0xde, 0x00}},
	{"ge",		{0xb9, 0xde, 0x00}},
	{"go",		{0xba, 0xde, 0x00}},
	{"ha",		{0xca, 0x00}},
	{"hi",		{0xcb, 0x00}},
	{"hu",		{0xcc, 0x00}},
	{"he",		{0xcd, 0x00}},
	{"ho",		{0xce, 0x00}},
	{"ja",		{0xbc, 0xde, 0xac, 0x00}},
	{"ji",		{0xbc, 0xde, 0x00}},
	{"ju",		{0xbc, 0xde, 0xad, 0x00}},
	{"je",		{0xbc, 0xde, 0xaa, 0x00}},
	{"jo",		{0xbc, 0xde, 0xae, 0x00}},
	{"ka",		{0xb6, 0x00}},
	{"ki",		{0xb7, 0x00}},
	{"ku",		{0xb8, 0x00}},
	{"ke",		{0xb9, 0x00}},
	{"ko",		{0xba, 0x00}},
	{"la",		{0xa7, 0x00}},
	{"li",		{0xa8, 0x00}},
	{"lu",		{0xa9, 0x00}},
	{"le",		{0xaa, 0x00}},
	{"lo",		{0xab, 0x00}},
	{"ma",		{0xcf, 0x00}},
	{"mi",		{0xd0, 0x00}},
	{"mu",		{0xd1, 0x00}},
	{"me",		{0xd2, 0x00}},
	{"mo",		{0xd3, 0x00}},
	{"na",		{0xc5, 0x00}},
	{"ni",		{0xc6, 0x00}},
	{"nu",		{0xc7, 0x00}},
	{"ne",		{0xc8, 0x00}},
	{"no",		{0xc9, 0x00}},
//	{"nn",		{0xdd, 0x00}},
	{"pa",		{0xca, 0xdf, 0x00}},
	{"pi",		{0xcb, 0xdf, 0x00}},
	{"pu",		{0xcc, 0xdf, 0x00}},
	{"pe",		{0xcd, 0xdf, 0x00}},
	{"po",		{0xce, 0xdf, 0x00}},
	{"qa",		{0xb8, 0xa7, 0x00}},
	{"qi",		{0xb8, 0xa8, 0x00}},
	{"qu",		{0xb8, 0x00}},
	{"qe",		{0xb8, 0xaa, 0x00}},
	{"qo",		{0xb8, 0xab, 0x00}},
	{"ra",		{0xd7, 0x00}},
	{"ri",		{0xd8, 0x00}},
	{"ru",		{0xd9, 0x00}},
	{"re",		{0xda, 0x00}},
	{"ro",		{0xdb, 0x00}},
	{"sa",		{0xbb, 0x00}},
	{"si",		{0xbc, 0x00}},
	{"su",		{0xbd, 0x00}},
	{"se",		{0xbe, 0x00}},
	{"so",		{0xbf, 0x00}},
	{"ta",		{0xc0, 0x00}},
	{"ti",		{0xc1, 0x00}},
	{"tu",		{0xc2, 0x00}},
	{"te",		{0xc3, 0x00}},
	{"to",		{0xc4, 0x00}},
	{"va",		{0xb3, 0xde, 0xa7, 0x00}},
	{"vi",		{0xb3, 0xde, 0xa8, 0x00}},
	{"vu",		{0xb3, 0xde, 0x00}},
	{"ve",		{0xb3, 0xde, 0xaa, 0x00}},
	{"vo",		{0xb3, 0xde, 0xab, 0x00}},
	{"wa",		{0xdc, 0x00}},
	{"wi",		{0xb3, 0xa8, 0x00}},
	{"wu",		{0xb3, 0x00}},
	{"we",		{0xb3, 0xaa, 0x00}},
	{"wo",		{0xa6, 0x00}},
	{"xa",		{0xa7, 0x00}},
	{"xi",		{0xa8, 0x00}},
	{"xu",		{0xa9, 0x00}},
	{"xe",		{0xaa, 0x00}},
	{"xo",		{0xab, 0x00}},
	{"ya",		{0xd4, 0x00}},
	{"yi",		{0xb2, 0x00}},
	{"yu",		{0xd5, 0x00}},
	{"ye",		{0xb2, 0xaa, 0x00}},
	{"yo",		{0xd6, 0x00}},
	{"za",		{0xbb, 0xde, 0x00}},
	{"zi",		{0xbc, 0xde, 0x00}},
	{"zu",		{0xbd, 0xde, 0x00}},
	{"ze",		{0xbe, 0xde, 0x00}},
	{"zo",		{0xbf, 0xde, 0x00}},
	{"a",		{0xb1, 0x00}},
	{"i",		{0xb2, 0x00}},
	{"u",		{0xb3, 0x00}},
	{"e",		{0xb4, 0x00}},
	{"o",		{0xb5, 0x00}},
	{"[",		{0xa2, 0x00}},
	{"]",		{0xa3, 0x00}},
	{"-",		{0xb0, 0x00}},
	{",",		{0xa4, 0x00}},
	{".",		{0xa1, 0x00}},
	{"/",		{0xa5, 0x00}},
	// Pass through kana key codes.
	{"\x0bc",	{0xa4, 0x00}},
	{"\x0bd",   {0xb0, 0x00}},
	{"\x0be",	{0xa1, 0x00}},
	{"\x0bf",	{0xa5, 0x00}},
	{"\x0db",	{0xa2, 0x00}},
	{"\x0dd",	{0xa3, 0x00}},
	{"",		{0x00}},
};

void EMU::initialize_auto_key()
{
	auto_key_buffer = new FIFO(65536);
	auto_key_buffer->clear();
	auto_key_phase = auto_key_shift = 0;
	shift_pressed = false;
	osd->now_auto_key = false;
}

void EMU::release_auto_key()
{
	if(auto_key_buffer) {
		auto_key_buffer->release();
		delete auto_key_buffer;
	}
}

int EMU::get_auto_key_code(int code)
{
	static int auto_key_table[256];
	static bool initialized = false;
#ifdef USE_KEYBOARD_TYPE
	static int keyboard_type = -1;

	if(keyboard_type != config.keyboard_type) {
		initialized = false;
		keyboard_type = config.keyboard_type;
	}
#endif
	if(!initialized) {
		memset(auto_key_table, 0, sizeof(auto_key_table));
		for(int i = 0;; i++) {
			if(auto_key_table_base[i][0] == -1) {
				break;
			}
			auto_key_table[auto_key_table_base[i][0]] = auto_key_table_base[i][1];
		}
#if defined(_X1TURBO) || defined(_X1TURBOZ)
		// FIXME
		if(config.keyboard_type) {
			for(int i = 0;; i++) {
				if(auto_key_table_50on_base[i][0] == -1) {
					break;
				}
				auto_key_table[auto_key_table_50on_base[i][0]] = auto_key_table_50on_base[i][1];
			}
		} else
#endif
		for(int i = 0;; i++) {
			if(auto_key_table_kana_base[i][0] == -1) {
				break;
			}
			auto_key_table[auto_key_table_kana_base[i][0]] = auto_key_table_kana_base[i][1];
		}
#ifdef USE_VM_AUTO_KEY_TABLE
		for(int i = 0;; i++) {
			if(vm_auto_key_table_base[i][0] == -1) {
				break;
			}
			auto_key_table[vm_auto_key_table_base[i][0]] = vm_auto_key_table_base[i][1];
		}
#endif
		initialized = true;
	}
	return auto_key_table[code];
}

void EMU::set_auto_key_code(int code)
{
	if(code == 0xf2 || (code = get_auto_key_code(code)) != 0) {
		if(code == 0x08 || code == 0x09 || code == 0x0d || code == 0x1b || code == 0x20 || code == 0xf2) {
			// VK_BACK, VK_TAB, VK_RETURN, VK_ESCAPE, VK_SPACE, VK_OEM_COPY(Katakana/Hiragana)
			auto_key_buffer->write(code);
#ifdef USE_AUTO_KEY_NUMPAD
		} else if(code >= 0x30 && code <= 0x39) {
			// numpad
			auto_key_buffer->write(code - 0x30 + 0x60);
#endif
		} else if(code & 0x200) {
			// kana
			auto_key_buffer->write(code & 0x1ff);
		} else {
			// ank other than alphabet and kana
			auto_key_buffer->write(0xf2); // kana unlock
			auto_key_buffer->write(code & 0x1ff);
			auto_key_buffer->write(0xf2); // kana lock
		}
		if(!is_auto_key_running()) {
			start_auto_key();
		}
	}
}

void EMU::set_auto_key_list(char *buf, int size)
{
#if defined(USE_KEY_LOCKED)
	bool prev_caps = get_caps_locked();
	bool prev_kana = get_kana_locked();
#else
	bool prev_caps = false;
	bool prev_kana = false;
#endif
	auto_key_buffer->clear();

	for(int i = 0; i < size; i++) {
		int code = buf[i] & 0xff;
		if((0x81 <= code && code <= 0x9f) || 0xe0 <= code) {
			i++;	// kanji ?
			continue;
		} else if(code == 0x0a) {
			continue;	// cr-lf
		}
		if((code = get_auto_key_code(code)) != 0) {
			// kana lock
			bool kana = ((code & 0x200) != 0);
			if(prev_kana != kana) {
				auto_key_buffer->write(0xf2);
			}
			prev_kana = kana;
			// check handakuon
			if(i < (size - 1)) {
				bool dakuon_lock = false;
				bool handakuon_lock = false;
#if defined(USE_TWO_STROKE_AUTOKEY_HANDAKUON)
				if((buf[i + 1] & 0xff) == 0xdf) { // Is Handakuon
					for(int jj = 0; ; jj++) {
						if(kana_handakuon_keyboard_table[jj][0] == -1) break;
						if(kana_handakuon_keyboard_table[jj][0] == (buf[i] & 0xff)) { // Found
							handakuon_lock = true;
							for(int l = 1; l < 6; l++) {
								int n_code = kana_handakuon_keyboard_table[jj][l];
								if(n_code == 0x00) break;
								if(n_code & (0x100 | 0x400 | 0x800)) {
									auto_key_buffer->write((n_code & 0x30ff)| 0x100);
								} else {
									auto_key_buffer->write(n_code & 0x30ff);
								}
							}
							break;
						}
					}
					if(handakuon_lock) {
						i++;
						continue;
					}
				}
#endif
#if defined(USE_TWO_STROKE_AUTOKEY_DAKUON)
				if((buf[i + 1] & 0xff) == 0xde) { // Is Dakuon
					for(int jj = 0; ; jj++) {
						if(kana_dakuon_keyboard_table[jj][0] == -1) break;
						if(kana_dakuon_keyboard_table[jj][0] == (buf[i] & 0xff)) { // Found
							dakuon_lock = true;
							for(int l = 1; l < 6; l++) {
								int n_code = kana_dakuon_keyboard_table[jj][l];
								if(n_code == 0x00) break;
								if(n_code & (0x100 | 0x400 | 0x800)) {
									auto_key_buffer->write((n_code & 0x30ff)| 0x100);
								} else {
									auto_key_buffer->write(n_code & 0x30ff);
								}
							}
							break;
						}
					}
					if(dakuon_lock) {
						i++;
						continue;
					}
				}
#endif
			}
#if defined(USE_AUTO_KEY_CAPS_LOCK)
			// use caps lock key to switch uppercase/lowercase of alphabet
			// USE_AUTO_KEY_CAPS_LOCK shows the caps lock key code
			bool caps = ((code & 0x400) != 0);
			if(prev_caps != caps) {
				auto_key_buffer->write(USE_AUTO_KEY_CAPS_LOCK);
			}
			prev_caps = caps;
#endif
#if defined(USE_AUTO_KEY_CAPS_LOCK) || defined(USE_AUTO_KEY_NO_CAPS)
			code &= ~(0x400 | 0x800); // don't press shift key for both alphabet and ALPHABET
#elif defined(USE_KEY_LOCKED)
			if(get_caps_locked()) {
				code &= ~0x400; // don't press shift key for ALPHABET
			} else {
				code &= ~0x800; // don't press shift key for alphabet
			}
#elif defined(USE_AUTO_KEY_CAPS)
			code &= ~0x400; // don't press shift key for ALPHABET
#else
			code &= ~0x800; // don't press shift key for alphabet
#endif
			if(code & (0x100 | 0x400 | 0x800)) {
				auto_key_buffer->write((code & 0xff) | 0x100);
			} else {
				auto_key_buffer->write(code & 0xff);
			}
		}
	}
	// release kana lock
	if(prev_kana) {
		auto_key_buffer->write(0xf2);
	}
#if defined(USE_AUTO_KEY_CAPS_LOCK)
	// release caps lock
	if(prev_caps) {
		auto_key_buffer->write(USE_AUTO_KEY_CAPS_LOCK);
	}
#endif
}

bool is_alphabet(char code)
{
	return (code >= 'a' && code <= 'z');
}

bool is_vowel(char code)
{
	return (code == 'a' || code == 'i' || code == 'u' || code == 'e' || code == 'o');
}

bool is_consonant(char code)
{
	return (is_alphabet(code) && !is_vowel(code));
}

void EMU::set_auto_key_char(char code)
{
	static char codes[5] = {0};
	if(code == 1) {
		// start
#ifdef USE_KEY_LOCKED
		if(!get_kana_locked())
#endif
		set_auto_key_code(0xf2);
		memset(codes, 0, sizeof(codes));
	} else if(code == 0) {
		// end
		if(codes[3] == 'n') {
			set_auto_key_code(0xdd); // 'ﾝ'
		}
		set_auto_key_code(0xf2);
		memset(codes, 0, sizeof(codes));
	} else if(code == 0x08 || code == 0x09 || code == 0x0d || code == 0x1b || code == 0x20) {
		if(codes[3] == 'n') {
			set_auto_key_code(0xdd); // 'ﾝ'
		}
		set_auto_key_code(code);
		memset(codes, 0, sizeof(codes));
#ifdef USE_AUTO_KEY_NUMPAD
	} else if(code >= 0x30 && code <= 0x39) {
		if(codes[3] == 'n') {
			set_auto_key_code(0xdd); // 'ﾝ'
		}
		set_auto_key_code(code);
		memset(codes, 0, sizeof(codes));
#endif
	} else {
		codes[0] = codes[1];
		codes[1] = codes[2];
		codes[2] = codes[3];
		codes[3] = (code >= 'A' && code <= 'Z') ? ('a' + (code - 'A')) : code & 0xff;
		codes[4] = '\0';

		if(codes[2] == 'n' && !is_vowel(codes[3])) {
			set_auto_key_code(0xdd); // 'ﾝ'
			if(codes[3] == 'n') {
				memset(codes, 0, sizeof(codes));
				return;
			}
		} else if(codes[2] == codes[3] && is_consonant(codes[3])) {
			set_auto_key_code(0xaf); // 'ｯ'
			return;
		}
		for(int i = 0;; i++) {
			size_t len = strlen(romaji_table[i].romaji);
			int comp = -1;
			if(len == 0) {
				// end of table
				if(!(is_alphabet(codes[3])) /*&& !((codes[3] >= 0x2c) && (codes[3] <= 0x2e)) && !((codes[3] == 0x5b) || (codes[3] == 0x5d))*/) {
					set_auto_key_code(codes[3]);
					memset(codes, 0, sizeof(codes));
				}
				break;
			} else if(len == 1) {
				comp = strcmp(romaji_table[i].romaji, &codes[3]);
			} else if(len == 2) {
				comp = strcmp(romaji_table[i].romaji, &codes[2]);
			} else if(len == 3) {
				comp = strcmp(romaji_table[i].romaji, &codes[1]);
			} else if(len == 4) {
				comp = strcmp(romaji_table[i].romaji, &codes[0]);
			}
			if(comp == 0) {
				for(int j = 0; j < 4; j++) {
					if(!romaji_table[i].kana[j]) {
						break;
					}
					if(j == 0) {
						bool handakuon_found = false;
						bool dakuon_found = false;
#if defined(USE_TWO_STROKE_AUTOKEY_HANDAKUON)
						if(romaji_table[i].kana[1] == 0xdf) {
							// HANDAKUON
							for(int jj = 0;;jj++) {
								if(kana_handakuon_keyboard_table[jj][0] == -1) break;
								if(kana_handakuon_keyboard_table[jj][0] == romaji_table[i].kana[0]) {
									for(int l = 1; l < 6; l++) {
										if(kana_handakuon_keyboard_table[jj][l] == 0x00) break;
										auto_key_buffer->write(kana_handakuon_keyboard_table[jj][l] & 0x31ff);
										if(!is_auto_key_running()) {
											start_auto_key();
										}
									}
									handakuon_found = true;
									j += 1;
									break;
								}
							}
						}
#endif
#if defined(USE_TWO_STROKE_AUTOKEY_DAKUON)
						if(romaji_table[i].kana[1] == 0xde) {
							// DAKUON
							for(int jj = 0;;jj++) {
								if(kana_dakuon_keyboard_table[jj][0] == -1) break;
								if(kana_dakuon_keyboard_table[jj][0] == romaji_table[i].kana[0]) {
									for(int l = 1; l < 6; l++) {
										if(kana_dakuon_keyboard_table[jj][l] == 0x00) break;
										auto_key_buffer->write(kana_dakuon_keyboard_table[jj][l] & 0x31ff);
									}
									if(!is_auto_key_running()) {
										start_auto_key();
									}
									j += 1;
									dakuon_found = true;
									break;
								}
							}
						}
#endif
						if((handakuon_found) || (dakuon_found)) {
							continue;
						}
					}
					set_auto_key_code(romaji_table[i].kana[j]);
				}
				memset(codes, 0, sizeof(codes));
				break;
			}
		}
	}
}

void EMU::start_auto_key()
{
	auto_key_phase = 1;
	auto_key_shift = 0;
	osd->now_auto_key = true;
}

void EMU::stop_auto_key()
{
	if(auto_key_shift) {
		osd->key_up_native(VK_LSHIFT);
	}
	auto_key_phase = auto_key_shift = 0;
	osd->now_auto_key = false;
}

#ifndef USE_AUTO_KEY_SHIFT
#define USE_AUTO_KEY_SHIFT 0
#endif
#ifndef VK_LSHIFT
#define VK_LSHIFT 0xA0
#endif

void EMU::update_auto_key()
{
	switch(auto_key_phase) {
	case 1:
		if(auto_key_buffer && !auto_key_buffer->empty()) {
			// update shift key status
			int shift = auto_key_buffer->read_not_remove(0) & 0x100;
			if(shift && !auto_key_shift) {
				osd->key_down_native(VK_LSHIFT, false);
			} else if(!shift && auto_key_shift) {
				osd->key_up_native(VK_LSHIFT);
			}
			auto_key_shift = shift;
			auto_key_phase++;
			break;
		}
	case 3 + USE_AUTO_KEY_SHIFT:
		if(auto_key_buffer && !auto_key_buffer->empty()) {
			if(!(auto_key_buffer->read_not_remove(0) & 0x2000)) {
				osd->key_down_native(auto_key_buffer->read_not_remove(0) & 0xff, false);
//				printf("Press key: %04X\n", auto_key_buffer->read_not_remove(0));
			}
		}
		auto_key_phase++;
		break;
	case USE_AUTO_KEY + USE_AUTO_KEY_SHIFT:
		if(auto_key_buffer && !auto_key_buffer->empty()) {
			if(!(auto_key_buffer->read_not_remove(0) & 0x1000)) {
				osd->key_up_native(auto_key_buffer->read_not_remove(0) & 0xff);
//				printf("Release key: %04X\n", auto_key_buffer->read_not_remove(0));
			}
		}
		auto_key_phase++;
		break;
	case USE_AUTO_KEY_RELEASE + USE_AUTO_KEY_SHIFT:
		if(auto_key_buffer && !auto_key_buffer->empty()) {
			// wait enough while vm analyzes one line
			if(auto_key_buffer->read() == 0xd) {
				auto_key_phase++;
				break;
			}
		}
	case 30:
		if(auto_key_buffer && !auto_key_buffer->empty()) {
			auto_key_phase = 1;
		} else {
			stop_auto_key();
		}
		break;
	default:
		if(auto_key_phase) {
			auto_key_phase++;
		}
	}
}
#endif

#ifdef USE_JOYSTICK
void EMU::update_joystick()
{
	uint8_t *key_buffer = osd->get_key_buffer();

	uint32_t *joyp = osd->get_joy_buffer();
	uint32_t joy_buffer[4];
	memset(joy_status, 0, sizeof(joy_status));
	for(int i = 0; i < 4; i++) {
		joy_buffer[i] = joyp[i];
	}
	osd->release_joy_buffer(joyp);

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 16; j++) {
			if(config.joy_buttons[i][j] < 0) {
				int code = -config.joy_buttons[i][j];
				if(code < 256 && key_buffer[code]) {
					joy_status[i] |= (1 << j);
					//printf("%d %d %02x %02x\n", i, j, config.joy_buttons[i][j], joy_status[i]);
				}
			} else {
				int stick = config.joy_buttons[i][j] >> 5;
				int button = config.joy_buttons[i][j] & 0x1f;
				if(stick < 4 && (joy_buffer[stick & 3] & (1 << button))) {
					joy_status[i] |= (1 << j);
					//printf("%d %d %02x %02x\n", i, j, config.joy_buttons[i][j], joy_status[i]);
				}
			}
		}
	}
}
#endif

const uint8_t* EMU::get_key_buffer()
{
	return (const uint8_t*)osd->get_key_buffer();
}

#ifdef USE_JOYSTICK
const uint32_t* EMU::get_joy_buffer()
{
	// Update joystick data per query joystick buffer.
	update_joystick();
	return (const uint32_t*)joy_status;
}
void EMU::release_joy_buffer(const uint32_t* ptr)
{
	// ToDo: Unlock buffer.
}
#endif

#ifdef USE_MOUSE
const int32_t* EMU::get_mouse_buffer()
{
	return (const int32_t*)osd->get_mouse_buffer();
}
void EMU::release_mouse_buffer(const int32_t* ptr)
{
	// ToDo: Unlock buffer.
	osd->release_mouse_buffer((int32_t*)ptr);
}
const int32_t EMU::get_mouse_button()
{
	return (const int32_t)osd->get_mouse_button();
}

#endif

// ----------------------------------------------------------------------------
// screen
// ----------------------------------------------------------------------------

double EMU::get_window_mode_power(int mode)
{
	return osd->get_window_mode_power(mode);
}

int EMU::get_window_mode_width(int mode)
{
	return osd->get_window_mode_width(mode);
}

int EMU::get_window_mode_height(int mode)
{
	return osd->get_window_mode_height(mode);
}

void EMU::set_host_window_size(int window_width, int window_height, bool window_mode)
{
	osd->set_host_window_size(window_width, window_height, window_mode);
}

void EMU::set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect)
{
	osd->set_vm_screen_size(screen_width, screen_height, window_width, window_height, window_width_aspect, window_height_aspect);
}

void EMU::set_vm_screen_lines(int lines)
{
	osd->set_vm_screen_lines(lines);
}


int EMU::get_vm_window_width()
{
	return osd->get_vm_window_width();
}

int EMU::get_vm_window_height()
{
	return osd->get_vm_window_height();
}

int EMU::get_vm_window_width_aspect()
{
	return osd->get_vm_window_width_aspect();
}

int EMU::get_vm_window_height_aspect()
{
	return osd->get_vm_window_height_aspect();
}

#if defined(USE_MINIMUM_RENDERING)
bool EMU::is_screen_changed()
{
	__LIKELY_IF(vm != NULL) {
		return vm->is_screen_changed();
	}
	return true; // DUMMY
}
#endif

int EMU::draw_screen()
{
#ifdef ONE_BOARD_MICRO_COMPUTER
	if(now_waiting_in_debugger) {
		osd->reload_bitmap();
	}
#endif
	return osd->draw_screen();
}

scrntype_t* EMU::get_screen_buffer(int y)
{
	return osd->get_vm_screen_buffer(y);
}

#ifdef USE_SCREEN_FILTER
void EMU::screen_skip_line(bool skip_line)
{
	osd->screen_skip_line = skip_line;
}
#endif

#ifdef ONE_BOARD_MICRO_COMPUTER
void EMU::get_invalidated_rect(int *left, int *top, int *right, int *bottom)
{

#ifdef MAX_DRAW_RANGES
	for(int i = 0; i < MAX_DRAW_RANGES; i++) {
#else
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	for(int i = 0; i < vm->max_draw_ranges(); i++) { // for TK-80BS
#endif
		int x1 = vm_ranges[i].x;
		int y1 = vm_ranges[i].y;
		int x2 = x1 + vm_ranges[i].width;
		int y2 = y1 + vm_ranges[i].height;

		*left   = (i == 0) ? x1 : min(x1, *left  );
		*top    = (i == 0) ? y1 : min(y1, *top   );
		*right  = (i == 0) ? x2 : max(x2, *right );
		*bottom = (i == 0) ? y2 : max(y2, *bottom);
	}
}

void EMU::reload_bitmap()
{
	osd->reload_bitmap();
}
#endif

#ifdef OSD_WIN32
void EMU::invalidate_screen()
{
	osd->invalidate_screen();
}

void EMU::update_screen(HDC hdc)
{
	osd->update_screen(hdc);
}
#endif

void EMU::capture_screen()
{
	osd->capture_screen();
}

bool EMU::start_record_video(int fps)
{
	return osd->start_record_video(fps);
}

void EMU::stop_record_video()
{
	osd->stop_record_video();
}

bool EMU::is_video_recording()
{
	return osd->now_record_video;
}

// ----------------------------------------------------------------------------
// sound
// ----------------------------------------------------------------------------

void EMU::mute_sound()
{
	osd->mute_sound();
}

void EMU::start_record_sound()
{
	osd->start_record_sound();
}

void EMU::stop_record_sound()
{
	osd->stop_record_sound();
}

bool EMU::is_sound_recording()
{
	return osd->now_record_sound;
}

// ----------------------------------------------------------------------------
// video
// ----------------------------------------------------------------------------

#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
void EMU::get_video_buffer()
{
	osd->get_video_buffer();
}

void EMU::mute_video_dev(bool l, bool r)
{
	osd->mute_video_dev(l, r);
}
#endif

#ifdef USE_MOVIE_PLAYER
bool EMU::open_movie_file(const _TCHAR* file_path)
{
	return osd->open_movie_file(file_path);
}

void EMU::close_movie_file()
{
	osd->close_movie_file();
}

void EMU::play_movie()
{
	osd->play_movie();
}

void EMU::stop_movie()
{
	osd->stop_movie();
}

void EMU::pause_movie()
{
	osd->pause_movie();
}

double EMU::get_movie_frame_rate()
{
	return osd->get_movie_frame_rate();
}

int EMU::get_movie_sound_rate()
{
	return osd->get_movie_sound_rate();
}

void EMU::set_cur_movie_frame(int frame, bool relative)
{
	osd->set_cur_movie_frame(frame, relative);
}

uint32_t EMU::get_cur_movie_frame()
{
	return osd->get_cur_movie_frame();
}
#endif

#ifdef USE_VIDEO_CAPTURE
int EMU::get_cur_capture_dev_index()
{
	return osd->get_cur_capture_dev_index();
}

int EMU::get_num_capture_devs()
{
	return osd->get_num_capture_devs();
}

_TCHAR* EMU::get_capture_dev_name(int index)
{
	return osd->get_capture_dev_name(index);
}

void EMU::open_capture_dev(int index, bool pin)
{
	osd->open_capture_dev(index, pin);
}

void EMU::close_capture_dev()
{
	osd->close_capture_dev();
}

void EMU::show_capture_dev_filter()
{
	osd->show_capture_dev_filter();
}

void EMU::show_capture_dev_pin()
{
	osd->show_capture_dev_pin();
}

void EMU::show_capture_dev_source()
{
	osd->show_capture_dev_source();
}

void EMU::set_capture_dev_channel(int ch)
{
	osd->set_capture_dev_channel(ch);
}
#endif

// ----------------------------------------------------------------------------
// printer
// ----------------------------------------------------------------------------

#ifdef USE_PRINTER
void EMU::create_bitmap(bitmap_t *bitmap, int width, int height)
{
	osd->create_bitmap(bitmap, width, height);
}

void EMU::release_bitmap(bitmap_t *bitmap)
{
	osd->release_bitmap(bitmap);
}

void EMU::create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic)
{
	osd->create_font(font, family, width, height, rotate, bold, italic);
}

void EMU::release_font(font_t *font)
{
	osd->release_font(font);
}

void EMU::create_pen(pen_t *pen, int width, uint8_t r, uint8_t g, uint8_t b)
{
	osd->create_pen(pen, width, r, g, b);
}

void EMU::release_pen(pen_t *pen)
{
	osd->release_pen(pen);
}

void EMU::clear_bitmap(bitmap_t *bitmap, uint8_t r, uint8_t g, uint8_t b)
{
	osd->clear_bitmap(bitmap, r, g, b);
}

int EMU::get_text_width(bitmap_t *bitmap, font_t *font, const char *text)
{
	return osd->get_text_width(bitmap, font, text);
}

void EMU::draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b)
{
	osd->draw_text_to_bitmap(bitmap, font, x, y, text, r, g, b);
}

void EMU::draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey)
{
	osd->draw_line_to_bitmap(bitmap, pen, sx, sy, ex, ey);
}

void EMU::draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b)
{
	osd->draw_rectangle_to_bitmap(bitmap, x, y, width, height, r, g, b);
}

void EMU::draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	osd->draw_point_to_bitmap(bitmap, x, y, r, g, b);
}

void EMU::stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t *source, int source_x, int source_y, int source_width, int source_height)
{
	osd->stretch_bitmap(dest, dest_x, dest_y, dest_width, dest_height, source, source_x, source_y, source_width, source_height);
}

void EMU::write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path)
{
	osd->write_bitmap_to_file(bitmap, file_path);
}
#endif

// ----------------------------------------------------------------------------
// socket
// ----------------------------------------------------------------------------

#ifdef USE_SOCKET
SOCKET EMU::get_socket(int ch)
{
	return osd->get_socket(ch);
}

void EMU::notify_socket_connected(int ch)
{
	osd->notify_socket_connected(ch);
}

void EMU::notify_socket_disconnected(int ch)
{
	osd->notify_socket_disconnected(ch);
}

bool EMU::initialize_socket_tcp(int ch)
{
	return osd->initialize_socket_tcp(ch);
}

bool EMU::initialize_socket_udp(int ch)
{
	return osd->initialize_socket_udp(ch);
}

bool EMU::connect_socket(int ch, uint32_t ipaddr, int port)
{
	return osd->connect_socket(ch, ipaddr, port);
}

void EMU::disconnect_socket(int ch)
{
	osd->disconnect_socket(ch);
}

bool EMU::listen_socket(int ch)
{
	return osd->listen_socket(ch);
}

void EMU::send_socket_data_tcp(int ch)
{
	osd->send_socket_data_tcp(ch);
}

void EMU::send_socket_data_udp(int ch, uint32_t ipaddr, int port)
{
	osd->send_socket_data_udp(ch, ipaddr, port);
}

void EMU::send_socket_data(int ch)
{
	osd->send_socket_data(ch);
}

void EMU::recv_socket_data(int ch)
{
	osd->recv_socket_data(ch);
}
#endif

// ----------------------------------------------------------------------------
// MIDI
// ----------------------------------------------------------------------------

#ifdef USE_MIDI
void EMU::send_to_midi(uint8_t data, int ch, double timestamp_usec)
{
	osd->send_to_midi(data, ch, timestamp_usec);
}

bool EMU::recv_from_midi(uint8_t *data, int ch, double timestamp_usec)
{
	return osd->recv_from_midi(data, ch, timestamp_usec);
}
// ToDo: Will implement timeout function.
bool EMU::send_to_midi_timeout(uint8_t data, int ch, uint64_t timeout_ms, double timestamp_usec)
{
	// ToDo: will implement handshake.
	// Q: Need to check VM's timeout status?
	return osd->send_to_midi_timeout(data, ch, timeout_ms, timestamp_usec);
}

bool EMU::recv_from_midi_timeout(uint8_t* data, int ch, uint64_t timeout_ms, double timestamp_usec)
{
	// ToDo: will implement handshake.
	// Q: Need to check VM's timeout status?
	return osd->recv_from_midi_timeout(data, ch, timeout_ms, timestamp_usec);
}

//void EMU::notify_timeout_sending_to_midi(int ch)
//{
//	vm->notify_timeout_sending_to_midi(ch);
//}
//void EMU::notify_timeout_receiving_from_midi(int ch)
//{
//	vm->notify_timeout_receiving_from_midi(ch);
//}

void EMU::reset_to_midi(int ch, double timestamp_usec)
{
	osd->reset_to_midi(ch, timestamp_usec);
}

void EMU::initialize_midi_device(bool handshake_from_midi, bool handshake_to_midi, int ch)
{
	osd->initialize_midi_device(handshake_from_midi, handshake_to_midi, ch);
}

//void EMU::ready_receive_from_midi(int ch, double timestamp_usec)
//{
//	vm->ready_receive_from_midi(ch, timestamp_usec);
//}

void EMU::ready_send_to_midi(int ch, double timestamp_usec)
{
	osd->ready_send_to_midi(ch,timestamp_usec);
}

void EMU::request_stop_to_receive_from_midi(int ch, double timestamp_usec)
{
	osd->request_stop_to_receive_from_midi(ch, timestamp_usec);
}

//void EMU::request_stop_to_send_to_midi(int ch, double timestamp_usec)
//{
//	vm->request_stop_to_send_to_midi(ch, timestamp_usec);
//}


#endif
// ---------------------------------------------------------------------------
// debugger (some functions needed by libCSPcommon_vm 20190221 K.O)
// ---------------------------------------------------------------------------

void EMU::start_waiting_in_debugger()
{
#ifdef USE_DEBUGGER
	now_waiting_in_debugger = true;
#endif
	osd->mute_sound();
#ifdef USE_DEBUGGER
	osd->start_waiting_in_debugger();
#endif
}

void EMU::finish_waiting_in_debugger()
{
#ifdef USE_DEBUGGER
	osd->finish_waiting_in_debugger();
	now_waiting_in_debugger = false;
	osd->unmute_sound();
#endif
}

void EMU::process_waiting_in_debugger()
{
#ifdef USE_DEBUGGER
	osd->process_waiting_in_debugger();
#else
	osd->sleep(10);
#endif
}

// ----------------------------------------------------------------------------
// debug log
// ----------------------------------------------------------------------------

#ifdef _DEBUG_LOG
void EMU::initialize_debug_log()
{
	_TCHAR path[_MAX_PATH];
	debug_log = _tfopen(create_date_file_path(_T("log")), _T("w"));
}

void EMU::release_debug_log()
{
	if(debug_log) {
		fclose(debug_log);
		debug_log = NULL;
	}
}
#endif

#ifdef _DEBUG_LOG
static _TCHAR prev_buffer[2048] = {0};
#endif

void EMU::out_debug_log(const _TCHAR* format, ...)
{
	common_initialize();

#ifdef _DEBUG_LOG
	va_list ap;
	_TCHAR buffer[2048];

	va_start(ap, format);
	my_vstprintf_s(buffer, 2048, format, ap);
	va_end(ap);

	if(_tcscmp(prev_buffer, buffer) == 0) {
		return;
	}
	my_tcscpy_s(prev_buffer, 2048, buffer);

#if defined(_USE_QT) || defined(_USE_AGAR) || defined(_USE_SDL)
	std::shared_ptr<CSP_Logger> lp = csp_logger;
	lp->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_EMU, "%s", buffer);
#else
	if(debug_log) {
		__UNLIKELY_IF(vm == NULL) {
			_ftprintf(debug_log, _T("(unknown uS) %s"), buffer);
		} else {
			_ftprintf(debug_log, _T("(%f uS) %s"), vm->get_current_usec(), buffer);
		}
		static int size = 0;
		if((size += _tcslen(buffer)) > 0x8000000) { // 128MB
			fclose(debug_log);
			debug_log = _tfopen(create_date_file_path(_T("log")), _T("w"));
			size = 0;
		}
	}
#endif
#endif
}

void EMU::force_out_debug_log(const _TCHAR* format, ...)
{
#ifdef _DEBUG_LOG
	va_list ap;
	_TCHAR buffer[1024];

	va_start(ap, format);
	my_vstprintf_s(buffer, 1024, format, ap);
	va_end(ap);
	my_tcscpy_s(prev_buffer, 1024, buffer);

#if defined(_USE_QT) || defined(_USE_AGAR) || defined(_USE_SDL)
	std::shared_ptr<CSP_Logger> lp = csp_logger;
    lp->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_EMU, "%s", buffer);
#else
	if(debug_log) {
		_ftprintf(debug_log, _T("%s"), buffer);
		static int size = 0;
		if((size += _tcslen(buffer)) > 0x8000000) { // 128MB
			fclose(debug_log);
			debug_log = _tfopen(create_date_file_path(_T("log")), _T("w"));
			size = 0;
		}
	}
#endif
#endif
}

void EMU::out_message(const _TCHAR* format, ...)
{
//#if defined(_USE_QT)
//	_TCHAR mes_buf[1024];
//#endif
	va_list ap;
	va_start(ap, format);
	my_vstprintf_s(message, 1024, format, ap); // Security for MSVC:C6386.
//#if defined(_USE_QT)
//	memset(mes_buf, 0x00, sizeof(mes_buf));
//	my_vstprintf_s(mes_buf, 1024, format, ap); // Security for MSVC:C6386.
//	csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_EMU, "%s", mes_buf);
//#endif
	va_end(ap);
	message_count = 4; // 4sec
}

// ----------------------------------------------------------------------------
// misc
// ----------------------------------------------------------------------------


void EMU::sleep(uint32_t ms)
{
	osd->sleep(ms);
}


// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

static uint8_t hex2uint8(char *value)
{
	char tmp[3];
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, value, 2);
	return (uint8_t)strtoul(tmp, NULL, 16);
}

static uint16_t hex2uint16(char *value)
{
	char tmp[5];
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, value, 4);
	return (uint16_t)strtoul(tmp, NULL, 16);
}

static bool hex2bin(const _TCHAR* file_path, const _TCHAR* dest_path)
{
	bool result = false;
	FILEIO *fio_s = new FILEIO();
	if(fio_s->Fopen(file_path, FILEIO_READ_BINARY)) {
		int length = 0;
		char line[1024];
		uint8_t buffer[0x10000];
		memset(buffer, 0xff, sizeof(buffer));
		while(fio_s->Fgets(line, sizeof(line)) != NULL) {
			if(line[0] != ':') continue;
			int bytes = hex2uint8(line + 1);
			int offset = hex2uint16(line + 3);
			uint8_t record_type = hex2uint8(line + 7);
			if(record_type == 0x01) break;
			if(record_type != 0x00) continue;
			for(int i = 0; i < bytes; i++) {
				if((offset + i) < (int)sizeof(buffer)) {
					if(length < (offset + i)) {
						length = offset + i;
					}
					buffer[offset + i] = hex2uint8(line + 9 + 2 * i);
				}
			}
		}
		if(length > 0) {
			FILEIO *fio_d = new FILEIO();
			if(fio_d->Fopen(dest_path, FILEIO_WRITE_BINARY)) {
				fio_d->Fwrite(buffer, length, 1);
				fio_d->Fclose();
				result = true;
			}
			delete fio_d;
		}
		fio_s->Fclose();
	}
	delete fio_s;
	return result;
}

void EMU::initialize_media()
{
#ifdef USE_CART
	memset(cart_status, 0, sizeof(cart_status));
#endif
#ifdef USE_FLOPPY_DISK
	memset(floppy_disk_status, 0, sizeof(floppy_disk_status));
#endif
#ifdef USE_QUICK_DISK
	memset(quick_disk_status, 0, sizeof(quick_disk_status));
#endif
#ifdef USE_HARD_DISK
	memset(hard_disk_status, 0, sizeof(hard_disk_status));
#endif
#ifdef USE_TAPE
	memset(tape_status, 0, sizeof(tape_status));
#endif
#ifdef USE_COMPACT_DISC
	memset(compact_disc_status, 0, sizeof(compact_disc_status));
#endif
#ifdef USE_LASER_DISC
	memset(laser_disc_status, 0, sizeof(laser_disc_status));
#endif
#ifdef USE_BUBBLE
	memset(bubble_casette_status, 0, sizeof(bubble_casette_status));
#endif
}

#if defined(_USE_QT)
extern void DLL_PREFIX_I Convert_CP932_to_UTF8(char *dst, char *src, int n_limit, int i_limit);
#endif

void EMU::update_media()
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
#ifdef USE_FLOPPY_DISK
	for(int drv = 0; drv < USE_FLOPPY_DISK; drv++) {
		if(floppy_disk_status[drv].wait_count != 0 && --floppy_disk_status[drv].wait_count == 0) {
			if(strlen(floppy_disk_status[drv].path) > 0) {
				vm->open_floppy_disk(drv, floppy_disk_status[drv].path, floppy_disk_status[drv].bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK);
#if USE_FLOPPY_DISK > 1
				out_message(_T("FD%d: %s"), drv + BASE_FLOPPY_DISK_NUM, floppy_disk_status[drv].path);
#else
				out_message(_T("FD: %s"), floppy_disk_status[drv].path);
#endif
				osdcall_floppy_inserted(drv, floppy_disk_status[drv].path, floppy_disk_status[drv].bank);
			}
		}
	}
#endif
#ifdef USE_QUICK_DISK
	for(int drv = 0; drv < USE_QUICK_DISK; drv++) {
		if(quick_disk_status[drv].wait_count != 0 && --quick_disk_status[drv].wait_count == 0) {
			vm->open_quick_disk(drv, quick_disk_status[drv].path);
#if USE_QUICK_DISK > 1
			out_message(_T("QD%d: %s"), drv + BASE_QUICK_DISK_NUM, quick_disk_status[drv].path);
#else
			out_message(_T("QD: %s"), quick_disk_status[drv].path);
#endif
			osdcall_quickdisk_inserted(drv, quick_disk_status[drv].path);
		}
	}
#endif
#ifdef USE_HARD_DISK
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		if(hard_disk_status[drv].wait_count != 0 && --hard_disk_status[drv].wait_count == 0) {
			vm->open_hard_disk(drv, hard_disk_status[drv].path);
#if USE_HARD_DISK > 1
			out_message(_T("HD%d: %s"), drv + BASE_HARD_DISK_NUM, hard_disk_status[drv].path);
#else
			out_message(_T("HD: %s"), hard_disk_status[drv].path);
#endif
			osdcall_string(EMU_MEDIA_TYPE::HARD_DISK,
						   drv,
						   EMU_MESSAGE_TYPE::MEDIA_MOUNTED,
						   (_TCHAR *)hard_disk_status[drv].path);
		}
	}
#endif
#ifdef USE_TAPE
	for(int drv = 0; drv < USE_TAPE; drv++) {
		if(tape_status[drv].wait_count != 0 && --tape_status[drv].wait_count == 0) {
			if(tape_status[drv].play) {
				vm->play_tape(drv, tape_status[drv].path);
			} else {
				vm->rec_tape(drv, tape_status[drv].path);
			}
#if USE_TAPE > 1
			out_message(_T("CMT%d: %s"), drv + BASE_TAPE_NUM, tape_status[drv].path);
#else
			out_message(_T("CMT: %s"), tape_status[drv].path);
#endif
			// ToDo: Write protect.
			EMU_MESSAGE_TYPE::type_t mess;
			mess = 	(tape_status[drv].play) ? EMU_MESSAGE_TYPE::PLAY : EMU_MESSAGE_TYPE::RECORD;
			mess |= EMU_MESSAGE_TYPE::MEDIA_MOUNTED;
			osdcall_string(EMU_MEDIA_TYPE::TAPE,
						   drv,
						   mess,
						   (_TCHAR *)tape_status[drv].path);
 		}
	}
#endif
#ifdef USE_COMPACT_DISC
	for(int drv = 0; drv < USE_COMPACT_DISC; drv++) {
		if(compact_disc_status[drv].wait_count != 0 && --compact_disc_status[drv].wait_count == 0) {
			vm->open_compact_disc(drv, compact_disc_status[drv].path);
			//printf(_T("update_media(): LOAD CDROM: %s\n"), compact_disc_status[drv].path);
#if USE_COMPACT_DISC > 1
			out_message(_T("CD%d: %s"), drv + BASE_COMPACT_DISC_NUM, compact_disc_status[drv].path);
#else
			out_message(_T("CD: %s"), compact_disc_status[drv].path);
#endif
			osdcall_string(EMU_MEDIA_TYPE::COMPACT_DISC,
						   drv,
						   EMU_MESSAGE_TYPE::MEDIA_MOUNTED,
						   (_TCHAR *)compact_disc_status[drv].path);
		}
	}
#endif
#ifdef USE_LASER_DISC
	for(int drv = 0; drv < USE_LASER_DISC; drv++) {
		if(laser_disc_status[drv].wait_count != 0 && --laser_disc_status[drv].wait_count == 0) {
			vm->open_laser_disc(drv, laser_disc_status[drv].path);
#if USE_LASER_DISC > 1
			out_message(_T("LD%d: %s"), drv + BASE_LASER_DISC_NUM, laser_disc_status[drv].path);
#else
			out_message(_T("LD: %s"), laser_disc_status[drv].path);
#endif
			osdcall_string(EMU_MEDIA_TYPE::LASER_DISC,
						   drv,
						   EMU_MESSAGE_TYPE::MEDIA_MOUNTED,
						   (_TCHAR *)laser_disc_status[drv].path);
		}
	}
#endif
#ifdef USE_BUBBLE
	for(int drv = 0; drv < USE_BUBBLE; drv++) {
		if(bubble_casette_status[drv].wait_count != 0 && --bubble_casette_status[drv].wait_count == 0) {
			vm->open_bubble_casette(drv, bubble_casette_status[drv].path, bubble_casette_status[drv].bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK);
#if USE_BUBBLE > 1
			out_message(_T("Bubble%d: %s"), drv + BASE_BUBBLE_NUM, bubble_casette_status[drv].path);
#else
			out_message(_T("Bubble: %s"), bubble_casette_status[drv].path);
#endif
			// ToDo: Write protect.
			osdcall_bubble_inserted(drv, bubble_casette_status[drv].path, b77_file[drv].cur_bank);
		}
	}
#endif
}

void EMU::restore_media()
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
#ifdef USE_CART
	for(int drv = 0; drv < USE_CART; drv++) {
		if(cart_status[drv].path[0] != _T('\0')) {
			if(check_file_extension(cart_status[drv].path, _T(".hex")) && hex2bin(cart_status[drv].path, create_local_path(_T("hex2bin.$$$")))) {
				vm->open_cart(drv, create_local_path(_T("hex2bin.$$$")));
				FILEIO::RemoveFile(create_local_path(_T("hex2bin.$$$")));
			} else {
				vm->open_cart(drv, cart_status[drv].path);
			}
			EMU_MESSAGE_TYPE::type_t mess = EMU_MESSAGE_TYPE::MEDIA_MOUNTED;
			// ToDo: write protect
			osdcall_string(EMU_MEDIA_TYPE::CARTRIDGE,
						   drv,
						   mess,
						   cart_status[drv].path);

		}
	}
#endif
#ifdef USE_FLOPPY_DISK
	for(int drv = 0; drv < USE_FLOPPY_DISK; drv++) {
		if(floppy_disk_status[drv].path[0] != _T('\0')) {
			vm->open_floppy_disk(drv, floppy_disk_status[drv].path, floppy_disk_status[drv].bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK);
			osdcall_floppy_inserted(drv, floppy_disk_status[drv].path, floppy_disk_status[drv].bank);
		}
	}
#endif
#ifdef USE_QUICK_DISK
	for(int drv = 0; drv < USE_QUICK_DISK; drv++) {
		if(quick_disk_status[drv].path[0] != _T('\0')) {
			vm->open_quick_disk(drv, quick_disk_status[drv].path);
			osdcall_quickdisk_inserted(drv, quick_disk_status[drv].path);
		}
	}
#endif
#ifdef USE_HARD_DISK
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		if(hard_disk_status[drv].path[0] != _T('\0')) {
			vm->open_hard_disk(drv, hard_disk_status[drv].path);
			EMU_MESSAGE_TYPE::type_t mess = EMU_MESSAGE_TYPE::MEDIA_MOUNTED;
			osdcall_string(EMU_MEDIA_TYPE::HARD_DISK,
						   drv,
						   mess,
						   hard_disk_status[drv].path);
		}
	}
#endif
#ifdef USE_TAPE
	for(int drv = 0; drv < USE_TAPE; drv++) {
		if(tape_status[drv].path[0] != _T('\0')) {
			if(tape_status[drv].play) {
				vm->play_tape(drv, tape_status[drv].path);
				EMU_MESSAGE_TYPE::type_t mess = EMU_MESSAGE_TYPE::MEDIA_MOUNTED;
				osdcall_string(EMU_MEDIA_TYPE::TAPE,
							   drv,
							   mess,
							   tape_status[drv].path);
			} else {
				tape_status[drv].path[0] = _T('\0');
			}
		}
	}
#endif
#ifdef USE_COMPACT_DISC
	for(int drv = 0; drv < USE_COMPACT_DISC; drv++) {
		if(compact_disc_status[drv].path[0] != _T('\0')) {
			vm->open_compact_disc(drv, compact_disc_status[drv].path);
			//printf(_T("restore_media(): LOAD CDROM: %s\n"), compact_disc_status[drv].path);
			osdcall_string(EMU_MEDIA_TYPE::COMPACT_DISC,
						   drv,
						   EMU_MESSAGE_TYPE::MEDIA_MOUNTED,
						   (_TCHAR*)compact_disc_status[drv].path);
		}
	}
#endif
#ifdef USE_LASER_DISC
	for(int drv = 0; drv < USE_LASER_DISC; drv++) {
		if(laser_disc_status[drv].path[0] != _T('\0')) {
			vm->open_laser_disc(drv, laser_disc_status[drv].path);
			osdcall_string(EMU_MEDIA_TYPE::LASER_DISC,
						   drv,
						   EMU_MESSAGE_TYPE::MEDIA_MOUNTED,
						   laser_disc_status[drv].path);
		}
	}
#endif
#ifdef USE_BUBBLE
	for(int drv = 0; drv < USE_BUBBLE; drv++) {
		if(bubble_casette_status[drv].path[0] != _T('\0')) {
			vm->open_bubble_casette(drv, bubble_casette_status[drv].path, bubble_casette_status[drv].bank);
			osdcall_bubble_inserted(drv, bubble_casette_status[drv].path, b77_file[drv].cur_bank);
		}
	}
#endif
}

#ifdef USE_CART
void EMU::open_cart(int drv, const _TCHAR* file_path)
{
	if(drv < USE_CART) {
		if(check_file_extension(file_path, _T(".hex")) && hex2bin(file_path, create_local_path(_T("hex2bin.$$$")))) {
			vm->open_cart(drv, create_local_path(_T("hex2bin.$$$")));
			FILEIO::RemoveFile(create_local_path(_T("hex2bin.$$$")));
		} else {
			vm->open_cart(drv, file_path);
		}
		my_tcscpy_s(cart_status[drv].path, _MAX_PATH, file_path);
		out_message(_T("Cart%d: %s"), drv + 1, file_path);
		osdcall_string(EMU_MEDIA_TYPE::CARTRIDGE,
					   drv,
					   EMU_MESSAGE_TYPE::MEDIA_MOUNTED,
					   (_TCHAR *)file_path);
#if !defined(_USE_QT)
		// restart recording
		bool s = osd->now_record_sound;
		bool v = osd->now_record_video;
		stop_record_sound();
		stop_record_video();

		if(s) osd->start_record_sound();
		if(v) osd->start_record_video(-1);
#endif
	}
}

void EMU::close_cart(int drv)
{
	if(drv < USE_CART) {
		__LIKELY_IF(vm != NULL) {
			vm->close_cart(drv);
		}
		clear_media_status(&cart_status[drv]);
#if USE_CART > 1
		out_message(_T("Cart%d: Ejected"), drv + BASE_CART_NUM);
#else
		out_message(_T("Cart: Ejected"));
#endif
		osdcall_int(EMU_MEDIA_TYPE::CARTRIDGE,
					drv,
					EMU_MESSAGE_TYPE::MEDIA_REMOVED,
					0);
#if !defined(_USE_QT)
		// stop recording
		stop_record_video();
		stop_record_sound();
#endif
	}
}

bool EMU::is_cart_inserted(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return false;
	}
	if(drv < USE_CART) {
		return vm->is_cart_inserted(drv);
	} else {
		return false;
	}
}
#endif

#ifdef USE_FLOPPY_DISK
bool EMU::create_blank_floppy_disk(const _TCHAR* file_path, uint8_t type)
{
	/*
		type: 0x00 = 2D, 0x10 = 2DD, 0x20 = 2HD
	*/
	struct {
		char title[17];
		uint8_t rsrv[9];
		uint8_t protect;
		uint8_t type;
		uint32_t size;
		uint32_t trkptr[164];
	} d88_hdr;

	memset(&d88_hdr, 0, sizeof(d88_hdr));
	my_strcpy_s(d88_hdr.title, sizeof(d88_hdr.title), "BLANK");
	d88_hdr.type = type;
	d88_hdr.size = sizeof(d88_hdr);

	FILEIO *fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		fio->Fwrite(&d88_hdr, sizeof(d88_hdr), 1);
		fio->Fclose();
	}
	delete fio;
	return true;
}

void EMU::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < USE_FLOPPY_DISK) {
		d88_file[drv].bank_num = 0;
		d88_file[drv].cur_bank = -1;
		if(check_file_extension(file_path, _T(".d88")) || check_file_extension(file_path, _T(".d8e")) ||
		   check_file_extension(file_path, _T(".d77")) || check_file_extension(file_path, _T(".1dd"))) {
			FILEIO *fio = new FILEIO();
			if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
				try {
					fio->Fseek(0, FILEIO_SEEK_END);
					uint32_t file_size = fio->Ftell(), file_offset = 0;
					while(file_offset + 0x2b0 <= file_size && d88_file[drv].bank_num < MAX_D88_BANKS) {
						fio->Fseek(file_offset, FILEIO_SEEK_SET);
#ifdef _UNICODE
						char tmp[18];
						fio->Fread(tmp, 17, 1);
						tmp[17] = 0;
	#if defined(_USE_QT)
		
						if(strlen(tmp) > 0) {
							Convert_CP932_to_UTF8(d88_file[drv].disk_name[d88_file[drv].bank_num], tmp, 128, 18);
						}
	#else
						MultiByteToWideChar(CP_ACP, 0, tmp, -1, d88_file[drv].disk_name[d88_file[drv].bank_num], 18);
	#endif						
#else
						fio->Fread(d88_file[drv].disk_name[d88_file[drv].bank_num], 17, 1);
						d88_file[drv].disk_name[d88_file[drv].bank_num][17] = 0;
#endif
						fio->Fseek(file_offset + 0x1c, SEEK_SET);
						file_offset += fio->FgetUint32_LE();
						d88_file[drv].bank_num++;
					}
					my_tcscpy_s(d88_file[drv].path, _MAX_PATH, file_path);
					d88_file[drv].cur_bank = bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK;
				} catch(...) {
					d88_file[drv].bank_num = 0;
				}
				fio->Fclose();
			}
			delete fio;
		}
		if(vm->is_floppy_disk_inserted(drv)) {
			vm->close_floppy_disk(drv);
			// wait 0.5sec
			floppy_disk_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_FLOPPY_DISK > 1
			out_message(_T("FD%d: Ejected"), drv + BASE_FLOPPY_DISK_NUM);
#else
			out_message(_T("FD: Ejected"));
#endif
			osdcall_floppy_ejected(drv);			
		} else if(floppy_disk_status[drv].wait_count == 0) {
			vm->open_floppy_disk(drv, file_path, bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK);
#if USE_FLOPPY_DISK > 1
			out_message(_T("FD%d: %s"), drv + BASE_FLOPPY_DISK_NUM, file_path);
#else
			out_message(_T("FD: %s"), file_path);
#endif
			osdcall_floppy_inserted(drv, (_TCHAR *)file_path, bank);
		}
		//QString tmps = QString::fromLocal8Bit(file_path);
		//printf("%d %s\n", drv, tmps.toUtf8().constData());
		my_tcscpy_s(floppy_disk_status[drv].path, _MAX_PATH, file_path);
		floppy_disk_status[drv].bank = bank;
	}
}


void EMU::close_floppy_disk(int drv)
{
	if(drv < USE_FLOPPY_DISK) {
		d88_file[drv].bank_num = 0;
		d88_file[drv].cur_bank = -1;

		__LIKELY_IF(vm != NULL) {
			vm->close_floppy_disk(drv);
		}
		clear_media_status(&floppy_disk_status[drv]);
		__LIKELY_IF(vm != NULL) {
			floppy_disk_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
		}
#if USE_FLOPPY_DISK > 1
		out_message(_T("FD%d: Ejected"), drv + BASE_FLOPPY_DISK_NUM);
#else
		out_message(_T("FD: Ejected"));
#endif
		osdcall_floppy_ejected(drv);			
	}
}

bool EMU::is_floppy_disk_connected(int drv)
{
	if(drv < USE_FLOPPY_DISK) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_floppy_disk_connected(drv);
		}
		return false;
	} else {
		return false;
	}
}

bool EMU::is_floppy_disk_inserted(int drv)
{
	if(drv < USE_FLOPPY_DISK) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_floppy_disk_inserted(drv);
		}
		return false;
	} else {
		return false;
	}
}

void EMU::is_floppy_disk_protected(int drv, bool value)
{
	if(drv < USE_FLOPPY_DISK) {
		__LIKELY_IF(vm != NULL) {
			vm->is_floppy_disk_protected(drv, value);
		}
	}
	EMU_MESSAGE_TYPE::type_t mess =
		(value) ?
		(EMU_MESSAGE_TYPE::WRITE_PROTECT | EMU_MESSAGE_TYPE::MEDIA_OTHERS)
		: EMU_MESSAGE_TYPE::MEDIA_OTHERS;
	osdcall_int(EMU_MEDIA_TYPE::FLOPPY_DISK,
				drv,
				mess,
				0);
}

bool EMU::is_floppy_disk_protected(int drv)
{
	if(drv < USE_FLOPPY_DISK) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_floppy_disk_protected(drv);
		}
		return false;
	} else {
		return false;
	}
}

uint32_t EMU::is_floppy_disk_accessed()
{
	__LIKELY_IF(vm != NULL) {
		return vm->is_floppy_disk_accessed();
	}
	return 0x00000000;
}

uint32_t EMU::floppy_disk_indicator_color()
{
	__LIKELY_IF(vm != NULL) {
		return vm->floppy_disk_indicator_color();
	}
	return 0;
}

#endif

#ifdef USE_QUICK_DISK
void EMU::open_quick_disk(int drv, const _TCHAR* file_path)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_QUICK_DISK) {
		if(vm->is_quick_disk_inserted(drv)) {
			vm->close_quick_disk(drv);
			// wait 0.5sec
			quick_disk_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_QUICK_DISK > 1
			out_message(_T("QD%d: Ejected"), drv + BASE_QUICK_DISK_NUM);
#else
			out_message(_T("QD: Ejected"));
#endif
			osdcall_quickdisk_ejected(drv);
		} else if(quick_disk_status[drv].wait_count == 0) {
			vm->open_quick_disk(drv, file_path);
#if USE_QUICK_DISK > 1
			out_message(_T("QD%d: %s"), drv + BASE_QUICK_DISK_NUM, file_path);
#else
			out_message(_T("QD: %s"), file_path);
#endif
			osdcall_quickdisk_inserted(drv, (_TCHAR*)file_path);
		}
		my_tcscpy_s(quick_disk_status[drv].path, _MAX_PATH, file_path);
	}
}

void EMU::close_quick_disk(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_QUICK_DISK) {
		vm->close_quick_disk(drv);
		clear_media_status(&quick_disk_status[drv]);
		quick_disk_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_QUICK_DISK > 1
		out_message(_T("QD%d: Ejected"), drv + BASE_QUICK_DISK_NUM);
#else
		out_message(_T("QD: Ejected"));
#endif
		osdcall_quickdisk_ejected(drv);
	}
}

bool EMU::is_quick_disk_inserted(int drv)
{
	if(drv < USE_QUICK_DISK) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_quick_disk_inserted(drv);
		}
		return false;
	} else {
		return false;
	}
}

bool EMU::is_quick_disk_connected(int drv)
{
	if(drv < USE_QUICK_DISK) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_quick_disk_connected(drv);
		}
		return false;
	} else {
		return false;
	}
}

uint32_t EMU::is_quick_disk_accessed()
{
	__LIKELY_IF(vm != NULL) {
		return vm->is_quick_disk_accessed();
	}
	return false;
}
#endif

#ifdef USE_HARD_DISK
bool EMU::create_blank_hard_disk(const _TCHAR* file_path, int sector_size, int sectors, int surfaces, int cylinders)
{
	if(check_file_extension(file_path, _T(".nhd"))) {
		// T98-Next
		const char sig_nhd[] = "T98HDDIMAGE.R0";
		typedef struct nhd_header_s {
			char sig[16];
			char comment[256];
			int32_t header_size;	// +272
			int32_t cylinders;	// +276
			int16_t surfaces;	// +280
			int16_t sectors;	// +282
			int16_t sector_size;	// +284
			uint8_t reserved[0xe2];
		} nhd_header_t;
		nhd_header_t header;

		memset(&header, 0, sizeof(header));
		strcpy(header.sig, "T98HDDIMAGE.R0");
		header.header_size = sizeof(header);
		header.cylinders = cylinders;
		header.surfaces = surfaces;
		header.sectors = sectors;
		header.sector_size = sector_size;

		FILEIO *fio = new FILEIO();
		if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
			fio->Fwrite(&header, sizeof(header), 1);
			void *empty = calloc(sector_size, 1);
#if 0
			fio->Fwrite(empty, sector_size, sectors * surfaces * cylinders);
#else
			for(int i = 0; i < sectors * surfaces * cylinders; i++) {
				fio->Fwrite(empty, sector_size, 1);
			}
#endif
			free(empty);
			fio->Fclose();
		}
		delete fio;
		return true;
	} else if(check_file_extension(file_path, _T(".hdi"))) {
		// ANEX86
		typedef struct hdi_header_s {
			int32_t dummy;		// + 0
			int32_t hdd_type;	// + 4
			int32_t header_size;	// + 8
			int32_t hdd_size;	// +12
			int32_t sector_size;	// +16
			int32_t sectors;	// +20
			int32_t surfaces;	// +24
			int32_t cylinders;	// +28
			uint8_t padding[0x1000 - sizeof(int32_t) * 8];
		} hdi_header_t;
		hdi_header_t header;

		memset(&header, 0, sizeof(header));
		header.hdd_type = 0; // ???
		header.header_size = sizeof(header);
		header.hdd_size = sector_size * sectors * surfaces * cylinders;
		header.sector_size = sector_size;
		header.sectors = sectors;
		header.surfaces = surfaces;
		header.cylinders = cylinders;

		FILEIO *fio = new FILEIO();
		if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
			fio->Fwrite(&header, sizeof(header), 1);
			void *empty = calloc(sector_size, 1);
#if 0
			fio->Fwrite(empty, sector_size, sectors * surfaces * cylinders);
#else
			for(int i = 0; i < sectors * surfaces * cylinders; i++) {
				fio->Fwrite(empty, sector_size, 1);
			}
#endif
			free(empty);
			fio->Fclose();
		}
		delete fio;
		return true;
	}
	// unknown extension
	return false;
}

void EMU::open_hard_disk(int drv, const _TCHAR* file_path)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_HARD_DISK) {
		if(vm->is_hard_disk_inserted(drv)) {
			vm->close_hard_disk(drv);
			// wait 0.5sec
			hard_disk_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_HARD_DISK > 1
			out_message(_T("HD%d: Unmounted"), drv + BASE_HARD_DISK_NUM);
#else
			out_message(_T("HD: Unmounted"));
#endif
			osdcall_int(EMU_MEDIA_TYPE::HARD_DISK,
							drv,
							EMU_MESSAGE_TYPE::MEDIA_REMOVED,
							0);
		} else if(hard_disk_status[drv].wait_count == 0) {
			vm->open_hard_disk(drv, file_path);
#if USE_HARD_DISK > 1
			out_message(_T("HD%d: %s"), drv + BASE_HARD_DISK_NUM, file_path);
#else
			out_message(_T("HD: %s"), file_path);
#endif
			osdcall_string(EMU_MEDIA_TYPE::HARD_DISK,
							drv,
							EMU_MESSAGE_TYPE::MEDIA_MOUNTED,
							(_TCHAR*)file_path);
		}
		my_tcscpy_s(hard_disk_status[drv].path, _MAX_PATH, file_path);
		my_tcscpy_s(config.last_hard_disk_path[drv], _MAX_PATH, file_path);
	}
}

void EMU::close_hard_disk(int drv)
{
	__LIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_HARD_DISK) {
		vm->close_hard_disk(drv);
		clear_media_status(&hard_disk_status[drv]);
		hard_disk_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_HARD_DISK > 1
		out_message(_T("HD%d: Unmounted"), drv + BASE_HARD_DISK_NUM);
#else
		out_message(_T("HD: Unmounted"));
#endif
		osdcall_int(EMU_MEDIA_TYPE::HARD_DISK,
						drv,
						EMU_MESSAGE_TYPE::MEDIA_REMOVED,
						0);
		config.last_hard_disk_path[drv][0] = '\0';
	}
}

bool EMU::is_hard_disk_inserted(int drv)
{
	if(drv < USE_HARD_DISK) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_hard_disk_inserted(drv);
		}
		return false;
	} else {
		return false;
	}
}

uint32_t EMU::is_hard_disk_accessed()
{
	__LIKELY_IF(vm != NULL) {
		return vm->is_hard_disk_accessed();
	}
	return 0x00000000;
}

#endif

#ifdef USE_TAPE
void EMU::play_tape(int drv, const _TCHAR* file_path)
{
	__LIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_TAPE) {
		if(vm->is_tape_inserted(drv)) {
			vm->close_tape(drv);
			// wait 0.5sec
			tape_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_TAPE > 1
			out_message(_T("CMT%d: Ejected"), drv + BASE_TAPE_NUM);
#else
			out_message(_T("CMT: Ejected"));
#endif
			osdcall_int(EMU_MEDIA_TYPE::TAPE,
						drv,
						EMU_MESSAGE_TYPE::MEDIA_REMOVED,
						0);
		} else if(tape_status[drv].wait_count == 0) {
			vm->play_tape(drv, file_path);
#if USE_TAPE > 1
			out_message(_T("CMT%d: %s"), drv + BASE_TAPE_NUM, file_path);
#else
			out_message(_T("CMT: %s"), file_path);
#endif
			osdcall_string(EMU_MEDIA_TYPE::TAPE,
							drv,
							EMU_MESSAGE_TYPE::MEDIA_MOUNTED,
							(_TCHAR*)file_path);
		}
		my_tcscpy_s(tape_status[drv].path, _MAX_PATH, file_path);
		tape_status[drv].play = true;
	}
}

void EMU::rec_tape(int drv, const _TCHAR* file_path)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_TAPE) {
		if(vm->is_tape_inserted(drv)) {
			vm->close_tape(drv);
			// wait 0.5sec
			tape_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_TAPE > 1
			out_message(_T("CMT%d: Ejected"), drv + BASE_TAPE_NUM);
#else
			out_message(_T("CMT: Ejected"));
#endif
			osdcall_int(EMU_MEDIA_TYPE::TAPE,
						drv,
						EMU_MESSAGE_TYPE::MEDIA_REMOVED,
						0);
		} else if(tape_status[drv].wait_count == 0) {
			vm->rec_tape(drv, file_path);
#if USE_TAPE > 1
			out_message(_T("CMT%d: %s"), drv + BASE_TAPE_NUM, file_path);
#else
			out_message(_T("CMT: %s"), file_path);
#endif
			osdcall_string(EMU_MEDIA_TYPE::TAPE,
							drv,
						   EMU_MESSAGE_TYPE::MEDIA_MOUNTED | EMU_MESSAGE_TYPE::RECORD,
							(_TCHAR*)file_path);
		}
		my_tcscpy_s(tape_status[drv].path, _MAX_PATH, file_path);
		tape_status[drv].play = false;
	}
}

void EMU::close_tape(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_TAPE) {
		vm->close_tape(drv);
		clear_media_status(&tape_status[drv]);
		tape_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_TAPE > 1
		out_message(_T("CMT%d: Ejected"), drv + BASE_TAPE_NUM);
#else
		out_message(_T("CMT: Ejected"));
#endif
		osdcall_int(EMU_MEDIA_TYPE::TAPE,
					drv,
					EMU_MESSAGE_TYPE::MEDIA_REMOVED,
					0);
	}
}

bool EMU::is_tape_inserted(int drv)
{
	if(drv < USE_TAPE) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_tape_inserted(drv);
		}
		return false;
	} else {
		return false;
	}
}

bool EMU::is_tape_playing(int drv)
{
	if(drv < USE_TAPE) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_tape_playing(drv);
		}
		return false;
	} else {
		return false;
	}
}

bool EMU::is_tape_recording(int drv)
{
	if(drv < USE_TAPE) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_tape_recording(drv);
		}
		return false;
	} else {
		return false;
	}
}

int EMU::get_tape_position(int drv)
{
	if(drv < USE_TAPE) {
		__LIKELY_IF(vm != NULL) {
			return vm->get_tape_position(drv);
		}
		return 0;
	} else {
		return 0;
	}
}

const _TCHAR* EMU::get_tape_message(int drv)
{
	if(drv < USE_TAPE) {
		__LIKELY_IF(vm != NULL) {
			return vm->get_tape_message(drv);
		}
		return NULL;
	} else {
		return NULL;
	}
}

void EMU::push_play(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_TAPE) {
		vm->push_play(drv);
		osdcall_int(EMU_MEDIA_TYPE::TAPE,
					drv,
					EMU_MESSAGE_TYPE::TAPE_PLAY,
					0);
	}
}

void EMU::push_stop(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_TAPE) {
		vm->push_stop(drv);
		osdcall_int(EMU_MEDIA_TYPE::TAPE,
					drv,
					EMU_MESSAGE_TYPE::TAPE_STOP,
					0);
	}
}

void EMU::push_fast_forward(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_TAPE) {
		vm->push_fast_forward(drv);
		osdcall_int(EMU_MEDIA_TYPE::TAPE,
					drv,
					EMU_MESSAGE_TYPE::TAPE_FF,
					0);
	}
}

void EMU::push_fast_rewind(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_TAPE) {
		vm->push_fast_rewind(drv);
		osdcall_int(EMU_MEDIA_TYPE::TAPE,
					drv,
					EMU_MESSAGE_TYPE::TAPE_REW,
					0);
	}
}

void EMU::push_apss_forward(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_TAPE) {
		vm->push_apss_forward(drv);
		osdcall_int(EMU_MEDIA_TYPE::TAPE,
					drv,
					EMU_MESSAGE_TYPE::TAPE_APSS_FF,
					0);
	}
}

void EMU::push_apss_rewind(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_TAPE) {
		vm->push_apss_rewind(drv);
		osdcall_int(EMU_MEDIA_TYPE::TAPE,
					drv,
					EMU_MESSAGE_TYPE::TAPE_APSS_REW,
					0);
	}
}
#endif

#ifdef USE_COMPACT_DISC
void EMU::open_compact_disc(int drv, const _TCHAR* file_path)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_COMPACT_DISC) {
		//printf(_T("open_compact_disc(): CALLED: %s\n"), file_path);
		if(vm->is_compact_disc_inserted(drv)) {
			vm->close_compact_disc(drv);
			// wait 0.5sec
			clear_media_status(&compact_disc_status[drv]);
			compact_disc_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_COMPACT_DISC > 1
			out_message(_T("CD%d: Ejected"), drv + BASE_COMPACT_DISC_NUM);
#else
			out_message(_T("CD: Ejected"));
#endif
			osdcall_int(EMU_MEDIA_TYPE::COMPACT_DISC,
						drv,
						EMU_MESSAGE_TYPE::MEDIA_REMOVED,
						0);
		} else if(compact_disc_status[drv].wait_count == 0) {
			vm->open_compact_disc(drv, file_path);
			//printf(_T("open_compact_disc(): LOAD CDROM: %s\n"), file_path);
#if USE_COMPACT_DISC > 1
			out_message(_T("CD%d: %s"), drv + BASE_COMPACT_DISC_NUM, file_path);
#else
			out_message(_T("CD: %s"), file_path);
#endif
			osdcall_string(EMU_MEDIA_TYPE::COMPACT_DISC,
							drv,
							EMU_MESSAGE_TYPE::MEDIA_MOUNTED,
							(_TCHAR*)file_path);
		}
		my_tcscpy_s(compact_disc_status[drv].path,  _MAX_PATH, file_path);
	}
}

void EMU::close_compact_disc(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_COMPACT_DISC) {
		vm->close_compact_disc(drv);
		clear_media_status(&compact_disc_status[drv]);
		compact_disc_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_COMPACT_DISC > 1
		out_message(_T("CD%d: Ejected"), drv + BASE_COMPACT_DISC_NUM);
#else
		out_message(_T("CD: Ejected"));
#endif
		osdcall_int(EMU_MEDIA_TYPE::COMPACT_DISC,
					drv,
					EMU_MESSAGE_TYPE::MEDIA_REMOVED,
					0);
	}
}

bool EMU::is_compact_disc_inserted(int drv)
{
	if(drv < USE_COMPACT_DISC) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_compact_disc_inserted(drv);
		}
		return false;
	} else {
		return false;
	}
}

uint32_t EMU::is_compact_disc_accessed()
{
	__LIKELY_IF(vm != NULL) {
		return vm->is_compact_disc_accessed();
	}
	return 0x00000000;
}
#endif

#ifdef USE_LASER_DISC
void EMU::open_laser_disc(int drv, const _TCHAR* file_path)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_LASER_DISC) {
		if(vm->is_laser_disc_inserted(drv)) {
			vm->close_laser_disc(drv);
			// wait 0.5sec
			laser_disc_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_LASER_DISC > 1
			out_message(_T("LD%d: Ejected"), drv + BASE_LASER_DISC_NUM);
#else
			out_message(_T("LD: Ejected"));
#endif
			osdcall_int(EMU_MEDIA_TYPE::LASER_DISC,
						drv,
						EMU_MESSAGE_TYPE::MEDIA_REMOVED,
						0);
		} else if(laser_disc_status[drv].wait_count == 0) {
			vm->open_laser_disc(drv, file_path);
#if USE_LASER_DISC > 1
			out_message(_T("LD%d: %s"), drv + BASE_LASER_DISC_NUM, file_path);
#else
			out_message(_T("LD: %s"), file_path);
#endif
			osdcall_string(EMU_MEDIA_TYPE::LASER_DISC,
							drv,
							EMU_MESSAGE_TYPE::MEDIA_MOUNTED,
							(_TCHAR*)file_path);
		}
		my_tcscpy_s(laser_disc_status[drv].path, _MAX_PATH, file_path);
	}
}

void EMU::close_laser_disc(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_LASER_DISC) {
		vm->close_laser_disc(drv);
		clear_media_status(&laser_disc_status[drv]);
		laser_disc_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_LASER_DISC > 1
		out_message(_T("LD%d: Ejected"), drv + BASE_LASER_DISC_NUM);
#else
		out_message(_T("LD: Ejected"));
#endif
		osdcall_int(EMU_MEDIA_TYPE::LASER_DISC,
					drv,
					EMU_MESSAGE_TYPE::MEDIA_REMOVED,
					0);
	}
}

bool EMU::is_laser_disc_inserted(int drv)
{
	if(drv < USE_LASER_DISC) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_laser_disc_inserted(drv);
		}
		return false;
	} else {
		return false;
	}
}

uint32_t EMU::is_laser_disc_accessed()
{
	__LIKELY_IF(vm != NULL) {
		return vm->is_laser_disc_accessed();
	}
	return 0x00000000;
}

#endif

#ifdef USE_BINARY_FILE
void EMU::load_binary(int drv, const _TCHAR* file_path)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_BINARY_FILE) {
		if(check_file_extension(file_path, _T(".hex")) && hex2bin(file_path, create_local_path(_T("hex2bin.$$$")))) {
			vm->load_binary(drv, create_local_path(_T("hex2bin.$$$")));
			FILEIO::RemoveFile(create_local_path(_T("hex2bin.$$$")));
		} else {
			vm->load_binary(drv, file_path);
		}
		osdcall_string(EMU_MEDIA_TYPE::BINARY,
					   drv,
					   EMU_MESSAGE_TYPE::MEDIA_MOUNTED | EMU_MESSAGE_TYPE::LOAD,
					   (_TCHAR*)file_path);
#if USE_BINARY_FILE > 1
		out_message(_T("Load Binary%d: %s"), drv + BASE_BINARY_FILE_NUM, file_path);
#else
		out_message(_T("Load Binary: %s"), file_path);
#endif
	}
}

void EMU::save_binary(int drv, const _TCHAR* file_path)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_BINARY_FILE) {
 		vm->save_binary(drv, file_path);
#if USE_BINARY_FILE > 1
		out_message(_T("Save Binary%d: %s"), drv + BASE_BINARY_FILE_NUM, file_path);
#else
		out_message(_T("Save Binary: %s"), file_path);
#endif
		osdcall_string(EMU_MEDIA_TYPE::BINARY,
					   drv,
					   EMU_MESSAGE_TYPE::MEDIA_MOUNTED | EMU_MESSAGE_TYPE::SAVE,
					   (_TCHAR*)file_path);
	}
}

#endif
#ifdef USE_BUBBLE
void EMU::open_bubble_casette(int drv, const _TCHAR* file_path, int bank)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_BUBBLE) {
		if(vm->is_bubble_casette_inserted(drv)) {
			vm->close_bubble_casette(drv);
			// wait 0.5sec
			bubble_casette_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_BUBBLE > 1
			out_message(_T("Bubble%d: Ejected"), drv + BASE_BUBBLE_NUM);
#else
			out_message(_T("Bubble: Ejected"));
#endif
			osdcall_bubble_ejected(drv);
		} else if(bubble_casette_status[drv].wait_count == 0) {
			vm->open_bubble_casette(drv, file_path, bank);
#if USE_BUBBLE > 1
			out_message(_T("Bubble%d: %s"), drv + BASE_BUBBLE_NUM, file_path);
#else
			out_message(_T("Bubble: %s"), file_path);
#endif
			osdcall_bubble_inserted(drv, (_TCHAR*)file_path, bank);
		}
		my_tcscpy_s(bubble_casette_status[drv].path, _MAX_PATH, file_path);
		bubble_casette_status[drv].bank = bank;
	}
}

void EMU::close_bubble_casette(int drv)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_BUBBLE) {
		vm->close_bubble_casette(drv);
		clear_media_status(&bubble_casette_status[drv]);
		bubble_casette_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#if USE_BUBBLE > 1
		out_message(_T("Bubble%d: Ejected"), drv + BASE_BUBBLE_NUM);
#else
		out_message(_T("Bubble: Ejected"));
#endif
		osdcall_bubble_ejected(drv);
	}
}

bool EMU::is_bubble_casette_inserted(int drv)
{
	if(drv < USE_BUBBLE) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_bubble_casette_inserted(drv);
		}
		return false;
	} else {
		return false;
	}
}

bool EMU::is_bubble_casette_protected(int drv)
{
	if(drv < USE_BUBBLE) {
		__LIKELY_IF(vm != NULL) {
			return vm->is_bubble_casette_protected(drv);
		}
		return false;
	} else {
		return false;
	}
}

void EMU::is_bubble_casette_protected(int drv, bool flag)
{
	__UNLIKELY_IF(vm == NULL) {
		return;
	}
	if(drv < USE_BUBBLE) {
		vm->is_bubble_casette_protected(drv, flag);
		EMU_MESSAGE_TYPE::type_t mess = EMU_MESSAGE_TYPE::MEDIA_OTHERS;
		mess |= ((flag) ? EMU_MESSAGE_TYPE::WRITE_PROTECT : 0);
		osdcall_int(EMU_MEDIA_TYPE::BUBBLE_CASETTE,
					drv,
					mess,
					0);
		return;
	} else {
		return;
	}
}
#endif

#ifdef USE_LED_DEVICE
uint32_t EMU::get_led_status()
{
	__UNLIKELY_IF(vm == NULL) {
		return 0x00000000;
	}
	return vm->get_led_status();
}
#endif


#ifdef USE_SOUND_VOLUME
void EMU::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	__LIKELY_IF(vm != NULL) {
		vm->set_sound_device_volume(ch, decibel_l, decibel_r);
	}
}
#endif

void EMU::update_config()
{
	__LIKELY_IF(vm != NULL) {
		vm->update_config();
	}
}


// ----------------------------------------------------------------------------
// state
// ----------------------------------------------------------------------------

#ifdef USE_STATE
#define STATE_VERSION	2

void EMU::save_state(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	osd->lock_vm();
#ifdef USE_ZLIB
	if(config.compress_state) {
		fio->Gzopen(file_path, FILEIO_WRITE_BINARY);
	}
#endif
	if(!fio->IsOpened()) {
		fio->Fopen(file_path, FILEIO_WRITE_BINARY);
	}
	if(fio->IsOpened()) {
		// save state file version
		fio->FputUint32(STATE_VERSION);
		// save config
		process_config_state((void *)fio, false);
		// save inserted medias
#ifdef USE_CART
		fio->Fwrite(&cart_status, sizeof(cart_status), 1);
#endif
#ifdef USE_FLOPPY_DISK
		fio->Fwrite(floppy_disk_status, sizeof(floppy_disk_status), 1);
		fio->Fwrite(d88_file, sizeof(d88_file), 1);
#endif
#ifdef USE_QUICK_DISK
		fio->Fwrite(&quick_disk_status, sizeof(quick_disk_status), 1);
#endif
#ifdef USE_HARD_DISK
		fio->Fwrite(&hard_disk_status, sizeof(hard_disk_status), 1);
#endif
#ifdef USE_TAPE
		fio->Fwrite(&tape_status, sizeof(tape_status), 1);
#endif
#ifdef USE_COMPACT_DISC
		fio->Fwrite(&compact_disc_status, sizeof(compact_disc_status), 1);
#endif
#ifdef USE_LASER_DISC
		fio->Fwrite(&laser_disc_status, sizeof(laser_disc_status), 1);
#endif
#ifdef USE_BUBBLE
		fio->Fwrite(&bubble_casette_status, sizeof(bubble_casette_status), 1);
#endif
		// save vm state
		__LIKELY_IF(vm != NULL) {
			vm->process_state(fio, false);
		}
		// end of state file
		fio->FputInt32_LE(-1);
		fio->Fclose();
	}
	osd->unlock_vm();
	delete fio;
}

void EMU::load_state(const _TCHAR* file_path)
{
	if(FILEIO::IsFileExisting(file_path)) {
#ifdef USE_AUTO_KEY
		stop_auto_key();
		config.romaji_to_kana = false;
#endif

		save_state(create_local_path(_T("$temp$.sta")));
		if(!load_state_tmp(file_path)) {
			out_debug_log(_T("failed to load state file\n"));
			load_state_tmp(create_local_path(_T("$temp$.sta")));
		}
		FILEIO::RemoveFile(create_local_path(_T("$temp$.sta")));
	}
}

bool EMU::load_state_tmp(const _TCHAR* file_path)
{
	bool result = false;
	FILEIO* fio = new FILEIO();
	osd->lock_vm();
#ifdef USE_ZLIB
	if(config.compress_state) {
		fio->Gzopen(file_path, FILEIO_READ_BINARY);
	}
#endif
	if(!fio->IsOpened()) {
		fio->Fopen(file_path, FILEIO_READ_BINARY);
	}
	if(fio->IsOpened()) {
		// check state file version
		if(fio->FgetUint32() == STATE_VERSION) {
			// load config
			if(process_config_state((void *)fio, true)) {
				// load inserted medias
#ifdef USE_CART
				fio->Fread(&cart_status, sizeof(cart_status), 1);
#endif
#ifdef USE_FLOPPY_DISK
				fio->Fread(floppy_disk_status, sizeof(floppy_disk_status), 1);
				fio->Fread(d88_file, sizeof(d88_file), 1);
#endif
#ifdef USE_QUICK_DISK
				fio->Fread(&quick_disk_status, sizeof(quick_disk_status), 1);
#endif
#ifdef USE_HARD_DISK
				fio->Fread(&hard_disk_status, sizeof(hard_disk_status), 1);
#endif
#ifdef USE_TAPE
				fio->Fread(&tape_status, sizeof(tape_status), 1);
#endif
#ifdef USE_COMPACT_DISC
				fio->Fread(&compact_disc_status, sizeof(compact_disc_status), 1);
#endif
#ifdef USE_LASER_DISC
				fio->Fread(&laser_disc_status, sizeof(laser_disc_status), 1);
#endif
#ifdef USE_BUBBLE
				fio->Fread(&bubble_casette_status, sizeof(bubble_casette_status), 1);
#endif
				// check if virtual machine should be reinitialized
				bool reinitialize = false;
#ifdef USE_CPU_TYPE
				reinitialize |= (cpu_type != config.cpu_type);
				cpu_type = config.cpu_type;
#endif
#ifdef USE_DIPSWITCH
				reinitialize |= (dipswitch != config.dipswitch);
				dipswitch = config.dipswitch;
#endif
#ifdef USE_SOUND_TYPE
				reinitialize |= (sound_type != config.sound_type);
				sound_type = config.sound_type;
#endif
#ifdef USE_PRINTER_TYPE
				reinitialize |= (printer_type != config.printer_type);
				printer_type = config.printer_type;
#endif
#ifdef USE_SERIAL_TYPE
				reinitialize |= (serial_type != config.serial_type);
				serial_type = config.serial_type;
#endif
				if(!(0 <= config.sound_frequency && config.sound_frequency < 8)) {
					config.sound_frequency = 6;	// default: 48KHz
				}
				if(!(0 <= config.sound_latency && config.sound_latency < 5)) {
					config.sound_latency = 1;	// default: 100msec
				}
				reinitialize |= (sound_frequency != config.sound_frequency);
				reinitialize |= (sound_latency != config.sound_latency);
				sound_frequency = config.sound_frequency;
				sound_latency = config.sound_latency;

				if(reinitialize) {
					// stop sound
					//osd->lock_vm();
					// reinitialize virtual machine
					osd->stop_sound();
					osd->vm = nullptr;
					osd->force_unlock_vm();
					delete vm;
					vm = new VM(this);
					osd->vm = vm;
					#if defined(_USE_QT)
					osd->vm_has_set();
					osd->reset_vm_node();
					osd->update_keyname_table();
					osd->reset_screen_buffer();
					#endif
					int presented_rate;
					int presented_samples;
					#if !defined(_USE_QT)
					sound_rate = sound_frequency_table[config.sound_frequency];
					sound_samples = (int)(sound_rate * sound_latency_table[config.sound_latency] + 0.5);
					#else
					if(using_flags.get() != nullptr) {
						sound_rate = using_flags->get_sound_sample_rate(config.sound_frequency);
						sound_samples = (int)(sound_rate * using_flags->get_sound_latency(config.sound_latency) + 0.5);
					} else {
						// Fallback
						sound_rate = 48000;
						sound_samples = (int)(sound_rate * 0.1 + 0.5); // 100mSec
					}
					#endif
					
					osd->initialize_sound(sound_rate, sound_samples, &presented_rate, &presented_samples);
					if((sound_rate != presented_rate) ||
					   (sound_samples != presented_samples)) {
						sound_rate = presented_rate;
						sound_samples = presented_samples;
					}
					__LIKELY_IF(vm != NULL) {
						vm->initialize_sound(sound_rate, sound_samples);
					#ifdef USE_SOUND_VOLUME
						for(int i = 0; i < USE_SOUND_VOLUME; i++) {
							vm->set_sound_device_volume(i, config.sound_volume_l[i], config.sound_volume_r[i]);
						}
					#endif
						restore_media();
						vm->reset();
					}
					//osd->unlock_vm();
				} else {
					// restore inserted medias
					restore_media();
				}
				// load vm state
				__LIKELY_IF(vm != NULL) {
					if(vm->process_state(fio, true)) {
						// check end of state
						result = (fio->FgetInt32_LE() == -1);
					}
				} else {
					result = false;
				}
			}
		}
		fio->Fclose();
	}
	osd->unlock_vm();
	delete fio;
	return result;
}

const _TCHAR *EMU::state_file_path(int num)
{
	return create_local_path(_T("%s.sta%d"), _T(CONFIG_NAME), num);
}
#endif
