/*
 * UI->Qt->MainWindow : Cartridge Utils.
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

QT_BEGIN_NAMESPACE
#if defined(USE_CART1) || defined(USE_CART2)

void Ui_MainWindow::open_cart_dialog(int drive)
{
  QString ext;
  QString desc1;
  QString desc2;
  CSP_DiskDialog dlg;
  QString dirname;
#if defined(_GAMEGEAR)
  ext = "*.rom *.bin *.gg *.col";
  desc1 = "Game Cartridge";
#elif defined(_MASTERSYSTEM)
  ext = "*.rom *.bin *.sms";
  desc1 = "Game Cartridge";
#elif defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
  ext = "*.rom *.bin *.60";
  desc1 = "Game Cartridge";
#elif defined(_PCENGINE) || defined(_X1TWIN)
  ext = "*.rom *.bin *.pce";
  desc1 = "HuCARD";
#else
  ext = "*.rom *.bin";
  desc1 = "Game Cartridge";
#endif
  desc2.number(drive + 1);
  desc2 = QString::fromUtf8("Open Cartridge on #") + desc2;
  dlg.setWindowTitle(desc2);
			    
  desc2 = desc1 + " (" + ext.toLower() + ")";
  desc1 = desc1 + " (" + ext.toUpper() + ")";
  
  if(config.initial_tape_dir != NULL) {
    dirname = config.initial_cart_dir;	        
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
  dlg.setNameFilters(filter); 
  QObject::connect(&dlg, SIGNAL(fileSelected(QString)), dlg.param, SLOT(_open_cart(QString))); 
  QObject::connect(dlg.param, SIGNAL(sig_open_cart(int, QString)), this, SLOT(_open_cart(int, QString))); 
  dlg.show();
  dlg.exec();
  return;
}
#endif

void Ui_MainWindow::_open_cart(int drv, const QString fname)
{
   char path_shadow[PATH_MAX];
#ifdef USE_CART1
   if(fname.length() <= 0) return;
   strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
   UPDATE_HISTORY(path_shadow, config.recent_cart_path[drv]);
   get_parent_dir(path_shadow);
   strcpy(config.initial_cart_dir, path_shadow);

   strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
   if(emu) {
//     emu->LockVM();
     emu->open_cart(drv, path_shadow);
//     emu->UnlockVM();
   }
#endif
}

#if defined(USE_CART1) || defined(USE_CART2)

void Ui_MainWindow::eject_cart(int drv) 
{
   if(emu) {
//      emu->LockVM();
      emu->close_cart(drv);
//      emu->UnlockVM();
   }

}

void Ui_MainWindow::set_recent_cart(int drv, int num) 
{
    QString s_path;
    char path_shadow[PATH_MAX];
    int i;
    
    if((num < 0) || (num >= MAX_HISTORY)) return;
    
   s_path = QString::fromUtf8(config.recent_cart_path[drv][num]);
   strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
   UPDATE_HISTORY(path_shadow, config.recent_cart_path[drv]);
   strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
   
    get_parent_dir(path_shadow);
    strcpy(config.initial_cart_dir, path_shadow);
    strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
   
   if(emu) {
      eject_cart(drv);
      emu->LockVM();
      emu->open_cart(drv, path_shadow);
      for(i = 0; i < MAX_HISTORY; i++) {
	 if(action_Recent_List_CART[drv][i] != NULL) { 
	    action_Recent_List_CART[drv][i]->setText(QString::fromUtf8(config.recent_cart_path[drv][i]));
	    //actiont_Recent_List_FD[drv][i]->changed();
	 }
      }
      emu->UnlockVM();
   }
}
#endif

QT_END_NAMESPACE
