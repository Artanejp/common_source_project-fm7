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
	#include "libavutil/common.h"
	#include "libavutil/intreadwrite.h"
}
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
		ret = av_frame_make_writable(frame);
		return true;
		//return false;
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
	av_opt_set_int	   (audio_swr_context, "in_channel_count",   audio_codec_context->channels,	   0);
	av_opt_set_int	   (audio_swr_context, "in_sample_rate",	 audio_codec_context->sample_rate,	0);
	av_opt_set_sample_fmt(audio_swr_context, "in_sample_fmt",	  AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int	   (audio_swr_context, "out_channel_count",  audio_codec_context->channels,	   0);
	av_opt_set_int	   (audio_swr_context, "out_sample_rate",	audio_codec_context->sample_rate,	0);
	av_opt_set_sample_fmt(audio_swr_context, "out_sample_fmt",	 audio_codec_context->sample_fmt,	 0);
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
		c->bit_rate	= 64000;
		c->sample_rate = 44100;
		if ((*codec)->supported_samplerates) {
			c->sample_rate = (*codec)->supported_samplerates[0];
			for (i = 0; (*codec)->supported_samplerates[i]; i++) {
				if ((*codec)->supported_samplerates[i] == 44100)
					c->sample_rate = 44100;
			}
		}
		c->channels		= av_get_channel_layout_nb_channels(c->channel_layout);
		c->channel_layout = AV_CH_LAYOUT_STEREO;
		if ((*codec)->channel_layouts) {
			c->channel_layout = (*codec)->channel_layouts[0];
			for (i = 0; (*codec)->channel_layouts[i]; i++) {
				if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
					c->channel_layout = AV_CH_LAYOUT_STEREO;
			}
		}
		c->channels		= av_get_channel_layout_nb_channels(c->channel_layout);
		audio_stream->time_base = (AVRational){ 1, c->sample_rate };
	}
	/* Some formats want stream headers to be separate. */
	if (output_context->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}

void *MOVIE_SAVER::get_one_audio_frame(bool *continue_flag)
{
	int16_t *optr;
#if defined(USE_LIBAV)
	AVFrame *frame;
	frame = audio_frame_data;
	if(av_compare_ts(audio_next_pts, audio_stream->codec->time_base,
					 10.0, (AVRational){ 1, 1 }) >= 0) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "*");
		*continue_flag = false;
		//audio_continue = false;
		return (void *)NULL;
	}
	optr = (int16_t *)frame->data;
	for(int j = audio_frame_offset; j < frame->nb_samples; j++) {
		if(audio_remain <= 0) {
			*continue_flag = true;
			audio_offset = 0;
			return (void *)frame;
		}
		if(audio_offset >= audio_size) {
			audio_offset = 0;
			*continue_flag = true;
			return (void *)frame;
		}
		for(int k = 0; k < audio_stream->codec->channels; k++) {
			//optr[(audio_frame_offset * audio_stream->codec->channels)+ k] = ptr[audio_offset + k];
		}
		audio_offset += audio_stream->codec->channels;
		audio_remain -= audio_stream->codec->channels;
		audio_frame_offset++;
	}
	*continue_flag = false; // GO
	return (void *)frame;
#else
	return NULL;
#endif
}

