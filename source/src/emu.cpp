/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 emulation i/f ]
*/

#if defined(_USE_QT)
#include <string>
#endif
#include "emu.h"
#include "vm/vm.h"
#include "fifo.h"
#include "fileio.h"

#ifndef FD_BASE_NUMBER
#define FD_BASE_NUMBER 1
#endif
#ifndef QD_BASE_NUMBER
#define QD_BASE_NUMBER 1
#endif

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------
#if defined(_USE_QT)
// Please permit at least them m(.. )m
//extern void get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
#include <string>
#endif

#if defined(_USE_QT)
EMU::EMU(class Ui_MainWindow *hwnd, GLDrawClass *hinst)
#elif defined(OSD_WIN32)
EMU::EMU(HWND hwnd, HINSTANCE hinst)
#else
EMU::EMU()
#endif
{
	message_count = 0;
	// store main window handle
#ifdef USE_FD1
	// initialize d88 file info
	memset(d88_file, 0, sizeof(d88_file));
#endif
#ifdef USE_BUBBLE1
	// initialize d88 file info
	memset(b77_file, 0, sizeof(b77_file));
#endif
	// load sound config
	static const int freq_table[8] = {
		2000, 4000, 8000, 11025, 22050, 44100,
#ifdef OVERRIDE_SOUND_FREQ_48000HZ
		OVERRIDE_SOUND_FREQ_48000HZ,
#else
		48000,
#endif
		96000,
	};
	static const double late_table[5] = {0.05, 0.1, 0.2, 0.3, 0.4};
	
	if(!(0 <= config.sound_frequency && config.sound_frequency < 8)) {
		config.sound_frequency = 6;	// default: 48KHz
	}
	if(!(0 <= config.sound_latency && config.sound_latency < 5)) {
		config.sound_latency = 1;	// default: 100msec
	}
	sound_rate = freq_table[config.sound_frequency];
	sound_samples = (int)(sound_rate * late_table[config.sound_latency] + 0.5);

#ifdef USE_CPU_TYPE
	cpu_type = config.cpu_type;
#endif
#ifdef USE_SOUND_DEVICE_TYPE
	sound_device_type = config.sound_device_type;
#endif
#ifdef USE_PRINTER
	printer_device_type = config.printer_device_type;
#endif
	
	// initialize osd
	osd = new OSD();
#if defined(OSD_QT)
	osd->main_window_handle = hwnd;
	osd->glv = hinst;
	osd->host_cpus = 4;
#elif defined(OSD_WIN32)
	osd->main_window_handle = hwnd;
	osd->instance_handle = hinst;
#endif
	osd->initialize(sound_rate, sound_samples);
	// initialize vm
	osd->vm = vm = new VM(this);
	// Below is temporally workaround. I will fix ASAP (or give up): 20160311 K.Ohta
	// Problems seem to be resolved. See fm7.cpp. 20160319 K.Ohta
	// Still not resolved with FM-7/77 :-( 20160407 K.Ohta
#if defined(_FM7) || defined(_FMNEW7) || defined(_FM8) || \
	defined(_FM77_VARIANTS)
	delete vm;
	osd->vm = vm = new VM(this);
#endif
#ifdef USE_AUTO_KEY
	initialize_auto_key();
#endif
#ifdef USE_DEBUGGER
	initialize_debugger();
#endif
	initialize_media();
	vm->initialize_sound(sound_rate, sound_samples);
#ifdef USE_SOUND_VOLUME
	for(int i = 0; i < USE_SOUND_VOLUME; i++) {
		vm->set_sound_device_volume(i, config.sound_volume_l[i], config.sound_volume_r[i]);
	}
#endif
	vm->reset();
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
	delete vm;
	osd->release();
	delete osd;
#ifdef _DEBUG_LOG
	release_debug_log();
#endif
}

#ifdef OSD_QT
EmuThreadClass *EMU::get_parent_handler()
{
	return osd->get_parent_handler();
}

