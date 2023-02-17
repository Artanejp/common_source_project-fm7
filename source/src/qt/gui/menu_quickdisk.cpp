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
#include "emu_thread_tmpl.h"

Menu_QDClass::Menu_QDClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent, int drv, int base_drv) : Menu_MetaClass(root_entry, desc, p, parent, drv, base_drv)
{
	use_write_protect = true;
	use_d88_menus = false;
}

Menu_QDClass::~Menu_QDClass()
{
}

void Menu_QDClass::create_pulldown_menu_device_sub(void)
{
}

void Menu_QDClass::connect_via_emu_thread(EmuThreadClassBase *p)
{
	if(p == nullptr) return;
	connect(action_eject, SIGNAL(triggered()), p, SLOT(do_close_quick_disk()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_write_protect_media(int, bool)), p, SLOT(do_write_protect_quick_disk(int, bool)), Qt::QueuedConnection);
}

void Menu_QDClass::connect_menu_device_sub(void)
{
   	connect(this, SIGNAL(sig_open_media(int, QString)), p_wid, SLOT(do_open_quick_disk_ui(int, QString)));

	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_quick_disk(int, int)));
	//connect(this, SIGNAL(sig_write_protect_media(int, bool)), p_wid, SLOT(do_emu_write_protect_quick_disk(int, bool)));

}

void Menu_QDClass::retranslate_pulldown_menu_device_sub(void)
{

}
