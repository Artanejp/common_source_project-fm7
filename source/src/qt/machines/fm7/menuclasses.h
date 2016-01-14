
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE
class Ui_SoundDialog;

class Object_Menu_Control_7: public Object_Menu_Control
{
   Q_OBJECT
 public:
     Object_Menu_Control_7(QObject *parent);
     ~Object_Menu_Control_7();
signals:
	 //  int sig_sound_device(int);
   int sig_emu_update_config(void);
 public slots:
# if defined(_FM77AV_VARIANTS)   
   void do_set_hsync(bool flag);
# endif
# if defined(_FM8)
   void do_set_protect_ram(bool flag);
# else   
   void do_set_cyclesteal(bool flag);
# endif
};

class Action_Control_7 : public Action_Control
{
   Q_OBJECT
public:
   Object_Menu_Control_7 *fm7_binds;
   Action_Control_7(QObject *parent);
   ~Action_Control_7();
public slots:
	void do_set_frameskip();
};

class Ui_MainWindow;
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
  Q_OBJECT
protected:
  QMenu *menuFrameSkip;
  QActionGroup *actionGroup_FrameSkip;
  class Action_Control_7 *actionFrameSkip[4];
# if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
  class Action_Control_7 *actionExtRam;
# endif
# if defined(_FM8)
  class Action_Control_7 *actionRamProtect;
# else	
  class Action_Control_7 *actionCycleSteal;
# endif  
# if defined(_FM77AV_VARIANTS)   
  class Action_Control_7 *actionSyncToHsync;
# endif  
  void setupUI_Emu(void);
  void retranslateUi(void);
  void retranslateVolumeLabels(Ui_SoundDialog *p);
 public:
  META_MainWindow(QWidget *parent = 0);
  ~META_MainWindow();
 public slots:
    void do_set_extram(bool flag);
};

QT_END_NAMESPACE

#endif // END
