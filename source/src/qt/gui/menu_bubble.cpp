/*
 * Qt / Bubble casette Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   LIcense: GPLv2
 *   History: Mar 24, 2016  : Inhelit from Menu_FDClass.
 */

#include "commonclasses.h"
#include "mainwidget.h"
#include "menu_disk.h"
#include "menu_bubble.h"

#include "qt_dialogs.h"
#include "emu.h"

Menu_BubbleClass::Menu_BubbleClass(EMU *ep, QMenuBar *root_entry, QString desc, QWidget *parent, int drv) : Menu_MetaClass(ep, root_entry, desc, parent, drv)
{
	use_write_protect = true;
	use_d88_menus = true;
	//icon_bubble_casette = QIcon(":/icon_floppy.png");
}

Menu_BubbleClass::~Menu_BubbleClass()
{
}

void Menu_BubbleClass::create_pulldown_menu_device_sub(void)
{
}

void Menu_BubbleClass::connect_menu_device_sub(void)
{
   	connect(this, SIGNAL(sig_open_media(int, QString)), p_wid, SLOT(_open_bubble(int, QString)));
	connect(this, SIGNAL(sig_eject_media(int)), p_wid, SLOT(eject_bubble(int)));
	connect(this, SIGNAL(sig_write_protect_media(int, bool)), p_wid, SLOT(write_protect_bubble(int, bool)));	
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_bubble(int, int)));
	connect(this, SIGNAL(sig_set_inner_slot(int, int)), p_wid, SLOT(set_b77_slot(int, int)));
}

void Menu_BubbleClass::retranslate_pulldown_menu_device_sub(void)
{
	//action_insert->setIcon(icon_floppy);
}

