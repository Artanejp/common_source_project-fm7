/*
 * Common Source Code Project for Qt : movie saver.
 * (C) 2016 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  License: GPLv2
 *  History: May 27, 2016 : Initial. This refer from avidemux 2.5.6 .
 */

#include <QDateTime>

#include "./csp_avio_basic.h"
#include "movie_saver.h"
#include "../osd.h"
#include "common.h"
#include "csp_logger.h"

#if (LIBAVCODEC_VERSION_MAJOR > 56)
#define AVCODEC_UPPER_V56
#endif

MOVIE_SAVER::MOVIE_SAVER(int width, int height, int fps, OSD *osd, config_t *cfg) : QThread(0)
{
	buffer_size=8 * 1024 * 224;
	max_rate=4000 * 1000;
	min_rate=0;
	_width = width;
	_height = height;
	rec_fps = fps;
	
	p_osd = osd;
	p_config = cfg;
	p_logger = osd->get_logger();
	
	recording = false;
	audio_sample_rate = 48000;
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
	
	do_reset_encoder_options();

	//video_queue_mutex = new QMutex(QMutex::Recursive);
	totalSrcFrame = 0;
	totalDstFrame = 0;
	totalAudioFrame = 0;

	video_bit_rate = 1500 * 1000;
	audio_bit_rate = 160 * 1000;
	video_geometry = QSize(640, 480);
	video_encode_threads = 4;
//	audio_enqueue_count = 0;
	left_frames = 0;
	req_close = false;
	req_stop = false;
	bRunThread = false;
	
}

MOVIE_SAVER::~MOVIE_SAVER()
{
	req_close = true;
	if(recording) do_close_main();
}

QString MOVIE_SAVER::get_avio_version()
{
#if defined(__LIBAVIO_VERSION)
	return QString::fromUtf8(__LIBAVIO_VERSION);
#else
	return QString::fromUtf8("\0");
#endif
}

QString MOVIE_SAVER::ts2str(int64_t ts)
{
#if defined(USE_LIBAV)	
	char buffer[AV_TS_MAX_STRING_SIZE + 16];
	memset(buffer, 0x00, sizeof(buffer));
	return QString::fromLocal8Bit(av_ts_make_string(buffer, ts));
#else
	return QString::fromUtf8("");
#endif
}		   

QString MOVIE_SAVER::ts2timestr(int64_t ts, void *timebase)
{
#if defined(USE_LIBAV)	
	char buffer[AV_TS_MAX_STRING_SIZE + 16];
	AVRational *tb = (AVRational *)timebase;
	memset(buffer, 0x00, sizeof(buffer));
	return QString::fromLocal8Bit(av_ts_make_time_string(buffer, ts, tb));
#else
	return QString::fromUtf8("");
#endif
}		   

QString MOVIE_SAVER::err2str(int errnum)
{
#if defined(USE_LIBAV)	
	char buffer[AV_TS_MAX_STRING_SIZE + 16];
	memset(buffer, 0x00, sizeof(buffer));
	return QString::fromLocal8Bit(av_make_error_string(buffer, sizeof(buffer), errnum));
#else
	return QString::fromUtf8("");
#endif
}		   

//void MOVIE_SAVER::log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
void MOVIE_SAVER::log_packet(const void *_fmt_ctx, const void *_pkt)
{
#if defined(USE_LIBAV)	
	const AVFormatContext *fmt_ctx = (const AVFormatContext *)_fmt_ctx;
	const AVPacket *pkt = (const AVPacket *)_pkt;
	AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

	out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
		   ts2str(pkt->pts).toLocal8Bit().constData(),
		   ts2timestr(pkt->pts, (void *)time_base).toLocal8Bit().constData(),
		   ts2str(pkt->dts).toLocal8Bit().constData(),
		   ts2timestr(pkt->dts, (void *)time_base).toLocal8Bit().constData(),
		   ts2str(pkt->duration).toLocal8Bit().constData(),
		   ts2timestr(pkt->duration, (void *)time_base).toLocal8Bit().constData(),
		   pkt->stream_index);
#endif	
}


void MOVIE_SAVER::enqueue_video(int frames, int width, int height, QImage *p)
{
#if defined(USE_MOVIE_SAVER)
	if(!recording) return;
	if(p == NULL) return;
	if(req_stop) return;
	if((width  <= 0) || (height <= 0) || (frames <= 0)) return;
	if((p->width() <= 0) || (p->height() <= 0)) return;
	VIDEO_DATA *px = new VIDEO_DATA(frames, width, height, p);
	//out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "Movie: Enqueue video data %dx%d %d bytes %dx%d %d frames", width, height, p->byteCount(), p->width(), p->height(), frames);
	//video_queue_mutex->lock();
	video_data_queue.enqueue(px);
	//video_queue_mutex->unlock();
#endif   
}

