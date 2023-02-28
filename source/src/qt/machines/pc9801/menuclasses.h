
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"

// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as
QT_BEGIN_NAMESPACE

class USING_FLAGS;
class QMenu;
class QActionGroup;
class Ui_MainWindow;
class CSP_Logger;
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	// DipSW 1
#if defined(SUPPORT_EGC)
	Action_Control *actionEGC;
#endif
#if defined(SUPPORT_320KB_FDD_IF)
	Action_Control *actionConnect2D;
#endif
#if defined(_PC9801) || defined(_PC9801E)
	Action_Control *actionConnect2DD;
	Action_Control *actionConnect2HD;
#endif
	// DipSW 2
	Action_Control *actionGDC_FAST;
	Action_Control *actionFDD_INTFDD_OFF;
	Action_Control *actionINTHDD_ON;
	Action_Control *actionINIT_MEMSW;
	Action_Control *actionHEIGHT20;
	Action_Control *actionWIDTH40;
	Action_Control *actionTERMINAL;
	Action_Control *actionLT;

	// DipSW 3
	Action_Control *actionRAM_512K;
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	Action_Control *actionSUB_V30;
#endif

#if defined(_PC98DO)
	Action_Control *actionMemoryWait;
	Action_Control *actionCMD_Sing; //
	Action_Control *actionPalette; //
	Action_Control *actionFDD_5Inch; //
//	Action_Control *actionFDD_8Inch; //

	Action_Control *actionM88DRV; //
	Action_Control *actionQuasiS88CMT; //
#endif
#if !defined(SUPPORT_HIRESO)
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	QActionGroup   *actionGroup_RunningCpu;
	QMenu *menuRunCpu;
	Action_Control *actionRunMainCPU;
	Action_Control *actionRunSubCPU;
#endif
#endif
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent = 0);
	~META_MainWindow();
};

QT_END_NAMESPACE

#endif // END
