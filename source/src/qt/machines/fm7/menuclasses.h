
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "mainwidget.h"
#include "vm.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE
class Ui_SoundDialog;
class USING_FLAGS;
class CSP_Logger;
class Ui_MainWindow;
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	QMenu *menuFrameSkip;
	QActionGroup *actionGroup_FrameSkip;
	QAction *actionFrameSkip[4];
# if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	QAction *actionExtRam;
# endif
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	QAction *actionKanjiRom;
# endif
# if defined(CAPABLE_JCOMMCARD)
	QAction *actionJCOMMCARD;
# endif
	
# if defined(_FM8)
	QAction *actionRamProtect;
# else	
	QAction *actionCycleSteal;
# endif  

	QAction *actionSyncToHsync;

#if defined(CAPABLE_DICTROM) && !defined(_FM77AV40EX) && !defined(_FM77AV40SX)
	QAction *actionDictCard;
#endif
	QActionGroup *actionGroup_Auto_5_8key;
	QMenu *menuAuto5_8Key;
	QAction *action_Neither_5_or_8key;
	QAction *action_Auto_5key;
	QAction *action_Auto_8key;
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	QAction *action_320kFloppy;
# endif  
# if defined(HAS_2HD)
	QAction *action_1MFloppy;
# endif  
# if defined(WITH_Z80)
	QAction *actionZ80CARD_ON;
	QAction *actionZ80_IRQ;
	QAction *actionZ80_FIRQ;
	QAction *actionZ80_NMI;
#endif
	QAction *actionUART[3];
	
	void setupUI_Emu(void);
	void retranslateUi(void);
	void retranslateVolumeLabels(Ui_SoundDialog *p);
public:
	META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
	void do_set_frameskip();
	void do_set_autokey_5_8(void);
	
};

QT_END_NAMESPACE

#endif // END
