/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2015.11.10 Split from qt_main.cpp

	[ win32 main ] -> [ Qt main ] -> [Emu Thread]
*/

#include <QString>
#include <QTextCodec>
#include <QWaitCondition>

#include <SDL.h>

#include "emu_thread.h"

#include "qt_gldraw.h"
#include "agar_logger.h"

// buttons
#ifdef MAX_BUTTONS
#define MAX_FONT_SIZE 32
#endif
#define MAX_SKIP_FRAMES 10

EmuThreadClass::EmuThreadClass(META_MainWindow *rootWindow, EMU *pp_emu, QObject *parent) : QThread(parent) {
	MainWindow = rootWindow;
	p_emu = pp_emu;
	bRunThread = true;
	prev_skip = false;
	update_fps_time = SDL_GetTicks();
	next_time = update_fps_time;
	total_frames = 0;
	draw_frames = 0;
	skip_frames = 0;
	calc_message = true;
	mouse_flag = false;
	//p_emu->set_parent_handler(this);
	drawCond = new QWaitCondition();
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
	tape_play_flag = false;
	tape_rec_flag = false;
	tape_pos = 0;
#endif	
};

EmuThreadClass::~EmuThreadClass() {
	delete drawCond;
};


int EmuThreadClass::get_interval(void)
{
	static int accum = 0;
	accum += p_emu->frame_interval();
	int interval = accum >> 10;
	accum -= interval << 10;
	return interval;
}

void EmuThreadClass::doExit(void)
{
	int status;
	bRunThread = false;
}

void EmuThreadClass::moved_mouse(int x, int y)
{
	p_emu->set_mouse_pointer(x, y);
}

void EmuThreadClass::button_pressed_mouse(Qt::MouseButton button)
{
	int stat = p_emu->get_mouse_button();
	bool flag = p_emu->get_mouse_enabled();
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
	}
	p_emu->set_mouse_button(stat);
}

void EmuThreadClass::button_released_mouse(Qt::MouseButton button)
{
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
	}
	p_emu->set_mouse_button(stat);
}


void EmuThreadClass::set_tape_play(bool flag)
{
#ifdef USE_TAPE_BUTTON
	tape_play_flag = flag;
#endif
}

EmuThreadClass *EmuThreadClass::currentHandler()
{
	return this;
}

void EmuThreadClass::resize_screen(int screen_width, int screen_height, int stretched_width, int stretched_height)
{
	//emit sig_resize_uibar(stretched_width, stretched_height);
	emit sig_resize_screen(screen_width, screen_height);
}

void EmuThreadClass::do_start_auto_key(QString ctext)
{
#ifdef USE_AUTO_KEY
	QTextCodec *codec = QTextCodec::codecForName("Shift-Jis");
	QByteArray array;
	clipBoardText = ctext;
	if(clipBoardText.size() > 0) {
		array = codec->fromUnicode(clipBoardText.toUtf8());
		//p_emu->set_auto_key_string((const char *)array.constData());
		emit sig_auto_key_string(array);
		//AGAR_DebugLog(AGAR_LOG_DEBUG, "AutoKey: SET :%s\n", clipBoardText.toUtf8().constData());
		p_emu->start_auto_key();
	}
	//clipBoardText.clear();
#endif	
}

void EmuThreadClass::do_stop_auto_key(void)
{
#ifdef USE_AUTO_KEY
	AGAR_DebugLog(AGAR_LOG_DEBUG, "AutoKey: stop\n");
	p_emu->stop_auto_key();
#endif	
}

#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
void EmuThreadClass::do_write_protect_disk(int drv, bool flag)
{
	p_emu->set_disk_protected(drv, flag);
}

void EmuThreadClass::do_close_disk(int drv)
{
	p_emu->close_disk(drv);
	p_emu->d88_file[drv].bank_num = 0;
	p_emu->d88_file[drv].cur_bank = -1;
}

