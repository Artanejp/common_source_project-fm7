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

// buttons
#ifdef MAX_BUTTONS
#define MAX_FONT_SIZE 32
#endif
#define MAX_SKIP_FRAMES 10

extern EMU *emu;
extern CSP_Logger *csp_logger;
EmuThreadClass::EmuThreadClass(Ui_MainWindowBase *rootWindow, USING_FLAGS *p, QObject *parent)
	: EmuThreadClassBase(rootWindow, p, parent)
{
	emu = new EMU(rMainWindow, rMainWindow->getGraphicsView(), using_flags);
	p_emu = emu;
	p->set_emu(emu);
	p->set_osd(emu->get_osd());
	emu->get_osd()->moveToThread(this);

	connect(this, SIGNAL(sig_open_binary_load(int, QString)), MainWindow, SLOT(_open_binary_load(int, QString)));
	connect(this, SIGNAL(sig_open_binary_save(int, QString)), MainWindow, SLOT(_open_binary_save(int, QString)));
	connect(this, SIGNAL(sig_open_cart(int, QString)), MainWindow, SLOT(_open_cart(int, QString)));
	connect(this, SIGNAL(sig_open_cmt_load(int, QString)), MainWindow, SLOT(do_open_read_cmt(int, QString)));
	connect(this, SIGNAL(sig_open_cmt_write(int, QString)), MainWindow, SLOT(do_open_write_cmt(int, QString)));
	connect(this, SIGNAL(sig_open_fd(int, QString)), MainWindow, SLOT(_open_disk(int, QString)));
	
	connect(this, SIGNAL(sig_open_quick_disk(int, QString)), MainWindow, SLOT(_open_quick_disk(int, QString)));
	connect(this, SIGNAL(sig_open_bubble(int, QString)), MainWindow, SLOT(_open_bubble(int, QString)));
	connect(this, SIGNAL(sig_open_cdrom(int, QString)), MainWindow, SLOT(do_open_cdrom(int, QString)));
	connect(this, SIGNAL(sig_open_laser_disc(int, QString)), MainWindow, SLOT(do_open_laserdisc(int, QString)));
	
	connect(this, SIGNAL(sig_open_hdd(int, QString)), MainWindow, SLOT(_open_hard_disk(int, QString)));
	
	connect(this, SIGNAL(sig_set_d88_num(int, int)), MainWindow, SLOT(set_d88_slot(int, int)));
	connect(this, SIGNAL(sig_set_b77_num(int, int)), MainWindow, SLOT(set_b77_slot(int, int)));

}

EmuThreadClass::~EmuThreadClass()
{
}

void EmuThreadClass::set_romakana(bool flag)
{
#if defined(USE_AUTO_KEY)
	p_emu->set_auto_key_char(flag ? 1 : 0);
#endif
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
	//printf("Moved Mouse %d, %d\n", x, y);
#if defined(USE_MOUSE)
	bool flag = p_emu->get_osd()->is_mouse_enabled();
	if(!flag) return;
	//printf("Mouse Moved: %d, %d\n", x, y);
	p_emu->get_osd()->set_mouse_pointer(x, y);
#endif   
}

void EmuThreadClass::button_pressed_mouse_sub(Qt::MouseButton button)
{
#if defined(USE_MOUSE)   
	int stat = p_emu->get_osd()->get_mouse_button();
	bool flag = p_emu->get_osd()->is_mouse_enabled();
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
		return;
		break;
	default:
		break;
	}
	if(!flag) return;
	p_emu->get_osd()->set_mouse_button(stat);
#endif	   
}	

void EmuThreadClass::button_released_mouse_sub(Qt::MouseButton button)
{
#if defined(USE_MOUSE)   
	int stat = p_emu->get_osd()->get_mouse_button();
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
		p_emu->get_osd()->set_mouse_button(stat);
#endif
}

