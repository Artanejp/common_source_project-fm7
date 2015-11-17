/*
 * UI->Qt->MainWindow : FDD Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */


#include "mainwidget.h"
#include "commonclasses.h"
#include "menu_disk.h"

#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"

extern class EMU *emu;

void Object_Menu_Control::insert_fd(void) {
	emit sig_insert_fd(getDrive());
}
void Object_Menu_Control::eject_fd(void) {
	write_protect = false;
	emit sig_eject_fd(getDrive());
}
void Object_Menu_Control::on_d88_slot(void) {
	emit set_d88_slot(drive, s_num);
}
void Object_Menu_Control::on_recent_disk(void){
  //   write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit set_recent_disk(drive, s_num);
}
void Object_Menu_Control::write_protect_fd(void) {
	write_protect = true;
	emit sig_write_protect_fd(drive, write_protect);
}
void Object_Menu_Control::no_write_protect_fd(void) {
	write_protect = false;
	emit sig_write_protect_fd(drive, write_protect);
}

void Object_Menu_Control::do_set_ignore_crc_error(bool flag)
{
#ifdef USE_FD1
	config.ignore_disk_crc[drive] = flag;
	emit sig_emu_update_config();
#endif   
}

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

void Object_Menu_Control::do_set_correct_disk_timing(bool flag)
{
#ifdef USE_FD1
	config.correct_disk_timing[drive] = flag;
	emit sig_emu_update_config();
#endif   
}



int Ui_MainWindow::write_protect_fd(int drv, bool flag)
{
#ifdef USE_FD1
	if((drv < 0) || (drv >= MAX_FD)) return -1;
	emit sig_write_protect_disk(drv, flag);
#endif
	return 0;
}
  
