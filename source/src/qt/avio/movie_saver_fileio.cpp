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


#define GROW_ARRAY(array, nb_elems)\
    array = grow_array(array, sizeof(*array), &nb_elems, nb_elems + 1)

#define MATCH_PER_STREAM_OPT(name, type, outvar, fmtctx, st)\
{\
    int i, ret;\
    for (i = 0; i < o->nb_ ## name; i++) {\
        char *spec = o->name[i].specifier;\
        if ((ret = check_stream_specifier(fmtctx, st, spec)) > 0)\
            outvar = o->name[i].u.type;\
        else if (ret < 0)\
            exit_program(1);\
    }\
}

#define MATCH_PER_TYPE_OPT(name, type, outvar, fmtctx, mediatype)\
{\
    int i;\
    for (i = 0; i < o->nb_ ## name; i++) {\
        char *spec = o->name[i].specifier;\
        if (!strcmp(spec, mediatype))\
            outvar = o->name[i].u.type;\
    }\
}

void *MOVIE_SAVER::grow_array(void *array, int elem_size, int *size, int new_size)
{
    if (new_size >= INT_MAX / elem_size) {
        av_log(NULL, AV_LOG_ERROR, "Array too big.\n");
        exit_program(1);
    }
    if (*size < new_size) {
        uint8_t *tmp = av_realloc_array(array, new_size, elem_size);
        if (!tmp) {
            av_log(NULL, AV_LOG_ERROR, "Could not alloc buffer.\n");
            exit_program(1);
        }
        memset(tmp + *size*elem_size, 0, (new_size-*size) * elem_size);
        *size = new_size;
        return tmp;
    }
    return array;
}


// From ffmpeg 3.0.2
static OutputStream *MOVIE_SAVER::new_output_stream(OptionsContext *o, AVFormatContext *oc, enum AVMediaType type, int source_index)
{
    OutputStream *ost;
    AVStream *st = avformat_new_stream(oc, NULL);
    int idx      = oc->nb_streams - 1, ret = 0;
    char *bsf = NULL, *next, *codec_tag = NULL;
    AVBitStreamFilterContext *bsfc, *bsfc_prev = NULL;
    double qscale = -1;
    int i;

    if (!st) {
        av_log(NULL, AV_LOG_FATAL, "Could not alloc stream.\n");
        exit_program(1);
    }
	//FIXME
    if (oc->nb_streams - 1 < o->nb_streamid_map)
        st->id = o->streamid_map[oc->nb_streams - 1];

    GROW_ARRAY(output_streams, nb_output_streams);
    if (!(ost = av_mallocz(sizeof(*ost))))
        exit_program(1);
    output_streams[nb_output_streams - 1] = ost;

    ost->file_index = nb_output_files - 1;
    ost->index      = idx;
    ost->st         = st;
    st->codec->codec_type = type;
    choose_encoder(o, oc, ost);

    ost->enc_ctx = avcodec_alloc_context3(ost->enc);
    if (!ost->enc_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Error allocating the encoding context.\n");
        exit_program(1);
    }
    ost->enc_ctx->codec_type = type;

    if (ost->enc) {
        AVIOContext *s = NULL;
        char *buf = NULL, *arg = NULL, *preset = NULL;
		//FIXME
        ost->encoder_opts  = filter_codec_opts(o->g->codec_opts, ost->enc->id, oc, st, ost->enc);

        MATCH_PER_STREAM_OPT(presets, str, preset, oc, st);
        if (preset && (!(ret = get_preset_file_2(preset, ost->enc->name, &s)))) {
            do  {
                buf = get_line(s);
                if (!buf[0] || buf[0] == '#') {
                    av_free(buf);
                    continue;
                }
                if (!(arg = strchr(buf, '='))) {
                    av_log(NULL, AV_LOG_FATAL, "Invalid line found in the preset file.\n");
                    exit_program(1);
                }
                *arg++ = 0;
                av_dict_set(&ost->encoder_opts, buf, arg, AV_DICT_DONT_OVERWRITE);
                av_free(buf);
            } while (!s->eof_reached);
            avio_closep(&s);
        }
        if (ret) {
            av_log(NULL, AV_LOG_FATAL,
                   "Preset %s specified for stream %d:%d, but could not be opened.\n",
                   preset, ost->file_index, ost->index);
            exit_program(1);
        }
    } else {
		//FIXME
        ost->encoder_opts = filter_codec_opts(o->g->codec_opts, AV_CODEC_ID_NONE, oc, st, NULL);
    }

    ost->max_frames = INT64_MAX;
	//FIXME
    MATCH_PER_STREAM_OPT(max_frames, i64, ost->max_frames, oc, st);
    for (i = 0; i<o->nb_max_frames; i++) {
        char *p = o->max_frames[i].specifier;
        if (!*p && type != AVMEDIA_TYPE_VIDEO) {
            av_log(NULL, AV_LOG_WARNING, "Applying unspecific -frames to non video streams, maybe you meant -vframes ?\n");
            break;
        }
    }

    ost->copy_prior_start = -1;
    MATCH_PER_STREAM_OPT(copy_prior_start, i, ost->copy_prior_start, oc ,st);

    MATCH_PER_STREAM_OPT(bitstream_filters, str, bsf, oc, st);
    while (bsf) {
        char *arg = NULL;
        if (next = strchr(bsf, ','))
            *next++ = 0;
        if (arg = strchr(bsf, '='))
            *arg++ = 0;
        if (!(bsfc = av_bitstream_filter_init(bsf))) {
            av_log(NULL, AV_LOG_FATAL, "Unknown bitstream filter %s\n", bsf);
            exit_program(1);
        }
        if (bsfc_prev)
            bsfc_prev->next = bsfc;
        else
            ost->bitstream_filters = bsfc;
        if (arg)
            if (!(bsfc->args = av_strdup(arg))) {
                av_log(NULL, AV_LOG_FATAL, "Bitstream filter memory allocation failed\n");
                exit_program(1);
            }

        bsfc_prev = bsfc;
        bsf       = next;
    }

    MATCH_PER_STREAM_OPT(codec_tags, str, codec_tag, oc, st);
    if (codec_tag) {
        uint32_t tag = strtol(codec_tag, &next, 0);
        if (*next)
            tag = AV_RL32(codec_tag);
        ost->st->codec->codec_tag =
        ost->enc_ctx->codec_tag = tag;
    }

    MATCH_PER_STREAM_OPT(qscale, dbl, qscale, oc, st);
    if (qscale >= 0) {
        ost->enc_ctx->flags |= AV_CODEC_FLAG_QSCALE;
        ost->enc_ctx->global_quality = FF_QP2LAMBDA * qscale;
    }

    MATCH_PER_STREAM_OPT(disposition, str, ost->disposition, oc, st);
    ost->disposition = av_strdup(ost->disposition);

    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        ost->enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	//FIXME
    av_dict_copy(&ost->sws_dict, o->g->sws_dict, 0);
	//FIXME
    av_dict_copy(&ost->swr_opts, o->g->swr_opts, 0);
    if (ost->enc && av_get_exact_bits_per_sample(ost->enc->id) == 24)
        av_dict_set(&ost->swr_opts, "output_sample_bits", "24", 0);
	//FIXME
    av_dict_copy(&ost->resample_opts, o->g->resample_opts, 0);

    ost->source_index = source_index;
    if (source_index >= 0) {
        ost->sync_ist = input_streams[source_index];
        input_streams[source_index]->discard = 0;
        input_streams[source_index]->st->discard = input_streams[source_index]->user_set_discard;
    }
    ost->last_mux_dts = AV_NOPTS_VALUE;

    return ost;
}

