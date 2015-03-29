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
   int i;
   QString tmps;
#if 0
   menu_Emu_DipSw = new QMenu(menuMachine);
   menu_Emu_DipSw->setObjectName(QString::fromUtf8("menu_DipSw"));

   actionGroup_DipSw = new QActionGroup(this);
   actionGroup_DipSw->setExclusive(false);
   menuMachine->addAction(menu_Emu_DipSw->menuAction());
   for(i = 0; i < 4; i++) {
      	action_Emu_DipSw[i] = new Action_Control_HC20(this);
        action_Emu_DipSw[i]->setCheckable(true);
        action_Emu_DipSw[i]->hc20_binds->setValue1(i);
        tmps.number(i + 1);
        tmps = QString::fromUtf8("actionEmu_DipSw") + tmps;
        action_Emu_DipSw[i]->setObjectName(tmps);
      
        if(((1 << i) & config.dipswitch) != 0) action_Emu_DipSw[i]->setChecked(true);
      
         menu_Emu_DipSw->addAction(action_Emu_DipSw[i]);
         actionGroup_DipSw->addAction(action_Emu_DipSw[i]);
         connect(action_Emu_DipSw[i], SIGNAL(toggled(bool)),
	      action_Emu_DipSw[i]->hc20_binds, SLOT(set_dipsw(bool)));
         connect(action_Emu_DipSw[i]->hc20_binds, SIGNAL(sig_dipsw(int, bool)),
	      this, SLOT(set_dipsw(int, bool)));

   }
#endif   
   
}

void META_MainWindow::retranslateUi(void)
{
  int i;
   
  retranslateControlMenu(" ",  false);
  retranslateFloppyMenu(0, 0);
  retranslateFloppyMenu(1, 1);
//  retranslateCMTMenu();
  retranslateSoundMenu();
  retranslateScreenMenu();
   
  this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
  
  
  actionAbout->setText(QApplication::translate("MainWindow", "About...", 0, QApplication::UnicodeUTF8));
  

  menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0, QApplication::UnicodeUTF8));
  menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0, QApplication::UnicodeUTF8));
  
  menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0, QApplication::UnicodeUTF8));
#if 0
   // Set Labels
  menu_Emu_DipSw->setTitle(QApplication::translate("MainWindow", "DIP Switches", 0, QApplication::UnicodeUTF8));
  action_Emu_DipSw[0]->setText(QApplication::translate("MainWindow", "Dip Switch 1", 0, QApplication::UnicodeUTF8));
  action_Emu_DipSw[1]->setText(QApplication::translate("MainWindow", "Dip Switch 2", 0, QApplication::UnicodeUTF8));
  action_Emu_DipSw[2]->setText(QApplication::translate("MainWindow", "Dip Switch 3", 0, QApplication::UnicodeUTF8));
  action_Emu_DipSw[3]->setText(QApplication::translate("MainWindow", "Dip Switch 4", 0, QApplication::UnicodeUTF8));
#endif
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



