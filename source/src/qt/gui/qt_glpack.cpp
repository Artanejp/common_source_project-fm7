
#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QMatrix4x2>
#include <QMatrix4x4>
#include <QImage>
#include <QFile>
#include <QString>

#include "common.h"
#include "qt_glpack.h"

GLScreenPack::GLScreenPack(int _width, int _height, QString _name, QObject *parent, bool is_float, bool req_high_presicion) : QObject(parent)
{
	program = new QOpenGLShaderProgram(this);
	
	vertex_buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	vertex = new QOpenGLVertexArrayObject;
	QOpenGLContext *context = QOpenGLContext::currentContext();
	init_status = false;
	shader_status = false;
	obj_name = _name;
	
	Texture = 0;
	frame_buffer_num = 0;
	texture_is_float = is_float;
	texture_is_high_presicion = req_high_presicion;
	for(int i = 0; i < 4; i++) {
		Vertexs[i].x = 0.0;
		Vertexs[i].y = 0.0;
		Vertexs[i].z = -0.8;
		Vertexs[i].s = 0.0;
		Vertexs[i].t = 0.0;
	}
	
	if(_width <= 0) {
		_width = 640;
	}
	if(_height <= 0) {
		_height = 400;
	}
	viewport_w = _width;
	viewport_h = _height;
	viewport_x = 0;
	viewport_y = 0;

	tex_geometry_w = _width;
	tex_geometry_h = _height;
	tex_geometry_x = 0;
	tex_geometry_y = 0;

	has_extension_texture_float = false;
	has_extension_texture_half_float = false;
	has_extension_fragment_high_precision = false;

	log_str.clear();
	//genBuffer(_width, _height);
}
GLScreenPack::~GLScreenPack()
{
	if(vertex != NULL) {
		if(vertex->isCreated()) vertex->destroy();
		delete vertex;
	}
	if(vertex_buffer != NULL) {
		if(vertex_buffer->isCreated()) vertex_buffer->destroy();
		delete vertex_buffer;
	}
	if(program != NULL) {
		delete program;
	}
	QOpenGLContext *context = QOpenGLContext::currentContext();
	QOpenGLFunctions _fn(context);
	
	if(Texture != 0) {
		_fn.glDeleteTextures(1, &Texture);
	}
	if(frame_buffer_num != 0) {
		_fn.glDeleteFramebuffers(1, &frame_buffer_num);
	}
}

void GLScreenPack::genBuffer(int width, int height)
{
	QOpenGLContext *context = QOpenGLContext::currentContext();
	QOpenGLFunctions _fn(context);
	
	if(Texture != 0) {
		_fn.glDeleteTextures(1, &Texture);
	}
	if(frame_buffer_num != 0) {
		_fn.glDeleteFramebuffers(1, &frame_buffer_num);
	}
	_fn.glGenTextures(1, &Texture);
	_fn.glBindTexture(GL_TEXTURE_2D, Texture);
	if(context->isOpenGLES()) {
		if(texture_is_float) {
			if((context->hasExtension(QByteArray("GL_OES_texture_half_float"))) && !(texture_is_high_presicion)) {
				has_extension_texture_half_float = true;
#if !defined(Q_OS_WIN)
				_fn.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
#else
				_fn.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
#endif
				push_log("GLES: Using half float texture.");
			} else if(context->hasExtension(QByteArray("GL_OES_texture_float"))) {
				has_extension_texture_float = true;
#if !defined(Q_OS_WIN)
				_fn.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
#else
//			_fn.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA32F, GL_FLOAT, 0);
				_fn.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
#endif
				push_log("GLES: Using float texture.");
			} else	
			{
				_fn.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
				push_log("GLES: Using unsigned integer (UNSIGNED_BYTE) texture.");
			}
		} else { // Not Float
				_fn.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
				push_log("GLES: Using unsigned integer (UNSIGNED_BYTE) texture.");
		}			
		if(context->hasExtension("GL_OES_fragment_precision_high")) {
			has_extension_fragment_high_precision = true;
			push_log("GLES: Using high precision storage.");
		}
	} else {
		// OpenGL, not GLES
		if(texture_is_float) {
			has_extension_texture_float = true;
			_fn.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
			push_log("GL: Using float texture.");
		} else {
			_fn.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			push_log("GLES: Using unsigned integer (UNSIGNED_BYTE) texture.");
		}			
	}
    _fn.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _fn.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //_fn.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FLITER, GL_LINEAR);
    //_fn.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FLITER, GL_LINEAR);

	_fn.glGenFramebuffers(1, &frame_buffer_num);
    _fn.glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_num);
	_fn.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture, 0);

	_fn.glBindFramebuffer(GL_FRAMEBUFFER, 0);
	_fn.glBindTexture(GL_TEXTURE_2D, 0);
}

