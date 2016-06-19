/*
 * qt_glutil_gl3_0.cpp
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL v3.0 (extend from renderer with OpenGL v2.0).
 * History:
 * Jan 22, 2016 : Initial.
 */

#include "qt_gldraw.h"
#include "qt_glutil_gl3_0.h"
#include "menu_flags.h"

//extern USING_FLAGS *using_flags;

GLDraw_3_0::GLDraw_3_0(GLDrawClass *parent, USING_FLAGS *p, EMU *emu) : GLDraw_2_0(parent, p, emu)
{
	uTmpTextureID = 0;
	uTmpFrameBuffer = 0;
	uTmpDepthBuffer = 0;

	grids_shader = NULL;
	tmp_shader = NULL;
	
	buffer_screen_vertex = NULL;
	vertex_screen = NULL;

	buffer_vertex_tmp_texture = NULL;
	vertex_tmp_texture = NULL;
	
	grids_horizonal_buffer = NULL;
	grids_horizonal_vertex = NULL;
	
	grids_vertical_buffer = NULL;
	grids_vertical_vertex = NULL;

}

GLDraw_3_0::~GLDraw_3_0()
{
	p_wid->deleteTexture(uTmpTextureID);
	p_wid->deleteTexture(uTmpTextureID);
	extfunc_3_0->glDeleteFramebuffers(1, &uTmpFrameBuffer);
	extfunc_3_0->glDeleteRenderbuffers(1, &uTmpDepthBuffer);
	
	extfunc_3_0->glDeleteFramebuffers(1, &uTmpFrameBuffer);
	extfunc_3_0->glDeleteRenderbuffers(1, &uTmpDepthBuffer);
	
	if(buffer_vertex_tmp_texture->isCreated()) buffer_vertex_tmp_texture->destroy();
	if(vertex_tmp_texture->isCreated()) vertex_tmp_texture->destroy();

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

void GLDraw_3_0::setNormalVAO(QOpenGLShaderProgram *prg,
							   QOpenGLVertexArrayObject *vp,
							   QOpenGLBuffer *bp,
							   VertexTexCoord_t *tp,
							   int size)
{
	int vertex_loc = prg->attributeLocation("vertex");
	int texcoord_loc = prg->attributeLocation("texcoord");

	vp->bind();
	bp->bind();

	bp->write(0, tp, sizeof(VertexTexCoord_t) * size);
	prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
	prg->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
	prg->setUniformValue("a_texture", 0);
			   
	extfunc_3_0->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord_t), 0); 
	extfunc_3_0->glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord_t), 
							       (char *)NULL + 3 * sizeof(GLfloat)); 
	bp->release();
	vp->release();
	prg->enableAttributeArray(vertex_loc);
	prg->enableAttributeArray(texcoord_loc);
}

void GLDraw_3_0::initGLObjects()
{
	extfunc = new QOpenGLFunctions_2_0;
	extfunc->initializeOpenGLFunctions();
	extfunc_3_0 = new QOpenGLFunctions_3_0;
	extfunc_3_0->initializeOpenGLFunctions();
}	