void EMU::set_parent_handler(EmuThreadClass *p, DrawThreadClass *q)
{
	osd->set_parent_thread(p);
	osd->set_draw_thread(q);
}

void EMU::set_host_cpus(int v)
{
	osd->host_cpus = (v <= 0) ? 1 : v;
}

int EMU::get_host_cpus()
{
	return osd->host_cpus;
}
#endif

// ----------------------------------------------------------------------------
// drive machine
// ----------------------------------------------------------------------------

int EMU::get_frame_interval()
{
#ifdef SUPPORT_VARIABLE_TIMING
	static int prev_interval = 0;
	static double prev_fps = -1;
	double fps = vm->get_frame_rate();
	if(prev_fps != fps) {
		prev_interval = (int)(1024. * 1000. / fps + 0.5);
		prev_fps = fps;
	}
	return prev_interval;
#else
	return (int)(1024. * 1000. / FRAMES_PER_SEC + 0.5);
#endif
}

bool EMU::is_frame_skippable()
{
	return vm->is_frame_skippable();
}

int EMU::run()
{
	if(now_suspended) {
		osd->restore();
		now_suspended = false;
	}
	osd->update_input();
#ifdef USE_AUTO_KEY
	update_auto_key();
#endif
#ifdef USE_JOYSTICK
	update_joystick();
#endif
	
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
		extra_frames = 1;
		osd->unlock_vm();
	}
	osd->add_extra_frames(extra_frames);
	return extra_frames;
}

void EMU::reset()
{
	// check if virtual machine should be reinitialized
	bool reinitialize = false;
#ifdef USE_CPU_TYPE
	reinitialize |= (cpu_type != config.cpu_type);
	cpu_type = config.cpu_type;
#endif
#ifdef USE_SOUND_DEVICE_TYPE
	reinitialize |= (sound_device_type != config.sound_device_type);
	sound_device_type = config.sound_device_type;
#endif
#ifdef USE_PRINTER
	reinitialize |= (printer_device_type != config.printer_device_type);
	printer_device_type = config.printer_device_type;
#endif
	if(reinitialize) {
		// stop sound
		osd->stop_sound();
		// reinitialize virtual machine
		osd->lock_vm();		
		delete vm;
		osd->vm = vm = new VM(this);
		vm->initialize_sound(sound_rate, sound_samples);
#ifdef USE_SOUND_VOLUME
		for(int i = 0; i < USE_SOUND_VOLUME; i++) {
			vm->set_sound_device_volume(i, config.sound_volume_l[i], config.sound_volume_r[i]);
		}
#endif
		vm->reset();
		osd->unlock_vm();
		// restore inserted medias
		restore_media();
	} else {
		// reset virtual machine
		osd->lock_vm();		
		vm->reset();
		osd->unlock_vm();		
	}
	
	// restart recording
#if !defined(_USE_QT) // Temporally
	osd->restart_record_sound();
	osd->restart_record_video();
#endif	
}

