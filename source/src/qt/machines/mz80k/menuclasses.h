
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
#include "commonclasses.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

//  wrote of MZ80 Specific menu.
class CSP_Logger;
class USING_FLAGS;
class Object_Menu_Control_MZ80: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_MZ80(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_MZ80();
signals:
	int sig_dipsw(int, bool);
public slots:
	void set_dipsw(bool);
};

class Action_Control_MZ80 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_MZ80 *mz_binds;
	Action_Control_MZ80(QObject *parent, USING_FLAGS *p);
	~Action_Control_MZ80();
};

class Ui_MainWindow;
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	Action_Control_MZ80 *action_Emu_DipSw;
	
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
};

QT_END_NAMESPACE

#endif // END
