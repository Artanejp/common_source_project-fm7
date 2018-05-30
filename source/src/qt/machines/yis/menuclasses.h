
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
#include "commonclasses.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class Ui_MainWindow;
class USING_FLAGS;

class Object_Menu_Control_YIS: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_YIS(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_YIS();
signals:
public slots:
};

class Action_Control_YIS : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_YIS *yis_binds;
	Action_Control_YIS(QObject *parent, USING_FLAGS *p);
	~Action_Control_YIS();
};

class QMenu;
class QActionGroup;
class CSP_Logger;
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	QActionGroup   *actionGroup_SoundDevice;
	QMenu *menu_Emu_SoundDevice;
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
};

QT_END_NAMESPACE

#endif // END
