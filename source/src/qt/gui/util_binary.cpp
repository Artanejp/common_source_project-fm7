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
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"

#include "menu_binary.h"

#ifdef USE_BINARY_FILE1
void Object_Menu_Control::on_recent_binary_load(void){
	//   write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit set_recent_binary_load(drive, s_num);
}
void Object_Menu_Control::on_recent_binary_save(void){
	//   write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit set_recent_binary_save(drive, s_num);
}

void Object_Menu_Control::_open_binary(QString s){
	bool load = this->isPlay();
	int d = this->getDrive();
	emit sig_open_binary_file(d, s, load);
}
void Object_Menu_Control::insert_binary_load(void) {
	emit sig_open_binary(getDrive(), true);
}
void Object_Menu_Control::insert_binary_save(void) {
	emit sig_open_binary(getDrive(), false);
}
#endif

#if defined(USE_BINARY_FILE1)
int Ui_MainWindow::set_recent_binary_load(int drv, int num) 
{

	QString s_path;
	char path_shadow[PATH_MAX];
	int i;
	
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	
	s_path = QString::fromLocal8Bit(config.recent_binary_path[drv][num]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_binary_path[drv], listBINs[drv]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	
	get_parent_dir(path_shadow);
	strcpy(config.initial_binary_dir, path_shadow);
	//strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	
	emit sig_load_binary(drv, s_path);
	menu_BINs[drv]->do_update_histories(listBINs[drv]);
	menu_BINs[drv]->do_set_initialize_directory(config.initial_binary_dir);
	return 0;
}

int Ui_MainWindow::set_recent_binary_save(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	int i;
	
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	
	s_path = QString::fromLocal8Bit(config.recent_binary_path[drv][num]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_binary_path[drv], listBINs[drv]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	
	get_parent_dir(path_shadow);
	strcpy(config.initial_binary_dir, path_shadow);
	//strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	
	emit sig_save_binary(drv, s_path);
	menu_BINs[drv]->do_update_histories(listBINs[drv]);
	menu_BINs[drv]->do_set_initialize_directory(config.initial_binary_dir);
	return 0;
}


void Ui_MainWindow::_open_binary_load(int drv, const QString fname)
{
	char path_shadow[PATH_MAX];
	int i;
	if(fname.length() <= 0) return;
	drv = drv & 7;
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_binary_path[drv], listBINs[drv]);
	get_parent_dir(path_shadow);
	strcpy(config.initial_binary_dir, path_shadow);
	// Update List
	emit sig_load_binary(drv, fname);
		
	menu_BINs[drv]->do_update_histories(listBINs[drv]);
	menu_BINs[drv]->do_set_initialize_directory(config.initial_binary_dir);
}

void Ui_MainWindow::_open_binary_save(int drv, const QString fname)
{
	char path_shadow[PATH_MAX];
	int i;
	if(fname.length() <= 0) return;
	drv = drv & 7;
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_binary_path[drv], listBINs[drv]);
	get_parent_dir(path_shadow);
	strcpy(config.initial_binary_dir, path_shadow);
	// Update List
	emit sig_save_binary(drv, fname);

	menu_BINs[drv]->do_update_histories(listBINs[drv]);
	menu_BINs[drv]->do_set_initialize_directory(config.initial_binary_dir);
}

#endif
#if defined(USE_BINARY_FILE1)

void Ui_MainWindow::CreateBinaryMenu(int drv, int drv_base)
{
	QString drv_base_name = QString::number(drv_base);
	QString ext, desc1;
# if defined(_TK80BS) || defined(_TK80)
	ext = "*.ram *.bin *.tk8";
# else
	ext = "*.ram *.bin";
# endif	
# if defined(_PASOPIA) || defined(PASOPIA7)
	desc1 = "RAM Pack Cartridge";
# else
	desc1 = "Memory Dump";
# endif
	menu_BINs[drv] = new Menu_BinaryClass(emu, menubar, QString::fromUtf8("Obj_Binary"), this, drv);
	menu_BINs[drv]->create_pulldown_menu();
	
	menu_BINs[drv]->do_clear_inner_media();
	menu_BINs[drv]->do_add_media_extension(ext, desc1);
	SETUP_HISTORY(config.recent_binary_path[drv], listBINs[drv]);
	menu_BINs[drv]->do_update_histories(listBINs[drv]);
	menu_BINs[drv]->do_set_initialize_directory(config.initial_binary_dir);
	listBINs[drv].clear();

	QString name = QString::fromUtf8("Binary");
	QString tmpv;
	tmpv.setNum(drv_base);
	name.append(tmpv);
	menu_BINs[drv]->setTitle(name);

}

void Ui_MainWindow::CreateBinaryPulldownMenu(int drv)
{
}

void Ui_MainWindow::ConfigBinaryMenuSub(int drv)
{
  // Translate Menu
}
#endif

void Ui_MainWindow::retranslateBinaryMenu(int drv, int basedrv)
{
#if defined(USE_BINARY_FILE1)
  QString drive_name = (QApplication::translate("MainWindow", "Binary", 0));
  drive_name += QString::number(basedrv);
  
  if((drv < 0) || (drv >= 8)) return;
  menu_BINs[drv]->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0));
  menu_BINs[drv]->retranslateUi();
#endif
}

void Ui_MainWindow::ConfigBinaryMenu(void)
{
  
#if defined(USE_BINARY_FILE1)
	ConfigBinaryMenuSub(0); 
#endif
#if defined(USE_BINARY_FILE2)
	ConfigBinaryMenuSub(1);
#endif
}
