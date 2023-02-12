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

	if(using_flags->is_use_auto_key()) {
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
	if(using_flags->is_use_auto_key()) {
		p_emu->stop_auto_key();
	}
}

void EmuThreadClassBase::do_write_protect_floppy_disk(int drv, bool flag)
{
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

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(using_flags->is_use_fd())) return;
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
	if(p_emu == nullptr) return;

	int bank_num = p_emu->d88_file[drive].bank_num;
	if(bank_num <= 0) return;

	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
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
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_tape()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->play_tape(drv, name.toLocal8Bit().constData());
	}
}

void EmuThreadClassBase::do_rec_tape(int drv, QString name)
{
	if(using_flags->is_use_tape()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->rec_tape(drv, name.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_CMT, drv, name);
	}
}
void EmuThreadClassBase::sub_close_tape_internal(int drv)
{
	if(using_flags->is_use_tape()) {
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
	// ToDo: limit drive.
	// ToDo: Need update config?
	if(p_config != nullptr) {
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
	if(p_config != nullptr) {
		p_config->direct_load_mzt[drv] = stat;
	}
}

void EmuThreadClassBase::do_cmt_push_play()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	int drv = tmp.drive;
	// ToDo: limit drive.
	if(using_flags->is_use_tape()) {
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
	// ToDo: limit drive.
	if(using_flags->is_use_tape()) {
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
	// ToDo: limit drive.
	if(using_flags->is_use_tape()) {
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
	// ToDo: limit drive.
	if(using_flags->is_use_tape()) {
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
	// ToDo: limit drive.
	if(using_flags->is_use_tape()) {
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
	// ToDo: limit drive.
	if(using_flags->is_use_tape()) {
		////QMutexLocker _locker(&uiMutex);
		p_emu->push_apss_rewind(drv);
	}
}

// QuickDisk
void EmuThreadClassBase::do_write_protect_quickdisk(int drv, bool flag)
{
	if(using_flags->is_use_qd()) {
		////QMutexLocker _locker(&uiMutex);
		//p_emu->write_protect_Qd(drv, flag);
	}
}

void EmuThreadClassBase::do_close_quickdisk(int drv)
{
	if(using_flags->is_use_qd()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_quick_disk(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_QD, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_quickdisk(int drv, QString path)
{
	if(using_flags->is_use_qd()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->open_quick_disk(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_QD, drv, path);
	}
}
// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_quick_disk(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_qd())) return;
	if((drive < 0) || (drive >= using_flags->get_max_qd())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_quick_disk_path[drive], list);
	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_quick_disk_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_quick_disk_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_QD, drive, relpath);
}
void EmuThreadClassBase::done_close_quick_disk(int drive)
{
	emit sig_ui_close_quick_disk(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_QD, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_open_cdrom(int drv, QString path)
{
	if(using_flags->is_use_compact_disc()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->open_compact_disc(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_CD, drv, path);
	}
}
void EmuThreadClassBase::do_eject_cdrom(int drv)
{
	if(using_flags->is_use_compact_disc()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_compact_disc(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_CD, drv, QString::fromUtf8(""));
	}
}
// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_compact_disc(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_compact_disc())) return;
	if((drive < 0) || (drive >= using_flags->get_max_cd())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_compact_disc_path[drive], list);

	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_compact_disc_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_compact_disc_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_CD, drive, relpath);
}
void EmuThreadClassBase::done_close_compact_disc(int drive)
{
	emit sig_ui_close_compact_disc(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_CD, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::sub_close_hard_disk_internal(int drv)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
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

	if(p->is_use_hdd()) {
		//QMutexLocker _locker(&uiMutex);
		const _TCHAR *file_path = (const _TCHAR *)(path.toLocal8Bit().constData());
		if(!(FILEIO::IsFileExisting(file_path))) return; // File not found.
		p_emu->open_hard_disk(drv, file_path);
	}
}

void EmuThreadClassBase::do_close_cart(int drv)
{
	if(using_flags->is_use_cart()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_cart(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_Cart, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_cart(int drv, QString path)
{
	if(using_flags->is_use_cart()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->open_cart(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_Cart, drv, path);
	}
}

// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_cart(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_cart())) return;
	if((drive < 0) || (drive >= using_flags->get_max_cart())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_cart_path[drive], list);

	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_cart_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_cart_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Cart, drive, relpath);
}
void EmuThreadClassBase::done_close_cart(int drive)
{
	emit sig_ui_close_cart(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Cart, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_close_laser_disc(int drv)
{
	if(using_flags->is_use_laser_disc()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_laser_disc(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_LD, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_laser_disc(int drv, QString path)
{
	if(using_flags->is_use_laser_disc()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->open_laser_disc(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_LD, drv, path);
	}
}
// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_laser_disc(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_laser_disc())) return;
	if((drive < 0) || (drive >= using_flags->get_max_ld())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_laser_disc_path[drive], list);

	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_laser_disc_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_laser_disc_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_LD, drive, relpath);
}
void EmuThreadClassBase::done_close_laser_disc(int drive)
{
	emit sig_ui_close_laser_disc(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_LD, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_load_binary(int drv, QString path)
{
	if(using_flags->is_use_binary_file()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->load_binary(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_Binary, drv, path);
	}
}

void EmuThreadClassBase::do_save_binary(int drv, QString path)
{
	if(using_flags->is_use_binary_file()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->save_binary(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_Binary, drv, QString::fromUtf8(""));
	}
}

// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_binary(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_binary_file())) return;
	if((drive < 0) || (drive >= using_flags->get_max_binary())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_binary_path[drive], list);
	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_binary_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_binary_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Binary, drive, relpath);
}
void EmuThreadClassBase::done_close_binary(int drive)
{
	emit sig_ui_close_binary(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Binary, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_write_protect_bubble_casette(int drv, bool flag)
{
	if(using_flags->is_use_bubble()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->is_bubble_casette_protected(drv, flag);
	}
}

void EmuThreadClassBase::do_close_bubble_casette(int drv)
{
	if(using_flags->is_use_bubble()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_bubble_casette(drv);
		p_emu->b77_file[drv].bank_num = 0;
		p_emu->b77_file[drv].cur_bank = -1;
//		emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_bubble_casette(int drv, QString path, int bank)
{
	if(!(using_flags->is_use_bubble())) return;

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
				while(file_offset + 0x2b0 <= file_size && p_emu->b77_file[drv].bank_num < using_flags->get_max_b77_banks()) {
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
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, drv, path);
	emit sig_update_recent_bubble(drv);

}
// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_bubble(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_bubble())) return;
	if((drive < 0) || (drive >= using_flags->get_max_bubble())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_bubble_casette_path[drive], list);

	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_bubble_casette_dir, __dir, _MAX_PATH - 1);

	emit sig_ui_update_bubble_casette_list(drive, list);
	emit sig_ui_clear_b77(drive);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	bool _f = check_file_extension(path_shadow, ".b77");
	if(_f) {
		if(p_emu != nullptr) {
			int slot = p_emu->b77_file[drive].cur_bank;
			for(int i = 0; i < p_emu->b77_file[drive].bank_num; i++) {
				if(i >= 16) break;
				_TCHAR tmpname[128] = {0};
				my_strcpy_s(tmpname, 127, p_emu->b77_file[drive].bubble_name[i]);
				QString tmps = QString::fromLocal8Bit(tmpname);
				emit sig_ui_update_b77(drive, i, tmps);
				if(i == slot) {
					emit sig_ui_select_b77(drive, i);
					relpath = tmps;
				}
			}
		}
	}
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, drive, relpath);
}

void EmuThreadClassBase::done_close_bubble(int drive)
{
	emit sig_ui_close_bubble_casette(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::done_select_b77(int drive, int slot)
{
	if(p_emu == nullptr) return;

	if(slot < 0) return;
	if(slot >= 16) return;
	if(p_emu->b77_file[drive].bank_num < 0) return;
	if(p_emu->b77_file[drive].bank_num >= 64) return;
	if(p_emu->b77_file[drive].bank_num <= slot) return;
	p_emu->b77_file[drive].cur_bank = slot;
	_TCHAR tmpname[128] = {0};
	my_strcpy_s(tmpname, 127, p_emu->b77_file[drive].bubble_name[slot]);
	QString tmps = QString::fromLocal8Bit(tmpname);
	emit sig_ui_select_b77(drive, slot);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, drive, tmps);
}

// Debugger

void EmuThreadClassBase::do_close_debugger(void)
{
	if(using_flags->is_use_debugger()) {
		emit sig_quit_debugger();
	}
}

void EmuThreadClassBase::set_romakana(bool flag)
{
	if(using_flags->is_use_auto_key()) {
		p_emu->set_auto_key_char(flag ? 1 : 0);
	}
}

void EmuThreadClassBase::moved_mouse(double x, double y, double globalx, double globaly)
{
	if(using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0)) {
		mouse_x = x;
		mouse_y = y;
//		bool flag = p_osd->is_mouse_enabled();
//		if(!flag) return;
//		printf("Mouse Moved: %g, %g\n", x, y);
//		p_osd->set_mouse_pointer(floor(x), floor(y));
	} else if(using_flags->is_use_mouse()) {
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

	if(using_flags->is_use_one_board_computer() || using_flags->is_use_mouse() || (using_flags->get_max_button() > 0)) {
		int stat = p_osd->get_mouse_button();
		bool flag = (p_osd->is_mouse_enabled() || using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0));
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

	if(using_flags->is_use_one_board_computer() || using_flags->is_use_mouse() || (using_flags->get_max_button() > 0)) {
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
	p_emu->suspend();
	p_emu->set_host_window_size(w, h, true);
}

void EmuThreadClassBase::dec_message_count(void)
{
	p_emu->message_count--;
}
