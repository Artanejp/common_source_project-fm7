/*
 * UI->Qt->MainWindow : Some Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */
#include <QApplication>
#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>
#include <QThread>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "qt_gldraw.h"

//#include "menuclasses.h"
#include "qt_dialogs.h"
//#include "csp_logger.h"


void Ui_MainWindowBase::do_set_machine_feature(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_MainWidgets::MachineFeaturePair pval = cp->data().value<CSP_Ui_MainWidgets::MachineFeaturePair>();

	int devnum = pval.devnum;
	uint32_t value = pval.value;
	if((devnum < 0) || (devnum >= using_flags->get_use_machine_features())) return;
	p_config->machine_features[devnum] = value;
	emit sig_emu_update_config();
}


void Ui_MainWindowBase::do_set_single_dipswitch(bool f)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_MainWidgets::DipSwitchPair pval = cp->data().value<CSP_Ui_MainWidgets::DipSwitchPair>();

	if(p_config == nullptr) return;

	uint32_t nval = (pval.data & pval.mask);
	p_config->dipswitch &= ~(pval.mask);
	if(f) { // ON
		p_config->dipswitch |= nval;
	}
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::do_set_multi_dipswitch()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_MainWidgets::DipSwitchPair pval = cp->data().value<CSP_Ui_MainWidgets::DipSwitchPair>();

	if(p_config == nullptr) return;

	uint32_t nval = (pval.data & pval.mask);
	p_config->dipswitch &= ~(pval.mask);
	p_config->dipswitch |= nval;

	emit sig_emu_update_config();
}

void Ui_MainWindowBase::do_block_task(void)
{
	emit sig_block_task();
}

void Ui_MainWindowBase::do_unblock_task(void)
{
	emit sig_unblock_task();
}

void Ui_MainWindowBase::do_start_emu_thread(void)
{
	emit sig_start_emu_thread();
	emit sig_emu_launched();
}

void Ui_MainWindowBase::do_start_draw_thread(void)
{
	emit sig_start_draw_thread();
}

