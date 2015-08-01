/*
 * UI->Qt->MainWindow : FDD Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */


#include "menuclasses.h"
#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"

int Ui_MainWindow::write_protect_fd(int drv, bool flag)
{
#ifdef USE_FD1
	if((drv < 0) || (drv >= MAX_FD)) return -1;
	if(emu) {
		emu->set_disk_protected(drv, flag);
	}
#endif
	return 0;
}
  
#ifdef USE_FD1
int Ui_MainWindow::set_d88_slot(int drive, int num)
{
	if((num < 0) || (num >= MAX_D88_BANKS)) return -1;
	if(emu && emu->d88_file[drive].cur_bank != num) {
		//    emu->open_disk(drive, emu->d88_file[drive].path, emu->d88_file[drive].bank[num].offset);
		emu->open_disk(drive, emu->d88_file[drive].path, num);
		if(emu->get_disk_protected(drive)) {
			actionProtection_ON_FD[drive]->setChecked(true);
		} else {
			actionProtection_OFF_FD[drive]->setChecked(true);
		}
		action_D88_ListImage_FD[drive][num]->setChecked(true);
		emu->d88_file[drive].cur_bank = num;
	}
	return 0;
}

int Ui_MainWindow::set_recent_disk(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	int i;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	s_path = QString::fromUtf8(config.recent_disk_path[drv][num]);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_disk_path[drv]);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
   
	get_parent_dir(path_shadow);
	strcpy(config.initial_disk_dir, path_shadow);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
   
	if(emu) {
		close_disk(drv);
		emu->LockVM();
		open_disk(drv, path_shadow, 0);
		if((actionGroup_D88_Image_FD[drv] != NULL) && (emu != NULL)){
			for(i = 0; i < emu->d88_file[drv].bank_num; i++) {
				if(action_D88_ListImage_FD[drv][i] != NULL) { 
					action_D88_ListImage_FD[drv][i]->setText(QString::fromUtf8(emu->d88_file[drv].disk_name[i]));
					action_D88_ListImage_FD[drv][i]->setVisible(true);
					if(i == 0) action_D88_ListImage_FD[drv][i]->setChecked(true);
					//emit action_D88_ListImage_FD[drv][i]->changed();
				}
			}
			for(; i < MAX_D88_BANKS; i++) {
				if(action_D88_ListImage_FD[drv][i] != NULL) { 
					action_D88_ListImage_FD[drv][i]->setVisible(false);
					//emit action_D88_ListImage_FD[drv][i]->changed();
				}
			}
			actionSelect_D88_Image_FD[drv][0].setChecked(true);
		}
		for(i = 0; i < MAX_HISTORY; i++) {
			if(action_Recent_List_FD[drv][i] != NULL) { 
				action_Recent_List_FD[drv][i]->setText(QString::fromUtf8(config.recent_disk_path[drv][i]));
				//actiont_Recent_List_FD[drv][i]->changed();
			}
		}
		emu->UnlockVM();
# if defined(USE_DISK_WRITE_PROTECT)
		emu->LockVM();
		if(emu->get_disk_protected(drv)) {
			actionProtection_ON_FD[drv]->setChecked(true);
		} else {
			actionProtection_OFF_FD[drv]->setChecked(true);
		}
		emu->UnlockVM();
# endif      
# ifdef USE_FD2
		strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
		if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
			if(((drv & 1) == 0) && (drv + 1 < MAX_FD) && (1 < emu->d88_file[drv].bank_num)) {
				int drv2 = drv + 1;
				close_disk(drv2);
				emu->LockVM();
				open_disk(drv2, path_shadow, 1);
				
				if((actionGroup_D88_Image_FD[drv2] != NULL) && (emu != NULL)){
					for(i = 0; i < emu->d88_file[drv2].bank_num; i++) {
						if(action_D88_ListImage_FD[drv2][i] != NULL) { 
							action_D88_ListImage_FD[drv2][i]->setText(QString::fromUtf8(emu->d88_file[drv2].disk_name[i]));
							if(i == 1) action_D88_ListImage_FD[drv2][i]->setChecked(true);
							action_D88_ListImage_FD[drv2][i]->setVisible(true);
						}
					}
					for(; i < MAX_D88_BANKS; i++) {
						if(action_D88_ListImage_FD[drv2][i] != NULL) { 
							action_D88_ListImage_FD[drv2][i]->setVisible(false);
						}
					}
					actionSelect_D88_Image_FD[drv2][1].setChecked(true);
				}
				emu->UnlockVM();
#  if defined(USE_DISK_WRITE_PROTECT)
				emu->LockVM();
				if(emu->get_disk_protected(drv2)) {
					actionProtection_ON_FD[drv2]->setChecked(true);
				} else {
					actionProtection_OFF_FD[drv2]->setChecked(true);
				}
#  endif
				emu->UnlockVM();
			}
		}
