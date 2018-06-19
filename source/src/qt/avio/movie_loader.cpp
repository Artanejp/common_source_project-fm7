/*
 * Common Source Code Project for Qt : movie loader
 * (C) 2016 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  License: GPLv2
 *  History: June 30, 2016 : Initial. This refer from sample of ffmpeg .
 */

#include "../osd.h"
#include "movie_loader.h"
#include "csp_logger.h"

#include <QMutex>
#include <QMutexLocker>

MOVIE_LOADER::MOVIE_LOADER(OSD *osd, config_t *cfg) : QObject(NULL)
{
	p_osd = osd;
	p_cfg = cfg;
	p_logger = osd->get_logger();
	
	video_mutex = new QMutex(QMutex::Recursive);
	snd_write_lock = new QMutex(QMutex::Recursive);
	
	frame_rate = 29.97;
	video_frame_count = 0;
	duration_us = 0;
	audio_total_samples = 0;
	
	now_opening = false;
	use_hwaccel = false;
	
	_filename.clear();

	video_format.clear();
	video_codec.clear();
	audio_codec.clear();
	hwaccel_method.clear();

	now_pausing = false;
	now_playing = false;
	
	src_width = 640;
	src_height = 200;
	dst_width = 640;
	dst_height = 200;
	old_dst_width = dst_width;
	old_dst_height = dst_height;
	mod_frames = 0.0;
	sound_rate = 44100;
	req_transfer = true;

#if defined(USE_LIBAV)
	fmt_ctx = NULL;
	video_dec_ctx = NULL;
	audio_dec_ctx = NULL;
	sws_context = NULL;
	swr_context = NULL;
	video_stream = NULL;
	audio_stream = NULL;
	for(int i = 0; i < 4; i++) video_dst_data[i] = NULL;
	for(int i = 0; i < 4; i++) video_dst_linesize[i] = 0;
	video_dst_bufsize = 0;
	video_stream_idx = -1;
	audio_stream_idx = -1;
	frame = NULL;
	refcount = 0;
#endif

}

MOVIE_LOADER::~MOVIE_LOADER()
{
	delete video_mutex;
}


