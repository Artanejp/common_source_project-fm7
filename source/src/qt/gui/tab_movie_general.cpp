

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
#include "tab_movie_general.h"
#include "../avio/movie_saver.h"
#include "../../config.h"
#include "menu_flags.h"

//extern USING_FLAGS *using_flags;

CSP_TabMovieGeneral::CSP_TabMovieGeneral(MOVIE_SAVER *ms, CSP_DialogMovie *parent_window, std::shared_ptr<USING_FLAGS> p, QWidget *parent) : QWidget(parent)
{
	using_flags = p;
	p_config = p->get_config_ptr();
	p_wid = parent;
	p_movie = ms;
	p_window = parent_window;
	QString tmps;
	grid_layout = new QGridLayout(this);

	label_vcodec = new QLabel(QApplication::translate("MovieTabGeneral", "Video Codec", 0), this);
	combo_vcodec = new QComboBox(this);
	combo_vcodec->addItem(QString::fromUtf8("MPEG4"), VIDEO_CODEC_MPEG4);
	combo_vcodec->addItem(QString::fromUtf8("H.264 (Drop tail frames)"), VIDEO_CODEC_H264);
	combo_vcodec->setToolTip(QApplication::translate("MovieTabGeneral", "MPEG4 will make larger and lower quality file.\nBut very fast.\nH.264 will make smaller and better quality file.\nBut very slowly.\n**Note: Movie file is using MP4 container, not AVI.**", 0));

	for(int i = 0; i < combo_vcodec->count(); i++) {
		int ii = combo_vcodec->itemData(i).toInt();
		if(ii == p_config->video_codec_type) {
			combo_vcodec->setCurrentIndex(ii);
		}
	}
	video_codec_type = p_config->video_codec_type;
	connect(combo_vcodec, SIGNAL(activated(int)), this, SLOT(do_set_video_codec_type(int)));

	label_resolution = new QLabel(QApplication::translate("MovieTabGeneral", "Resolution", 0), this);
	combo_resolution = new QComboBox(this);
	combo_resolution->setToolTip(QApplication::translate("MovieTabGeneral", "Set resolution of encoded movie file.", 0));
	
	geometry.setWidth(p_config->video_width);
	geometry.setHeight(p_config->video_height);
	audio_bitrate = p_config->audio_bitrate;
	audio_codec_type = p_config->audio_codec_type;

	label_video_threads = new QLabel(QApplication::translate("MovieTabGeneral", "Video Threads", 0), this);
	combo_video_threads = new QComboBox(this);
	combo_video_threads->setToolTip(QApplication::translate("MovieTabGeneral", "Set number of threads used by H.264 movie endcoding.", 0));
	combo_audio_bitrate = new QComboBox(this);

	label_audio_bitrate = new QLabel(QApplication::translate("MovieTabGeneral", "Audio Bitrate", 0), this);
	combo_audio_codec = new QComboBox(this);
	label_audio_codec = new QLabel(QApplication::translate("MovieTabGeneral", "Audio Codec", 0), this);
	combo_audio_codec->setToolTip(QApplication::translate("MovieTabGeneral", "Set codec of audio.\nMP3 is using LAME.\nAAC is experimental; using libAV's AAC encoder.", 0));
	label_video_fps = new QLabel(QApplication::translate("MovieTabGeneral", "Framerate", 0), this);
	combo_video_fps = new QComboBox(this);
	video_fps = p_config->video_frame_rate;
	
	// Value for resolution
	//bool skipf = false;
	int x_w[4];
	int y_h[4];
	for(int ii = 1; ii < 5; ii++) {
		x_w[ii - 1] = using_flags->get_screen_width() * ii;
	}
	for(int ii = 1; ii < 5; ii++) {
		y_h[ii - 1] = using_flags->get_screen_height() * ii;
	}

	for(int ii = 0; ii < 4; ii++) {
		if(x_w[ii] < 256) {
			QString tmps_w, tmps_h;
			QString tmps;
			tmps_w.setNum(x_w[ii]);
			tmps_h.setNum(y_h[ii]);
			tmps = tmps_w + QString::fromUtf8("x") + tmps_h;
			combo_resolution->addItem(tmps, QSize(x_w[ii], y_h[ii]));
		}
	}
	combo_resolution->addItem(QString::fromUtf8("256x160"), QSize(256, 160));
	combo_resolution->addItem(QString::fromUtf8("256x240"), QSize(256, 240));

	for(int ii = 0; ii < 4; ii++) {
		if((x_w[ii] < 320) && (x_w[ii] > 256)){
			QString tmps_w, tmps_h;
			QString tmps;
			tmps_w.setNum(x_w[ii]);
			tmps_h.setNum(y_h[ii]);
			tmps = tmps_w + QString::fromUtf8("x") + tmps_h;
			combo_resolution->addItem(tmps, QSize(x_w[ii], y_h[ii]));
		}
	}
	
	combo_resolution->addItem(QString::fromUtf8("320x128"), QSize(320, 128));
	combo_resolution->addItem(QString::fromUtf8("320x200"), QSize(320, 200));
	combo_resolution->addItem(QString::fromUtf8("320x240"), QSize(320, 240));
	
	for(int ii = 0; ii < 4; ii++) {
		if((x_w[ii] < 512) && (x_w[ii] > 320)){
			QString tmps_w, tmps_h;
			QString tmps;
			tmps_w.setNum(x_w[ii]);
			tmps_h.setNum(y_h[ii]);
			tmps = tmps_w + QString::fromUtf8("x") + tmps_h;
			combo_resolution->addItem(tmps, QSize(x_w[ii], y_h[ii]));
		}
	}
	combo_resolution->addItem(QString::fromUtf8("512x400"), QSize(512, 400));
	combo_resolution->addItem(QString::fromUtf8("512x480"), QSize(512, 480));

	for(int ii = 0; ii < 4; ii++) {
		if((x_w[ii] < 640) && (x_w[ii] > 512)){
			QString tmps_w, tmps_h;
			QString tmps;
			tmps_w.setNum(x_w[ii]);
			tmps_h.setNum(y_h[ii]);
			tmps = tmps_w + QString::fromUtf8("x") + tmps_h;
			combo_resolution->addItem(tmps, QSize(x_w[ii], y_h[ii]));
		}
	}
	
	combo_resolution->addItem(QString::fromUtf8("640x200"), QSize(640, 200));
	combo_resolution->addItem(QString::fromUtf8("640x400"), QSize(640, 400));
	combo_resolution->addItem(QString::fromUtf8("640x480"), QSize(640, 480));

	for(int ii = 0; ii < 4; ii++) {
		if((x_w[ii] < 1024) && (x_w[ii] > 640)){
			QString tmps_w, tmps_h;
			QString tmps;
			tmps_w.setNum(x_w[ii]);
			tmps_h.setNum(y_h[ii]);
			tmps = tmps_w + QString::fromUtf8("x") + tmps_h;
			combo_resolution->addItem(tmps, QSize(x_w[ii], y_h[ii]));
		}
	}
	
	combo_resolution->addItem(QString::fromUtf8("1024x768"), QSize(1024, 768));
	
	for(int ii = 0; ii < 4; ii++) {
		if((x_w[ii] < 1280) && (x_w[ii] > 1024)){
			QString tmps_w, tmps_h;
			QString tmps;
			tmps_w.setNum(x_w[ii]);
			tmps_h.setNum(y_h[ii]);
			tmps = tmps_w + QString::fromUtf8("x") + tmps_h;
			combo_resolution->addItem(tmps, QSize(x_w[ii], y_h[ii]));
		}
	}
	combo_resolution->addItem(QString::fromUtf8("1280x800"), QSize(1280, 800));
	combo_resolution->addItem(QString::fromUtf8("1280x960"), QSize(1280, 960));

	for(int ii = 0; ii < 4; ii++) {
		if((x_w[ii] <= 1920) && (x_w[ii] > 1280)){
			QString tmps_w, tmps_h;
			QString tmps;
			tmps_w.setNum(x_w[ii]);
			tmps_h.setNum(y_h[ii]);
			tmps = tmps_w + QString::fromUtf8("x") + tmps_h;
			combo_resolution->addItem(tmps, QSize(x_w[ii], y_h[ii]));
		}
	}
	
	for(int i = 0; i < combo_resolution->count(); i++) {
		QSize s = combo_resolution->itemData(i).toSize();
		if((s.width() == p_config->video_width) && (s.height() == p_config->video_height)) {
			combo_resolution->setCurrentIndex(i);
		}
	}
	connect(combo_resolution, SIGNAL(activated(int)), this, SLOT(do_set_video_resolution(int)));

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
		if(br == p_config->video_threads) {
			combo_video_threads->setCurrentIndex(i);
		}
	}
	video_threads = p_config->video_threads;
	connect(combo_video_threads, SIGNAL(activated(int)), this, SLOT(do_set_video_threads(int)));

	// Audio bitrate
	combo_audio_bitrate->addItem(QString::fromUtf8("32kbps"), 32);
	combo_audio_bitrate->addItem(QString::fromUtf8("48kbps"), 48);
	combo_audio_bitrate->addItem(QString::fromUtf8("64kbps"), 64);
	combo_audio_bitrate->addItem(QString::fromUtf8("128kbps"), 128);
	combo_audio_bitrate->addItem(QString::fromUtf8("160kbps"), 160);
	combo_audio_bitrate->addItem(QString::fromUtf8("192kbps"), 192);
	combo_audio_bitrate->addItem(QString::fromUtf8("224kbps"), 224);
	combo_audio_bitrate->addItem(QString::fromUtf8("256kbps"), 256);
	combo_audio_bitrate->addItem(QString::fromUtf8("320kbps"), 320);
	combo_audio_bitrate->addItem(QString::fromUtf8("384kbps"), 384);
	for(int i = 0; i < combo_audio_bitrate->count(); i++) {
		int br = combo_audio_bitrate->itemData(i).toInt();
		if(br == p_config->audio_bitrate) {
			combo_audio_bitrate->setCurrentIndex(i);
		}
	}
	connect(combo_audio_bitrate, SIGNAL(activated(int)), this, SLOT(do_set_audio_bitrate(int)));

	combo_audio_codec->addItem(QString::fromUtf8("MP3(Lame)"), AUDIO_CODEC_MP3);
	combo_audio_codec->addItem(QString::fromUtf8("AAC"), AUDIO_CODEC_AAC);
	//combo_audio_codec->addItem(QString::fromUtf8("VORBIS(Maybe not working)"), AUDIO_CODEC_VORBIS);
	for(int i = 0; i < combo_audio_codec->count(); i++) {
		int br = combo_audio_codec->itemData(i).toInt();
		if(br == p_config->audio_codec_type) {
			combo_audio_codec->setCurrentIndex(i);
		}
	}
	connect(combo_audio_codec, SIGNAL(activated(int)), this, SLOT(do_set_audio_codec_type(int)));
	
	// Video bitrates
	combo_video_fps->addItem(QString::fromUtf8("15fps"), 15);	
	combo_video_fps->addItem(QString::fromUtf8("24fps"), 24);	
	combo_video_fps->addItem(QString::fromUtf8("30fps"), 30);
	combo_video_fps->addItem(QString::fromUtf8("60fps"), 60); // Temporally disabled
	for(int i = 0; i < combo_video_fps->count(); i++) {
		int fps = combo_video_fps->itemData(i).toInt();
		if(fps == p_config->video_frame_rate) {
			combo_video_fps->setCurrentIndex(i);
		}
	}
	connect(combo_video_fps, SIGNAL(activated(int)), this, SLOT(do_set_video_fps(int)));
	

	grid_layout->addWidget(label_resolution, 1, 0);
	grid_layout->addWidget(combo_resolution, 2, 0);
	grid_layout->addWidget(label_vcodec, 3, 0);
	grid_layout->addWidget(combo_vcodec, 4, 0);

	grid_layout->addWidget(label_audio_codec, 3, 2);
	grid_layout->addWidget(combo_audio_codec, 4, 2);
	
	grid_layout->addWidget(label_audio_bitrate, 5, 2);
	grid_layout->addWidget(combo_audio_bitrate, 5, 3);
	
	grid_layout->addWidget(label_video_fps, 3, 1);
	grid_layout->addWidget(combo_video_fps, 4, 1);
	
	grid_layout->addWidget(label_video_threads, 5, 0);
	grid_layout->addWidget(combo_video_threads, 5, 1);

	this->setLayout(grid_layout);
	
	connect(this, SIGNAL(sig_video_add_option(QString, QString)), p_movie, SLOT(do_add_option(QString, QString)));
	connect(this, SIGNAL(sig_set_audio_bitrate(int)), p_movie, SLOT(do_set_audio_bitrate(int)));
	connect(this, SIGNAL(sig_set_video_resolution(QSize)), p_movie, SLOT(do_set_video_geometry(QSize)));
	
}