bool MOVIE_SAVER::dequeue_video(uint32_t *p)
{
	//if(!recording) return false;
	if(p == NULL) return false;

	VIDEO_DATA *pp = video_data_queue.dequeue();
#if defined(USE_MOVIE_SAVER)
	if(pp == NULL) return false;
	int y;
	uint32_t *pq;
	QImage *pp_i = &(pp->frame_data);
	left_frames = pp->frames;
	_width = pp->_width;
	_height = pp->_height;
	if((left_frames > 0) && (pp_i != NULL)){ 
		for(y = 0; y < _height; y++) {
			if(y >= pp_i->height()) break;
			pq = (uint32_t *)(pp_i->constScanLine(y));
			my_memcpy(&(p[_width * y]), pq, (((uint)_width * sizeof(uint32_t)) > pp_i->bytesPerLine()) ? pp_i->bytesPerLine() : _width * sizeof(uint32_t));
		}
		video_size = _width * y;
		//out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "Movie: Dequeue video data %d bytes", video_size);
	}
#else
	video_size = 0;
#endif   
	if(pp != NULL) delete pp;
	return true;
}

void MOVIE_SAVER::enqueue_audio(int16_t *p, int size)
{
#if defined(USE_MOVIE_SAVER)
	if(!recording) return;
	if(p == NULL) return;
	if(req_stop) return;
	QByteArray *pp = new QByteArray((const char *)p, size);
	//out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "Movie: Enqueue audio data %d bytes", size);
	audio_data_queue.enqueue(pp);
//	audio_enqueue_count++;
#endif   
}

bool MOVIE_SAVER::dequeue_audio(int16_t *p)
{
	//if(!recording) return false;
	if(audio_data_queue.isEmpty()) return false;
	if(p == NULL) return false;
	
	QByteArray *pp = audio_data_queue.dequeue();
	int16_t *q = (int16_t *)pp->constData();
	int32_t tmpd;
#if defined(USE_MOVIE_SAVER)
	if(pp == NULL) return false;
	audio_size = pp->size();
	if(audio_size <= 0) return false;
	// I expect to use SIMD.
	for(int i = 0; i < (int)(audio_size / sizeof(int16_t)); i++) {
		tmpd = q[i];
		tmpd <<= 4;
		tmpd = tmpd / 3;
		tmpd >>= 3;
		p[i] = tmpd;
	}
	//memcpy(p, pp->constData(), audio_size);
	audio_count++;
#else
	audio_size = 0;
#endif   
	if(pp != NULL) delete pp;
	return true;
}


