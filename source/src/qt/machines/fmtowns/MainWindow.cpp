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

#include "emu.h"
#include "qt_main.h"
#include "../../vm/fmtowns/fmtowns.h"
#include "../../vm/fmtowns/towns_common.h"

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

#if defined(USE_MACHINE_FEATURES)
	if(menuMachineFeatures[TOWNS_MACHINE_JOYPORT1] != nullptr) {
		menuMachineFeatures[TOWNS_MACHINE_JOYPORT1]->setTitle(QApplication::translate("Machine", "Joystick Port #1", 0));
	}
	if(menuMachineFeatures[TOWNS_MACHINE_JOYPORT2] != nullptr) {
		menuMachineFeatures[TOWNS_MACHINE_JOYPORT2]->setTitle(QApplication::translate("Machine", "Joystick Port #2", 0));
	}
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
	if(menuMachineFeatures[TOWNS_MACHINE_MIDI] != nullptr) {
		menuMachineFeatures[TOWNS_MACHINE_MIDI]->setTitle(QApplication::translate("Machine", "MIDI", 0));
	}
	for(int i = TOWNS_MACHINE_SIO0; i <= TOWNS_MACHINE_SIO3; i++) {
		menuMachineFeatures[i]->setTitle(QApplication::translate("Machine", "SIO%1", 0).arg(i - TOWNS_MACHINE_SIO0));
	}
	for(int i = TOWNS_MACHINE_LPT0_OUT; i <= TOWNS_MACHINE_LPT1_OUT; i++) {
		menuMachineFeatures[i]->setTitle(QApplication::translate("Machine", "LPT%1", 0).arg(i - TOWNS_MACHINE_LPT0_OUT));
	}
	#if USE_MACHINE_FEATURES >= TOWNS_MACHINE_FASTER_CLOCK
	/* Has clock settings */
	if(menuMachineFeatures[TOWNS_MACHINE_FASTER_CLOCK] != nullptr) {
		menuMachineFeatures[TOWNS_MACHINE_FASTER_CLOCK]->setTitle(QApplication::translate("Machine", "Fast mode", 0));
	}
	if(actionForceTo16MHz[0] != nullptr) {
		actionForceTo16MHz[0]->setText(QApplication::translate("Machine", "16MHz", 0));
		actionForceTo16MHz[0]->setToolTip(QApplication::translate("Machine", "Force to set CPU clock to 16MHz when faster mode.\nThis is useful when applications is too fast.", 0));
	}
	if(actionForceTo16MHz[1] != nullptr) {
		actionForceTo16MHz[1]->setText(QApplication::translate("Machine", "MAXIMUM", 0));
		actionForceTo16MHz[1]->setToolTip(QApplication::translate("Machine", "Set CPU clock to MAXIMUM when faster mode.\nThis may be native behavior.", 0));
	}
	if(actionForceTo16MHz[2] != nullptr) {
		actionForceTo16MHz[2]->setText(QApplication::translate("Machine", "User setting", 0));
		actionForceTo16MHz[2]->setToolTip(QApplication::translate("Machine", "Set faster CPU clock to user settings, when faster mode.\nThis limits native CPU clock of VM.", 0));
		actionForceTo16MHz[2]->setEnabled(false);
	}
	if(menuMachineFeatures[TOWNS_MACHINE_SET_MAX_CLOCK] != nullptr) {
		menuMachineFeatures[TOWNS_MACHINE_SET_MAX_CLOCK]->setTitle(QApplication::translate("Machine", "Set MAX Freq", 0));
	}
	
	#endif
	#if USE_MACHINE_FEATURES >= TOWNS_MACHINE_WITH_I386
	if(menuMachineFeatures[TOWNS_MACHINE_WITH_I386] != nullptr) {
	/* force to use i386SX / i386DX for any VMs. */
		menuMachineFeatures[TOWNS_MACHINE_WITH_I386]->setTitle(QApplication::translate("Machine", "CPU TYPE", 0));
	}
	#endif
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
	// menuMachineFeatures[2] : Midi
	// menuMachineFeatures[3] : SIO0
	// menuMachineFeatures[4] : SIO1
	// menuMachineFeatures[5] : SIO2
	// menuMachineFeatures[6] : SIO3
	// menuMachineFeatures[7] : LPT0
	// menuMachineFeatures[8] : LPT1
	// menuMachineFeatures[9] : Use faster clock.
	// menuMachineFeatures[10] : Set maximum clock.
	// menuMachineFeatures[11] : Force to use i386.
	
	for(int i = 0; i < 2; i++) {
		actionGroup_JOYPortType[i] = new QActionGroup(this);
		actionGroup_JOYPortType[i]->setExclusive(true);
		actionGroup_JOYPortType[i]->setObjectName(QString("actionGroupJOYPort%1").arg(i + 1));
		for(int j = 0; j < 8; j++) {
			SET_ACTION_MACHINE_FEATURE_CONNECT(actionJOYPORT_TYPE[i][j], i, (uint32_t)j, (p_config->machine_features[i] == (uint32_t)j), SIGNAL(triggered()), SLOT(do_set_machine_feature()));

			actionJOYPORT_TYPE[i][j]->setVisible((j < 6) ? true : false);
			
			actionGroup_JOYPortType[i]->addAction(actionJOYPORT_TYPE[i][j]);
			if(menuMachineFeatures[TOWNS_MACHINE_JOYPORT1 + i] != nullptr) {
				menuMachineFeatures[TOWNS_MACHINE_JOYPORT1 + i]->addAction(actionJOYPORT_TYPE[i][j]);
			}
		}
	}
	/* ToDo: Make UIs for MIDI, SIOs and LPTs. */

	if(menuMachineFeatures[TOWNS_MACHINE_MIDI] != nullptr) {
		menuMachineFeatures[TOWNS_MACHINE_MIDI]->setEnabled(false);
	}
	for(int i = TOWNS_MACHINE_SIO0; i <= TOWNS_MACHINE_SIO3; i++) {
		if(menuMachineFeatures[i] != nullptr) {
			menuMachineFeatures[i]->setEnabled(false);
		}
	}		
	for(int i = TOWNS_MACHINE_LPT0_OUT; i <= TOWNS_MACHINE_LPT1_OUT; i++) {
		if(menuMachineFeatures[i] != nullptr) {
			menuMachineFeatures[i]->setEnabled(false);
		}
	}
	
	#if USE_MACHINE_FEATURES >= TOWNS_MACHINE_FASTER_CLOCK
	/* Has clock settings */
	actionGroup_ForceTo16MHz = new QActionGroup(this);
	actionGroup_ForceTo16MHz->setExclusive(true);
	actionGroup_ForceTo16MHz->setObjectName(QString("actionGroupForceTo16MHz"));
	for(uint32_t i = 0; i < 3; i++) {
		SET_ACTION_MACHINE_FEATURE_CONNECT(actionForceTo16MHz[i], TOWNS_MACHINE_FASTER_CLOCK, i,
										   (p_config->machine_features[TOWNS_MACHINE_FASTER_CLOCK] == i),
										   SIGNAL(triggered()), SLOT(do_set_machine_feature()));
		actionForceTo16MHz[i]->setVisible(true);
		actionGroup_ForceTo16MHz->addAction(actionForceTo16MHz[i]);
		if(menuMachineFeatures[TOWNS_MACHINE_FASTER_CLOCK] != nullptr) {
			menuMachineFeatures[TOWNS_MACHINE_FASTER_CLOCK]->setVisible(true);
			menuMachineFeatures[TOWNS_MACHINE_FASTER_CLOCK]->addAction(actionForceTo16MHz[i]);
		}
	}
	if(menuMachineFeatures[TOWNS_MACHINE_SET_MAX_CLOCK] != nullptr) {
		menuMachineFeatures[TOWNS_MACHINE_SET_MAX_CLOCK]->setEnabled(false); // ToDo.
	}
	#endif
	#if USE_MACHINE_FEATURES >= TOWNS_MACHINE_WITH_I386
	/* force to use i386SX / i386DX for any VMs. */
	if(menuMachineFeatures[TOWNS_MACHINE_WITH_I386] != nullptr) {
		menuMachineFeatures[TOWNS_MACHINE_WITH_I386]->setEnabled(false); // ToDo
	}
	#endif

#endif
}


META_MainWindow::META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



