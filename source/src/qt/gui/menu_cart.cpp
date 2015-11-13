/*
 * UI->Qt->MainWindow : Cart Menu.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */

//#include "menuclasses.h"
#include "commonclasses.h"
#include "mainwidget.h"
#include "menu_metaclass.h"
#include "menu_cart.h"

#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"


Menu_CartClass::Menu_CartClass(EMU *ep, QMenuBar *root_entry, QString desc, QWidget *parent, int drv) : Menu_MetaClass(ep, root_entry, desc, parent, drv)
{
	use_write_protect = false;
	use_d88_menus = false;
}

Menu_CartClass::~Menu_CartClass()
{
}

void Menu_CartClass::create_pulldown_menu_device_sub(void)
{
}


void Menu_CartClass::connect_menu_device_sub(void)
{
   	connect(this, SIGNAL(sig_open_media(int, QString)), p_wid, SLOT(_open_cart(int, QString)));
	connect(this, SIGNAL(sig_eject_media(int)), p_wid, SLOT(eject_cart(int)));
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_cart(int, int)));
}

void Menu_CartClass::retranslate_pulldown_menu_device_sub(void)
{
	int drv = media_drive;
	QString drive_name = (QApplication::translate("MainWindow", "Cartridge ", 0));
	drive_name += QString::number(drv);
  
	if((drv < 0) || (drv >= 8)) return;
	action_insert->setText(QApplication::translate("MainWindow", "Insert", 0));
	action_eject->setText(QApplication::translate("MainWindow", "Eject", 0));

	menu_history->setTitle(QApplication::translate("MainWindow", "Recent Opened", 0));
	this->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0));
}
