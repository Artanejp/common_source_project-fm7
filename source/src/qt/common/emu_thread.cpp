/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2015.11.10 Split from qt_main.cpp
	Note: This class must be compiled per VM, must not integrate shared units.
	[ win32 main ] -> [ Qt main ] -> [Emu Thread]
*/

#include <QString>
#include <QTextCodec>
#include <QWaitCondition>
#include <QTimer>

#include <SDL.h>

#include "emu_thread.h"
#include "../gui/dock_disks.h"

#include "qt_gldraw.h"
#include "csp_logger.h"
#include "menu_flags.h"
#include "../osd.h"
#include "mainwidget_base.h"
#include "../../fileio.h"

// buttons
#define MAX_SKIP_FRAMES 10

EMU* DLL_PREFIX emu;

EmuThreadClass::EmuThreadClass(Ui_MainWindowBase *rootWindow, std::shared_ptr<USING_FLAGS> p, QObject *parent)
	: EmuThreadClassBase(rootWindow, p, parent)
{
//	emu = new EMU((Ui_MainWindow *)rootWindow, rootWindow->getGraphicsView(), using_flags);
	emu = new EMU((Ui_MainWindow *)rootWindow, rootWindow->getGraphicsView(), rootWindow->get_logger(), p);
	p_emu = emu;
	p_osd = emu->get_osd();
	p->set_emu(emu);
	p->set_osd((OSD*)p_osd);

	m_poweroff_notified = false;

	p_osd->setParent(this);
	//p_osd->moveToThread(this);
	connect(p_osd, SIGNAL(sig_notify_power_off()), this, SLOT(do_notify_power_off()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_restart_sound_timer()), p_osd, SLOT(do_restart_sound_timer()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_stop_sound_timer()), p_osd, SLOT(do_stop_sound_timer()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_sound_stop()), p_osd, SLOT(stop_sound()), Qt::QueuedConnection);

	//m_call_timer = new QTimer(this);
	//m_call_timer->setTimerType(Qt::PreciseTimer);
	//m_call_timer->setSingleShot(true);
	
	//connect(m_call_timer, SIGNAL(timeout()), this, SLOT(doWork()));
	//connect(this, SIGNAL(sig_timer_start(int)), m_call_timer, SLOT(start(int)));
	//connect(this, SIGNAL(sig_timer_stop()), m_call_timer, SLOT(stop()));
	//connect(this, SIGNAL(finished()), m_call_timer, SLOT(stop()));
	m_interval = 0;
	m_sleep_period = 0;
	m_run_frames = 0;
	m_current_time = 0;
	m_first = true;
	// LED
	m_led_data_old = 0x00000000;
	// Tape
	// DIG_RESOLUTION
	//
	m_req_draw = true;
	m_vert_line_bak = false;
	m_horiz_line_bak = false;
	m_gl_crt_filter_bak = false;
	m_opengl_filter_num_bak = 0;
	//connect(this, SIGNAL(sig_call_initialize()), this, SLOT(do_initialize()));
	//connect(this, SIGNAL(started()), this, SLOT(do_initialize()));
}

EmuThreadClass::~EmuThreadClass()
{
}

#include <QStringList>
#include <QFileInfo>

void EmuThreadClass::run()
{
	do_initialize();
	doWork();
}
void EmuThreadClass::do_initialize()
{
	check_scanline_params(true);
	m_req_draw = true;
	//uint32_t key_mod_old = 0xffffffff;
	initialize_variables();

	clear_key_queue();
	m_thread_id = currentThreadId();
	
	emit sig_emu_launched();
	m_block_task = false;
//	m_current_time = get_current_tick_usec();
//	emit sig_timer_start(10);

}

