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

//#include <SDL.h>

#include "emu_thread_tmpl.h"
#include "mainwidget_base.h"
#include "qt_gldraw.h"
#include "common.h"
#include "dock_disks.h"
#include "../../osdcall_types.h"

#include "virtualfileslist.h"
#include "menu_metaclass.h"

#include "menu_flags.h"

EmuThreadClassBase::EmuThreadClassBase(Ui_MainWindowBase *rootWindow, std::shared_ptr<USING_FLAGS> p, QObject *parent) : QThread(parent)
{
	MainWindow = rootWindow;
	bBlockTask = true;
	using_flags = p;
	p_config = p->get_config_ptr();
	p_emu = nullptr;
	p_osd = nullptr;
	poweroff_notified = false; // OK?

	is_shared_glcontext = false;
	glContext = nullptr;
	glContext = new QOpenGLContext(this);

	if(glContext != nullptr) {
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
	update_fps_time = 0;

	total_frames = 0;
	draw_frames = 0;
	skip_frames = 0;
	mouse_flag = false;

	drawCond = new QWaitCondition();
//	keyMutex = new QMutex(QMutex::Recursive);

	mouse_x = 0;
	mouse_y = 0;
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if((p.get() != nullptr) && (p_config != nullptr)) {
		if(up->is_use_tape() && !(up->is_tape_binary_only())) {
			tape_play_flag = false;
			tape_rec_flag = false;
			tape_pos = 0;
		}

		if(up->get_use_sound_volume() > 0) {
			for(int i = 0; i < up->get_use_sound_volume(); i++) {
				bUpdateVolumeReq[i] = true;
				volume_avg[i] = (p_config->sound_volume_l[i] +
								 p_config->sound_volume_r[i]) / 2;
				volume_balance[i] = (p_config->sound_volume_r[i] -
									 p_config->sound_volume_l[i]) / 2;
			}
		}
	}
	connect(this, SIGNAL(sig_draw_finished()), rootWindow->getGraphicsView(), SLOT(do_quit()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_emu_finished()), rootWindow->getGraphicsView(), SLOT(deleteLater()));
	virtualMediaList.clear();

	QMutexLocker _n(&keyMutex);

	key_fifo = new FIFO(512 * 6);
	key_fifo->clear();
}

EmuThreadClassBase::~EmuThreadClassBase()
{

	delete drawCond;

	key_fifo->release();
	delete key_fifo;
};

void EmuThreadClassBase::do_start(QThread::Priority prio)
{
	start(prio);
}

void EmuThreadClassBase::doExit(void)
{
	bRunThread = false;
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

void EmuThreadClassBase::do_special_reset(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();

	if(num < 0) return;

	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() != nullptr) {
		if(num >= up->get_use_special_reset_num()) return;
	}
	bSpecialResetReq = true;
	specialResetNum = num;
}

void EmuThreadClassBase::do_load_state(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	QString s = cp->data().toString();

	lStateFile = s;
	bLoadStateReq = true;
}

void EmuThreadClassBase::do_save_state(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	QString s = cp->data().toString();

	sStateFile = s;
	bSaveStateReq = true;
}

void EmuThreadClassBase::do_start_record_video()
{
	if(p_config == nullptr) return;
	int fps = p_config->video_frame_rate;
	record_fps = fps;
	bStartRecordMovieReq = true;
}

void EmuThreadClassBase::do_stop_record_video()
{
	bStartRecordMovieReq = false;
}

void EmuThreadClassBase::do_update_volume_level(int num, int level)
{
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() == nullptr) return;
	if(up->get_use_sound_volume() > 0) {
		if((num < up->get_use_sound_volume()) && (num >= 0)) {
			calc_volume_from_level(num, level);
			bUpdateVolumeReq[num] = true;
		}
	}
}

void EmuThreadClassBase::do_update_volume_balance(int num, int level)
{
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() == nullptr) return;
	if(up->get_use_sound_volume() > 0) {
		if((num < up->get_use_sound_volume()) && (num >= 0)) {
			calc_volume_from_balance(num, level);
			bUpdateVolumeReq[num] = true;
		}
	}
}
int EmuThreadClassBase::parse_drive(QString key)
{
	int index = 0;
	for(int i = (key.size() - 1); i != 0; --i) {
		if((key.at(i) < '0') || (key.at(i) > '9')) break;
		index++;
	}
	if(index <= 0) return 0;
	if(index > 2) index = 2;
	QString s = key.right(index);
	return s.toInt();
}

