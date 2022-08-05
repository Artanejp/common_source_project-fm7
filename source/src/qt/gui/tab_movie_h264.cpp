#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include <QApplication>

#include "tab_movie_h264.h"
#include "dialog_movie.h"
#include "../avio/movie_saver.h"
#include "../../config.h"
#include "menu_flags.h"

CSP_TabMovieH264::CSP_TabMovieH264(MOVIE_SAVER *ms, CSP_DialogMovie *parent_window,  std::shared_ptr<USING_FLAGS> p, QWidget *parent) : QWidget(parent)
{
	QString tmps;
	using_flags = p;
	p_config = p->get_config_ptr();
	p_wid = parent;
	p_movie = ms;
	p_window = parent_window;

	video_maxq = p_config->video_h264_maxq;
	video_minq = p_config->video_h264_minq;

	label_video_bframes = new QLabel(QApplication::translate("MovieTabH264", "Max B Frames", 0), this);
	combo_video_bframes = new QComboBox(this);
	label_video_b_adapt = new QLabel(QApplication::translate("MovieTabH264", "B Adaption", 0), this);
	combo_video_b_adapt = new QComboBox(this);
	label_video_subme = new QLabel(QApplication::translate("MovieTabH264", "Subpixel motion estimate", 0), this);
	combo_video_subme = new QComboBox(this);

	// Video bitrates
	label_video_bitrate = new QLabel(QApplication::translate("MovieTabH264", "Bitrate", 0), this);
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
	combo_video_bitrate->setToolTip(QApplication::translate("MovieTabH264", "Set bitrate of video.\nLarger is better quality, but makes larger file.", 0));

	for(int i = 0; i < combo_video_bitrate->count(); i++) {
		int br = combo_video_bitrate->itemData(i).toInt();
		if(br == p_config->video_h264_bitrate) {
			combo_video_bitrate->setCurrentIndex(i);
		}
	}
	connect(combo_video_bitrate, SIGNAL(activated(int)), this, SLOT(do_set_video_bitrate(int)));
	video_bitrate = p_config->video_h264_bitrate;

	// Video bframes
	combo_video_bframes->addItem(QString::fromUtf8("0"), 0);
	combo_video_bframes->addItem(QString::fromUtf8("1"), 1);
	combo_video_bframes->addItem(QString::fromUtf8("2"), 2);
	combo_video_bframes->addItem(QString::fromUtf8("3"), 3);
	combo_video_bframes->addItem(QString::fromUtf8("4"), 4);
	combo_video_bframes->addItem(QString::fromUtf8("5"), 5);
	combo_video_bframes->addItem(QString::fromUtf8("6"), 6);
	combo_video_bframes->addItem(QString::fromUtf8("7"), 7);
	combo_video_bframes->addItem(QString::fromUtf8("8"), 8);
	combo_video_bframes->setToolTip(QApplication::translate("MovieTabH264", "Max numbers of B FRAMEs.\nLarger value will make smaller file, but slowly.", 0));
	for(int i = 0; i < combo_video_bframes->count(); i++) {
		int br = combo_video_bframes->itemData(i).toInt();
		if(br == p_config->video_h264_bframes) {
			combo_video_bframes->setCurrentIndex(i);
		}
	}
	video_bframes = p_config->video_h264_bframes;
	connect(combo_video_bframes, SIGNAL(activated(int)), this, SLOT(do_set_bframes(int)));

	// B adapt
	combo_video_b_adapt->addItem(QApplication::translate("MovieTabH264", "None", 0), 0);
	combo_video_b_adapt->addItem(QApplication::translate("MovieTabH264", "Fast", 0), 1);
	combo_video_b_adapt->addItem(QApplication::translate("MovieTabH264", "Optimal (Slow with high B-Frames)", 0), 2);
	combo_video_b_adapt->setToolTip(QApplication::translate("MovieTabH264", "Set decision of using B FRAMEs.", 0));
	for(int i = 0; i < combo_video_b_adapt->count(); i++) {
		int br = combo_video_b_adapt->itemData(i).toInt();
		if(br == p_config->video_h264_b_adapt) {
			combo_video_b_adapt->setCurrentIndex(i);
		}
	}
	video_b_adapt = p_config->video_h264_b_adapt;
	connect(combo_video_b_adapt, SIGNAL(activated(int)), this, SLOT(do_set_b_adapt(int)));

	slider_qmin = new QSlider(Qt::Horizontal, this);
	slider_qmin->setMinimum(0);
	slider_qmin->setMaximum(63);
	slider_qmin->setValue(p_config->video_h264_minq);
	label_qmin_val = new QLabel(this);
	tmps.setNum(p_config->video_h264_minq);
	label_qmin_val->setText(tmps);
	label_qmin_name = new QLabel(QString::fromUtf8("QP Min"), this);
	slider_qmin->setToolTip(QApplication::translate("MovieTabH264", "Minimum Quant.\nSmaller value is better quality, but making larger file.\nLarger value is dirty picture, but making smaller file.\n15 to 24 is recommended.", 0));

	video_minq = p_config->video_h264_minq;
	connect(slider_qmin, SIGNAL(valueChanged(int)), this, SLOT(do_set_qmin(int)));
		
	slider_qmax = new QSlider(Qt::Horizontal, this);
	slider_qmax->setMinimum(0);
	slider_qmax->setMaximum(63);
	slider_qmax->setValue(p_config->video_h264_maxq);
	label_qmax_val = new QLabel(this);
	tmps.setNum(p_config->video_h264_maxq);
	label_qmax_val->setText(tmps);
	label_qmax_name = new QLabel(QString::fromUtf8("QP Max"), this);
	slider_qmax->setToolTip(QApplication::translate("MovieTabH264", "Maximum Quant.\nSmaller value is better quality, but making larger file.\nLarger value is dirty picture, but making smaller file.\n20 to 28 is recommended.", 0));
	connect(slider_qmax, SIGNAL(valueChanged(int)), this, SLOT(do_set_qmax(int)));
	video_maxq = p_config->video_h264_maxq;
	
	// Subme
	combo_video_subme->addItem(QApplication::translate("MovieTabH264", "RD mode decision for I/P-frames", 0), 6);
	combo_video_subme->addItem(QApplication::translate("MovieTabH264", "RD mode decision for all frames", 0), 7);
	combo_video_subme->addItem(QApplication::translate("MovieTabH264", "RD refinement for  I/P-frames", 0), 8);
	combo_video_subme->addItem(QApplication::translate("MovieTabH264", "RD refinement for all frames", 0), 9);
	combo_video_subme->addItem(QApplication::translate("MovieTabH264", "QP-RD", 0), 10); // Trellis 2, admode > 0
	combo_video_subme->addItem(QApplication::translate("MovieTabH264", "Full RD: disable all early terminations", 0), 11);
	combo_video_subme->setToolTip(QApplication::translate("MovieTabH264", "Set motion estimation.\nLarger value is better, but slowly.", 0));

	for(int i = 0; i < combo_video_subme->count(); i++) {
		int br = combo_video_subme->itemData(i).toInt();
		if(br == p_config->video_h264_subme) {
			combo_video_subme->setCurrentIndex(i);
		}
	}
	video_subme = p_config->video_h264_subme;
	connect(combo_video_subme, SIGNAL(activated(int)), this, SLOT(do_set_subme(int)));
		
	label_title = new QLabel(QApplication::translate("MovieTabH264", "Set H.264 parameter.", 0), this);
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
	
	
	grid_layout->addWidget(label_video_bframes, 6, 0);
	grid_layout->addWidget(combo_video_bframes, 6, 1);
	
	grid_layout->addWidget(label_video_b_adapt, 6, 2);
	grid_layout->addWidget(combo_video_b_adapt, 6, 3);
	
	grid_layout->addWidget(label_video_subme, 7, 0);
	grid_layout->addWidget(combo_video_subme, 7, 1);
	
	this->setLayout(grid_layout);

	connect(this, SIGNAL(sig_video_add_option(QString, QString)), p_movie, SLOT(do_add_option(QString, QString)));
	connect(this, SIGNAL(sig_set_video_bitrate(int)), p_movie, SLOT(do_set_video_bitrate(int)));
	this->show();
}


