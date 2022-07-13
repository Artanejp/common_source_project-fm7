/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QMenu>
#include <QActionGroup>

#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

void META_MainWindow::setupUI_Emu(void)
{
	QString tmps;
	menuMachine->setVisible(true);
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(action_Emu_DipSw, 0x01,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	
	tmps = QString::fromUtf8("actionEmu_DipSw0");
	action_Emu_DipSw->setObjectName(tmps);
	
	menuMachine->addAction(action_Emu_DipSw);
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(" ",  false);
#if defined(_MZ80A) || defined(_MZ80K)	
	action_Emu_DipSw->setText(QApplication::translate("MachineMZ80K", "PCG-8000", 0));
	action_Emu_DipSw->setToolTip(QApplication::translate("MachineMZ80K", "HAL laboratory PCG-8000 PCG.", 0));
#elif defined(_MZ1200)
	action_Emu_DipSw->setText(QApplication::translate("MachineMZ80K", "PCG-1200", 0));
	action_Emu_DipSw->setToolTip(QApplication::translate("MachineMZ80K", "HAL laboratory PCG-1200 PCG.", 0));
#endif

#if defined(_MZ1200) || defined(_MZ80K) || defined(_MZ80C)
	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-80P3"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineMZ80K", "Sharp MZ-80P3 thermal printer.", 0));
#elif defined(_MZ80A)
	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-80P4"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineMZ80K", "Sharp MZ-80P4 thermal printer.", 0));
//#else
//	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-1P17"));
//	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineMZ80K", "Sharp MZ-1P17 kanji thermal printer.", 0));
#endif
	actionPrintDevice[2]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MachineMZ80K", "NEC PC-PR201 printer.", 0));
	actionPrintDevice[2]->setEnabled(false);
	
	this->setWindowTitle(QApplication::translate("MachineMZ80K", "MachineMZ80K", 0));
#if defined(USE_DRIVE_TYPE)
	menuDriveType->setTitle(QApplication::translate("MachineMZ80K", "Floppy Type", 0));
	actionDriveType[0]->setText(QApplication::translate("MachineMZ80K", "2D", 0));
	actionDriveType[1]->setText(QApplication::translate("MachineMZ80K", "2DD", 0));
#endif
#if defined(USE_MONITOR_TYPE)
	actionMonitorType[0]->setText(QApplication::translate("MachineMZ80K", "WHITE (MZ-80K)", 0));
	actionMonitorType[0]->setToolTip(QApplication::translate("MachineMZ80K", "Using MZ-80K's B&W DISPLAY.", 0));
	actionMonitorType[1]->setText(QApplication::translate("MachineMZ80K", "GREEN (MZ-80C)", 0));
	actionMonitorType[1]->setToolTip(QApplication::translate("MachineMZ80K", "Using MZ-80C's GREEN DISPLAY.", 0));
#endif	

#ifdef USE_DEBUGGER
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



