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
	#if (LIBAVCODEC_VERSION_MAJOR > 56)
	#define AVCODEC_UPPER_V56
	#endif
#endif
/**************************************************************/
/* audio output */

//static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
//								  uint64_t channel_layout,
//								  int sample_rate, int nb_samples)
void *MOVIE_SAVER::alloc_audio_frame(void *ctx,  int nb_samples)
{
	if((ctx == nullptr) || (nb_samples <= 0)) return nullptr;
#if defined(USE_LIBAV)
	AVFrame *frame = av_frame_alloc();
	AVCodecContext* p_ctx = (AVCodecContext*)ctx;
	int ret;
	enum AVSampleFormat sample_fmt = p_ctx->sample_fmt; 
	if (!frame) {
		out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error allocating an audio frame\n");
		return nullptr;
	}

	frame->format = p_ctx->sample_fmt;
    if(av_channel_layout_copy(&(frame->ch_layout), &(p_ctx->ch_layout)) < 0) {
		av_frame_free(&frame);
		return nullptr;
	}
	frame->sample_rate = p_ctx->sample_rate;
	frame->nb_samples = nb_samples;
	if(av_frame_get_buffer(frame, 0) < 0) {
			out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error allocating an audio buffer\n");
			av_frame_free(&frame);
			return nullptr;
		}
	}

	return (void *)frame;
#else
	return nullptr;
#endif
}

//static void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
bool MOVIE_SAVER::open_audio(void)
{
#if defined(USE_LIBAV)
 	AVDictionary *opt_arg = raw_options_list;
	OutputStream *ost = &audio_st;
	AVCodec *codec = audio_codec;
	AVCodecContext *c = NULL;
	int nb_samples;
	int ret;
	AVDictionary *opt = NULL;

#ifdef AVCODEC_UPPER_V56
	c = ost->context;
#else
	c = ost->st->codec;
#endif
	/* open it */
	av_dict_copy(&opt, opt_arg, 0);
	ret = avcodec_open2(c, codec, &opt);
	av_dict_free(&opt);
	if (ret < 0) {
#ifdef AVCODEC_UPPER_V56
		avcodec_free_context(&(ost->context));
#endif
		out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not open audio codec: %s\n", err2str(ret).toLocal8Bit().constData());
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
	/* set options */
	ost->swr_ctx = swr_alloc();
	if (!ost->swr_ctx) {
#ifdef AVCODEC_UPPER_V56
		avcodec_free_context(&(ost->context));
#endif
		out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not allocate resampler context\n");
		return false;
	}
	av_opt_set_int	   (ost->swr_ctx, "in_channel_count",   c->channels,	   0);
	av_opt_set_int	   (ost->swr_ctx, "in_sample_rate",	 audio_sample_rate,	0);
	av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt",	  AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int	   (ost->swr_ctx, "out_channel_count",  c->channels,	   0);
	av_opt_set_int	   (ost->swr_ctx, "out_sample_rate",	c->sample_rate,	0);
	av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt",	 c->sample_fmt,	 0);
	
	/* initialize the resampling context */
	if ((ret = swr_init(ost->swr_ctx)) < 0) {
#ifdef AVCODEC_UPPER_V56 
		avcodec_free_context(&(ost->context));
#endif
		out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Failed to initialize the resampling context\n");
		return false;
	}
#ifdef AVCODEC_UPPER_V56
	ret = avcodec_parameters_from_context(ost->st->codecpar, c);
	if (ret < 0) {
		out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "Could not copy the stream parameters\n");
		return false;
	}
#endif
	out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "Open to write audio : Success.");
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
	AVFrame *frame = audio_st.tmp_frame;
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
		int chs;
	#ifdef AVCODEC_UPPER_V56
		chs = audio_st.context->channels;
	#else
		chs = audio_st.st->codec->channels;
	#endif
		for (i = 0; i < chs; i++) {
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
	#ifdef AVCODEC_UPPER_V56
	frame->pts = av_rescale_rnd(audio_st.next_pts, audio_st.context->sample_rate, audio_sample_rate, AV_ROUND_UP);
	#else
	frame->pts = av_rescale_rnd(audio_st.next_pts, audio_st.st->codec->sample_rate, audio_sample_rate, AV_ROUND_UP);
	#endif
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
	std::shared_ptr<AVFormatContext> oc = output_context;
	__UNLIKELY_IF(oc.get() == nullptr) {
		return 0;
	}
	OutputStream *ost = &audio_st;
	AVCodecContext *c;
	AVPacket *pkt = NULL; // data and size must be 0;
	AVFrame *frame_src;
	AVFrame *frame_dst;
	int ret;
	int got_packet;
	int dst_nb_samples;

	#ifdef AVCODEC_UPPER_V56
	c = ost->context;
	#else
	c = ost->st->codec;
	#endif
#ifdef AVCODEC_UPPER_V56
	pkt = av_packet_alloc();
	if(pkt == nullptr) {
		return 0;
	}
#else
	pkt = malloc(sizeof(AVPacket));
	if(pkt == nullptr) {
		return 0;
	}
	av_init_packet(pkt);
#endif	
	frame_src = (AVFrame *)get_audio_frame();
	//if(req_close) return 1;
	if (frame_src == NULL) {
		goto __ret_zero;
	}
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
			out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER, "Audio: Fail to make writable frame.");
			goto __error_1;
		}
		/* convert to destination format */
		ret = swr_convert(ost->swr_ctx,
						  ost->frame->data, dst_nb_samples,
						  (const uint8_t **)frame_src->data, frame_src->nb_samples);
		if (ret < 0) {
			out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error while converting\n");
			goto __error_1;
		}

		frame_dst = ost->frame;
		frame_dst->nb_samples = dst_nb_samples;
		
		frame_dst->pts = av_rescale_q(ost->samples_count, (AVRational){1, c->sample_rate}, c->time_base);
		ost->samples_count += dst_nb_samples;
		ost->next_pts += dst_nb_samples;
		totalAudioFrame++;
	}