void GLDraw_3_0::initLocalGLObjects(void)
{

	main_shader = new QOpenGLShaderProgram(p_wid);
	if(main_shader != NULL) {
		main_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
		main_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragment_shader.glsl");
		main_shader->link();
	}

	
	buffer_screen_vertex = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	vertex_screen = new QOpenGLVertexArrayObject;
	if(vertex_screen != NULL) {
		if(vertex_screen->create()) {
			{
				QVector4D c;
				c = QVector4D(1.0, 1.0, 1.0, 1.0);
				main_shader->setUniformValue("color", c);
			}
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
			
			
			buffer_screen_vertex->create();
			buffer_screen_vertex->setUsagePattern(QOpenGLBuffer::DynamicDraw);
			
			vertex_screen->bind();
			buffer_screen_vertex->bind();
			buffer_screen_vertex->allocate(sizeof(VertexTexCoord_t) * 4);
			vertex_screen->release();
			buffer_screen_vertex->release();
			setNormalVAO(main_shader, vertex_screen,
						 buffer_screen_vertex,
						 vertexFormat, 4);
			//QMatrix4x4 mat;
			//mat.ortho(-1.0, 1.0, -1.0, +1.0, -1.0, 1.0);
			//mat.translate(0, 0, 0);
		}
	}
	tmp_shader = new QOpenGLShaderProgram(p_wid);
	if(tmp_shader != NULL) {
		tmp_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/tmp_vertex_shader.glsl");
		tmp_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/chromakey_fragment_shader.glsl");
		tmp_shader->link();
	}
	buffer_vertex_tmp_texture = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	vertex_tmp_texture = new QOpenGLVertexArrayObject;
	if(vertex_tmp_texture != NULL) {
		if(vertex_tmp_texture->create()) {
			float iw, ih;
			float wfactor, hfactor;
			int ww, hh;
			int w, h;
			ww = w = screen_texture_width;
			hh = h = screen_texture_height;
			if(w <= 0) ww = w = using_flags->get_screen_width();
			if(h <= 0) hh = h = using_flags->get_screen_height();
			wfactor = 1.0f;
			hfactor = 1.0f;
			if(imgptr != NULL) {
				iw = (float)imgptr->width();
				ih = (float)imgptr->height();
			} else {
				iw = (float)using_flags->get_screen_width();
				ih = (float)using_flags->get_screen_height();
			}
			//if(screen_multiply < 1.0f) {
			if((w > p_wid->width()) || (h > p_wid->height())) {
				ww = (int)(screen_multiply * (float)w);
				hh = (int)(screen_multiply * (float)h);
				wfactor = screen_multiply * 2.0f - 1.0f;
				hfactor = -screen_multiply * 2.0f + 1.0f;
			}
			if(screen_multiply < 1.0f) {
				ww = (int)(screen_multiply * (float)w);
				hh = (int)(screen_multiply * (float)h);
				wfactor = screen_multiply * 2.0f - 1.0f;
				hfactor = -screen_multiply * 2.0f + 1.0f;
			}
		   
			vertexTmpTexture[0].x = -1.0f;
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
			buffer_vertex_tmp_texture->create();
			int vertex_loc = tmp_shader->attributeLocation("vertex");
			int texcoord_loc = tmp_shader->attributeLocation("texcoord");
			vertex_tmp_texture->bind();
			buffer_vertex_tmp_texture->bind();
			buffer_vertex_tmp_texture->allocate(sizeof(vertexTmpTexture));
			buffer_vertex_tmp_texture->setUsagePattern(QOpenGLBuffer::StaticDraw);
			buffer_vertex_tmp_texture->release();
			vertex_tmp_texture->release();
			setNormalVAO(tmp_shader, vertex_tmp_texture,
						 buffer_vertex_tmp_texture,
						 vertexTmpTexture, 4);
		}
	}
	if(uTmpTextureID == 0) {
		QImage img(using_flags->get_screen_width(), using_flags->get_screen_height(), QImage::Format_ARGB32);
		QColor col(0, 0, 0, 255);
		img.fill(col);
		uTmpTextureID = p_wid->bindTexture(img);
		extfunc_3_0->glBindTexture(GL_TEXTURE_2D, 0);
	}
	if(uTmpFrameBuffer == 0) {
		extfunc_3_0->glGenFramebuffers(1, &uTmpFrameBuffer);
	}
	if(uTmpDepthBuffer == 0) {
		extfunc_3_0->glGenRenderbuffers(1, &uTmpDepthBuffer);
		extfunc_3_0->glBindRenderbuffer(GL_RENDERBUFFER, uTmpDepthBuffer);
		extfunc_3_0->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, using_flags->get_screen_width(), using_flags->get_screen_height());
		extfunc_3_0->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	grids_shader = new QOpenGLShaderProgram(p_wid);
	if(using_flags->is_use_screen_rotate()) {
		if(grids_shader != NULL) {
			grids_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grids_vertex_shader.glsl");
			grids_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grids_fragment_shader.glsl");
			grids_shader->link();
		}
	} else {
		if(grids_shader != NULL) {
			grids_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grids_vertex_shader_fixed.glsl");
			grids_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grids_fragment_shader.glsl");
			grids_shader->link();
		}
	}
	grids_horizonal_buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	grids_horizonal_vertex = new QOpenGLVertexArrayObject;
	grids_horizonal_vertex->create();
	updateGridsVAO(grids_horizonal_buffer, grids_horizonal_vertex,
				   glHorizGrids, using_flags->get_screen_height() + 2);
	
	grids_vertical_buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	grids_vertical_vertex = new QOpenGLVertexArrayObject;
	grids_vertical_vertex->create();
	updateGridsVAO(grids_vertical_buffer, grids_vertical_vertex,
				   glVertGrids, using_flags->get_screen_width() + 2);

			
}

