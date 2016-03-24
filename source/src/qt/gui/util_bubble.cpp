/*
 * UI->Qt->MainWindow : Bubble Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Mar 24, 2016 : Initial.
 */


#include "mainwidget.h"
#include "commonclasses.h"
#include "menu_bubble.h"

#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"

extern class EMU *emu;

void Object_Menu_Control::insert_bubble(void) {
	emit sig_insert_bubble(getDrive());
}
void Object_Menu_Control::eject_bubble(void) {
	write_protect = false;
	emit sig_eject_bubble(getDrive());
}
void Object_Menu_Control::on_b77_slot(void) {
	emit set_b77_slot(drive, s_num);
}
void Object_Menu_Control::on_recent_bubble(void){
	emit set_recent_bubble(drive, s_num);
}
void Object_Menu_Control::write_protect_bubble(void) {
	write_protect = true;
	emit sig_write_protect_bubble(drive, write_protect);
}
void Object_Menu_Control::no_write_protect_bubble(void) {
	write_protect = false;
	emit sig_write_protect_bubble(drive, write_protect);
}

#ifdef USE_BUBBLE1
#ifndef UPDATE_B77_LIST
#define UPDATE_B77_LIST(__d, lst) { \
	lst.clear(); \
	QString __tmps; \
	for(int iii = 0; iii < MAX_B77_BANKS; iii++) { \
		__tmps = QString::fromUtf8(""); \
		if(iii < emu->b77_file[__d].bank_num) { \
	 		__tmps = QString::fromUtf8(emu->b77_file[__d].bubble_name[iii]); \
		} \
	lst << __tmps; \
	} \
}
#endif
#endif


int Ui_MainWindow::write_protect_bubble(int drv, bool flag)
{
#ifdef USE_BUBBLE1
	if((drv < 0) || (drv >= MAX_BUBBLE)) return -1;
	emit sig_write_protect_bubble(drv, flag);
#endif
	return 0;
}
  
#ifdef USE_BUBBLE1
int Ui_MainWindow::set_b77_slot(int drive, int num)
{
	QString path;
	
	if((num < 0) || (num >= MAX_B77_BANKS)) return -1;
	path = QString::fromUtf8(emu->b77_file[drive].path);
	menu_bubbles[drive]->do_select_inner_media(num);

	if(emu && emu->b77_file[drive].cur_bank != num) {
		emit sig_open_bubble(drive, path, num);
		if(emu->is_bubble_casette_protected(drive)) {
			menu_bubbles[drive]->do_set_write_protect(true);
		} else {
			menu_bubbles[drive]->do_set_write_protect(false);
		}
	}
	return 0;
}

void Ui_MainWindow::do_update_recent_bubble(int drv)
{
	int i;
	if(emu == NULL) return;
	menu_bubbles[drv]->do_update_histories(listBubbles[drv]);
	menu_bubbles[drv]->do_set_initialize_directory(config.initial_bubble_casette_dir);
	if(emu->is_bubble_casette_protected(drv)) {
		menu_bubbles[drv]->do_write_protect_media();
	} else {
		menu_bubbles[drv]->do_write_unprotect_media();
	}		
}


int Ui_MainWindow::set_recent_bubble(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	int i;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	s_path = QString::fromLocal8Bit(config.recent_bubble_casette_path[drv][num]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_bubble_casette_path[drv], listBubbles[drv]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
   
	get_parent_dir(path_shadow);
	strcpy(config.initial_bubble_casette_dir, path_shadow);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);

	if(emu) {
		emit sig_close_bubble(drv);
		emit sig_open_bubble(drv, s_path, 0);
		menu_bubbles[drv]->do_update_histories(listBubbles[drv]);
		menu_bubbles[drv]->do_set_initialize_directory(config.initial_bubble_casette_dir);
		if(check_file_extension(path_shadow, ".b77")) {
			UPDATE_B77_LIST(drv, listB77[drv]);
			menu_bubbles[drv]->do_update_inner_media_bubble(listB77[drv], 0);
		} else {
			menu_bubbles[drv]->do_clear_inner_media();
		}
	}
	return 0;
}

