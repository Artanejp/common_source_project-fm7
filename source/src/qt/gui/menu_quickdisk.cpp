/*
 * Qt / DIsk Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   LIcense: GPLv2
 *   History: Jan 10, 2015 (MAYBE) : Initial.
 */

#include "menuclasses.h"
#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"

QT_BEGIN_NAMESPACE
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


// Common Routine
int Ui_MainWindow::write_protect_Qd(int drv, bool flag)
{
#ifdef USE_QD1
  if((drv < 0) || (drv >= MAX_QD)) return;
  if(emu) {
//    emu->write_protect_Qd(drv, flag);
  }
#endif
}
  
int Ui_MainWindow::set_recent_quick_disk(int drv, int num) 
 {
#ifdef USE_QD1
    QString s_path;
    char path_shadow[_MAX_PATH];
    int i;
    if((num < 0) || (num >= MAX_HISTORY)) return;
    s_path = QString::fromUtf8(config.recent_quickdisk_path[drv][num]);
    strncpy(path_shadow, s_path.toUtf8().constData(), _MAX_PATH);
    UPDATE_HISTORY(path_shadow, config.recent_quickdisk_path[drv]);
    
    strncpy(path_shadow, s_path.toUtf8().constData(), _MAX_PATH);
    get_parent_dir(path_shadow);
    strncpy(config.initial_quickdisk_dir, path_shadow, _MAX_PATH);
    strncpy(path_shadow, s_path.toUtf8().constData(), _MAX_PATH);

    if(emu) {
       emu->LockVM();
       emu->open_quickdisk(drv, path_shadow);
       emu->UnlockVM();
       //if(emu->is_write_protected_Qd(drive)) {
       //   actionProtection_ON_QD[drive]->setChecked(true);
       // } else {
	   actionProtection_OFF_QD[drv]->setChecked(true);
       // }
    }
    for(i = 0; i < MAX_HISTORY; i++) {
       if(action_Recent_List_QD[drv][i] != NULL) { 
	  action_Recent_List_QD[drv][i]->setText(QString::fromUtf8(config.recent_quickdisk_path[drv][i]));
	  //emit action_Recent_List_QD[drive][i]->changed();
       }
    }
#endif
}


void Ui_MainWindow::open_quick_disk_dialog(int drv)
{
#ifdef USE_QD1
  QString ext = "*.mzt *.q20 *.qdf";
  QString desc1 = "Quick DIsk";
  QString desc2;
  CSP_DiskDialog dlg;
  QString dirname;

  dlg.setWindowTitle("Open Quick Disk");
  
  desc2 = desc1 + " (" + ext.toLower() + ")";
  desc1 = desc1 + " (" + ext.toUpper() + ")";
  if(config.initial_disk_dir != NULL) {
    dirname = config.initial_quickdisk_dir;	        
  } else {
    char app[_MAX_PATH];
    QDir df;
    dirname = df.currentPath();
    strncpy(app, dirname.toUtf8().constData(), _MAX_PATH);
    dirname = get_parent_dir(app);
  }
  QStringList filter;
  filter << desc1 << desc2;

  dlg.param->setDrive(drv);
  dlg.setDirectory(dirname);
  dlg.setNameFilters(filter);
  QObject::connect(&dlg, SIGNAL(fileSelected(QString)), dlg.param, SLOT(_open_quick_disk(QString))); 
  QObject::connect(dlg.param, SIGNAL(do_open_quick_disk(int, QString)), this, SLOT(_open_quick_disk(int, QString))); 
  dlg.show();
  dlg.exec();
  return;
#endif
}



void Ui_MainWindow::_open_quick_disk(int drv, const QString fname)
{
   char path_shadow[_MAX_PATH];
   QString s_name = fname;
   int i;

#ifdef USE_QD1
   if(fname.length() <= 0) return;
//    s_name = fname;
    strncpy(path_shadow, s_name.toUtf8().constData(), _MAX_PATH);
//    UPDATE_HISTORY(path_shadow, config.recent_quickdisk_path[drv]);
    for(i = MAX_HISTORY - 1; i > 0; i--) {  
	strncpy(config.recent_quickdisk_path[drv][i], config.recent_quickdisk_path[drv][i -1], _MAX_PATH);
    }
    strncpy(config.recent_quickdisk_path[drv][0], path_shadow, _MAX_PATH);
   
    strncpy(path_shadow, s_name.toUtf8().constData(), _MAX_PATH);
    get_parent_dir(path_shadow);
    strncpy(config.initial_quickdisk_dir, path_shadow, _MAX_PATH);
    strncpy(path_shadow, s_name.toUtf8().constData(), _MAX_PATH);
    
   // Update List
   if(emu) {
      emu->LockVM();
      emu->open_quickdisk(drv, path_shadow);
      emu->UnlockVM();
   }
   
   for(i = 0; i < MAX_HISTORY; i++) {
       if(action_Recent_List_QD[drv][i] != NULL) { 
//	  printf("%s\n", config.recent_quickdisk_path[drv][i]);
	  action_Recent_List_QD[drv][i]->setText(QString::fromUtf8(config.recent_quickdisk_path[drv][i]));
       }
    }
//   if(emu->is_write_protected_Qd(drv)) {
//	actionProtection_ON_QD[drv]->setChecked(true);
//    } else {
	actionProtection_OFF_QD[drv]->setChecked(true);
//    }

#endif
}