#ifdef AVCODEC_UPPER_V56
	got_packet = 0;
	{
		ret = avcodec_send_frame(c, frame_dst);
		if(ret < 0) {
			goto __error_1;
		}
		while (ret >= 0) {
			ret = avcodec_receive_packet(c, pkt);
			if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				
				goto __return_normal;
			} else if (ret < 0) {
				out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error while writing audio frame: %s\n",
									err2str(ret).toLocal8Bit().constData());
				goto __error_1;
			}
			got_packet = 1;
			pkt->stream_index = 1;
			int ret2 = this->write_frame((const void *)&c->time_base, (void *)ost->st, (void *)pkt);
			if (ret2 < 0) {
				out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error while writing audio frame: %s\n", err2str(ret2).toLocal8Bit().constData());
				goto __error_1;
			}
			av_packet_unref(pkt);
		}
	}
#else
	ret = avcodec_encode_audio2(c, pkt, frame_dst, &got_packet);
	if (ret < 0) {
		out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error encoding audio frame: %s\n", err2str(ret).toLocal8Bit().constData());
		goto __error_1;
	}

	if (got_packet) {
		ret = this->write_frame((const void *)&c->time_base, (void *)ost->st, (void *)pkt);
		if (ret < 0) {
			out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error while writing audio frame: %s\n",
					err2str(ret).toLocal8Bit().constData());
			goto __error_1;
		}
		//if(frame_dst) {
		//	char buf[256];
		//	memset(buf, 0x00, sizeof(buf));
		//	char *s = av_ts_make_time_string(buf, frame_dst->pts, &c->time_base);
		//	AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Write audio to file. pts=%s", s);
		//}
	}
#endif
__return_normal:
#ifdef AVCODEC_UPPER_V56
		av_packet_free(&pkt);
#else
		free(pkt);
#endif
	return (frame_dst || got_packet) ? 0 : 1;
__error_1:
#ifdef AVCODEC_UPPER_V56
		av_packet_free(&pkt);
#else
		free(pkt);
#endif
		return -1;
__ret_zero:
#ifdef AVCODEC_UPPER_V56
		av_packet_free(&pkt);
#else
		free(pkt);
#endif
		return 0;
__eof:
#ifdef AVCODEC_UPPER_V56
		av_packet_free(&pkt);
#else
		free(pkt);
#endif
		return 1;

#else
	return 1;
#endif
}

