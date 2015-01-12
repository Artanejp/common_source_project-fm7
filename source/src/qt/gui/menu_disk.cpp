/*
 * Qt / DIsk Menu, Utilities
 */

#include "menuclasses.h"
#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"

QT_BEGIN_NAMESPACE
void Object_Menu_Control::insert_fd(void) {
   emit sig_insert_fd(drive);
}
void Object_Menu_Control::eject_fd(void) {
   emit sig_eject_fd(drive);
}

#ifdef USE_FD1
void Ui_MainWindow::open_disk_dialog(int drv)
{
  QString ext = "Disk Images (*.d88,*.d77,*.td0,*.imd,*.dsk,*.fdi,*.hdm,*.tfd,*.xdf,*.2d,*.sf7)";
  QString desc = "Floppy Disk";
  CSP_DiskDialog dlg;
  QString dirname;
  
  if(config.initial_disk_dir != NULL) {
    dirname = config.initial_disk_dir;	        
  } else {
    char app[PATH_MAX];
    QDir df;
    dirname = df.currentPath();
    strncpy(app, dirname.toUtf8().constData(), PATH_MAX);
    dirname = get_parent_dir(app);
  }
  QStringList filter(ext);
  dlg.param->setDrive(drv);
  dlg.setDirectory(dirname);
//  dlg.setNameFilters(filter); 
  QObject::connect(&dlg, SIGNAL(fileSelected(QString)), dlg.param, SLOT(_open_disk(QString))); 
  QObject::connect(dlg.param, SIGNAL(do_open_disk(int, QString)), this, SLOT(_open_disk(int, QString))); 
  dlg.show();
  dlg.exec();
  return;
}

#endif

void Ui_MainWindow::_open_disk(int drv, const QString fname)
{
   char path_shadow[PATH_MAX];

#ifdef USE_FD1
   
   if(fname.length() <= 0) return;
   strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
   UPDATE_HISTORY(path_shadow, config.recent_disk_path[drv]);
//   get_parent_dir(path_shadow);
   strcpy(config.initial_disk_dir, path_shadow);
   open_disk(drv, path_shadow, 0);
#endif
}

void Ui_MainWindow::_open_cart(int drv, const QString fname)
{
   char path_shadow[PATH_MAX];
#ifdef USE_CART1
   if(fname.length() <= 0) return;
   strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
   UPDATE_HISTORY(path_shadow, config.recent_cart_path[drv]);
   get_parent_dir(path_shadow);
   strcpy(config.initial_cart_dir, path_shadow);

   if(emu) emu->open_cart(drv, path_shadow);
#endif
}

void Ui_MainWindow::_open_cmt(bool mode, const QString path)
{
  char path_shadow[PATH_MAX];
  int play;
   
   play = (mode == false)? 0 : 1;
#ifdef USE_TAPE
  if(path.length() <= 0) return;
  strncpy(path_shadow, path.toUtf8().constData(), PATH_MAX);
  UPDATE_HISTORY(path_shadow, config.recent_tape_path);
  get_parent_dir(path_shadow);
  strcpy(config.initial_tape_dir, path_shadow);
   if(play != 0) {
      emu->play_tape(path_shadow);
  } else {
      emu->rec_tape(path_shadow);
  }
#endif
}

void Ui_MainWindow::eject_fd(int drv) 
{
   close_disk(drv);
}



