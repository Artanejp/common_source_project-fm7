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
		
		AUDIO = (1 << 18) + (30 << 16),
		VIDEO = (2 << 18) + (30 << 16),
		ANY_MEDIA = (7 << 18) + (30 << 16),
		END = UINT64_MAX
	};
}


namespace EMU_MESSAGE_TYPE {
	typedef uint64_t type_t ;
	enum {
		NONE,
		MEDIA_MOUNTED,
		MEDIA_REMOVED,
		MEDIA_SUSPENDED,
		VIRT_MEDIA_SELECTED,
		MESSAGE_END
	};
}
