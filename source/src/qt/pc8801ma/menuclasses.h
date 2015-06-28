
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class Object_Menu_Control_88: public Object_Menu_Control
{
   Q_OBJECT
 public:
     Object_Menu_Control_88(QObject *parent);
     ~Object_Menu_Control_88();
signals:
   int sig_sound_device(int);
   int sig_device_type(int);
 public slots:
   void do_set_sound_device(void);
   void do_set_device_type(void);
   void do_set_memory_wait(bool);
};

class Action_Control_88 : public Action_Control
{
   Q_OBJECT
 public:
   Object_Menu_Control_88 *pc88_binds;
   Action_Control_88(QObject *parent);
   ~Action_Control_88();
};


class Ui_MainWindow;
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
  Q_OBJECT
 protected:
  QActionGroup   *actionGroup_SoundDevice;
  QMenu *menuSoundDevice;
  class Action_Control_88 *actionSoundDevice[2]; //
  class Action_Control_88 *actionMemoryWait; //
  void setupUI_Emu(void);
  void retranslateUi(void);
#ifdef USE_DEVICE_TYPE
	QActionGroup *actionGroup_DeviceType;
	QMenu *menuDeviceType;
	class Action_Control_88 *actionDeviceType[USE_DEVICE_TYPE];
	void ConfigDeviceType(int num);
#endif   
#ifdef USE_DRIVE_TYPE
	QActionGroup *actionGroup_DriveType;
	QMenu *menuDriveType;
	class Action_Control_88 *actionDriveType[USE_DRIVE_TYPE];
	void ConfigDriveType(int num);
#endif   
 public:
  META_MainWindow(QWidget *parent = 0);
  ~META_MainWindow();
 public slots:
    void do_set_sound_device(int);
};

QT_END_NAMESPACE

#endif // END
