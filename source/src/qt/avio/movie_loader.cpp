


#include "../osd.h"
#include "movie_loader.h"
#include "agar_logger.h"

#include <QMutex>

MOVIE_LOADER::MOVIE_LOADER(OSD *osd, config_t *cfg) : QObject(NULL)
{
	p_osd = osd;
	p_cfg = cfg;

	video_mutex = new QMutex(QMutex::Recursive);
	snd_write_lock = new QMutex(QMutex::Recursive);
	
	frame_count = 0;
	frame_rate = 29.97;
	video_frame_count = 0;
	audio_frame_count = 0;
	
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
#if defined(USE_LIBAV)
	fmt_ctx = NULL;
	video_dec_ctx = NULL;
	audio_dec_ctx = NULL;
	sws_context = NULL;
	swr_context = NULL;
	video_stream = NULL;
	audio_stream = NULL;
	for(int i = 0; i < 4; i++) video_dst_data[i] = NULL;
	video_dst_bufsize = 0;
	video_stream_idx = -1;
	audio_stream_idx = -1;
	frame = NULL;
	tmp_frame = NULL;
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
            AGAR_DebugLog(AGAR_LOG_INFO, "Error decoding video frame (%s)\n", str_buf);
            return ret;
        }

        if (*got_frame) {
            if (frame->width != src_width || frame->height != src_height ||
                frame->format != pix_fmt) {
                /* To handle this change, one could call av_image_alloc again and
                 * decode the following frames into another rawvideo file. */
                AGAR_DebugLog(AGAR_LOG_INFO, "Error: Width, height and pixel format have to be "
                        "constant in a rawvideo file, but the width, height or "
                        "pixel format of the input video changed:\n"
                        "old: width = %d, height = %d, format = %s\n"
                        "new: width = %d, height = %d, format = %s\n",
                        src_width, src_height, av_get_pix_fmt_name(pix_fmt),
                        frame->width, frame->height,
                        av_get_pix_fmt_name(frame->format));
                return -1;
            }
			if((old_dst_width != dst_width) || (old_dst_height != dst_height)) { // You sould allocate on opening.
				if(tmp_frame != NULL) {
					video_mutex->lock();
					av_frame_free(&tmp_frame);
					tmp_frame = av_frame_alloc();
					if (tmp_frame == NULL) {
						AGAR_DebugLog(AGAR_LOG_INFO, "MOVIE_LOADER: Could not re-allocate video frame\n");
						video_mutex->unlock();
						return -1;
					}
					tmp_frame->format = AV_PIX_FMT_BGRA;
					tmp_frame->width = dst_width;
					tmp_frame->height = dst_height;
					ret = av_frame_get_buffer(tmp_frame, 32); //*
					if(ret < 0) {
						av_frame_free(&tmp_frame);
						AGAR_DebugLog(AGAR_LOG_INFO, "MOVIE_LOADER: Could not re-allocate output buffer\n");
						video_mutex->unlock();
						return -1;
					}
					video_mutex->unlock();
				}
			}
			old_dst_width = dst_width;
			old_dst_height = dst_height;
			
			char str_buf2[AV_TS_MAX_STRING_SIZE] = {0};
			av_ts_make_time_string(str_buf2, frame->pts, &video_dec_ctx->time_base);
            AGAR_DebugLog(AGAR_LOG_DEBUG, "video_frame%s n:%d coded_n:%d pts:%s\n",
						  cached ? "(cached)" : "",
						  video_frame_count++, frame->coded_picture_number,
						  str_buf2);
			
            /* copy decoded frame to destination buffer:
             * this is required since rawvideo expects non aligned data */
            //av_image_copy(video_dst_data, video_dst_linesize,
            //              (const uint8_t **)(frame->data), frame->linesize,
            //            pix_fmt, src_width, src_height);
			
			if(sws_context == NULL) {
				sws_context = sws_getContext(frame->width, frame->height,
											 AV_PIX_FMT_BGRA,
											 dst_width, dst_height,
											 pix_fmt,
											 SCALE_FLAGS, NULL, NULL, NULL);
				if (sws_context == NULL) {
					AGAR_DebugLog(AGAR_LOG_INFO,
								  "MOVIE_LOADER: Could not initialize the conversion context\n");
					return -1;
				}
				
			}
			if(tmp_frame != NULL) { // You must pull final frame (and need locking?) before emit sig_decode_frames_complete().
				video_mutex->lock();
				ret = av_frame_make_writable(tmp_frame);
				sws_scale(sws_context,
						  frame->data, frame->linesize,
						  0, dst_height, tmp_frame->data, tmp_frame->linesize);
				video_mutex->unlock();
			}
		
		} else if (pkt.stream_index == audio_stream_idx) {
			/* decode audio frame */
			ret = avcodec_decode_audio4(audio_dec_ctx, frame, got_frame, &pkt);
			if (ret < 0) {
				char str_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
				av_make_error_string(str_buf, AV_ERROR_MAX_STRING_SIZE, ret);
				AGAR_DebugLog(AGAR_LOG_INFO, "Error decoding audio frame (%s)\n", str_buf);
				return ret;
			}
			/* Some audio decoders decode only part of the packet, and have to be
			 * called again with the remainder of the packet data.
			 * Sample: fate-suite/lossless-audio/luckynight-partial.shn
			 * Also, some decoders might over-read the packet. */
			decoded = FFMIN(ret, pkt.size);
			
			if (*got_frame) {
				size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(frame->format);
				char str_buf[AV_TS_MAX_STRING_SIZE] = {0};
				av_ts_make_time_string(str_buf, frame->pts, &audio_dec_ctx->time_base);
				AGAR_DebugLog(AGAR_LOG_DEBUG,"audio_frame%s n:%d nb_samples:%d pts:%s\n",
							  cached ? "(cached)" : "",
							  audio_frame_count++, frame->nb_samples,
							  str_buf);
				
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
					px->data = (uint8_t *)malloc((long)unpadded_linesize);
					memcpy(px->data, (uint8_t *)(frame->extended_data[0]), (long)unpadded_linesize); // post tmp-buffer.
					px->unpadded_linesize = (long)unpadded_linesize;
					snd_write_lock->lock();
					sound_data_queue.enqueue(px);
					snd_write_lock->unlock();
				}
				//fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);
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
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), _filename.toLocal8Bit().constData());
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec_ctx = st->codec;
        dec = avcodec_find_decoder(dec_ctx->codec_id);
        if (!dec) {
            AGAR_DebugLog(AGAR_LOG_INFO, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Init the decoders, with or without reference counting */
        av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
            AGAR_DebugLog(AGAR_LOG_INFO, "Failed to open %s codec\n",
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

    AGAR_DebugLog(AGAR_LOG_INFO,
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
	
	frame_count = 0;
	audio_frame_count = 0;
	video_frame_count = 0;
	mod_frames = 0.0;
    /* register all formats and codecs */
    av_register_all();

    /* open input file, and allocate format context */
    if (avformat_open_input(&fmt_ctx, _filename.toLocal8Bit().constData(), NULL, NULL) < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not open source file %s\n", _filename.toLocal8Bit().constData());
        return -1;
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not find stream information\n");
        exit(1);
    }

    if (open_codec_context(&video_stream_idx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        video_stream = fmt_ctx->streams[video_stream_idx];
        video_dec_ctx = video_stream->codec;

        /* allocate image where the decoded image will be put */
        src_width = video_dec_ctx->width;
        src_height = video_dec_ctx->height;
        pix_fmt = video_dec_ctx->pix_fmt;
        ret = av_image_alloc(video_dst_data, video_dst_linesize,
                             src_width, src_height, pix_fmt, 1);
        if (ret < 0) {
            AGAR_DebugLog(AGAR_LOG_INFO, "Could not allocate raw video buffer\n");
            goto _end;
        }
        video_dst_bufsize = ret;
    }

    if (open_codec_context(&audio_stream_idx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        audio_stream = fmt_ctx->streams[audio_stream_idx];
        audio_dec_ctx = audio_stream->codec;
    }

    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, _filename.toLocal8Bit().constData(), 0);

    if (!audio_stream && !video_stream) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not find audio or video stream in the input, aborting\n");
        ret = 1;
        goto _end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto _end;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    /* read frames from the file */
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        AVPacket orig_pkt = pkt;
        do {
            ret = decode_packet(&got_frame, 0);
            if (ret < 0)
                break;
            pkt.data += ret;
            pkt.size -= ret;
        } while (pkt.size > 0);
        av_packet_unref(&orig_pkt);
    }

    /* flush cached frames */
    pkt.data = NULL;
    pkt.size = 0;

	// Re-allocate buffer;
	video_mutex->lock();
	tmp_frame = av_frame_alloc();
	if (tmp_frame == NULL) {
		AGAR_DebugLog(AGAR_LOG_INFO, "MOVIE_LOADER: Could not allocate output frame\n");
		goto _end;
	}
	tmp_frame->format = AV_PIX_FMT_BGRA;
	tmp_frame->width = dst_width;
	tmp_frame->height = dst_height;
	ret = av_frame_get_buffer(tmp_frame, 32); //*
	video_mutex->unlock();
	if(ret < 0) {
		av_frame_free(&tmp_frame);
		AGAR_DebugLog(AGAR_LOG_INFO, "MOVIE_LOADER: Could not re-allocate output buffer\n");
		goto _end;
	}

	// ToDo : Initialize SWScaler and SWresampler.
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
	if(tmp_frame != NULL) av_frame_free(&tmp_frame);
	video_mutex->unlock();
	
	video_dec_ctx = NULL;
	audio_dec_ctx = NULL;
	sws_context = NULL;
	swr_context = NULL;
	
	now_playing = false;
	now_pausing = false;
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
	return frame_count;
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
	double d_frames = (double)frames * (p_osd->vm_frame_rate() / frame_rate);
	mod_frames = mod_frames + d_frames;
	real_frames = (int)mod_frames;
	mod_frames = fmod(mod_frames, 1.0);

	if(width > 0) dst_width = width;
	if(height > 0) dst_height = height;
	
	if(real_frames <= 0) {
		do_dequeue_audio();
		//if(frames < 0) emit sig_decoding_error(MOVIE_LOADER_ILL_FRAME_NUMBER);
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
			if(tmp_frame != NULL) {
				uint32_t q;
				video_mutex->lock();
				for(int yy = 0; yy < dst_height; yy++) { 
					q = (uint32_t *)(&(tmp_frame->data[0][yy * tmp_frame->linesize[0]]));
					if((q != NULL) && (dst_width != 0)) {
						memset(q, 0x00, dst_width * sizeof(uint32_t));
					}
				}
				video_mutex->unlock();
			}
		}
		return;
	}
    for(int i = 0; i < real_frames; i++) {
        decode_packet(&got_frame, 1);
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
	
	p_osd->screen_mutex->lock();
	if(tmp_frame != NULL) {
		video_mutex->lock();
		for(int yy = 0; yy < dst_height; yy++) {
			p = (uint32_t *)(&(tmp_frame->data[0][yy * tmp_frame->linesize[0]]));
			q = (uint32_t *)(p_osd->get_vm_screen_buffer(yy));
			if((p == NULL) || (q == NULL)) break;
			for(xx = dst_width; xx > 7;) {
				cacheline[0] = p[0];
				cacheline[1] = p[1];
				cacheline[2] = p[2];
				cacheline[3] = p[3];
				cacheline[4] = p[4];
				cacheline[5] = p[5];
				cacheline[6] = p[6];
				cacheline[7] = p[7];
				
				q[0] = cacheline[0];
				q[1] = cacheline[1];
				q[2] = cacheline[2];
				q[3] = cacheline[3];
				q[4] = cacheline[4];
				q[5] = cacheline[5];
				q[6] = cacheline[6];
				q[7] = cacheline[7];
				
				p += 8;
				q += 8;
				xx -= 8;
				if(xx < 8) break;
			}
			for(; xx > 0; xx--) {
				*q++ = *p++;
			}
		}
		video_mutex->unlock();
	}
	p_osd->screen_mutex->unlock();
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
				if(tmpq->data != NULL) {
					if((tmpq->unpadded_linesize != 0) && (dptr < audio_data_size)) {
						if(tmpdata != NULL) {
							memcpy(&tmpdata[dptr], tmpq->data, tmpq->unpadded_linesize);
						}
						dptr += tmpq->unpadded_linesize;
					}
					free(tmpq->data);
				}
				free(tmpq);
			}
		}
	}
	snd_write_lock->unlock();
	if((tmpdata != NULL) && (dptr != 0)) {
		emit sig_send_audio_frame(tmpdata,dptr); // Please free arg1 after calling sound_callback of vm.
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
