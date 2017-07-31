/*
 * qt_glutil_gl2_0.cpp
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL v2.0 .
 * History:
 * Jan 21, 2016 : Initial.
 */

//#include "emu.h"

#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObject>
#include <QColor>
#include <QImageReader>

#include "qt_gldraw.h"
//#include "csp_logger.h"
#include "qt_glutil_gl2_0.h"
#include "menu_flags.h"

GLDraw_2_0::GLDraw_2_0(GLDrawClass *parent, USING_FLAGS *p, CSP_Logger *logger, EMU *emu) : QObject(parent)
{
	p_wid = parent;
	using_flags = p;
	csp_logger = logger;
	
	gl_grid_horiz = false;
	gl_grid_vert = false;
	glVertGrids = NULL;
	glHorizGrids = NULL;

	vert_lines = using_flags->get_real_screen_height();
	horiz_pixels = using_flags->get_real_screen_width();
	set_brightness = false;
	crt_flag = false;
	smoosing = false;
	uVramTextureID = 0;
	emu_launched = false;
	
	imgptr = NULL;
	screen_multiply = 1.0f;
	screen_texture_width = using_flags->get_screen_width();
	screen_texture_width_old = using_flags->get_screen_width();
	screen_texture_height = using_flags->get_screen_height();
	screen_texture_height_old = using_flags->get_screen_height();
	extfunc_2 = NULL;
	redraw_required = false;
	osd_led_status = 0x00000000;
	osd_led_status_bak = 0x00000000;
	osd_led_bit_width = 12;
	osd_onoff = true;

	uBitmapTextureID = 0;
	bitmap_uploaded = false;
	texture_max_size = 128;
	low_resolution_screen = false;
	int i;
	// Will fix: Must fix setup of vm_buttons[].
	button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
	if(vm_buttons_d != NULL) {
		for(i = 0; i < using_flags->get_max_button(); i++) {
# if defined(_USE_GLAPI_QT5_4)   
			uButtonTextureID[i] = new QOpenGLTexture(QOpenGLTexture::Target2D);
# else	   
			uButtonTextureID[i] = 0;
# endif
			fButtonX[i] = -1.0 + (float)(vm_buttons_d[i].x * 2) / (float)using_flags->get_screen_width();
			fButtonY[i] = 1.0 - (float)(vm_buttons_d[i].y * 2) / (float)using_flags->get_screen_height();
			fButtonWidth[i] = (float)(vm_buttons_d[i].width * 2) / (float)using_flags->get_screen_width();
			fButtonHeight[i] = (float)(vm_buttons_d[i].height * 2) / (float)using_flags->get_screen_height();
		} // end of will fix.
	}
	button_updated = false;
	button_drawn = false;

	fBrightR = 1.0; // 輝度の初期化
	fBrightG = 1.0;
	fBrightB = 1.0;
	set_brightness = false;
	crt_flag = false;
	smoosing = false;
	
	gl_grid_horiz = false;
	gl_grid_vert = false;

	vert_lines = using_flags->get_screen_height();
	horiz_pixels = using_flags->get_screen_width();
	screen_width = 1.0;
	screen_height = 1.0;

	buffer_screen_vertex = NULL;
	vertex_screen = NULL;

	offscreen_frame_buffer = NULL;
	offscreen_frame_buffer_format = NULL;
	rec_count = 0;
	rec_width  = using_flags->get_screen_width();
	rec_height = using_flags->get_screen_height();
	ButtonImages.clear();
}

GLDraw_2_0::~GLDraw_2_0()
{
	if(buffer_screen_vertex->isCreated()) buffer_screen_vertex->destroy();
	if(vertex_screen->isCreated()) vertex_screen->destroy();
	if(glVertGrids != NULL) free(glVertGrids);
	if(glHorizGrids != NULL) free(glHorizGrids);
	if(using_flags->is_use_one_board_computer()) {
		if(uBitmapTextureID != 0) {
			p_wid->deleteTexture(uBitmapTextureID);
		}
	}
	if(using_flags->get_max_button() > 0) {
		int i;
		for(i = 0; i < using_flags->get_max_button(); i++) {
			if(uButtonTextureID[i] != 0) p_wid->deleteTexture(uButtonTextureID[i]);
		}
		for(i = 0; i < using_flags->get_max_button(); i++) {
			if(vertex_button[i]->isCreated()) vertex_button[i]->destroy();
		}
	}

	if(using_flags->is_use_one_board_computer()) {
		if(vertex_bitmap->isCreated()) vertex_bitmap->destroy();
	}
	if(uVramTextureID != 0) {
		p_wid->deleteTexture(uVramTextureID);
	}
	if(extfunc_2 != NULL) delete extfunc_2;
}

