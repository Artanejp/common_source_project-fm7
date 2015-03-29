/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QtCore/QVariant>
#include <QtGui>
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE




void META_MainWindow::retranslateUi(void)
{

  retranslateControlMenu("", false);
  retranslateCMTMenu();
  retranslateSoundMenu();
  retranslateScreenMenu();
   // Will implement : RAM LOAD/SAVE
  
  menuBootMode->setTitle("BOOT Mode");
  actionBootMode[0]->setText(QString::fromUtf8("L1 BASIC"));
  actionBootMode[1]->setText(QString::fromUtf8("L2 BASIC"));	
   
  this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
  
  
  actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0, QApplication::UnicodeUTF8));
  
  actionAbout->setText(QApplication::translate("MainWindow", "About...", 0, QApplication::UnicodeUTF8));
  

   //	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0, QApplication::UnicodeUTF8));
  //      actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0, QApplication::UnicodeUTF8));
  // 
  menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0, QApplication::UnicodeUTF8));
  menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0, QApplication::UnicodeUTF8));
  // FP1100 Specified
	
  menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0, QApplication::UnicodeUTF8));
  menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0, QApplication::UnicodeUTF8));
  
  menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0, QApplication::UnicodeUTF8));
   // Set Labels
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
   
   menuBootMode = new QMenu(menuMachine);
   menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
   menuMachine->addAction(menuBootMode->menuAction());

   ConfigCPUBootMode(2);
}


META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
   setupUI_Emu();
   retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



