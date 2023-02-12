/*
 * UI->Qt->MainWindow : FDD Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */
#include <QApplication>

#include "mainwidget_base.h"
#include "commonclasses.h"
#include "menu_harddisk.h"
#include "dock_disks.h"

#include "qt_dialogs.h"
#include "csp_logger.h"

#include "menu_flags.h"

// Common Routine

void Ui_MainWindowBase::CreateHardDiskMenu(int drv, int drv_base)
{
	{
		QString ext = "*.thd *.nhd *.hdi *.hdd *.h[0-9]";
		QString desc1 = "Hard Disk Drive";
		menu_hdds[drv] = new Menu_HDDClass(menubar, QString::fromUtf8("HDD"), using_flags, this, drv, drv_base);
		menu_hdds[drv]->create_pulldown_menu();

		menu_hdds[drv]->do_clear_inner_media();
		menu_hdds[drv]->do_add_media_extension(ext, desc1);
		SETUP_HISTORY(p_config->recent_hard_disk_path[drv], listHDDs[drv]);
		menu_hdds[drv]->do_update_histories(listHDDs[drv]);
		menu_hdds[drv]->do_set_initialize_directory(p_config->initial_hard_disk_dir);
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
	drive_name = QString::fromUtf8(":");
	drive_name = specName + drive_name;
	//drive_name += QString::number(basedrv);

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((drv < 0) || (drv >= p->get_max_hdd())) return;
	menu_hdds[drv]->setTitle(QApplication::translate("MenuMedia", drive_name.toUtf8().constData() , 0));
	menu_hdds[drv]->retranslateUi();
}

void Ui_MainWindowBase::ConfigHardDiskMenu(void)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	for(int i = 0; i < p->get_max_hdd(); i++) {
		ConfigHardDiskMenuSub(i);
	}
}
