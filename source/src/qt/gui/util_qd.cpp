/*
 * UI->Qt->MainWindow : Quick Disk Utils.
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

int Ui_MainWindow::write_protect_Qd(int drv, bool flag)
{
#ifdef USE_QD1
  if((drv < 0) || (drv >= MAX_QD)) return -1;
  if(emu) {
//    emu->write_protect_Qd(drv, flag);
  }
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
    return 0;
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

QT_END_NAMESPACE
