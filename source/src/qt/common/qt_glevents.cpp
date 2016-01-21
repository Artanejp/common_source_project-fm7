/*
 * qt_events.cpp
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */


#include "emu.h"

#include <QtGui>
#include <QMouseEvent>
#include <QApplication>

#include "common.h"
#include "emu.h"
#include "osd.h"
#include "qt_gldraw.h"
#include "qt_glutil_gl2_0.h"

#include <QEvent>
#include <QDateTime>

void GLDrawClass::enterEvent(QEvent *event)
{
	this->grabKeyboard();
}

void GLDrawClass::leaveEvent(QEvent *event)
{
	this->releaseKeyboard();
}

void GLDrawClass::setEnableMouse(bool enable)
{
#if defined(ONE_BOARD_MICRO_COMPUTER) || defined(USE_MOUSE)
	enable_mouse = enable;
#else
	enable_mouse = false;
#endif	
}

#if defined(ONE_BOARD_MICRO_COMPUTER) || defined(USE_MOUSE)
void GLDrawClass::mouseMoveEvent(QMouseEvent *event)
{
	int xpos = event->x();
	int ypos = event->y();
	int d_ww, d_hh;
	int c_ww, c_hh;

	if(!enable_mouse) return;
	emit do_notify_move_mouse(xpos, ypos);
}

void GLDrawClass::mousePressEvent(QMouseEvent *event)
{
	int xpos = event->x();
	int ypos = event->y();
	int d_ww, d_hh;
	int c_ww, c_hh;

	if((xpos < 0) || (ypos < 0)) return;
	d_ww = this->width();
	c_ww = this->width();
	d_hh = this->height();
	c_hh = this->height();
		
	int left = (c_ww - d_ww) / 2;
	int right = (c_ww - d_ww) / 2 + d_ww;
	int up = (c_hh - d_hh) / 2;
	int down = (c_hh - d_hh) / 2 + d_hh;

	if((xpos < left) || (xpos >= right)) {
		if(QApplication::overrideCursor() != NULL) QApplication::restoreOverrideCursor();
		return;
	}
	if((ypos < up) || (ypos >= down)) {
		if(QApplication::overrideCursor() != NULL) QApplication::restoreOverrideCursor();
		return;
	}
	if(QApplication::overrideCursor() == NULL) {
		if(p_emu != NULL) {
			if(p_emu->get_mouse_enabled()) {
				QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
			}
		}
	}
	xpos = xpos - left;
	ypos = ypos - up;
	double xx;
	double yy;
# if defined(USE_SCREEN_ROTATE)
	if(config.rotate_type) {
		xx = (double)ypos * ((double)SCREEN_WIDTH / (double)d_hh);
		yy = (double)xpos * ((double)SCREEN_HEIGHT / (double)d_ww);
	} else
# endif	  
	{
		xx = (double)xpos * ((double)SCREEN_WIDTH / (double)d_ww);
		yy = (double)ypos * ((double)SCREEN_HEIGHT / (double)d_hh);
	}
	emit do_notify_move_mouse((int)xx, (int) yy);
	if(!enable_mouse) return;
	emit do_notify_button_pressed(event->button());
	if(event->button() == Qt::MiddleButton)	emit sig_check_grab_mouse(true);
}

void GLDrawClass::mouseReleaseEvent(QMouseEvent *event)
{
	if(!enable_mouse) return;
	emit do_notify_button_released(event->button());
}
#endif	

void GLDrawClass::closeEvent(QCloseEvent *event)
{
	//emit sig_finished();
}

void GLDrawClass::do_save_frame_screen(void)
{
	if(!save_pixmap_req) {
		save_pixmap_req = true;
		QDateTime nowTime = QDateTime::currentDateTime();
		QString tmps = QString::fromUtf8("Screen_Save_emu");
		tmps = tmps + QString::fromUtf8(CONFIG_NAME);
		tmps = tmps + QString::fromUtf8("_");
		tmps = tmps + nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz"));
		tmps = tmps + QString::fromUtf8(".png");
		filename_screen_pixmap = QString::fromLocal8Bit(application_path()) + tmps;
	}
}

void GLDrawClass::do_save_frame_screen(const char *name)
{
	if(!save_pixmap_req) {
		save_pixmap_req = true;
		filename_screen_pixmap = QString::fromUtf8(name);
	}
}

void GLDrawClass::do_set_screen_multiply(float mul)
{
	if(extfunc != NULL) extfunc->do_set_screen_multiply(mul);
}

void GLDrawClass::do_set_texture_size(QImage *p, int w, int h)
{
	if(extfunc != NULL) extfunc->do_set_texture_size(p, w, h);
}

