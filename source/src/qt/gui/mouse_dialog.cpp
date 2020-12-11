
#include <QApplication>
#include <QString>
#include <QVariant>
#include <QAction>
#include <QStyle>
#include <QString>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>

#include "mouse_dialog.h"

#include "qt_main.h"
#include "commonclasses.h"

#include "menu_flags.h"

Ui_MouseDialog::Ui_MouseDialog(USING_FLAGS *p, QWidget *parent) : QWidget(0)
{
	using_flags = p;
	p_config = p->get_config_ptr();
	sensitivity = p_config->mouse_sensitivity & 0xffff;

	layout = new QGridLayout;
	
	label_slider1 = new QLabel(QApplication::translate("Ui_MouseDialog", "Mouse Sensitivity", 0), this);
	label_value = new QLabel(QString::fromUtf8("1.0"));
	slider1 = new QSlider(Qt::Horizontal, this);
	reset_button = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogResetButton), "");

	
	slider1->setMaximum(12288); // 8192 * 1.5
	slider1->setMinimum(1024);  // 8192 * 0.125
	
	if(sensitivity < slider1->minimum()) sensitivity = slider1->minimum();
	if(sensitivity > slider1->maximum()) sensitivity = slider1->maximum();
	slider1->setValue(sensitivity);
	p_config->mouse_sensitivity = sensitivity;
	
	QString n;
	double doubleVal;
	doubleVal = ((double)sensitivity) / 8192.0;
	n.setNum(doubleVal, 'f', 3);
	label_value->setText(n);
	
	connect(slider1, SIGNAL(valueChanged(int)), this, SLOT(do_set_value(int)));
	connect(this, SIGNAL(sig_set_nvalue(int)), slider1, SLOT(setValue(int)));
	connect(this, SIGNAL(sig_set_value(QString)), label_value, SLOT(setText(QString)));
	connect(reset_button, SIGNAL(pressed()), this, SLOT(do_reset_values()));

	layout->addWidget(label_slider1, 0, 0);
	layout->addWidget(label_value, 1, 0);
	layout->addWidget(slider1, 2, 0);
	layout->addWidget(reset_button, 2, 1);
	this->setLayout(layout);

}

Ui_MouseDialog::~Ui_MouseDialog()
{
}

void Ui_MouseDialog::do_set_value(int val)
{
	if(val < 0) val = 0;
	if(val > 32768) val = 32768;
	sensitivity = val; //  Default Value
	p_config->mouse_sensitivity = sensitivity;
	
	QString n;
	double doubleVal;
	doubleVal = ((double)sensitivity) / 8192.0;
	n.setNum(doubleVal, 'f', 3);
	emit sig_set_value(n);
	emit sig_set_nvalue(val);
}	

void Ui_MouseDialog::do_reset_values()
{
	sensitivity = 8192; //  Default Value
	p_config->mouse_sensitivity = sensitivity;
	
	QString n;
	double doubleVal = 1.0;
	n.setNum(doubleVal, 'f', 3);

	emit sig_set_value(n);
	emit sig_set_nvalue(sensitivity);
}
