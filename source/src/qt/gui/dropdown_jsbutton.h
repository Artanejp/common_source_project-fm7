/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#ifndef _CSP_QT_DROPDOWN_JSBUTTON_H
#define _CSP_QT_DROPDOWN_JSBUTTON_H

#include <QString>
#include <QStringList>
#include <QComboBox>
#include <QWidget>
#include <QHBoxLayout>

#include "dropdown_keytables.h"

class USING_FLAGS;

class DLL_PREFIX CSP_DropDownJSButton: public QWidget {
	Q_OBJECT;
protected:
	QWidget *p_wid;
	QHBoxLayout *layout;
	QComboBox *combo;
	
	int bind_button;
	int bind_jsnum;
	USING_FLAGS *using_flags;
public:
	CSP_DropDownJSButton(USING_FLAGS *p, QWidget *parent = 0, QStringList *lst = 0, int jsnum = 0, int button_num = 0);
	~CSP_DropDownJSButton();
public slots:
	void do_select(int index);
signals:
	int sig_select_js_button(int jsnum, int button_num, int assigned_value);
	int sig_select_js_button_idx(int jsnum, int button_num, int assigned_value);
};

#endif //_CSP_QT_DROPDOWN_JSBUTTON_H
