#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include <QApplication>

#include "tab_movie_mpeg4.h"
#include "dialog_movie.h"
#include "../avio/movie_saver.h"
#include "../../config.h"
#include "menu_flags.h"

CSP_TabMovieMPEG4::CSP_TabMovieMPEG4(MOVIE_SAVER *ms, CSP_DialogMovie *parent_window,  USING_FLAGS *p, QWidget *parent) : QWidget(parent)
{
	QString tmps;
	using_flags = p;
	p_wid = parent;
	p_movie = ms;
	p_window = parent_window;

	video_maxq = using_flags->get_config_ptr()->video_mpeg4_maxq;
	video_minq = using_flags->get_config_ptr()->video_mpeg4_minq;
	if(video_maxq < video_minq) {
		int n = video_maxq;
		video_maxq = video_minq;
		video_minq = n;
	}
	if(video_maxq < 1) video_maxq = 1;
	if(video_maxq > 31) video_maxq = 31;
	if(video_minq < 1) video_minq = 1;
	if(video_minq > 31) video_minq = 31;

	label_video_bframes = new QLabel(QApplication::translate("MovieTabMPEG4", "Max B Frames", 0), this);
	combo_video_bframes = new QComboBox(this);
	// Video bitrates
	label_video_bitrate = new QLabel(QApplication::translate("MovieTabMPEG4", "Bitrate", 0), this);
	combo_video_bitrate = new QComboBox(this);
	combo_video_bitrate->addItem(QString::fromUtf8("128Kbps"), 128);	
	combo_video_bitrate->addItem(QString::fromUtf8("256Kbps"), 256);
	combo_video_bitrate->addItem(QString::fromUtf8("300Kbps"), 300);
	combo_video_bitrate->addItem(QString::fromUtf8("512Kbps"), 512);
	combo_video_bitrate->addItem(QString::fromUtf8("600Kbps"), 600);
	combo_video_bitrate->addItem(QString::fromUtf8("768Kbps"), 768);
	combo_video_bitrate->addItem(QString::fromUtf8("900Kbps"), 900);
	combo_video_bitrate->addItem(QString::fromUtf8("1000Kbps"), 1000);
	combo_video_bitrate->addItem(QString::fromUtf8("1200Kbps"), 1200);
	combo_video_bitrate->addItem(QString::fromUtf8("1500Kbps"), 1500);
	combo_video_bitrate->addItem(QString::fromUtf8("1800Kbps"), 1800);
	combo_video_bitrate->addItem(QString::fromUtf8("3000Kbps"), 3000);
	combo_video_bitrate->addItem(QString::fromUtf8("4500Kbps"), 4500);
	combo_video_bitrate->addItem(QString::fromUtf8("5000Kbps"), 5000);
	combo_video_bitrate->addItem(QString::fromUtf8("7500Kbps"), 7500);
	combo_video_bitrate->addItem(QString::fromUtf8("9000Kbps"), 9000);
	combo_video_bitrate->addItem(QString::fromUtf8("10000Kbps"), 10000);
	combo_video_bitrate->addItem(QString::fromUtf8("15000Kbps"), 15000);
	combo_video_bitrate->addItem(QString::fromUtf8("20000Kbps"), 20000);
	for(int i = 0; i < combo_video_bitrate->count(); i++) {
		int br = combo_video_bitrate->itemData(i).toInt();
		if(br == using_flags->get_config_ptr()->video_mpeg4_bitrate) {
			combo_video_bitrate->setCurrentIndex(i);
		}
	}
	connect(combo_video_bitrate, SIGNAL(activated(int)), this, SLOT(do_set_video_bitrate(int)));
	video_bitrate = using_flags->get_config_ptr()->video_mpeg4_bitrate;

	// Video bframes
	combo_video_bframes->addItem(QString::fromUtf8("1"), 1);
	combo_video_bframes->addItem(QString::fromUtf8("2"), 2);
	combo_video_bframes->addItem(QString::fromUtf8("3"), 3);
	combo_video_bframes->addItem(QString::fromUtf8("4"), 4);
	combo_video_bframes->addItem(QString::fromUtf8("5"), 5);
	combo_video_bframes->addItem(QString::fromUtf8("6"), 6);
	combo_video_bframes->addItem(QString::fromUtf8("7"), 7);
	combo_video_bframes->addItem(QString::fromUtf8("8"), 8);
	for(int i = 0; i < combo_video_bframes->count(); i++) {
		int br = combo_video_bframes->itemData(i).toInt();
		if(br == using_flags->get_config_ptr()->video_mpeg4_bframes) {
			combo_video_bframes->setCurrentIndex(i);
		}
	}
	video_bframes = using_flags->get_config_ptr()->video_mpeg4_bframes;
	connect(combo_video_bframes, SIGNAL(activated(int)), this, SLOT(do_set_bframes(int)));


	slider_qmin = new QSlider(Qt::Horizontal, this);
	slider_qmin->setMinimum(1);
	slider_qmin->setMaximum(31);
	slider_qmin->setValue(using_flags->get_config_ptr()->video_mpeg4_minq);
	label_qmin_val = new QLabel(this);
	tmps.setNum(using_flags->get_config_ptr()->video_mpeg4_minq);
	label_qmin_val->setText(tmps);
	label_qmin_name = new QLabel(QString::fromUtf8("QP Min"), this);
	video_minq = using_flags->get_config_ptr()->video_mpeg4_minq;
	connect(slider_qmin, SIGNAL(valueChanged(int)), this, SLOT(do_set_qmin(int)));
		
	slider_qmax = new QSlider(Qt::Horizontal, this);
	slider_qmax->setMinimum(1);
	slider_qmax->setMaximum(31);
	slider_qmax->setValue(using_flags->get_config_ptr()->video_mpeg4_maxq);
	label_qmax_val = new QLabel(this);
	tmps.setNum(using_flags->get_config_ptr()->video_mpeg4_maxq);
	label_qmax_val->setText(tmps);
	label_qmax_name = new QLabel(QString::fromUtf8("QP Max"), this);
	connect(slider_qmax, SIGNAL(valueChanged(int)), this, SLOT(do_set_qmax(int)));
	video_maxq = using_flags->get_config_ptr()->video_mpeg4_maxq;
	
	label_title = new QLabel(QApplication::translate("MovieTabMPEG4", "Set MPEG4v1 parameter.", 0), this);
	grid_layout = new QGridLayout(this);

	grid_layout->addWidget(label_title, 0, 0);
	
	grid_layout->addWidget(label_qmin_name, 1, 0);
	grid_layout->addWidget(label_qmin_val, 1, 3);
	grid_layout->addWidget(slider_qmin, 2, 0, 1, 4);
	grid_layout->addWidget(label_qmax_name, 3, 0);
	grid_layout->addWidget(label_qmax_val, 3, 3);
	grid_layout->addWidget(slider_qmax, 4, 0, 1, 4);
	grid_layout->addWidget(label_video_bitrate, 5, 0);
	grid_layout->addWidget(combo_video_bitrate, 5, 1);
	
	grid_layout->addWidget(label_video_bframes, 5, 2);
	grid_layout->addWidget(combo_video_bframes, 5, 3);
	
	this->setLayout(grid_layout);

	connect(this, SIGNAL(sig_video_add_option(QString, QString)), p_movie, SLOT(do_add_option(QString, QString)));
	connect(this, SIGNAL(sig_set_video_bitrate(int)), p_movie, SLOT(do_set_video_bitrate(int)));
	this->show();
}


