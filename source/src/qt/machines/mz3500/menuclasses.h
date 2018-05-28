
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
class Object_Menu_Control_MZ3500: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_MZ3500(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_MZ3500();
signals:
	int sig_dipsw(int, bool);
	int sig_update_config();
public slots:
	void set_dipsw(bool);
	void do_set_monitor_type(void);
};

class Action_Control_MZ3500 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_MZ3500 *mz_binds;
	Action_Control_MZ3500(QObject *parent, USING_FLAGS *p);
	~Action_Control_MZ3500();
};

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
	Action_Control_MZ3500 *action_Emu_DipSw[3];
  
	QMenu *menuDisplayType;
	QActionGroup *actionGroup_DisplayType;
	
	Action_Control_MZ3500 *action_DisplayType[4];
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
	void do_mz35_update_config(void);
};

QT_END_NAMESPACE

#endif // END
