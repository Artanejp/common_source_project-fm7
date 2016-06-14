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

/**************************************************************/
/* audio output */

//static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
//								  uint64_t channel_layout,
//								  int sample_rate, int nb_samples)
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

	if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
		nb_samples = 10000;
	else
		nb_samples = c->frame_size;

	ost->frame	 = (AVFrame *)alloc_audio_frame((uint64_t)c->sample_fmt, c->channel_layout,
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
	av_opt_set_int	   (ost->swr_ctx, "in_channel_count",   c->channels,	   0);
	av_opt_set_int	   (ost->swr_ctx, "in_sample_rate",	 c->sample_rate,	0);
	av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt",	  AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int	   (ost->swr_ctx, "out_channel_count",  c->channels,	   0);
	av_opt_set_int	   (ost->swr_ctx, "out_sample_rate",	c->sample_rate,	0);
	av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt",	 c->sample_fmt,	 0);
	
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
			audio_remain -= sizeof(int16_t);
			if(audio_remain <= 0) {
				audio_offset = 0;
				break;
			}
			//if(audio_offset >= audio_size) {
			//	//audio_offset = (audio_offset / ost->st->codec->channels) * ost->st->codec->channels;
			//	audio_offset = 0;
			//	break;
			//}
		}
	}
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
		//printf("RATE=%d %d\n", c->sample_rate, audio_sample_rate);	
		dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, c->sample_rate) + frame->nb_samples,
										c->sample_rate, c->sample_rate,  AV_ROUND_UP);
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
