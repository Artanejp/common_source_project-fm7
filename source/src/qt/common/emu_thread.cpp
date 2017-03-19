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

#include "qt_gldraw.h"
#include "csp_logger.h"
#include "menu_flags.h"

// buttons
#ifdef MAX_BUTTONS
#define MAX_FONT_SIZE 32
#endif
#define MAX_SKIP_FRAMES 10

extern EMU *emu;
extern CSP_Logger *csp_logger;
EmuThreadClass::EmuThreadClass(META_MainWindow *rootWindow, USING_FLAGS *p, QObject *parent)
	: EmuThreadClassBase(rootWindow, p, parent)
{
	emu = new EMU(rMainWindow, rMainWindow->getGraphicsView(), using_flags);
	p_emu = emu;
	p->set_emu(emu);
	p->set_osd(emu->get_osd());
	emu->get_osd()->moveToThread(this);
}

EmuThreadClass::~EmuThreadClass()
{
}

int EmuThreadClass::get_interval(void)
{
	static int accum = 0;
	accum += p_emu->get_frame_interval();
	int interval = accum >> 10;
	accum -= interval << 10;
	return interval;
}

void EmuThreadClass::moved_mouse(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
#if defined(USE_MOUSE)   
	p_emu->set_mouse_pointer(x, y);
#endif   
}

void EmuThreadClass::button_pressed_mouse_sub(Qt::MouseButton button)
{
#if defined(USE_MOUSE)   
	int stat = p_emu->get_mouse_button();
	bool flag = p_emu->is_mouse_enabled();
	switch(button) {
	case Qt::LeftButton:
		stat |= 0x01;
		break;
	case Qt::RightButton:
		stat |= 0x02;
		break;
	case Qt::MiddleButton:
		flag = !flag;
		emit sig_mouse_enable(flag);
		break;
	default:
		break;
	}
	p_emu->set_mouse_button(stat);
#endif	   
}	

void EmuThreadClass::button_released_mouse_sub(Qt::MouseButton button)
{
#if defined(USE_MOUSE)   
		int stat = p_emu->get_mouse_button();
		switch(button) {
		case Qt::LeftButton:
			stat &= 0x7ffffffe;
			break;
		case Qt::RightButton:
			stat &= 0x7ffffffd;
			break;
		case Qt::MiddleButton:
			//emit sig_mouse_enable(false);
			break;
		default:
			break;
		}
		p_emu->set_mouse_button(stat);
#endif
}

void EmuThreadClass::get_fd_string(void)
{
#if defined(USE_FD1)
	int i;
	QString tmpstr;
	QString iname;
	QString alamp;
	uint32_t access_drv = 0;
	access_drv = p_emu->is_floppy_disk_accessed();
	{
		for(i = 0; i < (int)using_flags->get_max_drive(); i++) {
			if(p_emu->is_floppy_disk_inserted(i)) {
				if(i == (access_drv - 1)) {
					alamp = QString::fromUtf8("<FONT COLOR=RED>●</FONT> ");
				} else {
					alamp = QString::fromUtf8("○ ");
				}
				tmpstr = QString::fromUtf8("FD");
				tmpstr = alamp + tmpstr + QString::number(i) + QString::fromUtf8(":");
				if(emu->d88_file[i].bank_num > 0) {
					iname = QString::fromUtf8(emu->d88_file[i].disk_name[emu->d88_file[i].cur_bank]);
				} else {
					iname = QString::fromUtf8("*Inserted*");
				}
				tmpstr = tmpstr + iname;
			} else {
				tmpstr = QString::fromUtf8("× FD") + QString::number(i) + QString::fromUtf8(":");
				tmpstr = tmpstr + QString::fromUtf8(" ");
			}
			if(tmpstr != fd_text[i]) {
				emit sig_change_osd_fd(i, tmpstr);
				fd_text[i] = tmpstr;
			}
		}
	}
#endif
}

