/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2016.8.26 Split from emu_thread.cpp
	Note: This class must be compiled per VM, must not integrate shared units.
	[ win32 main ] -> [ Qt main ] -> [Emu Thread] -> [QObject SLOTs depended by VM]
*/

#include <QString>
#include <QTextCodec>
#include <QWaitCondition>

#include <SDL.h>

#include "../gui/emu_thread_tmpl.h"

#include "qt_gldraw.h"
#include "csp_logger.h"
#include "../gui/menu_flags.h"
#include "dock_disks.h"
#include "menu_metaclass.h"


void EmuThreadClassBase::do_start_auto_key(QString ctext)
{
	//QMutexLocker _locker(&uiMutex);
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_auto_key()) {
		QTextCodec *codec = QTextCodec::codecForName("Shift-Jis");
		QByteArray array;
		QVector<uint> ucs4_src = ctext.toUcs4();
		QString dst;
		dst.clear();
		uint32_t pool[8] = {0};
		for(auto itr = ucs4_src.constBegin(); itr != ucs4_src.constEnd(); ++itr) {
			uint val = (*itr);
			int chrs = ucs4_kana_zenkaku_to_hankaku((const uint32_t)val, pool, sizeof(pool) / sizeof(uint32_t));
			if(chrs > 0) {
		#if QT_VERSION >= 0x060000
				dst.append(QString::fromUcs4((char32_t*)pool, chrs));
		#else
				dst.append(QString::fromUcs4((uint*)pool, chrs));
		#endif
			}
		}
		clipBoardText = dst;
		//printf("%s\n", clipBoardText.toLocal8Bit().constData());
		array = codec->fromUnicode(clipBoardText);
		//printf("Array is:");
		//for(int l = 0; l < array.size(); l++) {
		//	printf("%02X ", array.at(l));
		//}
		//printf("\n");
		if(clipBoardText.size() > 0) {
			int size = array.size();
			const char *buf = (char *)(array.constData());
			p_emu->stop_auto_key();
			p_emu->set_auto_key_list((char *)buf, size);
			p_emu->start_auto_key();
		}
	}

}

void EmuThreadClassBase::do_stop_auto_key(void)
{
	//QMutexLocker _locker(&uiMutex);
	//csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL,
	//					  "AutoKey: stop\n");
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_auto_key()) {
		p_emu->stop_auto_key();
	}
}

void EmuThreadClassBase::do_write_protect_floppy_disk(int drv, bool flag)
{
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	//QMutexLocker _locker(&uiMutex);
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if((p->get_max_drive() > drv) && (p->is_use_fd())) {
		p_emu->is_floppy_disk_protected(drv, flag);
	}
}

void EmuThreadClassBase::do_close_floppy_disk()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	sub_close_floppy_disk_internal(drv);
}

void EmuThreadClassBase::do_close_floppy_disk_ui(int drive)
{
	sub_close_floppy_disk_internal(drive);
}

void EmuThreadClassBase::sub_close_floppy_disk_internal(int drv)
{
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	//QMutexLocker _locker(&uiMutex);
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if((p->get_max_drive() > drv) && (p->is_use_fd())) {
		p_emu->close_floppy_disk(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_FD, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_floppy_disk(int drv, QString path, int bank)
{
	if(path.isEmpty()) return;
	if(path.isNull()) return;
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!((p->get_max_drive() > drv) && (p->is_use_fd()))) return;

	const _TCHAR *file_path = (const _TCHAR *)(path.toLocal8Bit().constData());
	if(!(FILEIO::IsFileExisting(file_path))) return; // File not found.

	p_emu->open_floppy_disk(drv, file_path, bank);

	if((p->get_max_drive() > (drv + 1)) && ((drv & 1) == 0) /* EVEN DRIVE NUM */ &&
	   ((bank & EMU_MEDIA_TYPE::MULTIPLE_SLOT_DETECT_MASK) == 0)) {
		if(check_file_extension(file_path, ".d88") || check_file_extension(file_path, ".d77")) {
			if((bank + 1) < get_d88_file_bank_num(drv)) {
				p_emu->close_floppy_disk(drv + 1);
				p_emu->open_floppy_disk(drv + 1, file_path, (bank + 1) | EMU_MEDIA_TYPE::MULTIPLE_SLOT_DETECT_MASK);
			}
		}
	}
}


void EmuThreadClassBase::do_select_floppy_disk_d88(int drive, int slot)
{
	if(drive < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(!((p->is_use_fd()) && (p->get_max_drive() > drive))) return;

	int bank_num = p_emu->d88_file[drive].bank_num;
	if(bank_num <= 0) return;

	if(p->get_max_d88_banks() <= slot) slot = p->get_max_d88_banks() - 1;
	if(slot < 0) return;
	if(bank_num <= slot) return;

	if((p_emu->is_floppy_disk_inserted(drive)) &&
	   (slot != p_emu->d88_file[drive].cur_bank)) {
		QString path = get_d88_file_path(drive);
		do_open_floppy_disk(drive, path, slot);
	}
}

void EmuThreadClassBase::do_play_tape(int drv, QString name)
{
	if(name.isEmpty()) return;
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_tape()) && (p->get_max_tape() > drv)) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->play_tape(drv, name.toLocal8Bit().constData());
	}
}

