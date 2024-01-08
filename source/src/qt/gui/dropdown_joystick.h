/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#ifndef _CSP_DROPDOWN_JOYSTICK_H
#define _CSP_DROPDOWN_JOYSTICK_H

#include <QWidget>
#include <QTabWidget>
#include <QStringList>
#include <QString>
#include <QHBoxLayout>
#include <memory>

#include "config.h"
#include "common.h"
#include "menu_flags.h"

class CSP_DropDownJSPage;
class JoyThreadClass;

QT_BEGIN_NAMESPACE

class DLL_PREFIX CSP_DropDownJoysticks: public QWidget {
	Q_OBJECT
protected:
	config_t *p_config;
	std::shared_ptr<JoyThreadClass> p_joy;
	QWidget *p_wid;
	QWidget *window;
	QHBoxLayout *layout;
	std::shared_ptr<USING_FLAGS> using_flags;
	
	QTabWidget *tabBox;
	CSP_DropDownJSPage *pages[4];
public:
	CSP_DropDownJoysticks(QWidget *parent, QStringList *lst, std::shared_ptr<USING_FLAGS> p, std::shared_ptr<JoyThreadClass> joy);
	~CSP_DropDownJoysticks();

public slots:
	void do_set_js_button(int jsnum, int button_num, int assigned_value);
	void do_set_js_button_idx(int jsnum, int button_num, int assigned_value);
	void do_check_dpademu(int num, bool val);
	void do_changed_state_dpademu(int num, bool val);
	void do_assign_joynum(int joynum, int num);
	
signals:
	int sig_set_emulate_dpad(int, bool);
	int sig_assign_joynum(int, int);
};

QT_END_NAMESPACE
#endif //#ifndef _CSP_DROPDOWN_JOYSTICK_H
