/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QVariant>
#include <QtGui>
#include <QApplication>
#include <QMenu>

#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE


void META_MainWindow::setupUI_Emu(void)
{
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(" ",  false);
	retranslateOpMenuZ80(true);

#if defined(MSX_PSG_STEREO)
	actionSoundDevice[0]->setText(QApplication::translate("MachineMSX", "PSG Mono", 0));
	actionSoundDevice[0]->setToolTip(QApplication::translate("MachineMSX", "Use PSG as monoral.\nCompatibility for generic MSX.", 0));
	actionSoundDevice[1]->setText(QApplication::translate("MachineMSX", "PSG Stereo", 0));
	actionSoundDevice[1]->setToolTip(QApplication::translate("MachineMSX", "Use PSG as stereo.\nHX-20's special feature.", 0));
#endif
#if defined(USE_PRINTER_TYPE)
	actionPrintDevice[1]->setText(QApplication::translate("MachineMSX", "MSX PRINTER", 0));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineMSX", "Use MSX spec. printer.", 0));
	actionPrintDevice[2]->setText(QApplication::translate("MachineMSX", "PC-PR201", 0));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MachineMSX", "Use NEC PC-PR201 printer.", 0));
	actionPrintDevice[1]->setEnabled(false);
	actionPrintDevice[2]->setEnabled(false);
#endif
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
	// Set Labels
} // retranslateUi

META_MainWindow::META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE
