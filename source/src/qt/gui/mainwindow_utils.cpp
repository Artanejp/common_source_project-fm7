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
		emu->LockVM();
		if(start) {
			emu->start_rec_sound();
		} else {
			emu->stop_rec_sound();
		}
		emu->UnlockVM();
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

#if defined(USE_SCREEN_ROTATE)
void Ui_MainWindow::set_screen_rotate(bool flag)
{
	config.rotate_type = flag;
	if(config.window_mode >= _SCREEN_MODE_NUM) config.window_mode = _SCREEN_MODE_NUM - 1;
	if(config.window_mode < 0) config.window_mode = 0;
	if(actionScreenSize[config.window_mode] != NULL) {
		actionScreenSize[config.window_mode]->binds->set_screen_size();
	}
}
#endif

#ifdef DATAREC_SOUND
void Ui_MainWindow::set_cmt_sound(bool flag)
{
	config.tape_sound = flag;
	if(emu) {
		emu->LockVM();
		emu->update_config();
		emu->UnlockVM();
	}
}
#endif
#ifdef USE_DEVICE_TYPE
void Ui_MainWindow::set_device_type(int num)
{
	if((num < USE_DEVICE_TYPE) && (num >= 0)) {
		config.device_type = num;
		//if(emu) {
		//	emu->update_config();
		//}
	}
}
#endif

#ifdef USE_DRIVE_TYPE
void Ui_MainWindow::set_drive_type(int num)
{
	if((num < USE_DRIVE_TYPE) && (num >= 0)) {
		config.drive_type = num;
		if(emu) {
			emu->update_config();
		}
	}
}
#endif
   

void Ui_MainWindow::set_screen_size(int w, int h)
{
	if((w <= 0) || (h <= 0)) return;
#if defined(USE_SCREEN_ROTATE)
	if(config.rotate_type) {
		this->graphicsView->setFixedSize(h, w);
	} else
#endif     
	{
		this->graphicsView->setFixedSize(w, h);
	}
   
	MainWindow->centralWidget()->adjustSize();
	MainWindow->adjustSize();
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
 
QT_END_NAMESPACE