CSP_TabMovieH264::~CSP_TabMovieH264()
{
}

void CSP_TabMovieH264::do_set_codecs(void)
{
	QString value;
	// See:
	// https://libav.org/avconv.html#Video-Options
	p_config->video_h264_bitrate = video_bitrate;

	p_config->video_h264_maxq = video_maxq;
	p_config->video_h264_minq = video_minq;
	p_config->video_h264_bframes = video_bframes;
	p_config->video_h264_b_adapt = video_b_adapt;
	p_config->video_h264_subme = video_subme;
}

void CSP_TabMovieH264::do_set_qmin(int n)
{
	if(n < 0) n = 0;
	if(n > 63) n = 63;
	QString tmps;
	video_minq = n;
	tmps.setNum(n);
	label_qmin_val->setText(tmps);
}

void CSP_TabMovieH264::do_set_qmax(int n)
{
	if(n < 0) n = 0;
	if(n > 63) n = 63;
	
	QString tmps;
	video_maxq = n;
	tmps.setNum(n);
	label_qmax_val->setText(tmps);
}

void CSP_TabMovieH264::do_set_bframes(int n)
{
	int val = combo_video_bframes->itemData(n).toInt();
	if(val < 0) val = 0;
	if(val > 10) val = 10;
	video_bframes = val;
}

void CSP_TabMovieH264::do_set_video_bitrate(int n)
{
	int val = combo_video_bitrate->itemData(n).toInt();
	if(val < 64) val = 64;
	video_bitrate = val;
}

void CSP_TabMovieH264::do_set_b_adapt(int n)
{
	int val = combo_video_b_adapt->itemData(n).toInt();
	if(val < 0) val = 0;
	if(val > 2) val = 2;
	video_b_adapt = val;
}

void CSP_TabMovieH264::do_set_subme(int n)
{
	int val = combo_video_subme->itemData(n).toInt();
	if(val < 4) val = 4;
	if(val > 11) val = 11;
	video_subme = val;
}
