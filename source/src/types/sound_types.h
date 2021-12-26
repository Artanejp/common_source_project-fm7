#pragma once

#include "./basic_types.h"
#include "./pair16_t.h"
#include "./pair32_t.h"

//! @todo Support Big-Endian. 20211123 K.O
// wav file header
#pragma pack(1)
typedef struct {
	char id[4];
	uint32_t size;
} wav_chunk_t;
#pragma pack()

#pragma pack(1)
typedef struct {
	wav_chunk_t riff_chunk;
	char wave[4];
	wav_chunk_t fmt_chunk;
	uint16_t format_id;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t data_speed;
	uint16_t block_size;
	uint16_t sample_bits;
} wav_header_t;
#pragma pack()

//  See http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html.
#pragma pack(1)
typedef struct {
	wav_chunk_t riff_chunk;
	char wave[4];
	wav_chunk_t fmt_chunk;
	uint16_t format_id;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t data_speed;
	uint16_t block_size;
	uint16_t sample_bits;
	uint16_t cbsize; // Extension size.Normaly set to 0.
	wav_chunk_t fact_chunk; // "fact", 4.
} wav_header_float_t;
#pragma pack()

