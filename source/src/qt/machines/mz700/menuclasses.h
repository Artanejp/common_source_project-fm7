
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class USING_FLAGS;
class Ui_MainWindow;
class CSP_Logger;
//  wrote of MZ700 Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
#if defined(_MZ700)
	Action_Control *action_PCG700;
#endif	
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
};

QT_END_NAMESPACE
#endif // END
