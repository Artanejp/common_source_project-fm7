/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 24, 2016 : Initial
 */

#include "dropdown_keyset.h"
#include "dialog_set_key_combo.h"
#include <QApplication>

CSP_KeySetupCombo::CSP_KeySetupCombo(QWidget *parent,
									 int num,
									 keydef_table_t *key_table,
									 const keydef_table_t *base_table) : QComboBox(parent)
{
	if((key_table == NULL) || (base_table == NULL)) return;
	this_vk = key_table->vk;
	this_scan = key_table->scan;
	scan_table = base_table;
	int i;
	scanname_list.clear();
	scancode_list.clear();
	scancode_list.append(0xffffffff);
	scanname_list.append(QApplication::translate("KeySetDialog", "Undefined", 0));
	for(i = 0; i < KEYDEF_MAXIMUM; i++) {
		if(base_table[i].vk == 0xffffffff) break;
		scanname_list.append(QString::fromUtf8(base_table[i].name));
		scancode_list.append(base_table[i].scan);
	}
	this->addItems(scanname_list);
	for(i = 0; i < scancode_list.size(); i++) {
		if(key_table->scan == scancode_list.value(i)) {
			this->setCurrentIndex(i);
			break;
		}
	}
	connect(this, SIGNAL(activated(int)), this, SLOT(do_selected(int)));
}

CSP_KeySetupCombo::~CSP_KeySetupCombo()
{
}

void CSP_KeySetupCombo::do_selected(int index)
{
	if((scancode_list.size() <= index) || (index < 1)) return;
	uint32 vk = this_vk;
	uint32 scan = scancode_list.value(index);
	emit sig_selected(vk, scan);
}

