#include "mainwidget.h"
#include "commonclasses.h"
#include "menu_disk.h"

#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"

#include "menu_flags.h"

//extern USING_FLAGS *using_flags;
extern class EMU *emu;

#if defined(USE_FD1)	

#ifndef UPDATE_D88_LIST
#define UPDATE_D88_LIST(__d, lst) { \
	lst.clear(); \
	QString __tmps; \
	for(int iii = 0; iii < MAX_D88_BANKS; iii++) { \
		__tmps = QString::fromUtf8(""); \
		if(iii < emu->d88_file[__d].bank_num) { \
	 		__tmps = QString::fromUtf8(emu->d88_file[__d].disk_name[iii]); \
		} \
	lst << __tmps; \
	} \
}
#endif

int Ui_MainWindow::set_d88_slot(int drive, int num)
{
	QString path;
	if((num < 0) || (num >= MAX_D88_BANKS)) return -1;
	path = QString::fromUtf8(emu->d88_file[drive].path);
	menu_fds[drive]->do_select_inner_media(num);

	if(emu && emu->d88_file[drive].cur_bank != num) {
		emit sig_open_disk(drive, path, num);
		if(emu->is_floppy_disk_protected(drive)) {
			menu_fds[drive]->do_set_write_protect(true);
		} else {
			menu_fds[drive]->do_set_write_protect(false);
		}
	}
	return 0;
}

void Ui_MainWindow::do_update_recent_disk(int drv)
{
	int i;
	if(emu == NULL) return;
	menu_fds[drv]->do_update_histories(listFDs[drv]);
	menu_fds[drv]->do_set_initialize_directory(config.initial_floppy_disk_dir);
	if(emu->is_floppy_disk_protected(drv)) {
		menu_fds[drv]->do_write_protect_media();
	} else {
		menu_fds[drv]->do_write_unprotect_media();
	}
}


int Ui_MainWindow::set_recent_disk(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	int i;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	s_path = QString::fromLocal8Bit(config.recent_floppy_disk_path[drv][num]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_floppy_disk_path[drv], listFDs[drv]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
   
	get_parent_dir(path_shadow);
	strcpy(config.initial_floppy_disk_dir, path_shadow);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);

	if(emu) {
		emit sig_close_disk(drv);
		emit sig_open_disk(drv, s_path, 0);
		menu_fds[drv]->do_update_histories(listFDs[drv]);
		menu_fds[drv]->do_set_initialize_directory(config.initial_floppy_disk_dir);
		if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
			UPDATE_D88_LIST(drv, listD88[drv]);
			menu_fds[drv]->do_update_inner_media(listD88[drv], 0);
		} else {
			menu_fds[drv]->do_clear_inner_media();
		}
		if(using_flags->get_max_drive() >= 2) {
			strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
			if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
				if(((drv & 1) == 0) && (drv + 1 < using_flags->get_max_drive()) && (1 < emu->d88_file[drv].bank_num)) {
					int drv2 = drv + 1;
					emit sig_close_disk(drv2);
					emit sig_open_disk(drv2, s_path, 1);
					menu_fds[drv2]->do_update_histories(listFDs[drv2]);
					menu_fds[drv2]->do_set_initialize_directory(config.initial_floppy_disk_dir);
					UPDATE_D88_LIST(drv2, listD88[drv2]);
					menu_fds[drv2]->do_update_inner_media(listD88[drv2], 1);
				}
			}
		}
	}
	return 0;
}

void Ui_MainWindow::_open_disk(int drv, const QString fname)
{
	char path_shadow[PATH_MAX];
	int i;

	if(fname.length() <= 0) return;
	drv = drv & 7;
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_floppy_disk_path[drv], listFDs[drv]);
	get_parent_dir(path_shadow);
	strcpy(config.initial_floppy_disk_dir, path_shadow);
	// Update List
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX);
	if(emu) {
		emit sig_close_disk(drv);
		//emu->LockVM();
		emit sig_open_disk(drv, fname, 0);
		menu_fds[drv]->do_update_histories(listFDs[drv]);
		menu_fds[drv]->do_set_initialize_directory(config.initial_floppy_disk_dir);
		if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
			UPDATE_D88_LIST(drv, listD88[drv]);
			menu_fds[drv]->do_update_inner_media(listD88[drv], 0);
		} else {
			menu_fds[drv]->do_clear_inner_media();
		}
	}
	if(using_flags->get_max_drive() >= 2) {
		if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
			if(((drv & 1) == 0) && (drv + 1 < using_flags->get_max_drive()) && (1 < emu->d88_file[drv].bank_num)) {
				int drv2 = drv + 1;
				emit sig_close_disk(drv2);
				//emu->LockVM();
				strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX);
				emit sig_open_disk(drv2, fname, 1);
				menu_fds[drv2]->do_update_histories(listFDs[drv2]);
				menu_fds[drv2]->do_set_initialize_directory(config.initial_floppy_disk_dir);
				UPDATE_D88_LIST(drv2, listD88[drv2]);
				menu_fds[drv2]->do_update_inner_media(listD88[drv2], 1);
			}
		}
	}
}

#endif	
