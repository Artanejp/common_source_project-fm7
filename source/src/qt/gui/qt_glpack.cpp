#include "qt_glpack.h"



GLScreenPack::GLScreenPack(QObject *parent) : QObject(parent)
{
	program = new QOpenGLShaderProgram(this);
	
	vertex_buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	vertex = new QOpenGLVertexArrayObject;

	init_status = false;
	shader_status = false;
	
	for(int i = 0; i < 4; i++) {
		Vertexs[i].x = 0.0;
		Vertexs[i].y = 0.0;
		Vertexs[i].z = -0.8;
		Vertexs[i].s = 0.0;
		Vertexs[i].t = 0.0;
	}
	viewport_w = 0;
	viewport_h = 0;
	viewport_w = 0;
	viewport_y = 0;

	tex_geometry_w = 0;
	tex_geometry_h = 0;
	tex_geometry_x = 0;
	tex_geometry_y = 0;

	Texture = 0;
}

bool GLScreenPack::initialize(int total_width, int total_height, int width, int height, const QString &vertex_shader_file, const QString &fragment_shader_file)
{
	
	viewport_w = total_width;
	viewport_h = total_height;
	viewport_w = 0;
	viewport_y = 0;

	tex_geometry_w = width;
	tex_geometry_h = height;
	tex_geometry_x = 0;
	tex_geometry_y = 0;
	
	if(program != NULL) {
		shader_status  = program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertex_shader_file);
		shader_status &= program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragment_shader_file);
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
			GLfloat sfactor = 1.0f;
			GLfloat tfactor = 1.0f;
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
			   
	//extfunc_3_0->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord_t), 0); 
	//extfunc_3_0->glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord_t), 
	//						       (char *)NULL + 3 * sizeof(GLfloat)); 
	vertex_buffer->release();
	vertex->release();
	program->enableAttributeArray(vertex_loc);
	program->enableAttributeArray(texcoord_loc);
}

void GLScreenPack::setTexture(GLuint id)
{
	Texture = id;
}

GLuint GLScreenPack::getTexture()
{
	return Texture;
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
	
