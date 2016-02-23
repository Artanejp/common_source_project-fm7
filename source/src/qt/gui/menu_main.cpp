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

#include "commonclasses.h"
#include "display_about.h"
#include "display_text_document.h"
#include "mainwidget.h"
#include "menuclasses.h"
#include "menu_disk.h"
#include "menu_cmt.h"
#include "menu_cart.h"
#include "menu_quickdisk.h"
#include "menu_binary.h"

#include "qt_gldraw.h"
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
	graphicsView->releaseKeyboard();
}


void Action_Control::do_check_grab_mouse(bool flag)
{
	this->toggle();
}

void Action_Control::do_send_string(void)
{
	emit sig_send_string(bindString);
}

void Action_Control::do_set_string(QString s)
{
	bindString = s;
}


void Ui_MainWindow::do_show_about(void)
{
	Dlg_AboutCSP *dlg = new Dlg_AboutCSP;
	dlg->show();
}

void Ui_MainWindow::do_browse_document(QString fname)
{
	Dlg_BrowseText *dlg = new Dlg_BrowseText(fname);
	dlg->show();
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
#if !defined(WITHOUT_SOUND)	
	ConfigSoundMenu();
#endif	
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
	ConfigEmulatorMenu();	
	actionAbout = new Action_Control(this);
	actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
   
	graphicsView = new GLDrawClass(this);
	graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
	graphicsView->setMaximumSize(2560, 2560); // ?
	graphicsView->setMinimumSize(240, 192); // ?
	//graphicsView->grabKeyboard();
	graphicsView->setAttribute(Qt::WA_InputMethodEnabled, false); // Disable [Zenkaku / Hankaku] with IM.
	graphicsView->setAttribute(Qt::WA_KeyboardFocusChange, false); 
	//graphicsView->setFocusPolicy(Qt::StrongFocus);
	//this->setFocusPolicy(Qt::ClickFocus);
   
#if defined(USE_BITMAP)
	bitmapImage = NULL;
#endif   
	MainWindow->setCentralWidget(graphicsView);
	MainWindow->setFocusProxy(graphicsView);
	
	MainWindow->centralWidget()->adjustSize();
	MainWindow->adjustSize();

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
#ifdef USE_AUTO_KEY
	menuCopy_Paste = new QMenu(menuControl);
	menuCopy_Paste->setObjectName(QString::fromUtf8("menuCopy_Paste"));
#endif	
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
	
	connect(this, SIGNAL(sig_update_screen(void)), graphicsView, SLOT(update(void)));
	//connect(this, SIGNAL(sig_update_screen(void)), graphicsView, SLOT(updateGL(void)));

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
	
	ConfigDeviceType();
	ConfigDriveType();
	ConfigSoundDeviceType();
	ConfigPrinterType();

#if !defined(WITHOUT_SOUND)	
	menuSound = new QMenu(menubar);
	menuSound->setObjectName(QString::fromUtf8("menuSound"));
#endif
	menuEmulator = new QMenu(menubar);
	menuEmulator->setObjectName(QString::fromUtf8("menuEmulator"));
	
	menuHELP = new QMenu(menubar);
	menuHELP->setObjectName(QString::fromUtf8("menuHELP"));
	MainWindow->setMenuBar(menubar);

	menubar->addAction(menuControl->menuAction());
	connectActions_ControlMenu();
#if defined(USE_FD1)
	menubar->addAction(menu_fds[0]->menuAction());
#endif
#if defined(USE_FD2)
	menubar->addAction(menu_fds[1]->menuAction());
#endif
#if defined(USE_FD3)
	menubar->addAction(menu_fds[2]->menuAction());
#endif
#if defined(USE_FD4)
	menubar->addAction(menu_fds[3]->menuAction());
#endif
#if defined(USE_FD5)
	menubar->addAction(menu_fds[4]->menuAction());
#endif
#if defined(USE_FD6)
	menubar->addAction(menu_fds[5]->menuAction());
#endif
#if defined(USE_FD7)
	menubar->addAction(menu_fds[6]->menuAction());
#endif
#if defined(USE_FD8)
	menubar->addAction(menu_fds[7]->menuAction());
#endif
#if defined(USE_QD1)
   	menubar->addAction(menu_QDs[0]->menuAction());
#endif
#if defined(USE_QD2)
   	menubar->addAction(menu_QDs[1]->menuAction());
#endif
#if defined(USE_TAPE)
	menubar->addAction(menu_CMT->menuAction());
#endif
#if defined(USE_CART1)
	menubar->addAction(menu_Cart[0]->menuAction());
#endif
#if defined(USE_CART2)
	menubar->addAction(menu_Cart[1]->menuAction());
#endif
#if defined(USE_BINARY_FILE1)
	menubar->addAction(menu_BINs[0]->menuAction());
#endif
#if defined(USE_BINARY_FILE2)
	menubar->addAction(menu_BINs[1]->menuAction());
#endif
	menubar->addAction(menuMachine->menuAction());
	
#if !defined(WITHOUT_SOUND)	
	menubar->addAction(menuSound->menuAction());
#endif   
	menubar->addAction(menuScreen->menuAction());
//	menubar->addAction(menuRecord->menuAction());
	menubar->addAction(menuEmulator->menuAction());
	menubar->addAction(menuHELP->menuAction());
#if defined(USE_QD1)
	CreateQuickDiskPulldownMenu(0);
#endif
#if defined(USE_QD2)
	CreateQuickDiskPulldownMenu(1);
#endif
#if defined(USE_BINARY_FILE1)
	CreateBinaryPulldownMenu(0);
#endif
#if defined(USE_BINARY_FILE2)
	CreateBinaryPulldownMenu(1);
#endif
#if !defined(WITHOUT_SOUND)	
	CreateSoundMenu();
#endif
	CreateEmulatorMenu();
  
	menuHELP->addAction(actionAbout);
	connect(actionAbout, SIGNAL(triggered()), this, SLOT(do_show_about()));
	menuHELP->addSeparator();
	
	actionHelp_AboutQt = new Action_Control(this);
	actionHelp_AboutQt->setObjectName(QString::fromUtf8("menuHelp_AboutQt"));
	menuHELP->addAction(actionHelp_AboutQt);
	menuHELP->addSeparator();
	menuHelp_Readme = new QMenu(menuHELP);
	menuHelp_Readme->setObjectName(QString::fromUtf8("menuHelp_Readme_menu"));;
	menuHELP->addAction(menuHelp_Readme->menuAction());

	actionHelp_README = new Action_Control(this);
	actionHelp_README->setObjectName(QString::fromUtf8("menuHelp_README"));
	actionHelp_README->do_set_string(QString::fromUtf8("readme.txt"));
	connect(actionHelp_README, SIGNAL(triggered()), actionHelp_README, SLOT(do_send_string()));
	connect(actionHelp_README, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Readme->addAction(actionHelp_README);
	
	actionHelp_README_QT = new Action_Control(this);
	actionHelp_README_QT->setObjectName(QString::fromUtf8("menuHelp_README_QT"));
	actionHelp_README_QT->do_set_string(QString::fromUtf8("readme.qt.txt"));
	menuHelp_Readme->addAction(actionHelp_README_QT);
	connect(actionHelp_README_QT, SIGNAL(triggered()), actionHelp_README_QT, SLOT(do_send_string()));
	connect(actionHelp_README_QT, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	actionHelp_README_Artane = new Action_Control(this);
	actionHelp_README_Artane->setObjectName(QString::fromUtf8("menuHelp_README_Artane"));
	actionHelp_README_Artane->do_set_string(QString::fromUtf8("readme_by_artane.txt"));
	connect(actionHelp_README_Artane, SIGNAL(triggered()), actionHelp_README_Artane, SLOT(do_send_string()));
	connect(actionHelp_README_Artane, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Readme->addAction(actionHelp_README_Artane);
	menuHelp_Readme->addSeparator();
	
	actionHelp_README_MR_TANAM = new Action_Control(this);
	actionHelp_README_MR_TANAM->setObjectName(QString::fromUtf8("menuHelp_README_MR_TANAM"));
	actionHelp_README_MR_TANAM->do_set_string(QString::fromUtf8("readme_by_mr_tanam.txt"));
	connect(actionHelp_README_MR_TANAM, SIGNAL(triggered()), actionHelp_README_MR_TANAM, SLOT(do_send_string()));
	connect(actionHelp_README_MR_TANAM, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Readme->addAction(actionHelp_README_MR_TANAM);
	
	menuHelp_Readme->addSeparator();
	
	actionHelp_README_FM7 = new Action_Control(this);
	actionHelp_README_FM7->setObjectName(QString::fromUtf8("menuHelp_README_FM7"));
	actionHelp_README_FM7->do_set_string(QString::fromUtf8("readme_fm7.txt"));
	connect(actionHelp_README_FM7, SIGNAL(triggered()), actionHelp_README_FM7, SLOT(do_send_string()));
	connect(actionHelp_README_FM7, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Readme->addAction(actionHelp_README_FM7);
	
	actionHelp_README_FM7_JP = new Action_Control(this);
	actionHelp_README_FM7_JP->setObjectName(QString::fromUtf8("menuHelp_README_FM7_JP"));
	actionHelp_README_FM7_JP->do_set_string(QString::fromUtf8("readme_fm7.jp.txt"));
	connect(actionHelp_README_FM7_JP, SIGNAL(triggered()), actionHelp_README_FM7_JP, SLOT(do_send_string()));
	connect(actionHelp_README_FM7_JP, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Readme->addAction(actionHelp_README_FM7_JP);

	menuHelp_Histories = new QMenu(menuHELP);
	menuHelp_Histories->setObjectName(QString::fromUtf8("menuHelp_Histories"));;
	menuHELP->addAction(menuHelp_Histories->menuAction());

	actionHelp_History = new Action_Control(this);
	actionHelp_History->setObjectName(QString::fromUtf8("menuHelp_History"));
	actionHelp_History->do_set_string(QString::fromUtf8("history.txt"));
	connect(actionHelp_History, SIGNAL(triggered()), actionHelp_History, SLOT(do_send_string()));
	connect(actionHelp_History, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Histories->addAction(actionHelp_History);
	
	actionHelp_History_Relnote = new Action_Control(this);
	actionHelp_History_Relnote->setObjectName(QString::fromUtf8("menuHelp_History_Relnote"));
	actionHelp_History_Relnote->do_set_string(QString::fromUtf8("RELEASENOTE.txt"));
	connect(actionHelp_History_Relnote, SIGNAL(triggered()), actionHelp_History_Relnote, SLOT(do_send_string()));
	connect(actionHelp_History_Relnote, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Histories->addAction(actionHelp_History_Relnote);
	
	actionHelp_History_ChangeLog = new Action_Control(this);
	actionHelp_History_ChangeLog->setObjectName(QString::fromUtf8("menuHelp_History_ChangeLog"));
	actionHelp_History_ChangeLog->do_set_string(QString::fromUtf8("ChangeLog.txt"));
	connect(actionHelp_History_ChangeLog, SIGNAL(triggered()), actionHelp_History_ChangeLog, SLOT(do_send_string()));
	connect(actionHelp_History_ChangeLog, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Histories->addAction(actionHelp_History_ChangeLog);

	actionHelp_History_MR_TANAM = new Action_Control(this);
	actionHelp_History_MR_TANAM->setObjectName(QString::fromUtf8("menuHelp_History_MR_TANAM"));
	actionHelp_History_MR_TANAM->do_set_string(QString::fromUtf8("history_by_mr_tanam.txt"));
	connect(actionHelp_History_MR_TANAM, SIGNAL(triggered()), actionHelp_History_MR_TANAM, SLOT(do_send_string()));
	connect(actionHelp_History_MR_TANAM, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Histories->addAction(actionHelp_History_MR_TANAM);
	
	actionHelp_License = new Action_Control(this);
	actionHelp_License->setObjectName(QString::fromUtf8("menuHelp_License"));
	actionHelp_License->do_set_string(QString::fromUtf8("LICENSE.txt"));
	connect(actionHelp_License, SIGNAL(triggered()), actionHelp_License, SLOT(do_send_string()));
	connect(actionHelp_License, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHELP->addAction(actionHelp_License);
	
	actionHelp_License_JP = new Action_Control(this);
	actionHelp_License_JP->setObjectName(QString::fromUtf8("menuHelp_License_JP"));
	actionHelp_License_JP->do_set_string(QString::fromUtf8("LICENSE.ja.txt"));
	connect(actionHelp_License_JP, SIGNAL(triggered()), actionHelp_License_JP, SLOT(do_send_string()));
	connect(actionHelp_License_JP, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHELP->addAction(actionHelp_License_JP);
	
	if(config.window_mode <= 0) config.window_mode = 0;
	if(config.window_mode >= _SCREEN_MODE_NUM) config.window_mode = _SCREEN_MODE_NUM - 1;
	w = SCREEN_WIDTH;
	h = SCREEN_HEIGHT;
	if(actionScreenSize[config.window_mode] != NULL) {
		double nd = actionScreenSize[config.window_mode]->binds->getDoubleValue();
		w = (int)(nd * (double)w);
		h = (int)(nd * (double)h);
#if defined(USE_SCREEN_ROTATE)
		if(config.rotate_type) {
			int tmp_w = w;
			w = h;
			h = tmp_w;
		}
#endif	   
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
	for(int i = 0; i < _SCREEN_MODE_NUM; i++) {
		if(actionScreenSize[i] != NULL) {
			connect(actionScreenSize[i]->binds, SIGNAL(sig_screen_multiply(float)),
				graphicsView, SLOT(do_set_screen_multiply(float)));
		}
	}
	this->set_screen_size(w, h);
	this->set_screen_aspect(config.stretch_type);
	if(actionScreenSize[config.window_mode] != NULL) {
		double nd = actionScreenSize[config.window_mode]->binds->getDoubleValue();
		graphicsView->do_set_screen_multiply(nd);
	}
	connect(action_SetupJoystick, SIGNAL(triggered()), this, SLOT(rise_joystick_dialog()));
	   
	QImageReader reader(":/default.ico");
	QImage result = reader.read();
	MainWindow->setWindowIcon(QPixmap::fromImage(result));
	QMetaObject::connectSlotsByName(MainWindow);
} // setupUi


// Emulator
#include "dropdown_joystick.h"

void Ui_MainWindow::retranslateEmulatorMenu(void)
{
	action_SetupJoystick->setText(QApplication::translate("MainWindow", "Configure Joysticks", 0));
	//action_SetupKeyboard->setText(QApplication::translate("MainWindow", "Configure Joysticks", 0));
}
void Ui_MainWindow::CreateEmulatorMenu(void)
{
	menuEmulator->addAction(action_SetupJoystick);
}

void Ui_MainWindow::ConfigEmulatorMenu(void)
{
	action_SetupJoystick = new Action_Control(this);
}

void Ui_MainWindow::rise_joystick_dialog(void)
{
	if(graphicsView != NULL) {
		QStringList *lst = graphicsView->getVKNames();
		CSP_DropDownJoysticks *dlg = new CSP_DropDownJoysticks(NULL, lst);
		dlg->setWindowTitle(QApplication::translate("CSP_DropDownJoysticks", "Configure Joysticks", 0));
		dlg->show();
	}
}

// Retranslate
void Ui_MainWindow::retranslateUI_Help(void)
{
	menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0));
	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));

	menuHelp_Readme->setTitle(QApplication::translate("MainWindow", "READMEs", 0));
	
	actionHelp_README->setText(QApplication::translate("MainWindow", "General Document", 0));
	actionHelp_README_QT->setText(QApplication::translate("MainWindow", "About Qt ports", 0));
	actionHelp_README_Artane->setText(QApplication::translate("MainWindow", "About Qt ports (Japanese).", 0));
	actionHelp_README_MR_TANAM->setText(QApplication::translate("MainWindow", "By Mr. tanam", 0));
	actionHelp_README_FM7->setText(QApplication::translate("MainWindow", "About eFM-7/8/77/AV.", 0));
	actionHelp_README_FM7_JP->setText(QApplication::translate("MainWindow", "About eFM-7/8/77/AV (Japanese).", 0));

	menuHelp_Histories->setTitle(QApplication::translate("MainWindow", "Histories", 0));
	actionHelp_History->setText(QApplication::translate("MainWindow", "General History", 0));
	actionHelp_History_Relnote->setText(QApplication::translate("MainWindow", "Release Note", 0));
	actionHelp_History_ChangeLog->setText(QApplication::translate("MainWindow", "Change Log", 0));
	actionHelp_History_MR_TANAM->setText(QApplication::translate("MainWindow", "History by Tanam", 0));

	actionHelp_License->setText(QApplication::translate("MainWindow", "Show License", 0));
	actionHelp_License_JP->setText(QApplication::translate("MainWindow", "Show License (Japanese)", 0));
	
}

// You can Override this function: Re-define on foo/MainWindow.cpp.
// This code is example: by X1(TurboZ).
void Ui_MainWindow::retranslateMachineMenu(void)
{
	int i;
	QString tmps;
	QString tmps2;
#if defined(USE_DEVICE_TYPE)
	menuDeviceType->setTitle(QApplication::translate("MainWindow", "Device Type", 0));
	for(i = 0; i < USE_DEVICE_TYPE; i++) {
		tmps2.setNum(i + 1);
		tmps = QString::fromUtf8("Machine Device ") + tmps2;
		actionDeviceType[i]->setText(tmps); 
	}
#endif
#if defined(USE_SOUND_DEVICE_TYPE)
	menuSoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Cards", 0));
	for(i = 0; i < USE_SOUND_DEVICE_TYPE; i++) {
		tmps2.setNum(i + 1);
		tmps = QString::fromUtf8("Sound Device ") + tmps2;
		actionSoundDevice[i]->setText(tmps); 
	}
#endif
#if defined(USE_DRIVE_TYPE)
	menuDriveType->setTitle(QApplication::translate("MainWindow", "Drive Type", 0));
	for(i = 0; i < USE_DRIVE_TYPE; i++) {
		tmps2.setNum(i + 1);
		tmps = QString::fromUtf8("Drive Type ") + tmps2;
		actionDriveType[i]->setText(tmps); 
	}
#endif
#if defined(USE_PRINTER)
	menuPrintDevice->setTitle(QApplication::translate("MainWindow", "Printer (Need RESET)", 0));
	i = 1;
	actionPrintDevice[0]->setText(QApplication::translate("MainWindow", "Dump to File", 0));
  #if defined(USE_PRINTER_TYPE)
	for(i = 1; i < (USE_PRINTER_TYPE - 1); i++) {
		tmps2.setNum(i + 1);
		tmps = QApplication::translate("MainWindow", "Printer", 0) + tmps2;
		actionPrintDevice[i]->setText(tmps); 
	}
   #endif
	actionPrintDevice[i]->setText(QApplication::translate("MainWindow", "Not connect", 0));
#endif
}
void Ui_MainWindow::retranslateUi(void)
{
	retranslateControlMenu("NMI Reset",  true);
	retranslateFloppyMenu(0, 0);
	retranslateFloppyMenu(1, 1);
	retranslateCMTMenu();
#if !defined(WITHOUT_SOUND)	
	retranslateSoundMenu();
#endif   
	retranslateScreenMenu();
	retranslateCartMenu(0, 1);
	retranslateCartMenu(1, 2);
	retranslateBinaryMenu(0, 1);
	retranslateBinaryMenu(1, 2);
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
   
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));


	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));
} // retranslateUi

void Ui_MainWindow::setCoreApplication(QApplication *p)
{
	this->CoreApplication = p;
	connect(actionExit_Emulator, SIGNAL(triggered()),
			this->CoreApplication, SLOT(closeAllWindows())); // OnGuiExit()?  
	connect(actionHelp_AboutQt, SIGNAL(triggered()),
			this->CoreApplication, SLOT(aboutQt()));
	
}
QT_END_NAMESPACE
