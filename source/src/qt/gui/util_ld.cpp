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
#include "csp_logger.h"

#include "menu_laserdisc.h"


void Object_Menu_Control::insert_laserdisc(void) {
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "%d", play);
	emit sig_insert_laserdisc(play);
}
void Object_Menu_Control::eject_laserdisc(void) {
	emit sig_eject_laserdisc();
}
void Object_Menu_Control::on_recent_laserdisc(){
	emit sig_recent_laserdisc(s_num);
}

void Ui_MainWindowBase::CreateLaserdiscMenu(int drv, int drv_base)
{
	QString ext_play, desc_play;
	
	listLaserdisc[drv].clear();
	menu_Laserdisc[drv] = new Menu_LaserdiscClass(menubar, "Laserdisc", using_flags, this, drv, drv_base);
	menu_Laserdisc[drv]->setObjectName(QString::fromUtf8("menuLaserdisc", -1));
	
	menu_Laserdisc[drv]->create_pulldown_menu();	
	// Translate Menu
	SETUP_HISTORY(p_config->recent_laser_disc_path[drv], listLaserdisc[drv]);
	menu_Laserdisc[drv]->do_update_histories(listLaserdisc[drv]);
	menu_Laserdisc[drv]->do_set_initialize_directory(p_config->initial_laser_disc_dir);
	
	ext_play = "*.ogv *.mp4 *.avi *.mkv";
	desc_play = "Laserisc";
	menu_Laserdisc[drv]->do_add_media_extension(ext_play, desc_play);

}

void Ui_MainWindowBase::CreateLaserdiscPulldownMenu(void)
{
}

void Ui_MainWindowBase::ConfigLaserdiscMenuSub(void)
{
	
}

int Ui_MainWindowBase::set_recent_laserdisc(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
    
	s_path = QString::fromLocal8Bit(p_config->recent_laser_disc_path[drv][num]);
	memset(path_shadow, 0x00, PATH_MAX * sizeof(char));
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_laser_disc_path[drv], listLaserdisc[drv]);
   
	strcpy(p_config->initial_laser_disc_dir, get_parent_dir(path_shadow));
	memset(path_shadow, 0x00, PATH_MAX * sizeof(char));
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX - 1);
	emit sig_close_laserdisc(drv);
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_LASERDISC + 0, "Open : filename = %s", path_shadow);
	emit sig_open_laserdisc(drv, s_path);
	menu_Laserdisc[drv]->do_update_histories(listLaserdisc[drv]);
	menu_Laserdisc[drv]->do_set_initialize_directory(p_config->initial_laser_disc_dir);
	return 0;
}

void Ui_MainWindowBase::do_eject_laserdisc(int drv) 
{
	emit sig_close_laserdisc(drv);
}

void Ui_MainWindowBase::do_open_laserdisc(int drv, QString path) 
{
	char path_shadow[PATH_MAX];

	if(path.length() <= 0) return;
	memset(path_shadow, 0x00, PATH_MAX * sizeof(char));
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_laser_disc_path[drv], listLaserdisc[drv]);
	strcpy(p_config->initial_laser_disc_dir, get_parent_dir(path_shadow));
	// Copy filename again.
	memset(path_shadow, 0x00, PATH_MAX * sizeof(char));
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX - 1);

	emit sig_close_laserdisc(drv);
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_LASERDISC + 0, "Open : filename = %s", path_shadow);
	emit sig_open_laserdisc(drv, path);
	menu_Laserdisc[drv]->do_update_histories(listLaserdisc[drv]);
	menu_Laserdisc[drv]->do_set_initialize_directory(p_config->initial_laser_disc_dir);
}

void Ui_MainWindowBase::retranslateLaserdiscMenu(void)
{
	if(using_flags->is_use_laser_disc()) {
		for(int drv = 0; drv < using_flags->get_max_ld(); drv++) {
			menu_Laserdisc[drv]->retranslateUi();
		}
	}
}

void Ui_MainWindowBase::ConfigLaserdiscMenu(void)
{
	ConfigLaserdiscMenuSub(); 
}
