/*
 * Qt -> GUI -> CommonClasses
 * commonclasses.h
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Licence : GPLv2
 * History : Jan 13 2015 : Split from x1turboz / menuclasses.h
 */

#ifndef _CSP_QT_GUI_COMMONCLASSES_H
#define _CSP_QT_GUI_COMMONCLASSES_H

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
//#include "qt_main.h"
#include "qt_gldraw.h"


extern class EMU* emu;

QT_BEGIN_NAMESPACE
typedef class Object_Menu_Control: public QObject {
    Q_OBJECT
public:
//   explicit Object_Menu_Control(QObject *parent = 0);
   Object_Menu_Control(QObject *parent) : QObject(parent){
      bindValue = 0;
      drive = 0;
      s_num = 0;
      play = true; // Read
      write_protect = false; // Enable to write
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
 bool play;
 bool write_protect;
signals:
     int on_boot_mode(int);   
     int on_cpu_type(int);   
     int on_cpu_power(int); 
     int on_open_debugger(int);
     
     int sig_insert_fd(int);
     int sig_eject_fd(int);
     int set_d88_slot(int, int);
     int set_recent_disk(int, int);
     int sig_write_protect_fd(int, bool);
     
     int sig_insert_play_cmt(bool);
     int sig_eject_cmt(void);
     int sig_recent_cmt(int);
     int sig_set_write_protect_cmt(bool);
     int sig_freq(int);
     int sig_latency(int);
     int sig_sounddevice(int);
     int sig_set_dipsw(int, bool);
public slots:
     void set_boot_mode(void);
     void set_cpu_type(void);
     void set_cpupower(void);
     void open_debugger(void);
     void do_set_write_protect_cmt(void);
     void do_unset_write_protect_cmt(void);
     void insert_fd(void);
     void eject_fd(void);
     void on_d88_slot(void);
     void on_recent_disk(void);
     void write_protect_fd(void);
     void no_write_protect_fd(void);
     
     void start_insert_play_cmt(void);
     void eject_cmt(void);
     void on_recent_cmt(void);
   
     void on_set_freq(void);
     void on_set_latency(void);
 public:
   void setValue1(int v) {bindValue = v;}
   int getValue1(void) {return bindValue;}
   void setDrive(int num) { drive = num;}
   int getDrive(void) { return drive;}
   void setNumber(int num) { s_num = num;}
   int getNumber(void) { return s_num;}
   
   bool isPlay(void) { return play; }
   void setPlay(bool b) { play = b; }
   
   bool isWriteProtect(void) { return write_protect; }
   void setWriteProtect(bool b) {write_protect = b;}

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
QT_END_NAMESPACE

#endif


 
