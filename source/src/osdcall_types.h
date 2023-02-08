/*
 * This is messaging definitions from EMU:: to OSD.
 *
 * Usage:
 * string_message_from_emu(enum EMU_MEDIA_TYPE, int drive, enum EMU_MESSAGE_TYPE, _TCHAR *real_message);
 * int_message_from_emu(enum EMU_MEDIA_TYPE, int drive, enum EMU_MESSAGE_TYPE, int64_t real_message);
 *
 * (C) 2023 K.Ohta <whatisthis.sowhat@gmail.com>
 *
 */

#pragma once

namespace EMU_MEDIA_TYPE {
	typedef uint64_t type_t ;
	enum  {
		NONE = 0 << 16,
		BINARY = 1 << 16,
		BUBBLE_CASETTE = 2 << 16,
		CARTRIDGE = 3 << 16,
		COMPACT_DISC = 4 << 16,
		FLOPPY_DISK = 5 << 16,
		HARD_DISK = 6 << 16,
		LASER_DISC = 7 << 16,
		QUICK_DISK = 8 << 16,
		TAPE = 9 << 16,

		AUDIO = (28 << 16),
		VIDEO = (29 << 16),
		ANY_MEDIA = (127 << 16),
		UI_MEDIA_MASK = (127 << 16),

		EMU_SLOT_MASK = 0x7f,
		MULTIPLE_SLOT_MASK = 0xff,

		END = INT_MAX
	};
	const type_t MULTIPLE_SLOT_DETECT_MASK = ((~EMU_SLOT_MASK) & MULTIPLE_SLOT_MASK);

}


namespace EMU_MESSAGE_TYPE {
	typedef uint64_t type_t ;

	enum {
		NONE = 0,
		PLAY = 0,
		LOAD = 0,
		RECORD = 1,
		SAVE   = 1,
		STOP   = 2,
		PAUSE   = (1 << 2),
		UNPAUSE = (2 << 2),
		FAST_FORWARD = (2 << 4),
		FAST_REWIND  = (3 << 4),
		APSS_FORWARD = (6 << 4),
		APSS_REWIND  = (7 << 4),
		WRITE_PROTECT = (8 << 4),

		MEDIA_MOUNTED = (1    << 8),
		MEDIA_REMOVED = (2    << 8),
		MEDIA_OTHERS  = (0 << 8),
		VIRT_MEDIA_SELECTED = (4 << 8),

		TYPE_MEDIA   = (0x00 << 24),
		TYPE_MESSAGE = (0x10 << 24),
		TYPE_MASK    = (0xff << 24),

		MEDIA_MASK = 0x0000ffff,
		MEDIA_PLAYREC_MASK = 0x01,
		MEDIA_STOP_MASK    = 0x02,
		MEDIA_PAUSE_MASK = (3 << 2),
		MEDIA_DIRECTION_MASK = (1 << 4),
		MEDIA_FF_APSS_MASK = (4 << 4),
		MEDIA_FFREW_MASK  = (MEDIA_DIRECTION_MASK | MEDIA_FF_APSS_MASK),
		MEDIA_MODE_MASK = (0xff << 8),

		END = INT_MAX
	};
	const type_t MOUNT         = MEDIA_MOUNTED | 0;
	const type_t EJECT         = MEDIA_REMOVED | 0;
	const type_t MOUNT_PLAY    = MEDIA_MOUNTED | PLAY;
	const type_t MOUNT_RECORD  = MEDIA_MOUNTED | RECORD;

	const type_t TOGGLE_PAUSE  = PAUSE | UNPAUSE;
	const type_t TAPE_FF       = MEDIA_OTHERS  | FAST_FORWARD;
	const type_t TAPE_REW      = MEDIA_OTHERS  | FAST_REWIND;
	const type_t TAPE_APSS_FF  = MEDIA_OTHERS  | APSS_FORWARD;
	const type_t TAPE_APSS_REW = MEDIA_OTHERS  | APSS_REWIND;
	const type_t TAPE_PLAY     = MEDIA_OTHERS  | PLAY;
	const type_t TAPE_STOP     = MEDIA_OTHERS  | STOP;

	const type_t PROTECT_ON    = WRITE_PROTECT;
	const type_t PROTECT_OFF   = 0;

}
