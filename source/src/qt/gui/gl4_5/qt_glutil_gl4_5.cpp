/*
 * qt_glutil_gl3_0.cpp
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL v3.0 (extend from renderer with OpenGL v2.0).
 * History:
 * Jan 22, 2016 : Initial.
 */

#include "osd_types.h"
#include "qt_gldraw.h"
#include "qt_glpack.h"
#include "qt_glutil_gl4_5.h"
#include "csp_logger.h"
#include "menu_flags.h"

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QImage>
#include <QImageReader>
#include <QMatrix4x4>
#include <QOpenGLPixelTransferOptions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLPixelTransferOptions>

#include <QMatrix4x2>
#include <QMatrix4x4>

#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMutex>

#include <QOpenGLFunctions_4_5_Core>

//extern USING_FLAGS *using_flags;

GLDraw_4_5::GLDraw_4_5(GLDrawClass *parent, USING_FLAGS *p, CSP_Logger *logger, EMU_TEMPLATE *emu) : GLDraw_Tmpl(parent, p, logger, emu)
{
	uTmpTextureID = 0;
	
	grids_shader = NULL;
	
	main_pass = NULL;
	std_pass = NULL;
	ntsc_pass1 = NULL;
	ntsc_pass2 = NULL;
	led_pass = NULL;
	for(int i = 0; i < 32; i++) {
		led_pass_vao[i] = NULL;
		led_pass_vbuffer[i] = NULL;
		osd_pass_vao[i] = NULL;
		osd_pass_vbuffer[i] = NULL;
	}
	grids_horizonal_buffer = NULL;
	grids_horizonal_vertex = NULL;
	
	grids_vertical_buffer = NULL;
	grids_vertical_vertex = NULL;
	ringing_phase = 0.0f;
#if defined(__LITTLE_ENDIAN__)
	swap_byteorder = true;
#else
	swap_byteorder = false;
#endif
	pixel_width = 0;
	pixel_height = 0;
	main_texture_buffer = 0;
	main_read_texture_buffer = 0;
	map_base_address = NULL;
	main_mutex = new QMutex();
	main_texture_ready = false;
	sync_fence = 0;

	// ToDo
	screen_texture_width = -1 ;
	screen_texture_height = -1;

}

GLDraw_4_5::~GLDraw_4_5()
{

	// 20200812 K.O: MUST WAIT when changing texture feature.
	extfunc->glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	extfunc->glDeleteSync(sync_fence);
	sync_fence = extfunc->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,  0);
	
	if(main_pass  != NULL) delete main_pass;
	if(std_pass   != NULL) delete std_pass;
	if(ntsc_pass1 != NULL) delete ntsc_pass1;
	if(ntsc_pass2 != NULL) delete ntsc_pass2;
	if(led_pass   != NULL) delete led_pass;
	for(int i = 0; i < 32; i++) {
		if(led_pass_vao[i] != NULL) delete led_pass_vao[i];	
		if(led_pass_vbuffer[i] != NULL) delete led_pass_vbuffer[i];
		if(osd_pass_vao[i] != NULL) delete osd_pass_vao[i];	
		if(osd_pass_vbuffer[i] != NULL) delete osd_pass_vbuffer[i];
	}
	
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
}

QOpenGLTexture *GLDraw_4_5::createMainTexture(QImage *img)
{
	QOpenGLTexture *tx;
	QImage *ip = NULL;
	int w;
	int h;
	
	if(img == NULL) {
		w = using_flags->get_real_screen_width();
		h = using_flags->get_real_screen_height();
//		return NULL;
	} else {
		w = img->width();
		h = img->height();
	}
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SCREEN, "createMainTexture(): WxH: %dx%d\n",w, h);
	if((w <= 0) || (h <= 0)) return NULL;
	QImage *im = NULL;
	if(img == NULL) {
		im = new QImage(w, h, QImage::Format_RGBA8888);
		ip = im;
	} else {
		ip = img;
	}
	//tx->setFormat(QOpenGLTexture::RGBA8_UNorm);
	
	if(main_texture_buffer != 0) {
		this->unmap_vram_texture();
		main_texture_ready = false;
	}
	{
		QMutexLocker Locker_S(main_mutex);
		if(sync_fence != 0) {
			extfunc->glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
			extfunc->glDeleteSync(sync_fence);
		}
		sync_fence = extfunc->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,  0);
		
		tx = new QOpenGLTexture(QOpenGLTexture::Target2D);
		tx->setFormat(QOpenGLTexture::RGBA8_UNorm);
		tx->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Nearest);
		tx->setWrapMode(QOpenGLTexture::ClampToEdge);
		tx->setData(*ip, QOpenGLTexture::DontGenerateMipMaps);
		tx->bind();
		extfunc->glGenBuffers(1, &main_texture_buffer);
		extfunc->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, main_texture_buffer);
		extfunc->glBufferStorage(GL_PIXEL_UNPACK_BUFFER,
								 w * h * sizeof(uint32_t), ip->constBits(),
								 GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		extfunc->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		tx->release();

		tx->bind();
		extfunc->glGenBuffers(1, &main_read_texture_buffer);
		extfunc->glBindBuffer(GL_PIXEL_PACK_BUFFER, main_read_texture_buffer);
		extfunc->glBufferStorage(GL_PIXEL_PACK_BUFFER, w * h * sizeof(uint32_t), ip->constBits(),  GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		extfunc->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		tx->release();
		
//		extfunc->glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
//		extfunc->glDeleteSync(sync_fence);
//		sync_fence = extfunc->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,  0);
		
		pixel_width = w;
		pixel_height = h;
	}
	if(map_vram_texture()) {
		main_texture_ready = true;
	}			
	if(im != NULL) delete im;
	return tx;
}

void GLDraw_4_5::initBitmapVertex(void)
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

void GLDraw_4_5::initFBO(void)
{
	glHorizGrids = (GLfloat *)malloc(sizeof(float) * (using_flags->get_real_screen_height() + 2) * 6);

	if(glHorizGrids != NULL) {
		doSetGridsHorizonal(using_flags->get_real_screen_height(), true);
	}
	glVertGrids  = (GLfloat *)malloc(sizeof(float) * (using_flags->get_real_screen_width() + 2) * 6);
	if(glVertGrids != NULL) {
		doSetGridsVertical(using_flags->get_real_screen_width(), true);
	}
	if(using_flags->get_max_button() > 0) {
		initButtons();
	}
	// Init view
	extfunc->glClearColor(0.0, 0.0, 0.0, 1.0);

	sync_fence = extfunc->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,  0);
}

void GLDraw_4_5::setNormalVAO(QOpenGLShaderProgram *prg,
							   QOpenGLVertexArrayObject *vp,
							   QOpenGLBuffer *bp,
							   VertexTexCoord_t *tp,
							   int size)
{
	int vertex_loc = prg->attributeLocation("vertex");
	int texcoord_loc = prg->attributeLocation("texcoord");

	vp->bind();
	bp->bind();

	if(tp == NULL) {
	} else {
		bp->write(0, tp, sizeof(VertexTexCoord_t) * size);
	}
	prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
	prg->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
	prg->setUniformValue("a_texture", 0);
			   
	extfunc->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord_t), 0); 
	extfunc->glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord_t), 
							       (char *)NULL + 3 * sizeof(GLfloat)); 
	bp->release();
	vp->release();
	prg->enableAttributeArray(vertex_loc);
	prg->enableAttributeArray(texcoord_loc);
}

