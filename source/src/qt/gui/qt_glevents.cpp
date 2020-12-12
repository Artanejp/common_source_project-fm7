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
#include "../osd_base.h"
#include "qt_gldraw.h"
#include "gl2/qt_glutil_gl2_0.h"

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
	double xx, gxx;
	double yy, gyy;
	OSD_BASE *p_osd = using_flags->get_osd();
	int d_ww, d_hh;
	if(using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0)) {
		d_ww = using_flags->get_screen_width();
		d_hh = using_flags->get_screen_height();
	} else {
		if(p_osd != NULL) {
			d_ww = p_osd->get_vm_screen_width();
			d_hh = p_osd->get_vm_screen_height();
		} else {
			d_ww = using_flags->get_screen_width();
			d_hh = using_flags->get_screen_height();
		}
	}
	QPointF pos;
	if(using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0)) {
		pos = event->localPos();
	} else {
		pos = event->screenPos();
	}
	double xpos = (double)(pos.x()) / (double)width();
	double ypos = (double)(pos.y()) / (double)height();
	double gxpos = (double)(event->globalPos().x()) / (double)width();
	double gypos = (double)(event->globalPos().y()) / (double)height();
	//printf("@@ %d %d\n", pos.x(), pos.y());
	if(using_flags->is_use_one_board_computer() || using_flags->is_use_mouse() || (using_flags->get_max_button() > 0)) {
		if(!enable_mouse) return;
		//if(QApplication::overrideCursor() == NULL) {
			//QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
		//}
		switch(p_config->rotate_type) {
			case 0:
				xx = xpos * (double)d_ww;
				yy = ypos * (double)d_hh;
				gxx = gxpos * (double)d_ww;
				gyy = gypos * (double)d_hh;
				break;
			case 2:
				xx = (1.0 - xpos) * (double)d_ww;
				yy = (1.0 - ypos) * (double)d_hh;
				gxx = (1.0 - gxpos) * (double)d_ww;
				gyy = (1.0 - gypos) * (double)d_hh;
				break;
			case 1:
				xx = ypos * (double)d_ww;
				yy = (1.0 - xpos) * (double)d_hh;
				gxx = gypos * (double)d_ww;
				gyy = (1.0 - gxpos) * (double)d_hh;
				break;
			case 3:
				xx = (1.0 - ypos) * (double)d_ww;
				yy = xpos * (double)d_hh;
				gxx = (1.0 - gypos) * (double)d_ww;
				gyy = gxpos * (double)d_hh;
				break;
			default:
				xx = xpos * (double)d_ww;
				yy = ypos * (double)d_hh;
				gxx = gxpos * (double)d_ww;
				gyy = gypos * (double)d_hh;
				break;
		}
	} else {
		xx = xpos * (double)d_ww;
		yy = ypos * (double)d_hh;
		gxx = gxpos * (double)d_ww;
		gyy = gypos * (double)d_hh;
	}

	//csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Mouse Move: (%f,%f) -> (%d, %d)\n", xpos, ypos, (int)xx, (int)yy);
	emit sig_notify_move_mouse(xx, yy, gxx, gyy);

}
// Will fix. zap of emu-> or ??
void GLDrawClass::mousePressEvent(QMouseEvent *event)
{
	if(using_flags->is_use_one_board_computer() || using_flags->is_use_mouse() || (using_flags->get_max_button() > 0)) {
		if(event->button() == Qt::MiddleButton)	{
			emit sig_check_grab_mouse(true);
			return;
		}
		if(!enable_mouse) return;
		mouseMoveEvent(event); // Update pointer's location. 20180514
		emit do_notify_button_pressed(event->button());
	}
}

void GLDrawClass::mouseReleaseEvent(QMouseEvent *event)
{
	if(using_flags->is_use_one_board_computer() || using_flags->is_use_mouse() || (using_flags->get_max_button() > 0)) {
		if(!enable_mouse) return;
		mouseMoveEvent(event); // Update pointer's location. 20180514
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

