#pragma once

extern "C" {
	#include "libavutil/channel_layout.h"
	#include "libavutil/opt.h"
	#include "libavutil/mathematics.h"
	#include "libavutil/timestamp.h"
	#include "libavutil/frame.h"
	#include "libavutil/imgutils.h"
	
	#include "libavformat/avformat.h"
	
	#include "libswscale/swscale.h"
	#include "libswresample/swresample.h"
	
	#include "libavcodec/avcodec.h"
}
#if !defined(AVCODEC_UPPER_V56)
	#if (LIBAVCODEC_VERSION_MAJOR > 56) /* FFMPeg 3.0 or later */
	#define AVCODEC_UPPER_V56
	#endif
#endif
#if !defined(AVCODEC_UPPER_V60) /* FFMpeg 7.0 or later */
	#if (LIBAVCODEC_VERSION_MAJOR > 60) 
	#define AVCODEC_UPPER_V60
	#endif
#endif
#if !defined(AVCODEC_UPPER_V61) /* FFMpeg 8.0 or later */
	#if (LIBAVCODEC_VERSION_MAJOR > 61) 
	#define AVCODEC_UPPER_V61
	#endif
#endif

