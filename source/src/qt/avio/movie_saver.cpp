/*
 * Common Source Code Project for Qt : movie saver.
 * (C) 2016 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  License: GPLv2
 *  History: May 27, 2016 : Initial. This refer from avidemux 2.5.6 .
 */

#include <QDateTime>
#include "movie_saver.h"
#include "../osd.h"
#include "agar_logger.h"

MOVIE_SAVER::MOVIE_SAVER(int width, int height, int fps, OSD *osd) : QThread(0)
{
	buffer_size=8 * 1024 * 224;
	max_rate=4000 * 1000;
	min_rate=0;
	bitrate = 2000 * 1000;
	_width = width;
	_height = height;
	rec_fps = fps;
	p_osd = osd;
	recording = false;
#if defined(USE_MOVIE_SAVER)
	memset(audio_frame, 0x00, sizeof(audio_frame));
	memset(video_frame, 0x00, sizeof(video_frame));
	memset(video_dst, 0x00, sizeof(video_dst));
#endif
	audio_data_queue.clear();

	video_data_queue.clear();
	video_width_queue.clear();
	video_height_queue.clear();
	
	totalSrcFrame = 0;
	totalDstFrame = 0;
	totalAudioFrame = 0;
	bRunThread = false;
}

MOVIE_SAVER::~MOVIE_SAVER()
{
	do_close();
}

void MOVIE_SAVER::enqueue_video(QByteArray *p, int width, int height)
{
#if defined(USE_MOVIE_SAVER)
	if(!recording) return;
	if(p == NULL) return;
	QByteArray *pp = new QByteArray(p->data(), p->size());
	
	video_data_queue.enqueue(pp);
	video_width_queue.enqueue(width);
	video_height_queue.enqueue(height);
#endif   
}

bool MOVIE_SAVER::dequeue_video(uint32_t *p)
{
	if(!recording) return false;
	if(video_data_queue.isEmpty()) return false;
	if(p == NULL) return false;

	QByteArray *pp = video_data_queue.dequeue();
#if defined(USE_MOVIE_SAVER)
	if(pp == NULL) return false;
	
	video_size = pp->size();
	if(video_size <= 0) return false;
	memcpy(p, pp->data(), video_size);
#else
	video_size = 0;
#endif   
	if(pp != NULL) delete pp;
	
	return true;
}

void MOVIE_SAVER::enqueue_audio(QByteArray *p)
{
#if defined(USE_MOVIE_SAVER)
	if(!recording) return;
	if(p == NULL) return;
	QByteArray *pp = new QByteArray(p->data(), p->size());
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Enqueue audio data %d bytes", p->size());
	audio_data_queue.enqueue(pp);
#endif   
}

bool MOVIE_SAVER::dequeue_audio(int16_t *p)
{
	if(!recording) return false;
	if(audio_data_queue.isEmpty()) return false;
	if(p == NULL) return false;
	QByteArray *pp = audio_data_queue.dequeue();
#if defined(USE_MOVIE_SAVER)
	if(pp == NULL) return false;
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Dequeue audio data %d bytes", pp->size());

	audio_size = pp->size();
	if(audio_size <= 0) return false;
	memcpy(p, pp->constData(), audio_size);
#else
	audio_size = 0;
#endif   
	if(pp != NULL) delete pp;
	return true;
}


void MOVIE_SAVER::run()
{
	bRunThread = true;
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE THREAD: Start");
#if defined(USE_LIBAV)
    AVFormatContext *os;
    OutputStream *ost;
    InputStream *ist;
#endif	
    int ret;
    int got_packet;
    int dst_nb_samples;

	int fps_wait = (int)((1000.0 / p_osd->vm_frame_rate()) / 4.0);
	int tmp_wait = fps_wait;
	int ncount_audio = 0;
	int ncount_video = 0;
	bool audio_continue = false;
	bool video_continue = false;
	bool need_audio_transcode = false;
	bool need_video_transcode = false;
	int i;
    int64_t total_packets_written = 0;
	
	audio_remain = 0;
	video_remain = 0;
	audio_offset = 0;
	audio_frame_offset = 0;
	video_offset = 0;
	
#if defined(USE_LIBAV)
	av_init_packet(&pkt);
#endif
	
	while(bRunThread) {
		if(recording) {
			if(!bRunThread) break;
			if(old_recording != recording) {
				if(_filename.isEmpty()) {
					goto _next_turn;
				}
				ret = transcode_init();
				if (ret < 0) {
					goto _final;
				}
				timer_start = av_gettime_relative();
				audio_remain = 0;
				video_remain = 0;
				audio_offset = 0;
				audio_frame_offset = 0;
				video_frame_offset = 0;
				video_offset = 0;
						
			}
			if(audio_remain <= 0) {
				if(audio_data_queue.isEmpty()) goto _video;
				decueue_audio(audio_frame_buf);
				audio_remain = audio_size;
				audio_offset = 0;
				need_audio_transcode = true;
			}
		_video:
			if(video_remain <= 0) {
				if(video_data_queue.isEmpty() || video_width_queue.isEmpty()
				   || video_height_queue.isEmpty())
					goto _video;
				decueue_video(video_frame_buf);
				video_remain = video_size;
				video_offset = 0;
				need_video_transcode = true;
			}			
			//int64_t cur_time= av_gettime_relative();

			/* if 'q' pressed, exits */
			//if (stdin_interaction)
            //if (check_keyboard_interaction(cur_time) < 0)
            //    break;

			/* check if there's any stream where output is still needed */
			if (!need_output()) {
				AGAR_DebugLog(AGAR_LOG_INFO, "No more output streams to write to, finishing.\n");
				goto _final;
			}

			//if(!need_audio_transcode || !need_video_transcode) goto _next_turn;
		
			if(!need_audio_transcode) goto _next_turn;
			ret = transcode_step();
			need_audio_transcode = false;
			need_video_transcode = false;

			if (ret < 0 && ret != AVERROR_EOF) {
				char errbuf[128];
				av_strerror(ret, errbuf, sizeof(errbuf));
				AGAR_DebugLog(AGAR_LOG_INFO, "Error while filtering: %s\n", errbuf);
				goto _final;
			}

			/* dump report by using the output first video and audio streams */
			//print_report(0, timer_start, cur_time);
		}
	_next_turn:
		if(!bRunThread) break;
			
		if(fps_wait >= tmp_wait) {
			this->msleep(tmp_wait);
			tmp_wait = 0;
		} else {
			this->msleep(fps_wait);
			tmp_wait -= fps_wait;
		}
		if(tmp_wait <= 0) {
			fps_wait = (int)((1000.0 / p_osd->vm_frame_rate()) / 4.0);
			tmp_wait = fps_wait;
		}
		old_recording = recording;
		continue;
	_final:
		do_close();
		old_recording = false;
	}
	do_close();
}

