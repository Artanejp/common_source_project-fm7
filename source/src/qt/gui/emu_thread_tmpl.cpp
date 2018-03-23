/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2015.11.10 Split from qt_main.cpp
	Note: This class must be compiled per VM, must not integrate shared units.
	[ win32 main ] -> [ Qt main ] -> [Emu Thread] -> [Independed from VMs]
*/

#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QTextCodec>
#include <QWaitCondition>
#include <QWidget>

#include <SDL.h>

#include "emu_thread_tmpl.h"

#include "qt_gldraw.h"
//#include "../../romakana.h"

//#include "csp_logger.h"
#include "menu_flags.h"

EmuThreadClassBase::EmuThreadClassBase(META_MainWindow *rootWindow, USING_FLAGS *p, QObject *parent) : QThread(parent) {
	MainWindow = rootWindow;
	using_flags = p;
	p_config = p->get_config_ptr();
	
	bRunThread = true;
	prev_skip = false;
	tick_timer.start();
	update_fps_time = tick_timer.elapsed();
	next_time = update_fps_time;
	total_frames = 0;
	draw_frames = 0;
	skip_frames = 0;
	calc_message = true;
	mouse_flag = false;

	drawCond = new QWaitCondition();
	keyMutex = new QMutex(QMutex::Recursive);
	mouse_x = 0;
	mouse_y = 0;
	if(using_flags->is_use_tape() && !using_flags->is_tape_binary_only()) {
		tape_play_flag = false;
		tape_rec_flag = false;
		tape_pos = 0;
	}

	if(using_flags->get_use_sound_volume() > 0) {
		for(int i = 0; i < using_flags->get_use_sound_volume(); i++) {
			bUpdateVolumeReq[i] = true;
			volume_avg[i] = (using_flags->get_config_ptr()->sound_volume_l[i] +
							 using_flags->get_config_ptr()->sound_volume_r[i]) / 2;
			volume_balance[i] = (using_flags->get_config_ptr()->sound_volume_r[i] -
								 using_flags->get_config_ptr()->sound_volume_l[i]) / 2;
		}
	}
	keyMutex->lock();
	key_fifo = new FIFO(512 * 6);
	key_fifo->clear();
	keyMutex->unlock();

};

EmuThreadClassBase::~EmuThreadClassBase() {
	delete drawCond;

	key_fifo->release();
	delete key_fifo;
};

void EmuThreadClassBase::calc_volume_from_balance(int num, int balance)
{
	int level = volume_avg[num];
	int right;
	int left;
	volume_balance[num] = balance;
	right = level + balance;
	left  = level - balance;
	using_flags->get_config_ptr()->sound_volume_l[num] = left;	
	using_flags->get_config_ptr()->sound_volume_r[num] = right;
}

void EmuThreadClassBase::calc_volume_from_level(int num, int level)
{
	int balance = volume_balance[num];
	int right,left;
	volume_avg[num] = level;
	right = level + balance;
	left  = level - balance;
	using_flags->get_config_ptr()->sound_volume_l[num] = left;	
	using_flags->get_config_ptr()->sound_volume_r[num] = right;
}

void EmuThreadClassBase::doExit(void)
{
	bRunThread = false;
}