void GLDraw_4_5::initGLObjects()
{
	extfunc = new QOpenGLFunctions_4_5_Core();
	extfunc->initializeOpenGLFunctions();
	extfunc->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture_max_size);
}	

void GLDraw_4_5::initPackedGLObject(GLScreenPack **p,
									int _width, int _height,
									const QString vertex_shader, const QString fragment_shader,
									const QString _name, bool req_float, bool req_highp, bool req_alpha_channel)
{
	QString s;
	GLScreenPack *pp;
	if(p != NULL) {
		pp = new GLScreenPack(_width, _height, _name,  p_wid, req_float, req_highp, req_alpha_channel);
		*p = pp;
		if(pp != NULL) {
			pp->initialize(_width, _height, vertex_shader, fragment_shader);
			s = pp->getShaderLog();
			if(s.size() > 0) {
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GL_SHADER, "In shader of %s ", _name.toLocal8Bit().constData());
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GL_SHADER, "Vertex: %s ",  vertex_shader.toLocal8Bit().constData());
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GL_SHADER, "Fragment: %s ", fragment_shader.toLocal8Bit().constData());
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GL_SHADER, "%s", s.toLocal8Bit().constData());
			}
			s = pp->getGLLog();
			if(s.size() > 0) {
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GL_SHADER, "In shader of %s ", _name.toLocal8Bit().constData());
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GL_SHADER, "Vertex: %s ",  vertex_shader.toLocal8Bit().constData());
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GL_SHADER, "Fragment: %s ", fragment_shader.toLocal8Bit().constData());
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GL_SHADER, "%s", s.toLocal8Bit().constData());
			}
			pp->clearGLLog();
		}
	}
	QOpenGLContext *context = QOpenGLContext::currentContext();
	gl_major_version = context->format().version().first;
	gl_minor_version = context->format().version().second;

}
					


bool GLDraw_4_5::initGridShaders(const QString vertex_fixed, const QString vertex_rotate, const QString fragment)
{
	QOpenGLContext *context = QOpenGLContext::currentContext();
	QPair<int, int> _version = QOpenGLVersionProfile(context->format()).version();
	QString versionext = QString::fromUtf8("");
	if(((_version.first == 4) && (_version.second >= 3)) || (_version.first >= 5)) {
		versionext = QString::fromUtf8("#version 430 core \n"); // OK?
	} else if((_version.first == 4)) {
		versionext = QString::fromUtf8("#version 400 core \n");
	} else { // Require GLVersion >= 3.2
		versionext = QString::fromUtf8("#version 150 \n");
	}
	bool f = false;
	
	grids_shader = new QOpenGLShaderProgram(p_wid);
	if(grids_shader == NULL) return false;
	
	QFile vertex_src(vertex_rotate);
	if (vertex_src.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QString srcs = versionext;
		srcs = srcs + QString::fromUtf8(vertex_src.readAll());
		f  = grids_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, srcs);
		vertex_src.close();
	} else {
		return false;
	}
	
	QFile fragment_src(fragment);
	if ((fragment_src.open(QIODevice::ReadOnly | QIODevice::Text)) && (f)){
		QString _src;
		QString _ext = QString::fromUtf8("");;
		_ext = versionext;
		_src = QString::fromUtf8(fragment_src.readAll());
		f &= grids_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, _ext + _src);
		fragment_src.close();
	} else {
		return false;
	}

	f &= grids_shader->link();
	return f;
}

bool GLDraw_4_5::initGridVertexObject(QOpenGLBuffer **vbo, QOpenGLVertexArrayObject **vao, int alloc_size)
{
	QOpenGLBuffer *bp = NULL;
	QOpenGLVertexArrayObject *ap = NULL;
	*vao = NULL;
	*vbo = NULL;
	*vbo  = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	bp = *vbo;
	if(bp != NULL) {
		if(bp->create()) {
			bp->bind();
			bp->allocate(alloc_size * sizeof(GLfloat) * 3 * 2);
			bp->release();
		} else {
			delete *vbo;
			return false;
		}
	} else {
		return false;
	}
	
	*vao = new QOpenGLVertexArrayObject;
	ap = *vao;
	if(ap == NULL) {
		bp->destroy();
		delete *vbo;
		return false;
	}
	if(!ap->create()) {
		delete *vao;
		bp->destroy();
		delete *vbo;
		return false;
	}
	return true;
}


void GLDraw_4_5::initLocalGLObjects(void)
{

	int _width = using_flags->get_screen_width();
	int _height = using_flags->get_screen_height();
	
	if(((int)(_width * 4)) <= texture_max_size) {
		_width = (int)(_width * 4);
		low_resolution_screen = true;
	} else {
		_width = _width * 2;
	}
	p_wid->makeCurrent();
	
	vertexFormat[0].x = -1.0f;
	vertexFormat[0].y = -1.0f;
	vertexFormat[0].z = -0.9f;
	vertexFormat[0].s = 0.0f;
	vertexFormat[0].t = 1.0f;
	
	vertexFormat[1].x = +1.0f;
	vertexFormat[1].y = -1.0f;
	vertexFormat[1].z = -0.9f;
	vertexFormat[1].s = 1.0f;
	vertexFormat[1].t = 1.0f;
	
	vertexFormat[2].x = +1.0f;
	vertexFormat[2].y = +1.0f;
	vertexFormat[2].z = -0.9f;
	vertexFormat[2].s = 1.0f;
	vertexFormat[2].t = 0.0f;
	
	vertexFormat[3].x = -1.0f;
	vertexFormat[3].y = +1.0f;
	vertexFormat[3].z = -0.9f;
	vertexFormat[3].s = 0.0f;
	vertexFormat[3].t = 0.0f;

	if(using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0)) {
		initPackedGLObject(&main_pass,
						   using_flags->get_screen_width() * 2, using_flags->get_screen_height() * 2,
						   ":/gl4_5/vertex_shader.glsl" , ":/gl4_5/chromakey_fragment_shader2.glsl",
						   "Main Shader", true, false, true);
	} else {
		initPackedGLObject(&main_pass,
						   using_flags->get_screen_width() * 2, using_flags->get_screen_height() * 2,
						   ":/gl4_5/vertex_shader.glsl" , ":/gl4_5/fragment_shader.glsl",
						   "Main Shader", false, false, true);
	}		
	if(main_pass != NULL) {
		setNormalVAO(main_pass->getShader(), main_pass->getVAO(),
					 main_pass->getVertexBuffer(),
					 vertexFormat, 4);
	}
#if 0
	initPackedGLObject(&std_pass,
					   using_flags->get_screen_width(), using_flags->get_screen_height(),
					   ":/gl4_5/vertex_shader.glsl" , ":/gl4_5/chromakey_fragment_shader.glsl",
					   "Standard Shader", true, true, true);