void EmuThreadClassBase::parse_file(QString val, QString& filename)
{
	ssize_t check_at = val.lastIndexOf(QString::fromUtf8("@"));
	if((check_at < 0) || (check_at >= (val.size() - 1))) { // Not Found
		filename = val;
	} else {
		QString tmps = val.right(check_at + 1);
		filename = tmps;
	}
}

void EmuThreadClassBase::parse_file_slot(QString val, QString& filename, bool& protect_changed, bool& is_protected, int& slot )
{
	ssize_t check_at = val.lastIndexOf(QString::fromUtf8("@"));
	protect_changed = false;
	if((check_at < 0) || (check_at >= (val.size() - 1))) { // Not Found
		filename = val;
	} else {
		QString tmps = val.right(check_at + 1);
		filename = tmps;
		ssize_t _at2 = val.indexOf(QString::fromUtf8("@"));
		while((_at2 < val.size()) && (_at2 >= 0))  {
			ssize_t _at3 = val.indexOf(QString::fromUtf8("@"), _at2 + 1);
			if(_at3 <= (_at2 + 1)) {
				break;
			}
			QString tmps2 = val.mid(_at2 + 1, _at3 - _at2);
			if(!(tmps2.isEmpty())) {
				if(tmps2.toLower() == QString::fromUtf8("wp")) {
					is_protected = true;
					protect_changed = true;
				} else if(tmps2.toLower() == QString::fromUtf8("w")) {
					is_protected = false;
					protect_changed = true;
				} else {
					bool _ok;
					int _t = tmps.toInt(&_ok);
					if(_ok) {
						slot = _t & EMU_MEDIA_TYPE::EMU_SLOT_MASK;
					}
				}
			}
			_at2 = _at3;
		}
	}
}