bool GLScreenPack::initialize(int total_width, int total_height, const QString &vertex_shader_file, const QString &fragment_shader_file, int width, int height)
{

	viewport_w = total_width;
	viewport_h = total_height;
	viewport_x = 0;
	viewport_y = 0;

	if(width <= 0) width = tex_geometry_w;
	if(height <= 0) height = tex_geometry_h;
	{
		genBuffer(width, height);
		tex_geometry_w = width;
		tex_geometry_h = height;
	}
		
	//tex_geometry_x = 0;
	//tex_geometry_y = 0;
	
	if(program != NULL) {
		//if(program->isLinked()) break;
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
		} else if(context->format().profile() == QSurfaceFormat::CoreProfile) {
			if(((_version.first == 4) && (_version.second >= 3)) || (_version.first >= 5)) {
				versionext = QString::fromUtf8("#version 430 core \n"); // OK?
			} else if((_version.first == 4)) {
				versionext = QString::fromUtf8("#version 400 core \n");
			} else { // Require GLVersion >= 3.2
				versionext = QString::fromUtf8("#version 150 \n");
			}					
		} else { // Compatibility
			if((_version.first >= 3)) {
				versionext = QString::fromUtf8("#version 130 \n");
			} else {
				//_ext = _ext + QString::fromUtf8("#version 110 \n");
			}
		}
		QFile vertex_src(vertex_shader_file);
		if (vertex_src.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QString srcs = versionext;
			srcs = srcs + QString::fromUtf8(vertex_src.readAll());
			shader_status  = program->addShaderFromSourceCode(QOpenGLShader::Vertex, srcs);
			vertex_src.close();
		} else {
			shader_status = false;
		}


		QFile fragment_src(fragment_shader_file);
		if ((fragment_src.open(QIODevice::ReadOnly | QIODevice::Text)) && (shader_status)){
			QString _src;
			QString _ext = QString::fromUtf8("");;
			_ext = versionext;
			_src = QString::fromUtf8(fragment_src.readAll());
#if defined(__LITTLE_ENDIAN__)
			_ext = _ext + QString::fromUtf8("#define HOST_ENDIAN_IS_LITTLE \n");
#else
			_ext = _ext + QString::fromUtf8("#define HOST_ENDIAN_IS_BIG \n");
#endif

			if((has_extension_texture_float) || (has_extension_texture_half_float)) {
				_ext = _ext + QString::fromUtf8("#define HAS_FLOAT_TEXTURE \n");
			}
			if(has_extension_texture_half_float) {
				_ext = _ext + QString::fromUtf8("#define HAS_HALF_FLOAT_TEXTURE \n");
			}
			if(has_extension_fragment_high_precision) {
				_ext = _ext + QString::fromUtf8("#define HAS_FRAGMENT_HIGH_PRECISION \n");
			}
			shader_status &= program->addShaderFromSourceCode(QOpenGLShader::Fragment, _ext + _src);
			 //shader_status &= program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragment_shader_file);
			fragment_src.close();
		}
		shader_status &= program->link();
	} else {
		init_status = false;
		return false;
	}
	if(shader_status != true) {
		init_status = false;
		return false;
	}
	if((vertex != NULL) && (vertex_buffer != NULL)) {
		if(vertex->create()) {
			GLfloat xfactor = ((GLfloat)tex_geometry_w / (GLfloat)viewport_w) * 2.0f - 1.0f;
			GLfloat yfactor = ((GLfloat)tex_geometry_h / (GLfloat)viewport_h) * 2.0f - 1.0f;
			//GLfloat sfactor = 1.0f;
			//GLfloat tfactor = 1.0f;
			Vertexs[0].x = -1.0f;
			Vertexs[0].y = -1.0f;
			Vertexs[0].z = -0.9f;
			Vertexs[0].s = 0.0f;
			Vertexs[0].t = 1.0f;
			
			Vertexs[1].x = xfactor;
			Vertexs[1].y = -1.0f;
			Vertexs[1].z = -0.9f;
			Vertexs[1].s = 1.0f;
			Vertexs[1].t = 1.0f;
			
			Vertexs[2].x = xfactor;
			Vertexs[2].y = yfactor;
			Vertexs[2].z = -0.9f;
			Vertexs[2].s = 1.0f;
			Vertexs[2].t = 0.0f;
			
			Vertexs[3].x = -1.0f;
			Vertexs[3].y = yfactor;
			Vertexs[3].z = -0.9f;
			Vertexs[3].s = 0.0f;
			Vertexs[3].t = 0.0f;
			
			
			if(vertex_buffer->create()) {
				vertex_buffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
				vertex->bind();
				vertex_buffer->bind();
				vertex_buffer->allocate(sizeof(VertexTexCoord_t) * 4);
				vertex->release();
				vertex_buffer->release();
				setNormalVAO(Vertexs, 4);
			} else {
				init_status = false;
				return false;
			}
		} else {
			init_status = false;
			return false;
		}
	} else {
		init_status = false;
		return false;
	}
	init_status = true;
	return true;
}

