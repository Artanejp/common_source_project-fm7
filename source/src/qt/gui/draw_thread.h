/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from qt_main.h
	[ win32 main ] -> [ Qt main ] -> [Draw]
*/
#ifndef _CSP_QT_DRAW_THREAD_H
#define _CSP_QT_DRAW_THREAD_H

#include <QThread>
#include <memory>
#include "qt_gldraw.h"

class Ui_MainWindowBase;
class EMU_TEMPLATE;
class OSD_BASE;
class CSP_Logger;
class QSemaphore;
class QScreen;
class QOpenGLContext;
class USING_FLAGS;
QT_BEGIN_NAMESPACE
#include "../osd_types.h"

class DLL_PREFIX DrawThreadClass : public QThread {
	Q_OBJECT
 private:
	OSD_BASE *p_osd;
	config_t *p_config; 
	Ui_MainWindowBase *MainWindow;
	GLDrawClass *glv;
	
	QOpenGLContext *glContext;
	bool is_shared_glcontext;
	
	qreal refresh_rate;
	qreal wait_refresh;
	qreal wait_count;
	int wait_factor;
	int rec_frame_count;
	int rec_frame_width;
	int rec_frame_height;
	
 protected:
	std::shared_ptr<USING_FLAGS> using_flags;
	QScreen *screen;
	int draw_frames;
	bool bRunThread;
	bool bDrawReq;
	bool bRecentRenderStatus;
	bool use_separate_thread_draw;
	bitmap_t *draw_screen_buffer;
	std::shared_ptr<CSP_Logger> csp_logger;
	int ncount;
	double emu_frame_rate;

	bool mapping_status;
	scrntype_t *mapping_pointer;
	int mapping_width;
	int mapping_height;
	bool mapped_drawn;
	void doDrawMain(bool flag);
 public:
	DrawThreadClass(OSD_BASE *o, std::shared_ptr<CSP_Logger> logger, QObject *parent = 0);
	~DrawThreadClass();
	QSemaphore *renderSemaphore;
	QSemaphore *textureMappingSemaphore;
	
	void run() { doWork("");}
	void SetEmu(EMU_TEMPLATE *p);
public slots:
	void do_start_draw_thread(QThread::Priority prio);
	void do_set_priority(QThread::Priority prio);
	void doWork(const QString &);
	void doExit(void);
	void doDraw(bool flag);
	void do_change_refresh_rate(qreal rate);
	void do_update_screen(void *p, bool is_mapped);
	void do_req_encueue_video(int count, int width, int height);
	void do_draw_one_turn(bool _req_draw);
	void do_set_frames_per_second(double fps);

	void req_map_screen_texture();
	void req_unmap_screen_texture();

	bool is_glcontext_shared(void)
	{
		return is_shared_glcontext;
	}
	
	QOpenGLContext *get_gl_context(void)
	{
		return glContext;
	}
signals:
	int sig_draw_frames(int);
	int message_changed(QString);
	int sig_update_screen(void *, bool);
	int sig_update_osd(void);
	int sig_draw_timing(bool);
	int sig_push_frames_to_avio(int, int, int);
	int sig_call_draw_screen();
	int sig_call_no_draw_screen();
	int sig_map_texture();
	int sig_unmap_texture();
};

QT_END_NAMESPACE
#endif
