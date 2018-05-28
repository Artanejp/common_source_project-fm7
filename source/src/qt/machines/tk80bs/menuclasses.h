
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE


class Ui_MainWindow;
class USING_FLAGS;
class CSP_Logger;
class Object_Menu_Control_TK: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_TK(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_TK();
signals:
	//  int sig_sound_device(int);
	int sig_emu_update_config(void);
public slots:
	void do_set_dipsw(bool flag);
};
class Action_Control_TK : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_TK *tk_binds;
	Action_Control_TK(QObject *parent, USING_FLAGS *p);
	~Action_Control_TK();
public slots:

};

class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	Action_Control_TK *action_DipSWs[4];
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
};

QT_END_NAMESPACE

#endif // END
