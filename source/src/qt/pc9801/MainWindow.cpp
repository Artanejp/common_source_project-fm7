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


void META_MainWindow::do_set_sound_device(int num)
{
   if((num < 0) || (num >= 3)) return;
#ifdef USE_SOUND_DEVICE_TYPE
   if(emu) {
      config.sound_device_type = num;
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }
#endif
}

void META_MainWindow::retranslateUi(void)
{
  const char title="";
  retranslateControlMenu(title, false);
#if defined(USE_FD1)
  retranslateFloppyMenu(0, 1);
#endif
#if defined(USE_FD2)
  retranslateFloppyMenu(1, 2);
#endif
#if defined(USE_FD3)
  retranslateFloppyMenu(2, 3);
#endif
#if defined(USE_FD4)
  retranslateFloppyMenu(3, 4);
#endif
#if defined(USE_FD5)
  retranslateFloppyMenu(4, 5);
#endif
#if defined(USE_FD6)
  retranslateFloppyMenu(5, 6);
#endif
#if defined(_PC9801) || defined(_PC9801E)
   // Drive 3,4
   menuFD[2]->setTitle(QApplication::translate("MainWindow", "2DD-1", 0, QApplication::UnicodeUTF8));
   menuFD[3]->setTitle(QApplication::translate("MainWindow", "2DD-2", 0, QApplication::UnicodeUTF8));
   // Drive 5, 6
   menuFD[4]->setTitle(QApplication::translate("MainWindow", "2D-1", 0, QApplication::UnicodeUTF8));
   menuFD[5]->setTitle(QApplication::translate("MainWindow", "2D-2", 0, QApplication::UnicodeUTF8));
#elif defined(_PC98DO)
   menuFD[2]->setTitle(QApplication::translate("MainWindow", "PC88-1", 0, QApplication::UnicodeUTF8));
   menuFD[3]->setTitle(QApplication::translate("MainWindow", "PC88-2", 0, QApplication::UnicodeUTF8));
#endif
   
#if defined(USE_TAPE)
  retranslateCMTMenu();
#endif
  retranslateSoundMenu();
  retranslateScreenMenu();
   
  this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
  
  
  actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0, QApplication::UnicodeUTF8));
  
  actionAbout->setText(QApplication::translate("MainWindow", "About...", 0, QApplication::UnicodeUTF8));
  

   //	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0, QApplication::UnicodeUTF8));
  //      actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0, QApplication::UnicodeUTF8));
  // 
  menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0, QApplication::UnicodeUTF8));
  menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0, QApplication::UnicodeUTF8));

#ifdef USE_CPU_TYPE
  menuCpuType->setTitle("CPU Frequency");
# if  defined(_PC98DO)
  actionCpuType[0]->setText(QString::fromUtf8("10/8MHz"));
  actionCpuType[1]->setText(QString::fromUtf8("5/4MHz"));
# elif  defined(_PC9801E) || defined(_PC9801VM)
  actionCpuType[0]->setText(QString::fromUtf8("8MHz"));
  actionCpuType[1]->setText(QString::fromUtf8("5MHz"));
# endif
#endif
   
#ifdef USE_BOOT_MODE
# ifdef _PC98DO
  menuBootMode->setTitle("Machine Mode");
  actionBootMode[0]->setText(QString::fromUtf8("PC-98"));
  actionBootMode[1]->setText(QString::fromUtf8("N88-V1(S) Mode"));
  actionBootMode[2]->setText(QString::fromUtf8("N88-V1(H) Mode"));	
  actionBootMode[3]->setText(QString::fromUtf8("N88-V2 Mode"));
  actionBootMode[4]->setText(QString::fromUtf8("N Mode (N80 compatible)"));
# endif
#endif   
 // End.
 // 
//        menuRecord->setTitle(QApplication::translate("MainWindow", "Record", 0, QApplication::UnicodeUTF8));
//        menuRecoad_as_movie->setTitle(QApplication::translate("MainWindow", "Recoad as movie", 0, QApplication::UnicodeUTF8));
	
  menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0, QApplication::UnicodeUTF8));
  menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0, QApplication::UnicodeUTF8));
  
  menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0, QApplication::UnicodeUTF8));
   // Set Labels
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
#ifdef USE_CPU_TYPE
   menuCpuType = new QMenu(menuMachine);
   menuCpuType->setObjectName(QString::fromUtf8("menuControl_CpuType"));
   menuMachine->addAction(menuCpuType->menuAction());
   ConfigCPUTypes(2);
#endif
   
#ifdef USE_BOOT_MODE
# if defined(_PC98DO)
   menuBootMode = new QMenu(menuMachine);
   menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
   menuMachine->addAction(menuBootMode->menuAction());
   ConfigCPUBootMode(5);
# endif
#endif   
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



