/*
 * UI->Qt->MainWindow : Cartridge Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */

#include "mainwidget.h"
#include "commonclasses.h"
#include "menu_cart.h"
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"


#ifdef USE_CART1
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
#endif

void Ui_MainWindow::_open_cart(int drv, const QString fname)
{
	char path_shadow[PATH_MAX];
	int i;
#ifdef USE_CART1
	if(fname.length() <= 0) return;
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_cart_path[drv], listCARTs[drv]);
	get_parent_dir(path_shadow);
	strcpy(config.initial_cart_dir, path_shadow);
	menu_Cart[drv]->do_update_histories(listCARTs[drv]);
	menu_Cart[drv]->do_set_initialize_directory(config.initial_cart_dir);
	
	emit sig_close_cart(drv);
	emit sig_open_cart(drv, fname);
#endif
}

#if defined(USE_CART1) || defined(USE_CART2)

void Ui_MainWindow::eject_cart(int drv) 
{
	emit sig_close_cart(drv);
}

void Ui_MainWindow::set_recent_cart(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	int i;
    
	if((num < 0) || (num >= MAX_HISTORY)) return;
 
	s_path = QString::fromLocal8Bit(config.recent_cart_path[drv][num]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_cart_path[drv], listCARTs[drv]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
   
	get_parent_dir(path_shadow);
	strcpy(config.initial_cart_dir, path_shadow);
	menu_Cart[drv]->do_update_histories(listCARTs[drv]);
	menu_Cart[drv]->do_set_initialize_directory(config.initial_cart_dir);
   
	eject_cart(drv);
	emit sig_open_cart(drv, s_path);
}
#endif

void Ui_MainWindow::CreateCartMenu(int drv, int drv_base)
{
#ifdef USE_CART1
	QString ext;
	QString desc;
	
	QString drv_base_name = QString::number(drv_base); 
	
#if defined(_GAMEGEAR)
	ext = "*.rom *.bin *.gg *.col";
	desc = "Game Cartridge";
#elif defined(_MASTERSYSTEM)
	ext = "*.rom *.bin *.sms";
	desc = "Game Cartridge";
#elif defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
	ext = "*.rom *.bin *.60";
	desc = "Game Cartridge";
#elif defined(_PCENGINE) || defined(_X1TWIN)
	ext = "*.rom *.bin *.pce";
	desc = "HuCARD";
#elif defined(_Z80TVGAME)
	ext = "*.rom *.bin *.hex";
	desc = "GameData";
#else
	ext = "*.rom *.bin";
	desc = "Game Cartridge";
#endif
	
	menu_Cart[drv] = new Menu_CartClass(emu, menubar, QString::fromUtf8("Obj_Cart"), this, drv);	
	menu_Cart[drv]->create_pulldown_menu();
		
	menu_Cart[drv]->do_clear_inner_media();
	menu_Cart[drv]->do_add_media_extension(ext, desc);
	SETUP_HISTORY(config.recent_cart_path[drv], listCARTs[drv]);
	menu_Cart[drv]->do_update_histories(listCARTs[drv]);
	menu_Cart[drv]->do_set_initialize_directory(config.initial_cart_dir);

	QString name = QString::fromUtf8("Cart");
	QString tmpv;
	tmpv.setNum(drv_base);
	name.append(tmpv);
	menu_Cart[drv]->setTitle(name);
#endif
}

void Ui_MainWindow::CreateCartPulldownMenu(int drv)
{
}

void Ui_MainWindow::ConfigCartMenuSub(int drv)
{
}

void Ui_MainWindow::retranslateCartMenu(int drv, int basedrv)
{
#ifdef USE_CART1
	menu_Cart[drv]->retranslateUi();
#endif	
}

void Ui_MainWindow::ConfigCartMenu(void)
{
#if defined(USE_CART1)
	ConfigCartMenuSub(0); 
#endif
#if defined(USE_CART2)
	ConfigCartMenuSub(1);
#endif
}
