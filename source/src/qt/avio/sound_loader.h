/*
 * Common Source Code Project for Qt : movie saver.
 * (C) 2016 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  License: GPLv2
 *  History: Oct 02, 2016 : Initial. 
 */

#ifndef _QT_OSD_SOUND_LOADER_H
#define _QT_OSD_SOUND_LOADER_H

#include <QString>

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

#include "config.h"
class CSP_Logger;
class DLL_PREFIX SOUND_LOADER
{
private:
	CSP_Logger *p_logger;
#if defined(USE_LIBAV)
	AVFormatContext *fmt_ctx; // = NULL;
	AVCodecContext *audio_dec_ctx;
	AVStream *audio_stream; // NULL
	
	int audio_stream_idx; //int video_stream_idx = -1, audio_stream_idx = -1;
	AVFrame *frame; //AVFrame *frame = NULL;
	AVPacket *packet;
	struct SwrContext *swr_context;
	int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt);
	int open_codec_context(int *stream_idx, AVFormatContext *fmt_ctx, AVCodecContext **ctx, enum AVMediaType type);
	int decode_packet(int *got_frame, int cached);
	int decode_audio(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame, int *got_frame);
#endif
	int sound_rate;
	bool _opened;
protected:
	QString _filename;
	int _dst_size;
	int _dataptr;
	int _data_size;
	uint8_t* _data[4];
	int16_t *sound_buffer;
	
	void *prev_loader;
	int this_id;
public:
	SOUND_LOADER(void *prev_sound_loader, CSP_Logger *logger);
	~SOUND_LOADER();
	
	bool open(int id, QString filename);
	void close();
	int do_decode_frames(void);

	const int16_t *get_sound_buffer(void);
	const int get_id(void);
	void set_id(int id);
	const void *get_prev_sound_loader(void);
	const int get_dst_size(void);
	void set_sound_rate(int rate);
	
	void free_sound_buffer(int16_t *p);

	template <class... Args>
		void out_debug_log(int type, int subtype, Args... args)
	{
		__LIKELY_IF(p_logger != nullptr) {
			p_logger->debug_log(type, subtype, args...);
		}
	}

};
#endif
