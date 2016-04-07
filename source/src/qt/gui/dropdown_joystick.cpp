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

CSP_DropDownJoysticks::CSP_DropDownJoysticks(QWidget *parent, QStringList *lst) : QWidget(parent)
{
	p_wid = parent;

	layout = new QHBoxLayout(this);
	int i;

	tabBox = new QTabWidget(this);
	for(i = 0; i < 4; i++) {
		QString tmps;
		QString ns;
		pages[i] = new CSP_DropDownJSPage(this, lst, i);
		ns.setNum(i + 1);
		tmps = QString::fromUtf8("Joystick") + ns;
		tabBox->addTab(pages[i], tmps);
	}
	layout->addWidget(tabBox);

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
	config.joy_buttons[jsnum][button_num] = assigned_value;
}

void CSP_DropDownJoysticks::do_set_js_button_idx(int jsnum, int button_num, int assigned_value)
{
	if((button_num < 0) || (button_num >= 16)) return;
	if((jsnum < 0) || (jsnum >= 4)) return;
	//printf("Select_Idx: %d %d %d\n", jsnum, button_num, assigned_value);
	config.joy_buttons[jsnum][button_num] = assigned_value;
}

