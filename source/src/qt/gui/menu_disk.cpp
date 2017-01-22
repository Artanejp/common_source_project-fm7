/*
 * Qt / DIsk Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   LIcense: GPLv2
 *   History: Jan 10, 2015 (MAYBE) : Initial.
 */

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_disk.h"

#include "qt_dialogs.h"
//#include "emu.h"


Menu_FDClass::Menu_FDClass(EMU *ep, QMenuBar *root_entry, QString desc, USING_FLAGS *p, QWidget *parent, int drv) : Menu_MetaClass(ep, root_entry, desc, p, parent, drv)
{
	use_write_protect = true;
	use_d88_menus = true;
	icon_floppy = QIcon(":/icon_floppy.png");
}

Menu_FDClass::~Menu_FDClass()
{
}

void Menu_FDClass::create_pulldown_menu_device_sub(void)
{
	action_ignore_crc_error = new Action_Control(p_wid, using_flags);
	action_ignore_crc_error->setVisible(true);
	action_ignore_crc_error->setCheckable(true);
	connect(action_ignore_crc_error->binds, SIGNAL(sig_emu_update_config()),
			p_wid, SLOT(do_emu_update_config()));
	
	action_correct_timing = new Action_Control(p_wid, using_flags);
	action_correct_timing->setVisible(true);
	action_correct_timing->setCheckable(true);
	connect(action_correct_timing->binds, SIGNAL(sig_emu_update_config()),
			p_wid, SLOT(do_emu_update_config()));
}


void Menu_FDClass::connect_menu_device_sub(void)
{
	this->addSeparator();
	this->addAction(action_ignore_crc_error);
	this->addAction(action_correct_timing);
	
	connect(action_ignore_crc_error, SIGNAL(toggled(bool)),
			action_ignore_crc_error->binds, SLOT(do_set_ignore_crc_error(bool)));
	
	connect(action_correct_timing, SIGNAL(toggled(bool)),
			action_correct_timing->binds, SLOT(do_set_correct_disk_timing(bool)));
   
   	connect(this, SIGNAL(sig_open_media(int, QString)), p_wid, SLOT(_open_disk(int, QString)));
	connect(this, SIGNAL(sig_eject_media(int)), p_wid, SLOT(eject_fd(int)));
	connect(this, SIGNAL(sig_write_protect_media(int, bool)), p_wid, SLOT(write_protect_fd(int, bool)));	
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_disk(int, int)));
	connect(this, SIGNAL(sig_set_inner_slot(int, int)), p_wid, SLOT(set_d88_slot(int, int)));
}

void Menu_FDClass::retranslate_pulldown_menu_device_sub(void)
{
	action_insert->setIcon(icon_floppy);
	action_insert->setToolTip(QApplication::translate("MainWindow", "Insert virtual floppy disk file.", 0));
	action_eject->setToolTip(QApplication::translate("MainWindow", "Eject virtual floppy disk.", 0));
	
	action_ignore_crc_error->setText(QApplication::translate("MainWindow", "Ignore CRC error", 0));
	action_ignore_crc_error->setToolTip(QApplication::translate("MainWindow", "Ignore CRC error of virtual floppy.\nUseful for some softwares,\n but causes wrong working with some softwares.", 0));
	action_correct_timing->setText(QApplication::translate("MainWindow", "Correct transfer timing", 0));
	action_correct_timing->setToolTip(QApplication::translate("MainWindow", "Correct transferring timing.\nUseful for some softwares\n needs strict transfer timing.", 0));
}


