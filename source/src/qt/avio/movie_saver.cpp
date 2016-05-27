/*
 * Common Source Code Project for Qt : movie saver.
 * (C) 2016 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  License: GPLv2
 *  History: May 27, 2016 : Initial. This refer from avidemux 2.5.6 .
 */

#include <QDateTime>
#include "movie_saver.h"
#include "osd.h"

MOVIE_SAVER::MOVIE_SAVER(int width, int height, int fps, OSD *osd) : QThread(0)
{
	buffer_size=8 * 1024 * 224;
	max_rate=4000 * 1000;
	min_rate=0;
	bitrate = 2000 * 1000;
	_width = width;
	_height = height;
	rec_fps = fps;
	p_osd = osd;
	recording = false;

	audio_data_queue.clear();

	video_data_queue.clear();
	video_width_queue.clear();
	video_height_queue.clear();
	
	totalSrcFrame = 0;
	totalDstFrame = 0;
	totalAudioFrame = 0;
	bRunThread = false;
}

MOVIE_SAVER::~MOVIE_SAVER()
{
	do_close();
}

void MOVIE_SAVER::enqueue_video(QByteArray *p, int width, int height)
{
	if(!recording) return false;
	if(p == NULL) return;
	QByteArray *pp = new QByteArray(p->data(), p->size());
	
	video_data_queue.enqueue(pp);
	video_width_queue.enqueue(width);
	video_height_queue.enqueue(height);
}

bool MOVIE_SAVER::dequeue_video(uint32_t *p)
{
	if(!recording) return false;
	if(video_data_queue.isEmpty()) return false;
	if(p == NULL) return false;

	QByteArray *pp = video_data_queue.dequeue();
	if((pp == NULL) || (video_size <= 0)) return false;
	
	video_size = pp->size();
	memcpy(p, pp->data(), video_size);
	delete pp;
	
	return true;
}

void MOVIE_SAVER::enqueue_audio(QByteArray *p)
{
	if(!recording) return;
	if(p == NULL) return;
	QByteArray *pp = new QByteArray(p->data(), p->size());
	audio_data_queue.enqueue(pp);
}

bool MOVIE_SAVER::dequeue_audio(int16_t *p)
{
	if(!recording) return false;
	if(audio_data_queue.isEmpty()) return false;
	if(p == NULL) return false;

	QByteArray *pp = audio_data_queue.dequeue();
	if((pp == NULL) || (audio_size <= 0)) return false;

	audio_size = pp->size();
	memcpy(p, pp->data(), audio_size);
	delete pp;
	return true;
}

void MOVIE_SAVER::run()
{
	bRunThread = true;
	
	int fps_wait = (int)((1000.0 / this->vm_frame_rate()) / 2.0);
	int tmp_wait = fps_wait;
	while(bRunThread) {
		if(recording) {
			if(!bRunThread) break;
			if(!recording) break;
			if(!audio_data_queue.isEmpty()) {
				if(dequeue_audio(audio_frame)) {
					AVPacket pkt;
					uint64_t bytes = audio_size;
					uint64_t us = (uint64_t)floor(((double)bytes * 1000000.0) / (double)audio_codec->sample_rate);
					double samples = ((double)us / 1000000.0) * (double)audio_codec->sample_rate()   
					int ret;
					
					if(bytes == 0) goto _video;
					
					av_init_packet(&pkt);
					pkt.dts = pkt.pts = (int64_t)samples;
					pkt.flags |= AV_PKT_FLAG_KEY;
					pkt.data = (uint8_t *)audio_frame;
					pkt.size = (uint32_t)bytes;
					pkt.stream_index = 1;
					ret = av_write_frame(output_context, &pkt);
				
					totalAudioFrame++;
				}
			}
		_video:
#if 0 // ToDo			
			if(!bRunThread) break;
			if(!recording) break;
			if(!video_data_queue.isEmpty() &&
			   !video_height_queue.isEmpty() && !video_width_queue.isEmpty()) {
				// Scale Video
				if(dequeue_video(video_frame)) {
					AVRational fps = {1, 1000000};
					AVPacket pkt;
					uint64_t bytes = video_size;
					int i;
					av_init_packet(&pkt);
					// Call encoder
					
					// Call muxer
					
					totalSrcFrame++;
					totalDstFrame++;
				}
			}
#endif			
		}
		if(fps_wait >= tmp_wait) {
			this->msleep(tmp_wait);
			tmp_wait = 0;
		} else {
			this->msleep(fps_wait);
			tmp_wait -= fps_wait;
		}
		if(tmp_wait <= 0) {
			fps_wait = (int)((1000.0 / this->vm_frame_rate()) / 2.0);
			tmp_wait = fps_wait;
		}
	}
}

