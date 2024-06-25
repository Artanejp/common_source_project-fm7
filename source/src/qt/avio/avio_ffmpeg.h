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
	#if (LIBAVCODEC_VERSION_MAJOR > 56)
	#define AVCODEC_UPPER_V56
	#endif
#endif
