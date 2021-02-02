
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
public slots:
	void do_set_memory_wait(bool);
	void do_set_egc(bool);
	void do_set_gdc_fast(bool);
	void do_set_ram_512k(bool);	
	void do_set_init_memsw(bool);
	void do_set_enable_v30(bool flag);
	void do_set_connect_2d(bool flag);
	void do_set_connect_2dd(bool flag);
	void do_set_connect_2hd(bool flag);
	void do_set_palette_vblank(bool flag);
	void do_set_fdd_5inch(bool flag);
//	void do_set_fdd_8inch(bool flag);
	void do_set_cmd_sing(bool flag);
	void do_set_m88drv(bool flag);
	void do_set_quasis88_cmt(bool flag);
signals:
	int sig_emu_update_config();
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
	Action_Control_98 *actionRAM_512K;
	Action_Control_98 *actionINIT_MEMSW;
	Action_Control_98 *actionGDC_FAST;
#if defined(SUPPORT_EGC)
	Action_Control_98 *actionEGC;
#endif
#if defined(_PC98DO)
	Action_Control_98 *actionMemoryWait;
	Action_Control_98 *actionCMD_Sing; //
	Action_Control_98 *actionPalette; //
	Action_Control_98 *actionFDD_5Inch; //
//	Action_Control_98 *actionFDD_8Inch; //
	
	Action_Control_98 *actionM88DRV; //
	Action_Control_98 *actionQuasiS88CMT; //
#endif
#if defined(SUPPORT_320KB_FDD_IF)
	Action_Control_98 *actionConnect2D;
#endif
#if defined(_PC9801) || defined(_PC9801E)
	Action_Control_98 *actionConnect2DD;
	Action_Control_98 *actionConnect2HD;
#endif
#if !defined(SUPPORT_HIRESO)
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	Action_Control_98 *actionSUB_V30;
	QActionGroup   *actionGroup_RunningCpu;
	QMenu *menuRunCpu;
	Action_Control_98 *actionRunMainCPU;
	Action_Control_98 *actionRunSubCPU;
#endif
#endif
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
	void do_use_ix86();
	void do_use_v30();
	
};

QT_END_NAMESPACE

#endif // END
