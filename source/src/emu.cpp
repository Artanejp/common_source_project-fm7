/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 emulation i/f ]
*/

#include "emu.h"
#include "vm/vm.h"
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

EMU::EMU(HWND hwnd, HINSTANCE hinst)
{
#ifdef _DEBUG_LOG
	// open debug logfile
	initialize_debug_log();
#endif
	message_count = 0;
	
	// store main window handle
	main_window_handle = hwnd;
	instance_handle = hinst;
	
	// get module path
	_TCHAR tmp_path[_MAX_PATH], *ptr;
	GetModuleFileName(NULL, tmp_path, _MAX_PATH);
	GetFullPathName(tmp_path, _MAX_PATH, app_path, &ptr);
	*ptr = _T('\0');
	
#ifdef USE_FD1
	// initialize d88 file info
	memset(d88_file, 0, sizeof(d88_file));
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
	
	// initialize
	vm = new VM(this);
#ifdef USE_DEBUGGER
	initialize_debugger();
#endif
	initialize_input();
	initialize_screen();
	initialize_sound();
	initialize_media();
	initialize_printer();
#ifdef USE_SOCKET
	initialize_socket();
#endif
#ifdef USE_DIRECT_SHOW
	CoInitialize(NULL);
	initialize_direct_show();
#endif
	vm->initialize_sound(sound_rate, sound_samples);
	vm->reset();
	now_suspended = false;
}

EMU::~EMU()
{
#ifdef USE_DEBUGGER
	release_debugger();
#endif
	release_input();
	release_screen();
	release_sound();
	release_printer();
#ifdef USE_SOCKET
	release_socket();
#endif
#ifdef USE_DIRECT_SHOW
	release_direct_show();
	CoInitialize(NULL);
#endif
	delete vm;
#ifdef _DEBUG_LOG
	release_debug_log();
#endif
}

// ----------------------------------------------------------------------------
// drive machine
// ----------------------------------------------------------------------------

int EMU::frame_interval()
{
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
}

int EMU::run()
{
	if(now_suspended) {
#ifdef USE_LASER_DISC
		if(now_movie_play && !now_movie_pause) {
			play_movie();
		}
#endif
		now_suspended = false;
	}
	
	update_input();
	update_media();
	update_printer();
#ifdef USE_SOCKET
	update_socket();
#endif
	
	// virtual machine may be driven to fill sound buffer
	int extra_frames = 0;
	update_sound(&extra_frames);
	
	// drive virtual machine
	if(extra_frames == 0) {
		vm->run();
		extra_frames = 1;
	}
	rec_video_run_frames += extra_frames;
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
		if(sound_ok && sound_started) {
			lpdsb->Stop();
			sound_started = false;
		}
		// reinitialize virtual machine
		delete vm;
		vm = new VM(this);
		vm->initialize_sound(sound_rate, sound_samples);
		vm->reset();
		// restore inserted medias
		restore_media();
	} else {
		// reset virtual machine
		vm->reset();
	}
	
	// reset printer
	reset_printer();
	
	// restart recording
	restart_rec_sound();
	restart_rec_video();
}

#ifdef USE_SPECIAL_RESET
void EMU::special_reset()
{
	// reset virtual machine
	vm->special_reset();
	
	// reset printer
	reset_printer();
	
	// restart recording
	restart_rec_sound();
	restart_rec_video();
}
#endif

#ifdef USE_POWER_OFF
void EMU::notify_power_off()
{
	vm->notify_power_off();
}
#endif

_TCHAR* EMU::bios_path(_TCHAR* file_name)
{
	static _TCHAR file_path[_MAX_PATH];
	_stprintf(file_path, _T("%s%s"), app_path, file_name);
	return file_path;
}

void EMU::suspend()
{
	if(!now_suspended) {
#ifdef USE_LASER_DISC
		if(now_movie_play && !now_movie_pause) {
			pause_movie();
			now_movie_pause = false;
		}
#endif
		mute_sound();
		now_suspended = true;
	}
}

