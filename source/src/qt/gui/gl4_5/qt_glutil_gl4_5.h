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
class QMutex;

class DLL_PREFIX GLDraw_4_5 : public GLDraw_Tmpl
{
	Q_OBJECT
private:
	QOpenGLFunctions_4_5_Core *extfunc;
	float ringing_phase;
protected:
	const float luma_filter[24 + 1] = {
		-0.000012020,
		-0.000022146,
		-0.000013155,
		-0.000012020,
		-0.000049979,
		-0.000113940,
		-0.000122150,
		-0.000005612,
		0.000170516,
		0.000237199,
		0.000169640,
		0.000285688,
		0.000984574,
		0.002018683,
		0.002002275,
		-0.000909882,
		-0.007049081,
		-0.013222860,
		-0.012606931,
		0.002460860,
		0.035868225,
		0.084016453,
		0.135563500,
		0.175261268,
		0.190176552
	};
	const float chroma_filter[24 + 1] = {
		-0.000118847,
		-0.000271306,
		-0.000502642,
		-0.000930833,
		-0.001451013,
		-0.002064744,
		-0.002700432,
		-0.003241276,
		-0.003524948,
		-0.003350284,
		-0.002491729,
		-0.000721149,
		0.002164659,
		0.006313635,
		0.011789103,
		0.018545660,
		0.026414396,
		0.035100710,
		0.044196567,
		0.053207202,
		0.061590275,
		0.068803602,
		0.074356193,
		0.077856564,
		0.079052396
	};
	const float rot0[4] =   {1, -0,  0, 1};
	const float rot90[4] =  {0,  1, -1,  0};
	const float rot180[4] = {-1, 0,  0, -1};
	const float rot270[4] = {0, -1,  1, 0};

	int gl_major_version;
	int gl_minor_version;

	int pixel_width;
	int pixel_height;
	GLuint main_texture_buffer;
	GLuint main_read_texture_buffer;
	GLsync sync_fence;
	QMutex *main_mutex;
	scrntype_t *map_base_address;
	
	GLScreenPack *main_pass;
	GLScreenPack *std_pass;
	GLScreenPack *ntsc_pass1;
	GLScreenPack *ntsc_pass2;
	GLScreenPack *bitmap_block;
	
	QOpenGLShaderProgram *grids_shader;
	QOpenGLBuffer *grids_horizonal_buffer;
	QOpenGLVertexArrayObject *grids_horizonal_vertex;
	QOpenGLBuffer *grids_vertical_buffer;
	QOpenGLVertexArrayObject *grids_vertical_vertex;

	GLuint uTmpTextureID;
	bool swap_byteorder;
	bool main_texture_ready;
	
	virtual void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
							  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4);
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
	virtual void resizeGL_Screen(void);
	virtual void initPackedGLObject(GLScreenPack **p,
									int _width, int _height,
									const QString vertex_shader, const QString fragment_shader,
									const QString _name,
									bool req_float = false, bool req_highp = false,
									bool req_alpha_channel = true);
	
	virtual void drawGridsHorizonal(void);
	virtual void drawGridsVertical(void);
	virtual void drawGrids();

	virtual void drawMain(QOpenGLShaderProgram *prg,
						  QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  GLuint texid,
						  QVector4D color,
						  bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f));
	virtual void drawMain(GLScreenPack *obj,
						  GLuint texid,
						  QVector4D color, bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f));
	virtual void renderToTmpFrameBuffer_nPass(GLuint src_texture,
										  GLuint src_w,
										  GLuint src_h,
										  GLScreenPack *renderObject,
										  GLuint dst_w,
										  GLuint dst_h,
										  bool use_chromakey = false);
	virtual void drawBitmapTexture(void);
	virtual void drawButtonsMain(int num, bool f_smoosing);
//	virtual void drawOsdLeds();
//	virtual void drawOsdIcons();
//	virtual void drawLedMain(GLScreenPack *obj, int num, QVector4D color);
	virtual void prologueBlending();
	virtual void epilogueBlending();
	virtual void drawPolygon(int vertex_loc, uintptr_t p = 0);
	virtual void set_led_vertex(int bit);
	virtual void set_osd_vertex(int bit);
	virtual void initBitmapVertex(void);
	virtual QOpenGLTexture *createMainTexture(QImage *img);
	void updateButtonTexture(void);

public:
	GLDraw_4_5(GLDrawClass *parent, USING_FLAGS *p, CSP_Logger *logger, EMU_TEMPLATE *emu = 0);
	~GLDraw_4_5();
	void drawButtons(void);
	virtual void initGLObjects();
	virtual void initLocalGLObjects(void);
	virtual void initFBO(void);
	void initButtons(void);
	//virtual void initBitmapVertex(void);
	
	virtual void uploadMainTexture(QImage *p, bool chromakey, bool was_mapped);
	virtual void drawScreenTexture(void);
	virtual void do_set_screen_multiply(float mul);
	virtual void doSetGridsHorizonal(int lines, bool force);
	virtual void doSetGridsVertical(int pixels, bool force);
	void uploadBitmapTexture(QImage *p);
	
// Note: Mapping vram from draw_thread does'nt work well.
// This feature might be disable. 20180728 K.Ohta.
	void get_screen_geometry(int *w, int *h);
	bool copy_screen_buffer(scrntype_t* target,int w, int h, int stride);

	scrntype_t *get_screen_buffer(int y);
	bool is_ready_to_map_vram_texture(void);
	bool map_vram_texture(void);
	bool unmap_vram_texture(void);
	virtual bool is_mapped_buffer(void) {
		if(main_texture_buffer == 0) {
			return false;
		}
		return true;
	}
	// ToDo: Double buffer
	virtual GLuint get_mapped_buffer_num(int region) {
		return (GLuint)main_texture_buffer;
	}

public slots:
	void updateBitmap(QImage *);
	void uploadIconTexture(QPixmap *p, int icon_type, int localnum);
	void setBrightness(GLfloat r, GLfloat g, GLfloat b);
	void do_set_texture_size(QImage *p, int w, int h);
	void do_set_horiz_lines(int lines);
	virtual void paintGL(void);
	virtual void resizeGL(int width, int height);

};
QT_END_NAMESPACE
#endif
