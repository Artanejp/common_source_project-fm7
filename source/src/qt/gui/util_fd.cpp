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
#include "menu_disk.h"

#include "qt_dialogs.h"
#include "csp_logger.h"

#include "menu_flags.h"

//extern std::shared_ptr<USING_FLAGS> using_flags;
//extern class EMU *emu;


int Ui_MainWindowBase::do_emu_write_protect_floppy_disk(int drv, bool flag)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	
	if((drv < 0) || (drv >= p->get_max_drive())) return -1;
	
	emit sig_write_protect_floppy_disk(drv, flag);
	return 0;
}
  

void Ui_MainWindowBase::eject_fd(int drv) 
{
	emit sig_close_floppy_disk(drv);
	menu_fds[drv]->do_clear_inner_media();
}

// Common Routine

void Ui_MainWindowBase::CreateFloppyMenu(int drv, int drv_base)
{
	{
		QString ext = "*.d88 *.d77 *.1dd *.td0 *.imd *.dsk *.nfd *.fdi *.hdm *.hd5 *.hd4 *.hdb *.dd9 *.dd6 *.tfd *.xdf *.2d *.sf7 *.img *.ima *.vfd";
		QString desc1 = "Floppy Disk";
		menu_fds[drv] = new Menu_FDClass(menubar, QString::fromUtf8("Floppy"), using_flags, this, drv, drv_base);
		menu_fds[drv]->create_pulldown_menu();
		
		menu_fds[drv]->do_clear_inner_media();
		menu_fds[drv]->do_add_media_extension(ext, desc1);

	
		SETUP_HISTORY(p_config->recent_floppy_disk_path[drv], listFDs[drv]);
		menu_fds[drv]->do_update_histories(listFDs[drv]);
		menu_fds[drv]->do_set_initialize_directory(p_config->initial_floppy_disk_dir);
		listD88[drv].clear();
	}
}

void Ui_MainWindowBase::CreateFloppyPulldownMenu(int drv)
{
}

void Ui_MainWindowBase::ConfigFloppyMenuSub(int drv)
{
}

void Ui_MainWindowBase::retranslateFloppyMenu(int drv, int basedrv)
{
	QString s = QApplication::translate("MenuMedia", "FDD", 0);
	s = s + QString::number(basedrv);
	retranslateFloppyMenu(drv, basedrv, s);
}

void Ui_MainWindowBase::retranslateFloppyMenu(int drv, int basedrv, QString specName)
{
	QString drive_name;
	drive_name = QString::fromUtf8("[") + QString::number(basedrv) + QString::fromUtf8(":] ");
	drive_name = drive_name + specName;
	//drive_name += QString::number(basedrv);
  
	if((drv < 0) || (drv >= using_flags->get_max_drive())) return;
	menu_fds[drv]->setTitle(QApplication::translate("MenuMedia", drive_name.toUtf8().constData() , 0));
	menu_fds[drv]->retranslateUi();
}

void Ui_MainWindowBase::ConfigFloppyMenu(void)
{
	for(int i = 0; i < using_flags->get_max_drive(); i++) {
		ConfigFloppyMenuSub(i);
	}
}

void Ui_MainWindowBase::do_update_floppy_history(int drive, QStringList lst)
{
	if((drive < 0) || (drive >= using_flags->get_max_drive())) return;
	if(menu_fds[drive] != nullptr) {
		menu_fds[drive]->do_update_histories(lst);
	}
}

void Ui_MainWindowBase::do_insert_floppy_history(int drive, QString path)
{
	if((drive < 0) || (drive >= using_flags->get_max_drive())) {
		if(menu_fds[drive] != nullptr) {
			menu_fds[drive]->do_insert_history(path);
		}
	}
}