static int open_output_file(OptionsContext *o, const char *filename)
{
    AVFormatContext *oc;
    int i, j, err;
    AVOutputFormat *file_oformat;
    OutputFile *of;
    OutputStream *ost;
    InputStream  *ist;
    AVDictionary *unused_opts = NULL;
    AVDictionaryEntry *e = NULL;

	//FIXME
    if (o->stop_time != INT64_MAX && o->recording_time != INT64_MAX) {
        o->stop_time = INT64_MAX;
        av_log(NULL, AV_LOG_WARNING, "-t and -to cannot be used together; using -t.\n");
    }
	//FIXME
    if (o->stop_time != INT64_MAX && o->recording_time == INT64_MAX) {
        int64_t start_time = o->start_time == AV_NOPTS_VALUE ? 0 : o->start_time;
        if (o->stop_time <= start_time) {
            av_log(NULL, AV_LOG_ERROR, "-to value smaller than -ss; aborting.\n");
            exit_program(1);
        } else {
            o->recording_time = o->stop_time - start_time;
        }
    }

    GROW_ARRAY(output_files, nb_output_files);
    of = av_mallocz(sizeof(*of));
    if (!of)
        exit_program(1);
    output_files[nb_output_files - 1] = of;
	//FIXME
    of->ost_index      = nb_output_streams;
    of->recording_time = o->recording_time;
    of->start_time     = o->start_time;
    of->limit_filesize = o->limit_filesize;
    of->shortest       = o->shortest;
    av_dict_copy(&of->opts, o->g->format_opts, 0);

    if (!strcmp(filename, "-"))
        filename = "pipe:";
	//FIXME
    err = avformat_alloc_output_context2(&oc, NULL, o->format, filename);
    if (!oc) {
        print_error(filename, err);
        exit_program(1);
    }

    of->ctx = oc;
	//FIXME
    if (o->recording_time != INT64_MAX)
        oc->duration = o->recording_time;

    file_oformat= oc->oformat;
    oc->interrupt_callback = int_cb;

    /* create streams for all unlabeled output pads */
    for (i = 0; i < nb_filtergraphs; i++) {
        FilterGraph *fg = filtergraphs[i];
        for (j = 0; j < fg->nb_outputs; j++) {
            OutputFilter *ofilter = fg->outputs[j];

            if (!ofilter->out_tmp || ofilter->out_tmp->name)
                continue;
			//FIXME
            switch (ofilter->type) {
            case AVMEDIA_TYPE_VIDEO:    o->video_disable    = 1; break;
            case AVMEDIA_TYPE_AUDIO:    o->audio_disable    = 1; break;
            case AVMEDIA_TYPE_SUBTITLE: o->subtitle_disable = 1; break;
            }
            init_output_filter(ofilter, o, oc);
        }
    }

    /* ffserver seeking with date=... needs a date reference */
    if (!strcmp(file_oformat->name, "ffm") &&
        av_strstart(filename, "http:", NULL)) {
        int err = parse_option(o, "metadata", "creation_time=now", options);
        if (err < 0) {
            print_error(filename, err);
            exit_program(1);
        }
    }

    if (!strcmp(file_oformat->name, "ffm") && !override_ffserver &&
        av_strstart(filename, "http:", NULL)) {
        int j;
        /* special case for files sent to ffserver: we get the stream
           parameters from ffserver */
        int err = read_ffserver_streams(o, oc, filename);
        if (err < 0) {
            print_error(filename, err);
            exit_program(1);
        }
        for(j = nb_output_streams - oc->nb_streams; j < nb_output_streams; j++) {
            ost = output_streams[j];
            for (i = 0; i < nb_input_streams; i++) {
                ist = input_streams[i];
                if(ist->st->codec->codec_type == ost->st->codec->codec_type){
                    ost->sync_ist= ist;
                    ost->source_index= i;
                    if(ost->st->codec->codec_type == AVMEDIA_TYPE_AUDIO) ost->avfilter = av_strdup("anull");
                    if(ost->st->codec->codec_type == AVMEDIA_TYPE_VIDEO) ost->avfilter = av_strdup("null");
                    ist->discard = 0;
                    ist->st->discard = ist->user_set_discard;
                    break;
                }
            }
            if(!ost->sync_ist){
                av_log(NULL, AV_LOG_FATAL, "Missing %s stream which is required by this ffm\n", av_get_media_type_string(ost->st->codec->codec_type));
                exit_program(1);
            }
        }
		//FIXME
    } else if (!o->nb_stream_maps) {
        char *subtitle_codec_name = NULL;
        /* pick the "best" stream of each type */

        /* video: highest resolution */
        if (!o->video_disable && av_guess_codec(oc->oformat, NULL, filename, NULL, AVMEDIA_TYPE_VIDEO) != AV_CODEC_ID_NONE) {
            int area = 0, idx = -1;
            int qcr = avformat_query_codec(oc->oformat, oc->oformat->video_codec, 0);
            for (i = 0; i < nb_input_streams; i++) {
                int new_area;
                ist = input_streams[i];
                new_area = ist->st->codec->width * ist->st->codec->height + 100000000*!!ist->st->codec_info_nb_frames;
                if((qcr!=MKTAG('A', 'P', 'I', 'C')) && (ist->st->disposition & AV_DISPOSITION_ATTACHED_PIC))
                    new_area = 1;
                if (ist->st->codec->codec_type == AVMEDIA_TYPE_VIDEO &&
                    new_area > area) {
                    if((qcr==MKTAG('A', 'P', 'I', 'C')) && !(ist->st->disposition & AV_DISPOSITION_ATTACHED_PIC))
                        continue;
                    area = new_area;
                    idx = i;
                }
            }
            if (idx >= 0)
                new_video_stream(o, oc, idx);
        }

        /* audio: most channels */
		//FIXME
        if (!o->audio_disable && av_guess_codec(oc->oformat, NULL, filename, NULL, AVMEDIA_TYPE_AUDIO) != AV_CODEC_ID_NONE) {
            int best_score = 0, idx = -1;
            for (i = 0; i < nb_input_streams; i++) {
                int score;
                ist = input_streams[i];
                score = ist->st->codec->channels + 100000000*!!ist->st->codec_info_nb_frames;
                if (ist->st->codec->codec_type == AVMEDIA_TYPE_AUDIO &&
                    score > best_score) {
                    best_score = score;
                    idx = i;
                }
            }
            if (idx >= 0)
                new_audio_stream(o, oc, idx);
        }

        /* subtitles: pick first */
        MATCH_PER_TYPE_OPT(codec_names, str, subtitle_codec_name, oc, "s");
		//FIXME
        if (!o->subtitle_disable && (avcodec_find_encoder(oc->oformat->subtitle_codec) || subtitle_codec_name)) {
            for (i = 0; i < nb_input_streams; i++)
                if (input_streams[i]->st->codec->codec_type == AVMEDIA_TYPE_SUBTITLE) {
                    AVCodecDescriptor const *input_descriptor =
                        avcodec_descriptor_get(input_streams[i]->st->codec->codec_id);
                    AVCodecDescriptor const *output_descriptor = NULL;
                    AVCodec const *output_codec =
                        avcodec_find_encoder(oc->oformat->subtitle_codec);
                    int input_props = 0, output_props = 0;
                    if (output_codec)
                        output_descriptor = avcodec_descriptor_get(output_codec->id);
                    if (input_descriptor)
                        input_props = input_descriptor->props & (AV_CODEC_PROP_TEXT_SUB | AV_CODEC_PROP_BITMAP_SUB);
                    if (output_descriptor)
                        output_props = output_descriptor->props & (AV_CODEC_PROP_TEXT_SUB | AV_CODEC_PROP_BITMAP_SUB);
                    if (subtitle_codec_name ||
                        input_props & output_props ||
                        // Map dvb teletext which has neither property to any output subtitle encoder
                        input_descriptor && output_descriptor &&
                        (!input_descriptor->props ||
                         !output_descriptor->props)) {
                        new_subtitle_stream(o, oc, i);
                        break;
                    }
                }
        }
        /* Data only if codec id match */
		//FIXME
        if (!o->data_disable ) {
            enum AVCodecID codec_id = av_guess_codec(oc->oformat, NULL, filename, NULL, AVMEDIA_TYPE_DATA);
            for (i = 0; codec_id != AV_CODEC_ID_NONE && i < nb_input_streams; i++) {
                if (input_streams[i]->st->codec->codec_type == AVMEDIA_TYPE_DATA
                    && input_streams[i]->st->codec->codec_id == codec_id )
                    new_data_stream(o, oc, i);
            }
        }
    } else {
		//FIXME
        for (i = 0; i < o->nb_stream_maps; i++) {
            StreamMap *map = &o->stream_maps[i];

            if (map->disabled)
                continue;

            if (map->linklabel) {
                FilterGraph *fg;
                OutputFilter *ofilter = NULL;
                int j, k;

                for (j = 0; j < nb_filtergraphs; j++) {
                    fg = filtergraphs[j];
                    for (k = 0; k < fg->nb_outputs; k++) {
                        AVFilterInOut *out = fg->outputs[k]->out_tmp;
                        if (out && !strcmp(out->name, map->linklabel)) {
                            ofilter = fg->outputs[k];
                            goto loop_end;
                        }
                    }
                }
loop_end:
                if (!ofilter) {
                    av_log(NULL, AV_LOG_FATAL, "Output with label '%s' does not exist "
                           "in any defined filter graph, or was already used elsewhere.\n", map->linklabel);
                    exit_program(1);
                }
                init_output_filter(ofilter, o, oc);
            } else {
                int src_idx = input_files[map->file_index]->ist_index + map->stream_index;

                ist = input_streams[input_files[map->file_index]->ist_index + map->stream_index];
				//FIXME
                if(o->subtitle_disable && ist->st->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
                    continue;
                if(o->   audio_disable && ist->st->codec->codec_type == AVMEDIA_TYPE_AUDIO)
                    continue;
                if(o->   video_disable && ist->st->codec->codec_type == AVMEDIA_TYPE_VIDEO)
                    continue;
                if(o->    data_disable && ist->st->codec->codec_type == AVMEDIA_TYPE_DATA)
                    continue;

                ost = NULL;
                switch (ist->st->codec->codec_type) {
                case AVMEDIA_TYPE_VIDEO:      ost = new_video_stream     (o, oc, src_idx); break;
                case AVMEDIA_TYPE_AUDIO:      ost = new_audio_stream     (o, oc, src_idx); break;
                case AVMEDIA_TYPE_SUBTITLE:   ost = new_subtitle_stream  (o, oc, src_idx); break;
                case AVMEDIA_TYPE_DATA:       ost = new_data_stream      (o, oc, src_idx); break;
                case AVMEDIA_TYPE_ATTACHMENT: ost = new_attachment_stream(o, oc, src_idx); break;
                case AVMEDIA_TYPE_UNKNOWN:
                    if (copy_unknown_streams) {
                        ost = new_unknown_stream   (o, oc, src_idx);
                        break;
                    }
                default:
                    av_log(NULL, ignore_unknown_streams ? AV_LOG_WARNING : AV_LOG_FATAL,
                           "Cannot map stream #%d:%d - unsupported type.\n",
                           map->file_index, map->stream_index);
                    if (!ignore_unknown_streams) {
                        av_log(NULL, AV_LOG_FATAL,
                               "If you want unsupported types ignored instead "
                               "of failing, please use the -ignore_unknown option\n"
                               "If you want them copied, please use -copy_unknown\n");
                        exit_program(1);
                    }
                }
                if (ost)
                    ost->sync_ist = input_streams[  input_files[map->sync_file_index]->ist_index
                                                  + map->sync_stream_index];
            }
        }
    }

    /* handle attached files */
	//FIXME
    for (i = 0; i < o->nb_attachments; i++) {
        AVIOContext *pb;
        uint8_t *attachment;
        const char *p;
        int64_t len;

        if ((err = avio_open2(&pb, o->attachments[i], AVIO_FLAG_READ, &int_cb, NULL)) < 0) {
            av_log(NULL, AV_LOG_FATAL, "Could not open attachment file %s.\n",
                   o->attachments[i]);
            exit_program(1);
        }
        if ((len = avio_size(pb)) <= 0) {
            av_log(NULL, AV_LOG_FATAL, "Could not get size of the attachment %s.\n",
                   o->attachments[i]);
            exit_program(1);
        }
        if (!(attachment = av_malloc(len))) {
            av_log(NULL, AV_LOG_FATAL, "Attachment %s too large to fit into memory.\n",
                   o->attachments[i]);
            exit_program(1);
        }
        avio_read(pb, attachment, len);

        ost = new_attachment_stream(o, oc, -1);
        ost->stream_copy               = 1;
        ost->attachment_filename       = o->attachments[i];
        ost->finished                  = 1;
        ost->st->codec->extradata      = attachment;
        ost->st->codec->extradata_size = len;

        p = strrchr(o->attachments[i], '/');
        av_dict_set(&ost->st->metadata, "filename", (p && *p) ? p + 1 : o->attachments[i], AV_DICT_DONT_OVERWRITE);
        avio_closep(&pb);
    }

    for (i = nb_output_streams - oc->nb_streams; i < nb_output_streams; i++) { //for all streams of this output file
        AVDictionaryEntry *e;
        ost = output_streams[i];

        if ((ost->stream_copy || ost->attachment_filename)
            && (e = av_dict_get(o->g->codec_opts, "flags", NULL, AV_DICT_IGNORE_SUFFIX))
            && (!e->key[5] || check_stream_specifier(oc, ost->st, e->key+6)))
            if (av_opt_set(ost->st->codec, "flags", e->value, 0) < 0)
                exit_program(1);
    }

    if (!oc->nb_streams && !(oc->oformat->flags & AVFMT_NOSTREAMS)) {
        av_dump_format(oc, nb_output_files - 1, oc->filename, 1);
        av_log(NULL, AV_LOG_ERROR, "Output file #%d does not contain any stream\n", nb_output_files - 1);
        exit_program(1);
    }

    /* check if all codec options have been used */
	//FIXME
    unused_opts = strip_specifiers(o->g->codec_opts);
    for (i = of->ost_index; i < nb_output_streams; i++) {
        e = NULL;
        while ((e = av_dict_get(output_streams[i]->encoder_opts, "", e,
                                AV_DICT_IGNORE_SUFFIX)))
            av_dict_set(&unused_opts, e->key, NULL, 0);
    }

    e = NULL;
    while ((e = av_dict_get(unused_opts, "", e, AV_DICT_IGNORE_SUFFIX))) {
        const AVClass *class = avcodec_get_class();
        const AVOption *option = av_opt_find(&class, e->key, NULL, 0,
                                             AV_OPT_SEARCH_CHILDREN | AV_OPT_SEARCH_FAKE_OBJ);
        const AVClass *fclass = avformat_get_class();
        const AVOption *foption = av_opt_find(&fclass, e->key, NULL, 0,
                                              AV_OPT_SEARCH_CHILDREN | AV_OPT_SEARCH_FAKE_OBJ);
        if (!option || foption)
            continue;


        if (!(option->flags & AV_OPT_FLAG_ENCODING_PARAM)) {
            av_log(NULL, AV_LOG_ERROR, "Codec AVOption %s (%s) specified for "
                   "output file #%d (%s) is not an encoding option.\n", e->key,
                   option->help ? option->help : "", nb_output_files - 1,
                   filename);
            exit_program(1);
        }

        // gop_timecode is injected by generic code but not always used
        if (!strcmp(e->key, "gop_timecode"))
            continue;

        av_log(NULL, AV_LOG_WARNING, "Codec AVOption %s (%s) specified for "
               "output file #%d (%s) has not been used for any stream. The most "
               "likely reason is either wrong type (e.g. a video option with "
               "no video streams) or that it is a private option of some encoder "
               "which was not actually used for any stream.\n", e->key,
               option->help ? option->help : "", nb_output_files - 1, filename);
    }
    av_dict_free(&unused_opts);

    /* set the encoding/decoding_needed flags */
    for (i = of->ost_index; i < nb_output_streams; i++) {
        OutputStream *ost = output_streams[i];

        ost->encoding_needed = !ost->stream_copy;
        if (ost->encoding_needed && ost->source_index >= 0) {
            InputStream *ist = input_streams[ost->source_index];
            ist->decoding_needed |= DECODING_FOR_OST;
        }
    }

    /* check filename in case of an image number is expected */
    if (oc->oformat->flags & AVFMT_NEEDNUMBER) {
        if (!av_filename_number_test(oc->filename)) {
            print_error(oc->filename, AVERROR(EINVAL));
            exit_program(1);
        }
    }

    if (!(oc->oformat->flags & AVFMT_NOSTREAMS) && !input_stream_potentially_available) {
        av_log(NULL, AV_LOG_ERROR,
               "No input streams but output needs an input stream\n");
        exit_program(1);
    }

    if (!(oc->oformat->flags & AVFMT_NOFILE)) {
        /* test if it already exists to avoid losing precious files */
        assert_file_overwrite(filename);

        /* open the file */
        if ((err = avio_open2(&oc->pb, filename, AVIO_FLAG_WRITE,
                              &oc->interrupt_callback,
                              &of->opts)) < 0) {
            print_error(filename, err);
            exit_program(1);
        }
    } else if (strcmp(oc->oformat->name, "image2")==0 && !av_filename_number_test(filename))
        assert_file_overwrite(filename);
	//FIXME
    if (o->mux_preload) {
        av_dict_set_int(&of->opts, "preload", o->mux_preload*AV_TIME_BASE, 0);
    }
    oc->max_delay = (int)(o->mux_max_delay * AV_TIME_BASE);

    /* copy metadata */
    for (i = 0; i < o->nb_metadata_map; i++) {
        char *p;
        int in_file_index = strtol(o->metadata_map[i].u.str, &p, 0);

        if (in_file_index >= nb_input_files) {
            av_log(NULL, AV_LOG_FATAL, "Invalid input file index %d while processing metadata maps\n", in_file_index);
            exit_program(1);
        }
        copy_metadata(o->metadata_map[i].specifier, *p ? p + 1 : p, oc,
                      in_file_index >= 0 ?
                      input_files[in_file_index]->ctx : NULL, o);
    }

    /* copy chapters */
	//FIXME
    if (o->chapters_input_file >= nb_input_files) {
        if (o->chapters_input_file == INT_MAX) {
            /* copy chapters from the first input file that has them*/
            o->chapters_input_file = -1;
            for (i = 0; i < nb_input_files; i++)
                if (input_files[i]->ctx->nb_chapters) {
                    o->chapters_input_file = i;
                    break;
                }
        } else {
            av_log(NULL, AV_LOG_FATAL, "Invalid input file index %d in chapter mapping.\n",
                   o->chapters_input_file);
            exit_program(1);
        }
    }
    if (o->chapters_input_file >= 0)
        copy_chapters(input_files[o->chapters_input_file], of,
                      !o->metadata_chapters_manual);

    /* copy global metadata by default */
    if (!o->metadata_global_manual && nb_input_files){
        av_dict_copy(&oc->metadata, input_files[0]->ctx->metadata,
                     AV_DICT_DONT_OVERWRITE);
        if(o->recording_time != INT64_MAX)
            av_dict_set(&oc->metadata, "duration", NULL, 0);
        av_dict_set(&oc->metadata, "creation_time", NULL, 0);
    }
    if (!o->metadata_streams_manual)
        for (i = of->ost_index; i < nb_output_streams; i++) {
            InputStream *ist;
            if (output_streams[i]->source_index < 0)         /* this is true e.g. for attached files */
                continue;
            ist = input_streams[output_streams[i]->source_index];
            av_dict_copy(&output_streams[i]->st->metadata, ist->st->metadata, AV_DICT_DONT_OVERWRITE);
            if (!output_streams[i]->stream_copy) {
                av_dict_set(&output_streams[i]->st->metadata, "encoder", NULL, 0);
                if (ist->autorotate)
                    av_dict_set(&output_streams[i]->st->metadata, "rotate", NULL, 0);
            }
        }

    /* process manually set programs */
    for (i = 0; i < o->nb_program; i++) {
        const char *p = o->program[i].u.str;
        int progid = i+1;
        AVProgram *program;

        while(*p) {
            const char *p2 = av_get_token(&p, ":");
            const char *to_dealloc = p2;
            char *key;
            if (!p2)
                break;

            if(*p) p++;

            key = av_get_token(&p2, "=");
            if (!key || !*p2) {
                av_freep(&to_dealloc);
                av_freep(&key);
                break;
            }
            p2++;

            if (!strcmp(key, "program_num"))
                progid = strtol(p2, NULL, 0);
            av_freep(&to_dealloc);
            av_freep(&key);
        }

        program = av_new_program(oc, progid);

        p = o->program[i].u.str;
        while(*p) {
            const char *p2 = av_get_token(&p, ":");
            const char *to_dealloc = p2;
            char *key;
            if (!p2)
                break;
            if(*p) p++;

            key = av_get_token(&p2, "=");
            if (!key) {
                av_log(NULL, AV_LOG_FATAL,
                       "No '=' character in program string %s.\n",
                       p2);
                exit_program(1);
            }
            if (!*p2)
                exit_program(1);
            p2++;

            if (!strcmp(key, "title")) {
                av_dict_set(&program->metadata, "title", p2, 0);
            } else if (!strcmp(key, "program_num")) {
            } else if (!strcmp(key, "st")) {
                int st_num = strtol(p2, NULL, 0);
                av_program_add_stream_index(oc, progid, st_num);
            } else {
                av_log(NULL, AV_LOG_FATAL, "Unknown program key %s.\n", key);
                exit_program(1);
            }
            av_freep(&to_dealloc);
            av_freep(&key);
        }
    }

    /* process manually set metadata */
	//FIXME
    for (i = 0; i < o->nb_metadata; i++) {
        AVDictionary **m;
        char type, *val;
        const char *stream_spec;
        int index = 0, j, ret = 0;
        char now_time[256];

        val = strchr(o->metadata[i].u.str, '=');
        if (!val) {
            av_log(NULL, AV_LOG_FATAL, "No '=' character in metadata string %s.\n",
                   o->metadata[i].u.str);
            exit_program(1);
        }
        *val++ = 0;

        if (!strcmp(o->metadata[i].u.str, "creation_time") &&
            !strcmp(val, "now")) {
            time_t now = time(0);
            struct tm *ptm, tmbuf;
            ptm = localtime_r(&now, &tmbuf);
            if (ptm) {
                if (strftime(now_time, sizeof(now_time), "%Y-%m-%d %H:%M:%S", ptm))
                    val = now_time;
            }
        }

        parse_meta_type(o->metadata[i].specifier, &type, &index, &stream_spec);
        if (type == 's') {
            for (j = 0; j < oc->nb_streams; j++) {
                ost = output_streams[nb_output_streams - oc->nb_streams + j];
                if ((ret = check_stream_specifier(oc, oc->streams[j], stream_spec)) > 0) {
                    av_dict_set(&oc->streams[j]->metadata, o->metadata[i].u.str, *val ? val : NULL, 0);
                    if (!strcmp(o->metadata[i].u.str, "rotate")) {
                        ost->rotate_overridden = 1;
                    }
                } else if (ret < 0)
                    exit_program(1);
            }
        }
        else {
            switch (type) {
            case 'g':
                m = &oc->metadata;
                break;
            case 'c':
                if (index < 0 || index >= oc->nb_chapters) {
                    av_log(NULL, AV_LOG_FATAL, "Invalid chapter index %d in metadata specifier.\n", index);
                    exit_program(1);
                }
                m = &oc->chapters[index]->metadata;
                break;
            case 'p':
                if (index < 0 || index >= oc->nb_programs) {
                    av_log(NULL, AV_LOG_FATAL, "Invalid program index %d in metadata specifier.\n", index);
                    exit_program(1);
                }
                m = &oc->programs[index]->metadata;
                break;
            default:
                av_log(NULL, AV_LOG_FATAL, "Invalid metadata specifier %s.\n", o->metadata[i].specifier);
                exit_program(1);
            }
            av_dict_set(m, o->metadata[i].u.str, *val ? val : NULL, 0);
        }
    }

    return 0;
}
static void uninit_options(OptionsContext *o)
{
    const OptionDef *po = options;
    int i;

    /* all OPT_SPEC and OPT_STRING can be freed in generic way */
    while (po->name) {
        void *dst = (uint8_t*)o + po->u.off;

        if (po->flags & OPT_SPEC) {
            SpecifierOpt **so = dst;
            int i, *count = (int*)(so + 1);
            for (i = 0; i < *count; i++) {
                av_freep(&(*so)[i].specifier);
                if (po->flags & OPT_STRING)
                    av_freep(&(*so)[i].u.str);
            }
            av_freep(so);
            *count = 0;
        } else if (po->flags & OPT_OFFSET && po->flags & OPT_STRING)
            av_freep(dst);
        po++;
    }

    for (i = 0; i < o->nb_stream_maps; i++)
        av_freep(&o->stream_maps[i].linklabel);
    av_freep(&o->stream_maps);
    av_freep(&o->audio_channel_maps);
    av_freep(&o->streamid_map);
    av_freep(&o->attachments);
}

static void init_options(OptionsContext *o)
{
    memset(o, 0, sizeof(*o));

    o->stop_time = INT64_MAX;
    o->mux_max_delay  = 0.7;
    o->start_time     = AV_NOPTS_VALUE;
    o->start_time_eof = AV_NOPTS_VALUE;
    o->recording_time = INT64_MAX;
    o->limit_filesize = UINT64_MAX;
    o->chapters_input_file = INT_MAX;
    o->accurate_seek  = 1;
}

static int open_files(OptionGroupList *l, const char *inout,
                      int (*open_file)(OptionsContext*, const char*))
{
    int i, ret;

    for (i = 0; i < l->nb_groups; i++) {
        OptionGroup *g = &l->groups[i];
        OptionsContext o;

        init_options(&o);
        o.g = g;

        ret = parse_optgroup(&o, g);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Error parsing options for %s file "
                   "%s.\n", inout, g->arg);
            return ret;
        }

        av_log(NULL, AV_LOG_DEBUG, "Opening an %s file: %s.\n", inout, g->arg);
        ret = open_file(&o, g->arg);
        uninit_options(&o);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Error opening %s file %s.\n",
                   inout, g->arg);
            return ret;
        }
        av_log(NULL, AV_LOG_DEBUG, "Successfully opened the file.\n");
    }

    return 0;
}


