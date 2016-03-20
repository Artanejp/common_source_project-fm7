
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE


class Object_Menu_Control_MZ700: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_MZ700(QObject *parent);
	~Object_Menu_Control_MZ700();
signals:
	void sig_monitor_type(int);
public slots:
	void do_monitor_type(void);
};

class Action_Control_MZ700 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_MZ700 *mz_binds;
	Action_Control_MZ700(QObject *parent);
	~Action_Control_MZ700();
};


class Ui_MainWindow;
//  wrote of MZ700 Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
#if defined(_MZ700)
	QAction *action_PCG700;
#endif	
#if defined(USE_MONITOR_TYPE)     
	QActionGroup   *actionGroup_MonitorType;
	QMenu *menuMonitorType;
	class Action_Control_MZ700 *actionMonitorType[USE_MONITOR_TYPE];
#endif
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(QWidget *parent = 0);
	~META_MainWindow();
public slots:
	void set_monitor_type(int);
	void do_set_pcg(bool flag);
signals:
};

QT_END_NAMESPACE
#endif // END
