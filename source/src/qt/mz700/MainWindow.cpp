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
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE


Action_Control_MZ700::Action_Control_MZ700(QObject *parent) : Action_Control(parent)
{
   mz_binds = new Object_Menu_Control_MZ700(parent);
}

Action_Control_MZ700::~Action_Control_MZ700(){
   delete mz_binds;
}

Object_Menu_Control_MZ700::Object_Menu_Control_MZ700(QObject *parent) : Object_Menu_Control(parent)
{
}

Object_Menu_Control_MZ700::~Object_Menu_Control_MZ700(){
}

void Object_Menu_Control_MZ700::do_monitor_type(void)
{
   emit sig_monitor_type(getValue1());
}

void META_MainWindow::set_monitor_type(int num)
{
#ifdef USE_MONITOR_TYPE
   if((num < 0) || (num >= USE_MONITOR_TYPE)) return;
   if(emu) {
      config.monitor_type = num;
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }
#endif
}


	

void META_MainWindow::setupUI_Emu(void)
{
#if !defined(_MZ800)
	menuMachine->setVisible(false);
#endif   
#if defined(USE_BOOT_MODE)
	menuBootMode = new QMenu(menuMachine);
	menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
	menuMachine->addAction(menuBootMode->menuAction());
	ConfigCPUBootMode(USE_BOOT_MODE);
#endif
#if defined(USE_MONITOR_TYPE)
	{
		int ii;
		menuMonitorType = new QMenu(menuMachine);
		menuMonitorType->setObjectName(QString::fromUtf8("menuControl_MonitorType"));
		menuMachine->addAction(menuMonitorType->menuAction());
		
		actionGroup_MonitorType = new QActionGroup(this);
		actionGroup_MonitorType->setExclusive(true);
		for(ii = 0; ii < USE_MONITOR_TYPE; ii++) {
			actionMonitorType[ii] = new Action_Control_MZ700(this);
			actionGroup_MonitorType->addAction(actionMonitorType[ii]);
			actionMonitorType[ii]->setCheckable(true);
			actionMonitorType[ii]->setVisible(true);
			actionMonitorType[ii]->mz_binds->setValue1(ii);
			if(config.monitor_type == ii) actionMonitorType[ii]->setChecked(true);
			menuMonitorType->addAction(actionMonitorType[ii]);
			connect(actionMonitorType[ii], SIGNAL(triggered()),
				actionMonitorType[ii]->mz_binds, SLOT(do_monitor_type()));
			connect(actionMonitorType[ii]->mz_binds, SIGNAL(sig_monitor_type(int)),
				this, SLOT(set_monitor_type(int)));
		}
	}
#endif

}

void META_MainWindow::retranslateUi(void)
{
  retranslateControlMenu(" ",  true);
  retranslateFloppyMenu(0, 0);
  retranslateFloppyMenu(1, 1);
  retranslateQuickDiskMenu(0, 0);
  retranslateCMTMenu();
  retranslateSoundMenu();
  retranslateScreenMenu();
	retranslateUI_Help();
   
  this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
  
  
  actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0));
  
  actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));
  

  //	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0));
  //      actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0));

#if defined(USE_QD1)   
   menuQD[0]->setTitle(QApplication::translate("MainWindow", "QD1", 0));
   menuWrite_Protection_QD[0]->setTitle(QApplication::translate("MainWindow", "Write Protection", 0));
#endif   
#if defined(USE_QD2)   
   menuQD[1]->setTitle(QApplication::translate("MainWindow", "QD2", 0));
   menuWrite_Protection_QD[1]->setTitle(QApplication::translate("MainWindow", "Write Protection", 0));
#endif   
#if defined(_MZ800)
	menuBootMode->setTitle("BOOT Mode");
	actionBootMode[0]->setText(QString::fromUtf8("MZ-800"));
	actionBootMode[1]->setText(QString::fromUtf8("MZ-700"));
   
	menuMonitorType->setTitle("Monitor Type");
	actionMonitorType[0]->setText(QApplication::translate("MainWindow", "Color", 0));
	actionMonitorType[1]->setText(QApplication::translate("MainWindow", "Monochrome", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));;

#endif
	menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0));
	menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0));
	
	
//        menuRecord->setTitle(QApplication::translate("MainWindow", "Record", 0));
//        menuRecoad_as_movie->setTitle(QApplication::translate("MainWindow", "Recoad as movie", 0));
	
   menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
  
  menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0));
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