void EmuThreadClassBase::button_pressed_mouse(Qt::MouseButton button)
{
	if(using_flags->is_use_mouse()) {
		button_pressed_mouse_sub(button);
	} else {		
		if(using_flags->get_max_button() > 0) {
			button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
			if(vm_buttons_d == NULL) return;
			switch(button) {
			case Qt::LeftButton:
			case Qt::RightButton:
				for(int i = 0; i < using_flags->get_max_button(); i++) {
					if((mouse_x >= vm_buttons_d[i].x) &&
					   (mouse_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((mouse_y >= vm_buttons_d[i].y) &&
						   (mouse_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
							if(vm_buttons_d[i].code != 0x00) {
								key_queue_t sp;
								sp.code = vm_buttons_d[i].code;
								sp.mod = key_mod;
								sp.repeat = false;
								enqueue_key_down(sp);
							} else {
								bResetReq = true;
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

void EmuThreadClassBase::button_released_mouse(Qt::MouseButton button)
{
	if(using_flags->is_use_mouse()) {
		button_released_mouse_sub(button);
	} else {
		if(using_flags->get_max_button() > 0) {
			button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
			if(vm_buttons_d == NULL) return;
			
			switch(button) {
			case Qt::LeftButton:
			case Qt::RightButton:
				for(int i = 0; i < using_flags->get_max_button(); i++) {
					if((mouse_x >= vm_buttons_d[i].x) &&
					   (mouse_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((mouse_y >= vm_buttons_d[i].y) &&
						   (mouse_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
							if(vm_buttons_d[i].code != 0x00) {
								key_queue_t sp;
								sp.code = vm_buttons_d[i].code;
								sp.mod = key_mod;
								sp.repeat = false;
								enqueue_key_up(sp);
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}
}


void EmuThreadClassBase::do_key_down(uint32_t vk, uint32_t mod, bool repeat)
{
	key_queue_t sp;
	sp.code = vk;
	sp.mod = mod;
	sp.repeat = repeat;
	//key_changed = true;
	enqueue_key_down(sp);
	key_mod = mod;
}

void EmuThreadClassBase::do_key_up(uint32_t vk, uint32_t mod)
{
	key_queue_t sp;
	sp.code = vk;
	sp.mod = mod;
	sp.repeat = false;
	enqueue_key_up(sp);
	key_mod = mod;
}

void EmuThreadClassBase::set_tape_play(bool flag)
{
	tape_play_flag = flag;
}

void EmuThreadClassBase::resize_screen(int screen_width, int screen_height, int stretched_width, int stretched_height)
{
	emit sig_resize_screen(screen_width, screen_height);
	emit sig_resize_osd(screen_width);
}

void EmuThreadClassBase::sample_access_drv(void)
{
	if(using_flags->is_use_qd()) get_qd_string();
	if(using_flags->is_use_fd()) get_fd_string();
	if(using_flags->is_use_tape() && !using_flags->is_tape_binary_only()) get_tape_string();
	if(using_flags->is_use_compact_disc()) get_cd_string();
	if(using_flags->is_use_bubble()) get_bubble_string();
}

void EmuThreadClassBase::do_update_config()
{
	bUpdateConfigReq = true;
}

void EmuThreadClassBase::do_start_record_sound()
{
	bStartRecordSoundReq = true;
}

void EmuThreadClassBase::do_stop_record_sound()
{
	bStopRecordSoundReq = true;
}

void EmuThreadClassBase::do_reset()
{
	bResetReq = true;
}

void EmuThreadClassBase::do_special_reset()
{
	bSpecialResetReq = true;
}

void EmuThreadClassBase::do_load_state(QString s)
{
	sStateFile = s;
	bLoadStateReq = true;
}

void EmuThreadClassBase::do_save_state(QString s)
{
	sStateFile = s;
	bSaveStateReq = true;
}

void EmuThreadClassBase::do_start_record_video(int fps)
{
	record_fps = fps;
	bStartRecordMovieReq = true;
}

void EmuThreadClassBase::do_stop_record_video()
{
	bStartRecordMovieReq = false;
}

void EmuThreadClassBase::do_update_volume_level(int num, int level)
{
	if(using_flags->get_use_sound_volume() > 0) {
		if((num < using_flags->get_use_sound_volume()) && (num >= 0)) {
			calc_volume_from_level(num, level);
			bUpdateVolumeReq[num] = true;
		}
	}
}

void EmuThreadClassBase::do_update_volume_balance(int num, int level)
{
	if(using_flags->get_use_sound_volume() > 0) {
		if((num < using_flags->get_use_sound_volume()) && (num >= 0)) {
			calc_volume_from_balance(num, level);
			bUpdateVolumeReq[num] = true;
		}
	}
}

int EmuThreadClassBase::parse_command_queue(QStringList _l, int _begin)
{
	int _ret = _l.size() / 2;
	for(int i = _begin * 2; i < _l.size(); i+= 2) {
		QString _dom = _l.at(i);
		QString _file = _l.at(i + 1);
		QString _dom_type = _dom.left(_dom.size() - 1);
		int _dom_num;
		bool _num_ok;
		_dom_num = _dom.right(1).toInt(&_num_ok);
		if(_num_ok) {
			if((_dom_type == QString::fromUtf8("vFloppyDisk")) ||
			   (_dom_type == QString::fromUtf8("vBubble"))) {
				int _n = _file.indexOf(QString::fromUtf8("@"));
				int _slot = 0;
				QFileInfo fileInfo;
				if((_n > 0) && (_n < 4)) {
					_slot = _file.left(_n).toInt(&_num_ok);
					if(_num_ok) {
						fileInfo = QFileInfo(_file.right(_file.size() - (_n + 1)));
					} else {
						fileInfo = QFileInfo(_file);
					}
				} else {
					fileInfo = QFileInfo(_file);
				}
				if(fileInfo.isFile()) {
					if(_dom_type == QString::fromUtf8("vFloppyDisk")) {
						emit sig_open_fd(_dom_num, fileInfo.absoluteFilePath());
						emit sig_set_d88_num(_dom_num, _slot);
					} else if(_dom_type == QString::fromUtf8("vBubble")) {
						emit sig_open_bubble(_dom_num, fileInfo.absoluteFilePath());
						emit sig_set_b77_num(_dom_num, _slot);
					}
				}
			} else {
				QFileInfo fileInfo = QFileInfo(_file);
				if(fileInfo.isFile()) {
					if(_dom_type == QString::fromUtf8("vQuickDisk")) {
						emit sig_open_quick_disk(_dom_num, _file);
					} else if(_dom_type == QString::fromUtf8("vCmt")) {
						emit sig_open_cmt_load(_dom_num, _file);
					} else if(_dom_type == QString::fromUtf8("vBinary")) {
						emit sig_open_binary_load(_dom_num, _file);
					} else if(_dom_type == QString::fromUtf8("vCart")) {
						emit sig_open_cart(_dom_num, _file);
					} else if(_dom_type == QString::fromUtf8("vLD")) {
						emit sig_open_laser_disc(_dom_num, _file);
					} else if(_dom_type == QString::fromUtf8("vCD")) {
						emit sig_open_cdrom(_dom_num, _file);
					}
				}
			}
		}
	}
	_ret = _ret - _begin;
	if(_ret < 0) _ret = 0;
	return _ret;
}

const _TCHAR *EmuThreadClassBase::get_emu_message(void)
{
	static const _TCHAR str[] = "";
	return (const _TCHAR *)str;
}

double EmuThreadClassBase::get_emu_frame_rate(void)
{
	return 30.00;
}
int EmuThreadClassBase::get_message_count(void)
{
	return 0;
}

void EmuThreadClassBase::dec_message_count(void)
{

}

const _TCHAR *EmuThreadClassBase::get_device_name(void)
{
	return (const _TCHAR *)_T("TEST");
}

bool EmuThreadClassBase::get_power_state(void)
{
	return true;
}

void EmuThreadClassBase::print_framerate(int frames)
{
	if(frames >= 0) draw_frames += frames;
	if(calc_message) {
		qint64 current_time = tick_timer.elapsed();
		if(update_fps_time <= current_time && update_fps_time != 0) {
			_TCHAR buf[256];
			QString message;
			//int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);

			if(get_power_state() == false){ 	 
					snprintf(buf, 255, _T("*Power OFF*"));
				} else if(now_skip) {
					int ratio = (int)(100.0 * (double)total_frames / get_emu_frame_rate() + 0.5);
					snprintf(buf, 255, create_string(_T("%s - Skip Frames (%d %%)"), get_device_name(), ratio));
				} else {
					if(get_message_count() > 0) {
						snprintf(buf, 255, _T("%s - %s"), get_device_name(), get_emu_message());
						dec_message_count();
					} else {
						int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);
						snprintf(buf, 255, _T("%s - %d fps (%d %%)"), get_device_name(), draw_frames, ratio);
					}
				}
				if(p_config->romaji_to_kana) {
					message = QString::fromUtf8("[R]");
					message = message + QString::fromUtf8(buf);
				} else {
					message = buf;
				}
				emit message_changed(message);
				emit window_title_changed(message);
				update_fps_time += 1000;
				total_frames = draw_frames = 0;
				
			}
			if(update_fps_time <= current_time) {
				update_fps_time = current_time + 1000;
			}
			calc_message = false;  
		} else {
			calc_message = true;
		}
}
