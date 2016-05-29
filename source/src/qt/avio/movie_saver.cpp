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
	memset(audio_frame_buf, 0x00, sizeof(audio_frame_buf));
	memset(video_frame_buf, 0x00, sizeof(video_frame_buf));
	output_context = NULL;
	stream_format = NULL;
	audio_codec = video_codec = NULL;
	raw_options_list = NULL;
	
	memset(&video_st, 0x00, sizeof(video_st));
	memset(&audio_st, 0x00, sizeof(audio_st));
#endif

	output_context = NULL;
	
	audio_data_queue.clear();

	video_data_queue.clear();
	video_width_queue.clear();
	video_height_queue.clear();
	
	encode_opt_keys.clear();
	encode_options.clear();

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
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Dequeue video data %d bytes", pp->size());
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
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Enqueue audio data %d bytes", p->size());
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
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Dequeue audio data %d bytes", pp->size());

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
	int encode_audio = 0;
	int encode_video = 0;
	bool old_recording = false;
	audio_remain = 0;
	video_remain = 0;
	audio_offset = 0;
	audio_frame_offset = 0;
	video_offset = 0;
	
	while(bRunThread) {
		if(recording) {
			if(!bRunThread) break;
			if(old_recording != recording) {
				//if(_filename.isEmpty()) {
				//	goto _next_turn;
				//}
				//ret = transcode_init();
				//if (ret < 0) {
				//	goto _final;
				//}
				AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE/Saver: Start to recording.");
				encode_video = encode_audio = 1;
				audio_remain = 0;
				video_remain = 0;
				audio_offset = 0;
				audio_frame_offset = 0;
				video_offset = 0;
			}
			if(audio_remain <= 0) {
				if(audio_data_queue.isEmpty()) goto _video;
				dequeue_audio(audio_frame_buf);
				audio_remain = audio_size;
				audio_offset = 0;
				need_audio_transcode = true;
			}
		_video:
			if(video_remain <= 0) {
				if(video_data_queue.isEmpty() || video_width_queue.isEmpty()
				   || video_height_queue.isEmpty())
					goto _write_frame;
				dequeue_video(video_frame_buf);
				video_remain = video_size;
				video_offset = 0;
				need_video_transcode = true;
			}
		_write_frame:
			if ((encode_video == 0) &&
				((encode_audio != 0) || av_compare_ts(video_st.next_pts, video_st.st->codec->time_base,
														audio_st.next_pts, audio_st.st->codec->time_base) <= 0)) {
				encode_video = write_video_frame();
				if(encode_video < 0) {
					AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE/Saver: Something wrong with encoding video.");
					goto _final;
				}
				need_video_transcode = false;
			}  else {
				if(encode_audio == 0) {
					encode_audio = write_audio_frame();
					if(encode_audio < 0) {
						AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE/Saver: Something wrong with encoding audio.");
						goto _final;
					}
					need_audio_transcode = false;
				}
			}
			if (ret < 0 && ret != AVERROR_EOF) {
				char errbuf[128];
				av_strerror(ret, errbuf, sizeof(errbuf));
				AGAR_DebugLog(AGAR_LOG_INFO, "Error while filtering: %s\n", (const char *)errbuf);
				goto _final;
			}
			
			/* dump report by using the output first video and audio streams */
			//print_report(0, timer_start, cur_time);
		}
	_next_turn:
		old_recording = recording;
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
		old_recording = recording;
		do_close();
		old_recording = false;
	}
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Exit thread.");
	do_close();
}

void MOVIE_SAVER::do_close()
{
	recording = false;
#if defined(USE_LIBAV)
    int ret, i;
    AVFormatContext *os;
    OutputStream *ost;
    //InputStream *ist;
    int64_t total_packets_written = 0;
	if(output_context != NULL) {
		AVFormatContext *oc = output_context;
		AVOutputFormat *fmt = oc->oformat;
 
		av_write_trailer(oc);

		/* Close each codec. */
		if (have_video)
			close_stream(oc, &video_st);
		if (have_audio)
			close_stream(oc, &audio_st);
		have_video = have_audio = 0;
		if (!(fmt->flags & AVFMT_NOFILE))
			/* Close the output file. */
			avio_closep(&oc->pb);

		/* free the stream */
		avformat_free_context(output_context);
		output_context = NULL;
		stream_format = NULL;
		audio_codec = video_codec = NULL;
		raw_options_list = NULL;

		memset(&video_st, 0x00, sizeof(video_st));
		memset(&audio_st, 0x00, sizeof(audio_st));
	}
#endif   // defined(USE_LIBAV)
	memset(audio_frame_buf, 0x00, sizeof(audio_frame_buf));
	memset(video_frame_buf, 0x00, sizeof(video_frame_buf));


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

void MOVIE_SAVER::do_clear_options_list(void)
{
	encode_opt_keys.clear();
	encode_options.clear();
}

void MOVIE_SAVER::do_add_option(QString key, QString value)
{
	if(key.isEmpty()) return;
	encode_opt_keys.append(key);
	if(value.isEmpty()) {
		encode_options.append(QString::fromUtf8(""));
	} else {
		encode_options.append(value);
	}
}		
