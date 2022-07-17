// Note: Still working.

#include "osd_base.h"
#include "osd_sound_mod_sdl2.h"

bool SOUND_MODULE_SDL2::initalizeSoundOutputDevice(int channels, int sample_rate, int& samples_per_chunk, int& chunks, std::string device_name)
{
	bool pre_initialized = sound_initialized;
	if(sound_initialized) {
		release_sound();
	}
	int device_num = search_output_device_by_name(device_name);
	if(device_num < 0) {
		return false;
	}
	
}
