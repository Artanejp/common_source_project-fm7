/*
 * Common Source code Project:
 * Ui->Qt->gui->menu_main for generic.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, moved from qt/x1turboz/MainWindow.cpp .
 */

#include <QtCore/QVariant>
#include <QtGui>
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"


QT_BEGIN_NAMESPACE

void Ui_MainWindow::set_screen_aspect(int num)
{
  if((num < 0) || (num >= 3)) return;
  double ww = SCREEN_WIDTH;
  double hh = SCREEN_HEIGHT;
  double whratio = ww / hh;
  double ratio;
  int width, height;
  QSizePolicy policy;
  // 0 = DOT
  // 1 = ASPECT
  // 2 = FILL
  // On Common Sourcecode Project / Agar,
  // Scaling is done by Agar Widget.
  // So, does need below action?
  // Maybe, needs Agar's changing action. 

  config.stretch_type = num;
  
  if(emu) {
    int w, h;
    w = this->graphicsView->width();
    h = this->graphicsView->height();
    this->graphicsView->resizeGL(w, h);
  }
}

void Object_Menu_Control::set_screen_aspect(void) {
  int num = getValue1();
  emit sig_screen_aspect(num);
}



  
void Ui_MainWindow::ConfigScreenMenu(void)
{

        actionZoom = new Action_Control(this);
        actionZoom->setObjectName(QString::fromUtf8("actionZoom"));
        actionDisplay_Mode = new Action_Control(this);
        actionDisplay_Mode->setObjectName(QString::fromUtf8("actionDisplay_Mode"));
        actionScanLine = new Action_Control(this);
        actionScanLine->setObjectName(QString::fromUtf8("actionScanLine"));
        actionScanLine->setCheckable(true);
        actionScanLine->setChecked(true);
	
        actionCRT_Filter = new Action_Control(this);
        actionCRT_Filter->setObjectName(QString::fromUtf8("actionCRT_Filter"));
        actionCRT_Filter->setEnabled(false);

        actionDot_by_Dot = new Action_Control(this);
        actionDot_by_Dot->setObjectName(QString::fromUtf8("actionDot_by_Dot"));
        actionDot_by_Dot->setCheckable(true);
        if(config.stretch_type == 0) actionDot_by_Dot->setChecked(true);
	actionDot_by_Dot->binds->setValue1(0);
	
        actionKeep_Aspect = new Action_Control(this);
        actionKeep_Aspect->setObjectName(QString::fromUtf8("actionKeep_Aspect"));
        actionKeep_Aspect->setCheckable(true);
	actionKeep_Aspect->binds->setValue1(1);
	if(config.stretch_type == 1) actionKeep_Aspect->setChecked(true);
	
        actionFill_Display = new Action_Control(this);
        actionFill_Display->setObjectName(QString::fromUtf8("actionFill_Display"));
        actionFill_Display->setCheckable(true);
	actionFill_Display->binds->setValue1(2);
	if(config.stretch_type == 2) actionFill_Display->setChecked(true);
	
	actionGroup_Stretch = new QActionGroup(this);
	actionGroup_Stretch->setExclusive(true);
	actionGroup_Stretch->addAction(actionDot_by_Dot);
	actionGroup_Stretch->addAction(actionKeep_Aspect);
	actionGroup_Stretch->addAction(actionFill_Display);
	connect(actionDot_by_Dot,   SIGNAL(triggered()), actionDot_by_Dot->binds,   SLOT(set_screen_aspect()));
	connect(actionDot_by_Dot->binds,   SIGNAL(sig_screen_aspect(int)), this,    SLOT(set_screen_aspect(int)));
	
	connect(actionKeep_Aspect,  SIGNAL(triggered()), actionKeep_Aspect->binds,  SLOT(set_screen_aspect()));
	connect(actionKeep_Aspect->binds,  SIGNAL(sig_screen_aspect(int)), this,    SLOT(set_screen_aspect(int)));
	
	connect(actionFill_Display, SIGNAL(triggered()), actionFill_Display->binds, SLOT(set_screen_aspect()));
	connect(actionFill_Display->binds, SIGNAL(sig_screen_aspect(int)), this, SLOT(set_screen_aspect(int)));
	
        actionCapture_Screen = new Action_Control(this);
        actionCapture_Screen->setObjectName(QString::fromUtf8("actionCapture_Screen"));

        actionStart_Record_Movie = new Action_Control(this);
        actionStart_Record_Movie->setObjectName(QString::fromUtf8("actionStart_Record_Movie"));
        actionStart_Record_Movie->setCheckable(true);
        actionStop_Record_Movie = new Action_Control(this);
        actionStop_Record_Movie->setObjectName(QString::fromUtf8("actionStop_Record_Movie"));
        actionStop_Record_Movie->setCheckable(false);

  
}

