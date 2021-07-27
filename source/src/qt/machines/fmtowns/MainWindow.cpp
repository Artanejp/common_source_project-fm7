/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for Babbage2nd .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */
#include <QApplication>
#include <QtCore/QVariant>
#include <QtGui>
#include "menuclasses.h"
#include "commonclasses.h"

#include "emu.h"
#include "qt_main.h"
#include "../../vm/fmtowns/fmtowns.h"
#include "menu_binary.h"
#include "menu_cart.h"

//QT_BEGIN_NAMESPACE


void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("Reset with CD", true);
	actionSpecial_Reset[0]->setText(QApplication::translate("Machine", "Reset with CD", 0));
	actionSpecial_Reset[0]->setToolTip(QApplication::translate("Machine", "Reset and boot from CD-ROM", 0));
	for(int i = 1; i < 5; i++) {
		QString tmps2, tmps3;
		QString numname = QString::number(i - 1);
		tmps2 = QApplication::translate("Machine", "Reset with F%1", 0).arg(numname);
		tmps3 = QApplication::translate("Machine", "Reset and boot from FLOPPY #%1", 0).arg(numname);
		actionSpecial_Reset[i]->setText(tmps2);
		actionSpecial_Reset[i]->setToolTip(tmps3);
	}
	for(int i = 5; i < 10; i++) {
		QString tmps2, tmps3;
		QString numname = QString::number(i - 5);
		tmps2 = QApplication::translate("Machine", "Reset with H%1", 0).arg(numname);
		tmps3 = QApplication::translate("Machine", "Reset and boot from HDD #%1", 0).arg(numname);
		actionSpecial_Reset[i]->setText(tmps2);
		actionSpecial_Reset[i]->setToolTip(tmps3);
	}
	actionSpecial_Reset[10]->setText(QApplication::translate("Machine", "Reset with ICM", 0));
	actionSpecial_Reset[10]->setToolTip(QApplication::translate("Machine", "Reset with boot from IC CARD #0", 0));
	actionSpecial_Reset[11]->setText(QApplication::translate("Machine", "Reset with DEBUG", 0));
	actionSpecial_Reset[11]->setToolTip(QApplication::translate("Machine", "Reset with DEBUGGING MODE", 0));

	menuMAchineFeatures[0]->setTitle(QApplication::translate("Machine", "Joystick Port #1", 0));
	menuMAchineFeatures[1]->setTitle(QApplication::translate("Machine", "Joystick Port #2", 0));
#if defined(USE_MACHINE_FEATURES)
	for(int i = 0; i < 2; i++) {
		actionJOYPORT_TYPE[i][0]->setText(QApplication::translate("Machine", "none", 0));
		actionJOYPORT_TYPE[i][1]->setText(QApplication::translate("Machine", "2Buttons PAD", 0));
		actionJOYPORT_TYPE[i][2]->setText(QApplication::translate("Machine", "6Buttons PAD", 0));
		actionJOYPORT_TYPE[i][3]->setText(QApplication::translate("Machine", "Towns Mouse", 0));
		actionJOYPORT_TYPE[i][4]->setText(QApplication::translate("Machine", "Analog Stick", 0));
		actionJOYPORT_TYPE[i][5]->setText(QApplication::translate("Machine", "Joystick for Libble Rabble", 0));
		for(int j = 0; j < 4; j++) {
			actionJOYPORT_TYPE[i][j]->setEnabled(true);
		}
		for(int j = 4; j < 6; j++) {
			actionJOYPORT_TYPE[i][j]->setEnabled(false);
		}

		actionJOYPORT_TYPE[i][0]->setToolTip(QApplication::translate("Machine", "No devices are connected to this port", 0));
		actionJOYPORT_TYPE[i][1]->setToolTip(QApplication::translate("Machine", "Connect standard Towns PAD, 2Buttons.", 0));
		actionJOYPORT_TYPE[i][2]->setToolTip(QApplication::translate("Machine", "Connect extended Towns PAD, 6Buttons.", 0));
		actionJOYPORT_TYPE[i][3]->setToolTip(QApplication::translate("Machine", "Connect MOUSE", 0));
		actionJOYPORT_TYPE[i][4]->setToolTip(QApplication::translate("Machine", "Connect analog joystick, this still be unimplemented", 0));
		actionJOYPORT_TYPE[i][5]->setToolTip(QApplication::translate("Machine", "Connect hacked joystick for Libble Rabble, this still be unimplemented.", 0));
		
	}
#endif
										   
#if defined(USE_CART)
	if(menu_Cart[0] != NULL) {
		menu_Cart[0]->setTitle(QApplication::translate("MainWindow", "IC1", 0));
	}
	if(menu_Cart[1] != NULL) {
		menu_Cart[1]->setTitle(QApplication::translate("MainWindow", "IC2", 0));
	}
#endif
//	menu_BINs[0]->setTitle(QApplication::translate("MenuBABBAGE", "RAM", 0));
	//menuMachine->setVisible(false);
	// Set Labels
	
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
#if defined(USE_MACHINE_FEATURES)
	// menuMachineFeatures[0] : Joystick Port #1
	// menuMachineFeatures[1] : Joystick Port #2
	// menuMachineFeatures[2] : Reserved
	// menuMachineFeatures[3] : Reserved
	// menuMachineFeatures[4] : Reserved
	// menuMachineFeatures[5] : Reserved
	for(int i = 0; i < 2; i++) {
		actionGroup_JOYPortType[i] = new QActionGroup(this);
		actionGroup_JOYPortType[i]->setExclusive(true);
		actionGroup_JOYPortType[i]->setObjectName(QString("actionGroupJOYPort%1").arg(i + 1));
		menuMachineFeatures[i]->addAction(actionGroup_JOYPortType[i]);
		for(int j = 0; j < 8; j++) {
			actionJOYPORT_TYPE[i][j] = new Action_Control(this, using_flags);
			actionJOYPORT_TYPE[i][j]->setCheckable(true);
			actionJOYPORT_TYPE[i][j]->setVisible(false);
			if(p_config->machine_features[i] == (uint32_t)j) {
				actionJOYPORT_TYPE[i][j]->setChecked(true);
			} else {
				actionJOYPORT_TYPE[i][j]->setChecked(false);
			}
			actionJOYPORT_TYPE[i][j]->binds->setNumber(i);
			actionJOYPORT_TYPE[i][j]->binds->setValue1(j);
			actionGroup_JOYPortType[i]->addAction(actionJOYPORT_TYPE[i][j]);
			connect(actionJOYPORT_TYPE[i][j], SIGNAL(triggered()),
					actionJOYPORT_TYPE[i][j]->binds, SLOT(do_select_machine_feature_single()));
			connect(actionJOYPORT_TYPE[i][j]->binds,
					SIGNAL(sig_set_machine_feature(int, uint32_t)),
					this, SLOT(do_set_machine_feature(int, uint32_t)));
		}
		for(int j = 0; j < 6; j++) {
			actionJOYPORT_TYPE[i][j]->setVisible(true);
		}
	}
	for(int i = 2; i < 6; i++) {
		menuMachineFeatures[i]->setVisible(false);
		menuMachineFeatures[i]->setEnabled(false);
	}		
#endif
}


META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



