/********************************************************************************
** Form generated from reading UI file 'mainwindowjLG445.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

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

QT_BEGIN_NAMESPACE


extern class EMU* emu;

typedef class Object_Menu_Control: public QObject {
    Q_OBJECT
public:
//   explicit Object_Menu_Control(QObject *parent = 0);
   Object_Menu_Control(QObject *parent) : QObject(parent){
      bindValue = 0;
      drive = 0;
      s_num = 0;
   }
   Object_Menu_Control() {}
   // Virtual Functions
//   bool event(QEvent *e) { return true;} 
//   bool eventFilter ( QObject * watched, QEvent * event ){
//	return true;
//   }
//   void childEvent (QChildEvent * event ){ };
//   void connectNotify ( const char * signal ) {}
//   void customEvent ( QEvent * event ) { }
//   void disconnectNotify ( const char * signal ) {  }
//   void timerEvent ( QTimerEvent * event ){ }
   // End
private:
 int bindValue;
 int drive;
 int s_num;
signals:
     int on_boot_mode(int);   
     int on_cpu_type(int);   
     int on_cpu_power(int); 
     int on_open_debugger(int);
     int sig_insert_fd(int);
     int sig_eject_fd(int);
     int set_d88_slot(int, int);
     int set_recent_disk(int, int);
public slots:
     void set_boot_mode(void);
     void set_cputype(void);
     void set_cpupower(void);
     void open_debugger(void);
     void insert_fd(void);
     void eject_fd(void);
     void on_d88_slot(void);
     void on_recent_disk(void);
 public:
   void setValue1(int v) {bindValue = v;}
   int getValue1(void) {return bindValue;}
   void setDrive(int num) { drive = num;}
   int getDrive(void) { return drive;}
   void setNumber(int num) { s_num = num;}
   int getNumber(void) { return s_num;}
	
	
} Object_Menu_Control ;

typedef class Action_Control: public QAction {
    Q_OBJECT
  protected:
//    virtual void addedTo ( QWidget * actionWidget, QWidget * container ){}
 //   virtual void addedTo ( int index, QPopupMenu * menu ){}
 public:
  Object_Menu_Control *binds;
  Action_Control (QObject *parent) : QAction(parent) {
     binds = new Object_Menu_Control(parent);
  }
  ~Action_Control() {
	delete binds;
  }
// Action_Control(QObject * parent, const char *name = 0) : QAction(parent, name)
//    {binds.setValue1(0);}
// Action_Control(const QString &menuText, QKeySequence accel, QObject *parent, const char *name = 0) : QAction(menuText, accel, parent, name)
//    {binds.setValue1(0);}
// Action_Control (const QIconSet &icon, const QString &menuText, QKeySequence accel, QObject *parent, const char *name) :
//     QAction(icon, menuText, accel, parent, name)
//    {binds.setValue1(0);}

} ActionControl;




class Ui_MainWindow : public QMainWindow
{
    Q_OBJECT
  private: // Test
     QSlider *createSlider();

     GLDrawClass *glWidget;
     QSlider *xSlider;
     QSlider *ySlider;
     QSlider *zSlider;

 protected:
  void keyPressEvent(QKeyEvent *event);
      // End Test
  void ConfigCpuSpeed(Ui_MainWindow* );
  void ConfigControlMenu(Ui_MainWindow* );
  void connectActions_ControlMenu(QMenuBar *p);
  void retranslateControlMenu(Ui_MainWindow *p, const char *SpecialResetTitle,  bool WithSpecialReset);
  void ConfigFloppyMenu(Ui_MainWindow *p);
  
  QMainWindow *MainWindow;
public:
   
  Action_Control *actionReset;
  Action_Control *actionSpecial_Reset;
  Action_Control *actionExit_Emulator;
//#ifdef USE_CPU_TYPE
  QActionGroup *actionGroup_CpuSpeed;
    Action_Control *actionSpeed_x1;
    Action_Control *actionSpeed_x2;
    Action_Control *actionSpeed_x4;
    Action_Control *actionSpeed_x8;
    Action_Control *actionSpeed_x16;
//#endif
#ifdef USE_BOOT_MODE
#endif    
    Action_Control *actionPaste_from_Clipboard;
    Action_Control *actionStop_Pasting;
#ifdef USE_STATE
    Action_Control *actionSave_State;
    Action_Control *actionLoad_State;
#endif
#ifdef USE_DEBUGGER
    Action_Control *actionDebugger_1;
    Action_Control *actionDebugger_2;
    Action_Control *actionDebugger_3;
    Action_Control *actionClose_Debuggers;
#endif

    QActionGroup   *actionGroup_Opened_FD[8];
    Action_Control *actionRecent_Opened_FD[8];
    Action_Control *action_Recent_List_FD[8][MAX_HISTORY];
   
    QActionGroup   *actionGroup_D88_Image_FD[8];
    Action_Control *actionSelect_D88_Image_FD[8];
    Action_Control *action_D88_ListImage_FD[8][64];
   
#ifdef USE_FD0
    Action_Control *actionInsert_FD0;
    Action_Control *actionEject_FD0;
   
    Action_Control *actionProtection_ON_FD0;
    Action_Control *actionProtection_OFF_FD0;
#endif
#ifdef USE_FD1
    Action_Control *actionInsert_FD1;
    Action_Control *actionEject_FD1;
   
    Action_Control *actionProtection_ON_FD1;
    Action_Control *actionProtection_OFF_FD1;
#endif
#ifdef USE_FD2
    Action_Control *actionInsert_FD2;
    Action_Control *actionEject_FD2;
    Action_Control *actionRecent_Opened_FD2;
    Action_Control *actionSelect_D88_Image_FD2;
    Action_Control *actionProtection_ON_FD2;
    Action_Control *actionProtection_OFF_FD2;

#endif
#ifdef USE_FD3
    Action_Control *actionInsert_FD3;
    Action_Control *actionEject_FD3;
    Action_Control *actionProtection_ON_FD3;
    Action_Control *actionProtection_OFF_FD3;

#endif
#ifdef USE_FD4
    Action_Control *actionInsert_FD4;
    Action_Control *actionEject_FD4;
    Action_Control *actionProtection_ON_FD4;
    Action_Control *actionProtection_OFF_FD4;

#endif
#ifdef USE_FD5
    Action_Control *actionInsert_FD5;
    Action_Control *actionEject_FD5;
    Action_Control *actionProtection_ON_FD5;
    Action_Control *actionProtection_OFF_FD5;

#endif
#ifdef USE_FD6
    Action_Control *actionInsert_FD6;
    Action_Control *actionEject_FD6;
    Action_Control *actionProtection_ON_FD6;
    Action_Control *actionProtection_OFF_FD6;

#endif
#ifdef USE_FD7
    Action_Control *actionInsert_FD7;
    Action_Control *actionEject_FD7;
    Action_Control *actionProtection_ON_FD7;
    Action_Control *actionProtection_OFF_FD7;

#endif
#ifdef USE_FD8
    Action_Control *actionInsert_FD8;
    Action_Control *actionEject_FD8;
    Action_Control *actionProtection_ON_FD8;
    Action_Control *actionProtection_OFF_FD8;

#endif
#ifdef USE_QD1    
    Action_Control *actionInsert_QD1;
    Action_Control *actionEject_QD1;
    Action_Control *actionResent_Images_QD1;
    Action_Control *actionProtection_ON_QD1;
    Action_Control *actionProtection_OFF_QD1;
#endif
#ifdef USE_QD2    
    Action_Control *actionInsert_QD2;
    Action_Control *actionEject_QD2;
    Action_Control *actionResent_Images_QD2;
    Action_Control *actionProtection_ON_QD2;
    Action_Control *actionProtection_OFF_QD2;
#endif
#ifdef USE_TAPE    
    Action_Control *actionInsert_CMT;
    Action_Control *actionEject_CMT;
    Action_Control *actionPlay_Start;
    Action_Control *actionPlay_Stop;
    Action_Control *actionRecording;
    Action_Control *actionProtection_ON_CMT;
    Action_Control *actionProtection_OFF_CMT;

    QActionGroup   *actionGroup_Opened_CMT;
    Action_Control *actionRecent_Opened_CMT;
    Action_Control *action_Recent_List_CMT[MAX_HISTORY];
#endif    
    Action_Control *actionZoom;
    Action_Control *actionDisplay_Mode;
    Action_Control *actionScanLine;
    Action_Control *actionCRT_Filter;
    Action_Control *actionDot_by_Dot;
    Action_Control *actionKeep_Aspect;
    Action_Control *actionFill_Display;
    Action_Control *actionCapture_Screen;
    Action_Control *actionAbout;
    Action_Control *action2000Hz;
    Action_Control *action4000Hz;
    Action_Control *action8000Hz;
    Action_Control *action11025Hz;
    Action_Control *action22050Hz;
    Action_Control *action44100Hz;
    Action_Control *action48000Hz;
    Action_Control *action96000Hz;
    Action_Control *action50ms;
    Action_Control *action100ms;
    Action_Control *action200ms;
    Action_Control *action300ms;
    Action_Control *action400ms;
    Action_Control *actionStart_Record;
    Action_Control *actionStop_Record;
    Action_Control *actionStart_Record_Movie;
    Action_Control *actionStop_Record_Movie;
    QWidget *centralwidget;
//    QGraphicsView *graphicsView;
    GLDrawClass *graphicsView;
    QStatusBar *statusbar;
    QMenuBar *menubar;
    QMenu *menuControl;
    QMenu *menuState;
    QMenu *menuCopy_Paste;
    QMenu *menuCpu_Speed;
    QMenu *menuDebugger;
#ifdef USE_FD1    
    QMenu *menuFD1;
    QMenu *menuFD1_Recent;
    QMenu *menuFD1_D88;
    QMenu *menuWrite_Protection_FD1;
#endif
#ifdef USE_FD2
    QMenu *menuFD2;
    QMenu *menuFD2_Recent;
    QMenu *menuFD2_D88;
    QMenu *menuWrite_Protection_FD2;
#endif
#ifdef USE_FD3
    QMenu *menuFD3;
    QMenu *menuFD3_Recent;
    QMenu *menuFD3_D88;
    QMenu *menuWrite_Protection_FD3;
#endif
#ifdef USE_FD4
    QMenu *menuFD4;
    QMenu *menuWrite_Protection_FD4;
#endif
#ifdef USE_FD4
    QMenu *menuFD4;
    QMenu *menuWrite_Protection_FD4;
#endif
#ifdef USE_FD5
    QMenu *menuFD5;
    QMenu *menuWrite_Protection_FD5;
#endif
#ifdef USE_FD6
    QMenu *menuFD6;
    QMenu *menuWrite_Protection_FD6;
#endif
#ifdef USE_FD7
    QMenu *menuFD7;
    QMenu *menuWrite_Protection_FD7;
#endif
#ifdef USE_FD8
    QMenu *menuFD8;
    QMenu *menuWrite_Protection_FD8;
#endif
#ifdef USE_QD1
    QMenu *menuQD1;
    QMenu *menuWrite_Protection_QD1;
#endif
#ifdef USE_QD2
    QMenu *menuQD2;
    QMenu *menuWrite_Protection_QD2;
#endif
#ifdef USE_TAPE    
    QMenu *menuCMT;
    QMenu *menuWrite_Protection_CMT;
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
  // Test
   Ui_MainWindow(QWidget *parent = 0);
  ~Ui_MainWindow();
    void createContextMenu(void);
    void setupUi(void);
    void retranslateUi(Ui_MainWindow *p);
    QMainWindow *getWindow(void) { return MainWindow; }
    QMenuBar    *getMenuBar(void) { return menubar;}
    GLDrawClass *getGraphicsView(void) { return graphicsView; }
    QStatusBar *getStatusBar(void) { return statusbar;}
 private:
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
 public slots:
     void open_disk_dialog(int drv);
     void _open_disk(int drv, const QString fname);
     void _open_cart(int drv, const QString fname);
     void _open_cmt(bool mode,const QString path);
     void eject_fd(int drv);
     void on_actionExit_triggered() {
	save_config();
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
