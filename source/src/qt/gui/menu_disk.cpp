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


Menu_FDClass::Menu_FDClass(QMenuBar *root_entry, QString desc, USING_FLAGS *p, QWidget *parent, int drv) : Menu_MetaClass(root_entry, desc, p, parent, drv)
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
	config_t *p;
	action_ignore_crc_error = new Action_Control(p_wid, using_flags);
	action_ignore_crc_error->setVisible(true);
	action_ignore_crc_error->setCheckable(true);
	
	action_correct_timing = new Action_Control(p_wid, using_flags);
	action_correct_timing->setVisible(true);
	action_correct_timing->setCheckable(true);

	action_count_immediate = new Action_Control(p_wid, using_flags);
	action_count_immediate->setVisible(true);
	action_count_immediate->setCheckable(true);
	p = using_flags->get_config_ptr();
	if(p != NULL) {
		if(p->correct_disk_timing[media_drive]) action_correct_timing->setChecked(true);
		if(p->ignore_disk_crc[media_drive]) action_ignore_crc_error->setChecked(true);
		if(p->disk_count_immediate[media_drive]) action_count_immediate->setChecked(true);
	}		
}


void Menu_FDClass::connect_menu_device_sub(void)
{
	this->addSeparator();
	this->addAction(action_ignore_crc_error);
	this->addAction(action_correct_timing);
	this->addAction(action_count_immediate);
	
	connect(action_ignore_crc_error, SIGNAL(toggled(bool)),
			action_ignore_crc_error->binds, SLOT(do_set_ignore_crc_error(bool)));
	
	connect(action_correct_timing, SIGNAL(toggled(bool)),
			action_correct_timing->binds, SLOT(do_set_correct_disk_timing(bool)));
	
	connect(action_count_immediate, SIGNAL(toggled(bool)),
			action_count_immediate->binds, SLOT(do_set_disk_count_immediate(bool)));
   
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
	
	action_count_immediate->setText(QApplication::translate("MainWindow", "Immediate increment", 0));
	action_count_immediate->setToolTip(QApplication::translate("MainWindow", "Increment data pointer immediately.\nThis is test hack for MB8877.\nUseful for some softwares\n needs strict transfer timing.", 0));
}


