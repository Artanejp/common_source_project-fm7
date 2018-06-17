/*
 * UI->Qt->MainWindow : Quick Disk Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */
#include <QApplication>

#include "mainwidget_base.h"
#include "commonclasses.h"
#include "qt_dialogs.h"
#include "csp_logger.h"
#include "menu_quickdisk.h"

void Object_Menu_Control::insert_Qd(void) {
	//write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit sig_insert_Qd(drive);
}
void Object_Menu_Control::eject_Qd(void) {
	write_protect = false;
	emit sig_eject_Qd(drive);
}
void Object_Menu_Control::on_recent_quick_disk(void){
	//   write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit set_recent_quick_disk(drive, s_num);
}

void CSP_FileParams::_open_quick_disk(QString s){
	//   write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit do_open_quick_disk(getDrive(), s);
}
void Object_Menu_Control::write_protect_Qd(void) {
	write_protect = true;
	emit sig_write_protect_Qd(drive, write_protect);
}
void Object_Menu_Control::no_write_protect_Qd(void) {
	write_protect = false;
	emit sig_write_protect_Qd(drive, write_protect);
}

void Ui_MainWindowBase::CreateQuickDiskPulldownMenu(int drv)
{
}

void Ui_MainWindowBase::ConfigQuickDiskMenuSub(int drv)
{
}


// Common Routine
void Ui_MainWindowBase::open_quick_disk_dialog(int drv)
{
	QString ext = "*.mzt *.q20 *.qdf";
	QString desc1 = "Quick DIsk";
	QString desc2;
	CSP_DiskDialog dlg;
	QString dirname;

	dlg.setWindowTitle("Open Quick Disk");
  
	desc2 = desc1 + " (" + ext.toLower() + " " + ext.toUpper() + ")";
	//desc2 = desc1 + " (" + ext.toLower() + ")";
	//desc1 = desc1 + " (" + ext.toUpper() + ")";
	if(p_config->initial_quick_disk_dir != NULL) {
		dirname = p_config->initial_quick_disk_dir;	        
	} else {
		char app[_MAX_PATH];
		QDir df;
		dirname = df.currentPath();
		memset(app, 0x00, _MAX_PATH * sizeof(char));
		strncpy(app, dirname.toLocal8Bit().constData(), _MAX_PATH - 1);
		dirname = get_parent_dir(app);
	}
	QStringList filter;
	filter << desc2;

	dlg.param->setDrive(drv);
	dlg.setDirectory(dirname);
	dlg.setNameFilters(filter);
	QObject::connect(&dlg, SIGNAL(fileSelected(QString)),
			 dlg.param, SLOT(_open_quick_disk(QString))); 
	QObject::connect(dlg.param, SIGNAL(do_open_quick_disk(int, QString)),
			 this, SLOT(_open_quick_disk(int, QString))); 
	dlg.show();
	dlg.exec();
	return;
}

int Ui_MainWindowBase::write_protect_Qd(int drv, bool flag)
{
	if((drv < 0) || (drv >= using_flags->get_max_qd())) return -1;
	emit sig_write_protect_quickdisk(drv, flag);
	return 0;
}
  
int Ui_MainWindowBase::set_recent_quick_disk(int drv, int num) 
{
	QString s_path;
	char path_shadow[_MAX_PATH];
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	s_path = QString::fromLocal8Bit(p_config->recent_quick_disk_path[drv][num]);
	memset(path_shadow, 0x00, _MAX_PATH * sizeof(char));
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_quick_disk_path[drv], listQDs[drv]);
    
	memset(path_shadow, 0x00, _MAX_PATH * sizeof(char));
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), _MAX_PATH - 1);
	strncpy(p_config->initial_quick_disk_dir, 	get_parent_dir(path_shadow), _MAX_PATH - 1);
	
	emit sig_close_quickdisk(drv);
	emit sig_open_quickdisk(drv, s_path);
	menu_QDs[drv]->do_update_histories(listQDs[drv]);
	menu_QDs[drv]->do_set_initialize_directory(p_config->initial_quick_disk_dir);
	//if(emu->get_quickdisk_protected(drv)) {
	//	menu_QDs[drv]->do_write_protect_media();
	//} else {
	//	menu_QDs[drv]->do_write_unprotect_media();
	//}		
	return 0;
}

void Ui_MainWindowBase::_open_quick_disk(int drv, const QString fname)
{
	char path_shadow[_MAX_PATH];
	QString s_name = fname;
	
	if(fname.length() <= 0) return;
	memset(path_shadow, 0x00, _MAX_PATH * sizeof(char));
	strncpy(path_shadow, s_name.toLocal8Bit().constData(), _MAX_PATH - 1);

	UPDATE_HISTORY(path_shadow, p_config->recent_quick_disk_path[drv], listQDs[drv]);
		
	memset(path_shadow, 0x00, _MAX_PATH * sizeof(char));
	strncpy(path_shadow, s_name.toLocal8Bit().constData(), _MAX_PATH - 1);
	strncpy(p_config->initial_quick_disk_dir, 	get_parent_dir(path_shadow), _MAX_PATH - 1);

	emit sig_close_quickdisk(drv);
	emit sig_open_quickdisk(drv, s_name);
	menu_QDs[drv]->do_update_histories(listQDs[drv]);
	menu_QDs[drv]->do_set_initialize_directory(p_config->initial_quick_disk_dir);
	//if(emu->get_quickdisk_protected(drv)) {
	//	menu_QDs[drv]->do_write_protect_media();
	//} else {
	//	menu_QDs[drv]->do_write_unprotect_media();
	//}		
}

void Ui_MainWindowBase::eject_Qd(int drv) 
{
	emit sig_close_quickdisk(drv);
}

void Ui_MainWindowBase::CreateQuickDiskMenu(int drv, int drv_base)
{
	{
		QString ext = "*.mzt *.q20 *.qdf";
		QString desc1 = "Quick DIsk";
		menu_QDs[drv] = new Menu_QDClass(menubar, QString::fromUtf8("QD"), using_flags, this, drv);
		menu_QDs[drv]->create_pulldown_menu();
		
		menu_QDs[drv]->do_clear_inner_media();
		menu_QDs[drv]->do_add_media_extension(ext, desc1);
		SETUP_HISTORY(p_config->recent_quick_disk_path[drv], listQDs[drv]);
		menu_QDs[drv]->do_update_histories(listQDs[drv]);
		menu_QDs[drv]->do_set_initialize_directory(p_config->initial_quick_disk_dir);

	}
}

void Ui_MainWindowBase::retranslateQuickDiskMenu(int drv, int basedrv)
{
	if((drv < 0) || (drv >= using_flags->get_max_qd())) return;
	QString drive_name = (QApplication::translate("MenuMedia", "Quick Disk ", 0));
	drive_name += QString::number(basedrv);
  
	menu_QDs[drv]->retranslateUi();
	menu_QDs[drv]->setTitle(QApplication::translate("MenuMedia", drive_name.toUtf8().constData() , 0));
}
								 
void Ui_MainWindowBase::ConfigQuickDiskMenu(void)
{
	for(int i = 0; i < using_flags->get_max_qd(); i++) {
		ConfigQuickDiskMenuSub(i);
	}
}
