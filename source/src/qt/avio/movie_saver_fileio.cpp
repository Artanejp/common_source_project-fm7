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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
extern "C" {
	#include <libavutil/avassert.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/opt.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/timestamp.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
}

QString MOVIE_SAVER::ts2str(int64_t ts)
{
	char buffer[AV_TS_MAX_STRING_SIZE + 16];
	memset(buffer, 0x00, sizeof(buffer));
	return QString::fromLocal8Bit(av_ts_make_string(buffer, ts));
}		   

QString MOVIE_SAVER::ts2timestr(int64_t ts, void *timebase)
{
	char buffer[AV_TS_MAX_STRING_SIZE + 16];
	AVRational *tb = (AVRational *)timebase;
	memset(buffer, 0x00, sizeof(buffer));
	return QString::fromLocal8Bit(av_ts_make_time_string(buffer, ts, tb));
}		   

QString MOVIE_SAVER::err2str(int errnum)
{
	char buffer[AV_TS_MAX_STRING_SIZE + 16];
	memset(buffer, 0x00, sizeof(buffer));
	return QString::fromLocal8Bit(av_make_error_string(buffer, sizeof(buffer), errnum));
}		   

//void MOVIE_SAVER::log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
void MOVIE_SAVER::log_packet(const void *_fmt_ctx, const void *_pkt)
{
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
}

//int MOVIE_SAVER::write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
int MOVIE_SAVER::write_frame(void *_fmt_ctx, const void *_time_base, void *_st, void *_pkt)
{
	AVFormatContext *fmt_ctx = (AVFormatContext *)_fmt_ctx;
	const AVRational *time_base = (const AVRational *)_time_base;
	AVStream *st = (AVStream *)_st;
	AVPacket *pkt = (AVPacket *)_pkt;
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    /* Write the compressed frame to the media file. */
    //log_packet(fmt_ctx, pkt);
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

/* Add an output stream. */
//static void add_stream(OutputStream *ost, AVFormatContext *oc,
//                       AVCodec **codec,
//                       enum AVCodecID codec_id)
bool MOVIE_SAVER::add_stream(void *_ost, void *_oc,
                       void **_codec,
                       uint64_t _codec_id)
{
    AVCodecContext *c;
    int i;

	OutputStream *ost = (OutputStream *)_ost;
	AVFormatContext *oc = (AVFormatContext *)_oc;
	AVCodec **codec = (AVCodec **)_codec;
	enum AVCodecID codec_id = (enum AVCodecID)_codec_id;
    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not find encoder for '%s'\n",
                (const char *)avcodec_get_name(codec_id));
        return false;
    }

    ost->st = avformat_new_stream(oc, *codec);
    if (!ost->st) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not allocate stream\n");
        return false;
    }
    ost->st->id = oc->nb_streams-1;
    c = ost->st->codec;

    switch ((*codec)->type) {
    case AVMEDIA_TYPE_AUDIO:
        c->sample_fmt  = (*codec)->sample_fmts ?
            (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        c->bit_rate    = 64000;
        c->sample_rate = 44100;
        if ((*codec)->supported_samplerates) {
            c->sample_rate = (*codec)->supported_samplerates[0];
            for (i = 0; (*codec)->supported_samplerates[i]; i++) {
                if ((*codec)->supported_samplerates[i] == 44100)
                    c->sample_rate = 44100;
            }
        }
        c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
        c->channel_layout = AV_CH_LAYOUT_STEREO;
        if ((*codec)->channel_layouts) {
            c->channel_layout = (*codec)->channel_layouts[0];
            for (i = 0; (*codec)->channel_layouts[i]; i++) {
                if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                    c->channel_layout = AV_CH_LAYOUT_STEREO;
            }
        }
        c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
        ost->st->time_base = (AVRational){ 1, c->sample_rate };
        break;

    case AVMEDIA_TYPE_VIDEO:
        c->codec_id = codec_id;

        c->bit_rate = 400000;
        /* Resolution must be a multiple of two. */
        c->width    = 352;
        c->height   = 288;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
        ost->st->time_base = (AVRational){ 1, STREAM_FRAME_RATE };
        c->time_base       = ost->st->time_base;

        c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt       = STREAM_PIX_FMT;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B frames */
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
        }
    break;

    default:
        break;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	return true;
}

/**************************************************************/
/* audio output */

