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
#if defined(_WINDOWS) || defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
#include <GL/gl.h>
#include <GL/glext.h>
#else
#include <GL/glx.h>
#include <GL/glxext.h>
#endif
#include <GL/glu.h>
#include "qt_gldraw.h"
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
	//if(draw_width >= this->width()) {
		d_ww = this->width();
		c_ww = this->width();
		//} else {
		//d_ww = draw_width;
		//c_ww = this->width();
		//}
		//if(draw_height >= this->height()) {
		d_hh = this->height();
		c_hh = this->height();
		//} else {
		//d_hh = draw_height;
		//c_hh = this->height();
		//}
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
	screen_multiply = mul;
	//printf("multiply %f\n", mul);
	do_set_texture_size(imgptr, screen_texture_width, screen_texture_height);
}

void GLDrawClass::do_set_texture_size(QImage *p, int w, int h)
{
	if(w <= 0) w = SCREEN_WIDTH;
	if(h <= 0) h = SCREEN_HEIGHT;
	float wfactor = 1.0f;
	float hfactor = 1.0f;
	float iw, ih;
	imgptr = p;
	if(p != NULL) {
		iw = (float)p->width();
		ih = (float)p->height();
	} else {
		iw = (float)SCREEN_WIDTH;
		ih = (float)SCREEN_HEIGHT;
	}
	if(p != NULL) {
		int ww = w;
		int hh = h;
		//if(screen_multiply < 1.0f) {
		if((w > this->width()) || (h > this->height())) {
			ww = (int)(screen_multiply * (float)w);
			hh = (int)(screen_multiply * (float)h);
			wfactor = screen_multiply * 2.0f - 1.0f;
			hfactor = -screen_multiply * 2.0f + 1.0f;
		}
		screen_texture_width = w;
		screen_texture_height = h;
		this->makeCurrent();
		{
			vertexTmpTexture[0].x = -1.0f;
			vertexTmpTexture[0].y = -1.0f;
			vertexTmpTexture[0].z = -0.1f;
			vertexTmpTexture[0].s = 0.0f;
			vertexTmpTexture[0].t = 0.0f;
		
			vertexTmpTexture[1].x = wfactor;
			vertexTmpTexture[1].y = -1.0f;
			vertexTmpTexture[1].z = -0.1f;
			vertexTmpTexture[1].s = (float)w / iw;
			vertexTmpTexture[1].t = 0.0f;
		
			vertexTmpTexture[2].x = wfactor;
			vertexTmpTexture[2].y = hfactor;
			vertexTmpTexture[2].z = -0.1f;
			vertexTmpTexture[2].s = (float)w / iw;
			vertexTmpTexture[2].t = (float)h / ih;
		
			vertexTmpTexture[3].x = -1.0f;
			vertexTmpTexture[3].y = hfactor;
			vertexTmpTexture[3].z = -0.1f;
			vertexTmpTexture[3].s = 0.0f;
			vertexTmpTexture[3].t = (float)h / ih;
			setNormalVAO(tmp_shader, vertex_tmp_texture,
					 buffer_vertex_tmp_texture,
					 vertexTmpTexture, 4);
		}
		{
			this->deleteTexture(uVramTextureID);
			uVramTextureID = this->bindTexture(*p);
		}
		vertexFormat[0].s = 0.0f;
		vertexFormat[0].t = (float)hh / ih;
		vertexFormat[1].s = (float)ww / iw;
		vertexFormat[1].t = (float)hh / ih;
		vertexFormat[2].s = (float)ww / iw;
		vertexFormat[2].t = 0.0f;
		vertexFormat[3].s = 0.0f;
		vertexFormat[3].t = 0.0f;
		
		setNormalVAO(main_shader, vertex_screen,
					 buffer_screen_vertex,
					 vertexFormat, 4);
		this->doSetGridsHorizonal(h, true);
		this->doSetGridsVertical(w, true);
		this->doneCurrent();
	}
}

