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


void *MOVIE_SAVER::alloc_audio_frame(int  _sample_fmt,
										uint64_t channel_layout,
										int sample_rate, int nb_samples)
{
#if defined(USE_LIBAV)
	enum AVSampleFormat sample_fmt = (enum AVSampleFormat)_sample_fmt;
    AVFrame *frame = av_frame_alloc();
    int ret;
    if (!frame) {
        AGAR_DebugLog(AGAR_LOG_DEBUG, "Error allocating an audio frame\n");
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
            AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Saver: Error allocating an audio buffer");
            return NULL;
        }
    }
    return (void *)frame;
#else
	return NULL;
#endif
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

bool MOVIE_SAVER::audio_resample(void *_frame)
{
#if defined(USE_LIBAV)
	int ret;
	int64_t audio_dst_nb_samples;
	//AVFrame *frame = (AVFrame *)_frame;
	AVFrame *frame = audio_frame_data;
	/* convert samples from native format to destination codec format, using the resampler */
	/* compute destination number of samples */
	audio_dst_nb_samples = av_rescale_rnd(swr_get_delay(audio_swr_context, audio_codec_context->sample_rate)
										  + frame->nb_samples,
										  audio_codec_context->sample_rate,
										  audio_codec_context->sample_rate,
										  AV_ROUND_UP);
	if(audio_dst_nb_samples == frame->nb_samples) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Saver: Error while re-sampling sound:");
		//ret = av_frame_make_writable(frame);
		//return true;
		return false;
	}
	/* when we pass a frame to the encoder, it may keep a reference to it
	 * internally;
	 * make sure we do not overwrite it here
	 */
	ret = av_frame_make_writable(frame);
	if (ret < 0) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Audio: Error while converting\n");
		return false;
	}
	/* convert to destination format */
	ret = swr_convert(audio_swr_context,
					  frame->data, audio_dst_nb_samples,
					  (const uint8_t **)(frame->data), frame->nb_samples);
	if (ret < 0) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Saver: Error while converting\n");
		return false;
	}
	frame->pts = av_rescale_q(audio_samples_count,
							  (AVRational){1, audio_codec_context->sample_rate},
							  audio_codec_context->time_base);
	audio_samples_count += audio_dst_nb_samples;
	return true;
#else
	return false;
#endif
}

bool MOVIE_SAVER::setup_audio_resampler(void)
{
#if defined(USE_LIBAV)
	int ret;
	audio_swr_context =  swr_alloc();
	if (audio_swr_context == NULL) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Saver/Audio: Could not allocate resampler context\n");
		return false;
	}
	av_opt_set_int       (audio_swr_context, "in_channel_count",   audio_codec_context->channels,       0);
	av_opt_set_int       (audio_swr_context, "in_sample_rate",     audio_codec_context->sample_rate,    0);
	av_opt_set_sample_fmt(audio_swr_context, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int       (audio_swr_context, "out_channel_count",  audio_codec_context->channels,       0);
	av_opt_set_int       (audio_swr_context, "out_sample_rate",    audio_codec_context->sample_rate,    0);
	av_opt_set_sample_fmt(audio_swr_context, "out_sample_fmt",     audio_codec_context->sample_fmt,     0);
	/* initialize the resampling context */
	if ((ret = swr_init(audio_swr_context)) < 0) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Saver/Audio: Failed to initialize the resampling context\n");
		return false;;
	}
	// Context
#endif
	return true;
}

void MOVIE_SAVER::add_stream_audio(void **_codec, int _codec_id)
{
    AVCodecContext *c;
	AVCodec **codec = (AVCodec **)_codec;
	enum AVCodecID codec_id = (enum AVCodecID)_codec_id;
    int i;

    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        fprintf(stderr, "Could not find encoder for '%s'\n",
                avcodec_get_name(codec_id));
        exit(1);
    }

    audio_stream = avformat_new_stream(output_context, *codec);
    if (!audio_stream) {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }
    audio_stream->id = output_context->nb_streams-1;
    c = audio_stream->codec;

    {
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
        audio_stream->time_base = (AVRational){ 1, c->sample_rate };
	}
    /* Some formats want stream headers to be separate. */
    if (output_context->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}
