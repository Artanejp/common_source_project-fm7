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
#include "emu.h"
#include "vm.h"

#include "qt_main.h"

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

#ifndef _SCREEN_MODE_NUM
#define _SCREEN_MODE_NUM 32
#endif

class Ui_MainWindow : public QMainWindow
{
	Q_OBJECT
 protected:
	QMainWindow *MainWindow;
	QApplication *CoreApplication;
	
	QWidget *centralwidget;
	GLDrawClass *graphicsView;
	QStatusBar  *statusbar;
	QMenuBar    *menubar;
	QTimer *statusUpdateTimer;
#ifdef SUPPORT_DUMMY_DEVICE_LED
	QTimer *ledUpdateTimer;
#endif
	QIcon WindowIcon;
	int screen_mode_count;
	
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
	virtual void retranslateVolumeLabels(Ui_SoundDialog *);
	
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
	//class Action_Control *actionClose_Debuggers;
#endif

#if defined(USE_CART1) || defined(USE_CART2)
	QStringList listCARTs[2];
#endif

#if defined(USE_QD1) || defined(USE_QD2)
	QStringList listQDs[2];
#endif
#ifdef USE_TAPE    
	QStringList listCMT;
	bool cmt_write_protect;
#endif    
#if defined(USE_LASER_DISC)
	class Action_Control *actionInsert_LD;
	class Action_Control *actionEject_LD;
	QActionGroup   *actionGroup_Opened_LD;
	class Action_Control *actionRecent_Opened_LD;
	class Action_Control *action_Recent_List_LD[MAX_HISTORY];
	QStringList listLaserDisc;
#endif
#if defined(USE_BINARY_FILE1)
	QStringList listBINs[8];
#endif
	// Screen
	QActionGroup *actionGroup_Stretch;
	class Action_Control *actionZoom;
	class Action_Control *actionDisplay_Mode;
#if defined(USE_SCANLINE)	
	class Action_Control *actionScanLine;
#endif
	class Action_Control *actionGLScanLineHoriz;
	class Action_Control *actionGLScanLineVert;
	class Action_Control *actionRotate;
#ifdef USE_CRT_FILTER   
	class Action_Control *actionCRT_Filter;
#endif   
	class Action_Control *actionOpenGL_Filter;
	class Action_Control *actionDot_by_Dot;
	class Action_Control *actionKeep_Aspect;
	class Action_Control *actionFill_Display;
	class Action_Control *actionCapture_Screen;

	QActionGroup *actionGroup_ScreenSize;
	class Action_Control *actionScreenSize[_SCREEN_MODE_NUM]; 
	class Action_Control *actionAbout;
	class Action_Control *actionHelp_README;
	class Action_Control *actionHelp_README_QT;
	class Action_Control *actionHelp_README_MR_TANAM;
	class Action_Control *actionHelp_README_Artane;
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
#ifdef DATAREC_SOUND
	class Action_Control *actionSoundCMT;
#endif

	class Action_Control *action_Freq[8];
	class Action_Control *action_Latency[6];
	class Action_Control *actionStart_Record;
	class Action_Control *actionStop_Record;
	class Action_Control *actionStart_Record_Movie;
	class Action_Control *actionStop_Record_Movie;
	class Action_Control *action_VolumeDialog;
	
	class Action_Control *actionMouseEnable;