static OutputStream *new_video_stream(OptionsContext *o, AVFormatContext *oc, int source_index)
{
    AVStream *st;
    OutputStream *ost;
    AVCodecContext *video_enc;
    char *frame_rate = NULL, *frame_aspect_ratio = NULL;

    ost = new_output_stream(o, oc, AVMEDIA_TYPE_VIDEO, source_index);
    st  = ost->st;
    video_enc = ost->enc_ctx;

    MATCH_PER_STREAM_OPT(frame_rates, str, frame_rate, oc, st);
    if (frame_rate && av_parse_video_rate(&ost->frame_rate, frame_rate) < 0) {
        av_log(NULL, AV_LOG_FATAL, "Invalid framerate value: %s\n", frame_rate);
        exit_program(1);
    }
    if (frame_rate && video_sync_method == VSYNC_PASSTHROUGH)
        av_log(NULL, AV_LOG_ERROR, "Using -vsync 0 and -r can produce invalid output files\n");

    MATCH_PER_STREAM_OPT(frame_aspect_ratios, str, frame_aspect_ratio, oc, st);
    if (frame_aspect_ratio) {
        AVRational q;
        if (av_parse_ratio(&q, frame_aspect_ratio, 255, 0, NULL) < 0 ||
            q.num <= 0 || q.den <= 0) {
            av_log(NULL, AV_LOG_FATAL, "Invalid aspect ratio: %s\n", frame_aspect_ratio);
            exit_program(1);
        }
        ost->frame_aspect_ratio = q;
    }

    MATCH_PER_STREAM_OPT(filter_scripts, str, ost->filters_script, oc, st);
    MATCH_PER_STREAM_OPT(filters,        str, ost->filters,        oc, st);

    if (!ost->stream_copy) {
        const char *p = NULL;
        char *frame_size = NULL;
        char *frame_pix_fmt = NULL;
        char *intra_matrix = NULL, *inter_matrix = NULL;
        char *chroma_intra_matrix = NULL;
        int do_pass = 0;
        int i;

        MATCH_PER_STREAM_OPT(frame_sizes, str, frame_size, oc, st);
        if (frame_size && av_parse_video_size(&video_enc->width, &video_enc->height, frame_size) < 0) {
            av_log(NULL, AV_LOG_FATAL, "Invalid frame size: %s.\n", frame_size);
            exit_program(1);
        }

        video_enc->bits_per_raw_sample = frame_bits_per_raw_sample;
        MATCH_PER_STREAM_OPT(frame_pix_fmts, str, frame_pix_fmt, oc, st);
        if (frame_pix_fmt && *frame_pix_fmt == '+') {
            ost->keep_pix_fmt = 1;
            if (!*++frame_pix_fmt)
                frame_pix_fmt = NULL;
        }
        if (frame_pix_fmt && (video_enc->pix_fmt = av_get_pix_fmt(frame_pix_fmt)) == AV_PIX_FMT_NONE) {
            av_log(NULL, AV_LOG_FATAL, "Unknown pixel format requested: %s.\n", frame_pix_fmt);
            exit_program(1);
        }
        st->sample_aspect_ratio = video_enc->sample_aspect_ratio;

        if (intra_only)
            video_enc->gop_size = 0;
        MATCH_PER_STREAM_OPT(intra_matrices, str, intra_matrix, oc, st);
        if (intra_matrix) {
            if (!(video_enc->intra_matrix = av_mallocz(sizeof(*video_enc->intra_matrix) * 64))) {
                av_log(NULL, AV_LOG_FATAL, "Could not allocate memory for intra matrix.\n");
                exit_program(1);
            }
            parse_matrix_coeffs(video_enc->intra_matrix, intra_matrix);
        }
        MATCH_PER_STREAM_OPT(chroma_intra_matrices, str, chroma_intra_matrix, oc, st);
        if (chroma_intra_matrix) {
            uint16_t *p = av_mallocz(sizeof(*video_enc->chroma_intra_matrix) * 64);
            if (!p) {
                av_log(NULL, AV_LOG_FATAL, "Could not allocate memory for intra matrix.\n");
                exit_program(1);
            }
            av_codec_set_chroma_intra_matrix(video_enc, p);
            parse_matrix_coeffs(p, chroma_intra_matrix);
        }
        MATCH_PER_STREAM_OPT(inter_matrices, str, inter_matrix, oc, st);
        if (inter_matrix) {
            if (!(video_enc->inter_matrix = av_mallocz(sizeof(*video_enc->inter_matrix) * 64))) {
                av_log(NULL, AV_LOG_FATAL, "Could not allocate memory for inter matrix.\n");
                exit_program(1);
            }
            parse_matrix_coeffs(video_enc->inter_matrix, inter_matrix);
        }

        MATCH_PER_STREAM_OPT(rc_overrides, str, p, oc, st);
        for (i = 0; p; i++) {
            int start, end, q;
            int e = sscanf(p, "%d,%d,%d", &start, &end, &q);
            if (e != 3) {
                av_log(NULL, AV_LOG_FATAL, "error parsing rc_override\n");
                exit_program(1);
            }
            video_enc->rc_override =
                av_realloc_array(video_enc->rc_override,
                                 i + 1, sizeof(RcOverride));
            if (!video_enc->rc_override) {
                av_log(NULL, AV_LOG_FATAL, "Could not (re)allocate memory for rc_override.\n");
                exit_program(1);
            }
            video_enc->rc_override[i].start_frame = start;
            video_enc->rc_override[i].end_frame   = end;
            if (q > 0) {
                video_enc->rc_override[i].qscale         = q;
                video_enc->rc_override[i].quality_factor = 1.0;
            }
            else {
                video_enc->rc_override[i].qscale         = 0;
                video_enc->rc_override[i].quality_factor = -q/100.0;
            }
            p = strchr(p, '/');
            if (p) p++;
        }
        video_enc->rc_override_count = i;

        if (do_psnr)
            video_enc->flags|= AV_CODEC_FLAG_PSNR;

        /* two pass mode */
        MATCH_PER_STREAM_OPT(pass, i, do_pass, oc, st);
        if (do_pass) {
            if (do_pass & 1) {
                video_enc->flags |= AV_CODEC_FLAG_PASS1;
                av_dict_set(&ost->encoder_opts, "flags", "+pass1", AV_DICT_APPEND);
            }
            if (do_pass & 2) {
                video_enc->flags |= AV_CODEC_FLAG_PASS2;
                av_dict_set(&ost->encoder_opts, "flags", "+pass2", AV_DICT_APPEND);
            }
        }

        MATCH_PER_STREAM_OPT(passlogfiles, str, ost->logfile_prefix, oc, st);
        if (ost->logfile_prefix &&
            !(ost->logfile_prefix = av_strdup(ost->logfile_prefix)))
            exit_program(1);

        if (do_pass) {
            char logfilename[1024];
            FILE *f;

            snprintf(logfilename, sizeof(logfilename), "%s-%d.log",
                     ost->logfile_prefix ? ost->logfile_prefix :
                                           DEFAULT_PASS_LOGFILENAME_PREFIX,
                     i);
            if (!strcmp(ost->enc->name, "libx264")) {
                av_dict_set(&ost->encoder_opts, "stats", logfilename, AV_DICT_DONT_OVERWRITE);
            } else {
                if (video_enc->flags & AV_CODEC_FLAG_PASS2) {
                    char  *logbuffer = read_file(logfilename);

                    if (!logbuffer) {
                        av_log(NULL, AV_LOG_FATAL, "Error reading log file '%s' for pass-2 encoding\n",
                               logfilename);
                        exit_program(1);
                    }
                    video_enc->stats_in = logbuffer;
                }
                if (video_enc->flags & AV_CODEC_FLAG_PASS1) {
                    f = av_fopen_utf8(logfilename, "wb");
                    if (!f) {
                        av_log(NULL, AV_LOG_FATAL,
                               "Cannot write log file '%s' for pass-1 encoding: %s\n",
                               logfilename, strerror(errno));
                        exit_program(1);
                    }
                    ost->logfile = f;
                }
            }
        }

        MATCH_PER_STREAM_OPT(forced_key_frames, str, ost->forced_keyframes, oc, st);
        if (ost->forced_keyframes)
            ost->forced_keyframes = av_strdup(ost->forced_keyframes);

        MATCH_PER_STREAM_OPT(force_fps, i, ost->force_fps, oc, st);

        ost->top_field_first = -1;
        MATCH_PER_STREAM_OPT(top_field_first, i, ost->top_field_first, oc, st);


        ost->avfilter = get_ost_filters(o, oc, ost);
        if (!ost->avfilter)
            exit_program(1);
    } else {
        MATCH_PER_STREAM_OPT(copy_initial_nonkeyframes, i, ost->copy_initial_nonkeyframes, oc ,st);
    }

    if (ost->stream_copy)
        check_streamcopy_filters(o, oc, ost, AVMEDIA_TYPE_VIDEO);

    return ost;
}

static OutputStream *new_audio_stream(OptionsContext *o, AVFormatContext *oc, int source_index)
{
    int n;
    AVStream *st;
    OutputStream *ost;
    AVCodecContext *audio_enc;

    ost = new_output_stream(o, oc, AVMEDIA_TYPE_AUDIO, source_index);
    st  = ost->st;

    audio_enc = ost->enc_ctx;
    audio_enc->codec_type = AVMEDIA_TYPE_AUDIO;

    MATCH_PER_STREAM_OPT(filter_scripts, str, ost->filters_script, oc, st);
    MATCH_PER_STREAM_OPT(filters,        str, ost->filters,        oc, st);

    if (!ost->stream_copy) {
        char *sample_fmt = NULL;

        MATCH_PER_STREAM_OPT(audio_channels, i, audio_enc->channels, oc, st);

        MATCH_PER_STREAM_OPT(sample_fmts, str, sample_fmt, oc, st);
        if (sample_fmt &&
            (audio_enc->sample_fmt = av_get_sample_fmt(sample_fmt)) == AV_SAMPLE_FMT_NONE) {
            av_log(NULL, AV_LOG_FATAL, "Invalid sample format '%s'\n", sample_fmt);
            exit_program(1);
        }

        MATCH_PER_STREAM_OPT(audio_sample_rate, i, audio_enc->sample_rate, oc, st);

        MATCH_PER_STREAM_OPT(apad, str, ost->apad, oc, st);
        ost->apad = av_strdup(ost->apad);

        ost->avfilter = get_ost_filters(o, oc, ost);
        if (!ost->avfilter)
            exit_program(1);

        /* check for channel mapping for this audio stream */
		//FIXME
        for (n = 0; n < o->nb_audio_channel_maps; n++) {
            AudioChannelMap *map = &o->audio_channel_maps[n];
            if ((map->ofile_idx   == -1 || ost->file_index == map->ofile_idx) &&
                (map->ostream_idx == -1 || ost->st->index  == map->ostream_idx)) {
                InputStream *ist;

                if (map->channel_idx == -1) {
                    ist = NULL;
                } else if (ost->source_index < 0) {
                    av_log(NULL, AV_LOG_FATAL, "Cannot determine input stream for channel mapping %d.%d\n",
                           ost->file_index, ost->st->index);
                    continue;
                } else {
                    ist = input_streams[ost->source_index];
                }

                if (!ist || (ist->file_index == map->file_idx && ist->st->index == map->stream_idx)) {
                    if (av_reallocp_array(&ost->audio_channels_map,
                                          ost->audio_channels_mapped + 1,
                                          sizeof(*ost->audio_channels_map)
                                          ) < 0 )
                        exit_program(1);

                    ost->audio_channels_map[ost->audio_channels_mapped++] = map->channel_idx;
                }
            }
        }
    }

    if (ost->stream_copy)
        check_streamcopy_filters(o, oc, ost, AVMEDIA_TYPE_AUDIO);

    return ost;
}
void parse_options(void *optctx, int argc, char **argv, const OptionDef *options,
                   void (*parse_arg_function)(void *, const char*))
{
    const char *opt;
    int optindex, handleoptions = 1, ret;

    /* perform system-dependent conversions for arguments list */
    prepare_app_arguments(&argc, &argv);

    /* parse options */
    optindex = 1;
    while (optindex < argc) {
        opt = argv[optindex++];

        if (handleoptions && opt[0] == '-' && opt[1] != '\0') {
            if (opt[1] == '-' && opt[2] == '\0') {
                handleoptions = 0;
                continue;
            }
            opt++;

            if ((ret = parse_option(optctx, opt, argv[optindex], options)) < 0)
                exit_program(1);
            optindex += ret;
        } else {
            if (parse_arg_function)
                parse_arg_function(optctx, opt);
        }
    }
}

