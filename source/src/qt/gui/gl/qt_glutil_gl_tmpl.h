/*
 * qt_glutil_gl2_0.h
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL v2.0 .
 * History:
 * Jan 21, 2016 : Initial.
 */

#ifndef _QT_COMMON_GLUTIL_TMPL_H
#define _QT_COMMON_GLUTIL_TMPL_H

#include <QObject>
#include <QVector>
#include <QWidget>
#include <QString>
#include <QList>
#include <QOpenGLTexture>

#include "config.h"
#include "common.h"
#include "qt_glpack.h"
#include "menu_flags.h"

QT_BEGIN_NAMESPACE
class EMU;
class QEvent;
class GLDrawClass;
class QOpenGLFramebufferObject;
class QOpenGLFramebufferObjectFormat;
class USING_FLAGS;
class CSP_Logger;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QOpenGLShaderProgram;
class DLL_PREFIX GLDraw_Tmpl : public QObject
{
	Q_OBJECT
protected:
	EMU *p_emu;
	GLDrawClass *p_wid;
	
	USING_FLAGS *using_flags;
	QImage *imgptr;
	CSP_Logger *csp_logger;
	config_t *p_config;
	
	bool smoosing;
	bool gl_grid_horiz;
	bool gl_grid_vert;
	
	int  vert_lines;
	int  horiz_pixels;
	GLfloat *glVertGrids;
	GLfloat *glHorizGrids;
	float screen_multiply;

	float screen_width;
	float screen_height;
	
	int screen_texture_width;
	int screen_texture_width_old;
	int screen_texture_height;
	int screen_texture_height_old;

	QOpenGLTexture *icon_texid[10][10];

	int rec_count;
	int rec_width;
	int rec_height;

	VertexTexCoord_t vertexFormat[4];
	QOpenGLShaderProgram *main_shader;
	
	QOpenGLVertexArrayObject *vertex_screen;
	QOpenGLBuffer *buffer_screen_vertex;
	
	VertexTexCoord_t vertexBitmap[4];
	QOpenGLShaderProgram *bitmap_shader;
	QOpenGLBuffer *buffer_bitmap_vertex;
	QOpenGLVertexArrayObject *vertex_bitmap;
	QOpenGLVertexArrayObject *vertex_button[128];
	QOpenGLBuffer *buffer_button_vertex[128];
	QOpenGLShaderProgram *button_shader;
	VertexTexCoord_t vertexOSD[32][4];
	QOpenGLVertexArrayObject *vertex_osd[32];
	QOpenGLBuffer *buffer_osd[32];
	QOpenGLShaderProgram *osd_shader;
	const float rot0[4] =   {1, -0,  0, 1};
	const float rot90[4] =  {0,  1, -1,  0};
	const float rot180[4] = {-1, 0,  0, -1};
	const float rot270[4] = {0, -1,  1, 0};

	QOpenGLTexture *uVramTextureID;
	QOpenGLTexture *uButtonTextureID[128];
	GLfloat fButtonX[128];
	GLfloat fButtonY[128];
	GLfloat fButtonWidth[128];
	GLfloat fButtonHeight[128];
	QVector<VertexTexCoord_t> *vertexButtons;
	QVector<QImage> ButtonImages;
	bool button_updated;
	bool set_brightness;
	bool InitVideo;
	
	QOpenGLTexture *uBitmapTextureID;
	bool bitmap_uploaded;
	bool button_drawn;
	bool crt_flag;
	bool redraw_required;


	GLfloat fBrightR;
	GLfloat fBrightG;
	GLfloat fBrightB;
	
	QOpenGLFramebufferObject *offscreen_frame_buffer;
	QOpenGLFramebufferObjectFormat *offscreen_frame_buffer_format;
	QImage offscreen_image;
	GLint texture_max_size;
	
	bool low_resolution_screen;
	bool emu_launched;

	uint32_t osd_led_status;
	uint32_t osd_led_status_bak;
	int osd_led_bit_width;
	bool osd_onoff;

	virtual void drawBitmapTexture(void) { }
	virtual void initButtons(void) { }
	virtual void initBitmapVertex(void) { }
	virtual void initBitmapVAO(void) { }
	virtual void updateButtonTexture(void) { }

