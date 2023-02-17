/*
 * UI->Qt->MainWindow : Quick Disk Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */
#include <QApplication>

#include "mainwidget_base.h"
#include "commonclasses.h"
#include "qt_dialogs.h"
#include "csp_logger.h"
#include "menu_quickdisk.h"

#include "dock_disks.h"
#include "../../osdcall_types.h"

void Ui_MainWindowBase::CreateQuickDiskPulldownMenu(int drv)
{
}

void Ui_MainWindowBase::ConfigQuickDiskMenuSub(int drv)
{
}

int Ui_MainWindowBase::do_emu_write_protect_quick_disk(int drv, bool flag)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	if((drv < 0) || (drv >= p->get_max_qd())) return -1;

	emit sig_write_protect_quick_disk(drv, flag);
	return 0;
}

void Ui_MainWindowBase::do_ui_quick_disk_write_protect(int drive, quint64 flag)
{
	if(drive < 0) return;

	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_qd()) || (p->get_max_qd() <= drive)) return;
	if(menu_QDs[drive] == nullptr) return;

	if((flag & EMU_MESSAGE_TYPE::WRITE_PROTECT) != 0) {
		menu_QDs[drive]->do_set_write_protect(true);
	} else {
		menu_QDs[drive]->do_set_write_protect(false);
	}
}

int Ui_MainWindowBase::set_recent_quick_disk(int drv, int num)
{
	QString s_path;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	if(p_config == nullptr) return -1;

	if(p->get_max_qd() <= drv) return -1;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;

	s_path = QString::fromLocal8Bit(p_config->recent_quick_disk_path[drv][num]);
	if(!(s_path.isEmpty())) {
		do_open_quick_disk_ui(drv, s_path);
		return 0;
	}
	return -1;
}

void Ui_MainWindowBase::do_ui_quick_disk_insert_history(int drv, QString fname)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(fname.length() <= 0) return;
	if(p->get_max_qd() <= drv) return;

	_TCHAR path_shadow[_MAX_PATH] = {0};

	my_strncpy_s(path_shadow, _MAX_PATH, fname.toLocal8Bit().constData(), _TRUNCATE);
	if(!(FILEIO::IsFileExisting(path_shadow))) return;

	UPDATE_HISTORY(fname, p_config->recent_quick_disk_path[drv], listQDs[drv]);

	my_strncpy_s(p_config->initial_quick_disk_dir,
				 sizeof(p_config->initial_quick_disk_dir) / sizeof(_TCHAR),
				 get_parent_dir((const _TCHAR *)path_shadow),
				 _TRUNCATE);

	if(menu_QDs[drv] != nullptr) {
		menu_QDs[drv]->do_set_initialize_directory(p_config->initial_quick_disk_dir);
	}
	do_update_quick_disk_history(drv, listQDs[drv]);

	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_QD, drv, fname);
	}

}


void Ui_MainWindowBase::do_open_quick_disk_ui(int drv, const QString fname)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(fname.length() <= 0) return;
	if(p->get_max_qd() <= drv) return;

	const _TCHAR *fnamep = (const _TCHAR*)(fname.toLocal8Bit().constData());
	if(fnamep == nullptr) return;

	if(!(FILEIO::IsFileExisting(fnamep))) return; // File not found.

	emit sig_close_quick_disk_ui(drv);
	emit sig_open_quick_disk(drv, fname);
}

void Ui_MainWindowBase::do_eject_quick_disk(int drv)
{
	emit sig_close_quick_disk_ui(drv);
}

void Ui_MainWindowBase::do_ui_eject_quick_disk(int drv)
{
	if(menu_QDs[drv] != nullptr) {
		menu_QDs[drv]->do_clear_inner_media();
	}
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_QD, drv, QString::fromUtf8(""));
	}
}

void Ui_MainWindowBase::CreateQuickDiskMenu(int drv, int drv_base)
{
	{
		QString ext = "*.mzt *.q20 *.qdf *.gz";
		QString desc1 = "Quick DIsk";

		std::shared_ptr<USING_FLAGS> p = using_flags;
		if(p_config == nullptr) return;
		if(p.get() == nullptr) return;

		menu_QDs[drv] = new Menu_QDClass(menubar, QString::fromUtf8("QD"), p, this, drv);
		menu_QDs[drv]->create_pulldown_menu();

		menu_QDs[drv]->do_clear_inner_media();
		menu_QDs[drv]->do_add_media_extension(ext, desc1);

		SETUP_HISTORY(p_config->recent_quick_disk_path[drv], listQDs[drv]);

		menu_QDs[drv]->do_update_histories(listQDs[drv]);
		menu_QDs[drv]->do_set_initialize_directory(p_config->initial_quick_disk_dir);

	}
}

void Ui_MainWindowBase::retranslateQuickDiskMenu(int drv, int basedrv)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((drv < 0) || (drv >= p->get_max_qd())) return;
	QString drive_name = (QApplication::translate("MenuMedia", "Quick Disk ", 0));
	drive_name += QString::number(basedrv);

	menu_QDs[drv]->retranslateUi();
	menu_QDs[drv]->setTitle(QApplication::translate("MenuMedia", drive_name.toUtf8().constData() , 0));
}

void Ui_MainWindowBase::ConfigQuickDiskMenu(void)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	for(int i = 0; i < p->get_max_qd(); i++) {
		ConfigQuickDiskMenuSub(i);
	}
}

void Ui_MainWindowBase::do_update_quick_disk_history(int drive, QStringList lst)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((drive < 0) || (drive >= p->get_max_qd())) return;
	if(menu_QDs[drive] != nullptr) {
		menu_QDs[drive]->do_update_histories(lst);
	}
}
