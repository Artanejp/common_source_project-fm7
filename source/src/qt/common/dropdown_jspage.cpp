
#include "vm.h"
#include "dropdown_keyset.h"
#include "dropdown_jsbutton.h"
#include "dropdown_jspage.h"
#include <QApplication>

CSP_DropDownJSPage::CSP_DropDownJSPage(QWidget *parent, QStringList *lst, int jsnum)
{
	int i;
	QString nm;
	char tmps[32];
	p_wid = parent;
	layout = new QGridLayout(this);
	bind_jsnum = jsnum;
	
	for(i = 0; i < 4; i++) {
		//label[i] = new QLabel(this);
		combo_js[i] = new CSP_DropDownJSButton(this, lst, jsnum, i);
	}
	label_axis = new QLabel(QApplication::translate("MainWindow", "<B>Physical Axis:</B>", 0), this);
	layout->addWidget(label_axis, 0, 0, Qt::AlignLeft);
	// Down
	layout->addWidget(combo_js[1], 1, 1, Qt::AlignRight);
	// Up
	layout->addWidget(combo_js[0], 3, 1, Qt::AlignRight);
	// Left
	layout->addWidget(combo_js[3], 2, 0, Qt::AlignRight);
	// Right
	layout->addWidget(combo_js[2], 2, 2, Qt::AlignRight);
	label_buttons = new QLabel(QApplication::translate("MainWindow", "<B>Physical Buttons:</B>", 0), this);
	layout->addWidget(label_buttons, 4, 0, Qt::AlignLeft);
#if defined(USE_JOY_BUTTON_CAPTIONS)
	int joybuttons = sizeof(joy_button_captions) / sizeof(_TCHAR *) - 4;
#endif
	
	for(i = 0; i < 12; i++) {

#if defined(USE_JOY_BUTTON_CAPTIONS)		
		if(joybuttons > i) {
#endif			
			memset(tmps, 0x00, sizeof(char) * 20);
			label_button[i] = new QLabel(this);
			js_button[i] = new CSP_DropDownJSButton(this, lst, jsnum, i + 4);
#if defined(USE_JOY_BUTTON_CAPTIONS)		
			snprintf(tmps, 32, "<B>%s</B>", joy_button_captions[i + 4]);
#else		
			snprintf(tmps, 32, "<B>#%02d:</B>", i + 1);
#endif		
			nm = QString::fromUtf8(tmps);
			label_button[i]->setText(nm);
			layout->addWidget(label_button[i], (i / 4) * 2 + 5 + 0, i % 4, Qt::AlignLeft);
			layout->addWidget(js_button[i], (i / 4) * 2 + 5 + 1, i % 4, Qt::AlignLeft);
#if defined(USE_JOY_BUTTON_CAPTIONS)
		}
#endif
	}
	this->setLayout(layout);
	connect(this, SIGNAL(sig_select_js_button(int, int, int)), parent, SLOT(do_set_js_button(int, int, int)));
	connect(this, SIGNAL(sig_select_js_button_idx(int, int, int)), parent, SLOT(do_set_js_button_idx(int, int, int)));
}

CSP_DropDownJSPage::~CSP_DropDownJSPage()
{
}


void CSP_DropDownJSPage::do_select_up(int index)
{
	if(index < 16) {
		emit sig_select_js_button(bind_jsnum, 0, joystick_define_tbl[index].scan);
	}
	emit sig_select_js_button_idx(bind_jsnum, 0, -(index - 16));
}

void CSP_DropDownJSPage::do_select_down(int index)
{
	if(index < 16) {
		emit sig_select_js_button(bind_jsnum, 1, joystick_define_tbl[index].scan);
	}
	emit sig_select_js_button_idx(bind_jsnum, 1, -(index - 16));
}

void CSP_DropDownJSPage::do_select_left(int index)
{
	if(index < 16) {
		emit sig_select_js_button(bind_jsnum, 2, joystick_define_tbl[index].scan);
	}
	emit sig_select_js_button_idx(bind_jsnum, 2, -(index - 16));
}

void CSP_DropDownJSPage::do_select_right(int index)
{
	if(index < 16) {
		emit sig_select_js_button(bind_jsnum, 3, joystick_define_tbl[index].scan);
	}
	emit sig_select_js_button_idx(bind_jsnum, 3, -(index - 16));
}

void CSP_DropDownJSPage::do_select_js_button(int jsnum, int button, int scan)
{
	//printf("Select: %d %d %d\n", jsnum, button, scan);
	emit sig_select_js_button(jsnum, button, scan);
}

void CSP_DropDownJSPage::do_select_js_button_idx(int jsnum, int button, int scan)
{
	emit sig_select_js_button_idx(jsnum, button, scan);
	//printf("Select_Idx: %d %d %d\n", jsnum, button, scan);
}