int EmuThreadClassBase::parse_command_queue(QMap<QString, QString> __list)
{
	int _ret = 0;
	for(auto _s = __list.constBegin(); _s != __list.constEnd(); ++_s) {
		QString _key = _s.key();
		QString _val = _s.value();
		//printf("%s %s\n", _key.toLocal8Bit().constData(), _val.toLocal8Bit().constData());
		int slot = 0;
		QString _file = QString::fromUtf8("");
		bool is_protected = false;
		bool protect_changed = false;
		if(!(_key.isEmpty())) {
			int drv = parse_drive(_key);
			if(_key.contains("vBinary", Qt::CaseInsensitive)) {
				parse_file(_val, _file);
				do_load_binary(drv, _file);
				_ret++;
			} else if(_key.contains("vSaveBinary", Qt::CaseInsensitive)) {
				parse_file(_val, _file);
				do_save_binary(drv, _file);
				_ret++;
			} else if(_key.contains("vBubble", Qt::CaseInsensitive)) {
				parse_file_slot(_val, _file, protect_changed, is_protected, slot);
				do_open_bubble_casette(drv, _file, slot);
				if(protect_changed) {
					do_write_protect_bubble_casette(drv, is_protected);
				}
				_ret++;
			} else if(_key.contains("vCart", Qt::CaseInsensitive)) {
				parse_file(_val, _file);
				do_open_cartridge(drv, _file);
				_ret++;
			} else if(_key.contains("vCompactDisc", Qt::CaseInsensitive)) {
				parse_file(_val, _file);
				do_open_compact_disc(drv, _file);
				_ret++;
			} else if(_key.contains("vFloppy", Qt::CaseInsensitive)) {
				parse_file_slot(_val, _file, protect_changed, is_protected, slot);
				do_open_floppy_disk(drv, _file, slot);
				if(protect_changed) {
					do_write_protect_floppy_disk(drv, is_protected);
				}
				_ret++;
			} else if(_key.contains("vHardDisk", Qt::CaseInsensitive)) {
				parse_file(_val, _file);
				do_open_hard_disk(drv, _file);
				_ret++;
			} else if(_key.contains("vLaserDisc", Qt::CaseInsensitive)) {
				parse_file(_val, _file);
				do_open_laser_disc(drv, _file);
				_ret++;
			} else if(_key.contains("vQuickDisk", Qt::CaseInsensitive)) {
				parse_file_slot(_val, _file, protect_changed, is_protected, slot);
				do_open_quick_disk(drv, _file);
				if(protect_changed) {
					do_write_protect_quick_disk(drv, is_protected);
				}
				_ret++;
			} else if(_key.contains("vTape", Qt::CaseInsensitive)) {
				parse_file(_val, _file);
				do_play_tape(drv, _file);
				_ret++;
			} else if(_key.contains("vSaveCMT", Qt::CaseInsensitive)) {
				parse_file_slot(_val, _file, protect_changed, is_protected, slot);
				if(!(is_protected) && (protect_changed)) {
					do_rec_tape(drv, _file);
				}
				_ret++;
			} else if(_key.contains("vCloseBubble", Qt::CaseInsensitive)) {
				do_close_bubble_casette_ui(drv);
				_ret++;
			} else if(_key.contains("vCloseCart", Qt::CaseInsensitive)) {
				do_close_cartridge_ui(drv);
				_ret++;
			} else if(_key.contains("vCloseCompactDisc", Qt::CaseInsensitive)) {
				do_eject_compact_disc_ui(drv);
				_ret++;
			} else if(_key.contains("vCloseFloppy", Qt::CaseInsensitive)) {
				do_close_floppy_disk_ui(drv);
				_ret++;
			} else if(_key.contains("vCloseHardDisk", Qt::CaseInsensitive)) {
				do_close_hard_disk_ui(drv);
				_ret++;
			} else if(_key.contains("vCloseLaserDisc", Qt::CaseInsensitive)) {
				do_close_laser_disc_ui(drv);
				_ret++;
			} else if(_key.contains("vCloseQuickDisk", Qt::CaseInsensitive)) {
				do_close_quick_disk_ui(drv);
				_ret++;
			} else if(_key.contains("vCloseTape", Qt::CaseInsensitive)) {
				do_close_tape_ui(drv);
				_ret++;
			} else if(_key.contains("vCloseSaveCMT", Qt::CaseInsensitive)) {
				do_close_tape_ui(drv);
				_ret++;
			}
			// ToDo: Write Protect
			// ToDo: Choose SLOT
		}
	}
	return _ret;
}


