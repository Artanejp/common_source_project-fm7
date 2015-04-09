
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class Object_Menu_Control_7: public Object_Menu_Control
{
   Q_OBJECT
 public:
     Object_Menu_Control_7(QObject *parent);
     ~Object_Menu_Control_7();
signals:
   int sig_sound_device(int);
 public slots:
   void do_set_sound_device(void);
   void do_set_cyclesteal(bool flag);
};

class Action_Control_7 : public Action_Control
{
   Q_OBJECT
 public:
   Object_Menu_Control_7 *fm7_binds;
   Action_Control_7(QObject *parent);
   ~Action_Control_7();
};


class Ui_MainWindow;
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
  Q_OBJECT
 protected:
  QActionGroup   *actionGroup_SoundDevice;
  QMenu *menuSoundDevice;
  class Action_Control_7 *actionSoundDevice[8]; //
  class Action_Control_7 *actionCycleSteal;
  class Action_Control_7 *actionIgnoreCRC;
  void setupUI_Emu(void);
  void retranslateUi(void);
 public:
  META_MainWindow(QWidget *parent = 0);
  ~META_MainWindow();
 public slots:
    void do_set_sound_device(int);
    void do_set_ignore_crc_error(bool);
};

QT_END_NAMESPACE

#endif // END
