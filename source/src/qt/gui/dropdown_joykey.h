/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#ifndef _CSP_DROPDOWN_JOYKEY_H
#define _CSP_DROPDOWN_JOYKEY_H

#include <QWidget>
#include <QStringList>
#include <QString>
#include <QGridLayout>
#include <memory>

#include "config.h"
#include "common.h"
#include "menu_flags.h"


QT_BEGIN_NAMESPACE
class QLabel;
class CSP_DropDownJoykeyButton;

class DLL_PREFIX CSP_DropDownJoykey: public QWidget {
	Q_OBJECT
protected:
	config_t *p_config;
	QWidget *p_wid;
	QWidget *window;
	QGridLayout *layout;
	std::shared_ptr<USING_FLAGS> using_flags;

	QLabel *label_button[12];
	CSP_DropDownJoykeyButton *js_button[12];
public:
	CSP_DropDownJoykey(QWidget *parent, QStringList *lst, std::shared_ptr<USING_FLAGS> p);
	~CSP_DropDownJoykey();

public slots:
	void do_set_check_numpad5(bool n);
	void do_set_type_cursor();
	void do_set_type_2468();
	void do_set_type_1379();
	void do_set_type_1235();

signals:
	
};

QT_END_NAMESPACE
#endif //#ifndef _CSP_DROPDOWN_JOYKEY_H
