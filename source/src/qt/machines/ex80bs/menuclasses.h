


#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "emu.h"

#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class Ui_MainWindow;
class USING_FLAGS;
class CSP_Logger;
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
private:
	QMenu *menuDipSW1;
	QMenu *menuDipSW2;
	QMenu *menuVramAddr;
	
	QActionGroup *actionGroup_DipSW1;
	QActionGroup *actionGroup_DipSW2;
	QActionGroup *actionGroup_VramAddr;
	
	QAction *actionDipSW1_ON;
	QAction *actionDipSW1_OFF;
	
	QAction *actionDipSW2_ON;
	QAction *actionDipSW2_OFF;
	
	QAction *actionVramAddr[4];
public:
	META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent = 0);
	~META_MainWindow();
	
	void setupUI_Emu(void) override;
	void retranslateUi(void) override;
};

QT_END_NAMESPACE

#endif // END
