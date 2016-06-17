/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 24, 2016 : Initial
 */
#ifndef _CSP_DIALOG_SET_KEY_COMBO_H
#define _CSP_DIALOG_SET_KEY_COMBO_H

#include "dropdown_keyset.h"
#include <QComboBox>
#include <QList>
#include <QStringList>
#include "common.h"

QT_BEGIN_NAMESPACE
class DLL_PREFIX CSP_KeySetupCombo: public QComboBox
{
	Q_OBJECT
protected:
	const keydef_table_t *scan_table;
	uint32_t this_vk;
	uint32_t this_scan;

	QStringList scanname_list;
	QList<uint32_t> scancode_list;
public:
	CSP_KeySetupCombo(QWidget *parent,
					  int num,
					  keydef_table_t *key_table,
					  const keydef_table_t *base_table);
	
	~CSP_KeySetupCombo();
public slots:
	void do_selected(int index);
signals:
	int sig_selected(uint32_t, uint32_t);
};
QT_END_NAMESPACE

#endif  //_CSP_DIALOG_SET_KEY_COMBO_H