int parse_optgroup(void *optctx, OptionGroup *g)
{
    int i, ret;

    av_log(NULL, AV_LOG_DEBUG, "Parsing a group of options: %s %s.\n",
           g->group_def->name, g->arg);

    for (i = 0; i < g->nb_opts; i++) {
        Option *o = &g->opts[i];

        if (g->group_def->flags &&
			// FIXME
            !(g->group_def->flags & o->opt->flags)) {
            av_log(NULL, AV_LOG_ERROR, "Option %s (%s) cannot be applied to "
                   "%s %s -- you are trying to apply an input option to an "
                   "output file or vice versa. Move this option before the "
                   "file it belongs to.\n", o->key, o->opt->help,
                   g->group_def->name, g->arg);
            return AVERROR(EINVAL);
        }
		//FIXME
        av_log(NULL, AV_LOG_DEBUG, "Applying option %s (%s) with argument %s.\n",
               o->key, o->opt->help, o->val);
		//FIXME
        ret = write_option(optctx, o->opt, o->key, o->val);
        if (ret < 0)
            return ret;
    }

    av_log(NULL, AV_LOG_DEBUG, "Successfully parsed a group of options.\n");

    return 0;
}

// From ffmpeg 3.0.2
int MOVIE_SAVER::transcode_init(void)
{
    int ret = 0, i, j, k;
    AVFormatContext *oc;
    OutputStream *ost;
    InputStream *ist;
    char error[1024] = {0};
    int want_sdp = 1;

//    for (i = 0; i < nb_filtergraphs; i++) {
//        FilterGraph *fg = filtergraphs[i];
//        for (j = 0; j < fg->nb_outputs; j++) {
//            OutputFilter *ofilter = fg->outputs[j];
//            if (!ofilter->ost || ofilter->ost->source_index >= 0)
//                continue;
//            if (fg->nb_inputs != 1)
//                continue;
//            for (k = nb_input_streams-1; k >= 0 ; k--)
//                if (fg->inputs[0]->ist == input_streams[k])
//                    break;
//            ofilter->ost->source_index = k;
//        }
//    }

    /* init framerate emulation */
//    for (i = 0; i < nb_input_files; i++) {
//        InputFile *ifile = input_files[i];
//        if (ifile->rate_emu)
//            for (j = 0; j < ifile->nb_streams; j++)
//                input_streams[j + ifile->ist_index]->start = av_gettime_relative();
//    }

	//nb_output_streams = 2; // Audio and Video
	nb_output_streams = 1; // Audio only
    /* for each output stream, we compute the right encoding parameters */
    for (i = 0; i < nb_output_streams; i++) {
        AVCodecContext *enc_ctx;
        AVCodecContext *dec_ctx = NULL;
        ost = output_streams[i];
        oc  = output_files[ost->file_index]->ctx;
        //ist = get_input_stream(ost);
		ist = NULL;
        if (ost->attachment_filename)
            continue;

        enc_ctx = ost->stream_copy ? ost->st->codec : ost->enc_ctx;

        if (ist) {
            dec_ctx = ist->dec_ctx;

            ost->st->disposition          = ist->st->disposition;
            enc_ctx->bits_per_raw_sample    = dec_ctx->bits_per_raw_sample;
            enc_ctx->chroma_sample_location = dec_ctx->chroma_sample_location;
        } else {
			for (j=0; j< oc->nb_streams; j++) {
                AVStream *st = oc->streams[j];
                if (st != ost->st && st->codec->codec_type == enc_ctx->codec_type)
                    break;
            }
            if (j == oc->nb_streams)
                if (enc_ctx->codec_type == AVMEDIA_TYPE_AUDIO || enc_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
                    ost->st->disposition = AV_DISPOSITION_DEFAULT;
		}
		
		{
            if (!ost->enc)
                ost->enc = avcodec_find_encoder(enc_ctx->codec_id);
            if (!ost->enc) {
                /* should only happen when a default codec is not present. */
                snprintf(error, sizeof(error), "Encoder (codec %s) not found for output stream #%d:%d",
                         avcodec_get_name(ost->st->codec->codec_id), ost->file_index, ost->index);
                ret = AVERROR(EINVAL);
                goto dump_format;
            }

            set_encoder_id(output_files[ost->file_index], ost);

//#if CONFIG_LIBMFX
//            if (qsv_transcode_init(ost))
//                exit_program(1);
//#endif
			if (!ost->filter &&
				(enc_ctx->codec_type == AVMEDIA_TYPE_VIDEO ||
                 enc_ctx->codec_type == AVMEDIA_TYPE_AUDIO)) {
				FilterGraph *fg;
				fg = init_simple_filtergraph(ist, ost);
				if (configure_filtergraph(fg)) {
					av_log(NULL, AV_LOG_FATAL, "Error opening filters!\n");
					exit_program(1);
				}
            }

            if (enc_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                if (!ost->frame_rate.num)
                    ost->frame_rate = av_buffersink_get_frame_rate(ost->filter->filter);
                //if (ist && !ost->frame_rate.num)
                //    ost->frame_rate = ist->framerate;
                //if (ist && !ost->frame_rate.num)
                //    ost->frame_rate = ist->st->r_frame_rate;
                if (!ost->frame_rate.num) {
                    ost->frame_rate = (AVRational){rec_fps, 1};
                    av_log(NULL, AV_LOG_WARNING,
                           "No information "
                           "about the input framerate is available. Falling "
                           "back to a default value of %d fps for output stream #%d:%d. Use the -r option "
                           "if you want a different framerate.\n",
                           rec_fps, ost->file_index, ost->index);
                }
                //ost->frame_rate = (AVRational){25, 1};
                if (ost->enc && ost->enc->supported_framerates && !ost->force_fps) {
                    int idx = av_find_nearest_q_idx(ost->frame_rate, ost->enc->supported_framerates);
                    ost->frame_rate = ost->enc->supported_framerates[idx];
                }
                // reduce frame rate for mpeg4 to be within the spec limits
                if (enc_ctx->codec_id == AV_CODEC_ID_MPEG4) {
                    av_reduce(&ost->frame_rate.num, &ost->frame_rate.den,
                              ost->frame_rate.num, ost->frame_rate.den, 65535);
                }
            }

            switch (enc_ctx->codec_type) {
            case AVMEDIA_TYPE_AUDIO:
                enc_ctx->sample_fmt     = ost->filter->filter->inputs[0]->format;
                enc_ctx->sample_rate    = ost->filter->filter->inputs[0]->sample_rate;
                enc_ctx->channel_layout = ost->filter->filter->inputs[0]->channel_layout;
                enc_ctx->channels       = avfilter_link_get_channels(ost->filter->filter->inputs[0]);
                enc_ctx->time_base      = (AVRational){ 1, enc_ctx->sample_rate };
                break;
            case AVMEDIA_TYPE_VIDEO:
                enc_ctx->time_base = av_inv_q(ost->frame_rate);
                if (!(enc_ctx->time_base.num && enc_ctx->time_base.den))
                    enc_ctx->time_base = ost->filter->filter->inputs[0]->time_base;
                if (   av_q2d(enc_ctx->time_base) < 0.001 && video_sync_method != VSYNC_PASSTHROUGH
                   && (video_sync_method == VSYNC_CFR || video_sync_method == VSYNC_VSCFR || (video_sync_method == VSYNC_AUTO && !(oc->oformat->flags & AVFMT_VARIABLE_FPS)))){
                    av_log(oc, AV_LOG_WARNING, "Frame rate very high for a muxer not efficiently supporting it.\n"
                                               "Please consider specifying a lower framerate, a different muxer or -vsync 2\n");
                }
                for (j = 0; j < ost->forced_kf_count; j++)
                    ost->forced_kf_pts[j] = av_rescale_q(ost->forced_kf_pts[j],
                                                         AV_TIME_BASE_Q,
                                                         enc_ctx->time_base);

                enc_ctx->width  = ost->filter->filter->inputs[0]->w;
                enc_ctx->height = ost->filter->filter->inputs[0]->h;
                enc_ctx->sample_aspect_ratio = ost->st->sample_aspect_ratio =
                    ost->frame_aspect_ratio.num ? // overridden by the -aspect cli option
                    av_mul_q(ost->frame_aspect_ratio, (AVRational){ enc_ctx->height, enc_ctx->width }) :
                    ost->filter->filter->inputs[0]->sample_aspect_ratio;
                if (!strncmp(ost->enc->name, "libx264", 7) &&
                    enc_ctx->pix_fmt == AV_PIX_FMT_NONE &&
                    ost->filter->filter->inputs[0]->format != AV_PIX_FMT_YUV420P)
                    av_log(NULL, AV_LOG_WARNING,
                           "No pixel format specified, %s for H.264 encoding chosen.\n"
                           "Use -pix_fmt yuv420p for compatibility with outdated media players.\n",
                           av_get_pix_fmt_name(ost->filter->filter->inputs[0]->format));
                if (!strncmp(ost->enc->name, "mpeg2video", 10) &&
                    enc_ctx->pix_fmt == AV_PIX_FMT_NONE &&
                    ost->filter->filter->inputs[0]->format != AV_PIX_FMT_YUV420P)
                    av_log(NULL, AV_LOG_WARNING,
                           "No pixel format specified, %s for MPEG-2 encoding chosen.\n"
                           "Use -pix_fmt yuv420p for compatibility with outdated media players.\n",
                           av_get_pix_fmt_name(ost->filter->filter->inputs[0]->format));
                enc_ctx->pix_fmt = ost->filter->filter->inputs[0]->format;

                ost->st->avg_frame_rate = ost->frame_rate;

                if (!dec_ctx ||
                    enc_ctx->width   != dec_ctx->width  ||
                    enc_ctx->height  != dec_ctx->height ||
                    enc_ctx->pix_fmt != dec_ctx->pix_fmt) {
                    enc_ctx->bits_per_raw_sample = frame_bits_per_raw_sample;
                }

                if (ost->forced_keyframes) {
                    if (!strncmp(ost->forced_keyframes, "expr:", 5)) {
                        ret = av_expr_parse(&ost->forced_keyframes_pexpr, ost->forced_keyframes+5,
                                            forced_keyframes_const_names, NULL, NULL, NULL, NULL, 0, NULL);
                        if (ret < 0) {
                            av_log(NULL, AV_LOG_ERROR,
                                   "Invalid force_key_frames expression '%s'\n", ost->forced_keyframes+5);
                            return ret;
                        }
                        ost->forced_keyframes_expr_const_values[FKF_N] = 0;
                        ost->forced_keyframes_expr_const_values[FKF_N_FORCED] = 0;
                        ost->forced_keyframes_expr_const_values[FKF_PREV_FORCED_N] = NAN;
                        ost->forced_keyframes_expr_const_values[FKF_PREV_FORCED_T] = NAN;

                        // Don't parse the 'forced_keyframes' in case of 'keep-source-keyframes',
                        // parse it only for static kf timings
                    } else if(strncmp(ost->forced_keyframes, "source", 6)) {
                        parse_forced_key_frames(ost->forced_keyframes, ost, ost->enc_ctx);
                    }
                }
                break;
//            case AVMEDIA_TYPE_SUBTITLE:
//                enc_ctx->time_base = (AVRational){1, 1000};
//                if (!enc_ctx->width) {
//                    enc_ctx->width     = input_streams[ost->source_index]->st->codec->width;
//                    enc_ctx->height    = input_streams[ost->source_index]->st->codec->height;
//                }
//                break;
//            case AVMEDIA_TYPE_DATA:
//                break;
//            default:
//                abort();
//                break;
            }
        }

        if (ost->disposition) {
            static const AVOption opts[] = {
                { "disposition"         , NULL, 0, AV_OPT_TYPE_FLAGS, { .i64 = 0 }, INT64_MIN, INT64_MAX, .unit = "flags" },
                { "default"             , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_DEFAULT           },    .unit = "flags" },
                { "dub"                 , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_DUB               },    .unit = "flags" },
                { "original"            , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_ORIGINAL          },    .unit = "flags" },
                { "comment"             , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_COMMENT           },    .unit = "flags" },
                { "lyrics"              , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_LYRICS            },    .unit = "flags" },
                { "karaoke"             , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_KARAOKE           },    .unit = "flags" },
                { "forced"              , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_FORCED            },    .unit = "flags" },
                { "hearing_impaired"    , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_HEARING_IMPAIRED  },    .unit = "flags" },
                { "visual_impaired"     , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_VISUAL_IMPAIRED   },    .unit = "flags" },
                { "clean_effects"       , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_CLEAN_EFFECTS     },    .unit = "flags" },
                { "captions"            , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_CAPTIONS          },    .unit = "flags" },
                { "descriptions"        , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_DESCRIPTIONS      },    .unit = "flags" },
                { "metadata"            , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_DISPOSITION_METADATA          },    .unit = "flags" },
                { NULL },
            };
            static const AVClass class = {
                .class_name = "",
                .item_name  = av_default_item_name,
                .option     = opts,
                .version    = LIBAVUTIL_VERSION_INT,
            };
            const AVClass *pclass = &class;

            ret = av_opt_eval_flags(&pclass, &opts[0], ost->disposition, &ost->st->disposition);
            if (ret < 0)
                goto dump_format;
        }
    }

    /* open each encoder */
    for (i = 0; i < nb_output_streams; i++) {
        ret = init_output_stream(output_streams[i], error, sizeof(error));
        if (ret < 0)
            goto dump_format;
    }

    /* open files and write file headers */
    for (i = 0; i < nb_output_files; i++) {
        oc = output_files[i]->ctx;
        oc->interrupt_callback = int_cb;
        if ((ret = avformat_write_header(oc, &output_files[i]->opts)) < 0) {
            snprintf(error, sizeof(error),
                     "Could not write header for output file #%d "
                     "(incorrect codec parameters ?): %s",
                     i, av_err2str(ret));
            ret = AVERROR(EINVAL);
            goto dump_format;
        }
//         assert_avoptions(output_files[i]->opts);
        if (strcmp(oc->oformat->name, "rtp")) {
            want_sdp = 0;
        }
    }

