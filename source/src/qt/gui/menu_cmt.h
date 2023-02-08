/*
 * Menu_MetaClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */


#ifndef _CSP_QT_MENU_CMT_CLASSES_H
#define _CSP_QT_MENU_CMT_CLASSES_H

#include "menu_metaclass.h"

QT_BEGIN_NAMESPACE

class DLL_PREFIX Menu_CMTClass: public Menu_MetaClass {
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
	class Action_Control *action_direct_load_mzt;

	QIcon icon_cmt;
	QIcon icon_play_start;
	QIcon icon_play_stop;
	QIcon icon_ff;
	QIcon icon_rew;
	QIcon icon_apss_forward;
	QIcon icon_apss_backward;
	QIcon icon_record_to_wav;

public:
	Menu_CMTClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent = 0, int drv = 0, int base_drv = 1);
	~Menu_CMTClass();
	void create_pulldown_menu_device_sub() override;
	void connect_menu_device_sub(void) override;
	void retranslate_pulldown_menu_device_sub(void) override;
	void connect_via_emu_thread(EmuThreadClassBase *p) override;

public slots:
	void do_open_rec_dialog();
	void do_add_rec_media_extension(QString ext, QString description);
	void do_open_write_cmt(QString s);

signals:
	int sig_direct_load_mzt(int, bool);
	int sig_open_write_cmt(int, QString);
};

QT_END_NAMESPACE

#endif
