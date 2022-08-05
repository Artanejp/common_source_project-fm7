/*
 * Qt / DIsk Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   LIcense: GPLv2
 *   History: Jan 10, 2015 (MAYBE) : Initial.
 *            Nov 13, 2015 : Integrate to Menu_MetaClass.
 */

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_quickdisk.h"

#include "qt_dialogs.h"
//#include "emu.h"

Menu_QDClass::Menu_QDClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent, int drv, int base_drv) : Menu_MetaClass(root_entry, desc, p, parent, drv, base_drv)
{
	QString tmps;
	use_write_protect = true;
	tmps.setNum(drv);
}

Menu_QDClass::~Menu_QDClass()
{
}

void Menu_QDClass::create_pulldown_menu_device_sub(void)
{
}


void Menu_QDClass::connect_menu_device_sub(void)
{
	this->addSeparator();
   
   	connect(this, SIGNAL(sig_open_media(int, QString)), p_wid, SLOT(_open_quick_disk(int, QString)));
	connect(this, SIGNAL(sig_eject_media(int)), p_wid, SLOT(eject_Qd(int)));
	connect(this, SIGNAL(sig_write_protect_media(int, bool)), p_wid, SLOT(write_protect_Qd(int, bool)));	
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_quick_disk(int, int)));
}

void Menu_QDClass::retranslate_pulldown_menu_device_sub(void)
{

}



