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

	connect(this, SIGNAL(sig_emu_launched()), rootWindow->getGraphicsView(), SLOT(set_emu_launched()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_open_binary_load(int, QString)), MainWindow, SLOT(_open_binary_load(int, QString)), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_open_binary_save(int, QString)), MainWindow, SLOT(_open_binary_save(int, QString)), Qt::QueuedConnection);
//	connect(this, SIGNAL(sig_open_cart(int, QString)), MainWindow, SLOT((int, QString)));

//	connect(this, SIGNAL(sig_open_bubble(int, QString)), MainWindow, SLOT(_open_bubble(int, QString)));
//	connect(this, SIGNAL(sig_open_b77_bubble(int, QString, int)), this, SLOT(do_open_bubble_casette(int, QString, int)));

	p_osd->setParent(this);
	//p_osd->moveToThread(this);
	connect(p_osd, SIGNAL(sig_notify_power_off()), this, SLOT(do_notify_power_off()));
	connect(this, SIGNAL(sig_sound_stop()), p_osd, SLOT(stop_sound()));
	connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

EmuThreadClass::~EmuThreadClass()
{
}

#include <QStringList>
#include <QFileInfo>



void EmuThreadClass::doWork(const QString &params)
{
	int interval = 0;
	int64_t sleep_period = 0;
	int run_frames;
	qint64 current_time;
	bool first = true;
	// LED
	uint32_t led_data = 0x00000000;
	uint32_t led_data_old = 0x00000000;
	int turn_count = 0;
	// Tape
	// DIG_RESOLUTION
	//
	QString ctext;
	bool req_draw = true;
	bool vert_line_bak = config.opengl_scanline_vert;
	bool horiz_line_bak = config.opengl_scanline_horiz;
	bool gl_crt_filter_bak = config.use_opengl_filters;
	int opengl_filter_num_bak = config.opengl_filter_num;
	//uint32_t key_mod_old = 0xffffffff;
	int no_draw_count = 0;
	bool prevRecordReq = false;
	double nr_fps = -1.0;
	int _queue_begin;
	bool multithread_draw = config.use_separate_thread_draw;

	bool state_power_off = false;

	doing_debug_command = false;
	ctext.clear();
//	draw_timing = false;
	bResetReq = false;
	bSpecialResetReq = false;
	bLoadStateReq = false;
	bSaveStateReq = false;
	bUpdateConfigReq = false;
	bStartRecordSoundReq = false;
	bStopRecordSoundReq = false;
	bStartRecordMovieReq = false;
	specialResetNum = 0;
	sStateFile.clear();
	lStateFile.clear();
	thread_id = this->currentThreadId();
	record_fps = -1;
	tick_timer.start();
	update_fps_time = tick_timer.elapsed();
	//update_fps_time = SDL_GetTicks();
	next_time = 0;
	mouse_flag = false;

	key_mod = 0;
	//key_up_queue.clear();
	//key_down_queue.clear();
	clear_key_queue();
	bool half_count = false;
	std::shared_ptr<CSP_Logger> csp_logger = p_osd->get_logger();
	std::shared_ptr<USING_FLAGS> u_p = using_flags;

	bool is_up_null = (u_p.get() == nullptr);
	if(is_up_null) {
		for(int i = 0; i < u_p->get_max_qd(); i++) qd_text[i].clear();
		for(int i = 0; i < u_p->get_max_drive(); i++) {
			fd_text[i].clear();
			fd_lamp[i] = QString::fromUtf8("×");
		}
		for(int i = 0; i < u_p->get_max_tape(); i++) {
			cmt_text[i].clear();
		}
		for(int i = 0; i < (int)u_p->get_max_cd(); i++) {
			cdrom_text[i] = QString::fromUtf8("×");
		}
		for(int i = 0; i < (int)u_p->get_max_ld(); i++) {
			laserdisc_text[i].clear();
		}
		for(int i = 0; i < (int)u_p->get_max_hdd(); i++) {
			hdd_text[i].clear();
			hdd_lamp[i].clear();
		}
		for(int i = 0; i < u_p->get_max_bubble(); i++) bubble_text[i].clear();
	}

//	_queue_begin = parse_command_queue(virtualMediaList);
//	virtualMediaList.clear();
	//SDL_SetHint(SDL_HINT_TIMER_RESOLUTION, "2");
	emit sig_emu_launched();
	do {
		//p_emu->SetHostCpus(this->idealThreadCount());
		// Chack whether using_flags exists.
		if(is_up_null) {
			u_p = using_flags;
		}
		is_up_null = (u_p.get() == nullptr);

   		if((MainWindow == NULL) || (bBlockTask.load())) {
			if(bRunThread.load() == false){
				goto _exit;
			}
			msleep(10);
			//SDL_Delay(10);
			continue;
		}
		if(queue_fixed_cpu >= 0) {
			do_set_emu_thread_to_fixed_cpu(queue_fixed_cpu);
			queue_fixed_cpu = -1;
		}
		if(first) {
			if(!(is_up_null)) {

			if((u_p->get_use_led_devices() > 0) || (u_p->get_use_key_locked())) emit sig_send_data_led((quint32)led_data);
			for(int ii = 0; ii < u_p->get_max_drive(); ii++ ) {
				emit sig_change_access_lamp(CSP_DockDisks_Domain_FD, ii, fd_lamp[ii]);
			}
			for(int ii = 0; ii < u_p->get_max_cd(); ii++ ) {
				emit sig_change_access_lamp(CSP_DockDisks_Domain_CD, ii, cdrom_text[ii]);
			}
			for(int ii = 0; ii < u_p->get_max_hdd(); ii++ ) {
				emit sig_change_access_lamp(CSP_DockDisks_Domain_HD, ii, hdd_text[ii]);
			}

			first = false;
			}
		}
		interval = 0;
		sleep_period = 0;
		if(p_emu) {
			// drive machine
			if(!(half_count)) { // OK?
				_queue_begin = parse_command_queue(virtualMediaList);
				virtualMediaList.clear();
			}
			if(bLoadStateReq.load() != false) {
				loadState();
				bLoadStateReq = false;
				req_draw = true;
			}

			if(bResetReq.load() != false) {
				half_count = false;
				resetEmu();
				bResetReq = false;
				req_draw = true;
			}
			if(bSpecialResetReq.load() != false) {
				half_count = false;
				specialResetEmu(specialResetNum);
				bSpecialResetReq = false;
				specialResetNum = 0;
			}
			if(bSaveStateReq.load() != false) {
				saveState();
				bSaveStateReq = false;
			}
			if(!(is_up_null)) {
			if(u_p->is_use_minimum_rendering()) {
				if((vert_line_bak != config.opengl_scanline_vert) ||
				   (horiz_line_bak != config.opengl_scanline_horiz) ||
				   (gl_crt_filter_bak != config.use_opengl_filters) ||
				   (opengl_filter_num_bak != config.opengl_filter_num)) req_draw = true;
				vert_line_bak = config.opengl_scanline_vert;
				horiz_line_bak = config.opengl_scanline_horiz;
				gl_crt_filter_bak = config.use_opengl_filters;
				opengl_filter_num_bak = config.opengl_filter_num;
			}
			}
			if(bStartRecordSoundReq.load() != false) {
				p_emu->start_record_sound();
				bStartRecordSoundReq = false;
				req_draw = true;
			}
			if(bStopRecordSoundReq.load() != false) {
				p_emu->stop_record_sound();
				bStopRecordSoundReq = false;
				req_draw = true;
			}
			if(bUpdateConfigReq != false) {
				p_emu->update_config();
				bUpdateConfigReq = false;
				req_draw = true;
			}
			if(bStartRecordMovieReq.load() != false) {
				int rfps = record_fps.load();
				if(!prevRecordReq && (rfps > 0) && (rfps < 75)) {
					p_emu->start_record_video(rfps);
					prevRecordReq = true;
				}
			} else {
				if(prevRecordReq) {
					p_emu->stop_record_video();
					record_fps = -1;
					prevRecordReq = false;
				}
			}
#if defined(USE_SOUND_VOLUME)
			for(int ii = 0; ii < USE_SOUND_VOLUME; ii++) {
				if(bUpdateVolumeReq[ii].load()) {
					p_emu->set_sound_device_volume(ii, config.sound_volume_l[ii], config.sound_volume_r[ii]);
					bUpdateVolumeReq[ii] = false;
				}
			}
#endif
			if(p_osd != nullptr) {
				while(!is_empty_key()) {
					key_queue_t sp;
					dequeue_key(&sp);
					//printf("%08x %04x %08x %d\n", sp.type, sp.code, sp.mod, sp.repeat);
					switch(sp.type) {
					case KEY_QUEUE_UP:
						key_mod = sp.mod;
						p_osd->key_modifiers(sp.mod);
						p_emu->key_up(sp.code, true); // need decicion of extend.
						break;
					case KEY_QUEUE_DOWN:
						if(config.romaji_to_kana) {
							p_osd->key_modifiers(sp.mod);
							p_emu->key_char(sp.code);
						} else {
							p_osd->key_modifiers(sp.mod);
							p_emu->key_down(sp.code, true, sp.repeat);
						}
						break;
					default:
						break;
					}
				}
			}

			if(!(half_count)  && (multithread_draw)) {
				if(nr_fps < 0.0) {
					nr_fps = get_emu_frame_rate();
					if(nr_fps >= 1.0) emit sig_set_draw_fps(nr_fps);
				}
			}
			#ifdef USE_NOTIFY_POWER_OFF
			if((poweroff_notified) && !(state_power_off))  {
				p_emu->notify_power_off();
				state_power_off = true;
			}
			#endif
			run_frames = p_emu->run();
			total_frames += run_frames;
			// After frame, delayed open
			if(!(half_count)) {
				led_data = 0x00;
				bool _ind_caps_kana = false;
				bool _key_lock = false;
				int _led_shift = 0;
				if(!(is_up_null)) {
					_ind_caps_kana = u_p->get_independent_caps_kana_led();
					_key_lock = u_p->get_use_key_locked();
					_led_shift = u_p->get_use_led_devices();
					if(u_p->is_use_minimum_rendering()) {
						req_draw |= p_emu->is_screen_changed();
					} else {
						req_draw = true;
					}
					if((_key_lock) && (_ind_caps_kana)) {
						led_data |= ((p_emu->get_caps_locked()) ? 0x01 : 0x00);
						led_data |= ((p_emu->get_kana_locked()) ? 0x02 : 0x00);
						if(_led_shift > 0) {
							led_data <<= _led_shift;
						}
					}
				} else {
					req_draw = true;
				}
				led_data |= p_emu->get_led_status();

				if((_led_shift > 0) || (_key_lock)) {
					if(led_data != led_data_old) {
						emit sig_send_data_led((quint32)led_data);
						led_data_old = led_data;
					}
				}
				sample_access_drv();
				now_skip = p_emu->is_frame_skippable() && !p_emu->is_video_recording();
				if(config.full_speed) {
					interval = 1;
				} else {
					interval = get_interval();
				}
				if((prev_skip && !now_skip) || next_time == 0) {
					next_time = tick_timer.elapsed();
					//next_time = SDL_GetTicks();
				}
				if(!now_skip) {
					next_time += interval;
				}
//				prev_skip = now_skip;
			} else { // (half_count)
				if(config.full_speed) {
					interval = 1;
				} else {
					interval = get_interval();
				}
//				if((prev_skip && !now_skip) || next_time == 0) {
//					next_time = tick_timer.elapsed();
//					//next_time = SDL_GetTicks();
//				}
				if(!now_skip) {
					next_time += interval;
				}
				prev_skip = now_skip;
			}
			if(next_time > tick_timer.elapsed()) {
				//if(next_time > SDL_GetTicks()) {
				//  update window if enough time
				if(!req_draw) {
					no_draw_count++;
					int count_limit = (int)(FRAMES_PER_SEC / 3);
					if(!(is_up_null)) {
						if((u_p->is_support_tv_render()) && (config.rendering_type == CONFIG_RENDER_TYPE_TV)) {
							count_limit = 0;
						}
					}
					if(no_draw_count > count_limit) {
						req_draw = true;
						no_draw_count = 0;
					}
				} else {
					no_draw_count = 0;
					//emit sig_draw_thread(true);
				}
				if(!(half_count))
				{
					double nd;
					nd = p_emu->get_frame_rate();
					if(nr_fps != nd) emit sig_set_draw_fps(nd);
					nr_fps = nd;
					//printf("DRAW %dmsec\n", tick_timer.elapsed());
					if(multithread_draw) {
						emit sig_draw_thread(req_draw);
					} else {
						emit sig_draw_thread(req_draw);
						emit sig_draw_one_turn(true);
					}
					skip_frames = 0;
				}
				// sleep 1 frame priod if need
				current_time = tick_timer.elapsed();
				//current_time = SDL_GetTicks();
				sleep_period = 0;
				if(next_time > current_time) {
					if((int)(next_time - current_time) >= 1) {
						sleep_period = next_time - current_time;
					}
				}
			} else if(++skip_frames > MAX_SKIP_FRAMES) {
				// update window at least once per 10 frames
				if(!(half_count))
				{
					double nd;
					nd = p_emu->get_frame_rate();
					if(nr_fps != nd) emit sig_set_draw_fps(nd);
					nr_fps = nd;
				}
				if(!(half_count)) {
					//printf("DRAW(SKIP) %dmsec\n", tick_timer.elapsed());
					if(multithread_draw) {
						emit sig_draw_thread(req_draw);
					} else {
						emit sig_draw_thread(req_draw);
						emit sig_draw_one_turn(true);
					}
				}
				no_draw_count = 0;
				skip_frames = 0;
				qint64 tt = tick_timer.elapsed();
				//quint32 tt = SDL_GetTicks();
				if(next_time > tt) {
					sleep_period = next_time - tt;
				} else {
					sleep_period = 0;
				}
				if(config.full_speed) {
					next_time = tt + 1;
				} else {
					next_time = tt;// + get_interval();
					next_time = next_time + get_interval();
				}
				sleep_period = 0;
			}
		}
		req_draw = false;
		if(bRunThread.load() == false){
			goto _exit;
		}
		if(sleep_period > 0) {
			//sleep_period = 1;
			msleep(sleep_period);
			//SDL_Delay(sleep_period);
		}
//		printf("HALF=%s %dmsec\n", (half_count) ? "YES" : "NO ", tick_timer.elapsed());
		half_count = !(half_count);
//		if(!(half_count)) { // OK?
//			_queue_begin = parse_command_queue(virtualMediaList);
//			virtualMediaList.clear();
//		}
		//SDL_Delay(sleep_period);
	} while(1);
_exit:
	//emit quit_draw_thread();
	#ifdef USE_NOTIFY_POWER_OFF
	if((poweroff_notified) && !(state_power_off) && (p_emu != nullptr))  {
		p_emu->notify_power_off();
		state_power_off = true;
	}
	#endif

	if(csp_logger.get() != NULL) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL,
							  "EmuThread : EXIT");
	}
	emit sig_draw_finished();
	emit sig_sound_stop();
	this->quit();
}

const _TCHAR *EmuThreadClass::get_device_name(void)
{
	return (const _TCHAR *)_T(DEVICE_NAME);
}
