/*
 * UI->Qt->MainWindow : Binary Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */


#include "mainwidget.h"
#include "commonclasses.h"
//#include "menuclasses.h"
#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"

QT_BEGIN_NAMESPACE

#if defined(USE_BINARY_FILE1)
int Ui_MainWindow::set_recent_binary_load(int drv, int num) 
{

	QString s_path;
	char path_shadow[PATH_MAX];
	int i;
	
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	
	s_path = QString::fromUtf8(config.recent_binary_path[drv][num]);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_binary_path[drv]);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	
	get_parent_dir(path_shadow);
	strcpy(config.initial_binary_dir, path_shadow);
	//strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	
	emit sig_load_binary(drv, s_path);
	for(i = 0; i < MAX_HISTORY; i++) {
		if(action_Recent_List_BIN[drv][i] != NULL) { 
			action_Recent_List_BIN[drv][i]->setText(QString::fromUtf8(config.recent_binary_path[drv][i]));
		}
	}
	return 0;
}

int Ui_MainWindow::set_recent_binary_save(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	int i;
	
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	
	s_path = QString::fromUtf8(config.recent_binary_path[drv][num]);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_binary_path[drv]);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	
	get_parent_dir(path_shadow);
	strcpy(config.initial_binary_dir, path_shadow);
	//strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	
	emit sig_save_binary(drv, s_path);
	for(i = 0; i < MAX_HISTORY; i++) {
		if(action_Recent_List_BIN[drv][i] != NULL) { 
			action_Recent_List_BIN[drv][i]->setText(QString::fromUtf8(config.recent_binary_path[drv][i]));
			//actiont_Recent_List_FD[drv][i]->changed();
		}
	}
	return 0;
}


void Ui_MainWindow::_open_binary(int drv, const QString fname, bool load)
{
	char path_shadow[PATH_MAX];
	int i;
	if(fname.length() <= 0) return;
	drv = drv & 7;
	strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_binary_path[drv]);
	get_parent_dir(path_shadow);
	strcpy(config.initial_binary_dir, path_shadow);
	// Update List
	//strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
	if(load) {
		emit sig_load_binary(drv, fname);
	} else {
		emit sig_save_binary(drv, fname);
	}
	for(i = 0; i < MAX_HISTORY; i++) {
		if(action_Recent_List_BIN[drv][i] != NULL) { 
			action_Recent_List_BIN[drv][i]->setText(QString::fromUtf8(config.recent_binary_path[drv][i]));
			//actiont_Recent_List_FD[drv][i]->changed();
		}
	}
}

#endif

QT_END_NAMESPACE
