/*
 * MainWidget : Defines
 * Modified by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */

#ifndef _CSP_QT_MAINWIDGET_BASE_H
#define _CSP_QT_MAINWIDGET_BASE_H

//#include <QVariant>
//#include <QObject>
//#include <QApplication>
#include <QMainWindow>
#include <QIcon>
#include <QString>
#include <QStringList>
#include <QMap>
//#include <QMenu>
//#include <QMenuBar>

#include "common.h"
#include "config.h"
#include "menu_flags.h"
#include "csp_logger.h"

class DLL_PREFIX_I MOVIE_SAVER;

#include "qt_main.h"
#define _MAX_DEBUGGER 8

enum {
	CSP_MAINWIDGET_SAVE_MOVIE_STOP = 0,
	CSP_MAINWIDGET_SAVE_MOVIE_15FPS = 1,
	CSP_MAINWIDGET_SAVE_MOVIE_24FPS,
	CSP_MAINWIDGET_SAVE_MOVIE_30FPS,
	CSP_MAINWIDGET_SAVE_MOVIE_60FPS,
	CSP_MAINWIDGET_SAVE_MOVIE_END,
};

#define MAX_RENDER_PLATFORMS 12

enum {
	RENDER_PLATFORMS_OPENGL3_MAIN = 0,
	RENDER_PLATFORMS_OPENGL2_MAIN,
	RENDER_PLATFORMS_OPENGL_CORE,
	RENDER_PLATFORMS_OPENGL_ES_2,
	RENDER_PLATFORMS_OPENGL_ES_31,
	RENDER_PLATFORMS_END
};

QT_BEGIN_NAMESPACE


#define SET_ACTION_SINGLE(__action,__checkable,__enabled,__cfgif) \
		__action = new Action_Control(this, using_flags);		  \
		__action->setCheckable(__checkable);					  \
		__action->setEnabled(__enabled);						  \
		__action->setChecked(false);							  \
		if(__cfgif) {											  \
			__action->setChecked(true);							  \
		}														  \

#define SET_ACTION_SINGLE_CONNECT(__action,__checkable,__enabled,__cfgif,__signal1,__slot1) \
		__action = new Action_Control(this, using_flags);		  \
		__action->setCheckable(__checkable);					  \
		__action->setEnabled(__enabled);						  \
		__action->setChecked(false);							  \
		if(__cfgif) {											  \
			__action->setChecked(true);							  \
		}														  \
		connect(__action, __signal1, this, __slot1);			  

#define SET_ACTION_NUMERIC_CONNECT(__action,__num,__condval,__signal1,__slot1) \
	__action = new Action_Control(this, using_flags);					\
	__action->setCheckable(true);										\
	__action->setEnabled(true);											\
	__action->setData(QVariant((int)__num));							\
	__action->setChecked(false);										\
	if(__condval == __num) {											\
		__action->setChecked(true);										\
	}																	\
	/*connect(__action, __signal1, __action->binds, __slot1);*/			\
	connect(__action, __signal1, this, __slot1); 


#define SET_ACTION_CHECKABLE_SINGLE_CONNECT(__menu,__action,__objname,__cond,__signal1,__slot1) \
	__action = new Action_Control(this, using_flags);					\
	__action->setObjectName(QString::fromUtf8(__objname));				\
	__action->setCheckable(true);										\
	if(__cond) __action->setChecked(true);								\
	connect(__action, __signal1, this, __slot1);						\
	__menu->addAction(__action);

#define SET_ACTION_CHECKABLE_SINGLE_CONNECT_NOMENU(__action,__objname,__cond,__signal1,__slot1) \
	__action = new Action_Control(this, using_flags);					\
	__action->setObjectName(QString::fromUtf8(__objname));				\
	__action->setCheckable(true);										\
	if(__cond) __action->setChecked(true);								\
	connect(__action, __signal1, this, __slot1);						\

#define SET_HELP_MENUENTRY(__menu,__action,__objname,__txtname) \
	__action = new Action_Control(this, using_flags); \
	__action->setObjectName(QString::fromUtf8(__objname)); \
	__action->setData(QVariant(QString::fromUtf8(__txtname)));			\
	connect(__action, SIGNAL(triggered()), this, SLOT(do_browse_document())); \
	__menu->addAction(__action);
	

