
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

class Object_Menu_Control_BX1: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_BX1(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_BX1();
signals:
	//  int sig_sound_device(int);
	int sig_emu_update_config(void);
public slots:
	void do_set_dipsw(bool flag);
};

class Action_Control_BX1 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_BX1 *bx1_binds;
	Action_Control_BX1(QObject *parent, USING_FLAGS *p, int num);
	~Action_Control_BX1();
public slots:
};

class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	Action_Control_BX1 *action_DipSWs[4];
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
};

QT_END_NAMESPACE

#endif // END
