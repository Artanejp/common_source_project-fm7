
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
#include "commonclasses.h"

// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class Object_Menu_Control_QC10: public Object_Menu_Control
{
	Q_OBJECT
 public:
	Object_Menu_Control_QC10(QObject *parent);
	~Object_Menu_Control_QC10();
signals:
	int sig_dipsw(int, bool);
public slots:
	void set_dipsw(bool);
};

class Action_Control_QC10 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_QC10 *qc_binds;
	Action_Control_QC10(QObject *parent);
	~Action_Control_QC10();
};

class Ui_MainWindow;
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	QMenu *menu_Emu_DipSw;
	QActionGroup *actionGroup_DipSw;
	Action_Control_QC10 *action_Emu_DipSw[8];
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(QWidget *parent = 0);
	~META_MainWindow();
public slots:
};

QT_END_NAMESPACE

#endif // END
