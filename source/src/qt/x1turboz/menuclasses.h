
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE


class Object_Menu_Control_X1: public Object_Menu_Control
{
   Q_OBJECT
 public:
     Object_Menu_Control_X1(QObject *parent);
     ~Object_Menu_Control_X1();
signals:
   int sig_sound_device(int);
#if defined(_X1TURBOZ)
   void sig_display_mode(int);
#endif
#if defined(USE_DEVICE_TYPE)
   void sig_device_type(int);
#endif
#if defined(USE_DRIVE_TYPE)
   void sig_drive_type(int);
#endif
 public slots:
   void do_set_sound_device(void);
#if defined(_X1TURBOZ)
   void do_set_display_mode(void);
#endif
#ifdef USE_DEVICE_TYPE
   void do_set_device_type(void);
#endif
#ifdef USE_DRIVE_TYPE
   void do_set_drive_type(void);
#endif

};

class Action_Control_X1 : public Action_Control
{
   Q_OBJECT
 public:
   Object_Menu_Control_X1 *x1_binds;
   Action_Control_X1(QObject *parent);
   ~Action_Control_X1();
};


class Ui_MainWindow;
//  wrote of X1 Specific menu.
class META_MainWindow : public Ui_MainWindow {
  Q_OBJECT
 protected:
  QActionGroup   *actionGroup_SoundDevice;
  class Action_Control_X1 *action_Emu_SoundDevice[3]; // 0 = PSG, 1 = FM(CZ-8BS1)x1, 2 = FMx2
  QMenu *menu_Emu_SoundDevice;
#if defined(_X1TURBOZ)  
  QActionGroup   *actionGroup_DisplayMode;
  class Action_Control_X1 *action_Emu_DisplayMode[2]; // 0=Hi / 1 = Lo
  QMenu *menu_Emu_DisplayMode;
#endif
#ifdef USE_DEVICE_TYPE
	QActionGroup *actionGroup_DeviceType;
	QMenu *menuDeviceType;
	class Action_Control_X1 *actionDeviceType[USE_DEVICE_TYPE];
	void ConfigDeviceType(int num);
#endif   
#ifdef USE_DRIVE_TYPE
	QActionGroup *actionGroup_DriveType;
	QMenu *menuDriveType;
	class Action_Control_X1 *actionDriveType[USE_DRIVE_TYPE];
	void ConfigDriveType(int num);
#endif   
  void setupUI_Emu(void);
  void retranslateUi(void);
 public:
  META_MainWindow(QWidget *parent = 0);
  ~META_MainWindow();
public slots:

};

QT_END_NAMESPACE

#endif // END
