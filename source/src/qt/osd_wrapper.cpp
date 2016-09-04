/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ VM/OSD Wrapper ]
*/

#include <QImage>
#include <QPainter>
#include <QColor>
#include <QPen>
#include <QPoint>
#include <QTextCodec>

#include "osd.h"
#include "../vm/vm.h"
#include "../vm/device.h"

#include "emu.h"

#include "emu_thread.h"
#include "draw_thread.h"
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
#include "avio/movie_loader.h"
#endif

#include "qt_gldraw.h"
#include "csp_logger.h"

void OSD::vm_draw_screen(void)
{
	vm->draw_screen();
}

double OSD::vm_frame_rate(void)
{
#ifdef SUPPORT_VARIABLE_TIMING
	return vm->get_frame_rate();
#else
	return FRAMES_PER_SEC;
#endif   
}

Sint16* OSD::create_sound(int *extra_frames)
{
	return (Sint16 *)vm->create_sound(extra_frames);
}


bool OSD::get_use_socket(void)
{
#ifdef USE_SOCKET
	return true;
#else
	return false;
#endif
}

bool OSD::get_support_variable_timing(void)
{
#ifdef SUPPORT_VARIABLE_TIMING
	return true;
#else
	return false;
#endif
}

bool OSD::get_notify_key_down(void)
{
#ifdef NOTIFY_KEY_DOWN
	return true;
#else
	return false;
#endif	
}

bool OSD::get_notify_key_down_lr_shift(void)
{
#ifdef NOTIFY_KEY_DOWN_LR_SHIFT
	return true;
#else
	return false;
#endif
}

bool OSD::get_notify_key_down_lr_control(void)
{
#ifdef NOTIFY_KEY_DOWN_LR_CONTROL
	return true;
#else
	return false;
#endif
}

bool OSD::get_notify_key_down_lr_menu(void)
{
#ifdef NOTIFY_KEY_DOWN_LR_MEHU
	return true;
#else
	return false;
#endif
}

bool OSD::get_use_shift_numpad_key(void)
{
#ifdef USE_SHIFT_NUMPAD_KEY
	return true;
#else
	return false;
#endif
}

bool OSD::get_use_auto_key(void)
{
#ifdef USE_AUTO_KEY
	return true;
#else
	return false;
#endif
}

bool OSD::get_dont_keeep_key_pressed(void)
{
#ifdef DONT_KEEEP_KEY_PRESSED
	return true;
#else
	return false;
#endif
}

bool OSD::get_one_board_micro_computer(void)
{
#ifdef ONE_BOARD_MICRO_COMPUTER
	return true;
#else
	return false;
#endif
}

bool OSD::get_use_screen_rotate(void)
{
#ifdef USE_SCREEN_ROTATE
	return true;
#else
	return false;
#endif
}

bool OSD::get_use_movie_player(void)
{
#ifdef USE_MOVIE_PLAYER
	return true;
#else
	return false;
#endif
}

bool OSD::get_use_video_capture(void)
{
#ifdef USE_VIDEO_CAPTURE
	return true;
#else
	return false;
#endif
}

void OSD::vm_key_down(int code, bool flag)
{
#ifdef NOTIFY_KEY_DOWN
	vm->key_down(code, flag);
#endif
}

void OSD::vm_key_up(int code)
{
#ifdef NOTIFY_KEY_DOWN
	vm->key_up(code);
#endif
}

void OSD::vm_reset(void)
{
	vm->reset();
}

int OSD::get_vm_buttons_code(int num)
{
#ifdef ONE_BOARD_MICRO_COMPUTER
	if(num < 0) return 0;
	return vm_buttons[num].code;
#else
	return 0;
#endif
}	

