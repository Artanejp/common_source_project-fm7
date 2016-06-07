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
#include "mainwidget_base.h"
//#include "menuclasses.h"
#include "menu_disk.h"
#include "menu_cmt.h"
#include "menu_cart.h"
#include "menu_quickdisk.h"
#include "menu_binary.h"
#include "menu_compactdisc.h"
#include "menu_bubble.h"

#include "qt_gldraw.h"
//#include "emu.h"
#include "qt_main.h"
#include "menu_flags.h"

extern EMU *emu;
extern USING_FLAGS *using_flags;

Ui_MainWindowBase::Ui_MainWindowBase(QWidget *parent) : QMainWindow(parent)
{
	setupUi();
	createContextMenu();
}

Ui_MainWindowBase::~Ui_MainWindowBase()
{
	delete using_flags;
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


void Ui_MainWindowBase::do_show_about(void)
{
	Dlg_AboutCSP *dlg = new Dlg_AboutCSP;
	dlg->show();
}

void Ui_MainWindowBase::do_browse_document(QString fname)
{
	Dlg_BrowseText *dlg = new Dlg_BrowseText(fname);
	dlg->show();
}


void Ui_MainWindowBase::setupUi(void)
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
	if(!using_flags->is_without_sound()) {
		ConfigSoundMenu();
	}
	if(using_flags->is_use_binary_file()) {
		ConfigBinaryMenu(); 
	}
	if(using_flags->is_use_qd()) {
		ConfigQuickDiskMenu();
	}
	ConfigScreenMenu();
	if(using_flags->is_use_cart()) {
		ConfigCartMenu();
	}
	if(using_flags->is_use_compact_disc()) {
		ConfigCDROMMenu();
	}
	if(using_flags->is_use_bubble()) {
		ConfigBubbleMenu();
	}
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
   
	bitmapImage = NULL;
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
	if(using_flags->is_use_auto_key()) {
		menuCopy_Paste = new QMenu(menuControl);
		menuCopy_Paste->setObjectName(QString::fromUtf8("menuCopy_Paste"));
	}
	menuCpu_Speed = new QMenu(menuControl);
	menuCpu_Speed->setObjectName(QString::fromUtf8("menuCpu_Speed"));
	menuDebugger = new QMenu(menuControl);
	menuDebugger->setObjectName(QString::fromUtf8("menuDebugger"));
	if(using_flags->is_use_fd()) {
		int i;
		for(i = 0; i < using_flags->get_max_drive(); i++) CreateFloppyMenu(i, i + 1);
	}
	if(using_flags->is_use_qd()) {
		int i;
		for(i = 0; i < using_flags->get_max_qd(); i++) CreateQuickDiskMenu(i, i + 1);
	}
	if(using_flags->is_use_tape()) {
		CreateCMTMenu();
	}
	CreateScreenMenu();
	if(using_flags->is_use_cart()) {
		int i;
		for(i = 0; i < using_flags->get_max_cart(); i++) {
			CreateCartMenu(i, i + 1);
		}
	}
	if(using_flags->is_use_binary_file()) {
		int i;
		for(i = 0; i < using_flags->get_max_binary(); i++) {
			CreateBinaryMenu(i, i + 1);
		}
	}
	if(using_flags->is_use_compact_disc()) {
		CreateCDROMMenu();
	}
	if(using_flags->is_use_bubble()) {
		int i;
		for(i = 0; i < using_flags->get_max_bubble(); i++) {
			CreateBubbleMenu(i, i + 1);
		}
	}
	connect(this, SIGNAL(sig_update_screen(void)), graphicsView, SLOT(update(void)));
	//connect(this, SIGNAL(sig_update_screen(void)), graphicsView, SLOT(updateGL(void)));

	menuMachine = new QMenu(menubar);
	menuMachine->setObjectName(QString::fromUtf8("menuMachine"));

	if(using_flags->is_use_mouse()) {
		actionMouseEnable = new Action_Control(this);
		actionMouseEnable->setCheckable(true);
		actionMouseEnable->setVisible(true);
		actionMouseEnable->setChecked(false);
		menuMachine->addAction(actionMouseEnable);
		connect(actionMouseEnable, SIGNAL(toggled(bool)),
				this, SLOT(do_set_mouse_enable(bool)));
		connect(graphicsView, SIGNAL(sig_check_grab_mouse(bool)),
				actionMouseEnable, SLOT(do_check_grab_mouse(bool)));
	}

	ConfigDeviceType();
	ConfigDriveType();
	ConfigSoundDeviceType();
	ConfigPrinterType();

	if(!using_flags->is_without_sound()) {
		menuSound = new QMenu(menubar);
		menuSound->setObjectName(QString::fromUtf8("menuSound"));
	}
	menuEmulator = new QMenu(menubar);
	menuEmulator->setObjectName(QString::fromUtf8("menuEmulator"));
	
	menuHELP = new QMenu(menubar);
	menuHELP->setObjectName(QString::fromUtf8("menuHELP"));
	MainWindow->setMenuBar(menubar);

	menubar->addAction(menuControl->menuAction());
	connectActions_ControlMenu();
	if(using_flags->is_use_fd()) {
		int i;
		for(i = 0; i < using_flags->get_max_drive(); i++) {
			menubar->addAction(menu_fds[i]->menuAction());
		}
	}
	if(using_flags->is_use_qd()) {
		int i;
		for(i = 0; i < using_flags->get_max_qd(); i++) {
			menubar->addAction(menu_QDs[i]->menuAction());
		}
	}
	if(using_flags->is_use_tape()) {
		menubar->addAction(menu_CMT->menuAction());
	}
	if(using_flags->is_use_cart()) {
		int i;
		for(i = 0; i < using_flags->get_max_cart(); i++) {
			menubar->addAction(menu_Cart[i]->menuAction());
		}
	}
	if(using_flags->is_use_binary_file()) {
		int i;
		for(i = 0; i < using_flags->get_max_binary(); i++) {
			menubar->addAction(menu_BINs[i]->menuAction());
		}
	}
	if(using_flags->is_use_compact_disc()) {
		menubar->addAction(menu_CDROM->menuAction());
	}
//	if(using_flags->is_use_laser_disc()) {
//		menubar->addAction(menu_LaserDisc->menuAction());
//	}
	if(using_flags->is_use_bubble()) {
		int i;
		for(i = 0; i < using_flags->get_max_bubble(); i++) {
			menubar->addAction(menu_bubbles[i]->menuAction());
		}
	}
	menubar->addAction(menuMachine->menuAction());
	if(!using_flags->is_without_sound()) {
		menubar->addAction(menuSound->menuAction());
	}

	menubar->addAction(menuScreen->menuAction());
//	menubar->addAction(menuRecord->menuAction());
	menubar->addAction(menuEmulator->menuAction());
	menubar->addAction(menuHELP->menuAction());
	if(using_flags->is_use_qd()) {
		int i;
		for(i = 0; i < using_flags->get_max_qd(); i++) {
			CreateQuickDiskPulldownMenu(i);
		}
	}
	if(using_flags->is_use_binary_file()) {
		int i;
		for(i = 0; i < using_flags->get_max_binary(); i++) {
			CreateBinaryPulldownMenu(1);
		}
	}
	if(!using_flags->is_without_sound()) {
		CreateSoundMenu();
	}
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
	
	actionHelp_README_BIOS = new Action_Control(this);
	actionHelp_README_BIOS->setObjectName(QString::fromUtf8("menuHelp_README_BIOS"));
	actionHelp_README_BIOS->do_set_string(QString::fromUtf8("bios_and_keys.txt"));
	connect(actionHelp_README_BIOS, SIGNAL(triggered()), actionHelp_README_BIOS, SLOT(do_send_string()));
	connect(actionHelp_README_BIOS, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Readme->addAction(actionHelp_README_BIOS);
	menuHelp_Readme->addSeparator();
	
	actionHelp_README_MR_TANAM = new Action_Control(this);
	actionHelp_README_MR_TANAM->setObjectName(QString::fromUtf8("menuHelp_README_MR_TANAM"));
	actionHelp_README_MR_TANAM->do_set_string(QString::fromUtf8("readme_by_mr_tanam.txt"));
	connect(actionHelp_README_MR_TANAM, SIGNAL(triggered()), actionHelp_README_MR_TANAM, SLOT(do_send_string()));
	connect(actionHelp_README_MR_TANAM, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Readme->addAction(actionHelp_README_MR_TANAM);
	
	menuHelp_Readme->addSeparator();
	
	actionHelp_README_FAQ = new Action_Control(this);
	actionHelp_README_FAQ->setObjectName(QString::fromUtf8("menuHelp_README_FAQ"));
	actionHelp_README_FAQ->do_set_string(QString::fromUtf8("FAQ.html"));
	connect(actionHelp_README_FAQ, SIGNAL(triggered()), actionHelp_README_FAQ, SLOT(do_send_string()));
	connect(actionHelp_README_FAQ, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Readme->addAction(actionHelp_README_FAQ);

	actionHelp_README_FAQ_JP = new Action_Control(this);
	actionHelp_README_FAQ_JP->setObjectName(QString::fromUtf8("menuHelp_README_FAQ_JP"));
	actionHelp_README_FAQ_JP->do_set_string(QString::fromUtf8("FAQ.ja.html"));
	connect(actionHelp_README_FAQ_JP, SIGNAL(triggered()), actionHelp_README_FAQ_JP, SLOT(do_send_string()));
	connect(actionHelp_README_FAQ_JP, SIGNAL(sig_send_string(QString)), this, SLOT(do_browse_document(QString)));
	menuHelp_Readme->addAction(actionHelp_README_FAQ_JP);
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
	if(config.window_mode >= using_flags->get_screen_mode_num()) config.window_mode = using_flags->get_screen_mode_num() - 1;
	w = using_flags->get_screen_width();
	h = using_flags->get_screen_height();
	if(actionScreenSize[config.window_mode] != NULL) {
		double nd = actionScreenSize[config.window_mode]->binds->getDoubleValue();
		w = (int)(nd * (double)w);
		h = (int)(nd * (double)h);
		if(using_flags->is_use_screen_rotate()) {
			if(config.rotate_type) {
				int tmp_w = w;
				w = h;
				h = tmp_w;
			}
		}
	} else {
		if(using_flags->is_use_screen_rotate()) {
			if(config.rotate_type) {
				w = 600;
				h = 960;
			} else {		   
				w = 1280;
				h = 800;
			}
		} else {
			w = 1280;
			h = 800;
		}
	}
	graphicsView->setFixedSize(w, h);
	for(int i = 0; i < using_flags->get_screen_mode_num(); i++) {
		if(actionScreenSize[i] != NULL) {
			connect(actionScreenSize[i]->binds, SIGNAL(sig_screen_multiply(float)),
				graphicsView, SLOT(do_set_screen_multiply(float)));
		}
	}
	this->set_screen_size(w, h);
	this->set_screen_aspect(config.window_stretch_type);
	if(actionScreenSize[config.window_mode] != NULL) {
		double nd = actionScreenSize[config.window_mode]->binds->getDoubleValue();
		graphicsView->do_set_screen_multiply(nd);
	}
	if(using_flags->is_use_joystick()) {
		connect(action_SetupJoystick, SIGNAL(triggered()), this, SLOT(rise_joystick_dialog()));
	}
	connect(action_SetupKeyboard, SIGNAL(triggered()), this, SLOT(rise_keyboard_dialog()));
	   
	QImageReader reader(":/default.ico");
	QImage result = reader.read();

	MainWindow->setWindowIcon(QPixmap::fromImage(result));
	this->set_window_title();
//	QIcon WindowIcon;
	InsertIcon = QApplication::style()->standardIcon(QStyle::SP_FileDialogStart);
	EjectIcon  = QIcon(":/icon_eject.png");
	StopIcon = QIcon(":/icon_process_stop.png");
	RecordSoundIcon = QIcon(":/icon_record_to_wav.png");
	ResetIcon = QApplication::style()->standardIcon(QStyle::SP_BrowserReload);

	VolumeMutedIcon = QIcon(":/icon_volume_muted.png");
	VolumeLowIcon = QIcon(":/icon_volume_low.png");
	VolumeMidIcon = QIcon(":/icon_volume_mid.png");
	VolumeHighIcon = QIcon(":/icon_volume_high.png");
	
	ExitIcon = QIcon(":/icon_exit.png");

	QMetaObject::connectSlotsByName(MainWindow);
} // setupUi

// Emulator
#include "dropdown_joystick.h"
#include "dialog_set_key.h"

void Ui_MainWindowBase::retranslateEmulatorMenu(void)
{
	int i;
	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	if(using_flags->is_use_joystick()) {
		action_SetupJoystick->setText(QApplication::translate("MainWindow", "Configure Joysticks", 0));
		action_SetupJoystick->setIcon(QIcon(":/icon_gamepad.png"));
	}
	action_SetupKeyboard->setText(QApplication::translate("MainWindow", "Configure Keyboard", 0));
	action_SetupKeyboard->setIcon(QIcon(":/icon_keyboard.png"));
	action_SetupMovie->setText(QApplication::translate("MainWindow", "Configure movie encoding", 0));
	
}

void Ui_MainWindowBase::CreateEmulatorMenu(void)
{
	if(using_flags->is_use_joystick()) {
		menuEmulator->addAction(action_SetupJoystick);
	}
	menuEmulator->addAction(action_SetupKeyboard);
	menuEmulator->addAction(action_SetupMovie);
}

void Ui_MainWindowBase::ConfigEmulatorMenu(void)
{
	int i;
	QString tmps;
	if(using_flags->is_use_joystick()) {
		action_SetupJoystick = new Action_Control(this);
	}
	action_SetupKeyboard = new Action_Control(this);

	action_SetupMovie = new Action_Control(this);
  
}

void Ui_MainWindowBase::rise_joystick_dialog(void)
{
	if(graphicsView != NULL) {
		QStringList *lst = graphicsView->getVKNames();
		CSP_DropDownJoysticks *dlg = new CSP_DropDownJoysticks(NULL, lst);
		dlg->setWindowTitle(QApplication::translate("CSP_DropDownJoysticks", "Configure Joysticks", 0));
		dlg->show();
	}
}

void Ui_MainWindowBase::rise_movie_dialog(void)
{

}

void Ui_MainWindowBase::rise_keyboard_dialog(void)
{
	if(graphicsView != NULL) {
		CSP_KeySetDialog *dlg = new CSP_KeySetDialog(NULL, graphicsView);
		dlg->setWindowTitle(QApplication::translate("KeySetDialog", "Configure Keyboard", 0));
		dlg->show();
	}
}
// Retranslate
void Ui_MainWindowBase::retranslateUI_Help(void)
{
	menuHELP->setTitle(QApplication::translate("MainWindow", "Help", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0));
	actionHelp_AboutQt->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarMenuButton));
	
	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));
	actionAbout->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion));

	menuHelp_Readme->setTitle(QApplication::translate("MainWindow", "READMEs", 0));
	
	actionHelp_README->setText(QApplication::translate("MainWindow", "General Document", 0));
	actionHelp_README_QT->setText(QApplication::translate("MainWindow", "About Qt ports", 0));
	actionHelp_README_Artane->setText(QApplication::translate("MainWindow", "About Qt ports (Japanese).", 0));
	actionHelp_README_MR_TANAM->setText(QApplication::translate("MainWindow", "By Mr. tanam", 0));
	actionHelp_README_FM7->setText(QApplication::translate("MainWindow", "About eFM-7/8/77/AV.", 0));
	actionHelp_README_FM7_JP->setText(QApplication::translate("MainWindow", "About eFM-7/8/77/AV (Japanese).", 0));
	actionHelp_README_FAQ->setText(QApplication::translate("MainWindow", "FAQs(English)", 0));
	actionHelp_README_FAQ_JP->setText(QApplication::translate("MainWindow", "FAQs(Japanese)", 0));
	actionHelp_README_BIOS->setText(QApplication::translate("MainWindow", "BIOS and Key assigns", 0));

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
void Ui_MainWindowBase::retranslateMachineMenu(void)
{
	int i;
	QString tmps;
	QString tmps2;
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));
	if(using_flags->get_use_device_type() > 0) {
		menuDeviceType->setTitle(QApplication::translate("MainWindow", "Device Type", 0));
		for(i = 0; i < using_flags->get_use_device_type(); i++) {
			tmps2.setNum(i + 1);
			tmps = QString::fromUtf8("Machine Device ") + tmps2;
			actionDeviceType[i]->setText(tmps); 
		}
	}
	if(using_flags->get_use_sound_device_type() > 0) {
		menuSoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Cards", 0));
		for(i = 0; i < using_flags->get_use_sound_device_type(); i++) {
			tmps2.setNum(i + 1);
			tmps = QString::fromUtf8("Sound Device ") + tmps2;
			actionSoundDevice[i]->setText(tmps); 
		}
	}
	if(using_flags->get_use_drive_type() > 0) {
		menuDriveType->setTitle(QApplication::translate("MainWindow", "Drive Type", 0));
		for(i = 0; i < using_flags->get_use_drive_type(); i++) {
			tmps2.setNum(i + 1);
			tmps = QString::fromUtf8("Drive Type ") + tmps2;
			actionDriveType[i]->setText(tmps); 
		}
	}
	if(using_flags->is_use_printer()) {
		menuPrintDevice->setTitle(QApplication::translate("MainWindow", "Printer (Need RESET)", 0));
		i = 1;
		actionPrintDevice[0]->setText(QApplication::translate("MainWindow", "Dump to File", 0));
		if(using_flags->get_use_printer_type() > 0) {
			for(i = 1; i < (using_flags->get_use_printer_type() - 1); i++) {
				tmps2.setNum(i + 1);
				tmps = QApplication::translate("MainWindow", "Printer", 0) + tmps2;
				actionPrintDevice[i]->setText(tmps); 
			}
		}
		actionPrintDevice[i]->setText(QApplication::translate("MainWindow", "Not Connect", 0));
	}
}
void Ui_MainWindowBase::retranslateUi(void)
{
	retranslateControlMenu("NMI Reset",  true);
	retranslateFloppyMenu(0, 0);
	retranslateFloppyMenu(1, 1);
	retranslateCMTMenu();
	if(!using_flags->is_without_sound()) {
		retranslateSoundMenu();
	}
	retranslateScreenMenu();
	retranslateCartMenu(0, 1);
	retranslateCartMenu(1, 2);
	retranslateCDROMMenu();
	
	retranslateBinaryMenu(0, 1);
	retranslateBinaryMenu(1, 2);

	retranslateBubbleMenu(0, 1);
	retranslateBubbleMenu(1, 2);
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
   
} // retranslateUi