	class Action_Control *actionHelp_AboutQt;

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
#ifdef USE_MULTIPLE_SOUNDCARDS
	class Action_Control *actionSoundMultipleSpeakers;
#endif
#ifdef USE_PRINTER
	QActionGroup *actionGroup_PrintDevice;
	QMenu *menuPrintDevice;
  #if defined(USE_PRINTER_TYPE)
	class Action_Control *actionPrintDevice[USE_PRINTER_TYPE];
  #else	
	class Action_Control *actionPrintDevice[2];
  #endif
#endif
	// Menus    
	QMenu *menuControl;
	QMenu *menuState;
	QMenu *menuCopy_Paste;
	QMenu *menuCpu_Speed;
	QMenu *menuDebugger;
	
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	Menu_FDClass *menu_fds[MAX_FD];
	QStringList listFDs[MAX_FD];
	QStringList listD88[MAX_FD];
#endif
#if defined(USE_QD1) || defined(USE_QD2)
	Menu_QDClass *menu_QDs[2];
#endif
#ifdef USE_TAPE    
	Menu_CMTClass *menu_CMT;
#endif
#ifdef USE_CART1
	Menu_CartClass *menu_Cart[2];
#endif	
#ifdef USE_LASER_DISC    
	QMenu *menuLD;
	QMenu *menuLD_Recent;
#endif
#if defined(USE_BINARY_FILE1)
	Menu_BinaryClass *menu_BINs[8];
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
	QMenu *menuHelp_Readme;
	QMenu *menuHelp_Histories;
	// Status Bar
	QWidget *dummyStatusArea1;
	QLabel *messagesStatusBar;
	QWidget *dummyStatusArea2;
#ifdef USE_FD1
	QLabel *fd_StatusBar[8];
	QString osd_str_fd[8];
#endif
#ifdef USE_QD1
	QLabel *qd_StatusBar[8];
	QString osd_str_qd[8];
#endif
#ifdef USE_TAPE
	QLabel *cmt_StatusBar;
	QString osd_str_cmt;
#endif
#ifdef USE_BITMAP
	QImage *bitmapImage;
#endif
#ifdef SUPPORT_DUMMY_DEVICE_LED
	bool flags_led[SUPPORT_DUMMY_DEVICE_LED];
	bool flags_led_bak[SUPPORT_DUMMY_DEVICE_LED];
	QGraphicsView *led_graphicsView;
	QGraphicsScene *led_gScene;
	QGraphicsEllipseItem *led_leds[SUPPORT_DUMMY_DEVICE_LED];
	uint32_t osd_led_data;
#endif
#ifdef USE_AUTO_KEY
	QClipboard *ClipBoard;
#endif	
	// About Status bar
	virtual void initStatusBar(void);
	// Constructor
	class EmuThreadClass *hRunEmu;
	class DrawThreadClass *hDrawEmu;
#if defined(USE_JOYSTICK)	
	class JoyThreadClass *hRunJoy;
#endif	
public:
	Ui_MainWindow(QWidget *parent = 0);
	~Ui_MainWindow();
	// Initializer : using from InitContext.
	void setCoreApplication(QApplication *p);
	
