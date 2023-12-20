/*!
  @todo will move to another directory.
*/

#pragma once

#include "../types/sound_types.h"

// Use this before writing wav_data.
bool DLL_PREFIX write_dummy_wav_header(void *__fio);
// Use this after writng wav_data.
bool DLL_PREFIX set_wav_header(wav_header_t *header, wav_chunk_t *first_chunk, uint16_t channels, uint32_t rate,
							   uint16_t bits, size_t file_length);
bool DLL_PREFIX load_wav_to_stereo(void *__fio, int16_t **left_buf, int16_t **right_buf, uint32_t *rate, int *got_samples);
bool DLL_PREFIX load_wav_to_monoral(void *__fio, int16_t **buffer, uint32_t *rate, int *got_samples);

inline int __FASTCALL decibel_to_volume(int decibel)
{
	// +1 equals +0.5dB (same as fmgen)
	return (int)(1024.0 * pow(10.0, decibel / 40.0) + 0.5);
}

inline int32_t __FASTCALL apply_volume(int32_t sample, int volume)
{
	//int64_t output = ((uint64_t)sample) * ((uint64_t)volume);
	int64_t output = ((int64_t)sample) * volume;
	output >>= 10;
	return output;
}

// High pass filter and Low pass filter.
void DLL_PREFIX calc_high_pass_filter(int32_t* dst, int32_t* src, int sample_freq, int hpf_freq, int samples, double quality = 1.0, bool is_add = true);
void DLL_PREFIX calc_low_pass_filter(int32_t* dst, int32_t* src, int sample_freq, int lpf_freq, int samples, double quality = 1.0, bool is_add = true);
