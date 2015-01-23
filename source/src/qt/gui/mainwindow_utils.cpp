/*
 * UI->Qt->MainWindow : Some Utils.
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

extern const int s_freq_table[];
extern const double s_late_table[];

void Ui_MainWindow::set_latency(int num)
{
   if((num < 0) || (num > 4)) return;
   config.sound_latency = num;
   if(emu) {
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }
}

void Ui_MainWindow::set_freq(int num)
{
   if((num < 0) || (num > 7)) return;
   config.sound_frequency = num;
   if(emu) {
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }
}

void Ui_MainWindow::set_sound_device(int num)
{
#ifdef USE_SOUND_DEVICE_TYPE
   if((num < 0) || (num >7)) return;
   config.sound_device_type = num;
   if(emu) {
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }
#endif
}

void Ui_MainWindow::start_record_sound(bool start)
{
   if(emu) {
	if(start) {
	  emu->LockVM();
	  emu->start_rec_sound();
	  emu->UnlockVM();
	} else {
	  emu->LockVM();
	  emu->stop_rec_sound();
	  emu->UnlockVM();
	}
   }
}

void Ui_MainWindow::set_monitor_type(int num)
{
#ifdef USE_MONITOR_TYPE
   if((num < 0) || (num >7)) return;
   config.monitor_type = num;
   if(emu) {
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }
#endif
}

#if defined(USE_SCANLINE)
void Ui_MainWindow::set_scan_line(bool flag)
{
  if(flag) {
    config.scan_line = ~0;
  } else {
    config.scan_line = 0;
  }
  if(emu) {
    emu->LockVM();
    emu->update_config();
    emu->UnlockVM();
  }
}
#endif

void Ui_MainWindow::set_screen_size(int w, int h)
{
  if((w <= 0) || (h <= 0)) return;
  //QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  
  this->graphicsView->setFixedSize(w, h);
  MainWindow->centralWidget()->adjustSize();
  MainWindow->adjustSize();

  //  graphicsView->setSizePolicy(sizePolicy);
  //MainWindow->centralWidget()->setSizePolicy(sizePolicy);
	
}


void Ui_MainWindow::set_screen_aspect(int num)
{
  if((num < 0) || (num >= 3)) return;
  double ww = SCREEN_WIDTH;
  double hh = SCREEN_HEIGHT;
  double whratio = ww / hh;
  double ratio;
  int width, height;
  QSizePolicy policy;
  // 0 = DOT
  // 1 = ASPECT
  // 2 = FILL
  // On Common Sourcecode Project / Agar,
  // Scaling is done by Agar Widget.
  // So, does need below action?
  // Maybe, needs Agar's changing action. 

  config.stretch_type = num;
  
  if(emu) {
    int w, h;
    w = this->graphicsView->width();
    h = this->graphicsView->height();
    this->graphicsView->resizeGL(w, h);
  }
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

 
QT_END_NAMESPACE
