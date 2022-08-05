
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include <QMenu>
#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class USING_FLAGS;
class CSP_Logger;
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	void setupUI_Emu(void);
	void retranslateUi(void);
	QMenu *menuAddrBase;
	Action_Control *actionAddress8000;
	Action_Control *actionAddressBase[4]; //
public:
	META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent = 0);
	~META_MainWindow();
};

QT_END_NAMESPACE

#endif // END