	virtual void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
							  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4) { }
	
	virtual void resizeGL_SetVertexs(void) { }
	virtual void resizeGL_Screen(void) { }
	virtual void drawGridsHorizonal(void) { }
	virtual void drawGridsVertical(void) { }
	virtual void drawGridsMain(GLfloat *tp,
					   int number,
					   GLfloat lineWidth = 0.2f,
							   QVector4D color = QVector4D(0.0f, 0.0f, 0.0f, 1.0f)) { }
	virtual void drawButtons() { }
	virtual void drawOsdLeds() { }
	virtual void drawOsdIcons() { }
	virtual void set_osd_vertex(int xbit) { }

public:
	GLDraw_Tmpl(GLDrawClass *parent, USING_FLAGS *p, CSP_Logger *logger, EMU *emu = 0) : QObject((QObject *)parent)
	{
		p_wid = parent;
		using_flags = p;
		p_config = p->get_config_ptr();
		vert_lines = using_flags->get_real_screen_height();
		horiz_pixels = using_flags->get_real_screen_width();
		screen_texture_width = using_flags->get_screen_width();
		screen_texture_width_old = using_flags->get_screen_width();
		screen_texture_height = using_flags->get_screen_height();
		screen_texture_height_old = using_flags->get_screen_height();
		
		if(using_flags->is_use_fd()) {
			osd_led_bit_width = 10;
		}
		if(using_flags->is_use_qd()) {
			osd_led_bit_width = 12;
		}
		if(using_flags->is_use_tape()) {
			osd_led_bit_width = 16;
		}
		if(using_flags->get_max_scsi() > 0) {
			osd_led_bit_width = 24;
		}
		
		int i;
		// Will fix: Must fix setup of vm_buttons[].
		button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
		if(vm_buttons_d != NULL) {
			for(i = 0; i < using_flags->get_max_button(); i++) {
				uButtonTextureID[i] = new QOpenGLTexture(QOpenGLTexture::Target2D);;
				fButtonX[i] = -1.0 + (float)(vm_buttons_d[i].x * 2) / (float)using_flags->get_screen_width();
				fButtonY[i] = 1.0 - (float)(vm_buttons_d[i].y * 2) / (float)using_flags->get_screen_height();
				fButtonWidth[i] = (float)(vm_buttons_d[i].width * 2) / (float)using_flags->get_screen_width();
				fButtonHeight[i] = (float)(vm_buttons_d[i].height * 2) / (float)using_flags->get_screen_height();
			} // end of will fix.
		}
		
		for(int i = 0; i < 10; i++) {
			for(int j = 0; j < 10; j++) {
				icon_texid[i][j] = NULL;
			}
		}
		
		rec_width  = using_flags->get_screen_width();
		rec_height = using_flags->get_screen_height();
		ButtonImages.clear();
		
		csp_logger = logger;
		
		gl_grid_horiz = false;
		gl_grid_vert = false;
		glVertGrids = NULL;
		glHorizGrids = NULL;
		
		set_brightness = false;
		crt_flag = false;
		smoosing = false;
		uVramTextureID = NULL;
		emu_launched = false;
		
		imgptr = NULL;
		screen_multiply = 1.0f;
		redraw_required = false;
		
		osd_led_status = 0x00000000;
		osd_led_status_bak = 0x00000000;
		osd_led_bit_width = 12;

		osd_onoff = true;
		
		uBitmapTextureID = NULL;
		bitmap_uploaded = false;
		texture_max_size = 128;
		low_resolution_screen = false;

		button_updated = false;
		button_drawn = false;
		
		fBrightR = 1.0; // 輝度の初期化
		fBrightG = 1.0;
		fBrightB = 1.0;
		set_brightness = false;
		crt_flag = false;
		smoosing = false;
		
		screen_width = 1.0;
		screen_height = 1.0;

		buffer_screen_vertex = NULL;
		vertex_screen = NULL;
		
		offscreen_frame_buffer = NULL;
		offscreen_frame_buffer_format = NULL;
		rec_count = 0;

	}
	~GLDraw_Tmpl() {}

	virtual void initGLObjects() {}
	virtual void initFBO(void) {}
	virtual void initLocalGLObjects(void) {}
	virtual void initOsdObjects(void) {}

	virtual void uploadMainTexture(QImage *p, bool chromakey, bool was_mapped) {}

	virtual void drawScreenTexture(void) {}
	virtual void drawGrids(void) { }
	virtual void uploadBitmapTexture(QImage *p) {}

	virtual void drawMain(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  VertexTexCoord_t *vertex_data,
						  GLuint texid,
						  QVector4D color, bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f)) {}
	virtual void doSetGridsHorizonal(int lines, bool force)
	{
		int i;
		GLfloat yf;
		GLfloat delta;
	
		if((lines == vert_lines) && !force) return;
		//printf("lines: %d\n", lines);
		vert_lines = lines;
		yf = -screen_height;
		if(vert_lines <= 0) return;
		if(vert_lines > using_flags->get_real_screen_height()) vert_lines = using_flags->get_real_screen_height();
	
		delta = (2.0f * screen_height) / (float)vert_lines;
		yf = yf - delta * 1.0f;
		if(glHorizGrids != NULL) {
			for(i = 0; i < (vert_lines + 1) ; i++) {
				glHorizGrids[i * 6]     = -screen_width; // XBegin
				glHorizGrids[i * 6 + 3] = +screen_width; // XEnd
				glHorizGrids[i * 6 + 1] = yf; // YBegin
				glHorizGrids[i * 6 + 4] = yf; // YEnd
				glHorizGrids[i * 6 + 2] = -0.95f; // ZBegin
				glHorizGrids[i * 6 + 5] = -0.95f; // ZEnd
				yf = yf + delta;
			}
		}
	}
	virtual void doSetGridsVertical(int pixels, bool force) 
	{
		int i;
		GLfloat xf;
		GLfloat delta;
	
		if((pixels == horiz_pixels) && !force) return;
		horiz_pixels = pixels;
		if(horiz_pixels <= 0) return;
		if(horiz_pixels > using_flags->get_real_screen_width()) horiz_pixels = using_flags->get_real_screen_width();
	
		xf = -screen_width;
		delta = (2.0f * screen_width) / (float)horiz_pixels;
		xf = xf - delta * 0.75f;
		if(glVertGrids != NULL) {
			if(horiz_pixels > using_flags->get_real_screen_width()) horiz_pixels = using_flags->get_real_screen_width();
			for(i = 0; i < (horiz_pixels + 1) ; i++) {
				glVertGrids[i * 6]     = xf; // XBegin
				glVertGrids[i * 6 + 3] = xf; // XEnd
				glVertGrids[i * 6 + 1] = -screen_height; // YBegin
				glVertGrids[i * 6 + 4] =  screen_height; // YEnd
				glVertGrids[i * 6 + 2] = -0.95f; // ZBegin
				glVertGrids[i * 6 + 5] = -0.95f; // ZEnd
				xf = xf + delta;
			}
		}
	}
	virtual bool copy_screen_buffer(scrntype_t* target,int w, int h, int stride) { return false;};
	virtual scrntype_t *get_screen_buffer(int y) { return NULL; }
	virtual void get_screen_geometry(int *w, int *h) {
		if(w != NULL) *w = 0;
		if(h != NULL) *h = 0;
	}
	virtual bool is_ready_to_map_vram_texture(void) { return false; }
	virtual bool map_vram_texture(void) { return false; }
	virtual bool unmap_vram_texture(void) { return false; }
