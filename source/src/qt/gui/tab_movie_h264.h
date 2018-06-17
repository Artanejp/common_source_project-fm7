/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#ifndef _CSP_QT_TAB_MOVIE_H264_H
#define _CSP_QT_TAB_MOVIE_H264_H

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

class DLL_PREFIX CSP_TabMovieH264: public QWidget {
	Q_OBJECT;
private:
	
	QLabel *label_title;
	QGridLayout *grid_layout;

	QLabel *label_video_bitrate;
	QComboBox *combo_video_bitrate;
	
	QLabel *label_video_bframes;
	QComboBox *combo_video_bframes;
	
	QLabel *label_video_b_adapt;
	QComboBox *combo_video_b_adapt;
	
	QLabel *label_video_subme;
	QComboBox *combo_video_subme;
	
	QSlider *slider_qmin, *slider_qmax;
	QLabel *label_audio_bitrate;

	QLabel *label_qmin_val;
	QLabel *label_qmax_val;
	QLabel *label_qmin_name;
	QLabel *label_qmax_name;
	QComboBox *combo_audio_bitrate;

	QLabel *label_video_fps;
	QComboBox *combo_video_fps;

	QPushButton *cancel_button;
	QPushButton *close_button;

protected:
	config_t *p_config;
	QWidget *p_wid;
	MOVIE_SAVER *p_movie;
	CSP_DialogMovie *p_window;
	USING_FLAGS *using_flags;
	
	int video_bitrate;
	int video_bframes;
	int video_b_adapt;
	int video_subme;
	int video_minq;
	int video_maxq;
public:
	CSP_TabMovieH264(MOVIE_SAVER *ms, CSP_DialogMovie *parent_window,  USING_FLAGS *p, QWidget *parent = NULL);
	~CSP_TabMovieH264();
	void do_set_codecs(void);
public slots:
	void do_set_video_bitrate(int);
	void do_set_qmin(int n);
	void do_set_qmax(int n);

	void do_set_bframes(int);
	void do_set_b_adapt(int);
	void do_set_subme(int);
signals:
	int sig_set_video_bitrate(int);
	int sig_video_add_option(QString, QString);

};

#endif
