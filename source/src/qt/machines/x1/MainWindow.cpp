/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QtGui>
#include <QMenu>

#include "menuclasses.h"
#include "qt_main.h"
#include "menu_cart.h"

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("NMI Reset",  true);
	retranslateOpMenuZ80(true);

	retranslateFloppyMenu(0, 0, QApplication::translate("MachineX1", "FDD", 0));
	retranslateFloppyMenu(1, 1, QApplication::translate("MachineX1", "FDD", 0));

	actionSpecial_Reset[0]->setText(QApplication::translate("Machine", "NMI Reset", 0));
	actionSpecial_Reset[0]->setToolTip(QApplication::translate("MachineX1", "Do NMI reset.", 0));

	// Set Labels
	menuSoundDevice->setTitle(QApplication::translate("MachineX1", "Sound Device", 0));
	actionSoundDevice[0]->setText(QApplication::translate("MachineX1", "PSG", 0));
	actionSoundDevice[1]->setText(QApplication::translate("MachineX1", "CZ-8BS1 Single", 0));
	actionSoundDevice[2]->setText(QApplication::translate("MachineX1", "CZ-8BS1 Double", 0));

#if defined(_X1)
	menuMonitorType->deleteLater();
	//menuMonitorType->setVisible(false);
	//actionMonitorType[0]->setVisible(false);
	//actionMonitorType[1]->setVisible(false);
#else
	actionMonitorType[0]->setText(QApplication::translate("MachineX1", "High Resolution (400line)", 0));
	actionMonitorType[1]->setText(QApplication::translate("MachineX1", "Standarsd Resolution (200line)", 0));
#endif
#if defined(USE_KEYBOARD_TYPE)
	menuKeyboardType->setTitle(QApplication::translate("MachineX1", "Keyboard Mode", 0));
	actionKeyboardType[0]->setText(QApplication::translate("MachineX1", "Mode A", 0));
	actionKeyboardType[1]->setText(QApplication::translate("MachineX1", "Mode B", 0));
#endif
#if defined(USE_JOYSTICK_TYPE)
	menuJoystickType->setTitle(QApplication::translate("MachineX1", "Joy PAD Type", 0));
	actionJoystickType[0]->setText(QApplication::translate("MachineX1", "2-Buttons Joy Pad", 0));
	actionJoystickType[1]->setText(QApplication::translate("MachineX1", "6-Buttons Joy Pad", 0));
	actionJoystickType[2]->setText(QApplication::translate("MachineX1", "2-Buttons with Multi-Tap", 0));
	actionJoystickType[3]->setText(QApplication::translate("MachineX1", "6-Buttons with Multi-Tap", 0));
#endif
#if defined(USE_DRIVE_TYPE)
	menuDriveType->setTitle(QApplication::translate("MachineX1", "Floppy Type", 0));
	actionDriveType[0]->setText(QApplication::translate("MachineX1", "2D", 0));
	actionDriveType[1]->setText(QApplication::translate("MachineX1", "2DD", 0));
	actionDriveType[2]->setText(QApplication::translate("MachineX1", "2HD", 0));
#endif
#ifdef USE_DEBUGGER
	actionDebugger[3]->setVisible(false);

	actionDebugger[0]->setText(QApplication::translate("MachineX1", "Main CPU", 0));
	actionDebugger[1]->setText(QApplication::translate("MachineX1", "Sub CPU", 0));
	actionDebugger[2]->setText(QApplication::translate("MachineX1", "Keyboard CPU", 0));
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
	actionDebugger[2]->setVisible(true);
#ifdef _X1TWIN
	actionDebugger[3]->setText(QApplication::translate("MachineX1", "PC-ENGINE CPU", 0));
	actionDebugger[3]->setVisible(true);
#endif
#endif
#if defined(USE_PRINTER)
	actionPrintDevice[1]->setText(QString::fromUtf8("Sharp MZ-1P17"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineX1", "Sharp MZ-1P17 kanji thermal printer.", 0));
	actionPrintDevice[1]->setEnabled(true);

	actionPrintDevice[2]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MachineX1", "NEC PC-PR201 kanji serial printer.", 0));
	actionPrintDevice[2]->setEnabled(false);

	actionPrintDevice[3]->setText(QString::fromUtf8("JAST SOUND"));
	actionPrintDevice[3]->setToolTip(QApplication::translate("MachineX1", "Use JAST SOUND : PCM sound unit.", 0));
	actionPrintDevice[3]->setEnabled(true);
#endif
#if defined(_X1TWIN)
	menu_Cart[0]->setTitle(QApplication::translate("MachineX1", "HuCARD", 0));
#endif

} // retranslateUi



META_MainWindow::META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE
