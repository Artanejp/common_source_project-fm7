/*
 * Common Source Code Project for Qt : movie saver.
 * (C) 2016 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  License: GPLv2
 *  History: Oct 02, 2016 : Initial. 
 */

#include "csp_logger.h"
#include "sound_loader.h"
#include "../osd_base.h"

SOUND_LOADER::SOUND_LOADER(void *prev_sound_loader)
{
	_filename.clear();
	_dst_size = _dataptr = 0;
	_data[0] = _data[1] = _data[2] = _data[3] = NULL;
	sound_buffer = NULL;
	_data_size = 0;
	sound_rate = 48000;
	this_id = -1;
	prev_loader = prev_sound_loader;
#if defined(USE_LIBAV)
	fmt_ctx = NULL;
	audio_dec_ctx = NULL;
	swr_context = NULL;
	audio_stream = NULL;
	audio_stream_idx = -1;
	frame = NULL;
#endif
}

SOUND_LOADER::~SOUND_LOADER()
{
	if(sound_buffer != NULL) free(sound_buffer);
	for(int i = 0; i < 4; i++) {
		if(_data[i] != NULL) free(_data);
	}
}

bool SOUND_LOADER::open(int id, QString filename)
{
	int ret = 0;
	_filename = filename;
	if(filename.isEmpty()) {
		return false;
	}
	_dst_size = 0;
	_data_size = 0;
	_dataptr = 0;
	/* register all formats and codecs */
	av_register_all();
	
	/* open input file, and allocate format context */
	if (avformat_open_input(&fmt_ctx, _filename.toLocal8Bit().constData(), NULL, NULL) < 0) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Could not open source file %s\n", _filename.toLocal8Bit().constData());
        return false;
	}
	this_id = id;
	/* retrieve stream information */
	if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Could not find stream information\n");
		return false;
	}
	if (open_codec_context(&audio_stream_idx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
		audio_stream = fmt_ctx->streams[audio_stream_idx];
		audio_dec_ctx = audio_stream->codec;
		//sound_rate = audio_stream->codec->sample_rate;
	}
	swr_context = swr_alloc();
	if(swr_context == NULL) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Could not allocate resampler context\n");
		goto _end;
	}
	av_opt_set_int	   (swr_context, "in_channel_count",   audio_stream->codec->channels,	   0);
	av_opt_set_int	   (swr_context, "in_sample_rate",	   audio_stream->codec->sample_rate,	0);
	av_opt_set_sample_fmt(swr_context, "in_sample_fmt",	   audio_stream->codec->sample_fmt, 0);
	av_opt_set_int	   (swr_context, "out_channel_count",  2,	   0);
	av_opt_set_int	   (swr_context, "out_sample_rate",	   sound_rate,	0);
	av_opt_set_sample_fmt(swr_context, "out_sample_fmt",   AV_SAMPLE_FMT_S16,	 0);
	
	/* initialize the resampling context */
	if ((ret = swr_init(swr_context)) < 0) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Failed to initialize the resampling context\n");
		goto _end;
	}
	
	/* dump input information to stderr */
	av_dump_format(fmt_ctx, 0, _filename.toLocal8Bit().constData(), 0);
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Audio is %d Hz ", sound_rate);
	if (!audio_stream) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Could not find audio or video stream in the input, aborting\n");
		ret = 1;
		goto _end;
	}
	frame = av_frame_alloc();
	if (!frame) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Could not allocate frame\n");
		ret = AVERROR(ENOMEM);
		goto _end;
	}
	
	/* initialize packet, set data to NULL, let the demuxer fill it */
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	// ToDo : Initialize SWScaler and SWresampler.
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "SOUND_LOADER: Loading movie completed.\n");
	return true;
_end:
	this->close();
	return false;
}

void SOUND_LOADER::close(void)
{
#if defined(USE_LIBAV)
	if(audio_dec_ctx != NULL) avcodec_close(audio_dec_ctx);
	avformat_close_input(&fmt_ctx);
	swr_free(&swr_context);
	
	fmt_ctx = NULL;
	audio_dec_ctx = NULL;
	swr_context = NULL;
	audio_stream = NULL;
	audio_stream_idx = -1;
	frame = NULL;
#endif
	for(int i = 0; i < 4; i++) {
		if(_data[i] != NULL) free(_data[i]);
		_data[i] = NULL;
	}
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "SOUND_LOADER: Close sound.");
}

