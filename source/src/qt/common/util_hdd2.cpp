#include "mainwidget.h"
#include "commonclasses.h"
#include "menu_harddisk.h"

#include "qt_dialogs.h"
#include "emu.h"
//#include "csp_logger.h"

#include "menu_flags.h"

//extern USING_FLAGS *using_flags;
extern class EMU *emu;

#if defined(USE_HARD_DISK)
void Ui_MainWindow::do_update_recent_hard_disk(int drv)
{
	if(emu == NULL) return;
	menu_hdds[drv]->do_update_histories(listHDDs[drv]);
	menu_hdds[drv]->do_set_initialize_directory(config.initial_hard_disk_dir);
}

int Ui_MainWindow::set_recent_hard_disk(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	s_path = QString::fromLocal8Bit(config.recent_hard_disk_path[drv][num]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX - 1);
	UPDATE_HISTORY(path_shadow, config.recent_hard_disk_path[drv], listHDDs[drv]);
   
	strncpy(config.initial_hard_disk_dir, 	get_parent_dir((const _TCHAR *)path_shadow), _MAX_PATH - 1);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX - 1);

	if(emu) {
		emit sig_close_hard_disk(drv);
		emit sig_open_hard_disk(drv, s_path);
		menu_hdds[drv]->do_update_histories(listHDDs[drv]);
		menu_hdds[drv]->do_set_initialize_directory(config.initial_hard_disk_dir);
		menu_hdds[drv]->do_clear_inner_media();
	}
	return 0;
}

void Ui_MainWindow::_open_hard_disk(int drv, const QString fname)
{
	char path_shadow[PATH_MAX];

	if(fname.length() <= 0) return;
	drv = drv & 7;
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX - 1);
	UPDATE_HISTORY(path_shadow, config.recent_hard_disk_path[drv], listHDDs[drv]);
	strncpy(config.initial_hard_disk_dir, 	get_parent_dir((const _TCHAR *)path_shadow), _MAX_PATH - 1);
	// Update List
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX - 1);
	if(emu) {
		emit sig_close_hard_disk(drv);
		//emu->LockVM();
		emit sig_open_hard_disk(drv, fname);
		menu_hdds[drv]->do_update_histories(listHDDs[drv]);
		menu_hdds[drv]->do_set_initialize_directory(config.initial_hard_disk_dir);
		menu_hdds[drv]->do_clear_inner_media();
	}
}

#endif