# endif
	}
	return 0;
}

#endif

void Ui_MainWindow::_open_disk(int drv, const QString fname)
{
	char path_shadow[PATH_MAX];
	int i;
#ifdef USE_FD1
	if(fname.length() <= 0) return;
	drv = drv & 7;
	strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_disk_path[drv]);
	get_parent_dir(path_shadow);
	strcpy(config.initial_disk_dir, path_shadow);
	// Update List
	strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
	if(emu) {
		close_disk(drv);
		emu->LockVM();
		open_disk(drv, path_shadow, 0);
		if((actionGroup_D88_Image_FD[drv] != NULL) && (emu != NULL)){
			for(i = 0; i < emu->d88_file[drv].bank_num; i++) {
				if(action_D88_ListImage_FD[drv][i] != NULL) { 
					action_D88_ListImage_FD[drv][i]->setText(QString::fromUtf8(emu->d88_file[drv].disk_name[i]));
					if(i == 0) action_D88_ListImage_FD[drv][i]->setChecked(true);
					action_D88_ListImage_FD[drv][i]->setVisible(true);
					//emit action_D88_ListImage_FD[drv][i]->changed();
				}
			}
			for(; i < MAX_D88_BANKS; i++) {
				if(action_D88_ListImage_FD[drv][i] != NULL) { 
					//actionSelect_D88_Image_FD[drv][i]->setText(emu->d88_file[drv].bank[i].name);
					action_D88_ListImage_FD[drv][i]->setVisible(false);
					//emit action_D88_ListImage_FD[drv][i]->changed();
				}
			}
			actionSelect_D88_Image_FD[drv][0].setChecked(true);
		}
		for(i = 0; i < MAX_HISTORY; i++) {
			if(action_Recent_List_FD[drv][i] != NULL) { 
				action_Recent_List_FD[drv][i]->setText(QString::fromUtf8(config.recent_disk_path[drv][i]));
				//actiont_Recent_List_FD[drv][i]->changed();
			}
		}
		emu->UnlockVM();
# if defined(USE_DISK_WRITE_PROTECT)
		emu->LockVM();
		if(emu->get_disk_protected(drv)) {
			actionProtection_ON_FD[drv]->setChecked(true);
		} else {
		  actionProtection_OFF_FD[drv]->setChecked(true);
		}
# endif
		emu->UnlockVM();
	}
# ifdef USE_FD2
	if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
		if(((drv & 1) == 0) && (drv + 1 < MAX_FD) && (1 < emu->d88_file[drv].bank_num)) {
			int drv2 = drv + 1;
			close_disk(drv2);
			emu->LockVM();
			strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
			open_disk(drv2, path_shadow, 1);
	      
			if((actionGroup_D88_Image_FD[drv2] != NULL) && (emu != NULL)){
				for(i = 0; i < emu->d88_file[drv2].bank_num; i++) {
					if(action_D88_ListImage_FD[drv2][i] != NULL) { 
						action_D88_ListImage_FD[drv2][i]->setText(QString::fromUtf8(emu->d88_file[drv2].disk_name[i]));
						if(i == 1) action_D88_ListImage_FD[drv2][i]->setChecked(true);
						action_D88_ListImage_FD[drv2][i]->setVisible(true);
					}
				}
				for(; i < MAX_D88_BANKS; i++) {
					if(action_D88_ListImage_FD[drv2][i] != NULL) { 
						action_D88_ListImage_FD[drv2][i]->setVisible(false);
					}
				}
				actionSelect_D88_Image_FD[drv2][1].setChecked(true);
			}
			emu->UnlockVM();
# if defined(USE_DISK_WRITE_PROTECT)
			emu->LockVM();
			if(emu->get_disk_protected(drv2)) {
				actionProtection_ON_FD[drv2]->setChecked(true);
			} else {
				actionProtection_OFF_FD[drv2]->setChecked(true);
			}
# endif
			emu->UnlockVM();
		}
	}
# endif
#endif
}

void Ui_MainWindow::eject_fd(int drv) 
{
	int i;
#ifdef USE_FD1
	if(emu) {
		//      emu->LockVM();
		close_disk(drv);
		for(i = 0; i < MAX_D88_BANKS; i++) {
			if(action_D88_ListImage_FD[drv][i] != NULL) { 
				action_D88_ListImage_FD[drv][i]->setVisible(false);
				//	       actionSelect_D88_Image_FD[drv][i].setChecked(false);
			}
		}
		//      emu->UnlockVM();
	}
#endif
}
