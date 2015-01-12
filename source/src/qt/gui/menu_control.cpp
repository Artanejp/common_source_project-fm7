/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  History: Jan 10, 2015 : Initial
 */

#include "menuclasses.h"

QT_BEGIN_NAMESPACE


void Object_Menu_Control::set_boot_mode(void) {
   emit on_boot_mode(bindValue);
}
   
void Object_Menu_Control::set_cputype(void) {
   emit on_cpu_type(bindValue);
}
void Object_Menu_Control::set_cpupower(void) {
   emit on_cpu_power(bindValue);
}
void Object_Menu_Control::open_debugger(void) {
   emit on_open_debugger(bindValue);
}


void Ui_MainWindow::ConfigCpuSpeed(Ui_MainWindow *p)
{
   
   
  actionSpeed_x1 = new Action_Control(p);
  actionSpeed_x1->setObjectName(QString::fromUtf8("actionSpeed_x1"));
  actionSpeed_x1->setCheckable(true);
  actionSpeed_x1->setChecked(true);
  actionSpeed_x1->binds->setValue1(0);
  connect(actionSpeed_x1, SIGNAL(triggered()), actionSpeed_x1->binds, SLOT(set_cpupower())); // OK?  
  connect(actionSpeed_x1->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  
  
  actionSpeed_x2 = new Action_Control(p);
  actionSpeed_x2->setObjectName(QString::fromUtf8("actionSpeed_x2"));
  actionSpeed_x2->setCheckable(true);
  actionSpeed_x2->binds->setValue1(1);
  connect(actionSpeed_x2, SIGNAL(triggered()), actionSpeed_x2->binds, SLOT(set_cpupower())); // OK?  
  connect(actionSpeed_x2->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  
  
  actionSpeed_x4 = new Action_Control(p);
  actionSpeed_x4->setObjectName(QString::fromUtf8("actionSpeed_x4"));
  actionSpeed_x4->setCheckable(true);
  actionSpeed_x4->binds->setValue1(2);
  connect(actionSpeed_x4, SIGNAL(triggered()), actionSpeed_x4->binds, SLOT(set_cpupower())); // OK?  
  connect(actionSpeed_x4->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  
  
  actionSpeed_x8 = new Action_Control(p);
  actionSpeed_x8->setObjectName(QString::fromUtf8("actionSpeed_x8"));
  actionSpeed_x8->setCheckable(true);
  actionSpeed_x8->binds->setValue1(3);
  connect(actionSpeed_x8, SIGNAL(triggered()), actionSpeed_x8->binds, SLOT(set_cpupower())); // OK?  
  connect(actionSpeed_x8->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  

  
  actionSpeed_x16 = new Action_Control(p);
  actionSpeed_x16->setObjectName(QString::fromUtf8("actionSpeed_x16"));
  actionSpeed_x16->setCheckable(true);
  actionSpeed_x16->binds->setValue1(4);
  connect(actionSpeed_x16, SIGNAL(triggered()), actionSpeed_x16->binds, SLOT(set_cpupower())); // OK?  
  connect(actionSpeed_x16->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  

  actionGroup_CpuSpeed = new QActionGroup(p);
  actionGroup_CpuSpeed->setExclusive(true);
  actionGroup_CpuSpeed->addAction(actionSpeed_x1);
  actionGroup_CpuSpeed->addAction(actionSpeed_x2);
  actionGroup_CpuSpeed->addAction(actionSpeed_x4);
  actionGroup_CpuSpeed->addAction(actionSpeed_x8);
  actionGroup_CpuSpeed->addAction(actionSpeed_x16);
   
}

void Ui_MainWindow::ConfigControlMenu(Ui_MainWindow *p)
{
  actionReset = new Action_Control(p);
  actionReset->setObjectName(QString::fromUtf8("actionReset"));
  connect(actionReset, SIGNAL(triggered()), this, SLOT(OnReset())); // OK?  

  actionSpecial_Reset = new Action_Control(p);
  actionSpecial_Reset->setObjectName(QString::fromUtf8("actionSpecial_Reset"));
  connect(actionSpecial_Reset, SIGNAL(triggered()), this, SLOT(OnSpecialReset())); // OK?  

  actionExit_Emulator = new Action_Control(p);
  actionExit_Emulator->setObjectName(QString::fromUtf8("actionExit_Emulator"));
  connect(actionExit_Emulator, SIGNAL(triggered()), this, SLOT(on_actionExit_triggered())); // OnGuiExit()?  

  actionPaste_from_Clipboard = new Action_Control(p);
  actionPaste_from_Clipboard->setObjectName(QString::fromUtf8("actionPaste_from_Clipboard"));
  connect(actionPaste_from_Clipboard, SIGNAL(triggered()),
		     this, SLOT(OnStartAutoKey())); // OK?  

  actionStop_Pasting = new Action_Control(p);
  actionStop_Pasting->setObjectName(QString::fromUtf8("actionStop_Pasting"));
  connect(actionStop_Pasting, SIGNAL(triggered()),
		   this, SLOT(OnStopAutoKey())); // OK?  
  
  actionSave_State = new Action_Control(p);
  actionSave_State->setObjectName(QString::fromUtf8("actionSave_State"));
  connect(actionSave_State, SIGNAL(triggered()),
		   this, SLOT(OnSaveState())); // OK?  

  actionLoad_State = new Action_Control(p);
  actionLoad_State->setObjectName(QString::fromUtf8("actionLoad_State"));
  connect(actionLoad_State, SIGNAL(triggered()),
		   this, SLOT(OnLoadState())); // OK?  

  actionDebugger_1 = new Action_Control(p);
  actionDebugger_1->setObjectName(QString::fromUtf8("actionDebugger_1"));
  actionDebugger_1->binds->setValue1(0);
  connect(actionDebugger_1, SIGNAL(triggered()), actionDebugger_1->binds, SLOT(call_open_debugger())); // OK?  
  connect(actionDebugger_1->binds, SIGNAL(on_open_debugger(int)), this, SLOT(open_debugger(int))); // OK?  

  actionDebugger_2 = new Action_Control(p);
  actionDebugger_2->setObjectName(QString::fromUtf8("actionDebugger_2"));
  actionDebugger_2->binds->setValue1(1);
  connect(actionDebugger_2, SIGNAL(triggered()), actionDebugger_2->binds, SLOT(call_open_debugger())); // OK?  
  connect(actionDebugger_2->binds, SIGNAL(on_open_debugger(int)), this, SLOT(open_debugger(int))); // OK?  
  
  actionDebugger_3 = new Action_Control(p);
  actionDebugger_3->binds->setValue1(2);
  actionDebugger_3->setObjectName(QString::fromUtf8("actionDebugger_3"));
  connect(actionDebugger_3, SIGNAL(triggered()), actionDebugger_3->binds, SLOT(call_open_debugger())); // OK?  
  connect(actionDebugger_3->binds, SIGNAL(on_open_debugger(int)), this, SLOT(open_debugger(int))); // OK?  

  actionClose_Debuggers = new Action_Control(p);
  actionClose_Debuggers->setObjectName(QString::fromUtf8("actionClose_Debuggers"));
  connect(actionClose_Debuggers, SIGNAL(triggered()),
		   this, SLOT(OnCloseDebugger())); // OK?  

  ConfigCpuSpeed(p);
}

void Ui_MainWindow::connectActions_ControlMenu(QMenuBar *menubar)
{
	
        menuControl->addAction(actionReset);
        menuControl->addAction(actionSpecial_Reset);
        menuControl->addSeparator();
        menuControl->addAction(menuCpu_Speed->menuAction());
        menuControl->addSeparator();
        menuControl->addAction(menuCopy_Paste->menuAction());
        menuControl->addSeparator();
        menuControl->addAction(menuState->menuAction());
        menuControl->addAction(menuDebugger->menuAction());
        menuControl->addSeparator();
        menuControl->addAction(actionExit_Emulator);
        menuState->addAction(actionSave_State);
        menuState->addSeparator();
        menuState->addAction(actionLoad_State);
        menuCopy_Paste->addAction(actionPaste_from_Clipboard);
        menuCopy_Paste->addAction(actionStop_Pasting);
        menuCpu_Speed->addAction(actionSpeed_x1);
        menuCpu_Speed->addAction(actionSpeed_x2);
        menuCpu_Speed->addAction(actionSpeed_x4);
        menuCpu_Speed->addAction(actionSpeed_x8);
        menuCpu_Speed->addAction(actionSpeed_x16);
        menuDebugger->addAction(actionDebugger_1);
        menuDebugger->addAction(actionDebugger_2);
        menuDebugger->addAction(actionDebugger_3);
        menuDebugger->addSeparator();
        menuDebugger->addAction(actionClose_Debuggers);
}

void Ui_MainWindow::createContextMenu(void)
{
        addAction(actionReset);
        addAction(actionSpecial_Reset);
        addAction(menuCpu_Speed->menuAction());
        addAction(menuCopy_Paste->menuAction());
        addAction(menuState->menuAction());
        addAction(menuDebugger->menuAction());
        addAction(actionExit_Emulator);
}


void Ui_MainWindow::retranslateControlMenu(Ui_MainWindow *p, const char *SpecialResetTitle,  bool WithSpecialReset)
{
  actionReset->setText(QApplication::translate("MainWindow", "Reset", 0, QApplication::UnicodeUTF8));
  if(WithSpecialReset)
    actionSpecial_Reset->setText(QApplication::translate("MainWindow", SpecialResetTitle, 0, QApplication::UnicodeUTF8));

  actionExit_Emulator->setText(QApplication::translate("MainWindow", "Exit Emulator", 0, QApplication::UnicodeUTF8));

  actionSpeed_x1->setText(QApplication::translate("MainWindow", "Speed x1", 0, QApplication::UnicodeUTF8));
  actionSpeed_x2->setText(QApplication::translate("MainWindow", "Speed x2", 0, QApplication::UnicodeUTF8));
  actionSpeed_x4->setText(QApplication::translate("MainWindow", "Speed x4", 0, QApplication::UnicodeUTF8));
  actionSpeed_x8->setText(QApplication::translate("MainWindow", "Seppd x8", 0, QApplication::UnicodeUTF8));
  actionSpeed_x16->setText(QApplication::translate("MainWindow", "Speed x16", 0, QApplication::UnicodeUTF8));

  actionPaste_from_Clipboard->setText(QApplication::translate("MainWindow", "Paste from Clipboard", 0, QApplication::UnicodeUTF8));
  actionStop_Pasting->setText(QApplication::translate("MainWindow", "Stop Pasting", 0, QApplication::UnicodeUTF8));

  actionSave_State->setText(QApplication::translate("MainWindow", "Save State", 0, QApplication::UnicodeUTF8));
  actionLoad_State->setText(QApplication::translate("MainWindow", "Load State", 0, QApplication::UnicodeUTF8));

  actionDebugger_1->setText(QApplication::translate("MainWindow", "Debugger 1", 0, QApplication::UnicodeUTF8));
  actionDebugger_2->setText(QApplication::translate("MainWindow", "Debugger 2", 0, QApplication::UnicodeUTF8));
  actionDebugger_3->setText(QApplication::translate("MainWindow", "Debugger 3", 0, QApplication::UnicodeUTF8));
  actionClose_Debuggers->setText(QApplication::translate("MainWindow", "Close Debuggers", 0, QApplication::UnicodeUTF8));

  menuControl->setTitle(QApplication::translate("MainWindow", "control", 0, QApplication::UnicodeUTF8));
  menuState->setTitle(QApplication::translate("MainWindow", "State", 0, QApplication::UnicodeUTF8));

  menuCopy_Paste->setTitle(QApplication::translate("MainWindow", "Copy/Paste", 0, QApplication::UnicodeUTF8));
  menuCpu_Speed->setTitle(QApplication::translate("MainWindow", "Cpu Speed", 0, QApplication::UnicodeUTF8));
  menuDebugger->setTitle(QApplication::translate("MainWindow", "Debugger", 0, QApplication::UnicodeUTF8));
}

QT_END_NAMESPACE