void OSD::update_buttons(bool press_flag, bool release_flag)
{
#if defined(MAX_BUTTONS)
	if(!press_flag && !release_flag) {
		int ii;
		ii = 0;
		for(ii = 0; vm_buttons[ii].code != 0x00; ii++) { 
			if((mouse_ptrx >= vm_buttons[ii].x) && (mouse_ptrx < (vm_buttons[ii].x + vm_buttons[ii].width))) {
				if((mouse_ptry >= vm_buttons[ii].y) && (mouse_ptry < (vm_buttons[ii].y + vm_buttons[ii].height))) {
					if((key_status[vm_buttons[ii].code] & 0x7f) == 0) this->press_button(ii);
				}
			}
		}
		if((mouse_ptrx >= vm_buttons[ii].x) && (mouse_ptrx < (vm_buttons[ii].x + vm_buttons[ii].width))) {
			if((mouse_ptry >= vm_buttons[ii].y) && (mouse_ptry < (vm_buttons[ii].y + vm_buttons[ii].height))) {
				this->press_button(ii);
			}
		}
		mouse_ptrx = mouse_ptry = 0;
	}
	//return;
#endif			
}	

QString OSD::get_vm_config_name(void)
{
#if defined(CONFIG_NAME)
	return QString::fromUtf8(CONFIG_NAME);
#else
	return QString::fromUtf8(" ");
#endif
}

int OSD::get_screen_width(void)
{
	return SCREEN_WIDTH;
}

int OSD::get_screen_height(void)
{
	return SCREEN_HEIGHT;
}

void OSD::lock_vm(void)
{
	locked_vm = true;
	if(parent_thread != NULL) { 
		if(!parent_thread->now_debugging()) VMSemaphore->acquire(1);
	} else {
		VMSemaphore->acquire(1);
	}
}

void OSD::unlock_vm(void)
{
	if(parent_thread != NULL) { 
		if(!parent_thread->now_debugging()) VMSemaphore->release(1);
	} else {
		VMSemaphore->release(1);
	}
	locked_vm = false;
}


bool OSD::is_vm_locked(void)
{
	return locked_vm;
}

void OSD::force_unlock_vm(void)
{
	if(parent_thread == NULL) {
		while(VMSemaphore->available() < 1) VMSemaphore->release(1);
		locked_vm = false;
		return;
	}
	if(parent_thread->now_debugging()) {
		locked_vm = false;
		return;
	}
	while(VMSemaphore->available() < 1) VMSemaphore->release(1);
	locked_vm = false;
}

void OSD::set_draw_thread(DrawThreadClass *handler)
{
	this->moveToThread(handler);
	connect(this, SIGNAL(sig_update_screen(bitmap_t *)), handler, SLOT(do_update_screen(bitmap_t *)));
	connect(this, SIGNAL(sig_save_screen(const char *)), glv, SLOT(do_save_frame_screen(const char *)));
	connect(this, SIGNAL(sig_resize_vm_screen(QImage *, int, int)), glv, SLOT(do_set_texture_size(QImage *, int, int)));
	connect(parent_thread, SIGNAL(sig_debugger_input(QString)), this, SLOT(do_set_input_string(QString)));
	connect(parent_thread, SIGNAL(sig_quit_debugger()), this, SLOT(do_close_debugger_thread()));
	connect(this, SIGNAL(sig_close_window()), parent_thread, SLOT(doExit()));
	connect(this, SIGNAL(sig_console_input_string(QString)), parent_thread, SLOT(do_call_debugger_command(QString)));
}

void OSD::initialize_screen()
{
	host_window_width = base_window_width = WINDOW_WIDTH;
	host_window_height = base_window_height = WINDOW_HEIGHT;
	host_window_mode = true;
	
	vm_screen_width = SCREEN_WIDTH;
	vm_screen_height = SCREEN_HEIGHT;
	vm_window_width = WINDOW_WIDTH;
	vm_window_height = WINDOW_HEIGHT;
	vm_window_width_aspect = WINDOW_WIDTH_ASPECT;
	vm_window_height_aspect = WINDOW_HEIGHT_ASPECT;
	
	QColor col(0, 0, 0, 255);

	vm_screen_buffer.width = SCREEN_WIDTH;
	vm_screen_buffer.height = SCREEN_HEIGHT;
	vm_screen_buffer.pImage = QImage(SCREEN_WIDTH, SCREEN_HEIGHT, QImage::Format_ARGB32);
	vm_screen_buffer.pImage.fill(col);
	now_record_video = false;
	
	first_draw_screen = false;
	first_invalidate = true;
	self_invalidate = false;
}

void OSD::release_screen()
{
	stop_record_video();
	release_screen_buffer(&vm_screen_buffer);
}