#if defined(USE_LIBAV)
void MOVIE_SAVER::setup_audio(AVCodecContext *codec_context, AVCodec **codec, enum AVCodecID codec_id)
{
	AVCodecContext *c = codec_context;

	__UNLIKELY_IF((codec == nullptr) || (c == nullptr)) {
		return;
	}
	// Check support samplerate
	std::list<int> samplerates;	
	if((*codec)->supported_samplerates != nullptr) {
		int* p = (*codec)->supported_samplerates;
		for(i = 0; p[i] != 0; i++) {
			samplerates.insert(p[i]);
		}
		samplerates.sort();
	}
	int newrate = -1;
	for(auto _v = samplerates.begin(); _v != samplerates.end(); ++_v) {
		if((*_v) == audio_sample_rate) {
			newrate = audio_sample_rate;
			break;
		}
		if((*_v) > audio_sample_rate) {
			break;
		}
		newrate = (*_v);
	}
	if(newrate <= 0) {
		return;
	}
	int cutoff;
	if(newrate < 32000) {
		cutoff = (newrate * 3) / 5;
	} else {
		cutoff = newrate / 2;
	}
	
	#ifndef AVCODEC_UPPER_V56
	c->sample_fmt  = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
	c->bit_rate	= audio_bit_rate;
	c->sample_rate = newrate;
	c->codec_id = codec_id;
	if(codec_id == AV_CODEC_ID_AAC) {
		c->strict_std_compliance = -2; // For internal AAC
		c->global_quality = 100;
	}
	if(codec_id == AV_CODEC_ID_MP3) {
		c->compression_level = 9; // Quality
		// ABR/CBR/VBR
	}
	c->cutoff = cutoff;
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
	#else
	int __type = CSP_AVIO_BASIC::csp_avio_get_codec_type((_TCHAR*)avcodec_get_name(codec_id));
	struct AVCodecParameters params;
	if(avcodec_parameters_from_context(&params, c) < 0) {
		return;
	}
	params.codec_id = codec_id;
	params.bit_rate = audio_bit_rate;
	params.sample_rate = newrate;
	params.frame_size = 1024; // ToDo: Modify value.
	switch(__type) {
	case CSP_AVIO_BASIC::TYPE_MP3:
		params.format = AV_SAMPLE_FMT_S16P;
		// ABR/CBR/VBR
		break;
	case CSP_AVIO_BASIC::TYPE_AAC:
	case CSP_AVIO_BASIC::TYPE_AAC_LATM:
		params.format = AV_SAMPLE_FMT_FLTP;
		break;
	case CSP_AVIO_BASIC::TYPE_FLAC:
		params.format = AV_SAMPLE_FMT_S16; // OK?
		// ToDo
		break;
	case CSP_AVIO_BASIC::TYPE_PCM_S16LE:
		params.format = AV_SAMPLE_FMT_S16;
		params.bits_per_raw_sample = 16;
		// ToDo
		break;
	case CSP_AVIO_BASIC::TYPE_PCM_S24LE:
		params.format = AV_SAMPLE_FMT_S32;
		params.bits_per_raw_sample = 24;
		// ToDo
		break;
	case CSP_AVIO_BASIC::TYPE_VORBIS:
		params.format = AV_SAMPLE_FMT_FLTP;
		// ToDo
		break;
	default:
		break;
	}
	if(avcodec_parameters_to_context(c, &params) < 0) {
		return;
	}
	switch(__type) {
	case CSP_AVIO_BASIC::TYPE_MP3:
		c->compression_level = 9; // Quality: Will modify?
		break;
	case CSP_AVIO_BASIC::TYPE_AAC:
	case CSP_AVIO_BASIC::TYPE_AAC_LATM:
		c->global_quality = 100;
		break;
	case CSP_AVIO_BASIC::TYPE_FLAC:
		c->compression_level = 8; // Maximum compress. Will modify?
		break;
	case CSP_AVIO_BASIC::TYPE_VORBIS:
		break;
	default:
		break;
	}
	c->cutoff = cutoff;
	AVChannelLayout a_layout;
	av_channel_layout_default(&a_layout, 2);
	if(av_channel_layout_copy(&(c->channel_layout), &a_layout) < 0) {
		return;
	}
	
	#endif
}
#endif
