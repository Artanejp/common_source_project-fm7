/*
 * Common Source Code Project for Qt : movie saver.
 * (C) 2016 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  License: GPLv2
 *  History: May 27, 2016 : Initial. This refer from avidemux 2.5.6 .
 */
#include <QDateTime>
#include "movie_saver.h"
#include "../osd.h"
#include "csp_logger.h"

#if defined(USE_LIBAV)
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
#endif
/**************************************************************/
/* audio output */

//static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
//								  uint64_t channel_layout,
//								  int sample_rate, int nb_samples)
void *MOVIE_SAVER::alloc_audio_frame(uint64_t _sample_fmt,
								  uint64_t channel_layout,
								  int sample_rate, int nb_samples)
{
#if defined(USE_LIBAV)
	AVFrame *frame = av_frame_alloc();
	int ret;
	enum AVSampleFormat sample_fmt = (enum AVSampleFormat)_sample_fmt; 
	if (!frame) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error allocating an audio frame\n");
		exit(1);
	}

	frame->format = sample_fmt;
	frame->channel_layout = channel_layout;
	frame->sample_rate = sample_rate;
	frame->nb_samples = nb_samples;

	if (nb_samples) {
		ret = av_frame_get_buffer(frame, 0);
		if (ret < 0) {
			csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error allocating an audio buffer\n");
			return NULL;
		}
	}

	return (void *)frame;
#else
	return (void *)NULL;
#endif
}

//static void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
bool MOVIE_SAVER::open_audio(void)
{
#if defined(USE_LIBAV)
 	AVDictionary *opt_arg = raw_options_list;
	OutputStream *ost = &audio_st;
	AVCodec *codec = audio_codec;
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
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not open audio codec: %s\n", err2str(ret).toLocal8Bit().constData());
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
#if 1
	/* set options */
	ost->swr_ctx = swr_alloc();
	if (!ost->swr_ctx) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not allocate resampler context\n");
		return false;
	}
	av_opt_set_int	   (ost->swr_ctx, "in_channel_count",   c->channels,	   0);
	av_opt_set_int	   (ost->swr_ctx, "in_sample_rate",	 audio_sample_rate,	0);
	av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt",	  AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int	   (ost->swr_ctx, "out_channel_count",  c->channels,	   0);
	av_opt_set_int	   (ost->swr_ctx, "out_sample_rate",	c->sample_rate,	0);
	av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt",	 c->sample_fmt,	 0);
	
#else
	ost->swr_ctx = swr_alloc_set_opts(NULL,
									  AV_CH_LAYOUT_STEREO, c->sample_fmt, c->sample_rate,
									  AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, audio_sample_rate,
									  0, NULL);
	if (ost->swr_ctx == NULL) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Failed to initialize the resampling context\n");
		return false;
	}
#endif	
	/* initialize the resampling context */
	if ((ret = swr_init(ost->swr_ctx)) < 0) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Failed to initialize the resampling context\n");
		return false;
	}
	csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "Open to write audio : Success.");
	return true;
#else
	return true;