#endif
	initPackedGLObject(&led_pass,
					   10, 10,
					   ":/gl4_5/led_vertex_shader.glsl" , ":/gl4_5/led_fragment_shader.glsl",
					   "LED Shader", true, false, true);
	for(int i = 0; i < 32; i++) {
		led_pass_vao[i] = new QOpenGLVertexArrayObject;
		led_pass_vbuffer[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		if(led_pass_vao[i]->create()) {
			if(led_pass_vbuffer[i]->create()) {
				led_pass_vbuffer[i]->setUsagePattern(QOpenGLBuffer::DynamicDraw);
				led_pass_vao[i]->bind();
				led_pass_vbuffer[i]->bind();
				led_pass_vbuffer[i]->allocate(sizeof(VertexTexCoord_t) * 4);
				led_pass_vbuffer[i]->release();
				led_pass_vao[i]->release();
				set_led_vertex(i);
			}
		}
	}
	initPackedGLObject(&osd_pass,
					   48.0, 48.0,
					   ":/gl4_5/vertex_shader.glsl" , ":/gl4_5/icon_fragment_shader.glsl",
					   "OSD Shader", false, false, true);
	for(int i = 0; i < 32; i++) {
		osd_pass_vao[i] = new QOpenGLVertexArrayObject;
		osd_pass_vbuffer[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		if(osd_pass_vao[i]->create()) {
			if(osd_pass_vbuffer[i]->create()) {
				osd_pass_vbuffer[i]->setUsagePattern(QOpenGLBuffer::DynamicDraw);
				osd_pass_vao[i]->bind();
				osd_pass_vbuffer[i]->bind();
				osd_pass_vbuffer[i]->allocate(sizeof(VertexTexCoord_t) * 4);
				osd_pass_vbuffer[i]->release();
				osd_pass_vao[i]->release();
				set_osd_vertex(i);
			}
		}
	}
#if 1

	initPackedGLObject(&ntsc_pass1,
//					   using_flags->get_screen_width() * 2, using_flags->get_screen_height() * 2,
					   _width, _height,
					   ":/gl4_5/vertex_shader.glsl" , ":/gl4_5/ntsc_pass1.glsl",
					   "NTSC Shader Pass1", true, false, false);
	initPackedGLObject(&ntsc_pass2,
//					   using_flags->get_screen_width() * 2, using_flags->get_screen_height() * 2,
					   _width / 2, _height,
					   ":/gl4_5/vertex_shader.glsl" , ":/gl4_5/ntsc_pass2.glsl",
					   "NTSC Shader Pass2", false, false, true);
	if(!(((gl_major_version >= 3) && (gl_minor_version >= 1)) || (gl_major_version >= 4))){
		int ii;
		QOpenGLShaderProgram *shader = ntsc_pass2->getShader();
		shader->bind();
		ii = shader->uniformLocation("luma_filter");
		if(ii >= 0) {
			shader->setUniformValueArray(ii, luma_filter, 24 + 1, 1);
		}
		ii = shader->uniformLocation("chroma_filter");
		if(ii >= 0) {
			shader->setUniformValueArray(ii, chroma_filter, 24 + 1, 1);
		}
		shader->release();
	}

#endif   
	if(using_flags->is_use_one_board_computer()) {
		initBitmapVertex();
		initPackedGLObject(&bitmap_block,
						   _width * 2, _height * 2,
						   ":/gl4_5/vertex_shader.glsl", ":/gl4_5/normal_fragment_shader.glsl",
						   "Background Bitmap Shader", true, true, true);
		if(bitmap_block != NULL) {
			setNormalVAO(bitmap_block->getShader(), bitmap_block->getVAO(),
						 bitmap_block->getVertexBuffer(),
						 vertexBitmap, 4);
		}
	}

	initGridShaders(":/gl4_5/grids_vertex_shader_fixed.glsl", ":/gl4_5/grids_vertex_shader.glsl", ":/gl4_5/grids_fragment_shader.glsl");
	
	initGridVertexObject(&grids_horizonal_buffer, &grids_horizonal_vertex, using_flags->get_real_screen_height() + 3);
	doSetGridsHorizonal(using_flags->get_real_screen_height(), true);
	
	initGridVertexObject(&grids_vertical_buffer, &grids_vertical_vertex, using_flags->get_real_screen_width() + 3);
	doSetGridsVertical(using_flags->get_real_screen_width(), true);

	do_set_texture_size(NULL, -1, -1);
	p_wid->doneCurrent();
}

void GLDraw_4_5::updateGridsVAO(QOpenGLBuffer *bp,
								QOpenGLVertexArrayObject *vp,
								GLfloat *tp,
								int number)

{
	bool checkf = false;
	if((bp != NULL) && (vp != NULL)) {
		if(bp->isCreated()) {
			if(bp->size() < (int)((number + 1) * sizeof(GLfloat) * 3 * 2)) {
				bp->destroy();
				bp->create();
				checkf = true;
			}
		} else {
			bp->create();
			checkf = true;
		}
		vp->bind();
		bp->bind();
		if(checkf) {
			bp->allocate((number + 1) * sizeof(GLfloat) * 3 * 2);
		}
		if(tp != NULL) {
			bp->write(0, tp, number * sizeof(GLfloat) * 3 * 2);
		}
		bp->release();
		vp->release();
	}
}

void GLDraw_4_5::drawGrids(void)
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

void GLDraw_4_5::drawGridsMain(QOpenGLShaderProgram *prg,
								 QOpenGLBuffer *bp,
								 QOpenGLVertexArrayObject *vp,
								 int number,
								 GLfloat lineWidth,
								 QVector4D color)
{
	if(number <= 0) return;
	extfunc->glDisable(GL_DEPTH_TEST);
	extfunc->glDisable(GL_BLEND);

	if((bp == NULL) || (vp == NULL) || (prg == NULL)) return;
	if((!bp->isCreated()) || (!vp->isCreated()) || (!prg->isLinked())) return;
	{
		bp->bind();
		vp->bind();
		prg->bind();
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
		prg->setUniformValue("color", color);
		prg->enableAttributeArray("vertex");
		int vertex_loc = prg->attributeLocation("vertex");
		
		extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());
		extfunc->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0); 
		extfunc->glEnableVertexAttribArray(vertex_loc);
		
		extfunc->glLineWidth(lineWidth);
		extfunc->glDrawArrays(GL_LINES, 0, (number + 1) * 2);
		prg->release();
		vp->release();
		bp->release();
	}
}

void GLDraw_4_5::doSetGridsVertical(int pixels, bool force)
{
	GLDraw_Tmpl::doSetGridsVertical(pixels, force);
	updateGridsVAO(grids_vertical_buffer,
				   grids_vertical_vertex,
				   glVertGrids,
				   pixels);
	
}
void GLDraw_4_5::doSetGridsHorizonal(int lines, bool force)
{
	if((lines == vert_lines) && !force) return;
	GLDraw_Tmpl::doSetGridsHorizonal(lines, force);

	updateGridsVAO(grids_horizonal_buffer,
				   grids_horizonal_vertex,
				   glHorizGrids,
				   lines);
}

