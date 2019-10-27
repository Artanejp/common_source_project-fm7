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
	#if (LIBAVCODEC_VERSION_MAJOR > 56)
	#define AVCODEC_UPPER_V56
	#endif
#endif
/**************************************************************/
/* video output */

void MOVIE_SAVER::setup_h264(void *_codec_context)
{
#if defined(USE_LIBAV)
	AVCodecContext *c = (AVCodecContext *)_codec_context;

	c->qmin	 = p_config->video_h264_minq;
	c->qmax	 = p_config->video_h264_maxq;
	c->bit_rate = p_config->video_h264_bitrate * 1000;
	c->max_b_frames = p_config->video_h264_bframes;
	c->b_quant_offset = 2;
	c->temporal_cplx_masking = 0.1;
	c->spatial_cplx_masking = 0.15;
	c->bidir_refine = 2;
	c->refs = 5;
	c->max_qdiff = 6;
	c->me_subpel_quality = p_config->video_h264_subme;
	c->i_quant_offset = 1.2;
	c->i_quant_factor = 1.5;
	c->trellis = 2;
	c->mb_lmin = 4;
	c->mb_lmax = 18;
	c->bidir_refine = 2;
	c->keyint_min = rec_fps / 5;
	c->gop_size = rec_fps * 5;
	/* Needed to avoid using macroblocks in which some coeffs overflow.
	 * This does not happen with normal video, it just happens here as
	 * the motion of the chroma plane does not match the luma plane. */
	c->mb_decision = 2;
	c->profile=FF_PROFILE_H264_HIGH;
#ifdef AVCODEC_UPPER_V56
	av_dict_set(&raw_options_list, _T("me_method"), _T("umh"), 0);
	av_dict_set_int(&raw_options_list, _T("qmin"), p_config->video_h264_minq, 0);
	av_dict_set_int(&raw_options_list, _T("qmax"), p_config->video_h264_maxq, 0);
	av_dict_set_int(&raw_options_list, _T("sc_threshold"), 40, 0);
	av_dict_set_int(&raw_options_list, _T("noise_reduction"), 0, 0);
	av_dict_set_int(&raw_options_list, _T("chromaoffset"), 2, 0);	
	av_dict_set_int(&raw_options_list, _T("b_strategy"), p_config->video_h264_b_adapt, 0);	
#else
	c->scenechange_threshold = 40;
	c->noise_reduction = 0;
	c->chromaoffset = 2;
	c->b_frame_strategy = p_config->video_h264_b_adapt;
	c->b_sensitivity = 55;
	c->me_method = ME_UMH;
#endif
#endif
}

void MOVIE_SAVER::setup_mpeg4(void *_codec)
{
#if defined(USE_LIBAV)	
	AVCodecContext *c = (AVCodecContext *)_codec;
	c->bidir_refine = 2;
	c->refs = 5;
	c->max_qdiff = 6;
	c->i_quant_offset = 1.2;
	c->i_quant_factor = 1.5;
	c->trellis = 2;
	c->mb_lmin = 4;
	c->mb_lmax = 18;
	c->bidir_refine = 2;
	c->keyint_min = rec_fps / 5;
	c->gop_size = rec_fps * 5;
	c->gop_size  = rec_fps; /* emit one intra frame every one second */
	c->qmin	 = p_config->video_mpeg4_minq;
	c->qmax	 = p_config->video_mpeg4_maxq;
	if(c->qmin > c->qmax) {
		int tmp;
		tmp = c->qmin;
		c->qmin = c->qmax;
		c->qmax = tmp;
	}
	if(c->qmin <= 0) c->qmin = 1;
	if(c->qmax <= 0) c->qmax = 1;
	c->max_b_frames	 = p_config->video_mpeg4_bframes;
	c->bit_rate = p_config->video_mpeg4_bitrate * 1000;
	c->b_quant_offset = 2;
	c->temporal_cplx_masking = 0.1;
	c->spatial_cplx_masking = 0.15;
#ifdef AVCODEC_UPPER_V56
	av_dict_set_int(&raw_options_list, _T("sc_threshold"), 30, 0);
	av_dict_set_int(&raw_options_list, _T("quantizer_noise_shaping"), 0, 0);
	av_dict_set_int(&raw_options_list, _T("b_strategy"), 1, 0);
	av_dict_set_int(&raw_options_list, _T("b_sensitivity"), 55, 0);
#else
	c->scenechange_threshold = 30;
	c->noise_reduction = 0;
	c->chromaoffset = 2;
//	c->b_strategy = 1;
	c->b_sensitivity = 55;
#endif
#endif
}
//static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
void *MOVIE_SAVER::alloc_picture(uint64_t _pix_fmt, int width, int height)
{
#if defined(USE_LIBAV)
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
		p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not allocate frame data.\n");
		return (void *)NULL;
	}

	return (void *)picture;