void EmuThreadClassBase::do_rec_tape(int drv, QString name)
{
	if(name.isEmpty()) return;
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_tape()) && (p->get_max_tape() > drv)) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->rec_tape(drv, name.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_CMT, drv, name);
	}
}

void EmuThreadClassBase::do_close_tape_ui(int drv)
{
	sub_close_tape_internal(drv);
}

void EmuThreadClassBase::sub_close_tape_internal(int drv)
{
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_tape()) && (p->get_max_tape() > drv)) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_tape(drv);
		//emit sig_change_virtual_media(CSP_DockDisks_Domain_CMT, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_close_tape()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	sub_close_tape_internal(drv);
}

void EmuThreadClassBase::do_cmt_wave_shaper(bool stat)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	// ToDo: Need update config?
	if(drv < 0) return;
	if(p_emu == nullptr) return;
	if(p_config == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_tape()) && (p->get_max_tape() > drv)) {
		p_config->wave_shaper[drv] = stat;
	}
}
void EmuThreadClassBase::do_cmt_direct_load_from_mzt(bool stat)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	// ToDo: limit drive.
	if(drv < 0) return;
	if(p_emu == nullptr) return;
	if(p_config == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_tape()) && (p->get_max_tape() > drv)) {
		p_config->direct_load_mzt[drv] = stat;
	}
}

void EmuThreadClassBase::do_cmt_push_play()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_tape()) && (p->get_max_tape() > drv)) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->push_play(drv);
	}
}

void EmuThreadClassBase::do_cmt_push_stop()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_tape()) && (p->get_max_tape() > drv)) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->push_stop(drv);
	}
}

void EmuThreadClassBase::do_cmt_push_fast_forward()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_tape()) && (p->get_max_tape() > drv)) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->push_fast_forward(drv);
	}
}

void EmuThreadClassBase::do_cmt_push_fast_rewind()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_tape()) && (p->get_max_tape() > drv)) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->push_fast_rewind(drv);
	}
}

void EmuThreadClassBase::do_cmt_push_apss_forward()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_tape()) && (p->get_max_tape() > drv)) {
		////QMutexLocker _locker(&uiMutex);
		p_emu->push_apss_forward(drv);
	}
}

void EmuThreadClassBase::do_cmt_push_apss_rewind()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	if(drv < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_tape()) && (p->get_max_tape() > drv)) {
		////QMutexLocker _locker(&uiMutex);
		p_emu->push_apss_rewind(drv);
	}
}

// QuickDisk
void EmuThreadClassBase::do_write_protect_quick_disk(int drv, bool flag)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(p_emu == nullptr) return;
	if(drv < 0) return;

	if((p->get_max_qd() > drv) && (p->is_use_qd())) {
		p_emu->is_quick_disk_protected(drv, flag);
	}
}
void EmuThreadClassBase::do_close_quick_disk()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	sub_close_quick_disk_internal(drv);
}

void EmuThreadClassBase::do_close_quick_disk_ui(int drive)
{
	sub_close_quick_disk_internal(drive);
}