void Ui_MainWindow::ConfigFloppyMenu(Ui_MainWindow *p)
{
   
#if defined(USE_FD1)
        actionInsert_FD1 = new Action_Control(p);
        actionInsert_FD1->setObjectName(QString::fromUtf8("actionInsert_FD1"));
        actionInsert_FD1->binds->setDrive(0);
        actionInsert_FD1->binds->setNumber(0);
   
        actionEject_FD1 = new Action_Control(p);
        actionEject_FD1->setObjectName(QString::fromUtf8("actionEject_FD1"));
        actionEject_FD1->binds->setDrive(0);
        actionEject_FD1->binds->setNumber(0);
   
        actionRecent_Opened_FD1 = new Action_Control(p);
        actionRecent_Opened_FD1->setObjectName(QString::fromUtf8("actionRecent_Opened_FD1"));
        actionRecent_Opened_FD1->binds->setDrive(0);
        actionRecent_Opened_FD1->binds->setNumber(0);
   
        actionSelect_D88_Image_FD1 = new Action_Control(p);
        actionSelect_D88_Image_FD1->setObjectName(QString::fromUtf8("actionSelect_D88_Image_FD1"));
        actionSelect_D88_Image_FD1->binds->setDrive(0);
        actionSelect_D88_Image_FD1->binds->setNumber(0);
  
        actionProtection_ON_FD1 = new Action_Control(p);
        actionProtection_ON_FD1->setObjectName(QString::fromUtf8("actionProtection_ON_FD1"));
        actionProtection_ON_FD1->setCheckable(true);
        actionProtection_ON_FD1->setChecked(true);
        actionProtection_OFF_FD1 = new Action_Control(p);
        actionProtection_OFF_FD1->setObjectName(QString::fromUtf8("actionProtection_OFF_FD1"));
        actionProtection_OFF_FD1->setCheckable(true);
   
        connect(actionInsert_FD1, SIGNAL(triggered()), actionInsert_FD1->binds, SLOT(insert_fd()));
        connect(actionInsert_FD1->binds, SIGNAL(sig_insert_fd(int)), this, SLOT(open_disk_dialog(int)));
        connect(actionEject_FD1, SIGNAL(triggered()), actionEject_FD1->binds, SLOT(eject_fd()));
        connect(actionEject_FD1->binds, SIGNAL(sig_eject_fd(int)), this, SLOT(eject_fd(int)));
   
#endif
   
#if defined(USE_FD2)
        actionInsert_FD2 = new Action_Control(p);
        actionInsert_FD2->setObjectName(QString::fromUtf8("actionInsert_FD2"));
        actionInsert_FD2->binds->setDrive(1);
        actionInsert_FD2->binds->setNumber(0);
   
        actionEject_FD2 = new Action_Control(p);
        actionEject_FD2->setObjectName(QString::fromUtf8("actionEject_FD2"));
        actionEject_FD2->binds->setDrive(1);
        actionEject_FD2->binds->setNumber(0);

        actionRecent_Opened_FD2 = new Action_Control(p);
        actionRecent_Opened_FD2->setObjectName(QString::fromUtf8("actionRecent_Opened_FD2"));
        actionRecent_Opened_FD2->binds->setDrive(1);
        actionRecent_Opened_FD2->binds->setNumber(0);

        actionSelect_D88_Image_FD2 = new Action_Control(p);
        actionSelect_D88_Image_FD2->setObjectName(QString::fromUtf8("actionSelect_D88_Image_FD2"));
        actionSelect_D88_Image_FD2->binds->setDrive(1);
        actionSelect_D88_Image_FD2->binds->setNumber(0);

        actionProtection_ON_FD2 = new Action_Control(p);
        actionProtection_ON_FD2->setObjectName(QString::fromUtf8("actionProtection_ON_FD2"));
        actionProtection_ON_FD2->setCheckable(true);
        actionProtection_ON_FD2->setChecked(true);
        actionProtection_ON_FD2->binds->setDrive(1);
        actionProtection_ON_FD2->binds->setNumber(0);

        actionProtection_OFF_FD2 = new Action_Control(p);
        actionProtection_OFF_FD2->setObjectName(QString::fromUtf8("actionProtection_OFF_FD2"));
        actionProtection_OFF_FD2->setCheckable(true);
        actionProtection_OFF_FD2->binds->setDrive(1);
        actionProtection_OFF_FD2->binds->setNumber(0);
   
       // Connect
        connect(actionInsert_FD2, SIGNAL(triggered()), actionInsert_FD2->binds, SLOT(insert_fd()));
        connect(actionInsert_FD2->binds, SIGNAL(sig_insert_fd(int)), this, SLOT(open_disk_dialog(int)));
        connect(actionEject_FD2, SIGNAL(triggered()), actionEject_FD2->binds, SLOT(eject_fd()));
        connect(actionEject_FD2->binds, SIGNAL(sig_eject_fd(int)), this, SLOT(eject_fd(int)));
      
#endif
   
}
QT_END_NAMESPACE