#endif
}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */
//static AVFrame *get_audio_frame(OutputStream *ost)
void *MOVIE_SAVER::get_audio_frame()
{
#if defined(USE_LIBAV)
	OutputStream *ost = &audio_st;
	AVFrame *frame = ost->tmp_frame;
	int j, i;
	int16_t *q = (int16_t*)frame->data[0];
	int offset = 0;
	
	for(j = 0; j < frame->nb_samples; j++) {
		if(audio_remain <= 0) {
			while(audio_data_queue.isEmpty()) {
				if(!bRunThread) return NULL;
				if(!recording) return NULL;
				if(req_close) return NULL;
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
	frame->pts = av_rescale_rnd(ost->next_pts, audio_st.st->codec->sample_rate, audio_sample_rate, AV_ROUND_UP);
	//frame->pts = ost->next_pts;
	//ost->next_pts += frame->nb_samples;

	return frame;
#else
	return (void *)NULL;
#endif
}

/*
 * encode one audio frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
//static int write_audio_frame(AVFormatContext *oc, OutputStream *ost)
int MOVIE_SAVER::write_audio_frame()
{
#if defined(USE_LIBAV)
	AVFormatContext *oc = output_context;
	OutputStream *ost = &audio_st;
	AVCodecContext *c;
	AVPacket pkt = { 0 }; // data and size must be 0;
	AVFrame *frame_src;
	AVFrame *frame_dst;
	int ret;
	int got_packet;
	int dst_nb_samples;

	av_init_packet(&pkt);
	c = ost->st->codec;

	frame_src = (AVFrame *)get_audio_frame();
	//if(req_close) return 1;
	if (frame_src == NULL) return 0;
	{
		/* convert samples from native format to destination codec format, using the resampler */
		/* compute destination number of samples */
		dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, audio_sample_rate) + frame_src->nb_samples,
										c->sample_rate,  audio_sample_rate, AV_ROUND_UP);
		//dst_nb_samples = av_rescale_rnd(frame_src->nb_samples,
		//								c->sample_rate,  audio_sample_rate, AV_ROUND_UP);

		//printf("IN %d Hz %d Samples OUT %d Hz %d Samples\n",
		//	   audio_sample_rate, frame_src->nb_samples,
		//	   c->sample_rate, dst_nb_samples);
		/* when we pass a frame to the encoder, it may keep a reference to it
		 * internally;
		 * make sure we do not overwrite it here
		 */
		ret = av_frame_make_writable(ost->frame);
		if (ret < 0) {
			csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "Audio: Fail to make writable frame.");
			return -1;
		}
		/* convert to destination format */
		ret = swr_convert(ost->swr_ctx,
						  ost->frame->data, dst_nb_samples,
						  (const uint8_t **)frame_src->data, frame_src->nb_samples);
		if (ret < 0) {
			csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error while converting\n");
			return -1;
		}

		frame_dst = ost->frame;
		frame_dst->nb_samples = dst_nb_samples;
		
		frame_dst->pts = av_rescale_q(ost->samples_count, (AVRational){1, c->sample_rate}, c->time_base);
		ost->samples_count += dst_nb_samples;
		ost->next_pts += dst_nb_samples;
		totalAudioFrame++;
	}

	ret = avcodec_encode_audio2(c, &pkt, frame_dst, &got_packet);
	if (ret < 0) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error encoding audio frame: %s\n", err2str(ret).toLocal8Bit().constData());
		return -1;
	}

	if (got_packet) {
		ret = this->write_frame((void *)oc, (const void *)&c->time_base, (void *)ost->st, (void *)&pkt);
		if (ret < 0) {
			csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error while writing audio frame: %s\n",
					err2str(ret).toLocal8Bit().constData());
			return -1;
		}
		//if(frame_dst) {
		//	char buf[256];
		//	memset(buf, 0x00, sizeof(buf));
		//	char *s = av_ts_make_time_string(buf, frame_dst->pts, &c->time_base);
		//	AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Write audio to file. pts=%s", s);
		//}
	}
	return (frame_dst || got_packet) ? 0 : 1;
#else
	return 1;
#endif
}

void MOVIE_SAVER::setup_audio(void *_codec_context, void **_codec)
{
#if defined(USE_LIBAV)
	AVCodecContext *c = (AVCodecContext *)_codec_context;
	AVCodec **codec = (AVCodec **)_codec;
	
	c->sample_fmt  = (*codec)->sample_fmts ?
		(*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
	c->bit_rate	= audio_bit_rate;
	c->sample_rate = audio_sample_rate;
	if(c->codec_id == AUDIO_CODEC_AAC) {
		c->strict_std_compliance = -2; // For internal AAC
		c->global_quality = 100;
	}
	if(c->codec_id == AUDIO_CODEC_MP3) {
		c->compression_level = 9; // Quality
		// ABR/CBR/VBR
	}
	if(audio_sample_rate < 32000) {
		c->cutoff = (audio_sample_rate * 3) / 5;
	} else {
		c->cutoff = audio_sample_rate / 2;
	}		
	if ((*codec)->supported_samplerates) {
		c->sample_rate = (*codec)->supported_samplerates[0];
		for (int i = 0; (*codec)->supported_samplerates[i]; i++) {
			if ((*codec)->supported_samplerates[i] == audio_sample_rate)
				c->sample_rate = audio_sample_rate;
		}
	}
	c->channels		= av_get_channel_layout_nb_channels(c->channel_layout);
	c->channel_layout = AV_CH_LAYOUT_STEREO;
	if ((*codec)->channel_layouts) {
		c->channel_layout = (*codec)->channel_layouts[0];
		for (int i = 0; (*codec)->channel_layouts[i]; i++) {
			if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
				c->channel_layout = AV_CH_LAYOUT_STEREO;
		}
	}
	c->channels		= av_get_channel_layout_nb_channels(c->channel_layout);
#endif
}
