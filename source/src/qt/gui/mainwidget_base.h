/*
 * MainWidget : Defines
 * Modified by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */

#ifndef _CSP_QT_MAINWIDGET_BASE_H
#define _CSP_QT_MAINWIDGET_BASE_H

#if defined(_USE_QT5)
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QGraphicsView>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QGraphicsEllipseItem>
#include <QStringList>
#include <QClipboard>
#else
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
#include <QtGui/QIconSet>
#include <QLabel>
#endif

#include "simd_types.h"
#include "common.h"
#include "config.h"
#include "menu_flags.h"
//#include "emu.h"
//#include "vm.h"

class MOVIE_SAVER;

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

QT_BEGIN_NAMESPACE

class Ui_SoundDialog;
class GLDrawClass;
class Action_Control;
class Menu_MetaClass;
class Menu_FDClass;
class Menu_CMTClass;
class Menu_CartClass;
class Menu_QDClass;
class Menu_BinaryClass;
class Menu_BubbleClass;
class Menu_CompactDiscClass;
class Menu_LaserdiscClass;
class MOVIE_SAVER;


class DLL_PREFIX Ui_MainWindowBase : public QMainWindow
{
	Q_OBJECT
 protected:
	USING_FLAGS *using_flags;
	config_t *p_config;
	QMainWindow *MainWindow;
	QApplication *CoreApplication;
	
	QWidget *centralwidget;
	GLDrawClass *graphicsView;
	QStatusBar  *statusbar;
	QMenuBar    *menubar;
	QTimer *statusUpdateTimer;

	QTimer *ledUpdateTimer;

	int screen_mode_count;
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
	// Some Functions
	void ConfigCpuSpeed(void);
	void ConfigControlMenu(void);
	void connectActions_ControlMenu(void);
	void retranslateControlMenu(const char *SpecialResetTitle,  bool WithSpecialReset);
	void ConfigFloppyMenu(void);
	void ConfigSoundMenu(void);
	void CreateSoundMenu(void);

	void CreateEmulatorMenu(void);
	void ConfigEmulatorMenu(void);
	
	void CreateFloppyMenu(int drv, int drv_base);
	void CreateFloppyPulldownMenu(int drv);
	void ConfigFloppyMenuSub(int drv);
	void retranslateFloppyMenu(int drv, int basedrv);

	// Bubble
	void CreateBubbleMenu(int drv, int drv_base);
	void CreateBubblePulldownMenu(int drv);
	void ConfigBubbleMenuSub(int drv);
	void retranslateBubbleMenu(int drv, int basedrv);
	void ConfigBubbleMenu(void);
	
	void CreateCMTMenu(void);
	void CreateCMTPulldownMenu(void);
	void ConfigCMTMenuSub(void);
	void retranslateCMTMenu(void);
	void ConfigCMTMenu(void);
   
	void ConfigQuickDiskMenu(void);
	void retranslateQuickDiskMenu(int drv, int basedrv);
	void ConfigQuickDiskMenuSub(int drv);
	void CreateQuickDiskPulldownMenu(int drv);
	void CreateQuickDiskMenu(int drv, int drv_base);

	void CreateCartMenu(int drv, int drv_base);
	void CreateCartPulldownMenu(int drv);
	void ConfigCartMenuSub(int drv);
	void ConfigCartMenu(void);

	void CreateCDROMMenu(void);
	void ConfigCDROMMenu(void);
	void ConfigCDROMMenuSub(void);
	void CreateCDROMPulldownMenu(void);
	void retranslateCDROMMenu(void);

	void CreateLaserdiscMenu(void);
	void ConfigLaserdiscMenu(void);
	void ConfigLaserdiscMenuSub(void);
	void CreateLaserdiscPulldownMenu(void);
	void retranslateLaserdiscMenu(void);

	void ConfigBinaryMenu(void);
	void retranslateBinaryMenu(int drv, int basedrv);
	
	void retranslateSoundMenu(void);

	void ConfigScreenMenu(void);
	void ConfigScreenMenu_List(void);
	void CreateScreenMenu(void);
	void ConfigDeviceType(void);
	void ConfigDriveType(void);
	void ConfigSoundDeviceType(void);
	void ConfigPrinterType(void);
	
	void retranslateScreenMenu(void);
	void retranslateMachineMenu(void);

	class Action_Control *actionReset;
	class Action_Control *actionSpecial_Reset;
	class Action_Control *actionExit_Emulator;

	// Pls.Override
	QActionGroup *actionGroup_CpuType;
	QMenu *menuCpuType;
	class Action_Control *actionCpuType[8];
	void ConfigCPUTypes(int num);

