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
//					   AVCodec **codec,
//					   enum AVCodecID codec_id)
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
		c->bit_rate	= audio_bit_rate;
		c->sample_rate = audio_sample_rate;
		c->strict_std_compliance = -2; // For internal AAC
		if ((*codec)->supported_samplerates) {
			c->sample_rate = (*codec)->supported_samplerates[0];
			for (i = 0; (*codec)->supported_samplerates[i]; i++) {
				if ((*codec)->supported_samplerates[i] == audio_sample_rate)
					c->sample_rate = audio_sample_rate;
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
		ost->st->time_base = (AVRational){ 1, c->sample_rate };
		break;

	case AVMEDIA_TYPE_VIDEO:
		c->codec_id = codec_id;
		c->bit_rate = video_bit_rate;
		// See:
		// https://libav.org/avconv.html#Video-Options
		/* Resolution must be a multiple of two. */
		c->width	= video_geometry.width();
		c->height   = video_geometry.height();
		c->qmin	 = config.video_minq;
		c->qmax	 = config.video_maxq;
		c->thread_count	 = video_encode_threads;
		c->max_b_frames	 = config.video_bframes;
		
		c->b_quant_offset = 2;
		c->temporal_cplx_masking = 0.1;
		c->spatial_cplx_masking = 0.15;
		c->scenechange_threshold = 35;
		c->noise_reduction = 0;
		c->bidir_refine = 2;
		c->refs = 5;
		c->chromaoffset = 2;
		c->max_qdiff = 6;
		//c->b_frame_strategy = config.video_b_adapt;
		c->me_subpel_quality = config.video_subme;
		c->i_quant_offset = 1.2;
		c->i_quant_factor = 1.5;
		c->trellis = 2;
		c->mb_lmin = 4;
		c->mb_lmax = 18;
		c->bidir_refine = 2;
		c->keyint_min = rec_fps / 5;
		c->gop_size = rec_fps * 5;
		c->b_sensitivity = 55;
		c->scenechange_threshold = 50;
		
		/* timebase: This is the fundamental unit of time (in seconds) in terms
		 * of which frame timestamps are represented. For fixed-fps content,
		 * timebase should be 1/framerate and timestamp increments should be
		 * identical to 1. */
		ost->st->time_base = (AVRational){ 1, rec_fps};
		c->time_base	   = ost->st->time_base;

		//c->gop_size	  = rec_fps; /* emit one intra frame every one second */
		c->pix_fmt	   = STREAM_PIX_FMT;
		if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
			/* just for testing, we also add B frames */
			c->max_b_frames = 2;
			c->gop_size  = rec_fps; /* emit one intra frame every one second */
		}
		if (c->codec_id == AV_CODEC_ID_MPEG4) {
			if(c->qmin > c->qmax) {
				int tmp;
				tmp = c->qmin;
				c->qmin = c->qmax;
				c->qmax = tmp;
			}
			if(c->qmin <= 0) c->qmin = 1;
			if(c->qmax <= 0) c->qmax = 1;
			c->gop_size  = rec_fps; /* emit one intra frame every one second */
		}
		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
			/* Needed to avoid using macroblocks in which some coeffs overflow.
			 * This does not happen with normal video, it just happens here as
			 * the motion of the chroma plane does not match the luma plane. */
			c->mb_decision = 2;
		}
		if (c->codec_id == AV_CODEC_ID_H264) {
			/* Needed to avoid using macroblocks in which some coeffs overflow.
			 * This does not happen with normal video, it just happens here as
			 * the motion of the chroma plane does not match the luma plane. */
			c->mb_decision = 2;
			c->me_method = ME_UMH;
			c->profile=FF_PROFILE_H264_HIGH;
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



//void MOVIE_SAVER::close_stream(AVFormatContext *oc, OutputStream *ost)
void MOVIE_SAVER::close_stream(void *_oc, void *_ost)
{
	AVFormatContext *oc = (AVFormatContext *)_oc;
	OutputStream *ost = (OutputStream *)_ost;
	avcodec_close(ost->st->codec);
	while(avcodec_is_open(ost->st->codec) != 0) {}
	av_frame_free(&ost->frame);
	av_frame_free(&ost->tmp_frame);
	sws_freeContext(ost->sws_ctx);
	swr_free(&ost->swr_ctx);
}



bool MOVIE_SAVER::do_open(QString filename, int _fps, int _sample_rate)
{
	AVOutputFormat *fmt;
	AVFormatContext *oc;
	//AVCodec *audio_codec, *video_codec;
	int ret;
	have_video = 0, have_audio = 0;
	int encode_video = 0, encode_audio = 0;
	if(recording) do_close_main();
	raw_options_list = NULL;

	do_set_record_fps(_fps);
	audio_sample_rate = _sample_rate;
	_filename = filename;
	
	video_st = { 0 };
	audio_st = { 0 };
	/* Initialize libavcodec, and register all codecs and formats. */
	av_register_all();


	{
		QString value;
		do_clear_options_list();
		do_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("mpeg4"));
		do_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("aac"));
		//do_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("theora"));
		//do_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("vorbis"));
		
		value.setNum(config.video_minq);
		do_add_option(QString::fromUtf8("qmin"), value);
		value.setNum(config.video_maxq);
		do_add_option(QString::fromUtf8("qmax"), value);
		
		value.setNum(config.video_bframes);
		do_add_option(QString::fromUtf8("bframes"), value);
		
		value.setNum(config.video_b_adapt);
		do_add_option(QString::fromUtf8("b_adapt"), value);
		
		value.setNum(config.video_subme);
		do_add_option(QString::fromUtf8("subme"), value);

		value.setNum(config.video_subme);
		do_add_option(QString::fromUtf8("subme"), value);
		
		video_encode_threads = config.video_threads;
		video_geometry = QSize(config.video_width, config.video_height);
		video_bit_rate = config.video_bitrate * 1000;
		audio_bit_rate = config.audio_bitrate * 1000;
		
	}

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
	fmt->video_codec = AV_CODEC_ID_MPEG4;
	//fmt->video_codec = AV_CODEC_ID_H264;
	fmt->audio_codec = AV_CODEC_ID_AAC;
	
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
	recording = false;
	avformat_free_context(oc);
	oc = NULL;
	output_context = NULL;
	stream_format  = NULL;
	return false;
}

