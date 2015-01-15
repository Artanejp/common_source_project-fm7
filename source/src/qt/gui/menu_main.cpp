/*
 * Common Source code Project:
 * Ui->Qt->gui->menu_main for generic.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, moved from qt/x1turboz/MainWindow.cpp .
 */

#include <QtCore/QVariant>
#include <QtGui>
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"


QT_BEGIN_NAMESPACE

Ui_MainWindow::Ui_MainWindow(QWidget *parent) : QMainWindow(parent)
{
   setupUi();
   createContextMenu();
}

Ui_MainWindow::~Ui_MainWindow()
{
}

void Ui_MainWindow::setupUi(void)
{
  
   MainWindow = new QMainWindow();
   if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1288, 862);
   
	ConfigControlMenu();
        ConfigFloppyMenu();
        ConfigCMTMenu();
	ConfigSoundMenu();

	
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
	
        actionZoom = new Action_Control(this);
        actionZoom->setObjectName(QString::fromUtf8("actionZoom"));
        actionDisplay_Mode = new Action_Control(this);
        actionDisplay_Mode->setObjectName(QString::fromUtf8("actionDisplay_Mode"));
        actionScanLine = new Action_Control(this);
        actionScanLine->setObjectName(QString::fromUtf8("actionScanLine"));
        actionScanLine->setCheckable(true);
        actionScanLine->setChecked(true);
	
        actionCRT_Filter = new Action_Control(this);
        actionCRT_Filter->setObjectName(QString::fromUtf8("actionCRT_Filter"));
        actionCRT_Filter->setEnabled(false);
        actionDot_by_Dot = new Action_Control(this);
        actionDot_by_Dot->setObjectName(QString::fromUtf8("actionDot_by_Dot"));
        actionDot_by_Dot->setCheckable(true);
        actionDot_by_Dot->setChecked(true);
        actionKeep_Aspect = new Action_Control(this);
        actionKeep_Aspect->setObjectName(QString::fromUtf8("actionKeep_Aspect"));
        actionKeep_Aspect->setCheckable(true);
        actionFill_Display = new Action_Control(this);
        actionFill_Display->setObjectName(QString::fromUtf8("actionFill_Display"));
        actionFill_Display->setCheckable(true);
	
        actionCapture_Screen = new Action_Control(this);
        actionCapture_Screen->setObjectName(QString::fromUtf8("actionCapture_Screen"));
	
        actionAbout = new Action_Control(this);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
	
	
        actionStart_Record_Movie = new Action_Control(this);
        actionStart_Record_Movie->setObjectName(QString::fromUtf8("actionStart_Record_Movie"));
        actionStart_Record_Movie->setCheckable(true);
        actionStop_Record_Movie = new Action_Control(this);
        actionStop_Record_Movie->setObjectName(QString::fromUtf8("actionStop_Record_Movie"));
        actionStop_Record_Movie->setCheckable(false);
	
   
        graphicsView = new GLDrawClass(MainWindow);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
        graphicsView->setGeometry(QRect(0, 0, 1280, 800));
        MainWindow->setCentralWidget(graphicsView);
	

        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);
        initStatusBar();
	
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1288, 27));
        menuControl = new QMenu(menubar);
        menuControl->setObjectName(QString::fromUtf8("menuControl"));
#ifdef USE_BOOT_MODE
//        menuBootMode = new QMenu(menuControl);
//        menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
#endif
#ifdef USE_CPU_TYPE
//        menuCpuType = new QMenu(menuControl);
//        menuCpuType->setObjectName(QString::fromUtf8("menuControl_CpuType"));
#endif
        menuState = new QMenu(menuControl);
        menuState->setObjectName(QString::fromUtf8("menuState"));
        menuCopy_Paste = new QMenu(menuControl);
        menuCopy_Paste->setObjectName(QString::fromUtf8("menuCopy_Paste"));
        menuCpu_Speed = new QMenu(menuControl);
        menuCpu_Speed->setObjectName(QString::fromUtf8("menuCpu_Speed"));
        menuDebugger = new QMenu(menuControl);
        menuDebugger->setObjectName(QString::fromUtf8("menuDebugger"));
#ifdef USE_FD1	
	CreateFloppyMenu(0, 1);
#endif
#ifdef USE_FD2	
	CreateFloppyMenu(1, 2);
#endif
#ifdef USE_FD3	
	CreateFloppyMenu(2, 3);
#endif
#ifdef USE_FD4	
	CreateFloppyMenu(3, 4);
#endif
#ifdef USE_FD5	
	CreateFloppyMenu(4, 5);
#endif
#ifdef USE_FD6	
	CreateFloppyMenu(5, 6);
#endif
#ifdef USE_FD7	
	CreateFloppyMenu(6, 7);
#endif
#ifdef USE_FD8	
	CreateFloppyMenu(7, 8);
#endif


	//        menuQD0 = new QMenu(menubar);
        //menuQD0->setObjectName(QString::fromUtf8("menuQD0"));
        //menuWrite_Protection_QD0 = new QMenu(menuQD0);
        //menuWrite_Protection_QD0->setObjectName(QString::fromUtf8("menuWrite_Protection_QD0"));
#ifdef USE_TAPE
        CreateCMTMenu();
#endif
        menuScreen = new QMenu(menubar);
        menuScreen->setObjectName(QString::fromUtf8("menuScreen"));
        menuStretch_Mode = new QMenu(menuScreen);
        menuStretch_Mode->setObjectName(QString::fromUtf8("menuStretch_Mode"));

        menuMachine = new QMenu(menubar);
        menuMachine->setObjectName(QString::fromUtf8("menuMachine"));
	//        menuMachine_SoundDevice = new QMenu(menuScreen);
        //menuMachine_SoundDevice->setObjectName(QString::fromUtf8("menuMachine_SoundDevice"));

	menuSound = new QMenu(menubar);
	menuSound->setObjectName(QString::fromUtf8("menuSound"));