//static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
//                                  uint64_t channel_layout,
//                                  int sample_rate, int nb_samples)
void *MOVIE_SAVER::alloc_audio_frame(uint64_t _sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;
	enum AVSampleFormat sample_fmt = (enum AVSampleFormat)_sample_fmt; 
    if (!frame) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            AGAR_DebugLog(AGAR_LOG_INFO, "Error allocating an audio buffer\n");
            return NULL;
        }
    }

    return (void *)frame;
}

//static void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
bool MOVIE_SAVER::open_audio(void)
{
 	AVDictionary *opt_arg = raw_options_list;
	OutputStream *ost = &audio_st;
	AVCodec *codec = audio_codec;
	AVFormatContext *oc = output_context;
	AVCodecContext *c;
    int nb_samples;
    int ret;
    AVDictionary *opt = NULL;

    c = ost->st->codec;

    /* open it */
    av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not open audio codec: %s\n", err2str(ret).toLocal8Bit().constData());
		return false;
    }

    /* init signal generator */
    ost->t     = 0;
    ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;

    ost->frame     = (AVFrame *)alloc_audio_frame((uint64_t)c->sample_fmt, c->channel_layout,
                                       c->sample_rate, nb_samples);
    ost->tmp_frame = (AVFrame *)alloc_audio_frame((uint64_t)AV_SAMPLE_FMT_S16, c->channel_layout,
                                       c->sample_rate, nb_samples);

    /* create resampler context */
	ost->swr_ctx = swr_alloc();
	if (!ost->swr_ctx) {
		AGAR_DebugLog(AGAR_LOG_INFO, "Could not allocate resampler context\n");
		return false;
	}
	
	/* set options */
	av_opt_set_int       (ost->swr_ctx, "in_channel_count",   c->channels,       0);
	av_opt_set_int       (ost->swr_ctx, "in_sample_rate",     c->sample_rate,    0);
	av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int       (ost->swr_ctx, "out_channel_count",  c->channels,       0);
	av_opt_set_int       (ost->swr_ctx, "out_sample_rate",    c->sample_rate,    0);
	av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt",     c->sample_fmt,     0);
	
	/* initialize the resampling context */
	if ((ret = swr_init(ost->swr_ctx)) < 0) {
		AGAR_DebugLog(AGAR_LOG_INFO, "Failed to initialize the resampling context\n");
		return false;
	}
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Open to write audio : Success.");
	return true;
}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */
//static AVFrame *get_audio_frame(OutputStream *ost)
void *MOVIE_SAVER::get_audio_frame()
{
	OutputStream *ost = &audio_st;
    AVFrame *frame = ost->tmp_frame;
    int j, i, v;
    int16_t *q = (int16_t*)frame->data[0];
	int offset = 0;
    /* check if we want to generate more frames */
    if (av_compare_ts(ost->next_pts, ost->st->codec->time_base,
                      STREAM_DURATION, (AVRational){ 1, 1 }) >= 0)
        return NULL;
	for(j = 0; j < frame->nb_samples; j++) {
		if(audio_remain <= 0) {
			while(audio_data_queue.isEmpty()) {
				if(!bRunThread) return NULL;
				if(!recording) return NULL;
				msleep(1);
			}
			dequeue_audio(audio_frame_buf);
			audio_remain = audio_size;
			audio_offset = 0;
		}
        for (i = 0; i < ost->st->codec->channels; i++) {
            q[offset++] = audio_frame_buf[audio_offset++];
			if(audio_offset >= audio_size) {
				offset = (offset / ost->st->codec->channels) * ost->st->codec->channels;
				audio_remain = -1;
				break;
			}
		}
	}
//    for (j = 0; j <frame->nb_samples; j++) {
//        v = (int)(sin(ost->t) * 10000);
//        for (i = 0; i < ost->st->codec->channels; i++)
//            *q++ = v;
//        ost->t     += ost->tincr;
//        ost->tincr += ost->tincr2;
//    }

    frame->pts = ost->next_pts;
    ost->next_pts  += frame->nb_samples;

    return frame;
}

