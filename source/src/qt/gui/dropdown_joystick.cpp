/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

//#include "osd.h"
#include "config.h"

#include "dropdown_keyset.h"
#include "dropdown_jsbutton.h"
#include "dropdown_jspage.h"
#include "dropdown_joystick.h"
#include "joy_thread.h"

CSP_DropDownJoysticks::CSP_DropDownJoysticks(QWidget *parent, QStringList *lst, std::shared_ptr<USING_FLAGS> p,std::shared_ptr<JoyThreadClass> joy) : QWidget(parent)
{
	p_wid = parent;
	using_flags = p;
	p_config = p->get_config_ptr();
	p_joy = joy;
	layout = new QHBoxLayout(this);
	int i;

	tabBox = new QTabWidget(this);
	for(i = 0; i < 4; i++) {
		QString tmps;
		QString ns;
		pages[i] = new CSP_DropDownJSPage(using_flags, this, lst, i);
		ns.setNum(i + 1);
		tmps = QString::fromUtf8("Joystick") + ns;
		tabBox->addTab(pages[i], tmps);
	}
	layout->addWidget(tabBox);
	if(p_joy.get() != NULL) {
		connect(p_joy.get(), SIGNAL(sig_state_dpad(int, bool)), this, SLOT(do_check_dpademu(int, bool)));
		connect(this, SIGNAL(sig_set_emulate_dpad(int, bool)), p_joy.get(), SLOT(do_set_emulate_dpad(int, bool)));
		connect(this, SIGNAL(sig_assign_joynum(int, int)), p_joy.get(), SLOT(do_map_joy_num(int, int)));
	}			

	if(p_wid == NULL) this->setWindowIcon(QIcon(":/icon_gamepad.png"));
	this->setLayout(layout);
	this->show();
}

CSP_DropDownJoysticks::~CSP_DropDownJoysticks()
{
}

void CSP_DropDownJoysticks::do_set_js_button(int jsnum, int button_num, int assigned_value)
{
	if((button_num < 0) || (button_num >= 16)) return;
	if((jsnum < 0) || (jsnum >= 4)) return;
	//printf("Select: %d %d %d\n", jsnum, button_num, assigned_value);
	p_config->joy_buttons[jsnum][button_num] = assigned_value;
}

void CSP_DropDownJoysticks::do_set_js_button_idx(int jsnum, int button_num, int assigned_value)
{
	if((button_num < 0) || (button_num >= 16)) return;
	if((jsnum < 0) || (jsnum >= 4)) return;
	//printf("Select_Idx: %d %d %d\n", jsnum, button_num, assigned_value);
	p_config->joy_buttons[jsnum][button_num] = assigned_value;
}

void CSP_DropDownJoysticks::do_check_dpademu(int num, bool val)
{
	if(num < 0) return;
	if(num >= 4) return;
	if(pages[num]) {
		pages[num]->do_set_dpademu_state(val);
	}
}

void CSP_DropDownJoysticks::do_changed_state_dpademu(int num, bool val)
{
	emit sig_set_emulate_dpad(num, val);
}

void CSP_DropDownJoysticks::do_assign_joynum(int joynum, int num)
{
	emit sig_assign_joynum(joynum, num);
}