void Ui_MainWindowBase::setCoreApplication(QApplication *p)
{
	this->CoreApplication = p;
	connect(actionExit_Emulator, SIGNAL(triggered()),
			this->CoreApplication, SLOT(closeAllWindows())); // OnGuiExit()?  
	connect(actionHelp_AboutQt, SIGNAL(triggered()),
			this->CoreApplication, SLOT(aboutQt()));
	
}

#include <string>
// Move from common/qt_main.cpp
// menu
extern std::string cpp_homedir;
extern std::string cpp_confdir;
extern std::string my_procname;
extern bool now_menuloop;
// timing control

// screen
extern unsigned int desktop_width;
extern unsigned int desktop_height;
//int desktop_bpp;
extern int prev_window_mode;
extern bool now_fullscreen;
extern int window_mode_count;

void Ui_MainWindowBase::set_window(int mode)
{
	//	static LONG style = WS_VISIBLE;
}

void Ui_MainWindowBase::do_emu_update_volume_level(int num, int level)
{
	emit sig_emu_update_volume_level(num, level);
}

void Ui_MainWindowBase::do_emu_update_volume_balance(int num, int level)
{
	emit sig_emu_update_volume_balance(num, level);
}

void Ui_MainWindowBase::do_emu_update_config(void)
{
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::doChangeMessage_EmuThread(QString message)
{
      emit message_changed(message);
}



void Ui_MainWindowBase::StopEmuThread(void)
{
	emit quit_emu_thread();
}

void Ui_MainWindowBase::delete_emu_thread(void)
{
	//do_release_emu_resources();
	emit sig_quit_all();
}  
// Utility
#include <QTextCodec>
#include <QString>
#include <QByteArray>

void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit, int i_limit)
{
	QTextCodec *srcCodec = QTextCodec::codecForName( "SJIS" );
	QTextCodec *dstCodec = QTextCodec::codecForName( "UTF-8" );
	QString dst_b;
	QByteArray dst_s;
	if(src == NULL) {
		if(dst != NULL) dst[0] = '\0';
		return;
	}
	if(dst == NULL) return;
	dst_b = srcCodec->toUnicode(src, strlen(src));
	dst_s = dstCodec->fromUnicode(dst_b);
	if(n_limit > 0) {
		memset(dst, 0x00, n_limit);
		strncpy(dst, dst_s.constData(), n_limit - 1);
	}
}