void EmuThreadClassBase::sub_close_quick_disk_internal(int drv)
{
	//QMutexLocker _locker(&uiMutex);
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(p_emu == nullptr) return;
	if(drv < 0) return;

	if((p->get_max_qd() > drv) && (p->is_use_qd())) {
		p_emu->close_quick_disk(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_QD, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_quick_disk(int drv, QString path)
{
	if(path.isEmpty()) return;
	if(path.isNull()) return;
	if(drv < 0) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!((p->get_max_qd() > drv) && (p->is_use_qd()))) return;
	if(p_emu == nullptr) return;

	const _TCHAR *file_path = (const _TCHAR *)(path.toLocal8Bit().constData());
	if(!(FILEIO::IsFileExisting(file_path))) return; // File not found.

	p_emu->open_quick_disk(drv, path.toLocal8Bit().constData());
}


void EmuThreadClassBase::sub_close_compact_disc_internal(int drv)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(p_emu == nullptr) return;
	if(drv < 0) return;

	if((p->is_use_compact_disc()) && (p->get_max_cd() > drv)) {
		//QMutexLocker _locker(&uiMutex);

		p_emu->close_compact_disc(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_CD, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_compact_disc(int drv, QString path)
{
	if(path.isEmpty()) return;
	if(path.isNull()) return;
	if(p_emu == nullptr) return;
	if(drv < 0) return;

	std::shared_ptr<USING_FLAGS>up = using_flags;
	if(up.get() == nullptr) return;
	if(!((up->get_max_cd() > drv) && (up->is_use_compact_disc()))) return;

	const _TCHAR *file_path = (const _TCHAR *)(path.toLocal8Bit().constData());
	if(!(FILEIO::IsFileExisting(file_path))) return; // File not found.

	p_emu->open_compact_disc(drv, file_path);
}

void EmuThreadClassBase::do_eject_compact_disc_ui(int drive)
{
	sub_close_compact_disc_internal(drive);
}

void EmuThreadClassBase::do_eject_compact_disc()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	sub_close_compact_disc_internal(drv);
}

void EmuThreadClassBase::do_close_hard_disk_ui(int drv)
{
	sub_close_hard_disk_internal(drv);
}

void EmuThreadClassBase::sub_close_hard_disk_internal(int drv)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(p_emu == nullptr) return;
	if(drv < 0) return;

	if((p->get_max_hdd() > drv) && (p->is_use_hdd())) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_hard_disk(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_HD, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_close_hard_disk()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	sub_close_hard_disk_internal(drv);
}

void EmuThreadClassBase::do_open_hard_disk(int drv, QString path)
{
	if(path.isEmpty()) return;
	if(path.isNull()) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(p_emu == nullptr) return;
	if(drv < 0) return;

	if((p->is_use_hdd()) && (p->get_max_hdd() > drv)) {
		//QMutexLocker _locker(&uiMutex);
		const _TCHAR *file_path = (const _TCHAR *)(path.toLocal8Bit().constData());
		if(!(FILEIO::IsFileExisting(file_path))) return; // File not found.
		p_emu->open_hard_disk(drv, file_path);
	}
}

void EmuThreadClassBase::do_close_cartridge()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	sub_close_cartridge_internal(drv);
}
void EmuThreadClassBase::do_close_cartridge_ui(int drv)
{
	sub_close_cartridge_internal(drv);
}

void EmuThreadClassBase::sub_close_cartridge_internal(int drv)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(p_emu == nullptr) return;
	if(drv < 0) return;

	if((p->is_use_cart()) && (drv < p->get_max_cart())) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_cart(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_Cart, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_cartridge(int drv, QString path)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(path.isEmpty()) return;
	if(p_emu == nullptr) return;
	if(drv < 0) return;

	if((p->is_use_cart()) && (drv < p->get_max_cart())) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->open_cart(drv, path.toLocal8Bit().constData());
	}
}


void EmuThreadClassBase::do_close_laser_disc_ui(int drive)
{
	sub_close_laser_disc_internal(drive);
}

void EmuThreadClassBase::do_close_laser_disc()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;

	sub_close_laser_disc_internal(drv);
}

void EmuThreadClassBase::sub_close_laser_disc_internal(int drv)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(p_emu == nullptr) return;
	if(drv < 0) return;
	if((p->is_use_laser_disc()) && (p->get_max_ld() > drv)) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_laser_disc(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_LD, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_laser_disc(int drv, QString path)
{
	if(path.isEmpty()) return;
	if(path.isNull()) return;
	if(p_emu == nullptr) return;
	if(drv < 0) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!((p->get_max_ld() > drv) && (p->is_use_laser_disc()))) return;

	const _TCHAR *file_path = (const _TCHAR *)(path.toLocal8Bit().constData());
	if(!(FILEIO::IsFileExisting(file_path))) return; // File not found.

	p_emu->open_laser_disc(drv, file_path);
}

