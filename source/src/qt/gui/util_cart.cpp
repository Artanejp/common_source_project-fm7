/*
 * UI->Qt->MainWindow : Cartridge Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */

#include "mainwidget_base.h"
#include "commonclasses.h"
#include "menu_cart.h"
#include "qt_dialogs.h"
//#include "csp_logger.h"

void Object_Menu_Control::insert_cart(void) {
	emit sig_insert_cart(getDrive());
}
void Object_Menu_Control::eject_cart(void) {
	write_protect = false;
	emit sig_eject_cart(getDrive());
}
void Object_Menu_Control::on_recent_cart(void){
	emit set_recent_cart(drive, s_num);
}

void Ui_MainWindowBase::_open_cart(int drv, const QString fname)
{
	char path_shadow[PATH_MAX];

	if(fname.length() <= 0) return;
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, using_flags->get_config_ptr()->recent_cart_path[drv], listCARTs[drv]);
	get_parent_dir(path_shadow);
	strcpy(using_flags->get_config_ptr()->initial_cart_dir, path_shadow);
	menu_Cart[drv]->do_update_histories(listCARTs[drv]);
	menu_Cart[drv]->do_set_initialize_directory(using_flags->get_config_ptr()->initial_cart_dir);
	
	emit sig_close_cart(drv);
	emit sig_open_cart(drv, fname);

}

void Ui_MainWindowBase::eject_cart(int drv) 
{
	emit sig_close_cart(drv);
}

void Ui_MainWindowBase::set_recent_cart(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
    
	if((num < 0) || (num >= MAX_HISTORY)) return;
 
	s_path = QString::fromLocal8Bit(using_flags->get_config_ptr()->recent_cart_path[drv][num]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, using_flags->get_config_ptr()->recent_cart_path[drv], listCARTs[drv]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
   
	get_parent_dir(path_shadow);
	strcpy(using_flags->get_config_ptr()->initial_cart_dir, path_shadow);
	menu_Cart[drv]->do_update_histories(listCARTs[drv]);
	menu_Cart[drv]->do_set_initialize_directory(using_flags->get_config_ptr()->initial_cart_dir);
   
	//eject_cart(drv);
	emit sig_open_cart(drv, s_path);
}

void Ui_MainWindowBase::CreateCartMenu(int drv, int drv_base)
{
	QString ext;
	QString desc;
	
	QString drv_base_name = QString::number(drv_base); 
	
	if(using_flags->is_machine_gamegear()) {
		ext = "*.rom *.bin *.gg *.col";
		desc = "Game Cartridge";
	} else if(using_flags->is_machine_mastersystem()) {
		ext = "*.rom *.bin *.sms";
		desc = "Game Cartridge";
	} else if(using_flags->is_machine_pc6001()) {
		ext = "*.rom *.bin *.60";
		desc = "Game Cartridge";
	} else if(using_flags->is_machine_has_pcengine()) {
		ext = "*.rom *.bin *.pce";
		desc = "HuCARD";
	} else if(using_flags->is_machine_z80tvgame()) {
		ext = "*.rom *.bin *.hex";
		desc = "GameData";
	} else if(using_flags->is_machine_sc3000()) {
		ext = "*.rom *.bin *.sms *.sg";
		desc = "SC-3000/1000 Game Cartridge";
	} else {
		ext = "*.rom *.bin";
		desc = "Game Cartridge";
	}
	
	menu_Cart[drv] = new Menu_CartClass(menubar, QString::fromUtf8("Obj_Cart"), using_flags, this, drv);	
	menu_Cart[drv]->create_pulldown_menu();
		
	menu_Cart[drv]->do_clear_inner_media();
	menu_Cart[drv]->do_add_media_extension(ext, desc);
	SETUP_HISTORY(using_flags->get_config_ptr()->recent_cart_path[drv], listCARTs[drv]);
	menu_Cart[drv]->do_update_histories(listCARTs[drv]);
	menu_Cart[drv]->do_set_initialize_directory(using_flags->get_config_ptr()->initial_cart_dir);

	QString name = QString::fromUtf8("Cart");
	QString tmpv;
	tmpv.setNum(drv_base);
	name.append(tmpv);
	menu_Cart[drv]->setTitle(name);
}

void Ui_MainWindowBase::CreateCartPulldownMenu(int drv)
{
}

void Ui_MainWindowBase::ConfigCartMenuSub(int drv)
{
}

void Ui_MainWindowBase::retranslateCartMenu(int drv, int basedrv)
{
	menu_Cart[drv]->retranslateUi();
}

void Ui_MainWindowBase::ConfigCartMenu(void)
{
	for(int i = 0; i > using_flags->get_max_cart(); i++) {
		ConfigCartMenuSub(0);
	}
}
