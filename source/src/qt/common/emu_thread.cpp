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
	connect(p_osd, SIGNAL(sig_notify_power_off()), this, SLOT(do_notify_power_off()));
	connect(this, SIGNAL(sig_sound_stop()), p_osd, SLOT(stop_sound()));

	//call_timer = new QTimer(this);
	//call_timer->setTimerType(Qt::PreciseTimer);
	//call_timer->setSingleShot(true);
	
	//connect(call_timer, SIGNAL(timeout()), this, SLOT(doWork()));
	//connect(this, SIGNAL(sig_timer_start(int)), call_timer, SLOT(start(int)));
	//connect(this, SIGNAL(sig_timer_stop()), call_timer, SLOT(stop()));
	//connect(this, SIGNAL(finished()), call_timer, SLOT(stop()));
	interval = 0;
	sleep_period = 0;
	drawn_time = 0;
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
	check_scanline_params(true, vert_line_bak, horiz_line_bak, gl_crt_filter_bak, opengl_filter_num_bak, req_draw);
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
	do {
		if((MainWindow == NULL) || (bBlockTask.load()) || (is_up_null)) {
			if(bRunThread.load() == false){
				break;
			}
			//emit sig_timer_start(10);
			//return;
			do_print_framerate(0);
			msleep(10);
			continue;
		}
		if(queue_fixed_cpu >= 0) {
			do_set_emu_thread_to_fixed_cpu(queue_fixed_cpu);
			queue_fixed_cpu = -1;
		}
		if(first) {
			if(initialize_messages()) {
				if(!(is_up_null)) {
					if((u_p->get_use_led_devices() > 0) || (u_p->get_use_key_locked())) {
						emit sig_send_data_led((quint32)led_data_old);
					}
				}
				first = false;
			}
			drawn_time = get_current_tick_usec();
		}
		interval = 0;
		current_time = get_current_tick_usec();
		if(bRunThread.load() == false){
			break;
		}

		if(p_emu) {
			// drive machine
			if(!(half_count)) { // Start of frame.
				process_command_queue(req_draw);
				check_power_off();
				check_scanline_params(false, vert_line_bak, horiz_line_bak, gl_crt_filter_bak, opengl_filter_num_bak, req_draw);
			}
			
			process_key_input();
			run_frames = p_emu->run();
			total_frames += run_frames;
			half_count = !(half_count);
			// After frame, delayed open
			__LIKELY_IF(p_config != nullptr) {
//				__UNLIKELY_IF(full_speed != p_config->full_speed) {
//					next_time = 0;
//					fps_accum = 0;
					full_speed = p_config->full_speed;
//				}
			} else {
				full_speed = false;
			}
			if(bRunThread.load() == false){
				break;
			}
			
			if(!(half_count)) { // End of a frame.
				set_led(led_data_old, req_draw);
				sample_access_drv();
				now_skip = p_emu->is_frame_skippable() && !p_emu->is_video_recording();
				if((prev_skip && !now_skip) || (next_time == 0)) {
					next_time = get_current_tick_usec();
					//next_time = current_time;
					//fps_accum = 0;
				}
				prev_skip = now_skip;
			} else { // Half of a frame.
				prev_skip = now_skip;
			}
			if(!(full_speed)) {
				interval = get_interval();
			} else {
				interval = 0;
			}
#if 0
			next_time = current_time + interval;
#else
			if(!(now_skip) && !(full_speed) && (next_time > 0)) {
				next_time += interval;
			} /*else {
				next_time = current_time;
			}*/
#endif
			if(!(half_count)) { // End of a frame.
				if(!(is_up_null) && (p_config != nullptr)) {
					if((u_p->is_support_tv_render()) && (p_config->rendering_type == CONFIG_RENDER_TYPE_TV)) {
						req_draw = true;
					}
				}
				if(!req_draw) {
					if(next_time < get_current_tick_usec()) { // Even draw
						if(++skip_frames > MAX_SKIP_FRAMES) {
							req_draw = true;
							skip_frames = 0;
							//next_time = get_current_tick_usec();
						}
					}
				} else {
					skip_frames = 0;
				}
				if((full_speed) && (req_draw)) {
#if 0
					const int count_limit = (int)(FRAMES_PER_SEC / MAX_SKIP_FRAMES);
					no_draw_count++;
					if(no_draw_count >= count_limit) {
						req_draw = true;
						skip_frames = 0;
						no_draw_count = 0;
					} else {
						req_draw = false;
					}
#endif
				} else {
					no_draw_count = 0;
				}
				double nd;
				nd = p_emu->get_frame_rate();
				if(nr_fps != nd) emit sig_set_draw_fps(nd);
				nr_fps = nd;
				//printf("DRAW %dmsec\n", get_current_tick_usec());
				emit sig_draw_thread(req_draw); // Call offloading thread.
			}
		} else {
			// Fallback for not setting EMU:: .
			//emit sig_timer_start(10);
			//return;
			do_print_framerate(0);
			msleep(10);
			continue;
		}
		do_print_framerate(0);
		current_time = get_current_tick_usec();
		sleep_period = 0;
		if(next_time > current_time) {
			sleep_period = next_time - current_time;
		}
#if 0
		if(csp_logger.get() != nullptr) {
			csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_EMU,
								  "EMU: Wait: %d uSec CURRENT=%d NEXT=%d", sleep_period, current_time, next_time);
		}
#endif
		req_draw = false;
		//if(bRunThread.load()) {
		//	emit sig_timer_start(sleep_period / 1000);
		//}
		if(sleep_period >= 1000) {
			usleep(sleep_period);
		} else {
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
	emit sig_draw_finished();
	emit sig_sound_stop();
	
	quit();
}

const _TCHAR *EmuThreadClass::get_device_name(void)
{
	return (const _TCHAR *)_T(DEVICE_NAME);
}
