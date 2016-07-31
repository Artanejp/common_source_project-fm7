/*
 * Common Source Code Project for Qt : movie saver.
 * (C) 2016 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  License: GPLv2
 *  History: May 27, 2016 : Initial. This refer from ffmpeg: doc/examples/demuxing_decoding.c.
 */

#ifndef _QT_OSD_MOVIE_LOADER_H
#define _QT_OSD_MOVIE_LOADER_H

#if defined(USE_LIBAV)
extern "C" {
	#include "libavutil/channel_layout.h"
	#include "libavutil/opt.h"
	#include "libavutil/mathematics.h"
	#include "libavutil/timestamp.h"
	#include "libavutil/imgutils.h"
	#include "libavutil/samplefmt.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
	#include "libswresample/swresample.h"
}
#endif


// Copy from FFMPEG-3.0.2; doc/example/muxing.c .
#define STREAM_PIX_FMT	AV_PIX_FMT_RGBA /* default pix_fmt */

//#define SCALE_FLAGS SWS_BICUBLIN
#define SCALE_FLAGS SWS_POINT

#include <QObject>
#include <QReadWriteLock>
#include <QWaitCondition>

#include "config.h"

typedef struct {
	uint8_t *data;
	long unpadded_linesize;
} sound_data_cueue_t;

QT_BEGIN_NAMESPACE
class OSD;
class MOVIE_LOADER: public QObject
{
	Q_OBJECT
private:
#if defined(USE_LIBAV)
	AVFormatContext *fmt_ctx; // = NULL;
	AVCodecContext *video_dec_ctx;// = NULL
	AVCodecContext *audio_dec_ctx;
	enum AVPixelFormat pix_fmt;
	AVStream *video_stream, *audio_stream; // NULL
	uint8_t *video_dst_data[4]; // = {NULL};
	int      video_dst_linesize[4];
	int video_dst_bufsize;
	int video_stream_idx, audio_stream_idx; //int video_stream_idx = -1, audio_stream_idx = -1;
	AVFrame *frame; //AVFrame *frame = NULL;
	AVPacket pkt;
	int video_frame_count; // = 0;
	int audio_frame_count; // = 0;
	int refcount; // = 0
	int decode_packet(int *got_frame, int cached);
	int open_codec_context(int *stream_idx, AVFormatContext *fmt_ctx, enum AVMediaType type);
	int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt);

#endif
protected:
	OSD *p_osd;
	config_t *p_cfg;
	uint64_t frame_count;
	double frame_rate;
	int sound_rate;
	bool now_opening;
	QString _filename;
	
	bool use_hwaccel;
	QString video_format;
	QString video_codec;
	QString audio_codec;
	QString hwaccel_method;

	bool now_pausing;
	bool now_playing;
	
	int src_width, src_height;
	int dst_width, dst_height;

	QList<sound_data_cueue_t *> sound_data_queue;
public:
	MOVIE_LOADER(OSD *osd, config_t *cfg);
	~MOVIE_LOADER();
	bool open(QString filename);
	bool close();
	double  get_movie_frame_rate(void);
	int get_movie_sound_rate(void);
	
	QReadWriteLock frame_lock;
	QWaitCondition lock_cond;
	
	uint64_t get_current_frame(void);
	bool is_playing(void);
	bool is_pausing(void);

public slots:
	void do_set_dst_geometry(int width, int height);
	void do_set_enable_hwaccel_decoding(bool enable);
	void do_set_enable_hwaccel_scaling(bool enable);
	void do_set_dst_pixfmt(int type);
	
	void do_play();
	void do_stop();
	void do_pause(bool flag);
//	void do_fast_forward(int ticks);
//	void do_fast_rewind(int ticks);
	void do_mute(bool left, bool right);

	void do_decode_frames(int frames);
	void do_seek_frame(bool relative, int frames);
	void do_dequeue_audio();
	
signals:
	int sig_call_sound_callback(uint8_t *, long); // Call callback.
	int sig_movie_end(bool); // MOVIE END
	int sig_movie_ready(bool); // ACK
	int sig_decoding_error(int); // error_num
};
QT_END_NAMESPACE
#endif //_QT_OSD_MOVIE_LOADER_H