	QActionGroup *actionGroup_CpuSpeed;
	class Action_Control *actionSpeed_x1;
	class Action_Control *actionSpeed_x2;
	class Action_Control *actionSpeed_x4;
	class Action_Control *actionSpeed_x8;
	class Action_Control *actionSpeed_x16;


	// Pls.Override
	QActionGroup *actionGroup_BootMode;
	QMenu *menuBootMode;
	class Action_Control *actionBootMode[8];
	void ConfigCPUBootMode(int num);

	class Action_Control *actionPaste_from_Clipboard;
	class Action_Control *actionStop_Pasting;

	class Action_Control *actionSave_State;
	class Action_Control *actionLoad_State;
	
	class Action_Control *actionDebugger[_MAX_DEBUGGER];
	//class Action_Control *actionClose_Debuggers;


	QStringList listCARTs[8];

	QStringList listQDs[8];
	
	QStringList listCMT;
	bool cmt_write_protect;
	QStringList listCDROM;
	QStringList listLaserdisc;
	QStringList listBINs[8];
	// Screen
	class Action_Control *actionZoom;
	class Action_Control *actionDisplay_Mode;

	class Action_Control *actionScanLine;

	class Action_Control *actionGLScanLineHoriz;
	class Action_Control *actionGLScanLineVert;
	class Action_Control *actionRotate;

	class Action_Control *actionCRT_Filter;
	class Action_Control *actionOpenGL_Filter;

	QActionGroup *actionGroup_Stretch;
	class Action_Control *actionDot_by_Dot;
	class Action_Control *actionReferToX_Display;
	class Action_Control *actionReferToY_Display;
	class Action_Control *actionFill_Display;

	class Action_Control *actionCapture_Screen;

	QActionGroup *actionGroup_ScreenSize;
	class Action_Control *actionScreenSize[32]; 
	class Action_Control *actionAbout;
	class Action_Control *actionHelp_README_BIOS;
	class Action_Control *actionHelp_README;
	class Action_Control *actionHelp_README_QT;
	class Action_Control *actionHelp_README_MR_TANAM;
	class Action_Control *actionHelp_README_Artane;
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
	
	
	QActionGroup   *actionGroup_Sound_Freq;
	QActionGroup   *actionGroup_Sound_Latency;

	class Action_Control *actionSoundCMT;

	class Action_Control *action_Freq[8];
	class Action_Control *action_Latency[6];
	class Action_Control *actionStart_Record;
	class Action_Control *actionStop_Record;
	class Action_Control *actionStart_Record_Movie;
	class Action_Control *actionStop_Record_Movie;
	class Action_Control *action_VolumeDialog;
	

	class Action_Control *actionMouseEnable;
	class Action_Control *actionHelp_AboutQt;


	QActionGroup *actionGroup_DeviceType;
	QMenu *menuDeviceType;
	class Action_Control *actionDeviceType[16];

	QActionGroup *actionGroup_DriveType;
	QMenu *menuDriveType;
	class Action_Control *actionDriveType[8];

	QActionGroup   *actionGroup_SoundDevice;
	QMenu *menuSoundDevice;
	class Action_Control *actionSoundDevice[32]; //

	QActionGroup *actionGroup_PrintDevice;
	QMenu *menuPrintDevice;
	class Action_Control *actionPrintDevice[16];
	// Emulator
	class Action_Control *action_SetupJoystick;
	class Action_Control *action_SetupKeyboard;

	QMenu *menuLogToConsole;
	QMenu *menuLogToSyslog;
	class Action_Control *action_LogToSyslog;
	class Action_Control *action_LogToConsole;
	class Action_Control *action_LogRecord;

	class Action_Control *action_SetupMovie; // 15, 24, 30, 60
	
	// Menus    
	QMenu *menuControl;
	QMenu *menuState;
	QMenu *menuCopy_Paste;
	QMenu *menuCpu_Speed;
	QMenu *menuDebugger;
	
	Menu_FDClass *menu_fds[16];
	QStringList listFDs[16];
	QStringList listD88[16];
	
	Menu_QDClass *menu_QDs[8];
	
	Menu_CMTClass *menu_CMT;
	
	Menu_CompactDiscClass *menu_CDROM;
	Menu_LaserdiscClass *menu_Laserdisc;
	Menu_CartClass *menu_Cart[8];
	Menu_BinaryClass *menu_BINs[8];
	Menu_BubbleClass *menu_bubbles[8];
	
	QStringList listBubbles[8];
	QStringList listB77[8];

	QMenu *menuScreen;

	QMenu *menuStretch_Mode;
	QMenu *menuScreenSize;
  