// Got from ffmpeg 3.0.2 .
// static void write_frame(AVFormatContext *s, AVPacket *pkt, OutputStream *ost)
bool MOVIE_SAVER::write_frame(void *_avformatcontext_s, void *_pkt, void *_ost)
{
#if defined(USE_LIBAV)
	OutputStream *ost = (OutputStream *)_ost;
	AVPacket *pkt = _pkt;
	AVFormatContext *s = _avformatcontext_s;
    AVBitStreamFilterContext *bsfc = ost->bitstream_filters;
    AVCodecContext          *avctx = ost->encoding_needed ? ost->enc_ctx : ost->st->codec;
    int ret;

	if (!ost->st->codec->extradata_size && ost->enc_ctx->extradata_size) {
		ost->st->codec->extradata = av_mallocz(ost->enc_ctx->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
		if (ost->st->codec->extradata) {
			memcpy(ost->st->codec->extradata, ost->enc_ctx->extradata, ost->enc_ctx->extradata_size);
			ost->st->codec->extradata_size = ost->enc_ctx->extradata_size;
		}
	}
	
	if ((avctx->codec_type == AVMEDIA_TYPE_VIDEO && video_sync_method == VSYNC_DROP) ||
		(avctx->codec_type == AVMEDIA_TYPE_AUDIO && audio_sync_method < 0))
		pkt->pts = pkt->dts = AV_NOPTS_VALUE;
	
	/*
	 * Audio encoders may split the packets --  #frames in != #packets out.
	 * But there is no reordering, so we can limit the number of output packets
	 * by simply dropping them here.
	 * Counting encoded video frames needs to be done separately because of
	 * reordering, see do_video_out()
	 */
	if (!(avctx->codec_type == AVMEDIA_TYPE_VIDEO && avctx->codec)) {
		if (ost->frame_number >= ost->max_frames) {
			av_packet_unref(pkt);
			return false; // Overflow
		}
		ost->frame_number++;
	}
	if (avctx->codec_type == AVMEDIA_TYPE_VIDEO) {
		int i;
		uint8_t *sd = av_packet_get_side_data(pkt, AV_PKT_DATA_QUALITY_STATS,
											  NULL);
		ost->quality = sd ? AV_RL32(sd) : -1;
		ost->pict_type = sd ? sd[4] : AV_PICTURE_TYPE_NONE;

		for (i = 0; i<FF_ARRAY_ELEMS(ost->error); i++) {
			if (sd && i < sd[5])
				ost->error[i] = AV_RL64(sd + 8 + 8*i);
			else
				ost->error[i] = -1;
		}

		if (ost->frame_rate.num && ost->is_cfr) {
			if (pkt->duration > 0)
				AGAR_DebugLog(AGAR_LOG_WARN, "Movie/Saver: Overriding packet duration by frame rate, this should not happen\n");
			pkt->duration = av_rescale_q(1, av_inv_q(ost->frame_rate),
										 ost->st->time_base);
		}
	}

	if (bsfc)
		av_packet_split_side_data(pkt);

	if ((ret = av_apply_bitstream_filters(avctx, pkt, bsfc)) < 0) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Saver: BitstreamFilters failed", ret);
		return false;
		//if (exit_on_error)
		//	exit_program(1);
	}

	if (!(s->oformat->flags & AVFMT_NOTIMESTAMPS)) {
		if (pkt->dts != AV_NOPTS_VALUE &&
			pkt->pts != AV_NOPTS_VALUE &&
			pkt->dts > pkt->pts) {
			AGAR_DebugLog(AGAR_LOG_WARN, "Movie/Saver: Invalid DTS: %"PRId64" PTS: %"PRId64" in output stream %d:%d, replacing by guess\n",
						  pkt->dts, pkt->pts,
						  ost->file_index, ost->st->index);
			pkt->pts =
			pkt->dts = pkt->pts + pkt->dts + ost->last_mux_dts + 1
				- FFMIN3(pkt->pts, pkt->dts, ost->last_mux_dts + 1)
				- FFMAX3(pkt->pts, pkt->dts, ost->last_mux_dts + 1);
		}
		if(
		 (avctx->codec_type == AVMEDIA_TYPE_AUDIO || avctx->codec_type == AVMEDIA_TYPE_VIDEO) &&
		 pkt->dts != AV_NOPTS_VALUE &&
		 ost->last_mux_dts != AV_NOPTS_VALUE) {
			int64_t max = ost->last_mux_dts + !(s->oformat->flags & AVFMT_TS_NONSTRICT);
			if (pkt->dts < max) {
				int loglevel = max - pkt->dts > 2 || avctx->codec_type == AVMEDIA_TYPE_VIDEO ? AGAR_LOG_WARN : AGAR_LOG_DEBUG;
				AGAR_DebugLog(loglevel, "Movie/Saver: Non-monotonous DTS in output stream "
							  "%d:%d; previous: %"PRId64", current: %"PRId64"; ",
							  ost->file_index, ost->st->index, ost->last_mux_dts, pkt->dts);
				//if (exit_on_error) {
				//	av_log(NULL, AV_LOG_FATAL, "aborting.\n");
				//	exit_program(1);
				//}
				AGAR_DebugLog(loglevel, "Movie/Saver: changing to %"PRId64". This may result "
							  "in incorrect timestamps in the output file.\n",
							  max);
				if(pkt->pts >= pkt->dts)
					pkt->pts = FFMAX(pkt->pts, max);
				pkt->dts = max;
			}
		}
	}
	ost->last_mux_dts = pkt->dts;

	ost->data_size += pkt->size;
	ost->packets_written++;

	pkt->stream_index = ost->index;

	if (debug_timestamp) {
		AGAR_DebugLog(AGAR_LOG_INFO, "Movie/Saver: muxer <- type:%s "
					  "pkt_pts:%s pkt_pts_time:%s pkt_dts:%s pkt_dts_time:%s size:%d\n",
					  (const char *)av_get_media_type_string(ost->enc_ctx->codec_type),
					  (const char *)av_ts2str(pkt->pts),
					  (const char *)av_ts2timestr(pkt->pts, &ost->st->time_base),
					  (const char *)av_ts2str(pkt->dts),
					  (const char *)av_ts2timestr(pkt->dts, &ost->st->time_base),
					  pkt->size
			);
	}

	ret = av_interleaved_write_frame(s, pkt);
	if (ret < 0) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Saver: av_interleaved_write_frame()", ret);
		//main_return_code = 1;
		//close_all_output_streams(ost, MUXER_FINISHED | ENCODER_FINISHED, ENCODER_FINISHED);
		do_close();
	}
	av_packet_unref(pkt);
