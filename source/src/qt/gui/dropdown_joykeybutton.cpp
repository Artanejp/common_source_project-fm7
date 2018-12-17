/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#include <QVariant>
#include <QApplication>
#include "dropdown_keytables.h"
#include "dropdown_joykeybutton.h"
#include "menu_flags.h"

CSP_DropDownJoykeyButton::CSP_DropDownJoykeyButton(USING_FLAGS *p, QWidget *parent, QStringList *lst, int button_num) : QWidget(parent)
{
	p_wid = parent;
	using_flags = p;
	p_config = p->get_config_ptr();
	
	bind_button = button_num;
	layout = new QHBoxLayout(this);
	combo = new QComboBox(this);	

	for(int i = 0; i < 256; i++) {
		if(default_key_table_106_QtScan[i].vk == 0xffffffff) break;
		combo->addItem(QString::fromUtf8(default_key_table_106_QtScan[i].name), QVariant((int)default_key_table_106_QtScan[i].vk));
	}		
	connect(combo, SIGNAL(activated(int)), this, SLOT(do_select(int)));
	if((button_num < 16) && (button_num >= 0)) {
		for(int i = 0; i < combo->count(); i++) {
			if(p_config->joy_to_key_buttons[button_num] == default_key_table_106_QtScan[i].vk) {
				combo->setCurrentIndex(i);
				break;
			}
		}
	}
	layout->addWidget(combo);
	this->setLayout(layout);
}

CSP_DropDownJoykeyButton::~CSP_DropDownJoykeyButton()
{
}

void CSP_DropDownJoykeyButton::do_select(int idx)
{
	int vk = default_key_table_106_QtScan[idx].vk;
	p_config->joy_to_key_buttons[bind_button] = -vk;	
}

