
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE
class Ui_SoundDialog;
class USING_FLAGS;
class Object_Menu_Control_88: public Object_Menu_Control
{
	Q_OBJECT
 public:
	Object_Menu_Control_88(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_88();
signals:
	int sig_display_mode(int);
public slots:
	void do_set_memory_wait(bool);
	void do_set_display_mode();
	
};

class Action_Control_88 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_88 *pc88_binds;
	Action_Control_88(QObject *parent, USING_FLAGS *p);
	~Action_Control_88();
};


class Ui_MainWindow;
class CSP_Logger;
class QActionGroup;
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	int config_sound_device_type;
	QActionGroup *actionGroup_DisplayMode;
	class Action_Control_88 *actionMemoryWait; //
#if defined(USE_MONITOR_TYPE)
	QMenu *menu_Emu_DisplayMode;
	class Action_Control_88 *action_Emu_DisplayMode[USE_MONITOR_TYPE]; // 0=Hi / 1 = Lo
#endif
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
	void retranslateVolumeLabels(Ui_SoundDialog *p);
public slots:
};

QT_END_NAMESPACE

#endif // END
