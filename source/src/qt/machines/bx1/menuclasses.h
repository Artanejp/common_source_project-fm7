
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"
#include "vm.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
class USING_FLAGS;
class CSP_Logger;

QT_BEGIN_NAMESPACE

class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	Action_Control *action_DipSWs[4];
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
};

QT_END_NAMESPACE

#endif // END