/*
 * encode one audio frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
//static int write_audio_frame(AVFormatContext *oc, OutputStream *ost)
int MOVIE_SAVER::write_audio_frame()
{
	AVFormatContext *oc = output_context;
	OutputStream *ost = &audio_st;
    AVCodecContext *c;
    AVPacket pkt = { 0 }; // data and size must be 0;
    AVFrame *frame;
    int ret;
    int got_packet;
    int dst_nb_samples;

    av_init_packet(&pkt);
    c = ost->st->codec;

    frame = (AVFrame *)get_audio_frame();

    if (frame) {
        /* convert samples from native format to destination codec format, using the resampler */
            /* compute destination number of samples */
            dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, c->sample_rate) + frame->nb_samples,
                                            c->sample_rate, c->sample_rate, AV_ROUND_UP);
            av_assert0(dst_nb_samples == frame->nb_samples);

        /* when we pass a frame to the encoder, it may keep a reference to it
         * internally;
         * make sure we do not overwrite it here
         */
        ret = av_frame_make_writable(ost->frame);
        if (ret < 0) {
			AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE/Saver: Audio: Fail to make writable frame.");
            return -1;
		}
		/* convert to destination format */
		ret = swr_convert(ost->swr_ctx,
                              ost->frame->data, dst_nb_samples,
                              (const uint8_t **)frame->data, frame->nb_samples);
            if (ret < 0) {
                AGAR_DebugLog(AGAR_LOG_INFO, "Error while converting\n");
                return -1;
            }
            frame = ost->frame;

        frame->pts = av_rescale_q(ost->samples_count, (AVRational){1, c->sample_rate}, c->time_base);
        ost->samples_count += dst_nb_samples;
		totalAudioFrame++;
    }

    ret = avcodec_encode_audio2(c, &pkt, frame, &got_packet);
    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Error encoding audio frame: %s\n", err2str(ret).toLocal8Bit().constData());
        return -1;
    }

    if (got_packet) {
		
        ret = this->write_frame((void *)oc, (const void *)&c->time_base, (void *)ost->st, (void *)&pkt);
        if (ret < 0) {
            AGAR_DebugLog(AGAR_LOG_INFO, "Error while writing audio frame: %s\n",
					err2str(ret).toLocal8Bit().constData());
            return -1;
        }
    }

    return (frame || got_packet) ? 0 : 1;
}

/**************************************************************/
/* video output */

//static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
void *MOVIE_SAVER::alloc_picture(uint64_t _pix_fmt, int width, int height)
{
	enum AVPixelFormat pix_fmt = (enum AVPixelFormat)_pix_fmt;
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return (void *)NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not allocate frame data.\n");
        return (void *)NULL;
    }

    return (void *)picture;
}

//void MOVIE_SAVER::open_video(OutputStream *_ost, AVDictionary *_opt_arg)
bool MOVIE_SAVER::open_video()
{
    int ret;
	AVDictionary *opt_arg = raw_options_list;
	OutputStream *ost = &video_st;
	AVCodec *codec = video_codec;
	AVFormatContext *oc = output_context;
    AVCodecContext *c = ost->st->codec;
    AVDictionary *opt = NULL;

    av_dict_copy(&opt, opt_arg, 0);

    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not open video codec: %s\n", err2str(ret).toLocal8Bit().constData());
		return false;
    }

    /* allocate and init a re-usable frame */
    ost->frame = (AVFrame *)alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not allocate video frame\n");
        return false;
    }

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
	old_width = old_height = -1;
//    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
//        ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
//        if (!ost->tmp_frame) {
//            fprintf(stderr, "Could not allocate temporary picture\n");
//            return false;
//        }
//    }
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Open to write video : Success.");
	return true;
}


//static AVFrame *get_video_frame(OutputStream *ost)
void *MOVIE_SAVER::get_video_frame(void)
{
	OutputStream *ost = &video_st;
    AVCodecContext *c = ost->st->codec;

    /* check if we want to generate more frames */
    if (av_compare_ts(ost->next_pts, ost->st->codec->time_base,
                      STREAM_DURATION, (AVRational){ 1, 1 }) >= 0)
        return (void *)NULL;

    //if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        /* as we only generate a YUV420P picture, we must convert it
         * to the codec pixel format if needed */
	if((old_width != _width) || (old_height != _height)) {
		if(ost->sws_ctx != NULL) { 
			sws_freeContext(ost->sws_ctx);
			ost->sws_ctx = NULL;
		}
	}
	old_height = _height;
	old_width = _width;
	if (!ost->sws_ctx) {
		ost->sws_ctx = sws_getContext(c->width, c->height,
									  AV_PIX_FMT_YUV420P,
									  _width, _height,
									  AV_PIX_FMT_RGBA,
									  SCALE_FLAGS, NULL, NULL, NULL);
		if (!ost->sws_ctx) {
			AGAR_DebugLog(AGAR_LOG_INFO,
					"Could not initialize the conversion context\n");
			return (void *)NULL;
		}
	}
	const int in_linesize[1] = { 4 * _width };
	//fill_yuv_image(ost->tmp_frame, ost->next_pts, c->width, c->height);
	sws_scale(ost->sws_ctx,
			  (const uint8_t * const *)video_frame_buf, in_linesize,
			  0, _height, ost->frame->data, ost->frame->linesize);
	//} else {
	//fill_yuv_image(ost->frame, ost->next_pts, c->width, c->height);
	//}
	
    ost->frame->pts = ost->next_pts++;
	
    return (void *)(ost->frame);
}

