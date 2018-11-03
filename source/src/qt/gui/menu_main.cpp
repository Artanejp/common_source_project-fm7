/*
 * Common Source code Project:
 * Ui->Qt->gui->menu_main for generic.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, moved from qt/x1turboz/MainWindow.cpp .
 */

#include <QApplication>
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
#include <QMenu>
#include <QMenuBar>
#include <QStyle>

#include "commonclasses.h"
#include "display_about.h"
#include "display_text_document.h"
#include "mainwidget_base.h"
//#include "menuclasses.h"
#include "menu_disk.h"
#include "menu_harddisk.h"
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
#include "common.h"

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

QString Ui_MainWindowBase::get_gui_version()
{
	QString retval;
	retval.clear();
#if defined(__GUI_LIBRARY_NAME)
	retval = QString::fromUtf8(__GUI_LIBRARY_NAME);
#endif
	return retval;
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

void Action_Control::do_set_window_focus_type(bool f)
{
	emit sig_set_window_focus_type(f);
}

void Action_Control::do_set_emulate_cursor_as(void)
{
	int num = this->binds->getValue1();
	emit sig_set_emulate_cursor_as(num);
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


void Ui_MainWindowBase::do_set_window_focus_type(bool flag)
{
	p_config->focus_with_click = flag;
	if(flag) {
		graphicsView->setFocusPolicy(Qt::ClickFocus);
		graphicsView->setFocus(Qt::MouseFocusReason);
	} else {
		graphicsView->setFocusPolicy(Qt::NoFocus);
		graphicsView->clearFocus();
	}
}

void Ui_MainWindowBase::do_show_about(void)
{
	Dlg_AboutCSP *dlg = new Dlg_AboutCSP(using_flags, static_cast<QWidget *>(this));
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
		p_config->sound_noise_fdd = 1;
	} else {
		p_config->sound_noise_fdd = 0;
	}
}

void Ui_MainWindowBase::do_set_sound_files_relay(bool f)
{
	if(f) {
		p_config->sound_noise_cmt = 1;
	} else {
		p_config->sound_noise_cmt = 0;
	}
}


void Ui_MainWindowBase::do_set_conslog(bool f)
{
	p_config->log_to_console = f;
	csp_logger->set_log_stdout(-1, f);
}

void Ui_MainWindowBase::do_set_syslog(bool f)
{
	p_config->log_to_syslog = f;
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


void Ui_MainWindowBase::do_set_logging_fdc(bool f)
{
	p_config->special_debug_fdc = f;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::do_set_dev_log_to_console(int num, bool f)
{
	csp_logger->set_device_node_log(num, 2, CSP_LOG_DEBUG, f);
	p_config->dev_log_to_console[num][0] = f;
}

void Ui_MainWindowBase::do_set_state_log_to_console(bool f)
{
	csp_logger->set_state_log(2, f);
	p_config->state_log_to_console = f;
}

void Ui_MainWindowBase::do_set_state_log_to_syslog(bool f)
{
	csp_logger->set_state_log(1, f);
	p_config->state_log_to_syslog = f;
}

void Ui_MainWindowBase::do_set_state_log_to_record(bool f)
{
	csp_logger->set_state_log(0, f);
	p_config->state_log_to_recording = f;
}



void Ui_MainWindowBase::do_set_emulate_cursor_as(int num)
{
	if((num < 0) || (num > 2)) return;
	p_config->cursor_as_ten_key = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::do_set_dev_log_to_syslog(int num, bool f)
{
	csp_logger->set_device_node_log(num, 2, CSP_LOG_DEBUG, f);
	p_config->dev_log_to_syslog[num][0] = f;
}

void Ui_MainWindowBase::do_select_render_platform(int num)
{
	int _major = 0;
	int _minor = 0;
	int _type = -1;

	switch(num) {
	case RENDER_PLATFORMS_OPENGL_ES_2:
		_type = CONFIG_RENDER_PLATFORM_OPENGL_ES;
		_major = 2;
		_minor = 0;
		break;
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
		_major = 4;
		_minor = 3;
		break;
	default:
		break;
	}
	if(_type >= 0) {
		p_config->render_platform = _type;
		p_config->render_major_version = _major;
		p_config->render_minor_version = _minor;
	}
}

void Ui_MainWindowBase::set_dipsw(int num, bool flag)
{
	if((num < 0) || (num >= 32)) return;
	if(flag) {
		p_config->dipswitch = p_config->dipswitch | (1 << num);
	} else {
		p_config->dipswitch = p_config->dipswitch & ~(1 << num);
	}
}

bool Ui_MainWindowBase::get_dipsw(int num)
{
	if((num < 0) || (num >= 32)) return false;
	if(((1 << num) & p_config->dipswitch) == 0) return false;
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
	ConfigHardDiskMenu();
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

		QSurfaceFormat fmt;
		{
			int render_type = p_config->render_platform;
			QOpenGLContext *glContext = QOpenGLContext::globalShareContext();
			//int _major_version = p_config->render_major_version;
			//int _minor_version = p_config->render_minor_version;

			if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_ES) {
				fmt.setRenderableType(QSurfaceFormat::OpenGLES);
				csp_logger->debug_log(CSP_LOG_DEBUG,  CSP_LOG_TYPE_GENERAL, "Try to use OpenGL ES.");
			} else if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_CORE) { 
				fmt.setProfile(QSurfaceFormat::CoreProfile); // Requires >=Qt-4.8.0
				fmt.setVersion(4, 7); // Requires >=Qt-4.8.0
				csp_logger->debug_log(CSP_LOG_DEBUG,  CSP_LOG_TYPE_GENERAL, "Try to use OpenGL CORE profile.");
			} else { // Fallback
				fmt.setProfile(QSurfaceFormat::CompatibilityProfile); // Requires >=Qt-4.8.0
				csp_logger->debug_log(CSP_LOG_DEBUG,  CSP_LOG_TYPE_GENERAL, "Try to use OpenGL Compatible(MAIN) profile.");
			}
		}
		graphicsView = new GLDrawClass(using_flags, csp_logger, this, fmt);
		graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
		graphicsView->setMaximumSize(2560, 2560); // ?
		graphicsView->setMinimumSize(240, 192); // ?
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "GraphicsView OK");
		graphicsView->setAttribute(Qt::WA_InputMethodEnabled, false); // Disable [Zenkaku / Hankaku] with IM.
		graphicsView->setAttribute(Qt::WA_KeyboardFocusChange, false);
		graphicsView->setAttribute(Qt::WA_KeyCompression, false);
		connect(this, SIGNAL(sig_set_display_osd(bool)), graphicsView, SLOT(do_set_display_osd(bool)));
		connect(this, SIGNAL(sig_set_led_width(int)), graphicsView, SLOT(do_set_led_width(int)));
	}
	
	bitmapImage = NULL;
	driveData = new CSP_DockDisks(this, using_flags);
	MainWindow->setDockOptions(QMainWindow::AnimatedDocks);
	if(p_config->virtual_media_position > 0) {
		driveData->setVisible(true);
	} else {	
		driveData->setVisible(false);
	}	

	pCentralWidget = new QWidget(this);
	pCentralLayout = new QVBoxLayout(pCentralWidget);
	pCentralLayout->setContentsMargins(0, 0, 0, 0);
	pCentralLayout->addWidget(graphicsView);
	pCentralLayout->addWidget(driveData);
	switch(p_config->virtual_media_position) {
	case 0:
		pCentralLayout->setDirection(QBoxLayout::TopToBottom);
		pCentralLayout->removeWidget(driveData);
		driveData->setVisible(false);
		graphicsView->setVisible(true);
		//emit sig_set_display_osd(true);
		break;
	case 1:
		pCentralLayout->setDirection(QBoxLayout::BottomToTop);
		driveData->setVisible(true);
		graphicsView->setVisible(true);
		//emit sig_set_display_osd(false);
		break;
	case 2:
		pCentralLayout->setDirection(QBoxLayout::TopToBottom);
		driveData->setVisible(true);
		graphicsView->setVisible(true);
		//emit sig_set_display_osd(false);
		break;
	default:
		pCentralLayout->setDirection(QBoxLayout::TopToBottom);
		driveData->setVisible(true);
		graphicsView->setVisible(true);
		//emit sig_set_display_osd(false);
		break;
	}
	pCentralWidget->setLayout(pCentralLayout);
	MainWindow->setCentralWidget(pCentralWidget);
	
	if(p_config->focus_with_click) {
		graphicsView->setFocusPolicy(Qt::ClickFocus);
		graphicsView->setFocus(Qt::MouseFocusReason);
	} else {
		graphicsView->setFocusPolicy(Qt::NoFocus);
	}
	driveData->setOrientation(p_config->virtual_media_position);
	connect(this, SIGNAL(sig_set_orientation_osd(int)), driveData, SLOT(setOrientation(int)));
	connect(graphicsView, SIGNAL(sig_resize_osd(int)), driveData, SLOT(setScreenWidth(int)));

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
		int base_drv = using_flags->get_base_floppy_disk_num();
		for(int i = 0; i < using_flags->get_max_drive(); i++) CreateFloppyMenu(i, base_drv + i);
	}
	if(using_flags->is_use_qd()) {
		int base_drv = using_flags->get_base_quick_disk_num();
		for(int i = 0; i < using_flags->get_max_qd(); i++) CreateQuickDiskMenu(i, base_drv + i);
	}
	if(using_flags->is_use_tape()) {
		int base_drv = using_flags->get_base_tape_num();
		for(int i = 0; i < using_flags->get_max_tape(); i++) CreateCMTMenu(i, base_drv + i);
	}
	if(using_flags->is_use_hdd()) {
		int base_drv = using_flags->get_base_hdd_num();
		for(int i = 0; i < using_flags->get_max_hdd(); i++) CreateHardDiskMenu(i, base_drv + i);
	}
	CreateScreenMenu();
	if(using_flags->is_use_cart()) {
		int base_drv = using_flags->get_base_cart_num();
		for(int i = 0; i < using_flags->get_max_cart(); i++) {
			CreateCartMenu(i, base_drv + i);
		}
	}
	if(using_flags->is_use_binary_file()) {
		int base_drv = using_flags->get_base_binary_num();
		for(int i = 0; i < using_flags->get_max_binary(); i++) {
			CreateBinaryMenu(i, base_drv + i);
		}
	}
	if(using_flags->is_use_compact_disc()) {
		int base_drv = using_flags->get_base_compact_disc_num();
		for(int i = 0; i < using_flags->get_max_cd(); i++) {
			CreateCDROMMenu(i, base_drv + i);
		}
	}
	if(using_flags->is_use_laser_disc()) {
		int base_drv = using_flags->get_base_laser_disc_num();
		for(int i = 0; i < using_flags->get_max_ld(); i++) {
			CreateLaserdiscMenu(i, base_drv + i);
		}
	}
	if(using_flags->is_use_bubble()) {
		int base_drv = using_flags->get_base_bubble_num();
		for(int i = 0; i < using_flags->get_max_bubble(); i++) {
			CreateBubbleMenu(i, base_drv + i);
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
	ConfigMonitorType();
	
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
	if(using_flags->is_use_hdd()) {
		int i;
		for(i = 0; i < using_flags->get_max_hdd(); i++) {
			menubar->addAction(menu_hdds[i]->menuAction());
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
		for(int i = 0; i < using_flags->get_max_binary(); i++) {
			menubar->addAction(menu_BINs[i]->menuAction());
		}
	}
	if(using_flags->is_use_compact_disc()) {
		for(int i = 0; i < using_flags->get_max_cd(); i++) {
			menubar->addAction(menu_CDROM[i]->menuAction());
		}
	}
	if(using_flags->is_use_laser_disc()) {
		for(int i = 0; i < using_flags->get_max_ld(); i++) {
			menubar->addAction(menu_Laserdisc[i]->menuAction());
		}
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
	
	if(p_config->window_mode <= 0) p_config->window_mode = 0;
	if(p_config->window_mode >= using_flags->get_screen_mode_num()) p_config->window_mode = using_flags->get_screen_mode_num() - 1;
	w = using_flags->get_screen_width();
	h = using_flags->get_screen_height();
	if(actionScreenSize[p_config->window_mode] != NULL) {
		double nd = actionScreenSize[p_config->window_mode]->binds->getDoubleValue();
		w = (int)(nd * (double)w);
		h = (int)(nd * (double)h);
		switch(p_config->rotate_type) {
		case 0:
		case 2:
			break;
		case 1:
		case 4:
			 {
				int tmp_w = w;
				w = h;
				h = tmp_w;
			}
		}
	} else {
		switch(p_config->rotate_type) {
		case 0:
		case 2:
			w = 1208;
			h = 800;
			break;
		case 1:
		case 4:
			w = 600;
			h = 960;
			break;
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
	this->set_screen_aspect(p_config->window_stretch_type);
	if(actionScreenSize[p_config->window_mode] != NULL) {
		double nd = actionScreenSize[p_config->window_mode]->binds->getDoubleValue();
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
		action_SetupJoystick->setText(QApplication::translate("MenuEmulator", "Configure Joysticks", 0));
		action_SetupJoystick->setToolTip(QApplication::translate("MenuEmulator", "Configure assigning buttons/directions of joysticks.", 0));
		action_SetupJoystick->setIcon(QIcon(":/icon_gamepad.png"));
	}
	if(using_flags->is_use_auto_key()) {
		action_UseRomaKana->setText(QApplication::translate("MenuEmulator", "ROMA-KANA Conversion", 0));
		action_UseRomaKana->setToolTip(QApplication::translate("MenuEmulator", "Use romaji-kana conversion assistant of emulator.", 0));
	}
	actionSpeed_FULL->setText(QApplication::translate("MenuEmulator", "Emulate as FULL SPEED", 0));
	actionSpeed_FULL->setToolTip(QApplication::translate("MenuEmulator", "Run emulation thread without frame sync.", 0));
	
	action_NumPadEnterAsFullkey->setText(QApplication::translate("MenuEmulator", "Numpad's Enter is Fullkey's", 0));
	action_NumPadEnterAsFullkey->setToolTip(QApplication::translate("MenuEmulator", "Numpad's enter key makes full key's enter.\nUseful for some VMs.", 0));

	action_PrintCpuStatistics->setText(QApplication::translate("MenuEmulator", "Print Statistics", 0));
	action_PrintCpuStatistics->setToolTip(QApplication::translate("MenuEmulator", "Print statistics of CPUs (or some devices).\nUseful for debugging.", 0));

	if(action_Logging_FDC != NULL) {
		action_Logging_FDC->setText(QApplication::translate("MenuEmulator", "FDC: Turn ON Debug log.", 0));
		action_Logging_FDC->setToolTip(QApplication::translate("MenuEmulator", "Turn ON debug logging for FDCs.Useful to resolve issues from guest software.", 0));
	}
	// ToDo
	menu_EmulateCursorAs->setTitle(QApplication::translate("MenuEmulator", "Emulate cursor as", 0));
	menu_EmulateCursorAs->setToolTip(QApplication::translate("MenuEmulator", "Emulate cursor as ten-key.", 0));
	action_EmulateCursorAs[0]->setText(QApplication::translate("MenuEmulator", "None", 0));
	action_EmulateCursorAs[1]->setText(QApplication::translate("MenuEmulator", "2 4 6 8", 0));
	action_EmulateCursorAs[2]->setText(QApplication::translate("MenuEmulator", "1 2 3 5", 0));
	
	menuEmulator->setTitle(QApplication::translate("MenuEmulator", "Emulator", 0));

	
	action_FocusWithClick->setText(QApplication::translate("MenuEmulator", "Focus on click", 0));
	action_FocusWithClick->setToolTip(QApplication::translate("MenuEmulator", "If set, focus with click, not mouse-over.", 0));
	
	action_SetupKeyboard->setText(QApplication::translate("MenuEmulator", "Configure Keyboard", 0));
	action_SetupKeyboard->setToolTip(QApplication::translate("MenuEmulator", "Set addignation of keyboard.", 0));
	action_SetupKeyboard->setIcon(QIcon(":/icon_keyboard.png"));
	action_SetupMovie->setText(QApplication::translate("MenuEmulator", "Configure movie encoding", 0));
	action_SetupMovie->setToolTip(QApplication::translate("MenuEmulator", "Configure parameters of movie encoding.", 0));

	action_LogToConsole->setText(QApplication::translate("MenuEmulator", "Log to Console", 0));
	action_LogToConsole->setToolTip(QApplication::translate("MenuEmulator", "Enable logging to STDOUT if checked.", 0));
#if !defined(Q_OS_WIN)
	action_LogToSyslog->setText(QApplication::translate("MenuEmulator", "Log to Syslog", 0));
	action_LogToSyslog->setToolTip(QApplication::translate("MenuEmulator", "Enable logging to SYSTEM log.\nMay be having permission to system and using *nix OS.", 0));
	//action_LogRecord->setText(QApplication::translate("MenuEmulator", "Recording Log", 0));
#endif
	if(using_flags->is_use_sound_files_fdd()) {
		action_SoundFilesFDD->setText(QApplication::translate("MenuEmulator", "Sound FDD Seek", 0));
		action_SoundFilesFDD->setToolTip(QApplication::translate("MenuEmulator", "Enable FDD HEAD seeking sound.\nNeeds sound file.\nSee HELP->READMEs->Bios and Key assigns", 0));
	}
	if(using_flags->is_use_sound_files_relay()) {
		action_SoundFilesRelay->setText(QApplication::translate("MenuEmulator", "Sound CMT Relay and Buttons", 0));
		action_SoundFilesRelay->setToolTip(QApplication::translate("MenuEmulator", "Enable CMT relay's sound and buttons's sounds.\nNeeds sound file.\nSee HELP->READMEs->Bios and Key assigns", 0));
		if(using_flags->is_tape_binary_only()) action_SoundFilesRelay->setEnabled(false);
	}
	menuDevLogToConsole->setTitle(QApplication::translate("MenuEmulator", "Per Device", 0));
#if !defined(Q_OS_WIN)
	menuDevLogToSyslog->setTitle(QApplication::translate("MenuEmulator", "Per Device", 0));
#endif
	menu_SetRenderPlatform->setTitle(QApplication::translate("MenuEmulator", "Video Platform(need restart)", 0));
	
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL_ES_2]->setText(QApplication::translate("MenuEmulator", "OpenGL ES v2.0", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL3_MAIN]->setText(QApplication::translate("MenuEmulator", "OpenGLv3.0", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL2_MAIN]->setText(QApplication::translate("MenuEmulator", "OpenGLv2.0", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL_CORE]->setText(QApplication::translate("MenuEmulator", "OpenGL(Core profile)", 0));
	
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL_ES_2]->setToolTip(QApplication::translate("MenuEmulator", "Using OpenGL ES v2.0.\nThis is recommanded.\nIf changed, need to restart this emulator.", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL3_MAIN]->setToolTip(QApplication::translate("MenuEmulator", "Using OpenGL v3.0(MAIN).\nThis is recommanded.\nIf changed, need to restart this emulator.", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL2_MAIN]->setToolTip(QApplication::translate("MenuEmulator", "Using OpenGLv2.\nThis is fallback of some systems.\nIf changed, need to restart this emulator.", 0));
	action_SetRenderPlatform[RENDER_PLATFORMS_OPENGL_CORE]->setToolTip(QApplication::translate("MenuEmulator", "Using OpenGL core profile.\nThis still not implement.\nIf changed, need to restart this emulator.", 0));

	menu_DispVirtualMedias->setTitle(QApplication::translate("MenuEmulator", "Show Virtual Medias.", 0));
	action_DispVirtualMedias[0]->setText(QApplication::translate("MenuEmulator", "None.", 0));
	action_DispVirtualMedias[1]->setText(QApplication::translate("MenuEmulator", "Upper.", 0));
	action_DispVirtualMedias[2]->setText(QApplication::translate("MenuEmulator", "Lower.", 0));
	//action_DispVirtualMedias[3]->setText(QApplication::translate("MenuEmulator", "Left.", 0));
	//action_DispVirtualMedias[4]->setText(QApplication::translate("MenuEmulator", "Right.", 0));
	action_LogView->setText(QApplication::translate("MenuEmulator", "View Log", 0));
	action_LogView->setToolTip(QApplication::translate("MenuEmulator", "View emulator logs with a dialog.", 0));
}

void Ui_MainWindowBase::retranselateUi_Depended_OSD(void)
{
	for(int i=0; i < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1) ; i++) {
		const _TCHAR *p;
		p = using_flags->get_vm_node_name(i);
		do_update_device_node_name(i, p);
	}
}

void Ui_MainWindowBase::do_set_roma_kana(bool flag)
{
	p_config->romaji_to_kana = flag;
	emit sig_set_roma_kana(flag);
}

void Ui_MainWindowBase::do_set_numpad_enter_as_fullkey(bool flag)
{
	p_config->numpad_enter_as_fullkey = flag;
}

void Ui_MainWindowBase::do_set_print_cpu_statistics(bool flag)
{
	p_config->print_statistics = flag;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::CreateEmulatorMenu(void)
{
	//menuEmulator->addAction(action_LogRecord);
	menuEmulator->addAction(action_FocusWithClick);
	menuEmulator->addAction(menu_DispVirtualMedias->menuAction());
	menuEmulator->addSeparator();
	if(using_flags->is_use_auto_key()) {
		menuEmulator->addAction(action_UseRomaKana);
	}
	menuEmulator->addAction(action_NumPadEnterAsFullkey);
	menuEmulator->addSeparator();
	menuEmulator->addAction(actionSpeed_FULL);
	menuEmulator->addSeparator();
	menuEmulator->addAction(menu_EmulateCursorAs->menuAction());
	if(action_Logging_FDC != NULL) {
		menuEmulator->addSeparator();
		menuEmulator->addAction(action_Logging_FDC);
	}
	menuEmulator->addAction(action_PrintCpuStatistics);
	menuEmulator->addSeparator();
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

void Ui_MainWindowBase::ConfigMonitorType(void)
{
	if(using_flags->get_use_monitor_type() > 0) {
		int ii;
		menuMonitorType = new QMenu(menuMachine);
		menuMonitorType->setObjectName(QString::fromUtf8("menuControl_MonitorType"));
		menuMachine->addAction(menuMonitorType->menuAction());
		
		actionGroup_MonitorType = new QActionGroup(this);
		actionGroup_MonitorType->setExclusive(true);
		for(ii = 0; ii < using_flags->get_use_monitor_type(); ii++) {
			actionMonitorType[ii] = new Action_Control(this, using_flags);
			actionGroup_MonitorType->addAction(actionMonitorType[ii]);
			actionMonitorType[ii]->setCheckable(true);
			actionMonitorType[ii]->setVisible(true);
			actionMonitorType[ii]->binds->setValue1(ii);
			if(p_config->monitor_type == ii) actionMonitorType[ii]->setChecked(true);
			menuMonitorType->addAction(actionMonitorType[ii]);
			connect(actionMonitorType[ii], SIGNAL(triggered()),
					actionMonitorType[ii]->binds, SLOT(do_set_monitor_type()));
			connect(actionMonitorType[ii]->binds, SIGNAL(sig_monitor_type(int)),
					this, SLOT(set_monitor_type(int)));
		}
	}
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
		if(i == p_config->virtual_media_position) action_DispVirtualMedias[i]->setChecked(true);
		action_DispVirtualMedias[i]->setEnabled(true);
		actionGroup_DispVirtualMedias->addAction(action_DispVirtualMedias[i]);
		menu_DispVirtualMedias->addAction(action_DispVirtualMedias[i]);
	}
	connect(action_DispVirtualMedias[0], SIGNAL(triggered()), this, SLOT(do_set_visible_virtual_media_none()));
	connect(action_DispVirtualMedias[1], SIGNAL(triggered()), this, SLOT(do_set_visible_virtual_media_upper()));
	connect(action_DispVirtualMedias[2], SIGNAL(triggered()), this, SLOT(do_set_visible_virtual_media_lower()));
			
	if(using_flags->is_use_auto_key()) {
		// ToDo: Setup if checked.
		SET_ACTION_SINGLE(action_UseRomaKana, true, true, (p_config->romaji_to_kana)); 
		connect(action_UseRomaKana, SIGNAL(toggled(bool)), this, SLOT(do_set_roma_kana(bool)));
	}
	SET_ACTION_SINGLE(action_NumPadEnterAsFullkey, true, true, (p_config->numpad_enter_as_fullkey));
	connect(action_NumPadEnterAsFullkey, SIGNAL(toggled(bool)), this, SLOT(do_set_numpad_enter_as_fullkey(bool)));

	SET_ACTION_SINGLE(action_PrintCpuStatistics, true, true, (p_config->print_statistics));
	connect(action_PrintCpuStatistics, SIGNAL(toggled(bool)), this, SLOT(do_set_print_cpu_statistics(bool)));
	
	// Cursor to ten key.
	menu_EmulateCursorAs = new QMenu(this);
	menu_EmulateCursorAs->setToolTipsVisible(true);
	actionGroup_EmulateCursorAs = new QActionGroup(this);
	actionGroup_EmulateCursorAs->setExclusive(true);
	{
		for(i = 0; i < 3; i++) {
			tmps = QString::number(i);
			action_EmulateCursorAs[i] = new Action_Control(this, using_flags);
			action_EmulateCursorAs[i]->setObjectName(QString::fromUtf8("action_EmulateCursorAs", -1) + tmps);
			action_EmulateCursorAs[i]->setCheckable(true);
			action_EmulateCursorAs[i]->binds->setValue1(i);
			actionGroup_EmulateCursorAs->addAction(action_EmulateCursorAs[i]);
			menu_EmulateCursorAs->addAction(action_EmulateCursorAs[i]);
			if(i == p_config->cursor_as_ten_key) action_EmulateCursorAs[i]->setChecked(true);
				
			connect(action_EmulateCursorAs[i], SIGNAL(triggered()),
					action_EmulateCursorAs[i], SLOT(do_set_emulate_cursor_as()));
			connect(action_EmulateCursorAs[i], SIGNAL(sig_set_emulate_cursor_as(int)),
					this, SLOT(do_set_emulate_cursor_as(int)));
		}
	}
	
	actionSpeed_FULL = new Action_Control(this, using_flags);
	actionSpeed_FULL->setObjectName(QString::fromUtf8("actionSpeed_FULL"));
	actionSpeed_FULL->setVisible(true);
	actionSpeed_FULL->setCheckable(true);
	actionSpeed_FULL->setChecked(false);
	if(p_config->full_speed) actionSpeed_FULL->setChecked(true);
	connect(actionSpeed_FULL, SIGNAL(toggled(bool)), this,SLOT(do_emu_full_speed(bool))); // OK?
	
	if(using_flags->is_use_joystick()) {
		action_SetupJoystick = new Action_Control(this, using_flags);
	}
	if(using_flags->is_use_sound_files_fdd()) {
		/*
		action_SoundFilesFDD = new Action_Control(this, using_flags);
		action_SoundFilesFDD->setCheckable(true);
		action_SoundFilesFDD->setEnabled(true);
		action_SoundFilesFDD->setChecked(false);
		if(p_config->sound_noise_fdd != 0) {
			action_SoundFilesFDD->setChecked(true);
		}
		*/
		SET_ACTION_SINGLE(action_SoundFilesFDD, true, true, (p_config->sound_noise_fdd != 0));
	}
	if(using_flags->is_use_sound_files_relay()) {
		/*
		action_SoundFilesRelay = new Action_Control(this, using_flags);
		action_SoundFilesRelay->setCheckable(true);
		action_SoundFilesRelay->setEnabled(true);
		action_SoundFilesRelay->setChecked(false);
		if(p_config->sound_noise_cmt != 0) {
			action_SoundFilesRelay->setChecked(true);
		}
		*/
		SET_ACTION_SINGLE(action_SoundFilesRelay, true, true, (p_config->sound_noise_cmt != 0));
	}
	action_FocusWithClick = new Action_Control(this, using_flags);
	action_FocusWithClick->setCheckable(true);
	action_FocusWithClick->setEnabled(true);
	if(p_config->focus_with_click) {
		action_FocusWithClick->setChecked(true);
	}

	connect(action_FocusWithClick, SIGNAL(toggled(bool)),
				this, SLOT(do_set_window_focus_type(bool)));
	//connect(action_FocusWithClick, SIGNAL(sig_set_window_focus_type(bool)),
	//			this, SLOT(do_set_window_focus_type(bool)));

	action_Logging_FDC = NULL;
	if(using_flags->is_use_fd()) {
		SET_ACTION_SINGLE(action_Logging_FDC, true, true, (p_config->special_debug_fdc != 0));
		connect(action_Logging_FDC, SIGNAL(toggled(bool)), this, SLOT(do_set_logging_fdc(bool)));
	}
#if !defined(Q_OS_WIN)
	action_LogToSyslog = new Action_Control(this, using_flags);
	action_LogToSyslog->setCheckable(true);
	action_LogToSyslog->setEnabled(true);
	if(p_config->log_to_syslog != 0) action_LogToSyslog->setChecked(true);
	menuDevLogToSyslog = new QMenu(this);
	menuDevLogToSyslog->setToolTipsVisible(true);
	for(int i = 0; i < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1); i++) {
		action_DevLogToSyslog[i] = new Action_Control(this, using_flags);
		action_DevLogToSyslog[i]->setCheckable(true);
		action_DevLogToSyslog[i]->setEnabled(false);
		action_DevLogToSyslog[i]->binds->setValue1(i);
		menuDevLogToSyslog->addAction(action_DevLogToSyslog[i]);
		if(p_config->dev_log_to_syslog[i][0]) action_DevLogToSyslog[i]->setChecked(true);
		connect(action_DevLogToSyslog[i], SIGNAL(toggled(bool)),
				action_DevLogToSyslog[i], SLOT(do_set_dev_log_to_syslog(bool)));
		connect(action_DevLogToSyslog[i], SIGNAL(sig_set_dev_log_to_syslog(int, bool)),
				this, SLOT(do_set_dev_log_to_syslog(int, bool)));
	}
#endif
	action_LogToConsole = new Action_Control(this, using_flags);
	action_LogToConsole->setCheckable(true);
	action_LogToConsole->setEnabled(true);
	if(p_config->log_to_console != 0) action_LogToConsole->setChecked(true);

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
		if(p_config->dev_log_to_console[i][0]) action_DevLogToConsole[i]->setChecked(true);
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
			int render_type = p_config->render_platform;
			int _major_version = p_config->render_major_version;
			//int _minor_version = p_config->render_minor_version; // ToDo
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
					if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_ES) {
						if(_major_version >= 2) {
							if(i == RENDER_PLATFORMS_OPENGL_ES_2) {
								action_SetRenderPlatform[i]->setChecked(true);
							}
						}
					} else if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_MAIN) {
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

QString Ui_MainWindowBase::get_system_version()
{
	return QString::fromUtf8("Dummy");
}

QString Ui_MainWindowBase::get_build_date()
{
	return QString::fromUtf8("Dummy");
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
	menuHELP->setTitle(QApplication::translate("MenuHelp", "Help", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MenuHelp", "About Qt", 0));
	actionHelp_AboutQt->setToolTip(QApplication::translate("MenuHelp", "Display Qt version.", 0));
	actionHelp_AboutQt->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarMenuButton));
	
	actionAbout->setText(QApplication::translate("MenuHelp", "About...", 0));
	actionAbout->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion));
	actionAbout->setToolTip(QApplication::translate("MenuHelp", "About this emulator.", 0));

	menuHelp_Readme->setTitle(QApplication::translate("MenuHelp", "READMEs", 0));
	
	actionHelp_README->setText(QApplication::translate("MenuHelp", "General Document", 0));
	actionHelp_README_QT->setText(QApplication::translate("MenuHelp", "About Qt ports", 0));
	actionHelp_README_Artane->setText(QApplication::translate("MenuHelp", "About Qt ports (Japanese).", 0));
	actionHelp_README_Umaiboux->setText(QApplication::translate("MenuHelp", "By Mr. Umaiboux.", 0));
	actionHelp_README_MR_TANAM->setText(QApplication::translate("MenuHelp", "By Mr. tanam", 0));
	actionHelp_README_FM7->setText(QApplication::translate("MenuHelp", "About eFM-7/8/77/AV.", 0));
	actionHelp_README_FM7_JP->setText(QApplication::translate("MenuHelp", "About eFM-7/8/77/AV (Japanese).", 0));
	actionHelp_README_FAQ->setText(QApplication::translate("MenuHelp", "FAQs(English)", 0));
	actionHelp_README_FAQ_JP->setText(QApplication::translate("MenuHelp", "FAQs(Japanese)", 0));
	actionHelp_README_BIOS->setText(QApplication::translate("MenuHelp", "BIOS and Key assigns", 0));

	menuHelp_Histories->setTitle(QApplication::translate("MenuHelp", "Histories", 0));
	actionHelp_History->setText(QApplication::translate("MenuHelp", "General History", 0));
	actionHelp_History_Relnote->setText(QApplication::translate("MenuHelp", "Release Note", 0));
	actionHelp_History_ChangeLog->setText(QApplication::translate("MenuHelp", "Change Log", 0));
	actionHelp_History_MR_TANAM->setText(QApplication::translate("MenuHelp", "History by Tanam", 0));

	actionHelp_License->setText(QApplication::translate("MenuHelp", "Show License", 0));
	actionHelp_License->setToolTip(QApplication::translate("MenuHelp", "Show general license (GPLv2).", 0));
	actionHelp_License_JP->setText(QApplication::translate("MenuHelp", "Show License (Japanese)", 0));
	actionHelp_License_JP->setToolTip(QApplication::translate("MenuHelp", "Show general license (GPLv2).\nTranslated to Japanese.", 0));
	ui_retranslate_completed = true;
}

// You can Override this function: Re-define on foo/MainWindow.cpp.
// This code is example: by X1(TurboZ).
void Ui_MainWindowBase::retranslateMachineMenu(void)
{
	int i;
	QString tmps;
	QString tmps2;
	menuMachine->setTitle(QApplication::translate("MenuMachine", "Machine", 0));
	if(using_flags->get_use_device_type() > 0) {
		menuDeviceType->setTitle(QApplication::translate("MenuMachine", "Device Type", 0));
		for(i = 0; i < using_flags->get_use_device_type(); i++) {
			tmps2.setNum(i + 1);
			tmps = QString::fromUtf8("Machine Device ") + tmps2;
			actionDeviceType[i]->setText(tmps); 
		}
	}
	if(using_flags->get_use_sound_device_type() > 0) {
		menuSoundDevice->setTitle(QApplication::translate("MenuMachine", "Sound Cards", 0));
		for(i = 0; i < using_flags->get_use_sound_device_type(); i++) {
			tmps2.setNum(i + 1);
			tmps = QString::fromUtf8("Sound Device ") + tmps2;
			actionSoundDevice[i]->setText(tmps); 
		}
	}
	if(using_flags->get_use_drive_type() > 0) {
		menuDriveType->setTitle(QApplication::translate("MenuMachine", "Drive Type", 0));
		for(i = 0; i < using_flags->get_use_drive_type(); i++) {
			tmps2.setNum(i + 1);
			tmps = QString::fromUtf8("Drive Type ") + tmps2;
			actionDriveType[i]->setText(tmps); 
		}
	}
	if(using_flags->is_use_printer()) {
		menuPrintDevice->setTitle(QApplication::translate("MenuMachine", "Printer (Need RESET)", 0));
		i = 1;
		actionPrintDevice[0]->setText(QApplication::translate("MenuMachine", "Dump to File", 0));
		actionPrintDevice[0]->setToolTip(QApplication::translate("MenuMachine", "Dump printer output to file.\nMaybe output only ascii text.", 0));
		if(using_flags->get_use_printer_type() > 0) {
			for(i = 1; i < (using_flags->get_use_printer_type() - 1); i++) {
				tmps2.setNum(i + 1);
				tmps = QApplication::translate("MenuMachine", "Printer", 0) + tmps2;
				actionPrintDevice[i]->setText(tmps); 
				actionPrintDevice[i]->setToolTip(tmps); 
			}
		}
		actionPrintDevice[i]->setText(QApplication::translate("MenuMachine", "Not Connect", 0));
		actionPrintDevice[i]->setToolTip(QApplication::translate("MenuMachine", "None devices connect to printer port.", 0));
	}
	if(using_flags->get_use_monitor_type() > 0) {
		menuMonitorType->setTitle("Monitor Type");
		menuMonitorType->setToolTipsVisible(true);
		for(int ii = 0; ii < using_flags->get_use_monitor_type(); ii++) {
			tmps = QString::fromUtf8("Monitor %1").arg(ii + 1);
			actionMonitorType[ii]->setText(tmps);
		}
	}
}
void Ui_MainWindowBase::retranslateUi(void)
{
	retranslateControlMenu("Reset",  true);
	if(!using_flags->is_without_sound()) {
		retranslateSoundMenu();
	}
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
	if(using_flags->is_use_binary_file()) {
		int basedrv = using_flags->get_base_binary_num();
		for(int i = 0; i < using_flags->get_max_binary(); i++) {
			retranslateBinaryMenu(i, basedrv);
		}
	}
	if(using_flags->is_use_bubble()) {
		int basedrv = using_flags->get_base_bubble_num();
		for(int i = 0; i < using_flags->get_max_bubble(); i++) {
			retranslateBubbleMenu(i, basedrv);
		}
	}
	if(using_flags->is_use_cart()) {
		int basedrv = using_flags->get_base_cart_num();
		for(int i = 0; i < using_flags->get_max_cart(); i++) {
			retranslateCartMenu(i, basedrv);
		}
	}
	if(using_flags->is_use_compact_disc()) {
		retranslateCDROMMenu();
	}
	if(using_flags->is_use_tape()) {
		int basedrv = using_flags->get_base_tape_num();
		for(int i = 0; i < using_flags->get_max_tape(); i++) {
			retranslateCMTMenu(i);
		}
	}
	if(using_flags->is_use_fd()) {
		int basedrv = using_flags->get_base_floppy_disk_num();
		for(int i = 0; i < using_flags->get_max_drive(); i++) {
			retranslateFloppyMenu(i, basedrv + i);
		}
	}
	if(using_flags->is_use_hdd()) {
		int basedrv = using_flags->get_base_hdd_num();
		for(int i = 0; i < using_flags->get_max_hdd(); i++) {
			retranslateHardDiskMenu(i, basedrv + i);
		}
	}
	if(using_flags->is_use_laser_disc()) {
		retranslateLaserdiscMenu();
	}
	if(using_flags->is_use_qd()) {
		int basedrv = using_flags->get_base_quick_disk_num();
		for(int i = 0; i < using_flags->get_max_qd(); i++) {
			retranslateQuickDiskMenu(i, basedrv);
		}
	}
} // retranslateUi

void Ui_MainWindowBase::doBeforeCloseMainWindow(void)
{
	//emit quit_debugger_thread();
	emit quit_emulator_all();
}

void Ui_MainWindowBase::setCoreApplication(QApplication *p)
{
	this->CoreApplication = p;
	connect(actionExit_Emulator, SIGNAL(triggered()),
			this, SLOT(doBeforeCloseMainWindow())); // OnGuiExit()?
	connect(this, SIGNAL(quit_emulator_all()), CoreApplication, SLOT(closeAllWindows()));
	connect(actionHelp_AboutQt, SIGNAL(triggered()),
			this->CoreApplication, SLOT(aboutQt()));
	
}

#include <string>
// Move from common/qt_main.cpp
// menu
DLL_PREFIX std::string cpp_confdir;
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
	QRect rect;
	driveData->setVisible(false);
	p_config->virtual_media_position = 0;
	set_screen_size(graphicsView->width(), graphicsView->height());
	
	pCentralLayout->setDirection(QBoxLayout::TopToBottom);
	rect.setRect(0, 0, graphicsView->width(), graphicsView->height() + 2);
	
	pCentralLayout->removeWidget(driveData);
	pCentralWidget->setGeometry(rect);
	//pCentralLayout->setGeometry(rect);
	pCentralLayout->update();
	pCentralWidget->setLayout(pCentralLayout);
	MainWindow->setCentralWidget(pCentralWidget);
	//emit sig_set_display_osd(true);
}

void Ui_MainWindowBase::do_set_visible_virtual_media_upper()
{
	QRect rect;
	driveData->setVisible(true);
	p_config->virtual_media_position = 1;
	set_screen_size(graphicsView->width(), graphicsView->height());
	emit sig_set_orientation_osd(1);
	pCentralLayout->setDirection(QBoxLayout::TopToBottom);
	pCentralLayout->removeWidget(driveData);
	pCentralLayout->removeWidget(graphicsView);
	pCentralLayout->addWidget(driveData);
	pCentralLayout->addWidget(graphicsView);
	rect.setRect(0, 0, graphicsView->width(), graphicsView->height() + driveData->height() + 2);
	pCentralWidget->setGeometry(rect);
	//pCentralLayout->setGeometry(rect);
	pCentralLayout->update();
	pCentralWidget->setLayout(pCentralLayout);
	MainWindow->setCentralWidget(pCentralWidget);
	//emit sig_set_display_osd(false);
}

void Ui_MainWindowBase::do_set_visible_virtual_media_lower()
{
	QRect rect;
	driveData->setVisible(true);
	p_config->virtual_media_position = 2;
	set_screen_size(graphicsView->width(), graphicsView->height());
	emit sig_set_orientation_osd(2);
	pCentralLayout->setDirection(QBoxLayout::BottomToTop);
	pCentralLayout->removeWidget(driveData);
	pCentralLayout->removeWidget(graphicsView);
	pCentralLayout->addWidget(driveData);
	pCentralLayout->addWidget(graphicsView);
	
	rect.setRect(0, 0, graphicsView->width(), graphicsView->height() + driveData->height() + 2);
	pCentralWidget->setGeometry(rect);
	pCentralLayout->update();
	pCentralWidget->setLayout(pCentralLayout);
	MainWindow->setCentralWidget(pCentralWidget);
}

void Ui_MainWindowBase::do_set_visible_virtual_media_left()
{
#if 0
	driveData->setVisible(true);
	p_config->virtual_media_position = 3;
	set_screen_size(graphicsView->width(), graphicsView->height());
	emit sig_set_orientation_osd(3);
	pCentralLayout->removeWidget(driveData);
	pCentralLayout->addWidget(driveData, 1, 0);
	//emit sig_set_display_osd(false);
#endif
}

void Ui_MainWindowBase::do_set_visible_virtual_media_right()
{
#if 0
	driveData->setVisible(true);
	p_config->virtual_media_position = 4;
	set_screen_size(graphicsView->width(), graphicsView->height());
	emit sig_set_orientation_osd(4);
	pCentralLayout->removeWidget(driveData);
	pCentralLayout->addWidget(driveData, 1, 2);
	//emit sig_set_display_osd(false);
#endif
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

