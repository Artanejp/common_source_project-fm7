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
	enum  {
		NONE,
		BINARY,
		BUBBLE_CASETTE,
		CARTRIDGE,
		COMPACT_DISC,		
		FLOPPY_DISK,
		HARD_DISK,
		LASER_DISC,
		QUICK_DISK,
		TAPE,
		
		AUDIO,
		VIDEO,
		ANY_MEDIA,
		END
	};
}


namespace EMU_MESSAGE_TYPE {
	enum {
		NONE,
		MEDIA_MOUNTED,
		MEDIA_REMOVED,
		MEDIA_SUSPENDED,
		VIRT_MEDIA_SELECTED,
		MESSAGE_END
	};
}