// ----------------------------------------------------------------------------
// timer
// ----------------------------------------------------------------------------

void EMU::get_host_time(cur_time_t* time)
{
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);
	
	time->year = sTime.wYear;
	time->month = sTime.wMonth;
	time->day = sTime.wDay;
	time->day_of_week = sTime.wDayOfWeek;
	time->hour = sTime.wHour;
	time->minute = sTime.wMinute;
	time->second = sTime.wSecond;
}

// ----------------------------------------------------------------------------
// printer
// ----------------------------------------------------------------------------

void EMU::initialize_printer()
{
	prn_fio = new FILEIO();
	prn_data = -1;
	prn_strobe = false;
}

void EMU::release_printer()
{
	close_printer_file();
	delete prn_fio;
}

void EMU::reset_printer()
{
	close_printer_file();
	prn_data = -1;
	prn_strobe = false;
}

void EMU::update_printer()
{
	if(prn_fio->IsOpened() && --prn_wait_frames == 0) {
		close_printer_file();
	}
}

void EMU::open_printer_file()
{
	cur_time_t time;
	get_host_time(&time);
	_stprintf(prn_file_name, _T("prn_%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.txt"), time.year, time.month, time.day, time.hour, time.minute, time.second);
	prn_fio->Fopen(bios_path(prn_file_name), FILEIO_WRITE_BINARY);
}

void EMU::close_printer_file()
{
	if(prn_fio->IsOpened()) {
		// remove if the file size is less than 2 bytes
		bool remove = (prn_fio->Ftell() < 2);
		prn_fio->Fclose();
		if(remove) {
			prn_fio->Remove(bios_path(prn_file_name));
		}
	}
}

void EMU::printer_out(uint8 value)
{
	prn_data = value;
}

void EMU::printer_strobe(bool value)
{
	bool falling = (prn_strobe && !value);
	prn_strobe = value;
	
	if(falling) {
		if(!prn_fio->IsOpened()) {
			if(prn_data == -1) {
				return;
			}
			open_printer_file();
		}
		prn_fio->Fputc(prn_data);
		// wait 10sec
#ifdef SUPPORT_VARIABLE_TIMING
		prn_wait_frames = (int)(vm->frame_rate() * 10.0 + 0.5);
#else
		prn_wait_frames = (int)(FRAMES_PER_SEC * 10.0 + 0.5);
#endif
	}
}

// ----------------------------------------------------------------------------
// debug log
// ----------------------------------------------------------------------------

#ifdef _DEBUG_LOG
void EMU::initialize_debug_log()
{
	debug_log = _tfopen(_T("d:\\debug.log"), _T("w"));
}

void EMU::release_debug_log()
{
	if(debug_log) {
		fclose(debug_log);
	}
}
#endif

void EMU::out_debug_log(const _TCHAR* format, ...)
{
#ifdef _DEBUG_LOG
	va_list ap;
	_TCHAR buffer[1024];
	static _TCHAR prev_buffer[1024] = {0};
	
	va_start(ap, format);
	_vstprintf(buffer, format, ap);
	va_end(ap);
	
	if(_tcscmp(prev_buffer, buffer) == 0) {
		return;
	}
	_tcscpy(prev_buffer, buffer);
	
	if(debug_log) {
		_ftprintf(debug_log, _T("%s"), buffer);
		static int size = 0;
		if((size += _tcslen(buffer)) > 0x8000000) { // 128MB
			static int index = 1;
			TCHAR path[_MAX_PATH];
			_stprintf(path, _T("d:\\debug_#%d.log"), ++index);
			fclose(debug_log);
			debug_log = _tfopen(path, _T("w"));
			size = 0;
		}
	}
#endif
}

