
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class Ui_MainWindow;
class USING_FLAGS;
class CSP_Logger;
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	QActionGroup *actionGroup_JOYPortType[2];
	Action_Control *actionJOYPORT_TYPE[2][8];
	
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();

public slots:
};

QT_END_NAMESPACE

#endif // END
