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
		UI_MESSAGE_MASK = ((0xffffffff) << 32),

		EMU_SLOT_MASK = 0x7f,
		MULTIPLE_SLOT_MASK = 0xff,

		END = UINT64_MAX
	};
	const type_t MULTIPLE_SLOT_DETECT_MASK = ((~EMU_SLOT_MASK) & MULTIPLE_SLOT_MASK);

}


namespace EMU_MESSAGE_TYPE {
	typedef uint64_t type_t ;
	enum {
		NONE,
		MEDIA_MOUNTED,
		MEDIA_REMOVED,
		MEDIA_SUSPENDED,
		VIRT_MEDIA_SELECTED,
		MEDIA_WRITE_PROTECT,
		MESSAGE_END
	};
}
