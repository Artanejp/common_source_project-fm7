


#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "emu.h"
#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class Ui_MainWindow;
class USING_FLAGS;

class Action_Control_EX80 : public Action_Control
{
	Q_OBJECT
private:
	int bind_int;
	bool bind_bool;
public:
	Action_Control_EX80(QObject *parent, USING_FLAGS *pp);
	~Action_Control_EX80();
	void setBoolValue(bool flag);
	void setIntValue(int val);
	
public slots:
	void do_set_sw1();
	void do_set_sw2();
	void do_set_vram_addr();
signals:
	int sig_set_dipsw(int, bool);
};

class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
private:
	QMenu *menuDipSW1;
	QMenu *menuDipSW2;
	QMenu *menuVramAddr;
	
	QActionGroup *actionGroup_DipSW1;
	QActionGroup *actionGroup_DipSW2;
	QActionGroup *actionGroup_VramAddr;
	
	Action_Control_EX80 *actionDipSW1_ON;
	Action_Control_EX80 *actionDipSW1_OFF;
	
	Action_Control_EX80 *actionDipSW2_ON;
	Action_Control_EX80 *actionDipSW2_OFF;
	
	Action_Control_EX80 *actionVramAddr[4];
protected:
	void setupUI_Emu(void);
	void retranslateUi(void);
  
public:
	META_MainWindow(USING_FLAGS *p, QWidget *parent = 0);
	~META_MainWindow();
public slots:

};

QT_END_NAMESPACE

#endif // END