void MOVIE_SAVER::do_close()
{
	int i;
#if defined(USE_LIBAV)
    int ret, i;
    AVFormatContext *os;
    OutputStream *ost;
    //InputStream *ist;
    int64_t total_packets_written = 0;
    /* at the end of stream, we must flush the decoder buffers */
    //for (i = 0; i < nb_input_streams; i++) {
        //ist = input_streams[i];
        //if (!input_files[ist->file_index]->eof_reached && ist->decoding_needed) {
		//process_input_packet(ist, NULL, 0);
			//}
    //}
    flush_encoders();

    //term_exit();

    /* write the trailer if needed and close file */
    for (i = 0; i < nb_output_files; i++) {
        os = output_files[i]->ctx;
        if ((ret = av_write_trailer(os)) < 0) {
            AGAR_DebugLog(AGAR_LOG_INFO, "Movie/Saver Error writing trailer of %s: %s", os->filename, (const char *)av_err2str(ret));
			return 1;
            //if (exit_on_error)
            //    exit_program(1);
        }
    }

    /* dump report by using the first video and audio streams */
    //print_report(1, timer_start, av_gettime_relative());

    /* close each encoder */
    for (i = 0; i < nb_output_streams; i++) {
        ost = output_streams[i];
        if (ost->encoding_needed) {
            av_freep(&ost->enc_ctx->stats_in);
        }
        total_packets_written += ost->packets_written;
    }

    if (!total_packets_written && (abort_on_flags & ABORT_ON_FLAG_EMPTY_OUTPUT)) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Empty output\n");
		return 1;
        //exit_program(1);
    }

    /* close each decoder */
    //for (i = 0; i < nb_input_streams; i++) {
        //ist = input_streams[i];
        //if (ist->decoding_needed) {
        //    avcodec_close(ist->dec_ctx);
        //    if (ist->hwaccel_uninit)
        //        ist->hwaccel_uninit(ist->dec_ctx);
        //}
    //}
#endif   // defined(USE_LIBAV)
	recording = false;

	while(!audio_data_queue.isEmpty()) {
		QByteArray *pp = audio_data_queue.dequeue();
		if(pp != NULL) delete pp;
	}
	audio_data_queue.clear();

	while(!video_data_queue.isEmpty()) {
		QByteArray *pp = video_data_queue.dequeue();
		if(pp != NULL) delete pp;
	}
	video_data_queue.clear();
	video_width_queue.clear();
	video_height_queue.clear();

	// Message
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Close: Read:   Video %lld frames, Audio %lld frames", totalSrcFrame, totalAudioFrame);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Close: Write:  Video %lld frames, Audio %lld frames", totalDstFrame, totalAudioFrame);
	totalSrcFrame = 0;
	totalDstFrame = 0;
	totalAudioFrame = 0;
}


QString MOVIE_SAVER::create_date_file_name(void)
{
	QDateTime nowTime = QDateTime::currentDateTime();
	QString tmps = nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz."));
	return tmps;
}

bool MOVIE_SAVER::is_recording(void)
{
	return recording;
}


void MOVIE_SAVER::do_exit()
{
	bRunThread = false;
}

void MOVIE_SAVER::do_set_record_fps(int fps)
{
	if((fps > 0) && (fps <= 60)) rec_fps = fps;
}

void MOVIE_SAVER::do_set_width(int width)
{
	_width = width;
}

void MOVIE_SAVER::do_set_height(int height)
{
	_height = height;
}

