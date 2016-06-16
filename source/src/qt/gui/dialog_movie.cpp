
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include <QApplication>

#include "dialog_movie.h"
#include "../avio/movie_saver.h"
#include "../../config.h"

CSP_DialogMovie::CSP_DialogMovie(MOVIE_SAVER *ms, QWidget *parent) : QWidget(parent)
{
	p_wid = parent;
	p_movie = ms;

	QString tmps;
	label_title = new QLabel(QApplication::translate("MainWindow", "Set movie codecs.", 0), this);
	grid_layout = new QGridLayout(this);

	label_resolution = new QLabel(QApplication::translate("MainWindow", "Resolution", 0), this);
	combo_resolution = new QComboBox(this);
	
	geometry.setWidth(config.video_width);
	geometry.setHeight(config.video_height);
	video_maxq = config.video_maxq;
	video_minq = config.video_minq;
	video_bitrate = config.video_bitrate;
	audio_bitrate = config.audio_bitrate;

	label_video_bitrate = new QLabel(QApplication::translate("MainWindow", "Video Bitrate", 0), this);
	combo_video_bitrate = new QComboBox(this);
	label_video_bframes = new QLabel(QApplication::translate("MainWindow", "Max B Frames", 0), this);
	combo_video_bframes = new QComboBox(this);
	label_video_b_adapt = new QLabel(QApplication::translate("MainWindow", "B Adaption", 0), this);
	combo_video_b_adapt = new QComboBox(this);
	label_video_subme = new QLabel(QApplication::translate("MainWindow", "Subpixel motion estimate", 0), this);
	combo_video_subme = new QComboBox(this);
	label_video_threads = new QLabel(QApplication::translate("MainWindow", "Video Threads", 0), this);
	combo_video_threads = new QComboBox(this);
	combo_audio_bitrate = new QComboBox(this);
	cancel_button = new QPushButton(QApplication::translate("MainWindow", "Cancel", 0));
	close_button = new QPushButton(QApplication::translate("MainWindow", "Save Options", 0));
	label_audio_bitrate = new QLabel(QApplication::translate("MainWindow", "Audio Bitrate", 0), this);
	label_video_fps = new QLabel(QApplication::translate("MainWindow", "Framerate", 0), this);
	combo_video_fps = new QComboBox(this);
	video_fps = config.video_frame_rate;
	
	// Value for resolution
	combo_resolution->addItem(QString::fromUtf8("256x160"), QSize(256, 160));
	combo_resolution->addItem(QString::fromUtf8("256x240"), QSize(256, 240));
	combo_resolution->addItem(QString::fromUtf8("320x128"), QSize(320, 128));
	combo_resolution->addItem(QString::fromUtf8("320x200"), QSize(320, 200));
	combo_resolution->addItem(QString::fromUtf8("320x240"), QSize(320, 240));
	combo_resolution->addItem(QString::fromUtf8("512x400"), QSize(512, 400));
	combo_resolution->addItem(QString::fromUtf8("512x480"), QSize(512, 480));
	combo_resolution->addItem(QString::fromUtf8("640x200"), QSize(640, 200));
	combo_resolution->addItem(QString::fromUtf8("640x400"), QSize(640, 400));
	combo_resolution->addItem(QString::fromUtf8("640x480"), QSize(640, 480));
	combo_resolution->addItem(QString::fromUtf8("1024x768"), QSize(1024, 768));
	combo_resolution->addItem(QString::fromUtf8("1280x800"), QSize(1280, 800));
	combo_resolution->addItem(QString::fromUtf8("1280x960"), QSize(1280, 960));
	for(int i = 0; i < combo_resolution->count(); i++) {
		QSize s = combo_resolution->itemData(i).toSize();
		if((s.width() == config.video_width) && (s.height() == config.video_height)) {
			combo_resolution->setCurrentIndex(i);
		}
	}
	connect(combo_resolution, SIGNAL(activated(int)), this, SLOT(do_set_video_resolution(int)));

	// Video bitrates
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
	for(int i = 0; i < combo_video_bitrate->count(); i++) {
		int br = combo_video_bitrate->itemData(i).toInt();
		if(br == config.video_bitrate) {
			combo_video_bitrate->setCurrentIndex(i);
		}
	}
	connect(combo_video_bitrate, SIGNAL(activated(int)), this, SLOT(do_set_video_bitrate(int)));

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
	for(int i = 0; i < combo_video_bframes->count(); i++) {
		int br = combo_video_bframes->itemData(i).toInt();
		if(br == config.video_bframes) {
			combo_video_bframes->setCurrentIndex(i);
		}
	}
	video_bframes = config.video_bframes;
	connect(combo_video_bframes, SIGNAL(activated(int)), this, SLOT(do_set_bframes(int)));

	// B adapt
	combo_video_b_adapt->addItem(QApplication::translate("MainWindow", "None", 0), 0);
	combo_video_b_adapt->addItem(QApplication::translate("MainWindow", "Fast", 0), 1);
	combo_video_b_adapt->addItem(QApplication::translate("MainWindow", "Optimal (Slow with high B-Frames)", 0), 2);
	for(int i = 0; i < combo_video_b_adapt->count(); i++) {
		int br = combo_video_b_adapt->itemData(i).toInt();
		if(br == config.video_b_adapt) {
			combo_video_b_adapt->setCurrentIndex(i);
		}
	}
	video_b_adapt = config.video_b_adapt;
	connect(combo_video_b_adapt, SIGNAL(activated(int)), this, SLOT(do_set_b_adapt(int)));

	slider_qmin = new QSlider(Qt::Horizontal, this);
	//slider_qmin->setMinimum(0);
	//slider_qmin->setMaximum(63);
	slider_qmin->setMinimum(1);
	slider_qmin->setMaximum(31);
	slider_qmin->setValue(config.video_minq);
	label_qmin_val = new QLabel(this);
	tmps.setNum(config.video_minq);
	label_qmin_val->setText(tmps);
	label_qmin_name = new QLabel(QString::fromUtf8("QP Min"), this);
	video_minq = config.video_minq;
	connect(slider_qmin, SIGNAL(valueChanged(int)), this, SLOT(do_set_qmin(int)));
		
	slider_qmax = new QSlider(Qt::Horizontal, this);
	//slider_qmax->setMinimum(0);
	//slider_qmax->setMaximum(63);
	slider_qmax->setMinimum(1);
	slider_qmax->setMaximum(31);
	slider_qmax->setValue(config.video_maxq);
	label_qmax_val = new QLabel(this);
	tmps.setNum(config.video_maxq);
	label_qmax_val->setText(tmps);
	label_qmax_name = new QLabel(QString::fromUtf8("QP Max"), this);
	connect(slider_qmax, SIGNAL(valueChanged(int)), this, SLOT(do_set_qmax(int)));
	video_maxq = config.video_maxq;
	
	// Subme
	combo_video_subme->addItem(QApplication::translate("MainWindow", "RD mode decision for I/P-frames", 0), 6);
	combo_video_subme->addItem(QApplication::translate("MainWindow", "RD mode decision for all frames", 0), 7);
	combo_video_subme->addItem(QApplication::translate("MainWindow", "RD refinement for  I/P-frames", 0), 8);
	combo_video_subme->addItem(QApplication::translate("MainWindow", "RD refinement for all frames", 0), 9);
	combo_video_subme->addItem(QApplication::translate("MainWindow", "QP-RD", 0), 10); // Trellis 2, admode > 0
	combo_video_subme->addItem(QApplication::translate("MainWindow", "Full RD: disable all early terminations", 0), 11);
	for(int i = 0; i < combo_video_subme->count(); i++) {
		int br = combo_video_subme->itemData(i).toInt();
		if(br == config.video_subme) {
			combo_video_subme->setCurrentIndex(i);
		}
	}
	video_subme = config.video_subme;
	connect(combo_video_subme, SIGNAL(activated(int)), this, SLOT(do_set_subme(int)));
		
	// Threads
	combo_video_threads->addItem(QString::fromUtf8("Auto"), 0);
	combo_video_threads->addItem(QString::fromUtf8("1"), 1);
	combo_video_threads->addItem(QString::fromUtf8("2"), 2);
	combo_video_threads->addItem(QString::fromUtf8("3"), 3);
	combo_video_threads->addItem(QString::fromUtf8("4"), 4);
	combo_video_threads->addItem(QString::fromUtf8("5"), 5);
	combo_video_threads->addItem(QString::fromUtf8("6"), 6);
	combo_video_threads->addItem(QString::fromUtf8("7"), 7);
	combo_video_threads->addItem(QString::fromUtf8("8"), 8);
	combo_video_threads->addItem(QString::fromUtf8("9"), 9);
	combo_video_threads->addItem(QString::fromUtf8("10"), 10);
	combo_video_threads->addItem(QString::fromUtf8("11"), 11);
	combo_video_threads->addItem(QString::fromUtf8("12"), 12);
	for(int i = 0; i < combo_video_threads->count(); i++) {
		int br = combo_video_threads->itemData(i).toInt();
		if(br == config.video_threads) {
			combo_video_threads->setCurrentIndex(i);
		}
	}
	video_threads = config.video_threads;
	connect(combo_video_threads, SIGNAL(activated(int)), this, SLOT(do_set_video_threads(int)));

	// Audio bitrate
	combo_audio_bitrate->addItem(QString::fromUtf8("32kbps"), 32);
	combo_audio_bitrate->addItem(QString::fromUtf8("48kbps"), 48);
	combo_audio_bitrate->addItem(QString::fromUtf8("64kbps"), 64);
	combo_audio_bitrate->addItem(QString::fromUtf8("128kbps"), 128);
	combo_audio_bitrate->addItem(QString::fromUtf8("160kbps"), 160);
	combo_audio_bitrate->addItem(QString::fromUtf8("192kbps"), 192);
	for(int i = 0; i < combo_audio_bitrate->count(); i++) {
		int br = combo_audio_bitrate->itemData(i).toInt();
		if(br == config.audio_bitrate) {
			combo_audio_bitrate->setCurrentIndex(i);
		}
	}
	connect(combo_audio_bitrate, SIGNAL(activated(int)), this, SLOT(do_set_audio_bitrate(int)));
	// Video bitrates
	combo_video_fps->addItem(QString::fromUtf8("15fps"), 15);	
	combo_video_fps->addItem(QString::fromUtf8("24fps"), 24);	
	combo_video_fps->addItem(QString::fromUtf8("30fps"), 30);
	combo_video_fps->addItem(QString::fromUtf8("60fps"), 60); // Temporally disabled
	for(int i = 0; i < combo_video_fps->count(); i++) {
		int fps = combo_video_fps->itemData(i).toInt();
		if(fps == config.video_frame_rate) {
			combo_video_fps->setCurrentIndex(i);
		}
	}
	connect(combo_video_fps, SIGNAL(activated(int)), this, SLOT(do_set_video_fps(int)));
	

	grid_layout->addWidget(label_title, 0, 0);
	grid_layout->addWidget(label_resolution, 1, 0);
	grid_layout->addWidget(combo_resolution, 2, 0);

	
	grid_layout->addWidget(label_qmin_name, 3, 0);
	grid_layout->addWidget(label_qmin_val, 3, 3);
	grid_layout->addWidget(slider_qmin, 4, 0, 1, 4);
	grid_layout->addWidget(label_qmax_name, 5, 0);
	grid_layout->addWidget(label_qmax_val, 5, 3);
	grid_layout->addWidget(slider_qmax, 6, 0, 1, 4);
	grid_layout->addWidget(label_video_bitrate, 7, 0);
	grid_layout->addWidget(combo_video_bitrate, 7, 1);
	grid_layout->addWidget(label_audio_bitrate, 7, 2);
	grid_layout->addWidget(combo_audio_bitrate, 7, 3);
	
	grid_layout->addWidget(label_video_fps, 8, 0);
	grid_layout->addWidget(combo_video_fps, 8, 1);
	
	grid_layout->addWidget(label_video_bframes, 9, 0);
	grid_layout->addWidget(combo_video_bframes, 9, 1);
	
	grid_layout->addWidget(label_video_b_adapt, 9, 2);
	grid_layout->addWidget(combo_video_b_adapt, 9, 3);
	
	grid_layout->addWidget(label_video_subme, 10, 0);
	grid_layout->addWidget(combo_video_subme, 10, 1);
	
	grid_layout->addWidget(label_video_threads, 10, 2);
	grid_layout->addWidget(combo_video_threads, 10, 3);

	grid_layout->addWidget(cancel_button, 11, 2);
	grid_layout->addWidget(close_button, 11, 3);

	this->setLayout(grid_layout);
	
	connect(this, SIGNAL(sig_video_add_option(QString, QString)), p_movie, SLOT(do_add_option(QString, QString)));
	connect(this, SIGNAL(sig_video_clear_options()), p_movie, SLOT(do_clear_options_list()));
	connect(this, SIGNAL(sig_set_audio_bitrate(int)), p_movie, SLOT(do_set_audio_bitrate(int)));
	connect(this, SIGNAL(sig_set_video_bitrate(int)), p_movie, SLOT(do_set_video_bitrate(int)));
	connect(this, SIGNAL(sig_set_video_resolution(QSize)), p_movie, SLOT(do_set_video_geometry(QSize)));
	
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(close()));
	connect(close_button, SIGNAL(clicked()), this, SLOT(do_set_codecs()));
	
	this->show();
}

