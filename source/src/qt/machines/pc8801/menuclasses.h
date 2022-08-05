
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
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
	Action_Control *actionMemoryWait; //

	Action_Control *actionHMB20; //
	Action_Control *actionGSX8800; //
	Action_Control *actionPCG8100; //
	
	Action_Control *actionCMD_Sing; //
	Action_Control *actionPalette; //
	
	Action_Control *actionFDD_5Inch; //
	Action_Control *actionFDD_8Inch; //
	
	Action_Control *actionM88DRV; //
	Action_Control *actionQuasiS88CMT; //
	
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent = 0);
	~META_MainWindow();
	void retranslateVolumeLabels(Ui_SoundDialog *p);

};

QT_END_NAMESPACE

#endif // END
