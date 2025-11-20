/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2025.05.10 -

	[FM-Towns CD-ROM based on SCSI CDROM; Definitions of CD-ROM Sector types and some useful utilities.]
	History:
	  2025-11-02 K.Ohta : Maybe initial.
*/

#pragma once

namespace CDROM_DEFS {
/*!
 * @brief ENUM DEFINITION of CD-ROM modes from CUE SYNTAX Sheet.
 * @note See https://totalsonic.net/cuesheetsyntax.htm .
 */
enum {
	TYPE_AUDIO      = 0,	/* AUDIO */ 
	TYPE_MODE1_2352 = 1,	/* MODE1/2352 , with header and footer; RAW data. */
	TYPE_MODE1_2048 = 2,	/* MODE1/2048 , *without* header and footer. */
	TYPE_NOOP       = 3,	/* Reserved for future. */
	TYPE_MODE2_2336 = 4,	/* MODE2/2336, *without* header. */
	TYPE_MODE2_2352 = 5,	/* MODE2/2352, with header ; RAW data. */
	TYPE_CDI_2336   = 6,	/* CD-I/2336,  *without* header. */
	TYPE_CDI_2352   = 7,	/* CD-I/2352, with header ; RAW data. */
	TYPE_CDG        = 8,	/* Karaoke CD+G , 2448 bytes per sector. RAW sector and SUB fields. */
	TYPE_UNUSED,			/* This indicates UNUSED SECTOR */
};

static const _TCHAR *_CUE_TRACK_IDENTS[] =
{
	"AUDIO",
	"MODE1/2352",
	"MODE1/2048",
	"DUMMY",
	"MODE2/2336",
	"MODE2/2352",
	"CDI/2336",
	"CDI/2352",
	"CDG",
	"TAIL",
};

enum {
	IMG_NONE    = 0,			/* This indicates empty or unused. */
	IMG_RAW     = 1,			/* RAW BINARY IMAGE ; maybe includes only IMG file with CCD. */
	IMG_RAW_SUB = 3,			/* RAW BINARY IMAGE with SUBQ raw datas.
								   (Reserved for future; similar to Karaoke CD+G) */
	IMG_IMG_SUB,				/* foo.img + foo.sub ; mostly at CCD image. */
	IMG_MOTOROLA = 8,			/* This indicates to need to swap endian. */
	IMG_WAV     = 16,			/* WAVE image */
	IMG_MP3     = 17,			/* MP3 image */
	IMG_AIFF    = 18,			/* AIFF image */
	IMG_EXTERNAL_CODEC = 65536,	/* Need external codec to decode data; reserved for future. */
};

static const size_t _PHYSICAL_SECTOR_SIZE[] = {
	2352, /* AUDIO */
	2352, /* MODE1/2352 */
	2048, /* MODE1/2048 */
	0,	  /* NOOP */
	2336, /* MODE2/2336 */
	2352, /* MODE2/2352 */
	2336, /* CDI/2336 */
	2352, /* CDI/2352 */
	2448, /* CD+G (2448) */
	0,
};
static const size_t _LOGICAL_SECTOR_SIZE[] = {
	2352, /* AUDIO */
	2048, /* MODE1/2352 */
	2048, /* MODE1/2048 */
	0,
	2336, /* MODE2/2336 */
	2336, /* MODE2/2352 */
	2336, /* CDI/2336 */
	2336, /* CDI/2352 */
	2448, /* CD+G (2448) */
	0,
};

/*!
 * @note Some utilities.
 */
constexpr bool is_audio_track(const int typ)
{
	return (typ == TYPE_AUDIO) ? true : false;
}


constexpr size_t get_physical_sector_size(const unsigned int typ)
{
	if(typ >= (sizeof(_PHYSICAL_SECTOR_SIZE) / sizeof(size_t))) {
		return 0; // ERROR!!
	}
	return  _PHYSICAL_SECTOR_SIZE(typ);
}

constexpr size_t get_logical_sector_size(const unsigned int typ)
{
	if(typ >= (sizeof(_LOGICAL_SECTOR_SIZE) / sizeof(size_t))) {
		return 0; // ERROR!!
	}
	return  _LOGICAL_SECTOR_SIZE(typ);
}

}
