/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#ifndef _CSP_QT_TAB_MOVIE_GENERAL_H
#define _CSP_QT_TAB_MOVIE_GENERAL_H

#include <QString>
#include <QStringList>
#include <QSize>
#include <QWidget>
#include "config.h"
#include "common.h"

class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QLabel;
class QComboBox;
class QPushButton;
class QSlider;

class MOVIE_SAVER;
class CSP_DialogMovie;
class USING_FLAGS;

class DLL_PREFIX CSP_TabMovieGeneral: public QWidget {
	Q_OBJECT;
private:
	QSize geometry;
	
	QGridLayout *grid_layout;

	QLabel *label_vcodec;
	QComboBox *combo_vcodec;
	
	QLabel *label_resolution;
	QComboBox *combo_resolution;
	
	QLabel *label_video_threads;
	QComboBox *combo_video_threads;

	QLabel *label_audio_codec;
	QComboBox *combo_audio_codec;
	
	QLabel *label_audio_bitrate;
	QComboBox *combo_audio_bitrate;

	QLabel *label_video_fps;
	QComboBox *combo_video_fps;

protected:
	config_t *p_config;
	std::shared_ptr<USING_FLAGS> using_flags;
	QWidget *p_wid;
	MOVIE_SAVER *p_movie;
	CSP_DialogMovie *p_window;
	
	int resolution;
	int video_codec_type;
	int audio_codec_type;
	int video_threads;
	int audio_bitrate;
	int video_fps;
public:
	CSP_TabMovieGeneral(MOVIE_SAVER *ms, CSP_DialogMovie *parent_window, std::shared_ptr<USING_FLAGS> p, QWidget *parent = NULL);
	~CSP_TabMovieGeneral();
public slots:
	void do_set_video_codec_type(int);
	void do_set_audio_codec_type(int);
	void do_set_video_resolution(int);
	void do_set_video_threads(int);
	void do_set_audio_bitrate(int);
	void do_set_video_fps(int);
	void do_set_codecs();
signals:
	int sig_video_add_option(QString, QString);
	int sig_set_video_resolution(QSize);
	int sig_set_audio_bitrate(int);
};

#endif