void EmuThreadClass::get_qd_string(void)
{
#if defined(USE_QD1)
	int i;
	QString iname;
	QString alamp;
	QString tmpstr;
	uint32_t access_drv = 0;
	access_drv = p_emu->is_quick_disk_accessed();
	for(i = 0; i < using_flags->get_max_qd(); i++) {
		if(p_emu->is_quick_disk_inserted(i)) {
			if(i == (access_drv - 1)) {
				alamp = QString::fromUtf8("● ");
			} else {
				alamp = QString::fromUtf8("○ ");
			}
			tmpstr = QString::fromUtf8("QD");
			tmpstr = alamp + tmpstr + QString::number(i) + QString::fromUtf8(":");
			iname = QString::fromUtf8("*Inserted*");
			tmpstr = tmpstr + iname;
		} else {
			tmpstr = QString::fromUtf8("× QD") + QString::number(i) + QString::fromUtf8(":");
			tmpstr = tmpstr + QString::fromUtf8(" ");
		}
		if(tmpstr != qd_text[i]) {
			emit sig_change_osd_qd(i, tmpstr);
			qd_text[i] = tmpstr;
		}
	}
#endif
}	

void EmuThreadClass::get_tape_string()
{
	QString tmpstr;
	
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
	if(p_emu->is_tape_inserted()) {
		const _TCHAR *ts = p_emu->get_tape_message();
		if(ts != NULL) {
			tmpstr = QString::fromUtf8(ts);
		}
	} else {
		tmpstr = QString::fromUtf8("EMPTY");
	}
	if(tmpstr != cmt_text) {
		emit sig_change_osd_cmt(tmpstr);
		cmt_text = tmpstr;
	}
#endif
}

void EmuThreadClass::get_cd_string(void)
{
#if defined(USE_COMPACT_DISC)
	QString tmpstr;
	if(p_emu->is_compact_disc_inserted()) {
		tmpstr = QString::fromUtf8("○");
	} else {
		tmpstr = QString::fromUtf8("Not Inserted");
	}
	if(tmpstr != cdrom_text) {
		emit sig_change_osd_cdrom(tmpstr);
		cdrom_text = tmpstr;
	}
#endif
}

void EmuThreadClass::get_bubble_string(void)
{
#if defined(USE_BUBBLE1)
	uint32_t access_drv;
	int i;
	QString alamp;
	QString tmpstr;
	for(i = 0; i < using_flags->get_max_bubble() ; i++) {
		if(p_emu->is_bubble_casette_inserted(i)) {
			alamp = QString::fromUtf8("● ");
			tmpstr = QString::fromUtf8("BUB");
			tmpstr = alamp + tmpstr + QString::number(i) + QString::fromUtf8(":");
		} else {
			tmpstr = QString::fromUtf8("× BUB") + QString::number(i) + QString::fromUtf8(":");
			tmpstr = tmpstr + QString::fromUtf8(" ");
		}
		if(tmpstr != bubble_text[i]) {
			emit sig_change_osd_bubble(i, tmpstr);
			bubble_text[i] = tmpstr;
		}
	}
#endif
}