void MOVIE_SAVER::run()
{
	bRunThread = true;
	//out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "MOVIE THREAD: Start");
	int ret;
	int fps_wait = (int)((1000.0 / (double)rec_fps) / 4.0);
	int tmp_wait = fps_wait;
	bool need_audio_transcode = false;
	bool need_video_transcode = false;
	bool a_f, v_f;
	a_f = v_f = false;
	volatile bool old_recording = false;
	audio_remain = 0;
	video_remain = 0;
	audio_offset = 0;
	audio_frame_offset = 0;
	video_offset = 0;
	n_encode_audio = 0;
	n_encode_video = 0;
	req_close = false;
	req_stop = false;
	left_frames = 0;
	while(bRunThread) {
		if(recording) {
			if(!bRunThread) break;
			if(!old_recording) {
				//n_encode_video = n_encode_audio = 1;
				audio_remain = 0;
				video_remain = 0;
				audio_offset = 0;
				audio_frame_offset = 0;
				video_offset = 0;
				video_count = 0;
				audio_count = 0;
				req_close = false;
				req_stop = false;
				left_frames = 0;
				if(!do_open_main()) {
					recording = false;
					goto _next_turn;
				}
				out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "MOVIE/Saver: Start to recording.");
				old_recording = true;
				a_f = v_f = false;
			}
			if(audio_remain <= 0) {
				a_f = audio_data_queue.isEmpty();
				if(a_f) goto _video;
				dequeue_audio(audio_frame_buf);
				audio_remain = audio_size;
				audio_offset = 0;
				need_audio_transcode = true;
			}
		_video:
			{
				v_f = video_data_queue.isEmpty();
				if(v_f)
					goto _write_frame;
				if(left_frames <= 0) dequeue_video(video_frame_buf);
				left_frames--;
				video_remain = video_size;
				video_offset = 0;
				need_video_transcode = true;
			}
		_write_frame:
			int result_ts = 0;
#ifdef AVCODEC_UPPER_V56
			if(audio_st.context != NULL) {
				if(video_st.context != NULL) {
					result_ts = av_compare_ts(video_st.next_pts, video_st.context->time_base,
											  audio_st.next_pts, audio_st.context->time_base);
				} else {
					result_ts = 1; // AUDIO ONLY
				}
			} else {
				if(video_st.context != NULL) {
					result_ts = -1; // VIDEO ONLY
				}
			}
#else
			result_ts =	 av_compare_ts(video_st.next_pts, video_st.st->codec->time_base,
									   audio_st.next_pts, audio_st.st->codec->time_base);
#endif
			if ((n_encode_video == 0) &&
				((n_encode_audio != 0) ||
				 (result_ts <= 0))) {
				n_encode_video = write_video_frame();
				ret = n_encode_video;
				if(n_encode_video < 0) {
					out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "MOVIE/Saver: Something wrong with encoding video.");
					goto _final;
				}
			} else {
				n_encode_audio = write_audio_frame();
				ret = n_encode_audio;
				if(n_encode_audio < 0) {
					out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "MOVIE/Saver: Something wrong with encoding audio.");
					goto _final;
				}
			}
			if (ret < 0 && ret != AVERROR_EOF) {
				char errbuf[128];
				av_strerror(ret, errbuf, sizeof(errbuf));
				out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error while filtering: %s\n", (const char *)errbuf);
				goto _final;
			}
			
			/* dump report by using the output first video and audio streams */
			//print_report(0, timer_start, cur_time);
			if(req_close) {
				do_close_main();
				need_video_transcode = need_audio_transcode = false;
				old_recording = false;
			}
		}
	_next_turn:
		//if(req_stop && a_f && v_f) req_close = true;
		if(!bRunThread) break;
		if(need_video_transcode || need_audio_transcode) {
			need_video_transcode = need_audio_transcode = false;
			continue;
		}
		if(fps_wait >= tmp_wait) {
			this->msleep(tmp_wait);
			tmp_wait = 0;
		} else {
			this->msleep(fps_wait);
			tmp_wait -= fps_wait;
		}
		if(tmp_wait <= 0) {
			fps_wait = (int)((1000.0 / (double)rec_fps) / 4.0);
			//fps_wait = 10;
			tmp_wait = fps_wait;
		}
		if(req_close) {
			do_close_main();
			old_recording = false;
		}
		continue;
	_final:
		req_close = true;
		do_close_main();
		old_recording = false;
	}
	out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "MOVIE: Exit thread.");
	if(recording) {
		req_close = true;
		do_close_main();
	}
}

bool MOVIE_SAVER::is_recording(void)
{
	return recording;
}


void MOVIE_SAVER::do_exit()
{
	bRunThread = false;
	req_close = true;
	req_stop = true;
}

void MOVIE_SAVER::do_set_record_fps(int fps)
{
	if((fps > 0) && (fps <= 75)) rec_fps = fps;
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

void MOVIE_SAVER::do_set_video_bitrate(int kbps)
{
	if(kbps < 10) return;
	video_bit_rate = kbps * 1000;
}

void MOVIE_SAVER::do_set_audio_bitrate(int kbps)
{
	if(kbps < 8) return;
	audio_bit_rate = kbps * 1000;
}

void MOVIE_SAVER::do_set_video_geometry(QSize geometry)
{
	if(geometry.width() < 100) return;
	if(geometry.height() < 80) return;
	video_geometry = geometry;
}

void MOVIE_SAVER::do_set_video_threads(int threads)
{
	video_encode_threads = threads;
}

void MOVIE_SAVER::do_reset_encoder_options(void)
{
	encode_opt_keys.clear();
	encode_options.clear();

	do_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("libx264"));
	do_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("aac"));
	do_add_option(QString::fromUtf8("preset"), QString::fromUtf8("slow"));
	do_add_option(QString::fromUtf8("tune"), QString::fromUtf8("grain"));
	//do_add_option(QString::fromUtf8("crf"), QString::fromUtf8("20"));
	do_add_option(QString::fromUtf8("qmin"), QString::fromUtf8("16"));
	do_add_option(QString::fromUtf8("qmax"), QString::fromUtf8("24"));
	// Dummy
	do_add_option(QString::fromUtf8("qmax"), QString::fromUtf8("24"));
}

