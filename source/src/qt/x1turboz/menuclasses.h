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

//QT_BEGIN_NAMESPACE
extern class EMU* emu;

typedef class Object_Menu_Control: public QObject {
// Do not add Q_OBJECT macro.
// See: http://www.qtcentre.org/threads/37731-QOBJECT-and-undefined-reference-to-vtable-errors
    Q_OBJECT
//    Q_DISABLE_COPY(Object_Menu_Control)
public:
//    explicit Object_Menu_Control(QObject *parent = 0);
   Object_Menu_Control(QObject *parent) : QObject(parent){
      bindValue = 0;
   }
   Object_Menu_Control() {}
   // Virtual Functions
   bool event(QEvent *e) { return true;} 
   bool eventFilter ( QObject * watched, QEvent * event ){
	return true;
   }
   void childEvent (QChildEvent * event ){ };
   void connectNotify ( const char * signal ) {}
   void customEvent ( QEvent * event ) { }
   void disconnectNotify ( const char * signal ) {  }
   void timerEvent ( QTimerEvent * event ){ }
   // End
     void OnReset(void);
     void OnSpecialReset(void);
#ifdef USE_STATE
     void OnLoadState(void);
     void OnSaveState(void);
#endif
#ifdef USE_BOOT_MODE
     void OnBootMode(void);
#endif
#ifdef USE_CPU_TYPE
     void OnCpuType(void);
#endif
     void OnCpuPower(void);
#ifdef USE_AUTO_KEY
     void OnStartAutoKey(void);
     void OnStopAutoKey(void);
#endif
#ifdef USE_DEBUGGER
     void OnOpenDebugger(void);
     void OnCloseDebugger(void);
#endif
public slots: // [1]
     void setValue1(int v) {bindValue = v;}
     int getValue1(void) {return bindValue;}
signals:
private:
 int bindValue;
} Object_Menu_Control ;

typedef class Action_Control: public QAction {
    Q_OBJECT
 private:
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
//  virtual void setIconSet ( const QIconSet str ){}
//  virtual void setText ( const QString str ){}
//  virtual void setMenuText ( const QString str ){}
//  virtual void setToolTip ( const QString str ){}
//  virtual void setStatusTip ( const QString str ){}
//  virtual void setWhatsThis ( const QString str){}
//  virtual void setAccel ( const QKeySequence key ){}
//  virtual void setToggleAction ( bool toggle){}
//  virtual bool addTo ( QWidget * w ) {return true;}
//  virtual bool removeFrom ( QWidget *w ) {return true;}
   
private slots: 
     void OnReset(void) {
	binds->OnReset();
     }
     void OnSpecialReset(void) {
	binds->OnSpecialReset();
     }
     virtual void setOn ( bool a) {}
     virtual void setEnabled ( bool a){}
#ifdef USE_STATE
     void OnLoadState(void){
	binds->OnSaveState();
     }
   
     void OnSaveState(void) {
	binds->OnSaveState();
     }
   
#endif
#ifdef USE_BOOT_MODE
     void OnBootMode(void){
	binds->OnBootMode();
     }
   
#endif
#ifdef USE_CPU_TYPE
     void OnCpuType(void){
	binds->OnCpuType();
     }
   
#endif
     void OnCpuPower(void) {
	binds->OnCpuPower();
     }
#ifdef USE_AUTO_KEY
     void OnStartAutoKey(void) {
	binds->OnStartAutoKey();
     }
     void OnStopAutoKey(void) {
	binds->OnStopAutoKey();
     }
#endif
#ifdef USE_DEBUGGER
     void OnOpenDebugger(void) {
	binds->OnOpenDebugger();
     }
     void OnCloseDebugger(void) {
	binds->OnCloseDebugger();
     }
   
#endif
signals:
   void triggered();
// Action_Control(QObject * parent, const char *name = 0) : QAction(parent, name)
//    {binds.setValue1(0);}
// Action_Control(const QString &menuText, QKeySequence accel, QObject *parent, const char *name = 0) : QAction(menuText, accel, parent, name)
//    {binds.setValue1(0);}
// Action_Control (const QIconSet &icon, const QString &menuText, QKeySequence accel, QObject *parent, const char *name) :
//     QAction(icon, menuText, accel, parent, name)
//    {binds.setValue1(0);}

} ActionControl;


class Ui_MainWindow : public QWidget
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
      void ConfigCpuSpeed(QMainWindow *MainWindow);
  void ConfigControlMenu(QMainWindow *MainWindow);
  void connectActions_ControlMenu(QMenuBar *MainWindow);
  void retranslateControlMenu(QMainWindow *MainWindow, const char *SpecialResetTitle,  bool WithSpecialReset);
  QMainWindow *MainWindow;
public:
  // Test
  //   Ui_MainWindow(QWidget *parent = 0) : QWidget(parent) {
  //	setupUi();
  // }
  Ui_MainWindow();
   ~Ui_MainWindow() { }
   
    QApplication *GuiMain;
    Action_Control *actionReset;
    Action_Control *actionSpecial_Reset;
    Action_Control *actionExit_Emulator;
//#ifdef USE_CPU_TYPE
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