#define SET_ACTION_CONTROL_ARRAY(__start,__end,							\
								 __parent,__using_flags,				\
								 __menu,__action,						\
								 __checkable,__enabled,__cnf,			\
								 __signal1,__slot1						\
								 ) 					\
	for(int _i = __start; _i < __end;  _i++) {							\
		__action[_i] = new Action_Control(__parent, __using_flags);		\
		__action[_i]->setCheckable(__checkable);						\
		__action[_i]->setEnabled(__enabled);							\
		__action[_i]->setData(QVariant(_i));							\
		__menu->addAction(__action[_i]);								\
		if(p_config->__cnf[_i][0]) __action[_i]->setChecked(true); \
		/*connect(__action[_i], __signal1, __action[_i], __slot1);*/	\
		connect(__action[_i], __signal1, this, __slot1);				\
	}																	\

class OSD;
class QVariant;
class QAction;
class QActionGroup;
class QButtonGroup;
class QGraphicsView;
class QGraphicsScene;
class QHeaderView;
class QMenu;
class QMenuBar;
class QApplication;

class QStatusBar;
class QWidget;
class QLabel;
class QGraphicsEllipseItem;
class QClipboard;
class QDockWidget;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class Ui_SoundDialog;
class GLDrawClass;
class Action_Control;
class Menu_MetaClass;
class Menu_FDClass;
class Menu_CMTClass;
class Menu_CartClass;
class Menu_HDDClass;
class Menu_QDClass;
class Menu_BinaryClass;
class Menu_BubbleClass;
class Menu_CompactDiscClass;
class Menu_LaserdiscClass;
class CSP_DockDisks;
class MOVIE_SAVER;
class EmuThreadClassBase;
class EmuThreadClass;
class DLL_PREFIX Ui_MainWindowBase : public QMainWindow
{
	Q_OBJECT
private:
	QIcon WindowIcon;
	QIcon InsertIcon;
	QIcon EjectIcon;
	QIcon StopIcon;
	QIcon RecordSoundIcon;
	QIcon ResetIcon;
	QIcon ExitIcon;
	QIcon VolumeMutedIcon;
	QIcon VolumeLowIcon;
	QIcon VolumeMidIcon;
	QIcon VolumeHighIcon;

	// File
	QActionGroup *actionGroup_CpuSpeed;
	class Action_Control *actionSpeed_x1;
	class Action_Control *actionSpeed_x2;
	class Action_Control *actionSpeed_x4;
	class Action_Control *actionSpeed_x8;
	class Action_Control *actionSpeed_x16;
	class Action_Control *actionSpeed_FULL;
	class Action_Control *actionPaste_from_Clipboard;
	class Action_Control *actionStop_Pasting;
	QMenu *menuSave_State;
	QMenu *menuLoad_State;

	// Screen
	QActionGroup *actionGroup_Stretch;
	QActionGroup *actionGroup_SetRenderPlatform;
	QActionGroup *actionGroup_RotateType;
	class Action_Control *action_ScreenSeparateThread;
	class Action_Control *action_ScreenUseOSD;
	class Action_Control *actionZoom;
	class Action_Control *actionDisplay_Mode;
	class Action_Control *actionScanLine;
	class Action_Control *actionGLScanLineHoriz;
	class Action_Control *actionGLScanLineVert;
	
	class Action_Control *actionRotate[4];
	class Action_Control *actionCRT_Filter;
	class Action_Control *actionOpenGL_Filter;
	class Action_Control *actionDot_by_Dot;
	class Action_Control *actionReferToX_Display;
	class Action_Control *actionReferToY_Display;
	class Action_Control *actionFill_Display;
	QActionGroup *actionGroup_ScreenSize;
	QActionGroup *actionGroup_RenderMode;

	// Sound
	QActionGroup   *actionGroup_Sound_Freq;
	QActionGroup   *actionGroup_Sound_Latency;
	QActionGroup   *actionGroup_Sound_HostDevices;
	//class Action_Control *actionSoundCMT;
	class Action_Control *action_VolumeDialog;
	class Action_Control *actionSoundTapeSignal;
	class Action_Control *actionSoundTapeVoice;
	class Action_Control *actionSoundStrictRendering;
	class Action_Control *action_SoundFilesFDD;
	class Action_Control *action_SoundFilesRelay;
	class Action_Control *action_HostSoundDevice[16];
	//QMenu *menuLogToConsole;
	//QMenu *menuLogToSyslog;
	QMenu *menuDevLogToConsole;
	QMenu *menuDevLogToSyslog;
	QMenu *menu_SetRenderPlatform;

	// Misc
	QMenu *menu_DispVirtualMedias;
	QActionGroup *actionGroup_DispVirtualMedias;
	class Action_Control *action_DispVirtualMedias[5];
	class Action_Control *action_FocusWithClick;
	class Action_Control *action_UseRomaKana;
	class Action_Control *action_NumPadEnterAsFullkey;
	class Action_Control *action_UseJoykey;
	class Action_Control *action_Logging_FDC;
	class Action_Control *action_LogToSyslog;
	class Action_Control *action_LogToConsole;
	class Action_Control *action_LogRecord;
	class Action_Control *action_DevLogToSyslog[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1];
	class Action_Control *action_DevLogToConsole[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1];
	class Action_Control *action_DevLogRecord[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1];
	// Emulator
	class Action_Control *action_SetupMouse;
	class Action_Control *action_SetupJoystick;
	class Action_Control *action_SetupJoykey;
	class Action_Control *action_SetupKeyboard;
	class Action_Control *action_LogView;
	class Action_Control *action_PrintCpuStatistics;

	QMenu *menu_EmulateCursorAs;
	QActionGroup *actionGroup_EmulateCursorAs;
	class Action_Control *action_EmulateCursorAs[4];

	// Help
	class Action_Control *actionHelp_README_BIOS;
	class Action_Control *actionHelp_README;
	class Action_Control *actionHelp_README_QT;
	class Action_Control *actionHelp_README_MR_TANAM;
	class Action_Control *actionHelp_README_MR_GORRY;
	class Action_Control *actionHelp_README_MR_MEISTER;
	class Action_Control *actionHelp_README_Artane;
	class Action_Control *actionHelp_README_Umaiboux;
	class Action_Control *actionHelp_README_FAQ;
	class Action_Control *actionHelp_README_FAQ_JP;
	class Action_Control *actionHelp_README_FM7;
	class Action_Control *actionHelp_README_FM7_JP;
	class Action_Control *actionHelp_History;
	class Action_Control *actionHelp_History_Relnote;
	class Action_Control *actionHelp_History_ChangeLog;
	class Action_Control *actionHelp_History_MR_TANAM;
	class Action_Control *actionHelp_License;
	class Action_Control *actionHelp_License_JP;
	
	// Led: OSD.
	bool flags_led[32];
	bool flags_led_bak[32];
	QGraphicsView *led_graphicsView;
	QGraphicsScene *led_gScene;
	QGraphicsEllipseItem *led_leds[32];
	uint32_t osd_led_data;

	// Inner functions
	
	void ConfigCpuSpeed(void);
	void ConfigControlMenu(void);
	void connectActions_ControlMenu(void);
	void ConfigFloppyMenu(void);
	void ConfigHardDiskMenu(void);
	void ConfigSoundMenu(void);
	void CreateSoundMenu(void);

	void CreateEmulatorMenu(void);
	void ConfigEmulatorMenu(void);
	
	void CreateFloppyMenu(int drv, int drv_base);
	void CreateFloppyPulldownMenu(int drv);
	void ConfigFloppyMenuSub(int drv);

	void CreateHardDiskMenu(int drv, int drv_base);
	void CreateHardDiskPulldownMenu(int drv);
	void ConfigHardDiskMenuSub(int drv);
// Bubble
	void CreateBubbleMenu(int drv, int drv_base);
	void CreateBubblePulldownMenu(int drv);
	void ConfigBubbleMenuSub(int drv);
	void ConfigBubbleMenu(void);
	virtual int GetBubbleBankNum(int drv) { return 0; }
	virtual int GetBubbleCurrentBankNum(int drv) { return 0; }
	virtual bool GetBubbleCasetteIsProtected(int drv) { return false; }
	virtual QString GetBubbleB77FileName(int drv) { return QString::fromUtf8(""); }
	virtual QString GetBubbleB77BubbleName(int drv, int num) { return QString::fromUtf8(""); }
	
	void CreateCMTMenu(int drive, int drv_base);
	void ConfigCMTMenu(void);
   
	void ConfigQuickDiskMenu(void);
	void ConfigQuickDiskMenuSub(int drv);
	void CreateQuickDiskPulldownMenu(int drv);
	void CreateQuickDiskMenu(int drv, int drv_base);

