/*
 * UI->Qt->MainWindow : CD ROM Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Mar 20, 2016 : Initial
 */

#include <QApplication>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "qt_dialogs.h"
#include "csp_logger.h"

#include "menu_compactdisc.h"


void Ui_MainWindowBase::CreateCDROMMenu(int drv, int drv_base)
{
	QString ext_play, desc_play;
	
	listCDROM[drv].clear();
	menu_CDROM[drv] = new Menu_CompactDiscClass(menubar, "CDROM", using_flags, this, drv, drv_base);
	menu_CDROM[drv]->setObjectName(QString::fromUtf8("menuCDROM", -1));
	
	menu_CDROM[drv]->create_pulldown_menu();	
	// Translate Menu
	SETUP_HISTORY(p_config->recent_compact_disc_path[drv], listCDROM[drv]);
	menu_CDROM[drv]->do_update_histories(listCDROM[drv]);
	menu_CDROM[drv]->do_set_initialize_directory(p_config->initial_compact_disc_dir);
	
	ext_play = "*.ccd *.cue *.gz";
	desc_play = "Compact Disc";
	menu_CDROM[drv]->do_add_media_extension(ext_play, desc_play);

}

void Ui_MainWindowBase::CreateCDROMPulldownMenu(void)
{
}

void Ui_MainWindowBase::ConfigCDROMMenuSub(void)
{
	
}

int Ui_MainWindowBase::set_recent_cdrom(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
    
	s_path = QString::fromLocal8Bit(p_config->recent_compact_disc_path[drv][num]);
	memset(path_shadow, 0x00, PATH_MAX * sizeof(char));
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_compact_disc_path[drv], listCDROM[drv]);
   
	strcpy(p_config->initial_compact_disc_dir, 	get_parent_dir(path_shadow));
	memset(path_shadow, 0x00, PATH_MAX * sizeof(char));
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX - 1);
	emit sig_close_cdrom(drv);
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_COMPACTDISC + 0, "Open : filename = %s", path_shadow);
	emit sig_open_cdrom(drv, s_path);
	menu_CDROM[drv]->do_update_histories(listCDROM[drv]);
	menu_CDROM[drv]->do_set_initialize_directory(p_config->initial_compact_disc_dir);
	return 0;
}

void Ui_MainWindowBase::do_eject_cdrom(int drv) 
{
	emit sig_close_cdrom(drv);
}

void Ui_MainWindowBase::do_open_cdrom(int drv, QString path) 
{
	char path_shadow[PATH_MAX];
	if(path.length() <= 0) return;
	memset(path_shadow, 0x00, PATH_MAX * sizeof(char));
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_compact_disc_path[drv], listCDROM[drv]);
	strcpy(p_config->initial_compact_disc_dir, get_parent_dir(path_shadow));
	// Copy filename again.
	memset(path_shadow, 0x00, PATH_MAX * sizeof(char));
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX - 1);

	emit sig_close_cdrom(drv);
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_COMPACTDISC + 0, "Open : filename = %s", path_shadow);
	emit sig_open_cdrom(drv, path);
	menu_CDROM[drv]->do_update_histories(listCDROM[drv]);
	menu_CDROM[drv]->do_set_initialize_directory(p_config->initial_compact_disc_dir);
}

void Ui_MainWindowBase::retranslateCDROMMenu(void)
{
	if(using_flags->is_use_compact_disc()) {
		for(int drv = 0; drv < using_flags->get_max_cd(); drv++) {
			menu_CDROM[drv]->retranslateUi();
		}
	}
}

void Ui_MainWindowBase::ConfigCDROMMenu(void)
{
	ConfigCDROMMenuSub(); 
}
