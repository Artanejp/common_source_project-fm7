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




	

void META_MainWindow::setupUI_Emu(void)
{
   menuMachine->setVisible(false);
}

void META_MainWindow::retranslateUi(void)
{
  retranslateControlMenu(" ",  true);
  retranslateFloppyMenu(0, 1);
  retranslateFloppyMenu(1, 2);
  retranslateFloppyMenu(2, 3);
  retranslateFloppyMenu(3, 4);
  retranslateCMTMenu();
  retranslateSoundMenu();
   
  this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
  
  actionZoom->setText(QApplication::translate("MainWindow", "Zoom Screen", 0, QApplication::UnicodeUTF8));
  actionDisplay_Mode->setText(QApplication::translate("MainWindow", "Display Mode", 0, QApplication::UnicodeUTF8));
  actionScanLine->setText(QApplication::translate("MainWindow", "Set ScanLine", 0, QApplication::UnicodeUTF8));
  actionCRT_Filter->setText(QApplication::translate("MainWindow", "CRT Filter", 0, QApplication::UnicodeUTF8));
  actionDot_by_Dot->setText(QApplication::translate("MainWindow", "Dot by Dot", 0, QApplication::UnicodeUTF8));
  actionKeep_Aspect->setText(QApplication::translate("MainWindow", "Keep Aspect", 0, QApplication::UnicodeUTF8));
  actionFill_Display->setText(QApplication::translate("MainWindow", "Fill Display", 0, QApplication::UnicodeUTF8));
  
  actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0, QApplication::UnicodeUTF8));
  
  actionAbout->setText(QApplication::translate("MainWindow", "About...", 0, QApplication::UnicodeUTF8));
  

  //	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0, QApplication::UnicodeUTF8));
  //      actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0, QApplication::UnicodeUTF8));

   menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0, QApplication::UnicodeUTF8));
   menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0, QApplication::UnicodeUTF8));
	
	
//        menuRecord->setTitle(QApplication::translate("MainWindow", "Record", 0, QApplication::UnicodeUTF8));
//        menuRecoad_as_movie->setTitle(QApplication::translate("MainWindow", "Recoad as movie", 0, QApplication::UnicodeUTF8));
	
  menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0, QApplication::UnicodeUTF8));
  
  menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0, QApplication::UnicodeUTF8));
   // Set Labels
} // retranslateUi



META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
   setupUI_Emu();
   retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