void GLDraw_4_5::drawGridsHorizonal(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	drawGridsMain(grids_shader,
					grids_horizonal_buffer,
					grids_horizonal_vertex,
					vert_lines,
					0.15f,
					c);
}

void GLDraw_4_5::drawGridsVertical(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	drawGridsMain(grids_shader,
					grids_vertical_buffer,
					grids_vertical_vertex,
					horiz_pixels,
					0.5f,
					c);
}

void GLDraw_4_5::renderToTmpFrameBuffer_nPass(GLuint src_texture,
											  GLuint src_w,
											  GLuint src_h,
											  GLScreenPack *renderObject,
											  GLuint dst_w,
											  GLuint dst_h,
											  bool use_chromakey)
{
	{
		QOpenGLShaderProgram *shader = renderObject->getShader();
		int ii;
		// NTSC_PASSx

		extfunc->glClearColor(0.0, 0.0, 0.0, 1.0);
		extfunc->glClearDepthf(1.0f);
		extfunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		{
			if((src_texture != 0) && (shader != NULL)) {
				QMatrix4x4 ortho;
				//ortho.ortho(0.0f, (float)dst_w, 0.0f, (float)dst_h, -1.0, 1.0);
				ortho.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);


				renderObject->bind();
				extfunc->glViewport(0, 0, dst_w, dst_h);
				//extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
				extfunc->glActiveTexture(GL_TEXTURE0);
				extfunc->glBindTexture(GL_TEXTURE_2D, src_texture);
				extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				//extfunc->glColor4f(1.0, 1.0, 1.0, 1.0);
				shader->setUniformValue("a_texture", 0);
				shader->setUniformValue("v_ortho", ortho);
				//shader->setUniformValue("a_texture", src_texture);
				{
					ii = shader->uniformLocation("source_size");
					if(ii >= 0) {
						QVector4D source_size = QVector4D((float)src_w, (float)src_h, 0, 0);
						shader->setUniformValue(ii, source_size);
					}
					ii = shader->uniformLocation("target_size");
					if(ii >= 0) {
						QVector4D target_size = QVector4D((float)dst_w, (float)dst_h, 0, 0);
						shader->setUniformValue(ii, target_size);
					}
					ii = shader->uniformLocation("phase");
					if(ii >= 0) {
						ringing_phase = ringing_phase + 0.093;
						if(ringing_phase > 1.0) ringing_phase = ringing_phase - 1.0;
						shader->setUniformValue(ii,  ringing_phase);
					}
					QMatrix2x2 rot;
					/*
					 * Note : Not rotate within renderer.
					 */
					/*
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
					*/
					rot = QMatrix2x2(rot0);
					shader->setUniformValue("rotate_mat", rot);
					//if(!(((gl_major_version >= 3) && (gl_minor_version >= 1)) || (gl_major_version >= 4))){
						//ii = shader->uniformLocation("luma_filter");
						//if(ii >= 0) {
						//	shader->setUniformValueArray(ii, luma_filter, 24 + 1, 1);
						//}
						//ii = shader->uniformLocation("chroma_filter");
						//if(ii >= 0) {
						//	shader->setUniformValueArray(ii, chroma_filter, 24 + 1, 1);
						//}
					//}
				}
				{
					QVector4D c(fBrightR, fBrightG, fBrightB, 1.0);
					QVector3D chromakey(0.0, 0.0, 0.0);
		
					ii = shader->uniformLocation("color");
					if(ii >= 0) {
						shader->setUniformValue(ii, c);
					}
					ii = shader->uniformLocation("do_chromakey");
					if(ii >= 0) {
						if(use_chromakey) {
							int ij;
							ij = shader->uniformLocation("chromakey");
							if(ij >= 0) {
								shader->setUniformValue(ij, chromakey);
							}
							shader->setUniformValue(ii, GL_TRUE);
						} else {
							shader->setUniformValue(ii, GL_FALSE);
						}
					}
				}

				shader->enableAttributeArray("texcoord");
				shader->enableAttributeArray("vertex");
				
				int vertex_loc = shader->attributeLocation("vertex");
				int texcoord_loc = shader->attributeLocation("texcoord");
				shader->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
				shader->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
				extfunc->glEnableVertexAttribArray(vertex_loc);
				extfunc->glEnableVertexAttribArray(texcoord_loc);

				extfunc->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
				
				//extfunc->glViewport(0, 0, dst_w, dst_h);
				//extfunc->glOrtho(0.0f, (float)dst_w, 0.0f, (float)dst_h, -1.0, 1.0);
				renderObject->release();
				extfunc->glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
	}
}

void GLDraw_4_5::uploadMainTexture(QImage *p, bool use_chromakey, bool was_mapped)
{
	// set vertex
	redraw_required = true;
	//if(p == NULL) return;
	//redraw_required = true;
	imgptr = p;
	if(uVramTextureID == NULL) {
		uVramTextureID = createMainTexture(p);
	} else 
	{
		if((screen_texture_width <= 0) || (screen_texture_height <= 0)) return;

		// Upload to main texture
		bool is_dummy = false;
		
		if(((map_base_address == NULL) && (p == NULL)) && (main_texture_buffer != 0)){
				is_dummy = true;
		}
		if((map_base_address == NULL) && !(is_dummy)) {
			if(main_texture_buffer != 0) {
				extfunc->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, main_texture_buffer);
				uint32_t* pp = (uint32_t *)(extfunc->glMapBufferRange(GL_PIXEL_UNPACK_BUFFER,
																	  0,
																	  pixel_width *pixel_height * sizeof(scrntype_t),
																	  GL_MAP_WRITE_BIT ));
				int hh = (pixel_height < p->height()) ? pixel_height : p->height();
				int ww = (pixel_width < p->width()) ? pixel_width : p->width();
				if(pp != NULL) {
					for(int y = 0; y < hh; y++) {
						memcpy(&(pp[y * pixel_width]), p->scanLine(y), ww * sizeof(uint32_t));
					}
				}
				extfunc->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
				if(sync_fence != 0) {
					extfunc->glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
					extfunc->glDeleteSync(sync_fence);
				}
				sync_fence = extfunc->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,  0);
					
				extfunc->glBindTexture(GL_TEXTURE_2D, uVramTextureID->textureId());
				extfunc->glActiveTexture(GL_TEXTURE0);
				extfunc->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pixel_width, hh, GL_RGBA, GL_UNSIGNED_BYTE, 0);
				extfunc->glBindTexture(GL_TEXTURE_2D, 0);
				extfunc->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			}
		} else {
			// p == NULL
			main_mutex->lock();
			// Flush buffer range
			extfunc->glFlushMappedNamedBufferRange(main_texture_buffer, 0, pixel_width *pixel_height * sizeof(scrntype_t));
			if(sync_fence != 0) {
				extfunc->glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
				extfunc->glDeleteSync(sync_fence);
			}
			sync_fence = extfunc->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,  0);

			uVramTextureID->bind();
			extfunc->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, main_texture_buffer);
			extfunc->glActiveTexture(GL_TEXTURE0);
			extfunc->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pixel_width, pixel_height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			extfunc->glBindTexture(GL_TEXTURE_2D, 0);
			extfunc->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			main_mutex->unlock();
		}
	}
