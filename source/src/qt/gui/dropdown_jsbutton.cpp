/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#include "config.h"

#include "dropdown_keyset.h"
#include "dropdown_jsbutton.h"
#include "menu_flags.h"

CSP_DropDownJSButton::CSP_DropDownJSButton(std::shared_ptr<USING_FLAGS> p, QWidget *parent, QStringList *lst, int jsnum, int button_num) : QWidget(parent)
{
	config_t *p_config;
	p_wid = parent;
	using_flags = p;
	p_config = p->get_config_ptr();
	
	bind_jsnum = jsnum;
	bind_button = button_num;
	layout = new QHBoxLayout(this);
	combo = new QComboBox(this);	
	int i;
	for(i = 0; i < (sizeof(joystick_define_tbl) / sizeof(keydef_table_t)) ; i++) {
		combo->addItem(QString::fromUtf8(joystick_define_tbl[i].name));
	}
	if(lst != NULL) combo->addItems(*lst);
	connect(combo, SIGNAL(activated(int)), this, SLOT(do_select(int)));
	if(p_wid != NULL) {
		connect(this, SIGNAL(sig_select_js_button(int, int, int)), p_wid, SLOT(do_select_js_button(int, int, int)));
		connect(this, SIGNAL(sig_select_js_button_idx(int, int, int)), p_wid, SLOT(do_select_js_button_idx(int, int, int)));
	}
	if((button_num < 16) && (button_num >= 0)) {
		if((jsnum < 4) && (jsnum >= 0)){
			if((p_config->joy_buttons[jsnum][button_num] < 0) && (p_config->joy_buttons[jsnum][button_num] > -256)) {
				combo->setCurrentIndex(-p_config->joy_buttons[jsnum][button_num] + 16);
			} else if((p_config->joy_buttons[jsnum][button_num] >= 0) && (p_config->joy_buttons[jsnum][button_num] < 16)) {
				combo->setCurrentIndex(p_config->joy_buttons[jsnum][button_num]);
			}
		}
	}
	layout->addWidget(combo);
	this->setLayout(layout);
}

CSP_DropDownJSButton::~CSP_DropDownJSButton()
{
}

void CSP_DropDownJSButton::do_select(int idx)
{
	if(idx < 0) return;
	if(idx < 16) {
		emit sig_select_js_button(bind_jsnum, bind_button, idx);
		return;
	}
	emit sig_select_js_button_idx(bind_jsnum, bind_button, -(idx - 16));
}

