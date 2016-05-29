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

extern "C" {
	#include "libavutil/common.h"
	#include "libavutil/intreadwrite.h"
}