void EmuThreadClassBase::do_load_binary(int drv, QString path)
{
	if(p_emu == nullptr) return;
	if(drv < 0) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!((p->get_max_binary() > drv) && (p->is_use_binary_file()))) return;

	//QMutexLocker _locker(&uiMutex);
	p_emu->load_binary(drv, path.toLocal8Bit().constData());
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Binary, drv, path);
}

void EmuThreadClassBase::do_save_binary(int drv, QString path)
{
	if(p_emu == nullptr) return;
	if(drv < 0) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!((p->get_max_binary() > drv) && (p->is_use_binary_file()))) return;

		//QMutexLocker _locker(&uiMutex);
	p_emu->save_binary(drv, path.toLocal8Bit().constData());
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Binary, drv, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_write_protect_bubble_casette(int drv, bool flag)
{
	if(p_emu == nullptr) return;
	if(drv < 0) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_bubble()) && (drv < p->get_max_bubble())) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->is_bubble_casette_protected(drv, flag);
	}
}

void EmuThreadClassBase::do_close_bubble_casette()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	sub_close_bubble_casette_internal(drv);
}

void EmuThreadClassBase::do_close_bubble_casette_ui(int drive)
{
	sub_close_bubble_casette_internal(drive);
}

void EmuThreadClassBase::sub_close_bubble_casette_internal(int drv)
{
	if(drv < 0) return;
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((p->is_use_bubble()) && (drv < p->get_max_bubble())) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_bubble_casette(drv);
		p_emu->b77_file[drv].bank_num = 0;
		p_emu->b77_file[drv].cur_bank = -1;
		emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_bubble_casette(int drv, QString path, int bank)
{
	if(path.isEmpty()) return;
	if(drv < 0) return;
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(!((p->is_use_bubble()) && (drv < p->get_max_bubble()))) return;

	//QMutexLocker _locker(&uiMutex);
	QByteArray localPath = path.toLocal8Bit();

	p_emu->b77_file[drv].bank_num = 0;
	p_emu->b77_file[drv].cur_bank = -1;

	if(check_file_extension(localPath.constData(), ".b77")) {

		FILEIO *fio = new FILEIO();
		if(fio->Fopen(localPath.constData(), FILEIO_READ_BINARY)) {
			try {
				fio->Fseek(0, FILEIO_SEEK_END);
				int file_size = fio->Ftell(), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && p_emu->b77_file[drv].bank_num < p->get_max_b77_banks()) {
					fio->Fseek(file_offset, FILEIO_SEEK_SET);
					char tmp[18];
					memset(tmp, 0x00, sizeof(tmp));
					fio->Fread(tmp, 16, 1);
					memset(p_emu->b77_file[drv].bubble_name[p_emu->b77_file[drv].bank_num], 0x00, 128);
					if(strlen(tmp) > 0) Convert_CP932_to_UTF8(p_emu->b77_file[drv].bubble_name[p_emu->b77_file[drv].bank_num], tmp, 127, 17);

					fio->Fseek(file_offset + 0x1c, FILEIO_SEEK_SET);
					file_offset += fio->FgetUint32_LE();
					p_emu->b77_file[drv].bank_num++;
				}
				strcpy(p_emu->b77_file[drv].path, path.toUtf8().constData());
				if(bank >= p_emu->b77_file[drv].bank_num) bank = p_emu->b77_file[drv].bank_num - 1;
				if(bank < 0) bank = 0;
				p_emu->b77_file[drv].cur_bank = bank;
			}
			catch(...) {
				bank = 0;
				p_emu->b77_file[drv].bank_num = 0;
			}
		   	fio->Fclose();
		}
	   	delete fio;
	} else {
	   bank = 0;
	}
	p_emu->open_bubble_casette(drv, localPath.constData(), bank);

#if 0 /* Open multiple slots in .B77 . */
	if((p->get_max_bubble() > (drv + 1)) && ((drv & 1) == 0) /* EVEN DRIVE NUM */ &&
	   ((bank & EMU_MEDIA_TYPE::MULTIPLE_SLOT_DETECT_MASK) == 0)) {
		if(check_file_extension(file_path, ".b77")) {
			if((bank + 1) < p_emu->b77_file[drv].bank_num) {
				p_emu->open_bubble_casette(drv + 1, file_path, (bank + 1) | EMU_MEDIA_TYPE::MULTIPLE_SLOT_DETECT_MASK);
			}
		}
	}
#endif
}

