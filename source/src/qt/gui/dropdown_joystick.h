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
#include "common.h"
#include "menu_flags.h"

class CSP_DropDownJSPage;

QT_BEGIN_NAMESPACE

class DLL_PREFIX CSP_DropDownJoysticks: public QWidget {
	Q_OBJECT
protected:
	QWidget *p_wid;
	QWidget *window;
	QHBoxLayout *layout;

	QTabWidget *tabBox;
	CSP_DropDownJSPage *pages[4];
public:
	CSP_DropDownJoysticks(QWidget *parent, QStringList *lst, USING_FLAGS *using_flags);
	~CSP_DropDownJoysticks();

public slots:
	void do_set_js_button(int jsnum, int button_num, int assigned_value);
	void do_set_js_button_idx(int jsnum, int button_num, int assigned_value);

signals:
	
};

QT_END_NAMESPACE
#endif //#ifndef _CSP_DROPDOWN_JOYSTICK_H
