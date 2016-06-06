/*
 * Common Source Code Project for Qt : movie saver.
 * (C) 2016 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  License: GPLv2
 *  History: May 27, 2016 : Initial. This refer from avidemux 2.5.6 .
 */

#include <QDateTime>
#include "common.h"
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

/**************************************************************/
/* video output */

//static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
void *MOVIE_SAVER::alloc_picture(uint64_t _pix_fmt, int width, int height)
{
	enum AVPixelFormat pix_fmt = (enum AVPixelFormat)_pix_fmt;
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return (void *)NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not allocate frame data.\n");
        return (void *)NULL;
    }

    return (void *)picture;
}

//void MOVIE_SAVER::open_video(OutputStream *_ost, AVDictionary *_opt_arg)
bool MOVIE_SAVER::open_video()
{
    int ret;
	AVDictionary *opt_arg = raw_options_list;
	OutputStream *ost = &video_st;
	AVCodec *codec = video_codec;
	AVFormatContext *oc = output_context;
    AVCodecContext *c = ost->st->codec;
    AVDictionary *opt = NULL;

    av_dict_copy(&opt, opt_arg, 0);

    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not open video codec: %s\n", err2str(ret).toLocal8Bit().constData());
		return false;
    }

    /* allocate and init a re-usable frame */
    ost->frame = (AVFrame *)alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Could not allocate video frame\n");
        return false;
    }

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
#if 1	
	old_width = old_height = -1;
    //if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
	ost->tmp_frame = (AVFrame *)alloc_picture(AV_PIX_FMT_RGBA, _width, _height);
	if (!ost->tmp_frame) {
		fprintf(stderr, "Could not allocate temporary picture\n");
		return false;
	}
	//}
#endif	
	AGAR_DebugLog(AGAR_LOG_DEBUG, "MOVIE: Open to write video : Success.");
	return true;
}

static void fill_yuv_image(AVFrame *pict, int frame_index,
                           int width, int height)
{
    int x, y, i, ret;

    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally;
     * make sure we do not overwrite it here
     */
    ret = av_frame_make_writable(pict);
    if (ret < 0)
        exit(1);

    i = frame_index;

    /* Y */
    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;

    /* Cb and Cr */
    for (y = 0; y < height / 2; y++) {
        for (x = 0; x < width / 2; x++) {
            pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
            pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
        }
    }
}

//static AVFrame *get_video_frame(OutputStream *ost)
void *MOVIE_SAVER::get_video_frame(void)
{
	OutputStream *ost = &video_st;
    AVCodecContext *c = ost->st->codec;

    //if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        /* as we only generate a YUV420P picture, we must convert it
         * to the codec pixel format if needed */
	if((old_width != _width) || (old_height != _height)) {
		if(ost->sws_ctx != NULL) { 
			sws_freeContext(ost->sws_ctx);
			ost->sws_ctx = NULL;
		}
		if(ost->tmp_frame != NULL) {
			av_frame_free(&ost->tmp_frame);
			ost->tmp_frame = (AVFrame *)alloc_picture(AV_PIX_FMT_BGRA, _width, _height);
			if (ost->tmp_frame == NULL) {
				AGAR_DebugLog(AGAR_LOG_INFO, "Could not re-allocate video frame\n");
				return (void *)NULL;
			}
		}
    }
	
	old_height = _height;
	old_width = _width;
	
	if (!ost->sws_ctx) {
		ost->sws_ctx = sws_getContext(ost->tmp_frame->width, ost->tmp_frame->height,
									  AV_PIX_FMT_BGRA,
									  c->width, c->height,
									  c->pix_fmt,
									  SCALE_FLAGS, NULL, NULL, NULL);
		if (!ost->sws_ctx) {
			AGAR_DebugLog(AGAR_LOG_INFO,
					"Could not initialize the conversion context\n");
			return (void *)NULL;
		}
	}
	const int in_linesize[1] = { 4 * _width };
	//fill_yuv_image(ost->tmp_frame, ost->next_pts, c->width, c->height);
	{
		int w = _width;
		int h = _height;
		int x;
		int y;
		int yy[4];
		uint32_t *p;
		AVFrame *qq = ost->tmp_frame;
		uint32_t *q;
		int i;
		int ret;
		if(ost->tmp_frame != NULL) {
			ret = av_frame_make_writable(ost->tmp_frame);
			//if (ret < 0)
			//exit(1);
			for(y = 0; y < h; y++) {
				yy[0] = y * qq->linesize[0];
				//yy[0] = y * in_linesize[0];
				p = (uint32_t *)(&(video_frame_buf[y * w]));
				q = (uint32_t *)(&(qq->data[0][yy[0]]));
				for(x = 0; x < w; x++) {
					*q++ = *p++;
				}			
			}
		}
		sws_scale(ost->sws_ctx,
				  ost->tmp_frame->data, ost->tmp_frame->linesize,
				  0, _height, ost->frame->data, ost->frame->linesize);
	}
    ost->frame->pts = ost->next_pts++;
    return (void *)(ost->frame);
}

/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
//static int write_video_frame(AVFormatContext *oc, OutputStream *ost)
int MOVIE_SAVER::write_video_frame()
{
    int ret;
	AVFormatContext *oc = output_context;
	OutputStream *ost = &video_st;
    AVCodecContext *c;
    AVFrame *frame;
    int got_packet = 0;
    AVPacket pkt = { 0 };

    c = ost->st->codec;

    frame = (AVFrame *)get_video_frame();
    av_init_packet(&pkt);

    /* encode the image */
    ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Error encoding video frame: %s\n", err2str(ret).toLocal8Bit().constData());
        return -1;
    }
	totalDstFrame++;

    if (got_packet) {
        ret = this->write_frame((void *)oc, (const void *)&c->time_base, (void *)ost->st, (void *)&pkt);
		// ret = write_frame(oc, &c->time_base, ost->st, &pkt);
    } else {
        ret = 0;
    }

    if (ret < 0) {
        AGAR_DebugLog(AGAR_LOG_INFO, "Error while writing video frame: %s\n", err2str(ret).toLocal8Bit().constData());
        return -1;
    }
	
    return (frame || got_packet) ? 0 : 1;
}
