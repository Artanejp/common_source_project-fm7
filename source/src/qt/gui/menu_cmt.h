/*
 * Menu_MetaClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */


#ifndef _CSP_QT_MENU_CMT_CLASSES_H
#define _CSP_QT_MENU_CMT_CLASSES_H

#include "menu_metaclass.h"

QT_BEGIN_NAMESPACE

class Menu_CMTClass: public Menu_MetaClass {
	Q_OBJECT
protected:
	QString desc_rec;
	QStringList ext_rec_filter;
	
	QActionGroup *action_group_tape_button;
	class Action_Control *action_play_start;
	class Action_Control *action_play_stop;
	class Action_Control *action_fast_forward;
	class Action_Control *action_fast_rewind;
	class Action_Control *action_apss_forward;
	class Action_Control *action_apss_rewind;
	class Action_Control *action_recording;

	class Action_Control *action_wave_shaper;
#if defined(_MZ80A) || defined(_MZ80K) || defined(_MZ1200) || defined(_MZ700) || defined(_MZ800) || defined(_MZ1500) || \
	defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
	class Action_Control *action_direct_load_mzt;
#endif	
	QIcon icon_cmt;
	QIcon icon_play_start;
	QIcon icon_play_stop;
	QIcon icon_ff;
	QIcon icon_rew;
	QIcon icon_apss_forward;
	QIcon icon_apss_backward;
	QIcon icon_record_to_wav;

public:
	Menu_CMTClass(EMU *ep, QMenuBar *root_entry, QString desc, QWidget *parent = 0, int drv = 0);
	~Menu_CMTClass();
	void create_pulldown_menu_device_sub();
	void connect_menu_device_sub(void);
	void retranslate_pulldown_menu_device_sub(void);
public slots:
	void do_open_rec_dialog();
	void do_add_rec_media_extension(QString ext, QString description);
	void do_eject_cmt(int dummy);
signals:
	int sig_close_tape(void);
};

QT_END_NAMESPACE

#endif
