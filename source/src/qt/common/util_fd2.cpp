#include "emu_thread_tmpl.h"
#include "mainwidget_base.h"
#include "commonclasses.h"
#include "menu_disk.h"
#include "qt_dialogs.h"

#include "menu_flags.h"

//#if defined(USE_FLOPPY_DISK)

#ifndef UPDATE_D88_LIST
#define UPDATE_D88_LIST(__d, lst) { \
	lst.clear(); \
	QString __tmps; \
	for(int iii = 0; iii < 64; iii++) { \
		__tmps = QString::fromUtf8(""); \
		if(iii < hRunEmu->get_d88_file_bank_num(__d)) {		\
	 		__tmps = hRunEmu->get_d88_file_disk_name(__d, iii); \
		} \
	lst << __tmps; \
	} \
}
#endif


//extern DLL_PREFIX_I EMU *emu;
int Ui_MainWindowBase::set_d88_slot(int drive, int num)
{
	QString path;
	if((num < 0) || (num >= 64)) return -1;
	//path = QString::fromUtf8(emu->d88_file[drive].path);
	path = hRunEmu->get_d88_file_path(drive);
	menu_fds[drive]->do_select_inner_media(num);

	if(hRunEmu->get_d88_file_cur_bank(drive) != num) {
		emit sig_close_disk(drive);
		emit sig_open_disk(drive, path, num);
		if(hRunEmu->is_floppy_disk_protected(drive)) {
			menu_fds[drive]->do_set_write_protect(true);
		} else {
			menu_fds[drive]->do_set_write_protect(false);
		}
	}
	return 0;
}

void Ui_MainWindowBase::do_update_recent_disk(int drv)
{
	if(hRunEmu == NULL) return;
	menu_fds[drv]->do_update_histories(listFDs[drv]);
	menu_fds[drv]->do_set_initialize_directory(p_config->initial_floppy_disk_dir);
	if(hRunEmu != NULL) {
//		if(hRunEmu->get_d88_file_cur_bank(drv) != num) {
			if(hRunEmu->is_floppy_disk_protected(drv)) {
				menu_fds[drv]->do_set_write_protect(true);
			} else {
				menu_fds[drv]->do_set_write_protect(false);
			}
//		}
	}
}


extern const _TCHAR* DLL_PREFIX_I get_parent_dir(const _TCHAR* file);

int Ui_MainWindowBase::set_recent_disk(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	s_path = QString::fromLocal8Bit(p_config->recent_floppy_disk_path[drv][num]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_floppy_disk_path[drv], listFDs[drv]);
   
	strncpy(p_config->initial_floppy_disk_dir, 	get_parent_dir((const _TCHAR *)path_shadow), _MAX_PATH - 1);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX - 1);

//	if(emu) {
		emit sig_close_disk(drv);
		emit sig_open_disk(drv, s_path, 0);
		menu_fds[drv]->do_update_histories(listFDs[drv]);
		menu_fds[drv]->do_set_initialize_directory(p_config->initial_floppy_disk_dir);
		if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
			UPDATE_D88_LIST(drv, listD88[drv]);
			menu_fds[drv]->do_update_inner_media(listD88[drv], 0);
		} else {
			menu_fds[drv]->do_clear_inner_media();
		}
		if(using_flags->get_max_drive() >= 2) {
			strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX - 1);
			if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
				if(((drv & 1) == 0) && ((drv + 1) < using_flags->get_max_drive()) && (1 < hRunEmu->get_d88_file_bank_num(drv))) {

					int drv2 = drv + 1;
					emit sig_close_disk(drv2);
					emit sig_open_disk(drv2, s_path, 1);
					menu_fds[drv2]->do_update_histories(listFDs[drv2]);
					menu_fds[drv2]->do_set_initialize_directory(p_config->initial_floppy_disk_dir);
					UPDATE_D88_LIST(drv2, listD88[drv2]);
					menu_fds[drv2]->do_update_inner_media(listD88[drv2], 1);
				}
			}
		}
//	}
	return 0;
}

void Ui_MainWindowBase::_open_disk(int drv, const QString fname)
{
	char path_shadow[PATH_MAX];

	if(fname.length() <= 0) return;
	drv = drv & 7;
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_floppy_disk_path[drv], listFDs[drv]);
	strcpy(p_config->initial_floppy_disk_dir, 	get_parent_dir((const _TCHAR *)path_shadow));
	// Update List
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX - 1);
//	if(emu) {
		emit sig_close_disk(drv);
		emit sig_open_disk(drv, fname, 0);
		menu_fds[drv]->do_update_histories(listFDs[drv]);
		menu_fds[drv]->do_set_initialize_directory(p_config->initial_floppy_disk_dir);
		if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
			UPDATE_D88_LIST(drv, listD88[drv]);
			menu_fds[drv]->do_update_inner_media(listD88[drv], 0);
		} else {
			menu_fds[drv]->do_clear_inner_media();
		}
//	}
	if(using_flags->get_max_drive() >= 2) {
		if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
			if(((drv & 1) == 0) && (drv + 1 < using_flags->get_max_drive()) && (1 < hRunEmu->get_d88_file_bank_num(drv))) {
				int drv2 = drv + 1;
				emit sig_close_disk(drv2);
				strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX - 1);
				emit sig_open_disk(drv2, fname, 1);
				menu_fds[drv2]->do_update_histories(listFDs[drv2]);
				menu_fds[drv2]->do_set_initialize_directory(p_config->initial_floppy_disk_dir);
				UPDATE_D88_LIST(drv2, listD88[drv2]);
				menu_fds[drv2]->do_update_inner_media(listD88[drv2], 1);
			}
		}
	}
}

void Ui_MainWindowBase::do_update_d88_list(int drv, int bank)
{
	UPDATE_D88_LIST(drv, listD88[drv]);
	menu_fds[drv]->do_update_inner_media(listD88[drv], bank);
}
 
//#endif
