
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
 public slots:
   void do_set_sound_device(void);
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
  QMenu *menu_Emu_SoundDevice;
  class Action_Control_X1 *action_Emu_SoundDevice[3]; // 0 = PSG, 1 = FM(CZ-8BS1)x1, 2 = FMx2
  void setupUI_Emu(void);
 public:
  META_MainWindow(QWidget *parent = 0);
  ~META_MainWindow();
 public slots:
    void do_set_sound_device(int);
};

QT_END_NAMESPACE

#endif // END
