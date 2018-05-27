/*
 * UI->Qt->MainWindow : FDD Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */


#include "mainwidget_base.h"
#include "commonclasses.h"
#include "menu_harddisk.h"

#include "qt_dialogs.h"
#include "csp_logger.h"

#include "menu_flags.h"

//extern USING_FLAGS *using_flags;
//extern class EMU *emu;


void Ui_MainWindowBase::do_update_recent_hard_disk(int drv)
{
}


int Ui_MainWindowBase::set_recent_hard_disk(int drv, int num) 
{
	return 0;
}

void Ui_MainWindowBase::_open_hard_disk(int drv, const QString fname)
{
}

void Ui_MainWindowBase::eject_hard_disk(int drv) 
{
	emit sig_close_hard_disk(drv);
	menu_hdds[drv]->do_clear_inner_media();
}

// Common Routine

void Ui_MainWindowBase::CreateHardDiskMenu(int drv, int drv_base)
{
	{
		QString ext = "*.thd;*.nhd;*.hdi;*.dat";
		QString desc1 = "Hard Disk Drive";
		menu_hdds[drv] = new Menu_HDDDClass(menubar, QString::fromUtf8("HDD"), using_flags, this, drv, drv_base);
		menu_hdds[drv]->create_pulldown_menu();
		
		menu_hdds[drv]->do_clear_inner_media();
		menu_hdds[drv]->do_add_media_extension(ext, desc1);
		SETUP_HISTORY(using_flags->get_config_ptr()->recent_hard_disk_path[drv], listHDDs[drv]);
		menu_hdds[drv]->do_update_histories(listHDDs[drv]);
		menu_hdds[drv]->do_set_initialize_directory(using_flags->get_config_ptr()->initial_hard_disk_dir);
	}
}

void Ui_MainWindowBase::CreateHardDiskPulldownMenu(int drv)
{
}

void Ui_MainWindowBase::ConfigHardDiskMenuSub(int drv)
{
}

void Ui_MainWindowBase::retranslateHardDiskMenu(int drv, int basedrv)
{
	QString s = QApplication::translate("MenuMedia", "HDD", 0);
	s = s + QString::number(basedrv);
	retranslateHardDiskMenu(drv, basedrv, s);
}

void Ui_MainWindowBase::retranslateHardDiskMenu(int drv, int basedrv, QString specName)
{
	QString drive_name;
	drive_name = QString::fromUtf8("[") + QString::number(basedrv) + QString::fromUtf8(":] ");
	drive_name = drive_name + specName;
	//drive_name += QString::number(basedrv);
  
	if((drv < 0) || (drv >= using_flags->get_max_drive())) return;
	menu_hdds[drv]->setTitle(QApplication::translate("MenuMedia", drive_name.toUtf8().constData() , 0));
	menu_hdds[drv]->retranslateUi();
}

void Ui_MainWindowBase::ConfigHardDiskMenu(void)
{
	for(int i = 0; i < using_flags->get_max_hdd(); i++) {
		ConfigHardDiskMenuSub(i);
	}
}
