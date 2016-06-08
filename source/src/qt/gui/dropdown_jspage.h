/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#ifndef _CSP_DROPDOWN_JSPAGE_H
#define _CSP_DROPDOWN_JSPAGE_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QStringList>
#include <QString>
#include "common.h"

#include "dropdown_jsbutton.h"


QT_BEGIN_NAMESPACE

class DLL_PREFIX CSP_DropDownJSPage: public QWidget {
	Q_OBJECT
protected:
	QWidget *p_wid;
	QGridLayout *layout;
	//QLabel *label[4];
	QLabel *label_buttons;
	QLabel *label_axis;

	CSP_DropDownJSButton *combo_js[4];
	
	CSP_DropDownJSButton *js_button[12];
	QLabel *label_button[12];
	int bind_jsnum;
public:
	CSP_DropDownJSPage(QWidget *parent = 0, QStringList *lst = 0, int jsnum = 0);
	~CSP_DropDownJSPage();

public slots:
	void do_select_up(int index);
	void do_select_down(int index);
	void do_select_left(int index);
	void do_select_right(int index);
	void do_select_js_button(int jsnum, int button_num, int assigned_value);
	void do_select_js_button_idx(int jsnum, int button_num, int assigned_value);
signals:
	int sig_select_js_button(int jsnum, int button_num, int assigned_value);
	int sig_select_js_button_idx(int jsnum, int button_num, int assigned_value);
};	

QT_END_NAMESPACE
#endif // _CSP_DROPDOWN_JSPAGE_H
