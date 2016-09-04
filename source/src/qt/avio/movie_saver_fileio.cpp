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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
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

//int MOVIE_SAVER::write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
int MOVIE_SAVER::write_frame(void *_fmt_ctx, const void *_time_base, void *_st, void *_pkt)
{
#if defined(USE_LIBAV)
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
#else
	return 0;
#endif
}

/* Add an output stream. */
//static void add_stream(OutputStream *ost, AVFormatContext *oc,
//					   AVCodec **codec,
//					   enum AVCodecID codec_id)
bool MOVIE_SAVER::add_stream(void *_ost, void *_oc,
					   void **_codec,
					   uint64_t _codec_id)
{
#if defined(USE_LIBAV)
	AVCodecContext *c;
	int i;

	OutputStream *ost = (OutputStream *)_ost;
	AVFormatContext *oc = (AVFormatContext *)_oc;
	AVCodec **codec = (AVCodec **)_codec;
	enum AVCodecID codec_id = (enum AVCodecID)_codec_id;
	/* find the encoder */
	*codec = avcodec_find_encoder(codec_id);
	if (!(*codec)) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not find encoder for '%s'\n",
				(const char *)avcodec_get_name(codec_id));
		return false;
	}

	ost->st = avformat_new_stream(oc, *codec);
	if (!ost->st) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not allocate stream\n");
		return false;
	}
	ost->st->id = oc->nb_streams-1;
	c = ost->st->codec;

	switch ((*codec)->type) {
	case AVMEDIA_TYPE_AUDIO:
		setup_audio(c, codec);
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
		c->thread_count	 = video_encode_threads;
		
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
			setup_mpeg4((void *)c);
		}
		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
			/* Needed to avoid using macroblocks in which some coeffs overflow.
			 * This does not happen with normal video, it just happens here as
			 * the motion of the chroma plane does not match the luma plane. */
			c->mb_decision = 2;
		}
		if (c->codec_id == AV_CODEC_ID_H264) {
			setup_h264((void *)c);
		}
	break;

	default:
		break;
	}

	/* Some formats want stream headers to be separate. */
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	return true;
#else
	return false;
#endif	
}



//void MOVIE_SAVER::close_stream(AVFormatContext *oc, OutputStream *ost)
void MOVIE_SAVER::close_stream(void *_oc, void *_ost)
{
#if defined(USE_LIBAV)
	AVFormatContext *oc = (AVFormatContext *)_oc;
	OutputStream *ost = (OutputStream *)_ost;
	avcodec_close(ost->st->codec);
	while(avcodec_is_open(ost->st->codec) != 0) { this->msleep(5);}
	av_frame_free(&ost->frame);
	av_frame_free(&ost->tmp_frame);
	sws_freeContext(ost->sws_ctx);
	swr_free(&ost->swr_ctx);
#endif	
}

void MOVIE_SAVER::do_open(QString filename, int _fps, int _sample_rate)
{
	if(recording || req_close || req_stop) return;
	if(filename.isEmpty()) return;
	if(_fps <= 0) return;
	if(_sample_rate <= 10) return;
	do_set_record_fps(_fps);
	audio_sample_rate = _sample_rate;
	_filename = filename;
	recording = true;
}

bool MOVIE_SAVER::do_open_main()
{
#if defined(USE_LIBAV)
	if(req_close) return false;
	if(req_stop) return false;
	if(_filename.isEmpty()) return false;
	if(rec_fps <= 0) return false;
	if(audio_sample_rate <= 10) return false;
	AVOutputFormat *fmt;
	AVFormatContext *oc;
   	int ret;
	have_video = 0, have_audio = 0;
	int encode_video = 0, encode_audio = 0;
	 
	raw_options_list = NULL;
	printf("Sample Rate:%d\n",audio_sample_rate);
	
	video_st = { 0 };
	audio_st = { 0 };
	/* Initialize libavcodec, and register all codecs and formats. */
	av_register_all();

	{
		QString value;
		do_clear_options_list();
		do_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("mpeg4"));
		//do_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("aac"));
		//do_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("theora"));
		do_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("vorbis"));
		
		video_encode_threads = p_config->video_threads;
		video_geometry = QSize(p_config->video_width, p_config->video_height);
		audio_bit_rate = p_config->audio_bitrate * 1000;
		
	}

	for(int i = 0; i < encode_options.size(); i++) {
		if(encode_opt_keys.size() <= i) break;
		av_dict_set(&raw_options_list, encode_opt_keys.takeAt(i).toUtf8().constData(),
					encode_options.takeAt(i).toUtf8().constData(), 0);
	}

	/* allocate the output media context */
	avformat_alloc_output_context2(&oc, NULL, NULL, _filename.toLocal8Bit().constData());
	if (!oc) {
		printf("Could not reduce output format from file extension: using MPEG.\n");
		avformat_alloc_output_context2(&oc, NULL, "mpeg", _filename.toLocal8Bit().constData());
	}
	if (!oc)
		return false;

	fmt = oc->oformat;
	switch(p_config->video_codec_type) {
	case VIDEO_CODEC_MPEG4:
		fmt->video_codec = AV_CODEC_ID_MPEG4;
		break;
	case VIDEO_CODEC_H264:
		fmt->video_codec = AV_CODEC_ID_H264;
		break;
	}
	switch(p_config->audio_codec_type) {
	case AUDIO_CODEC_MP3:
		fmt->audio_codec = AV_CODEC_ID_MP3;
		break;
	case AUDIO_CODEC_AAC:
		fmt->audio_codec = AV_CODEC_ID_AAC;
		break;
	case AUDIO_CODEC_VORBIS:
		fmt->audio_codec = AV_CODEC_ID_VORBIS;
		break;
	}		
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
		ret = avio_open(&oc->pb, _filename.toLocal8Bit().constData(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER,
								  "Could not open '%s': %s\n", _filename.toLocal8Bit().constData(),
					err2str(ret).toLocal8Bit().constData());
			goto _err_final;
		} else {
			csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER,
								  "Success to Open file: '%s\n", _filename.toLocal8Bit().constData());
		}			
	}
	totalSrcFrame = 0;
	totalDstFrame = 0;
	totalAudioFrame = 0;