#if defined(USE_LIBAV)
int MOVIE_LOADER::decode_packet(int *got_frame, int cached)
{
	int ret = 0;
	int decoded = pkt.size;
	
	*got_frame = 0;
	
	if (pkt.stream_index == video_stream_idx) {
		/* decode video frame */
		ret = avcodec_decode_video2(video_dec_ctx, frame, got_frame, &pkt);
		if (ret < 0) {
			char str_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
			av_make_error_string(str_buf, AV_ERROR_MAX_STRING_SIZE, ret);
			p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Error decoding video frame (%s)\n", str_buf);
			return ret;
		}
		if (*got_frame) {
            if (frame->width != src_width || frame->height != src_height ||
                frame->format != pix_fmt) {
                /* To handle this change, one could call av_image_alloc again and
                 * decode the following frames into another rawvideo file. */
                p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Error: Width, height and pixel format have to be "
							  "constant in a rawvideo file, but the width, height or "
							  "pixel format of the input video changed:\n"
							  "old: width = %d, height = %d, format = %s\n"
							  "new: width = %d, height = %d, format = %s\n",
							  src_width, src_height, av_get_pix_fmt_name(pix_fmt),
							  frame->width, frame->height,
									  av_get_pix_fmt_name((enum AVPixelFormat)frame->format));
                return -1;
            }
			if((old_dst_width != dst_width) || (old_dst_height != dst_height)) { // You sould allocate on opening.
				QMutexLocker Locker_V(video_mutex);
				//video_mutex->lock();
				for(int i = 0; i < 4; i++) av_free(video_dst_data[i]);
				ret = av_image_alloc(video_dst_data, video_dst_linesize,
									 dst_width, dst_height, AV_PIX_FMT_BGRA, 1);
				
				if(ret < 0) {
					p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "MOVIE_LOADER: Could not re-allocate output buffer\n");
					old_dst_width = dst_width;
					old_dst_height = dst_height;
					//video_mutex->unlock();
					return -1;
				}
				old_dst_width = dst_width;
				old_dst_height = dst_height;
				//video_mutex->unlock();
			}
			
			char str_buf2[AV_TS_MAX_STRING_SIZE] = {0};


			video_frame_count++;
			//av_ts_make_time_string(str_buf2, frame->pts, &video_dec_ctx->time_base);
			//p_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_LOADER, "video_frame%s n:%d coded_n:%d pts:%s\n",
			//			  cached ? "(cached)" : "",
			//			  video_frame_count++, frame->coded_picture_number,
			//			  str_buf2);
			
            /* copy decoded frame to destination buffer:
             * this is required since rawvideo expects non aligned data */
			if(sws_context == NULL) {
				sws_context = sws_getContext(frame->width, frame->height,
											 (enum AVPixelFormat) frame->format,
											 dst_width, dst_height,
											 AV_PIX_FMT_BGRA,
											 SCALE_FLAGS, NULL, NULL, NULL);
				//p_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_LOADER, "Src frame=%dx%d Allocate frame: %dx%d", frame->width, frame->height, dst_width, dst_height);

				if (sws_context == NULL) {
					p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER,
								  "MOVIE_LOADER: Could not initialize the conversion context\n");
					return -1;
				}
				
			}
			video_mutex->lock();
			sws_scale(sws_context,
					  frame->data, frame->linesize,
					  0, frame->height, video_dst_data, video_dst_linesize);
			req_transfer = true;
			video_mutex->unlock();
		}
	} else if (pkt.stream_index == audio_stream_idx) {
		/* decode audio frame */
		ret = avcodec_decode_audio4(audio_dec_ctx, frame, got_frame, &pkt);
		if (ret < 0) {
			char str_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
			av_make_error_string(str_buf, AV_ERROR_MAX_STRING_SIZE, ret);
			p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Error decoding audio frame (%s)\n", str_buf);
			return ret;
		}
		/* Some audio decoders decode only part of the packet, and have to be
		 * called again with the remainder of the packet data.
		 * Sample: fate-suite/lossless-audio/luckynight-partial.shn
		 * Also, some decoders might over-read the packet. */
		decoded = FFMIN(ret, pkt.size);
			
		if (*got_frame) {
			//size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((enum AVSampleFormat)frame->format);
			char str_buf[AV_TS_MAX_STRING_SIZE] = {0};
			AVCodecContext *c = audio_stream->codec;
			int dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_context, c->sample_rate) + frame->nb_samples,
												c->sample_rate, c->sample_rate,  AV_ROUND_UP);
			//av_ts_make_time_string(str_buf, frame->pts, &audio_dec_ctx->time_base);
			//p_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_LOADER,"audio_frame%s n:%d nb_samples:%d pts:%s\n",
			//			  cached ? "(cached)" : "",
			//			  audio_frame_count++, frame->nb_samples,
			//			  str_buf);
			audio_total_samples += (int64_t)dst_nb_samples;
			/* Write the raw audio data samples of the first plane. This works
			 * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
			 * most audio decoders output planar audio, which uses a separate
			 * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
			 * In other words, this code will write only the first audio channel
			 * in these cases.
			 * You should use libswresample or libavfilter to convert the frame
			 * to packed data. */
			sound_data_queue_t *px = (sound_data_queue_t *)malloc(sizeof(sound_data_queue_t));
			if(px != NULL) {
				px->data[0] = (uint8_t *)malloc((long)dst_nb_samples * 2 * sizeof(int16_t));
				px->data[1] = px->data[2] = px->data[3] = NULL;
				if(px->data[0] == NULL) {
					free(px);
					p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Error while converting\n");
					return -1;
				}
				ret = swr_convert(swr_context,
								  px->data, dst_nb_samples,
								  (const uint8_t **)frame->data, frame->nb_samples);
				if (ret < 0) {
					free(px->data[0]);
					free(px);
					p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Error while converting\n");
					return -1;
				}
				px->unpadded_linesize = (long)dst_nb_samples * 2 * sizeof(int16_t);
				snd_write_lock->lock();
				sound_data_queue.enqueue(px);
				snd_write_lock->unlock();
			} else {
				p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Error while converting\n");
				return -1;
			}
		}
	}


    /* If we use frame reference counting, we own the data and need
     * to de-reference it when we don't use it anymore */
    if (*got_frame && refcount)
        av_frame_unref(frame);

    return decoded;
}

