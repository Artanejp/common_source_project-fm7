
#include <QGridLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QTabWidget>

#include "dialog_movie.h"
#include "tab_movie_general.h"
#include "tab_movie_h264.h"
#include "tab_movie_mpeg4.h"

#include "../avio/movie_saver.h"
#include "../../config.h"

CSP_DialogMovie::CSP_DialogMovie(MOVIE_SAVER *ms, std::shared_ptr<USING_FLAGS> p, QWidget *parent) : QWidget(parent)
{
	p_wid = parent;
	p_movie = ms;
	using_flags = p;
	tab_widget = new QTabWidget(this);

	tab_general = new CSP_TabMovieGeneral(ms, this, using_flags, parent);
	tab_h264 = new CSP_TabMovieH264(ms, this, using_flags, parent);
	tab_mpeg4 = new CSP_TabMovieMPEG4(ms, this, using_flags, parent);

	tab_widget->addTab(tab_general, QApplication::translate("MovieDialog", "General", 0));
	tab_widget->addTab(tab_h264, QApplication::translate("MovieDialog", "H.264", 0));
	tab_widget->addTab(tab_mpeg4, QApplication::translate("MovieDialog", "MPEG4v1", 0));
	
	label_title = new QLabel(QApplication::translate("MovieDialog", "Set movie codecs.", 0), this);
	grid_layout = new QGridLayout(this);
	
	cancel_button = new QPushButton(QApplication::translate("MovieDialog", "Cancel", 0));
	close_button = new QPushButton(QApplication::translate("MovieDialog", "Save Options", 0));

	grid_layout->addWidget(label_title, 0, 0, 1, 2);
	grid_layout->addWidget(tab_widget, 1, 0, 8, 4);
	grid_layout->addWidget(cancel_button, 9, 2);
	grid_layout->addWidget(close_button, 9, 3);
	this->setLayout(grid_layout);

	connect(cancel_button, SIGNAL(clicked()), this, SLOT(close()));
	connect(close_button, SIGNAL(clicked()), this, SLOT(do_set_codecs()));
	
	this->show();
	//this->exec();
}

CSP_DialogMovie::~CSP_DialogMovie()
{
}

void CSP_DialogMovie::do_set_codecs(void)
{
	QString value;

	// See:
	// https://libav.org/avconv.html#Video-Options
	emit sig_video_clear_options();
	if(tab_general != NULL) tab_general->do_set_codecs();
	if(tab_h264 != NULL) tab_h264->do_set_codecs();
	if(tab_mpeg4 != NULL) tab_mpeg4->do_set_codecs();
	
	this->close();
}

