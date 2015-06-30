
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
#if defined(_X1TURBOZ)
   void sig_display_mode(int);
#endif
 public slots:
#if defined(_X1TURBOZ)
   void do_set_display_mode(void);
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
#if defined(_X1TURBOZ)  
  QActionGroup   *actionGroup_DisplayMode;
  class Action_Control_X1 *action_Emu_DisplayMode[2]; // 0=Hi / 1 = Lo
  QMenu *menu_Emu_DisplayMode;
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
