/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  History: Jan 10, 2015 : Initial
 */

#include "menuclasses.h"

QT_BEGIN_NAMESPACE
void Ui_MainWindow::ConfigCpuSpeed(QMainWindow *MainWindow)
{
  actionSpeed_x1 = new Action_Control(MainWindow);
  actionSpeed_x1->setObjectName(QString::fromUtf8("actionSpeed_x1"));
  actionSpeed_x1->setCheckable(true);
  actionSpeed_x1->setChecked(true);
  actionSpeed_x1->binds->setValue1(0);
  QObject::connect(actionSpeed_x1, SIGNAL(triggered()), actionSpeed_x1, SLOT(OnCpuPower())); // OK?  
  
  actionSpeed_x2 = new Action_Control(MainWindow);
  actionSpeed_x2->setObjectName(QString::fromUtf8("actionSpeed_x2"));
  actionSpeed_x2->setCheckable(true);
  actionSpeed_x2->binds->setValue1(1);
  QObject::connect(actionSpeed_x2, SIGNAL(triggered()), actionSpeed_x2, SLOT(OnCpuPower())); // OK?  
  
  actionSpeed_x4 = new Action_Control(MainWindow);
  actionSpeed_x4->setObjectName(QString::fromUtf8("actionSpeed_x4"));
  actionSpeed_x4->setCheckable(true);
  actionSpeed_x4->binds->setValue1(2);
  QObject::connect(actionSpeed_x4, SIGNAL(triggered()), actionSpeed_x4, SLOT(OnCpuPower())); // OK?  
  
  actionSpeed_x8 = new Action_Control(MainWindow);
  actionSpeed_x8->setObjectName(QString::fromUtf8("actionSpeed_x8"));
  actionSpeed_x8->setCheckable(true);
  actionSpeed_x8->binds->setValue1(3);
  QObject::connect(actionSpeed_x8, SIGNAL(triggered()), actionSpeed_x8, SLOT(OnCpuPower())); // OK?  
  
  actionSpeed_x16 = new Action_Control(MainWindow);
  actionSpeed_x16->setObjectName(QString::fromUtf8("actionSpeed_x16"));
  actionSpeed_x16->setCheckable(true);
  actionSpeed_x16->binds->setValue1(4);
  QObject::connect(actionSpeed_x16, SIGNAL(triggered()), actionSpeed_x16, SLOT(OnCpuPower())); // OK?  

}




void Ui_MainWindow::ConfigControlMenu(QMainWindow *MainWindow)
{
  actionReset = new Action_Control(MainWindow);
  actionReset->setObjectName(QString::fromUtf8("actionReset"));
  QObject::connect(actionReset, SIGNAL(triggered()), actionReset, SLOT(OnReset())); // OK?  

  actionSpecial_Reset = new Action_Control(MainWindow);
  actionSpecial_Reset->setObjectName(QString::fromUtf8("actionSpecial_Reset"));
  QObject::connect(actionSpecial_Reset, SIGNAL(triggered()), actionSpecial_Reset, SLOT(OnSpecialReset())); // OK?  

  actionExit_Emulator = new Action_Control(MainWindow);
  actionExit_Emulator->setObjectName(QString::fromUtf8("actionExit_Emulator"));
  QObject::connect(actionExit_Emulator, SIGNAL(triggered()), MainWindow, SLOT(OnGuiExit())); // OK?  

  actionPaste_from_Clipboard = new Action_Control(MainWindow);
  actionPaste_from_Clipboard->setObjectName(QString::fromUtf8("actionPaste_from_Clipboard"));
  QObject::connect(actionPaste_from_Clipboard, SIGNAL(triggered()),
		     actionPaste_from_Clipboard, SLOT(OnStartAutoKey())); // OK?  

  actionStop_Pasting = new Action_Control(MainWindow);
  actionStop_Pasting->setObjectName(QString::fromUtf8("actionStop_Pasting"));
  QObject::connect(actionStop_Pasting, SIGNAL(triggered()),
		   actionStop_Pasting, SLOT(OnStopAutoKey())); // OK?  
  
  actionSave_State = new Action_Control(MainWindow);
  actionSave_State->setObjectName(QString::fromUtf8("actionSave_State"));
  QObject::connect(actionSave_State, SIGNAL(triggered()),
		   actionSave_State, SLOT(OnSaveState())); // OK?  

  actionLoad_State = new Action_Control(MainWindow);
  actionLoad_State->setObjectName(QString::fromUtf8("actionLoad_State"));
  QObject::connect(actionLoad_State, SIGNAL(triggered()),
		   actionLoad_State, SLOT(OnLoadState())); // OK?  

  actionDebugger_1 = new Action_Control(MainWindow);
  actionDebugger_1->setObjectName(QString::fromUtf8("actionDebugger_1"));
  actionDebugger_1->binds->setValue1(0);
  QObject::connect(actionDebugger_1, SIGNAL(triggered()),
		   actionDebugger_1, SLOT(OnOpenDebugger())); // OK?  
  
  actionDebugger_2 = new Action_Control(MainWindow);
  actionDebugger_2->setObjectName(QString::fromUtf8("actionDebugger_2"));
  actionDebugger_2->binds->setValue1(1);
  QObject::connect(actionDebugger_2, SIGNAL(triggered()),
		   actionDebugger_2, SLOT(OnOpenDebugger())); // OK?  
  
  actionDebugger_3 = new Action_Control(MainWindow);
  actionDebugger_3->binds->setValue1(2);
  actionDebugger_3->setObjectName(QString::fromUtf8("actionDebugger_3"));
  QObject::connect(actionDebugger_3, SIGNAL(triggered()),
		   actionDebugger_3, SLOT(OnOpenDebugger())); // OK?  

  actionClose_Debuggers = new Action_Control(MainWindow);
  actionClose_Debuggers->setObjectName(QString::fromUtf8("actionClose_Debuggers"));
  QObject::connect(actionClose_Debuggers, SIGNAL(triggered()),
		   actionClose_Debuggers, SLOT(OnCloseDebugger())); // OK?  

  ConfigCpuSpeed(MainWindow);
}

void Ui_MainAction::connectActions_ControlMenu(QMainWindow *MainWindow)
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



void Ui_MainWindow::retranslateControlMenu(QMainWindow *MainWindow, Qstring *SpecialResetTitle,  bool WithSpecialReset)
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