void EmuThreadClass::get_fd_string(void)
{
#if defined(USE_FLOPPY_DISK)
	int i;
	QString tmpstr;
	QString iname;
	QString alamp;
	uint32_t access_drv = 0;
	bool lamp_stat = false;
	access_drv = p_emu->is_floppy_disk_accessed();
	{
		for(i = 0; i < (int)using_flags->get_max_drive(); i++) {
			tmpstr.clear();
			alamp.clear();
			lamp_stat = false;
			if(p_emu->is_floppy_disk_inserted(i)) {
				if(i == (access_drv - 1)) {
					lamp_stat = true;
					alamp = QString::fromUtf8("<FONT COLOR=FIREBRICK>●</FONT>"); // 💾U+1F4BE Floppy Disk
				} else {
					alamp = QString::fromUtf8("<FONT COLOR=BLUE>○</FONT>"); // 💾U+1F4BE Floppy Disk
				}
				if(emu->d88_file[i].bank_num > 0) {
					iname = QString::fromUtf8(emu->d88_file[i].disk_name[emu->d88_file[i].cur_bank]);
				} else {
					iname = QString::fromUtf8("*Inserted*");
				}
				tmpstr = iname;
			} else {
				tmpstr.clear();
				alamp = QString::fromUtf8("×");
			}
			if(alamp != fd_lamp[i]) {
				emit sig_set_access_lamp(i + 2, lamp_stat);
				emit sig_change_access_lamp(CSP_DockDisks_Domain_FD, i, alamp);
				fd_lamp[i] = alamp;
			}
			if(tmpstr != fd_text[i]) {
				emit sig_set_access_lamp(i + 2, lamp_stat);
				emit sig_change_osd(CSP_DockDisks_Domain_FD, i, tmpstr);
				fd_text[i] = tmpstr;
			}
			lamp_stat = false;
		}
	}
#endif
}

void EmuThreadClass::get_qd_string(void)
{
#if defined(USE_QUICK_DISK)
	int i;
	QString iname;
	QString alamp;
	QString tmpstr;
	uint32_t access_drv = 0;
	bool lamp_stat = false;
	access_drv = p_emu->is_quick_disk_accessed();
	for(i = 0; i < using_flags->get_max_qd(); i++) {
		tmpstr.clear();
		lamp_stat = false;
		if(p_emu->is_quick_disk_inserted(i)) {
			if(i == (access_drv - 1)) {
				alamp = QString::fromUtf8("<FONT COLOR=MAGENTA>●</FONT>"); // 💽　U+1F4BD MiniDisc
				lamp_stat = true;
			} else {
				alamp = QString::fromUtf8("<FONT COLOR=BLUE>○</FONT>"); // 💽U+1F4BD MiniDisc
			}
			tmpstr = alamp;
			//tmpstr = tmpstr + QString::fromUtf8(" ");
			//iname = QString::fromUtf8("*Inserted*");
			//tmpstr = tmpstr + iname;
		} else {
			tmpstr = QString::fromUtf8("×");
		}
		if(tmpstr != qd_text[i]) {
			emit sig_set_access_lamp(i + 10, lamp_stat);
			emit sig_change_access_lamp(CSP_DockDisks_Domain_QD, i, tmpstr);
			qd_text[i] = tmpstr;
		}
		lamp_stat = false;
	}
#endif
}	

void EmuThreadClass::get_tape_string()
{
	QString tmpstr;
	bool readwrite;
	bool inserted;
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
	for(int i = 0; i < USE_TAPE; i++) {
		inserted = p_emu->is_tape_inserted(i);
		readwrite = false;
		if(inserted) {
			tmpstr.clear();
			const _TCHAR *ts = p_emu->get_tape_message(i);
			if(ts != NULL) {
				tmpstr = QString::fromUtf8(ts);
				readwrite = p_emu->is_tape_recording(i);
				if(readwrite) {
					tmpstr = QString::fromUtf8("<FONT COLOR=RED><B>") + tmpstr + QString::fromUtf8("</B></FONT>");
				} else {
					tmpstr = QString::fromUtf8("<FONT COLOR=GREEN><B>") + tmpstr + QString::fromUtf8("</B></FONT>");
				}					
			}
		} else {
			tmpstr = QString::fromUtf8("<FONT COLOR=BLUE>   EMPTY   </FONT>");
		}
		if(tmpstr != cmt_text[i]) {
			//emit sig_set_access_lamp(i + 12 + ((readwrite) ? 2 : 0), inserted);
			emit sig_change_osd(CSP_DockDisks_Domain_CMT, i, tmpstr);
			cmt_text[i] = tmpstr;
		}
	}
#endif
}

