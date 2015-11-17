/*
 * Qt / DIsk Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   LIcense: GPLv2
 *   History: Jan 10, 2015 (MAYBE) : Initial.
 */

#include "commonclasses.h"
#include "mainwidget.h"
#include "menu_disk.h"

#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"


Menu_FDClass::Menu_FDClass(EMU *ep, QMenuBar *root_entry, QString desc, QWidget *parent, int drv) : Menu_MetaClass(ep, root_entry, desc, parent, drv)
{
	use_write_protect = true;
	use_d88_menus = true;
}

Menu_FDClass::~Menu_FDClass()
{
}

void Menu_FDClass::create_pulldown_menu_device_sub(void)
{
	action_ignore_crc_error = new Action_Control(p_wid);
	action_ignore_crc_error->setVisible(true);
	action_ignore_crc_error->setCheckable(true);
	connect(action_ignore_crc_error->binds, SIGNAL(sig_emu_update_config()),
			p_wid, SLOT(do_emu_update_config()));
	
	action_correct_timing = new Action_Control(p_wid);
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
	action_ignore_crc_error->setText(QApplication::translate("MainWindow", "Ignore CRC error", 0));
	action_correct_timing->setText(QApplication::translate("MainWindow", "Correct transfer timing", 0));
}


