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
#include <QSize>

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

// Copy from FFMPEG-3.0.2; doc/example/muxing.c .

#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

//#define SCALE_FLAGS SWS_BICUBLIN
#define SCALE_FLAGS SWS_POINT

// a wrapper around a single output AVStream
typedef struct OutputStream {
    AVStream *st;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;

class OSD;

QT_BEGIN_NAMESPACE
class MOVIE_SAVER: public QThread
{
	Q_OBJECT
protected:
	OSD *p_osd;

	bool have_video;
	bool have_audio;
	bool encode_video;
	bool encode_audio;

	QStringList encode_opt_keys;
	QStringList encode_options;
#if defined(USE_LIBAV)
    AVOutputFormat *stream_format;
    AVFormatContext *output_context;
    AVCodec *audio_codec, *video_codec;
    AVDictionary *raw_options_list;

	OutputStream video_st;
	OutputStream audio_st;
#endif   
	QString _filename;
	bool bRunThread;
	bool debug_timestamp;
	
	uint min_rate;
	uint max_rate;
	uint buffer_size;
	int audio_sample_rate;
	int _width;
	int _height;
	int old_width;
	int old_height;
	
	bool recording;
	int rec_fps;

	
	uint64_t audio_size;
	uint64_t video_size;
	uint32_t ptsFrame;

	uint64_t totalSrcFrame;
	uint64_t totalDstFrame;
	uint64_t totalAudioFrame;

	int16_t audio_frame_buf[2 * 48000 * sizeof(int16_t)]; // 1Sec
	uint32_t video_frame_buf[1280 * 1024 * sizeof(uint32_t)]; // 1 frame : right?

	QQueue<QImage *> video_data_queue;
	QQueue<QByteArray *> audio_data_queue;
	int64_t audio_remain;
	int64_t video_remain;
	uint32_t audio_offset;
	uint32_t audio_frame_offset;
	uint32_t video_offset;
	uint64_t audio_frame_number;
	uint64_t audio_frame_max;
	uint64_t video_frame_number;
	uint64_t video_frame_max;

	int audio_bit_rate;
	int video_bit_rate;
	QSize video_geometry;
	int video_encode_threads;
	
	bool dequeue_audio(int16_t *);
	bool dequeue_video(uint32_t *);
	
	QString create_date_file_name(void);

	// Got from FFMPEG 3.0.2, doc/examples/muxer.c 
	//void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
	void log_packet(const void *_fmt_ctx, const void *_pkt);
	//int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
	int write_frame(void *_fmt_ctx, const void *_time_base, void *_st, void *_pkt);
	//void add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id)
	bool add_stream(void *_ost, void *_oc, void **_codec, uint64_t codec_id);
	//AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples)
	void *alloc_audio_frame(uint64_t _sample_fmt, uint64_t channel_layout,
							int sample_rate, int nb_samples);
	//static void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
	bool open_audio(void);
	//static AVFrame *get_audio_frame(OutputStream *ost)
	void *get_audio_frame();
	//static int write_audio_frame(AVFormatContext *oc, OutputStream *ost)
	int write_audio_frame();
	//static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
	void *alloc_picture(uint64_t _pix_fmt, int width, int height);
	//void open_video(OutputStream *_ost, AVDictionary *_opt_arg)
	bool open_video();
	//AVFrame *get_video_frame(OutputStream *ost)
	void *get_video_frame(void);
	//static int write_video_frame(AVFormatContext *oc, OutputStream *ost)
	int write_video_frame();
	//void MOVIE_SAVER::close_stream(AVFormatContext *oc, OutputStream *ost)
	void close_stream(void *_oc, void *_ost);


	QString ts2str(int64_t ts);
	QString ts2timestr(int64_t ts, void *timebase);
	QString err2str(int errnum);

public:
	MOVIE_SAVER(int width, int height, int fps, OSD *osd);
	~MOVIE_SAVER();
	bool is_recording(void);

public slots:
	void run();
	void enqueue_video(QImage *p);
	void enqueue_audio(int16_t *p, int size);
	void do_close();
	bool do_open(QString filename, int frame_rate, int sample_rate);
	void do_set_width(int width);
	void do_set_height(int height);
	void do_set_record_fps(int fps);

	void do_set_video_bitrate(int kbps);
	void do_set_audio_bitrate(int kbps);
	void do_set_video_geometry(QSize geometry);
	void do_set_video_threads(int threads);
	
	void do_clear_options_list(void);
	void do_add_option(QString key, QString value);
	void do_reset_encoder_options(void);
	
	void do_exit();
};
QT_END_NAMESPACE

#endif
