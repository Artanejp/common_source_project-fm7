/*
 * qt_gldraw.h
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */
#ifndef _CSP_QT_GLDRAW_H
#define _CSP_QT_GLDRAW_H

//#include "emu.h"
//#include "../osd_types.h"
#include "dropdown_keyset.h"
#include "config.h"
#include "menu_flags.h"


#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <QColor>
#include <QPixmap>
#include <QString>
#include <QPair>

class QEvent;
class GLDraw_Tmpl;
class CSP_KeyTables;
class USING_FLAGS;
class CSP_Logger;
class CSP_DrawItem;
struct NativeScanCode {
	uint32_t vk;
	uint32_t scan;
};
struct NativeVirtualKeyCode {
	uint32_t vk;
	uint32_t key;
};

class DLL_PREFIX GLDrawClass: public QOpenGLWidget 
{
	Q_OBJECT
 private:
	CSP_Logger *csp_logger;
	USING_FLAGS *using_flags;
	config_t *p_config;
	
	bool enable_mouse;
	GLfloat screen_width, screen_height;
	int vram_width;
	int vram_height;
	int draw_width;
	int draw_height;
	
	bool delay_update;
	
 protected:
	bool run_vm;
	QString render_string;
	
	void keyReleaseEvent(QKeyEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void initializeGL();
	void paintGL();
	void drawGrids(void);
	//void paintEvent(QPaintEvent *ev);

	uint32_t get106Scancode2VK(uint32_t data);
	bool QueryGLExtensions(const char *str);
	void InitGLExtensionVars(void);
	void InitContextCL(void);
	
	QString filename_screen_pixmap;
	bool save_pixmap_req;
	void SaveToPixmap(void);
	CSP_KeyTables *key_table;
	CSP_DrawItem *draw_item;

public:

	GLDrawClass(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0, const QSurfaceFormat &fmt = QSurfaceFormat::defaultFormat());
	~GLDrawClass();
	GLDraw_Tmpl *extfunc;
	
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
	
	void InitFBO(void);
	void closeEvent(QCloseEvent *event);
	void drawUpdateTexture(void *p, bool was_mapped);
	QString logGLString(bool getExtensions = false);
	bool emu_launched;
	quint32 modifier;

	const bool is_ready_to_map_vram_texture(void);
	scrntype_t *do_map_vram_texture(int *r_width, int *r_height);
	void do_unmap_vram_texture();
	bool is_mapped_buffer(void);
	GLuint get_mapped_buffer_num(int region);

	scrntype_t* get_screen_buffer(int y);
	bool copy_screen_buffer(scrntype_t* target, int w, int h, int stride);

	virtual QString getRenderString();
public slots:
	void initKeyCode(void);
	void releaseKeyCode(void);
	
	void update_screen(void *p, bool was_mapped);
	void update_osd(void);
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

	void set_emu_launched(void);
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);
	void do_save_frame_screen(void);
	void do_save_frame_screen(const char *);
	void do_set_texture_size(QImage *p, int w, int h);
	void do_set_horiz_lines(int lines);
	
	void do_set_screen_multiply(float mul);
	void do_update_keyboard_scan_code(uint32_t vk, uint32_t scan);
	void do_set_mouse_enabled(bool flag);
	void do_stop_run_vm();
	void do_enable_mouse(void);
	void do_disable_mouse(void);
	void do_move_mouse_to_center(void);
	void do_set_display_osd(bool onoff);
	void do_display_osd_leds(int lednum,bool onoff);
	void do_set_led_width(int bitwidth);
	void do_update_icon(int icon_type,  int localnum, QPixmap *p);
	void do_update_icon(int icon_type, int localnum, QString message, QColor bg, QColor fg, QColor fg2, QColor fg3, QColor lg, QColor tg, float pt);

signals:
	void update_screenChanged(int tick);
	void do_notify_move_mouse(int x, int y, int globalx, int globaly);

	void sig_toggle_mouse(void);
	void do_notify_button_pressed(Qt::MouseButton button);
	void do_notify_button_released(Qt::MouseButton button);
	void sig_check_grab_mouse(bool);
	void sig_resize_uibar(int, int);
	void sig_draw_timing(void);
	int sig_finished(void);
	int sig_key_down(uint32_t, uint32_t, bool);
	int sig_key_up(uint32_t, uint32_t);
	int sig_set_display_osd(bool);
	int sig_display_osd_leds(int,bool);
	int sig_resize_osd(int);

	int sig_map_texture_reply(bool, void *, int, int);
	int sig_unmap_texture_reply();
};

#endif // End.