void EMU::out_message(const _TCHAR* format, ...)
{
	va_list ap;
	va_start(ap, format);
	_vstprintf(message, format, ap);
	va_end(ap);
	message_count = 4; // 4sec
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

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
			vm->open_disk(drv, disk_status[drv].path, disk_status[drv].offset);
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
			vm->open_cart(drv, cart_status[drv].path);
		}
	}
#endif
#ifdef USE_FD1
	for(int drv = 0; drv < MAX_FD; drv++) {
		if(disk_status[drv].path[0] != _T('\0')) {
			vm->open_disk(drv, disk_status[drv].path, disk_status[drv].offset);
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
void EMU::open_cart(int drv, _TCHAR* file_path)
{
	if(drv < MAX_CART) {
		vm->open_cart(drv, file_path);
		_tcscpy(cart_status[drv].path, file_path);
		out_message(_T("Cart%d: %s"), drv + 1, file_path);
		
		// restart recording
		bool s = now_rec_sound;
		bool v = now_rec_video;
		stop_rec_sound();
		stop_rec_video();
		if(s) start_rec_sound();
		if(v) start_rec_video(-1);
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
void EMU::open_disk(int drv, _TCHAR* file_path, int offset)
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
			vm->open_disk(drv, file_path, offset);
			out_message(_T("FD%d: %s"), drv + FD_BASE_NUMBER, file_path);
		}
		_tcscpy(disk_status[drv].path, file_path);
		disk_status[drv].offset = offset;
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
#endif

#ifdef USE_QD1
void EMU::open_quickdisk(int drv, _TCHAR* file_path)
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
		_tcscpy(quickdisk_status[drv].path, file_path);
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
void EMU::play_tape(_TCHAR* file_path)
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
	_tcscpy(tape_status.path, file_path);
	tape_status.play = true;
}

void EMU::rec_tape(_TCHAR* file_path)
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
	_tcscpy(tape_status.path, file_path);
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
#endif

#ifdef USE_LASER_DISC
void EMU::open_laser_disc(_TCHAR* file_path)
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
	_tcscpy(laser_disc_status.path, file_path);
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

#ifdef USE_TAPE_BUTTON
void EMU::push_play()
{
	vm->push_play();
}

void EMU::push_stop()
{
	vm->push_stop();
}
#endif

#ifdef USE_BINARY_FILE1
void EMU::load_binary(int drv, _TCHAR* file_path)
{
	if(drv < MAX_BINARY) {
		vm->load_binary(drv, file_path);
		out_message(_T("Load: %s"), file_path);
	}
}

void EMU::save_binary(int drv, _TCHAR* file_path)
{
	if(drv < MAX_BINARY) {
		vm->save_binary(drv, file_path);
		out_message(_T("Save: %s"), file_path);
	}
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

#ifdef USE_STATE
// ----------------------------------------------------------------------------
// state
// ----------------------------------------------------------------------------

#define STATE_VERSION	1

void EMU::save_state()
{
	_TCHAR file_name[_MAX_PATH];
	_stprintf(file_name, _T("%s.sta"), _T(CONFIG_NAME));
	save_state_tmp(bios_path(file_name));
}

void EMU::load_state()
{
	_TCHAR file_name[_MAX_PATH];
	_stprintf(file_name, _T("%s.sta"), _T(CONFIG_NAME));
	save_state_tmp(bios_path(_T("$temp$.sta")));
	if(!load_state_tmp(bios_path(file_name))) {
		load_state_tmp(bios_path(_T("$temp$.sta")));
	}
	DeleteFile(bios_path(_T("$temp$.sta")));
}

void EMU::save_state_tmp(_TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
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
	delete fio;
}

bool EMU::load_state_tmp(_TCHAR* file_path)
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
					if(sound_ok && sound_started) {
						lpdsb->Stop();
						sound_started = false;
					}
					// reinitialize virtual machine
					delete vm;
					vm = new VM(this);
					vm->initialize_sound(sound_rate, sound_samples);
					vm->reset();
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

