/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for JR100 .
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
#include "commonclasses.h"
#include "menuclasses.h"
#include "qt_main.h"


void Action_Control_EX80::do_set_sw1()
{
	emit sig_set_dipsw(0, bind_bool);
}

void Action_Control_EX80::do_set_sw2()
{
	emit sig_set_dipsw(1, bind_bool);
}

void Action_Control_EX80::do_set_vram_addr()
{
	int addr = bind_int;
	emit sig_set_dipsw(2, ((addr & 1) != 0));
	emit sig_set_dipsw(3, ((addr & 2) != 0));
}

void Action_Control_EX80::setBoolValue(bool flag)
{
	bind_bool = flag;
}

void Action_Control_EX80::setIntValue(int val)
{
	bind_int = val;
}


Action_Control_EX80::Action_Control_EX80(QObject *parent, USING_FLAGS *pp) : Action_Control(parent, pp)
{
	bind_bool = false;
	bind_int = 0;
}

Action_Control_EX80::~Action_Control_EX80()
{
}

extern config_t config;

void META_MainWindow::setupUI_Emu(void)
{
   int i; 
   actionGroup_DipSW1 = new QActionGroup(this);
   actionGroup_DipSW1->setExclusive(true);
   
   actionDipSW1_ON = new Action_Control_EX80(this, using_flags);
   actionDipSW1_ON->setCheckable(true);
   actionDipSW1_ON->setVisible(true);
   actionDipSW1_ON->setBoolValue(true);
   actionGroup_DipSW1->addAction(actionDipSW1_ON);
   
   actionDipSW1_OFF = new Action_Control_EX80(this, using_flags);
   actionDipSW1_OFF->setCheckable(true);
   actionDipSW1_OFF->setVisible(true);
   actionDipSW1_OFF->setBoolValue(false);
   actionGroup_DipSW1->addAction(actionDipSW1_OFF);
   
   if((config.dipswitch & 0x01) != 0) {
	   actionDipSW1_ON->setChecked(true);
   } else {
	   actionDipSW1_OFF->setChecked(true);
   }
   
   actionGroup_DipSW2 = new QActionGroup(this);
   actionGroup_DipSW2->setExclusive(true);
   
   actionDipSW2_ON = new Action_Control_EX80(this, using_flags);
   actionDipSW2_ON->setCheckable(true);
   actionDipSW2_ON->setVisible(true);
   actionDipSW2_ON->setBoolValue(true);
   actionGroup_DipSW2->addAction(actionDipSW2_ON);
   
   actionDipSW2_OFF = new Action_Control_EX80(this, using_flags);
   actionDipSW2_OFF->setCheckable(true);
   actionDipSW2_OFF->setVisible(true);
   actionDipSW2_OFF->setBoolValue(false);
   actionGroup_DipSW2->addAction(actionDipSW2_OFF);
   
   if((config.dipswitch & 0x02) != 0) {
	   actionDipSW2_ON->setChecked(true);
   } else {
	   actionDipSW2_OFF->setChecked(true);
   }
   
   menuDipSW1 = new QMenu(menuMachine);
   menuDipSW1->setObjectName(QString::fromUtf8("menuControl_DipSW1"));
   menuDipSW1->addAction(actionDipSW1_ON);
   menuDipSW1->addAction(actionDipSW1_OFF);
   menuMachine->addAction(menuDipSW1->menuAction());
   connect(actionDipSW1_ON, SIGNAL(triggered()), actionDipSW2_ON, SLOT(do_set_sw1()));
   connect(actionDipSW1_OFF, SIGNAL(triggered()), actionDipSW2_OFF, SLOT(do_set_sw1()));
   connect(actionDipSW1_ON,  SIGNAL(sig_set_dipsw(int, bool)), this, SLOT(set_dipsw(int, bool)));
   connect(actionDipSW1_OFF,  SIGNAL(sig_set_dipsw(int, bool)), this, SLOT(set_dipsw(int, bool)));

   menuDipSW2 = new QMenu(menuMachine);
   menuDipSW2->setObjectName(QString::fromUtf8("menuControl_DipSW2"));
   menuDipSW2->addAction(actionDipSW2_ON);
   menuDipSW2->addAction(actionDipSW2_OFF);
   menuMachine->addAction(menuDipSW2->menuAction());
   connect(actionDipSW2_ON, SIGNAL(triggered()), actionDipSW2_ON, SLOT(do_set_sw2()));
   connect(actionDipSW2_OFF, SIGNAL(triggered()), actionDipSW2_OFF, SLOT(do_set_sw2()));
   connect(actionDipSW2_ON,  SIGNAL(sig_set_dipsw(int, bool)), this, SLOT(set_dipsw(int, bool)));
   connect(actionDipSW2_OFF,  SIGNAL(sig_set_dipsw(int, bool)), this, SLOT(set_dipsw(int, bool)));
		   
   menuVramAddr = new QMenu(menuMachine);
   menuVramAddr->setObjectName(QString::fromUtf8("menuControl_VramAddr"));
   
   actionGroup_VramAddr = new QActionGroup(this);
   actionGroup_VramAddr->setExclusive(true);
   int dipbit = (config.dipswitch & 0x0c) >> 2;
   for(i = 0; i < 4; i++) {
	   actionVramAddr[i] = new Action_Control_EX80(this, using_flags);
	   actionVramAddr[i]->setCheckable(true);
	   actionVramAddr[i]->setVisible(true);
	   actionVramAddr[i]->setIntValue(i);
	   actionGroup_VramAddr->addAction(actionVramAddr[i]);
	   menuVramAddr->addAction(actionVramAddr[i]);
	   
	   if(i == dipbit) actionVramAddr[i]->setChecked(true);
	   connect(actionVramAddr[i], SIGNAL(triggered()), actionVramAddr[i], SLOT(do_set_vram_addr()));
	   connect(actionVramAddr[i],  SIGNAL(sig_set_dipsw(int, bool)), this, SLOT(set_dipsw(int, bool)));
   }
   menuMachine->addAction(menuVramAddr->menuAction());
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("System Reset",  false);
	
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