CSP_TabMovieMPEG4::~CSP_TabMovieMPEG4()
{
}

void CSP_TabMovieMPEG4::do_set_codecs(void)
{
	QString value;
	// See:
	// https://libav.org/avconv.html#Video-Options
	using_flags->get_config_ptr()->video_mpeg4_bitrate = video_bitrate;

	using_flags->get_config_ptr()->video_mpeg4_maxq = video_maxq;
	using_flags->get_config_ptr()->video_mpeg4_minq = video_minq;
	using_flags->get_config_ptr()->video_mpeg4_bframes = video_bframes;
}

void CSP_TabMovieMPEG4::do_set_qmin(int n)
{
	if(n < 1) n = 1;
	if(n > 31) n = 31;
	QString tmps;
	video_minq = n;
	tmps.setNum(n);
	label_qmin_val->setText(tmps);
}

void CSP_TabMovieMPEG4::do_set_qmax(int n)
{
	if(n < 1) n = 1;
	if(n > 31) n = 31;
	
	QString tmps;
	video_maxq = n;
	tmps.setNum(n);
	label_qmax_val->setText(tmps);
}

void CSP_TabMovieMPEG4::do_set_bframes(int n)
{
	int val = combo_video_bframes->itemData(n).toInt();
	if(val < 1) val = 1;
	if(val > 10) val = 10;
	video_bframes = val;
}

void CSP_TabMovieMPEG4::do_set_video_bitrate(int n)
{
	int val = combo_video_bitrate->itemData(n).toInt();
	if(val < 64) val = 64;
	video_bitrate = val;
}

