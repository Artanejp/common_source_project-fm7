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
#include "emu.h"

#include "emu_thread.h"
#include "draw_thread.h"

#include "qt_gldraw.h"
#include "agar_logger.h"

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
		return;
	}
	if(parent_thread->now_debugging()) return;
	while(VMSemaphore->available() < 1) VMSemaphore->release(1);
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
	//memset(&vm_screen_buffer, 0, sizeof(bitmap_t));
	vm_screen_buffer.width = SCREEN_WIDTH;
	vm_screen_buffer.height = SCREEN_HEIGHT;
	vm_screen_buffer.pImage = QImage(SCREEN_WIDTH, SCREEN_HEIGHT, QImage::Format_ARGB32);
	vm_screen_buffer.pImage.fill(col);
	now_record_video = false;
	//pAVIStream = NULL;
	//pAVICompressed = NULL;
	//pAVIFile = NULL;
	
	first_draw_screen = false;
	first_invalidate = true;
	self_invalidate = false;
}

void OSD::release_screen()
{
	stop_record_video();
	
	//release_d3d9();
	//if(vm_screen_buffer.pImage != NULL) delete vm_screen_buffer.pImage;
	release_screen_buffer(&vm_screen_buffer);
}

int OSD::get_window_mode_width(int mode)
{
	if(get_use_screen_rotate()) {
		if(config.rotate_type == 1 || config.rotate_type == 3) {
			return (config.window_stretch_type == 0 ? vm_window_height : vm_window_height_aspect) * (mode + WINDOW_MODE_BASE);
		}
	}
	return (config.window_stretch_type == 0 ? vm_window_width : vm_window_width_aspect) * (mode + WINDOW_MODE_BASE);
}

int OSD::get_window_mode_height(int mode)
{
	if(get_use_screen_rotate()) {
		if(config.rotate_type == 1 || config.rotate_type == 3) {
			return (config.window_stretch_type == 0 ? vm_window_width : vm_window_width_aspect) * (mode + WINDOW_MODE_BASE);
		}
	}
	return (config.window_stretch_type == 0 ? vm_window_height : vm_window_height_aspect) * (mode + WINDOW_MODE_BASE);
}