#ifdef USE_FD1
int Ui_MainWindow::set_d88_slot(int drive, int num)
{
	QString path;
	
	if((num < 0) || (num >= MAX_D88_BANKS)) return -1;
	path = QString::fromUtf8(emu->d88_file[drive].path);
	menu_fds[drive]->do_select_inner_media(num);

	if(emu && emu->d88_file[drive].cur_bank != num) {
		emit sig_open_disk(drive, path, num);
		if(emu->get_disk_protected(drive)) {
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
	menu_fds[drv]->do_set_initialize_directory(config.initial_disk_dir);
	if(emu->get_disk_protected(drv)) {
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
	s_path = QString::fromUtf8(config.recent_disk_path[drv][num]);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_disk_path[drv], listFDs[drv]);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
   
	get_parent_dir(path_shadow);
	strcpy(config.initial_disk_dir, path_shadow);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);

	if(emu) {
		emit sig_close_disk(drv);
		emit sig_open_disk(drv, s_path, 0);
		menu_fds[drv]->do_update_histories(listFDs[drv]);
		menu_fds[drv]->do_set_initialize_directory(config.initial_disk_dir);
		if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
			UPDATE_D88_LIST(drv, listD88[drv]);
			menu_fds[drv]->do_update_inner_media(listD88[drv], 0);
		} else {
			menu_fds[drv]->do_clear_inner_media();
		}
# ifdef USE_FD2
		strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
		if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
			if(((drv & 1) == 0) && (drv + 1 < MAX_FD) && (1 < emu->d88_file[drv].bank_num)) {
				int drv2 = drv + 1;
				emit sig_close_disk(drv2);
				emit sig_open_disk(drv2, s_path, 1);
				menu_fds[drv2]->do_update_histories(listFDs[drv2]);
				menu_fds[drv2]->do_set_initialize_directory(config.initial_disk_dir);
				UPDATE_D88_LIST(drv2, listD88[drv2]);
				menu_fds[drv2]->do_update_inner_media(listD88[drv2], 1);
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
	UPDATE_HISTORY(path_shadow, config.recent_disk_path[drv], listFDs[drv]);
	get_parent_dir(path_shadow);
	strcpy(config.initial_disk_dir, path_shadow);
	// Update List
	strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
	if(emu) {
		emit sig_close_disk(drv);
		//emu->LockVM();
		emit sig_open_disk(drv, fname, 0);
		menu_fds[drv]->do_update_histories(listFDs[drv]);
		menu_fds[drv]->do_set_initialize_directory(config.initial_disk_dir);
		if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
			UPDATE_D88_LIST(drv, listD88[drv]);
			menu_fds[drv]->do_update_inner_media(listD88[drv], 0);
		} else {
			menu_fds[drv]->do_clear_inner_media();
		}
	}
# ifdef USE_FD2
	if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
		if(((drv & 1) == 0) && (drv + 1 < MAX_FD) && (1 < emu->d88_file[drv].bank_num)) {
			int drv2 = drv + 1;
			emit sig_close_disk(drv2);
			//emu->LockVM();
			strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
			emit sig_open_disk(drv2, fname, 1);
			menu_fds[drv2]->do_update_histories(listFDs[drv2]);
			menu_fds[drv2]->do_set_initialize_directory(config.initial_disk_dir);
			UPDATE_D88_LIST(drv2, listD88[drv2]);
			menu_fds[drv2]->do_update_inner_media(listD88[drv2], 1);
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
		emit sig_close_disk(drv);
		menu_fds[drv]->do_clear_inner_media();
	}
#endif
}

// Common Routine

void Ui_MainWindow::CreateFloppyMenu(int drv, int drv_base)
{
#if defined(USE_FD1)
	{
		QString ext = "*.d88 *.d77 *.1dd *.td0 *.imd *.dsk *.fdi *.hdm *.tfd *.xdf *.2d *.sf7 *.img *.ima *.vfd";
		QString desc1 = "Floppy Disk";
		menu_fds[drv] = new Menu_FDClass(emu, menubar, QString::fromUtf8("Obj_Floppy"), this, drv);
		menu_fds[drv]->create_pulldown_menu();
		
		menu_fds[drv]->do_clear_inner_media();
		menu_fds[drv]->do_add_media_extension(ext, desc1);
		SETUP_HISTORY(config.recent_disk_path[drv], listFDs[drv]);
		menu_fds[drv]->do_update_histories(listFDs[drv]);
		menu_fds[drv]->do_set_initialize_directory(config.initial_disk_dir);
		listD88[drv].clear();

		QString name = QString::fromUtf8("FD");
		QString tmpv;
		tmpv.setNum(drv_base);
		name.append(tmpv);
		menu_fds[drv]->setTitle(name);
	}
#endif	
}

void Ui_MainWindow::CreateFloppyPulldownMenu(int drv)
{
}

void Ui_MainWindow::ConfigFloppyMenuSub(int drv)
{
}

void Ui_MainWindow::retranslateFloppyMenu(int drv, int basedrv)
{
# ifdef USE_FD1
	QString drive_name = (QApplication::translate("MainWindow", "Floppy ", 0));
	drive_name += QString::number(basedrv);
  
	if((drv < 0) || (drv >= 8)) return;
	menu_fds[drv]->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0));
	menu_fds[drv]->retranslateUi();
# endif
}

void Ui_MainWindow::ConfigFloppyMenu(void)
{
#if defined(USE_FD1)
	ConfigFloppyMenuSub(0); 
#endif
#if defined(USE_FD2)
	ConfigFloppyMenuSub(1);
#endif
#if defined(USE_FD3)
	ConfigFloppyMenuSub(2);
#endif
#if defined(USE_FD4)
	ConfigFloppyMenuSub(3);
#endif
#if defined(USE_FD5)
	ConfigFloppyMenuSub(4);
#endif
#if defined(USE_FD6)
	ConfigFloppyMenuSub(5);
#endif
#if defined(USE_FD7)
	ConfigFloppyMenuSub(6);
#endif
#if defined(USE_FD8)
	ConfigFloppyMenuSub(7);
#endif
}
