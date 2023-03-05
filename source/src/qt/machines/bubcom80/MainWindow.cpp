/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for Buncom80 .
 * (C) 2018 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * July 13, 2018 : Initial, split from FM-7.
 */

#include <QApplication>
#include <QtCore/QVariant>
#include <QtGui>
#include <QMenu>

#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"
#include "sound_dialog.h"

#include "menu_disk.h"
//QT_BEGIN_NAMESPACE

extern config_t config;

void META_MainWindow::retranslateUi(void)
{

	Ui_MainWindowBase::retranslateUi();

	retranslateControlMenu("", false);

	retranslateFloppyMenu(0, 1, QString::fromUtf8("FD"));
	retranslateFloppyMenu(1, 2, QString::fromUtf8("FD"));
	retranslateFloppyMenu(2, 3, QString::fromUtf8("FD"));
	retranslateFloppyMenu(3, 4, QString::fromUtf8("FD"));

	retranslateCMTMenu(0);

	for(int _drv = 0; _drv < USE_BUBBLE; _drv++) {
		retranslateBubbleMenu(_drv, _drv + 1);
	}

	actionPrintDevice[1]->setText(QApplication::translate("Machine", "BC-861/862/863", 0));
	actionPrintDevice[1]->setToolTip(QApplication::translate("Machine", "Use printer BC-861/862/863.", 0));
	actionPrintDevice[1]->setEnabled(false);
	retranslateOpMenuZ80(true);

#ifdef USE_DEBUGGER
	actionDebugger[0]->setText(QApplication::translate("Machine", "Main CPU", 0));
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
#endif
	//	actionStart_Record_Movie->setText(QApplication::translate("Machine", "Start Record Movie", 0));
	//      actionStop_Record_Movie->setText(QApplication::translate("Machine", "Stop Record Movie", 0));
	//
	// FM-7 Specified

#if defined(USE_MONITOR_TYPE) && defined(USE_GREEN_DISPLAY)
	actionMonitorType[0]->setText(QApplication::translate("Machine", "Color Display (need reset)", 0));
	actionMonitorType[0]->setToolTip(QApplication::translate("Machine", "Using color display.\nChanges will be applied at reset, not immediately.", 0));
	actionMonitorType[1]->setText(QApplication::translate("Machine", "Green Display (need reset)", 0));
	actionMonitorType[1]->setToolTip(QApplication::translate("Machine", "Using ancient \"Green Display\" to display.\nChanges will be applied at reset, not immediately.", 0));
#endif
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
//	int i;
//	uint32_t skip;
//	ConfigCPUTypes(1);
//	ConfigCPUBootMode(USE_BOOT_MODE);
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