void EmuThreadClassBase::do_print_framerate(int frames)
{
	if(frames >= 0) draw_frames += frames;
	qint64 current_time = (qint64)get_current_tick_usec();
	//qint64	current_time = SDL_GetTicks();

	if((update_fps_time <= current_time) && (update_fps_time != 0)) {
		_TCHAR buf[256];
		QString message;
		//int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);

		if((poweroff_notified) || (p_emu == nullptr)) {
			my_stprintf_s(buf, 255, _T("*Power OFF*"));
		} else if(now_skip) {
			int ratio = (int)(100.0 * (((double)total_frames / get_emu_frame_rate())  * 2.0) + 0.5);
			my_stprintf_s(buf, 255, _T("%s - Skip Frames (%d %%)"), get_device_name(), ratio);
		} else {
			if(get_message_count() > 0) {
				snprintf(buf, 255, _T("%s - %s"), get_device_name(), get_emu_message());
				dec_message_count();
			} else {
				int ratio;
				__LIKELY_IF(p_emu != NULL) {
					ratio = lrint(100.0 * ((double)draw_frames / (p_emu->get_frame_rate() * ((double)(current_time - update_fps_time + (1000 * 1000)) / 1.0e6))));
				} else {
					ratio = (int)(100.0 * ((double)draw_frames / (double)total_frames) * 2.0 + 0.5);
				}
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
		update_fps_time = current_time + (1000 * 1000);
		//update_fps_time += (1000 * 1000);
		total_frames = draw_frames = 0;
	}
	if(update_fps_time <= current_time) {
		update_fps_time = current_time + (1000 * 1000);
	}
}

int EmuThreadClassBase::get_d88_file_cur_bank(int drive)
{
	if(drive < 0) return -1;
	if(p_emu == nullptr) return -1;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	if(!(p->is_use_fd())) return -1;

	if((drive < p->get_max_drive()) && (p_emu != nullptr)) {
//		QMutexLocker _locker(&uiMutex);
		int bank_num = p_emu->d88_file[drive].bank_num;
		int cur_bank = p_emu->d88_file[drive].cur_bank;
		if((bank_num > 0) && (cur_bank < bank_num)) {
			return cur_bank;
		}
	}

	return -1;
}

int EmuThreadClassBase::get_d88_file_bank_num(int drive)
{
	if(drive < 0) return -1;
	if(p_emu == nullptr) return -1;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	if(!(p->is_use_fd())) return -1;

	if(drive < p->get_max_drive()) {
//		QMutexLocker _locker(&uiMutex);
		return p_emu->d88_file[drive].bank_num;
	}

	return -1;
}


QString EmuThreadClassBase::get_d88_file_disk_name(int drive, int banknum)
{
	if(drive < 0) return QString::fromUtf8("");
	if(p_emu == nullptr) return QString::fromUtf8("");

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return QString::fromUtf8("");
	if(!(p->is_use_fd())) return QString::fromUtf8("");

	if((drive < p->get_max_drive()) && (banknum < get_d88_file_bank_num(drive))) {
//		QMutexLocker _locker(&uiMutex);
		QString _n = QString::fromLocal8Bit((const char *)(&(p_emu->d88_file[drive].disk_name[banknum][0])));
		return _n;
	}

	return QString::fromUtf8("");
}


bool EmuThreadClassBase::is_floppy_disk_protected(int drive)
{
	if(drive < 0) return false;
	if(p_emu == nullptr) return false;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return false;
	if(!(p->is_use_fd())) return false;

//	QMutexLocker _locker(&uiMutex);
	bool _b = p_emu->is_floppy_disk_protected(drive);
	return _b;
}


QString EmuThreadClassBase::get_d88_file_path(int drive)
{
	if(drive < 0) return QString::fromUtf8("");
	if(p_emu == nullptr) return QString::fromUtf8("");

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return QString::fromUtf8("");
	if(!(p->is_use_fd())) return QString::fromUtf8("");

	if(drive < p->get_max_drive()) {
//		QMutexLocker _locker(&uiMutex);
		QString _n = QString::fromLocal8Bit((const char *)(&(p_emu->d88_file[drive].path)));
		return _n;
	}

	return QString::fromUtf8("");
}

void EmuThreadClassBase::set_floppy_disk_protected(int drive, bool flag)
{
	if(drive < 0) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_fd())) return;
//	QMutexLocker _locker(&uiMutex);
	p_emu->is_floppy_disk_protected(drive, flag);
}


int EmuThreadClassBase::get_b77_file_cur_bank(int drive)
{
	if(drive < 0) return -1;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	if(!(p->is_use_bubble())) return -1;

	if((drive < p->get_max_bubble()) && (p_emu != nullptr)) {
//		QMutexLocker _locker(&uiMutex);
		int bank_num = p_emu->b77_file[drive].bank_num;
		int cur_bank = p_emu->b77_file[drive].cur_bank;
		if((bank_num > 0) && (cur_bank < bank_num)) {
			return cur_bank;
		}
	}
	return -1;
}

int EmuThreadClassBase::get_b77_file_bank_num(int drive)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	if(!(p->is_use_bubble())) return -1;

	if((drive < p->get_max_bubble()) && (p_emu != nullptr)) {
//		QMutexLocker _locker(&uiMutex);
		return p_emu->b77_file[drive].bank_num;
	}
	return -1;
}


QString EmuThreadClassBase::get_b77_file_media_name(int drive, int banknum)
{
	if(drive < 0) return QString::fromUtf8("");
	if(p_emu == nullptr) return QString::fromUtf8("");

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return QString::fromUtf8("");
	if(!(p->is_use_bubble())) return QString::fromUtf8("");

	if((drive < p->get_max_bubble()) && (banknum < get_b77_file_bank_num(drive))) {
//		QMutexLocker _locker(&uiMutex);
		QString _n = QString::fromLocal8Bit((const char *)(&(p_emu->b77_file[drive].bubble_name[banknum][0])));
		return _n;
	}

	return QString::fromUtf8("");
}


