/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 emulation i/f ]
*/

#include "emu.h"
#include "vm/vm.h"
#include "fileio.h"
#if defined(_USE_AGAR)
#include <SDL/SDL.h>
#include "agar_main.h"
#include "agar_logger.h"
#include <ctime>
# elif defined(_USE_QT)
//#include <SDL/SDL.h>

#include "qt_main.h"
#include "agar_logger.h"
#include <ctime>
# endif

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
extern void get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
#include <string>
#endif

#if defined(_USE_QT)
EMU::EMU(Ui_MainWindow *hwnd, GLDrawClass *hinst)
#elif defined(OSD_WIN32)
EMU::EMU(HWND hwnd, HINSTANCE hinst)
#else
EMU::EMU()
#endif
{
#ifdef _DEBUG_LOG
	initialize_debug_log();
#endif
	message_count = 0;
	
	// store main window handle
#if !defined(_USE_QT)
	// check os version
	OSVERSIONINFO os_info;
	os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&os_info);
	vista_or_later = (os_info.dwPlatformId == 2 && os_info.dwMajorVersion >= 6);
#endif	
	// get module path
	// Initialize keymod.
	modkey_status = 0;
#if defined(_USE_QT)
	std::string tmps;
	_TCHAR tmp_path[PATH_MAX], *ptr;
	my_procname.copy(tmp_path, PATH_MAX, 0);
	memset(app_path, 0x00, sizeof(app_path));
	get_long_full_path_name(tmp_path, app_path);
	//AGAR_DebugLog("APPPATH=%s\n", app_path);
	use_opengl = true;
	use_opencl = false;
	VMSemaphore = new QMutex(QMutex::Recursive);
	host_cpus = 4;
#else
	_TCHAR tmp_path[_MAX_PATH], *ptr;
	memset(tmp_path, 0x00, _MAX_PATH);
	GetModuleFileName(NULL, tmp_path, _MAX_PATH);
	GetFullPathName(tmp_path, _MAX_PATH, app_path, &ptr);
	*ptr = _T('\0');
#endif	
#ifdef USE_FD1
		// initialize d88 file info
	memset(d88_file, 0, sizeof(d88_file));
#endif
#ifdef USE_AUTO_KEY
	memset(auto_key_str, 0x00, sizeof(auto_key_str));
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
	osd = new OSD();
#if defined(OSD_WIN32) || defined(_USE_QT)
	osd->main_window_handle = hwnd;
	osd->instance_handle = hinst;
#endif	
	osd->initialize(sound_rate, sound_samples);
	osd->vm = vm = new VM(this);
	
#ifdef USE_DEBUGGER
	initialize_debugger();
#endif
	initialize_media();
	vm->initialize_sound(sound_rate, sound_samples);
	vm->reset();
	now_suspended = false;
}

EMU::~EMU()
{
#ifdef USE_DEBUGGER
	release_debugger();
#endif
	osd->release();
	delete osd;
 	delete vm;

#ifdef _DEBUG_LOG
	release_debug_log();
#endif
#if defined(_USE_AGAR)
	if(pVMSemaphore) SDL_DestroySemaphore(pVMSemaphore);
#elif defined(_USE_QT)
	delete VMSemaphore;
#endif
}

// ----------------------------------------------------------------------------
// drive machine
// ----------------------------------------------------------------------------

int EMU::frame_interval()
{
#if 1
#ifdef SUPPORT_VARIABLE_TIMING
	static int prev_interval = 0;
	static double prev_fps = -1;
	double fps = vm->frame_rate();
	if(prev_fps != fps) {
		prev_interval = (int)(1024. * 1000. / fps + 0.5);
		prev_fps = fps;
	}
	return prev_interval;
#else
	return (int)(1024. * 1000. / FRAMES_PER_SEC + 0.5);
#endif
#else
        return (int)(1024. * 1000. / FRAMES_PER_SEC + 0.5);
#endif
}

