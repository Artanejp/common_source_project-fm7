/*
 * MainWidget : Defines
 * Modified by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */

#ifndef _CSP_QT_MAINWIDGET_H
#define _CSP_QT_MAINWIDGET_H

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
#include "emu.h"


#include "qt_main.h"
#include "qt_gldraw.h"
#include "commonclasses.h"

QT_BEGIN_NAMESPACE

//#include "menuclasses.h"

#ifndef _SCREEN_MODE_NUM
#define _SCREEN_MODE_NUM 32
#endif

class Ui_MainWindow : public QMainWindow
{
	Q_OBJECT
 protected:
	QMainWindow *MainWindow;
	QWidget *centralwidget;
	GLDrawClass *graphicsView;
	QStatusBar  *statusbar;
	QMenuBar    *menubar;
	QTimer *statusUpdateTimer;
	QIcon WindowIcon;
    
	// Some Functions
	void ConfigCpuSpeed(void);
	void ConfigControlMenu(void);
	void connectActions_ControlMenu(void);
	void retranslateControlMenu(const char *SpecialResetTitle,  bool WithSpecialReset);
	void ConfigFloppyMenu(void);
	void ConfigSoundMenu(void);
	void CreateSoundMenu(void);

	void OnWindowResize(void);
	void OnWindowMove(void);
	void OnWindowRedraw(void);
	void CreateFloppyMenu(int drv, int drv_base);
	void CreateFloppyPulldownMenu(int drv);
	void ConfigFloppyMenuSub(int drv);
	void retranslateFloppyMenu(int drv, int basedrv);
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
	virtual void retranslateCartMenu(int drv, int basedrv);

	void ConfigBinaryMenu(void);
	void retranslateBinaryMenu(int drv, int basedrv);
	
	void retranslateSoundMenu(void);
	
	void ConfigScreenMenu(void);
	void ConfigScreenMenu_List(void);
	void CreateScreenMenu(void);
	void ConfigDeviceType(void);
	void ConfigDriveType(void);
	void ConfigSoundDeviceType(void);
	
	void retranslateScreenMenu(void);
	class Action_Control *actionReset;
	class Action_Control *actionSpecial_Reset;
	class Action_Control *actionExit_Emulator;
#ifdef USE_CPU_TYPE
	// Pls.Override
	QActionGroup *actionGroup_CpuType;
	QMenu *menuCpuType;
	class Action_Control *actionCpuType[8];
	void ConfigCPUTypes(int num);
#endif
	QActionGroup *actionGroup_CpuSpeed;
	class Action_Control *actionSpeed_x1;
	class Action_Control *actionSpeed_x2;
	class Action_Control *actionSpeed_x4;
	class Action_Control *actionSpeed_x8;
	class Action_Control *actionSpeed_x16;

#ifdef USE_BOOT_MODE
	// Pls.Override
	QActionGroup *actionGroup_BootMode;
	QMenu *menuBootMode;
	class Action_Control *actionBootMode[8];
	void ConfigCPUBootMode(int num);
#endif    
	class Action_Control *actionPaste_from_Clipboard;
	class Action_Control *actionStop_Pasting;
#ifdef USE_STATE
	class Action_Control *actionSave_State;
	class Action_Control *actionLoad_State;
#endif
#ifdef USE_DEBUGGER
	class Action_Control *actionDebugger_1;
	class Action_Control *actionDebugger_2;
	class Action_Control *actionDebugger_3;
	class Action_Control *actionClose_Debuggers;
#endif

#if defined(USE_CART1) || defined(USE_CART2)
	QActionGroup   *actionGroup_Opened_CART[2];
	class Action_Control *actionRecent_Opened_CART[2];
	class Action_Control *action_Recent_List_CART[2][MAX_HISTORY];
	class Action_Control *actionInsert_CART[2];
	class Action_Control *actionEject_CART[2];
#endif

#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
  defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	QActionGroup   *actionGroup_Opened_FD[8];
	QActionGroup   *actionGroup_Protect_FD[8];
	class Action_Control *actionRecent_Opened_FD[8];
	class Action_Control *action_Recent_List_FD[8][MAX_HISTORY];
	
