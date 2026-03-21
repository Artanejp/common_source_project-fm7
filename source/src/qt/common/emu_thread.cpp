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

	poweroff_notified = false;

	p_osd->setParent(this);
	//p_osd->moveToThread(this);
	connect(p_osd, SIGNAL(sig_notify_power_off()), this, SLOT(do_notify_power_off()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_restart_sound_timer()), p_osd, SLOT(do_restart_sound_timer()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_stop_sound_timer()), p_osd, SLOT(do_stop_sound_timer()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_sound_stop()), p_osd, SLOT(stop_sound()), Qt::QueuedConnection);

	//call_timer = new QTimer(this);
	//call_timer->setTimerType(Qt::PreciseTimer);
	//call_timer->setSingleShot(true);
	
	//connect(call_timer, SIGNAL(timeout()), this, SLOT(doWork()));
	//connect(this, SIGNAL(sig_timer_start(int)), call_timer, SLOT(start(int)));
	//connect(this, SIGNAL(sig_timer_stop()), call_timer, SLOT(stop()));
	//connect(this, SIGNAL(finished()), call_timer, SLOT(stop()));
	interval = 0;
	sleep_period = 0;
	run_frames = 0;
	current_time = 0;
	first = true;
	// LED
	led_data_old = 0x00000000;
	turn_count = 0;
	// Tape
	// DIG_RESOLUTION
	//
	req_draw = true;
	vert_line_bak = false;
	horiz_line_bak = false;
	gl_crt_filter_bak = false;
	opengl_filter_num_bak = 0;
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
	req_draw = true;
	no_draw_count = 0;
	//uint32_t key_mod_old = 0xffffffff;
	initialize_variables();
	tick_timer.start();
	fps_accum = 0;

	clear_key_queue();
	thread_id = currentThreadId();
	
	emit sig_emu_launched();
	bBlockTask = false;
//	current_time = get_current_tick_usec();
//	emit sig_timer_start(10);

}