//	audio_enqueue_count = 0;

	/* Write the stream header, if any. */
	ret = avformat_write_header(oc, &raw_options_list);
	if (ret < 0) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER,
							  "Error occurred when opening output file: %s\n",
				err2str(ret).toLocal8Bit().constData());
		goto _err_final;
	}
	emit sig_set_state_saving_movie(true);
	return true;
_err_final:
	avformat_free_context(oc);
	oc = NULL;
	output_context = NULL;
	stream_format  = NULL;
	emit sig_set_state_saving_movie(false);
	return false;
#else
	emit sig_set_state_saving_movie(true);
	return true;
#endif	
}

void MOVIE_SAVER::do_close()
{
	req_close = true;
	req_stop = true;
}

void MOVIE_SAVER::do_close_main()
{
	if(!req_close) return;
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
		bool a_f, v_f;
		bool need_video_transcode, need_audio_transcode;
		a_f = v_f = false;
		need_video_transcode = need_audio_transcode = false;
		//av_write_trailer(oc);
		/* Close each codec. */
		//if(totalSrcFrame != totalDstFrame) {
		while(!a_f || !v_f) {
			if(audio_remain <= 0) {
				a_f = audio_data_queue.isEmpty();
				if(!a_f) {
					dequeue_audio(audio_frame_buf);
					audio_remain = audio_size;
					audio_offset = 0;
				}
			}
			{
				v_f = video_data_queue.isEmpty();
				if(!v_f) {
					if(left_frames <= 0) dequeue_video(video_frame_buf);
					left_frames--;
					video_remain = video_size;
					video_offset = 0;
				}
			}

			if ((n_encode_video == 0) &&
				((n_encode_audio != 0) || av_compare_ts(video_st.next_pts, video_st.st->codec->time_base,
														audio_st.next_pts, audio_st.st->codec->time_base) <= 0)) {
				n_encode_video = write_video_frame();
				if(n_encode_video < 0) break;
			} else {
				n_encode_audio = write_audio_frame();
				if(n_encode_audio < 0) break;
			}
		}
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

	uint64_t leftq_a, leftq_v;
	leftq_a = leftq_v = 0;
	while(!audio_data_queue.isEmpty()) {
		QByteArray *pp = audio_data_queue.dequeue();
		if(pp != NULL) {
			leftq_a++;
			delete pp;
		}
	}
	audio_data_queue.clear();

	while(!video_data_queue.isEmpty()) {
		VIDEO_DATA *pp = video_data_queue.dequeue();
		if(pp != NULL) {
			if(pp->frames >= 0) leftq_v += pp->frames;
			delete pp;
		}
	}
	video_data_queue.clear();

	csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER,
						  "Close: Left:  Video %lld frames, Audio %lld frames",
				  leftq_v,
				  leftq_a
	);
	// Message
	csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER,
				  "MOVIE: Close: Write:  Video %lld frames, Audio %lld frames", totalDstFrame, totalAudioFrame);
	csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER,
						  "MOVIE: Close: Dequeue:  Video %lld frames, Audio %lld frames", totalDstFrame, audio_count);
	totalSrcFrame = 0;
	totalDstFrame = 0;
	totalAudioFrame = 0;
	//audio_enqueue_count = 0;
	audio_remain = 0;
	video_remain = 0;
	audio_offset = 0;
	audio_frame_offset = 0;
	video_offset = 0;
	video_count = 0;
	audio_count = 0;
	
	req_close = false;
	req_stop = false;
	recording = false;
	
	emit sig_set_state_saving_movie(false);
}


QString MOVIE_SAVER::create_date_file_name(void)
{
	QDateTime nowTime = QDateTime::currentDateTime();
	QString tmps = nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz."));
	return tmps;
}

