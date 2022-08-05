
#include "osd_types.h"
#include "qt_gldraw.h"
#include "qt_glpack.h"
#include "./qt_glutil_gl_tmpl.h"
#include "csp_logger.h"
#include <QOpenGLTexture>
#include <QOpenGLFunctions_3_0>

#include <QColor>
#include <QImageReader>
#include <QRect>
#include <QOpenGLTexture>

GLDraw_Tmpl::GLDraw_Tmpl(GLDrawClass *parent, std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, EMU_TEMPLATE *emu) : QObject((QObject *)parent)
{
	p_wid = parent;
	p_emu = emu;
	using_flags = p;
	p_config = p->get_config_ptr();
	vert_lines = using_flags->get_real_screen_height();
	horiz_pixels = using_flags->get_real_screen_width();
	screen_texture_width = using_flags->get_screen_width();
	screen_texture_width_old = using_flags->get_screen_width();
	screen_texture_height = using_flags->get_screen_height();
	screen_texture_height_old = using_flags->get_screen_height();
		
	osd_pass = NULL;
	led_pass = NULL;
	uTmpTextureID = 0;
	ringing_phase = 0.0f;
		
	grids_shader = NULL;
	main_pass = NULL;
	std_pass = NULL;
	ntsc_pass1 = NULL;
	ntsc_pass2 = NULL;
	grids_horizonal_buffer = NULL;
	grids_horizonal_vertex = NULL;
	
	grids_vertical_buffer = NULL;
	grids_vertical_vertex = NULL;
	for(int i = 0; i < 32; i++) {
		led_pass_vao[i] = NULL;
		led_pass_vbuffer[i] = NULL;
		osd_pass_vao[i] = NULL;
		osd_pass_vbuffer[i] = NULL;
	}

#if 0		
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
	if(using_flags->is_use_compact_disc()) {
		osd_led_bit_width = 26;
	}
	if(using_flags->is_use_laser_disc()) {
		osd_led_bit_width = 28;
	}
#else
	osd_led_bit_width = 32;
#endif		
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

GLDraw_Tmpl::~GLDraw_Tmpl()
{
	if(main_pass  != NULL) delete main_pass;
	if(std_pass   != NULL) delete std_pass;
	if(ntsc_pass1 != NULL) delete ntsc_pass1;
	if(ntsc_pass2 != NULL) delete ntsc_pass2;
	
	if(grids_horizonal_buffer != NULL) {
		if(grids_horizonal_buffer->isCreated()) grids_horizonal_buffer->destroy();
	}
	if(grids_horizonal_vertex != NULL) {
		if(grids_horizonal_vertex->isCreated()) grids_horizonal_vertex->destroy();
	}
	if(grids_vertical_buffer != NULL) {
		if(grids_vertical_buffer->isCreated()) grids_vertical_buffer->destroy();
	}
	if(grids_horizonal_vertex != NULL) {
		if(grids_vertical_vertex->isCreated()) grids_vertical_vertex->destroy();
	}
	if(osd_pass   != NULL) delete osd_pass;
	if(led_pass   != NULL) delete led_pass;
	for(int i = 0; i < 32; i++) {
		if(led_pass_vao[i] != NULL) delete led_pass_vao[i];	
		if(led_pass_vbuffer[i] != NULL) delete led_pass_vbuffer[i];
		if(osd_pass_vao[i] != NULL) delete osd_pass_vao[i];	
		if(osd_pass_vbuffer[i] != NULL) delete osd_pass_vbuffer[i];
	}
}

void GLDraw_Tmpl::initBitmapVertex(void)
{
	if(using_flags->is_use_one_board_computer()) {
		vertexBitmap[0].x = -1.0f;
		vertexBitmap[0].y = -1.0f;
		vertexBitmap[0].z = 0.5f;
		vertexBitmap[0].s = 0.0f;
		vertexBitmap[0].t = 1.0f;
		
		vertexBitmap[1].x = +1.0f;
		vertexBitmap[1].y = -1.0f;
		vertexBitmap[1].z = 0.5f;
		vertexBitmap[1].s = 1.0f;
		vertexBitmap[1].t = 1.0f;
		
		vertexBitmap[2].x = +1.0f;
		vertexBitmap[2].y = +1.0f;
		vertexBitmap[2].z = 0.5f;
		vertexBitmap[2].s = 1.0f;
		vertexBitmap[2].t = 0.0f;
		
		vertexBitmap[3].x = -1.0f;
		vertexBitmap[3].y = +1.0f;
		vertexBitmap[3].z = 0.5f;
		vertexBitmap[3].s = 0.0f;
		vertexBitmap[3].t = 0.0f;
		
	}
}

void GLDraw_Tmpl::initButtons(void)
{
	button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
	QOpenGLContext *context = QOpenGLContext::currentContext();
	QPair<int, int> _version = QOpenGLVersionProfile(context->format()).version();
	QString versionext = QString::fromUtf8("");
	
	if(context->isOpenGLES()) {
		if(((_version.first == 3) && (_version.second >= 1)) || (_version.first >= 4)){
			versionext = QString::fromUtf8("#version 310 es \n");
		} /* else if((_version.first == 3)) {
			 _ext = _ext + QString::fromUtf8("#version 300 es \n");
			 } */ else {
			versionext = QString::fromUtf8("#version 100 \n");
		}
	} else {
		if(((_version.first == 4) && (_version.second >= 3)) || (_version.first >= 5)) {
			versionext = QString::fromUtf8("#version 430 core \n"); // OK?
		} else if(_version.first == 4) {
			versionext = QString::fromUtf8("#version 400 core \n");
		} else if((_version.first == 3) && (_version.second >= 3)) {
			versionext = QString::fromUtf8("#version 330 core \n");
		} else if(_version.first == 3) {
			versionext = QString::fromUtf8("#version 130 \n");
		} else { // Require GLVersion >= 2
			versionext = QString::fromUtf8("#version 120 \n");
		}
	}	
	if(vm_buttons_d != NULL) {
		button_shader = new QOpenGLShaderProgram(p_wid);
		if(button_shader != NULL) {
			bool f = false;
			QFile vertex_src(QString::fromUtf8(":/gl/shaders/vertex_shader.glsl"));
			if (vertex_src.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QString srcs = versionext;
				srcs = srcs + QString::fromUtf8(vertex_src.readAll());
				f  = button_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, srcs);
				vertex_src.close();
			} else {
				return;
			}
			QFile fragment_src(QString::fromUtf8(":/gl/shaders/normal_fragment_shader.glsl"));
			if (fragment_src.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QString srcs = versionext;
				srcs = srcs + QString::fromUtf8(fragment_src.readAll());
				f &= button_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, srcs);
				fragment_src.close();
			} else {
				return;
			}
			if(!f) return;
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

void GLDraw_Tmpl::drawLedMain(GLScreenPack *obj, int num, QVector4D color)
{
	QOpenGLShaderProgram *prg = obj->getShader();
	QOpenGLVertexArrayObject *vp = led_pass_vao[num];
	QOpenGLBuffer *bp = led_pass_vbuffer[num];
	int ii;
		
	{
		prologueBlending();
		vp->bind();
		bp->bind();
		prg->bind();
		
		ii = prg->uniformLocation("color");
		if(ii >= 0) {
			prg->setUniformValue(ii,  color);
		}
		
		prg->enableAttributeArray("vertex");
		int vertex_loc = prg->attributeLocation("vertex");
		prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
		drawPolygon(vertex_loc);
		bp->release();
		vp->release();
		
		prg->release();
		epilogueBlending();
	}
}

void GLDraw_Tmpl::drawOsdLeds()
{
	QVector4D color_on;
	QVector4D color_off;
	uint32_t bit = 0x00000001;
	if(osd_onoff) {
		color_on = QVector4D(0.95, 0.0, 0.05, 1.0);
		color_off = QVector4D(0.05,0.05, 0.05, 0.10);
	} else {
		color_on = QVector4D(0.00,0.00, 0.00, 0.0);
		color_off = QVector4D(0.00,0.00, 0.00, 0.0);
	}
	if(osd_onoff) {
		//if(osd_led_status_bak != osd_led_status) {
			for(int i = 0; i < osd_led_bit_width; i++) {
				if((bit & osd_led_status) == (bit & osd_led_status_bak)) {
					bit <<= 1;
					continue;
				}
				drawLedMain(led_pass, i,
							((osd_led_status & bit) != 0) ? color_on : color_off);
				bit <<= 1;
			}
			osd_led_status_bak = osd_led_status;
		//}
	}
}

void GLDraw_Tmpl::drawOsdIcons()
{
	QVector4D color_on;
	QVector4D color_off;
	uint32_t bit = 0x00000001;
	if(osd_onoff) {
		color_on = QVector4D(1.0,  1.0, 1.0, 0.8);
		color_off = QVector4D(1.0, 1.0, 1.0, 0.00);
	} else {
		color_on = QVector4D(0.00,0.00, 0.00, 0.0);
		color_off = QVector4D(0.00,0.00, 0.00, 0.0);
	}
	if(osd_onoff) {
		int major, minor;
		//if(osd_led_status_bak != osd_led_status) {
			for(int i = 0; i < osd_led_bit_width; i++) {
				if((bit & osd_led_status) == (bit & osd_led_status_bak)) {
					if((bit & osd_led_status) == 0) { 
						bit <<= 1;
						continue;
					}
				}
				if((i >= 2) && (i < 10)) { // FD
					major = 2;
					minor = i - 2;
				} else if((i >= 10) && (i < 12)) { // QD
					major = 3;
					minor = i - 10;
				} else if((i >= 12) && (i < 14)) { // CMT(R)
					major = 4;
					minor = i - 12;
				} else if((i >= 14) && (i < 16)) { // CMT(W)
					major = 5;
					minor = i - 14;
				} else if((i >= 16) && (i < 24)) { // HDD
					major = 8;
					minor = i - 16;
				} else if((i >= 24) && (i < 26)) { // CD
					major = 6;
					minor = i - 24;
				} else if((i >= 26) && (i < 28)) { // LD
					major = 7;
					minor = i - 26;
				} else {
					major = 0;
					minor = 0;
				}
				// ToDo: CD(6),LD(7) and HDD(8).

				if((major != 0) && (icon_texid[major][minor] != NULL)) {
					if(icon_texid[major][minor] != NULL) {
						drawMain(osd_pass->getShader(), osd_pass_vao[i], osd_pass_vbuffer[i],
								 icon_texid[major][minor]->textureId(),
								 ((osd_led_status & bit) != 0) ? color_on : color_off,
								 false, false, QVector3D(0.0, 0.0, 0.0));
					}
				}
 				bit <<= 1;
			}
			osd_led_status_bak = osd_led_status;
		//}
	}
}


void GLDraw_Tmpl::updateButtonTexture(void)
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

void GLDraw_Tmpl::uploadBitmapTexture(QImage *p)
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

void GLDraw_Tmpl::updateBitmap(QImage *p)
{
	if(!using_flags->is_use_one_board_computer()) return;
	redraw_required = true;
	bitmap_uploaded = false;
	uploadBitmapTexture(p);
}

void GLDraw_Tmpl::uploadIconTexture(QPixmap *p, int icon_type, int localnum)
{
	if((icon_type >  7) || (icon_type < 0)) return;
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
// Slots
void GLDraw_Tmpl::doSetGridsHorizonal(int lines, bool force)
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

void GLDraw_Tmpl::doSetGridsVertical(int pixels, bool force) 
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

void GLDraw_Tmpl::do_set_display_osd(bool onoff)
{
	osd_onoff = onoff;
}

void GLDraw_Tmpl::do_display_osd_leds(int lednum, bool onoff)
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
