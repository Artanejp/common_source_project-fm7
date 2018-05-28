


#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "emu.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class Ui_MainWindow;
class USING_FLAGS;
class CSP_Logger;

class Object_Menu_Control_PC100: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_PC100(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_PC100();
signals:
	int sig_update_config();
public slots:
	void do_set_monitor_type(void);
};

class Action_Control_PC100 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_PC100 *pc100_binds;
	Action_Control_PC100(QObject *parent, USING_FLAGS *p);
	~Action_Control_PC100();
};

class QMenu;
class QActionGroup;
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	QMenu *menuDisplayType;
	QActionGroup *actionGroup_DisplayType;
	Action_Control_PC100 *action_DisplayTypp[4];
	
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
	void do_pc100_update_config(void);
};

QT_END_NAMESPACE

#endif // END