CSP_DialogMovie::~CSP_DialogMovie()
{
}

void CSP_DialogMovie::do_set_video_resolution(int n)
{
	QSize s = combo_resolution->itemData(n).toSize();
	int w = s.width();
	int h = s.height();
	if(w < 128) w = 128;
	if(h < 80) h = 80;
	geometry = QSize(w, h);
	
}

void CSP_DialogMovie::do_set_video_fps(int n)
{
	int val = combo_video_fps->itemData(n).toInt();
	if(val < 15) val = 15;
	if(val > 75) val = 75;
	video_fps = val;
}

void CSP_DialogMovie::do_set_qmin(int n)
{
	if(n < 0) n = 0;
	if(n > 63) n = 63;
	QString tmps;
	video_minq = n;
	tmps.setNum(n);
	label_qmin_val->setText(tmps);
}

void CSP_DialogMovie::do_set_qmax(int n)
{
	if(n < 0) n = 0;
	if(n > 63) n = 63;
	
	QString tmps;
	video_maxq = n;
	tmps.setNum(n);
	label_qmax_val->setText(tmps);
}

void CSP_DialogMovie::do_set_bframes(int n)
{
	int val = combo_video_bframes->itemData(n).toInt();
	if(val < 0) val = 0;
	if(val > 10) val = 10;
	video_bframes = val;
}