int MOVIE_LOADER::open_codec_context(int *stream_idx,
									 AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    AVCodecContext *dec_ctx = NULL;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), _filename.toLocal8Bit().constData());
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec_ctx = st->codec;
        dec = avcodec_find_decoder(dec_ctx->codec_id);
        if (!dec) {
            p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Init the decoders, with or without reference counting */
        av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
            p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

int MOVIE_LOADER::get_format_from_sample_fmt(const char **fmt,
											 enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}

#endif // USE_LIBAV

bool MOVIE_LOADER::open(QString filename)
{
    int ret = 0, got_frame;
	_filename = filename;
	if(_filename.isEmpty()) return false;
	
	mod_frames = 0.0;
	req_transfer = true;
	
	duration_us = 0;
	video_frame_count = 0;
	audio_total_samples = 0;
	
    /* register all formats and codecs */
    av_register_all();

    /* open input file, and allocate format context */
    if (avformat_open_input(&fmt_ctx, _filename.toLocal8Bit().constData(), NULL, NULL) < 0) {
        p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Could not open source file %s\n", _filename.toLocal8Bit().constData());
        return -1;
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Could not find stream information\n");
        return -1;;
    }

    if (open_codec_context(&video_stream_idx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        video_stream = fmt_ctx->streams[video_stream_idx];
        video_dec_ctx = video_stream->codec;

        /* allocate image where the decoded image will be put */
        src_width = video_dec_ctx->width;
        src_height = video_dec_ctx->height;
        pix_fmt = video_dec_ctx->pix_fmt;
		AVRational rate;
		rate = av_stream_get_r_frame_rate(video_stream);
		frame_rate = av_q2d(rate);
        if (ret < 0) {
            p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Could not allocate raw video buffer\n");
            goto _end;
        }
        video_dst_bufsize = ret;
    }

    if (open_codec_context(&audio_stream_idx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        audio_stream = fmt_ctx->streams[audio_stream_idx];
        audio_dec_ctx = audio_stream->codec;
		sound_rate = audio_stream->codec->sample_rate;
    }
	swr_context = swr_alloc();
	if(swr_context == NULL) {
		p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Could not allocate resampler context\n");
		goto _end;
	}
	av_opt_set_int	   (swr_context, "in_channel_count",   audio_stream->codec->channels,	   0);
	av_opt_set_int	   (swr_context, "in_sample_rate",	   audio_stream->codec->sample_rate,	0);
	av_opt_set_sample_fmt(swr_context, "in_sample_fmt",	   audio_stream->codec->sample_fmt, 0);
	av_opt_set_int	   (swr_context, "out_channel_count",  2,	   0);
	av_opt_set_int	   (swr_context, "out_sample_rate",	   sound_rate,	0);
	av_opt_set_sample_fmt(swr_context, "out_sample_fmt",   AV_SAMPLE_FMT_S16,	 0);
	
	/* initialize the resampling context */
	if ((ret = swr_init(swr_context)) < 0) {
		p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Failed to initialize the resampling context\n");
		goto _end;
	}

    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, _filename.toLocal8Bit().constData(), 0);
	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Video is %f fps", frame_rate);
	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Audio is %d Hz ", sound_rate);
    if (!audio_stream && !video_stream) {
        p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Could not find audio or video stream in the input, aborting\n");
        ret = 1;
        goto _end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto _end;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

	// Re-allocate buffer;
	video_mutex->lock();
	ret = av_image_alloc(video_dst_data, video_dst_linesize,
                             dst_width, dst_height, AV_PIX_FMT_BGRA, 1);
	old_dst_width = dst_width;
	old_dst_height = dst_height;
	video_mutex->unlock();
	if(ret < 0) {
		p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "MOVIE_LOADER: Could not re-allocate output buffer\n");
		//video_mutex->unlock();
		goto _end;
	}

	// ToDo : Initialize SWScaler and SWresampler.
	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "MOVIE_LOADER: Loading movie completed.\n");
	return true;
_end:
	this->close();
	return false;
}

void MOVIE_LOADER::close(void)
{
    if(video_dec_ctx != NULL) avcodec_close(video_dec_ctx);
    if(audio_dec_ctx != NULL) avcodec_close(audio_dec_ctx);
    avformat_close_input(&fmt_ctx);
	if(sws_context != NULL) {
		sws_freeContext(sws_context);
	}
	swr_free(&swr_context);
	
	video_mutex->lock();
	req_transfer = true;
	if(frame != NULL) av_frame_free(&frame);
	for(int i = 0; i < 4; i++) {
		if(video_dst_data[i] != NULL) av_free(video_dst_data[i]);
		video_dst_data[i] = NULL;
		video_dst_linesize[i] = 0;
	}
	video_mutex->unlock();
	
	video_dec_ctx = NULL;
	audio_dec_ctx = NULL;
	sws_context = NULL;
	swr_context = NULL;
	video_frame_count = 0;
	duration_us = 0;
	
	now_playing = false;
	now_pausing = false;
	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_LOADER, "MOVIE_LOADER: Close movie.");
}

