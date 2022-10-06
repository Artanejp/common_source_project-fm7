#pragma once
namespace SOUND_MODULE {
/* SOUND_MODULE */
	enum  class __FORMAT {
		Unsigned_Int,
		Signed_Int,
		Float,
		Double,
	};
	enum  class __BYTEORDER {
		Little,
		Big,
	};
	typedef struct {
		__FORMAT	format;
		__BYTEORDER	endian;
		size_t		word_size;
		size_t		channels;
	} sound_attribute;
/* SOUND_MODULE */
}