#else
	return (void *)NULL;
#endif	
}

//void MOVIE_SAVER::open_video(OutputStream *_ost, AVDictionary *_opt_arg)
bool MOVIE_SAVER::open_video()
{
#if defined(USE_LIBAV)
	int ret;
	AVDictionary *opt_arg = raw_options_list;
	OutputStream *ost = &video_st;
	AVCodec *codec = video_codec;
	AVCodecContext *c;
	AVDictionary *opt = NULL;
#ifdef AVCODEC_UPPER_V56
	c = ost->context;
#else
	c = ost->st->codec;
#endif

	av_dict_copy(&opt, opt_arg, 0);

	/* open the codec */
	ret = avcodec_open2(c, codec, &opt);
	av_dict_free(&opt);
	if (ret < 0) {
		p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not open video codec: %s\n", err2str(ret).toLocal8Bit().constData());
		return false;
	}

	/* allocate and init a re-usable frame */
	ost->frame = (AVFrame *)alloc_picture(c->pix_fmt, c->width, c->height);
	if (!ost->frame) {
		p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not allocate video frame\n");
		return false;
	}

	/* If the output format is not YUV420P, then a temporary YUV420P
	 * picture is needed too. It is then converted to the required
	 * output format. */
	ost->tmp_frame = NULL;
	old_width = old_height = -1;
	//if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
	ost->tmp_frame = (AVFrame *)alloc_picture(AV_PIX_FMT_RGBA, _width, _height);
	if (!ost->tmp_frame) {
		p_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER,  "Could not allocate temporary picture\n");
		return false;
	}
	//}
#ifdef AVCODEC_UPPER_V56
	ret = avcodec_parameters_from_context(ost->st->codecpar, c);
	if (ret < 0) {
		p_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER,  "Could not copy the stream parameters\n");
		return false;
	}
#endif
	p_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_MOVIE_SAVER,
						  "MOVIE: Open to write video : Success.");
	return true;
#else
	return false;
#endif
}


//static AVFrame *get_video_frame(OutputStream *ost)
void *MOVIE_SAVER::get_video_frame(void)
{
#if defined(USE_LIBAV)
	OutputStream *ost = &video_st;
	AVCodecContext *c;
#ifdef AVCODEC_UPPER_V56
	c = ost->context;
#else
	c = ost->st->codec;
#endif

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
			ost->tmp_frame = (AVFrame *)alloc_picture(AV_PIX_FMT_RGBA, _width, _height);
			if (ost->tmp_frame == NULL) {
				p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Could not re-allocate video frame\n");
				return (void *)NULL;
			}
		}
	}
	
	old_height = _height;
	old_width = _width;
	
	if (!ost->sws_ctx) {
		ost->sws_ctx = sws_getContext(ost->tmp_frame->width, ost->tmp_frame->height,
									  AV_PIX_FMT_RGBA,
									  c->width, c->height,
									  c->pix_fmt,
									  SCALE_FLAGS, NULL, NULL, NULL);
		if (!ost->sws_ctx) {
			p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER,
					"Could not initialize the conversion context\n");
			return (void *)NULL;
		}
	}
	//const int in_linesize[1] = { 4 * _width };
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
		uint32_t cacheline[8];
		//int ret;
		if(ost->tmp_frame != NULL) {
			//ret = av_frame_make_writable(ost->tmp_frame);
			av_frame_make_writable(ost->tmp_frame);
			//if (ret < 0)
			//exit(1);
			for(y = 0; y < h; y++) {
				yy[0] = y * qq->linesize[0];
				p = (uint32_t *)(&(video_frame_buf[y * w]));
				q = (uint32_t *)(&(qq->data[0][yy[0]]));
				for(x = 0; x < (w / 8); x++) {
					cacheline[0] = p[0];
					cacheline[1] = p[1];
					cacheline[2] = p[2];
					cacheline[3] = p[3];
					cacheline[4] = p[4];
					cacheline[5] = p[5];
					cacheline[6] = p[6];
					cacheline[7] = p[7];

					q[0] = cacheline[0];
					q[1] = cacheline[1];
					q[2] = cacheline[2];
					q[3] = cacheline[3];
					q[4] = cacheline[4];
					q[5] = cacheline[5];
					q[6] = cacheline[6];
					q[7] = cacheline[7];

					q += 8;
					p += 8;
				}
				if((w & 7) != 0) {
					for(x = 0; x < (w & 7); x++) {
						q[x] = p[x];
					}
				}
			}
		}
		sws_scale(ost->sws_ctx,
				  ost->tmp_frame->data, ost->tmp_frame->linesize,
				  0, _height, ost->frame->data, ost->frame->linesize);
	}
	ost->frame->pts = ost->next_pts++;
	return (void *)(ost->frame);
