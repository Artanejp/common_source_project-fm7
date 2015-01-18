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

#if defined(USE_QD1) || defined(USE_QD2)
        ConfigQuickDiskMenu();
#endif

	ConfigScreenMenu();
	
        actionAbout = new Action_Control(this);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));

   
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

#if defined(USE_QD1)
        CreateQuickDiskMenu(0, 1);
#endif
#if defined(USE_QD2)
        CreateQuickDiskMenu(1, 2);
#endif
#ifdef USE_TAPE
        CreateCMTMenu();
#endif

	CreateScreenMenu();
	
        menuMachine = new QMenu(menubar);
        menuMachine->setObjectName(QString::fromUtf8("menuMachine"));

	menuSound = new QMenu(menubar);
	menuSound->setObjectName(QString::fromUtf8("menuSound"));

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
#if defined(USE_QD1)
   	menubar->addAction(menuQD[0]->menuAction());
#endif
#if defined(USE_QD2)
   	menubar->addAction(menuQD[1]->menuAction());
#endif
#if defined(USE_TAPE)
        menubar->addAction(menuCMT->menuAction());
#endif
        menubar->addAction(menuMachine->menuAction());
	
        menubar->addAction(menuSound->menuAction());
        menubar->addAction(menuScreen->menuAction());
//        menubar->addAction(menuRecord->menuAction());
        menubar->addAction(menuEmulator->menuAction());
        menubar->addAction(menuHELP->menuAction());
#if defined(USE_FD1)
        CreateFloppyPulldownMenu(0);
#endif
#if defined(USE_FD2)
        CreateFloppyPulldownMenu(1);
#endif
#if defined(USE_FD3)
        CreateFloppyPulldownMenu(2);
#endif
#if defined(USE_FD4)
        CreateFloppyPulldownMenu(3);
#endif
#if defined(USE_FD5)
        CreateFloppyPulldownMenu(4);
#endif
#if defined(USE_FD6)
        CreateFloppyPulldownMenu(5);
#endif
#if defined(USE_FD7)
        CreateFloppyPulldownMenu(6);
#endif
#if defined(USE_FD8)
        CreateFloppyPulldownMenu(7);
#endif
#ifdef USE_TAPE
        CreateCMTPulldownMenu();
#endif
#if defined(USE_QD1)
        CreateQuickDiskPulldownMenu(0);
#endif
#if defined(USE_QD2)
        CreateQuickDiskPulldownMenu(1);
#endif

	CreateSoundMenu();
	
	menuHELP->addAction(actionAbout);
        menuHELP->addSeparator();

//        retranslateUi();
        QObject::connect(actionCRT_Filter, SIGNAL(toggled(bool)), actionCRT_Filter, SLOT(setChecked(bool)));
        QObject::connect(actionExit_Emulator, SIGNAL(destroyed()), MainWindow, SLOT(close()));


	QObject::connect(this, SIGNAL(destroyed()), this, SLOT(on_actionExit_triggered()));
	QObject::connect(this, SIGNAL(closed()), this, SLOT(on_actionExit_triggered()));
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
  
  
  actionAbout->setText(QApplication::translate("MainWindow", "About...", 0, QApplication::UnicodeUTF8));
  

	
   menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0, QApplication::UnicodeUTF8));
   menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0, QApplication::UnicodeUTF8));
  
   menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0, QApplication::UnicodeUTF8));
} // retranslateUi

QT_END_NAMESPACE
