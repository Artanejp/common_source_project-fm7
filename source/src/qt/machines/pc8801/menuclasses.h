
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
public slots:
	void do_set_memory_wait(bool);
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
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	int config_sound_device_type;
	class Action_Control_88 *actionMemoryWait; //
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, QWidget *parent = 0);
	~META_MainWindow();
	void retranslateVolumeLabels(Ui_SoundDialog *p);
};

QT_END_NAMESPACE

#endif // END
