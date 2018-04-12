/*
 * qt_events.cpp
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */



#include <QtGui>
#include <QMouseEvent>
#include <QApplication>

#include "common.h"
//#include "emu.h"
#include "osd_types.h"
#include "qt_gldraw.h"
#include "qt_glutil_gl2_0.h"

#include <QEvent>
#include <QDateTime>

//extern USING_FLAGS *using_flags;

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
	if(using_flags->is_use_one_board_computer() || using_flags->is_use_mouse()) {
		enable_mouse = enable;
	} else {
		enable_mouse = false;
	}
}

void GLDrawClass::mouseMoveEvent(QMouseEvent *event)
{
	if(using_flags->is_use_one_board_computer() || using_flags->is_use_mouse()) {
		if(!enable_mouse) return;
		if(QApplication::overrideCursor() == NULL) {
			QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
		}
		int d_ww = using_flags->get_screen_width();
		int d_hh = using_flags->get_screen_height();
		double xx;
		double yy;
		QPointF pos = event->localPos();
		double xpos = (double)(pos.x()) / (double)width();
		double ypos = (double)(pos.y()) / (double)height();
		if(using_flags->is_use_screen_rotate()) {
			if(using_flags->get_config_ptr()->rotate_type) {
				xx = ypos * (double)d_hh;
				yy = xpos * (double)d_ww;
			} else 	{
				xx = xpos * (double)d_ww;
				yy = ypos * (double)d_hh;
			}
		} else {
			xx = xpos * (double)d_ww;
			yy = ypos * (double)d_hh;
		}

		//printf("Mouse Move: (%f,%f) -> (%d, %d)\n", xpos, ypos, (int)xx, (int)yy);
		emit do_notify_move_mouse((int)xx, (int)yy);
	}
}
// Will fix. zap of emu-> or ??
void GLDrawClass::mousePressEvent(QMouseEvent *event)
{
	if(using_flags->is_use_one_board_computer() || using_flags->is_use_mouse()) {
		if(event->button() == Qt::MiddleButton)	{
			emit sig_check_grab_mouse(true);
			return;
		}
		if(!enable_mouse) return;
		emit do_notify_button_pressed(event->button());
	}
}

void GLDrawClass::mouseReleaseEvent(QMouseEvent *event)
{
	if(using_flags->is_use_one_board_computer() || using_flags->is_use_mouse()) {
		if(!enable_mouse) return;
		emit do_notify_button_released(event->button());
	}
}

void GLDrawClass::closeEvent(QCloseEvent *event)
{
	//emit sig_finished();
}

void GLDrawClass::do_set_mouse_enabled(bool flag)
{
	enable_mouse = flag;
}

void GLDrawClass::do_save_frame_screen(void)
{
	if(!save_pixmap_req) {
		save_pixmap_req = true;
		QDateTime nowTime = QDateTime::currentDateTime();
		QString tmps = QString::fromUtf8("Screen_Save_emu");
		tmps = tmps + using_flags->get_config_name();
		tmps = tmps + QString::fromUtf8("_");
		tmps = tmps + nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz"));
		tmps = tmps + QString::fromUtf8(".png");
		filename_screen_pixmap = QString::fromLocal8Bit(get_application_path()) + tmps;
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
	vram_width = w;
	vram_height = h;
	if(extfunc != NULL) extfunc->do_set_texture_size(p, w, h);
}

void GLDrawClass::do_set_horiz_lines(int lines)
{
	if(extfunc != NULL) extfunc->do_set_horiz_lines(lines);
}

