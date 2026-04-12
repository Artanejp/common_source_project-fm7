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
#include <QElapsedTimer>
#include <mutex>
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

	std::atomic<bool>  m_about_to_quit;
	std::atomic<qreal> m_refresh_rate;
	std::atomic<qreal> m_wait_refresh;
	std::atomic<qreal> m_wait_count;
	std::atomic<int>   m_wait_factor;
	std::atomic<int>   m_rec_frame_count;
	std::atomic<int>   m_rec_frame_width;
	std::atomic<int>   m_rec_frame_height;
	
 protected:
	std::recursive_mutex m_main_locker;
	std::recursive_timed_mutex m_worker_locker;
	std::shared_ptr<USING_FLAGS> using_flags;
	QScreen *screen;

	bitmap_t *draw_screen_buffer;
	std::shared_ptr<CSP_Logger> csp_logger;

	std::atomic<bool>   m_req_draw;
	std::atomic<double> m_emu_frame_rate;

	std::atomic<bool> m_vsync_happened;
	std::atomic<bool> m_drawreq_from_host;
	std::atomic<bool> m_update_req;	
	std::atomic<bool> m_mapping_status;
	std::atomic<bool> m_mapped_drawn;

	int m_draw_frames;
	int m_ncount;
	std::atomic<bool> m_req_lock;
	std::atomic<bool> m_ack_lock;
	
	void run() override;
	//QElapsedTimer tick_timer;
 public:
	DrawThreadClass(OSD_BASE *o, std::shared_ptr<CSP_Logger> logger, QObject *parent = 0);
	~DrawThreadClass();
	QSemaphore *textureMappingSemaphore;
	
	void SetEmu(EMU_TEMPLATE *p);
	inline void lock_draw_thread()
	{
		m_ack_lock = false;
		m_req_lock = true;
		m_main_locker.lock();
		while(!(m_ack_lock.load())) {
			msleep(1);
		}
	}
	inline void unlock_draw_thread()
	{
		m_ack_lock = false;
		m_req_lock = false;
		m_main_locker.unlock();
	}
								
public slots:
	void do_exit_draw_thread(void);
	void do_set_priority(QThread::Priority prio);
	
	void do_draw(bool flag);
	void do_change_refresh_rate(qreal rate);
	void do_update_screen(void *p, bool is_mapped, bool already_drawn);
	void do_req_encueue_video(int count, int width, int height);
	void do_draw_one_turn(bool _req_draw);
	void do_set_frames_per_second(double fps);

	void do_vsync();

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
