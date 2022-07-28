
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
#include "commonclasses.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE
class USING_FLAGS;
class CSP_Logger;
//  wrote of MZ3500 Specific menu.
class QMenu;
class QActionGroup;
class Ui_MainWindow;
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	void setupUI_Emu(void);
	void retranslateUi(void);
	QMenu *menu_Emu_DipSw;
	QActionGroup *actionGroup_DipSw;
	Action_Control *action_Emu_DipSw[3];
  
public:
	META_MainWindow(USING_FLAGS *p, std::shared_ptr<CSP_Logger> logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
};

QT_END_NAMESPACE

#endif // END