void CSP_DialogMovie::do_set_video_bitrate(int n)
{
	int val = combo_video_bitrate->itemData(n).toInt();
	if(val < 64) val = 64;
	video_bitrate = val;
}

void CSP_DialogMovie::do_set_b_adapt(int n)
{
	int val = combo_video_b_adapt->itemData(n).toInt();
	if(val < 0) val = 0;
	if(val > 2) val = 2;
	video_b_adapt = val;
}

void CSP_DialogMovie::do_set_subme(int n)
{
	int val = combo_video_subme->itemData(n).toInt();
	if(val < 4) val = 4;
	if(val > 11) val = 11;
	video_subme = val;
}

void CSP_DialogMovie::do_set_video_threads(int n)
{
	int val = combo_video_threads->itemData(n).toInt();
	if(val < 0) val = 0;
	if(val > 12) val = 12;
	video_threads = val;
}

void CSP_DialogMovie::do_set_audio_bitrate(int n)
{
	int val = combo_audio_bitrate->itemData(n).toInt();
	if(val < 16) val = 16;
	if(val > 448) val = 448;
	audio_bitrate = val;
}

void CSP_DialogMovie::do_set_codecs(void)
{
	QString value;

	// See:
	// https://libav.org/avconv.html#Video-Options
	config.video_bitrate = video_bitrate;
	emit sig_set_video_bitrate(video_bitrate);
	config.audio_bitrate = audio_bitrate;
	emit sig_set_audio_bitrate(audio_bitrate);

	emit sig_video_clear_options();
	emit sig_video_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("mpeg4"));
	emit sig_video_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("aac"));
	//emit sig_video_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("theora"));
	//emit sig_video_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("vorbis"));
	config.video_maxq = video_maxq;
	config.video_minq = video_minq;
	config.video_bframes = video_bframes;
	config.video_b_adapt = video_b_adapt;
	config.video_subme = video_subme;

	value.setNum(video_bframes);
	emit sig_video_add_option(QString::fromUtf8("bf"), value);

	value.setNum(video_b_adapt);
	emit sig_video_add_option(QString::fromUtf8("b_strategy"), value);

	value.setNum(video_subme);
	emit sig_video_add_option(QString::fromUtf8("subq"), value);


	config.video_width = geometry.width();
	config.video_height = geometry.height();
	emit sig_set_video_resolution(geometry);

	config.video_threads = video_threads;
	config.video_frame_rate = video_fps;
	this->close();
}