#if 1
	if(using_flags->is_support_tv_render() && (p_config->rendering_type == CONFIG_RENDER_TYPE_TV)) {
		renderToTmpFrameBuffer_nPass(uVramTextureID->textureId(),
									 screen_texture_width,
									 screen_texture_height,
									 ntsc_pass1,
									 ntsc_pass1->getViewportWidth(),
									 ntsc_pass1->getViewportHeight());
		
		renderToTmpFrameBuffer_nPass(ntsc_pass1->getTexture(),
									 ntsc_pass1->getViewportWidth(),
									 ntsc_pass1->getViewportHeight(),
									 ntsc_pass2,
									 ntsc_pass2->getViewportWidth(),
									 ntsc_pass2->getViewportHeight());
		uTmpTextureID = ntsc_pass2->getTexture();
	} else
#endif
	{
		uTmpTextureID = uVramTextureID->textureId();
	}
	crt_flag = true;
}

void GLDraw_4_5::drawScreenTexture(void)
{
	if(using_flags->is_use_one_board_computer()) {
		extfunc->glEnable(GL_BLEND);
		extfunc->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		extfunc->glDisable(GL_BLEND);
	}

	QVector4D color;
	smoosing = p_config->use_opengl_filters;
	if(set_brightness) {
		color = QVector4D(fBrightR, fBrightG, fBrightB, 1.0);
	} else {
		color = QVector4D(1.0, 1.0, 1.0, 1.0);
	}
	if(using_flags->is_use_one_board_computer()) {
		drawMain(main_pass,
				 uTmpTextureID, // v2.0
				 color, smoosing,
				 true, QVector3D(0.0, 0.0, 0.0));	
		extfunc->glDisable(GL_BLEND);
	} else {
		drawMain(main_pass,
				 uTmpTextureID, // v2.0
				 color, smoosing);	
	}
}

void GLDraw_4_5::drawMain(QOpenGLShaderProgram *prg,
						  QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  GLuint texid,
						  QVector4D color,
						  bool f_smoosing,
						  bool do_chromakey,
						  QVector3D chromakey)
{
	int ii;
	if((screen_texture_width <= 0) || (screen_texture_height <= 0)) return;

	if(sync_fence != 0) {
		extfunc->glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
		extfunc->glDeleteSync(sync_fence);
	}
	sync_fence = extfunc->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,  0);
   if(texid != 0) {
		vp->bind();
		bp->bind();
		prg->bind();
		extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());
		QMatrix4x4 ortho;
		ortho.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);

		extfunc->glActiveTexture(GL_TEXTURE0);
		extfunc->glBindTexture(GL_TEXTURE_2D, texid);
		//extfunc->glBindTexture(GL_PIXEL_UNPACK_BUFFER, texid);

		extfunc->glClearColor(1.0, 1.0, 1.0, 1.0);
		if(!f_smoosing) {
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		} else {
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		prg->setUniformValue("a_texture", 0);
		prg->setUniformValue("v_ortho", ortho);
		
		ii = prg->uniformLocation("color");
		if(ii >= 0) {
			prg->setUniformValue(ii,  color);
		}
		
		ii = prg->uniformLocation("tex_width");
		if(ii >= 0) {
			prg->setUniformValue(ii,  (float)screen_texture_width);
		}
		
		ii = prg->uniformLocation("tex_height");
		if(ii >= 0) {
			prg->setUniformValue(ii,  (float)screen_texture_height);
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
		//} else {
		//prg->setUniformValue("rotate", GL_FALSE);
		//}

		if(do_chromakey) {
			ii = prg->uniformLocation("chromakey");
			if(ii >= 0) {
				prg->setUniformValue(ii, chromakey);
			}
			ii = prg->uniformLocation("do_chromakey");
			if(ii >= 0) {
				prg->setUniformValue(ii, GL_TRUE);
			}
		} else {
			ii = prg->uniformLocation("do_chromakey");
			if(ii >= 0) {
				prg->setUniformValue(ii, GL_FALSE);
			}
		}
		
		prg->enableAttributeArray("texcoord");
		prg->enableAttributeArray("vertex");
		int vertex_loc = prg->attributeLocation("vertex");
		int texcoord_loc = prg->attributeLocation("texcoord");
		
		//prg->enableAttributeArray(vertex_loc);
		prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
		prg->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
		extfunc->glEnableVertexAttribArray(vertex_loc);
		extfunc->glEnableVertexAttribArray(texcoord_loc);
		
		extfunc->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		bp->release();
		vp->release();
		
		prg->release();
		extfunc->glBindTexture(GL_TEXTURE_2D, 0);
		//extfunc->glBindTexture(GL_TEXTURE_2D, 0);
   }
	else {
		vp->bind();
		bp->bind();
		prg->bind();
		extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());
		QMatrix4x4 ortho;
		ortho.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
		//extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
		ii = prg->uniformLocation("color");
		if(ii >= 0) {
			prg->setUniformValue(ii,  color);
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
		prg->setUniformValue("v_ortho", ortho);

		if(do_chromakey) {
			ii = prg->uniformLocation("chromakey");
			if(ii >= 0) {
				prg->setUniformValue(ii, chromakey);
			}
			ii = prg->uniformLocation("do_chromakey");
			if(ii >= 0) {
				prg->setUniformValue(ii, GL_TRUE);
			}
		} else {
			ii = prg->uniformLocation("do_chromakey");
			if(ii >= 0) {
				prg->setUniformValue(ii, GL_FALSE);
			}
		}
		
		prg->enableAttributeArray("vertex");
		int vertex_loc = prg->attributeLocation("vertex");
		extfunc->glEnableVertexAttribArray(vertex_loc);
		extfunc->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		bp->release();
		vp->release();
		prg->release();
		extfunc->glBindTexture(GL_TEXTURE_2D, 0);
	}
	
}

void GLDraw_4_5::drawMain(GLScreenPack *obj,
						  GLuint texid,
						  QVector4D color,
						  bool f_smoosing,
						  bool do_chromakey,
						  QVector3D chromakey)
						   
{
	QOpenGLShaderProgram *prg = obj->getShader();
	QOpenGLVertexArrayObject *vp = obj->getVAO();
	QOpenGLBuffer *bp = obj->getVertexBuffer();

	drawMain(prg, vp, bp, texid, color, f_smoosing, do_chromakey, chromakey);
}

