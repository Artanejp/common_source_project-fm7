
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
	class Action_Control *actionFrameSkip[4];
# if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	class Action_Control *actionExtRam;
# endif
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	class Action_Control *actionKanjiRom;
# endif
# if defined(CAPABLE_JCOMMCARD)
	class Action_Control *actionJCOMMCARD;
# endif
	
# if defined(_FM8)
	class Action_Control *actionRamProtect;
# else	
	class Action_Control *actionCycleSteal;
# endif  

	class Action_Control *actionSyncToHsync;

#if defined(CAPABLE_DICTROM) && !defined(_FM77AV40EX) && !defined(_FM77AV40SX)
	class Action_Control *actionDictCard;
#endif
	QActionGroup *actionGroup_Auto_5_8key;
	QMenu *menuAuto5_8Key;
	class Action_Control *action_Neither_5_or_8key;
	class Action_Control *action_Auto_5key;
	class Action_Control *action_Auto_8key;
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	class Action_Control *action_320kFloppy;
# endif  
# if defined(HAS_2HD)
	class Action_Control *action_1MFloppy;
# endif  
# if defined(WITH_Z80)
	class Action_Control *actionZ80CARD_ON;
	class Action_Control *actionZ80_IRQ;
	class Action_Control *actionZ80_FIRQ;
	class Action_Control *actionZ80_NMI;
#endif
	class Action_Control *actionUART[3];
	
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
