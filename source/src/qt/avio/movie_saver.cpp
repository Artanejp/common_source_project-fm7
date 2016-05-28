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
    AVCodecContext *c;
    AVPacket pkt = { 0 }; // data and size must be 0;
    AVFrame *frame;
#endif	
    int ret;
    int got_packet;
    int dst_nb_samples;

	int fps_wait = (int)((1000.0 / p_osd->vm_frame_rate()) / 2.0);
	int tmp_wait = fps_wait;
	int ncount_audio = 0;
	int ncount_video = 0;
	int64_t audio_remain = 0;
	int64_t video_remain = 0;
	uint32_t audio_offset = 0;
	uint32_t audio_frame_offset = 0;
	uint32_t video_offset = 0;
	bool audio_continue;
	bool video_continue;

#if defined(USE_LIBAV)
	av_init_packet(&pkt);
#endif
	
	while(bRunThread) {
		if(recording) {
			if(!bRunThread) break;
			if(!audio_data_queue.isEmpty()) {
				bool f = false;
				if(audio_remain <= 0) {
					f = true;
				}
				if(f || audio_continue) {
					f = dequeue_audio(audio_frame);
					audio_offset = 0;
					audio_remain = audio_size;
				}
				audio_continue = false;
				if(f || (audio_remain > 0)) {
#if defined(USE_LIBAV)
					uint64_t bytes;
					uint64_t us;
					double samples;
					int ret;
					int16_t *ptr = audio_frame;
					int16_t *optr;
					
					if(audio_remain <= 0) {
						bytes = audio_size;
						audio_remain = bytes;
					} else {
						bytes = audio_remain;
					}
					us = (uint64_t)floor(((double)bytes * 1000000.0) / (double)audio_codec_context->sample_rate);
					samples = ((double)us / 1000000.0) * (double)audio_codec_context->sample_rate;
					AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Saver: Write audio data %d bytes", bytes);
					
					if(bytes == 0) goto _video;

					{
						frame = audio_tmp_frame;
						if (av_compare_ts(audio_next_pts, audio_stream->codec->time_base,
										  (double)fps_wait, (AVRational){ 1, 1 }) >= 0) {
							//audio_continue = false;
							goto _video;
						}
						optr = (int16_t *)frame->data;
						for(int j = audio_frame_offset; j < frame->nb_samples; j++) {
							if(audio_remain <= 0) {
								audio_continue = true;
								goto _video;
							}
							if(audio_offset >= audio_size) {
								audio_offset = 0;
								audio_continue = true;
								goto _video;
							}
							for(int k = 0; k < audio_stream->codec->channels; k++) {
								optr[(audio_frame_offset * audio_stream->codec->channels)+ k] = ptr[audio_offset + k];
							}
							audio_offset += audio_stream->codec->channels;
							audio_remain -= audio_stream->codec->channels;
							audio_frame_offset++;
						}
						frame->pts = audio_next_pts;
						audio_next_pts += frame->nb_samples;
					}
					audio_frame_offset = 0;
					//if(!audio_resample((void *)audio_frame_data)) {
					//	do_close();
					//	audio_continue = false;
					//	goto _video;
					//}
					av_init_packet(&pkt);
					ret = avcodec_encode_audio2(audio_codec_context, &pkt, frame, &got_packet);
					if (ret < 0) {
						AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Saver : Error encoding audio frame\n");
						do_close();
						audio_continue = false;
						goto _video;
					}
					if (got_packet) {
						ret = write_audio_frame((const void *)(&audio_codec_context->time_base), (void *)(&pkt));
						if (ret < 0) {
							AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Audio Error while writing audio frame\n");
							do_close();
							audio_continue = false;
							goto _video;
						}
					}
#endif // defined(USE_LIBAV)
					totalAudioFrame++;
					ncount_audio++;
				}
			}
		_video:
			if(0) {
			}
		}
		if(ncount_audio > 10) { 
			ncount_audio = 0;
		}
		
		if(fps_wait >= tmp_wait) {
			this->msleep(tmp_wait);
			tmp_wait = 0;
		} else {
			this->msleep(fps_wait);
			tmp_wait -= fps_wait;
		}
		if(tmp_wait <= 0) {
			fps_wait = (int)((1000.0 / p_osd->vm_frame_rate()) / 2.0);
			tmp_wait = fps_wait;
		}
	}
}

void MOVIE_SAVER::do_close()
{
	int i;
#if defined(USE_LIBAV)
	if(output_context != NULL) {
		av_write_trailer(output_context);
		avio_close(output_context->pb);
	        //av_freep(&output_context->pb);
	}
	if(audio_stream != NULL) {
		av_free(audio_stream);
	}
	if(video_stream != NULL) {
		av_free(video_stream);
	}
	audio_stream = NULL;
	video_stream = NULL;

	if(output_context != NULL) {
		av_free(output_context);
		output_context = NULL;
	}
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

