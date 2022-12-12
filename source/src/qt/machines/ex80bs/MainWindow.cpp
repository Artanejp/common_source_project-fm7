/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for EX-80BS .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */
#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QMenu>

#include "emu.h"
#include "vm.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "qt_main.h"
#include "menu_binary.h"

void META_MainWindow::setupUI_Emu(void)
{
   int i;
   ConfigCPUBootMode(USE_BOOT_MODE);
   
   actionGroup_DipSW1 = new QActionGroup(this);
   actionGroup_DipSW1->setExclusive(true);

   SET_ACTION_DIPSWITCH_CONNECT(actionDipSW1_ON,  0x01, 0x01, p_config->dipswitch, SIGNAL(triggered()), SLOT(do_set_multi_dipswitch()));
   SET_ACTION_DIPSWITCH_CONNECT(actionDipSW1_OFF, 0x00, 0x01, p_config->dipswitch, SIGNAL(triggered()), SLOT(do_set_multi_dipswitch()));
   
   actionGroup_DipSW1->addAction(actionDipSW1_ON);
   actionGroup_DipSW1->addAction(actionDipSW1_OFF);
   
   actionGroup_DipSW2 = new QActionGroup(this);
   actionGroup_DipSW2->setExclusive(true);
   
   SET_ACTION_DIPSWITCH_CONNECT(actionDipSW2_ON , 0x02, 0x02, p_config->dipswitch, SIGNAL(triggered()), SLOT(do_set_multi_dipswitch()));
   SET_ACTION_DIPSWITCH_CONNECT(actionDipSW2_OFF, 0x00, 0x02, p_config->dipswitch, SIGNAL(triggered()), SLOT(do_set_multi_dipswitch()));
   
   actionGroup_DipSW2->addAction(actionDipSW2_ON);
   actionGroup_DipSW2->addAction(actionDipSW2_OFF);
   
   menuDipSW1 = new QMenu(menuMachine);
   menuDipSW1->setObjectName(QString::fromUtf8("menuControl_DipSW1"));
   menuDipSW1->addAction(actionDipSW1_ON);
   menuDipSW1->addAction(actionDipSW1_OFF);
   menuMachine->addAction(menuDipSW1->menuAction());

   menuDipSW2 = new QMenu(menuMachine);
   menuDipSW2->setObjectName(QString::fromUtf8("menuControl_DipSW2"));
   menuDipSW2->addAction(actionDipSW2_ON);
   menuDipSW2->addAction(actionDipSW2_OFF);
   menuMachine->addAction(menuDipSW2->menuAction());
		   
   menuVramAddr = new QMenu(menuMachine);
   menuVramAddr->setObjectName(QString::fromUtf8("menuControl_VramAddr"));
   
   actionGroup_VramAddr = new QActionGroup(this);
   actionGroup_VramAddr->setExclusive(true);
   int dipbit = p_config->dipswitch & 0x0c;
   for(i = 0; i < 4; i++) {
	   SET_ACTION_DIPSWITCH_CONNECT(actionVramAddr[i] , ((uint32_t)i) << 2, 0x0c, dipbit, SIGNAL(triggered()), SLOT(do_set_multi_dipswitch()));
	   
	   actionGroup_VramAddr->addAction(actionVramAddr[i]);
	   menuVramAddr->addAction(actionVramAddr[i]);
	   
   }
   menuMachine->addAction(menuVramAddr->menuAction());
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("System Reset",  false);
	
	menuBootMode->setTitle(QApplication::translate("Machine", "BOOT Mode", 0));
	menuBootMode->setToolTipsVisible(false);
	actionBootMode[0]->setText(QApplication::translate("Machine", "EX-80", 0));
	actionBootMode[1]->setText(QApplication::translate("Machine", "EX-80BS Lv1 BASIC", 0));
	actionBootMode[2]->setText(QApplication::translate("Machine", "EX-80BS Lv2 BASIC", 0));
	
	if(menu_BINs[0] != NULL) menu_BINs[0]->setTitle(QApplication::translate("MenuEX80", "RAM", 0));
#ifdef USE_MOUSE
	actionMouseEnable->setVisible(false);
#endif
	menuDipSW1->setTitle(QApplication::translate("MenuEX80", "SW1", 0));
	actionDipSW1_ON->setText(QApplication::translate("MenuEX80", "STEP", 0));
	actionDipSW1_OFF->setText(QApplication::translate("MenuEX80", "AUTO", 0));

	menuDipSW2->setTitle(QApplication::translate("MenuEX80", "SW2", 0));
	actionDipSW2_ON->setText(QApplication::translate("MenuEX80", "CHAR", 0));
	actionDipSW2_OFF->setText(QApplication::translate("MenuEX80", "BIT", 0));

	menuVramAddr->setTitle(QApplication::translate("MenuEX80", "VRAM Address", 0));
	actionVramAddr[0]->setText(QApplication::translate("MenuEX80", "$8000-$81FF", 0));
	actionVramAddr[1]->setText(QApplication::translate("MenuEX80", "$8200-$83FF", 0));
	actionVramAddr[2]->setText(QApplication::translate("MenuEX80", "$8400-$85FF", 0));
	actionVramAddr[3]->setText(QApplication::translate("MenuEX80", "$8600-$87FF", 0));

#if defined(USE_MONITOR_TYPE)
	actionMonitorType[0]->setText(QApplication::translate("MenuEX80", "Show TV Monitor"));
	actionMonitorType[1]->setText(QApplication::translate("MenuEX80", "Hide TV Monitor"));
//	actionMonitorType[0]->setToolTipVisible(false);
//	actionMonitorType[1]->setToolTipVisible(false);
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
   setupUI_Emu();
   retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