void GLDraw_3_0::updateGridsVAO(QOpenGLBuffer *bp,
								QOpenGLVertexArrayObject *vp,
								GLfloat *tp,
								int number)

{
	bool checkf = false;
	if(bp != NULL) {
		if(bp->isCreated()) {
			if(bp->size() != (number * sizeof(GLfloat) * 3 * 2)) {
				bp->destroy();
				bp->create();
				checkf = true;
			}
		} else {
			bp->create();
			checkf = true;
		}
		if(checkf) {
			bp->bind();
			bp->allocate((number + 1) * sizeof(GLfloat) * 3 * 2);
			if(tp != NULL) {
				bp->write(0, tp, (number + 1) * sizeof(GLfloat) * 3 * 2);
			}
			bp->release();
		}
	}
}
void GLDraw_3_0::drawGridsMain_3(QOpenGLShaderProgram *prg,
								 QOpenGLBuffer *bp,
								 QOpenGLVertexArrayObject *vp,
								 int number,
								 GLfloat lineWidth,
								 QVector4D color)
{
	if(number <= 0) return;
	extfunc->glDisable(GL_TEXTURE_2D);
	extfunc->glDisable(GL_DEPTH_TEST);
	extfunc->glDisable(GL_BLEND);

	if((bp == NULL) || (vp == NULL) || (prg == NULL)) return;
	if((!bp->isCreated()) || (!vp->isCreated()) || (!prg->isLinked())) return;
	{
		int i, p;
		bp->bind();
		vp->bind();
		prg->bind();
		
		prg->setUniformValue("color", color);
		prg->enableAttributeArray("vertex");
		int vertex_loc = prg->attributeLocation("vertex");
		extfunc_3_0->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0); 
		extfunc_3_0->glEnableVertexAttribArray(vertex_loc);
		
		extfunc_3_0->glEnableClientState(GL_VERTEX_ARRAY);
		extfunc_3_0->glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		extfunc_3_0->glLineWidth(lineWidth);
		extfunc_3_0->glVertexPointer(3, GL_FLOAT, 0, 0);
		extfunc_3_0->glDrawArrays(GL_LINES, 0, (number + 1) * 2);
		extfunc_3_0->glDisableClientState(GL_VERTEX_ARRAY);
		prg->release();
		vp->release();
		bp->release();
	}
}

void GLDraw_3_0::drawGridsHorizonal(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	updateGridsVAO(grids_horizonal_buffer,
				   grids_horizonal_vertex,
				   glHorizGrids,
				   vert_lines);
	drawGridsMain_3(grids_shader,
					grids_horizonal_buffer,
					grids_horizonal_vertex,
					vert_lines,
					0.15f,
					c);
}

void GLDraw_3_0::drawGridsVertical(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	updateGridsVAO(grids_vertical_buffer,
				   grids_vertical_vertex,
				   glVertGrids,
				   horiz_pixels);
	drawGridsMain_3(grids_shader,
					grids_vertical_buffer,
					grids_vertical_vertex,
					horiz_pixels,
					0.5f,
					c);
}