#endif
	return true; // Bool
}


//static void do_audio_out(AVFormatContext *s, OutputStream *ost, AVFrame *frame)
bool MOVIE_SAVER::do_audio_out(void *_s, void *_ost, void *_frame)
{
#if defined(USE_LIBAV)
	AVFormatContext *s = (AVFormatContext *)_s;
	OutputStream *ost = (OutputStream *)_ost,
	AVFrame *frame = (AVFrame *)_frame;
    AVCodecContext *enc = ost->enc_ctx;
    AVPacket pkt;
    int got_packet = 0;

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    if (!check_recording_time(ost)){
		//AGAR_DebugLog(AGAR_LOG_INFO, "Movie/Saver: AUDIO SEND: GOT Null packet.");
        return false;
	}
    if (frame->pts == AV_NOPTS_VALUE || audio_sync_method < 0)
        frame->pts = ost->sync_opts;
    ost->sync_opts = frame->pts + frame->nb_samples;
    ost->samples_encoded += frame->nb_samples;
    ost->frames_encoded++;

    if(pkt.size || !pkt.data) {
		AGAR_DebugLog(AGAR_LOG_INFO, "Movie/Saver: AUDIO SEND: GOT Null packet.");
		do_close();
		return false;
	}
    //update_benchmark(NULL);
    if (debug_timestamp) {
		AGAR_DebugLog(AGAR_LOG_INFO, "encoder <- type:audio "
					  "frame_pts:%s frame_pts_time:%s time_base:%d/%d\n",
					  av_ts2str(frame->pts), av_ts2timestr(frame->pts, &enc->time_base),
					  enc->time_base.num, enc->time_base.den);
    }

    if (avcodec_encode_audio2(enc, &pkt, frame, &got_packet) < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Audio encoding failed (avcodec_encode_audio2)\n");
		return false;
        //exit_program(1);
    }
//update_benchmark("encode_audio %d.%d", ost->file_index, ost->index);

    if (got_packet) {
        av_packet_rescale_ts(&pkt, enc->time_base, ost->st->time_base);

        if (debug_timestamp) {
            AGAR_DebugLog(AGAR_LOG_INFO, "Movie/Saver: encoder -> type:audio "
						  "pkt_pts:%s pkt_pts_time:%s pkt_dts:%s pkt_dts_time:%s\n",
						  (const char *)av_ts2str(pkt.pts),
						  (const char *)av_ts2timestr(pkt.pts, &ost->st->time_base),
						  (const char *)av_ts2str(pkt.dts),
						  (const char *)av_ts2timestr(pkt.dts, &ost->st->time_base));
        }

        write_frame(s, &pkt, ost);
    }
#endif
	return true;
}


/**
 * Get and encode new output from any of the filtergraphs, without causing
 * activity.
 *
 * @return  0 for success, <0 for severe errors
 */