void EmuThreadClass::do_open_disk(int drv, QString path, int bank)
{
   
	QByteArray localPath = path.toLocal8Bit();
   
	p_emu->d88_file[drv].bank_num = 0;
	p_emu->d88_file[drv].cur_bank = -1;
	
	if(check_file_extension(localPath.constData(), ".d88") || check_file_extension(localPath.constData(), ".d77")) {
		
		FILEIO *fio = new FILEIO();
		if(fio->Fopen(localPath.constData(), FILEIO_READ_BINARY)) {
			try {
				fio->Fseek(0, FILEIO_SEEK_END);
				int file_size = fio->Ftell(), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && p_emu->d88_file[drv].bank_num < MAX_D88_BANKS) {
					fio->Fseek(file_offset, FILEIO_SEEK_SET);
					char tmp[18];
					memset(tmp, 0x00, sizeof(tmp));
					fio->Fread(tmp, 17, 1);
					memset(p_emu->d88_file[drv].disk_name[p_emu->d88_file[drv].bank_num], 0x00, 128);
					if(strlen(tmp) > 0) Convert_CP932_to_UTF8(p_emu->d88_file[drv].disk_name[p_emu->d88_file[drv].bank_num], tmp, 127, 17);
					
					fio->Fseek(file_offset + 0x1c, FILEIO_SEEK_SET);
				        file_offset += fio->FgetUint32_LE();
					p_emu->d88_file[drv].bank_num++;
				}
				strcpy(p_emu->d88_file[drv].path, path.toUtf8().constData());
			        if(bank >= p_emu->d88_file[drv].bank_num) bank = p_emu->d88_file[drv].bank_num - 1;
			        if(bank < 0) bank = 0;
				p_emu->d88_file[drv].cur_bank = bank;
			}
			catch(...) {
				bank = 0;
				p_emu->d88_file[drv].bank_num = 0;
			}
		   	fio->Fclose();
		}
	   	delete fio;
	} else {
	   bank = 0;
	}
	p_emu->open_disk(drv, localPath.constData(), bank);
	emit sig_update_recent_disk(drv);
}

#endif

#ifdef USE_TAPE
void EmuThreadClass::do_play_tape(QString name)
{
	p_emu->play_tape(name.toLocal8Bit().constData());
}

void EmuThreadClass::do_rec_tape(QString name)
{
	p_emu->rec_tape(name.toLocal8Bit().constData());
}

void EmuThreadClass::do_close_tape(void)
{
	p_emu->close_tape();
}

# ifdef USE_TAPE_BUTTON
void EmuThreadClass::do_cmt_push_play(void)
{
	p_emu->push_play();
}

void EmuThreadClass::do_cmt_push_stop(void)
{
	p_emu->push_stop();
}

void EmuThreadClass::do_cmt_push_fast_forward(void)
{
	p_emu->push_fast_forward();
}

void EmuThreadClass::do_cmt_push_fast_rewind(void)
{
	p_emu->push_fast_rewind();
}

void EmuThreadClass::do_cmt_push_apss_forward(void)
{
	p_emu->push_apss_forward();
}

void EmuThreadClass::do_cmt_push_apss_rewind(void)
{
	p_emu->push_apss_rewind();
}

# endif
#endif

#ifdef USE_QD1
void EmuThreadClass::do_write_protect_quickdisk(int drv, bool flag)
{
	//p_emu->write_protect_Qd(drv, flag);
}

void EmuThreadClass::do_close_quickdisk(int drv)
{
	p_emu->close_quickdisk(drv);
}

void EmuThreadClass::do_open_quickdisk(int drv, QString path)
{
	p_emu->open_quickdisk(drv, path.toLocal8Bit().constData());
}
#endif

#ifdef USE_CART1
void EmuThreadClass::do_close_cart(int drv)
{
	p_emu->close_cart(drv);
}

void EmuThreadClass::do_open_cart(int drv, QString path)
{
	p_emu->open_cart(drv, path.toLocal8Bit().constData());
}
#endif

#ifdef USE_LASER_DISK
void EmuThreadClass::do_close_laser_disk(void)
{
	p_emu->close_laser_disk();
}

void EmuThreadClass::do_open_laser_disk(QString path)
{
	p_emu->open_laser_disk(path.toLocal8Bit().constData());
}
#endif
#ifdef USE_BINARY_FILE1
void EmuThreadClass::do_load_binary(int drv, QString path)
{
	p_emu->load_binary(drv, path.toLocal8Bit().constData());
}

void EmuThreadClass::do_save_binary(int drv, QString path)
{
	p_emu->save_binary(drv, path.toLocal8Bit().constData());
}
#endif