double MOVIE_LOADER::get_movie_frame_rate(void)
{
	return frame_rate;
}

int MOVIE_LOADER::get_movie_sound_rate(void)
{
	return sound_rate;
}

uint64_t MOVIE_LOADER::get_current_frame(void)
{
	return video_frame_count;
}

bool MOVIE_LOADER::is_playing(void)
{
	return now_playing;
}


bool MOVIE_LOADER::is_pausing(void)
{
	return now_pausing;
}


void MOVIE_LOADER::do_play()
{
	now_playing = true;
	now_pausing = false;
}

void MOVIE_LOADER::do_stop()
{
	now_playing = false;
	now_pausing = false;
	duration_us = 0;
	// Still not closing.
}

void MOVIE_LOADER::do_pause(bool flag)
{
	if(now_playing) {
		// Pause button
		now_pausing = flag;
	}
}

void MOVIE_LOADER::do_mute(bool left, bool right)
{
	// Real Mute
}

void MOVIE_LOADER::do_decode_frames(int frames, int width, int height)
{
	int got_frame;
	bool end_of_frame = false;
	int real_frames = 0;
	double d_frames = (double)frames * (frame_rate / p_osd->vm_frame_rate());
	if(now_pausing) return;
	if(now_playing) duration_us = duration_us + (int64_t)((1.0e6 * (double)frames) / p_osd->vm_frame_rate());

	mod_frames = mod_frames + d_frames;
	real_frames = (int)mod_frames;
	mod_frames = mod_frames - (double)real_frames;

	if(width > 0) dst_width = width;
	if(height > 0) dst_height = height;
	
	if(real_frames <= 0) {
		do_dequeue_audio();
		///if(frames < 0) emit sig_decoding_error(MOVIE_LOADER_ILL_FRAME_NUMBER);
		return;
	}

	if(!now_playing || now_pausing) {
		uint8_t *null_sound;
		double t;
		uint32_t l;
	    t = (double)(p_cfg->sound_frequency) / frame_rate;
		l = 2 * sizeof(int16_t) * (int)(t + 0.5);
		null_sound = (uint8_t *)malloc(l);
		if(null_sound != NULL) {
			memset(null_sound, 0x00, sizeof(null_sound));
			emit sig_send_audio_frame(null_sound, l);
		}
		if(!now_pausing) {
			if(video_dst_data[0] != NULL) {
				uint32_t *q;
				video_mutex->lock();
				for(int yy = 0; yy < dst_height; yy++) { 
					q = (uint32_t *)(&(video_dst_data[0][yy * video_dst_linesize[0]]));
					if((q != NULL) && (dst_width != 0)) {
						memset(q, 0x00, dst_width * sizeof(uint32_t));
					}
				}
				video_mutex->unlock();
			}
		}
		return;
	}
	bool a_f = true;
	bool v_f = true;
	while(v_f || a_f) {
		v_f = (av_rescale_rnd(video_frame_count, 1000000000, (int64_t)(frame_rate * 1000.0), AV_ROUND_UP) < duration_us);
		a_f = (av_rescale_rnd(audio_total_samples, 1000000, audio_stream->codec->sample_rate, AV_ROUND_UP) < duration_us);
		//p_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_LOADER, "%lld usec. V=%lld A=%lld, %d - %d\n", duration_us, video_frame_count, audio_total_samples, v_f, a_f);
		if(!a_f && !v_f) break; 
		if(av_read_frame(fmt_ctx, &pkt) < 0) {
			this->close();
			return;
		}
		decode_packet(&got_frame, 0);
		if(got_frame == 0) {
			end_of_frame = true;
			break;
		}
	}
	if(end_of_frame) {
		// if real_frames > 1 then clear screen ?
		do_dequeue_audio();
		emit sig_movie_end(true);
		return;
	}
	do_dequeue_audio();
	return;
}

