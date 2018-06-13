
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class USING_FLAGS;
class Object_Menu_Control_X1: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_X1(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_X1();
signals:
public slots:
};

class Action_Control_X1 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_X1 *x1_binds;
	Action_Control_X1(QObject *parent, USING_FLAGS *p);
	~Action_Control_X1();
};


class Ui_MainWindow;
class CSP_Logger;
//  wrote of X1 Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
};

QT_END_NAMESPACE

#endif // END
