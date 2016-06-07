/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#ifndef _CSP_QT_DIALOG_MOVIE_JSBUTTON_H
#define _CSP_QT_DIALOG_MOVIE_JSBUTTON_H

#include <QString>
#include <QStringList>
#include <QSize>
#include <QWidget>

	QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QLabel;
class QComboBox;
class QPushButton;
class QSlider;

class MOVIE_SAVER;

class CSP_DialogMovie: public QWidget {
	Q_OBJECT;
private:
	QSize geometry;
	
	QLabel *label_title;
	QGridLayout *grid_layout;

	QLabel *label_resolution;
	QComboBox *combo_resolution;
	
	QLabel *label_video_bitrate;
	QComboBox *combo_video_bitrate;
	
	QLabel *label_video_bframes;
	QComboBox *combo_video_bframes;
	
	QLabel *label_video_b_adapt;
	QComboBox *combo_video_b_adapt;
	
	QLabel *label_video_subme;
	QComboBox *combo_video_subme;
	
	QLabel *label_video_threads;
	QComboBox *combo_video_threads;

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
	QWidget *p_wid;
	MOVIE_SAVER *p_movie;

	int resolution;
	int video_bitrate;
	int video_bframes;
	int video_b_adapt;
	int video_subme;
	int video_threads;
	int video_minq;
	int video_maxq;
	int audio_bitrate;
	int video_fps;
public:
	CSP_DialogMovie(MOVIE_SAVER *ms, QWidget *parent = NULL);
	~CSP_DialogMovie();
public slots:
	void do_set_video_resolution(int);
	void do_set_video_bitrate(int);
	void do_set_qmin(int n);
	void do_set_qmax(int n);

	void do_set_bframes(int);
	void do_set_b_adapt(int);
	void do_set_subme(int);
	void do_set_video_threads(int);
	void do_set_audio_bitrate(int);
	void do_set_video_fps(int);
	void do_set_codecs();
	
signals:
	int sig_set_video_resolution(QSize);
	int sig_set_video_bitrate(int);
	int sig_video_reset_options(void);
	int sig_video_clear_options(void);
	int sig_video_add_option(QString, QString);
	int sig_set_audio_bitrate(int);
	
};
	QT_END_NAMESPACE
#endif