	QMenu *menuSound;
	QMenu *menuOutput_Frequency;
	QMenu *menuSound_Latency;
	QMenu *menuMachine;
	QMenu *menuRecord;
	QMenu *menuRecord_sound;
	QMenu *menuRecord_as_movie;
	QMenu *menuEmulator;
	QMenu *menuHELP;
	QMenu *menuHelp_Readme;
	QMenu *menuHelp_Histories;
	// Status Bar
	QWidget *dummyStatusArea1;
	QLabel *messagesStatusBar;
	QWidget *dummyStatusArea2;

	QLabel *fd_StatusBar[16];
	QString osd_str_fd[16];
	
	QLabel *qd_StatusBar[8];
	QString osd_str_qd[8];
	
	QLabel *cmt_StatusBar;
	QString osd_str_cmt;
	
	QLabel *cdrom_StatusBar;
	QString osd_str_cdrom;
	
	QLabel *laserdisc_StatusBar;
	QString osd_str_laserdisc;
	
	QLabel *bubble_StatusBar[8];
	QString osd_str_bubble[8];
	
	QImage *bitmapImage;
	
	bool flags_led[32];
	bool flags_led_bak[32];
	QGraphicsView *led_graphicsView;
	QGraphicsScene *led_gScene;
	QGraphicsEllipseItem *led_leds[32];
	uint32_t osd_led_data;
	
	QClipboard *ClipBoard;

	// About Status bar
	int Calc_OSD_Wfactor(void);
	// Constructor
	class EmuThreadClass *hRunEmu;
	class DrawThreadClass *hDrawEmu;
	class JoyThreadClass *hRunJoy;
	class MOVIE_SAVER *hSaveMovieThread;
public:
	Ui_MainWindowBase(USING_FLAGS *p, QWidget *parent = 0);
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
	// About Status bar
	virtual void initStatusBar(void);
	// EmuThread
	void StopEmuThread(void);
	virtual void LaunchEmuThread(void);
	// JoyThread
	virtual void StopJoyThread(void);
	virtual void LaunchJoyThread(void);
	// Screen
	virtual void OnWindowResize(void);
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
	bool get_wave_shaper(void);
	bool get_direct_load_mzt(void);
	virtual bool GetPowerState(void);
   
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
	void OnReset(void);
	void OnSpecialReset(void);
	virtual void do_set_mouse_enable(bool flag);
	virtual void do_toggle_mouse(void);
	void do_set_sound_device(int);
	void do_emu_update_volume_balance(int num, int level);
	void do_emu_update_volume_level(int num, int level);
	
	void rise_volume_dialog(void);

	void rise_joystick_dialog(void);

	void rise_keyboard_dialog(void);
	virtual void rise_movie_dialog(void);
	void do_stop_saving_movie(void);
	void do_start_saving_movie(void);
	void do_set_state_saving_movie(bool state);
	void OnLoadState(void);
	void OnSaveState(void);
	virtual void OnOpenDebugger(int n);
	virtual void OnCloseDebugger(void);
	
	void set_screen_rotate(bool);
	void set_crt_filter(bool);
	void set_gl_crt_filter(bool);
	void set_cpu_power(int pw) {
		OnCpuPower(pw);
	}
	virtual void on_actionExit_triggered();

	void OnStartAutoKey(void);
	void OnStopAutoKey(void);
	
	void do_change_osd_fd(int drv, QString tmpstr);

	void eject_cart(int);
	void set_recent_cart(int, int);

	int set_recent_cdrom(int drv, int num);
	void do_eject_cdrom(int drv);
	void do_open_cdrom(int drv, QString path);
	void do_change_osd_cdrom(QString tmpstr);

	int set_recent_laserdisc(int drv, int num); 
	void do_eject_laserdisc(int drv); 
	void do_open_laserdisc(int drv, QString path);
	void do_change_osd_laserdisc(QString tmpstr);

	void CreateBinaryMenu(int drv, int drv_base);
	void CreateBinaryPulldownMenu(int drv);
	void ConfigBinaryMenuSub(int drv);
	int set_recent_binary_load(int drv, int num);
	int set_recent_binary_save(int drv, int num);
	void _open_binary_load(int drive, const QString fname);
	void _open_binary_save(int drive, const QString fname);

	void open_quick_disk_dialog(int drv);
	int set_recent_quick_disk(int drive, int num); 
	int write_protect_Qd(int drv, bool flag);
	void _open_quick_disk(int drv, const QString fname);
	void eject_Qd(int drv);

	void do_change_osd_qd(int drv, QString tmpstr);