void MOVIE_LOADER::get_video_frame()
{
	uint32_t cacheline[8];
	uint32_t *p;
	uint32_t *q;
	int xx;

	QMutexLocker Locker_S(p_osd->screen_mutex);
	QMutexLocker Locker_V(video_mutex);

	if(req_transfer) {
		//p_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_LOADER, "Transfer frame: %dx%d", dst_width, dst_height);
		req_transfer = false;
		for(int yy = 0; yy < dst_height; yy++) {
			q = (uint32_t *)(p_osd->get_vm_screen_buffer(yy));
#if 1
			p = (uint32_t *)(&(video_dst_data[0][yy * video_dst_linesize[0]]));
			if((p == NULL) || (q == NULL)) break;
			for(xx = dst_width; xx > 7;) {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 8; i++) {
					cacheline[i] = p[i];
					q[i] = cacheline[i];
				}
				p += 8;
				q += 8;
				xx -= 8;
				if(xx < 8) break;
			}
			for(; xx > 0; xx--) {
				*q++ = *p++;
			}
#else
			//uint32_t col = 0xff000000 | (yy & 1) ? 0x00000000 : 0x00ff0000 | (yy & 2) ? 0x00000000 : 0x0000ff00 | (yy & 4) ? 0x00000000 : 0x000000ff;
			uint32_t col = 0xffffffff;
			if(q == NULL) break;
			for(xx = dst_width; xx > 7;) {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 8; i++) {
					q[i] = col;
				}
				p += 8;
				q += 8;
				xx -= 8;
				if(xx < 8) break;
			}
			for(; xx > 0; xx--) {
				*q++ = col;
			}
#endif		   
		}
	}
}

void MOVIE_LOADER::do_seek_frame(bool relative, int frames)
{
	// TODO.
}

void MOVIE_LOADER::do_dequeue_audio()
{
	long audio_data_size = 0;
	sound_data_queue_t *tmpq = NULL;
	
	snd_write_lock->lock();
	int il = sound_data_queue.count();
	while(il > 0) {
		tmpq = sound_data_queue.at(il - 1);
		if(tmpq != NULL) {
			audio_data_size += tmpq->unpadded_linesize;
		}
		il = il - 1;
	}
	tmpq = NULL;
	uint8_t *tmpdata = NULL;
	long dptr = 0;
	if(audio_data_size > 0) {
		tmpdata = (uint8_t *)malloc(audio_data_size);
		int ii;
		il = sound_data_queue.count();
		for(ii = 0; ii < il; ii++) {
			tmpq = sound_data_queue.dequeue();
			if(tmpq != NULL) {
				if(tmpq->data[0] != NULL) {
					if((tmpq->unpadded_linesize != 0) && (dptr < audio_data_size)) {
						if(tmpdata != NULL) {
							my_memcpy(&tmpdata[dptr], tmpq->data[0], tmpq->unpadded_linesize);
						}
						dptr += tmpq->unpadded_linesize;
					}
					free(tmpq->data[0]);
				}
				free(tmpq);
			}
		}
	}
	snd_write_lock->unlock();
	if((tmpdata != NULL) && (dptr != 0)) {
		//emit sig_send_audio_frame(tmpdata,dptr); // Please free arg1 after calling sound_callback of vm.
		//if(p_osd != NULL) p_osd->do_run_movie_audio_callback(tmpdata, dptr / (sizeof(int16_t) * 2));
		if(p_osd != NULL) p_osd->do_run_movie_audio_callback(tmpdata, dptr);
	}
}	

void MOVIE_LOADER::do_set_enable_hwaccel_decoding(bool enable)
{
	// ToDo.
}

void MOVIE_LOADER::do_set_enable_hwaccel_scaling(bool enable)
{
	// ToDo.
}

void MOVIE_LOADER::do_set_dst_geometry(int width, int height)
{
	if(width > 0) dst_width = width;
	if(height > 0) dst_height = height;
}

void MOVIE_LOADER::do_set_dst_pixfmt(int type)
{
	// ToDo.
}
