/*
 * qt_gldraw.h
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */
#ifndef _CSP_QT_GLDRAW_H
#define _CSP_QT_GLDRAW_H

//#include "emu.h"
#include "../osd_types.h"
#include "dropdown_keyset.h"
#include "menu_flags.h"

#if defined(_USE_GLAPI_QT5_4)
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#else
#include <QGLFormat>
#include <QGLWidget>
#endif
class EMU;
class QEvent;
class GLDraw_2_0;
class GLDraw_3_0;
class CSP_KeyTables;
class USING_FLAGS;

struct NativeScanCode {
	uint32_t vk;
	uint32_t scan;
};
struct NativeVirtualKeyCode {
	uint32_t vk;
	uint32_t key;
};

#if defined(_USE_GLAPI_QT5_4)
class DLL_PREFIX GLDrawClass: public QOpenGLWidget 
#else
class DLL_PREFIX GLDrawClass: public QGLWidget
#endif
{
	Q_OBJECT
 private:
	EMU *p_emu;
	USING_FLAGS *using_flags;
	bool is_mouse_enabled;
	
	bool enable_mouse;
	GLfloat screen_width, screen_height;
	int vram_width;
	int vram_height;
	int draw_width;
	int draw_height;
	
	bool delay_update;

 protected:
	bool run_vm;

	void keyReleaseEvent(QKeyEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void initializeGL();
	void paintGL();
	void drawGrids(void);

	uint32_t get106Scancode2VK(uint32_t data);
	bool QueryGLExtensions(const char *str);
	void InitGLExtensionVars(void);
	void InitContextCL(void);
	
	QString filename_screen_pixmap;
	bool save_pixmap_req;
	void SaveToPixmap(void);
	CSP_KeyTables *key_table;

public:
#if defined(_USE_GLAPI_QT5_4)
	GLDrawClass(USING_FLAGS *p, QWidget *parent = 0, const QSoufaceFormat &fmt = QSurfaceFormat::defaultFormat());
#else
	GLDrawClass(USING_FLAGS *p, QWidget *parent = 0, const QGLFormat &fmt = QGLFormat::defaultFormat());
#endif
	~GLDrawClass();
	GLDraw_2_0 *extfunc;
	
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	QSize getCanvasSize();
	QSize getDrawSize();
	
	QStringList *getKeyNames(void);
	QStringList *getVKNames(void);
	keydef_table_t *get_key_table(int index);
	keydef_table_t *get_key_tables(void);
	keydef_table_t *get_base_key_table(void);
	const keydef_table_t *get_default_key_table(void);
	int get_key_table_size(void);
	uint32_t get_vk_from_index(int index);
	uint32_t get_scan_from_index(int index);
	const char *get_key_vk_name(int index);
	quint32 getModState(void) { return modifier;}
	quint32 modifier;
	void InitFBO(void);
	void closeEvent(QCloseEvent *event);
	void drawUpdateTexture(bitmap_t *p);
	QString logGLString(bool getExtensions = false);

public slots:
	void initKeyCode(void);
	void releaseKeyCode(void);
	
	void update_screen(bitmap_t *);
	void resizeGL(int width, int height);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	void setEnableMouse(bool flag);
	void setSmoosing(bool);
	void setDrawGLGridVert(bool);
	void setDrawGLGridHoriz(bool);
	void setVirtualVramSize(int ,int);	
	void setChangeBrightness(bool);
	void setBrightness(GLfloat r, GLfloat g, GLfloat b);
	
	void updateBitmap(QImage *);

	void setEmuPtr(EMU *p);
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);
	void do_save_frame_screen(void);
	void do_save_frame_screen(const char *);
	void do_set_texture_size(QImage *p, int w, int h);
	
	void do_set_screen_multiply(float mul);
	void do_update_keyboard_scan_code(uint32_t vk, uint32_t scan);
	void do_set_mouse_enabled(bool flag);
	void do_stop_run_vm();
signals:
	void update_screenChanged(int tick);
	void do_notify_move_mouse(int x, int y);

	void sig_toggle_mouse(void);
	void do_notify_button_pressed(Qt::MouseButton button);
	void do_notify_button_released(Qt::MouseButton button);
	void sig_check_grab_mouse(bool);
	void sig_resize_uibar(int, int);
//	void sig_draw_timing(bool);
	int sig_finished(void);
	int sig_key_down(uint32_t, uint32_t, bool);
	int sig_key_up(uint32_t, uint32_t);
};

#endif // End.
