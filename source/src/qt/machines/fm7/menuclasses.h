
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"
// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE
class Ui_SoundDialog;
class USING_FLAGS;

class Object_Menu_Control_7: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_7(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_7();
signals:
	//  int sig_sound_device(int);
	int sig_emu_update_config(void);
public slots:
# if defined(WITH_Z80)
	void do_set_z80card_on(bool flag);
	void do_set_z80_irq(bool flag);
	void do_set_z80_firq(bool flag);
	void do_set_z80_nmi(bool flag);
# endif	
# if defined(_FM77AV_VARIANTS)   
	void do_set_hsync(bool flag);
# endif
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	void do_set_kanji_rom(bool flag);
	void do_set_320kFloppy(bool flag);
# endif
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
	void do_set_1MFloppy(bool flag);
# endif   
# if defined(_FM8)
	void do_set_protect_ram(bool flag);
# else   
	void do_set_cyclesteal(bool flag);
# endif
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
	void do_set_jis78_emulation(bool flag);
#endif
#if defined(CAPABLE_JCOMMCARD)
	void do_set_jcommcard(bool flag);
#endif
	void do_set_uart(bool flag);
	void do_set_autokey_5_8(void);
};

class Action_Control_7 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_7 *fm7_binds;
	Action_Control_7(QObject *parent, USING_FLAGS *p);
	~Action_Control_7();
public slots:
	void do_set_frameskip();
};

class CSP_Logger;
class Ui_MainWindow;
//  wrote of Specific menu.
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	QMenu *menuFrameSkip;
	QActionGroup *actionGroup_FrameSkip;
	class Action_Control_7 *actionFrameSkip[4];
# if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	class Action_Control_7 *actionExtRam;
# endif
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	class Action_Control_7 *actionKanjiRom;
# endif
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
	class Action_Control_7 *actionJIS78EMULATION;
# endif
# if defined(CAPABLE_JCOMMCARD)
	class Action_Control_7 *actionJCOMMCARD;
# endif	
# if defined(_FM8)
	class Action_Control_7 *actionRamProtect;
# else	
	class Action_Control_7 *actionCycleSteal;
# endif  
# if defined(_FM77AV_VARIANTS)   
	class Action_Control_7 *actionSyncToHsync;
# endif
	QActionGroup *actionGroup_Auto_5_8key;
	QMenu *menuAuto5_8Key;
	class Action_Control_7 *action_Neither_5_or_8key;
	class Action_Control_7 *action_Auto_5key;
	class Action_Control_7 *action_Auto_8key;
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	class Action_Control_7 *action_320kFloppy;
# endif  
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
	class Action_Control_7 *action_1MFloppy;
# endif  
# if defined(WITH_Z80)
	class Action_Control_7 *actionZ80CARD_ON;
	class Action_Control_7 *actionZ80_IRQ;
	class Action_Control_7 *actionZ80_FIRQ;
	class Action_Control_7 *actionZ80_NMI;
#endif
	class Action_Control_7 *actionUART[3];
	
	void setupUI_Emu(void);
	void retranslateUi(void);
	void retranslateVolumeLabels(Ui_SoundDialog *p);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
public slots:
	void do_set_extram(bool flag);
};

QT_END_NAMESPACE

#endif // END