	void createContextMenu(void);
	void setupUi(void);
	void set_window(int mode);
	// Belows are able to re-implement.
	virtual void retranslateUi(void);
	virtual void retranslateUI_Help(void);
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
	void do_emu_update_config(void);
	void delete_joy_thread(void);
	virtual void redraw_status_bar(void);
#ifdef SUPPORT_DUMMY_DEVICE_LED
	virtual void redraw_leds(void);
	void do_recv_data_led(quint32 d);
#endif
	void set_screen_aspect(int num);
	void set_screen_size(int w, int h);
	void OnReset(void);
	void OnSpecialReset(void);
	void do_set_mouse_enable(bool flag);
	void do_toggle_mouse(void);
	void do_set_sound_device(int);
	void rise_volume_dialog(void);
#ifdef USE_MULTIPLE_SOUNDCARDS
	void set_multiple_speakers(bool flag);
#endif
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
#ifdef USE_CRT_FILTER	
	void set_crt_filter(bool);
#endif
	void set_gl_crt_filter(bool);
	void set_cpu_power(int pw) {
		OnCpuPower(pw);
	}
	void on_actionExit_triggered() {
		save_config(create_local_path(_T("%s.ini"), _T(CONFIG_NAME)));
		OnMainWindowClosed();
	}
#ifdef USE_AUTO_KEY
	void OnStartAutoKey(void);
	void OnStopAutoKey(void);
#endif

#ifdef USE_FD1
	void do_update_recent_disk(int);
	void do_change_osd_fd(int drv, QString tmpstr);
#endif
#ifdef USE_CART1
	void eject_cart(int);
	void set_recent_cart(int, int);
#endif
#if defined(USE_BINARY_FILE1)
	void CreateBinaryMenu(int drv, int drv_base);
	void CreateBinaryPulldownMenu(int drv);
	void ConfigBinaryMenuSub(int drv);
	int set_recent_binary_load(int drv, int num);
	int set_recent_binary_save(int drv, int num);
	void _open_binary_load(int drive, const QString fname);
	void _open_binary_save(int drive, const QString fname);
#endif

//#ifdef USE_QD1
	void open_quick_disk_dialog(int drv);
	int set_recent_quick_disk(int drive, int num); 
	int write_protect_Qd(int drv, bool flag);
	void _open_quick_disk(int drv, const QString fname);
	void eject_Qd(int drv);
#ifdef USE_QD1
	void do_change_osd_qd(int drv, QString tmpstr);
#endif
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
	void do_write_protect_cmt(int drv, bool flag);
	int  set_recent_cmt(int drv, int num);
	void set_wave_shaper(bool f);
	void set_direct_load_from_mzt(bool f);
	void do_open_write_cmt(QString);
	void do_open_read_cmt(int dummy, QString path);
	void do_change_osd_cmt(QString tmpstr);
# ifdef USE_TAPE_BUTTON
	void do_push_play_tape(void);
	void do_push_stop_tape(void);
	void do_display_tape_play(bool flag);
	void do_push_fast_forward_tape(void);
	void do_push_rewind_tape(void);
	void do_push_apss_forward_tape(void);
	void do_push_apss_rewind_tape(void);
# endif
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
#if defined(USE_PRINTER)	
	void set_printer_device(int);
#endif	
	void set_monitor_type(int);
	void message_status_bar(QString);
	void resize_statusbar(int w, int h);
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
	void set_gl_scan_line_vert(bool);
	void set_gl_scan_line_horiz(bool);
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
	void do_show_about(void);
	void do_browse_document(QString);
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
	int sig_resize_uibar(int, int);
	int sig_resize_screen(int, int);
	int sig_update_screen(void);
	int sig_emu_update_config(void);
	int sig_emu_start_rec_sound(void);
	int sig_emu_stop_rec_sound(void);
	int sig_emu_set_display_size(int, int, int, int);
	
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	int sig_write_protect_disk(int drv, bool flag);
	int sig_open_disk(int, QString, int);
	int sig_close_disk(int);
#endif     
#ifdef USE_TAPE
	int sig_play_tape(QString name);
	int sig_rec_tape(QString name);
	int sig_close_tape(void);
# ifdef USE_TAPE_BUTTON
	int sig_cmt_push_play(void);
	int sig_cmt_push_stop(void);
	int sig_cmt_push_fast_forward(void);
	int sig_cmt_push_fast_rewind(void);
	int sig_cmt_push_apss_forward(void);
	int sig_cmt_push_apss_rewind(void);
# endif
#endif // USE_TAPE
#ifdef USE_QD1	
	int sig_write_protect_quickdisk(int drv, bool flag);
	int sig_close_quickdisk(int drv);
	int sig_open_quickdisk(int drv, QString path);
#endif
#ifdef USE_CART1
	int sig_close_cart(int drv);
	int sig_open_cart(int drv, QString path);
#endif
#ifdef USE_LASER_DISK
	int sig_close_laser_disk(void);
	int sig_open_laser_disk(QString path);
#endif
#ifdef USE_BINARY_FILE1
	int sig_load_binary(int drv, QString path);
	int sig_save_binary(int drv, QString path);
#endif
#ifdef SUPPORT_DUMMY_DEVICE_LED
	int sig_led_update(QRectF);
#endif
#ifdef USE_AUTO_KEY
	int sig_start_auto_key(QString);
	int sig_stop_auto_key(void);
#endif
#ifdef USE_DEBUGGER
	int quit_debugger_thread(void);
#endif
};
QT_END_NAMESPACE

#endif
