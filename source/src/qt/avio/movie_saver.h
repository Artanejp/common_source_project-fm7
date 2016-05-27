/*
 * Common Source Code Project for Qt : movie saver.
 * (C) 2016 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  License: GPLv2
 *  History: May 27, 2016 : Initial. This refer from avidemux 2.5.6 .
 */

#ifndef _QT_OSD_MOVIE_SAVER_H
#define _QT_OSD_MOVIE_SAVER_H

#include <QByteArray>
#include <QQueue>
#include <QString>
#include <QThread>

#if defined(USE_LIBAV)
extern "C" {
	#include "libavutil/channel_layout.h"
	#include "libavutil/opt.h"
	#include "libavutil/mathematics.h"
	#include "libavutil/timestamp.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
	#include "libswresample/swresample.h"
}
#endif

class OSD;

QT_BEGIN_NAMESPACE
class MOVIE_SAVER: public QThread
{
	Q_OBJECT
protected:
	OSD *p_osd;
#if defined(USE_LIBAV)
	AVFormatContext *output_context;
	AVOutputFormat *format;
	AVCodecContext *audio_codec;
	AVCodecContext *video_codec;
	AVStream *audio_stream;
	AVStream *video_stream;

	AVFrame *audio_frame_data;
	AVFrame *audio_tmp_frame;
	struct AVCodec codec_real;
	struct SwrContext *audio_swr_context;
	int audio_nb_samples;
	int audio_samples_count;
	
	int64_t audio_dst_nb_samples;
	int64_t audio_next_pts;
#endif   
	QString _filename;
	bool bRunThread;
	
	uint min_rate;
	uint max_rate;
	uint buffer_size;
	uint bitrate;
	int _width;
	int _height;
	bool recording;
	int rec_fps;

	AVRational time_base;
	
	uint64_t audio_size;
	uint64_t video_size;
	uint32_t ptsFrame;

	uint64_t totalSrcFrame;
	uint64_t totalDstFrame;
	uint64_t totalAudioFrame;

	int16_t audio_frame[2 * 48000 * sizeof(int16_t)]; // 1Sec
	uint32_t video_frame[1280 * 512 * sizeof(uint32_t)]; // 1 frame : right?
	uint32_t video_dst[1280 * 1024 * sizeof(uint32_t)]; // 1 frame : right?

	QQueue<int> video_width_queue;
	QQueue<int> video_height_queue;
	QQueue<QByteArray *> video_data_queue;
	
	QQueue<QByteArray *> audio_data_queue;
	
	bool dequeue_audio(int16_t *);
	bool dequeue_video(uint32_t *);
	QString create_date_file_name(void);
	int write_audio_frame(const void *_time_base, void *_pkt);
	AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
							   uint64_t channel_layout,
							   int sample_rate, int nb_samples);
	
public:
	MOVIE_SAVER(int width, int height, int fps, OSD *osd);
	~MOVIE_SAVER();
	bool is_recording(void);

public slots:
	void run();
	void enqueue_video(QByteArray *p, int width, int height);
	void enqueue_audio(QByteArray *p);
	void do_close();
	void do_open(QString filename, int);
	void do_set_width(int width);
	void do_set_height(int height);
	void do_set_record_fps(int fps);
	void do_exit();
};
QT_END_NAMESPACE

#endif
