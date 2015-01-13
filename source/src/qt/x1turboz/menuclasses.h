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

#ifndef MAINWINDOWJLG445_H
#define MAINWINDOWJLG445_H

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

#include "simd_types.h"
#include "common.h"
#include "config.h"
#include "emu.h"


#include "qt_main.h"
#include "qt_gldraw.h"
#include "commonclasses.h"

QT_BEGIN_NAMESPACE

extern class EMU* emu;

class Ui_MainWindow : public QMainWindow
{
    Q_OBJECT
 private:
    QStatusBar *statusbar;
    GLDrawClass *glWidget;
    
 protected:
    // Some Functions
  void ConfigCpuSpeed(Ui_MainWindow* );
  void ConfigControlMenu(Ui_MainWindow* );
  void connectActions_ControlMenu(QMenuBar *p);
  void retranslateControlMenu(Ui_MainWindow *p, const char *SpecialResetTitle,  bool WithSpecialReset);
  void ConfigFloppyMenu(Ui_MainWindow *p);

  void OnWindowResize(QMainWindow *p);
  void OnWindowMove(Ui_MainWindow *p);
  void OnWindowRedraw(Ui_MainWindow *p);
  void CreateFloppyMenu(Ui_MainWindow *p, int drv, int drv_base);
  void CreateFloppyPulldownMenu(Ui_MainWindow *p, int drv);
  void ConfigFloppyMenuSub(Ui_MainWindow *p, int drv);
  void retranslateFloppyMenu(Ui_MainWindow *p, int drv, int basedrv);

  // MainWindow
  QMainWindow *MainWindow;
   
  class Action_Control *actionReset;
  class Action_Control *actionSpecial_Reset;
  class Action_Control *actionExit_Emulator;
//#ifdef USE_CPU_TYPE
  QActionGroup *actionGroup_CpuSpeed;
  class Action_Control *actionSpeed_x1;
  class Action_Control *actionSpeed_x2;
  class Action_Control *actionSpeed_x4;
  class Action_Control *actionSpeed_x8;
  class Action_Control *actionSpeed_x16;
//#endif
#ifdef USE_BOOT_MODE
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
    class Action_Control *actionProtection_ON_CART[2];
    class Action_Control *actionProtection_OFF_CART[2];
#endif

#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
  defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
    QActionGroup   *actionGroup_Opened_FD[8];
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
    class Action_Control *actionRecent_Opened_QD[2];
    class Action_Control *action_Recent_List_QD[2][MAX_HISTORY];
    class Action_Control *actionInsert_QD[2];
    class Action_Control *actionEject_QD[2];
    class Action_Control *actionProtection_ON_QD[2];
    class Action_Control *actionProtection_OFF_QD[2];
#endif
#ifdef USE_TAPE    
    QActionGroup   *actionGroup_Opened_CMT;
    class Action_Control *actionRecent_Opened_CMT;
    class Action_Control *action_Recent_List_CMT[MAX_HISTORY];
    class Action_Control *actionInsert_CMT;
    class Action_Control *actionEject_CMT;
    class Action_Control *actionPlay_Start;
    class Action_Control *actionPlay_Stop;
    class Action_Control *actionRecording;
    class Action_Control *actionProtection_ON_CMT;
    class Action_Control *actionProtection_OFF_CMT;
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
    
    class Action_Control *action2000Hz;
    class Action_Control *action4000Hz;
    class Action_Control *action8000Hz;
    class Action_Control *action11025Hz;
    class Action_Control *action22050Hz;
    class Action_Control *action44100Hz;
    class Action_Control *action48000Hz;
    class Action_Control *action96000Hz;
    
    class Action_Control *action50ms;
    class Action_Control *action100ms;
    class Action_Control *action200ms;
    class Action_Control *action300ms;
    class Action_Control *action400ms;
    
    class Action_Control *actionStart_Record;
    class Action_Control *actionStop_Record;
    class Action_Control *actionStart_Record_Movie;
    class Action_Control *actionStop_Record_Movie;
    
    QWidget *centralwidget;
    GLDrawClass *graphicsView;
    // Manus    
    QMenuBar *menubar;
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
    QMenu *menuQD_D88[2];
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
    QMenu *menuFrequency;
    QMenu *menuRecord;
    QMenu *menuRecord_sound;
    QMenu *menuRecoad_as_movie;
    QMenu *menuEmulator;
    QMenu *menuHELP;
  // Constructor
    SDL_Thread *hRunEmuThread;
    bool bRunEmuThread;
public:
   Ui_MainWindow(QWidget *parent = 0);
  ~Ui_MainWindow();
  // Initializer : using from InitContext.
  void createContextMenu(void);
  void setupUi(void);
  void retranslateUi(Ui_MainWindow *p);
  // EmuThread
  void StopEmuThread(void) {
    bRunEmuThread = false;
    if(hRunEmuThread != NULL) {
      SDL_WaitThread(hRunEmuThread, NULL);
      hRunEmuThread = NULL;
    }
  }
  void LaunchEmuThread(int (*fn)(void *))
  {
    bRunEmuThread = true;
    hRunEmuThread = SDL_CreateThread(fn, (void *)this);
  }
  bool GetEmuThreadEnabled(void) {
	return bRunEmuThread;
  }
   
  // Getting important widgets.
  QMainWindow *getWindow(void) { return MainWindow; }
  QMenuBar    *getMenuBar(void) { return menubar;}
  GLDrawClass *getGraphicsView(void) { return graphicsView; }
  QStatusBar *getStatusBar(void) { return statusbar;}
  void OnMainWindowClosed(void);
  // Basic Action Definition
    void OnCpuPower(int mode);
#ifdef USE_BOOT_MODE
     void OnBootMode(int mode);
#endif
#ifdef USE_CPU_TYPE
     void OnCpuType(int mode);
#endif
#ifdef USE_DEBUGGER
     void OnOpenDebugger(int no);
#endif   
     // Basic slots
 public slots:
     void open_disk_dialog(int drv);
     void _open_disk(int drv, const QString fname);
     void _open_cart(int drv, const QString fname);
     void _open_cmt(bool mode,const QString path);
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
#ifdef USE_BOOT_MODE
     void set_boot_mode(int mode) {
	OnBootMode(mode);
     }
#endif
#ifdef USE_CPU_TYPE
     void set_cpu_type(int mode) {
	OnCpuType(mode);
     }
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
     int set_d88_slot(int drive, int num);
     int set_recent_disk(int, int);

signals:
   int on_boot_mode(int);
   int on_cpu_type(int);
   int on_cpu_power(int);
   int on_open_debugger(int);
   int on_insert_fd(int);
   int on_eject_fd(int);
   int do_open_disk(int, QString);
};
namespace Ui {
    class Ui_MainWindow;
} // namespace Ui

QT_END_NAMESPACE

#endif