void EmuThreadClass::doWork()
{
	if(!(bRunThread.load()) && !(first)) {
		return;
	}
	std::shared_ptr<CSP_Logger> csp_logger = p_osd->get_logger();
	std::shared_ptr<USING_FLAGS> u_p = using_flags;

	bool is_up_null = (u_p.get() == nullptr);
	thread_id = currentThreadId();
	
	is_up_null = (u_p.get() == nullptr);
	QElapsedTimer led_timer;
	bool req_calc_sleep = false;
	next_time = 0;

	do {
		if((MainWindow == NULL) || (bBlockTask.load()) || (is_up_null)) {
			if(bRunThread.load() == false){
				break;
			}
			//emit sig_timer_start(10);
			//return;
			//do_print_framerate(0);
			msleep(10);
			next_time = 0;
			continue;
		}
		if(!(queue_cpu_affinities.empty())) {
			do_apply_cpu_affinities_to_emu_thread();
		}
		if(first) {
			if(initialize_messages()) {
				if(!(is_up_null)) {
					if((u_p->get_use_led_devices() > 0) || (u_p->get_use_key_locked())) {
						emit sig_force_redraw_leds();
					}
				}
				first = false;
				nr_fps = get_emu_frame_rate();
				emit sig_restart_sound_timer();
				current_time = get_current_tick_usec();
				half_count = false;
				driven_by_half_of_frame = false;
			}
			led_timer.start();
			next_time = 0;
		}
		interval = 0;
		if(bRunThread.load() == false){
			break;
		}

		__LIKELY_IF(p_emu != NULL) {
			// drive machine
			__LIKELY_IF(!(half_count) || !(driven_by_half_of_frame)) { // Start of frame.
				now_skip = (((full_speed) || p_emu->is_frame_skippable()) && !(p_emu->is_video_recording())) ? true : false;
				process_command_queue();
				check_power_off();
				check_scanline_params(false);
			}
			
			current_time = get_current_tick_usec();
			process_key_input();

			run_frames = p_emu->run();
			half_count = p_emu->is_half_event();
			driven_by_half_of_frame = p_emu->is_driven_by_half_of_frame();
			#if 1
			if((driven_by_half_of_frame) && !(half_count)) {
				total_frames++;
			} else if(!(driven_by_half_of_frame)) {
				total_frames++;
			}
			#endif
			if(run_frames > 0) {
				total_frames += run_frames;
			}
			

			// After frame, delayed open
			if(bRunThread.load() == false){
				break;
			}
			set_led();
			
			__LIKELY_IF(!(half_count) || !(driven_by_half_of_frame)) { // End of a frame.
//				set_led();
				sample_access_drv();
				__LIKELY_IF(p_config != nullptr) {
					full_speed = p_config->full_speed;
				}
				now_skip = (((full_speed) || p_emu->is_frame_skippable()) && !(p_emu->is_video_recording())) ? true : false;
				__UNLIKELY_IF(((prev_skip) && !(now_skip)) || (next_time <= 0)) {
					next_time = get_current_tick_usec();
				}				
				double nd;
				nd = get_emu_frame_rate();
				if(nr_fps != nd) emit sig_set_draw_fps(nd);
				nr_fps = nd;
				prev_skip = now_skip;
			}
			//if(!(now_skip)) {
			//	interval = get_interval();
			//	next_time += interval;
			//}

			sleep_period = 0;
			//current_time = get_current_tick_usec();
			__LIKELY_IF(!(half_count) || !(driven_by_half_of_frame)) { // End of a frame.
				if(!(is_up_null) && (p_config != nullptr)) {
					if((u_p->is_support_tv_render()) && (p_config->rendering_type == CONFIG_RENDER_TYPE_TV)) {
						req_draw = true;
					}
				}
				if(!(req_draw)) {
					if(next_time <= current_time) { // Even draw
						if(++skip_frames > MAX_SKIP_FRAMES) {
							req_draw = true;
							skip_frames = 0;
							next_time = get_current_tick_usec();
						}
					} else {
						req_calc_sleep = true;
					}
				} else {
					skip_frames = 0;
					req_calc_sleep = true;
				}
				//printf("DRAW %dmsec\n", get_current_tick_usec());
				do_print_framerate(0);
				if(req_draw) {
					p_emu->request_update_screen();
				}
				#if 1
				if(p_osd != nullptr) {
					p_osd->do_draw(req_draw); // Call OSD , then off;pading by OSD to DRAW_THREAD.
				}
				#else
				//	emit sig_draw_thread(req_draw); // Call offloading thread.
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
				req_draw = false;
			} else {
				// Middle of frame
				req_calc_sleep = true;
			}
		} else {
			// Fallback for not setting EMU:: .
			//emit sig_timer_start(10);
			//return;
			nr_fps = get_emu_frame_rate();
			emit sig_set_draw_fps(nr_fps);
			do_print_framerate(0);
			msleep((qint64)((1000.0 / nr_fps) * 8));
			continue;
		}
		//current_time = get_current_tick_usec();
		if(!(now_skip)) {
			interval = get_interval();
			next_time += interval;
		}
		if(req_calc_sleep) {
			if(next_time > (current_time + 1000)) { // Upper 1mSec.
				sleep_period = next_time - current_time;
			}
			req_calc_sleep = false;
		}
#if 0
		if(csp_logger.get() != nullptr) {
			csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_EMU,
								  "EMU: POSITION=%s FRAMES:%d Wait: %d uSec INTERVAL=%d CURRENT=%d NEXT=%d", (half_count) ? _T("HALF") : _T("TOP ") , run_frames, sleep_period, interval, current_time, next_time);
		}
#endif
		req_draw = false;
		//if(bRunThread.load()) {
		//	emit sig_timer_start(sleep_period / 1000);
		//}
		if(sleep_period >= 500) {
			usleep(sleep_period);
			//msleep(sleep_period / 1000);
		} else {
			//next_time = current_time + interval;
			yieldCurrentThread();
		}
	} while(bRunThread.load());
	
	check_power_off();
	emit sig_timer_stop();
	if(p_osd != nullptr) {
		std::shared_ptr<CSP_Logger> csp_logger = p_osd->get_logger();
		if(csp_logger.get() != NULL) {
			csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL,
								  "EmuThread : EXIT");
		}
	}
	msleep(10);
	emit sig_stop_sound_timer();
	emit sig_sound_stop();
	emit sig_draw_finished();
	
	quit();
}

const _TCHAR *EmuThreadClass::get_device_name(void)
{
	return (const _TCHAR *)_T(DEVICE_NAME);
}
