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
#include <QOpenGLContext>

#include <SDL.h>

#include "emu_thread_tmpl.h"
#include "mainwidget_base.h"
#include "qt_gldraw.h"
#include "common.h"
#include "dock_disks.h"

//#include "../../romakana.h"

//#include "csp_logger.h"
#include "menu_flags.h"

EmuThreadClassBase::EmuThreadClassBase(Ui_MainWindowBase *rootWindow, USING_FLAGS *p, QObject *parent) : QThread(parent) {
	MainWindow = rootWindow;
	bBlockTask = true;
	using_flags = p;
	p_config = p->get_config_ptr();
	p_emu = NULL;
	p_osd = NULL;

	is_shared_glcontext = false;
	glContext = NULL;
	glContext = new QOpenGLContext(this);

	if(glContext != NULL) {
		glContext->setShareContext(rootWindow->getGraphicsView()->context());
		glContext->create();
	}
	if(glContext->isValid()) {
		is_shared_glcontext = true;
//		printf("Context sharing succeeded.ADDR=%08x GLES=%s\n", glContext, (glContext->isOpenGLES()) ? "YES" : "NO");
	}
	
	bRunThread = true;
	thread_id = (Qt::HANDLE)0;
	queue_fixed_cpu = -1;
	prev_skip = false;
	//tick_timer.start();
	//update_fps_time = tick_timer.elapsed();
	//next_time = update_fps_time;
	total_frames = 0;
	draw_frames = 0;
	skip_frames = 0;
	calc_message = true;
	mouse_flag = false;
	vMovieQueue.clear();
	
	drawCond = new QWaitCondition();
//	keyMutex = new QMutex(QMutex::Recursive);

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
			volume_avg[i] = (p_config->sound_volume_l[i] +
							 p_config->sound_volume_r[i]) / 2;
			volume_balance[i] = (p_config->sound_volume_r[i] -
								 p_config->sound_volume_l[i]) / 2;
		}
	}
	QMutexLocker _n(&keyMutex);

	key_fifo = new FIFO(512 * 6);
	key_fifo->clear();


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
	p_config->sound_volume_l[num] = left;	
	p_config->sound_volume_r[num] = right;
}

