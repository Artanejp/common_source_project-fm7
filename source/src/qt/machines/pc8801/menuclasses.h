
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as
QT_BEGIN_NAMESPACE
class Ui_SoundDialog;
class USING_FLAGS;
class CSP_Logger;
class QActionGroup;
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	int config_sound_device_type;
	QAction *actionMemoryWait; //

	QAction *actionHMB20; //
	QAction *actionGSX8800; //
	QAction *actionPCG8100; //

	QAction *actionCMD_Sing; //
	QAction *actionPalette; //

	QAction *actionFDD_5Inch; //
	QAction *actionFDD_8Inch; //

	QAction *action16bit; //

	QAction *actionM88DRV; //
	QAction *actionQuasiS88CMT; //

public:
	META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent = 0);
	~META_MainWindow();
	
	void setupUI_Emu(void) override;
	void retranslateUi(void) override;
	void retranslateVolumeLabels(Ui_SoundDialog *p) override;
};

QT_END_NAMESPACE

#endif // END
