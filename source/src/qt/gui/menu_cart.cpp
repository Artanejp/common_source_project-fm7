/*
 * UI->Qt->MainWindow : Cart Menu.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */

//#include "menuclasses.h"
#include <QApplication>

#include "mainwidget_base.h"
#include "menu_metaclass.h"
#include "menu_cart.h"

#include "emu_thread_tmpl.h"
#include "qt_dialogs.h"

Menu_CartClass::Menu_CartClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent, int drv, int base_drv) : Menu_MetaClass(root_entry, desc, p, parent, drv, base_drv)
{
	use_write_protect = false;
	use_d88_menus = false;
}

Menu_CartClass::~Menu_CartClass()
{
}

void Menu_CartClass::connect_via_emu_thread(EmuThreadClassBase *p)
{
	if(p == nullptr) return;
	connect(action_eject, SIGNAL(triggered()), p, SLOT(do_close_cartridge()), Qt::QueuedConnection);

}

void Menu_CartClass::create_pulldown_menu_device_sub(void)
{
}


void Menu_CartClass::connect_menu_device_sub(void)
{
   	connect(this, SIGNAL(sig_open_media_load(int, QString)), p_wid, SLOT(do_open_cartridge_ui(int, QString)));
//	connect(this, SIGNAL(sig_eject_media(int)), p_wid, SLOT(eject_cart(int)));
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_cart(int, int)));
}

void Menu_CartClass::retranslate_pulldown_menu_device_sub(void)
{
	int drv = media_drive;
	QString drive_name = (QApplication::translate("MenuMedia", "Cartridge ", 0));
	drive_name += QString::number(drv);

	if((drv < 0) || (drv >= 8)) return;
	action_insert->setText(QApplication::translate("MenuMedia", "Insert", 0));
	action_eject->setText(QApplication::translate("MenuMedia", "Eject", 0));
	action_insert->setToolTip(QApplication::translate("MenuMedia", "Insert a cartridge image file.", 0));
	action_eject->setToolTip(QApplication::translate("MenuMedia", "Eject a cartridge image.", 0));

	menu_history->setTitle(QApplication::translate("MenuMedia", "Recent Opened", 0));
	this->setTitle(QApplication::translate("MenuMedia", drive_name.toUtf8().constData() , 0));
}