void EmuThreadClassBase::calc_volume_from_level(int num, int level)
{
	int balance = volume_balance[num];
	int right,left;
	volume_avg[num] = level;
	right = level + balance;
	left  = level - balance;
	p_config->sound_volume_l[num] = left;	
	p_config->sound_volume_r[num] = right;
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
			int _x = (int)rint(mouse_x);
			int _y = (int)rint(mouse_y);
			switch(button) {
			case Qt::LeftButton:
//			case Qt::RightButton:
				for(int i = 0; i < using_flags->get_max_button(); i++) {
					if((_x >= vm_buttons_d[i].x) &&
					   (_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((_y >= vm_buttons_d[i].y) &&
						   (_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
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
			int _x = (int)rint(mouse_x);
			int _y = (int)rint(mouse_y);
			switch(button) {
			case Qt::LeftButton:
//			case Qt::RightButton:
				for(int i = 0; i < using_flags->get_max_button(); i++) {
					if((_x >= vm_buttons_d[i].x) &&
					   (_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((_y >= vm_buttons_d[i].y) &&
						   (_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
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
	if(using_flags->is_use_hdd()) get_hdd_string();	
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

void EmuThreadClassBase::do_unblock()
{
	bBlockTask = false;
}

void EmuThreadClassBase::do_block()
{
	bBlockTask = true;
}
void EmuThreadClassBase::do_start_emu_thread()
{
	start(QThread::TimeCriticalPriority);
}
void EmuThreadClassBase::do_special_reset(int num)
{
	if(num < 0) return;
	if(num >= using_flags->get_use_special_reset_num()) return;
	bSpecialResetReq = true;
	specialResetNum = num;
}

void EmuThreadClassBase::do_load_state(QString s)
{
	lStateFile = s;
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
						_slot = 0;
					}
				} else {
					fileInfo = QFileInfo(_file);
					_slot = 0;
				}
				if(fileInfo.isFile()) {
					const _TCHAR *path_shadow = (const _TCHAR *)(fileInfo.absoluteFilePath().toLocal8Bit().constData());
					if(_dom_type == QString::fromUtf8("vFloppyDisk")) {
						if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
							emit sig_open_d88_fd(_dom_num, fileInfo.absoluteFilePath(), _slot);
							emit sig_change_virtual_media(CSP_DockDisks_Domain_FD, _dom_num, fileInfo.absoluteFilePath());;
						} else {
							emit sig_open_fd(_dom_num, fileInfo.absoluteFilePath());
							emit sig_change_virtual_media(CSP_DockDisks_Domain_FD, _dom_num, fileInfo.absoluteFilePath());;
						}
					} else 	if(_dom_type == QString::fromUtf8("vHardDisk")) {
						emit sig_open_hdd(_dom_num, fileInfo.absoluteFilePath());
						emit sig_change_virtual_media(CSP_DockDisks_Domain_HD, _dom_num, fileInfo.absoluteFilePath());;
					} else if(_dom_type == QString::fromUtf8("vBubble")) {
						if(check_file_extension(path_shadow, ".b77")) {
							emit sig_open_b77_bubble(_dom_num, fileInfo.absoluteFilePath(), _slot);
							emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, _dom_num, fileInfo.absoluteFilePath());;
						} else {
							emit sig_open_bubble(_dom_num, fileInfo.absoluteFilePath());
							emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, _dom_num, fileInfo.absoluteFilePath());;
						}
					}
				}
			} else {
				QFileInfo fileInfo = QFileInfo(_file);
				if(fileInfo.isFile()) {
					if(_dom_type == QString::fromUtf8("vQuickDisk")) {
						emit sig_open_quick_disk(_dom_num, fileInfo.absoluteFilePath());
						emit sig_change_virtual_media(CSP_DockDisks_Domain_QD, _dom_num, fileInfo.absoluteFilePath());;
				} else if(_dom_type == QString::fromUtf8("vCmt")) {
						emit sig_open_cmt_load(_dom_num, fileInfo.absoluteFilePath());
						emit sig_change_virtual_media(CSP_DockDisks_Domain_CMT, _dom_num, fileInfo.absoluteFilePath());;
					} else if(_dom_type == QString::fromUtf8("vBinary")) {
						emit sig_open_binary_load(_dom_num, fileInfo.absoluteFilePath());
						emit sig_change_virtual_media(CSP_DockDisks_Domain_Binary, _dom_num, fileInfo.absoluteFilePath());;
					} else if(_dom_type == QString::fromUtf8("vCart")) {
						emit sig_open_cart(_dom_num, fileInfo.absoluteFilePath());
						emit sig_change_virtual_media(CSP_DockDisks_Domain_Cart, _dom_num, fileInfo.absoluteFilePath());;
					} else if(_dom_type == QString::fromUtf8("vLD")) {
						vMovieQueue.append(_dom);
						vMovieQueue.append(fileInfo.absoluteFilePath());
						emit sig_change_virtual_media(CSP_DockDisks_Domain_LD, _dom_num, fileInfo.absoluteFilePath());;
						//emit sig_open_laser_disc(_dom_num, fileInfo.absoluteFilePath());
					} else if(_dom_type == QString::fromUtf8("vCD")) {
						emit sig_open_cdrom(_dom_num, fileInfo.absoluteFilePath());
						emit sig_change_virtual_media(CSP_DockDisks_Domain_CD, _dom_num, fileInfo.absoluteFilePath());;
					}
				}
			}
		}
	}
	_ret = _ret - _begin;
	if(_ret < 0) _ret = 0;
	return _ret;
}


const _TCHAR *EmuThreadClassBase::get_device_name(void)
{
	return (const _TCHAR *)_T("TEST");
}

void EmuThreadClassBase::print_framerate(int frames)
{
	if(frames >= 0) draw_frames += frames;
	if(calc_message) {
		qint64 current_time = tick_timer.elapsed();
		//qint64	current_time = SDL_GetTicks();

		if(update_fps_time <= current_time && update_fps_time != 0) {
			_TCHAR buf[256];
			QString message;
			//int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);

			if(get_power_state() == false){ 	 
				my_stprintf_s(buf, 255, _T("*Power OFF*"));
			} else if(now_skip) {
				int ratio = (int)(100.0 * (((double)total_frames / get_emu_frame_rate())  * 2.0) + 0.5);
				my_stprintf_s(buf, 255, _T("%s - Skip Frames (%d %%)"), get_device_name(), ratio);
			} else {
					if(get_message_count() > 0) {
						snprintf(buf, 255, _T("%s - %s"), get_device_name(), get_emu_message());
						dec_message_count();
					} else {
						int ratio = (int)(100.0 * ((double)draw_frames / (double)total_frames) * 2.0 + 0.5);
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

int EmuThreadClassBase::get_d88_file_cur_bank(int drive)
{
	if(!(using_flags->is_use_fd())) return -1;

	if(drive < using_flags->get_max_drive()) {
		QMutexLocker _locker(&uiMutex);
		return p_emu->d88_file[drive].cur_bank;
	}

	return -1;
}

int EmuThreadClassBase::get_d88_file_bank_num(int drive)
{
	if(!(using_flags->is_use_fd())) return -1;

	if(drive < using_flags->get_max_drive()) {
		QMutexLocker _locker(&uiMutex);
		return p_emu->d88_file[drive].bank_num;
	}

	return -1;
}


QString EmuThreadClassBase::get_d88_file_disk_name(int drive, int banknum)
{
	if(!(using_flags->is_use_fd())) return QString::fromUtf8("");

	if(drive < 0) return QString::fromUtf8("");
	if((drive < using_flags->get_max_drive()) && (banknum < get_d88_file_bank_num(drive))) {
		QMutexLocker _locker(&uiMutex);
		QString _n = QString::fromLocal8Bit((const char *)(&(p_emu->d88_file[drive].disk_name[banknum][0])));
		return _n;
	}

	return QString::fromUtf8("");
}


bool EmuThreadClassBase::is_floppy_disk_protected(int drive)
{
	QMutexLocker _locker(&uiMutex);
	if(!(using_flags->is_use_fd())) return false;

	bool _b = p_emu->is_floppy_disk_protected(drive);
	return _b;

	return false;
}


QString EmuThreadClassBase::get_d88_file_path(int drive)
{
	QMutexLocker _locker(&uiMutex);

	if(!(using_flags->is_use_fd())) return QString::fromUtf8("");
	if(drive < 0) return QString::fromUtf8("");
	
	if(drive < using_flags->get_max_drive()) {
		QMutexLocker _locker(&uiMutex);
		QString _n = QString::fromUtf8((const char *)(&(p_emu->d88_file[drive].path)));
		return _n;
	}

	return QString::fromUtf8("");
}

void EmuThreadClassBase::set_floppy_disk_protected(int drive, bool flag)
{
	QMutexLocker _locker(&uiMutex);
	if(!(using_flags->is_use_fd())) return;

	p_emu->is_floppy_disk_protected(drive, flag);

}
#if defined(Q_OS_LINUX)
//#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#endif

void EmuThreadClassBase::set_emu_thread_to_fixed_cpu(int cpunum)
{
#if defined(Q_OS_LINUX)
	if(thread_id == (Qt::HANDLE)nullptr) {
		queue_fixed_cpu = cpunum;
		return;
	}

	long cpus = sysconf(_SC_NPROCESSORS_ONLN);
	cpu_set_t *mask;
	mask = CPU_ALLOC(cpus);
	CPU_ZERO_S(CPU_ALLOC_SIZE(cpus), mask);
	if((cpunum < 0) || (cpunum >= cpus)) {
		for(int i = 0; i < cpus; i++ ) {
			CPU_SET(i, mask);
		}
	} else {
		CPU_SET(cpunum, mask);
	}
//	sched_setaffinity((pid_t)thread_id, CPU_ALLOC_SIZE(cpus), (const cpu_set_t*)mask);
	pthread_setaffinity_np(*((pthread_t*)thread_id), CPU_ALLOC_SIZE(cpus),(const cpu_set_t *)mask);
	CPU_FREE(mask);
#else
	return;
#endif
	return;
}

void EmuThreadClassBase::get_fd_string(void)
{
	if(!(using_flags->is_use_fd())) return;
	int i;
	QString tmpstr;
	QString iname;
	QString alamp;
	uint32_t access_drv = 0;
	bool lamp_stat = false;
	access_drv = p_emu->is_floppy_disk_accessed();
	{
		for(i = 0; i < (int)using_flags->get_max_drive(); i++) {
			tmpstr.clear();
			alamp.clear();
			lamp_stat = false;
			if(p_emu->is_floppy_disk_inserted(i)) {
				if(i == (access_drv - 1)) {
					lamp_stat = true;
					alamp = QString::fromUtf8("<FONT COLOR=FIREBRICK>●</FONT>"); // 💾U+1F4BE Floppy Disk
				} else {
					alamp = QString::fromUtf8("<FONT COLOR=BLUE>○</FONT>"); // 💾U+1F4BE Floppy Disk
				}
				if(p_emu->d88_file[i].bank_num > 0) {
					iname = QString::fromUtf8(p_emu->d88_file[i].disk_name[p_emu->d88_file[i].cur_bank]);
				} else {
					iname = QString::fromUtf8("*Inserted*");
				}
				tmpstr = iname;
			} else {
				tmpstr.clear();
				alamp = QString::fromUtf8("×");
			}
			if(alamp != fd_lamp[i]) {
				emit sig_set_access_lamp(i + 2, lamp_stat);
				emit sig_change_access_lamp(CSP_DockDisks_Domain_FD, i, alamp);
				fd_lamp[i] = alamp;
			}
			if(tmpstr != fd_text[i]) {
				emit sig_set_access_lamp(i + 2, lamp_stat);
				emit sig_change_osd(CSP_DockDisks_Domain_FD, i, tmpstr);
				fd_text[i] = tmpstr;
			}
			lamp_stat = false;
		}
	}
}

void EmuThreadClassBase::get_qd_string(void)
{
	if(!(using_flags->is_use_qd())) return;
	int i;
	QString iname;
	QString alamp;
	QString tmpstr;
	uint32_t access_drv = 0;
	bool lamp_stat = false;
	access_drv = p_emu->is_quick_disk_accessed();
	for(i = 0; i < using_flags->get_max_qd(); i++) {
		tmpstr.clear();
		lamp_stat = false;
		if(p_emu->is_quick_disk_inserted(i)) {
			if(i == (access_drv - 1)) {
				alamp = QString::fromUtf8("<FONT COLOR=MAGENTA>●</FONT>"); // 💽　U+1F4BD MiniDisc
				lamp_stat = true;
			} else {
				alamp = QString::fromUtf8("<FONT COLOR=BLUE>○</FONT>"); // 💽U+1F4BD MiniDisc
			}
			tmpstr = alamp;
			//tmpstr = tmpstr + QString::fromUtf8(" ");
			//iname = QString::fromUtf8("*Inserted*");
			//tmpstr = tmpstr + iname;
		} else {
			tmpstr = QString::fromUtf8("×");
		}
		if(tmpstr != qd_text[i]) {
			emit sig_set_access_lamp(i + 10, lamp_stat);
			emit sig_change_access_lamp(CSP_DockDisks_Domain_QD, i, tmpstr);
			qd_text[i] = tmpstr;
		}
		lamp_stat = false;
	}
}	

void EmuThreadClassBase::get_tape_string()
{
	QString tmpstr;
	bool readwrite;
	bool inserted;

	if(!(using_flags->is_use_tape()) || (using_flags->is_tape_binary_only())) return;
	for(int i = 0; i < using_flags->get_max_tape(); i++) {
		inserted = p_emu->is_tape_inserted(i);
		readwrite = false;
		if(inserted) {
			tmpstr.clear();
			const _TCHAR *ts = p_emu->get_tape_message(i);
			if(ts != NULL) {
				tmpstr = QString::fromUtf8(ts);
				readwrite = p_emu->is_tape_recording(i);
				if(readwrite) {
					tmpstr = QString::fromUtf8("<FONT COLOR=RED><B>") + tmpstr + QString::fromUtf8("</B></FONT>");
				} else {
					tmpstr = QString::fromUtf8("<FONT COLOR=GREEN><B>") + tmpstr + QString::fromUtf8("</B></FONT>");
				}					
			}
		} else {
			tmpstr = QString::fromUtf8("<FONT COLOR=BLUE>   EMPTY   </FONT>");
		}
		emit sig_set_access_lamp(i + 12 + ((readwrite) ? 2 : 0), inserted);
		if(tmpstr != cmt_text[i]) {
			emit sig_change_osd(CSP_DockDisks_Domain_CMT, i, tmpstr);
			cmt_text[i] = tmpstr;
		}
	}

}

void EmuThreadClassBase::get_hdd_string(void)
{
	if(!(using_flags->is_use_hdd())) return;

	QString tmpstr, alamp;
	uint32_t access_drv = p_emu->is_hard_disk_accessed();
	bool lamp_stat = false;
	alamp.clear();
	tmpstr.clear();
	for(int i = 0; i < (int)using_flags->get_max_hdd(); i++) {
		if(p_emu->is_hard_disk_inserted(i)) {
			if((access_drv & (1 << i)) != 0) {
				alamp = QString::fromUtf8("<FONT COLOR=LIME>■</FONT>");  // 
				lamp_stat = true;
			} else {
				alamp = QString::fromUtf8("<FONT COLOR=BLUE>□</FONT>");  // 
			}
		} else {
			alamp = QString::fromUtf8("×");
		}
		if(tmpstr != hdd_text[i]) {
			emit sig_set_access_lamp(i + 16, lamp_stat);
			emit sig_change_osd(CSP_DockDisks_Domain_HD, i, tmpstr);
			hdd_text[i] = tmpstr;
		}
		if(alamp != hdd_lamp[i]) {
			emit sig_set_access_lamp(i + 16, lamp_stat); 
			emit sig_change_access_lamp(CSP_DockDisks_Domain_HD, i, alamp);
			hdd_lamp[i] = alamp;
		}
		lamp_stat = false;
	}

}
void EmuThreadClassBase::get_cd_string(void)
{
	if(!(using_flags->is_use_compact_disc())) return;

	QString tmpstr;
	uint32_t access_drv = p_emu->is_compact_disc_accessed();
	for(int i = 0; i < (int)using_flags->get_max_cd(); i++) {
		if(p_emu->is_compact_disc_inserted(i)) {
			if((access_drv & (1 << i)) != 0) {
				tmpstr = QString::fromUtf8("<FONT COLOR=DEEPSKYBLUE>●</FONT>");  // U+1F4BF OPTICAL DISC
			} else {
				tmpstr = QString::fromUtf8("<FONT COLOR=BLUE>○</FONT>");  // U+1F4BF OPTICAL DISC
			}				
		} else {
			tmpstr = QString::fromUtf8("×");
		}
		if(tmpstr != cdrom_text[i]) {
			emit sig_set_access_lamp(i + 24, ((access_drv & (1 << i)) != 0));
			emit sig_change_access_lamp(CSP_DockDisks_Domain_CD, i, tmpstr);
			cdrom_text[i] = tmpstr;
		}
	}

}

void EmuThreadClassBase::get_bubble_string(void)
{
	if(!(using_flags->is_use_bubble())) return;

	uint32_t access_drv;
	int i;
	QString alamp;
	QString tmpstr;
	for(i = 0; i < using_flags->get_max_bubble() ; i++) {
		if(p_emu->is_bubble_casette_inserted(i)) {
			alamp = QString::fromUtf8("● ");
			//tmpstr = alamp + QString::fromUtf8(" ");
		} else {
			tmpstr = QString::fromUtf8("×");
			//tmpstr = tmpstr + QString::fromUtf8(" ");
		}
		if(alamp != bubble_text[i]) {
			emit sig_change_access_lamp(CSP_DockDisks_Domain_Bubble, i, tmpstr);
			bubble_text[i] = alamp;
		}
	}

}

bool EmuThreadClassBase::get_power_state(void)
{
	return MainWindow->GetPowerState();
}

double EmuThreadClassBase::get_emu_frame_rate(void)
{
	return p_emu->get_frame_rate();
}

int EmuThreadClassBase::get_message_count(void)
{
	return p_emu->message_count;	
}

const _TCHAR *EmuThreadClassBase::get_emu_message(void)
{
	return (const _TCHAR *)(p_emu->message);
}

bool EmuThreadClassBase::now_debugging()
{
	if(using_flags->is_use_debugger()) {
		return p_emu->now_debugging;
	} else {
		return false;
	}
}

#if defined(Q_OS_LINUX)
//#undef _GNU_SOURCE
#endif
