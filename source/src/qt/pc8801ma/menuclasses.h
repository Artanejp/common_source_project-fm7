
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
 public slots:
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
  class Action_Control_88 *actionMemoryWait; //
  void setupUI_Emu(void);
  void retranslateUi(void);
 public:
  META_MainWindow(QWidget *parent = 0);
  ~META_MainWindow();
};

QT_END_NAMESPACE

#endif // END
