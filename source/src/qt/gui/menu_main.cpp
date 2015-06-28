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
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QPixmap>

#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

extern EMU *emu;

QT_BEGIN_NAMESPACE

Ui_MainWindow::Ui_MainWindow(QWidget *parent) : QMainWindow(parent)
{
	setupUi();
	createContextMenu();
}

Ui_MainWindow::~Ui_MainWindow()
{
}

void Action_Control::do_check_grab_mouse(bool flag)
{
	this->toggle();
}

void Ui_MainWindow::setupUi(void)
{
	int w, h;
	//   QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	MainWindow = new QMainWindow();
	if (MainWindow->objectName().isEmpty())
		MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
	//MainWindow->resize(1288, 862);
   
	ConfigControlMenu();
	ConfigFloppyMenu();
	ConfigCMTMenu();
	
	ConfigSoundMenu();
#if defined(USE_BINARY_FILE1)
	ConfigBinaryMenu(); 
#endif

#if defined(USE_QD1) || defined(USE_QD2)
	ConfigQuickDiskMenu();
#endif

	ConfigScreenMenu();
#if defined(USE_CART1) || defined(USE_CART2)
	ConfigCartMenu();
#endif
	actionAbout = new Action_Control(this);
	actionAbout->setObjectName(QString::fromUtf8("actionAbout"));

   
	graphicsView = new GLDrawClass(this);
	graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
	graphicsView->setMaximumSize(2560, 2560); // ?
	graphicsView->setMinimumSize(240, 192); // ?
   
#if defined(USE_BITMAP)
	bitmapImage = NULL;
#endif   
	MainWindow->setCentralWidget(graphicsView);
	
	MainWindow->centralWidget()->adjustSize();
	MainWindow->adjustSize();
	//	graphicsView->setSizePolicy(sizePolicy);

	statusbar = new QStatusBar(this);
	statusbar->setObjectName(QString::fromUtf8("statusbar"));
	MainWindow->setStatusBar(statusbar);
	initStatusBar();
	
	menubar = new QMenuBar(this);
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
#if defined(USE_CART1)
	CreateCartMenu(0, 1);
#endif
#if defined(USE_CART2)
	CreateCartMenu(1, 2);
#endif
#if defined(USE_BINARY_FILE1)
	CreateBinaryMenu(0, 1);
#endif
#if defined(USE_BINARY_FILE2)
	CreateBinaryMenu(1, 2);
#endif
	
	menuMachine = new QMenu(menubar);
	menuMachine->setObjectName(QString::fromUtf8("menuMachine"));
	actionMouseEnable = new Action_Control(this);
	actionMouseEnable->setCheckable(true);
	actionMouseEnable->setVisible(true);
	actionMouseEnable->setChecked(false);
	menuMachine->addAction(actionMouseEnable);
	connect(actionMouseEnable, SIGNAL(toggled(bool)),
		this, SLOT(do_set_mouse_enable(bool)));
	connect(graphicsView, SIGNAL(sig_check_grab_mouse(bool)),
		actionMouseEnable, SLOT(do_check_grab_mouse(bool)));

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
#if defined(USE_CART1)
	menubar->addAction(menuCART[0]->menuAction());
#endif
#if defined(USE_CART2)
	menubar->addAction(menuCART[1]->menuAction());
#endif
#if defined(USE_BINARY_FILE1)
	menubar->addAction(menuBIN[0]->menuAction());
#endif
#if defined(USE_BINARY_FILE2)
	menubar->addAction(menuBIN[1]->menuAction());
#endif
	menubar->addAction(menuMachine->menuAction());
	
	menubar->addAction(menuSound->menuAction());
	menubar->addAction(menuScreen->menuAction());
//	menubar->addAction(menuRecord->menuAction());
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
#if defined(USE_CART1)
	CreateCartPulldownMenu(0);
#endif
#if defined(USE_CART2)
	CreateCartPulldownMenu(1);
#endif
#if defined(USE_BINARY_FILE1)
	CreateBinaryPulldownMenu(0);
#endif
#if defined(USE_BINARY_FILE2)
	CreateBinaryPulldownMenu(1);
#endif
	CreateSoundMenu();
  
	menuHELP->addAction(actionAbout);
	menuHELP->addSeparator();
	if(config.window_mode <= 0) config.window_mode = 0;
	if(config.window_mode >= _SCREEN_MODE_NUM) config.window_mode = _SCREEN_MODE_NUM - 1;
	if(actionScreenSize[config.window_mode] != NULL) {
#if defined(USE_SCREEN_ROTATE)
		if(config.rotate_type) {
			actionScreenSize[config.window_mode]->binds->getSize(&h, &w);
		} else
#endif	   
		{
			actionScreenSize[config.window_mode]->binds->getSize(&w, &h);
		}
	} else {
#if defined(USE_SCREEN_ROTATE)
		if(config.rotate_type) {
			w = 600;
			h = 960;
		} else 
#endif
	        {		   
			w = 1280;
			h = 800;
		}
	}
	graphicsView->setFixedSize(w, h);
	QImageReader reader(":/default.ico");
	QImage result = reader.read();
	MainWindow->setWindowIcon(QPixmap::fromImage(result));
//	retranslateUi();
	QObject::connect(actionCRT_Filter, SIGNAL(toggled(bool)),
			 actionCRT_Filter, SLOT(setChecked(bool)));
	QObject::connect(actionExit_Emulator, SIGNAL(destroyed()),
			 MainWindow, SLOT(close()));

	QObject::connect(this, SIGNAL(destroyed()),
			 this, SLOT(on_actionExit_triggered()));
	QObject::connect(this, SIGNAL(closed()),
			 this, SLOT(on_actionExit_triggered()));
	QObject::connect(MainWindow, SIGNAL(closed()),
			 this, SLOT(on_actionExit_triggered()));
	QObject::connect(MainWindow, SIGNAL(destroyed()),
			 this, SLOT(on_actionExit_triggered()));
	
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
	retranslateScreenMenu();
	retranslateCartMenu(0, 1);
	retranslateCartMenu(1, 2);
	retranslateBinaryMenu(0, 1);
	retranslateBinaryMenu(1, 2);
   
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));

	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));
	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));
  
	menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
} // retranslateUi

QT_END_NAMESPACE