//        menuRecoad_as_movie = new QMenu(menuRecord);
//        menuRecoad_as_movie->setObjectName(QString::fromUtf8("menuRecoad_as_movie"));
        menuEmulator = new QMenu(menubar);
        menuEmulator->setObjectName(QString::fromUtf8("menuEmulator"));
        menuHELP = new QMenu(menubar);
        menuHELP->setObjectName(QString::fromUtf8("menuHELP"));
        MainWindow->setMenuBar(menubar);

	
        menubar->addAction(menuControl->menuAction());
	connectActions_ControlMenu();
#if defined(USE_FD1)
        menubar->addAction(menuFD[0]->menuAction());
#endif
#if defined(USE_FD2)
        menubar->addAction(menuFD[1]->menuAction());
#endif
#if defined(USE_FD3)
        menubar->addAction(menuFD[2]->menuAction());
#endif
#if defined(USE_FD4)
        menubar->addAction(menuFD[3]->menuAction());
#endif
#if defined(USE_FD5)
        menubar->addAction(menuFD[4]->menuAction());
#endif
#if defined(USE_FD6)
        menubar->addAction(menuFD[5]->menuAction());
#endif
#if defined(USE_FD7)
        menubar->addAction(menuFD[6]->menuAction());
#endif
#if defined(USE_FD8)
        menubar->addAction(menuFD[7]->menuAction());
#endif
	//        menubar->addAction(menuQD0->menuAction());
        menubar->addAction(menuCMT->menuAction());
        menubar->addAction(menuMachine->menuAction());
        menubar->addAction(menuSound->menuAction());
        menubar->addAction(menuScreen->menuAction());
//        menubar->addAction(menuRecord->menuAction());
        menubar->addAction(menuEmulator->menuAction());
        menubar->addAction(menuHELP->menuAction());
	CreateFloppyPulldownMenu(0);
	CreateFloppyPulldownMenu(1);
        CreateCMTPulldownMenu();
	//menuQD0->addAction(actionInsert_QD0);
        //menuQD0->addAction(actionEject_QD0);
        //menuQD0->addSeparator();
        //menuQD0->addAction(actionResent_Images_QD0);
        //menuQD0->addSeparator();
        //menuQD0->addAction(menuWrite_Protection_QD0->menuAction());
	//menuWrite_Protection_QD0->addAction(actionProtection_ON_QD0);
        //menuWrite_Protection_QD0->addAction(actionProtection_OFF_QD0);

	//	menuCMT->addAction(actionInsert_CMT);
        //menuCMT->addAction(actionEject_CMT);
        //menuCMT->addSeparator();
        //menuCMT->addAction(actionPlay_Start);
        //menuCMT->addAction(actionPlay_Stop);
        //menuCMT->addSeparator();
        //menuCMT->addAction(actionRecording);
        //menuCMT->addSeparator();
        //menuCMT->addAction(menuWrite_Protection_CMT->menuAction());
        //menuWrite_Protection_CMT->addAction(actionProtection_ON_CMT);
        //menuWrite_Protection_CMT->addAction(actionProtection_OFF_CMT);
	
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

	CreateSoundMenu();
	
//        menuRecord->addAction(actionCapture_Screen);
//        menuRecord->addSeparator();
//        menuRecord->addAction(menuRecoad_as_movie->menuAction());
//        menuRecord->addSeparator();
//        menuRecord->addAction(menuRecord_sound->menuAction());
//        menuRecord_sound->addAction(actionStart_Record);
//        menuRecord_sound->addAction(actionStop_Record);
//        menuRecoad_as_movie->addAction(actionStart_Record_Movie);
//        menuRecoad_as_movie->addAction(actionStop_Record_Movie);

	menuHELP->addAction(actionAbout);
        menuHELP->addSeparator();

//        retranslateUi();
        QObject::connect(actionCRT_Filter, SIGNAL(toggled(bool)), actionCRT_Filter, SLOT(setChecked(bool)));
        QObject::connect(actionExit_Emulator, SIGNAL(destroyed()), MainWindow, SLOT(close()));



	QObject::connect(this, SIGNAL(destroyed()), this, SLOT(on_actionExit_triggered()));
	QObject::connect(MainWindow, SIGNAL(destroyed()), this, SLOT(on_actionExit_triggered()));
        QMetaObject::connectSlotsByName(MainWindow);
} // setupUi


// You can Override this function: Re-define on foo/MainWindow.cpp.
// This code is example: by X1(TurboZ).
void Ui_MainWindow::retranslateUi(void)
{
  retranslateControlMenu("NMI Reset",  true);
  retranslateFloppyMenu(0, 0);
  retranslateFloppyMenu(1, 1);
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

   
	//        menuQD0->setTitle(QApplication::translate("MainWindow", "QD", 0, QApplication::UnicodeUTF8));
        //menuWrite_Protection_QD0->setTitle(QApplication::translate("MainWindow", "Write Protection", 0, QApplication::UnicodeUTF8));
   
   
        menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0, QApplication::UnicodeUTF8));
        menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0, QApplication::UnicodeUTF8));
	
	
//        menuRecord->setTitle(QApplication::translate("MainWindow", "Record", 0, QApplication::UnicodeUTF8));
//        menuRecoad_as_movie->setTitle(QApplication::translate("MainWindow", "Recoad as movie", 0, QApplication::UnicodeUTF8));
	
        menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0, QApplication::UnicodeUTF8));
  menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0, QApplication::UnicodeUTF8));
  
        menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0, QApplication::UnicodeUTF8));
} // retranslateUi

QT_END_NAMESPACE
