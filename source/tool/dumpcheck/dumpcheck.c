
/*
 * DUMP LIST CHECKER
 * (C) 2019 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * Histories:
 *     Sep 30, 2019 : Initial
 *
 * ToDo:
 *   Will implement converting to some formats.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
bool hex2uint8(char *p, uint8_t *r)
{
	if((p == NULL) || (r == NULL)) return false;
	uint8_t nstr[2] = {0};
	for(int i = 0; i < 2; i++) {
		if(p[i] < '0') return false;
		if(p[i] > 'f') return false;
		if(p[i] <= '9') {
			nstr[i] = (uint8_t)(p[i] - '0');
			continue;
		}
		if(p[i] < 'A') return false;
		if(p[i] < 'G') {
			nstr[i] = (uint8_t)(p[i] - 'A') + 10;
			continue;
		}
		if(p[i] < 'a') return false;
		if(p[i] < 'g') {
			nstr[i] = (uint8_t)(p[i] - 'a') + 10;
			continue;
		}
		return false;
	}
	// HEXtoI
	*r = (nstr[0] << 4) + nstr[1];
	return true;
}

bool hex2uint16(char *p, uint16_t *r)
{
	if((p == NULL) || (r == NULL)) return false;
	uint8_t nstr[4] = {0};
	for(int i = 0; i < 4; i++) {
		if(p[i] < '0') return false;
		if(p[i] > 'f') return false;
		if(p[i] <= '9') {
			nstr[i] = (uint8_t)(p[i] - '0');
			continue;
		}
		if(p[i] < 'A') return false;
		if(p[i] < 'G') {
			nstr[i] = (uint8_t)(p[i] - 'A') + 10;
			continue;
		}
		if(p[i] < 'a') return false;
		if(p[i] < 'g') {
			nstr[i] = (uint8_t)(p[i] - 'a') + 10;
			continue;
		}
		return false;
	}
	// HEXtoI
	uint16_t ret = 0;
	for(int i = 0; i < 4; i++) {
		ret <<= 4;
		ret = ret | ((uint16_t)nstr[i]);
	}
	*r = ret;
	return true;
}

bool hex2uint32(char *p, uint32_t *r)
{
	if((p == NULL) || (r == NULL)) return false;
	uint8_t nstr[8] = {0};
	for(int i = 0; i < 8; i++) {
		if(p[i] < '0') return false;
		if(p[i] > 'f') return false;
		if(p[i] <= '9') {
			nstr[i] = (uint8_t)(p[i] - '0');
			continue;
		}
		if(p[i] < 'A') return false;
		if(p[i] < 'G') {
			nstr[i] = (uint8_t)(p[i] - 'A') + 10;
			continue;
		}
		if(p[i] < 'a') return false;
		if(p[i] < 'g') {
			nstr[i] = (uint8_t)(p[i] - 'a') + 10;
			continue;
		}
		return false;
	}
	// HEXtoI
	uint32_t ret = 0;
	for(int i = 0; i < 8; i++) {
		ret <<= 4;
		ret = ret | ((uint32_t)nstr[i]);
	}
	*r = ret;
	return true;
}


void main(int argc, char *argv[])
{
	FILE *in = NULL;
	FILE *out = NULL;
	uint8_t ysum[16];
	uint8_t xsum[16];
	uint32_t addr = 0;
	uint32_t addr_pre = 0;
	bool check_only = 0;
	bool first_line = true;
	if(argc <= 0) {
		printf("DUMP LIST CHECKER\n");
//		printf("dumpcheck -i input_file [-o output_file] [--check_only]\n");
		printf("dumpcheck -i input_file [--check_only]\n");
		exiut(0);
	}
	for(int i = 0; i < argc; i++) {
		if((strcmp(argv[i], "-o") == 0) && (strlen(argv[i]) == 2)) {
			// -o file
			i++;
			if(i >= argc) goto _cmd_start;
			if(out != NULL) {
				fclose(out);
			}
			out = fopen(argv[i], "w");
		} else if((strcmp(argv[i], "-i") == 0) && (strlen(argv[i]) == 2)) {
			// -i file
			i++;
			if(i >= argc) goto _cmd_start;
			if(in != NULL) {
				fclose(in);
			}
			in = fopen(argv[i], "r");
		} else if(strcmp(argv[i], "--check-only") == 0) {
			check_only = true;
		}
	}
_cmd_start:
//	if((in == NULL) || (out == NULL)) exit(2);
	if(in == NULL) {
		exit(1);
	}
	bool is_ok = false;
	char linebuf[256];
	int yrow = 0;
	uint8_t tsum = 0;
	uint8_t yysum[16];
	memset(ysum, 0x00, 16);
	memset(yysum, 0x00, 16);
	memset(xsum, 0x00, 16);
	
	while(feof(in) == 0) {
		memset(linebuf, 0x00, sizeof(linebuf));
		int ptr = 0;
		do {
			int n = fgetc(in);
			if(feof(in) != 0) break;
			if(n == '\r') continue;
			if(n == '\n') break;
			linebuf[ptr++] = n;
		} while(ptr < 256);
		// Check line length
		int l = strlen(linebuf);
		if(l <= 0) continue; // Skip line
		// 
		for(int i = 0; i < l; i++) {
			int c = linebuf[i];
			if((c < ' ') || (c >= 0x80)) {
				continue; 
			}
			linebuf[i] = toupper(c);
		}		

		l = strlen(linebuf);
		ptr = 0;
		// Skip head space
		while(linebuf[ptr] == ' ') {
			ptr++;
		}
		if(ptr >= l) continue; // ALL ' '
		char localnb[16] = {0};
		for(int i = 0; i < 15; i++) {
			if(ptr >= l) break;
			if(linebuf[ptr] == ' ') break;
			localnb[i] = linebuf[ptr];
			ptr++;
		}
//		printf("%s\n", linebuf);
		if((strncmp(localnb, "SUM", 3) == 0) || (strncmp(localnb, "SUM:", 4) == 0)) {
			// SUM LINE
			char lsum[4];
			memset(ysum, 0x00, 16);
			printf("YSUM ->");
			for(int i = 0; i < 16; i++) {
				memset(lsum, 0x00, sizeof(lsum));
				while(linebuf[ptr] == ' ') {
					ptr++;
				}
				lsum[0] = linebuf[ptr++];
				if(ptr >= l) break;
				lsum[1] = linebuf[ptr++];
				if(ptr >= l) break;
				if(!(hex2uint8(lsum, &(ysum[i])))) {
					// SUM ERROR
					printf("Y SUM DATA ERROR at %08X\n", addr);
					break;
				} else {
				}
//				if((linebuf[ptr] != ' ') && (ptr < l)) break; // OK?
				printf(" %02X", ysum[i]);
			}
			printf("\n");
			for(int i = 0; i < 16; i++) {
				if(ysum[i] != yysum[i]) {
					printf("Y SUM ERROR: COLUMN %02X : SRC=%02X / DATA=%02X\n", i, ysum[i], yysum[i]);
				}
			}
			uint8_t totalsum = 0;
			memset(lsum, 0x00, sizeof(lsum));
			while(linebuf[ptr] == ' ') {
				ptr++;
			}
			if(linebuf[ptr] == ':') {
				ptr++;
				if(ptr >= l) continue;
				lsum[0] = linebuf[ptr++];
				if(ptr >= l) continue;
				lsum[1] = linebuf[ptr++];
//				if(ptr >= l) continue;
				if(!(hex2uint8(lsum, &totalsum))) {
					// SUM ERROR
					printf("TOTAL SUM DATA ERROR at %08X\n", addr);
					continue;
				}
				printf("TOTAL SRC SUM=%02X TOTAL DATA SUM=%02X --> %s\n", totalsum, tsum, (tsum == totalsum) ? "OK" : "NG");
			} else {
				printf("TOTAL SUM IS NOT PRESENTED\n");
			}
			yrow = 0;
			tsum = 0;
			memset(yysum, 0x00, 16);
		} else {
			if((strlen(localnb) < 4) || (strlen(localnb) > 8)) {
				printf("Illegal address\n");
				continue;
			}
			if(strlen(localnb) == 4) {
				uint16_t tmpadr;
				if(hex2uint16(localnb, &tmpadr)) {
					addr = tmpadr;
					if(!(first_line)) {
						if(addr_pre != (addr - 0x10)) {
							printf("WARN: ADDRESS DIFFER DATA=%08X PRESENT=%08X\n", addr, addr_pre + 0x10);
						}
					}
					addr_pre = addr;
					first_line = false;
				} else {
					printf("Illegal address\n");
					continue;
				}
			} else if(strlen(localnb) == 8) {
				uint32_t tmpadr;
				if(hex2uint32(localnb, &tmpadr)) {
					addr = tmpadr;
					if(!(first_line)) {
						if(addr_pre != (addr - 0x10)) {
							printf("WARN: ADDRESS DIFFER DATA=%08X PRESENT=%08X\n", addr, addr_pre + 0x10);
						}
					}
					addr_pre = addr;
					first_line = false;
				} else {
					printf("Illegal address\n");
					continue;
				}
			} else {
				printf("Illegal address\n");
				continue;
			}
			// Address OK
			uint8_t xdata[16] = {0};
			char sbufd[4] = {0};
			uint8_t sum = 0;
			int bytes = 0;
			for(int i = 0; i < 16; i++) {
				memset(sbufd, 0x00, sizeof(sbufd));
				while(linebuf[ptr] == ' ') {
					ptr++;
				}
				if(ptr >= l) continue;
				for(int j = 0; j < 4; j++) {
					sbufd[j] = linebuf[ptr++];
					if(ptr >= l) break;
					if(linebuf[ptr] == ' ') break;
				}
				if(ptr >= l) break;
				if(strlen(sbufd) != 2) break;
				if(hex2uint8(sbufd, &(xdata[i]))) {
					sum = sum + xdata[i];
					tsum = tsum + xdata[i];
					yysum[i] = yysum[i] + xdata[i];
					bytes++;
				} else {
					printf("Error data at line %08X\n", addr);
					break;
				}
			}
			if(bytes == 16) {
				// Get Sum
				uint8_t csum = 0;
				memset(sbufd, 0x00, sizeof(sbufd));
				while(linebuf[ptr] == ' ') {
					ptr++;
				}
				if(ptr >= l) continue;
				for(int j = 0; j < 4; j++) {
					sbufd[j] = linebuf[ptr++];
					if(ptr >= l) break;
					if(linebuf[ptr] == ' ') break;
				}
				if(strlen(sbufd) == 3) {
					if(sbufd[0] == ':') {
						if(hex2uint8(&(sbufd[1]), &csum)) {
							printf("ADDR=%08X SRC SUM=%02X DATA SUM=%02X --> %s\n", addr, csum, sum, (csum == sum) ? "OK" : "NG");
							xsum[yrow % 16] = sum;
						} else {
							printf("ADDR=%08X SRC SUM ERROR: DATA SUM=%02X\n", addr, sum);
						}
					} else {
						printf("ADDR=%08X SRC SUM ERROR: DATA SUM=%02X\n", addr, sum);
					}
				} else {
					printf("ADDR=%08X SRC SUM ERROR: DATA SUM=%02X\n", addr, sum);
				}
			} else {
				printf("ADDR=%08X : Too few bytes: \n", addr);
			}
			yrow++;
		}
	}
	fclose(in);
}