void EmuThreadClass::doWork(const QString &params)
{
	int interval = 0, sleep_period = 0;
	int run_frames;
	bool now_skip;
	qint64 current_time;
	bool first = true;
	// LED
	uint32_t led_data = 0x00000000;
	uint32_t led_data_old = 0x00000000;
	// Tape
	// DIG_RESOLUTION
	//
	QString ctext;
	bool req_draw = true;
	bool vert_line_bak = using_flags->get_config_ptr()->opengl_scanline_vert;
	bool horiz_line_bak = using_flags->get_config_ptr()->opengl_scanline_horiz;
	bool gl_crt_filter_bak = using_flags->get_config_ptr()->use_opengl_filters;
	int opengl_filter_num_bak = using_flags->get_config_ptr()->opengl_filter_num;
	//uint32_t key_mod_old = 0xffffffff;
	int no_draw_count = 0;	
	bool prevRecordReq = false;

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
	record_fps = -1;

	next_time = 0;
	mouse_flag = false;

	key_mod = 0;
	//key_up_queue.clear();
	//key_down_queue.clear();
	clear_key_queue();

	for(int i = 0; i < using_flags->get_max_drive(); i++) qd_text[i].clear();
	for(int i = 0; i < using_flags->get_max_drive(); i++) fd_text[i].clear();
	cmt_text.clear();
	cdrom_text.clear();
	for(int i = 0; i < using_flags->get_max_bubble(); i++) bubble_text[i].clear();
	
	do {
		//p_emu->SetHostCpus(this->idealThreadCount());
   		if(MainWindow == NULL) {
			if(bRunThread == false){
				goto _exit;
			}
			msleep(10);
			continue;
		}
		if(first) {
			if(using_flags->get_use_led_device() > 0) emit sig_send_data_led((quint32)led_data);
			first = false;
		}
		interval = 0;
		sleep_period = 0;
		if(p_emu) {
			// drive machine
#ifdef USE_STATE
			if(bLoadStateReq != false) {
				p_emu->load_state();
				bLoadStateReq = false;
				req_draw = true;
			}
#endif			
			if(bResetReq != false) {
				p_emu->reset();
				bResetReq = false;
				req_draw = true;
			}
#ifdef USE_SPECIAL_RESET
			if(bSpecialResetReq != false) {
				p_emu->special_reset();
				bSpecialResetReq = false;
			}
#endif
#ifdef USE_STATE
			if(bSaveStateReq != false) {
				p_emu->save_state();
				bSaveStateReq = false;
			}
#endif
#if defined(USE_MINIMUM_RENDERING)
			if((vert_line_bak != p_config->opengl_scanline_vert) ||
			   (horiz_line_bak != p_config->opengl_scanline_horiz) ||
			   (gl_crt_filter_bak != p_config->use_opengl_filters) ||
			   (opengl_filter_num_bak != p_config->opengl_filter_num)) req_draw = true;
			vert_line_bak = p_config->opengl_scanline_vert;
			horiz_line_bak = p_config->opengl_scanline_horiz;
			gl_crt_filter_bak = p_config->use_opengl_filters;
			opengl_filter_num_bak = p_config->opengl_filter_num;
#endif
			if(bStartRecordSoundReq != false) {
				p_emu->start_record_sound();
				bStartRecordSoundReq = false;
				req_draw = true;
			}
			if(bStopRecordSoundReq != false) {
				p_emu->stop_record_sound();
				bStopRecordSoundReq = false;
				req_draw = true;
			}
			if(bUpdateConfigReq != false) {
				p_emu->update_config();
				bUpdateConfigReq = false;
				req_draw = true;
			}
			if(bStartRecordMovieReq != false) {
				if(!prevRecordReq && (record_fps > 0) && (record_fps < 75)) { 		
					p_emu->start_record_video(record_fps);
					prevRecordReq = true;
				}
			} else {
				if(prevRecordReq) {
					p_emu->stop_record_video();
					record_fps = -1;
					prevRecordReq = false;
				}
			}
		   
		   

#if defined(USE_MOUSE)	// Will fix
			emit sig_is_enable_mouse(p_emu->is_mouse_enabled());
#endif			
#if defined(USE_SOUND_VOLUME)
			for(int ii = 0; ii < USE_SOUND_VOLUME; ii++) {
				if(bUpdateVolumeReq[ii]) {
					p_emu->set_sound_device_volume(ii, p_config->sound_volume_l[ii], p_config->sound_volume_r[ii]);
					bUpdateVolumeReq[ii] = false;
				}
			}
#endif
			if(roma_kana_conv) {
				if(!roma_kana_queue.isEmpty()) {
#if defined(USE_AUTO_KEY)
					FIFO *dmy = emu->get_auto_key_buffer();
					if(dmy != NULL) {
						if(dmy->empty()) {
							QString tmps = roma_kana_queue.dequeue();
							this->do_start_auto_key(tmps);
						}
					}
#else
						roma_kana_queue.clear();
#endif
				}
				if(roma_kana_queue.isEmpty()) {
					roma_kana_conv = false;
				}
			}
			// else
			{
				while(!is_empty_key_up()) {
					key_queue_t sp;
					dequeue_key_up(&sp);
					key_mod = sp.mod;
					p_emu->key_modifiers(sp.mod);
					p_emu->key_up(sp.code);
				}
				while(!is_empty_key_down()) {
					key_queue_t sp;
					dequeue_key_down(&sp);
					p_emu->key_modifiers(sp.mod);
					p_emu->key_down(sp.code, sp.repeat);
				}
			}
			run_frames = p_emu->run();
			total_frames += run_frames;
#if defined(USE_MINIMUM_RENDERING)
			req_draw |= p_emu->is_screen_changed();
#else
			req_draw = true;
#endif			
#ifdef USE_LED_DEVICE
	   		led_data = p_emu->get_led_status();
			if(led_data != led_data_old) {
				emit sig_send_data_led((quint32)led_data);
				led_data_old = led_data;
			}
#endif
			sample_access_drv();

			interval += get_interval();
			now_skip = p_emu->is_frame_skippable() && !p_emu->is_video_recording();

			if((prev_skip && !now_skip) || next_time == 0) {
				next_time = tick_timer.elapsed();
			}
			if(!now_skip) {
				next_time += interval;
			}
			prev_skip = now_skip;
			//printf("p_emu::RUN Frames = %d SKIP=%d Interval = %d NextTime = %d\n", run_frames, now_skip, interval, next_time);
			if(next_time > tick_timer.elapsed()) {
				//  update window if enough time
//				draw_timing = false;
				if(!req_draw) {
					no_draw_count++;
					int count_limit = (int)(FRAMES_PER_SEC / 3);
#if defined(SUPPORT_TV_RENDER)
					if(config.rendering_type == CONFIG_RENDER_TYPE_TV) {
						count_limit = 0;
					}
#endif
					if(no_draw_count > count_limit) {
						req_draw = true;
						no_draw_count = 0;
					}
				} else {
					no_draw_count = 0;
					//emit sig_draw_thread(true);
				}
				emit sig_draw_thread(req_draw);
				skip_frames = 0;
			
				// sleep 1 frame priod if need
				current_time = tick_timer.elapsed();
				if((int)(next_time - current_time) >= 10) {
					sleep_period = next_time - current_time;
				}
			} else if(++skip_frames > MAX_SKIP_FRAMES) {
				// update window at least once per 10 frames
//				draw_timing = false;
				emit sig_draw_thread(true);
				no_draw_count = 0;
				skip_frames = 0;
				qint64 tt = tick_timer.elapsed();
				next_time = tt + get_interval();
				sleep_period = next_time - tt;
			}
		}
		req_draw = false;
		if(bRunThread == false){
			goto _exit;
		}
		if(sleep_period <= 0) sleep_period = 1;
		msleep(sleep_period);
	} while(1);
_exit:
	//emit quit_draw_thread();
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL,
						  "EmuThread : EXIT");
	emit sig_finished();
	this->quit();
}