void EmuThreadClass::doWork()
{
	if(!(m_run_thread.load()) && !(m_first)) {
		return;
	}
	std::shared_ptr<CSP_Logger> csp_logger = p_osd->get_logger();
	std::shared_ptr<USING_FLAGS> u_p = using_flags;

	bool is_up_null = (u_p.get() == nullptr);
	m_thread_id = currentThreadId();
	
	is_up_null = (u_p.get() == nullptr);
	QElapsedTimer led_timer;
	bool req_calc_sleep = false;
	m_next_time = 0;

	do {
		if((MainWindow == NULL) || (m_block_task.load()) || (is_up_null)) {
			if(m_run_thread.load() == false){
				break;
			}
			//emit sig_timer_start(10);
			//return;
			//do_print_framerate(0);
			msleep(10);
			m_next_time = 0;
			continue;
		}
		if(!(m_queue_cpu_affinities.empty())) {
			do_apply_cpu_affinities_to_emu_thread();
		}
		if(m_first) {
			if(initialize_messages()) {
				if(!(is_up_null)) {
					if((u_p->get_use_led_devices() > 0) || (u_p->get_use_key_locked())) {
						emit sig_force_redraw_leds();
					}
				}
				m_first = false;
				m_nr_fps = get_emu_frame_rate();
				emit sig_restart_sound_timer();
				m_current_time = get_current_tick_usec();
				m_half_count = false;
				m_driven_by_half_of_frame = false;
			}
			led_timer.start();
			m_next_time = 0;
		}
		m_interval = 0;
		if(m_run_thread.load() == false){
			break;
		}

		__LIKELY_IF(p_emu != NULL) {
			// drive machine
			__LIKELY_IF(!(m_half_count) || !(m_driven_by_half_of_frame)) { // Start of frame.
				process_command_queue();
				check_power_off();
				check_scanline_params(false);
			}
			
			m_current_time = get_current_tick_usec();
			process_key_input();

			m_run_frames = p_emu->run();
			m_half_count = p_emu->is_half_event();
			m_driven_by_half_of_frame = p_emu->is_driven_by_half_of_frame();
			#if 1
			if((m_driven_by_half_of_frame) && !(m_half_count)) {
				m_total_frames++;
			} else if(!(m_driven_by_half_of_frame)) {
				m_total_frames++;
			}
			#endif
			if(m_run_frames > 0) {
				m_total_frames += m_run_frames;
			}
			

			// After frame, delayed open
			if(m_run_thread.load() == false){
				break;
			}
			set_led();
			
			__LIKELY_IF(!(m_half_count) || !(m_driven_by_half_of_frame)) { // End of a frame.
//				set_led();
				sample_access_drv();
				__LIKELY_IF(p_config != nullptr) {
					m_full_speed = p_config->full_speed;
				} else {
					m_full_speed = false;
				}
				m_now_skip = (((m_full_speed) || p_emu->is_frame_skippable()) && !(p_emu->is_video_recording())) ? true : false;
				__UNLIKELY_IF(((m_prev_skip) && !(m_now_skip)) || (m_next_time <= 0)) {
					m_next_time = get_current_tick_usec();
				}				
				double nd;
				nd = get_emu_frame_rate();
				if(m_nr_fps != nd) emit sig_set_draw_fps(nd);
				m_nr_fps = nd;
				m_prev_skip = m_now_skip;
			}
			__LIKELY_IF(!(m_half_count) || !(m_driven_by_half_of_frame)) { // End of a frame.
				if(!(is_up_null) && (p_config != nullptr)) {
					if((u_p->is_support_tv_render()) && (p_config->rendering_type == CONFIG_RENDER_TYPE_TV)) {
						m_req_draw = true;
					}
				}
				if(!(m_req_draw)) {
					if(m_next_time <= m_current_time) { // Even draw
						if(++m_skip_frames > MAX_SKIP_FRAMES) {
							m_req_draw = true;
							m_skip_frames = 0;
							m_next_time = get_current_tick_usec();
						}
					} else {
						req_calc_sleep = true;
					}
				} else {
					m_skip_frames = 0;
					req_calc_sleep = true;
				}
				//printf("DRAW %dmsec\n", get_current_tick_usec());
				do_print_framerate(0);
				if(m_req_draw) {
					p_emu->request_update_screen();
				}
				#if 0
				if(p_osd != nullptr) {
					p_osd->do_draw(m_req_draw); // Call OSD , then off;pading by OSD to DRAW_THREAD.
				}
				#else
					emit sig_draw_thread(m_req_draw); // Call offloading thread.
				#endif
				if(led_timer.hasExpired(100)) { // Update at least 100mSec.
					if((u_p->get_use_led_devices() > 0) || (u_p->get_use_key_locked())) {
						emit sig_force_redraw_leds();
					}
					led_timer.restart();
				}
				
				//if(req_draw) {
				//	yieldCurrentThread(); // Yield current thread;
				//}
				m_req_draw = false;
			} else {
				// Middle of frame
				req_calc_sleep = true;
			}
		} else {
			// Fallback for not setting EMU:: .
			//emit sig_timer_start(10);
			//return;
			m_nr_fps = get_emu_frame_rate();
			emit sig_set_draw_fps(m_nr_fps);
			do_print_framerate(0);
			double __tmp_fps = (m_nr_fps > 1.0) ? m_nr_fps : 1.0;
			msleep((qint64)((1000.0 / __tmp_fps) * 8));
			continue;
		}

		m_interval = get_interval();
		if(m_interval <= 0) {
			m_interval = 0;
		}
		int64_t __tmp_interval = (m_interval <= 1000) ? 1000 : m_interval;
		int64_t __tmp_cur_usec = get_current_tick_usec();
		int64_t __tmp_max_usec = std::max(m_current_time, std::max(__tmp_cur_usec, m_next_time));
		__UNLIKELY_IF((__tmp_max_usec < 0) || (__tmp_max_usec >= (INT64_MAX - __tmp_interval))) {
			// Workaround for timer overflow. - 20260504 K.O
			m_tick_timer.restart();
			m_current_time = get_current_tick_usec();
			m_next_time = m_current_time;
			__tmp_cur_usec = m_current_time;
		}
		
		if(!(m_now_skip)) {
			m_next_time += m_interval;
			double __tmp_nr_fps = (m_nr_fps <= 1.0) ? 1.0 : m_nr_fps;
			if(m_next_time < (__tmp_cur_usec - (int64_t)((1.0e6 / __tmp_nr_fps) * 1.0))) { // Trim.
				// Reset current_time
				m_current_time = __tmp_cur_usec;
				m_next_time = m_current_time + m_interval;
			}
		}
		m_sleep_period = 0;
		if(req_calc_sleep) {
			if(m_next_time > (m_current_time + 500)) { // Upper 500uSec.
				m_sleep_period = m_next_time - m_current_time;
			}
			req_calc_sleep = false;
		}
#if 0
		info_log("EMU: POSITION=%s FRAMES:%d Wait: %d uSec INTERVAL=%d CURRENT=%d NEXT=%d", (m_half_count) ? _T("HALF") : _T("TOP ") , run_frames, m_sleep_period, m_interval, m_current_time, next_time);
#endif
		m_req_draw = false;
		//if(m_run_thread.load()) {
		//	emit sig_timer_start(sleep_period / 1000);
		//}
		if(m_sleep_period >= 500) {
			usleep(m_sleep_period);
			//msleep(sleep_period / 1000);
		} else {
			//next_time = current_time + interval;
			yieldCurrentThread();
		}
	} while(m_run_thread.load());
	
	check_power_off();
	emit sig_timer_stop();
	emit sig_sound_stop();
	emit sig_stop_sound_timer();
	info_log(CSP_LOG_TYPE_GENERAL, "EmuThread : EXIT");
	msleep(10);
	emit sig_draw_finished();
	
	quit();
}

const _TCHAR *EmuThreadClass::get_device_name(void)
{
	return (const _TCHAR *)_T(DEVICE_NAME);
}
