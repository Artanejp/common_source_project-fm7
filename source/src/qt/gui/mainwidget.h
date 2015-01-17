/********************************************************************************
** Form generated from reading UI file 'mainwindowjLG445.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

/*
 * MainMenu / X1 Turbo Z
 * Modified by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */

#ifndef _CSP_QT_MAINWIDGET_H
#define _CSP_QT_MAINWIDGET_H

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

#include "simd_types.h"
#include "common.h"
#include "config.h"
#include "emu.h"


#include "qt_main.h"
#include "qt_gldraw.h"
#include "commonclasses.h"

/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  History: Jan 10, 2015 : Initial
 */
QT_BEGIN_NAMESPACE

//#include "menuclasses.h"

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
   void eject_Qd(int drv);

   void retranslateSoundMenu(void);

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
    QActionGroup   *actionGroup_Protect_CART[2]; // Is needed?
    class Action_Control *actionRecent_Opened_CART[2];
    class Action_Control *action_Recent_List_CART[2][MAX_HISTORY];
    class Action_Control *actionInsert_CART[2];
    class Action_Control *actionEject_CART[2];
    class Action_Control *actionProtection_ON_CART[2];
    class Action_Control *actionProtection_OFF_CART[2];
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
#if defined(USE_BINARY)
    QActionGroup   *actionGroup_Opened_BINARY;
    QActionGroup   *actionGroup_Protect_BINARY; // Is needed?
    class Action_Control *actionRecent_Opened_BINARY;
    class Action_Control *action_Recent_List_BINARY[MAX_HISTORY];
    class Action_Control *actionInsert_BINARY;
    class Action_Control *actionEject_BINARY;
    class Action_Control *actionProtection_ON_BINARY;
    class Action_Control *actionProtection_OFF_BINARY;
#endif
    // Screen
    class Action_Control *actionZoom;
    class Action_Control *actionDisplay_Mode;
    class Action_Control *actionScanLine;
    class Action_Control *actionCRT_Filter;
    class Action_Control *actionDot_by_Dot;
    class Action_Control *actionKeep_Aspect;
    class Action_Control *actionFill_Display;
    class Action_Control *actionCapture_Screen;
    
    class Action_Control *actionAbout;
    QActionGroup   *actionGroup_Sound_Freq;
    QActionGroup   *actionGroup_Sound_Latency;
    
    class Action_Control *action_Freq[8];
    
    class Action_Control *action_Latency[6];
    
    class Action_Control *actionStart_Record;
    class Action_Control *actionStop_Record;
    class Action_Control *actionStart_Record_Movie;
    class Action_Control *actionStop_Record_Movie;
    
    // Manus    
    QMenu *menuControl;
    QMenu *menuState;
    QMenu *menuCopy_Paste;
    QMenu *menuCpu_Speed;
    QMenu *menuDebugger;
    
#if defined(USE_CART1) || defined(USE_CART2)
    QMenu *menuCART[8];
    QMenu *menuCART_Recent[8];
    QMenu *menuCART_D88[8];
    QMenu *menuWrite_Protection_CART[8];
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
#if defined(USE_BINARY)
    QMenu *menuBINARY[1];
    QMenu *menuBINARY_Recent[1];
    QMenu *menuWrite_Protection_BINARY[1];
#endif
    
    QMenu *menuScreen;
    QMenu *menuStretch_Mode;
    QMenu *menuSound;
    QMenu *menuOutput_Frequency;
    QMenu *menuSound_Latency;
    QMenu *menuMachine;
    QMenu *menuRecord;
    QMenu *menuRecord_sound;
    QMenu *menuRecoad_as_movie;
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
    // About Status bar
    virtual void initStatusBar(void);
     // Constructor
    class EmuThreadClass *hRunEmu;
    class EmuThreadCore  *hRunEmuThread;
    bool bRunEmuThread;
   
    class JoyThreadClass *hRunJoy;
    class JoyThreadCore  *hRunJoyThread;
    bool bRunJoyThread;
public:
 Ui_MainWindow(QWidget *parent = 0);
  ~Ui_MainWindow();
  // Initializer : using from InitContext.
   void createContextMenu(void);
   void setupUi(void);
   virtual void retranslateUi(void);
  // EmuThread
   void StopEmuThread(void);
   void LaunchEmuThread(void);
   void StopJoyThread(void);
   void LaunchJoyThread(void);
   bool GetEmuThreadEnabled(void) {
	return bRunEmuThread;
  }
   bool GetJoyThreadEnabled(void) {
	return bRunJoyThread;
  }
  // Getting important widgets.
   QMainWindow *getWindow(void) { return MainWindow; }
   QMenuBar    *getMenuBar(void) { return menubar;}
   GLDrawClass *getGraphicsView(void) { return graphicsView; }
   QStatusBar *getStatusBar(void) { return statusbar;}
   void OnMainWindowClosed(void);
  // Basic Action Definition
   void OnCpuPower(int mode);
#ifdef USE_DEBUGGER
    void OnOpenDebugger(int no);
#endif   
//#ifdef USE_TAPE    
   bool get_wave_shaper(void);
   bool get_direct_load_mzt(void);
//#endif
    // Basic slots
 public slots:
   virtual void redraw_status_bar(void);
#ifdef USE_FD1
   void open_disk_dialog(int drv);
#endif
//#ifdef USE_QD1
   void open_quick_disk_dialog(int drv);
   int set_recent_quick_disk(int drive, int num); 
   int write_protect_Qd(int drv, bool flag);
   void _open_quick_disk(int drv, const QString fname);
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
   void OnPushPlayButton(void); // Obsolete
   void OnPushStopButton(void); // Obsolete
   void do_push_play_tape(void);
   void do_push_stop_tape(void);
#endif
#endif
   int write_protect_fd(int drv, bool flag);
   void eject_fd(int drv);
   void on_actionExit_triggered() {
	save_config();
	OnMainWindowClosed();
	QApplication::quit();
     }
   void OnReset(void);
   void OnSpecialReset(void);
#ifdef USE_STATE
   void OnLoadState(void);
   void OnSaveState(void);
#endif
   void set_cpu_power(int pw) {
	OnCpuPower(pw);
  }
   
#ifdef USE_AUTO_KEY
   void OnStartAutoKey(void);
   void OnStopAutoKey(void);
#endif
#ifdef USE_DEBUGGER
   void OnCloseDebugger(void);
   void open_debugger(int no){
	OnOpenDebugger(no);
  }
#endif
#ifdef USE_FD1
   int set_d88_slot(int drive, int num);
   int set_recent_disk(int, int);
#endif
   void start_record_sound(bool rec);
   void set_freq(int);
   void set_latency(int);
   void set_sound_device(int);
   void message_status_bar(QString);
signals:
   int call_emu_thread(EMU *);
   int call_joy_thread(EMU *);
   int on_boot_mode(int);
   int on_cpu_type(int);
   int on_cpu_power(int);
   int on_open_debugger(int);
   int on_insert_fd(int);
   int on_eject_fd(int);
   int do_open_disk(int, QString);
   int do_recent_cmt(bool);
};
//namespace Ui {
//    class Ui_MainWindow;
//} // namespace Ui

QT_END_NAMESPACE

#endif