void EmuThreadClassBase::do_select_bubble_casette_b77(int drive, int slot)
{
	if(drive < 0) return;
	if(p_emu == nullptr) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(!((p->is_use_bubble()) && (p->get_max_bubble() > drive))) return;

	int bank_num = p_emu->b77_file[drive].bank_num;
	if(bank_num <= 0) return;

	if(p->get_max_d88_banks() <= slot) slot = p->get_max_b77_banks() - 1;
	if(slot < 0) return;
	if(bank_num <= slot) return;

	if((p_emu->is_bubble_casette_inserted(drive)) &&
	   (slot != p_emu->b77_file[drive].cur_bank)) {
		QString path = get_b77_file_path(drive);
		do_open_bubble_casette(drive, path, slot);
	}
}

// Debugger

void EmuThreadClassBase::do_close_debugger(void)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_debugger()) {
		emit sig_quit_debugger();
	}
}

void EmuThreadClassBase::set_romakana(bool flag)
{
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_auto_key()) {
		p_emu->set_auto_key_char(flag ? 1 : 0);
	}
}

void EmuThreadClassBase::moved_mouse(double x, double y, double globalx, double globaly)
{
	if(p_osd == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_one_board_computer() || (p->get_max_button() > 0)) {
		mouse_x = x;
		mouse_y = y;
//		bool flag = p_osd->is_mouse_enabled();
//		if(!flag) return;
//		printf("Mouse Moved: %g, %g\n", x, y);
//		p_osd->set_mouse_pointer(floor(x), floor(y));
	} else if(p->is_use_mouse()) {
//		double factor = (double)(p_config->mouse_sensitivity & ((1 << 16) - 1));
//		mouse_x = (int)(floor((globalx * factor) / 8192.0));
//		mouse_y = (int)(floor((globaly * factor) / 8192.0));
		mouse_x = globalx;
		mouse_y = globaly;
		//printf("Moved Mouse %d, %d\n", x, y);
		bool flag = p_osd->is_mouse_enabled();
		if(!flag) return;
		//printf("Mouse Moved: %d, %d\n", x, y);
		p_osd->set_mouse_pointer(mouse_x, mouse_y);
	}
}

void EmuThreadClassBase::button_pressed_mouse_sub(Qt::MouseButton button)
{

	if(p_osd == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_one_board_computer() || p->is_use_mouse() || (p->get_max_button() > 0)) {
		int stat = p_osd->get_mouse_button();
		bool flag = (p_osd->is_mouse_enabled() || p->is_use_one_board_computer() || (p->get_max_button() > 0));
		switch(button) {
		case Qt::LeftButton:
			stat |= 0x01;
			break;
		case Qt::RightButton:
			stat |= 0x02;
			break;
		case Qt::MiddleButton:
			flag = !flag;
			emit sig_mouse_enable(flag);
			return;
			break;
		default:
			break;
		}
		if(!flag) return;
		p_osd->set_mouse_button(stat);
	}
}

void EmuThreadClassBase::button_released_mouse_sub(Qt::MouseButton button)
{

	if(p_osd == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_one_board_computer() || p->is_use_mouse() || (p->get_max_button() > 0)) {
		int stat = p_osd->get_mouse_button();
		switch(button) {
		case Qt::LeftButton:
			stat &= 0x7ffffffe;
			break;
		case Qt::RightButton:
			stat &= 0x7ffffffd;
			break;
		case Qt::MiddleButton:
			//emit sig_mouse_enable(false);
			break;
		default:
			break;
		}
		p_osd->set_mouse_button(stat);
	}
}

void EmuThreadClassBase::do_notify_power_off()
{
	poweroff_notified = true;
}

void EmuThreadClassBase::do_set_display_size(int w, int h, int ww, int wh)
{
	if(p_emu == nullptr) return;
	p_emu->suspend();
	p_emu->set_host_window_size(w, h, true);
}

void EmuThreadClassBase::dec_message_count(void)
{
	if(p_emu == nullptr) return;
	p_emu->message_count--;
}
