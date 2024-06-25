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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

//int MOVIE_SAVER::write_frame(const AVRational *time_base, AVStream *st, AVPacket *pkt)
int MOVIE_SAVER::write_frame(const void *_time_base, void *_st, void *_pkt)
{
	__UNLIKELY_IF((output_context == nullptr) || (_time_base == nullptr) || (_st == nullptr) || (_pkt == nullptr)) {
		return 0;
	}
#if defined(USE_LIBAV)
	AVFormatContext* fmt_ctx = output_context;
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
bool MOVIE_SAVER::add_stream(void *_ost, 
					   void **_codec,
					   int64_t _codec_id)
{
#if defined(USE_LIBAV)
	if(output_context == nullptr) { // OK?
		return false;
	}
	AVCodecContext *c;
	OutputStream *ost = (OutputStream *)_ost;
	AVFormatContext* oc = output_context;
	
	AVCodec **codec = (AVCodec **)_codec;
	enum AVCodecID codec_id = (enum AVCodecID)_codec_id;
	/* find the encoder */
	*codec = (AVCodec *)avcodec_find_encoder(codec_id);
	if (*codec == nullptr ) {
		out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not find encoder for '%s'\n",
				(const char *)avcodec_get_name(codec_id));
		return false;
	}

	ost->st = avformat_new_stream(oc, *codec);
	if (!ost->st) {
		out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not allocate stream\n");
		return false;
	}
	ost->st->id = oc->nb_streams - 1;
	#ifdef AVCODEC_UPPER_V56
	ost->context = avcodec_alloc_context3(*codec);
	if (ost->context == NULL) {
		out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Failed to allocate context for encoding: \n");
		return false;
	}
	c = ost->context;
	#else /* AVCODEC_UPPER_V56 */
	c = ost->st->codec;
	#endif /* AVCODEC_UPPER_V56 */
	switch ((*codec)->type) {
	case AVMEDIA_TYPE_AUDIO:
		setup_audio(c, ost, codec, codec_id);
		ost->st->time_base = (AVRational){ 1, c->sample_rate };
		c->time_base = (AVRational){ 1, c->sample_rate };
		break;
	case AVMEDIA_TYPE_VIDEO:
		setup_video(c, ost, codec, codec_id);
		// See:
		/* timebase: This is the fundamental unit of time (in seconds) in terms
		 * of which frame timestamps are represented. For fixed-fps content,
		 * timebase should be 1/framerate and timestamp increments should be
		 * identical to 1. */
		ost->st->time_base = (AVRational){ 1, rec_fps};
		c->time_base	   = ost->st->time_base;
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



//void MOVIE_SAVER::close_stream(OutputStream *ost)
void MOVIE_SAVER::close_stream(void *_ost)
{
#if defined(USE_LIBAV)
	OutputStream *ost = (OutputStream *)_ost;
	#ifdef AVCODEC_UPPER_V56
	avcodec_close(ost->context);
	while(avcodec_is_open(ost->context) != 0) { this->msleep(5);}
	#else
	avcodec_close(ost->st->codec);
	while(avcodec_is_open(ost->st->codec) != 0) { this->msleep(5);}
	#endif
	av_frame_free(&ost->frame);
	av_frame_free(&ost->tmp_frame);
	sws_freeContext(ost->sws_ctx);
	swr_free(&ost->swr_ctx);
	#ifdef AVCODEC_UPPER_V56
	avcodec_free_context(&(ost->context));
	#endif
#endif /* USE_LIBAV */
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
	if(output_context != nullptr) return false; // Already OPEN?
	AVOutputFormat *fmt;
	AVFormatContext *oc;
   	int ret;
	have_video = have_audio = false;
	 
	raw_options_list = NULL;
	//video_st = { 0 };
	//audio_st = { 0 };
	memset(&video_st, 0x00, sizeof(video_st));
	memset(&audio_st, 0x00, sizeof(audio_st));
	
	/* Initialize libavcodec, and register all codecs and formats. */
#ifndef AVCODEC_UPPER_V56
	av_register_all();
#endif

	{
		QString value;
		do_clear_options_list();
		do_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("mpeg4"));
		//do_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("aac"));
		//do_add_option(QString::fromUtf8("c:v"), QString::fromUtf8("theora"));
		do_add_option(QString::fromUtf8("c:a"), QString::fromUtf8("mp3"));
		
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
	if(oc == nullptr) {
//		printf("Could not reduce output format from file extension: using MPEG.\n");
		avformat_alloc_output_context2(&oc, NULL, "mpeg", _filename.toLocal8Bit().constData());
	}
	if(oc == nullptr)
		return false;
	
	fmt = (AVOutputFormat *)(oc->oformat);
	output_context = oc;

	enum AVCodecID vcodec = AV_CODEC_ID_NONE;
	if(auto n = video_codec_map.find((MOVIE_SAVER_VIDEO_Codec_t)(p_config->video_codec_type)) ; n != video_codec_map.end()) {
		vcodec = n->second;
	}
	enum AVCodecID acodec = AV_CODEC_ID_NONE;
	if(auto n = audio_codec_map.find((MOVIE_SAVER_AUDIO_Codec_t)(p_config->audio_codec_type)) ; n != audio_codec_map.end()) {
		acodec = n->second;
	}

//	fmt->video_codec = vcodec;
//	fmt->audio_codec = acodec;
	
	/* Add the audio and video streams using the default format codecs
	 * and initialize the codecs. */
	if (vcodec != AV_CODEC_ID_NONE) {
		if(!add_stream((void *)&video_st, (void **)&video_codec, (int64_t)(vcodec))) goto _err_final;
		have_video = true;
	}
	if (acodec != AV_CODEC_ID_NONE) {
		if(!add_stream((void *)&audio_st, (void **)&audio_codec, (int64_t)(acodec))) goto _err_final;
		have_audio = true;
	}
	/* Now that all the parameters are set, we can open the audio and
	 * video codecs and allocate the necessary encode buffers. */
	if (have_video)
		open_video();

	if (have_audio)
		open_audio();
	
	av_dump_format(oc, 0, _filename.toLocal8Bit().constData(), 1);

	/* open the output file, if needed */
	if (!(fmt->flags & AVFMT_NOFILE)) {
		ret = avio_open(&(oc->pb), _filename.toLocal8Bit().constData(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER,
								  "Could not open '%s': %s\n", _filename.toLocal8Bit().constData(),
					err2str(ret).toLocal8Bit().constData());
			goto _err_final;
		} else {
			out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER,
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
		out_debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER,
							  "Error occurred when opening output file: %s\n",
				err2str(ret).toLocal8Bit().constData());
		goto _err_final;
	}
	emit sig_set_state_saving_movie(true);
	return true;
_err_final:
	free_context();
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
	if(output_context != nullptr) {
		AVFormatContext* oc = output_context;
		AVOutputFormat *fmt = (AVOutputFormat *)(oc->oformat);
		//AVPacket pkt = {0};
		//AVCodecContext *c = video_st.st->codec;
		bool a_f, v_f;
		//bool need_video_transcode, need_audio_transcode;
		bool need_audio_transcode;
		a_f = v_f = false;
		//need_video_transcode = need_audio_transcode = false;
		need_audio_transcode = false;
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
			int result = 0;
#ifdef AVCODEC_UPPER_V56
			if(audio_st.context != NULL) {
				if(video_st.context != NULL) {
					result = av_compare_ts(video_st.next_pts, video_st.context->time_base,
										   audio_st.next_pts, audio_st.context->time_base);
				} else {
					result = 1;
				}
			} else {
				result = -1;
			}
#else
			result = av_compare_ts(video_st.next_pts, video_st.st->codec->time_base,
								   audio_st.next_pts, audio_st.st->codec->time_base);
#endif
			if ((n_encode_video == 0) &&
				((n_encode_audio != 0) || (result <= 0))) {
				n_encode_video = write_video_frame();
				if(n_encode_video < 0) break;
			} else {
				n_encode_audio = write_audio_frame();
				if(n_encode_audio < 0) break;
			}
		}
		if (have_video)	{
			close_stream(&video_st);
		}
		if (have_audio) {
			close_stream(&audio_st);
		}
		have_video = have_audio = false;

		av_write_trailer(oc);

		if (!(fmt->flags & AVFMT_NOFILE))
			avio_closep(&(oc->pb));

		/* free the stream */
		free_context();
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

	out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER,
						  "Close: Left:  Video %lld frames, Audio %lld frames",
				  leftq_v,
				  leftq_a
	);
	// Message
	out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER,
				  "MOVIE: Close: Write:  Video %lld frames, Audio %lld frames", totalDstFrame, totalAudioFrame);
	out_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER,
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

