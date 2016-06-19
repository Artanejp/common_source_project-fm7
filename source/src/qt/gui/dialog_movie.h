/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#ifndef _CSP_QT_DIALOG_MOVIE_H
#define _CSP_QT_DIALOG_MOVIE_H

#include <QString>
#include <QStringList>
#include <QSize>
#include <QWidget>

#include "common.h"


QT_BEGIN_NAMESPACE
class QGridLayout;
class QLabel;
class QPushButton;
class QSlider;
class QTabWidget;

class MOVIE_SAVER;
class USING_FLAGS;
class CSP_TabMovieGeneral;
class CSP_TabMovieH264;
class CSP_TabMovieMPEG4;

class DLL_PREFIX CSP_DialogMovie: public QWidget {
	Q_OBJECT;
private:
	QLabel *label_title;
	QGridLayout *grid_layout;
	QTabWidget *tab_widget;


	QPushButton *cancel_button;
	QPushButton *close_button;

	CSP_TabMovieGeneral *tab_general;
	CSP_TabMovieH264 *tab_h264;
	CSP_TabMovieMPEG4 *tab_mpeg4;
protected:
	QWidget *p_wid;
	MOVIE_SAVER *p_movie;
	USING_FLAGS *using_flags;
	
public:
	CSP_DialogMovie(MOVIE_SAVER *ms, USING_FLAGS *p, QWidget *parent = NULL);
	~CSP_DialogMovie();
public slots:
	void do_set_codecs(void);
signals:
	int sig_video_reset_options(void);
	int sig_video_clear_options(void);
	int sig_video_add_option(QString, QString);
};
	QT_END_NAMESPACE
#endif
