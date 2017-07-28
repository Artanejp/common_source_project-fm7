/*
 * Common Source code Project:
 * Ui->Qt->gui->menu_main for generic.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, moved from qt/x1turboz/MainWindow.cpp .
 */

#include <QVariant>
#include <QtGui>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QPixmap>
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QToolBar>

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
#include "menu_laserdisc.h"
#include "menu_bubble.h"
#include "dock_disks.h"

#include "qt_gldraw.h"
//#include "emu.h"
#include "qt_main.h"
#include "menu_flags.h"
#include "csp_logger.h"

extern EMU *emu;
//extern USING_FLAGS *using_flags;
void DLL_PREFIX _resource_init(void)
{
	Q_INIT_RESOURCE(commontexts);
	Q_INIT_RESOURCE(shaders);
}

void DLL_PREFIX _resource_free(void)
{
	Q_CLEANUP_RESOURCE(shaders);
	Q_CLEANUP_RESOURCE(commontexts);
}

Ui_MainWindowBase::Ui_MainWindowBase(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : QMainWindow(parent)
{
	csp_logger = logger;
	using_flags = p;
	p_config = p->get_config_ptr();
	driveData = NULL;
	ledUpdateTimer = NULL;
	setupUi();
	createContextMenu();
	max_vm_nodes = 0;
	ui_retranslate_completed = false;
	//csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "GUI OK");
}

Ui_MainWindowBase::~Ui_MainWindowBase()
{
	graphicsView->releaseKeyboard();
	if(ledUpdateTimer != NULL) delete ledUpdateTimer;
	if(driveData != NULL) delete driveData;
	delete using_flags;
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

void Action_Control::do_select_render_platform(void)
{
	int num = this->binds->getValue1();
	emit sig_select_render_platform(num);
}

void Action_Control::do_set_dev_log_to_console(bool f)
{
	int num = this->binds->getValue1();
	emit sig_set_dev_log_to_console(num, f);
}

void Action_Control::do_set_dev_log_to_syslog(bool f)
{
	int num = this->binds->getValue1();
	emit sig_set_dev_log_to_syslog(num, f);
}

void Ui_MainWindowBase::do_set_roma_kana(bool flag)
{
	using_flags->get_config_ptr()->roma_kana_conversion = flag;
}

void Ui_MainWindowBase::do_show_about(void)
{
	Dlg_AboutCSP *dlg = new Dlg_AboutCSP(using_flags);
	dlg->show();
}

void Ui_MainWindowBase::do_browse_document(QString fname)
{
	Dlg_BrowseText *dlg = new Dlg_BrowseText(fname, using_flags);
	dlg->show();
}

void Ui_MainWindowBase::do_set_sound_files_fdd(bool f)
{
	if(f) {
		using_flags->get_config_ptr()->sound_noise_fdd = 1;
	} else {
		using_flags->get_config_ptr()->sound_noise_fdd = 0;
	}
}

void Ui_MainWindowBase::do_set_sound_files_relay(bool f)
{
	if(f) {
		using_flags->get_config_ptr()->sound_noise_cmt = 1;
	} else {
		using_flags->get_config_ptr()->sound_noise_cmt = 0;
	}
}


void Ui_MainWindowBase::do_set_conslog(bool f)
{
	using_flags->get_config_ptr()->log_to_console = f;
	csp_logger->set_log_stdout(-1, f);
}

void Ui_MainWindowBase::do_set_syslog(bool f)
{
	using_flags->get_config_ptr()->log_to_syslog = f;
	csp_logger->set_log_syslog(-1, f);
}

void Ui_MainWindowBase::do_update_device_node_name(int id, const _TCHAR *name)
{
	if(action_DevLogToConsole[id] == NULL) return;
	if(!ui_retranslate_completed) return;
	if(using_flags->get_vm_node_size() > id) {
		action_DevLogToConsole[id]->setEnabled(true);
		action_DevLogToConsole[id]->setVisible(true);
#if !defined(Q_OS_WIN)
		action_DevLogToSyslog[id]->setEnabled(true);
		action_DevLogToSyslog[id]->setVisible(true);
#endif
	} else {
		action_DevLogToConsole[id]->setEnabled(false);
		action_DevLogToConsole[id]->setVisible(false);
		
#if !defined(Q_OS_WIN)
		action_DevLogToSyslog[id]->setEnabled(false);
		action_DevLogToSyslog[id]->setVisible(false);
#endif
	}
	char s[64] = {0};
	snprintf(s, 60, "#%02d: %s", id, name);
	action_DevLogToConsole[id]->setText(QString::fromUtf8(s));
#if !defined(Q_OS_WIN)
	action_DevLogToSyslog[id]->setText(QString::fromUtf8(s));
#endif
}


void Ui_MainWindowBase::do_set_dev_log_to_console(int num, bool f)
{
	csp_logger->set_device_node_log(num, 2, CSP_LOG_DEBUG, f);
	using_flags->get_config_ptr()->dev_log_to_console[num][0] = f;
}

void Ui_MainWindowBase::do_set_dev_log_to_syslog(int num, bool f)
{
	csp_logger->set_device_node_log(num, 2, CSP_LOG_DEBUG, f);
	using_flags->get_config_ptr()->dev_log_to_syslog[num][0] = f;
}

void Ui_MainWindowBase::do_select_render_platform(int num)
{
	int _major = 0;
	int _minor = 0;
	int _type = -1;

	switch(num) {
	case RENDER_PLATFORMS_OPENGL3_MAIN:
		_type = CONFIG_RENDER_PLATFORM_OPENGL_MAIN;
		_major = 3;
		_minor = 0;
		break;
	case RENDER_PLATFORMS_OPENGL2_MAIN:
		_type = CONFIG_RENDER_PLATFORM_OPENGL_MAIN;
		_major = 2;
		_minor = 0;
		break;
	case RENDER_PLATFORMS_OPENGL_CORE:
		_type = CONFIG_RENDER_PLATFORM_OPENGL_CORE;
		_major = 3;
		_minor = 2;
		break;
	default:
		break;
	}
	if(_type >= 0) {
		using_flags->get_config_ptr()->render_platform = _type;
		using_flags->get_config_ptr()->render_major_version = _major;
		using_flags->get_config_ptr()->render_minor_version = _minor;
	}
}

void Ui_MainWindowBase::set_dipsw(int num, bool flag)
{
	if((num < 0) || (num >= 32)) return;
	if(flag) {
		using_flags->get_config_ptr()->dipswitch = using_flags->get_config_ptr()->dipswitch | (1 << num);
	} else {
		using_flags->get_config_ptr()->dipswitch = using_flags->get_config_ptr()->dipswitch & ~(1 << num);
	}
}

bool Ui_MainWindowBase::get_dipsw(int num)
{
	if((num < 0) || (num >= 32)) return false;
	if(((1 << num) & using_flags->get_config_ptr()->dipswitch) == 0) return false;
	return true;
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
	if(using_flags->is_use_laser_disc()) {
		ConfigLaserdiscMenu();
	}
	if(using_flags->is_use_bubble()) {
		ConfigBubbleMenu();
	}
	ConfigEmulatorMenu();	
	actionAbout = new Action_Control(this, using_flags);
	actionAbout->setObjectName(QString::fromUtf8("actionAbout"));

	{
#if defined(_USE_GLAPI_QT5_4)
		QSurfaceFormat fmt;
#else
		QGLFormat fmt;
#endif
		{
			int render_type = using_flags->get_config_ptr()->render_platform;
			//int _major_version = using_flags->get_config_ptr()->render_major_version;
			//int _minor_version = using_flags->get_config_ptr()->render_minor_version;
#if defined(_USE_GLAPI_QT5_4)
				if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_CORE) { 
					fmt.setProfile(QSurfaceFormat::CoreProfile); // Requires >=Qt-4.8.0
					csp_logger->debug_log(CSP_LOG_DEBUG,  CSP_LOG_TYPE_GENERAL, "Try to use OpenGL CORE profile.");
				} else { // Fallback
					fmt.setProfile(QSurfaceFormat::CompatibilityProfile); // Requires >=Qt-4.8.0
					csp_logger->debug_log(CSP_LOG_DEBUG,  CSP_LOG_TYPE_GENERAL, "Try to use OpenGL Compatible(MAIN) profile.");
				}
#else
				if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_CORE) { 
					fmt.setProfile(QGLFormat::CoreProfile); // Requires >=Qt-4.8.0
					csp_logger->debug_log(CSP_LOG_DEBUG,  CSP_LOG_TYPE_GENERAL, "Try to use OpenGL CORE profile.");
					fmt.setVersion(3, 2);
				} else {
					fmt.setProfile(QGLFormat::CompatibilityProfile); // Requires >=Qt-4.8.0
					csp_logger->debug_log(CSP_LOG_DEBUG,  CSP_LOG_TYPE_GENERAL, "Try to use OpenGL Compatible(MAIN) profile.");
					fmt.setVersion(3, 0);
				}					
#endif
		}
		graphicsView = new GLDrawClass(using_flags, csp_logger, this, fmt);
		graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
		graphicsView->setMaximumSize(2560, 2560); // ?
		graphicsView->setMinimumSize(240, 192); // ?
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "GraphicsView OK");
		graphicsView->setAttribute(Qt::WA_InputMethodEnabled, false); // Disable [Zenkaku / Hankaku] with IM.
		graphicsView->setAttribute(Qt::WA_KeyboardFocusChange, false);
		graphicsView->setAttribute(Qt::WA_KeyCompression, false);
		
		//graphicsView->setFocusPolicy(Qt::StrongFocus);
		//this->setFocusPolicy(Qt::ClickFocus);
	}
	
	bitmapImage = NULL;
	driveData = new CSP_DockDisks(this);
	MainWindow->setDockOptions(QMainWindow::AnimatedDocks);
	if(using_flags->is_use_fd()) {
		int i;
		for(i = 0; i < using_flags->get_max_drive(); i++) driveData->setVisibleLabel(CSP_DockDisks_Domain_FD, i, true);
	}
	if(using_flags->is_use_qd()) {
		int i;
		for(i = 0; i < using_flags->get_max_qd(); i++) driveData->setVisibleLabel(CSP_DockDisks_Domain_QD, i, true);
	}
	if(using_flags->is_use_tape()) {
		driveData->setVisibleLabel(CSP_DockDisks_Domain_CMT, 0, true);
	}
	if(using_flags->is_use_cart()) {
		int i;
		for(i = 0; i < using_flags->get_max_cart(); i++) {
			driveData->setVisibleLabel(CSP_DockDisks_Domain_Cart, i, true);
		}
	}
	if(using_flags->is_use_binary_file()) {
		int i;
		for(i = 0; i < using_flags->get_max_binary(); i++) {
			driveData->setVisibleLabel(CSP_DockDisks_Domain_Binary, i, true);
		}
	}
	if(using_flags->is_use_compact_disc()) {
		driveData->setVisibleLabel(CSP_DockDisks_Domain_CD, 0, true);
	}
	if(using_flags->is_use_laser_disc()) {
		driveData->setVisibleLabel(CSP_DockDisks_Domain_LD, 0, true);
	}
	if(using_flags->is_use_bubble()) {
		int i;
		for(i = 0; i < using_flags->get_max_bubble(); i++) driveData->setVisibleLabel(CSP_DockDisks_Domain_Bubble, i, true);
	}
	if(using_flags->get_config_ptr()->virtual_media_position > 0) {
		driveData->setVisible(true);
	} else {	
		driveData->setVisible(false);
	}	

	pCentralWidget = new QWidget(this);
	pCentralLayout = new QGridLayout(pCentralWidget);
	pCentralLayout->setContentsMargins(0, 0, 0, 0);
	pCentralWidget->setLayout(pCentralLayout);
	MainWindow->setCentralWidget(pCentralWidget);
	switch(using_flags->get_config_ptr()->virtual_media_position) {
	case 0:
		pCentralLayout->addWidget(graphicsView, 0, 0);
		pCentralLayout->addWidget(driveData, 1, 0);
		driveData->setVisible(false);
		graphicsView->setVisible(true);
		break;
	case 1:
		pCentralLayout->addWidget(graphicsView, 1, 0);
		pCentralLayout->addWidget(driveData, 0, 0);
		driveData->setVisible(true);
		graphicsView->setVisible(true);
		break;
	case 2:
		pCentralLayout->addWidget(graphicsView, 0, 0);
		pCentralLayout->addWidget(driveData, 1, 0);
		driveData->setVisible(true);
		graphicsView->setVisible(true);
		break;
	}	
	driveData->setOrientation(using_flags->get_config_ptr()->virtual_media_position);
	connect(this, SIGNAL(sig_set_orientation_osd(int)), driveData, SLOT(setOrientation(int)));

	MainWindow->setFocusProxy(graphicsView);
	//driveData->setAllowedAreas(Qt::RightToolBarArea | Qt::BottomToolBarArea);
	//MainWindow->addToolBar(driveData);
	//MainWindow->setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
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
	menuControl->setToolTipsVisible(true);
	menuControl->setObjectName(QString::fromUtf8("menuControl"));
	menuState = new QMenu(menuControl);
	menuState->setToolTipsVisible(true);
	menuState->setObjectName(QString::fromUtf8("menuState"));

	menuSave_State = new QMenu(menuState);
	menuSave_State->setToolTipsVisible(true);
	menuSave_State->setObjectName(QString::fromUtf8("menuSaveState"));

	menuLoad_State = new QMenu(menuState);
	menuLoad_State->setToolTipsVisible(true);
	menuLoad_State->setObjectName(QString::fromUtf8("menuLoadState"));

	if(using_flags->is_use_auto_key()) {
		menuCopy_Paste = new QMenu(menuControl);
		menuCopy_Paste->setObjectName(QString::fromUtf8("menuCopy_Paste"));
		menuCopy_Paste->setToolTipsVisible(true);
	}
	menuCpu_Speed = new QMenu(menuControl);
	menuCpu_Speed->setObjectName(QString::fromUtf8("menuCpu_Speed"));
	menuCpu_Speed->setToolTipsVisible(true);
	menuDebugger = new QMenu(menuControl);
	menuDebugger->setObjectName(QString::fromUtf8("menuDebugger"));
	menuDebugger->setToolTipsVisible(true);
	if(using_flags->is_use_fd()) {
		int i;
		for(i = 0; i < using_flags->get_max_drive(); i++) CreateFloppyMenu(i, i + 1);
	}
	if(using_flags->is_use_qd()) {
		int i;
		for(i = 0; i < using_flags->get_max_qd(); i++) CreateQuickDiskMenu(i, i + 1);
	}
	if(using_flags->is_use_tape()) {
		for(int i = 0; i < using_flags->get_max_tape(); i++) CreateCMTMenu(i);
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
	if(using_flags->is_use_laser_disc()) {
		CreateLaserdiscMenu();
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
	menuMachine->setToolTipsVisible(true);

	if(using_flags->is_use_mouse()) {
		actionMouseEnable = new Action_Control(this, using_flags);
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
	ConfigMouseType();
	ConfigKeyboardType();
	ConfigJoystickType();
	ConfigDriveType();
	ConfigSoundDeviceType();
	ConfigPrinterType();

	if(!using_flags->is_without_sound()) {
		menuSound = new QMenu(menubar);
		menuSound->setObjectName(QString::fromUtf8("menuSound"));
		menuSound->setToolTipsVisible(true);
	}
	menuEmulator = new QMenu(menubar);
	menuEmulator->setObjectName(QString::fromUtf8("menuEmulator"));
	menuEmulator->setToolTipsVisible(true);

	menuHELP = new QMenu(menubar);
	menuHELP->setObjectName(QString::fromUtf8("menuHELP"));
	menuHELP->setToolTipsVisible(true);
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
		for(int i = 0; i < using_flags->get_max_tape(); i++) menubar->addAction(menu_CMT[i]->menuAction());
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
	if(using_flags->is_use_laser_disc()) {
		menubar->addAction(menu_Laserdisc->menuAction());
	}
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
	
	actionHelp_AboutQt = new Action_Control(this, using_flags);
	actionHelp_AboutQt->setObjectName(QString::fromUtf8("menuHelp_AboutQt"));
	menuHELP->addAction(actionHelp_AboutQt);
	menuHELP->addSeparator();
	menuHelp_Readme = new QMenu(menuHELP);
	menuHelp_Readme->setObjectName(QString::fromUtf8("menuHelp_Readme_menu"));;
	menuHelp_Readme->setToolTipsVisible(true);
	menuHELP->addAction(menuHelp_Readme->menuAction());

	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README, "menuHelp_README", "readme.txt");
	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README_QT, "menuHelp_README_QT", "readme.qt.txt");
	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README_Artane, "menuHelp_README_Artane", "readme_by_artane.txt");
	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README_Umaiboux, "menuHelp_README_Umaiboux", "readme_by_umaiboux.txt");
	menuHelp_Readme->addSeparator();
	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README_BIOS, "menuHelp_README_BIOS", "bios_and_keys.txt");
	menuHelp_Readme->addSeparator();
	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README_MR_TANAM, "menuHelp_README_MR_TANAM", "readme_by_mr_tanam.txt");
	menuHelp_Readme->addSeparator();
	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README_FAQ, "menuHelp_README_FAQ", "FAQ.html");
	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README_FAQ_JP, "menuHelp_README_FAQ_JP", "FAQ.ja.html");
	menuHelp_Readme->addSeparator();
	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README_FM7, "menuHelp_README_FM7", "readme_fm7.txt");
	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README_FM7_JP, "menuHelp_README_FM7_JP", "readme_fm7.jp.txt");

	menuHelp_Histories = new QMenu(menuHELP);
	menuHelp_Histories->setObjectName(QString::fromUtf8("menuHelp_Histories"));;
	menuHelp_Histories->setToolTipsVisible(true);
	menuHELP->addAction(menuHelp_Histories->menuAction());

	SET_HELP_MENUENTRY(menuHelp_Histories, actionHelp_History, "menuHelp_History", "history.txt");
	SET_HELP_MENUENTRY(menuHelp_Histories, actionHelp_History_Relnote, "menuHelp_History_Relnote", "RELEASENOTE.txt");
	SET_HELP_MENUENTRY(menuHelp_Histories, actionHelp_History_ChangeLog, "menuHelp_History_Changelog", "ChangeLog.txt");
	SET_HELP_MENUENTRY(menuHelp_Histories, actionHelp_History_MR_TANAM, "menuHelp_History_MR_TANAM", "history_by_mr_tanam.txt");
	
	SET_HELP_MENUENTRY(menuHELP, actionHelp_License, "menuHelp_License", "LICENSE.txt");
	SET_HELP_MENUENTRY(menuHELP, actionHelp_License_JP, "menuHelp_License_JP", "LICENSE.ja.txt");
	
	if(using_flags->get_config_ptr()->window_mode <= 0) using_flags->get_config_ptr()->window_mode = 0;
	if(using_flags->get_config_ptr()->window_mode >= using_flags->get_screen_mode_num()) using_flags->get_config_ptr()->window_mode = using_flags->get_screen_mode_num() - 1;
	w = using_flags->get_screen_width();
	h = using_flags->get_screen_height();
	if(actionScreenSize[using_flags->get_config_ptr()->window_mode] != NULL) {
		double nd = actionScreenSize[using_flags->get_config_ptr()->window_mode]->binds->getDoubleValue();
		w = (int)(nd * (double)w);
		h = (int)(nd * (double)h);
		if(using_flags->is_use_screen_rotate()) {
			if(using_flags->get_config_ptr()->rotate_type) {
				int tmp_w = w;
				w = h;
				h = tmp_w;
			}
		}
	} else {
		if(using_flags->is_use_screen_rotate()) {
			if(using_flags->get_config_ptr()->rotate_type) {
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
	this->set_screen_aspect(using_flags->get_config_ptr()->window_stretch_type);
	if(actionScreenSize[using_flags->get_config_ptr()->window_mode] != NULL) {
		double nd = actionScreenSize[using_flags->get_config_ptr()->window_mode]->binds->getDoubleValue();
		graphicsView->do_set_screen_multiply(nd);
	}
	if(using_flags->is_use_joystick()) {
		connect(action_SetupJoystick, SIGNAL(triggered()), this, SLOT(rise_joystick_dialog()));
	}

	if(using_flags->is_use_sound_files_fdd()) {
		connect(action_SoundFilesFDD, SIGNAL(toggled(bool)), this, SLOT(do_set_sound_files_fdd(bool)));
	}
	if(using_flags->is_use_sound_files_relay()) {
		connect(action_SoundFilesRelay, SIGNAL(toggled(bool)), this, SLOT(do_set_sound_files_relay(bool)));
	}
	connect(action_SetupKeyboard, SIGNAL(triggered()), this, SLOT(rise_keyboard_dialog()));
#if !defined(Q_OS_WIN)
	connect(action_LogToSyslog, SIGNAL(toggled(bool)), this, SLOT(do_set_syslog(bool)));
#endif	
	connect(action_LogToConsole, SIGNAL(toggled(bool)), this, SLOT(do_set_conslog(bool)));
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Menu OK");
	   
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
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "setupUI() OK");
} // setupUi

// Emulator
#include "dropdown_joystick.h"
#include "dialog_set_key.h"

void Ui_MainWindowBase::retranslateEmulatorMenu(void)
{
	if(using_flags->is_use_joystick()) {
		action_SetupJoystick->setText(QApplication::translate("MainWindow", "Configure Joysticks", 0));
		action_SetupJoystick->setToolTip(QApplication::translate("MainWindow", "Configure assigning buttons/directions of joysticks.", 0));
		action_SetupJoystick->setIcon(QIcon(":/icon_gamepad.png"));
	}
	if(using_flags->is_use_roma_kana_conversion()) {
		action_UseRomaKana->setText(QApplication::translate("MainWindow", "ROMA-KANA Conversion", 0));
		action_UseRomaKana->setToolTip(QApplication::translate("MainWindow", "Use romaji-kana conversion assistant of emulator.", 0));
	}
	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	action_SetupKeyboard->setText(QApplication::translate("MainWindow", "Configure Keyboard", 0));
	action_SetupKeyboard->setToolTip(QApplication::translate("MainWindow", "Set addignation of keyboard.", 0));
	action_SetupKeyboard->setIcon(QIcon(":/icon_keyboard.png"));
	action_SetupMovie->setText(QApplication::translate("MainWindow", "Configure movie encoding", 0));
	action_SetupMovie->setToolTip(QApplication::translate("MainWindow", "Configure parameters of movie encoding.", 0));

	action_LogToConsole->setText(QApplication::translate("MainWindow", "Log to Console", 0));
	action_LogToConsole->setToolTip(QApplication::translate("MainWindow", "Enable logging to STDOUT if checked.", 0));
#if !defined(Q_OS_WIN)
	action_LogToSyslog->setText(QApplication::translate("MainWindow", "Log to Syslog", 0));
	action_LogToSyslog->setToolTip(QApplication::translate("MainWindow", "Enable logging to SYSTEM log.\nMay be having permission to system and using *nix OS.", 0));
	//action_LogRecord->setText(QApplication::translate("MainWindow", "Recording Log", 0));
#endif
	if(using_flags->is_use_sound_files_fdd()) {
		action_SoundFilesFDD->setText(QApplication::translate("MainWindow", "Sound FDD Seek", 0));
		action_SoundFilesFDD->setToolTip(QApplication::translate("MainWindow", "Enable FDD HEAD seeking sound.\nNeeds sound file.\nSee HELP->READMEs->Bios and Key assigns", 0));
	}
	if(using_flags->is_use_sound_files_relay()) {
		action_SoundFilesRelay->setText(QApplication::translate("MainWindow", "Sound CMT Relay and Buttons", 0));
		action_SoundFilesRelay->setToolTip(QApplication::translate("MainWindow", "Enable CMT relay's sound and buttons's sounds.\nNeeds sound file.\nSee HELP->READMEs->Bios and Key assigns", 0));
	}
	menuDevLogToConsole->setTitle(QApplication::translate("MainWindow", "Per Device", 0));
#if !defined(Q_OS_WIN)
	menuDevLogToSyslog->setTitle(QApplication::translate("MainWindow", "Per Device", 0));
#endif
	menu_SetRenderPlatform->setTitle(QApplication::translate("MainWindow", "Video Platform(need restart)", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL3_MAIN]->setText(QApplication::translate("MainWindow", "OpenGLv3.0", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL2_MAIN]->setText(QApplication::translate("MainWindow", "OpenGLv2.0", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL_CORE]->setText(QApplication::translate("MainWindow", "OpenGL(Core profile)", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL3_MAIN]->setToolTip(QApplication::translate("MainWindow", "Using OpenGL v3.0(MAIN).\nThis is recommanded.\nIf changed, need to restart this emulator.", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL2_MAIN]->setToolTip(QApplication::translate("MainWindow", "Using OpenGLv2.\nThis is fallback of some systems.\nIf changed, need to restart this emulator.", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL_CORE]->setToolTip(QApplication::translate("MainWindow", "Using OpenGL core profile.\nThis still not implement.\nIf changed, need to restart this emulator.", 0));

	menu_DispVirtualMedias->setTitle(QApplication::translate("MainWindow", "Show Virtual Medias.", 0));
	action_DispVirtualMedias[0]->setText(QApplication::translate("MainWindow", "None.", 0));
	action_DispVirtualMedias[1]->setText(QApplication::translate("MainWindow", "Upper.", 0));
	action_DispVirtualMedias[2]->setText(QApplication::translate("MainWindow", "Lower.", 0));
	//action_DispVirtualMedias[3]->setText(QApplication::translate("MainWindow", "Left.", 0));
	//action_DispVirtualMedias[4]->setText(QApplication::translate("MainWindow", "Right.", 0));
	action_LogView->setText(QApplication::translate("MainWindow", "View Log", 0));
	action_LogView->setToolTip(QApplication::translate("MainWindow", "View emulator logs with a dialog.", 0));
}

void Ui_MainWindowBase::retranselateUi_Depended_OSD(void)
{
	for(int i=0; i < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1) ; i++) {
		const _TCHAR *p;
		p = using_flags->get_vm_node_name(i);
		do_update_device_node_name(i, p);
	}
}

void Ui_MainWindowBase::CreateEmulatorMenu(void)
{
	//menuEmulator->addAction(action_LogRecord);
	menuEmulator->addAction(menu_DispVirtualMedias->menuAction());
	menuEmulator->addSeparator();
	if(using_flags->is_use_roma_kana_conversion()) {
		menuEmulator->addAction(action_UseRomaKana);
		menuEmulator->addSeparator();
	}
	menuEmulator->addAction(action_LogToConsole);
	menuEmulator->addAction(menuDevLogToConsole->menuAction());
	menuEmulator->addSeparator();
#if !defined(Q_OS_WIN)
	menuEmulator->addAction(action_LogToSyslog);
	menuEmulator->addAction(menuDevLogToSyslog->menuAction());
#endif
	menuEmulator->addSeparator();
	menuEmulator->addAction(action_LogView);
	menuEmulator->addSeparator();
	if(using_flags->is_use_sound_files_fdd() || using_flags->is_use_sound_files_relay()) {
		if(using_flags->is_use_sound_files_fdd())     menuEmulator->addAction(action_SoundFilesFDD);
		if(using_flags->is_use_sound_files_relay())   menuEmulator->addAction(action_SoundFilesRelay);
		menuEmulator->addSeparator();
	}
	menuEmulator->addAction(menu_SetRenderPlatform->menuAction());
	
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
	actionGroup_DispVirtualMedias = new QActionGroup(this);
	actionGroup_DispVirtualMedias->setExclusive(true);
	menu_DispVirtualMedias = new QMenu(this);
	menu_DispVirtualMedias->setToolTipsVisible(true);
	for(i = 0; i < 3; i++) {
		action_DispVirtualMedias[i] = new Action_Control(this, using_flags);
		action_DispVirtualMedias[i]->setCheckable(true);
		action_DispVirtualMedias[i]->setChecked(false);
		if(i == using_flags->get_config_ptr()->virtual_media_position) action_DispVirtualMedias[i]->setChecked(true);
		action_DispVirtualMedias[i]->setEnabled(true);
		actionGroup_DispVirtualMedias->addAction(action_DispVirtualMedias[i]);
		menu_DispVirtualMedias->addAction(action_DispVirtualMedias[i]);
	}
	connect(action_DispVirtualMedias[0], SIGNAL(triggered()), this, SLOT(do_set_visible_virtual_media_none()));
	connect(action_DispVirtualMedias[1], SIGNAL(triggered()), this, SLOT(do_set_visible_virtual_media_upper()));
	connect(action_DispVirtualMedias[2], SIGNAL(triggered()), this, SLOT(do_set_visible_virtual_media_lower()));
	//connect(action_DispVirtualMedias[3], SIGNAL(triggered()), this, SLOT(do_set_visible_virtual_media_left()));
	//connect(action_DispVirtualMedias[4], SIGNAL(triggered()), this, SLOT(do_set_visible_virtual_media_right()));
			
	//SET_ACTION_SINGLE(action_DispVirtualMedias, true, true, (using_flags->get_config_ptr()->sound_noise_cmt != 0));
	
	if(using_flags->is_use_roma_kana_conversion()) {
		//action_UseRomaKana = new Action_Control(this, using_flags);
		//action_UseRomaKana->setCheckable(true);
		//if(using_flags->get_config_ptr()->roma_kana_conversion) action_UseRomaKana->setChecked(true);
		SET_ACTION_SINGLE(action_UseRomaKana, true, true, (using_flags->get_config_ptr()->roma_kana_conversion));
		connect(action_UseRomaKana, SIGNAL(toggled(bool)), this, SLOT(do_set_roma_kana(bool)));
	}
	if(using_flags->is_use_joystick()) {
		action_SetupJoystick = new Action_Control(this, using_flags);
	}
	if(using_flags->is_use_sound_files_fdd()) {
		/*
		action_SoundFilesFDD = new Action_Control(this, using_flags);
		action_SoundFilesFDD->setCheckable(true);
		action_SoundFilesFDD->setEnabled(true);
		action_SoundFilesFDD->setChecked(false);
		if(using_flags->get_config_ptr()->sound_noise_fdd != 0) {
			action_SoundFilesFDD->setChecked(true);
		}
		*/
		SET_ACTION_SINGLE(action_SoundFilesFDD, true, true, (using_flags->get_config_ptr()->sound_noise_fdd != 0));
	}
	if(using_flags->is_use_sound_files_relay()) {
		/*
		action_SoundFilesRelay = new Action_Control(this, using_flags);
		action_SoundFilesRelay->setCheckable(true);
		action_SoundFilesRelay->setEnabled(true);
		action_SoundFilesRelay->setChecked(false);
		if(using_flags->get_config_ptr()->sound_noise_cmt != 0) {
			action_SoundFilesRelay->setChecked(true);
		}
		*/
		SET_ACTION_SINGLE(action_SoundFilesRelay, true, true, (using_flags->get_config_ptr()->sound_noise_cmt != 0));
	}
#if !defined(Q_OS_WIN)
	action_LogToSyslog = new Action_Control(this, using_flags);
	action_LogToSyslog->setCheckable(true);
	action_LogToSyslog->setEnabled(true);
	if(using_flags->get_config_ptr()->log_to_syslog != 0) action_LogToSyslog->setChecked(true);
	menuDevLogToSyslog = new QMenu(this);
	menuDevLogToSyslog->setToolTipsVisible(true);
	for(int i = 0; i < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1); i++) {
		action_DevLogToSyslog[i] = new Action_Control(this, using_flags);
		action_DevLogToSyslog[i]->setCheckable(true);
		action_DevLogToSyslog[i]->setEnabled(false);
		action_DevLogToSyslog[i]->binds->setValue1(i);
		menuDevLogToSyslog->addAction(action_DevLogToSyslog[i]);
		if(using_flags->get_config_ptr()->dev_log_to_syslog[i][0]) action_DevLogToSyslog[i]->setChecked(true);
		connect(action_DevLogToSyslog[i], SIGNAL(toggled(bool)),
				action_DevLogToSyslog[i], SLOT(do_set_dev_log_to_syslog(bool)));
		connect(action_DevLogToSyslog[i], SIGNAL(sig_set_dev_log_to_syslog(int, bool)),
				this, SLOT(do_set_dev_log_to_syslog(int, bool)));
	}
#endif
	action_LogToConsole = new Action_Control(this, using_flags);
	action_LogToConsole->setCheckable(true);
	action_LogToConsole->setEnabled(true);
	if(using_flags->get_config_ptr()->log_to_console != 0) action_LogToConsole->setChecked(true);

	//menuDevLogToConsole = new QMenu(menuEmulator);
	menuDevLogToConsole = new QMenu(this);
	menuDevLogToConsole->setToolTipsVisible(true);

	SET_ACTION_CONTROL_ARRAY(0, (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1),
							 this, using_flags,
							 menuDevLogToConsole, action_DevLogToConsole, true, false,
							 dev_log_to_console,
							 SIGNAL(toggled(bool)),
							 SLOT(do_set_dev_log_to_console(bool)),
							 SIGNAL(sig_set_dev_log_to_console(int, bool)),
							 SLOT(do_set_dev_log_to_console(int, bool)));
	/*
	for(int i = 0; i < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1); i++) {
		action_DevLogToConsole[i] = new Action_Control(this, using_flags);
		action_DevLogToConsole[i]->setCheckable(true);
		action_DevLogToConsole[i]->setEnabled(false);
		action_DevLogToConsole[i]->binds->setValue1(i);
		menuDevLogToConsole->addAction(action_DevLogToConsole[i]);
		if(using_flags->get_config_ptr()->dev_log_to_console[i][0]) action_DevLogToConsole[i]->setChecked(true);
		connect(action_DevLogToConsole[i], SIGNAL(toggled(bool)),
				action_DevLogToConsole[i], SLOT(do_set_dev_log_to_console(bool)));
		connect(action_DevLogToConsole[i], SIGNAL(sig_set_dev_log_to_console(int, bool)),
				this, SLOT(do_set_dev_log_to_console(int, bool)));
	}
	*/
	action_LogView = new Action_Control(this, using_flags);
	connect(action_LogView, SIGNAL(triggered()),
			this, SLOT(rise_log_viewer()));
	
	menu_SetRenderPlatform = new QMenu(this);
	menu_SetRenderPlatform->setToolTipsVisible(true);
	actionGroup_SetRenderPlatform = new QActionGroup(this);
	actionGroup_SetRenderPlatform->setExclusive(true);
	{
			int render_type = using_flags->get_config_ptr()->render_platform;
			int _major_version = using_flags->get_config_ptr()->render_major_version;
			//int _minor_version = using_flags->get_config_ptr()->render_minor_version; // ToDo
			for(i = 0; i < MAX_RENDER_PLATFORMS; i++) {
				tmps = QString::number(i);
				action_SetRenderPlatform[i] = new Action_Control(this, using_flags);
				action_SetRenderPlatform[i]->setObjectName(QString::fromUtf8("action_SetRenderPlatform", -1) + tmps);
				action_SetRenderPlatform[i]->setCheckable(true);
				action_SetRenderPlatform[i]->binds->setValue1(i);
				actionGroup_SetRenderPlatform->addAction(action_SetRenderPlatform[i]);
				menu_SetRenderPlatform->addAction(action_SetRenderPlatform[i]);
				if(i >= RENDER_PLATFORMS_END) {
					action_SetRenderPlatform[i]->setVisible(false);
				} else {
					if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_MAIN) {
						if(_major_version >= 3) {
							if(i == RENDER_PLATFORMS_OPENGL3_MAIN) {
								action_SetRenderPlatform[i]->setChecked(true);
							}
						} else if(i == RENDER_PLATFORMS_OPENGL2_MAIN) {
							action_SetRenderPlatform[i]->setChecked(true);
						}
					} else if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_CORE) {
						if(i == RENDER_PLATFORMS_OPENGL_CORE) {
							action_SetRenderPlatform[i]->setChecked(true);
						}						
					}
					if(i == RENDER_PLATFORMS_OPENGL_CORE) {
							action_SetRenderPlatform[i]->setEnabled(false);
							//action_SetRenderPlatform[i]->setCheckable(false);
					}						
				}
				connect(action_SetRenderPlatform[i], SIGNAL(triggered()),
						action_SetRenderPlatform[i], SLOT(do_select_render_platform(void)));
				connect(action_SetRenderPlatform[i], SIGNAL(sig_select_render_platform(int)),
						this, SLOT(do_select_render_platform(int)));
			}
	}
	action_SetupKeyboard = new Action_Control(this, using_flags);

	action_SetupMovie = new Action_Control(this, using_flags);
  
}

#include "display_log.h"

void Ui_MainWindowBase::rise_log_viewer(void)
{
	Dlg_LogViewer *dlg = new Dlg_LogViewer(using_flags, csp_logger, NULL);
	dlg->show();
}

void Ui_MainWindowBase::rise_joystick_dialog(void)
{
	if(graphicsView != NULL) {
		QStringList *lst = graphicsView->getVKNames();
		CSP_DropDownJoysticks *dlg = new CSP_DropDownJoysticks(NULL, lst, using_flags);
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
	actionHelp_AboutQt->setToolTip(QApplication::translate("MainWindow", "Display Qt version.", 0));
	actionHelp_AboutQt->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarMenuButton));
	
	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));
	actionAbout->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion));
	actionAbout->setToolTip(QApplication::translate("MainWindow", "About this emulator.", 0));

	menuHelp_Readme->setTitle(QApplication::translate("MainWindow", "READMEs", 0));
	
	actionHelp_README->setText(QApplication::translate("MainWindow", "General Document", 0));
	actionHelp_README_QT->setText(QApplication::translate("MainWindow", "About Qt ports", 0));
	actionHelp_README_Artane->setText(QApplication::translate("MainWindow", "About Qt ports (Japanese).", 0));
	actionHelp_README_Umaiboux->setText(QApplication::translate("MainWindow", "By Mr. Umaiboux.", 0));
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
	actionHelp_License->setToolTip(QApplication::translate("MainWindow", "Show general license (GPLv2).", 0));
	actionHelp_License_JP->setText(QApplication::translate("MainWindow", "Show License (Japanese)", 0));
	actionHelp_License_JP->setToolTip(QApplication::translate("MainWindow", "Show general license (GPLv2).\nTranslated to Japanese.", 0));
	ui_retranslate_completed = true;
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
		actionPrintDevice[0]->setToolTip(QApplication::translate("MainWindow", "Dump printer output to file.\nMaybe output only ascii text.", 0));
		if(using_flags->get_use_printer_type() > 0) {
			for(i = 1; i < (using_flags->get_use_printer_type() - 1); i++) {
				tmps2.setNum(i + 1);
				tmps = QApplication::translate("MainWindow", "Printer", 0) + tmps2;
				actionPrintDevice[i]->setText(tmps); 
				actionPrintDevice[i]->setToolTip(tmps); 
			}
		}
		actionPrintDevice[i]->setText(QApplication::translate("MainWindow", "Not Connect", 0));
		actionPrintDevice[i]->setToolTip(QApplication::translate("MainWindow", "None devices connect to printer port.", 0));
	}
}
void Ui_MainWindowBase::retranslateUi(void)
{
	retranslateControlMenu("NMI Reset",  true);
	retranslateFloppyMenu(0, 0);
	retranslateFloppyMenu(1, 1);
	retranslateCMTMenu(0);
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
std::string cpp_confdir;
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

void Ui_MainWindowBase::do_set_visible_virtual_media_none()
{
	driveData->setVisible(false);
	using_flags->get_config_ptr()->virtual_media_position = 0;
	set_screen_size(graphicsView->width(), graphicsView->height());
	pCentralLayout->removeWidget(driveData);
	pCentralLayout->removeWidget(graphicsView);
	pCentralLayout->addWidget(driveData, 1, 0);
	pCentralLayout->addWidget(graphicsView, 0, 0);
	emit sig_set_display_osd(true);
}

void Ui_MainWindowBase::do_set_visible_virtual_media_upper()
{
	driveData->setVisible(true);
	using_flags->get_config_ptr()->virtual_media_position = 1;
	set_screen_size(graphicsView->width(), graphicsView->height());
	emit sig_set_orientation_osd(1);
	pCentralLayout->removeWidget(driveData);
	pCentralLayout->removeWidget(graphicsView);
	pCentralLayout->addWidget(driveData, 0, 0);
	pCentralLayout->addWidget(graphicsView, 1, 0);
	emit sig_set_display_osd(false);
}

void Ui_MainWindowBase::do_set_visible_virtual_media_lower()
{
	driveData->setVisible(true);
	using_flags->get_config_ptr()->virtual_media_position = 2;
	set_screen_size(graphicsView->width(), graphicsView->height());
	emit sig_set_orientation_osd(2);
	pCentralLayout->removeWidget(driveData);
	pCentralLayout->removeWidget(graphicsView);
	pCentralLayout->addWidget(graphicsView, 0, 0);
	pCentralLayout->addWidget(driveData, 1, 0);
	emit sig_set_display_osd(false);
}

void Ui_MainWindowBase::do_set_visible_virtual_media_left()
{
	driveData->setVisible(true);
	using_flags->get_config_ptr()->virtual_media_position = 3;
	set_screen_size(graphicsView->width(), graphicsView->height());
	emit sig_set_orientation_osd(3);
	pCentralLayout->removeWidget(driveData);
	pCentralLayout->addWidget(driveData, 1, 0);
	emit sig_set_display_osd(false);
}

void Ui_MainWindowBase::do_set_visible_virtual_media_right()
{
	driveData->setVisible(true);
	using_flags->get_config_ptr()->virtual_media_position = 4;
	set_screen_size(graphicsView->width(), graphicsView->height());
	emit sig_set_orientation_osd(4);
	pCentralLayout->removeWidget(driveData);
	pCentralLayout->addWidget(driveData, 1, 2);
	emit sig_set_display_osd(false);
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
	if(using_flags->get_emu()) {
		set_window(using_flags->get_config_ptr()->window_mode);
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

/*
 * This is main for Qt.
 */
DLL_PREFIX CSP_Logger *csp_logger;