void GLScreenPack::updateVertex(int viewport_width, int viewport_height,
								int screen_width, int screen_height,
								int texture_width, int texture_height,
								int texture_screen_width, int texture_screen_height,
								float position_z,
								QOpenGLBuffer::UsagePattern mode)
{
	if(viewport_width >= 0) {
		viewport_w = viewport_width;
	}
	if(viewport_height >= 0) {
		viewport_h = viewport_height;
	}
	if(texture_width >= 0) {
		tex_geometry_w = texture_width;
	}
	if(texture_height >= 0) {
		tex_geometry_h = texture_height;
	}
	
	GLfloat xfactor = ((GLfloat)screen_width / (GLfloat)viewport_w) * 2.0f - 1.0f;
	GLfloat yfactor = ((GLfloat)screen_height / (GLfloat)viewport_h) * 2.0f - 1.0f;
	GLfloat sfactor = (GLfloat)tex_geometry_w / (GLfloat)texture_screen_width;
	GLfloat tfactor = (GLfloat)tex_geometry_h / (GLfloat)texture_screen_height;

	
	Vertexs[0].x = -1.0f;
	Vertexs[0].y = -1.0f;
	Vertexs[0].z = position_z;
	Vertexs[0].s = 0.0f;
	Vertexs[0].t = tfactor;
	
	Vertexs[1].x = xfactor;
	Vertexs[1].y = -1.0f;
	Vertexs[1].z = position_z;
	Vertexs[1].s = sfactor;
	Vertexs[1].t = tfactor;
	
	Vertexs[2].x = xfactor;
	Vertexs[2].y = yfactor;
	Vertexs[2].z = position_z;
	Vertexs[2].s = sfactor;
	Vertexs[2].t = 0.0f;
			
	Vertexs[3].x = -1.0f;
	Vertexs[3].y = yfactor;
	Vertexs[3].z = position_z;
	Vertexs[3].s = 0.0f;
	Vertexs[3].t = 0.0f;

	if((vertex != NULL) && (vertex_buffer != NULL) && (program != NULL)) {
		vertex_buffer->setUsagePattern(mode);
		vertex->bind();
		vertex_buffer->bind();
		vertex_buffer->allocate(sizeof(VertexTexCoord_t) * 4);
		vertex->release();
		vertex_buffer->release();
		setNormalVAO(Vertexs, 4);
	}
}

void GLScreenPack::setNormalVAO(VertexTexCoord_t *tp,
								int size)
{
	int vertex_loc = program->attributeLocation("vertex");
	int texcoord_loc = program->attributeLocation("texcoord");

	vertex->bind();
	vertex_buffer->bind();

	vertex_buffer->write(0, tp, sizeof(VertexTexCoord_t) * size);
	program->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
	program->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
	program->setUniformValue("a_texture", 0);
			   
	vertex_buffer->release();
	vertex->release();
	program->enableAttributeArray(vertex_loc);
	program->enableAttributeArray(texcoord_loc);
}

VertexTexCoord_t GLScreenPack::getVertexPos(int pos)
{
	VertexTexCoord_t V;
	V.x = 0.0f;
	V.y = 0.0f;
	V.z = 0.0f;
	V.s = 0.0f;
	V.t = 0.0f;
	if((pos < 0) || (pos >= 4)) return V;
	memcpy(&V, &Vertexs[pos], sizeof(VertexTexCoord_t));
	return V;
}
	