public slots:
	virtual void paintGL(void) { }
	virtual void resizeGL(int width, int height) { }
	virtual void initializeGL() { }

	virtual void setBrightness(GLfloat r, GLfloat g, GLfloat b) { }
	virtual void do_set_texture_size(QImage *p, int w, int h) { }
	virtual void do_set_horiz_lines(int lines) { }
	virtual void setImgPtr(QImage *p) { }
	
	virtual void setSmoosing(bool) { }
	virtual void setDrawGLGridVert(bool) { }
	virtual void setDrawGLGridHoriz(bool) { }
	
	virtual void setVirtualVramSize(int ,int) { }	
	virtual void setChangeBrightness(bool) { }
	virtual void do_set_screen_multiply(float mul) { }
	
	virtual void updateBitmap(QImage *) { }
	virtual void paintGL_OffScreen(int count, int w, int h) { }
	virtual void uploadIconTexture(QPixmap *p, int icon_type, int localnum) { }
	
	virtual void set_emu_launched(void) { }
	virtual void do_set_display_osd(bool onoff) { }
	virtual void do_display_osd_leds(int lednum, bool onoff) { }
	virtual void do_set_led_width(int bitwidth) { }
	virtual bool is_mapped_buffer(void) { return false; }
	virtual GLuint get_mapped_buffer_num(int region) { return (GLuint)0; }
signals:
	int sig_push_image_to_movie(int, int, int, QImage *);
};

QT_END_NAMESPACE

#endif // _QT_COMMON_GLUTIL_2_0_H
