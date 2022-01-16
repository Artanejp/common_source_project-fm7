#pragma once

#include <QString>
#include <QMap>

#include "common.h"

namespace CSP_AVIO_BASIC {
	enum {
		TYPE_NONE = 0,
		TYPE_MP3 = 1,
		TYPE_AAC,
		TYPE_AAC_LATM,
		TYPE_VORBIS,
		TYPE_FLAC,
		TYPE_PCM_S16LE,
		TYPE_PCM_S24LE,
		
		TYPE_VIDEO = 1025,
		TYPE_MPEG1,
		TYPE_MPEG2,
		TYPE_MPEG4,
		TYPE_H264,
		TYPE_LIBX264,
		TYPE_VP9,
		TYPE_VPX_VP9,
		TYPE_HEVC,
		TYPE_LIBX265,
		TYPE_AV1,
		TYPE_LIBRAV1E_AV1,
		
		TYPE_CONTAINER = 65537,
		TYPE_WAV,
		TYPE_MP4,
		TYPE_WEBM,
		TYPE_MKV,
		TYPE_AVI,
		
		TYPE_UNKNOWN = -1
	};
	static inline int csp_avio_get_format_type(_TCHAR* name)
	{
		QMap<QString, int> _list;
		_list["none"] = TYPE_NONE;
		_list["wav"] = TYPE_WAV;
		_list["flac"] = TYPE_FLAC;
		_list["mp4"] = TYPE_MP4;
		_list["mkv"] = TYPE_MKV;
		_list["webm"] = TYPE_WEBM;
		_list["avi"] = TYPE_AVI;
		_list["mp3"] = TYPE_MP3;

		QString _name = QString::fromUtf8(name); 
		int _type = _list.value(_name, TYPE_UNKNOWN);
		return _type;
	}
	static inline int csp_avio_get_codec_type(_TCHAR* name)
	{
		QMap<QString, int> _list;
		_list["none"] = TYPE_NONE;
		_list["mp3"] = TYPE_MP3;
		_list["aac"] = TYPE_AAC;
		_list["aac_latm"] = TYPE_AAC_LATM;
		_list["vorbis"] = TYPE_VORBIS;
		_list["flac"] = TYPE_FLAC;
		_list["pcm_s16le"] = TYPE_PCM_S16LE;
		_list["pcm_s24le"] = TYPE_PCM_S24LE;

		_list["mpeg1video"] = TYPE_MPEG1;
		_list["mpeg2video"] = TYPE_MPEG2;
		_list["mpeg4"] = TYPE_MPEG4;
		_list["h264"] = TYPE_H264;
		_list["libx264"] = TYPE_LIBX264;
		_list["vp9"] = TYPE_VP9;
		_list["libvpx-vp9"] = TYPE_VPX_VP9;
		_list["hevc"] = TYPE_HEVC;
		_list["libx265"] = TYPE_LIBX265;
		_list["av1"] = TYPE_AV1;
		_list["librav1e"] = TYPE_LIBRAV1E_AV1;
		
		_list["unknown_codec"] = TYPE_UNKNOWN;
		QString _name = QString::fromUtf8(name); 
		int _type = _list.value(_name, TYPE_UNKNOWN);
		return _type;
	}
}
