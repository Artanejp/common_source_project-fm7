/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 24, 2016 : Initial
 */

#include "dialog_memory.h"

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>

#include "config.h"
#include "menu_flags.h"

CSP_MemoryDialog::CSP_MemoryDialog(std::shared_ptr<USING_FLAGS> p, QWidget *parent) : QWidget(parent)
{
	p_wid = parent;
	using_flags = p;
	p_config = NULL;
	_min = -1;
	_max = -1;
	current_val = -1;
	layout = new QGridLayout(this);
	label_head = new QLabel(QApplication::translate("MemorySetDialog", "<B>Memory Size</B>", 0));
	layout->addWidget(label_head);

	spin_ram = new QSpinBox(this);
	current_val = -1;
	
	if(p != NULL) {
		p_config = p->get_config_ptr();
		order = p->get_ram_size_order();
		_max = p->get_max_ram_size();
		_min = p->get_min_ram_size();
		current_val = p_config->current_ram_size;
	}
	QString orderstring;
	spin_ram->setEnabled(true);
	if((order == 0) || (_max < 0) || (_min < 0)) {
		spin_ram->setEnabled(false);
		orderstring = QString::fromUtf8("");
	} else if(order < 1024) {
		orderstring = QString::fromUtf8("x %1Bytes").arg(order);
	} else if(order == 1024) {
		orderstring = QString::fromUtf8("KBytes");
	} else if(order < (1024 * 1024)) {
		orderstring = QString::fromUtf8("x %1KBytes").arg(order);
	} else if(order == (1024 * 1024)) {
		orderstring = QString::fromUtf8("MBytes");
	} else {
		orderstring = QString::fromUtf8("x %1MBytes").arg(order);
	}
	order_label = new QLabel(orderstring);
	if(_max < _min) _min = _max;
	if(current_val > _max) current_val = _max;
	if(current_val < _min) current_val = _min;
	
	spin_ram->setMaximum(_max);
	spin_ram->setMinimum(_min);
	spin_ram->setValue(current_val);
	
	reset_button = new QPushButton(QApplication::translate("MemoryDialog", "Reset", 0));
	cancel_button = new QPushButton(QApplication::translate("MemoryDialog", "Cancel", 0));
	close_button = new QPushButton(QApplication::translate("MemoryDialog", "Save Options", 0));

	connect(close_button, SIGNAL(clicked()), this, SLOT(do_set_value()));
	connect(close_button, SIGNAL(clicked()), this, SLOT(do_reset_value()));
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(close()));

	layout->addWidget(label_head, 0, 0);
	layout->addWidget(spin_ram, 1, 0);
	layout->addWidget(order_label, 1, 2);
	layout->addWidget(reset_button, 3, 0);
	layout->addWidget(cancel_button, 3, 2);
	layout->addWidget(close_button, 3, 3);
	this->setLayout(layout);
}

CSP_MemoryDialog::~CSP_MemoryDialog()
{
}
void CSP_MemoryDialog::do_set_value()
{
	if(p_config != NULL) {
		int val = spin_ram->value();
		if(val < _min) val = _min;
		if(val > _max) val = _max;
		if(val < 0) val = 0;
		p_config->current_ram_size = val;
		// Update config?
	}
	close();
}

void CSP_MemoryDialog::do_reset_value()
{
	spin_ram->setValue(current_val);
}