	void CreateCartMenu(int drv, int drv_base);
	void CreateCartPulldownMenu(int drv);
	void ConfigCartMenuSub(int drv);
	void ConfigCartMenu(void);

	void CreateCDROMMenu(int drv, int drv_base);
	void ConfigCDROMMenu(void);
	void ConfigCDROMMenuSub(void);
	void CreateCDROMPulldownMenu(void);
	
	void CreateLaserdiscMenu(int drv, int drv_base);
	void ConfigLaserdiscMenu(void);
	void ConfigLaserdiscMenuSub(void);
	void CreateLaserdiscPulldownMenu(void);

	void ConfigBinaryMenu(void);
	void ConfigScreenMenu(void);
	void ConfigScreenMenu_List(void);
	void CreateScreenMenu(void);
	void ConfigDeviceType(void);
	void ConfigKeyboardType(void);
	void ConfigJoystickType(void);
	void ConfigMachineFeatures(void);
	void ConfigMouseType(void);
	void ConfigDriveType(void);
	void ConfigSoundDeviceType(void);
	void ConfigPrinterType(void);
	void ConfigMonitorType(void);

	// About Status bar
	int Calc_OSD_Wfactor(void);
protected:
	USING_FLAGS *using_flags;
	config_t *p_config;
	CSP_Logger *csp_logger;
	QMainWindow *MainWindow;
	QApplication *CoreApplication;
	QMap<uint32_t, QString>phys_key_name_map; // VK, NAME
	
	GLDrawClass *graphicsView;
	CSP_DockDisks *driveData;
	QWidget *pCentralWidget;
	QVBoxLayout *pCentralLayout;
	QStatusBar  *statusbar;
	QMenuBar    *menubar;

	QTimer *statusUpdateTimer;
	QTimer *ledUpdateTimer;
	
	const float screen_multiply_table[16] = {
		0.5, 1.0, 1.5, 2.0,
		2.25, 2.5, 3.0, 3.5,
		4.0, 5.0, 6.0, 8.0,
		0.0, 0.0, 0.0, 0.0
	};
	const float screen_multiply_table_mini[16] = {
		0.5, 1.0, 1.5, 2.0,
		3.0, 4.0, 5.0, 6.0,
		7.5, 10.0, 12.0, 15.0,
		0.0, 0.0, 0.0, 0.0
	};

	int screen_mode_count;
	// Virtual medias.
	QStringList listCARTs[8];
	QStringList listQDs[8];
	QStringList listCMT[8];
	bool cmt_write_protect[8];
	QStringList listCDROM[8];
	QStringList listLaserdisc[8];
	QStringList listBINs[8];
	QStringList listFDs[16];
	QStringList listHDDs[16];
	
	QStringList listD88[16];
	QStringList listBubbles[8];
	QStringList listB77[8];

	// Some Functions
	QActionGroup *actionGroup_BootMode;
	QActionGroup *actionGroup_CpuType;
	class Action_Control *actionReset;
	class Action_Control *actionSpecial_Reset[16];
	class Action_Control *actionExit_Emulator;
	class Action_Control *actionCpuType[8];
	class Action_Control *actionBootMode[8];
	class Action_Control *actionDebugger[_MAX_DEBUGGER];
	class Action_Control *actionSave_State[10];
	class Action_Control *actionLoad_State[10];
	//class Action_Control *actionClose_Debuggers;
	class Action_Control *actionScreenSize[32];
	class Action_Control *actionAbout;
	class Action_Control *actionMouseEnable;
	class Action_Control *actionHelp_AboutQt;
	class Action_Control *action_ResetFixedCpu;
	class Action_Control *action_SetFixedCpu[128];

	class Action_Control *action_RAMSize;
	// Screen
	class Action_Control *actionCapture_Screen;
	class Action_Control *action_SetRenderMode[8];
	// Sound
	class Action_Control *action_Freq[8];
	class Action_Control *action_Latency[6];
	class Action_Control *actionStart_Record;
	class Action_Control *actionStop_Record;

