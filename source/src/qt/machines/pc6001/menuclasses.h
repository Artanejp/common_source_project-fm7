
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE
class USING_FLAGS;
class CSP_Logger;
class Object_Menu_Control_60: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_60(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_60();
signals:
	int sig_sound_device(int);
public slots:
	void do_set_sound_device(void);
};

class Action_Control_60 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_60 *pc60_binds;
	Action_Control_60(QObject *parent, USING_FLAGS *p);
	~Action_Control_60();
};


class Ui_MainWindow;
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	QActionGroup   *actionGroup_SoundDevice;
	QMenu *menuSoundDevice;
	class Action_Control_60 *actionSoundDevice[2]; //
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
	 void do_set_sound_device(int);
};

QT_END_NAMESPACE

#endif // END
