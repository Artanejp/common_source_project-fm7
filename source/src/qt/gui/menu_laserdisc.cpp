/*
 * Qt / Tape Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License : GPLv2
 *   History : 
 *     Mar 20 2016 : Start
 */
#include <QApplication>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_laserdisc.h"

#include "qt_dialogs.h"
//#include "emu.h"


Menu_LaserdiscClass::Menu_LaserdiscClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent, int drv, int base_drv) : Menu_MetaClass(root_entry, desc, p, parent, drv, base_drv)
{
	use_write_protect = false;
	use_d88_menus = false;
}

Menu_LaserdiscClass::~Menu_LaserdiscClass()
{
}

void Menu_LaserdiscClass::create_pulldown_menu_device_sub(void)
{
	//
}


void Menu_LaserdiscClass::connect_menu_device_sub(void)
{
	connect(this, SIGNAL(sig_open_media(int, QString)),	p_wid, SLOT(do_open_laserdisc(int, QString)));
	connect(this, SIGNAL(sig_eject_media(int)),	p_wid, SLOT(do_eject_laserdisc(int)));
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_laserdisc(int, int)));
}


void Menu_LaserdiscClass::retranslate_pulldown_menu_device_sub(void)
{
	action_insert->setText(QApplication::translate("MenuMedia", "Insert Laserdisc", 0));
	action_insert->setToolTip(QApplication::translate("MenuMedia", "Insert a MOVIE file.", 0));
	action_eject->setText(QApplication::translate("MenuMedia", "Eject Laserdisc", 0));
	action_eject->setToolTip(QApplication::translate("MenuMedia", "Eject a MOVIE file.", 0));

	this->setTitle(QApplication::translate("MenuMedia", "Laserdisc" , 0));
	action_insert->setIcon(QIcon(":/icon_cd.png"));
}