CSP_TabMovieGeneral::~CSP_TabMovieGeneral()
{
}

void CSP_TabMovieGeneral::do_set_video_resolution(int n)
{
	QSize s = combo_resolution->itemData(n).toSize();
	int w = s.width();
	int h = s.height();
	if(w < 128) w = 128;
	if(h < 80) h = 80;
	geometry = QSize(w, h);
	
}

void CSP_TabMovieGeneral::do_set_video_fps(int n)
{
	int val = combo_video_fps->itemData(n).toInt();
	if(val < 15) val = 15;
	if(val > 75) val = 75;
	video_fps = val;
}


void CSP_TabMovieGeneral::do_set_video_threads(int n)
{
	int val = combo_video_threads->itemData(n).toInt();
	if(val < 0) val = 0;
	if(val > 12) val = 12;
	video_threads = val;
}

void CSP_TabMovieGeneral::do_set_video_codec_type(int n)
{
	int val = combo_vcodec->itemData(n).toInt();
	if(val < 0) val = 0;
	if(val >= VIDEO_CODEC_END) val = VIDEO_CODEC_END - 1;
	video_codec_type = val;
}

void CSP_TabMovieGeneral::do_set_audio_codec_type(int n)
{
	int val = combo_audio_codec->itemData(n).toInt();
	if(val < 0) return;
	if(val >= AUDIO_CODEC_END) return;
	audio_codec_type = val;
}