#ifdef USE_FD0
    QAction *actionInsert_FD0;
    QAction *actionEject_FD0;
    QAction *actionRecent_Opened_FD0;
    QAction *actionSelect_D88_Image_FD0;
    QAction *actionProtection_ON_FD0;
    QAction *actionProtection_OFF_FD0;
#endif
#ifdef USE_FD1
    QAction *actionInsert_FD1;
    QAction *actionEject_FD1;
    QAction *actionRecent_Opened_FD1;
    QAction *actionSelect_D88_Image_FD1;
    QAction *actionProtection_ON_FD1;
    QAction *actionProtection_OFF_FD1;
#endif
#ifdef USE_FD2
    QAction *actionInsert_FD2;
    QAction *actionEject_FD2;
    QAction *actionRecent_Opened_FD2;
    QAction *actionSelect_D88_Image_FD2;
    QAction *actionProtection_ON_FD2;
    QAction *actionProtection_OFF_FD2;
#endif
#ifdef USE_FD3
    QAction *actionInsert_FD3;
    QAction *actionEject_FD3;
    QAction *actionRecent_Opened_FD3;
    QAction *actionSelect_D88_Image_FD3;
    QAction *actionProtection_ON_FD3;
    QAction *actionProtection_OFF_FD3;
#endif
#ifdef USE_FD4
    QAction *actionInsert_FD4;
    QAction *actionEject_FD4;
    QAction *actionRecent_Opened_FD4;
    QAction *actionSelect_D88_Image_FD4;
    QAction *actionProtection_ON_FD4;
    QAction *actionProtection_OFF_FD4;
#endif
#ifdef USE_FD5
    QAction *actionInsert_FD5;
    QAction *actionEject_FD5;
    QAction *actionRecent_Opened_FD5;
    QAction *actionSelect_D88_Image_FD5;
    QAction *actionProtection_ON_FD5;
    QAction *actionProtection_OFF_FD5;
#endif
#ifdef USE_FD6
    QAction *actionInsert_FD6;
    QAction *actionEject_FD6;
    QAction *actionRecent_Opened_FD6;
    QAction *actionSelect_D88_Image_FD6;
    QAction *actionProtection_ON_FD6;
    QAction *actionProtection_OFF_FD6;
#endif
#ifdef USE_FD7
    QAction *actionInsert_FD7;
    QAction *actionEject_FD7;
    QAction *actionRecent_Opened_FD7;
    QAction *actionSelect_D88_Image_FD7;
    QAction *actionProtection_ON_FD7;
    QAction *actionProtection_OFF_FD7;
#endif
#ifdef USE_FD8
    QAction *actionInsert_FD8;
    QAction *actionEject_FD8;
    QAction *actionRecent_Opened_FD8;
    QAction *actionSelect_D88_Image_FD8;
    QAction *actionProtection_ON_FD8;
    QAction *actionProtection_OFF_FD8;
#endif
#ifdef USE_QD1    
    QAction *actionInsert_QD1;
    QAction *actionEject_QD1;
    QAction *actionResent_Images_QD1;
    QAction *actionProtection_ON_QD1;
    QAction *actionProtection_OFF_QD1;
#endif
#ifdef USE_QD2    
    QAction *actionInsert_QD2;
    QAction *actionEject_QD2;
    QAction *actionResent_Images_QD2;
    QAction *actionProtection_ON_QD2;
    QAction *actionProtection_OFF_QD2;
#endif
#ifdef USE_TAPE    
    QAction *actionInsert_CMT;
    QAction *actionEject_CMT;
    QAction *actionPlay_Start;
    QAction *actionPlay_Stop;
    QAction *actionRecording;
    QAction *actionProtection_ON_CMT;
    QAction *actionProtection_OFF_CMT;
#endif    
    QAction *actionZoom;
    QAction *actionDisplay_Mode;
    QAction *actionScanLine;
    QAction *actionCRT_Filter;
    QAction *actionDot_by_Dot;
    QAction *actionKeep_Aspect;
    QAction *actionFill_Display;
    QAction *actionCapture_Screen;
    QAction *actionAbout;
    QAction *action2000Hz;
    QAction *action4000Hz;
    QAction *action8000Hz;
    QAction *action11025Hz;
    QAction *action22050Hz;
    QAction *action44100Hz;
    QAction *action48000Hz;
    QAction *action96000Hz;
    QAction *action50ms;
    QAction *action100ms;
    QAction *action200ms;
    QAction *action300ms;
    QAction *action400ms;
    QAction *actionStart_Record;
    QAction *actionStop_Record;
    QAction *actionStart_Record_Movie;
    QAction *actionStop_Record_Movie;
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
    QMenu *menuWrite_Protection_FD1;
#endif
#ifdef USE_FD2
    QMenu *menuFD2;
    QMenu *menuWrite_Protection_FD2;
#endif
#ifdef USE_FD3
    QMenu *menuFD3;
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

    void setupUi(void);
    void retranslateUi(QMainWindow *MainWindow);
    QMainWindow *getWindow(void) { return MainWindow; }
    QMenuBar    *getMenuBar(void) { return menubar;}
    GLDrawClass *getGraphicsView(void) { return graphicsView; }
    QStatusBar *getStatusBar(void) { return statusbar;}

    void OnGuiExit(){
      //this->close();
    }
};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif
