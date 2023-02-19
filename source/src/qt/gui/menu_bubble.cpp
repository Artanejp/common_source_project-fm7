/*
 * Qt / Bubble casette Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   LIcense: GPLv2
 *   History: Mar 24, 2016  : Inhelit from Menu_FDClass.
 */
#include <QApplication>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_disk.h"
#include "menu_bubble.h"

#include "qt_dialogs.h"

Menu_BubbleClass::Menu_BubbleClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent, int drv, int base_drv) : Menu_MetaClass(root_entry, desc, p, parent, drv, base_drv)
{
	use_write_protect = true;
	use_d88_menus = true;
}

Menu_BubbleClass::~Menu_BubbleClass()
{
}

void Menu_HDDClass::connect_via_emu_thread(EmuThreadClassBase *p)
{
	if(p == nullptr) return;
	connect(action_eject, SIGNAL(triggered()), p, SLOT(do_close_bubble_casette()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_write_protect_media(int, bool)), p, SLOT(do_write_protect_bubble_casette(int, bool)));
}

void Menu_BubbleClass::create_pulldown_menu_device_sub(void)
{
}

void Menu_BubbleClass::connect_menu_device_sub(void)
{
   	connect(this, SIGNAL(sig_open_media(int, QString)), p_wid, SLOT(_open_bubble(int, QString)));
	//connect(this, SIGNAL(sig_eject_media(int)), p_wid, SLOT(eject_bubble(int)));
//	connect(this, SIGNAL(sig_write_protect_media(int, bool)), p_wid, SLOT(write_protect_bubble(int, bool)));
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_bubble(int, int)));
	connect(this, SIGNAL(sig_set_inner_slot(int, int)), p_wid, SLOT(set_b77_slot(int, int)));
}

void Menu_BubbleClass::retranslate_pulldown_menu_device_sub(void)
{

}