dump_format:
    /* dump the file output parameters - cannot be done before in case
       of stream copy */
    for (i = 0; i < nb_output_files; i++) {
        av_dump_format(output_files[i]->ctx, i, output_files[i]->ctx->filename, 1);
    }

    for (i = 0; i < nb_output_streams; i++) {
        ost = output_streams[i];

        if (ost->attachment_filename) {
            /* an attached file */
            av_log(NULL, AV_LOG_INFO, "  File %s -> Stream #%d:%d\n",
                   ost->attachment_filename, ost->file_index, ost->index);
            continue;
        }

        if (ost->filter && ost->filter->graph->graph_desc) {
            /* output from a complex graph */
            av_log(NULL, AV_LOG_INFO, "  %s", ost->filter->name);
            if (nb_filtergraphs > 1)
                av_log(NULL, AV_LOG_INFO, " (graph %d)", ost->filter->graph->index);

            av_log(NULL, AV_LOG_INFO, " -> Stream #%d:%d (%s)\n", ost->file_index,
                   ost->index, ost->enc ? ost->enc->name : "?");
            continue;
        }

        //av_log(NULL, AV_LOG_INFO, "  Stream #%d:%d -> #%d:%d",
        //       input_streams[ost->source_index]->file_index,
        //       input_streams[ost->source_index]->st->index,
        //       ost->file_index,
        //       ost->index);
        if (ost->stream_copy)
            av_log(NULL, AV_LOG_INFO, " (copy)");
        else {
            //const AVCodec *in_codec    = input_streams[ost->source_index]->dec;
            const AVCodec *in_codec    = NULL;
            const AVCodec *out_codec   = ost->enc;
            const char *decoder_name   = "?";
            const char *in_codec_name  = "?";
            const char *encoder_name   = "?";
            const char *out_codec_name = "?";
            const AVCodecDescriptor *desc;

            if (in_codec) {
                decoder_name  = in_codec->name;
                desc = avcodec_descriptor_get(in_codec->id);
                if (desc)
                    in_codec_name = desc->name;
                if (!strcmp(decoder_name, in_codec_name))
                    decoder_name = "native";
            }

            if (out_codec) {
                encoder_name   = out_codec->name;
                desc = avcodec_descriptor_get(out_codec->id);
                if (desc)
                    out_codec_name = desc->name;
                if (!strcmp(encoder_name, out_codec_name))
                    encoder_name = "native";
            }

            av_log(NULL, AV_LOG_INFO, " (%s (%s) -> %s (%s))",
                   in_codec_name, decoder_name,
                   out_codec_name, encoder_name);
        }
        av_log(NULL, AV_LOG_INFO, "\n");
    }

    if (ret) {
        av_log(NULL, AV_LOG_ERROR, "%s\n", error);
        return ret;
    }

    //if (sdp_filename || want_sdp) {
    //    print_sdp();
    //}

    transcode_init_done = true;

    return 0;
}


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
		AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC WARINING: Failed to get output_context; Try to use fallback  MP4");
		avformat_alloc_output_context2(&output_context, NULL, "mp4", filename.toLocal8Bit().constData());
		if(output_context == NULL) {
			AGAR_DebugLog(AGAR_LOG_DEBUG, "AVC ERROR: Failed to get output_context even fallback.");
			return false;
		}
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
//    audio_tmp_frame = (AVFrame *)alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout,
//												   c->sample_rate, nb_samples);
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

	recording = true;
}