void GLDraw_3_0::uploadMainTexture(QImage *p, bool use_chromakey)
{
	// set vertex
	redraw_required = true;
	if(p == NULL) return;
	//redraw_required = true;
	imgptr = p;
	if(uVramTextureID == 0) {
		uVramTextureID = p_wid->bindTexture(*p);
	}
	{
		// Upload to main texture
		extfunc_3_0->glBindTexture(GL_TEXTURE_2D, uVramTextureID);
		extfunc_3_0->glTexSubImage2D(GL_TEXTURE_2D, 0,
							 0, 0,
							 p->width(), p->height(),
							 GL_BGRA, GL_UNSIGNED_BYTE, p->constBits());
		extfunc_3_0->glBindTexture(GL_TEXTURE_2D, 0);
	}
	{
		// Render to tmp-frame buffer and transfer to texture.
		extfunc_3_0->glBindFramebuffer(GL_FRAMEBUFFER, uTmpFrameBuffer);
		extfunc_3_0->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uTmpTextureID, 0);
		extfunc_3_0->glBindRenderbuffer(GL_RENDERBUFFER, uTmpDepthBuffer);
		extfunc_3_0->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, uTmpDepthBuffer);
		
		extfunc_3_0->glClearColor(0.0, 0.0, 0.0, 1.0);
		extfunc_3_0->glClearDepth(1.0f);
		extfunc_3_0->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		QVector4D c(fBrightR, fBrightG, fBrightB, 1.0);
		QVector3D chromakey(0.0, 0.0, 0.0);
		
		tmp_shader->setUniformValue("color", c);
		{
			if(uVramTextureID != 0) {
				extfunc_3_0->glEnable(GL_TEXTURE_2D);
				vertex_tmp_texture->bind();
				buffer_vertex_tmp_texture->bind();
				tmp_shader->bind();
				extfunc_3_0->glViewport(0, 0, screen_texture_width, screen_texture_height);
				extfunc_3_0->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
				extfunc_3_0->glActiveTexture(GL_TEXTURE0);
				extfunc_3_0->glBindTexture(GL_TEXTURE_2D, uVramTextureID);
				extfunc_3_0->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				extfunc_3_0->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				if(use_chromakey) {
					tmp_shader->setUniformValue("chromakey", chromakey);
					tmp_shader->setUniformValue("do_chromakey", GL_TRUE);
				} else {
					tmp_shader->setUniformValue("do_chromakey", GL_FALSE);
				}
				//extfunc_3_0->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				tmp_shader->setUniformValue("a_texture", 0);
				tmp_shader->setUniformValue("color", c);
				tmp_shader->setUniformValue("tex_width",  (float)screen_texture_width); 
				tmp_shader->setUniformValue("tex_height", (float)screen_texture_height);
				tmp_shader->enableAttributeArray("texcoord");
				tmp_shader->enableAttributeArray("vertex");
				int vertex_loc = tmp_shader->attributeLocation("vertex");
				int texcoord_loc = tmp_shader->attributeLocation("texcoord");
				extfunc_3_0->glEnableVertexAttribArray(vertex_loc);
				extfunc_3_0->glEnableVertexAttribArray(texcoord_loc);
				extfunc_3_0->glEnable(GL_VERTEX_ARRAY);

				extfunc_3_0->glDrawArrays(GL_POLYGON, 0, 4);
				
				extfunc_3_0->glViewport(0, 0, p_wid->width(), p_wid->height());
				extfunc_3_0->glOrtho(0.0f, (float)p_wid->width(), 0.0f, (float)p_wid->height(), -1.0, 1.0);
				buffer_vertex_tmp_texture->release();
				vertex_tmp_texture->release();
		
				tmp_shader->release();
				extfunc_3_0->glBindTexture(GL_TEXTURE_2D, 0);
				extfunc_3_0->glDisable(GL_TEXTURE_2D);
			}
		}
		extfunc_3_0->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		extfunc_3_0->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	crt_flag = true;
}