void EmuThreadClass::get_hdd_string(void)
{
#if defined(USE_HARD_DISK)
	QString tmpstr, alamp;
	uint32_t access_drv = p_emu->is_hard_disk_accessed();
	bool lamp_stat = false;
	alamp.clear();
	tmpstr.clear();
	for(int i = 0; i < (int)using_flags->get_max_hdd(); i++) {
		if(p_emu->is_hard_disk_inserted(i)) {
			if((access_drv & (1 << i)) != 0) {
				alamp = QString::fromUtf8("<FONT COLOR=LIME>■</FONT>");  // 
				lamp_stat = true;
			} else {
				alamp = QString::fromUtf8("<FONT COLOR=BLUE>□</FONT>");  // 
			}
		} else {
			alamp = QString::fromUtf8("×");
		}
		if(tmpstr != hdd_text[i]) {
			emit sig_set_access_lamp(i + 16, lamp_stat);
			emit sig_change_osd(CSP_DockDisks_Domain_HD, i, tmpstr);
			hdd_text[i] = tmpstr;
		}
		if(alamp != hdd_lamp[i]) {
			emit sig_set_access_lamp(i + 16, lamp_stat); 
			emit sig_change_access_lamp(CSP_DockDisks_Domain_HD, i, alamp);
			hdd_lamp[i] = alamp;
		}
		lamp_stat = false;
	}
#endif
}
void EmuThreadClass::get_cd_string(void)
{
#if defined(USE_COMPACT_DISC)
	QString tmpstr;
	uint32_t access_drv = p_emu->is_compact_disc_accessed();
	for(int i = 0; i < (int)using_flags->get_max_cd(); i++) {
		if(p_emu->is_compact_disc_inserted(i)) {
			if((access_drv & (1 << i)) != 0) {
				tmpstr = QString::fromUtf8("<FONT COLOR=DEEPSKYBLUE>●</FONT>");  // U+1F4BF OPTICAL DISC
			} else {
				tmpstr = QString::fromUtf8("<FONT COLOR=BLUE>○</FONT>");  // U+1F4BF OPTICAL DISC
			}				
		} else {
			tmpstr = QString::fromUtf8("×");
		}
		if(tmpstr != cdrom_text[i]) {
			emit sig_change_access_lamp(CSP_DockDisks_Domain_CD, i, tmpstr);
			cdrom_text[i] = tmpstr;
		}
	}
#endif
}

void EmuThreadClass::get_bubble_string(void)
{
#if defined(USE_BUBBLE)
	uint32_t access_drv;
	int i;
	QString alamp;
	QString tmpstr;
	for(i = 0; i < using_flags->get_max_bubble() ; i++) {
		if(p_emu->is_bubble_casette_inserted(i)) {
			alamp = QString::fromUtf8("● ");
			//tmpstr = alamp + QString::fromUtf8(" ");
		} else {
			tmpstr = QString::fromUtf8("×");
			//tmpstr = tmpstr + QString::fromUtf8(" ");
		}
		if(alamp != bubble_text[i]) {
			emit sig_change_access_lamp(CSP_DockDisks_Domain_Bubble, i, tmpstr);
			bubble_text[i] = alamp;
		}
	}
#endif
}

#include <QStringList>
#include <QFileInfo>

extern QStringList virtualMediaList; // {TYPE, POSITION}

void EmuThreadClass::resetEmu()
{
	clear_key_queue();
	p_emu->reset();
}

void EmuThreadClass::specialResetEmu()
{
#ifdef USE_SPECIAL_RESET
	p_emu->special_reset();
#endif
}

