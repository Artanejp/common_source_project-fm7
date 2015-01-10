/********************************************************************************
** Form generated from reading UI file 'mainwindowjLG445.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef MAINWINDOWJLG445_H
#define MAINWINDOWJLG445_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGraphicsView>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

#include "menuclasses.h"


int Qt_GuiMain(int argc, char *argv[])
{
  rMainWindow.run();
}

QT_BEGIN_NAMESPACE

void Ui_MainWindow::setupUi(void)
{
  
        MainWindow = new QMainWindow();
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1288, 862);


	ConfigControlMenu(MainWindow);
	
        actionInsert_FD0 = new QAction(MainWindow);
        actionInsert_FD0->setObjectName(QString::fromUtf8("actionInsert_FD0"));
        actionEject_FD0 = new QAction(MainWindow);
        actionEject_FD0->setObjectName(QString::fromUtf8("actionEject_FD0"));
        actionRecent_Opened_FD0 = new QAction(MainWindow);
        actionRecent_Opened_FD0->setObjectName(QString::fromUtf8("actionRecent_Opened_FD0"));
        actionSelect_D88_Image_FD0 = new QAction(MainWindow);
        actionSelect_D88_Image_FD0->setObjectName(QString::fromUtf8("actionSelect_D88_Image_FD0"));
        actionProtection_ON_FD0 = new QAction(MainWindow);
        actionProtection_ON_FD0->setObjectName(QString::fromUtf8("actionProtection_ON_FD0"));
        actionProtection_ON_FD0->setCheckable(true);
        actionProtection_ON_FD0->setChecked(true);
        actionProtection_OFF_FD0 = new QAction(MainWindow);
        actionProtection_OFF_FD0->setObjectName(QString::fromUtf8("actionProtection_OFF_FD0"));
        actionProtection_OFF_FD0->setCheckable(true);

        actionInsert_FD1 = new QAction(MainWindow);
        actionInsert_FD1->setObjectName(QString::fromUtf8("actionInsert_FD1"));
        actionEject_FD1 = new QAction(MainWindow);
        actionEject_FD1->setObjectName(QString::fromUtf8("actionEject_FD1"));
        actionRecent_Opened_FD1 = new QAction(MainWindow);
        actionRecent_Opened_FD1->setObjectName(QString::fromUtf8("actionRecent_Opened_FD1"));
        actionSelect_D88_Image_FD1 = new QAction(MainWindow);
        actionSelect_D88_Image_FD1->setObjectName(QString::fromUtf8("actionSelect_D88_Image_FD1"));
        actionProtection_ON_FD1 = new QAction(MainWindow);
        actionProtection_ON_FD1->setObjectName(QString::fromUtf8("actionProtection_ON_FD1"));
        actionProtection_ON_FD1->setCheckable(true);
        actionProtection_ON_FD1->setChecked(true);
        actionProtection_OFF_FD1 = new QAction(MainWindow);
        actionProtection_OFF_FD1->setObjectName(QString::fromUtf8("actionProtection_OFF_FD1"));
        actionProtection_OFF_FD1->setCheckable(true);


	
	//actionInsert_QD0 = new QAction(MainWindow);
        //actionInsert_QD0->setObjectName(QString::fromUtf8("actionInsert_QD0"));
        //actionEject_QD0 = new QAction(MainWindow);
        //actionEject_QD0->setObjectName(QString::fromUtf8("actionEject_QD0"));
        //actionResent_Images_QD0 = new QAction(MainWindow);
        //actionResent_Images_QD0->setObjectName(QString::fromUtf8("actionResent_Images_QD0"));
        //actionProtection_ON_QD0 = new QAction(MainWindow);
        //actionProtection_ON_QD0->setObjectName(QString::fromUtf8("actionProtection_ON_QD0"));
        //actionProtection_OFF_QD0 = new QAction(MainWindow);
        //actionProtection_OFF_QD0->setObjectName(QString::fromUtf8("actionProtection_OFF_QD0"));
	
        actionInsert_CMT = new QAction(MainWindow);
        actionInsert_CMT->setObjectName(QString::fromUtf8("actionInsert_CMT"));
        actionEject_CMT = new QAction(MainWindow);
        actionEject_CMT->setObjectName(QString::fromUtf8("actionEject_CMT"));
        actionPlay_Start = new QAction(MainWindow);
        actionPlay_Start->setObjectName(QString::fromUtf8("actionPlay_Start"));
        actionPlay_Stop = new QAction(MainWindow);
        actionPlay_Stop->setObjectName(QString::fromUtf8("actionPlay_Stop"));
        actionRecording = new QAction(MainWindow);
        actionRecording->setObjectName(QString::fromUtf8("actionRecording"));
        actionProtection_ON_CMT = new QAction(MainWindow);
        actionProtection_ON_CMT->setObjectName(QString::fromUtf8("actionProtection_ON_CMT"));
        actionProtection_OFF_CMT = new QAction(MainWindow);
        actionProtection_OFF_CMT->setObjectName(QString::fromUtf8("actionProtection_OFF_CMT"));
	
        actionZoom = new QAction(MainWindow);
        actionZoom->setObjectName(QString::fromUtf8("actionZoom"));
        actionDisplay_Mode = new QAction(MainWindow);
        actionDisplay_Mode->setObjectName(QString::fromUtf8("actionDisplay_Mode"));
        actionScanLine = new QAction(MainWindow);
        actionScanLine->setObjectName(QString::fromUtf8("actionScanLine"));
        actionScanLine->setCheckable(true);
        actionScanLine->setChecked(true);
	
        actionCRT_Filter = new QAction(MainWindow);
        actionCRT_Filter->setObjectName(QString::fromUtf8("actionCRT_Filter"));
        actionCRT_Filter->setEnabled(false);
        actionDot_by_Dot = new QAction(MainWindow);
        actionDot_by_Dot->setObjectName(QString::fromUtf8("actionDot_by_Dot"));
        actionDot_by_Dot->setCheckable(true);
        actionDot_by_Dot->setChecked(true);
        actionKeep_Aspect = new QAction(MainWindow);
        actionKeep_Aspect->setObjectName(QString::fromUtf8("actionKeep_Aspect"));
        actionKeep_Aspect->setCheckable(true);
        actionFill_Display = new QAction(MainWindow);
        actionFill_Display->setObjectName(QString::fromUtf8("actionFill_Display"));
        actionFill_Display->setCheckable(true);
	
        actionCapture_Screen = new QAction(MainWindow);
        actionCapture_Screen->setObjectName(QString::fromUtf8("actionCapture_Screen"));
	
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
	
        action2000Hz = new QAction(MainWindow);
        action2000Hz->setObjectName(QString::fromUtf8("action2000Hz"));
        action2000Hz->setCheckable(true);
        action4000Hz = new QAction(MainWindow);
        action4000Hz->setObjectName(QString::fromUtf8("action4000Hz"));
        action4000Hz->setCheckable(true);
        action8000Hz = new QAction(MainWindow);
        action8000Hz->setObjectName(QString::fromUtf8("action8000Hz"));
        action8000Hz->setCheckable(true);
        action11025Hz = new QAction(MainWindow);
        action11025Hz->setObjectName(QString::fromUtf8("action11025Hz"));
        action11025Hz->setCheckable(true);
        action22050Hz = new QAction(MainWindow);
        action22050Hz->setObjectName(QString::fromUtf8("action22050Hz"));
        action22050Hz->setCheckable(true);
        action44100Hz = new QAction(MainWindow);
        action44100Hz->setObjectName(QString::fromUtf8("action44100Hz"));
        action44100Hz->setCheckable(true);
        action48000Hz = new QAction(MainWindow);
        action48000Hz->setObjectName(QString::fromUtf8("action48000Hz"));
        action48000Hz->setCheckable(true);
        action96000Hz = new QAction(MainWindow);
        action96000Hz->setObjectName(QString::fromUtf8("action96000Hz"));
        action96000Hz->setCheckable(true);
	
        action50ms = new QAction(MainWindow);
        action50ms->setObjectName(QString::fromUtf8("action50ms"));
        action50ms->setCheckable(true);
        action100ms = new QAction(MainWindow);
        action100ms->setObjectName(QString::fromUtf8("action100ms"));
        action100ms->setCheckable(true);
        action100ms->setChecked(false);
        action200ms = new QAction(MainWindow);
        action200ms->setObjectName(QString::fromUtf8("action200ms"));
        action300ms = new QAction(MainWindow);
        action300ms->setObjectName(QString::fromUtf8("action300ms"));
        action300ms->setCheckable(true);
        action400ms = new QAction(MainWindow);
        action400ms->setObjectName(QString::fromUtf8("action400ms"));
        action400ms->setCheckable(true);
	
        actionStart_Record = new QAction(MainWindow);
        actionStart_Record->setObjectName(QString::fromUtf8("actionStart_Record"));
        actionStart_Record->setCheckable(true);
        actionStop_Record = new QAction(MainWindow);
        actionStop_Record->setObjectName(QString::fromUtf8("actionStop_Record"));
        actionStop_Record->setCheckable(true);
        actionStart_Record_Movie = new QAction(MainWindow);
        actionStart_Record_Movie->setObjectName(QString::fromUtf8("actionStart_Record_Movie"));
        actionStart_Record_Movie->setCheckable(true);
        actionStop_Record_Movie = new QAction(MainWindow);
        actionStop_Record_Movie->setObjectName(QString::fromUtf8("actionStop_Record_Movie"));
        actionStop_Record_Movie->setCheckable(false);
	
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
	
        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
        graphicsView->setGeometry(QRect(0, 0, 1280, 800));
	
        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);
	
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1288, 27));
        menuControl = new QMenu(menubar);
        menuControl->setObjectName(QString::fromUtf8("menuControl"));
        menuState = new QMenu(menuControl);
        menuState->setObjectName(QString::fromUtf8("menuState"));
        menuCopy_Paste = new QMenu(menuControl);
        menuCopy_Paste->setObjectName(QString::fromUtf8("menuCopy_Paste"));
        menuCpu_Speed = new QMenu(menuControl);
        menuCpu_Speed->setObjectName(QString::fromUtf8("menuCpu_Speed"));
        menuDebugger = new QMenu(menuControl);
        menuDebugger->setObjectName(QString::fromUtf8("menuDebugger"));
	
        menuFD0 = new QMenu(menubar);
        menuFD0->setObjectName(QString::fromUtf8("menuFD0"));
        menuWrite_Protection_FD0 = new QMenu(menuFD0);
        menuWrite_Protection_FD0->setObjectName(QString::fromUtf8("menuWrite_Protection_FD0"));

        menuFD1 = new QMenu(menubar);
        menuFD1->setObjectName(QString::fromUtf8("menuFD1"));
        menuWrite_Protection_FD1 = new QMenu(menuFD1);
        menuWrite_Protection_FD1->setObjectName(QString::fromUtf8("menuWrite_Protection_FD1"));

	//        menuQD0 = new QMenu(menubar);
        //menuQD0->setObjectName(QString::fromUtf8("menuQD0"));
        //menuWrite_Protection_QD0 = new QMenu(menuQD0);
        //menuWrite_Protection_QD0->setObjectName(QString::fromUtf8("menuWrite_Protection_QD0"));
        //menuCMT = new QMenu(menubar);
	
        menuCMT->setObjectName(QString::fromUtf8("menuCMT"));
        menuWrite_Protection_CMT = new QMenu(menuCMT);
        menuWrite_Protection_CMT->setObjectName(QString::fromUtf8("menuWrite_Protection_CMT"));
	
        menuScreen = new QMenu(menubar);
        menuScreen->setObjectName(QString::fromUtf8("menuScreen"));
        menuStretch_Mode = new QMenu(menuScreen);
        menuStretch_Mode->setObjectName(QString::fromUtf8("menuStretch_Mode"));
        menuSound = new QMenu(menubar);
        menuSound->setObjectName(QString::fromUtf8("menuSound"));
        menuOutput_Frequency = new QMenu(menuSound);
        menuOutput_Frequency->setObjectName(QString::fromUtf8("menuOutput_Frequency"));
        menuSound_Latency = new QMenu(menuSound);
        menuSound_Latency->setObjectName(QString::fromUtf8("menuSound_Latency"));
        menuFrequency = new QMenu(menubar);
        menuFrequency->setObjectName(QString::fromUtf8("menuFrequency"));
        menuRecord = new QMenu(menubar);
        menuRecord->setObjectName(QString::fromUtf8("menuRecord"));
        menuRecord_sound = new QMenu(menuRecord);
        menuRecord_sound->setObjectName(QString::fromUtf8("menuRecord_sound"));
        menuRecoad_as_movie = new QMenu(menuRecord);
        menuRecoad_as_movie->setObjectName(QString::fromUtf8("menuRecoad_as_movie"));
        menuEmulator = new QMenu(menubar);
        menuEmulator->setObjectName(QString::fromUtf8("menuEmulator"));
        menuHELP = new QMenu(menubar);
        menuHELP->setObjectName(QString::fromUtf8("menuHELP"));
        MainWindow->setMenuBar(menubar);

	
        menubar->addAction(menuControl->menuAction());
	connectActions_ControlMenu(menubar);

        menubar->addAction(menuFD0->menuAction());
        menubar->addAction(menuFD1->menuAction());
	//        menubar->addAction(menuQD0->menuAction());
        menubar->addAction(menuCMT->menuAction());
        menubar->addAction(menuFrequency->menuAction());
        menubar->addAction(menuSound->menuAction());
        menubar->addAction(menuScreen->menuAction());
        menubar->addAction(menuRecord->menuAction());
        menubar->addAction(menuEmulator->menuAction());
        menubar->addAction(menuHELP->menuAction());

	menuFD0->addAction(actionInsert_FD0);
        menuFD0->addAction(actionEject_FD0);
        menuFD0->addSeparator();
        menuFD0->addAction(actionRecent_Opened_FD0);
        menuFD0->addSeparator();
        menuFD0->addAction(actionSelect_D88_Image_FD0);
        menuFD0->addSeparator();
        menuFD0->addAction(menuWrite_Protection_FD0->menuAction());

        menuFD1->addAction(actionInsert_FD1);
        menuFD1->addAction(actionEject_FD1);
        menuFD1->addSeparator();
        menuFD1->addAction(actionRecent_Opened_FD1);
        menuFD1->addSeparator();
        menuFD1->addAction(actionSelect_D88_Image_FD1);
        menuFD1->addSeparator();
        menuFD1->addAction(menuWrite_Protection_FD1->menuAction());

	menuWrite_Protection_FD0->addAction(actionProtection_ON_FD0);
        menuWrite_Protection_FD0->addAction(actionProtection_OFF_FD0);

	menuWrite_Protection_FD1->addAction(actionProtection_ON_FD1);
        menuWrite_Protection_FD1->addAction(actionProtection_OFF_FD1);

	//menuQD0->addAction(actionInsert_QD0);
        //menuQD0->addAction(actionEject_QD0);
        //menuQD0->addSeparator();
        //menuQD0->addAction(actionResent_Images_QD0);
        //menuQD0->addSeparator();
        //menuQD0->addAction(menuWrite_Protection_QD0->menuAction());
	//menuWrite_Protection_QD0->addAction(actionProtection_ON_QD0);
        //menuWrite_Protection_QD0->addAction(actionProtection_OFF_QD0);

	menuCMT->addAction(actionInsert_CMT);
        menuCMT->addAction(actionEject_CMT);
        menuCMT->addSeparator();
        menuCMT->addAction(actionPlay_Start);
        menuCMT->addAction(actionPlay_Stop);
        menuCMT->addSeparator();
        menuCMT->addAction(actionRecording);
        menuCMT->addSeparator();
        menuCMT->addAction(menuWrite_Protection_CMT->menuAction());
        menuWrite_Protection_CMT->addAction(actionProtection_ON_CMT);
        menuWrite_Protection_CMT->addAction(actionProtection_OFF_CMT);
	
        menuScreen->addAction(actionZoom);
        menuScreen->addAction(actionDisplay_Mode);
        menuScreen->addSeparator();
        menuScreen->addAction(menuStretch_Mode->menuAction());
        menuScreen->addSeparator();
        menuScreen->addAction(actionScanLine);
        menuScreen->addAction(actionCRT_Filter);
        menuStretch_Mode->addAction(actionDot_by_Dot);
        menuStretch_Mode->addAction(actionKeep_Aspect);
        menuStretch_Mode->addAction(actionFill_Display);
	
        menuSound->addAction(menuOutput_Frequency->menuAction());
        menuSound->addAction(menuSound_Latency->menuAction());
	
        menuOutput_Frequency->addAction(action2000Hz);
        menuOutput_Frequency->addAction(action4000Hz);
        menuOutput_Frequency->addAction(action8000Hz);
        menuOutput_Frequency->addAction(action11025Hz);
        menuOutput_Frequency->addAction(action22050Hz);
        menuOutput_Frequency->addAction(action44100Hz);
        menuOutput_Frequency->addAction(action48000Hz);
        menuOutput_Frequency->addAction(action96000Hz);
	
        menuSound_Latency->addAction(action50ms);
        menuSound_Latency->addAction(action100ms);
        menuSound_Latency->addAction(action200ms);
        menuSound_Latency->addAction(action300ms);
        menuSound_Latency->addAction(action400ms);
	
        menuRecord->addAction(actionCapture_Screen);
        menuRecord->addSeparator();
        menuRecord->addAction(menuRecoad_as_movie->menuAction());
        menuRecord->addSeparator();
        menuRecord->addAction(menuRecord_sound->menuAction());
        menuRecord_sound->addAction(actionStart_Record);
        menuRecord_sound->addAction(actionStop_Record);
        menuRecoad_as_movie->addAction(actionStart_Record_Movie);
        menuRecoad_as_movie->addAction(actionStop_Record_Movie);

	menuHELP->addAction(actionAbout);
        menuHELP->addSeparator();

        retranslateUi(MainWindow);
        QObject::connect(actionCRT_Filter, SIGNAL(toggled(bool)), actionCRT_Filter, SLOT(setChecked(bool)));
        QObject::connect(actionExit_Emulator, SIGNAL(destroyed()), MainWindow, SLOT(close()));

        QObject::connect(actionProtection_OFF_FD0, SIGNAL(triggered(bool)), actionProtection_OFF_FD0, SLOT(setChecked(bool)));
        QObject::connect(actionProtection_ON_FD0, SIGNAL(triggered(bool)), actionProtection_ON_FD0, SLOT(setChecked(bool)));
        QObject::connect(actionProtection_OFF_FD1, SIGNAL(triggered(bool)), actionProtection_OFF_FD1, SLOT(setChecked(bool)));
        QObject::connect(actionProtection_ON_FD1, SIGNAL(triggered(bool)), actionProtection_ON_FD1, SLOT(setChecked(bool)));
	
        QObject::connect(actionSpeed_x1, SIGNAL(triggered()), actionSpeed_x1, SLOT(trigger()));
        QObject::connect(actionSpeed_x2, SIGNAL(triggered()), actionSpeed_x2, SLOT(trigger()));
        QObject::connect(actionSpeed_x4, SIGNAL(triggered()), actionSpeed_x4, SLOT(trigger()));
        QObject::connect(actionSpeed_x8, SIGNAL(triggered()), actionSpeed_x8, SLOT(trigger()));
        QObject::connect(actionSpeed_x16, SIGNAL(triggered()), actionSpeed_x16, SLOT(trigger()));
        QObject::connect(actionEject_FD0, SIGNAL(triggered()), actionEject_FD0, SLOT(trigger()));
	QObject::connect(actionEject_FD0, SIGNAL(triggered()), actionEject_FD1, SLOT(trigger()));

	QObject::connect(action100ms, SIGNAL(triggered()), action100ms, SLOT(trigger()));
        QObject::connect(action200ms, SIGNAL(triggered()), action200ms, SLOT(trigger()));
        QObject::connect(action300ms, SIGNAL(triggered()), action300ms, SLOT(trigger()));
        QObject::connect(action400ms, SIGNAL(triggered()), action400ms, SLOT(trigger()));
        QObject::connect(action50ms, SIGNAL(triggered()), action50ms, SLOT(trigger()));
	
        QObject::connect(action2000Hz, SIGNAL(triggered()), action2000Hz, SLOT(trigger()));
        QObject::connect(action4000Hz, SIGNAL(triggered()), action4000Hz, SLOT(trigger()));
        QObject::connect(action8000Hz, SIGNAL(triggered()), action8000Hz, SLOT(trigger()));
        QObject::connect(action11025Hz, SIGNAL(triggered()), action11025Hz, SLOT(trigger()));
        QObject::connect(action22050Hz, SIGNAL(triggered()), action22050Hz, SLOT(trigger()));
        QObject::connect(action44100Hz, SIGNAL(triggered()), action44100Hz, SLOT(trigger()));
        QObject::connect(action48000Hz, SIGNAL(triggered()), action48000Hz, SLOT(trigger()));
        QObject::connect(action96000Hz, SIGNAL(triggered()), action96000Hz, SLOT(trigger()));

	QObject::connect(MainWindow, SIGNAL(destroyed()), MainWindow, SLOT(close()));

        QMetaObject::connectSlotsByName(MainWindow);
} // setupUi

void Ui_MainWindow::retranslateUi(QMainWindow *MainWindow)
{

  retranslateControlMenu(MainWindow, "NMI Reset",  true);
  
  MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
  actionInsert_FD0->setText(QApplication::translate("MainWindow", "Insert", 0, QApplication::UnicodeUTF8));
  actionEject_FD0->setText(QApplication::translate("MainWindow", "Eject", 0, QApplication::UnicodeUTF8));
  actionRecent_Opened_FD0->setText(QApplication::translate("MainWindow", "Recent Opened", 0, QApplication::UnicodeUTF8));
  actionSelect_D88_Image_FD0->setText(QApplication::translate("MainWindow", "Select D88 Image", 0, QApplication::UnicodeUTF8));
  actionProtection_ON_FD0->setText(QApplication::translate("MainWindow", "Protection ON", 0, QApplication::UnicodeUTF8));
  actionProtection_OFF_FD0->setText(QApplication::translate("MainWindow", "Protection OFF", 0, QApplication::UnicodeUTF8));
  //actionInsert_QD0->setText(QApplication::translate("MainWindow", "Insert", 0, QApplication::UnicodeUTF8));
  //actionEject_QD0->setText(QApplication::translate("MainWindow", "Eject", 0, QApplication::UnicodeUTF8));
  //actionResent_Images_QD0->setText(QApplication::translate("MainWindow", "Resent Images", 0, QApplication::UnicodeUTF8));
  //actionProtection_ON_QD0->setText(QApplication::translate("MainWindow", "Protection ON", 0, QApplication::UnicodeUTF8));
  //actionProtection_OFF_QD0->setText(QApplication::translate("MainWindow", "Protection OFF", 0, QApplication::UnicodeUTF8));
  actionInsert_CMT->setText(QApplication::translate("MainWindow", "Insert CMT", 0, QApplication::UnicodeUTF8));
  actionEject_CMT->setText(QApplication::translate("MainWindow", "Eject CMT", 0, QApplication::UnicodeUTF8));
  actionPlay_Start->setText(QApplication::translate("MainWindow", "Play Start", 0, QApplication::UnicodeUTF8));
  actionPlay_Stop->setText(QApplication::translate("MainWindow", "Play Stop", 0, QApplication::UnicodeUTF8));
  actionRecording->setText(QApplication::translate("MainWindow", "Recording", 0, QApplication::UnicodeUTF8));
  actionProtection_ON_CMT->setText(QApplication::translate("MainWindow", "Protection ON", 0, QApplication::UnicodeUTF8));
  actionProtection_OFF_CMT->setText(QApplication::translate("MainWindow", "Protection OFF", 0, QApplication::UnicodeUTF8));
  actionZoom->setText(QApplication::translate("MainWindow", "Zoom Screen", 0, QApplication::UnicodeUTF8));
  actionDisplay_Mode->setText(QApplication::translate("MainWindow", "Display Mode", 0, QApplication::UnicodeUTF8));
  actionScanLine->setText(QApplication::translate("MainWindow", "Set ScanLine", 0, QApplication::UnicodeUTF8));
  actionCRT_Filter->setText(QApplication::translate("MainWindow", "CRT Filter", 0, QApplication::UnicodeUTF8));
  actionDot_by_Dot->setText(QApplication::translate("MainWindow", "Dot by Dot", 0, QApplication::UnicodeUTF8));
  actionKeep_Aspect->setText(QApplication::translate("MainWindow", "Keep Aspect", 0, QApplication::UnicodeUTF8));
  actionFill_Display->setText(QApplication::translate("MainWindow", "Fill Display", 0, QApplication::UnicodeUTF8));
  
  actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0, QApplication::UnicodeUTF8));
  
  actionAbout->setText(QApplication::translate("MainWindow", "About...", 0, QApplication::UnicodeUTF8));
  
  action2000Hz->setText(QApplication::translate("MainWindow", "2000Hz", 0, QApplication::UnicodeUTF8));
  action4000Hz->setText(QApplication::translate("MainWindow", "4000Hz", 0, QApplication::UnicodeUTF8));
  action8000Hz->setText(QApplication::translate("MainWindow", "8000Hz", 0, QApplication::UnicodeUTF8));
  action11025Hz->setText(QApplication::translate("MainWindow", "11025Hz", 0, QApplication::UnicodeUTF8));
  action22050Hz->setText(QApplication::translate("MainWindow", "22050Hz", 0, QApplication::UnicodeUTF8));
  action44100Hz->setText(QApplication::translate("MainWindow", "44100Hz", 0, QApplication::UnicodeUTF8));
  action48000Hz->setText(QApplication::translate("MainWindow", "48000Hz", 0, QApplication::UnicodeUTF8));
  action96000Hz->setText(QApplication::translate("MainWindow", "96000Hz", 0, QApplication::UnicodeUTF8));
  
	action50ms->setText(QApplication::translate("MainWindow", "50ms", 0, QApplication::UnicodeUTF8));
        action100ms->setText(QApplication::translate("MainWindow", "100ms", 0, QApplication::UnicodeUTF8));
        action200ms->setText(QApplication::translate("MainWindow", "200ms", 0, QApplication::UnicodeUTF8));
        action300ms->setText(QApplication::translate("MainWindow", "300ms", 0, QApplication::UnicodeUTF8));
        action400ms->setText(QApplication::translate("MainWindow", "400ms", 0, QApplication::UnicodeUTF8));

	actionStart_Record->setText(QApplication::translate("MainWindow", "Start Record", 0, QApplication::UnicodeUTF8));
        actionStop_Record->setText(QApplication::translate("MainWindow", "Stop Record", 0, QApplication::UnicodeUTF8));

	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0, QApplication::UnicodeUTF8));
        actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0, QApplication::UnicodeUTF8));

        menuFD0->setTitle(QApplication::translate("MainWindow", "Floppy", 0, QApplication::UnicodeUTF8));
        menuWrite_Protection_FD0->setTitle(QApplication::translate("MainWindow", "Write Protection", 0, QApplication::UnicodeUTF8));
        menuFD1->setTitle(QApplication::translate("MainWindow", "Floppy 1", 0, QApplication::UnicodeUTF8));
        menuWrite_Protection_FD0->setTitle(QApplication::translate("MainWindow", "Write Protection", 0, QApplication::UnicodeUTF8));
	//        menuQD0->setTitle(QApplication::translate("MainWindow", "QD", 0, QApplication::UnicodeUTF8));
        //menuWrite_Protection_QD0->setTitle(QApplication::translate("MainWindow", "Write Protection", 0, QApplication::UnicodeUTF8));
        menuCMT->setTitle(QApplication::translate("MainWindow", "CMT", 0, QApplication::UnicodeUTF8));
        menuWrite_Protection_CMT->setTitle(QApplication::translate("MainWindow", "Write Protection", 0, QApplication::UnicodeUTF8));
        menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0, QApplication::UnicodeUTF8));
        menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0, QApplication::UnicodeUTF8));
	
        menuSound->setTitle(QApplication::translate("MainWindow", "Sound", 0, QApplication::UnicodeUTF8));
        menuOutput_Frequency->setTitle(QApplication::translate("MainWindow", "Output Frequency", 0, QApplication::UnicodeUTF8));
        menuSound_Latency->setTitle(QApplication::translate("MainWindow", "Sound Latency", 0, QApplication::UnicodeUTF8));
        menuFrequency->setTitle(QApplication::translate("MainWindow", "Machine", 0, QApplication::UnicodeUTF8));
	
        menuRecord->setTitle(QApplication::translate("MainWindow", "Record", 0, QApplication::UnicodeUTF8));
        menuRecord_sound->setTitle(QApplication::translate("MainWindow", "Record sound", 0, QApplication::UnicodeUTF8));
        menuRecoad_as_movie->setTitle(QApplication::translate("MainWindow", "Recoad as movie", 0, QApplication::UnicodeUTF8));
	
        menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0, QApplication::UnicodeUTF8));
	
        menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0, QApplication::UnicodeUTF8));
} // retranslateUi




QT_END_NAMESPACE

#endif // MAINWINDOWJLG445_H
