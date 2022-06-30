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
#include <QMessageBox>

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
#include "dialog_memory.h"

#include "qt_gldraw.h"
//#include "emu.h"
#include "qt_main.h"
#include "menu_flags.h"
#include "csp_logger.h"
#include "common.h"

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
	phys_key_name_map.clear();
	hRunJoy = NULL;
	about_to_close = false;
	if(csp_logger != nullptr) {
		connect(this, SIGNAL(sig_set_device_node_log(int, int, int, bool)),
				csp_logger, SLOT(set_device_node_log(int, int, int, bool)), Qt::QueuedConnection);
		connect(this, SIGNAL(sig_set_device_node_log(int, int, bool*, int, int)),
				csp_logger, SLOT(set_device_node_log(int, int, bool*, int, int)), Qt::QueuedConnection);
		connect(this, SIGNAL(sig_set_device_node_log(int, int, int*, int, int)),
				csp_logger, SLOT(set_device_node_log(int, int, int*, int, int)), Qt::QueuedConnection);
		
	}
}

Ui_MainWindowBase::~Ui_MainWindowBase()
{
	graphicsView->releaseKeyboard();
	if(ledUpdateTimer != NULL) delete ledUpdateTimer;
	if(driveData != NULL) delete driveData;
	delete using_flags;
}

QMenu  *Ui_MainWindowBase::createMenuNode(QMenuBar *parent, QString objname)
{
	QMenu *reto = new QMenu(parent);
	reto->setObjectName(objname);
	reto->setToolTipsVisible(true);
	return reto;
}

