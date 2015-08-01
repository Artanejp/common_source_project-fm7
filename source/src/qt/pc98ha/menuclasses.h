
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class Ui_MainWindow;
//  wrote of X1 Specific menu.
class META_MainWindow : public Ui_MainWindow {
  Q_OBJECT
 protected:
  QActionGroup   *actionGroup_SoundDevice;
  QMenu *menu_Emu_SoundDevice;
  void setupUI_Emu(void);
  void retranslateUi(void);
 public:
  META_MainWindow(QWidget *parent = 0);
  ~META_MainWindow();
 public slots:
    void do_set_sound_device(int);
};

QT_END_NAMESPACE

#endif // END
