


#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "emu.h"
#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class Ui_MainWindow;
class USING_FLAGS;
class CSP_Logger;
//  wrote of HC20 Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	void setupUI_Emu(void);
	void retranslateUi(void);
  
	QMenu *menu_Emu_DipSw;
	QActionGroup *actionGroup_DipSw;
	Action_Control *action_Emu_DipSw[4];
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
};

QT_END_NAMESPACE

#endif // END