#ifdef USE_SPECIAL_RESET
void EMU::special_reset()
{
	// reset virtual machine
	osd->lock_vm();		
	vm->special_reset();
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
	vm->notify_power_off();
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

#ifdef OSD_QT
void EMU::key_modifiers(uint32_t mod)
{
	osd->key_modifiers(mod);
}

	# ifdef USE_MOUSE
void EMU::set_mouse_pointer(int x, int y)
{
	osd->set_mouse_pointer(x, y);
}

void EMU::set_mouse_button(int button)
{
	osd->set_mouse_button(button);
}

int EMU::get_mouse_button()
{
	return osd->get_mouse_button();
}
	#endif
#endif

void EMU::key_down(int code, bool repeat)
{
	osd->key_down(code, repeat);
}

void EMU::key_up(int code)
{
	osd->key_up(code);
}

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
		vm->reset();
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
void EMU::initialize_auto_key()
{
	auto_key_buffer = new FIFO(65536);
	auto_key_buffer->clear();
	auto_key_phase = auto_key_shift = 0;
	osd->now_auto_key = false;
}

void EMU::release_auto_key()
{
	if(auto_key_buffer) {
		auto_key_buffer->release();
		delete auto_key_buffer;
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
			osd->key_down_native(auto_key_buffer->read_not_remove(0) & 0xff, false);
		}
		auto_key_phase++;
		break;
	case USE_AUTO_KEY + USE_AUTO_KEY_SHIFT:
		if(auto_key_buffer && !auto_key_buffer->empty()) {
			osd->key_up_native(auto_key_buffer->read_not_remove(0) & 0xff);
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
	uint32_t *joy_buffer = osd->get_joy_buffer();
	uint8_t *key_buffer = osd->get_key_buffer();
	
	memset(joy_status, 0, sizeof(joy_status));
	
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 16; j++) {
			if(config.joy_buttons[i][j] < 0) {
				int code = -config.joy_buttons[i][j];
				if(code < 256 && key_buffer[code]) {
					joy_status[i] |= (1 << j);
				}
			} else {
				int stick = config.joy_buttons[i][j] >> 4;
				int button = config.joy_buttons[i][j] & 15;
				if(stick < 2 && (joy_buffer[stick & 3] & (1 << button))) {
					joy_status[i] |= (1 << j);
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
	return (const uint32_t*)joy_status;
}
#endif

#ifdef USE_MOUSE
const int32_t* EMU::get_mouse_buffer()
{
	return (const int32_t*)osd->get_mouse_buffer();
}
#endif

// ----------------------------------------------------------------------------
// screen
// ----------------------------------------------------------------------------

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
	return vm->is_screen_changed();
}
#endif

int EMU::draw_screen()
{
	return osd->draw_screen();
}

scrntype_t* EMU::get_screen_buffer(int y)
{
	return osd->get_vm_screen_buffer(y);
}

#ifdef USE_CRT_FILTER
void EMU::screen_skip_line(bool skip_line)
{
	osd->screen_skip_line = skip_line;
}
#endif

#ifdef ONE_BOARD_MICRO_COMPUTER
void EMU::reload_bitmap()
{
	osd->reload_bitmap();
}
#endif

#ifdef OSD_WIN32
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
int EMU::get_socket(int ch)
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
static _TCHAR prev_buffer[1024] = {0};
#endif

void EMU::out_debug_log(const _TCHAR* format, ...)
{
#ifdef _DEBUG_LOG
	va_list ap;
	_TCHAR buffer[1024];
	
	va_start(ap, format);
	my_vstprintf_s(buffer, 1024, format, ap);
	va_end(ap);
	
	if(_tcscmp(prev_buffer, buffer) == 0) {
		return;
	}
	my_tcscpy_s(prev_buffer, 1024, buffer);
	
#if defined(_USE_QT) || defined(_USE_AGAR) || defined(_USE_SDL)
	AGAR_DebugLog(AGAR_LOG_DEBUG, "%s", buffer);
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
	AGAR_DebugLog(AGAR_LOG_DEBUG, "%s", buffer);
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
	va_list ap;
	va_start(ap, format);
	my_vstprintf_s(message, 1024, format, ap); // Security for MSVC:C6386.
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
				if(offset + i < sizeof(buffer)) {
					if(length < offset + i) {
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
#ifdef USE_CART1
	memset(&cart_status, 0, sizeof(cart_status));
#endif
#ifdef USE_FD1
	memset(floppy_disk_status, 0, sizeof(floppy_disk_status));
#endif
#ifdef USE_QD1
	memset(&quick_disk_status, 0, sizeof(quick_disk_status));
#endif
#ifdef USE_TAPE
	memset(&tape_status, 0, sizeof(tape_status));
#endif
#ifdef USE_COMPACT_DISC
	memset(&compact_disc_status, 0, sizeof(compact_disc_status));
#endif
#ifdef USE_LASER_DISC
	memset(&laser_disc_status, 0, sizeof(laser_disc_status));
#endif
}


void EMU::update_media()
{
#ifdef USE_FD1
	for(int drv = 0; drv < MAX_FD; drv++) {
		if(floppy_disk_status[drv].wait_count != 0 && --floppy_disk_status[drv].wait_count == 0) {
			vm->open_floppy_disk(drv, floppy_disk_status[drv].path, floppy_disk_status[drv].bank);
			out_message(_T("FD%d: %s"), drv + FD_BASE_NUMBER, floppy_disk_status[drv].path);
		}
	}
#endif
#ifdef USE_QD1
	for(int drv = 0; drv < MAX_QD; drv++) {
		if(quick_disk_status[drv].wait_count != 0 && --quick_disk_status[drv].wait_count == 0) {
			vm->open_quick_disk(drv, quick_disk_status[drv].path);
			out_message(_T("QD%d: %s"), drv + QD_BASE_NUMBER, quick_disk_status[drv].path);
		}
	}
#endif
#ifdef USE_TAPE
	if(tape_status.wait_count != 0 && --tape_status.wait_count == 0) {
		if(tape_status.play) {
			vm->play_tape(tape_status.path);
		} else {
			vm->rec_tape(tape_status.path);
		}
		out_message(_T("CMT: %s"), tape_status.path);
	}
#endif
#ifdef USE_COMPACT_DISC
	if(compact_disc_status.wait_count != 0 && --compact_disc_status.wait_count == 0) {
		vm->open_compact_disc(compact_disc_status.path);
		out_message(_T("CD: %s"), compact_disc_status.path);
	}
#endif
#ifdef USE_LASER_DISC
	if(laser_disc_status.wait_count != 0 && --laser_disc_status.wait_count == 0) {
		vm->open_laser_disc(laser_disc_status.path);
		out_message(_T("LD: %s"), laser_disc_status.path);
	}
#endif
#ifdef USE_BUBBLE1
	for(int drv = 0; drv < MAX_BUBBLE; drv++) {
		if(bubble_casette_status[drv].wait_count != 0 && --bubble_casette_status[drv].wait_count == 0) {
			vm->open_bubble_casette(drv, bubble_casette_status[drv].path, bubble_casette_status[drv].bank);
			out_message(_T("Bubble%d: %s"), drv, bubble_casette_status[drv].path);
		}
	}
#endif
}

void EMU::restore_media()
{
#ifdef USE_CART1
	for(int drv = 0; drv < MAX_CART; drv++) {
		if(cart_status[drv].path[0] != _T('\0')) {
			if(check_file_extension(cart_status[drv].path, _T(".hex")) && hex2bin(cart_status[drv].path, create_local_path(_T("hex2bin.$$$")))) {
				vm->open_cart(drv, create_local_path(_T("hex2bin.$$$")));
				FILEIO::RemoveFile(create_local_path(_T("hex2bin.$$$")));
			} else {
				vm->open_cart(drv, cart_status[drv].path);
			}
		}
	}
#endif
#ifdef USE_FD1
	for(int drv = 0; drv < MAX_FD; drv++) {
		if(floppy_disk_status[drv].path[0] != _T('\0')) {
			vm->open_floppy_disk(drv, floppy_disk_status[drv].path, floppy_disk_status[drv].bank);
		}
	}
#endif
#ifdef USE_QD1
	for(int drv = 0; drv < MAX_QD; drv++) {
		if(quick_disk_status[drv].path[0] != _T('\0')) {
			vm->open_quick_disk(drv, quick_disk_status[drv].path);
		}
	}
#endif
#ifdef USE_TAPE
	if(tape_status.path[0] != _T('\0')) {
		if(tape_status.play) {
			vm->play_tape(tape_status.path);
		} else {
			tape_status.path[0] = _T('\0');
		}
	}
#endif
#ifdef USE_COMPACT_DISC
	if(compact_disc_status.path[0] != _T('\0')) {
		vm->open_compact_disc(compact_disc_status.path);
	}
#endif
#ifdef USE_LASER_DISC
	if(laser_disc_status.path[0] != _T('\0')) {
		vm->open_laser_disc(laser_disc_status.path);
	}
#endif
#ifdef USE_BUBBLE1
	for(int drv = 0; drv < MAX_BUBBLE; drv++) {
		if(bubble_casette_status[drv].path[0] != _T('\0')) {
			vm->open_bubble_casette(drv, bubble_casette_status[drv].path, bubble_casette_status[drv].bank);
		}
	}
#endif
}

#ifdef USE_CART1
void EMU::open_cart(int drv, const _TCHAR* file_path)
{
	if(drv < MAX_CART) {
		if(check_file_extension(file_path, _T(".hex")) && hex2bin(file_path, create_local_path(_T("hex2bin.$$$")))) {
			vm->open_cart(drv, create_local_path(_T("hex2bin.$$$")));
			FILEIO::RemoveFile(create_local_path(_T("hex2bin.$$$")));
		} else {
			vm->open_cart(drv, file_path);
		}
		my_tcscpy_s(cart_status[drv].path, _MAX_PATH, file_path);
		out_message(_T("Cart%d: %s"), drv + 1, file_path);
		
		// restart recording
		bool s = osd->now_record_sound;
		bool v = osd->now_record_video;
		stop_record_sound();
		stop_record_video();

		if(s) osd->start_record_sound();
		if(v) osd->start_record_video(-1);
	}
}

void EMU::close_cart(int drv)
{
	if(drv < MAX_CART) {
		vm->close_cart(drv);
		clear_media_status(&cart_status[drv]);
		out_message(_T("Cart%d: Ejected"), drv + 1);
		
		// stop recording
		stop_record_video();
		stop_record_sound();
	}
}

bool EMU::is_cart_inserted(int drv)
{
	if(drv < MAX_CART) {
		return vm->is_cart_inserted(drv);
	} else {
		return false;
	}
}
#endif

#ifdef USE_FD1
void EMU::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_FD) {
		if(vm->is_floppy_disk_inserted(drv)) {
			vm->close_floppy_disk(drv);
			// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
			floppy_disk_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#else
			floppy_disk_status[drv].wait_count = (int)(FRAMES_PER_SEC / 2);
#endif
			out_message(_T("FD%d: Ejected"), drv + FD_BASE_NUMBER);
		} else if(floppy_disk_status[drv].wait_count == 0) {
			vm->open_floppy_disk(drv, file_path, bank);
			out_message(_T("FD%d: %s"), drv + FD_BASE_NUMBER, file_path);
		}
		my_tcscpy_s(floppy_disk_status[drv].path, _MAX_PATH, file_path);
		floppy_disk_status[drv].bank = bank;
	}
}

void EMU::close_floppy_disk(int drv)
{
	if(drv < MAX_FD) {
		vm->close_floppy_disk(drv);
		clear_media_status(&floppy_disk_status[drv]);
		out_message(_T("FD%d: Ejected"), drv + FD_BASE_NUMBER);
	}
}

bool EMU::is_floppy_disk_inserted(int drv)
{
	if(drv < MAX_FD) {
		return vm->is_floppy_disk_inserted(drv);
	} else {
		return false;
	}
}

void EMU::is_floppy_disk_protected(int drv, bool value)
{
	if(drv < MAX_FD) {
		vm->is_floppy_disk_protected(drv, value);
	}
}

bool EMU::is_floppy_disk_protected(int drv)
{
	if(drv < MAX_FD) {
		return vm->is_floppy_disk_protected(drv);
	} else {
		return false;
	}
}
#endif

#ifdef USE_QD1
void EMU::open_quick_disk(int drv, const _TCHAR* file_path)
{
	if(drv < MAX_QD) {
		if(vm->is_quick_disk_inserted(drv)) {
			vm->close_quick_disk(drv);
			// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
			quick_disk_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#else
			quick_disk_status[drv].wait_count = (int)(FRAMES_PER_SEC / 2);
#endif
			out_message(_T("QD%d: Ejected"), drv + QD_BASE_NUMBER);
		} else if(quick_disk_status[drv].wait_count == 0) {
			vm->open_quick_disk(drv, file_path);
			out_message(_T("QD%d: %s"), drv + QD_BASE_NUMBER, file_path);
		}
		my_tcscpy_s(quick_disk_status[drv].path, _MAX_PATH, file_path);
	}
}

void EMU::close_quick_disk(int drv)
{
	if(drv < MAX_QD) {
		vm->close_quick_disk(drv);
		clear_media_status(&quick_disk_status[drv]);
		out_message(_T("QD%d: Ejected"), drv + QD_BASE_NUMBER);
	}
}

bool EMU::is_quick_disk_inserted(int drv)
{
	if(drv < MAX_QD) {
		return vm->is_quick_disk_inserted(drv);
	} else {
		return false;
	}
}
#endif

#ifdef USE_TAPE
void EMU::play_tape(const _TCHAR* file_path)
{
	if(vm->is_tape_inserted()) {
		vm->close_tape();
		// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
		tape_status.wait_count = (int)(vm->get_frame_rate() / 2);
#else
		tape_status.wait_count = (int)(FRAMES_PER_SEC / 2);
#endif
		out_message(_T("CMT: Ejected"));
	} else if(tape_status.wait_count == 0) {
		vm->play_tape(file_path);
		out_message(_T("CMT: %s"), file_path);
	}
	my_tcscpy_s(tape_status.path, _MAX_PATH, file_path);
	tape_status.play = true;
}

void EMU::rec_tape(const _TCHAR* file_path)
{
	if(vm->is_tape_inserted()) {
		vm->close_tape();
		// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
		tape_status.wait_count = (int)(vm->get_frame_rate() / 2);
#else
		tape_status.wait_count = (int)(FRAMES_PER_SEC / 2);
#endif
		out_message(_T("CMT: Ejected"));
	} else if(tape_status.wait_count == 0) {
		vm->rec_tape(file_path);
		out_message(_T("CMT: %s"), file_path);
	}
	my_tcscpy_s(tape_status.path, _MAX_PATH, file_path);
	tape_status.play = false;
}

void EMU::close_tape()
{
	vm->close_tape();
	clear_media_status(&tape_status);
	out_message(_T("CMT: Ejected"));
}

bool EMU::is_tape_inserted()
{
	return vm->is_tape_inserted();
}

#ifndef TAPE_BINARY_ONLY
bool EMU::is_tape_playing()
{
	return vm->is_tape_playing();
}

bool EMU::is_tape_recording()
{
	return vm->is_tape_recording();
}

int EMU::get_tape_position()
{
	return vm->get_tape_position();
}
#endif

#ifdef USE_TAPE_BUTTON
void EMU::push_play()
{
	vm->push_play();
}

void EMU::push_stop()
{
	vm->push_stop();
}

void EMU::push_fast_forward()
{
	vm->push_fast_forward();
}

void EMU::push_fast_rewind()
{
	vm->push_fast_rewind();
}

void EMU::push_apss_forward()
{
	vm->push_apss_forward();
}

void EMU::push_apss_rewind()
{
	vm->push_apss_rewind();
}
#endif
#endif

#ifdef USE_COMPACT_DISC
void EMU::open_compact_disc(const _TCHAR* file_path)
{
	if(vm->is_compact_disc_inserted()) {
		vm->close_compact_disc();
		// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
		compact_disc_status.wait_count = (int)(vm->get_frame_rate() / 2);
#else
		compact_disc_status.wait_count = (int)(FRAMES_PER_SEC / 2);
#endif
		out_message(_T("CD: Ejected"));
	} else if(compact_disc_status.wait_count == 0) {
		vm->open_compact_disc(file_path);
		out_message(_T("CD: %s"), file_path);
	}
	my_tcscpy_s(compact_disc_status.path, _MAX_PATH, file_path);
}

void EMU::close_compact_disc()
{
	vm->close_compact_disc();
	clear_media_status(&compact_disc_status);
	out_message(_T("CD: Ejected"));
}

bool EMU::is_compact_disc_inserted()
{
	return vm->is_compact_disc_inserted();
}
#endif

#ifdef USE_LASER_DISC
void EMU::open_laser_disc(const _TCHAR* file_path)
{
	if(vm->is_laser_disc_inserted()) {
		vm->close_laser_disc();
		// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
		laser_disc_status.wait_count = (int)(vm->get_frame_rate() / 2);
#else
		laser_disc_status.wait_count = (int)(FRAMES_PER_SEC / 2);
#endif
		out_message(_T("LD: Ejected"));
	} else if(laser_disc_status.wait_count == 0) {
		vm->open_laser_disc(file_path);
		out_message(_T("LD: %s"), file_path);
	}
	my_tcscpy_s(laser_disc_status.path, _MAX_PATH, file_path);
}

void EMU::close_laser_disc()
{
	vm->close_laser_disc();
	clear_media_status(&laser_disc_status);
	out_message(_T("LD: Ejected"));
}

bool EMU::is_laser_disc_inserted()
{
	return vm->is_laser_disc_inserted();
}
#endif

#ifdef USE_BINARY_FILE1
void EMU::load_binary(int drv, const _TCHAR* file_path)
{
	if(drv < MAX_BINARY) {
		if(check_file_extension(file_path, _T(".hex")) && hex2bin(file_path, create_local_path(_T("hex2bin.$$$")))) {
			vm->load_binary(drv, create_local_path(_T("hex2bin.$$$")));
			FILEIO::RemoveFile(create_local_path(_T("hex2bin.$$$")));
		} else {
			vm->load_binary(drv, file_path);
		}
		out_message(_T("Load: %s"), file_path);
	}
}

void EMU::save_binary(int drv, const _TCHAR* file_path)
{
	if(drv < MAX_BINARY) {
		vm->save_binary(drv, file_path);
		out_message(_T("Save: %s"), file_path);
	}
}
#endif
#ifdef USE_BUBBLE1
void EMU::open_bubble_casette(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_BUBBLE) {
		if(vm->is_bubble_casette_inserted(drv)) {
			vm->close_bubble_casette(drv);
			// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
			bubble_casette_status[drv].wait_count = (int)(vm->get_frame_rate() / 2);
#else
			bubble_casette_status[drv].wait_count = (int)(FRAMES_PER_SEC / 2);
#endif
			out_message(_T("Bubble%d: Ejected"), drv + 1);
		} else if(bubble_casette_status[drv].wait_count == 0) {
			vm->open_bubble_casette(drv, file_path, bank);
			out_message(_T("Bubble%d: %s"), drv + 1, file_path);
		}
		my_tcscpy_s(bubble_casette_status[drv].path, _MAX_PATH, file_path);
		bubble_casette_status[drv].bank = bank;
	}
}

void EMU::close_bubble_casette(int drv)
{
	if(drv < MAX_BUBBLE) {
		vm->close_bubble_casette(drv);
		clear_media_status(&bubble_casette_status[drv]);
		out_message(_T("Bubble%d: Ejected"), drv + 1);
	}
}

bool EMU::is_bubble_casette_inserted(int drv)
{
	if(drv < MAX_BUBBLE) {
		return vm->is_bubble_casette_inserted(drv);
	} else {
		return false;
	}
}

bool EMU::is_bubble_casette_protected(int drv)
{
	if(drv < MAX_BUBBLE) {
		return vm->is_bubble_casette_protected(drv);
	} else {
		return false;
	}
}

void EMU::is_bubble_casette_protected(int drv, bool flag)
{
	if(drv < MAX_BUBBLE) {
		return vm->is_bubble_casette_protected(drv, flag);
	} else {
		return false;
	}
}
#endif

#ifdef USE_ACCESS_LAMP
uint32_t EMU::get_access_lamp_status()
{
	return vm->get_access_lamp_status();
}
#endif

#ifdef USE_LED_DEVICE
uint32_t EMU::get_led_status()
{
	return vm->get_led_status();
}
#endif

#ifdef USE_SOUND_VOLUME
void EMU::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	vm->set_sound_device_volume(ch, decibel_l, decibel_r);
}
#endif

void EMU::update_config()
{
	vm->update_config();
}



// ----------------------------------------------------------------------------
// state
// ----------------------------------------------------------------------------

#ifdef USE_STATE
#define STATE_VERSION	2

void EMU::save_state()
{
	save_state_tmp(create_local_path(_T("%s.sta"), _T(CONFIG_NAME)));
}

void EMU::load_state()
{
	if(FILEIO::IsFileExisting(create_local_path(_T("%s.sta"), _T(CONFIG_NAME)))) {
		save_state_tmp(create_local_path(_T("$temp$.sta")));
		if(!load_state_tmp(create_local_path(_T("%s.sta"), _T(CONFIG_NAME)))) {
			out_debug_log(_T("failed to load state file\n"));
			load_state_tmp(create_local_path(_T("$temp$.sta")));
		}
		FILEIO::RemoveFile(create_local_path(_T("$temp$.sta")));
	}
}

void EMU::save_state_tmp(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	osd->lock_vm();
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		// save state file version
		fio->FputUint32(STATE_VERSION);
		// save config
		save_config_state((void *)fio);
		// save inserted medias
#ifdef USE_CART1
		fio->Fwrite(&cart_status, sizeof(cart_status), 1);
#endif
#ifdef USE_FD1
		fio->Fwrite(floppy_disk_status, sizeof(floppy_disk_status), 1);
		fio->Fwrite(d88_file, sizeof(d88_file), 1);
#endif
#ifdef USE_QD1
		fio->Fwrite(&quick_disk_status, sizeof(quick_disk_status), 1);
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
		// save vm state
		vm->save_state(fio);
		// end of state file
		fio->FputInt32(-1);
		fio->Fclose();
	}
	osd->unlock_vm();
	delete fio;
}