	QActionGroup   *actionGroup_D88_Image_FD[8];
	class Action_Control *actionSelect_D88_Image_FD[8];
	class Action_Control *action_D88_ListImage_FD[8][64];
	class Action_Control *actionInsert_FD[8];
	class Action_Control *actionEject_FD[8];
	class Action_Control *actionIgnoreCRC[8];
	
	class Action_Control *actionProtection_ON_FD[8];
	class Action_Control *actionProtection_OFF_FD[8];
#endif

#if defined(USE_QD1) || defined(USE_QD2)
	QActionGroup   *actionGroup_Opened_QD[2];
	QActionGroup   *actionGroup_Protect_QD[2];
	class Action_Control *actionRecent_Opened_QD[2];
	class Action_Control *action_Recent_List_QD[2][MAX_HISTORY];
	class Action_Control *actionInsert_QD[2];
	class Action_Control *actionEject_QD[2];
	class Action_Control *actionProtection_ON_QD[2];
	class Action_Control *actionProtection_OFF_QD[2];
#endif
#ifdef USE_TAPE    
	QActionGroup   *actionGroup_Opened_CMT;
	QActionGroup   *actionGroup_Protect_CMT;
	class Action_Control *actionWaveShaper;
	class Action_Control *actionDirectLoadMZT;
	class Action_Control *actionRecent_Opened_CMT;
	class Action_Control *action_Recent_List_CMT[MAX_HISTORY];
	class Action_Control *actionInsert_CMT;
	class Action_Control *actionEject_CMT;
#ifdef USE_TAPE_BUTTON
	QActionGroup *actionGroup_PlayTape;
	class Action_Control *actionPlay_Start;
	class Action_Control *actionPlay_Stop;
#endif    
	class Action_Control *actionRecording;
	class Action_Control *actionProtection_ON_CMT;
	class Action_Control *actionProtection_OFF_CMT;
	bool write_protect;
#endif    
#if defined(USE_LASER_DISC)
	class Action_Control *actionInsert_LD;
	class Action_Control *actionEject_LD;
	QActionGroup   *actionGroup_Opened_LD;
	class Action_Control *actionRecent_Opened_LD;
	class Action_Control *action_Recent_List_LD[MAX_HISTORY];
#endif
#if defined(USE_BINARY_FILE1)
	QActionGroup   *actionGroup_Opened_BIN[8];
	QActionGroup   *actionGroup_Protect_BIN[8]; // Is needed?
	class Action_Control *actionRecent_Opened_BIN[8];
	class Action_Control *action_Recent_List_BIN[8][MAX_HISTORY];
	class Action_Control *actionLoad_BIN[8];
	class Action_Control *actionSave_BIN[8];
#endif
	// Screen
	QActionGroup *actionGroup_Stretch;
	class Action_Control *actionZoom;
	class Action_Control *actionDisplay_Mode;
	class Action_Control *actionScanLine;
	class Action_Control *actionRotate;
	class Action_Control *actionCRT_Filter;
	class Action_Control *actionDot_by_Dot;
	class Action_Control *actionKeep_Aspect;
	class Action_Control *actionFill_Display;
	class Action_Control *actionCapture_Screen;

	QActionGroup *actionGroup_ScreenSize;
	class Action_Control *actionScreenSize[_SCREEN_MODE_NUM]; 
	class Action_Control *actionAbout;
	QActionGroup   *actionGroup_Sound_Freq;
	QActionGroup   *actionGroup_Sound_Latency;
#ifdef DATAREC_SOUND
	class Action_Control *actionSoundCMT;
#endif
	class Action_Control *action_Freq[8];
	class Action_Control *action_Latency[6];
	class Action_Control *actionStart_Record;
	class Action_Control *actionStop_Record;
	class Action_Control *actionStart_Record_Movie;
	class Action_Control *actionStop_Record_Movie;
	