	// Emulator
	QActionGroup *actionGroup_DeviceType;
	QActionGroup *actionGroup_KeyboardType;
	QActionGroup *actionGroup_JoystickType;
	QActionGroup *actionGroup_MouseType;
	QActionGroup *actionGroup_DriveType;
	QActionGroup *actionGroup_SoundDevice;
	QActionGroup *actionGroup_PrintDevice;
	QActionGroup *actionGroup_SetFixedCpu;
	QMenu *menuDeviceType;
	QMenu *menuKeyboardType;
	QMenu *menuJoystickType;
	QMenu *menuMouseType;
	QMenu *menuDriveType;
	QMenu *menuSoundDevice;
	QMenu *menuPrintDevice;
	QMenu *menu_SetFixedCpu;
	
	class Action_Control *actionDeviceType[16];
	class Action_Control *actionKeyboardType[16];
	class Action_Control *actionJoystickType[16];
	class Action_Control *actionMouseType[8];
	class Action_Control *actionDriveType[8];
	class Action_Control *actionSoundDevice[32]; //
	class Action_Control *actionPrintDevice[16];
	class Action_Control *action_SetRenderPlatform[MAX_RENDER_PLATFORMS];

	
	class Action_Control *actionStart_Record_Movie;
	class Action_Control *actionStop_Record_Movie;
	class Action_Control *action_SetupMovie; // 15, 24, 30, 60

	QMenu *menuMonitorType;
	QActionGroup *actionGroup_MonitorType;
	Action_Control *actionMonitorType[16];
	
	// Menus    
	QMenu *menuControl;
	QMenu *menuState;
	QMenu *menuCopy_Paste;
	QMenu *menuCpu_Speed;
	QMenu *menuDebugger;
	QMenu *menuScreen;
	QMenu *menuStretch_Mode;
	QMenu *menuScreenSize;
	QMenu *menuScreen_Render;
	QMenu *menuScreen_Rotate;
	
	QMenu *menuCpuType;
	QMenu *menuBootMode;
	QMenu *menuSound;
	QMenu *menuOutput_Frequency;
	QMenu *menuSound_Latency;
	QMenu *menuSound_HostDevices;
	QMenu *menuMachine;
	QMenu *menuRecord;
	QMenu *menuRecord_sound;
	QMenu *menuRecord_as_movie;
	QMenu *menuEmulator;
	QMenu *menuHELP;
	QMenu *menuHelp_Readme;
	QMenu *menuHelp_Histories;

	QMenu *menuMachineFeatures[32];
	
	Menu_FDClass *menu_fds[16];
	Menu_QDClass *menu_QDs[8];
	Menu_CMTClass *menu_CMT[8];
	Menu_HDDClass *menu_hdds[16];
	Menu_CompactDiscClass *menu_CDROM[8];
	Menu_LaserdiscClass *menu_Laserdisc[8];
	Menu_CartClass *menu_Cart[8];
	Menu_BinaryClass *menu_BINs[8];
	Menu_BubbleClass *menu_bubbles[8];
	// Status Bar
	QWidget *dummyStatusArea1;
	QLabel *messagesStatusBar;
	QWidget *dummyStatusArea2;

	QLabel *cdrom_StatusBar;
	QString osd_str_cdrom;
	
	QLabel *laserdisc_StatusBar;
	QString osd_str_laserdisc;
	QLabel *bubble_StatusBar[8];
	QString osd_str_bubble[8];
	QImage *bitmapImage;
	QClipboard *ClipBoard;
	// Constructor
	EmuThreadClassBase *hRunEmu;
	class DrawThreadClass *hDrawEmu;
	class JoyThreadClass *hRunJoy;
	class MOVIE_SAVER *hSaveMovieThread;
	
	int max_vm_nodes;
	bool ui_retranslate_completed;
	bool about_to_close;

	virtual float getScreenMultiply(int num)
	{
		if(using_flags == nullptr) return 0.0f;
		if((num < 0) || (num > 15)) return 0.0f;
		if(using_flags->get_screen_width() > 320) {
			return screen_multiply_table[num];
		} else {
			return screen_multiply_table_mini[num];
		}
	}
	virtual void closeEvent(QCloseEvent *event);
	// CPU Type
	void ConfigCPUTypes(int num);
	void ConfigCPUBootMode(int num);
	// Translate UIs.
	void retranslateControlMenu(const char *SpecialResetTitle,  bool WithSpecialReset);
	void retranslateFloppyMenu(int drv, int basedrv);
	void retranslateFloppyMenu(int drv, int basedrv, QString specName);
	void retranslateHardDiskMenu(int drv, int basedrv);
	void retranslateHardDiskMenu(int drv, int basedrv, QString specName);
	void retranslateBubbleMenu(int drv, int basedrv);
	void retranslateCMTMenu(int drive);
	void retranslateQuickDiskMenu(int drv, int basedrv);