void EmuThreadClass::loadState()
{
#ifdef USE_STATE
	if(!lStateFile.isEmpty()) {
		p_emu->load_state(lStateFile.toLocal8Bit().constData());
		lStateFile.clear();
	}
#endif
}

void EmuThreadClass::saveState()
{
#ifdef USE_STATE
	if(!sStateFile.isEmpty()) {
		p_emu->save_state(sStateFile.toLocal8Bit().constData());
		sStateFile.clear();
	}
#endif
}

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

	for(int i = 0; i < using_flags->get_max_qd(); i++) qd_text[i].clear();
	for(int i = 0; i < using_flags->get_max_drive(); i++) {
		fd_text[i].clear();
		fd_lamp[i] = QString::fromUtf8("×");
	}
	for(int i = 0; i < using_flags->get_max_tape(); i++) {
		cmt_text[i].clear();
	}
	for(int i = 0; i < (int)using_flags->get_max_cd(); i++) {
		cdrom_text[i] = QString::fromUtf8("×");
	}
	for(int i = 0; i < (int)using_flags->get_max_ld(); i++) {
		laserdisc_text[i].clear();
	}
	for(int i = 0; i < (int)using_flags->get_max_hdd(); i++) {
		hdd_text[i].clear();
		hdd_lamp[i].clear();
	}
	for(int i = 0; i < using_flags->get_max_bubble(); i++) bubble_text[i].clear();

	_queue_begin = parse_command_queue(virtualMediaList, 0);
	//SDL_SetHint(SDL_HINT_TIMER_RESOLUTION, "2");
	
	do {
		//p_emu->SetHostCpus(this->idealThreadCount());
   		if(MainWindow == NULL) {
			if(bRunThread == false){
				goto _exit;
			}
			msleep(10);
			//SDL_Delay(10);
			continue;
		}
		if(queue_fixed_cpu >= 0) {
			set_emu_thread_to_fixed_cpu(queue_fixed_cpu);
			queue_fixed_cpu = -1;
		}
		if(first) {
			if((using_flags->get_use_led_devices() > 0) || (using_flags->get_use_key_locked())) emit sig_send_data_led((quint32)led_data);
			for(int ii = 0; ii < using_flags->get_max_drive(); ii++ ) {
				emit sig_change_access_lamp(CSP_DockDisks_Domain_FD, ii, fd_lamp[ii]);
			}
			for(int ii = 0; ii < using_flags->get_max_cd(); ii++ ) {
				emit sig_change_access_lamp(CSP_DockDisks_Domain_CD, ii, cdrom_text[ii]);
			}
			for(int ii = 0; ii < using_flags->get_max_hdd(); ii++ ) {
				emit sig_change_access_lamp(CSP_DockDisks_Domain_HD, ii, hdd_text[ii]);
			}
			first = false;
		}
		interval = 0;
		sleep_period = 0;
		if(p_emu) {
			// drive machine
			if(bLoadStateReq != false) {
				loadState();
				bLoadStateReq = false;
				req_draw = true;
			}

			if(bResetReq != false) {
				resetEmu();
				bResetReq = false;
				req_draw = true;
			}
			if(bSpecialResetReq != false) {
				specialResetEmu();
				bSpecialResetReq = false;
			}
			if(bSaveStateReq != false) {
				saveState();
				bSaveStateReq = false;
			}

			if(using_flags->is_use_minimum_rendering()) {
				if((vert_line_bak != config.opengl_scanline_vert) ||
				   (horiz_line_bak != config.opengl_scanline_horiz) ||
				   (gl_crt_filter_bak != config.use_opengl_filters) ||
				   (opengl_filter_num_bak != config.opengl_filter_num)) req_draw = true;
				vert_line_bak = config.opengl_scanline_vert;
				horiz_line_bak = config.opengl_scanline_horiz;
				gl_crt_filter_bak = config.use_opengl_filters;
				opengl_filter_num_bak = config.opengl_filter_num;
			}

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
#if defined(USE_LASER_DISC) || defined(USE_MOVIE_PLAYER)	   
			if(turn_count < 128) {
				turn_count++;
			} else {
				if(vMovieQueue.size() >= 2) {
					for(int ii = 0; ii < vMovieQueue.size(); ii += 2) {
						QString _dom = vMovieQueue.at(ii);
						QString _path = vMovieQueue.at(ii + 1);
						bool _num_ok;
						int _dom_num = _dom.right(1).toInt(&_num_ok);
						if(!_num_ok) _dom_num = 0;
						emit sig_open_laser_disc(_dom_num, _path);
					}
					vMovieQueue.clear();
					//turn_count = 0;
				}
			}
#endif
#if defined(USE_MOUSE)	// Will fix
			emit sig_is_enable_mouse(p_emu->is_mouse_enabled());
#endif			
#if defined(USE_SOUND_VOLUME)
			for(int ii = 0; ii < USE_SOUND_VOLUME; ii++) {
				if(bUpdateVolumeReq[ii]) {
					p_emu->set_sound_device_volume(ii, config.sound_volume_l[ii], config.sound_volume_r[ii]);
					bUpdateVolumeReq[ii] = false;
				}
			}
#endif
			{
				while(!is_empty_key()) {
					key_queue_t sp;
					dequeue_key(&sp);
					//printf("%08x %04x %08x %d\n", sp.type, sp.code, sp.mod, sp.repeat); 
					switch(sp.type) {
					case KEY_QUEUE_UP:
							key_mod = sp.mod;
							p_emu->get_osd()->key_modifiers(sp.mod);
							p_emu->key_up(sp.code, true); // need decicion of extend.
							break;
					case KEY_QUEUE_DOWN:
							if(config.romaji_to_kana) {
								p_emu->get_osd()->key_modifiers(sp.mod);
								p_emu->key_char(sp.code);
							} else {
								p_emu->get_osd()->key_modifiers(sp.mod);
								p_emu->key_down(sp.code, true, sp.repeat);
							}
							break;
					default:
						break;
					}
				}
			}
			if(multithread_draw) {
				if(nr_fps < 0.0) {
					nr_fps = get_emu_frame_rate();
					if(nr_fps >= 1.0) emit sig_set_draw_fps(nr_fps);
				}
			}
			run_frames = p_emu->run();
			total_frames += run_frames;
			if(using_flags->is_use_minimum_rendering()) {
#if defined(USE_MINIMUM_RENDERING)
				req_draw |= p_emu->is_screen_changed();
#else
				req_draw = true;
#endif
			} else {
				req_draw = true;
			}

#if defined(USE_KEY_LOCKED) && !defined(INDEPENDENT_CAPS_KANA_LED)
			led_data = p_emu->get_caps_locked() ? 0x01 : 0x00;
			led_data |= (p_emu->get_kana_locked() ? 0x02 : 0x00);
#else
			led_data = 0x00;
#endif
#if defined(USE_LED_DEVICE)
  #if !defined(INDEPENDENT_CAPS_KANA_LED)
			led_data <<= USE_LED_DEVICE;
  #endif
	   		led_data |= p_emu->get_led_status();
#endif

#if defined(USE_LED_DEVICE) || defined(USE_KEY_LOCKED)
			if(led_data != led_data_old) {
				emit sig_send_data_led((quint32)led_data);
				led_data_old = led_data;
			}
#endif
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
			prev_skip = now_skip;
#if 0
			{
				struct tm *timedat;
				time_t nowtime;
				char strbuf2[256];
				char strbuf3[24];
				struct timeval tv;
				nowtime = time(NULL);
				gettimeofday(&tv, NULL);
				memset(strbuf2, 0x00, sizeof(strbuf2));
				memset(strbuf3, 0x00, sizeof(strbuf3));
				timedat = localtime(&nowtime);
				strftime(strbuf2, 255, "%Y-%m-%d %H:%M:%S", timedat);
				snprintf(strbuf3, 23, ".%06ld", tv.tv_usec);
				printf("%s%s[EMU]::RUN Frames = %d SKIP=%d Interval = %d NextTime = %d SRC = %d\n", strbuf2, strbuf3, run_frames, now_skip, interval, next_time, tick_timer.clockType());
			}
#endif			
			if(next_time > tick_timer.elapsed()) {
				//if(next_time > SDL_GetTicks()) {
				//  update window if enough time
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
				{
					double nd;
					nd = emu->get_frame_rate();
					if(nr_fps != nd) emit sig_set_draw_fps(nd);
					nr_fps = nd;
				}
				if(multithread_draw) {
					emit sig_draw_thread(req_draw);
				} else {
					emit sig_draw_thread(req_draw);
					emit sig_draw_one_turn(true);
				}
				skip_frames = 0;
			
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
				{
					double nd;
					nd = emu->get_frame_rate();
					if(nr_fps != nd) emit sig_set_draw_fps(nd);
					nr_fps = nd;
				}
				if(multithread_draw) {
					emit sig_draw_thread(req_draw);
				} else {
					emit sig_draw_thread(req_draw);
					emit sig_draw_one_turn(true);
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
		if(bRunThread == false){
			goto _exit;
		}
		if(sleep_period > 0) {
			//sleep_period = 1;
			msleep(sleep_period);
			//SDL_Delay(sleep_period);
		}
		//SDL_Delay(sleep_period);
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
	p_emu->set_host_window_size(w, h, true);
}

const _TCHAR *EmuThreadClass::get_emu_message(void)
{
	return (const _TCHAR *)(p_emu->message);
}

double EmuThreadClass::get_emu_frame_rate(void)
{
	return emu->get_frame_rate();
}

int EmuThreadClass::get_message_count(void)
{
	return p_emu->message_count;	
}

void EmuThreadClass::dec_message_count(void)
{
	p_emu->message_count--;
}

const _TCHAR *EmuThreadClass::get_device_name(void)
{
	return (const _TCHAR *)_T(DEVICE_NAME);
}

bool EmuThreadClass::get_power_state(void)
{
	return MainWindow->GetPowerState();
}

int EmuThreadClass::get_d88_file_cur_bank(int drive)
{
#ifdef USE_FLOPPY_DISK
	if(drive < USE_FLOPPY_DISK) {
		QMutexLocker _locker(&uiMutex);
		return p_emu->d88_file[drive].cur_bank;
	}
#endif
	return -1;
}

int EmuThreadClass::get_d88_file_bank_num(int drive)
{
#ifdef USE_FLOPPY_DISK
	if(drive < USE_FLOPPY_DISK) {
		QMutexLocker _locker(&uiMutex);
		return p_emu->d88_file[drive].bank_num;
	}
#endif
	return -1;
}


QString EmuThreadClass::get_d88_file_disk_name(int drive, int banknum)
{
#ifdef USE_FLOPPY_DISK
	if(drive < 0) return QString::fromUtf8("");
	if((drive < USE_FLOPPY_DISK) && (banknum < get_d88_file_bank_num(drive))) {
		QMutexLocker _locker(&uiMutex);
		QString _n = QString::fromLocal8Bit((const char *)(&(p_emu->d88_file[drive].disk_name[banknum][0])));
		return _n;
	}
#endif
	return QString::fromUtf8("");
}


bool EmuThreadClass::is_floppy_disk_protected(int drive)
{

#ifdef USE_FLOPPY_DISK
	QMutexLocker _locker(&uiMutex);

	bool _b = p_emu->is_floppy_disk_protected(drive);
	return _b;
#endif
	return false;
}

void EmuThreadClass::set_floppy_disk_protected(int drive, bool flag)
{
#ifdef USE_FLOPPY_DISK
	QMutexLocker _locker(&uiMutex);

	p_emu->is_floppy_disk_protected(drive, flag);
#endif
}

QString EmuThreadClass::get_d88_file_path(int drive)
{
#ifdef USE_FLOPPY_DISK
	if(drive < 0) return QString::fromUtf8("");
	if(drive < USE_FLOPPY_DISK) {
		QMutexLocker _locker(&uiMutex);
		QString _n = QString::fromUtf8((const char *)(&(p_emu->d88_file[drive].path)));
		return _n;
	}
#endif
	return QString::fromUtf8("");
}