#else
	return (void *)NULL;
#endif	
}

/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
//static int write_video_frame(AVFormatContext *oc, OutputStream *ost)
int MOVIE_SAVER::write_video_frame()
{
#if defined(USE_LIBAV)
	int ret;
	AVFormatContext *oc = output_context;
	OutputStream *ost = &video_st;
	AVCodecContext *c;
	AVFrame *frame;
	int got_packet = 0;
	AVPacket pkt = { 0 };

#ifdef AVCODEC_UPPER_V56
	c = ost->context;
#else
	c = ost->st->codec;
#endif

	frame = (AVFrame *)get_video_frame();
	av_init_packet(&pkt);
	/* encode the image */
	frame->pts = totalDstFrame;
	//got_packet = 1;
	//while(got_packet) {
#ifdef AVCODEC_UPPER_V56
	if(frame != NULL) {
		ret = avcodec_send_frame(c, frame);
		if(ret < 0) {
			return -1;
			if(ret == AVERROR_EOF) {	
				return 1;
				//return 0;
			}
			return ((ret == AVERROR(EAGAIN)) ? 0 : -1);
		}
		totalDstFrame++;
		while (ret  >= 0) {
			ret = avcodec_receive_packet(c, &pkt);
			if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				return (frame || got_packet) ? 0 : 1;
			} else if (ret < 0) {
				p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error while writing video frame: %s\n",
									err2str(ret).toLocal8Bit().constData());
				return -1;
			}
			pkt.stream_index = 0;
			int ret2 = this->write_frame((void *)oc, (const void *)&c->time_base, (void *)ost->st, (void *)&pkt);
			if (ret2 < 0) {
				p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error while writing video frame: %s\n", err2str(ret2).toLocal8Bit().constData());
				return -1;
			}
			got_packet = 1;
			av_packet_unref(&pkt);
		}
	}
	return (frame || got_packet) ? 0 : 1;
#else
	ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
	if (ret < 0) {
		p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error encoding video frame: %s\n", err2str(ret).toLocal8Bit().constData());
		return -1;
	}
	//if(!got_packet) break;
	totalDstFrame++;
	if (got_packet) {
		ret = this->write_frame((void *)oc, (const void *)&c->time_base, (void *)ost->st, (void *)&pkt);
		
		// ret = write_frame(oc, &c->time_base, ost->st, &pkt);
	} else {
		ret = 0;
	}
	if (ret < 0) {
		p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_MOVIE_SAVER, "Error while writing video frame: %s\n", err2str(ret).toLocal8Bit().constData());
		return -1;
	}
	
	//}
	//if(frame) {
	//	char buf[256];
	//	memset(buf, 0x00, sizeof(buf));
	//	char *s = av_ts_make_time_string(buf, frame->pts, &c->time_base);
	//	AGAR_DebugLog(AGAR_LOG_DEBUG, "Movie: Write video to file. sec=%s", s);
	//}
	return (frame || got_packet) ? 0 : 1;
#endif
#else
	return 1;
#endif
}