void GLDraw_4_5::drawButtonsMain(int num, bool f_smoosing)
{
	GLuint texid = uButtonTextureID[num]->textureId();
	QOpenGLBuffer *bp = buffer_button_vertex[num];
	QOpenGLShaderProgram  *prg = button_shader;
	QOpenGLVertexArrayObject *vp = vertex_button[num];
	QVector4D color;
	int ii;
	
	color = QVector4D(1.0, 1.0, 1.0, 1.0);
	if((bp != NULL) && (vp != NULL) && (prg != NULL)) {
		if((bp->isCreated()) && (vp->isCreated()) && (prg->isLinked())) {
			bp->bind();
			vp->bind();
			prg->bind();
			QMatrix4x4 ortho;
			ortho.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
			extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());

			extfunc->glActiveTexture(GL_TEXTURE0);
			extfunc->glBindTexture(GL_TEXTURE_2D, texid);
			if(!f_smoosing) {
				extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			} else {
				extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			}
			prg->setUniformValue("a_texture", 0);
			prg->setUniformValue("v_ortho", ortho);
			
			ii = prg->uniformLocation("color");
			if(ii >= 0) {
				prg->setUniformValue(ii,  color);
			}
			ii = prg->uniformLocation("do_chromakey");
			if(ii >= 0) {
				prg->setUniformValue(ii, GL_FALSE);
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
			
			int vertex_loc = prg->attributeLocation("vertex");
			int texcoord_loc = prg->attributeLocation("texcoord");
			prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
			prg->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
			prg->enableAttributeArray(vertex_loc);
			prg->enableAttributeArray(texcoord_loc);
			//extfunc->glEnableVertexAttribArray(vertex_loc);
			//extfunc->glEnableVertexAttribArray(texcoord_loc);
			extfunc->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			bp->release();
			vp->release();
			prg->release();
			extfunc->glBindTexture(GL_TEXTURE_2D, 0);
			return;
			}
		}

}

void GLDraw_4_5::drawButtons(void)
{
	for(int i = 0; i < using_flags->get_max_button(); i++) {
		drawButtonsMain(i, false);
	}
}

void GLDraw_4_5::drawBitmapTexture(void)
{
	QVector4D color = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
	smoosing = p_config->use_opengl_filters;

	if(using_flags->is_use_one_board_computer() && (uBitmapTextureID != NULL)) {
		//extfunc->glDisable(GL_BLEND);
		drawMain(bitmap_block,
				 uBitmapTextureID->textureId(),
				 color, smoosing);
	}
}

void GLDraw_4_5::drawLedMain(GLScreenPack *obj, int num, QVector4D color)
{
	QOpenGLShaderProgram *prg = obj->getShader();
	QOpenGLVertexArrayObject *vp = led_pass_vao[num];
	QOpenGLBuffer *bp = led_pass_vbuffer[num];
	int ii;
		
	{
		extfunc->glEnable(GL_BLEND);
		extfunc->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		vp->bind();
		bp->bind();
		prg->bind();
		extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());
		//extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
		ii = prg->uniformLocation("color");
		if(ii >= 0) {
			prg->setUniformValue(ii,  color);
		}
		
		prg->enableAttributeArray("vertex");
		int vertex_loc = prg->attributeLocation("vertex");
		prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
		extfunc->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord_t), 0); 

		extfunc->glEnableVertexAttribArray(vertex_loc);
		extfunc->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		bp->release();
		vp->release();
		
		prg->release();
		extfunc->glDisable(GL_BLEND);
	}

}


void GLDraw_4_5::drawOsdLeds()
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

void GLDraw_4_5::drawOsdIcons()
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
				} else {
					major = 0;
					minor = 0;
				}
				if(major != 0) {
					drawMain(osd_pass->getShader(), osd_pass_vao[i], osd_pass_vbuffer[i],
							 icon_texid[major][minor]->textureId(),
							 ((osd_led_status & bit) != 0) ? color_on : color_off,
							 false, false, QVector3D(0.0, 0.0, 0.0));
				}
 				bit <<= 1;
			}
			osd_led_status_bak = osd_led_status;
		//}
	}
}

void GLDraw_4_5::paintGL(void)
{
	//p_wid->makeCurrent();
	
//	if(crt_flag || redraw_required) { //return;
		if(emu_launched) {
			crt_flag = false;
		}
		redraw_required = false;
		extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());
		//extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);

		extfunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		extfunc->glDisable(GL_DEPTH_TEST);
		extfunc->glDisable(GL_BLEND);
		if(using_flags->is_use_one_board_computer() || using_flags->is_use_bitmap()) {
			extfunc->glEnable(GL_BLEND);
			drawBitmapTexture();
		}
		if(using_flags->get_max_button() > 0) {
			extfunc->glEnable(GL_BLEND);
			drawButtons();
		}
		drawScreenTexture();
		extfunc->glEnable(GL_BLEND);
		extfunc->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		extfunc->glDisable(GL_DEPTH_TEST);
		//drawOsdLeds();
		if(p_config->use_osd_virtual_media) drawOsdIcons();
		extfunc->glDisable(GL_BLEND);
		if(!using_flags->is_use_one_board_computer() && (using_flags->get_max_button() <= 0)) {
			drawGrids();
		}
		extfunc->glFlush();
}

void GLDraw_4_5::setBrightness(GLfloat r, GLfloat g, GLfloat b)
{
	fBrightR = r;
	fBrightG = g;
	fBrightB = b;

	if(imgptr != NULL) {
		p_wid->makeCurrent();
		if(uVramTextureID == NULL) {
			uVramTextureID = createMainTexture(imgptr);
		}
//		if(using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0)) {
//			uploadMainTexture(imgptr, true);
//		} else {
//			uploadMainTexture(imgptr, false);
//		}
		p_wid->doneCurrent();
	}
	crt_flag = true;
}

void GLDraw_4_5::set_texture_vertex(float wmul, float hmul)
{
	float wfactor = 1.0f;
	float hfactor = 1.0f;

	vertexTmpTexture[0].x = -1.0f;
	vertexTmpTexture[0].y = -1.0f;
	vertexTmpTexture[0].z = -0.1f;
	vertexTmpTexture[0].s = 0.0f;
	vertexTmpTexture[0].t = 0.0f;
	
	vertexTmpTexture[1].x = wfactor;
	vertexTmpTexture[1].y = -1.0f;
	vertexTmpTexture[1].z = -0.1f;
	vertexTmpTexture[1].s = wmul;
	vertexTmpTexture[1].t = 0.0f;
	
	vertexTmpTexture[2].x = wfactor;
	vertexTmpTexture[2].y = hfactor;
	vertexTmpTexture[2].z = -0.1f;
	vertexTmpTexture[2].s = wmul;
	vertexTmpTexture[2].t = hmul;
	
	vertexTmpTexture[3].x = -1.0f;
	vertexTmpTexture[3].y = hfactor;
	vertexTmpTexture[3].z = -0.1f;
	vertexTmpTexture[3].s = 0.0f;
	vertexTmpTexture[3].t = hmul;
}


void GLDraw_4_5::set_osd_vertex(int xbit)
{
	float xbase, ybase, zbase;
	VertexTexCoord_t vertex[4];
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
	vertex[0].x = xbase;
	vertex[0].y = ybase;
	vertex[0].z = zbase;
	vertex[0].s = 0.0f;
	vertex[0].t = 0.0f;
	
	vertex[1].x = xbase + (48.0f / 640.0f);
	vertex[1].y = ybase;
	vertex[1].z = zbase;
	vertex[1].s = 1.0f;
	vertex[1].t = 0.0f;
	
	vertex[2].x = xbase + (48.0f / 640.0f);
	vertex[2].y = ybase - (48.0f / 400.0f);
	vertex[2].z = zbase;
	vertex[2].s = 1.0f;
	vertex[2].t = 1.0f;
	
	vertex[3].x = xbase;
	vertex[3].y = ybase - (48.0f / 400.0f);
	vertex[3].z = zbase;
	vertex[3].s = 0.0f;
	vertex[3].t = 1.0f;
	
	setNormalVAO(osd_pass->getShader(), osd_pass_vao[xbit],
				 osd_pass_vbuffer[xbit],
				 vertex, 4);
}