	virtual void _open_disk(int drv, const QString fname);
	void _open_cart(int drv, const QString fname);
	void eject_cmt(void);
	void do_change_boot_mode(int mode);
	void do_change_cpu_type(int mode);
	void do_write_protect_cmt(int drv, bool flag);
	int  set_recent_cmt(int drv, int num);
	void set_wave_shaper(bool f);
//#if defined(_MZ80A) || defined(_MZ80K) || defined(_MZ1200) || defined(_MZ700) || defined(_MZ800) || defined(_MZ1500) || \
//	defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
	void set_direct_load_from_mzt(bool f);
//#endif	
	void do_open_write_cmt(QString);
	void do_open_read_cmt(int dummy, QString path);
	void do_change_osd_cmt(QString tmpstr);

	void do_push_play_tape(void);
	void do_push_stop_tape(void);
	void do_display_tape_play(bool flag);
	void do_push_fast_forward_tape(void);
	void do_push_rewind_tape(void);
	void do_push_apss_forward_tape(void);
	void do_push_apss_rewind_tape(void);
	void set_cmt_sound(bool);

	int write_protect_fd(int drv, bool flag);
	void eject_fd(int drv);
	virtual void do_update_recent_disk(int);
	virtual int set_d88_slot(int drive, int num);
	virtual int set_recent_disk(int, int);

	// Bubble Casette
	int write_protect_bubble(int drv, bool flag);

	virtual int set_b77_slot(int drive, int num);
	virtual void do_update_recent_bubble(int drv);
	virtual int set_recent_bubble(int drv, int num);
	void do_change_osd_bubble(int drv, QString tmpstr);

	virtual void _open_bubble(int drv, const QString fname);
	virtual void eject_bubble(int drv);

	void start_record_sound(bool rec);
	void set_freq(int);
	void set_latency(int);
	void set_sound_device(int);
	void set_monitor_type(int);
	void message_status_bar(QString);
	void resize_statusbar(int w, int h);
	virtual void do_release_emu_resources(void);
	virtual void set_window_title();
	void set_device_type(int);
	void set_drive_type(int);
	void set_scan_line(bool);
	void set_gl_scan_line_vert(bool);
	void set_gl_scan_line_horiz(bool);
	void set_dipsw(int num, bool flag) {
		if((num < 0) || (num >= 32)) return;
		if(flag) {
			using_flags->get_config_ptr()->dipswitch = using_flags->get_config_ptr()->dipswitch | (1 << num);
		} else {
			using_flags->get_config_ptr()->dipswitch = using_flags->get_config_ptr()->dipswitch & ~(1 << num);
		}
	}
	bool get_dipsw(int num) {
		if((num < 0) || (num >= 32)) return false;
		if(((1 << num) & using_flags->get_config_ptr()->dipswitch) == 0) return false;
		return true;
	}
	void set_printer_device(int);
	void do_show_about(void);
	void do_browse_document(QString);
	void do_set_conslog(bool);
	void do_set_syslog(bool);
signals:
	int message_changed(QString);
	int quit_emu_thread();
	int call_joy_thread(EMU *);
	int quit_joy_thread();
	int quit_draw_thread();
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
	int do_recent_cmt(bool);
	int closed(void);
	int sig_quit_all(void);
	int sig_vm_reset(void);
	int sig_vm_specialreset(void);
	int sig_vm_loadstate(void);
	int sig_vm_savestate(void);
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
	
	int sig_write_protect_disk(int drv, bool flag);
	int sig_open_disk(int, QString, int);
	int sig_close_disk(int);
	int sig_play_tape(QString name);
	int sig_rec_tape(QString name);
	int sig_close_tape(void);
	int sig_cmt_push_play(void);
	int sig_cmt_push_stop(void);
	int sig_cmt_push_fast_forward(void);
	int sig_cmt_push_fast_rewind(void);
	int sig_cmt_push_apss_forward(void);
	int sig_cmt_push_apss_rewind(void);
	int sig_write_protect_quickdisk(int drv, bool flag);
	int sig_close_quickdisk(int drv);
	int sig_open_quickdisk(int drv, QString path);
	int sig_close_cart(int drv);
	int sig_open_cart(int drv, QString path);
	int sig_open_cdrom(QString path);
	int sig_close_cdrom(void);
	int sig_close_laserdisc(void);
	int sig_open_laserdisc(QString path);
	int sig_load_binary(int drv, QString path);
	int sig_save_binary(int drv, QString path);
	int sig_write_protect_bubble(int, bool);
	int sig_open_bubble(int, QString , int);
	int sig_close_bubble(int);
	int sig_led_update(QRectF);
	int sig_start_auto_key(QString);
	int sig_stop_auto_key(void);
   
	int quit_debugger_thread(void);
	int sig_quit_widgets(void);
};
QT_END_NAMESPACE

#endif
