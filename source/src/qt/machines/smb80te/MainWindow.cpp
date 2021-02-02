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
extern config_t config;

Object_Menu_Control_SMB::Object_Menu_Control_SMB(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_SMB::~Object_Menu_Control_SMB()
{
}

void Object_Menu_Control_SMB::do_set_adrs_base()
{
	int val = getValue1();
	if((val >= 0) && (val < 4)) {
		uint32_t xval = ((uint32_t)val) << 2;
		config.dipswitch = (config.dipswitch & (~(3 << 2))) | xval;
	}
}

Action_Control_SMB::Action_Control_SMB(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	smb_binds = new Object_Menu_Control_SMB(parent, p);
	smb_binds->setValue1(0);
}

Action_Control_SMB::~Action_Control_SMB()
{
	delete smb_binds;
}



void META_MainWindow::do_set_adrs_8000(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | 0x00000001;
	} else {
		config.dipswitch = config.dipswitch & ~0x00000001;
	}
}

void META_MainWindow::retranslateUi(void)
{

	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("", false);
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
	actionAddress8000 = new QAction(this);
	actionAddress8000->setCheckable(true);
	actionAddress8000->setVisible(true);
	menuMachine->addAction(actionAddress8000);
	actionAddress8000->setChecked(((config.dipswitch & 0x00000001) != 0) ? true : false);
	connect(actionAddress8000, SIGNAL(toggled(bool)), this, SLOT(do_set_adrs_8000(bool)));

	menuAddrBase = new QMenu(menuMachine);
	menuAddrBase->setObjectName(QString::fromUtf8("menuAddrBase"));
	menuMachine->addAction(menuAddrBase->menuAction());
	menuAddrBase->setToolTipsVisible(true);
	for(int i = 0; i < 4; i++) {
		actionAddressBase[i] = new Action_Control_SMB(this, using_flags);
		actionAddressBase[i]->setCheckable(true);
		actionAddressBase[i]->setVisible(true);
		actionAddressBase[i]->setChecked(false);
		menuAddrBase->addAction(actionAddressBase[i]);
		actionAddressBase[i]->smb_binds->setValue1(i);
		connect(actionAddressBase[i], SIGNAL(triggered()),
				actionAddressBase[i]->smb_binds, SLOT(do_set_adrs_base()));

	}
	uint32_t xval = (config.dipswitch >> 2) & 3;
	actionAddressBase[xval]->setChecked(true);
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



