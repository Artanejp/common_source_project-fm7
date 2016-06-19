/*
 * UI->Qt->MainWindow : CD ROM Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Mar 20, 2016 : Initial
 */


#include "commonclasses.h"
#include "mainwidget_base.h"
#include "qt_dialogs.h"
#include "agar_logger.h"

#include "menu_compactdisc.h"


void Object_Menu_Control::insert_cdrom(void) {
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "%d", play);
	emit sig_insert_cdrom(play);
}
void Object_Menu_Control::eject_cdrom(void) {
	emit sig_eject_cdrom();
}
void Object_Menu_Control::on_recent_cdrom(){
	emit sig_recent_cdrom(s_num);
}

void Ui_MainWindowBase::CreateCDROMMenu(void)
{
	QString ext_play, desc_play;
	
	listCDROM.clear();
	menu_CDROM = new Menu_CompactDiscClass(emu, menubar, "Object_CDROM_Menu", using_flags, this, 0);
	menu_CDROM->setObjectName(QString::fromUtf8("menuCDROM", -1));
	
	menu_CDROM->create_pulldown_menu();	
	// Translate Menu
	SETUP_HISTORY(using_flags->get_config_ptr()->recent_compact_disc_path, listCDROM);
	menu_CDROM->do_update_histories(listCDROM);
	menu_CDROM->do_set_initialize_directory(using_flags->get_config_ptr()->initial_compact_disc_dir);
	
	ext_play = "*.ccd *.cue";
	desc_play = "Compact Disc";
	menu_CDROM->do_add_media_extension(ext_play, desc_play);

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
	int i;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
    
	s_path = QString::fromLocal8Bit(using_flags->get_config_ptr()->recent_compact_disc_path[num]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, using_flags->get_config_ptr()->recent_compact_disc_path, listCDROM);
   
	get_parent_dir(path_shadow);
	strcpy(using_flags->get_config_ptr()->initial_compact_disc_dir, path_shadow);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	emit sig_close_cdrom();
	AGAR_DebugLog(AGAR_LOG_DEBUG, "CD-ROM: Open : filename = %s", path_shadow);
	emit sig_open_cdrom(s_path);
	menu_CDROM->do_update_histories(listCDROM);
	menu_CDROM->do_set_initialize_directory(using_flags->get_config_ptr()->initial_compact_disc_dir);
	return 0;
}

void Ui_MainWindowBase::do_eject_cdrom(int drv) 
{
	emit sig_close_cdrom();
}

void Ui_MainWindowBase::do_open_cdrom(int drv, QString path) 
{
	char path_shadow[PATH_MAX];
	int i;

	if(path.length() <= 0) return;
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, using_flags->get_config_ptr()->recent_compact_disc_path, listCDROM);
	get_parent_dir(path_shadow);
	strcpy(using_flags->get_config_ptr()->initial_compact_disc_dir, path_shadow);
	// Copy filename again.
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX);

	emit sig_close_cdrom();
	AGAR_DebugLog(AGAR_LOG_DEBUG, "CD-ROM: Open : filename = %s", path_shadow);
	emit sig_open_cdrom(path);
	menu_CDROM->do_update_histories(listCDROM);
	menu_CDROM->do_set_initialize_directory(using_flags->get_config_ptr()->initial_compact_disc_dir);
}

void Ui_MainWindowBase::retranslateCDROMMenu(void)
{
	if(using_flags->is_use_compact_disc()) {
		menu_CDROM->retranslateUi();
	}
}

void Ui_MainWindowBase::ConfigCDROMMenu(void)
{
	ConfigCDROMMenuSub(); 
}
