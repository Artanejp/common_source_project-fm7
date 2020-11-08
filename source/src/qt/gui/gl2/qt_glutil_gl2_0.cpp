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
#include <QColor>
#include <QImageReader>
#include <QRect>
#include <QOpenGLFunctions_2_0>

#include "osd_types.h"
#include "qt_gldraw.h"
#include "qt_glutil_gl2_0.h"
#include "menu_flags.h"

GLDraw_2_0::GLDraw_2_0(GLDrawClass *parent, USING_FLAGS *p, CSP_Logger *logger, EMU_TEMPLATE *emu) : GLDraw_Tmpl(parent, p, logger, emu)
{
	extfunc_2 = NULL;
	
}

GLDraw_2_0::~GLDraw_2_0()
{
	if(buffer_screen_vertex->isCreated()) buffer_screen_vertex->destroy();
	if(vertex_screen->isCreated()) vertex_screen->destroy();
	if(glVertGrids != NULL) free(glVertGrids);
	if(glHorizGrids != NULL) free(glHorizGrids);
	if(using_flags->is_use_one_board_computer()) {
		if(uBitmapTextureID != NULL) {
			delete uBitmapTextureID;
		}
	}
	if(using_flags->get_max_button() > 0) {
		int i;
		for(i = 0; i < using_flags->get_max_button(); i++) {
			if(uButtonTextureID[i] != NULL) delete uButtonTextureID[i];
		}
		for(i = 0; i < using_flags->get_max_button(); i++) {
			if(vertex_button[i]->isCreated()) vertex_button[i]->destroy();
		}
	}

	if(using_flags->is_use_one_board_computer()) {
		if(vertex_bitmap->isCreated()) vertex_bitmap->destroy();
	}
	if(uVramTextureID != NULL) {
		delete uVramTextureID;
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

void GLDraw_2_0::set_osd_vertex(int xbit)
{
	float xbase, ybase, zbase;
	int major, minor, nl;
	int i = xbit;
	if((xbit < 0) || (xbit >= 32)) return;
	if((i >= 2) && (i < 10)) { // FD
		major = 0;
		minor = i - 2;
		nl = using_flags->get_max_drive();
	} else if((i >= 10) && (i < 12)) { // QD
		major = 2;
		minor = i - 10;
		nl = using_flags->get_max_qd();
	} else if((i >= 12) && (i < 14)) { // CMT(R)
		major = 1;
		minor = i - 12;
		nl = using_flags->get_max_tape();
	} else if((i >= 14) && (i < 16)) { // CMT(W)
		major = 1;
		minor = i - 14;
		nl = using_flags->get_max_tape();
	} else if(i >= 16) {
		major = 4 + (i / 8) - 2;
		minor = i % 8;
		nl = 8;
	} else {
		major = 6;
		minor = i;
		nl = 2;
	}
	xbase =  1.0f - (1.0f * 48.0f / 640.0f) * (float)(nl - minor) - (4.0f / 640.0f);;
	ybase = -1.0f + (1.0f * 48.0f / 400.0f) * (float)(major + 1) + (4.0f / 400.0f);
	zbase = -0.998f;
	vertexOSD[i][0].x = xbase;
	vertexOSD[i][0].y = ybase;
	vertexOSD[i][0].z = zbase;
	vertexOSD[i][0].s = 0.0f;
	vertexOSD[i][0].t = 0.0f;
	
	vertexOSD[i][1].x = xbase + (48.0f / 640.0f);
	vertexOSD[i][1].y = ybase;
	vertexOSD[i][1].z = zbase;
	vertexOSD[i][1].s = 1.0f;
	vertexOSD[i][1].t = 0.0f;
	
	vertexOSD[i][2].x = xbase + (48.0f / 640.0f);
	vertexOSD[i][2].y = ybase - (48.0f / 400.0f);
	vertexOSD[i][2].z = zbase;
	vertexOSD[i][2].s = 1.0f;
	vertexOSD[i][2].t = 1.0f;
	
	vertexOSD[i][3].x = xbase;
	vertexOSD[i][3].y = ybase - (48.0f / 400.0f);
	vertexOSD[i][3].z = zbase;
	vertexOSD[i][3].s = 0.0f;
	vertexOSD[i][3].t = 1.0f;
	
	setNormalVAO(osd_shader, vertex_osd[xbit],
				 buffer_osd[xbit],
				 &(vertexOSD[i][0]), 4);
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


void GLDraw_2_0::drawOsdIcons()
{
	QVector4D color_on;
	QVector4D color_off;
	int major, minor;
	if(osd_onoff) {
		color_on = QVector4D(1.0, 1.0, 1.0, 0.8);
		color_off = QVector4D(1.0, 1.0, 1.0, 0.00);
	} else {
		color_on = QVector4D(0.00,0.00, 0.00, 0.0);
		color_off = QVector4D(0.00,0.00, 0.00, 0.0);
	}
	extfunc_2->glEnable(GL_BLEND);
	extfunc_2->glEnable(GL_TEXTURE_2D);
	extfunc_2->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	extfunc_2->glDisable(GL_DEPTH_TEST);
	extfunc_2->glViewport(0, 0, p_wid->width(), p_wid->height());
	extfunc_2->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
	if(osd_onoff) {
		uint32_t _bit = 0x00000001;
		bool checkf;
		for(int ii = 0; ii < osd_led_bit_width; ii++) {
			checkf = ((_bit & osd_led_status) != 0); 
			if(checkf == ((_bit & osd_led_status_bak) != 0)) {
				if(!checkf) {
					_bit <<= 1;
					continue;
				}
			}
			if((ii >= 2) && (ii < 10)) { // FD
				major = 2;
				minor = ii - 2;
			} else if((ii >= 10) && (ii < 12)) { // QD
				major = 3;
				minor = ii - 10;
			} else if((ii >= 12) && (ii < 14)) { // CMT(R)
				major = 4;
				minor = ii - 12;
			} else if((ii >= 14) && (ii < 16)) { // CMT(W)
				major = 5;
				minor = ii - 14;
			}  else if((ii >= 16) && (ii < 24)) { // HDD
				major = 8;
				minor = ii - 16;
			} else if((ii >= 24) && (ii < 26)) { // CD
				major = 6;
				minor = ii - 24;
			} else if((ii >= 26) && (ii < 28)) { // LD
				major = 7;
				minor = ii - 26;
			} else {
				major = 0;
				minor = 0;
			}
			// ToDo: CD,LD and HDD.
			if(icon_texid[major][minor] != NULL) {
			if(checkf) {
				drawMain(osd_shader, vertex_osd[ii],
						 buffer_osd[ii],
						 vertexOSD[ii],
						 icon_texid[major][minor]->textureId(),
						 color_on, false);
			} else {
				drawMain(osd_shader, vertex_osd[ii],
						 buffer_osd[ii],
						 vertexOSD[ii],
						 icon_texid[major][minor]->textureId(),
						 color_off, false);
			}
			}
			_bit <<= 1;
		}
		osd_led_status_bak = osd_led_status;
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
			button_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/gl2/vertex_shader.glsl");
			button_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/gl2/normal_fragment_shader.glsl");
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
					vt[0].t = 0.0f;
					
					vt[1].x =  fButtonX[i] + fButtonWidth[i];
					vt[1].y =  fButtonY[i];
					vt[1].z =  -0.5f;
					vt[1].s = 1.0f;
					vt[1].t = 0.0f;
					
					vt[2].x =  fButtonX[i] + fButtonWidth[i];
					vt[2].y =  fButtonY[i] - fButtonHeight[i];
					vt[2].z =  -0.5f;
					vt[2].s = 1.0f;
					vt[2].t = 1.0f;
					
					vt[3].x =  fButtonX[i];
					vt[3].y =  fButtonY[i] - fButtonHeight[i];
					vt[3].z =  -0.5f;
					vt[3].s = 0.0f;
					vt[3].t = 1.0f;
					
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
		vertexBitmap[0].t = 1.0f;
		
		vertexBitmap[1].x = +1.0f;
		vertexBitmap[1].y = -1.0f;
		vertexBitmap[1].z = -0.1f;
		vertexBitmap[1].s = 1.0f;
		vertexBitmap[1].t = 1.0f;
		
		vertexBitmap[2].x = +1.0f;
		vertexBitmap[2].y = +1.0f;
		vertexBitmap[2].z = -0.1f;
		vertexBitmap[2].s = 1.0f;
		vertexBitmap[2].t = 0.0f;
		
		vertexBitmap[3].x = -1.0f;
		vertexBitmap[3].y = +1.0f;
		vertexBitmap[3].z = -0.1f;
		vertexBitmap[3].s = 0.0f;
		vertexBitmap[3].t = 0.0f;
		
	}
}

void GLDraw_2_0::initBitmapVAO(void)
{
	if(using_flags->is_use_one_board_computer()) {
		bitmap_shader = new QOpenGLShaderProgram(p_wid);
		if(bitmap_shader != NULL) {
			bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/gl2/vertex_shader.glsl");
			bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/gl2/normal_fragment_shader.glsl");
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
		main_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/gl2/vertex_shader.glsl");
		main_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/gl2/chromakey_fragment_shader2.glsl");
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
	initOsdObjects();
}

void GLDraw_2_0::initOsdObjects(void)
{
	osd_shader = new QOpenGLShaderProgram(p_wid);
	if(osd_shader != NULL) {
		osd_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/gl2/vertex_shader.glsl");
		osd_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/gl2/icon_fragment_shader.glsl");
		osd_shader->link();
	}
	for(int i = 0; i < 32; i++) {
		buffer_osd[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		vertex_osd[i] = new QOpenGLVertexArrayObject;
		
		if(buffer_osd[i] != NULL) {
			if(buffer_osd[i]->create()) {
				{
					QVector4D c;
					c = QVector4D(1.0, 1.0, 1.0, 1.0);
					osd_shader->setUniformValue("color", c);
				}
				
				//buffer_screen_vertex->create();
				buffer_osd[i]->setUsagePattern(QOpenGLBuffer::StaticDraw);
				vertex_osd[i]->create();
				
				if(vertex_osd[i] != NULL) vertex_osd[i]->bind();
				buffer_osd[i]->bind();
				buffer_osd[i]->allocate(sizeof(VertexTexCoord_t) * 4);
				if(vertex_osd[i] != NULL) vertex_osd[i]->release();
				buffer_osd[i]->release();
				set_osd_vertex(i);
			}
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
			QMatrix2x2 rot;
			extfunc_2->glColor4f(color.x(), color.y(), color.z(), color.w());
			extfunc_2->glLineWidth(lineWidth);
			extfunc_2->glBegin(GL_LINES);
			if((p_config->rotate_type == 1) || (p_config->rotate_type == 3)) {
				for(i = 0; i < (number + 1); i++) {
					extfunc_2->glVertex3f(tp[p + 1], tp[p + 0], tp[p + 2]);
					extfunc_2->glVertex3f(tp[p + 4], tp[p + 3], tp[p + 5]);
					p += 6;
				}
			} else {
				for(i = 0; i < (number + 1); i++) {
					extfunc_2->glVertex3f(tp[p + 0], tp[p + 1], tp[p + 2]);
					extfunc_2->glVertex3f(tp[p + 3], tp[p + 4], tp[p + 5]);
					p += 6;
				}
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
	gl_grid_horiz = p_config->opengl_scanline_horiz;
	gl_grid_vert  = p_config->opengl_scanline_vert;
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
			if(uButtonTextureID[i] != NULL) {
				delete uButtonTextureID[i];
			}
			uButtonTextureID[i] = new QOpenGLTexture(img);
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
		if(uButtonTextureID[i] == NULL) continue;
		drawMain(button_shader, vertex_button[i],
				 buffer_button_vertex[i],
				 vt,
				 uButtonTextureID[i]->textureId(),
				 c, false);
	}
}


void GLDraw_2_0::drawBitmapTexture(void)
{
	if(using_flags->is_use_one_board_computer()) {
		QVector4D c;
		c = QVector4D(1.0, 1.0, 1.0, 1.0);
		if(uBitmapTextureID == NULL) return;
		drawMain(bitmap_shader, vertex_bitmap,
				 buffer_bitmap_vertex,
				 vertexBitmap,
				 uBitmapTextureID->textureId(),
				 c, false);
	}
}

void GLDraw_2_0::drawScreenTexture(void)
{
	if(using_flags->is_use_one_board_computer()) {
		if(uBitmapTextureID != NULL) {
			extfunc_2->glEnable(GL_BLEND);
			extfunc_2->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	} else {
		extfunc_2->glDisable(GL_BLEND);
	}
	if(uVramTextureID == NULL) return;
	QVector4D color;
	smoosing = p_config->use_opengl_filters;
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
				 uVramTextureID->textureId(), // v2.0
				 color, smoosing,
				 true, cc);
		extfunc_2->glDisable(GL_BLEND);
	} else{
		drawMain(main_shader, vertex_screen,
				 buffer_screen_vertex,
				 vertexFormat,
				 uVramTextureID->textureId(), // v2.0
				 color, smoosing, false);
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
//		extfunc_2->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
		extfunc_2->glOrtho(1.0f, -1.0f, 1.0f, -1.0f, 1.0, -1.0);
		extfunc_2->glBindTexture(GL_TEXTURE_2D, texid);

		if(!f_smoosing) {
			extfunc_2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			extfunc_2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		} else {
			extfunc_2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			extfunc_2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		}
#if 1
		if((bp != NULL) && (vp != NULL) && (prg != NULL)) {
			if((bp->isCreated()) && (vp->isCreated()) && (prg->isLinked())) {
				bp->bind();
				vp->bind();
				prg->bind();
				prg->setUniformValue("a_texture", 0);
				prg->setUniformValue("color", color);
				//prg->setUniformValue("tex_width",  (float)screen_texture_width); 
				//prg->setUniformValue("tex_height", (float)screen_texture_height);
				if(p != NULL) {
					prg->setUniformValue("tex_width",  (float)p->width()); 
					prg->setUniformValue("tex_height", (float)p->height());
				}
				QMatrix2x2 rot;
				switch(p_config->rotate_type) {
				case 0:
					rot = QMatrix2x2(rot0);
					break;
				case 1:
					rot = QMatrix2x2(rot90);
					break;
				case 2:
					rot = QMatrix2x2(rot180);
					break;
				case 3:
					rot = QMatrix2x2(rot270);
					break;
				default:
					rot = QMatrix2x2(rot0);
					break;
				}
				prg->setUniformValue("rotate_mat", rot);
				if(!(using_flags->is_use_one_board_computer())) {
					prg->setUniformValue("distortion_v", 0.08f, 0.08f); // ToDo: Change val
				}
				prg->setUniformValue("luminance", 0.9f); // ToDo: Change val
				prg->setUniformValue("lum_offset", 0.08f); // ToDo: Change val
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
				
				extfunc_2->glDisable(GL_DEPTH_TEST);
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
#endif		
		{ // Fallback
			int i;
			extfunc_2->glDisable(GL_DEPTH_TEST);
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
		if(uBitmapTextureID != NULL) {
	  		delete uBitmapTextureID;
		}
		uBitmapTextureID = new QOpenGLTexture(*p);
		p_wid->doneCurrent();
		bitmap_uploaded = true;
		crt_flag = true;
	}
}

void GLDraw_2_0::uploadIconTexture(QPixmap *p, int icon_type, int localnum)
{
	if((icon_type >  8) || (icon_type < 0)) return;
	if((localnum  >= 9) || (localnum  <  0)) return;
	if(p == NULL) return;
	p_wid->makeCurrent();
	QImage image = p->toImage();

	if(icon_texid[icon_type][localnum] != NULL) delete icon_texid[icon_type][localnum];
	{
		icon_texid[icon_type][localnum] = new QOpenGLTexture(image);
	}
	p_wid->doneCurrent();

}

void GLDraw_2_0::updateBitmap(QImage *p)
{
	if(!using_flags->is_use_one_board_computer()) return;
	redraw_required = true;
	bitmap_uploaded = false;
	uploadBitmapTexture(p);
}

void GLDraw_2_0::uploadMainTexture(QImage *p, bool use_chromakey, bool was_mapped)
{
	// set vertex
	redraw_required = true;
	imgptr = p;
	if(p == NULL) return;
	{
		// Upload to main texture
		if(uVramTextureID == NULL) {
			uVramTextureID = new QOpenGLTexture(*p);
		}
		extfunc_2->glBindTexture(GL_TEXTURE_2D, uVramTextureID->textureId());
		extfunc_2->glTexSubImage2D(GL_TEXTURE_2D, 0,
								 0, 0,
								 p->width(), p->height(),
								 GL_RGBA, GL_UNSIGNED_BYTE, p->constBits());
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
	vertexFormat[0].x = -screen_width;
	vertexFormat[0].y = -screen_height;
	vertexFormat[1].x = +screen_width;
	vertexFormat[1].y = -screen_height;
	vertexFormat[2].x = +screen_width;
	vertexFormat[2].y = +screen_height;
	vertexFormat[3].x = -screen_width;
	vertexFormat[3].y = +screen_height;
	
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
}

void GLDraw_2_0::paintGL(void)
{
	//p_wid->makeCurrent();
	//if(crt_flag || redraw_required) { //return;
		if(emu_launched) {
			crt_flag = false;
		}
		redraw_required = false;
		extfunc_2->glViewport(0, 0, p_wid->width(), p_wid->height());
		extfunc_2->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
		
		extfunc_2->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//extfunc_2->glDisable(GL_DEPTH_TEST);
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
		if(p_config->use_osd_virtual_media) drawOsdIcons();
		extfunc_2->glFlush();
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
	if(w > using_flags->get_real_screen_width()) {
		w = using_flags->get_real_screen_width();
	}			
	if(h > using_flags->get_real_screen_height()) {
		h = using_flags->get_real_screen_height();
	}
	this->doSetGridsVertical(w, false);
	this->doSetGridsHorizonal(h, false);

}

void GLDraw_2_0::do_set_horiz_lines(int lines)
{
	if(lines > using_flags->get_real_screen_height()) {
		lines = using_flags->get_real_screen_height();
	}
	this->doSetGridsHorizonal(lines, false);
}

void GLDraw_2_0::do_set_led_width(int bitwidth)
{
	if((bitwidth >= 0) && (bitwidth < 32)) osd_led_bit_width = bitwidth;
//	printf("%d\n", bitwidth);
}