int EMU::run()
{
	if(now_suspended) {
		osd->restore();
		now_suspended = false;
	}
	//LockVM();
	osd->update_input();
	osd->update_printer();
#ifdef USE_SOCKET
	//osd->update_socket();
#endif
	update_media();
	
	// virtual machine may be driven to fill sound buffer
	int extra_frames = 0;
	osd->update_sound(&extra_frames);
	
	// drive virtual machine
	if(extra_frames == 0) {
		vm->run();
		extra_frames = 1;
	}
	osd->add_extra_frames(extra_frames);
	//UnlockVM();
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
	if(reinitialize) {
		// stop sound
		osd->stop_sound();
		// reinitialize virtual machine
		//LockVM();		
		delete vm;
		osd->vm = vm = new VM(this);
		vm->initialize_sound(sound_rate, sound_samples);
		vm->reset();
		//UnlockVM();
		// restore inserted medias
		restore_media();
	} else {
	   // reset virtual machine
		vm->reset();
	}
	
	// reset printer
	osd->reset_printer();
	
	// restart recording
	osd->restart_rec_sound();
	osd->restart_rec_video();
}

#ifdef USE_SPECIAL_RESET
void EMU::special_reset()
{
	// reset virtual machine
	vm->special_reset();
	
	// reset printer
	osd->reset_printer();
	
	// restart recording
	restart_rec_sound();
	restart_rec_video();
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


_TCHAR* EMU::bios_path(const _TCHAR* file_name)
{
 	static _TCHAR file_path[_MAX_PATH];
	memset(file_path, 0x00, sizeof(file_path));
	_stprintf_s(file_path, _MAX_PATH, _T("%s%s"), app_path, file_name);
#if defined(_USE_AGAR) || defined(_USE_SDL) || defined(_USE_QT)
	AGAR_DebugLog(AGAR_LOG_INFO, "BIOS: %s", file_path);
#endif
 	return file_path;
//#if defined(_USE_AGAR) || defined(_USE_SDL) || defined(_USE_QT)
//        static _TCHAR file_path[_MAX_PATH];
//        strcpy(file_path, app_path);
//        strcat(file_path, file_name);
//#if defined(_USE_AGAR) || defined(_USE_SDL) || defined(_USE_QT)
//        AGAR_DebugLog(AGAR_LOG_INFO, "LOAD BIOS: %s\n", file_path);
//#endif
//#else
//        static _TCHAR file_path[_MAX_PATH];
//	_stprintf(file_path, _T("%s%s"), app_path, file_name);
//        printf("LOAD: %s\n", file_path);
//#endif
//	return file_path;
}

void EMU::suspend()
{
	if(!now_suspended) {
		osd->suspend();
		now_suspended = true;
	}
}

// ----------------------------------------------------------------------------
// input
// ----------------------------------------------------------------------------
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
	osd->press_button(num);
}
#endif

void EMU::enable_mouse()
{
	osd->enable_mouse();
}

void EMU::disenable_mouse()
{
	osd->disenable_mouse();
}

void EMU::toggle_mouse()
{
	osd->toggle_mouse();
}

bool EMU::get_mouse_enabled()
{
	return osd->get_mouse_enabled();
}

#ifdef USE_AUTO_KEY
void EMU::start_auto_key()
{
	osd->start_auto_key();
}

void EMU::stop_auto_key()
{
	osd->stop_auto_key();
}

bool EMU::now_auto_key()
{
	return osd->now_auto_key();
}
#endif

uint8* EMU::key_buffer()
{
	return osd->key_buffer();
}

uint32* EMU::joy_buffer()
{
	return osd->joy_buffer();
}
int* EMU::mouse_buffer()
{
	return osd->mouse_buffer();
}

// ----------------------------------------------------------------------------
// screen
// ----------------------------------------------------------------------------

int EMU::get_window_width(int mode)
{
	return osd->get_window_width(mode);
}

int EMU::get_window_height(int mode)
{
	return osd->get_window_height(mode);
}

void EMU::set_window_size(int width, int height, bool window_mode)
{
	osd->set_window_size(width, height, window_mode);
}

void EMU::set_vm_screen_size(int sw, int sh, int swa, int sha, int ww, int wh)
{
	osd->set_vm_screen_size(sw, sh, swa, sha, ww, wh);
}

int EMU::draw_screen()
{
	return osd->draw_screen();
}

scrntype* EMU::screen_buffer(int y)
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

bool EMU::start_rec_video(int fps)
{
	return osd->start_rec_video(fps);
}

void EMU::stop_rec_video()
{
	osd->stop_rec_video();
}

