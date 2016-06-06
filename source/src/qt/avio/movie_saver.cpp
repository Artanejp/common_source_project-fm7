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
	_width = width;
	_height = height;
	rec_fps = fps;
	p_osd = osd;
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
	totalSrcFrame = 0;
	totalDstFrame = 0;
	totalAudioFrame = 0;

	video_bit_rate = 1500 * 1000;
	audio_bit_rate = 160 * 1000;
	video_geometry = QSize(640, 480);
	video_encode_threads = 4;

	bRunThread = false;
}

MOVIE_SAVER::~MOVIE_SAVER()
{
	if(recording) do_close();
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

    AGAR_DebugLog(AGAR_LOG_DEBUG, "pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           ts2str(pkt->pts).toLocal8Bit().constData(),
		   ts2timestr(pkt->pts, (void *)time_base).toLocal8Bit().constData(),
           ts2str(pkt->dts).toLocal8Bit().constData(),
		   ts2timestr(pkt->dts, (void *)time_base).toLocal8Bit().constData(),
           ts2str(pkt->duration).toLocal8Bit().constData(),
		   ts2timestr(pkt->duration, (void *)time_base).toLocal8Bit().constData(),
           pkt->stream_index);
#endif	
}


void MOVIE_SAVER::enqueue_video(QImage *p)
{
#if defined(USE_MOVIE_SAVER)
	if(!recording) return;
	if(p == NULL) return;
	uint32_t *pq;
	QImage *pp = new QImage(*p);
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Enqueue video data %d bytes %dx%d", pp->byteCount(), pp->width(), pp->height());
	video_data_queue.enqueue(pp);
#endif   
}

bool MOVIE_SAVER::dequeue_video(uint32_t *p)
{
	if(!recording) return false;
	if(p == NULL) return false;

	QImage *pp = video_data_queue.dequeue();
#if defined(USE_MOVIE_SAVER)
	if(pp == NULL) return false;
	int y;
	int x;
	uint32_t *pq;
	for(y = 0; y < _height; y++) {
		if(y >= pp->height()) break;
		pq = (uint32_t *)(pp->constScanLine(y));
		memcpy(&(p[_width * y]), pq, ((_width * sizeof(uint32_t)) > pp->bytesPerLine()) ? pp->bytesPerLine() : _width * sizeof(uint32_t));
	}
	video_size = _width * (y + 1);
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Dequeue video data %d bytes", pp->size());
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
	QByteArray *pp = new QByteArray((const char *)p, size);
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Enqueue audio data %d bytes", size);
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

	int fps_wait = (int)((1000.0 / p_osd->vm_frame_rate()) / 8.0);
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
			{
				//if(video_data_queue.isEmpty() || video_width_queue.isEmpty()
				//   || video_height_queue.isEmpty())
				if(video_data_queue.isEmpty())
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
			}  else {
				if(encode_audio == 0) {
					encode_audio = write_audio_frame();
					if(encode_audio < 0) {
						AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE/Saver: Something wrong with encoding audio.");
						goto _final;
					}
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
			fps_wait = (int)((1000.0 / p_osd->vm_frame_rate()) / 8.0);
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
	if(recording) do_close();
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
	//printf("width = %d -> %d\n", _width, width);
	_width = width;
}

void MOVIE_SAVER::do_set_height(int height)
{
	//printf("height = %d -> %d\n", _height, height);
	_height = height;
}

void MOVIE_SAVER::do_clear_options_list(void)
{
	encode_opt_keys.clear();
	encode_options.clear();
	//do_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("libx264"));
	//do_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("libfaac"));
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
	do_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("libfaac"));
	do_add_option(QString::fromUtf8("preset"), QString::fromUtf8("slow"));
	do_add_option(QString::fromUtf8("tune"), QString::fromUtf8("grain"));
	//do_add_option(QString::fromUtf8("crf"), QString::fromUtf8("20"));
	do_add_option(QString::fromUtf8("qmin"), QString::fromUtf8("16"));
	do_add_option(QString::fromUtf8("qmax"), QString::fromUtf8("24"));
	// Dummy
	do_add_option(QString::fromUtf8("qmax"), QString::fromUtf8("24"));
}

