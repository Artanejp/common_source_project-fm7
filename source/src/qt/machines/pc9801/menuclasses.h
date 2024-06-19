
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

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
	QAction *actionEGC;
#endif
#if defined(SUPPORT_320KB_FDD_IF)
	QAction *actionConnect2D;
#endif
#if defined(_PC9801) || defined(_PC9801E)
	QAction *actionConnect2DD;
	QAction *actionConnect2HD;
#endif
	// DipSW 2
	QAction *actionGDC_FAST;
	QAction *actionFDD_INTFDD_OFF;
	QAction *actionINTHDD_ON;
	QAction *actionINIT_MEMSW;
	QAction *actionHEIGHT20;
	QAction *actionWIDTH40;
	QAction *actionTERMINAL;
	QAction *actionLT;

	// DipSW 3
	QAction *actionRAM_512K;
#if defined(HAS_SUB_V30)
	QAction *actionSUB_V30;
#endif

#if defined(_PC98DO)
	QAction *actionMemoryWait;
	QAction *actionCMD_Sing; //
	QAction *actionPalette; //
	QAction *actionFDD_5Inch; //
//	QAction *actionFDD_8Inch; //

	QAction *actionM88DRV; //
	QAction *actionQuasiS88CMT; //
#endif
public:
	META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent = 0);
	~META_MainWindow();

	void setupUI_Emu(void) override;
	void retranslateUi(void) override;
};

QT_END_NAMESPACE

#endif // END