void Ui_MainWindowBase::set_window_title()
{
   	QString tmps;
	tmps = QString::fromUtf8("emu");
	tmps = tmps + using_flags->get_config_name();
	tmps = tmps + QString::fromUtf8(" (");
	tmps = tmps + using_flags->get_device_name();
	tmps = tmps + QString::fromUtf8(")");
	MainWindow->setWindowTitle(tmps);
}

void Ui_MainWindowBase::do_set_window_title(QString s)
{
	QString tmps;
	tmps = QString::fromUtf8("emu");
	tmps = tmps + using_flags->get_config_name();
	tmps = tmps + QString::fromUtf8(" (");
	if(!s.isEmpty()) {
		tmps = tmps + s;
	}
	tmps = tmps + QString::fromUtf8(")");
	MainWindow->setWindowTitle(tmps);
}

void Ui_MainWindowBase::do_set_mouse_enable(bool flag)
{
}

void Ui_MainWindowBase::do_toggle_mouse(void)
{
}

void Ui_MainWindowBase::LaunchEmuThread(void)
{
}

void Ui_MainWindowBase::LaunchJoyThread(void)
{
}

void Ui_MainWindowBase::StopJoyThread(void)
{
}

void Ui_MainWindowBase::delete_joy_thread(void)
{
}

void Ui_MainWindowBase::on_actionExit_triggered()
{
	OnMainWindowClosed();
}

void Ui_MainWindowBase::OnWindowMove(void)
{
}

void Ui_MainWindowBase::OnWindowResize(void)
{
	if(emu) {
		set_window(config.window_mode);
	}
}

void Ui_MainWindowBase::OnWindowRedraw(void)
{
}

bool Ui_MainWindowBase::GetPowerState(void)
{
	return true;
}

void Ui_MainWindowBase::OnMainWindowClosed(void)
{
}


void Ui_MainWindowBase::do_release_emu_resources(void)
{
}

void Ui_MainWindowBase::OnOpenDebugger(int no)
{
}

void Ui_MainWindowBase::OnCloseDebugger(void )
{
}