void GLDraw_4_5::set_led_vertex(int xbit)
{
	float xbase, ybase, zbase;
	VertexTexCoord_t vertex[4];

	if((xbit < 0) || (xbit >=32)) return;
	xbase = 0.0f + (1.0f / 32.0f) * 31.0f - ((1.0f * (float)xbit) / 32.0f) + (1.0f / 128.0f);
	ybase = -1.0f + (2.0f / 64.0f) * 1.5f;
	zbase = -0.999f;
	vertex[0].x = xbase;
	vertex[0].y = ybase;
	vertex[0].z = zbase;
	vertex[0].s = 0.0f;
	vertex[0].t = 0.0f;
	
	vertex[1].x = xbase + (1.0f / 64.0f);
	vertex[1].y = ybase;
	vertex[1].z = zbase;
	vertex[1].s = 1.0f;
	vertex[1].t = 0.0f;
	
	vertex[2].x = xbase + (1.0f / 64.0f);
	vertex[2].y = ybase - (1.0f / 64.0f);
	vertex[2].z = zbase;
	vertex[2].s = 1.0f;
	vertex[2].t = 1.0f;
	
	vertex[3].x = xbase;
	vertex[3].y = ybase - (1.0f / 64.0f);
	vertex[3].z = zbase;
	vertex[3].s = 0.0f;
	vertex[3].t = 1.0f;
	
	setNormalVAO(led_pass->getShader(), led_pass_vao[xbit],
				 led_pass_vbuffer[xbit],
				 vertex, 4);
}

void GLDraw_4_5::do_set_screen_multiply(float mul)
{
	screen_multiply = mul;
	if((screen_texture_width <= 0) || (screen_texture_height <= 0)) return;
	do_set_texture_size(imgptr, screen_texture_width, screen_texture_height);
}

void GLDraw_4_5::do_set_texture_size(QImage *p, int w, int h)
{
	if(w <= 0) w = using_flags->get_real_screen_width();
	if(h <= 0) h = using_flags->get_real_screen_height();
	imgptr = p;
	float iw, ih;
	if(p != NULL) {
		iw = (float)p->width();
		ih = (float)p->height();
	} else {
		iw = (float)using_flags->get_real_screen_width();
		ih = (float)using_flags->get_real_screen_height();
	}
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SCREEN, "%dx%d -> %fx%f\n", w, h, iw, ih);
	if((p_wid != NULL) &&
	   ((screen_texture_width != w) || (screen_texture_height != h)) &&
	   (w > 0) && (h > 0)) {
		screen_texture_width = w;
		screen_texture_height = h;
		
		p_wid->makeCurrent();
		{
			//set_texture_vertex(p, p_wid->width(), p_wid->height(), w, h);
			set_texture_vertex((float)w / iw, (float)h / ih);
			setNormalVAO(ntsc_pass1->getShader(), ntsc_pass1->getVAO(),
						 ntsc_pass1->getVertexBuffer(),
						 vertexTmpTexture, 4);

			set_texture_vertex(1.0f, 1.0f);
			setNormalVAO(ntsc_pass2->getShader(), ntsc_pass2->getVAO(),
						 ntsc_pass2->getVertexBuffer(),
						 vertexTmpTexture, 4);
			
		}
		/*if(((int)iw != pixel_width) || ((int)ih != pixel_height))*/ {
			QImage im((int)screen_texture_width, (int)screen_texture_height, QImage::Format_RGBA8888);
			if(p == NULL) {
				p = &im;
			}
			if(uVramTextureID != NULL) {
//				p_wid->makeCurrent();
				uVramTextureID->destroy();
				delete uVramTextureID;
				uVramTextureID = createMainTexture(p);
//				p_wid->doneCurrent();
			} else {
//				p_wid->makeCurrent();
				uVramTextureID = createMainTexture(p);
//				p_wid->doneCurrent();
			}
		}
		vertexFormat[0].x = -1.0f;
		vertexFormat[0].y = -1.0f;
		vertexFormat[0].z = -0.9f;
		vertexFormat[1].x =  1.0f;
		vertexFormat[1].y = -1.0f;
		vertexFormat[1].z = -0.9f;
		vertexFormat[2].x =  1.0f;
		vertexFormat[2].y =  1.0f;
		vertexFormat[2].z = -0.9f;
		vertexFormat[3].x = -1.0f;
		vertexFormat[3].y =  1.0f;
		vertexFormat[3].z = -0.9f;

		vertexFormat[0].s = 0.0f;
		vertexFormat[0].t = (float)h / ih;
		vertexFormat[1].s = (float)w / iw;
		vertexFormat[1].t = (float)h / ih;
		vertexFormat[2].s = (float)w / iw;
		//vertexFormat[0].t = 1.0f;
		//vertexFormat[1].s = 1.0f;
		//vertexFormat[1].t = 1.0f;
		//vertexFormat[2].s = 1.0f;
		vertexFormat[2].t = 0.0f;
		vertexFormat[3].s = 0.0f;
		vertexFormat[3].t = 0.0f;
		
		setNormalVAO(main_pass->getShader(), main_pass->getVAO(),
					 main_pass->getVertexBuffer(),
					 vertexFormat, 4);
		
		if(w > using_flags->get_real_screen_width()) {
			w = using_flags->get_real_screen_width();
		}			
		if(h > using_flags->get_real_screen_height()) {
			h = using_flags->get_real_screen_height();
		}
		this->doSetGridsHorizonal(h, false);
		this->doSetGridsVertical(w, false);
		p_wid->doneCurrent();
	}
}
void GLDraw_4_5::do_set_horiz_lines(int lines)
{
	if(lines > using_flags->get_real_screen_height()) {
		lines = using_flags->get_real_screen_height();
	}			
	this->doSetGridsHorizonal(lines, false);
}

void GLDraw_4_5::resizeGL_Screen(void)
{
	if(main_pass != NULL) {
		setNormalVAO(main_pass->getShader(), main_pass->getVAO(),
					 main_pass->getVertexBuffer(),
					 vertexFormat, 4);
	}
}	

void GLDraw_4_5::resizeGL(int width, int height)
{
	//int side = qMin(width, height);
	extfunc->glViewport(0, 0, width, height);
	//extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
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
		setNormalVAO(bitmap_block->getShader(), bitmap_block->getVAO(),
					 bitmap_block->getVertexBuffer(),
					 vertexBitmap, 4);
	}	
	if(using_flags->get_max_button() > 0) {
		updateButtonTexture();
	}
}