#if defined(USE_LIBAV)	
int SOUND_LOADER::decode_packet(int *got_frame, int cached)
{
	int ret = 0;
	int decoded = pkt.size;

	if (pkt.stream_index == audio_stream_idx) {
		ret = avcodec_decode_audio4(audio_dec_ctx, frame, got_frame, &pkt);
		if (ret < 0) {
			char str_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
			av_make_error_string(str_buf, AV_ERROR_MAX_STRING_SIZE, ret);
			csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Error decoding audio frame (%s)\n", str_buf);
			return ret;
		}
		/* Some audio decoders decode only part of the packet, and have to be
		 * called again with the remainder of the packet data.
		 * Sample: fate-suite/lossless-audio/luckynight-partial.shn
		 * Also, some decoders might over-read the packet. */
		decoded = FFMIN(ret, pkt.size);
			
		if (*got_frame) {
			//size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((enum AVSampleFormat)frame->format);
			//char str_buf[AV_TS_MAX_STRING_SIZE] = {0};
			AVCodecContext *c = audio_stream->codec;
			int dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_context, c->sample_rate) + frame->nb_samples,
												c->sample_rate, c->sample_rate,  AV_ROUND_UP);
			//av_ts_make_time_string(str_buf, frame->pts, &audio_dec_ctx->time_base);
			//csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_SOUND_LOADER,"audio_frame%s n:%d nb_samples:%d pts:%s\n",
			//			  cached ? "(cached)" : "",
			//			  audio_frame_count++, frame->nb_samples,
			//			  str_buf);
			/* Write the raw audio data samples of the first plane. This works
			 * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
			 * most audio decoders output planar audio, which uses a separate
			 * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
			 * In other words, this code will write only the first audio channel
			 * in these cases.
			 * You should use libswresample or libavfilter to convert the frame
			 * to packed data. */
			_data[0] = (uint8_t *)malloc((long)dst_nb_samples * 2 * sizeof(int16_t));
			_data[1] = _data[2] = _data[3] = NULL;
			if(_data[0] == NULL) {
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Error allocating local buffers\n");
				return -1;
			}
				
			ret = swr_convert(swr_context,
							  _data, dst_nb_samples,
							  (const uint8_t **)frame->data, frame->nb_samples);
			if (ret < 0) {
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Error while converting\n");
				return -1;
			}
			_dst_size += dst_nb_samples;
			if(_data_size <= (uint)(_dst_size * 2 * sizeof(int16_t))) {
				sound_buffer = (int16_t *)realloc(sound_buffer, _data_size << 1);
				if(sound_buffer == NULL) {
					csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Error re-allocate sound buffer");
					if(_data[0] != NULL) free(_data[0]);
					_data[0] = NULL;
					return -1;
				}
				_data_size <<= 1;
			}
			memcpy(&sound_buffer[_dataptr], _data[0], (long)dst_nb_samples * 2 * sizeof(int16_t));
			_dataptr += (dst_nb_samples * 2);
			free(_data[0]);
			_data[0] = NULL;
		}
	} else {
		// Video
		return 0;
	}
	/* If we use frame reference counting, we own the data and need
	 * to de-reference it when we don't use it anymore */
	if (*got_frame)
		av_frame_unref(frame);
	return decoded;
}

int SOUND_LOADER::open_codec_context(int *stream_idx,
									 AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    AVCodecContext *dec_ctx = NULL;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), _filename.toLocal8Bit().constData());
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec_ctx = st->codec;
        dec = avcodec_find_decoder(dec_ctx->codec_id);
        if (!dec) {
            csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Init the decoders, with or without reference counting */
        av_dict_set(&opts, "refcounted_frames", "1", 0);
        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
            csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

int SOUND_LOADER::get_format_from_sample_fmt(const char **fmt,
											 enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *fmt = NULL;

    for (i = 0; i < (int)FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND_LOADER,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}
#endif	

int SOUND_LOADER::do_decode_frames(void)
{
	int got_frame = 1;
	int decoded;
	int ret;
	if(sound_buffer != NULL) {
		free(sound_buffer);
		sound_buffer = NULL;
	}
#if defined(USE_LIBAV)	
	_data_size = 65536 * 2 * sizeof(int16_t);
	sound_buffer = (int16_t *)malloc(_data_size);
	if(sound_buffer == NULL) {
		return -1;
	}
	_dst_size = 0;
	_dataptr = 0;
	do {
		ret = av_read_frame(fmt_ctx, &pkt);
		if(ret < 0) break;
		decoded = decode_packet(&got_frame, 0);
	} while((ret == 0) && (decoded >= 0));
	return _dst_size;
#else
	return 0;
#endif
}

const int16_t* SOUND_LOADER::get_sound_buffer(void)
{
	return (const int16_t *)sound_buffer;
}

const int SOUND_LOADER::get_id(void)
{
	return this_id;
}

void SOUND_LOADER::set_id(int id)
{
	this_id = id;
}

const void *SOUND_LOADER::get_prev_sound_loader(void)
{
	return (const void *)prev_loader;
}

const int SOUND_LOADER::get_dst_size(void)
{
	return (const int)_dst_size;
}

void SOUND_LOADER::free_sound_buffer(int16_t *p)
{
	if(p == NULL) {
		if(sound_buffer != NULL) free(sound_buffer);
		sound_buffer = NULL;
		this_id = -1;
		_dst_size = 0;
		_dataptr = 0;
		_data_size = 0;
		return;
	}
	if(p == sound_buffer) {
		free(sound_buffer);
		sound_buffer = NULL;
		this_id = -1;
		_dst_size = 0;
		_dataptr = 0;
		_data_size = 0;
	}
}

void SOUND_LOADER::set_sound_rate(int rate)
{
	sound_rate = rate;
}