void MOVIE_SAVER::do_close()
{
	if(output_context != NULL) {
		av_write_trailer(output_context);
		avio_close(output_context->pb);
	}
	if(audio_stream != NULL) {
		av_free(audio_stream);
	}
	if(video_stream != NULL) {
		av_free(video_stream);
	}
	audio_stream = NULL;
	video_stream = NULL;

	if(output_context != NULL) {
		av_free(output_context);
		output_context = NULL;
	}
	recording = false;

	audio_data_queue.clear();

	video_data_queue.clear();
	video_width_queue.clear();
	video_height_queue.clear();

	// Message
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Close: Read:   Video %ll frames, Audio %ll frames", totalSrcFrame, totalAudioFrame);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Close: Write:  Video %ll frames, Audio %ll frames", totalDstFrame, totalAudioFrame);
	totalSrcFrame = 0;
	totalDstFrame = 0;
	totalAudioFrame = 0;
}

QString MOVIE_PLAYER::create_date_file_name(void)
{
	QDateTime nowTime = QDateTime::currentDateTime();
	QString tmps = nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz."));
	return tmps;
}

bool MOVIE_SAVER::is_recording(void)
{
	return recording;
}
static const AVRational time_base_15 = (AVRational){1001, 14485};
static const AVRational time_base_24 = (AVRational){1001, 23976};
static const AVRational time_base_25 = (AVRational){1001, 25025};
static const AVRational time_base_30 = (AVRational){1001, 29970};
static const AVRational time_base_60 = (AVRational){1001, 59940};

void MOVIE_SAVER::do_open(QString filename)
{
	cur_time_t *t_time;
	do_close();

	_filename = filename;
	format = av_guess_format("mp4", NULL, NULL);

	if(format == NULL) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to initialize libavf");
		return;
	}
	
	output_context = avformat_alloc_context();
	if(output_context == NULL) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to get output_context");
		do_close();
		return;
	}
	
	output_context->oformat = format;
	snprintf(output_context->filename, 1000, "file://%s", filename.fromUtf8().constData());
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Start rec VIDEO: %s", output_context->filename);

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
	author = author + osd->get_vm_config_name();
	QString date_str = QString::fromUtf8("Record from");
	date_str = date_str + create_date_file_name();
	
	{ // Video Start
		av_dict_set(&output_context->metadata, "title", date_str.fromUtf8().constData(), 0);
		av_dict_set(&output_context->metadata, "author", author.fromUtf8().constData(), 0);
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
	
	//audio_stream = av_new_stream(output_context, 1);
	audio_stream = av_new_stream(output_context, 0);
	if(audio_stream == NULL) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to open audio stream");
		do_close();
		return;
	}

	// Temporally using libAV's AAC
	audio_codec = audio_stream->codec;
	audio_codec->frame_size = 1024; 
	audio_codec->codec_id = CODEC_ID_AAC;
	audio_codec->sample_rate = osd->get_sound_rate();

	audio_codec->codec_type = AVMEDIA_TYPE_AUDIO;
	
	audio_codec->bit_rate = audio_codec->sample_rate * 8 * 2;
	audio_codec->rc_buffer_size = audio_codec->sample_rate / 4; // 250 ms worth
	audio_codec->channels = 2;

	// Context
	output_context->mux_rate = 100080 * 1000;
	output_context->pre_load = AV_TIME_BASE / 10;
	output_context->max_delay = 100 * 1000; // MAX 100ms delay;

	if(avio_open(&(output_context->pb), filename.fromUtf8().constData(), AVIO_FLAG_WRITE) < 0) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to open file");
		do_close();
		return;
	}		

	av_dump_format(output_context, 0, filename.fromUtf8().constData(), 1);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Successfully opened AVC stream.");
		
	recording = true;
}

void MOVIE_SAVER::do_exit()
{
	bRunThread = false;
}

void MOVIE_SAVER::do_set_record_fps(int fps)
{
	if((fps > 0) && (fps <= 60)) rec_fps = fps;
}

void MOVIE_SAVER::do_set_width(int width)
{
	_width = width;
}

void MOVIE_SAVER::do_set_height(int height)
{
	_height = height;
}