void Ui_MainWindow::CreateScreenMenu(void)
{
        menuScreen = new QMenu(menubar);
        menuScreen->setObjectName(QString::fromUtf8("menuScreen"));
        menuStretch_Mode = new QMenu(menuScreen);
        menuStretch_Mode->setObjectName(QString::fromUtf8("menuStretch_Mode"));

//        menuRecoad_as_movie = new QMenu(menuRecord);
//        menuRecoad_as_movie->setObjectName(QString::fromUtf8("menuRecoad_as_movie"));


        menuScreen->addAction(actionZoom);
        menuScreen->addAction(actionDisplay_Mode);

        menuScreen->addSeparator();
        menuScreen->addAction(menuStretch_Mode->menuAction());

        menuStretch_Mode->addAction(actionDot_by_Dot);
        menuStretch_Mode->addAction(actionKeep_Aspect);
        menuStretch_Mode->addAction(actionFill_Display);

        menuScreen->addSeparator();
        menuScreen->addAction(actionScanLine);
        menuScreen->addAction(actionCRT_Filter);
//        menuRecord->addAction(actionCapture_Screen);
//        menuRecord->addSeparator();
//        menuRecord->addAction(menuRecoad_as_movie->menuAction());
//        menuRecord->addSeparator();
//        menuRecord->addAction(menuRecord_sound->menuAction());
//        menuRecord_sound->addAction(actionStart_Record);
//        menuRecord_sound->addAction(actionStop_Record);
//        menuRecoad_as_movie->addAction(actionStart_Record_Movie);
//        menuRecoad_as_movie->addAction(actionStop_Record_Movie);
	
}

void Ui_MainWindow::retranslateScreeMenu(void)
{
  actionZoom->setText(QApplication::translate("MainWindow", "Zoom Screen", 0, QApplication::UnicodeUTF8));
  actionDisplay_Mode->setText(QApplication::translate("MainWindow", "Display Mode", 0, QApplication::UnicodeUTF8));
  actionScanLine->setText(QApplication::translate("MainWindow", "Set ScanLine", 0, QApplication::UnicodeUTF8));
  actionCRT_Filter->setText(QApplication::translate("MainWindow", "CRT Filter", 0, QApplication::UnicodeUTF8));
  actionDot_by_Dot->setText(QApplication::translate("MainWindow", "Dot by Dot", 0, QApplication::UnicodeUTF8));
  actionKeep_Aspect->setText(QApplication::translate("MainWindow", "Keep Aspect", 0, QApplication::UnicodeUTF8));
  actionFill_Display->setText(QApplication::translate("MainWindow", "Fill Display", 0, QApplication::UnicodeUTF8));
  
  actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0, QApplication::UnicodeUTF8));

   menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0, QApplication::UnicodeUTF8));
   menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0, QApplication::UnicodeUTF8));

   actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0, QApplication::UnicodeUTF8));
   actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0, QApplication::UnicodeUTF8));

   menuRecord->setTitle(QApplication::translate("MainWindow", "Record", 0, QApplication::UnicodeUTF8));
   menuRecoad_as_movie->setTitle(QApplication::translate("MainWindow", "Recoad as movie", 0, QApplication::UnicodeUTF8));
   
}