/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
//static int write_video_frame(AVFormatContext *oc, OutputStream *ost)
int MOVIE_SAVER::write_video_frame()
{
    int ret;
	AVFormatContext *oc = output_context;
	OutputStream *ost = &video_st;
    AVCodecContext *c;
    AVFrame *frame;
    int got_packet = 0;
    AVPacket pkt = { 0 };

    c = ost->st->codec;

    frame = (AVFrame *)get_video_frame();
	//if(frame == NULL) {
	//	//AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE/Saver: Failed to get frame @Video.");
	//	return 0;
	//}
    av_init_packet(&pkt);

    /* encode the image */
    ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Error encoding video frame: %s\n", err2str(ret).toLocal8Bit().constData());
        return -1;
    }
	totalDstFrame++;

    if (got_packet) {
        ret = this->write_frame((void *)oc, (const void *)&c->time_base, (void *)ost->st, (void *)&pkt);
		// ret = write_frame(oc, &c->time_base, ost->st, &pkt);
    } else {
        ret = 0;
    }

    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Error while writing video frame: %s\n", err2str(ret).toLocal8Bit().constData());
        return -1;
    }
	
    return (frame || got_packet) ? 0 : 1;
}

//void MOVIE_SAVER::close_stream(AVFormatContext *oc, OutputStream *ost)
void MOVIE_SAVER::close_stream(void *_oc, void *_ost)
{
	AVFormatContext *oc = (AVFormatContext *)_oc;
	OutputStream *ost = (OutputStream *)_ost;
    avcodec_close(ost->st->codec);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}



bool MOVIE_SAVER::do_open(QString filename, int fps)
{
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    //AVCodec *audio_codec, *video_codec;
    int ret;
    have_video = 0, have_audio = 0;
    int encode_video = 0, encode_audio = 0;
    raw_options_list = NULL;
	do_close();
	
	do_set_record_fps(fps);
	_filename = filename;
	
    video_st = { 0 };
	audio_st = { 0 };
    /* Initialize libavcodec, and register all codecs and formats. */
    av_register_all();

	for(int i = 0; i < encode_options.size(); i++) {
		if(encode_opt_keys.size() <= i) break;
        av_dict_set(&raw_options_list, encode_opt_keys.takeAt(i).toUtf8().constData(),
					encode_options.takeAt(i).toUtf8().constData(), 0);
    }

    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, NULL, NULL, _filename.toLocal8Bit().constData());
    if (!oc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&oc, NULL, "mpeg", _filename.toLocal8Bit().constData());
    }
    if (!oc)
        return false;

    fmt = oc->oformat;

    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        if(!add_stream((void *)&video_st, (void *)oc, (void **)&video_codec, (uint64_t)fmt->video_codec)) goto _err_final;
        have_video = 1;
        encode_video = 1;
    }
    if (fmt->audio_codec != AV_CODEC_ID_NONE) {
        if(!add_stream((void *)&audio_st, (void *)oc, (void **)&audio_codec, (uint64_t)fmt->audio_codec)) goto _err_final;
        have_audio = 1;
        encode_audio = 1;
    }
	output_context = oc;
	stream_format  = fmt;
    /* Now that all the parameters are set, we can open the audio and
     * video codecs and allocate the necessary encode buffers. */
    if (have_video)
        open_video();

    if (have_audio)
        open_audio();
	
    av_dump_format(oc, 0, _filename.toLocal8Bit().constData(), 1);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, filename.toLocal8Bit().constData(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            AGAR_DebugLog(AGAR_LOG_INFO, "Could not open '%s': %s\n", filename.toLocal8Bit().constData(),
                    err2str(ret).toLocal8Bit().constData());
            goto _err_final;
        } else {
            AGAR_DebugLog(AGAR_LOG_INFO, "Success to Open file: '%s\n", filename.toLocal8Bit().constData());
		}			
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(oc, &raw_options_list);
    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Error occurred when opening output file: %s\n",
				err2str(ret).toLocal8Bit().constData());
        goto _err_final;
    }

	recording = true;
	return true;
_err_final:
    avformat_free_context(oc);
	oc = NULL;
	output_context = NULL;
	stream_format  = NULL;
	return false;
}
