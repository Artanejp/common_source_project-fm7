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



int MOVIE_SAVER::write_audio_frame(const void *_time_base, void *_pkt)
{
#if defined(USE_LIBAV)
	AVFormatContext *fmt_ctx = output_context;
	AVStream *st = (AVStream *)audio_stream;
	AVRational *time_base = (AVRational *)_time_base;
	AVPacket *pkt = (AVPacket *)_pkt;
	
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;
    /* Write the compressed frame to the media file. */
    //log_packet(fmt_ctx, pkt);
    return av_interleaved_write_frame(fmt_ctx, pkt);
#else
	return -1;
#endif
}

void MOVIE_SAVER::run()
{
	bRunThread = true;
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE THREAD: Start");
	
    AVCodecContext *c;
    AVPacket pkt = { 0 }; // data and size must be 0;
    AVFrame *frame;
    int ret;
    int got_packet;
    int dst_nb_samples;

	int fps_wait = (int)((1000.0 / p_osd->vm_frame_rate()) / 2.0);
	int tmp_wait = fps_wait;
	int ncount_audio = 0;
	int ncount_video = 0;

	while(bRunThread) {
		if(recording) {
			if(!bRunThread) break;
			if(!audio_data_queue.isEmpty()) {
				if(dequeue_audio(audio_frame)) {
#if defined(USE_LIBAV)
					AVPacket pkt;
					uint64_t bytes = audio_size;
					uint64_t us = (uint64_t)floor(((double)bytes * 1000000.0) / (double)audio_codec->sample_rate);
					double samples = ((double)us / 1000000.0) * (double)audio_codec->sample_rate;
					int ret;
					AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Write audio data %d bytes", bytes);
					
					if(bytes == 0) goto _video;
					
					av_init_packet(&pkt);
					{
						frame = audio_tmp_frame;
						if (av_compare_ts(audio_next_pts, audio_stream->codec->time_base,
										  (double)fps_wait, (AVRational){ 1, 1 }) >= 0)
							goto _video;
						frame->pts = audio_next_pts;
						audio_next_pts += frame->nb_samples;
					}

					/* convert samples from native format to destination codec format, using the resampler */
					/* compute destination number of samples */
					audio_dst_nb_samples = av_rescale_rnd(swr_get_delay(audio_swr_context, audio_codec->sample_rate)
														  + frame->nb_samples,
														  audio_codec->sample_rate,
														  audio_codec->sample_rate,
														  AV_ROUND_UP);
					//av_assert0(audio_dst_nb_samples == frame->nb_samples);
					/* when we pass a frame to the encoder, it may keep a reference to it
					 * internally;
					 * make sure we do not overwrite it here
					 */
					ret = av_frame_make_writable(frame);
					if (ret < 0) {
						AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Audio: Error while converting\n");
						do_close();
						continue;
					}
					/* convert to destination format */
					ret = swr_convert(audio_swr_context,
									  frame->data, audio_dst_nb_samples,
									  (const uint8_t **)frame->data, frame->nb_samples);
					if (ret < 0) {
						AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Audio: Error while converting\n");
						do_close();
						continue;
					}
					frame->pts = av_rescale_q(audio_samples_count, (AVRational){1, audio_codec->sample_rate},
											  audio_codec->time_base);
					audio_samples_count += dst_nb_samples;
					ret = avcodec_encode_audio2(audio_codec, &pkt, frame, &got_packet);
					if (ret < 0) {
						AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Audio : Error encoding audio frame\n");
						do_close();
						continue;
					}
					if (got_packet) {
						ret = write_audio_frame((const void *)(&audio_codec->time_base), (void *)(&pkt));
						if (ret < 0) {
							AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Audio Error while writing audio frame\n");
							do_close();
							continue;
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
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Close: Read:   Video %q frames, Audio %q frames", totalSrcFrame, totalAudioFrame);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Close: Write:  Video %q frames, Audio %q frames", totalDstFrame, totalAudioFrame);
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
#if defined(USE_MOVIE_SAVER) && defined(USE_LIBAV)
static const AVRational time_base_15 = (AVRational){1001, 14485};
static const AVRational time_base_24 = (AVRational){1001, 23976};
static const AVRational time_base_25 = (AVRational){1001, 25025};
static const AVRational time_base_30 = (AVRational){1001, 29970};
static const AVRational time_base_60 = (AVRational){1001, 59940};
#endif // defined(USE_MOVIE_SAVER) &&  defined(USE_LIBAV) 

AVFrame *MOVIE_SAVER::alloc_audio_frame(enum AVSampleFormat sample_fmt,
										uint64_t channel_layout,
										int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;
    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        //exit(1);
		return NULL;
    }
    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;
    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            return NULL;
        }
    }
    return frame;
}

void MOVIE_SAVER::do_open(QString filename, int fps)
{
	do_close();
	do_set_record_fps(fps);

	_filename = filename;
#if defined(USE_LIBAV)
	av_register_all();
	format = av_guess_format(NULL, filename.toLocal8Bit().constData(), NULL);
	printf("%s\n", filename.toLocal8Bit().constData());
	if(format == NULL) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to initialize libavf");
		return;
	}
	
	output_context = avformat_alloc_context();
	if(output_context == NULL) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to get output_context");
		do_close();
		return;
	}
	
	output_context->oformat = format;
	snprintf(output_context->filename, 1000, "file://%s", filename.toLocal8Bit().constData());
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Start rec VIDEO: %s", output_context->filename);

#if 0 // Todo	
	video_stream = av_new_stream(output_context, 0);
	if(video_stream == NULL) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to open video stream");
		do_close();
		return;
	}

	get_host_time(t_time);
	video_codec = video_stream->codec;
	video_codec->gop_size = 15;
	video_codec->max_b_frames = 8;
	video_codec->has_b_frames = 1;
	QString author = QString::fromUtf8("emu");
	author = author + p_osd->get_vm_config_name();
	QString date_str = QString::fromUtf8("Record from ");
	date_str = date_str + create_date_file_name();
	
	switch(rec_fps) {
	case 15:
		time_base = time_base_15;
		break;
	case 24:
		time_base = time_base_24;
		break;
	case 25:
		time_base = time_base_25;
		break;
	case 30:
		time_base = time_base_30;
		break;
	case 60:
		time_base = time_base_60;
		break;
	default:
		time_base = (AVRational){1001, rec_fps * 1000};
		break;
	}
	{ // Video Start
		av_dict_set(&output_context->metadata, "title", date_str.toUtf8().constData(), 0);
		av_dict_set(&output_context->metadata, "author", author.toUtf8().constData(), 0);
		video_codec->has_b_frames=2; // let muxer know we may have bpyramid
		video_codec->codec_id = CODEC_ID_H264;
		video_codec->codec = &codec_real;
		memset(video_codec->codec, 0, sizeof(struct AVCodec));
		strcpy(video_codec->codec->name, "H264");
		{
			// Set basic rate
			video_codec->rc_buffer_size = buffer_size;
			video_codec->rc_max_rate = max_rate;
			video_codec->rc_min_rate = min_rate;
			video_codec->bit_rate = bitrate;
		}
		video_codec->codec_type = AVMEDIA_TYPE_VIDEO;
		video_codec->flags = CODEC_FLAG_QSCALE;
		video_codec->width = _width;
		video_codec->height = _height;
	}

	switch(rec_fps) {
	case 15:
		video_codec->time_base = time_base_15;
		video_codec->bit_rate = bitrate / 2;
		video_codec->rc_max_rate = max_rate / 2;
		video_codec->rc_min_rate = min_rate / 2;
		break;
	case 24:
		video_codec->time_base = time_base_24;
		break;
	case 25:
		video_codec->time_base = time_base_25;
		break;
	case 30:
		video_codec->time_base = time_base_30;
		break;
	case 60:
		video_codec->time_base = time_base_60;
		video_codec->bit_rate = bitrate * 2;
		video_codec->rc_max_rate = max_rate * 2;
		video_codec->rc_min_rate = min_rate * 2;
		break;
	default:
		time_base = (AVRational){1001, rec_fps * 1000};
		video_codec->time_base = time_base;
		video_codec->bit_rate = (int)((double)bitrate * 30.0 / (double)rec_fps);
		video_codec->rc_max_rate = (int)((double)max_rate * 30.0 / (double)rec_fps);
		video_codec->rc_min_rate = (int)((double)min_rate * 30.0 / (double)rec_fps);
		break;
	}
#endif // ToDo
	
	//audio_stream = av_new_stream(&output_context, 1);
	audio_stream = avformat_new_stream(output_context, 0);
	if(audio_stream == NULL) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to open audio stream");
		do_close();
		return;
	}

	audio_next_pts = 0;
	audio_dst_nb_samples = 0;
	audio_samples_count = 0;
	// Temporally using libAV's AAC
	audio_codec = audio_stream->codec;
	audio_codec->frame_size = 1024; 
	audio_codec->codec_id = AV_CODEC_ID_AAC;
	audio_codec->sample_rate = p_osd->get_sound_rate();

	audio_codec->codec_type = AVMEDIA_TYPE_AUDIO;
	
	audio_codec->bit_rate = audio_codec->sample_rate * 8 * 2;
	audio_codec->rc_buffer_size = audio_codec->sample_rate / 4; // 250 ms worth
	audio_codec->channels = 2;
	
    if (audio_codec->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        audio_nb_samples = 10000;
    else
        audio_nb_samples = audio_codec->frame_size;
	audio_frame_data =  alloc_audio_frame(audio_codec->sample_fmt, audio_codec->channel_layout,
											 audio_codec->sample_rate, audio_nb_samples);
	audio_tmp_frame  =  alloc_audio_frame(audio_codec->sample_fmt, audio_codec->channel_layout,
											 audio_codec->sample_rate, audio_nb_samples);
	
	audio_swr_context =  swr_alloc();
	//if (audio_swr_context == NULL) {
	//	fprintf(stderr, "Could not allocate resampler context\n");
	//	return;
	//}
	av_opt_set_int       (audio_swr_context, "in_channel_count",   audio_codec->channels,       0);
	av_opt_set_int       (audio_swr_context, "in_sample_rate",     audio_codec->sample_rate,    0);
	av_opt_set_sample_fmt(audio_swr_context, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int       (audio_swr_context, "out_channel_count",  audio_codec->channels,       0);
	av_opt_set_int       (audio_swr_context, "out_sample_rate",    audio_codec->sample_rate,    0);
	av_opt_set_sample_fmt(audio_swr_context, "out_sample_fmt",     audio_codec->sample_fmt,     0);
	/* initialize the resampling context */
	//if ((ret = swr_init(ost->swr_ctx)) < 0) {
	//	fprintf(stderr, "Failed to initialize the resampling context\n");
	//	exit(1);
	//}
	// Context
	//output_context->mux_rate = 100080 * 1000;
	output_context->bit_rate = 100080 * 1000;
	output_context->audio_preload = AV_TIME_BASE / 10;
	output_context->max_delay = 100 * 1000; // MAX 100ms delay;

	if(avio_open(&(output_context->pb), filename.toLocal8Bit().constData(), AVIO_FLAG_WRITE) < 0) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to open file");
		do_close();
		return;
	}		

	av_dump_format(output_context, 0, filename.toLocal8Bit().constData(), 1);
	//av_dump_format(output_context, 1, NULL, 1);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Successfully opened AVC stream.");

#endif	// defined(USE_LIBAV)
	recording = true;
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