bool EMU::load_state_tmp(const _TCHAR* file_path)
{
	bool result = false;
	FILEIO* fio = new FILEIO();
	osd->lock_vm();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		// check state file version
		if(fio->FgetUint32() == STATE_VERSION) {
			// load config
			if(load_config_state((void *)fio)) {
				// load inserted medias
#ifdef USE_CART1
				fio->Fread(&cart_status, sizeof(cart_status), 1);
#endif
#ifdef USE_FD1
				fio->Fread(floppy_disk_status, sizeof(floppy_disk_status), 1);
				fio->Fread(d88_file, sizeof(d88_file), 1);
#endif
#ifdef USE_QD1
				fio->Fread(&quick_disk_status, sizeof(quick_disk_status), 1);
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
				// check if virtual machine should be reinitialized
				bool reinitialize = false;
#ifdef USE_CPU_TYPE
				reinitialize |= (cpu_type != config.cpu_type);
				cpu_type = config.cpu_type;
#endif
#ifdef USE_SOUND_DEVICE_TYPE
				reinitialize |= (sound_device_type != config.sound_device_type);
				sound_device_type = config.sound_device_type;
#endif
#ifdef USE_PRINTER
				reinitialize |= (printer_device_type != config.printer_device_type);
				printer_device_type = config.printer_device_type;
#endif
				if(reinitialize) {
					// stop sound
					//osd->lock_vm();
					// reinitialize virtual machine
					osd->stop_sound();
					delete vm;
					osd->vm = vm = new VM(this);
					vm->initialize_sound(sound_rate, sound_samples);
#ifdef USE_SOUND_VOLUME
					for(int i = 0; i < USE_SOUND_VOLUME; i++) {
						vm->set_sound_device_volume(i, config.sound_volume_l[i], config.sound_volume_r[i]);
					}
#endif
					vm->reset();
					//osd->unlock_vm();
				}
				// restore inserted medias
				restore_media();
				// load vm state
				if(vm->load_state(fio)) {
					// check end of state
					result = (fio->FgetInt32() == -1);
				}
			}
		}
		fio->Fclose();
	}
	osd->unlock_vm();
	delete fio;
	return result;
}
#endif