void Ui_MainWindow::eject_Qd(int drv) 
{
#ifdef USE_QD1
   if(emu) {
      emu->LockVM();
      emu->close_quickdisk(drv);
      emu->UnlockVM();
   }
   
#endif
}

void Ui_MainWindow::CreateQuickDiskMenu(int drv, int drv_base)
{
#ifdef USE_QD1
  QString drv_base_name = QString::number(drv_base); 
  menuQD[drv] = new QMenu(menubar);
  menuQD[drv]->setObjectName(QString::fromUtf8("menuQD", -1) + drv_base_name);
  menuWrite_Protection_QD[drv] = new QMenu(menuQD[drv]);
  menuWrite_Protection_QD[drv]->setObjectName(QString::fromUtf8("menuWrite_Protection_QD", -1) + drv_base_name);
#endif
}

void Ui_MainWindow::CreateQuickDiskPulldownMenu(int drv)
{
#ifdef USE_QD1
  menuQD[drv]->addAction(actionInsert_QD[drv]);
  menuQD[drv]->addAction(actionEject_QD[drv]);
  menuQD[drv]->addSeparator();
  menuQD_Recent[drv] = new QMenu(menuQD[drv]);
  menuQD_Recent[drv]->setObjectName(QString::fromUtf8("Recent_QD", -1) + QString::number(drv));
  menuQD[drv]->addAction(menuQD_Recent[drv]->menuAction());
  //        menuQD[drv]->addAction(actionRecent_Opened_QD[0]);
  {
    int ii;
    for(ii = 0; ii < MAX_HISTORY; ii++) {
      menuQD_Recent[drv]->addAction(action_Recent_List_QD[drv][ii]);
      action_Recent_List_QD[drv][ii]->setVisible(true);
    }
    
  }
  menuQD[drv]->addSeparator();
  menuQD[drv]->addAction(menuWrite_Protection_QD[drv]->menuAction());
  menuWrite_Protection_QD[drv]->addAction(actionProtection_ON_QD[drv]);
  menuWrite_Protection_QD[drv]->addAction(actionProtection_OFF_QD[drv]);
#endif
}

