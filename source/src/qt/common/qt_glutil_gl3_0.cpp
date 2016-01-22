
#include "qt_gldraw.h"
#include "qt_glutil_gl3_0.h"

GLDraw_3_0::GLDraw_3_0(GLDrawClass *parent, EMU *emu) : GLDraw_2_0(parent, emu)
{
	uTmpTextureID = 0;
	uTmpFrameBuffer = 0;
	uTmpDepthBuffer = 0;
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
			if(w <= 0) ww = w = SCREEN_WIDTH;
			if(h <= 0) hh = h = SCREEN_HEIGHT;
			wfactor = 1.0f;
			hfactor = 1.0f;
			if(imgptr != NULL) {
				iw = (float)imgptr->width();
				ih = (float)imgptr->height();
			} else {
				iw = (float)SCREEN_WIDTH;
				ih = (float)SCREEN_HEIGHT;
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
		QImage img(SCREEN_WIDTH, SCREEN_HEIGHT, QImage::Format_ARGB32);
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
		extfunc_3_0->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, SCREEN_WIDTH_ASPECT, SCREEN_HEIGHT_ASPECT);
		extfunc_3_0->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
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
#ifdef ONE_BOARD_MICRO_COMPUTER
	if(uBitmapTextureID != 0) {
		extfunc->glEnable(GL_BLEND);
		extfunc->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
#else
	extfunc->glDisable(GL_BLEND);
#endif
	
	QVector4D color;
	smoosing = config.use_opengl_filters;
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
#ifdef ONE_BOARD_MICRO_COMPUTER
	extfunc_3_0->glDisable(GL_BLEND);
#endif	
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
# ifdef USE_SCREEN_ROTATE
		if(config.rotate_type) {
			prg->setUniformValue("rotate", GL_TRUE);
		} else {
			prg->setUniformValue("rotate", GL_FALSE);
		}
#else		
		prg->setUniformValue("rotate", GL_FALSE);
#endif
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
# if defined(ONE_BOARD_MICRO_COMPUTER) || defined(MAX_BUTTONS)
		uploadMainTexture(imgptr, true);
# else
		uploadMainTexture(imgptr, false);
# endif
		crt_flag = true;
		p_wid->doneCurrent();
	}
}

void GLDraw_3_0::do_set_texture_size(QImage *p, int w, int h)
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
