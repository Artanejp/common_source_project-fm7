
#pragma once

#include <QWidget>

class QGridLayout;
class QLabel;
class QSpinBox;
class QLabel;
class QPushButton;

#include "../../config.h"
class USING_FLAGS;

QT_BEGIN_NAMESPACE
class CSP_MemoryDialog : public QWidget {
	Q_OBJECT
protected:
	config_t* p_config;
	USING_FLAGS* using_flags;
	QWidget *p_wid;
	
	int _min;
	int _max;
	int current_val;
	uint32_t order;

	QGridLayout *layout;
	QLabel *label_head;
	QSpinBox *spin_ram;
	QLabel *order_label;
	
	QPushButton *reset_button;
	QPushButton *cancel_button;
	QPushButton *close_button;
	
public:
	CSP_MemoryDialog(USING_FLAGS *p, QWidget *parent);
	~CSP_MemoryDialog();

public slots:
	void do_set_value();
	void do_reset_value();
};
QT_END_NAMESPACE