void Ui_MainWindowBase::do_set_latency(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
	if((num < 0) || (num >= 8)) return;
	p_config->sound_latency = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::do_set_freq(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();

	if((num < 0) || (num >= 16)) return;
	
	p_config->sound_frequency = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::do_set_sound_device(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
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

void Ui_MainWindowBase::set_monitor_type(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
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

void Ui_MainWindowBase::do_set_screen_rotate(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int type = cp->data().value<int>();

	p_config->rotate_type = type;
	if(p_config->window_mode >= using_flags->get_screen_mode_num()) p_config->window_mode = using_flags->get_screen_mode_num() - 1;
	if(p_config->window_mode < 0) p_config->window_mode = 0;
	update_screen_size(p_config->window_mode);
}

void Ui_MainWindowBase::update_screen_size(int num)
{
	if(using_flags == nullptr) return;
	if((num < 0) || (num >= using_flags->get_screen_mode_num())) return;
	if(actionScreenSize[num] == nullptr) return;
	
	int w, h;
	double nd, ww, hh;
	double xzoom = using_flags->get_screen_x_zoom();
	double yzoom = using_flags->get_screen_y_zoom();

	struct CSP_Ui_MainWidgets::ScreenMultiplyPair s_mul;
	s_mul = actionScreenSize[num]->data().value<CSP_Ui_MainWidgets::ScreenMultiplyPair>();
	nd = s_mul.value;
	
	ww = (double)using_flags->get_screen_width();
	hh = (double)using_flags->get_screen_height();
	if((using_flags->get_screen_height_aspect() != using_flags->get_screen_height()) ||
	   (using_flags->get_screen_width_aspect() != using_flags->get_screen_width())) {
		double par_w = (double)using_flags->get_screen_width_aspect() / ww;
		double par_h = (double)using_flags->get_screen_height_aspect() / hh;
		//float par = par_h / par_w;
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
	set_screen_size(w, h);
	emit sig_screen_multiply(nd);
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

void Ui_MainWindowBase::set_mouse_type(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
	if((num >= using_flags->get_use_mouse_type()) && (num < 0)) return;
	p_config->mouse_type = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::set_device_type(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
	if((num >= using_flags->get_use_device_type()) && (num < 0)) return;
	p_config->device_type = num;
	emit sig_emu_update_config();

}

void Ui_MainWindowBase::set_keyboard_type(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
	if((num >= using_flags->get_use_keyboard_type()) && (num < 0)) return;
	p_config->keyboard_type = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::set_joystick_type(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
	if((num >= using_flags->get_use_joystick_type()) && (num < 0)) return;
	p_config->joystick_type = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::set_drive_type(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
	if((num >= using_flags->get_use_drive_type()) && (num < 0)) return;
	p_config->drive_type = num;
	emit sig_emu_update_config();
}
   

void Ui_MainWindowBase::set_screen_size(int w, int h)
{
	if((w <= 0) || (h <= 0)) return;
	if((p_config->rotate_type == 1) || (p_config->rotate_type == 3)) {
		std::swap(w, h);
		//emit sig_resize_osd(h);
	}
	emit sig_glv_set_fixed_size(w, h);
	resize_statusbar(w, h);
   
	MainWindow->centralWidget()->adjustSize();
	MainWindow->adjustSize();
}

void Ui_MainWindowBase::set_screen_aspect(int num)
{
	if(p_config == nullptr) return;
	if(using_flags == nullptr) return;
	if((num < 0) || (num >= 4)) return;
	// 0 = DOT
	// 1 = ASPECT(Scale X)
	// 2 = ASPECT(SCale Y)
	// 3 = ASPECT(Scale X,Y)
	p_config->window_stretch_type = num;
	
	if(using_flags->get_emu()) {
		int w, h, n;
		double nd, ww, hh;
		double xzoom = using_flags->get_screen_x_zoom();
		double yzoom = using_flags->get_screen_y_zoom();
		n = p_config->window_mode;
		if((n < 0) || (n >= using_flags->get_screen_mode_num())) return;
		if(actionScreenSize[n] == nullptr) return;
		
		struct CSP_Ui_MainWidgets::ScreenMultiplyPair s_mul;
		s_mul = actionScreenSize[n]->data().value<CSP_Ui_MainWidgets::ScreenMultiplyPair>();
		nd = s_mul.value;
		
		if(nd <= 0.0f) return;
		
		ww = (double)using_flags->get_screen_width();
		hh = (double)using_flags->get_screen_height();
		
		if((using_flags->get_screen_height_aspect() != using_flags->get_screen_height()) ||
		   (using_flags->get_screen_width_aspect() != using_flags->get_screen_width())) {
			double par_w = (double)using_flags->get_screen_width_aspect() / ww;
			double par_h = (double)using_flags->get_screen_height_aspect() / hh;
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
		set_screen_size(w, h);
	}
}

void Ui_MainWindowBase::do_set_screen_aspect(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	set_screen_aspect(num);
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
			actionDeviceType[ii]->setData(QVariant(ii));
			if(p_config->device_type == ii) actionDeviceType[ii]->setChecked(true);
			menuDeviceType->addAction(actionDeviceType[ii]);
			connect(actionDeviceType[ii], SIGNAL(triggered()),
				this, SLOT(set_device_type()));
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
			actionJoystickType[ii]->setData(QVariant(ii));
			if(p_config->joystick_type == ii) actionJoystickType[ii]->setChecked(true);
			menuJoystickType->addAction(actionJoystickType[ii]);
			connect(actionJoystickType[ii], SIGNAL(triggered()),
					this, SLOT(set_joystick_type()));
		}
	}
}

// Note: YOU MUST ADD OWN ACTION ENTRIES TO MENU; menuMachineFeatures[foo].
void Ui_MainWindowBase::ConfigMachineFeatures(void)
{
	for(int i = 0; i < using_flags->get_use_machine_features(); i++) {
		
		menuMachineFeatures[i] = new QMenu(menuMachine);
		menuMachineFeatures[i]->setObjectName(QString::fromUtf8("menuMachineFeatures"));
		menuMachine->addAction(menuMachineFeatures[i]->menuAction());
		menuMachineFeatures[i]->setToolTipsVisible(true);
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
			actionKeyboardType[ii]->setData(QVariant(ii));
			if(p_config->keyboard_type == ii) actionKeyboardType[ii]->setChecked(true);
			menuKeyboardType->addAction(actionKeyboardType[ii]);
			connect(actionKeyboardType[ii], SIGNAL(triggered()),
				this, SLOT(set_keyboard_type()));
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
			actionMouseType[ii]->setData(QVariant(ii));
			if(p_config->mouse_type == ii) actionMouseType[ii]->setChecked(true);
			menuMouseType->addAction(actionMouseType[ii]);
			connect(actionMouseType[ii], SIGNAL(triggered()),
				this, SLOT(set_mouse_type()));
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
			actionDriveType[i]->setData(QVariant(i));
			if(i == p_config->drive_type) actionDriveType[i]->setChecked(true); // Need to write configure
			actionGroup_DriveType->addAction(actionDriveType[i]);
			menuDriveType->addAction(actionDriveType[i]);
			connect(actionDriveType[i], SIGNAL(triggered()),
					this, SLOT(set_drive_type()));
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
			actionSoundDevice[i]->setData(QVariant(i));
			if(i == p_config->sound_type) actionSoundDevice[i]->setChecked(true); // Need to write configure
			tmps = QString::fromUtf8("actionSoundDevice_");
			actionSoundDevice[i]->setObjectName(tmps + QString::number(i));
			menuSoundDevice->addAction(actionSoundDevice[i]);
			actionGroup_SoundDevice->addAction(actionSoundDevice[i]);
			connect(actionSoundDevice[i], SIGNAL(triggered()),
					this, SLOT(do_set_sound_device()));
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
			actionPrintDevice[i]->setData(QVariant(i));
			if(i == p_config->printer_type) actionPrintDevice[i]->setChecked(true); // Need to write configure
			tmps = QString::fromUtf8("actionPrintDevice_");
			actionPrintDevice[i]->setObjectName(tmps + QString::number(i));
			menuPrintDevice->addAction(actionPrintDevice[i]);
			actionGroup_PrintDevice->addAction(actionPrintDevice[i]);
			connect(actionPrintDevice[i], SIGNAL(triggered()),
					this, SLOT(set_printer_device()));
		}
	}
}

void Ui_MainWindowBase::set_printer_device(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int p_type = cp->data().value<int>();
	
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

void Ui_MainWindowBase::setTextAndToolTip(QAction *p, QString text, QString tooltip)
{
	if(p == nullptr) return;
	p->setText(text);
	p->setToolTip(tooltip);
}


void Ui_MainWindowBase::setTextAndToolTip(QMenu *p, QString text, QString tooltip)
{
	if(p == nullptr) return;
//	p->setText(text);
	p->setTitle(text);
	p->setToolTip(tooltip);
}

