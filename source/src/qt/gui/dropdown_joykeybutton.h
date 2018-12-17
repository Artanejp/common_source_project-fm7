/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: 18 Dec, 2018 : Initial
 */

#ifndef _CSP_QT_DROPDOWN_JOYKEY_BUTTON_H
#define _CSP_QT_DROPDOWN_JOYKEY_BUTTON_H

#include "config.h"
#include <QString>
#include <QStringList>
#include <QComboBox>
#include <QWidget>
#include <QHBoxLayout>

#include "dropdown_keytables.h"

class USING_FLAGS;

class DLL_PREFIX CSP_DropDownJoykeyButton: public QWidget {
	Q_OBJECT;
protected:
	QWidget *p_wid;
	config_t *p_config;
	QHBoxLayout *layout;
	QComboBox *combo;
	
	int bind_button;
	USING_FLAGS *using_flags;
public:
	CSP_DropDownJoykeyButton(USING_FLAGS *p, QWidget *parent = 0, QStringList *lst = 0, int button_num = 0);
	~CSP_DropDownJoykeyButton();
public slots:
	void do_select(int index);
signals:

};

#endif //_CSP_QT_DROPDOWN_JOYKEY_BUTTON_H