bool EmuThreadClassBase::is_bubble_casette_protected(int drive)
{
	if(drive < 0) return false;
	if(p_emu == nullptr) return false;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return false;
	if(!(p->is_use_bubble())) return false;

//	QMutexLocker _locker(&uiMutex);
	bool _b = p_emu->is_floppy_disk_protected(drive);
	return _b;
}


void EmuThreadClassBase::set_bubble_casette_protected(int drive, bool flag)
{
	if(drive < 0) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_bubble())) return;
//	QMutexLocker _locker(&uiMutex);
	p_emu->is_bubble_casette_protected(drive, flag);
}

QString EmuThreadClassBase::get_b77_file_path(int drive)
{
	if(drive < 0) return QString::fromUtf8("");
	if(p_emu == nullptr) return QString::fromUtf8("");

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return QString::fromUtf8("");
	if(!(p->is_use_bubble())) return QString::fromUtf8("");

	if(drive < p->get_max_bubble()) {
//		QMutexLocker _locker(&uiMutex);
		QString _n = QString::fromLocal8Bit((const char *)(&(p_emu->b77_file[drive].path)));
		return _n;
	}

	return QString::fromUtf8("");
}

void EmuThreadClassBase::sample_access_drv(void)
{
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() == nullptr) return;
	if(up->is_use_qd()) get_qd_string();
	if(up->is_use_fd()) get_fd_string();
	if(up->is_use_hdd()) get_hdd_string();
	if(up->is_use_tape() && !(up->is_tape_binary_only())) get_tape_string();
	if(up->is_use_compact_disc()) get_cd_string();
	if(up->is_use_bubble()) get_bubble_string();
}

void EmuThreadClassBase::get_fd_string(void)
{
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_fd())) return;

	int i;
	QString tmpstr;
	QString iname;
	QString alamp;
	uint32_t access_drv = 0;
	bool lamp_stat = false;
	access_drv = p_emu->is_floppy_disk_accessed();
	{
		for(i = 0; i < (int)p->get_max_drive(); i++) {
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
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_qd())) return;

	int i;
	QString iname;
	QString alamp;
	QString tmpstr;
	uint32_t access_drv = 0;
	bool lamp_stat = false;
	access_drv = p_emu->is_quick_disk_accessed();
	for(i = 0; i < p->get_max_qd(); i++) {
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
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!((p->is_use_tape()) || (p->is_tape_binary_only()))) return;

	QString tmpstr;
	bool readwrite;
	bool inserted;

	for(int i = 0; i < p->get_max_tape(); i++) {
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
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_hdd())) return;

	QString tmpstr, alamp;
	uint32_t access_drv = p_emu->is_hard_disk_accessed();
	bool lamp_stat = false;
	alamp.clear();
	tmpstr.clear();
	for(int i = 0; i < (int)p->get_max_hdd(); i++) {
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
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() == nullptr) return;
	if(!(up->is_use_compact_disc())) return;

	QString tmpstr;
	uint32_t access_drv = p_emu->is_compact_disc_accessed();
	for(int i = 0; i < (int)up->get_max_cd(); i++) {
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
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() == nullptr) return;
	if(!(up->is_use_bubble())) return;

	uint32_t access_drv;
	int i;
	QString alamp;
	QString tmpstr;
	for(i = 0; i < up->get_max_bubble() ; i++) {
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

double EmuThreadClassBase::get_emu_frame_rate(void)
{
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if((up.get() == nullptr) || (p_emu == nullptr)) return 30.0;
	return p_emu->get_frame_rate();
}

int EmuThreadClassBase::get_message_count(void)
{
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if((up.get() == nullptr) || (p_emu == nullptr)) return 0;
	return p_emu->message_count;
}

const _TCHAR *EmuThreadClassBase::get_emu_message(void)
{
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if((up.get() == nullptr) || (p_emu == nullptr)) return ((const _TCHAR *)_T(""));
	return (const _TCHAR *)(p_emu->message);
}

bool EmuThreadClassBase::now_debugging()
{
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() == nullptr) return false;
	if(up->is_use_debugger()) {
		return p_emu->now_debugging;
	} else {
		return false;
	}
}


#if defined(Q_OS_LINUX)
//#undef _GNU_SOURCE
#endif
