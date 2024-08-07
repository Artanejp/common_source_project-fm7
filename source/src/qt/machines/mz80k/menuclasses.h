
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

//  wrote of MZ80 Specific menu.
class CSP_Logger;
class USING_FLAGS;
class QMenu;
class QActionGroup;
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	QAction *action_Emu_DipSw;
	
public:
	META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent = 0);
	~META_MainWindow();

	void setupUI_Emu(void) override;
	void retranslateUi(void) override;
};

QT_END_NAMESPACE

#endif // END