void GLDraw_2_0::initializeGL(void)
{
}

void GLDraw_2_0::do_set_display_osd(bool onoff)
{
	osd_onoff = onoff;
}

void GLDraw_2_0::do_display_osd_leds(int lednum, bool onoff)
{
	if(lednum == -1) {
		osd_led_status = (onoff) ? 0xffffffff : 0x00000000;
	} else if((lednum >= 0) && (lednum < 32)) {
		uint32_t nn;
		nn = 0x00000001 << lednum;
		if(onoff) {
			osd_led_status |= nn;
		} else {
			osd_led_status &= ~nn;
		}
	}
}

void GLDraw_2_0::drawOsdLeds()
{
	QVector4D color_on;
	QVector4D color_off;
	VertexTexCoord_t vertex[4];
	float xbase, ybase, zbase;
	if(osd_onoff) {
		color_on = QVector4D(0.95, 0.0, 0.05, 1.0);
		color_off = QVector4D(0.05,0.05, 0.05, 0.10);
	} else {
		color_on = QVector4D(0.00,0.00, 0.00, 0.0);
		color_off = QVector4D(0.00,0.00, 0.00, 0.0);
	}
	xbase = 0.0f + (1.0f / 32.0f) * 31.0f + (1.0f / 128.0f);
	ybase = -1.0f + (2.0f / 64.0f) * 1.5f;
	zbase = -0.999f;
	
	extfunc_2->glEnable(GL_BLEND);
	extfunc_2->glDisable(GL_TEXTURE_2D);
	extfunc_2->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	extfunc_2->glDisable(GL_DEPTH_TEST);
	extfunc_2->glViewport(0, 0, p_wid->width(), p_wid->height());
	extfunc_2->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
	if(osd_led_status != osd_led_status_bak) {
		if(osd_onoff) {
			uint32_t _bit = 0x00000001;
			for(int ii = 0; ii < osd_led_bit_width; ii++) {
				if((_bit & osd_led_status) == (_bit & osd_led_status_bak)) {
					xbase = xbase - (1.0f / 32.0f);
					_bit <<= 1;
					continue;
				}
				vertex[0].x = xbase;
				vertex[0].y = ybase;
				vertex[0].z = zbase;
				
				vertex[1].x = xbase + (1.0f / 64.0f);
				vertex[1].y = ybase;
				vertex[1].z = zbase;
				
				vertex[2].x = xbase + (1.0f / 64.0f);
				vertex[2].y = ybase - (1.0f / 64.0f);
				vertex[2].z = zbase;
			
				vertex[3].x = xbase;
				vertex[3].y = ybase - (1.0f / 64.0f);
				vertex[3].z = zbase;
				if(_bit & osd_led_status) {
					extfunc_2->glColor4f(color_on.x(), color_on.y(), color_on.z(), color_on.w());
				} else {
					extfunc_2->glColor4f(color_off.x(), color_off.y(), color_off.z(), color_off.w());
				}			
				extfunc_2->glBegin(GL_POLYGON);
				for(int j = 0; j < 4; j++) {
					extfunc_2->glVertex3f(vertex[j].x, vertex[j].y, vertex[j].z);
				}
				extfunc_2->glEnd();
				xbase = xbase - (1.0f / 32.0f);
				_bit <<= 1;
			}
			osd_led_status_bak = osd_led_status;
		}
	}
}

void GLDraw_2_0::setNormalVAO(QOpenGLShaderProgram *prg,
							   QOpenGLVertexArrayObject *vp,
							   QOpenGLBuffer *bp,
							   VertexTexCoord_t *tp,
							   int size)
{
	int vertex_loc = prg->attributeLocation("vertex");
	int texcoord_loc = prg->attributeLocation("texcoord");

	if(bp == NULL) return;
	if(prg == NULL) return;
	if(!bp->isCreated()) return;

	bp->bind();
	bp->write(0, tp, sizeof(VertexTexCoord_t) * size);
	prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
	prg->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
	bp->release();

	prg->setUniformValue("a_texture", 0);
			   
}

void GLDraw_2_0::setChangeBrightness(bool flag)
{
	set_brightness = flag;
}

void GLDraw_2_0::setBrightness(GLfloat r, GLfloat g, GLfloat b)
{
	fBrightR = r;
	fBrightG = g;
	fBrightB = b;
}

void GLDraw_2_0::setImgPtr(QImage *p)
{
	imgptr = p;
}