void CSP_TabMovieGeneral::do_set_audio_bitrate(int n)
{
	int val = combo_audio_bitrate->itemData(n).toInt();
	if(val < 16) val = 16;
	if(val > 448) val = 448;
	audio_bitrate = val;
}

void CSP_TabMovieGeneral::do_set_codecs(void)
{
	QString value;

	// See:
	// https://libav.org/avconv.html#Video-Options
	p_config->audio_bitrate = audio_bitrate;
	emit sig_set_audio_bitrate(audio_bitrate);

	switch(video_codec_type) {
	case VIDEO_CODEC_MPEG4:
		emit sig_video_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("mpeg4"));
		break;
	case VIDEO_CODEC_H264:
		emit sig_video_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("h264"));
		break;
	}
	p_config->video_codec_type = video_codec_type;

	switch(audio_codec_type) {
	case AUDIO_CODEC_MP3:
		emit sig_video_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("mp3"));
		break;
	case AUDIO_CODEC_AAC:
		emit sig_video_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("aac"));
		break;
	case AUDIO_CODEC_VORBIS:
		emit sig_video_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("vorbis"));
		break;
	}
	p_config->video_codec_type = video_codec_type;
	p_config->audio_codec_type = audio_codec_type;

	p_config->video_threads = video_threads;
	p_config->video_frame_rate = video_fps;

	p_config->video_width = geometry.width();
	p_config->video_height = geometry.height();
	emit sig_set_video_resolution(geometry);

	p_config->video_threads = video_threads;
	p_config->video_frame_rate = video_fps;
}