void GLDraw_3_0::drawScreenTexture(void)
{
	if(using_flags->is_use_one_board_computer()) {
		if(uBitmapTextureID != 0) {
			extfunc->glEnable(GL_BLEND);
			extfunc->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	} else {
		extfunc->glDisable(GL_BLEND);
	}
	
	QVector4D color;
	smoosing = using_flags->get_config_ptr()->use_opengl_filters;
	if(set_brightness) {
		color = QVector4D(fBrightR, fBrightG, fBrightB, 1.0);
	} else {
		color = QVector4D(1.0, 1.0, 1.0, 1.0);
	}			
	{
		main_shader->setUniformValue("color", color);
		drawMain(main_shader, vertex_screen,
				 buffer_screen_vertex,
				 vertexFormat,
				 uTmpTextureID, // v2.0
				 color, smoosing);
	}		
	if(using_flags->is_use_one_board_computer()) {
		extfunc_3_0->glDisable(GL_BLEND);
	}
}


void GLDraw_3_0::drawMain(QOpenGLShaderProgram *prg,
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
		extfunc->glEnable(GL_TEXTURE_2D);
		vp->bind();
		bp->bind();
		prg->bind();
		extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());
		extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
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
		prg->setUniformValue("color", color);
		prg->setUniformValue("tex_width",  (float)screen_texture_width); 
		prg->setUniformValue("tex_height", (float)screen_texture_height);
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
		prg->enableAttributeArray("texcoord");
		prg->enableAttributeArray("vertex");
		int vertex_loc = prg->attributeLocation("vertex");
		int texcoord_loc = prg->attributeLocation("texcoord");
		extfunc->glEnableVertexAttribArray(vertex_loc);
		extfunc->glEnableVertexAttribArray(texcoord_loc);
		extfunc->glEnable(GL_VERTEX_ARRAY);
		extfunc->glDrawArrays(GL_POLYGON, 0, 4);
		bp->release();
		vp->release();
		
		prg->release();
		extfunc->glBindTexture(GL_TEXTURE_2D, 0);
		extfunc->glDisable(GL_TEXTURE_2D);
	}
}


void GLDraw_3_0::setBrightness(GLfloat r, GLfloat g, GLfloat b)
{
	fBrightR = r;
	fBrightG = g;
	fBrightB = b;

	if(imgptr != NULL) {
		p_wid->makeCurrent();
		if(uVramTextureID != 0) {
			uVramTextureID = p_wid->bindTexture(*imgptr);
		}
		if(using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0)) {
			uploadMainTexture(imgptr, true);
		} else {
			uploadMainTexture(imgptr, false);
		}
		crt_flag = true;
		p_wid->doneCurrent();
	}
}

void GLDraw_3_0::do_set_texture_size(QImage *p, int w, int h)
{
	if(w <= 0) w = using_flags->get_screen_width();
	if(h <= 0) h = using_flags->get_screen_height();
	float wfactor = 1.0f;
	float hfactor = 1.0f;
	float iw, ih;
	imgptr = p;
	if(p != NULL) {
		iw = (float)p->width();
		ih = (float)p->height();
	} else {
		iw = (float)using_flags->get_screen_width();
		ih = (float)using_flags->get_screen_height();
	}
	if(p != NULL) {
		int ww = w;
		int hh = h;
		//if(screen_multiply < 1.0f) {
		if((w > p_wid->width()) || (h > p_wid->height())) {
			ww = (int)(screen_multiply * (float)w);
			hh = (int)(screen_multiply * (float)h);
			wfactor = screen_multiply * 2.0f - 1.0f;
			hfactor = -screen_multiply * 2.0f + 1.0f;
		}
		screen_texture_width = w;
		screen_texture_height = h;

		p_wid->makeCurrent();
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
			p_wid->deleteTexture(uVramTextureID);
			uVramTextureID = p_wid->bindTexture(*p);
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
		p_wid->doneCurrent();

		this->doSetGridsHorizonal(h, true);
		this->doSetGridsVertical(w, true);
	}
}