int MOVIE_SAVER::reap_filters(int flush)
{
    AVFrame *filtered_frame = NULL;
    int i;

    /* Reap all buffers present in the buffer sinks */
    for (i = 0; i < nb_output_streams; i++) {
        OutputStream *ost = output_streams[i];
        OutputFile    *of = output_files[ost->file_index];
        AVFilterContext *filter;
        AVCodecContext *enc = ost->enc_ctx;
        int ret = 0;

        if (!ost->filter)
            continue;
        filter = ost->filter->filter;

        if (!ost->filtered_frame && !(ost->filtered_frame = av_frame_alloc())) {
            return AVERROR(ENOMEM);
        }
        filtered_frame = ost->filtered_frame;

        while (1) {
            double float_pts = AV_NOPTS_VALUE; // this is identical to filtered_frame.pts but with higher precision
            ret = av_buffersink_get_frame_flags(filter, filtered_frame,
                                               AV_BUFFERSINK_FLAG_NO_REQUEST);
            if (ret < 0) {
                if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                    av_log(NULL, AV_LOG_WARNING,
                           "Error in av_buffersink_get_frame_flags(): %s\n", av_err2str(ret));
                } else if (flush && ret == AVERROR_EOF) {
                    //if (filter->inputs[0]->type == AVMEDIA_TYPE_VIDEO)
                    //    do_video_out(of->ctx, ost, NULL, AV_NOPTS_VALUE);
                }
                break;
            }
            if (ost->finished) {
                av_frame_unref(filtered_frame);
                continue;
            }
            if (filtered_frame->pts != AV_NOPTS_VALUE) {
                int64_t start_time = (of->start_time == AV_NOPTS_VALUE) ? 0 : of->start_time;
                AVRational tb = enc->time_base;
                int extra_bits = av_clip(29 - av_log2(tb.den), 0, 16);

                tb.den <<= extra_bits;
                float_pts =
                    av_rescale_q(filtered_frame->pts, filter->inputs[0]->time_base, tb) -
                    av_rescale_q(start_time, AV_TIME_BASE_Q, tb);
                float_pts /= 1 << extra_bits;
                // avoid exact midoints to reduce the chance of rounding differences, this can be removed in case the fps code is changed to work with integers
                float_pts += FFSIGN(float_pts) * 1.0 / (1<<17);

                filtered_frame->pts =
                    av_rescale_q(filtered_frame->pts, filter->inputs[0]->time_base, enc->time_base) -
                    av_rescale_q(start_time, AV_TIME_BASE_Q, enc->time_base);
            }
            //if (ost->source_index >= 0)
            //    *filtered_frame= *input_streams[ost->source_index]->decoded_frame; //for me_threshold

            switch (filter->inputs[0]->type) {
            case AVMEDIA_TYPE_VIDEO:
                if (!ost->frame_aspect_ratio.num)
                    enc->sample_aspect_ratio = filtered_frame->sample_aspect_ratio;

                if (debug_timestamp) {
                    AGAR_DebugLog(AGAR_LOG_INFO, "filter -> pts:%s pts_time:%s exact:%f time_base:%d/%d\n",
								  (const char *)av_ts2str(filtered_frame->pts), av_ts2timestr(filtered_frame->pts, &enc->time_base),
								  float_pts,
								  enc->time_base.num, enc->time_base.den);
                }

                //do_video_out(of->ctx, ost, filtered_frame, float_pts);
                break;
            case AVMEDIA_TYPE_AUDIO:
                if (!(enc->codec->capabilities & AV_CODEC_CAP_PARAM_CHANGE) &&
                    enc->channels != av_frame_get_channels(filtered_frame)) {
                    AGAR_DebugLog(AGAR_LOG_INFO,
								  "Audio filter graph output is not normalized and encoder does not support parameter changes\n");
                    break;
                }
                do_audio_out(of->ctx, ost, filtered_frame);
                break;
            default:
                // TODO support subtitle filters
                //av_assert0(0);
				return 0;
            }
            av_frame_unref(filtered_frame);
        }
    }
    return 0;
}

