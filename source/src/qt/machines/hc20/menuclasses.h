


#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "emu.h"
#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class Ui_MainWindow;
//  wrote of HC20 Specific menu.
class Object_Menu_Control_HC20: public Object_Menu_Control
{
   Q_OBJECT
 public:
     Object_Menu_Control_HC20(QObject *parent);
     ~Object_Menu_Control_HC20();
signals:
   int sig_dipsw(int, bool);
 public slots:
   void set_dipsw(bool);
};

class Action_Control_HC20 : public Action_Control
{
   Q_OBJECT
 public:
   Object_Menu_Control_HC20 *hc20_binds;
   Action_Control_HC20(QObject *parent);
   ~Action_Control_HC20();
};

class META_MainWindow : public Ui_MainWindow {
  Q_OBJECT
 protected:
  void setupUI_Emu(void);
  void retranslateUi(void);
  
  QMenu *menu_Emu_DipSw;
  QActionGroup *actionGroup_DipSw;
  Action_Control_HC20 *action_Emu_DipSw[4];
  
 public:
  META_MainWindow(QWidget *parent = 0);
  ~META_MainWindow();
public slots:

};

QT_END_NAMESPACE

#endif // END