	class Action_Control *actionMouseEnable;
#ifdef USE_DEVICE_TYPE
	QActionGroup *actionGroup_DeviceType;
	QMenu *menuDeviceType;
	class Action_Control *actionDeviceType[USE_DEVICE_TYPE];
#endif   
#ifdef USE_DRIVE_TYPE
	QActionGroup *actionGroup_DriveType;
	QMenu *menuDriveType;
	class Action_Control *actionDriveType[USE_DRIVE_TYPE];
#endif   
#ifdef USE_SOUND_DEVICE_TYPE
	QActionGroup   *actionGroup_SoundDevice;
	QMenu *menuSoundDevice;
	class Action_Control *actionSoundDevice[USE_SOUND_DEVICE_TYPE]; //
#endif
	// Menus    
	QMenu *menuControl;
	QMenu *menuState;
	QMenu *menuCopy_Paste;
	QMenu *menuCpu_Speed;
	QMenu *menuDebugger;
#if defined(USE_CART1) || defined(USE_CART2)
	QMenu *menuCART[8];
	QMenu *menuCART_Recent[8];
#endif
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	QMenu *menuFD[8];
	QMenu *menuFD_Recent[8];
	QMenu *menuFD_D88[8];
	QMenu *menuWrite_Protection_FD[8];
#endif
#if defined(USE_QD1) || defined(USE_QD2)
	QMenu *menuQD[2];
	QMenu *menuQD_Recent[2];
	QMenu *menuWrite_Protection_QD[2];
#endif
#ifdef USE_TAPE    
	QMenu *menuCMT;
	QMenu *menuCMT_Recent;
	QMenu *menuWrite_Protection_CMT;
#endif
#ifdef USE_LASER_DISC    
	QMenu *menuLD;
	QMenu *menuLD_Recent;
#endif
#if defined(USE_BINARY_FILE1)
	QMenu *menuBIN[8];
	QMenu *menuBIN_Recent[8];
#endif
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
	// Status Bar
	QWidget *dummyStatusArea1;
	QLabel *messagesStatusBar;
	QWidget *dummyStatusArea2;
#ifdef USE_FD1
	QLabel *fd_StatusBar[8];
#endif
#ifdef USE_QD1
	QLabel *qd_StatusBar[8];
#endif
#ifdef USE_TAPE
	QLabel *cmt_StatusBar;
#endif
#ifdef USE_BITMAP
	QImage *bitmapImage;
#endif
	// About Status bar
	virtual void initStatusBar(void);
	// Constructor
	class EmuThreadClass *hRunEmu;
	class DrawThreadClass *hDrawEmu;
   
	class JoyThreadClass *hRunJoy;
public:
	Ui_MainWindow(QWidget *parent = 0);
	~Ui_MainWindow();
	// Initializer : using from InitContext.
	void createContextMenu(void);
	void setupUi(void);
	void set_window(int mode);
	// Belows are able to re-implement.
	virtual void retranslateUi(void);
	// EmuThread
	void StopEmuThread(void);
	void LaunchEmuThread(void);
	
	void StopJoyThread(void);
	void LaunchJoyThread(void);
   
	// Getting important widgets.
	QMainWindow *getWindow(void) { return MainWindow; }
	QMenuBar    *getMenuBar(void) { return menubar;}
	GLDrawClass *getGraphicsView(void) { return graphicsView; }
	QStatusBar *getStatusBar(void) { return statusbar;}
#ifdef USE_BITMAP
	QImage *getBitmapImage(void) { return bitmapImage; }
#endif
	
	void OnMainWindowClosed(void);
	// Basic Action Definition
	void OnCpuPower(int mode);
	//#ifdef USE_TAPE    
	bool get_wave_shaper(void);
	bool get_direct_load_mzt(void);
	//#endif
#ifdef USE_POWER_OFF
	bool GetPowerState(void);
#endif
   
	// Basic slots
public slots:
	void delete_emu_thread(void);
	void doChangeMessage_EmuThread(QString str);