void GLDraw_4_5::initButtons(void)
{
	button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
	QOpenGLContext *context = QOpenGLContext::currentContext();
	QPair<int, int> _version = QOpenGLVersionProfile(context->format()).version();
	QString versionext = QString::fromUtf8("");
	if(((_version.first == 4) && (_version.second >= 3)) || (_version.first >= 5)) {
		versionext = QString::fromUtf8("#version 430 core \n"); // OK?
	} else if((_version.first == 4)) {
		versionext = QString::fromUtf8("#version 400 core \n");
	} else { // Require GLVersion >= 3.2
		versionext = QString::fromUtf8("#version 150 \n");
	}
	
	if(vm_buttons_d != NULL) {
		button_shader = new QOpenGLShaderProgram(p_wid);
		if(button_shader != NULL) {
			bool f = false;
			QFile vertex_src(QString::fromUtf8(":/gl4_5/vertex_shader.glsl"));
			if (vertex_src.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QString srcs = versionext;
				srcs = srcs + QString::fromUtf8(vertex_src.readAll());
				f  = button_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, srcs);
				vertex_src.close();
			} else {
				return;
			}
			QFile fragment_src(QString::fromUtf8(":/gl4_5/normal_fragment_shader.glsl"));
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

void GLDraw_4_5::do_set_display_osd(bool onoff)
{
	osd_onoff = onoff;
}

void GLDraw_4_5::do_display_osd_leds(int lednum, bool onoff)
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

void GLDraw_4_5::uploadIconTexture(QPixmap *p, int icon_type, int localnum)
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


void GLDraw_4_5::updateBitmap(QImage *p)
{
	if(!using_flags->is_use_one_board_computer()) return;
	redraw_required = true;
	bitmap_uploaded = false;
	uploadBitmapTexture(p);
}

void GLDraw_4_5::uploadBitmapTexture(QImage *p)
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

void GLDraw_4_5::updateButtonTexture(void)
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

void GLDraw_4_5::get_screen_geometry(int *w, int *h)
{
	if(w != NULL) *w = pixel_width; 
	if(h != NULL) *h = pixel_height;
}

bool GLDraw_4_5::copy_screen_buffer(scrntype_t *target, int w, int h, int stride)
{
	int hh = h;
	if(stride <= 0) return false;
	if(target == NULL) return false;
	if((w <= 0) || (h <= 0)) return false;
	if(w >= pixel_width) w = pixel_width;
	if(h >= pixel_height) h = pixel_height;
	if(stride >= pixel_width) stride = pixel_width;
	if(w >= stride) w = stride;
	
//	if((map_base_address == NULL) || !(main_texture_ready)) {
//		return false;
//	}
	QMutexLocker Locker_S(main_mutex);	
	extfunc->glBindBuffer(GL_PIXEL_PACK_BUFFER, main_read_texture_buffer);
	extfunc->glBindTexture(GL_TEXTURE_2D, uVramTextureID->textureId());
	extfunc->glActiveTexture(GL_TEXTURE0);
	extfunc->glGetTextureImage(uVramTextureID->textureId(), 0,
							   GL_RGBA, GL_UNSIGNED_BYTE, pixel_width * pixel_height * sizeof(scrntype_t), NULL);
	scrntype_t*pp = (scrntype_t *)(extfunc->glMapNamedBufferRange(main_read_texture_buffer, 0, pixel_width * pixel_height * sizeof(scrntype_t),  GL_MAP_READ_BIT ));
	extfunc->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	if(pp == NULL) return false;
//	printf("READ SCREEN\n");
	scrntype_t *p = (scrntype_t *)pp;
	scrntype_t *q = target;
	for(int y = 0; y < hh; y++) {
		memcpy(&(pp[y * pixel_width]), q, w * sizeof(uint32_t));
		q = q + stride;
	}
//	extfunc->glBindBuffer(GL_PIXEL_PACK_BUFFER, main_read_texture_buffer);
//	extfunc->glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
//	extfunc->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	
	return true;
}

scrntype_t *GLDraw_4_5::get_screen_buffer(int y)
{
	QMutexLocker Locker_S(main_mutex);
	if((y < 0) || (y >= pixel_height) || (pixel_width < 0)) return NULL;
	if((map_base_address == NULL) || !(main_texture_ready)) {
		return NULL;
	} else {
//		int of = (y <= 0) ? 0 : (pixel_width * (y - 1));
//		int len = (y <= 0) ? (pixel_width * pixel_height) : pixel_width;
		
//		extfunc->glFlushMappedNamedBufferRange(main_texture_buffer, of, len * sizeof(scrntype_t));
//		if(sync_fence != 0) {
//			extfunc->glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
//			extfunc->glDeleteSync(sync_fence);
//		}
//		sync_fence = extfunc->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,  0);
		scrntype_t *p = (scrntype_t *)map_base_address;
		
		p = p + (pixel_width * y);
		//printf("%08x\n", (uintptr_t)p);   
		return p;
	}
}
// Note: Mapping vram from draw_thread does'nt work well.
// This feature might be disable. 20180728 K.Ohta.
bool GLDraw_4_5::is_ready_to_map_vram_texture(void)
{
	if(main_texture_buffer == 0) {
		return false;
	}
	if(gl_major_version < 4) {
		return false;
	}
	if(gl_minor_version < 4) {
		return false;
	}
	return true;
}

bool GLDraw_4_5::map_vram_texture(void)
{
	QMutexLocker Locker_S(main_mutex);
	if(main_texture_buffer == 0) {
		return false;
	}
//	if(!(main_texture_ready)) {
//		return false;
//	}

	if(gl_major_version < 4) {
		return false;
	}
	if(gl_minor_version < 4) {
		return false;
	}
#if 0
	return false;
#else
	// 20200812 K.O: MUST WAIT when changing texture feature.
	extfunc->glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	extfunc->glDeleteSync(sync_fence);
	sync_fence = extfunc->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,  0);
	
	extfunc->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, main_texture_buffer);
	map_base_address =
		(scrntype_t *)(extfunc->glMapNamedBufferRange(
						   main_texture_buffer, 0,
						   pixel_width * pixel_height * sizeof(scrntype_t),
						   GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT));
	
	// 20200812 K.O: MUST WAIT when changing texture feature.
	extfunc->glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	extfunc->glDeleteSync(sync_fence);
	sync_fence = extfunc->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,  0);
	
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SCREEN, "MAPPED SCREEN TO PHYSICAL ADDRESS:%0llx\n", (uintptr_t)map_base_address);
	extfunc->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	if(map_base_address == NULL) return false;
	
	return true;
#endif
}

bool GLDraw_4_5::unmap_vram_texture(void)
{
	QMutexLocker Locker_S(main_mutex);
	if((map_base_address == NULL) || (main_texture_buffer == 0)) return false;
	if(sync_fence != 0) {
		extfunc->glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
		extfunc->glDeleteSync(sync_fence);
	}
	sync_fence = extfunc->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,  0);
	map_base_address = NULL;
	
	extfunc->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, main_texture_buffer);
	extfunc->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	extfunc->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	
	extfunc->glDeleteBuffers(1, &main_texture_buffer);
	main_texture_buffer = 0;

	extfunc->glDeleteBuffers(1, &main_read_texture_buffer);
	main_read_texture_buffer = 0;

	return true;
}
