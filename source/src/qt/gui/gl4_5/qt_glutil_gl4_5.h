/*
 * qt_glutil_gl4_5.cpp
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL v4.5 (extend from renderer with OpenGL v2.0).
 * History:
 * Jan 22, 2016 : Initial.
 */

#ifndef _QT_COMMON_GLUTIL_4_5_H
#define _QT_COMMON_GLUTIL_4_5_H

#include <QString>
#include "../gl/qt_glutil_gl_tmpl.h"

QT_BEGIN_NAMESPACE
class GLScreenPack;
class CSP_Logger;
class QOpenGLFunctions_4_5_Core;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QOpenGLShaderProgram;
class QOpenGLPixelTransferOptions;
#if QT_VERSION >= 0x051400
class QRecursiveMutex;
#else
class QMutex;
#endif

class DLL_PREFIX GLDraw_4_5 : public GLDraw_Tmpl
{
	Q_OBJECT
private:
	QOpenGLFunctions_4_5_Core *extfunc;

protected:
	int pixel_width;
	int pixel_height;
	GLuint main_texture_buffer;
	GLuint main_read_texture_buffer;
	GLsync sync_fence;

#if QT_VERSION >= 0x051400
	QRecursiveMutex *main_mutex;
#else
	QMutex *main_mutex;
#endif
	
	scrntype_t *map_base_address;

	virtual void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
							  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4) override;
	virtual bool initGridShaders(const QString vertex_fixed, const QString vertex_rotate, const QString fragment);
	virtual bool initGridVertexObject(QOpenGLBuffer **vbo, QOpenGLVertexArrayObject **vao, int alloc_size);
	virtual void set_texture_vertex(float wmul = 1.0f, float hmul = 1.0f);
	virtual void updateGridsVAO(QOpenGLBuffer *bp,
									QOpenGLVertexArrayObject *vp,
									GLfloat *tp,
									int number);

	virtual void drawGridsMain(QOpenGLShaderProgram *prg,
								  QOpenGLBuffer *bp,
								  QOpenGLVertexArrayObject *vp,
								  int number,
								  GLfloat lineWidth = 0.2f,
								  QVector4D color = QVector4D(0.0, 0.0, 0.0, 1.0));
	virtual void resizeGL_Screen(void) override;
	virtual void initPackedGLObject(GLScreenPack **p,
									int _width, int _height,
									const QString vertex_shader, const QString fragment_shader,
									const QString _name,
									bool req_float = false, bool req_highp = false,
									bool req_alpha_channel = true);
	
	virtual void drawGridsHorizonal(void) override;
	virtual void drawGridsVertical(void) override;
	virtual void drawGrids() override;

	virtual void drawMain(QOpenGLShaderProgram *prg,
						  QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  GLuint texid,
						  QVector4D color,
						  bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f)) override;
	virtual void drawMain(GLScreenPack *obj,
						  GLuint texid,
						  QVector4D color, bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f)) override;
	virtual void renderToTmpFrameBuffer_nPass(GLuint src_texture,
										  GLuint src_w,
										  GLuint src_h,
										  GLScreenPack *renderObject,
										  GLuint dst_w,
										  GLuint dst_h,
										  bool use_chromakey = false) override;
	virtual void drawBitmapTexture(void) override;
	virtual void drawButtonsMain(int num, bool f_smoosing) override;
//	virtual void drawOsdLeds();
//	virtual void drawOsdIcons();
//	virtual void drawLedMain(GLScreenPack *obj, int num, QVector4D color);
	virtual void prologueBlending() override;
	virtual void epilogueBlending() override;
	virtual void drawPolygon(int vertex_loc, uintptr_t p = 0) override;

	virtual QOpenGLTexture *createMainTexture(QImage *img) override;
public:
	GLDraw_4_5(GLDrawClass *parent, std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, EMU_TEMPLATE *emu = 0);
	~GLDraw_4_5();
	void drawButtons(void) override;
	virtual void initGLObjects() override;
	virtual void initLocalGLObjects(void) override;
	virtual void initFBO(void) override;

	//virtual void initBitmapVertex(void) override;
	
	virtual void uploadMainTexture(QImage *p, bool chromakey, bool was_mapped) override;
	virtual void drawScreenTexture(void) override;
	virtual void do_set_screen_multiply(float mul) override;
	virtual void doSetGridsHorizonal(int lines, bool force) override;
	virtual void doSetGridsVertical(int pixels, bool force) override;
	
// Note: Mapping vram from draw_thread does'nt work well.
// This feature might be disable. 20180728 K.Ohta.
	void get_screen_geometry(int *w, int *h) override;
	bool copy_screen_buffer(scrntype_t* target,int w, int h, int stride) override;

	scrntype_t *get_screen_buffer(int y) override;
	bool is_ready_to_map_vram_texture(void) override;
	bool map_vram_texture(void) override;
	bool unmap_vram_texture(void) override;
	virtual bool is_mapped_buffer(void) override
	{
		if(main_texture_buffer == 0) {
			return false;
		}
		return true;
	}
	// ToDo: Double buffer
	virtual GLuint get_mapped_buffer_num(int region) override
	{
		return (GLuint)main_texture_buffer;
	}
	virtual void resizeGL_Pre(int width, int height) override;

public slots:
	void do_set_texture_size(QImage *p, int w, int h) override;
	void do_set_horiz_lines(int lines) override;
	virtual void paintGL(void) override;
};
QT_END_NAMESPACE
#endif
