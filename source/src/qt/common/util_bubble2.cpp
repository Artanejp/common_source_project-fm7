#include "vm.h"
#include "mainwidget.h"
#include "commonclasses.h"
#include "menu_bubble.h"

#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"

#if defined(USE_BUBBLE1)
extern EMU *emu;

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

int Ui_MainWindow::set_b77_slot(int drive, int num)
{
	QString path;
	if((num < 0) || (num >= using_flags->get_max_b77_banks())) return -1;
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

void Ui_MainWindow::_open_bubble(int drv, const QString fname)
{
	char path_shadow[PATH_MAX];
	int i;

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
}

void Ui_MainWindow::eject_bubble(int drv) 
{
	int i;
	emit sig_close_bubble(drv);
	menu_bubbles[drv]->do_clear_inner_media();
}
#endif
