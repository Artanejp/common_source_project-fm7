/*
 * UI->Qt->MainWindow : Some Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */
#include <QApplication>
#include <QMenu>
#include <QMenuBar>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "qt_gldraw.h"

//#include "menuclasses.h"
#include "qt_dialogs.h"
//#include "csp_logger.h"

void Ui_MainWindowBase::do_block_task(void)
{
	emit sig_block_task();
}

void Ui_MainWindowBase::do_unblock_task(void)
{
	emit sig_unblock_task();
}
void Ui_MainWindowBase::set_latency(int num)
{
	if((num < 0) || (num >= 8)) return;
	p_config->sound_latency = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::set_freq(int num)
{
	if((num < 0) || (num >= 16)) return;
	p_config->sound_frequency = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::set_sound_device(int num)
{
	if((num < 0) || (num >= using_flags->get_use_sound_device_type())) return;
	p_config->sound_type = num;
	emit sig_emu_update_config();
}



void Ui_MainWindowBase::start_record_sound(bool start)
{
	if(start) {
		actionStart_Record->setText(QApplication::translate("SoundMenu", "Stop Recorded Sound", 0));
		actionStart_Record->setIcon(StopIcon);
		emit sig_emu_start_rec_sound();
	} else {
		actionStart_Record->setText(QApplication::translate("SoundMenu", "Start Recording Sound", 0));
		actionStart_Record->setIcon(RecordSoundIcon);
		emit sig_emu_stop_rec_sound();
	}
}

void Ui_MainWindowBase::set_monitor_type(int num)
{
	if((num < 0) || (num >= using_flags->get_use_monitor_type())) return;
	p_config->monitor_type = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::set_scan_line(bool flag)
{
	p_config->scan_line = flag;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::do_clear_keyname_table()
{
	phys_key_name_map.clear();
}

void Ui_MainWindowBase::do_add_keyname_table(uint32_t vk, QString name)
{
	phys_key_name_map.insert(vk, name);
//	printf("VK=%02X NAME=%s\n", vk, name.toLocal8Bit().constData());
}

void Ui_MainWindowBase::do_set_screen_rotate(int type)
{
	p_config->rotate_type = type;
	if(p_config->window_mode >= using_flags->get_screen_mode_num()) p_config->window_mode = using_flags->get_screen_mode_num() - 1;
	if(p_config->window_mode < 0) p_config->window_mode = 0;
	if(actionScreenSize[p_config->window_mode] != NULL) {
		actionScreenSize[p_config->window_mode]->binds->set_screen_size();
	}
}

void Ui_MainWindowBase::set_gl_crt_filter(bool flag)
{
	p_config->use_opengl_filters = flag;
}

void Ui_MainWindowBase::set_cmt_sound(bool flag)
{
	//p_config->tape_sound = flag;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::set_mouse_type(int num)
{
	if((num >= using_flags->get_use_mouse_type()) && (num < 0)) return;
	p_config->mouse_type = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::set_device_type(int num)
{
	if((num >= using_flags->get_use_device_type()) && (num < 0)) return;
	p_config->device_type = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::set_keyboard_type(int num)
{
	if((num >= using_flags->get_use_keyboard_type()) && (num < 0)) return;
	p_config->keyboard_type = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::set_joystick_type(int num)
{
	if((num >= using_flags->get_use_joystick_type()) && (num < 0)) return;
	p_config->joystick_type = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::set_drive_type(int num)
{
	if((num >= using_flags->get_use_drive_type()) && (num < 0)) return;
	p_config->drive_type = num;
	emit sig_emu_update_config();
}
   

void Ui_MainWindowBase::set_screen_size(int w, int h)
{
	if((w <= 0) || (h <= 0)) return;
	if((p_config->rotate_type == 1) || (p_config->rotate_type == 3)) {
		this->graphicsView->setFixedSize(h, w);
		this->resize_statusbar(h, w);
		//emit sig_resize_osd(h);
	} else {
		this->graphicsView->setFixedSize(w, h);
		this->resize_statusbar(w, h);
		//emit sig_resize_osd(w);
	}
   
	MainWindow->centralWidget()->adjustSize();
	MainWindow->adjustSize();
}

void Ui_MainWindowBase::set_screen_aspect(int num)
{
	if((num < 0) || (num >= 4)) return;
	// 0 = DOT
	// 1 = ASPECT(Scale X)
	// 2 = ASPECT(SCale Y)
	// 3 = ASPECT(Scale X,Y)
	
	p_config->window_stretch_type = num;
	
	if(using_flags->get_emu()) {
		int w, h, n;
		float nd, ww, hh;
		float xzoom = using_flags->get_screen_x_zoom();
		float yzoom = using_flags->get_screen_y_zoom();
		n = p_config->window_mode;
		if(n < 0) n = 1;
		nd = actionScreenSize[n]->binds->getDoubleValue();
		ww = (float)using_flags->get_screen_width();
		hh = (float)using_flags->get_screen_height();
		
		if((using_flags->get_screen_height_aspect() != using_flags->get_screen_height()) ||
		   (using_flags->get_screen_width_aspect() != using_flags->get_screen_width())) {
			float par_w = (float)using_flags->get_screen_width_aspect() / ww;
			float par_h = (float)using_flags->get_screen_height_aspect() / hh;
			//double par = par_h / par_w;
			switch(p_config->window_stretch_type) {
			case 0: // refer to X and Y.
				ww = ww * nd * xzoom;
				hh = hh * nd * yzoom;
				break;
			case 1: // refer to X, scale Y only
				ww = ww * nd * xzoom;
				hh = hh * nd * par_h;
				break;
			case 2: // refer to Y, scale X only
				ww = (ww * nd) / par_h * yzoom;
				hh = hh * nd * yzoom;
				break;
			case 3:
				ww = ((ww * nd) / par_h) * yzoom;
				hh = ((hh * nd) / par_w) * xzoom;
				break;
			}
		} else {
			ww = ww * nd * xzoom;
			hh = hh * nd * yzoom;
		}
		w = (int)ww;
		h = (int)hh;
		this->set_screen_size(w, h);
	}
}


void Ui_MainWindowBase::ConfigDeviceType(void)
{
//	if(using_flags->is_use_variable_memory()) {
//	}		
	if(using_flags->get_use_device_type() > 0) {
		int ii;
		menuDeviceType = new QMenu(menuMachine);
		menuDeviceType->setObjectName(QString::fromUtf8("menuDeviceType"));
		menuMachine->addAction(menuDeviceType->menuAction());
		menuDeviceType->setToolTipsVisible(true);
      
		actionGroup_DeviceType = new QActionGroup(this);
		actionGroup_DeviceType->setExclusive(true);
		for(ii = 0; ii < using_flags->get_use_device_type(); ii++) {
			actionDeviceType[ii] = new Action_Control(this, using_flags);
			actionGroup_DeviceType->addAction(actionDeviceType[ii]);
			actionDeviceType[ii]->setCheckable(true);
			actionDeviceType[ii]->setVisible(true);
			actionDeviceType[ii]->binds->setValue1(ii);
			if(p_config->device_type == ii) actionDeviceType[ii]->setChecked(true);
			menuDeviceType->addAction(actionDeviceType[ii]);
			connect(actionDeviceType[ii], SIGNAL(triggered()),
				actionDeviceType[ii]->binds, SLOT(do_set_device_type()));
			connect(actionDeviceType[ii]->binds, SIGNAL(sig_device_type(int)),
				this, SLOT(set_device_type(int)));
		}
	}
}

void Ui_MainWindowBase::ConfigJoystickType(void)
{
	if(using_flags->get_use_joystick_type() > 0) {
		int ii;
		menuJoystickType = new QMenu(menuMachine);
		menuJoystickType->setObjectName(QString::fromUtf8("menuJoystickType"));
		menuMachine->addAction(menuJoystickType->menuAction());
		menuJoystickType->setToolTipsVisible(true);
      
		actionGroup_JoystickType = new QActionGroup(this);
		actionGroup_JoystickType->setExclusive(true);
		for(ii = 0; ii < using_flags->get_use_joystick_type(); ii++) {
			actionJoystickType[ii] = new Action_Control(this, using_flags);
			actionGroup_JoystickType->addAction(actionJoystickType[ii]);
			actionJoystickType[ii]->setCheckable(true);
			actionJoystickType[ii]->setVisible(true);
			actionJoystickType[ii]->binds->setValue1(ii);
			if(p_config->joystick_type == ii) actionJoystickType[ii]->setChecked(true);
			menuJoystickType->addAction(actionJoystickType[ii]);
			connect(actionJoystickType[ii], SIGNAL(triggered()),
				actionJoystickType[ii]->binds, SLOT(do_set_joystick_type()));
			connect(actionJoystickType[ii]->binds, SIGNAL(sig_joystick_type(int)),
				this, SLOT(set_joystick_type(int)));
		}
	}
}

void Ui_MainWindowBase::ConfigKeyboardType(void)
{
	if(using_flags->get_use_keyboard_type() > 0) {
		int ii;
		menuKeyboardType = new QMenu(menuMachine);
		menuKeyboardType->setObjectName(QString::fromUtf8("menuKeyboardType"));
		menuMachine->addAction(menuKeyboardType->menuAction());
		menuKeyboardType->setToolTipsVisible(true);
      
		actionGroup_KeyboardType = new QActionGroup(this);
		actionGroup_KeyboardType->setExclusive(true);
		for(ii = 0; ii < using_flags->get_use_keyboard_type(); ii++) {
			actionKeyboardType[ii] = new Action_Control(this, using_flags);
			actionGroup_KeyboardType->addAction(actionKeyboardType[ii]);
			actionKeyboardType[ii]->setCheckable(true);
			actionKeyboardType[ii]->setVisible(true);
			actionKeyboardType[ii]->binds->setValue1(ii);
			if(p_config->keyboard_type == ii) actionKeyboardType[ii]->setChecked(true);
			menuKeyboardType->addAction(actionKeyboardType[ii]);
			connect(actionKeyboardType[ii], SIGNAL(triggered()),
				actionKeyboardType[ii]->binds, SLOT(do_set_keyboard_type()));
			connect(actionKeyboardType[ii]->binds, SIGNAL(sig_keyboard_type(int)),
				this, SLOT(set_keyboard_type(int)));
		}
	}
}

void Ui_MainWindowBase::ConfigMouseType(void)
{
	if(using_flags->get_use_mouse_type() > 0) {
		int ii;
		menuMouseType = new QMenu(menuMachine);
		menuMouseType->setObjectName(QString::fromUtf8("menuMouseType"));
		menuMachine->addAction(menuMouseType->menuAction());
		menuMouseType->setToolTipsVisible(true);
      
		actionGroup_MouseType = new QActionGroup(this);
		actionGroup_MouseType->setExclusive(true);
		for(ii = 0; ii < using_flags->get_use_mouse_type(); ii++) {
			actionMouseType[ii] = new Action_Control(this, using_flags);
			actionGroup_MouseType->addAction(actionMouseType[ii]);
			actionMouseType[ii]->setCheckable(true);
			actionMouseType[ii]->setVisible(true);
			actionMouseType[ii]->binds->setValue1(ii);
			if(p_config->mouse_type == ii) actionMouseType[ii]->setChecked(true);
			menuMouseType->addAction(actionMouseType[ii]);
			connect(actionMouseType[ii], SIGNAL(triggered()),
				actionMouseType[ii]->binds, SLOT(do_set_mouse_type()));
			connect(actionMouseType[ii]->binds, SIGNAL(sig_mouse_type(int)),
				this, SLOT(set_mouse_type(int)));
		}
	}
}

void Ui_MainWindowBase::ConfigDriveType(void)
{
	int i;
	if(using_flags->get_use_drive_type() > 0) {
		menuDriveType = new QMenu(menuMachine);
		menuDriveType->setObjectName(QString::fromUtf8("menu_DriveType"));
		menuDriveType->setToolTipsVisible(true);
		
		actionGroup_DriveType = new QActionGroup(this);
		actionGroup_DriveType->setObjectName(QString::fromUtf8("actionGroup_DriveType"));
		actionGroup_DriveType->setExclusive(true);
		menuMachine->addAction(menuDriveType->menuAction());
		for(i = 0; i < using_flags->get_use_drive_type(); i++) {
			actionDriveType[i] = new Action_Control(this, using_flags);
			actionDriveType[i]->setCheckable(true);
			actionDriveType[i]->setVisible(true);
			actionDriveType[i]->binds->setValue1(i);
			if(i == p_config->drive_type) actionDriveType[i]->setChecked(true); // Need to write configure
			actionGroup_DriveType->addAction(actionDriveType[i]);
			menuDriveType->addAction(actionDriveType[i]);
			connect(actionDriveType[i], SIGNAL(triggered()),
					actionDriveType[i]->binds, SLOT(do_set_drive_type()));
			connect(actionDriveType[i]->binds, SIGNAL(sig_drive_type(int)),
					this, SLOT(set_drive_type(int)));
		}
	}
}

void Ui_MainWindowBase::ConfigSoundDeviceType(void)
{
	if(using_flags->get_use_sound_device_type() > 0) {
		int i;
		QString tmps;
		menuSoundDevice = new QMenu(menuMachine);
		menuSoundDevice->setObjectName(QString::fromUtf8("menu_SoundDevice"));
		menuSoundDevice->setToolTipsVisible(true);
		actionGroup_SoundDevice = new QActionGroup(this);
		actionGroup_SoundDevice->setObjectName(QString::fromUtf8("actionGroup_SoundDevice"));
		actionGroup_SoundDevice->setExclusive(true);
		menuMachine->addAction(menuSoundDevice->menuAction());   
		for(i = 0; i < using_flags->get_use_sound_device_type(); i++) {
			actionSoundDevice[i] = new Action_Control(this, using_flags);
			actionSoundDevice[i]->setCheckable(true);
			actionSoundDevice[i]->binds->setValue1(i);
			if(i == p_config->sound_type) actionSoundDevice[i]->setChecked(true); // Need to write configure
			tmps = QString::fromUtf8("actionSoundDevice_");
			actionSoundDevice[i]->setObjectName(tmps + QString::number(i));
			menuSoundDevice->addAction(actionSoundDevice[i]);
			actionGroup_SoundDevice->addAction(actionSoundDevice[i]);
			connect(actionSoundDevice[i], SIGNAL(triggered()),
					actionSoundDevice[i]->binds, SLOT(do_set_sound_device()));
			connect(actionSoundDevice[i]->binds, SIGNAL(sig_sound_device(int)),
					this, SLOT(set_sound_device(int)));
		}
	}
}

void Ui_MainWindowBase::ConfigPrinterType(void)
{
	if(using_flags->is_use_printer()) {
		int i;
		QString tmps;
		int ilim = 2;
		if(using_flags->get_use_printer_type() > 0) ilim = using_flags->get_use_printer_type();
		menuPrintDevice = new QMenu(menuMachine);
		menuPrintDevice->setObjectName(QString::fromUtf8("menu_PrintDevice"));
		menuPrintDevice->setToolTipsVisible(true);
		
		actionGroup_PrintDevice = new QActionGroup(this);
		actionGroup_PrintDevice->setObjectName(QString::fromUtf8("actionGroup_PrintDevice"));
		actionGroup_PrintDevice->setExclusive(true);
		menuMachine->addAction(menuPrintDevice->menuAction());

		for(i = 0; i < ilim; i++) {
			actionPrintDevice[i] = new Action_Control(this, using_flags);
			actionPrintDevice[i]->setCheckable(true);
			actionPrintDevice[i]->binds->setValue1(i);
			if(i == p_config->printer_type) actionPrintDevice[i]->setChecked(true); // Need to write configure
			tmps = QString::fromUtf8("actionPrintDevice_");
			actionPrintDevice[i]->setObjectName(tmps + QString::number(i));
			menuPrintDevice->addAction(actionPrintDevice[i]);
			actionGroup_PrintDevice->addAction(actionPrintDevice[i]);
			connect(actionPrintDevice[i], SIGNAL(triggered()),
					actionPrintDevice[i]->binds, SLOT(do_set_printer_device()));
			connect(actionPrintDevice[i]->binds, SIGNAL(sig_printer_device(int)),
					this, SLOT(set_printer_device(int)));
		}
	}
}

void Ui_MainWindowBase::set_printer_device(int p_type)
{
	// 0 = PRNFILE
	if(p_type < 0) p_type = 0; // OK?
	if(using_flags->get_use_printer_type() > 0) {
		if(p_type >= using_flags->get_use_printer_type()) {
			p_type = using_flags->get_use_printer_type() - 1;
		}
	} else {
		if(p_type >= 8) p_type = 0;
	}
	p_config->printer_type = p_type;
	emit sig_emu_update_config();
}
