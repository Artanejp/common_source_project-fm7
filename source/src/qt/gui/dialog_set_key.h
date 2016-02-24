/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 24, 2016 : Initial
 */

#ifndef _CSP_DIALOG_SET_KEY_H
#define _CSP_DIALOG_SET_KEY_H

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>

#include "dropdown_keyset.h"
#include "dialog_set_key_combo.h"

class GLDrawClass;
QT_BEGIN_NAMESPACE

class CSP_KeySetDialog: public QWidget
{
	Q_OBJECT
protected:
	GLDrawClass *p_glv;
	QWidget *p_wid;
	
	QVBoxLayout *layout;
	QLabel *label_head;
	QScrollArea *scroll_area;
	QWidget *keycodes_widget;
	QGridLayout *keycodes_layout;
	QLabel *setup_head_label[2];
	CSP_KeySetupCombo *setup_combo[KEYDEF_MAXIMUM];
	QLabel *setup_label[KEYDEF_MAXIMUM];
public:
	CSP_KeySetDialog(QWidget *parent = NULL, GLDrawClass *glv = NULL);
	~CSP_KeySetDialog();
};
QT_END_NAMESPACE

#endif //#ifndef _CSP_DIALOG_SET_KEY_H
