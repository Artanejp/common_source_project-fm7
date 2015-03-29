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
      height = 0;
      width = 0;
      play = true; // Read
      write_protect = false; // Enable to write
   }
   Object_Menu_Control() {}
private:
 int bindValue;
 int drive;
 int s_num;
 int width;
 int height;
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

#if defined(USE_QD1) || defined(USE_QD2)
     int set_recent_quick_disk(int, int);
     int sig_write_protect_Qd(int, bool);
     int sig_eject_Qd(int);
     int sig_insert_Qd(int);
#endif
   
     int sig_insert_play_cmt(bool);
     int sig_eject_cmt(void);
     int sig_recent_cmt(int);
     int sig_set_write_protect_cmt(bool);
#ifdef USE_CART1
     int sig_insert_cart(int);
     int sig_eject_cart(int);
     int set_recent_cart(int, int);
#endif
#if defined(USE_BINARY_FILE1)
     int sig_open_binary_file(int, QString, bool);
     int sig_open_binary(int, bool);
     int set_recent_binary_load(int, int);
     int set_recent_binary_save(int, int);
#endif
     int sig_freq(int);
     int sig_latency(int);
     int sig_sounddevice(int);
     int sig_set_dipsw(int, bool);
     int sig_screen_aspect(int);
     int sig_screen_size(int, int);
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
     
     void set_screen_aspect(void);
     void set_screen_size(void);
     
#if defined(USE_QD1) || defined(USE_QD2)
     void insert_Qd(void);
     void eject_Qd(void);
     void on_recent_quick_disk(void);
     void write_protect_Qd(void);
     void no_write_protect_Qd(void);
#endif
#if defined(USE_BINARY_FILE1)
     void on_recent_binary_load(void);
     void on_recent_binary_save(void);
     void _open_binary(QString s);
     void insert_binary_load(void);
     void insert_binary_save(void);
#endif
     void start_insert_play_cmt(void);
     void eject_cmt(void);
     void on_recent_cmt(void);
   
     void on_set_freq(void);
     void on_set_latency(void);
#ifdef USE_CART1
     void insert_cart(void);
     void eject_cart(void);
     void on_recent_cart(void);
#endif
     
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
   void setSize(int w, int h) { width = w, height = h;}
   void getSize(int *w, int *h) {
     if((w == NULL) || (h == NULL)) return;
     *w = width;
     *h = height;
   }

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
signals:
  int  quit_emu_thread(void);
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


 