#endif

void Ui_MainWindow::_open_bubble(int drv, const QString fname)
{
	char path_shadow[PATH_MAX];
	int i;
#ifdef USE_BUBBLE1
	if(fname.length() <= 0) return;
	drv = drv & 7;
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_bubble_casette_path[drv], listBubbles[drv]);
	get_parent_dir(path_shadow);
	strcpy(config.initial_bubble_casette_dir, path_shadow);
	// Update List
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX);
	if(emu) {
		emit sig_close_bubble(drv);
		//emu->LockVM();
		emit sig_open_bubble(drv, fname, 0);
		menu_bubbles[drv]->do_update_histories(listBubbles[drv]);
		menu_bubbles[drv]->do_set_initialize_directory(config.initial_bubble_casette_dir);
		if(check_file_extension(path_shadow, ".b77")) {
			UPDATE_B77_LIST(drv, listB77[drv]);
			menu_bubbles[drv]->do_update_inner_media_bubble(listB77[drv], 0);
		} else {
			menu_bubbles[drv]->do_clear_inner_media();
		}
	}
#endif
}

void Ui_MainWindow::eject_bubble(int drv) 
{
	int i;
#ifdef USE_BUBBLE1
	if(emu) {
		emit sig_close_bubble(drv);
		menu_bubbles[drv]->do_clear_inner_media();
	}
#endif
}

// Common Routine

void Ui_MainWindow::CreateBubbleMenu(int drv, int drv_base)
{
#if defined(USE_BUBBLE1)
	{
		QString ext = "*.b77 *.bbl";
		QString desc1 = "Bubble Casette";
		menu_bubbles[drv] = new Menu_BubbleClass(emu, menubar, QString::fromUtf8("Obj_Bubble"), this, drv);
		menu_bubbles[drv]->create_pulldown_menu();
		
		menu_bubbles[drv]->do_clear_inner_media();
		menu_bubbles[drv]->do_add_media_extension(ext, desc1);
		SETUP_HISTORY(config.recent_bubble_casette_path[drv], listBubbles[drv]);
		menu_bubbles[drv]->do_update_histories(listBubbles[drv]);
		menu_bubbles[drv]->do_set_initialize_directory(config.initial_bubble_casette_dir);
		listB77[drv].clear();

		QString name = QString::fromUtf8("BUBBLE");
		QString tmpv;
		tmpv.setNum(drv_base);
		name.append(tmpv);
		menu_bubbles[drv]->setTitle(name);
	}
#endif	
}

void Ui_MainWindow::CreateBubblePulldownMenu(int drv)
{
}

void Ui_MainWindow::ConfigBubbleMenuSub(int drv)
{
}

void Ui_MainWindow::retranslateBubbleMenu(int drv, int basedrv)
{
# ifdef USE_BUBBLE1
	QString drive_name = (QApplication::translate("MainWindow", "Bubble ", 0));
	drive_name += QString::number(basedrv);
  
	if((drv < 0) || (drv >= 8)) return;
	menu_bubbles[drv]->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0));
	menu_bubbles[drv]->retranslateUi();
# endif
}

void Ui_MainWindow::ConfigBubbleMenu(void)
{
#if defined(USE_BUBBLE1)
	ConfigBubbleMenuSub(0); 
#endif
#if defined(USE_BUBBLE2)
	ConfigBubbleMenuSub(1);
#endif
#if defined(USE_BUBBLE3)
	ConfigBubbleMenuSub(2);
#endif
#if defined(USE_BUBBLE4)
	ConfigBubbleMenuSub(3);
#endif
#if defined(USE_BUBBLE5)
	ConfigBubbleMenuSub(4);
#endif
#if defined(USE_BUBBLE6)
	ConfigBubbleMenuSub(5);
#endif
#if defined(USE_BUBBLE7)
	ConfigBubbleMenuSub(6);
#endif
#if defined(USE_BUBBLE8)
	ConfigBubbleMenuSub(7);
#endif
}