bool EMU::now_rec_video()
{
	return osd->now_rec_video;
}

// ----------------------------------------------------------------------------
// sound
// ----------------------------------------------------------------------------

void EMU::mute_sound()
{
	osd->mute_sound();
}

void EMU::start_rec_sound()
{
	osd->start_rec_sound();
}

void EMU::stop_rec_sound()
{
	osd->stop_rec_sound();
}

bool EMU::now_rec_sound()
{
	return osd->now_rec_sound;
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

uint32 EMU::get_cur_movie_frame()
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
// socket
// ----------------------------------------------------------------------------

#ifdef USE_SOCKET
int EMU::get_socket(int ch)
{
	return osd->get_socket(ch);
}

void EMU::socket_connected(int ch)
{
	osd->socket_connected(ch);
}

void EMU::socket_disconnected(int ch)
{
	osd->socket_disconnected(ch);
}

bool EMU::init_socket_tcp(int ch)
{
	return osd->init_socket_tcp(ch);
}

bool EMU::init_socket_udp(int ch)
{
	return osd->init_socket_udp(ch);
}

bool EMU::connect_socket(int ch, uint32 ipaddr, int port)
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

void EMU::send_data_tcp(int ch)
{
	osd->send_data_tcp(ch);
}

void EMU::send_data_udp(int ch, uint32 ipaddr, int port)
{
	osd->send_data_udp(ch, ipaddr, port);
}

void EMU::send_data(int ch)
{
	osd->send_data(ch);
}

void EMU::recv_data(int ch)
{
	osd->recv_data(ch);
}
#endif

// ----------------------------------------------------------------------------
// debug log
// ----------------------------------------------------------------------------

#ifdef _DEBUG_LOG
void EMU::initialize_debug_log()
{
#if defined(_USE_QT) || defined(_USE_AGAR) || defined(_USE_SDL)
	
#else // Window
	TCHAR path[_MAX_PATH];
	osd->create_date_file_name(path, _MAX_PATH, _T("log"));
	debug_log = _tfopen(path, _T("w"));
#endif
}

void EMU::release_debug_log()
{
#if defined(_USE_QT) || defined(_USE_AGAR) || defined(_USE_SDL)

#else
	if(debug_log) {
		fclose(debug_log);
		debug_log = NULL;
	}
#endif
}
#endif

void EMU::out_debug_log(const _TCHAR* format, ...)
{
#ifdef _DEBUG_LOG
	va_list ap;
	_TCHAR buffer[1024];
	static _TCHAR prev_buffer[1024] = {0};
	
	va_start(ap, format);
	my_vstprintf_s(buffer, 1024, format, ap);
	va_end(ap);

#if defined(_USE_QT) || defined(_USE_AGAR) || defined(_USE_SDL)
	AGAR_DebugLog(AGAR_LOG_DEBUG, "%s", buffer);
#else
	if(my_tcscmp(prev_buffer, buffer) == 0) {
		return;
	}
	my_tcscpy_s(prev_buffer, 1024, buffer);
	if(debug_log) {
		_ftprintf(debug_log, _T("%s"), buffer);
		static int size = 0;
		if((size += _tcslen(buffer)) > 0x8000000) { // 128MB
			TCHAR path[_MAX_PATH];
			osd->create_date_file_name(path, _MAX_PATH, _T("log"));
			fclose(debug_log);
			debug_log = _tfopen(path, _T("w"));
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
	my_vstprintf_s(message, 260, format, ap); // Security for MSVC:C6386.
	va_end(ap);
	message_count = 4; // 4sec
}

// ----------------------------------------------------------------------------
// misc
// ----------------------------------------------------------------------------

_TCHAR* EMU::application_path()
{
	return osd->application_path();
}

_TCHAR* EMU::bios_path(const _TCHAR* file_name)
{
	return osd->bios_path(file_name);
}

void EMU::sleep(uint32 ms)
{
	osd->sleep(ms);
}

void EMU::get_host_time(cur_time_t* time)
{
	osd->get_host_time(time);
}


// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

static uint8 hex2uint8(char *value)
{
	char tmp[3];
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, value, 2);
	return (uint8)strtoul(tmp, NULL, 16);
}

static uint16 hex2uint16(char *value)
{
	char tmp[5];
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, value, 4);
	return (uint16)strtoul(tmp, NULL, 16);
}

static bool hex2bin(const _TCHAR* file_path, const _TCHAR* dest_path)
{
	bool result = false;
	FILEIO *fio_s = new FILEIO();
	if(fio_s->Fopen(file_path, FILEIO_READ_BINARY)) {
		int length = 0;
		char line[1024];
		uint8 buffer[0x10000];
		memset(buffer, 0xff, sizeof(buffer));
		while(fio_s->Fgets(line, sizeof(line)) != NULL) {
			if(line[0] != ':') continue;
			int bytes = hex2uint8(line + 1);
			int offset = hex2uint16(line + 3);
			uint8 record_type = hex2uint8(line + 7);
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
	memset(disk_status, 0, sizeof(disk_status));
#endif
#ifdef USE_QD1
	memset(&quickdisk_status, 0, sizeof(quickdisk_status));
#endif
#ifdef USE_TAPE
	memset(&tape_status, 0, sizeof(tape_status));
#endif
#ifdef USE_LASER_DISC
	memset(&laser_disc_status, 0, sizeof(laser_disc_status));
#endif
}


void EMU::update_media()
{
#ifdef USE_FD1
	for(int drv = 0; drv < MAX_FD; drv++) {
		if(disk_status[drv].wait_count != 0 && --disk_status[drv].wait_count == 0) {
			vm->open_disk(drv, disk_status[drv].path, disk_status[drv].bank);
			out_message(_T("FD%d: %s"), drv + FD_BASE_NUMBER, disk_status[drv].path);
		}
	}
#endif
#ifdef USE_QD1
	for(int drv = 0; drv < MAX_QD; drv++) {
		if(quickdisk_status[drv].wait_count != 0 && --quickdisk_status[drv].wait_count == 0) {
			vm->open_quickdisk(drv, quickdisk_status[drv].path);
			out_message(_T("QD%d: %s"), drv + QD_BASE_NUMBER, quickdisk_status[drv].path);
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
#ifdef USE_LASER_DISC
	if(laser_disc_status.wait_count != 0 && --laser_disc_status.wait_count == 0) {
		vm->open_laser_disc(laser_disc_status.path);
		out_message(_T("LD: %s"), laser_disc_status.path);
	}
#endif
}

void EMU::restore_media()
{
#ifdef USE_CART1
	for(int drv = 0; drv < MAX_CART; drv++) {
		if(cart_status[drv].path[0] != _T('\0')) {
			if(check_file_extension(cart_status[drv].path, _T(".hex")) && hex2bin(cart_status[drv].path, osd->bios_path(_T("hex2bin.$$$")))) {
				vm->open_cart(drv, osd->bios_path(_T("hex2bin.$$$")));
				FILEIO::RemoveFile(osd->bios_path(_T("hex2bin.$$$")));
			} else {
				vm->open_cart(drv, cart_status[drv].path);
			}
		}
	}
#endif
#ifdef USE_FD1
	for(int drv = 0; drv < MAX_FD; drv++) {
		if(disk_status[drv].path[0] != _T('\0')) {
			vm->open_disk(drv, disk_status[drv].path, disk_status[drv].bank);
		}
	}
#endif
#ifdef USE_QD1
	for(int drv = 0; drv < MAX_QD; drv++) {
		if(quickdisk_status[drv].path[0] != _T('\0')) {
			vm->open_quickdisk(drv, quickdisk_status[drv].path);
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
#ifdef USE_LASER_DISC
	if(laser_disc_status.path[0] != _T('\0')) {
		vm->open_laser_disc(laser_disc_status.path);
	}
#endif
}

#ifdef USE_CART1
void EMU::open_cart(int drv, const _TCHAR* file_path)
{
	if(drv < MAX_CART) {
		if(check_file_extension(file_path, _T(".hex")) && hex2bin(file_path, osd->bios_path(_T("hex2bin.$$$")))) {
			vm->open_cart(drv, osd->bios_path(_T("hex2bin.$$$")));
			FILEIO::RemoveFile(osd->bios_path(_T("hex2bin.$$$")));
		} else {
			vm->open_cart(drv, file_path);
		}
		my_tcscpy_s(cart_status[drv].path, _MAX_PATH, file_path);
		out_message(_T("Cart%d: %s"), drv + 1, file_path);
		
		// restart recording
		bool s = osd->now_rec_sound;
		bool v = osd->now_rec_video;
		stop_rec_sound();
		stop_rec_video();
		if(s) osd->start_rec_sound();
		if(v) osd->start_rec_video(-1);
	}
}

void EMU::close_cart(int drv)
{
	if(drv < MAX_CART) {
		vm->close_cart(drv);
		clear_media_status(&cart_status[drv]);
		out_message(_T("Cart%d: Ejected"), drv + 1);
		
		// stop recording
		stop_rec_video();
		stop_rec_sound();
	}
}

bool EMU::cart_inserted(int drv)
{
	if(drv < MAX_CART) {
		return vm->cart_inserted(drv);
	} else {
		return false;
	}
}
#endif

#ifdef USE_FD1
void EMU::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_FD) {
		if(vm->disk_inserted(drv)) {
			vm->close_disk(drv);
			// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
			disk_status[drv].wait_count = (int)(vm->frame_rate() / 2);
#else
			disk_status[drv].wait_count = (int)(FRAMES_PER_SEC / 2);
#endif
			out_message(_T("FD%d: Ejected"), drv + FD_BASE_NUMBER);
		} else if(disk_status[drv].wait_count == 0) {
			vm->open_disk(drv, file_path, bank);
			out_message(_T("FD%d: %s"), drv + FD_BASE_NUMBER, file_path);
		}
		my_tcscpy_s(disk_status[drv].path, _MAX_PATH, file_path);
		disk_status[drv].bank = bank;
	}
}

void EMU::close_disk(int drv)
{
	if(drv < MAX_FD) {
		vm->close_disk(drv);
		clear_media_status(&disk_status[drv]);
		out_message(_T("FD%d: Ejected"), drv + FD_BASE_NUMBER);
	}
}

bool EMU::disk_inserted(int drv)
{
	if(drv < MAX_FD) {
		return vm->disk_inserted(drv);
	} else {
		return false;
	}
}

void EMU::set_disk_protected(int drv, bool value)
{
	if(drv < MAX_FD) {
		vm->set_disk_protected(drv, value);
	}
}

bool EMU::get_disk_protected(int drv)
{
	if(drv < MAX_FD) {
		return vm->get_disk_protected(drv);
	} else {
		return false;
	}
}
#endif

int EMU::get_access_lamp(void)
{
   int stat = 0;
#if defined(USE_ACCESS_LAMP)
# if defined(USE_FD1) || defined(USE_QD1)
#  if !defined(_MSC_VER)
//   LockVM();
#  endif

   stat = vm->access_lamp(); // Return accessing drive number.
#  if !defined(_MSC_VER)
//   UnlockVM();
#  endif
# endif
#endif
   return stat;
}


#ifdef USE_QD1
void EMU::open_quickdisk(int drv, const _TCHAR* file_path)
{
	if(drv < MAX_QD) {
		if(vm->quickdisk_inserted(drv)) {
			vm->close_quickdisk(drv);
			// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
			quickdisk_status[drv].wait_count = (int)(vm->frame_rate() / 2);
#else
			quickdisk_status[drv].wait_count = (int)(FRAMES_PER_SEC / 2);
#endif
			out_message(_T("QD%d: Ejected"), drv + QD_BASE_NUMBER);
		} else if(quickdisk_status[drv].wait_count == 0) {
			vm->open_quickdisk(drv, file_path);
			out_message(_T("QD%d: %s"), drv + QD_BASE_NUMBER, file_path);
		}
		my_tcscpy_s(quickdisk_status[drv].path, _MAX_PATH, file_path);
	}
}

void EMU::close_quickdisk(int drv)
{
	if(drv < MAX_QD) {
		vm->close_quickdisk(drv);
		clear_media_status(&quickdisk_status[drv]);
		out_message(_T("QD%d: Ejected"), drv + QD_BASE_NUMBER);
	}
}

bool EMU::quickdisk_inserted(int drv)
{
	if(drv < MAX_QD) {
		return vm->quickdisk_inserted(drv);
	} else {
		return false;
	}
}
#endif

#ifdef USE_TAPE
void EMU::play_tape(const _TCHAR* file_path)
{
	if(vm->tape_inserted()) {
		vm->close_tape();
		// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
		tape_status.wait_count = (int)(vm->frame_rate() / 2);
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
	if(vm->tape_inserted()) {
		vm->close_tape();
		// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
		tape_status.wait_count = (int)(vm->frame_rate() / 2);
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

bool EMU::tape_inserted()
{
	return vm->tape_inserted();
}

#ifndef TAPE_BINARY_ONLY
bool EMU::tape_playing()
{
	return vm->tape_playing();
}

bool EMU::tape_recording()
{
	return vm->tape_recording();
}

int EMU::tape_position()
{
	return vm->tape_position();
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

#ifdef USE_LASER_DISC
void EMU::open_laser_disc(const _TCHAR* file_path)
{
	if(vm->laser_disc_inserted()) {
		vm->close_laser_disc();
		// wait 0.5sec
#ifdef SUPPORT_VARIABLE_TIMING
		laser_disc_status.wait_count = (int)(vm->frame_rate() / 2);
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

bool EMU::laser_disc_inserted()
{
	return vm->laser_disc_inserted();
}
#endif

#ifdef USE_BINARY_FILE1
void EMU::load_binary(int drv, const _TCHAR* file_path)
{
	if(drv < MAX_BINARY) {
		if(check_file_extension(file_path, _T(".hex")) && hex2bin(file_path, osd->bios_path(_T("hex2bin.$$$")))) {
			vm->load_binary(drv, osd->bios_path(_T("hex2bin.$$$")));
			FILEIO::RemoveFile(osd->bios_path(_T("hex2bin.$$$")));
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
#ifdef SUPPORT_DUMMY_DEVICE_LED
uint32 EMU::get_led_status(void)
{
	return vm->get_led_status();
}
#endif

bool EMU::now_skip()
{
	return vm->now_skip();
}

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
	_TCHAR file_name[_MAX_PATH];
	my_stprintf_s(file_name, _MAX_PATH, _T("%s.sta"), _T(CONFIG_NAME));
	save_state_tmp(osd->bios_path(file_name));
}

void EMU::load_state()
{
	_TCHAR file_name[_MAX_PATH];
	my_stprintf_s(file_name, _MAX_PATH, _T("%s.sta"), _T(CONFIG_NAME));
	FILEIO ffp;
	if(ffp.IsFileExists(osd->bios_path(file_name))) {
		save_state_tmp(bios_path(_T("$temp$.sta")));
		if(!load_state_tmp(osd->bios_path(file_name))) {
			out_debug_log("failed to load state file\n");
			load_state_tmp(osd->bios_path(_T("$temp$.sta")));
		}
		DeleteFile(osd->bios_path(_T("$temp$.sta")));
	}
}

void EMU::save_state_tmp(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	//LockVM();
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
		fio->Fwrite(disk_status, sizeof(disk_status), 1);
		fio->Fwrite(d88_file, sizeof(d88_file), 1);
#endif
#ifdef USE_QD1
		fio->Fwrite(&quickdisk_status, sizeof(quickdisk_status), 1);
#endif
#ifdef USE_TAPE
		fio->Fwrite(&tape_status, sizeof(tape_status), 1);
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
	//UnlockVM();
	delete fio;
}

bool EMU::load_state_tmp(const _TCHAR* file_path)
{
	bool result = false;
	FILEIO* fio = new FILEIO();
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
				fio->Fread(disk_status, sizeof(disk_status), 1);
				fio->Fread(d88_file, sizeof(d88_file), 1);
#endif
#ifdef USE_QD1
				fio->Fread(&quickdisk_status, sizeof(quickdisk_status), 1);
#endif
#ifdef USE_TAPE
				fio->Fread(&tape_status, sizeof(tape_status), 1);
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
				if(reinitialize) {
					// stop sound
					//LockVM();
					// reinitialize virtual machine
					osd->stop_sound();
					delete vm;
					osd->vm = vm = new VM(this);
					vm->initialize_sound(sound_rate, sound_samples);
					vm->reset();
					//UnlockVM();
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
	delete fio;
	return result;
}
#endif

#if defined(USE_DIG_RESOLUTION)
void EMU::get_screen_resolution(int *w, int *h)
{
	vm->get_screen_resolution(w, h);
}
#endif