void EmuThreadClass::do_set_display_size(int w, int h, int ww, int wh)
{
	p_emu->suspend();
	//p_emu->set_vm_screen_size(w, h, -1, -1, ww, wh);
	p_emu->set_host_window_size(w, h, true);
}

void EmuThreadClass::print_framerate(int frames)
{
	if(frames >= 0) draw_frames += frames;
	if(calc_message) {
		qint64 current_time = tick_timer.elapsed();
		if(update_fps_time <= current_time && update_fps_time != 0) {
			_TCHAR buf[256];
			QString message;
			int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);

				if(MainWindow->GetPowerState() == false){ 	 
					snprintf(buf, 255, _T("*Power OFF*"));
				} else {
					if(p_emu->message_count > 0) {
						snprintf(buf, 255, _T("%s - %s"), DEVICE_NAME, p_emu->message);
						p_emu->message_count--;
					} else {
						snprintf(buf, 255, _T("%s - %d fps (%d %%)"), DEVICE_NAME, draw_frames, ratio);
					}
				}
				if(romakana_conversion_mode) {
					message = QString::fromUtf8("[R]");
					message = message + QString::fromUtf8(buf);
				} else {
					message = buf;
				}
				emit message_changed(message);
				emit window_title_changed(message);
				update_fps_time += 1000;
				total_frames = draw_frames = 0;
				
			}
			if(update_fps_time <= current_time) {
				update_fps_time = current_time + 1000;
			}
			calc_message = false;  
		} else {
			calc_message = true;
		}
}
