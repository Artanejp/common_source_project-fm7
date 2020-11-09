/*
 * qt_glutil_gl2_0.h
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL v2.0 .
 * History:
 * Jan 21, 2016 : Initial.
 */

#ifndef _QT_COMMON_GLUTIL_TMPL_H
#define _QT_COMMON_GLUTIL_TMPL_H

#include <QObject>
#include <QVector>
#include <QWidget>
#include <QString>
#include <QList>
#include <QStringList>
#include <QOpenGLTexture>

#include "config.h"
#include "common.h"
#include "qt_glpack.h"
#include "menu_flags.h"

QT_BEGIN_NAMESPACE
class EMU_TEMPLATE;
class QEvent;
class GLDrawClass;
class QOpenGLFramebufferObject;
class QOpenGLFramebufferObjectFormat;
class USING_FLAGS;
class CSP_Logger;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QOpenGLShaderProgram;
class GLScreenPack;

namespace GLShader {
class ShaderDesc {
protected:
	QString vertex_shader_name;
	QString fragment_shader_name;
	QString description;
public:	
	ShaderDesc(QString _vname = QString::fromUtf8(""), QString _fname = QString::fromUtf8(""), QString _desc = QString::fromUtf8("")) {
		vertex_shader_name = _vname;
		fragment_shader_name = _fname;
		description = _desc;
	}
	~ShaderDesc() {}
	void setShaders(QString _vname, QString _fname, QString _desc = QString::fromUtf8("")) {
		vertex_shader_name = _vname;
		fragment_shader_name = _fname;
		description = _desc;
	}
	QString getVertexShaderName() {
		return vertex_shader_name;
	}
	QString getFragmentShaderName() {
		return fragment_shader_name;
	}
	QString getDesc() {
		return description;
	}
	bool matchVertex(QString s, bool caseSensitive = true) {
		return (vertex_shader_name.compare(s, (caseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0) ? true : false;
	}
	bool matchFragment(QString s, bool caseSensitive = true) {
		return (fragment_shader_name.compare(s, (caseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0) ? true : false;
	}
	bool matchDesc(QString s, bool caseSensitive = true) {
		return (description.compare(s, (caseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0) ? true : false;
	}
};

class ShaderAttr {
protected:
	bool is_multiple_pass; // Shader needs multiple pass.
	bool is_multiple_frame_buffers; // Shader needs multiple frame buffer.
	int num_multiple_pass; // Nubmers of multiple pass.
	int num_multiple_frame_buffers; // Number of frame_buffers;
	QList<ShaderDesc> picture_shader_pair;
	QStringList compute_shader_name; // ToDo: Will change with some code.
public:		
	ShaderAttr(bool mupass = false, bool mbuf = false, int npass = 1, int nfbs = 2)
	{
		is_multiple_pass = false;
		is_multiple_frame_buffers = false;
		num_multiple_pass = 1;
		num_multiple_frame_buffers = 2;
		if(mupass) {
			is_multiple_pass = true;
			num_multiple_pass = npass;
		}
		if(mbuf) {
			is_multiple_frame_buffers = true;
			num_multiple_frame_buffers = nfbs;
		}
		picture_shader_pair.clear();
		compute_shader_name.clear();		
	}
	~ShaderAttr() {}

	QList<ShaderDesc> getShadersList() {
		return picture_shader_pair;
	}
	QStringList getComputeShadersList() {
		return compute_shader_name;
	}
	
	void addShaderPair(ShaderDesc n) {
		return picture_shader_pair.append(n);
	}
	ShaderDesc getShaderPair(int n) {
		return picture_shader_pair.at(n);
	}
	int numShaderPairs() {
		return picture_shader_pair.size();		
	}
	bool isEmptyShaderPairs() {
		return picture_shader_pair.isEmpty();		
	}
	void addComputeShader(QString name) {
		compute_shader_name.append(name);		
	}
	QString getComputeShader(int num) {
		if((num < 0) || (num >= compute_shader_name.size())) return QString::fromUtf8(""); 
		return compute_shader_name.at(num);		
	}
	int numComputeShaders() {
		return compute_shader_name.size();		
	}
	bool isEmptyComputeShaders() {
		return compute_shader_name.isEmpty();		
	}
	
};

class ShaderGroup {
protected:
	ShaderAttr attr;
	QList<GLScreenPack*> effect_shaders; //Compiled Pixel shaders
public:
	ShaderGroup(ShaderAttr _a, GLScreenPack* p = NULL) {
		attr = _a;
		effect_shaders.clear();
		if(p != NULL) {
			effect_shaders.append(p);
		}
	}
	~ShaderGroup() { }

	QList<GLScreenPack*> getShaders() {
		return effect_shaders;
	}
	ShaderAttr attribute() {
		return attr;
	}
	void append(GLScreenPack* p) {
		if(p != NULL) {
			effect_shaders.append(p);
		}
	}
	GLScreenPack *at(int num) {
		if((num < 0) || (num >= effect_shaders.size())) return nullptr;
		return effect_shaders.at(num);
	}
	int size() {
		return effect_shaders.size();
	}
	bool isEmpty() {
		return effect_shaders.isEmpty();
	}
};
}
// End namespace GLShader
	
class DLL_PREFIX GLDraw_Tmpl : public QObject
{
	Q_OBJECT
protected:
	EMU_TEMPLATE* p_emu;
	GLDrawClass *p_wid;
	
	USING_FLAGS *using_flags;
	QImage *imgptr;
	CSP_Logger *csp_logger;
	config_t *p_config;

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
	float ringing_phase;
	
	bool smoosing;
	bool gl_grid_horiz;
	bool gl_grid_vert;
	
	int  vert_lines;
	int  horiz_pixels;
	GLfloat *glVertGrids;
	GLfloat *glHorizGrids;
	float screen_multiply;

	float screen_width;
	float screen_height;
	
	int screen_texture_width;
	int screen_texture_width_old;
	int screen_texture_height;
	int screen_texture_height_old;

	QOpenGLTexture *icon_texid[10][10];

	int rec_count;
	int rec_width;
	int rec_height;

	VertexTexCoord_t vertexFormat[4];
	QOpenGLShaderProgram *main_shader;
	
	QOpenGLVertexArrayObject *vertex_screen;
	QOpenGLBuffer *buffer_screen_vertex;
	
	GLScreenPack *led_pass;
	GLScreenPack *osd_pass;
	QOpenGLBuffer *led_pass_vbuffer[32];
	QOpenGLVertexArrayObject *led_pass_vao[32];
	QOpenGLBuffer *osd_pass_vbuffer[32];
	QOpenGLVertexArrayObject *osd_pass_vao[32];

	VertexTexCoord_t vertexTmpTexture[4];
	VertexTexCoord_t vertexBitmap[4];
	QOpenGLShaderProgram *bitmap_shader;
	QOpenGLBuffer *buffer_bitmap_vertex;
	QOpenGLVertexArrayObject *vertex_bitmap;
	QOpenGLVertexArrayObject *vertex_button[128];
	QOpenGLBuffer *buffer_button_vertex[128];
	QOpenGLShaderProgram *button_shader;
	VertexTexCoord_t vertexOSD[32][4];
	QOpenGLVertexArrayObject *vertex_osd[32];
	QOpenGLBuffer *buffer_osd[32];
	QOpenGLShaderProgram *osd_shader;

	QOpenGLTexture *uVramTextureID;
	QOpenGLTexture *uButtonTextureID[128];
	GLfloat fButtonX[128];
	GLfloat fButtonY[128];
	GLfloat fButtonWidth[128];
	GLfloat fButtonHeight[128];
	QVector<VertexTexCoord_t> *vertexButtons;
	QVector<QImage> ButtonImages;
	bool button_updated;
	bool set_brightness;
	bool InitVideo;
	
	QOpenGLTexture *uBitmapTextureID;
	bool bitmap_uploaded;
	bool button_drawn;
	bool crt_flag;
	bool redraw_required;


	GLfloat fBrightR;
	GLfloat fBrightG;
	GLfloat fBrightB;
	
	QOpenGLFramebufferObject *offscreen_frame_buffer;
	QOpenGLFramebufferObjectFormat *offscreen_frame_buffer_format;
	QImage offscreen_image;
	GLint texture_max_size;
	
	bool low_resolution_screen;
	bool emu_launched;

	uint32_t osd_led_status;
	uint32_t osd_led_status_bak;
	int osd_led_bit_width;
	bool osd_onoff;

	virtual void drawBitmapTexture(void) { }
	virtual void initButtons(void) { }
	virtual void initBitmapVertex(void) { }
	virtual void initBitmapVAO(void) { }
	virtual void updateButtonTexture(void) { }

	virtual void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
							  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4) { }
	
	virtual void resizeGL_SetVertexs(void) { }
	virtual void resizeGL_Screen(void) { }
	virtual void drawGridsHorizonal(void) { }
	virtual void drawGridsVertical(void) { }
	virtual void drawGridsMain(GLfloat *tp,
							   int number,
							   GLfloat lineWidth = 0.2f,
							   QVector4D color = QVector4D(0.0f, 0.0f, 0.0f, 1.0f)) { }
	virtual void drawButtons() { }
	virtual void prologueBlending() {}
	virtual void epilogueBlending() {}
	virtual void drawPolygon(int vertex_loc, uintptr_t p = 0) {}
	
	virtual void drawLedMain(GLScreenPack *obj, int num, QVector4D color);
	virtual void drawOsdLeds();
	virtual void drawOsdIcons();
	virtual void set_osd_vertex(int xbit) { }

public:
	GLDraw_Tmpl(GLDrawClass *parent, USING_FLAGS *p, CSP_Logger *logger, EMU_TEMPLATE *emu = 0);
	~GLDraw_Tmpl();
	virtual void initGLObjects() {}
	virtual void initFBO(void) {}
	virtual void initLocalGLObjects(void) {}
	virtual void initOsdObjects(void) {}

	virtual void uploadMainTexture(QImage *p, bool chromakey, bool was_mapped) {}

	virtual void drawScreenTexture(void) {}
	virtual void drawGrids(void) { }
	virtual void uploadBitmapTexture(QImage *p) {}

	virtual void drawMain(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  VertexTexCoord_t *vertex_data,
						  GLuint texid,
						  QVector4D color, bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f)) {}
	virtual void drawMain(QOpenGLShaderProgram *prg,
						  QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  GLuint texid,
						  QVector4D color,
						  bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f)) {}
	
	virtual void drawMain(GLScreenPack *obj,
						  GLuint texid,
						  QVector4D color, bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f)) {}
	
	virtual void doSetGridsHorizonal(int lines, bool force);
	virtual void doSetGridsVertical(int pixels, bool force);
	virtual bool copy_screen_buffer(scrntype_t* target,int w, int h, int stride) { return false;};
	virtual scrntype_t *get_screen_buffer(int y) { return NULL; }
	virtual void get_screen_geometry(int *w, int *h) {
		if(w != NULL) *w = 0;
		if(h != NULL) *h = 0;
	}
	virtual bool is_ready_to_map_vram_texture(void) { return false; }
	virtual bool map_vram_texture(void) { return false; }
	virtual bool unmap_vram_texture(void) { return false; }

public slots:
	virtual void paintGL(void) { }
	virtual void resizeGL(int width, int height) { }
	virtual void initializeGL() { }

	virtual void setBrightness(GLfloat r, GLfloat g, GLfloat b) { }
	virtual void do_set_texture_size(QImage *p, int w, int h) { }
	virtual void do_set_horiz_lines(int lines) { }
	virtual void setImgPtr(QImage *p) { }
	
	virtual void setSmoosing(bool) { }
	virtual void setDrawGLGridVert(bool) { }
	virtual void setDrawGLGridHoriz(bool) { }
	
	virtual void setVirtualVramSize(int ,int) { }	
	virtual void setChangeBrightness(bool) { }
	virtual void do_set_screen_multiply(float mul) { }
	
	virtual void updateBitmap(QImage *) { }
	virtual void paintGL_OffScreen(int count, int w, int h) { }
	virtual void uploadIconTexture(QPixmap *p, int icon_type, int localnum) { }
	
	virtual void set_emu_launched(void) { }
	virtual void do_set_display_osd(bool onoff);
	virtual void do_display_osd_leds(int lednum, bool onoff);
	virtual void do_set_led_width(int bitwidth) { }
	virtual bool is_mapped_buffer(void) { return false; }
	virtual GLuint get_mapped_buffer_num(int region) { return (GLuint)0; }
signals:
	int sig_push_image_to_movie(int, int, int, QImage *);
};

QT_END_NAMESPACE

#endif // _QT_COMMON_GLUTIL_2_0_H
