/*
 * UI->Qt->MainWindow : Binary Menu.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */

#include "menuclasses.h"
#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"


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

QT_BEGIN_NAMESPACE
#if defined(USE_BINARY_FILE1)

void Ui_MainWindow::open_binary_dialog(int drive, bool load)
{
	QString ext;
	QString desc1;
	QString desc2;
	CSP_DiskDialog dlg;
	QString dirname;
	
#if defined(_TK80BS) || defined(_TK80)
	ext = "*.ram *.bin *.tk8";
#else
	ext = "*.ram *.bin";
#endif	
#if defined(_PASOPIA) || defined(PASOPIA7)
	desc1 = "RAM Pack Cartridge";
#else
	desc1 = "Memory Dump";
#endif
	desc2.number(drive + 1);
	desc2 = QString::fromUtf8("Open binary image on #") + desc2;
	dlg.setWindowTitle(desc2);
	
	desc2 = desc1 + " (" + ext.toLower() + ")";
	desc1 = desc1 + " (" + ext.toUpper() + ")";
	
	if(config.initial_binary_dir != NULL) {
		dirname = QString::fromUtf8(config.initial_binary_dir);	        
	} else {
		char app[PATH_MAX];
		QDir df;
		dirname = df.currentPath();
		strncpy(app, dirname.toUtf8().constData(), PATH_MAX);
		dirname = get_parent_dir(app);
	}
	QStringList filter;
	filter << desc1 << desc2;
	dlg.param->setDrive(drive);
	dlg.param->setPlay(load);

	dlg.setDirectory(dirname);
	dlg.setNameFilters(filter); 
	QObject::connect(&dlg, SIGNAL(fileSelected(QString)), dlg.param, SLOT(_open_binary(QString))); 
	QObject::connect(dlg.param, SIGNAL(sig_open_binary_file(int, QString, bool)),
			 this, SLOT(_open_binary(int, QString, bool))); 
	dlg.show();
	dlg.exec();
	return;
}

void Ui_MainWindow::CreateBinaryMenu(int drv, int drv_base)
{
	QString drv_base_name = QString::number(drv_base); 
	menuBIN[drv] = new QMenu(menubar);
	menuBIN[drv]->setObjectName(QString::fromUtf8("menuBIN", -1) + drv_base_name);
}

void Ui_MainWindow::CreateBinaryPulldownMenu(int drv)
{
	menuBIN[drv]->addAction(actionLoad_BIN[drv]);
	menuBIN[drv]->addAction(actionSave_BIN[drv]);
	menuBIN[drv]->addSeparator();
	menuBIN_Recent[drv] = new QMenu(menuBIN[drv]);
	menuBIN_Recent[drv]->setObjectName(QString::fromUtf8("Recent_BIN", -1) + QString::number(drv));
	menuBIN[drv]->addAction(menuBIN_Recent[drv]->menuAction());
	{
		int ii;
		for(ii = 0; ii < MAX_HISTORY; ii++) {
			menuBIN_Recent[drv]->addAction(action_Recent_List_BIN[drv][ii]);
			action_Recent_List_BIN[drv][ii]->setVisible(true);
		}
	}
}

void Ui_MainWindow::ConfigBinaryMenuSub(int drv)
{
	QString drive_name = QString::number(drv);
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "Create: %d\n", drv);
  
	actionLoad_BIN[drv] = new Action_Control(this);
	actionLoad_BIN[drv]->setObjectName(QString::fromUtf8("actionLoad_BIN") + drive_name);
	actionLoad_BIN[drv]->binds->setDrive(drv);
	actionLoad_BIN[drv]->binds->setNumber(1);
  
	actionSave_BIN[drv] = new Action_Control(this);
	actionSave_BIN[drv]->setObjectName(QString::fromUtf8("actionSave_BIN") + drive_name);
	actionSave_BIN[drv]->binds->setDrive(drv);
	actionSave_BIN[drv]->binds->setNumber(0);
  
	actionGroup_Opened_BIN[drv] = new QActionGroup(this);
	actionRecent_Opened_BIN[drv] = new Action_Control(this);
	actionRecent_Opened_BIN[drv]->setObjectName(QString::fromUtf8("actionRecent_Opened_BIN") + drive_name);
	actionRecent_Opened_BIN[drv]->binds->setDrive(drv);
	actionRecent_Opened_BIN[drv]->binds->setNumber(0);
  
	{
		int ii;
		actionGroup_Opened_BIN[drv] = new QActionGroup(this);
		actionGroup_Opened_BIN[drv]->setExclusive(true);
    
		actionRecent_Opened_BIN[drv] = new Action_Control(this);
		actionRecent_Opened_BIN[drv]->setObjectName(QString::fromUtf8("actionSelect_Recent_BIN") + drive_name);
		actionRecent_Opened_BIN[drv]->binds->setDrive(drv);
		actionRecent_Opened_BIN[drv]->binds->setNumber(0);
		for(ii = 0; ii < MAX_HISTORY; ii++) {
			action_Recent_List_BIN[drv][ii] = new Action_Control(this);
			action_Recent_List_BIN[drv][ii]->binds->setDrive(drv);
			action_Recent_List_BIN[drv][ii]->binds->setNumber(ii);
			action_Recent_List_BIN[drv][ii]->setText(QString::fromUtf8(config.recent_binary_path[drv][ii]));
			actionGroup_Opened_BIN[drv]->addAction(action_Recent_List_BIN[drv][ii]);
			connect(action_Recent_List_BIN[drv][ii], SIGNAL(triggered()),
				action_Recent_List_BIN[drv][ii]->binds, SLOT(on_recent_binary_load()));
			connect(action_Recent_List_BIN[drv][ii]->binds, SIGNAL(set_recent_binary_load(int, int)),
				this, SLOT(set_recent_binary_load(int, int)));
		}
	}
  
	connect(actionLoad_BIN[drv], SIGNAL(triggered()), actionLoad_BIN[drv]->binds, SLOT(insert_binary_load()));
	connect(actionLoad_BIN[drv]->binds, SIGNAL(sig_open_binary(int, bool)), this, SLOT(open_binary_dialog(int, bool)));
	
	connect(actionSave_BIN[drv], SIGNAL(triggered()), actionSave_BIN[drv]->binds, SLOT(insert_binary_save()));
	connect(actionSave_BIN[drv]->binds, SIGNAL(sig_open_binary(int, bool)), this, SLOT(open_binary_dialog(int, bool)));
  // Translate Menu
}
#endif

void Ui_MainWindow::retranslateBinaryMenu(int drv, int basedrv)
{
#if defined(USE_BINARY_FILE1)
  QString drive_name = (QApplication::translate("MainWindow", "Binary", 0, QApplication::UnicodeUTF8));
  drive_name += QString::number(basedrv);
  
  if((drv < 0) || (drv >= 8)) return;
  actionLoad_BIN[drv]->setText(QApplication::translate("MainWindow", "Load", 0, QApplication::UnicodeUTF8));
  actionSave_BIN[drv]->setText(QApplication::translate("MainWindow", "Save", 0, QApplication::UnicodeUTF8));

  menuBIN_Recent[drv]->setTitle(QApplication::translate("MainWindow", "Recent Opened", 0, QApplication::UnicodeUTF8));

  menuBIN[drv]->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0, QApplication::UnicodeUTF8));
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

QT_END_NAMESPACE
