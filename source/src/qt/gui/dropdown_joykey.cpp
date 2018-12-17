/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

//#include "osd.h"
#include <QApplication>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include "config.h"

#include "dropdown_keyset.h"
#include "dropdown_joykey.h"
#include "dropdown_joykeybutton.h"

CSP_DropDownJoykey::CSP_DropDownJoykey(QWidget *parent, QStringList *lst, USING_FLAGS *p) : QWidget(parent)
{
	p_wid = parent;
	using_flags = p;
	p_config = p->get_config_ptr();
	
	layout = new QGridLayout(this);
	int i;

	QCheckBox *numpad5 = new QCheckBox(QApplication::translate("JoykeyDialog", "Use 5 key to stop", 0), this);
	connect(numpad5, SIGNAL(toggled(bool)), this, SLOT(do_set_check_numpad5(bool)));
	layout->addWidget(numpad5, 0, 0, Qt::AlignLeft);
	numpad5->setChecked(p_config->joy_to_key_numpad5);
	
	QRadioButton *joykey_type_cursor = new QRadioButton(QApplication::translate("JoykeyDialog", "Cursor keys", 0), this);
	QRadioButton *joykey_type_2468 = new QRadioButton(QApplication::translate("JoykeyDialog", "2468", 0), this);
	QRadioButton *joykey_type_1379 = new QRadioButton(QApplication::translate("JoykeyDialog", "2468 + 1379", 0), this);
	QRadioButton *joykey_type_1235 = new QRadioButton(QApplication::translate("JoykeyDialog", "1235", 0), this);

	switch(p_config->joy_to_key_type) {
	case 0:
		joykey_type_cursor->setChecked(true);
		break;
	case 1:
		joykey_type_2468->setChecked(true);
		break;
	case 2:
		joykey_type_1379->setChecked(true);
		break;
	case 3:
		joykey_type_1235->setChecked(true);
		break;
	default:
		joykey_type_cursor->setChecked(true);
		p_config->joy_to_key_type = 0;
		break;
	}		
	QLabel *label_joykey_type = new QLabel(QApplication::translate("JoykeyDialog", "Joykey Type:", 0), this);
	layout->addWidget(label_joykey_type, 1, 0, Qt::AlignLeft);
	layout->addWidget(joykey_type_cursor, 2, 0, Qt::AlignLeft);
	layout->addWidget(joykey_type_2468, 2, 1, Qt::AlignLeft);
	layout->addWidget(joykey_type_1379, 3, 0, Qt::AlignLeft);
	layout->addWidget(joykey_type_1235, 3, 1, Qt::AlignLeft);

	connect(joykey_type_cursor, SIGNAL(clicked()), this, SLOT(do_set_type_cursor()));
	connect(joykey_type_2468, SIGNAL(clicked()), this, SLOT(do_set_type_2468()));
	connect(joykey_type_1379, SIGNAL(clicked()), this, SLOT(do_set_type_1379()));
	connect(joykey_type_1235, SIGNAL(clicked()), this, SLOT(do_set_type_1235()));
	
	QLabel *label_buttons = new QLabel(QApplication::translate("JoykeyDialog", "Physical Buttons:", 0), this);
	layout->addWidget(label_buttons, 4, 0, Qt::AlignLeft);

	_TCHAR tmps[32];
	QString nm;
	for(i = 0; i < 12; i++) {
		memset(tmps, 0x00, sizeof(char) * 20);
		label_button[i] = new QLabel(this);
		js_button[i] = new CSP_DropDownJoykeyButton(p, this, lst, i);
		snprintf(tmps, 32, "Button#%02d to:", i + 1);
		nm = QString::fromUtf8(tmps);
		label_button[i]->setText(nm);
		layout->addWidget(label_button[i], (i / 4) * 2 + 5 + 0, i % 4, Qt::AlignLeft);
		layout->addWidget(js_button[i], (i / 4) * 2 + 5 + 1, i % 4, Qt::AlignLeft);
	}


	if(p_wid == NULL) this->setWindowIcon(QIcon(":/icon_gamepad.png"));
	this->setLayout(layout);
	this->show();
}

CSP_DropDownJoykey::~CSP_DropDownJoykey()
{
}

void CSP_DropDownJoykey::do_set_check_numpad5(bool n)
{
	p_config->joy_to_key_numpad5 = n;
}

void CSP_DropDownJoykey::do_set_type_cursor()
{
	p_config->joy_to_key_type = 0;
}

void CSP_DropDownJoykey::do_set_type_2468()
{
	p_config->joy_to_key_type = 1;
}

void CSP_DropDownJoykey::do_set_type_1379()
{
	p_config->joy_to_key_type = 2;
}

void CSP_DropDownJoykey::do_set_type_1235()
{
	p_config->joy_to_key_type = 3;
}

