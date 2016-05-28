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


#if defined(USE_MOVIE_SAVER) && defined(USE_LIBAV)
static const AVRational time_base_15 = (AVRational){1001, 14485};
static const AVRational time_base_24 = (AVRational){1001, 23976};
static const AVRational time_base_25 = (AVRational){1001, 25025};
static const AVRational time_base_30 = (AVRational){1001, 29970};
static const AVRational time_base_60 = (AVRational){1001, 59940};
#endif // defined(USE_MOVIE_SAVER) &&  defined(USE_LIBAV) 

bool MOVIE_SAVER::setup_context(QString filename, int fps)
{
#if defined(USE_LIBAV)
	av_register_all();
	format = av_guess_format(NULL, filename.toLocal8Bit().constData(), NULL);
	printf("%s\n", filename.toLocal8Bit().constData());
	if(format == NULL) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to initialize libavf");
		return false;
	}
	
	avformat_alloc_output_context2(&output_context, NULL, NULL, filename.toLocal8Bit().constData());
	if(output_context == NULL) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to get output_context");
		return false;
	}
	
	output_context->oformat = format;
	
	snprintf(output_context->filename, 1000, "file://%s", filename.toLocal8Bit().constData());
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Start rec VIDEO: %s", output_context->filename);
#endif
	return true;
}

bool MOVIE_SAVER::setup_audio_codec(void *_opt)
{
#if defined(USE_LIBAV)
	AVFormatContext *oc = output_context;
	AVCodecContext *c;
	int nb_samples;
	int ret;
	AVDictionary *opt_arg = (AVDictionary *)_opt;;
	AVDictionary *opt = NULL;
    //c = oc->st->codec;
	c = audio_stream->codec;
    /* open it */
    //av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(c, audio_codec,  NULL);
    //av_dict_free(&opt);
	
    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie/Saver: Could not open audio codec\n");
        return false;
    }

	if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;

    audio_frame_data     = (AVFrame *)alloc_audio_frame(c->sample_fmt, c->channel_layout,
											 c->sample_rate, nb_samples);
    audio_tmp_frame = (AVFrame *)alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout,
                                       c->sample_rate, nb_samples);
#endif
    /* create resampler context */
	return true;
}	

bool MOVIE_SAVER::setup_video_codec()
{
#if defined(USE_LIBAV)
#if 0 // Todo	
	video_stream = av_new_stream(output_context, 0);
	if(video_stream == NULL) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to open video stream");
		do_close();
		return;
	}

	get_host_time(t_time);
	video_codec = video_stream->codec;
	video_codec->gop_size = 15;
	video_codec->max_b_frames = 8;
	video_codec->has_b_frames = 1;
	QString author = QString::fromUtf8("emu");
	author = author + p_osd->get_vm_config_name();
	QString date_str = QString::fromUtf8("Record from ");
	date_str = date_str + create_date_file_name();
	
	switch(rec_fps) {
	case 15:
		time_base = time_base_15;
		break;
	case 24:
		time_base = time_base_24;
		break;
	case 25:
		time_base = time_base_25;
		break;
	case 30:
		time_base = time_base_30;
		break;
	case 60:
		time_base = time_base_60;
		break;
	default:
		time_base = (AVRational){1001, rec_fps * 1000};
		break;
	}
	{ // Video Start
		av_dict_set(&output_context->metadata, "title", date_str.toUtf8().constData(), 0);
		av_dict_set(&output_context->metadata, "author", author.toUtf8().constData(), 0);
		video_codec->has_b_frames=2; // let muxer know we may have bpyramid
		video_codec->codec_id = CODEC_ID_H264;
		video_codec->codec = &codec_real;
		memset(video_codec->codec, 0, sizeof(struct AVCodec));
		strcpy(video_codec->codec->name, "H264");
		{
			// Set basic rate
			video_codec->rc_buffer_size = buffer_size;
			video_codec->rc_max_rate = max_rate;
			video_codec->rc_min_rate = min_rate;
			video_codec->bit_rate = bitrate;
		}
		video_codec->codec_type = AVMEDIA_TYPE_VIDEO;
		video_codec->flags = CODEC_FLAG_QSCALE;
		video_codec->width = _width;
		video_codec->height = _height;
	}

	switch(rec_fps) {
	case 15:
		video_codec->time_base = time_base_15;
		video_codec->bit_rate = bitrate / 2;
		video_codec->rc_max_rate = max_rate / 2;
		video_codec->rc_min_rate = min_rate / 2;
		break;
	case 24:
		video_codec->time_base = time_base_24;
		break;
	case 25:
		video_codec->time_base = time_base_25;
		break;
	case 30:
		video_codec->time_base = time_base_30;
		break;
	case 60:
		video_codec->time_base = time_base_60;
		video_codec->bit_rate = bitrate * 2;
		video_codec->rc_max_rate = max_rate * 2;
		video_codec->rc_min_rate = min_rate * 2;
		break;
	default:
		time_base = (AVRational){1001, rec_fps * 1000};
		video_codec->time_base = time_base;
		video_codec->bit_rate = (int)((double)bitrate * 30.0 / (double)rec_fps);
		video_codec->rc_max_rate = (int)((double)max_rate * 30.0 / (double)rec_fps);
		video_codec->rc_min_rate = (int)((double)min_rate * 30.0 / (double)rec_fps);
		break;
	}
#endif // ToDo
#endif
	return true;
}

void MOVIE_SAVER::do_open(QString filename, int fps)
{
	int ret;
	do_close();
	
	have_video = have_audio = false;
	encode_video = encode_audio = false;
	audio_option = NULL;
	video_option = NULL;	
	do_set_record_fps(fps);

	_filename = filename;
	if(!setup_context(filename, fps)) {
		do_close();
		return;
	}
#if defined(USE_LIBAV)
    //if (format->video_codec != AV_CODEC_ID_NONE) {
	//  add_stream_video(&video_stream, oc, &video_codec, format->video_codec);
    //    have_video = true;
	//   encode_video = true;
    //}
    if (format->audio_codec != AV_CODEC_ID_NONE) {
        //add_stream_audio((void **)(&audio_codec->codec), (int)(format->audio_codec));
        add_stream_audio((void **)(&audio_codec), (int)AV_CODEC_ID_AAC);
        have_audio = true;
        encode_audio = true;
    }
//    if (have_video)
//        open_video(oc, video_codec, &video_st, opt);
//    if (have_audio)
//        open_audio(oc, audio_codec, &audio_st, opt);

	audio_codec_context = audio_stream->codec;
	if(!setup_audio_codec(&audio_option)) {
		do_close();
		return;
	}
	if(!setup_audio_resampler()) {
		do_close();
		return;
	}
	output_context->bit_rate = 100080 * 1000;
	output_context->audio_preload = AV_TIME_BASE / 10;
	output_context->max_delay = 100 * 1000; // MAX 100ms delay;
	
	if(avio_open(&(output_context->pb), filename.toLocal8Bit().constData(), AVIO_FLAG_WRITE) < 0) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to open file");
		do_close();
		return;
	}		

	av_dump_format(output_context, 0, filename.toLocal8Bit().constData(), 1);
	//av_dump_format(output_context, 1, NULL, 1);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE/Saver: Successfully opened AVC stream.");
    /* Write the stream header, if any. */
    ret = avformat_write_header(output_context, &audio_option);
    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE/Saver: Error occurred when opening output header\n");
        return;
    }

#endif	// defined(USE_LIBAV)
	recording = true;
}
