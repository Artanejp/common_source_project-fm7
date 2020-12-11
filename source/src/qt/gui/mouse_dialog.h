#pragma once

#include "menu_flags.h"
#include "common.h"
#include "config.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QString;
class QGridLayout;
class QLabel;
class QPushButton;
class QSlider;

class USING_FLAGS;

class Ui_MouseDialog : public QWidget {
	Q_OBJECT
	
protected:
	USING_FLAGS* using_flags;
	config_t* p_config;
	QGridLayout* layout;
	QLabel* label_slider1;
	QLabel* label_value;
	QSlider* slider1;
	QPushButton* reset_button;
	
	int sensitivity;
public:
	Ui_MouseDialog(USING_FLAGS *p, QWidget *parent = NULL);
	~Ui_MouseDialog();
public slots:
	void do_set_value(int val);
	void do_reset_values();
signals:
	int sig_set_value(QString);
	int sig_set_nvalue(int);
};

QT_END_NAMESPACE
