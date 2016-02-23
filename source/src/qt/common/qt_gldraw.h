/*
 * qt_gldraw.h
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */
#ifndef _CSP_QT_GLDRAW_H
#define _CSP_QT_GLDRAW_H

#include "emu.h"
#include "osd.h"
#include "dropdown_keyset.h"

#include <QGLWidget>
class EMU;
class QEvent;
class GLDraw_2_0;
class GLDraw_3_0;
class CSP_KeyTables;

struct NativeScanCode {
	uint32_t vk;
	uint32_t scan;
};
struct NativeVirtualKeyCode {
	uint32_t vk;
	uint32_t key;
};

class GLDrawClass: public QGLWidget 
{
	Q_OBJECT
 private:
	EMU *p_emu;

	bool enable_mouse;
	GLfloat screen_width, screen_height;
	int vram_width;
	int vram_height;
	int draw_width;
	int draw_height;
	
	bool delay_update;

 protected:
	void keyReleaseEvent(QKeyEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void initializeGL();
	void paintGL();
	void drawGrids(void);

	uint32_t get106Scancode2VK(uint32_t data);
#ifdef _USE_OPENCL
	//     extern class GLCLDraw *cldraw;
#endif
	bool QueryGLExtensions(const char *str);
	void InitGLExtensionVars(void);
	void InitContextCL(void);
	
	QString filename_screen_pixmap;
	bool save_pixmap_req;
	void SaveToPixmap(void);
	GLDraw_2_0 *extfunc;
	CSP_KeyTables *key_table;
	
public:
	GLDrawClass(QWidget *parent = 0);
	~GLDrawClass();

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	QSize getCanvasSize();
	QSize getDrawSize();
	
	QStringList *getKeyNames(void);
	QStringList *getVKNames(void);
	keydef_table_t *getKeyTable(int index);
	uint32 get_vk_from_index(int index);
	uint32 get_scan_from_index(int index);

	quint32 getModState(void) { return modifier;}
	quint32 modifier;
	void InitFBO(void);
	void closeEvent(QCloseEvent *event);
	void drawUpdateTexture(bitmap_t *p);

public slots:
	void initKeyCode(void);
	void releaseKeyCode(void);
	
	void update_screen(bitmap_t *);
	void resizeGL(int width, int height);
#if defined(ONE_BOARD_MICRO_COMPUTER) || defined(USE_MOUSE)
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
#endif	
	void setEnableMouse(bool flag);
	void setSmoosing(bool);
	void setDrawGLGridVert(bool);
	void setDrawGLGridHoriz(bool);
	void setVirtualVramSize(int ,int);	
	void setChangeBrightness(bool);
	void setBrightness(GLfloat r, GLfloat g, GLfloat b);
	
#ifdef ONE_BOARD_MICRO_COMPUTER
	void updateBitmap(QImage *);
#endif   
	void setEmuPtr(EMU *p);
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);
	void do_save_frame_screen(void);
	void do_save_frame_screen(const char *);
	void do_set_texture_size(QImage *p, int w, int h);
	
	void do_set_screen_multiply(float mul);
	void do_update_keyboard_scan_code(uint32 vk, uint32 scan);
signals:
	void update_screenChanged(int tick);
	void do_notify_move_mouse(int x, int y);
	void sig_toggle_mouse(void);
	void do_notify_button_pressed(Qt::MouseButton button);
	void do_notify_button_released(Qt::MouseButton button);
	void sig_check_grab_mouse(bool);
	void sig_resize_uibar(int, int);
	void sig_draw_timing(bool);
	int sig_finished(void);
};

#endif // End.