	void delete_joy_thread(void);
	virtual void redraw_status_bar(void);
	void set_screen_aspect(int num);
	void set_screen_size(int w, int h);
	void OnReset(void);
	void OnSpecialReset(void);
	void do_set_mouse_enable(bool flag);
	void do_toggle_mouse(void);
	void do_set_sound_device(int);
	
#ifdef USE_STATE
	void OnLoadState(void);
	void OnSaveState(void);
#endif
#ifdef USE_DEBUGGER
	void OnOpenDebugger(int n);
	void OnCloseDebugger(void);
#endif
#ifdef USE_SCREEN_ROTATE
	void set_screen_rotate(bool);
#endif   
	void set_cpu_power(int pw) {
		OnCpuPower(pw);
	}
	void on_actionExit_triggered() {
		save_config();
		OnMainWindowClosed();
	}
#ifdef USE_AUTO_KEY
	void OnStartAutoKey(void);
	void OnStopAutoKey(void);
#endif

#ifdef USE_FD1
	void open_disk_dialog(int drv);
#endif
#ifdef USE_CART1
	void open_cart_dialog(int);
	void eject_cart(int);
	void set_recent_cart(int, int);
#endif
#if defined(USE_BINARY_FILE1)
	void open_binary_dialog(int drive, bool load);
	void CreateBinaryMenu(int drv, int drv_base);
	void CreateBinaryPulldownMenu(int drv);
	void ConfigBinaryMenuSub(int drv);
	int set_recent_binary_load(int drv, int num);
	int set_recent_binary_save(int drv, int num);
	void _open_binary(int drive, const QString fname, bool load);
#endif

//#ifdef USE_QD1
	void open_quick_disk_dialog(int drv);
	int set_recent_quick_disk(int drive, int num); 
	int write_protect_Qd(int drv, bool flag);
	void _open_quick_disk(int drv, const QString fname);
	void eject_Qd(int drv);
//#endif
	void _open_disk(int drv, const QString fname);
	void _open_cart(int drv, const QString fname);
	void _open_cmt(bool mode,const QString path);
	void eject_cmt(void);
#ifdef USE_BOOT_MODE
	void do_change_boot_mode(int mode);
#endif
#ifdef USE_CPU_TYPE
	void do_change_cpu_type(int mode);
#endif
#ifdef USE_TAPE
	void open_cmt_dialog(bool play);
	void do_write_protect_cmt(bool flag);
	int  set_recent_cmt(int num);
	void set_wave_shaper(bool f);
	void set_direct_load_from_mzt(bool f);
#ifdef USE_TAPE_BUTTON
	void do_push_play_tape(void);
	void do_push_stop_tape(void);
	void do_display_tape_play(bool flag);
#endif
#endif
#ifdef DATAREC_SOUND
	void set_cmt_sound(bool);
#endif
	int write_protect_fd(int drv, bool flag);
	void eject_fd(int drv);
#ifdef USE_FD1
	int set_d88_slot(int drive, int num);
	int set_recent_disk(int, int);
#endif
	void start_record_sound(bool rec);
	void set_freq(int);
	void set_latency(int);
	void set_sound_device(int);
	void set_monitor_type(int);
	void message_status_bar(QString);
	void do_release_emu_resources(void);
# if defined(USE_DEVICE_TYPE)
	void set_device_type(int);
# endif
# if defined(USE_DRIVE_TYPE)
	void set_drive_type(int);
# endif
# if defined(USE_SCANLINE)
	void set_scan_line(bool);
# endif

#if defined(USE_DIPSWITCH)
	void set_dipsw(int num, bool flag) {
		if((num < 0) || (num >= 32)) return;
		if(flag) {
			config.dipswitch = config.dipswitch | (1 << num);
		} else {
			config.dipswitch = config.dipswitch & ~(1 << num);
		}
	}
	bool get_dipsw(int num) {
		if((num < 0) || (num >= 32)) return false;
		if(((1 << num) & config.dipswitch) == 0) return false;
		return true;
	}
#endif
   
      
signals:
	int message_changed(QString);
	int quit_emu_thread();
	int call_joy_thread(EMU *);
	int quit_joy_thread();
	int quit_draw_thread();
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
};
QT_END_NAMESPACE

#endif
