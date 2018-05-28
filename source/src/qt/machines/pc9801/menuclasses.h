
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"

// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class USING_FLAGS;
class Object_Menu_Control_98: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_98(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_98();
signals:
	int sig_sound_device(int);
	int sig_device_type(int);
	void sig_display_mode(int);
public slots:
	void do_set_memory_wait(bool);
	void do_set_display_mode(void);
};

class Action_Control_98 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_98 *pc98_binds;
	Action_Control_98(QObject *parent, USING_FLAGS *p);
	~Action_Control_98();
};

class QMenu;
class QActionGroup;
class Ui_MainWindow;
class CSP_Logger;
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	QActionGroup   *actionGroup_SoundDevice;
	QMenu *menu_Emu_SoundDevice;
#if defined(_PC98DO)
	Action_Control_98 *actionMemoryWait;
#endif   
	void setupUI_Emu(void);
	void retranslateUi(void);
#if defined(USE_MONITOR_TYPE)  
	QActionGroup   *actionGroup_DisplayMode;
	class Action_Control_98 *action_Emu_DisplayMode[USE_MONITOR_TYPE]; // 0=Hi / 1 = Lo
	QMenu *menu_Emu_DisplayMode;
#endif
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
};

QT_END_NAMESPACE

#endif // END
