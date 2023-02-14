/*
 * UI->Qt->MainWindow : CD ROM Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Mar 20, 2016 : Initial
 */


#include "commonclasses.h"
#include "mainwidget_base.h"
#include "qt_dialogs.h"
#include "csp_logger.h"

#include "menu_laserdisc.h"
#include "dock_disks.h"

void Ui_MainWindowBase::CreateLaserdiscMenu(int drv, int drv_base)
{
	QString ext_play, desc_play;

	listLaserdisc[drv].clear();
	menu_Laserdisc[drv] = new Menu_LaserdiscClass(menubar, "Laserdisc", using_flags, this, drv, drv_base);
	menu_Laserdisc[drv]->setObjectName(QString::fromUtf8("menuLaserdisc", -1));

	menu_Laserdisc[drv]->create_pulldown_menu();

	ext_play = "*.ogv *.mp4 *.avi *.mkv";
	desc_play = "Laserisc";
	menu_Laserdisc[drv]->do_clear_inner_media();
	menu_Laserdisc[drv]->do_add_media_extension(ext_play, desc_play);

	SETUP_HISTORY(p_config->recent_laser_disc_path[drv], listLaserdisc[drv]);
	menu_Laserdisc[drv]->do_update_histories(listLaserdisc[drv]);
	menu_Laserdisc[drv]->do_set_initialize_directory(p_config->initial_laser_disc_dir);


}

void Ui_MainWindowBase::CreateLaserdiscPulldownMenu(void)
{
}

void Ui_MainWindowBase::ConfigLaserdiscMenuSub(void)
{

}


int Ui_MainWindowBase::set_recent_laserdisc(int drv, int num)
{
	QString s_path;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	if(p_config == nullptr) return -1;

	if(p->get_max_ld() <= drv) return -1;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;

	s_path = QString::fromLocal8Bit(p_config->recent_laser_disc_path[drv][num]);
	if(s_path.isEmpty()) return -1;
	do_open_laserdisc(drv, s_path);
	return 0;
}

void Ui_MainWindowBase::do_eject_laserdisc(int drv)
{
	emit sig_close_laserdisc_ui(drv);
}

void Ui_MainWindowBase::do_ui_eject_laser_disc(int drv)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(p->get_max_ld() <= drv) return;
	if(menu_Laserdisc[drv] != nullptr) {
		menu_Laserdisc[drv]->do_clear_inner_media();
	}
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_LD, drv, QString::fromUtf8(""));
	}
}

void Ui_MainWindowBase::do_ui_laser_disc_insert_history(int drv, QString fname)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(fname.length() <= 0) return;

	if(using_flags->get_max_ld() <= drv) return;

	_TCHAR path_shadow[_MAX_PATH] = {0};

	my_strncpy_s(path_shadow, _MAX_PATH, fname.toLocal8Bit().constData(), _TRUNCATE);
	if(!(FILEIO::IsFileExisting(path_shadow))) return;

	UPDATE_HISTORY(path_shadow, p_config->recent_laser_disc_path[drv], listLaserdisc[drv]);
	my_strncpy_s(p_config->initial_laser_disc_dir,
				 sizeof(p_config->initial_laser_disc_dir) / sizeof(_TCHAR),
				 get_parent_dir((const _TCHAR*)path_shadow),
				 _TRUNCATE);
	my_strncpy_s(path_shadow,
				 _MAX_PATH,
				 fname.toLocal8Bit().constData(),
				 _TRUNCATE);
	menu_Laserdisc[drv]->do_set_initialize_directory(p_config->initial_laser_disc_dir);
	do_update_laser_disc_history(drv, listLaserdisc[drv]);

	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_LD, drv, fname);
	}
}

void Ui_MainWindowBase::do_open_laserdisc(int drv, QString fname)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(fname.length() <= 0) return;
	if(p->get_max_ld() <= drv) return;

	const _TCHAR *fnamep = (const _TCHAR*)(fname.toLocal8Bit().constData());
	if(fnamep == nullptr) return;
	if(!(FILEIO::IsFileExisting(fnamep))) return; // File not found.

	emit sig_close_laserdisc_ui(drv);
	emit sig_open_laserdisc(drv, fname);

	return;
}

void Ui_MainWindowBase::retranslateLaserdiscMenu(void)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_laser_disc()) {
		for(int drv = 0; drv < p->get_max_ld(); drv++) {
			if(menu_Laserdisc[drv] != nullptr) {
				menu_Laserdisc[drv]->retranslateUi();
			}
		}
	}
}

void Ui_MainWindowBase::ConfigLaserdiscMenu(void)
{
	ConfigLaserdiscMenuSub();
}

void Ui_MainWindowBase::do_update_laser_disc_history(int drive, QStringList lst)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((drive < 0) || (drive >= p->get_max_ld())) return;
	if(menu_Laserdisc[drive] != nullptr) {
		menu_Laserdisc[drive]->do_update_histories(lst);
	}
}