int OSD::get_window_mode_width(int mode)
{
	if(get_use_screen_rotate()) {
		if(p_config->rotate_type == 1 || p_config->rotate_type == 3) {
			return (p_config->window_stretch_type == 0 ? vm_window_height : vm_window_height_aspect) * (mode + WINDOW_MODE_BASE);
		}
	}
	return (p_config->window_stretch_type == 0 ? vm_window_width : vm_window_width_aspect) * (mode + WINDOW_MODE_BASE);
}

int OSD::get_window_mode_height(int mode)
{
	if(get_use_screen_rotate()) {
		if(p_config->rotate_type == 1 || p_config->rotate_type == 3) {
			return (p_config->window_stretch_type == 0 ? vm_window_width : vm_window_width_aspect) * (mode + WINDOW_MODE_BASE);
		}
	}
	return (p_config->window_stretch_type == 0 ? vm_window_height : vm_window_height_aspect) * (mode + WINDOW_MODE_BASE);
}

void OSD::initialize_video()
{
	movie_loader = NULL;
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	movie_loader = new MOVIE_LOADER(this, &config);
	//connect(movie_loader, SIGNAL(sig_send_audio_frame(uint8_t *, long)), this, SLOT(do_run_movie_audio_callback2(uint8_t *, long)));
	connect(movie_loader, SIGNAL(sig_movie_end(bool)), this, SLOT(do_video_movie_end(bool)));
	connect(this, SIGNAL(sig_movie_play(void)), movie_loader, SLOT(do_play()));
	connect(this, SIGNAL(sig_movie_stop(void)), movie_loader, SLOT(do_stop()));
	connect(this, SIGNAL(sig_movie_pause(bool)), movie_loader, SLOT(do_pause(bool)));
	connect(this, SIGNAL(sig_movie_seek_frame(bool, int)), movie_loader, SLOT(do_seek_frame(bool, int)));
	//connect(this, SIGNAL(sig_movie_mute(bool, bool)), movie_loader, SLOT(do_mute(bool, bool)));
	connect(movie_loader, SIGNAL(sig_decoding_error(int)), this, SLOT(do_video_decoding_error(int)));
#endif	
}

void OSD::release_video()
{
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	delete movie_loader;
#endif	
	movie_loader = NULL;
}


bool OSD::open_movie_file(const _TCHAR* file_path)
{
	bool ret = false;
	if(file_path == NULL) return ret;
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	if(movie_loader != NULL) {
		ret = movie_loader->open(QString::fromUtf8(file_path));
	}
#endif	
	return ret;
}

void OSD::close_movie_file()
{
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	if(movie_loader != NULL) {
		movie_loader->close();
	}
#endif
	now_movie_play = false;
	now_movie_pause = false;
}

#include <limits.h>
uint32_t OSD::get_cur_movie_frame()
{
	uint64_t pos;
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	if(movie_loader != NULL) {
		pos = movie_loader->get_current_frame();
		if(pos > UINT_MAX) {
			return UINT_MAX;
		}
		return (uint32_t)pos;
	}
#endif	
	return 0;
}

void OSD::do_run_movie_audio_callback(uint8_t *data, long len)
{
	if(data == NULL) return;
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
#if defined(_PX7)
	{
		lock_vm();
		this->vm->movie_sound_callback(data, len);
		unlock_vm();
	}
#endif
#endif
	free(data);
}

void OSD::do_decode_movie(int frames)
{
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	movie_loader->do_decode_frames(frames, this->get_vm_window_width(), this->get_vm_window_height());
#endif	
}

void OSD::get_video_buffer()
{
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	//movie_loader->do_decode_frames(1, this->get_vm_window_width(), this->get_vm_window_height());
	movie_loader->get_video_frame();
	//printf("**\n");
#endif
}

int OSD::get_movie_sound_rate()
{
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	return movie_loader->get_movie_sound_rate();
#endif
}

void OSD::set_vm_node()
{
	device_node_t sp;
	device_node_list.clear();
	for(DEVICE *p = vm->first_device; p != NULL; p = p->next_device) {
		sp.id = p->this_device_id;
		sp.name = p->this_device_name;
		csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL,  "Device %d :%s", sp.id, sp.name);
		device_node_list.append(sp);
	}
}

