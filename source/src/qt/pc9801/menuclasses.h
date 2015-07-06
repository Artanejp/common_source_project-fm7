
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
#include <QCheckBox>
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE


class Object_Menu_Control_98: public Object_Menu_Control
{
   Q_OBJECT
 public:
     Object_Menu_Control_98(QObject *parent);
     ~Object_Menu_Control_98();
signals:
   int sig_sound_device(int);
   int sig_device_type(int);
 public slots:
   void do_set_memory_wait(bool);
};

class Action_Control_98 : public Action_Control
{
   Q_OBJECT
 public:
   Object_Menu_Control_98 *pc98_binds;
   Action_Control_98(QObject *parent);
   ~Action_Control_98();
};

class Ui_MainWindow;
//  wrote of X1 Specific menu.
class META_MainWindow : public Ui_MainWindow {
  Q_OBJECT
 protected:
  QActionGroup   *actionGroup_SoundDevice;
  QMenu *menu_Emu_SoundDevice;
#if defined(_PC98DO)
   Action_Control_98 *actionMemoryWait;
#endif   
  void setupUI_Emu(void);
  void retranslateUi(void);
 public:
  META_MainWindow(QWidget *parent = 0);
  ~META_MainWindow();
};

QT_END_NAMESPACE

#endif // END
