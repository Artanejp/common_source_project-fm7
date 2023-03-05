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

#include "./menuclasses.h"
#include "emu.h"
#include "qt_main.h"
#include "menu_binary.h"

//QT_BEGIN_NAMESPACE

void META_MainWindow::retranslateUi(void)
{

	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("", false);
	retranslateOpMenuZ80(true);

	actionAddress8000->setText(QApplication::translate("MenuSMB80", "SW: ADRS SW 8000", 0));
	actionAddress8000->setToolTip(QApplication::translate("MenuSMB80", "SW: ADRS SW 8000", 0));

	menuAddrBase->setTitle(QApplication::translate("MenuSMB80", "Display Base Address", 0));
	menuAddrBase->setToolTip(QApplication::translate("MenuSMB80", "Set display base address on RAM.", 0));

	actionAddressBase[0]->setText(QApplication::translate("MenuSMB80", "$000", 0));
	actionAddressBase[1]->setText(QApplication::translate("MenuSMB80", "$200", 0));
	actionAddressBase[2]->setText(QApplication::translate("MenuSMB80", "$400", 0));
	actionAddressBase[3]->setText(QApplication::translate("MenuSMB80", "$600", 0));

	menu_BINs[0]->setTitle(QApplication::translate("MenuSMB80", "RAM", 0));
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

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionAddress8000, 0x00000001, p_config->dipswitch , SIGNAL(toggled(bool)) , SLOT(do_set_single_dipswitdh(bool)));
	menuMachine->addAction(actionAddress8000);

	menuAddrBase = new QMenu(menuMachine);
	menuAddrBase->setObjectName(QString::fromUtf8("menuAddrBase"));
	menuMachine->addAction(menuAddrBase->menuAction());
	menuAddrBase->setToolTipsVisible(true);
	for(int i = 0; i < 4; i++) {
		SET_ACTION_DIPSWITCH_CONNECT(actionAddressBase[i], (((uint32_t)i) << 2), (0x03 << 2), p_config->dipswitch , SIGNAL(triggered()) ,SLOT(do_set_multi_dipswitch()));
		menuAddrBase->addAction(actionAddressBase[i]);
	}
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
