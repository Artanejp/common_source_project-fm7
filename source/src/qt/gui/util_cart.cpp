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

void Ui_MainWindow::_open_cart(int drv, const QString fname)
{
   char path_shadow[PATH_MAX];
   int i;
#ifdef USE_CART1
   if(fname.length() <= 0) return;
   strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
   UPDATE_HISTORY(path_shadow, config.recent_cart_path[drv]);
   get_parent_dir(path_shadow);
   strcpy(config.initial_cart_dir, path_shadow);

   strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
   for(i = 0; i < MAX_HISTORY; i++) {
	 if(action_Recent_List_CART[drv][i] != NULL) { 
	    action_Recent_List_CART[drv][i]->setText(QString::fromUtf8(config.recent_cart_path[drv][i]));
	    //actiont_Recent_List_FD[drv][i]->changed();
      }
   }
   
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
//      emu->LockVM();
      emu->open_cart(drv, path_shadow);
      for(i = 0; i < MAX_HISTORY; i++) {
	 if(action_Recent_List_CART[drv][i] != NULL) { 
	    action_Recent_List_CART[drv][i]->setText(QString::fromUtf8(config.recent_cart_path[drv][i]));
	    //actiont_Recent_List_FD[drv][i]->changed();
	 }
      }
//      emu->UnlockVM();
   }
}
#endif

QT_END_NAMESPACE