void EmuThreadClass::print_framerate(int frames)
{
	if(frames >= 0) draw_frames += frames;
	if(calc_message) {
		uint32_t current_time = SDL_GetTicks();
			if(update_fps_time <= current_time && update_fps_time != 0) {
				_TCHAR buf[256];
				QString message;
				int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);

#ifdef USE_POWER_OFF
				if(MainWindow->GetPowerState() == false){ 	 
					snprintf(buf, 255, _T("*Power OFF*"));
				} else {
#endif // USE_POWER_OFF		
					if(p_emu->message_count > 0) {
						snprintf(buf, 255, _T("%s - %s"), DEVICE_NAME, p_emu->message);
						p_emu->message_count--;
					} else {
						snprintf(buf, 255, _T("%s - %d fps (%d %%)"), DEVICE_NAME, draw_frames, ratio);
					}
#ifdef USE_POWER_OFF
				} 
#endif // USE_POWER_OFF	 
	      
				message = buf;
				emit message_changed(message);
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

void EmuThreadClass::do_draw_timing(bool f)
{
	draw_timing = f;
}

void EmuThreadClass::sample_access_drv(void)
{
	uint32 access_drv;
	QString alamp;
	QString tmpstr;
	QString iname;
	int i;
#if defined(USE_QD1)
# if defined(USE_ACCESS_LAMP)      
	access_drv = p_emu->get_vm()->access_lamp();
# endif
	for(i = 0; i < MAX_QD ; i++) {
		if(p_emu->quickdisk_inserted(i)) {
		//	     printf("%d\n", access_drv);
# if defined(USE_ACCESS_LAMP)      
			if(i == (access_drv - 1)) {
				alamp = QString::fromUtf8("● ");
			} else {
				alamp = QString::fromUtf8("○ ");
			}
			tmpstr = QString::fromUtf8("QD");
			tmpstr = alamp + tmpstr + QString::number(i) + QString::fromUtf8(":");
# else
			tmpstr = QString::fromUtf8("QD");
			tmpstr = tmpstr + QString::number(i) + QString::fromUtf8(":");
# endif
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

#if defined(USE_FD1)
# if defined(USE_ACCESS_LAMP)      
	access_drv = p_emu->get_vm()->access_lamp();
# endif
	for(i = 0; i < MAX_FD; i++) {
		if(p_emu->disk_inserted(i)) {
# if defined(USE_ACCESS_LAMP)      
			if(i == (access_drv - 1)) {
				alamp = QString::fromUtf8("<FONT COLOR=RED>●</FONT> ");
			} else {
				alamp = QString::fromUtf8("○ ");
			}
			tmpstr = QString::fromUtf8("FD");
			tmpstr = alamp + tmpstr + QString::number(i) + QString::fromUtf8(":");
# else
			tmpstr = QString::fromUtf8("FD");
			tmpstr = tmpstr + QString::number(i) + QString::fromUtf8(":");
# endif
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
#endif
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
	if(p_emu->tape_inserted()) {
		int tape_counter = p_emu->tape_position();
		tmpstr = QString::fromUtf8("");
		if(p_emu->tape_playing()) {
			tmpstr = QString::fromUtf8("<FONT COLOR=BLUE>▶ </FONT>");
		} else if(p_emu->tape_recording()) {
			tmpstr = QString::fromUtf8("<FONT COLOR=RED>● </FONT>");
		} else {
			tmpstr = QString::fromUtf8("<FONT COLOR=BLACK>■ </FONT>");
		}
		if(tape_counter >= 100) {
			tmpstr = tmpstr + QString::fromUtf8("BOTTOM");
		} else if(tape_counter >= 0) {
			tmpstr = tmpstr + QString::number(tape_counter) + QString::fromUtf8("%");
		} else {
			tmpstr = tmpstr + QString::fromUtf8("TOP");
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

void EmuThreadClass::doWork(const QString &params)
{
	int interval = 0, sleep_period = 0;
	int run_frames;
	bool now_skip;
	uint32 current_time;
	bool first = true;
#ifdef SUPPORT_DUMMY_DEVICE_LED
	uint32 led_data = 0x00000000;
	uint32 led_data_old = 0x00000000;
#endif
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
	bool tape_flag;
	int tpos;
#endif
#ifdef USE_DIG_RESOLUTION
	int width, height;
#endif
	QString ctext;
	bool req_draw = true;
	bool vert_line_bak = config.opengl_scanline_vert;
	bool horiz_line_bak = config.opengl_scanline_horiz;
	bool gl_crt_filter_bak = config.use_opengl_filters;
	int opengl_filter_num_bak = config.opengl_filter_num;
	int no_draw_count = 0;
	
	ctext.clear();
	draw_timing = false;
	bResetReq = false;
	bSpecialResetReq = false;
	bLoadStateReq = false;
	bSaveStateReq = false;
	bUpdateConfigReq = false;
	bStartRecordSoundReq = false;
	bStopRecordSoundReq = false;
	
	next_time = 0;
	mouse_flag = false;
	//(this->idealThreadCount());
	
#if defined(USE_QD1)
	for(int i = 0; i < 2; i++) qd_text[i].clear();
#endif
#if defined(USE_QD1)
	for(int i = 0; i < MAX_FD; i++) fd_text[i].clear();
#endif
#if defined(USE_TAPE)
	cmt_text.clear();
#endif
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
#ifdef SUPPORT_DUMMY_DEVICE_LED
			emit sig_send_data_led((quint32)led_data);
#endif
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
			if((vert_line_bak != config.opengl_scanline_vert) ||
			   (horiz_line_bak != config.opengl_scanline_horiz) ||
			   (gl_crt_filter_bak != config.use_opengl_filters) ||
			   (opengl_filter_num_bak != config.opengl_filter_num)) req_draw = true;
			vert_line_bak = config.opengl_scanline_vert;
			horiz_line_bak = config.opengl_scanline_horiz;
			gl_crt_filter_bak = config.use_opengl_filters;
			opengl_filter_num_bak = config.opengl_filter_num;
#endif
			if(bStartRecordSoundReq != false) {
				p_emu->start_rec_sound();
				bStartRecordSoundReq = false;
				req_draw = true;
			}
			if(bStopRecordSoundReq != false) {
				p_emu->stop_rec_sound();
				bStopRecordSoundReq = false;
				req_draw = true;
			}
			if(bUpdateConfigReq != false) {
				p_emu->update_config();
				bUpdateConfigReq = false;
				req_draw = true;
			}
			run_frames = p_emu->run();
			total_frames += run_frames;
#if defined(USE_MINIMUM_RENDERING)
			req_draw |= p_emu->screen_changed();
#else
			req_draw = true;
#endif			
#ifdef SUPPORT_DUMMY_DEVICE_LED
	   		led_data = p_emu->get_led_status();
			if(led_data != led_data_old) {
				emit sig_send_data_led((quint32)led_data);
				led_data_old = led_data;
			}
#endif
			sample_access_drv();

			interval += get_interval();
			now_skip = p_emu->now_skip() && !p_emu->now_rec_video();

			if((prev_skip && !now_skip) || next_time == 0) {
				next_time = SDL_GetTicks();
			}
			if(!now_skip) {
				next_time += interval;
			}
			prev_skip = now_skip;
			//printf("p_emu::RUN Frames = %d SKIP=%d Interval = %d NextTime = %d\n", run_frames, now_skip, interval, next_time);
      
			if(next_time > SDL_GetTicks()) {
				//  update window if enough time
				draw_timing = false;
				if(!req_draw) {
					no_draw_count++;
					if(no_draw_count > (int)(FRAMES_PER_SEC / 4)) {
						req_draw = true;
						no_draw_count = 0;
					}
				} else {
					no_draw_count = 0;
				}
				emit sig_draw_thread(req_draw);
				skip_frames = 0;
			
				// sleep 1 frame priod if need
				current_time = SDL_GetTicks();
				if((int)(next_time - current_time) >= 10) {
					sleep_period = next_time - current_time;
				}
			} else if(++skip_frames > MAX_SKIP_FRAMES) {
				// update window at least once per 10 frames
				draw_timing = false;
				emit sig_draw_thread(true);
				no_draw_count = 0;
				skip_frames = 0;
				uint32_t tt = SDL_GetTicks();
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
	AGAR_DebugLog(AGAR_LOG_DEBUG, "EmuThread : EXIT");
	emit sig_finished();
	this->quit();
}

void EmuThreadClass::doSetDisplaySize(int w, int h, int ww, int wh)
{
	p_emu->suspend();
	//p_emu->set_vm_screen_size(w, h, -1, -1, ww, wh);
	p_emu->set_window_size(w, h, true);
}

void EmuThreadClass::doUpdateConfig()
{
	bUpdateConfigReq = true;
}

void EmuThreadClass::doStartRecordSound()
{
	bStartRecordSoundReq = true;
}

void EmuThreadClass::doStopRecordSound()
{
	bStopRecordSoundReq = true;
}

void EmuThreadClass::doReset()
{
	bResetReq = true;
}

void EmuThreadClass::doSpecialReset()
{
	bSpecialResetReq = true;
}

void EmuThreadClass::doLoadState()
{
	bLoadStateReq = true;
}

void EmuThreadClass::doSaveState()
{
	bSaveStateReq = true;
}