	void retranslateCDROMMenu(void);
	void retranslateLaserdiscMenu(void);
	void retranslateScreenMenu(void);
	void retranslateMachineMenu(void);
	void retranslateBinaryMenu(int drv, int basedrv);
	void retranslateSoundMenu(void);
	QMenu  *createMenuNode(QMenuBar *parent, QString objname = QString::fromUtf8(""));
	QMenu  *createMenuNode(QMenu *parent, QString objname = QString::fromUtf8(""));

public:
	Ui_MainWindowBase(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~Ui_MainWindowBase();

	// Initializer : using from InitContext.
	void setCoreApplication(QApplication *p);
	void createContextMenu(void);
	void setupUi(void);
	virtual void set_window(int mode);
	// Belows are able to re-implement.
	virtual void retranslateUi(void);
	virtual void retranslateUI_Help(void);
	virtual void retranslateCartMenu(int drv, int basedrv);
	virtual void retranslateVolumeLabels(Ui_SoundDialog *);
	virtual void retranslateEmulatorMenu(void);
	virtual void retranselateUi_Depended_OSD(void);
	// About Status bar
	virtual void initStatusBar(void);
	// EmuThread
	void StopEmuThread(void);
	virtual void LaunchEmuThread(EmuThreadClassBase *m);
	// JoyThread
	virtual void StopJoyThread(void);
	virtual void LaunchJoyThread(void);
	// Screen
	virtual void OnWindowMove(void);
	virtual void OnWindowRedraw(void);
   
	// Getting important widgets.
	QMainWindow *getWindow(void) { return MainWindow; }
	QMenuBar    *getMenuBar(void) { return menubar;}
	GLDrawClass *getGraphicsView(void) { return graphicsView; }
	QStatusBar *getStatusBar(void) { return statusbar;}
	QImage *getBitmapImage(void) { return bitmapImage; }
	
	virtual void OnMainWindowClosed(void);
	// Basic Action Definition
	void OnCpuPower(int mode);
	bool get_wave_shaper(int drive);
	bool get_direct_load_mzt(int drive);
	virtual bool GetPowerState(void);
	void set_logger(CSP_Logger *logger) { csp_logger = logger; }

	virtual QString get_system_version();
	virtual QString get_build_date();
	QString get_gui_version();

	// Basic slots
public slots:
	void delete_emu_thread(void);
	void doChangeMessage_EmuThread(QString str);
	void do_emu_update_config(void);
	virtual void delete_joy_thread(void);
	virtual void do_set_window_title(QString s);
	virtual void redraw_status_bar(void);
	virtual void redraw_leds(void);
	void do_recv_data_led(quint32 d);

	void do_update_volume(int level);
	void set_screen_aspect(int num);
	void set_screen_size(int w, int h);
	void do_set_screen_rotate(int type);
	void OnReset(void);
	void do_special_reset(int);	

	virtual void do_set_mouse_enable(bool flag);
	virtual void do_toggle_mouse(void);
	void do_set_sound_device(int);
	void do_emu_update_volume_balance(int num, int level);
	void do_emu_update_volume_level(int num, int level);
	void rise_log_viewer(void);
	void rise_volume_dialog(void);
	void rise_mouse_dialog(void);
	void rise_joystick_dialog(void);
	void rise_joykey_dialog(void);
	void rise_keyboard_dialog(void);
	virtual void rise_movie_dialog(void);
	void do_stop_saving_movie(void);
	void do_start_saving_movie(void);
	void do_set_state_saving_movie(bool state);
	void set_osd_virtual_media(bool f);
	
	virtual void OnOpenDebugger(void);
	virtual void OnCloseDebugger(void);
	void doBeforeCloseMainWindow(void);	
	void set_gl_crt_filter(bool);
	void set_cpu_power(void)
	{
		QAction *cp = qobject_cast<QAction*>(QObject::sender());
		if(cp == nullptr) return;
		int pw = cp->data.value<int>();
		
		OnCpuPower(pw);
	}
	virtual void on_actionExit_triggered();
	void do_emu_full_speed(bool f);
	void OnStartAutoKey(void);
	void OnStopAutoKey(void);
	void eject_cart(int);
	void set_recent_cart(int, int);

	int set_recent_cdrom(int drv, int num);
	void do_eject_cdrom(int drv);
	void do_open_cdrom(int drv, QString path);
	void do_swap_cdaudio_byteorder(int drv, bool value);

	int set_recent_laserdisc(int drv, int num); 
	void do_eject_laserdisc(int drv); 
	void do_open_laserdisc(int drv, QString path);

	void CreateBinaryMenu(int drv, int drv_base);
	void CreateBinaryPulldownMenu(int drv);
	void ConfigBinaryMenuSub(int drv);
	int set_recent_binary_load(int drv, int num);
	int set_recent_binary_save(int drv, int num);
	void _open_binary_load(int drive, const QString fname);
	void _open_binary_save(int drive, const QString fname);

	int set_recent_quick_disk(int drive, int num); 
	int write_protect_Qd(int drv, bool flag);
	void _open_quick_disk(int drv, const QString fname);
	void eject_Qd(int drv);


	void _open_cart(int drv, const QString fname);
	void eject_cmt(int drv);
	
	void do_change_boot_mode(int mode);
	void do_change_cpu_type(int mode);
	
	void do_write_protect_cmt(int drv, bool flag);
	int  set_recent_cmt(int drv, int num);
	void set_wave_shaper(int drive, bool f);
	void set_direct_load_from_mzt(int drive, bool f);
	void do_open_write_cmt(int drive, QString);
	void do_open_read_cmt(int drive, QString path);

	void do_push_play_tape(int drive);
	void do_push_stop_tape(int drive);
	void do_push_fast_forward_tape(int drive);
	void do_push_rewind_tape(int drive);
	void do_push_apss_forward_tape(int drive);
	void do_push_apss_rewind_tape(int drive);
	void set_cmt_sound(bool);

	int write_protect_fd(int drv, bool flag);
	void eject_fd(int drv);
	void eject_hard_disk(int drv);
	virtual void do_create_d88_media(int drv, quint8 media_type, QString name) { }
	virtual void do_create_hard_disk(int drv, int sector_size, int sectors, int surfaces, int cylinders, QString name) { }
	void do_update_d88_list(int drv, int bank);

	// Bubble Casette
	int write_protect_bubble(int drv, bool flag);

	virtual int set_b77_slot(int drive, int num) { return 0; }
	virtual void do_update_recent_bubble(int drv) { }
	virtual int set_recent_bubble(int drv, int num) { return 0; }
	virtual void _open_bubble(int drv, const QString fname) { }
	virtual void eject_bubble(int drv) { }
	
	void _open_disk(int drv, const QString fname);
	void do_update_recent_disk(int);
	int set_d88_slot(int drive, int num);
	int set_recent_disk(int, int);

	void _open_hard_disk(int drv, const QString fname);
	void do_update_recent_hard_disk(int);
	int set_recent_hard_disk(int, int);

	void start_record_sound(bool rec);
	void set_freq(int);
	void set_latency(int);
	void set_sound_device(int);
	void do_set_sound_strict_rendering(bool f);
	void do_set_sound_tape_signal(bool f);
	void do_set_sound_tape_voice(bool f);
	void do_set_host_sound_device(int num);
	void do_set_host_sound_name(int num, QString sname);	
	void set_monitor_type(void);
	void message_status_bar(QString);
	void resize_statusbar(int w, int h);
	virtual void do_release_emu_resources(void);
	virtual void set_window_title();
	void set_device_type(int);
	void set_mouse_type(int);
	void set_keyboard_type(int);
	void set_joystick_type(int);
	void set_drive_type(int);
	void set_scan_line(bool);
	void set_gl_scan_line_vert(bool);
	void set_gl_scan_line_horiz(bool);
	void set_printer_device(int);
	void do_show_about(void);
	void do_browse_document(void);
	void set_dipsw(int num, bool flag);
	bool get_dipsw(int num);

	void do_set_sound_files_fdd(bool f);
	void do_set_sound_files_relay(bool f);
	void do_set_conslog(bool);
	void do_set_syslog(bool);
	void do_update_device_node_name(int id, const _TCHAR *name);	
	void do_set_dev_log_to_console(int id, bool f);
	void do_set_dev_log_to_syslog(int id, bool f);
	void do_set_roma_kana(bool f);
	void do_set_numpad_enter_as_fullkey(bool f);
	void do_set_render_mode_std(void);
	void do_set_render_mode_tv(void);
	void do_select_render_platform(int num);
	void do_set_window_focus_type(bool flag);

	void do_set_visible_virtual_media_none();
	void do_set_visible_virtual_media_upper();
	void do_set_visible_virtual_media_lower();
	void do_set_visible_virtual_media_left();
	void do_set_visible_virtual_media_right();

	void do_set_emulate_cursor_as(int num);
	void do_set_logging_fdc(bool onoff);	
	void do_set_separate_thread_draw(bool f);
	void do_set_print_cpu_statistics(bool flag);
	// ToDo: GUI
	void do_set_state_log_to_record(bool f);
	void do_set_state_log_to_console(bool f);
	void do_set_state_log_to_syslog(bool f);
	void do_set_joy_to_key(bool flag);
	void do_select_fixed_cpu(int num);
	void do_add_keyname_table(uint32_t vk, QString name);
	void do_clear_keyname_table();
	void do_show_ram_size_dialog(void);
	void do_block_task();
	void do_unblock_task();

	void do_set_machine_feature(int devnum, uint32_t value);

	void do_start_emu_thread();
	void do_start_draw_thread();
signals:
	int message_changed(QString);
	int quit_emu_thread();
	int call_joy_thread(EMU_TEMPLATE *);
	int quit_joy_thread();
	int quit_draw_thread();
	int quit_emulator_all();
	int sig_quit_movie_thread();
	int sig_stop_saving_movie(void);
	int sig_start_saving_movie(void);
	int on_boot_mode(int);
	int on_cpu_type(int);
	int on_cpu_power(int);
	int on_open_debugger(int);
	int on_insert_fd(int);
	int on_eject_fd(int);
	int do_open_disk(int, QString);
	
	int closed(void);
	int sig_quit_all(void);
	int sig_vm_reset(void);
	int sig_vm_specialreset(int);
	int sig_check_grab_mouse(bool);
	int sig_resize_uibar(int, int);
	int sig_resize_screen(int, int);
	int sig_update_screen(void);
	int sig_emu_update_config(void);
	int sig_emu_start_rec_sound(void);
	int sig_emu_stop_rec_sound(void);
	int sig_emu_set_display_size(int, int, int, int);
	int sig_emu_update_volume_level(int, int);
	int sig_emu_update_volume_balance(int, int);

	int sig_resize_osd(int);
	
	int sig_write_protect_disk(int drv, bool flag);
	int sig_open_disk(int, QString, int);
	int sig_close_disk(int);
	int sig_open_hard_disk(int, QString);
	int sig_close_hard_disk(int);
	int sig_play_tape(int ,QString);
	int sig_rec_tape(int, QString);
	int sig_close_tape(int);
	int sig_cmt_push_play(int);
	int sig_cmt_push_stop(int);
	int sig_cmt_push_fast_forward(int);
	int sig_cmt_push_fast_rewind(int);
	int sig_cmt_push_apss_forward(int);
	int sig_cmt_push_apss_rewind(int);
	int sig_write_protect_quickdisk(int drv, bool flag);
	int sig_close_quickdisk(int drv);
	int sig_open_quickdisk(int drv, QString path);
	int sig_close_cart(int drv);
	int sig_open_cart(int drv, QString path);
	int sig_open_cdrom(int drv, QString path);
	int sig_close_cdrom(int drv);
	int sig_close_laserdisc(int drv);
	int sig_open_laserdisc(int drv, QString path);
	int sig_load_binary(int drv, QString path);
	int sig_save_binary(int drv, QString path);
	int sig_write_protect_bubble(int, bool);
	int sig_open_bubble(int, QString , int);
	int sig_close_bubble(int);
	int sig_led_update(QRectF);
	int sig_start_auto_key(QString);
	int sig_stop_auto_key(void);
	int sig_set_display_osd(bool);
	int sig_display_osd_leds(int,bool);
	int sig_set_led_width(int);
	int sig_set_orientation_osd(int);
	int sig_set_roma_kana(bool);
	int quit_debugger_thread(void);
	int sig_quit_widgets(void);

	int sig_emu_thread_to_fixed_cpu(int);
	int sig_add_keyname_table(uint32_t, QString);

	int sig_block_task();
	int sig_unblock_task();
	int sig_start_emu_thread(void);
	int sig_start_draw_thread(void);
	int sig_emu_launched(void);
};

QT_END_NAMESPACE

#endif
