/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for PC-100 .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QVariant>
#include <QtGui>
#include "emu.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "qt_main.h"


void META_MainWindow::setupUI_Emu(void)
{
	int i;
	QString tmps;
}

void META_MainWindow::retranslateUi(void)
{
	int i;
   
	retranslateControlMenu("",  false);
	retranslateFloppyMenu(0, 1);
	retranslateFloppyMenu(1, 2);
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
	
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
#if defined(USE_DRIVE_TYPE)
	menuDriveType->setTitle(QApplication::translate("MachinePC100", "Floppy Type", 0));
	actionDriveType[0]->setText(QApplication::translate("MachinePC100", "2D", 0));
	actionDriveType[1]->setText(QApplication::translate("MachinePC100", "2DD", 0));
#endif
#if defined(USE_DEBUGGER)
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
	// Set Labels
} // retranslateUi



META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