void MOVIE_SAVER::do_close()
{
	req_close = true;
}

void MOVIE_SAVER::do_close_main()
{
	recording = false;
	req_close = false;
#if defined(USE_LIBAV)
	int ret, i;
	OutputStream *ost;
	int got_packet;
	int64_t total_packets_written = 0;
	if(output_context != NULL) {
		AVFormatContext *oc = output_context;
		AVOutputFormat *fmt = oc->oformat;
		AVPacket pkt = {0};
		AVCodecContext *c = video_st.st->codec;
		
		//av_write_trailer(oc);

		/* Close each codec. */
		if (have_video)	{
			close_stream(oc, &video_st);
		}
		if (have_audio)
			close_stream(oc, &audio_st);
		have_video = have_audio = 0;

		av_write_trailer(oc);

		if (!(fmt->flags & AVFMT_NOFILE))
			avio_closep(&oc->pb);

		/* free the stream */
		avformat_free_context(output_context);
		output_context = NULL;
		stream_format = NULL;
		audio_codec = video_codec = NULL;
		raw_options_list = NULL;

		memset(&video_st, 0x00, sizeof(video_st));
		memset(&audio_st, 0x00, sizeof(audio_st));
	}
	if(raw_options_list != NULL) {
		av_dict_free(&raw_options_list);
		raw_options_list = NULL;
	}
#endif   // defined(USE_LIBAV)
	memset(audio_frame_buf, 0x00, sizeof(audio_frame_buf));
	memset(video_frame_buf, 0x00, sizeof(video_frame_buf));

	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Close: Left:  Video %lld frames, Audio %lld frames", video_data_queue.size(),
				  audio_data_queue.size());

	while(!audio_data_queue.isEmpty()) {
		QByteArray *pp = audio_data_queue.dequeue();
		if(pp != NULL) delete pp;
	}
	audio_data_queue.clear();

	while(!video_data_queue.isEmpty()) {
		QImage *pp = video_data_queue.dequeue();
		if(pp != NULL) delete pp;
	}
	video_data_queue.clear();

	// Message
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Close: Write:  Video %lld frames, Audio %lld frames", totalDstFrame, totalAudioFrame);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Close: Dequeue:  Video %lld frames, Audio %lld frames", totalDstFrame, audio_count);
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

