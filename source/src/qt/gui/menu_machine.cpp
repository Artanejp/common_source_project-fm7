/*
 * Common Source code Project:
 * Ui->Qt->gui->menu_machine ; Template of machine menu.
 * (C) 2019 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Aug 13, 2019 : Split from menu_main.cpp .
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QIcon>
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMenu>
#include <QMenuBar>
#include <QStyle>

#include "mainwidget_base.h"

#include "qt_gldraw.h"
//#include "emu.h"
#include "qt_main.h"
#include "menu_flags.h"
#include "csp_logger.h"
#include "common.h"

// You can Override this function: Re-define on foo/MainWindow.cpp.
// This code is example: by X1(TurboZ).
void Ui_MainWindowBase::retranslateMachineMenu(void)
{
	int i;
	QString tmps;
	QString tmps2;
	menuMachine->setTitle(QApplication::translate("MenuMachine", "Machine", 0));
	if(using_flags->is_use_ram_size()) {
		action_RAMSize->setText(QApplication::translate("MenuMachine", "RAM Size", 0));
		action_RAMSize->setToolTip(QApplication::translate("MenuMachine", "Set (extra) memory size.\nThis will effect after restarting this emulator.", 0));
	}	
	if(using_flags->get_use_device_type() > 0) {
		menuDeviceType->setTitle(QApplication::translate("MenuMachine", "Device Type", 0));
		for(i = 0; i < using_flags->get_use_device_type(); i++) {
			tmps2.setNum(i + 1);
			tmps = QString::fromUtf8("Machine Device ") + tmps2;
			actionDeviceType[i]->setText(tmps); 
		}
	}
	if(using_flags->get_use_sound_device_type() > 0) {
		menuSoundDevice->setTitle(QApplication::translate("MenuMachine", "Sound Cards", 0));
		for(i = 0; i < using_flags->get_use_sound_device_type(); i++) {
			tmps2.setNum(i + 1);
			tmps = QString::fromUtf8("Sound Device ") + tmps2;
			actionSoundDevice[i]->setText(tmps); 
		}
	}
	if(using_flags->get_use_drive_type() > 0) {
		menuDriveType->setTitle(QApplication::translate("MenuMachine", "Drive Type", 0));
		for(i = 0; i < using_flags->get_use_drive_type(); i++) {
			tmps2.setNum(i + 1);
			tmps = QString::fromUtf8("Drive Type ") + tmps2;
			actionDriveType[i]->setText(tmps); 
		}
	}
	if(using_flags->is_use_printer()) {
		menuPrintDevice->setTitle(QApplication::translate("MenuMachine", "Printer (Need RESET)", 0));
		i = 1;
		actionPrintDevice[0]->setText(QApplication::translate("MenuMachine", "Dump to File", 0));
		actionPrintDevice[0]->setToolTip(QApplication::translate("MenuMachine", "Dump printer output to file.\nMaybe output only ascii text.", 0));
		if(using_flags->get_use_printer_type() > 0) {
			for(i = 1; i < (using_flags->get_use_printer_type() - 1); i++) {
				tmps2.setNum(i + 1);
				tmps = QApplication::translate("MenuMachine", "Printer", 0) + tmps2;
				actionPrintDevice[i]->setText(tmps); 
				actionPrintDevice[i]->setToolTip(tmps); 
			}
		}
		actionPrintDevice[i]->setText(QApplication::translate("MenuMachine", "Not Connect", 0));
		actionPrintDevice[i]->setToolTip(QApplication::translate("MenuMachine", "None devices connect to printer port.", 0));
	}
	if(using_flags->is_use_serial()) {
		menuSerialDevice->setTitle(QApplication::translate("MenuMachine", "Serial (Need RESET)", 0));
		QString _stdnames[3] = {
			QApplication::translate("MenuMachine", "Physical PORT", 0),
			QApplication::translate("MenuMachine", "Named Pipe", 0),
			QApplication::translate("MenuMachine", "MIDI Device", 0),
		};
		QString _stdtooltips[3] = {
			QApplication::translate("MenuMachine", "Use physical (HOST DEFINED) serial port.\nWill implement to available.", 0),
			QApplication::translate("MenuMachine", "Use HOST named pipe (a.k.a. FIFO FILE) as serial device.\nWill implement to available.", 0),
			QApplication::translate("MenuMachine", "Connect to  MIDI (pseudo) device.\nWill implement to available.", 0),
		};
		if(using_flags->get_use_printer_type() > 0) {
			for(i = 0; i < 3; i++) {
				if(actionSerialDevice[i] != nullptr) {
					tmps2.setNum(i + 1);
					actionSerialDevice[i]->setText(_stdnames[i]);
					actionSerialDevice[i]->setToolTip(_stdtooltips[i]);
					actionSerialDevice[i]->setObjectName(QString::fromUtf8("actionSerialDevice") + tmps2);
				}
			}
			for(i = 3; i < (using_flags->get_use_printer_type() - 1); i++) {
				if(actionSerialDevice[i] != nullptr) {
					tmps2.setNum(i + 1);
					tmps = QApplication::translate("MenuMachine", "Serial", 0) + tmps2;
					actionSerialDevice[i]->setText(tmps); 
					actionSerialDevice[i]->setToolTip(tmps);
					actionSerialDevice[i]->setObjectName(QString::fromUtf8("actionSerialDevice") + tmps2);
				}
			}
			if(actionSerialDevice[i] != nullptr) {
				tmps2.setNum(i + 1);
				actionSerialDevice[i]->setText(QApplication::translate("MenuMachine", "Not Connect", 0));
				actionSerialDevice[i]->setToolTip(QApplication::translate("MenuMachine", "None devices connect to serial port.", 0));
				actionSerialDevice[i]->setObjectName(QString::fromUtf8("actionSerialDevice") + tmps2);
			}
		}
	}
	if(using_flags->get_use_monitor_type() > 0) {
		menuMonitorType->setTitle(QApplication::translate("MenuMachine", "Monitor Type", 0));
		menuMonitorType->setToolTipsVisible(true);
		for(int ii = 0; ii < using_flags->get_use_monitor_type(); ii++) {
			tmps = QString::fromUtf8("Monitor %1").arg(ii + 1);
			actionMonitorType[ii]->setText(tmps);
		}
	}
}

void Ui_MainWindowBase::ConfigMonitorType(void)
{
	if(using_flags->get_use_monitor_type() > 0) {
		int ii;
		menuMonitorType = new QMenu(menuMachine);
		menuMonitorType->setObjectName(QString::fromUtf8("menuControl_MonitorType"));
		menuMachine->addAction(menuMonitorType->menuAction());
		
		actionGroup_MonitorType = new QActionGroup(this);
		actionGroup_MonitorType->setExclusive(true);
		for(ii = 0; ii < using_flags->get_use_monitor_type(); ii++) {
			SET_ACTION_NUMERIC_CONNECT(actionMonitorType[ii], ii, p_config->monitor_type,  SIGNAL(triggered()), SLOT(set_monitor_type()));
			
			actionGroup_MonitorType->addAction(actionMonitorType[ii]);
			menuMonitorType->addAction(actionMonitorType[ii]);
		}
	}

}