void Ui_MainWindow::ConfigQuickDiskMenuSub(int drv)
{
#ifdef USE_QD1
  QString drive_name = QString::number(drv);
  
  actionInsert_QD[drv] = new Action_Control(this);
  actionInsert_QD[drv]->setObjectName(QString::fromUtf8("actionInsert_QD") + drive_name);
  actionInsert_QD[drv]->binds->setDrive(drv);
  actionInsert_QD[drv]->binds->setNumber(0);
  
  actionEject_QD[drv] = new Action_Control(this);
  actionEject_QD[drv]->setObjectName(QString::fromUtf8("actionEject_QD") + drive_name);
  actionEject_QD[drv]->binds->setDrive(drv);
  actionEject_QD[drv]->binds->setNumber(0);
  
  actionGroup_Opened_QD[drv] = new QActionGroup(this);
  actionRecent_Opened_QD[drv] = new Action_Control(this);
  actionRecent_Opened_QD[drv]->setObjectName(QString::fromUtf8("actionRecent_Opened_QD") + drive_name);
  actionRecent_Opened_QD[drv]->binds->setDrive(drv);
  actionRecent_Opened_QD[drv]->binds->setNumber(0);
  
  {
    int ii;
    actionGroup_Opened_QD[drv] = new QActionGroup(this);
    actionGroup_Opened_QD[drv]->setExclusive(true);
    
    actionRecent_Opened_QD[drv] = new Action_Control(this);
    actionRecent_Opened_QD[drv]->setObjectName(QString::fromUtf8("actionSelect_Recent_QD") + drive_name);
    actionRecent_Opened_QD[drv]->binds->setDrive(drv);
    actionRecent_Opened_QD[drv]->binds->setNumber(0);
    for(ii = 0; ii < MAX_HISTORY; ii++) {
      action_Recent_List_QD[drv][ii] = new Action_Control(this);
      action_Recent_List_QD[drv][ii]->binds->setDrive(drv);
      action_Recent_List_QD[drv][ii]->binds->setNumber(ii);
      action_Recent_List_QD[drv][ii]->setText(QString::fromUtf8(config.recent_quickdisk_path[drv][ii]));
      actionGroup_Opened_QD[drv]->addAction(action_Recent_List_QD[drv][ii]);
      connect(action_Recent_List_QD[drv][ii], SIGNAL(triggered()),
	      action_Recent_List_QD[drv][ii]->binds, SLOT(on_recent_quick_disk()));
      connect(action_Recent_List_QD[drv][ii]->binds, SIGNAL(set_recent_quick_disk(int, int)),
	      this, SLOT(set_recent_quick_disk(int, int)));
    }
  }
    {
    int ii;
    actionGroup_Protect_QD[drv] = new QActionGroup(this);
    actionGroup_Protect_QD[drv]->setExclusive(true);
    actionProtection_ON_QD[drv] = new Action_Control(this);
    actionProtection_ON_QD[drv]->setObjectName(QString::fromUtf8("actionProtection_ON_QD") + drive_name);
    actionProtection_ON_QD[drv]->setCheckable(true);
    actionProtection_ON_QD[drv]->setChecked(true);
    actionProtection_ON_QD[drv]->binds->setDrive(drv);
    actionProtection_ON_QD[drv]->binds->setNumber(0);
    actionProtection_OFF_QD[drv] = new Action_Control(this);
    actionProtection_OFF_QD[drv]->setObjectName(QString::fromUtf8("actionProtection_OFF_QD") + drive_name);
    actionProtection_OFF_QD[drv]->setCheckable(true);
    actionProtection_OFF_QD[drv]->binds->setDrive(drv);
    actionProtection_OFF_QD[drv]->binds->setNumber(1);

    actionGroup_Protect_QD[drv]->addAction(actionProtection_ON_QD[drv]);
    actionGroup_Protect_QD[drv]->addAction(actionProtection_OFF_QD[drv]);
       
    connect(actionProtection_ON_QD[drv], SIGNAL(triggered()), actionProtection_ON_QD[drv]->binds, SLOT(write_protect_Qd()));
    connect(actionProtection_ON_QD[drv]->binds, SIGNAL(sig_write_protect_Qd(int, bool)), this, SLOT(write_protect_Qd(int, bool)));
    connect(actionProtection_OFF_QD[drv], SIGNAL(triggered()), actionProtection_OFF_QD[drv]->binds, SLOT(no_write_protect_Qd()));
    connect(actionProtection_OFF_QD[drv]->binds, SIGNAL(sig_write_protect_Qd(int, bool)), this, SLOT(write_protect_Qd(int, bool)));
  }

  
  connect(actionInsert_QD[drv], SIGNAL(triggered()), actionInsert_QD[drv]->binds, SLOT(insert_Qd()));
  connect(actionInsert_QD[drv]->binds, SIGNAL(sig_insert_Qd(int)), this, SLOT(open_quick_disk_dialog(int)));
  connect(actionEject_QD[drv], SIGNAL(triggered()), actionEject_QD[drv]->binds, SLOT(eject_Qd()));
  connect(actionEject_QD[drv]->binds, SIGNAL(sig_eject_Qd(int)), this, SLOT(eject_Qd(int)));
  // Translate Menu
#endif
}

void Ui_MainWindow::retranslateQuickDiskMenu(int drv, int basedrv)
{
#ifdef USE_QD1
  QString drive_name = (QApplication::translate("MainWindow", "Quick Disk ", 0, QApplication::UnicodeUTF8));
  drive_name += QString::number(basedrv);
  
  if((drv < 0) || (drv >= 2)) return;
  actionInsert_QD[drv]->setText(QApplication::translate("MainWindow", "Insert", 0, QApplication::UnicodeUTF8));
  actionEject_QD[drv]->setText(QApplication::translate("MainWindow", "Eject", 0, QApplication::UnicodeUTF8));

  menuQD_Recent[drv]->setTitle(QApplication::translate("MainWindow", "Recent Opened", 0, QApplication::UnicodeUTF8));
  
  actionProtection_ON_QD[drv]->setText(QApplication::translate("MainWindow", "Protection ON", 0, QApplication::UnicodeUTF8));
  actionProtection_OFF_QD[drv]->setText(QApplication::translate("MainWindow", "Protection OFF", 0, QApplication::UnicodeUTF8));

  menuQD[drv]->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0, QApplication::UnicodeUTF8));
  menuWrite_Protection_QD[drv]->setTitle(QApplication::translate("MainWindow", "Write Protection", 0, QApplication::UnicodeUTF8));
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
QT_END_NAMESPACE