QMenu  *Ui_MainWindowBase::createMenuNode(QMenu *parent, QString objname)
{
	QMenu *reto = new QMenu(parent);
	reto->setObjectName(objname);
	reto->setToolTipsVisible(true);
	return reto;
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



void Action_Control::do_set_window_focus_type(bool f)
{
	emit sig_set_window_focus_type(f);
}




void Ui_MainWindowBase::do_set_window_focus_type(bool flag)
{
	p_config->focus_with_click = flag;
	if(flag) {
		graphicsView->setFocusPolicy(Qt::ClickFocus);
		graphicsView->setFocus(Qt::MouseFocusReason);
	} else {
		graphicsView->setFocusPolicy(Qt::NoFocus);
		//graphicsView->setFocus(Qt::OtherFocusReason);
		graphicsView->setFocus(Qt::ActiveWindowFocusReason);
		//graphicsView->clearFocus();
	}
}


void Ui_MainWindowBase::do_show_ram_size_dialog(void)
{
	CSP_MemoryDialog *dlg = new CSP_MemoryDialog(using_flags, NULL);
	dlg->show();
}

void Ui_MainWindowBase::do_show_about(void)
{
	QString renderStr;
	if(graphicsView != NULL) {
		renderStr = graphicsView->getRenderString();
	}
	Dlg_AboutCSP *dlg = new Dlg_AboutCSP(using_flags, renderStr, static_cast<QWidget *>(this));
	dlg->show();
}

void Ui_MainWindowBase::do_browse_document()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	QString fname = cp->data().toString();
	
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

void Ui_MainWindowBase::do_set_dev_log_to_console(bool f)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
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



void Ui_MainWindowBase::do_set_emulate_cursor_as(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
	if((num < 0) || (num > 2)) return;
	p_config->cursor_as_ten_key = num;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::do_set_dev_log_to_syslog(bool f)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
	csp_logger->set_device_node_log(num, 2, CSP_LOG_DEBUG, f);
	p_config->dev_log_to_syslog[num][0] = f;
}

void Ui_MainWindowBase::do_select_render_platform(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int num = cp->data().value<int>();
	
	int _major = 0;
	int _minor = 0;
	int _type = -1;

	switch(num) {
	case RENDER_PLATFORMS_OPENGL_ES_2:
		_type = CONFIG_RENDER_PLATFORM_OPENGL_ES;
		_major = 2;
		_minor = 0;
		break;
	case RENDER_PLATFORMS_OPENGL_ES_31:
		_type = CONFIG_RENDER_PLATFORM_OPENGL_ES;
		_major = 3;
		_minor = 1;
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
		_minor = 5;
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

void Ui_MainWindowBase::do_set_machine_feature(int devnum, uint32_t value)
{
	if((devnum < 0) || (devnum >= using_flags->get_use_machine_features())) return;
	p_config->machine_features[devnum] = value;
	emit sig_emu_update_config();
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

void Ui_MainWindowBase::closeEvent(QCloseEvent *event)
{
	if(about_to_close) {
		return;
	}
	QMessageBox::StandardButton ret =
		QMessageBox::question(this,
							  QApplication::translate("MainWindow", "Exit EMULATOR", 0),
							  QApplication::translate("MainWindow", "Do you QUIT this emulator?", 0),
							  QMessageBox::Yes | QMessageBox::No,
							  QMessageBox::No);
	if(ret == QMessageBox::Yes) {
		about_to_close = true;
		emit quit_emulator_all();
		event->accept();
		return;
	} else {
		event->ignore();
	}
}

void Ui_MainWindowBase::setupUi(void)
{
	int w, h;
	//   QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//	MainWindow = new QMainWindow();
	MainWindow = this;
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
				if(p_config->render_major_version < 2) p_config->render_platform = 2;
				if(p_config->render_major_version > 3) p_config->render_platform = 3;
				if(p_config->render_major_version == 2) {
					if(p_config->render_minor_version < 0) p_config->render_minor_version = 0;
					if(p_config->render_minor_version > 1) p_config->render_minor_version = 1;
				} else {
					// major == 3
					if(p_config->render_minor_version < 0) p_config->render_minor_version = 0;
					if(p_config->render_minor_version > 1) p_config->render_minor_version = 1;
				}					
				fmt.setVersion(p_config->render_major_version , p_config->render_minor_version ); // Requires >=Qt-4.8.0
				csp_logger->debug_log(CSP_LOG_DEBUG,  CSP_LOG_TYPE_GENERAL, "Try to use OpenGL ES(v%d.%d).", p_config->render_major_version, p_config->render_minor_version);
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
		graphicsView->setFocus(Qt::ActiveWindowFocusReason);
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

	menuControl = createMenuNode(menubar, QString::fromUtf8("menuControl"));
	menuState = createMenuNode(menuControl, QString::fromUtf8("menuState"));
	menuSave_State = createMenuNode(menuState, QString::fromUtf8("menuSaveState"));
	menuLoad_State = createMenuNode(menuState, QString::fromUtf8("menuLoad_State"));
	if(using_flags->is_use_auto_key()) {
		menuCopy_Paste = createMenuNode(menuControl, QString::fromUtf8("menuCopy_Paste"));
	}
	menuCpu_Speed = createMenuNode(menuControl, QString::fromUtf8("menuCpu_Speed"));
	menuDebugger = createMenuNode(menuControl, QString::fromUtf8("menuDebugger"));
	
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

	menuMachine = createMenuNode(menubar, QString::fromUtf8("menuMachine"));
	
	if(using_flags->is_use_mouse()) {
		SET_ACTION_SINGLE_CONNECT(actionMouseEnable, true , true, false, SIGNAL(toggled(bool)), SLOT(do_set_mouse_enable(bool)));
//		actionMouseEnable = new Action_Control(this, using_flags);
//		actionMouseEnable->setCheckable(true);
//		actionMouseEnable->setVisible(true);
//		actionMouseEnable->setChecked(false);
//		connect(actionMouseEnable, SIGNAL(toggled(bool)),
//				this, SLOT(do_set_mouse_enable(bool)));
		menuMachine->addAction(actionMouseEnable);
	}
	if(using_flags->is_use_ram_size()) {
		action_RAMSize = new Action_Control(this, using_flags);
		menuMachine->addSeparator();
		menuMachine->addAction(action_RAMSize);
		connect(action_RAMSize, SIGNAL(triggered()), this, SLOT(do_show_ram_size_dialog()));
		menuMachine->addSeparator();
	}
		

	ConfigDeviceType();
	ConfigMouseType();
	ConfigKeyboardType();
	ConfigJoystickType();
	ConfigDriveType();
	ConfigSoundDeviceType();
	ConfigPrinterType();
	ConfigMonitorType();
	ConfigMachineFeatures();
	
	if(!using_flags->is_without_sound()) {
		menuSound = createMenuNode(menubar, QString::fromUtf8("menuSound"));
	}
	menuEmulator = createMenuNode(menubar, QString::fromUtf8("menuEmulator"));
	menuHELP = createMenuNode(menubar, QString::fromUtf8("menuHelp"));
	
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
	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README_MR_GORRY, "menuHelp_README_MR_GORRY", "readme_by_mr_gorry.txt");
	SET_HELP_MENUENTRY(menuHelp_Readme, actionHelp_README_MR_MEISTER, "menuHelp_README_MR_MEISTER", "readme_by_mr_meister.txt");
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
	if(actionScreenSize[p_config->window_mode] != nullptr) {
		double nd = getScreenMultiply(p_config->window_mode);
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
	connect(this, SIGNAL(sig_screen_multiply(double)), graphicsView, SLOT(do_set_screen_multiply(double)), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_glv_set_fixed_size(int, int)), graphicsView, SLOT(do_set_fixed_size(int, int)), Qt::QueuedConnection);
	
	set_screen_size(w, h);
	set_screen_aspect(p_config->window_stretch_type);
	{
		double nd = getScreenMultiply(p_config->window_mode);
		if(nd > 0.0f) {
			graphicsView->do_set_screen_multiply(nd);
		}
	}
	if(using_flags->is_use_mouse()) {
		connect(action_SetupMouse, SIGNAL(triggered()), this, SLOT(rise_mouse_dialog()));
	}
	if(using_flags->is_use_joystick()) {
		connect(action_SetupJoykey, SIGNAL(triggered()), this, SLOT(rise_joykey_dialog()));
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

	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "setupUI() OK");
} // setupUi


QString Ui_MainWindowBase::get_system_version()
{
	return QString::fromUtf8("Dummy");
}

QString Ui_MainWindowBase::get_build_date()
{
	return QString::fromUtf8("Dummy");
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
	actionHelp_README_MR_GORRY->setText(QApplication::translate("MenuHelp", "By Mr. GORRY", 0));
	actionHelp_README_MR_MEISTER->setText(QApplication::translate("MenuHelp", "By Mr. Meister", 0));
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
	QMessageBox::StandardButton ret =
		QMessageBox::question(this,
							  QApplication::translate("MainWindow", "Exit EMULATOR", 0),
							  QApplication::translate("MainWindow", "Do you QUIT this emulator?", 0),
							  QMessageBox::Yes | QMessageBox::No,
							  QMessageBox::No);
	if(ret == QMessageBox::Yes) {
		about_to_close = true;
		emit quit_emulator_all();
	}
//	emit quit_emulator_all();
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
#include <QActionGroup>
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

void Ui_MainWindowBase::do_select_fixed_cpu(int num)
{
	emit sig_emu_thread_to_fixed_cpu(num);
}

void Ui_MainWindowBase::do_toggle_mouse(void)
{
}

void Ui_MainWindowBase::LaunchEmuThread(EmuThreadClassBase *m)
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

void Ui_MainWindowBase::OnOpenDebugger(void)
{
}

void Ui_MainWindowBase::OnCloseDebugger(void)
{
}

/*
 * This is main for Qt.
 */
DLL_PREFIX CSP_Logger *csp_logger;

