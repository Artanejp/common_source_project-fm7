/*
 * UI->Qt->MainWindow : Quick Disk Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */


#include "mainwidget.h"
#include "commonclasses.h"
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"
#include "menu_quickdisk.h"

#if defined(USE_QD1) || defined(USE_QD2)
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
#endif


void Ui_MainWindow::CreateQuickDiskPulldownMenu(int drv)
{
}

void Ui_MainWindow::ConfigQuickDiskMenuSub(int drv)
{
}


// Common Routine
void Ui_MainWindow::open_quick_disk_dialog(int drv)
{
#ifdef USE_QD1
	QString ext = "*.mzt *.q20 *.qdf";
	QString desc1 = "Quick DIsk";
	QString desc2;
	CSP_DiskDialog dlg;
	QString dirname;

	dlg.setWindowTitle("Open Quick Disk");
  
	desc2 = desc1 + " (" + ext.toLower() + " " + ext.toUpper() + ")";
	//desc2 = desc1 + " (" + ext.toLower() + ")";
	//desc1 = desc1 + " (" + ext.toUpper() + ")";
	if(config.initial_quick_disk_dir != NULL) {
		dirname = config.initial_quick_disk_dir;	        
	} else {
		char app[_MAX_PATH];
		QDir df;
		dirname = df.currentPath();
		strncpy(app, dirname.toUtf8().constData(), _MAX_PATH);
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
#endif
}

int Ui_MainWindow::write_protect_Qd(int drv, bool flag)
{
#ifdef USE_QD1
	if((drv < 0) || (drv >= MAX_QD)) return -1;
	emit sig_write_protect_quickdisk(drv, flag);
#endif
	return 0;
}
  
int Ui_MainWindow::set_recent_quick_disk(int drv, int num) 
{
#ifdef USE_QD1
	QString s_path;
	char path_shadow[_MAX_PATH];
	int i;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	s_path = QString::fromUtf8(config.recent_quick_disk_path[drv][num]);
	strncpy(path_shadow, s_path.toUtf8().constData(), _MAX_PATH);
	UPDATE_HISTORY(path_shadow, config.recent_quick_disk_path[drv], listQDs[drv]);
    
	strncpy(path_shadow, s_path.toUtf8().constData(), _MAX_PATH);
	get_parent_dir(path_shadow);
	strncpy(config.initial_quick_disk_dir, path_shadow, _MAX_PATH);
	
	emit sig_close_quickdisk(drv);
	emit sig_open_quickdisk(drv, s_path);
	menu_QDs[drv]->do_update_histories(listQDs[drv]);
	menu_QDs[drv]->do_set_initialize_directory(config.initial_quick_disk_dir);
	//if(emu->get_quickdisk_protected(drv)) {
	//	menu_QDs[drv]->do_write_protect_media();
	//} else {
	//	menu_QDs[drv]->do_write_unprotect_media();
	//}		
#endif
	return 0;
}

void Ui_MainWindow::_open_quick_disk(int drv, const QString fname)
{
	char path_shadow[_MAX_PATH];
	QString s_name = fname;
	int i;
#ifdef USE_QD1
	if(fname.length() <= 0) return;
	strncpy(path_shadow, s_name.toUtf8().constData(), _MAX_PATH);

	UPDATE_HISTORY(path_shadow, config.recent_quick_disk_path[drv], listQDs[drv]);
	
	strncpy(path_shadow, s_name.toUtf8().constData(), _MAX_PATH);
	get_parent_dir(path_shadow);
	strncpy(config.initial_quick_disk_dir, path_shadow, _MAX_PATH);

	emit sig_close_quickdisk(drv);
	emit sig_open_quickdisk(drv, s_name);
	menu_QDs[drv]->do_update_histories(listQDs[drv]);
	menu_QDs[drv]->do_set_initialize_directory(config.initial_quick_disk_dir);
	//if(emu->get_quickdisk_protected(drv)) {
	//	menu_QDs[drv]->do_write_protect_media();
	//} else {
	//	menu_QDs[drv]->do_write_unprotect_media();
	//}		
#endif
}

void Ui_MainWindow::eject_Qd(int drv) 
{
#ifdef USE_QD1
	emit sig_close_quickdisk(drv);
#endif
}

void Ui_MainWindow::CreateQuickDiskMenu(int drv, int drv_base)
{
#ifdef USE_QD1
	{
		QString ext = "*.mzt *.q20 *.qdf";
		QString desc1 = "Quick DIsk";
		menu_QDs[drv] = new Menu_QDClass(emu, menubar, QString::fromUtf8("Obj_QuickDisk"), this, drv);
		menu_QDs[drv]->create_pulldown_menu();
		
		menu_QDs[drv]->do_clear_inner_media();
		menu_QDs[drv]->do_add_media_extension(ext, desc1);
		SETUP_HISTORY(config.recent_quick_disk_path[drv], listQDs[drv]);
		menu_QDs[drv]->do_update_histories(listQDs[drv]);
		menu_QDs[drv]->do_set_initialize_directory(config.initial_quick_disk_dir);

		QString name = QString::fromUtf8("Quick Disk");
		QString tmpv;
		tmpv.setNum(drv_base);
		name.append(tmpv);
		menu_QDs[drv]->setTitle(name);
	}
#endif
}

void Ui_MainWindow::retranslateQuickDiskMenu(int drv, int basedrv)
{
#ifdef USE_QD1
	if((drv < 0) || (drv >= 2)) return;
	QString drive_name = (QApplication::translate("MainWindow", "Quick Disk ", 0));
	drive_name += QString::number(basedrv);
  
	menu_QDs[drv]->retranslateUi();
	menu_QDs[drv]->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0));
#endif
}
								 
void Ui_MainWindow::ConfigQuickDiskMenu(void)
{
#if defined(USE_QD1)
	ConfigQuickDiskMenuSub(0); 
#endif
#if defined(USE_QD2)
	ConfigQuickDiskMenuSub(1); 
#endif
}
