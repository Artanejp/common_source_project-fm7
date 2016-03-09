/*
 * UI->Qt->MainWindow : Some Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */

#include "commonclasses.h"
#include "mainwidget.h"
#include "qt_gldraw.h"

//#include "menuclasses.h"
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
	emit sig_emu_update_config();
}

void Ui_MainWindow::set_freq(int num)
{
	if((num < 0) || (num > 7)) return;
	config.sound_frequency = num;
	emit sig_emu_update_config();
}

void Ui_MainWindow::set_sound_device(int num)
{
#ifdef USE_SOUND_DEVICE_TYPE
	if((num < 0) || (num >7)) return;
	config.sound_device_type = num;
	emit sig_emu_update_config();
#endif
}



void Ui_MainWindow::start_record_sound(bool start)
{
	if(start) {
		actionStart_Record->setText(QApplication::translate("MainWindow", "Stop Recorded Sound", 0));
		actionStart_Record->setIcon(StopIcon);
		emit sig_emu_start_rec_sound();
	} else {
		actionStart_Record->setText(QApplication::translate("MainWindow", "Start Recording Sound", 0));
		actionStart_Record->setIcon(RecordSoundIcon);
		emit sig_emu_stop_rec_sound();
	}
}

void Ui_MainWindow::set_monitor_type(int num)
{
#ifdef USE_MONITOR_TYPE
	if((num < 0) || (num >7)) return;
	config.monitor_type = num;
	emit sig_emu_update_config();
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
	emit sig_emu_update_config();
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
#if defined(USE_CRT_FILTER)
void Ui_MainWindow::set_crt_filter(bool flag)
{
	config.crt_filter = flag;
}
#endif

void Ui_MainWindow::set_gl_crt_filter(bool flag)
{
	config.use_opengl_filters = flag;
}

#ifdef DATAREC_SOUND
void Ui_MainWindow::set_cmt_sound(bool flag)
{
	config.tape_sound = flag;
	emit sig_emu_update_config();
}
#endif
#ifdef USE_DEVICE_TYPE
void Ui_MainWindow::set_device_type(int num)
{
	if((num < USE_DEVICE_TYPE) && (num >= 0)) {
		config.device_type = num;
		emit sig_emu_update_config();
	}
}
#endif

#ifdef USE_DRIVE_TYPE
void Ui_MainWindow::set_drive_type(int num)
{
	if((num < USE_DRIVE_TYPE) && (num >= 0)) {
		config.drive_type = num;
		emit sig_emu_update_config();
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
	this->resize_statusbar(w, h);
   
	MainWindow->centralWidget()->adjustSize();
	MainWindow->adjustSize();
}

void Ui_MainWindow::set_screen_aspect(int num)
{
	if((num < 0) || (num >= 4)) return;
	// 0 = DOT
	// 1 = ASPECT(Scale X)
	// 2 = ASPECT(SCale Y)
	// 3 = ASPECT(Scale X,Y)
	// On Common Sourcecode Project / Agar,
	// Scaling is done by Agar Widget.
	// So, does need below action?
	// Maybe, needs Agar's changing action. 
	
	config.window_stretch_type = num;
	
	if(emu) {
		int w, h, n;
		double nd, ww, hh;
		n = config.window_mode;
		if(n < 0) n = 1;
		nd = actionScreenSize[n]->binds->getDoubleValue();
		ww = nd * (double)SCREEN_WIDTH;
		hh = nd * (double)SCREEN_HEIGHT;
#if (WINDOW_HEIGHT_ASPECT != WINDOW_HEIGHT) || (WINDOW_WIDTH_ASPECT != WINDOW_WIDTH)
		double par_w = (double)WINDOW_WIDTH_ASPECT / (double)WINDOW_WIDTH;
		double par_h = (double)WINDOW_HEIGHT_ASPECT / (double)WINDOW_HEIGHT;
		double par = par_h / par_w;
		if(config.window_stretch_type == 1) { // refer to X, scale Y.
			hh = hh * par_h;
		} else if(config.window_stretch_type == 2) { // refer to Y, scale X only
			ww = ww / par_h;
		} else if(config.window_stretch_type == 3) { // Scale both X, Y
			ww = ww * par_w;
			hh = hh * par_h;
		}
#endif
		w = (int)ww;
		h = (int)hh;
		this->set_screen_size(w, h);
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

void Ui_MainWindow::ConfigPrinterType(void)
{
#if defined(USE_PRINTER)
	int i;
	QString tmps;
	int ilim = 2;
#if defined(USE_PRINTER_TYPE)
	ilim = USE_PRINTER_TYPE;
#endif	
	menuPrintDevice = new QMenu(menuMachine);
	menuPrintDevice->setObjectName(QString::fromUtf8("menu_PrintDevice"));
   
	actionGroup_PrintDevice = new QActionGroup(this);
	actionGroup_PrintDevice->setObjectName(QString::fromUtf8("actionGroup_PrintDevice"));
	actionGroup_PrintDevice->setExclusive(true);
	menuMachine->addAction(menuPrintDevice->menuAction());   
	for(i = 0; i < ilim; i++) {
		actionPrintDevice[i] = new Action_Control(this);
		actionPrintDevice[i]->setCheckable(true);
		actionPrintDevice[i]->binds->setValue1(i);
		if(i == config.printer_device_type) actionPrintDevice[i]->setChecked(true); // Need to write configure
		tmps = QString::fromUtf8("actionPrintDevice_");
		actionPrintDevice[i]->setObjectName(tmps + QString::number(i));
		menuPrintDevice->addAction(actionPrintDevice[i]);
		actionGroup_PrintDevice->addAction(actionPrintDevice[i]);
		connect(actionPrintDevice[i], SIGNAL(triggered()),
			actionPrintDevice[i]->binds, SLOT(do_set_printer_device()));
		connect(actionPrintDevice[i]->binds, SIGNAL(sig_printer_device(int)),
			this, SLOT(set_printer_device(int)));
	}
#endif
}


#if defined(USE_PRINTER)
void Ui_MainWindow::set_printer_device(int p_type)
{
	// 0 = PRNFILE
	if(p_type < 0) p_type = 0; // OK?
#if defined(USE_PRINTER_TYPE)
	if(p_type >= USE_PRINTER_TYPE) p_type = USE_PRINTER_TYPE - 1;
#else
	if(p_type >= 2) p_type = 1;
#endif	
	config.printer_device_type = p_type;
	emit sig_emu_update_config();
}
#endif
QT_END_NAMESPACE