void GLDraw_2_0::setSmoosing(bool flag)
{
	smoosing = flag;
	crt_flag = true;
}

void GLDraw_2_0::setVirtualVramSize(int width, int height)
{
	vert_lines = height;
	horiz_pixels = width;
	crt_flag = true;
}

void GLDraw_2_0::set_emu_launched(void)
{
	emu_launched = true;
}

void GLDraw_2_0::setDrawGLGridVert(bool flag)
{
	gl_grid_vert = flag;
	crt_flag = true;
}

void GLDraw_2_0::setDrawGLGridHoriz(bool flag)
{
	gl_grid_vert = flag;
	crt_flag = true;
}

void GLDraw_2_0::initGLObjects()
{
	extfunc_2 = new QOpenGLFunctions_2_0;
	extfunc_2->initializeOpenGLFunctions();
	extfunc_2->glViewport(0, 0, p_wid->width(), p_wid->height());
	extfunc_2->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
	extfunc_2->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture_max_size);
}	

void GLDraw_2_0::initButtons(void)
{
	button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
	
	if(vm_buttons_d != NULL) {
		button_shader = new QOpenGLShaderProgram(p_wid);
		if(button_shader != NULL) {
			button_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
			button_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/normal_fragment_shader.glsl");
			button_shader->link();
		}

		int ip = using_flags->get_max_button();
		if(ip > 0) {
			for(int num = 0; num < ip; num++) {
				QString tmps;
				tmps = QString::asprintf(":/button%02d.png", num);
				QImageReader *reader = new QImageReader(tmps);
				QImage *result = new QImage(reader->read());
				QImage pic;
				if(result != NULL) {
					if(!result->isNull()) {
						pic = result->convertToFormat(QImage::Format_ARGB32);
					} else {
						pic = QImage(10, 10, QImage::Format_RGBA8888);
						pic.fill(QColor(0,0,0,0));
					}
					delete result;
				}else {
					pic = QImage(10, 10, QImage::Format_RGBA8888);
					pic.fill(QColor(0,0,0,0));
				}
				ButtonImages.push_back(pic);
			}
		}
		vertexButtons = new QVector<VertexTexCoord_t>;
		for(int i = 0; i < using_flags->get_max_button(); i++) {
			buffer_button_vertex[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
			buffer_button_vertex[i]->create();
			fButtonX[i] = -1.0 + (float)(vm_buttons_d[i].x * 2) / (float)using_flags->get_screen_width();
			fButtonY[i] = 1.0 - (float)(vm_buttons_d[i].y * 2) / (float)using_flags->get_screen_height();
			fButtonWidth[i] = (float)(vm_buttons_d[i].width * 2) / (float)using_flags->get_screen_width();
			fButtonHeight[i] = (float)(vm_buttons_d[i].height * 2) / (float)using_flags->get_screen_height();
			
			vertex_button[i] = new QOpenGLVertexArrayObject;
			if(vertex_button[i] != NULL) {
				if(vertex_button[i]->create()) {
					VertexTexCoord_t vt[4];
					
					vt[0].x =  fButtonX[i];
					vt[0].y =  fButtonY[i];
					vt[0].z =  -0.5f;
					vt[0].s = 0.0f;
					vt[0].t = 1.0f;
					
					vt[1].x =  fButtonX[i] + fButtonWidth[i];
					vt[1].y =  fButtonY[i];
					vt[1].z =  -0.5f;
					vt[1].s = 1.0f;
					vt[1].t = 1.0f;
					
					vt[2].x =  fButtonX[i] + fButtonWidth[i];
					vt[2].y =  fButtonY[i] - fButtonHeight[i];
					vt[2].z =  -0.5f;
					vt[2].s = 1.0f;
					vt[2].t = 0.0f;
					
					vt[3].x =  fButtonX[i];
					vt[3].y =  fButtonY[i] - fButtonHeight[i];
					vt[3].z =  -0.5f;
					vt[3].s = 0.0f;
					vt[3].t = 0.0f;
					
					vertexButtons->append(vt[0]);
					vertexButtons->append(vt[1]);
					vertexButtons->append(vt[2]);
					vertexButtons->append(vt[3]);
					vertex_button[i]->bind();
					buffer_button_vertex[i]->bind();
					buffer_button_vertex[i]->allocate(4 * sizeof(VertexTexCoord_t));
					
					buffer_button_vertex[i]->setUsagePattern(QOpenGLBuffer::StaticDraw);
					buffer_button_vertex[i]->release();
					vertex_button[i]->release();
					setNormalVAO(button_shader, vertex_button[i],
								 buffer_button_vertex[i],
								 vt, 4);
				}
			}
		}
	}
}

void GLDraw_2_0::initBitmapVertex(void)
{
	if(using_flags->is_use_one_board_computer()) {
		vertexBitmap[0].x = -1.0f;
		vertexBitmap[0].y = -1.0f;
		vertexBitmap[0].z = -0.1f;
		vertexBitmap[0].s = 0.0f;
		vertexBitmap[0].t = 0.0f;
		
		vertexBitmap[1].x = +1.0f;
		vertexBitmap[1].y = -1.0f;
		vertexBitmap[1].z = -0.1f;
		vertexBitmap[1].s = 1.0f;
		vertexBitmap[1].t = 0.0f;
		
		vertexBitmap[2].x = +1.0f;
		vertexBitmap[2].y = +1.0f;
		vertexBitmap[2].z = -0.1f;
		vertexBitmap[2].s = 1.0f;
		vertexBitmap[2].t = 1.0f;
		
		vertexBitmap[3].x = -1.0f;
		vertexBitmap[3].y = +1.0f;
		vertexBitmap[3].z = -0.1f;
		vertexBitmap[3].s = 0.0f;
		vertexBitmap[3].t = 1.0f;
		
	}
}

void GLDraw_2_0::initBitmapVAO(void)
{
	if(using_flags->is_use_one_board_computer()) {
		bitmap_shader = new QOpenGLShaderProgram(p_wid);
		if(bitmap_shader != NULL) {
			bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
			bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/normal_fragment_shader.glsl");
			bitmap_shader->link();
		}
		buffer_bitmap_vertex = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		vertex_bitmap = new QOpenGLVertexArrayObject;
		if(vertex_bitmap != NULL) {
			if(vertex_bitmap->create()) {
				buffer_bitmap_vertex->create();
				buffer_bitmap_vertex->setUsagePattern(QOpenGLBuffer::StaticDraw);
				
				{
					QVector4D c;
					c = QVector4D(1.0, 1.0, 1.0, 1.0);
					bitmap_shader->setUniformValue("color", c);
				}
				vertex_bitmap->bind();
				buffer_bitmap_vertex->bind();
				buffer_bitmap_vertex->allocate(sizeof(vertexBitmap));
				buffer_bitmap_vertex->release();
				vertex_bitmap->release();
				setNormalVAO(bitmap_shader, vertex_bitmap,
							 buffer_bitmap_vertex,
							 vertexBitmap, 4);
			}
		}
	}	
}

void GLDraw_2_0::initFBO(void)
{
	// Will fix around vm_buttons[].
	initBitmapVertex();
	if(using_flags->get_max_button() > 0) {
		initButtons();
	}
	glHorizGrids = (GLfloat *)malloc(sizeof(float) * (using_flags->get_real_screen_height() + 2) * 6);
	if(glHorizGrids != NULL) {
		doSetGridsHorizonal(using_flags->get_real_screen_height(), true);
	}
	glVertGrids  = (GLfloat *)malloc(sizeof(float) * (using_flags->get_real_screen_width() + 2) * 6);
	if(glVertGrids != NULL) {
		doSetGridsVertical(using_flags->get_real_screen_width(), true);
	}
	// Init view
	extfunc_2->glClearColor(0.0, 0.0, 0.0, 1.0);
}

void GLDraw_2_0::initLocalGLObjects(void)
{
	main_shader = new QOpenGLShaderProgram(p_wid);
	if(main_shader != NULL) {
		main_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
		main_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/chromakey_fragment_shader2.glsl");
		main_shader->link();
	}
	buffer_screen_vertex = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	vertex_screen = new QOpenGLVertexArrayObject;
	
	vertexFormat[0].x = -0.5f;
	vertexFormat[0].y = -0.5f;
	vertexFormat[0].z = -0.9f;
	vertexFormat[0].s = 0.0f;
	vertexFormat[0].t = 1.0f;
			
	vertexFormat[1].x = +0.5f;
	vertexFormat[1].y = -0.5f;
	vertexFormat[1].z = -0.9f;
	vertexFormat[1].s = 1.0f;
	vertexFormat[1].t = 1.0f;
			
	vertexFormat[2].x = +0.5f;
	vertexFormat[2].y = +0.5f;
	vertexFormat[2].z = -0.9f;
	vertexFormat[2].s = 1.0f;
	vertexFormat[2].t = 0.0f;
	
	vertexFormat[3].x = -0.5f;
	vertexFormat[3].y = +0.5f;
	vertexFormat[3].z = -0.9f;
	vertexFormat[3].s = 0.0f;
	vertexFormat[3].t = 0.0f;
	if(buffer_screen_vertex != NULL) {
		if(buffer_screen_vertex->create()) {
			{
				QVector4D c;
				c = QVector4D(1.0, 1.0, 1.0, 1.0);
				main_shader->setUniformValue("color", c);
			}
			
			//buffer_screen_vertex->create();
			buffer_screen_vertex->setUsagePattern(QOpenGLBuffer::DynamicDraw);
			vertex_screen->create();
			
			if(vertex_screen != NULL) vertex_screen->bind();
			buffer_screen_vertex->bind();
			buffer_screen_vertex->allocate(sizeof(VertexTexCoord_t) * 4);
			if(vertex_screen != NULL) vertex_screen->release();
			buffer_screen_vertex->release();
			setNormalVAO(main_shader, vertex_screen,
						 buffer_screen_vertex,
						 vertexFormat, 4);
		}
	}
	initBitmapVAO();
}

void GLDraw_2_0::doSetGridsHorizonal(int lines, bool force)
{
	int i;
	GLfloat yf;
	GLfloat delta;
	
	if((lines == vert_lines) && !force) return;
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

void GLDraw_2_0::doSetGridsVertical(int pixels, bool force)
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
void GLDraw_2_0::drawGridsMain(GLfloat *tp,
							   int number,
							   GLfloat lineWidth,
							   QVector4D color)
{
	if(number <= 0) return;
	extfunc_2->glDisable(GL_TEXTURE_2D);
	extfunc_2->glDisable(GL_DEPTH_TEST);
	extfunc_2->glDisable(GL_BLEND);
	{ 
		if(tp != NULL) {
			int i;
			int p = 0;
			extfunc_2->glColor4f(color.x(), color.y(), color.z(), color.w());
			extfunc_2->glLineWidth(lineWidth);
			extfunc_2->glBegin(GL_LINES);
			for(i = 0; i < (number + 1); i++) {
				extfunc_2->glVertex3f(tp[p + 0], tp[p + 1], tp[p + 2]);
				extfunc_2->glVertex3f(tp[p + 3], tp[p + 4], tp[p + 5]);
				p += 6;
			}
			extfunc_2->glEnd();
		}
	}
}

void GLDraw_2_0::drawGridsHorizonal(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	drawGridsMain(glHorizGrids,
				  vert_lines,
				  0.15f,
				  c);
}

void GLDraw_2_0::drawGridsVertical(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	drawGridsMain(glVertGrids,
				  horiz_pixels,
				  0.5f,
				  c);
}


void GLDraw_2_0::drawGrids(void)
{
	gl_grid_horiz = using_flags->get_config_ptr()->opengl_scanline_horiz;
	gl_grid_vert  = using_flags->get_config_ptr()->opengl_scanline_vert;
	if(gl_grid_horiz && (vert_lines > 0)) {
		drawGridsHorizonal();
	} // Will fix.
	if(using_flags->is_use_vertical_pixel_lines()) {
		if(gl_grid_vert && (horiz_pixels > 0)) {
			drawGridsVertical();
		}
	}
}

void GLDraw_2_0::updateButtonTexture(void)
{
	int i;
	button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
	if(button_updated) return;
	if(vm_buttons_d != NULL) {
		for(i = 0; i < using_flags->get_max_button(); i++) {
			QImage img = ButtonImages.at(i);
			if(uButtonTextureID[i] != 0) {
				p_wid->deleteTexture(uButtonTextureID[i]);
			}
			uButtonTextureID[i] = p_wid->bindTexture(img);
		}
	}
	button_updated = true;
}

void GLDraw_2_0::drawButtons()
{
	int i;
	updateButtonTexture();
	for(i = 0; i < using_flags->get_max_button(); i++) {
		QVector4D c;
		VertexTexCoord_t vt[4];
		vt[0].x =  fButtonX[i];
		vt[0].y =  fButtonY[i];
		vt[0].z =  -0.5f;
		vt[0].s = 0.0f;
		vt[0].t = 1.0f;
		
		vt[1].x =  fButtonX[i] + fButtonWidth[i];
		vt[1].y =  fButtonY[i];
		vt[1].z =  -0.5f;
		vt[1].s = 1.0f;
		vt[1].t = 1.0f;
		
		vt[2].x =  fButtonX[i] + fButtonWidth[i];
		vt[2].y =  fButtonY[i] - fButtonHeight[i];
		vt[2].z =  -0.5f;
		vt[2].s = 1.0f;
		vt[2].t = 0.0f;
		
		vt[3].x =  fButtonX[i];
		vt[3].y =  fButtonY[i] - fButtonHeight[i];
		vt[3].z =  -0.5f;
		vt[3].s = 0.0f;
		vt[3].t = 0.0f;
		c = QVector4D(1.0, 1.0, 1.0, 1.0);
		drawMain(button_shader, vertex_button[i],
				 buffer_button_vertex[i],
				 vt,
				 uButtonTextureID[i],
				 c, false);
	}
}


void GLDraw_2_0::drawBitmapTexture(void)
{
	if(using_flags->is_use_one_board_computer()) {
		QVector4D c;
		c = QVector4D(1.0, 1.0, 1.0, 1.0);
		drawMain(bitmap_shader, vertex_bitmap,
				 buffer_bitmap_vertex,
				 vertexBitmap,
				 uBitmapTextureID,
				 c, false);
	}
}

void GLDraw_2_0::drawScreenTexture(void)
{
	if(using_flags->is_use_one_board_computer()) {
		if(uBitmapTextureID != 0) {
			extfunc_2->glEnable(GL_BLEND);
			extfunc_2->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	} else {
		extfunc_2->glDisable(GL_BLEND);
	}
	
	QVector4D color;
	smoosing = using_flags->get_config_ptr()->use_opengl_filters;
	if(set_brightness) {
		color = QVector4D(fBrightR, fBrightG, fBrightB, 1.0);
	} else {
		color = QVector4D(1.0, 1.0, 1.0, 1.0);
	}			
	if(using_flags->is_use_one_board_computer()) {
		QVector3D cc = QVector3D(0.0, 0.0, 0.0);
		drawMain(main_shader, vertex_screen,
				 buffer_screen_vertex,
				 vertexFormat,
				 uVramTextureID, // v2.0
				 color, smoosing,
				 true, cc);
		extfunc_2->glDisable(GL_BLEND);
	} else{
		drawMain(main_shader, vertex_screen,
				 buffer_screen_vertex,
				 vertexFormat,
				 uVramTextureID, // v2.0
				 color, smoosing);
	}		
}

void GLDraw_2_0::drawMain(QOpenGLShaderProgram *prg,
						  QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  VertexTexCoord_t *vertex_data,
						  GLuint texid,
						  QVector4D color,
						  bool f_smoosing,
						  bool do_chromakey,
						  QVector3D chromakey)
						   
{
	if(texid != 0) {
		QImage *p = imgptr;
		extfunc_2->glEnable(GL_TEXTURE_2D);

		extfunc_2->glViewport(0, 0, p_wid->width(), p_wid->height());
		extfunc_2->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
		extfunc_2->glBindTexture(GL_TEXTURE_2D, texid);

		if(!f_smoosing) {
			extfunc_2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			extfunc_2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		} else {
			extfunc_2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			extfunc_2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		}

		if((bp != NULL) && (vp != NULL) && (prg != NULL)) {
			if((bp->isCreated()) && (vp->isCreated()) && (prg->isLinked())) {
				bp->bind();
				vp->bind();
				prg->bind();
				prg->setUniformValue("a_texture", 0);
				prg->setUniformValue("color", color);
				//prg->setUniformValue("tex_width",  (float)screen_texture_width); 
				//prg->setUniformValue("tex_height", (float)screen_texture_height);
				prg->setUniformValue("tex_width",  (float)p->width()); 
				prg->setUniformValue("tex_height", (float)p->height());
				if(using_flags->is_use_screen_rotate()) {
					if(using_flags->get_config_ptr()->rotate_type) {
						prg->setUniformValue("rotate", GL_TRUE);
					} else {
						prg->setUniformValue("rotate", GL_FALSE);
					}
				} else {
					prg->setUniformValue("rotate", GL_FALSE);
				}
				if(do_chromakey) {
					prg->setUniformValue("chromakey", chromakey);
					prg->setUniformValue("do_chromakey", GL_TRUE);
				} else {
					prg->setUniformValue("do_chromakey", GL_FALSE);
				}			
				int vertex_loc = prg->attributeLocation("vertex");
				int texcoord_loc = prg->attributeLocation("texcoord");
				prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
				prg->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
				prg->enableAttributeArray(vertex_loc);
				prg->enableAttributeArray(texcoord_loc);
				
				extfunc_2->glEnableClientState(GL_VERTEX_ARRAY);
				extfunc_2->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				extfunc_2->glColor3f(1.0f, 1.0f, 1.0f);
				
				extfunc_2->glVertexPointer(3, GL_FLOAT, sizeof(VertexTexCoord_t), (void *)0);
				extfunc_2->glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTexCoord_t), (void *)(0 + 3 * sizeof(GLfloat)));
				extfunc_2->glDrawArrays(GL_POLYGON, 0, 4);
				extfunc_2->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				extfunc_2->glDisableClientState(GL_VERTEX_ARRAY);
				
				bp->release();
				vp->release();
				prg->release();
				extfunc_2->glBindTexture(GL_TEXTURE_2D, 0);
				extfunc_2->glDisable(GL_TEXTURE_2D);
				return;
			}
		}
		
		{ // Fallback
			int i;
			extfunc_2->glBegin(GL_POLYGON);
			for(i = 0; i < 4; i++) {
				extfunc_2->glTexCoord2f(vertex_data[i].s, vertex_data[i].t);
				extfunc_2->glVertex3f(vertex_data[i].x, vertex_data[i].y, vertex_data[i].z);
			}
			extfunc_2->glEnd();
			extfunc_2->glBindTexture(GL_TEXTURE_2D, 0);
			extfunc_2->glDisable(GL_TEXTURE_2D);
		}
	}
}

void GLDraw_2_0::uploadBitmapTexture(QImage *p)
{
	if(!using_flags->is_use_one_board_computer()) return;
	if(p == NULL) return;
	if(!bitmap_uploaded) {
		p_wid->makeCurrent();
		if(uBitmapTextureID != 0) {
	  		p_wid->deleteTexture(uBitmapTextureID);
		}
		uBitmapTextureID = p_wid->bindTexture(*p);
		p_wid->doneCurrent();
		bitmap_uploaded = true;
		crt_flag = true;
	}
}

void GLDraw_2_0::updateBitmap(QImage *p)
{
	if(!using_flags->is_use_one_board_computer()) return;
	redraw_required = true;
	bitmap_uploaded = false;
	uploadBitmapTexture(p);
}

void GLDraw_2_0::uploadMainTexture(QImage *p, bool use_chromakey)
{
	// set vertex
	redraw_required = true;
	imgptr = p;
	if(p == NULL) return;
	{
		if(use_chromakey) {
		}
		// Upload to main texture
		if(uVramTextureID == 0) {
			uVramTextureID = p_wid->bindTexture(*p);
		}
		extfunc_2->glBindTexture(GL_TEXTURE_2D, uVramTextureID);
		extfunc_2->glTexSubImage2D(GL_TEXTURE_2D, 0,
								 0, 0,
								 p->width(), p->height(),
								 GL_BGRA, GL_UNSIGNED_BYTE, p->constBits());
		extfunc_2->glBindTexture(GL_TEXTURE_2D, 0);
	}
	crt_flag = true;
}

void GLDraw_2_0::resizeGL_Screen(void)
{
	if(buffer_screen_vertex->isCreated()) {
		setNormalVAO(main_shader, vertex_screen,
					 buffer_screen_vertex,
					 vertexFormat, 4);
	}
}	

void GLDraw_2_0::resizeGL_SetVertexs(void)
{
	float iw = (float)using_flags->get_real_screen_width();
	float ih = (float)using_flags->get_real_screen_height();
	vertexFormat[0].x = -screen_width;
	vertexFormat[0].y = -screen_height;
	//vertexFormat[0].s = 0.0f;
	//vertexFormat[0].t = ih / screen_texture_height;

	vertexFormat[1].x = +screen_width;
	vertexFormat[1].y = -screen_height;
	//vertexFormat[1].s = iw / screen_texture_width;
	//vertexFormat[1].t = ih / screen_texture_height;
	
	vertexFormat[2].x = +screen_width;
	vertexFormat[2].y = +screen_height;
	//vertexFormat[2].s = iw / screen_texture_width;
	//vertexFormat[2].t = 0.0f;
	
	vertexFormat[3].x = -screen_width;
	vertexFormat[3].y = +screen_height;
	//vertexFormat[3].s = 0.0f;
	//vertexFormat[3].t = 0.0f;
	
	if(using_flags->is_use_one_board_computer()) {
#if !defined(BITMAP_OFFSET_X)
		#define BITMAP_OFFSET_X 0
#endif	   
#if !defined(BITMAP_OFFSET_Y)
		#define BITMAP_OFFSET_Y 0
#endif	   
			vertexBitmap[0].x = -1.0f;
			vertexBitmap[0].y = -1.0f;
			
			vertexBitmap[1].x = 1.0f - (float)BITMAP_OFFSET_X / (float)using_flags->get_screen_width();
			vertexBitmap[1].y = -1.0f;
			
			vertexBitmap[2].x = 1.0f - (float)BITMAP_OFFSET_X / (float)using_flags->get_screen_width();
			vertexBitmap[2].y = 1.0f - (float)BITMAP_OFFSET_Y * 2.0 / (float)using_flags->get_screen_height();
			
			vertexBitmap[3].x = -1.0f;
			vertexBitmap[3].y = 1.0f - (float)BITMAP_OFFSET_Y * 2.0 / (float)using_flags->get_screen_height();
			
	}
}

void GLDraw_2_0::resizeGL(int width, int height)
{
	//int side = qMin(width, height);
	p_wid->makeCurrent();
	extfunc_2->glViewport(0, 0, width, height);
	extfunc_2->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
	crt_flag = true;
	if(!using_flags->is_use_one_board_computer() && (using_flags->get_max_button() <= 0)) {
		doSetGridsHorizonal(vert_lines, true);
		if(using_flags->is_use_vertical_pixel_lines()) {
			doSetGridsVertical(horiz_pixels, true);
		}
	}
	resizeGL_SetVertexs();
	resizeGL_Screen();
	if(using_flags->is_use_one_board_computer()) {
		if(vertex_bitmap != NULL) {
			if(vertex_bitmap->isCreated()) {
				setNormalVAO(bitmap_shader, vertex_bitmap,
							 buffer_bitmap_vertex,
							 vertexBitmap, 4);
			}
		}
	}

	if(using_flags->get_max_button() > 0) {
		updateButtonTexture();
	}
	p_wid->doneCurrent();
}

void GLDraw_2_0::paintGL(void)
{
	//p_wid->makeCurrent();
	if(crt_flag || redraw_required) { //return;
		if(emu_launched) {
			crt_flag = false;
		}
		redraw_required = false;
		extfunc_2->glViewport(0, 0, p_wid->width(), p_wid->height());
		extfunc_2->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
		
		extfunc_2->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		extfunc_2->glEnable(GL_DEPTH_TEST);
		extfunc_2->glDisable(GL_BLEND);
		if(using_flags->is_use_one_board_computer() || using_flags->is_use_bitmap()) {
			drawBitmapTexture();
		}
		if((using_flags->get_max_button() > 0)) {
			drawButtons();
		}
		/*
		 * VRAMの表示:テクスチャ貼った四角形
		 */
		drawScreenTexture();
		extfunc_2->glDisable(GL_BLEND);
		if(!using_flags->is_use_one_board_computer() && (using_flags->get_max_button() <= 0)) {
			drawGrids();
		}
		drawOsdLeds();
		extfunc_2->glFlush();
	} else {
		drawOsdLeds();
		extfunc_2->glFlush();
	}
	//p_wid->doneCurrent();
}

void GLDraw_2_0::paintGL_OffScreen(int count, int w, int h)
{
	rec_count += count;
	rec_width  = w;
	rec_height = h;
}

void GLDraw_2_0::do_set_screen_multiply(float mul)
{
	screen_multiply = mul;
	do_set_texture_size(imgptr, screen_texture_width, screen_texture_height);
	//do_set_texture_size(imgptr, -1, -1);
}


void GLDraw_2_0::do_set_texture_size(QImage *p, int w, int h)
{
	if(w <= 0) w = using_flags->get_real_screen_width();
	if(h <= 0) h = using_flags->get_real_screen_height();
	float iw;
	float ih;
	imgptr = p;
	if(p != NULL) {
		iw = (float)p->width();
		ih = (float)p->height();
	} else {
		iw = (float)using_flags->get_real_screen_width();
		ih = (float)using_flags->get_real_screen_height();
	}
	screen_texture_width = w;
	screen_texture_height = h;

	if(p_wid != NULL) {
		p_wid->makeCurrent();
		vertexFormat[0].s = 0.0f;
		vertexFormat[0].t = (float)h / ih;
		vertexFormat[1].s = (float)w / iw;
		vertexFormat[1].t = (float)h / ih;
		vertexFormat[2].s = (float)w / iw;
		vertexFormat[2].t = 0.0f;
		vertexFormat[3].s = 0.0f;
		vertexFormat[3].t = 0.0f;
		setNormalVAO(main_shader, vertex_screen,
					 buffer_screen_vertex,
					 vertexFormat, 4);
		p_wid->doneCurrent();
	}		
	this->doSetGridsHorizonal(h, true);
	this->doSetGridsVertical(w, true);
}

void GLDraw_2_0::do_set_led_width(int bitwidth)
{
	if((bitwidth >= 0) && (bitwidth < 32)) osd_led_bit_width = bitwidth;
}
