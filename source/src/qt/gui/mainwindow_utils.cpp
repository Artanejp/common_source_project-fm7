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
		if(emu) {
			emu->LockVM();
			emu->update_config();
			emu->UnlockVM();
		}
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


void Ui_MainWindow::ConfigDeviceType(void)
{
#if defined(USE_DEVICE_TYPE)
	{
		int ii;
		menuDeviceType = new QMenu(menuMachine);
		menuDeviceType->setObjectName(QString::fromUtf8("menuDeviceType"));
		menuMachine->addAction(menuDeviceType->menuAction());
      
		actionGroup_DeviceType = new QActionGroup(this);
		actionGroup_DeviceType->setExclusive(true);
		for(ii = 0; ii < USE_DEVICE_TYPE; ii++) {
			actionDeviceType[ii] = new Action_Control(this);
			actionGroup_DeviceType->addAction(actionDeviceType[ii]);
			actionDeviceType[ii]->setCheckable(true);
			actionDeviceType[ii]->setVisible(true);
			actionDeviceType[ii]->binds->setValue1(ii);
			if(config.device_type == ii) actionDeviceType[ii]->setChecked(true);
			menuDeviceType->addAction(actionDeviceType[ii]);
			connect(actionDeviceType[ii], SIGNAL(triggered()),
				actionDeviceType[ii]->binds, SLOT(do_set_device_type()));
			connect(actionDeviceType[ii]->binds, SIGNAL(sig_device_type(int)),
				this, SLOT(set_device_type(int)));
		}
	}
#endif
}

void Ui_MainWindow::ConfigDriveType(void)
{
	int i;
#ifdef USE_DRIVE_TYPE
	menuDriveType = new QMenu(menuMachine);
	menuDriveType->setObjectName(QString::fromUtf8("menu_DriveType"));
   
	actionGroup_DriveType = new QActionGroup(this);
	actionGroup_DriveType->setObjectName(QString::fromUtf8("actionGroup_DriveType"));
	actionGroup_DriveType->setExclusive(true);
	menuMachine->addAction(menuDriveType->menuAction());
	for(i = 0; i < USE_DRIVE_TYPE; i++) {
		actionDriveType[i] = new Action_Control(this);
		actionDriveType[i]->setCheckable(true);
		actionDriveType[i]->setVisible(true);
		actionDriveType[i]->binds->setValue1(i);
		if(i == config.drive_type) actionDriveType[i]->setChecked(true); // Need to write configure
		actionGroup_DriveType->addAction(actionDriveType[i]);
		menuDriveType->addAction(actionDriveType[i]);
		connect(actionDriveType[i], SIGNAL(triggered()),
			actionDriveType[i]->binds, SLOT(do_set_drive_type()));
		connect(actionDriveType[i]->binds, SIGNAL(sig_drive_type(int)),
			this, SLOT(set_drive_type(int)));
	}
#endif
}

void Ui_MainWindow::ConfigSoundDeviceType(void)
{
#ifdef USE_SOUND_DEVICE_TYPE
	int i;
	QString tmps;
	menuSoundDevice = new QMenu(menuMachine);
	menuSoundDevice->setObjectName(QString::fromUtf8("menu_SoundDevice"));
   
	actionGroup_SoundDevice = new QActionGroup(this);
	actionGroup_SoundDevice->setObjectName(QString::fromUtf8("actionGroup_SoundDevice"));
	actionGroup_SoundDevice->setExclusive(true);
	menuMachine->addAction(menuSoundDevice->menuAction());   
	for(i = 0; i < USE_SOUND_DEVICE_TYPE; i++) {
		actionSoundDevice[i] = new Action_Control(this);
		actionSoundDevice[i]->setCheckable(true);
		actionSoundDevice[i]->binds->setValue1(i);
		if(i == config.sound_device_type) actionSoundDevice[i]->setChecked(true); // Need to write configure
		tmps = QString::fromUtf8("actionSoundDevice_");
		actionSoundDevice[i]->setObjectName(tmps + QString::number(i));
		menuSoundDevice->addAction(actionSoundDevice[i]);
		actionGroup_SoundDevice->addAction(actionSoundDevice[i]);
		connect(actionSoundDevice[i], SIGNAL(triggered()),
			actionSoundDevice[i]->binds, SLOT(do_set_sound_device()));
		connect(actionSoundDevice[i]->binds, SIGNAL(sig_sound_device(int)),
			this, SLOT(set_sound_device(int)));
	}
#endif
}
QT_END_NAMESPACE
