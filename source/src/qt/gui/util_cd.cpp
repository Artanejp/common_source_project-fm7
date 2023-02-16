/*
 * UI->Qt->MainWindow : CD ROM Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Mar 20, 2016 : Initial
 */

#include <QApplication>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "qt_dialogs.h"
#include "csp_logger.h"

#include "menu_compactdisc.h"
#include "dock_disks.h"

void Ui_MainWindowBase::CreateCDROMMenu(int drv, int drv_base)
{
	QString ext_play, desc_play;

	listCDROM[drv].clear();
	menu_CDROM[drv] = new Menu_CompactDiscClass(menubar, "CDROM", using_flags, this, drv, drv_base);
	menu_CDROM[drv]->setObjectName(QString::fromUtf8("menuCDROM", -1));

	menu_CDROM[drv]->create_pulldown_menu();
	// Translate Menu
	SETUP_HISTORY(p_config->recent_compact_disc_path[drv], listCDROM[drv]);
	menu_CDROM[drv]->do_update_histories(listCDROM[drv]);
	menu_CDROM[drv]->do_set_initialize_directory(p_config->initial_compact_disc_dir);

	ext_play = "*.ccd *.cue *.iso *.bin *.gz";
	desc_play = "Compact Disc";
	menu_CDROM[drv]->do_add_media_extension(ext_play, desc_play);

}

void Ui_MainWindowBase::CreateCDROMPulldownMenu(void)
{
}

void Ui_MainWindowBase::ConfigCDROMMenuSub(void)
{

}

int Ui_MainWindowBase::set_recent_compact_disc(int drv, int num)
{
	QString s_path;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	if(p_config == nullptr) return -1;

	if(p->get_max_cd() <= drv) return -1;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;

	s_path = QString::fromLocal8Bit(p_config->recent_compact_disc_path[drv][num]);
	if(s_path.isEmpty()) return -1;
	do_open_compact_disc(drv, s_path);
	return 0;
}

void Ui_MainWindowBase::do_eject_compact_disc(int drv)
{
	emit sig_eject_compact_disc_ui(drv);
}

void Ui_MainWindowBase::do_ui_eject_compact_disc(int drv)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(p->get_max_cd() <= drv) return;
	if(menu_CDROM[drv] != nullptr) {
		menu_CDROM[drv]->do_clear_inner_media();
	}
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_CD, drv, QString::fromUtf8(""));
	}
}

void Ui_MainWindowBase::do_ui_compact_disc_insert_history(int drv, QString fname)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(fname.length() <= 0) return;

	if(using_flags->get_max_cd() <= drv) return;

	_TCHAR path_shadow[_MAX_PATH] = {0};

	my_strncpy_s(path_shadow, _MAX_PATH, fname.toLocal8Bit().constData(), _TRUNCATE);
	if(!(FILEIO::IsFileExisting(path_shadow))) return;

	UPDATE_HISTORY(path_shadow, p_config->recent_compact_disc_path[drv], listCDROM[drv]);
	my_strncpy_s(p_config->initial_compact_disc_dir,
				 sizeof(p_config->initial_compact_disc_dir) / sizeof(_TCHAR),
				 get_parent_dir((const _TCHAR*)path_shadow),
				 _TRUNCATE);
	my_strncpy_s(path_shadow,
				 _MAX_PATH,
				 fname.toLocal8Bit().constData(),
				 _TRUNCATE);
	menu_CDROM[drv]->do_set_initialize_directory(p_config->initial_compact_disc_dir);
	do_update_compact_disc_history(drv, listCDROM[drv]);

	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_CD, drv, fname);
	}
}

void Ui_MainWindowBase::do_open_compact_disc(int drv, QString path)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(path.length() <= 0) return;
	if(p->get_max_cd() <= drv) return;

	const _TCHAR *fnamep = (const _TCHAR*)(path.toLocal8Bit().constData());
	if(fnamep == nullptr) return;
	if(!(FILEIO::IsFileExisting(fnamep))) return; // File not found.

	emit sig_eject_compact_disc_ui(drv);
	emit sig_open_compact_disc(drv, path);

	return;
}

void Ui_MainWindowBase::retranslateCDROMMenu(void)
{
	if(using_flags->is_use_compact_disc()) {
		for(int drv = 0; drv < using_flags->get_max_cd(); drv++) {
			menu_CDROM[drv]->retranslateUi();
		}
	}
}

void Ui_MainWindowBase::ConfigCDROMMenu(void)
{
	ConfigCDROMMenuSub();
}

void Ui_MainWindowBase::do_swap_cdaudio_byteorder(bool value)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int drv = cp->data().value<int>();

	std::shared_ptr<USING_FLAGS>up = using_flags;
	if((p_config != nullptr) && (up.get() != nullptr)) {
		if(drv < 0) return;
		if(drv >= up->get_max_cd()) return;
		p_config->swap_audio_byteorder[drv] = value;
	}
}

void Ui_MainWindowBase::do_update_compact_disc_history(int drive, QStringList lst)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((drive < 0) || (drive >= p->get_max_cd())) return;
	if(menu_CDROM[drive] != nullptr) {
		menu_CDROM[drive]->do_update_histories(lst);
	}
}
