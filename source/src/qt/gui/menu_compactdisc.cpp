/*
 * Qt / Tape Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License : GPLv2
 *   History : 
 *     Mar 20 2016 : Start
 */

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_compactdisc.h"

#include "qt_dialogs.h"
//#include "emu.h"


Menu_CompactDiscClass::Menu_CompactDiscClass(QMenuBar *root_entry, QString desc, USING_FLAGS *p, QWidget *parent, int drv) : Menu_MetaClass(root_entry, desc, p, parent, drv)
{
	use_write_protect = false;
	use_d88_menus = false;
}

Menu_CompactDiscClass::~Menu_CompactDiscClass()
{
}

void Menu_CompactDiscClass::create_pulldown_menu_device_sub(void)
{
	//
}


void Menu_CompactDiscClass::connect_menu_device_sub(void)
{
	connect(this, SIGNAL(sig_open_media(int, QString)),	p_wid, SLOT(do_open_cdrom(int, QString)));
	connect(this, SIGNAL(sig_eject_media(int)),	p_wid, SLOT(do_eject_cdrom(int)));
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_cdrom(int, int)));
}


void Menu_CompactDiscClass::retranslate_pulldown_menu_device_sub(void)
{
	action_insert->setText(QApplication::translate("MainWindow", "Insert Compact Disc", 0));
	action_insert->setToolTip(QApplication::translate("MainWindow", "Insert a image file; CD-ROM or CD audio.", 0));
	action_eject->setText(QApplication::translate("MainWindow", "Eject Compact Disc", 0));
	action_eject->setToolTip(QApplication::translate("MainWindow", "Eject a compact disc.", 0));

	this->setTitle(QApplication::translate("MainWindow", "CD ROM" , 0));
	action_insert->setIcon(QIcon(":/icon_cd.png"));
}