//void flush_encoders(void)
void MOVIE_SAVER::flush_encoders(void)
{
    int i, ret;
#if defined(USE_LIBAV)
    for (i = 0; i < nb_output_streams; i++) {
        OutputStream   *ost = output_streams[i];
        AVCodecContext *enc = ost->enc_ctx;
        AVFormatContext *os = output_files[ost->file_index]->ctx;
        int stop_encoding = 0;

        if (!ost->encoding_needed)
            continue;

        if (enc->codec_type == AVMEDIA_TYPE_AUDIO && enc->frame_size <= 1)
            continue;
#if FF_API_LAVF_FMT_RAWPICTURE
        if (enc->codec_type == AVMEDIA_TYPE_VIDEO && (os->oformat->flags & AVFMT_RAWPICTURE) && enc->codec->id == AV_CODEC_ID_RAWVIDEO)
            continue;
#endif

        for (;;) {
            int (*encode)(AVCodecContext*, AVPacket*, const AVFrame*, int*) = NULL;
            const char *desc;

            switch (enc->codec_type) {
            case AVMEDIA_TYPE_AUDIO:
                encode = avcodec_encode_audio2;
                desc   = "audio";
                break;
            case AVMEDIA_TYPE_VIDEO:
                encode = avcodec_encode_video2;
                desc   = "video";
                break;
            default:
                stop_encoding = 1;
            }

            if (encode) {
                AVPacket pkt;
                int pkt_size;
                int got_packet;
                av_init_packet(&pkt);
                pkt.data = NULL;
                pkt.size = 0;

                //update_benchmark(NULL);
                ret = encode(enc, &pkt, NULL, &got_packet);
                AGAR_DebugLog(AGAR_LOG_DEBUG, "flush_%s %d.%d",
							  (const char *)desc,
							  ost->file_index,
							  ost->index);
                if (ret < 0) {
                    AGAR_DebugLog(AGAR_LOG_INFO, "%s encoding failed: %s\n",
								  desc,
								  (const char *)av_err2str(ret));
                    do_close();
					return;
                }
                if (ost->logfile && enc->stats_out) {
                    fprintf(ost->logfile, "%s", enc->stats_out);
                }
                if (!got_packet) {
                    stop_encoding = 1;
                    break;
                }
                if (ost->finished & MUXER_FINISHED) {
                    av_packet_unref(&pkt);
                    continue;
                }
                av_packet_rescale_ts(&pkt, enc->time_base, ost->st->time_base);
                pkt_size = pkt.size;
                write_frame(os, &pkt, ost);
                if (ost->enc_ctx->codec_type == AVMEDIA_TYPE_VIDEO && vstats_filename) {
                    do_video_stats(ost, pkt_size);
                }
            }

            if (stop_encoding)
                break;
        }
    }
}


/**
 * Select the output stream to process.
 *
 * @return  selected output stream, or NULL if none available
 */
void *MOVIE_SAVER::*choose_output(void)
{
#if defined(USE_LIBAV)
    int i;
    int64_t opts_min = INT64_MAX;
    OutputStream *ost_min = NULL;

    for (i = 0; i < nb_output_streams; i++) {
        OutputStream *ost = output_streams[i];
        int64_t opts = ost->st->cur_dts == AV_NOPTS_VALUE ? INT64_MIN :
                       av_rescale_q(ost->st->cur_dts, ost->st->time_base,
                                    AV_TIME_BASE_Q);
        if (ost->st->cur_dts == AV_NOPTS_VALUE)
            AGAR_DebugLog(AGAR_LOG_DEBUG, "cur_dts is invalid (this is harmless if it occurs once at the start per stream)\n");

        if (!ost->finished && opts < opts_min) {
            opts_min = opts;
            ost_min  = ost->unavailable ? NULL : ost;
        }
    }
    return (void *)ost_min;
#else
	return (void *)NULL;
#endif	
}

int MOVIE_SAVER::got_eagain(void)
{
    int i;
    for (i = 0; i < nb_output_streams; i++)
        if (output_streams[i]->unavailable)
            return 1;
    return 0;
}

void MOVIE_SAVER::reset_eagain(void)
{
    int i;
    for (i = 0; i < nb_input_files; i++)
        input_files[i]->eagain = 0;
    for (i = 0; i < nb_output_streams; i++)
        output_streams[i]->unavailable = 0;
}


/**
 * Run a single step of transcoding.
 *
 * @return  0 for success, <0 for error
 */
int MOVIE_SAVER::transcode_step(void)
{
#if defined(USE_LIBAV)
    OutputStream *ost;
    InputStream  *ist;
    int ret;

    ost = (OutputStream *)choose_output();
    if (!ost) {
        if (got_eagain()) {
            reset_eagain();
            this->usleep(10000);
            return 0;
        }
        AGAR_DebugLog(AGAR_LOG_INFO, "No more inputs to read from, finishing.\n");
        return AVERROR_EOF;
    }

    //if (ost->filter) {
    //    if ((ret = transcode_from_filter(ost->filter->graph, &ist)) < 0)
    //        return ret;
    //    if (!ist)
    //        return 0;
    //} else {
        //av_assert0(ost->source_index >= 0);
        //ist = input_streams[ost->source_index];
    //}

    ret = process_input(ist->file_index);
    if (ret == AVERROR(EAGAIN)) {
        if (input_files[ist->file_index]->eagain)
            ost->unavailable = 1;
        return 0;
    }

    if (ret < 0)
        return ret == AVERROR_EOF ? 0 : ret;

    return reap_filters(0);
}


