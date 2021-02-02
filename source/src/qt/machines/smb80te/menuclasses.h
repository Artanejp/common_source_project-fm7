
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include <QMenu>
#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE


class Ui_MainWindow;
class USING_FLAGS;
class CSP_Logger;

class Object_Menu_Control_SMB: public Object_Menu_Control
{
	Q_OBJECT
 public:
	Object_Menu_Control_SMB(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_SMB();
public slots:
	void do_set_adrs_base();
};

class Action_Control_SMB : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_SMB *smb_binds;
	Action_Control_SMB(QObject *parent, USING_FLAGS *p);
	~Action_Control_SMB();
};

class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	void setupUI_Emu(void);
	void retranslateUi(void);
	QMenu *menuAddrBase;
	QAction *actionAddress8000;
	Action_Control_SMB *actionAddressBase[4]; //
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
	void do_set_adrs_8000(bool);
};

QT_END_NAMESPACE

#endif // END
