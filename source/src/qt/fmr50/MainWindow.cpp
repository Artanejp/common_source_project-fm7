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
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE


void META_MainWindow::retranslateUi(void)
{

  retranslateControlMenu(" ", false);
  retranslateFloppyMenu(0, 0);
  retranslateFloppyMenu(1, 1);
  retranslateFloppyMenu(2, 2);
  retranslateFloppyMenu(3, 3);
  retranslateSoundMenu();
  retranslateScreenMenu();
   
  this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
  
  
  actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0));
  
  actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));
  

   //	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0));
  //      actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0));
  // 
  menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0));
  menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0));
  // FM-7 Specified
  menuCpuType->setTitle("CPU Frequency");
#if defined(HAS_I286)
  actionCpuType[0]->setText(QString::fromUtf8("8MHz"));
  actionCpuType[1]->setText(QString::fromUtf8("12MHz"));
#elif defined(HAS_I386)
  actionCpuType[0]->setText(QString::fromUtf8("16MHz"));
  actionCpuType[1]->setText(QString::fromUtf8("20MHz"));
#else 
  actionCpuType[0]->setText(QString::fromUtf8("20MHz"));
  actionCpuType[1]->setText(QString::fromUtf8("25MHz"));
#endif

// End.
// 
//        menuRecord->setTitle(QApplication::translate("MainWindow", "Record", 0));
//        menuRecoad_as_movie->setTitle(QApplication::translate("MainWindow", "Recoad as movie", 0));
	
  menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
  menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));
  
  menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
   // Set Labels
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
   menuCpuType = new QMenu(menuMachine);
   menuCpuType->setObjectName(QString::fromUtf8("menuControl_CpuType"));
   menuMachine->addAction(menuCpuType->menuAction());
   ConfigCPUTypes(2);
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



