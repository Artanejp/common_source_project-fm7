// wav.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif
#ifndef uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifndef int8_t
typedef signed char int8_t;
#endif
#ifndef int16_t
typedef signed short int16_t;
#endif
#ifndef int32_t
typedef signed int int32_t;
#endif

#pragma pack(1)
typedef struct {
	char RIFF[4];
	uint32_t file_len;
	char WAVE[4];
	char fmt[4];
	uint32_t fmt_size;
	uint16_t format_id;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t data_speed;
	uint16_t block_size;
	uint16_t sample_bits;
} wav_header_t;
#pragma pack()

#pragma pack(1)
typedef struct {
	char data[4];
	uint32_t data_len;
} wav_data_t;
#pragma pack()

int _tmain(int argc, _TCHAR* argv[])
{
	if(argc < 2) {
		printf("wav_apss (wav file) [threshold]\n\n");
		printf("Cleanup MZ-2500 tape file for APSS.\n");
		printf("The wave file should be pcm format, 16bit and 2ch.\n");
		return -1;
	}
	int threshold = 4096;
	if(argc > 2) {
		threshold = _ttoi(argv[2]);
	}
	
	FILE *fp = _tfopen(argv[1],_T("rb"));
	
	// check wav header
	wav_header_t header;
	wav_data_t data;
	
	fread(&header, sizeof(header), 1, fp);
	fseek(fp, header.fmt_size - 0x10, SEEK_CUR);
	fread(&data, sizeof(data), 1, fp);
	long sample_top = ftell(fp);
	
	if(header.format_id != 1) {
		printf("The source wave file should be a pcm format.\n");
		fclose(fp);
		return -1;
	}
	if(header.channels != 2 || header.sample_bits != 16) {
		printf("The source wave file should be 16bit and 2ch.\n");
		fclose(fp);
		return -1;
	}
	int samples = data.data_len / 4;
	
	// load samples
	int16_t* buffer_l = (int16_t *)malloc(samples * sizeof(int16_t));
	int16_t* buffer_r = (int16_t *)malloc(samples * sizeof(int16_t));
	
	typedef union {
		int16_t s16;
		struct {
			uint8_t l, h;
		} b;
	} sample_pair;
	sample_pair sample;
	
	int count = 0, start, prev = 0;
	
	for(int i = 0; i < samples; i++) {
		sample.b.l = fgetc(fp);
		sample.b.h = fgetc(fp);
		buffer_l[i] = sample.s16;
		
		sample.b.l = fgetc(fp);
		sample.b.h = fgetc(fp);
		buffer_r[i] = sample.s16;
		
		if(abs(buffer_l[i]) > threshold) {
			if(count == 0) {
				start = i;
			}
			count = 32 * header.sample_rate / 44100;
		}
		if(count > 0 && --count == 0) {
			int end = i;
			unsigned int width = end - start + 1;
			
			if(width > header.sample_rate * 250 / 1000) {
				int count_p = 0, count_m = 0;
				double volume = 0;
				for(int j = start; j <= end; j++) {
					if(buffer_l[j] > threshold) {
						count_p++;
					} else if(buffer_l[j] < -threshold) {
						count_m++;
					}
					volume += buffer_l[j];
				}
				if((double)count_p / (double)width > 0.2 && (double)count_m / (double)width > 0.2) {
					int16_t volume_ave = (int16_t)(volume / (double)width + 0.5);
					int sec = start / header.sample_rate;
					printf("Start = %d:%d\tWidth = %d (msec)\n", sec / 60, sec % 60, (int)(1000 * width / header.sample_rate));
					for(int j = prev; j < start; j++) {
						buffer_l[j] = 0;
					}
					for(int j = start; j <= end; j++) {
						buffer_l[j] = buffer_l[j] > volume_ave ? 16384 : -16384;
						buffer_r[j] = 0;
					}
					prev = end + 1;
				}
			}
		}
	}
	for(int j = prev; j < samples; j++) {
		buffer_l[j] = 0;
	}
	
	// output wav file
	FILE *fo = _tfopen(_T("output.wav"), _T("wb"));
	fseek(fp, 0, SEEK_SET);
	for(long i = 0; i < sample_top; i++) {
		fputc(fgetc(fp), fo);
	}
	for(int i = 0; i < samples; i++) {
		sample.s16 = buffer_l[i];
		fputc(sample.b.l, fo);
		fputc(sample.b.h, fo);
		
		sample.s16 = buffer_r[i];
		fputc(sample.b.l, fo);
		fputc(sample.b.h, fo);
	}
	fclose(fo);
	
	free(buffer_l);
	free(buffer_r);
	
	fclose(fp);
	
	return 0;
}

