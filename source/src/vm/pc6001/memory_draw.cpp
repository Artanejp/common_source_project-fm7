/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         Refresh.c                       **/
/**                                                         **/
/** modified by Windy 2002-2004                             **/
/** by ISHIOKA Hiroshi 1998,1999                            **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/** and Adaptions for any X-terminal by Arnold Metselaar    **/
/*************************************************************/

/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.07.15-

	[ draw ]
*/

#include "memory.h"

#define CGROM1		(MEMORY_BASE + CGROM1_BASE)
#define CGROM5		(MEMORY_BASE + CGROM5_BASE)
#define CGROM6		(MEMORY_BASE + CGROM6_BASE)

#define SETSCRVARM1(y1)	dest = &screen[y1][0];W=0;
#define SETSCRVARM5(y1)	dest = &screen[y1][0];W=0;
#define M1HEIGHT 192
#define M5HEIGHT 200
#define M6HEIGHT 204
#define SeqPix21(c) dest[X*8+W]=c;W++;
#define SeqPix41(c) dest[X*8+W]=c;W++;dest[X*8+W]=c;W++;

void MEMORY::draw_screen()
{
	if (CRTKILL) {
		for(int y = 0; y < 400; y++) {
			scrntype_t* dest = emu->get_screen_buffer(y);
			memset(dest, 0, 640 * sizeof(scrntype_t));
		}
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	} else if (static_cast<VM *>(vm)->sr_mode) {
		if (CRTMode2) {
			if (CRTMode3) RefreshScr63();
			else RefreshScr62();
		} else RefreshScr61();
		// copy to screen
		emu->set_vm_screen_lines(200);
		if (bitmap) {
			for(int y = 0; y < 200; y++) {
				scrntype_t* dest = emu->get_screen_buffer(y*2);
				scrntype_t* dest1 = emu->get_screen_buffer(y*2+1);
				for(int x = 0; x < 320; x++) {
					dest[x*2] = dest[x*2+1] = palette_pc[screen[y][x*2]];
				}
				if(config.scan_line) {
					memset(dest1, 0, 640 * sizeof(scrntype_t));
				} else {
					my_memcpy(dest1, dest, 640 * sizeof(scrntype_t));
				}
			}
		} else if (cols==40) {
			for(int y = 0; y < 200; y++) {
				scrntype_t* dest = emu->get_screen_buffer(y*2);
				scrntype_t* dest1 = emu->get_screen_buffer(y*2+1);
				for(int x = 0; x < 320; x++) {
					dest[x*2] = dest[x*2+1] = palette_pc[screen[y][x]];
				}
				if(config.scan_line) {
					memset(dest1, 0, 640 * sizeof(scrntype_t));
				} else {
					my_memcpy(dest1, dest, 640 * sizeof(scrntype_t));
				}
			}
		} else {
			for(int y = 0; y < 200; y++) {
				scrntype_t* dest = emu->get_screen_buffer(y*2);
				scrntype_t* dest1 = emu->get_screen_buffer(y*2+1);
				for(int x = 0; x < 640; x++) {
					dest[x] = palette_pc[screen[y][x]];
				}
				if(config.scan_line) {
					memset(dest1, 0, 640 * sizeof(scrntype_t));
				} else {
					my_memcpy(dest1, dest, 640 * sizeof(scrntype_t));
				}
			}
		}
#endif
	} else {
		if (CRTMode1) {
			if (CRTMode2)
				if (CRTMode3) RefreshScr54();
				else RefreshScr53();
			else RefreshScr51();
			// copy to screen
			emu->set_vm_screen_lines(200);
			for(int y = 0; y < 200; y++) {
				scrntype_t* dest = emu->get_screen_buffer(y*2);
				scrntype_t* dest1 = emu->get_screen_buffer(y*2+1);
				for(int x = 0; x < 320; x++) {
					dest[x*2] = dest[x*2+1] = palette_pc[screen[y][x]];
				}
				if(config.scan_line) {
					memset(dest1, 0, 640 * sizeof(scrntype_t));
				} else {
					my_memcpy(dest1, dest, 640 * sizeof(scrntype_t));
				}
			}
		} else {
			RefreshScr10();
			// copy to screen
			emu->set_vm_screen_lines(200);
			for(int y = 0; y < 200; y++) {
				scrntype_t* dest = emu->get_screen_buffer(y*2);
				scrntype_t* dest1 = emu->get_screen_buffer(y*2+1);
				for(int x = 0; x < 320; x++) {
					if (x >= 32 && x < 288 && y >=4 && y < 196) {
						dest[x*2] = dest[x*2+1] = palette_pc[screen[y-4][x-32]];
					} else {
						dest[x*2] = dest[x*2+1] = palette_pc[8];
					}
				}
				if(config.scan_line) {
					memset(dest1, 0, 640 * sizeof(scrntype_t));
				} else {
					my_memcpy(dest1, dest, 640 * sizeof(scrntype_t));
				}
			}
		}
	}
	emu->screen_skip_line(true);
}

// RefreshScr10: N60-BASIC select function
void MEMORY::RefreshScr10()
{
	if ((*VRAM&0x80) == 0x00)
		RefreshScr11();
	else
		switch (*(VRAM)&0x1C) {
		case 0x00: case 0x10: //  64x 64 color / 128x 64
			RefreshScr13a(); break;
		case 0x08: // 128x 64 color
			RefreshScr13b(); break;
		case 0x18: // 128x 96
			RefreshScr13c(); break;
		case 0x04: // 128x 96 color
			RefreshScr13d(); break;
		case 0x14: // 128x192
			RefreshScr13e(); break;
		default: // 128x192 color / 256x192
			RefreshScr13(); break;
		}
}

// RefreshScr11: N60-BASIC screen 1,2
void MEMORY::RefreshScr11()
{
	uint8_t X,Y,K;
	int FC,BC;
	uint8_t *S,*T1,*T2;
	uint8_t *G;

	G = CGROM;		/* CGROM */ 
	T1 = VRAM;		/* attribute data */
	T2 = VRAM+0x0200;	/* ascii/semi-graphic data */
	for(Y=0; Y<M1HEIGHT; Y++) {
		SETSCRVARM1(Y);	/* Drawing area */
		for(X=0; X<32; X++, T1++, T2++) {
			/* get CGROM address and color */
			if (*T1&0x40) {	/* if semi-graphic */
				if (*T1&0x20) {		/* semi-graphic 6 */
					S = G+((*T2&0x3f)<<4)+0x1000;
					FC = BPal12[(*T1&0x02)<<1|(*T2)>>6]; BC = BPal[8];
				} else {		/* semi-graphic 4 */
					S = G+((*T2&0x0f)<<4)+0x2000;
					FC = BPal12[(*T2&0x70)>>4]; BC = BPal[8];
				}
			} else {		/* if normal character */
				S = G+((*T2)<<4); 
				FC = BPal11[(*T1&0x03)]; BC = BPal11[(*T1&0x03)^0x01];
			}
			K=*(S+Y%12);
W=0;
			SeqPix21(K&0x80? FC:BC); SeqPix21(K&0x40? FC:BC);
			SeqPix21(K&0x20? FC:BC); SeqPix21(K&0x10? FC:BC);
			SeqPix21(K&0x08? FC:BC); SeqPix21(K&0x04? FC:BC);
			SeqPix21(K&0x02? FC:BC); SeqPix21(K&0x01? FC:BC);
		}
		if (Y%12!=11) { T1-=32; T2-=32; }
	}
}

// RefreshScr13: N60-BASIC screen 3,4
void MEMORY::RefreshScr13()
{
	uint8_t X,Y;
	uint8_t *T1,*T2;
	uint8_t attr;

	T1 = VRAM;		/* attribute data */
	T2 = VRAM+0x0200;	/* graphic data */
	for (Y=0; Y<M1HEIGHT; Y++) {
		SETSCRVARM1(Y);	/* Drawing area */
		for (X=0; X<32; X++,T1++,T2++) {
			if (*T1&0x10) { /* 256x192 (SCREEN 4) */
				attr = *T1&0x02;
W=0;
				SeqPix21(BPal14[attr|(*T2&0x80)>>7]);
				SeqPix21(BPal14[attr|(*T2&0x40)>>6]);
				SeqPix21(BPal14[attr|(*T2&0x20)>>5]);
				SeqPix21(BPal14[attr|(*T2&0x10)>>4]);
				SeqPix21(BPal14[attr|(*T2&0x08)>>3]);
				SeqPix21(BPal14[attr|(*T2&0x04)>>2]);
				SeqPix21(BPal14[attr|(*T2&0x02)>>1]);
				SeqPix21(BPal14[attr|(*T2&0x01)   ]);
			} else { /* 128x192 color (SCREEN 3) */
				attr = (*T1&0x02)<<1;
W=0;
				SeqPix41(BPal13[attr|(*T2&0xC0)>>6]);
				SeqPix41(BPal13[attr|(*T2&0x30)>>4]);
				SeqPix41(BPal13[attr|(*T2&0x0C)>>2]);
				SeqPix41(BPal13[attr|(*T2&0x03)   ]);
			}
		}
		if (T1 == VRAM+0x200) T1=VRAM;
	}
}

// RefreshScr13a: N60-BASIC screen 3,4
void MEMORY::RefreshScr13a() /*  64x 64 color / 128x 64 */
{
	uint8_t X,Y;
	uint8_t *T1,*T2;
	uint8_t attr;
	int L;

	T1 = VRAM;		/* attribute data */
	T2 = VRAM+0x0200;	/* graphic data */
	for (Y=0; Y<M1HEIGHT; Y++) {
		SETSCRVARM1(Y);	/* Drawing area */
		for (X=0; X<16; X++,T1++,T2++) {
			if (*T1&0x10) { /* 128x 64 */
				attr = *T1&0x02;
W=0;
				SeqPix41(BPal14[attr|(*T2&0x80)>>7]);
				SeqPix41(BPal14[attr|(*T2&0x40)>>6]);
				SeqPix41(BPal14[attr|(*T2&0x20)>>5]);
				SeqPix41(BPal14[attr|(*T2&0x10)>>4]);
				SeqPix41(BPal14[attr|(*T2&0x08)>>3]);
				SeqPix41(BPal14[attr|(*T2&0x04)>>2]);
				SeqPix41(BPal14[attr|(*T2&0x02)>>1]);
				SeqPix41(BPal14[attr|(*T2&0x01)   ]);
			} else { /*  64x 64 color */
				attr = (*T1&0x02)<<1;
W=0;
				SeqPix41(L=BPal13[attr|(*T2&0xC0)>>6]);
				SeqPix41(L);
				SeqPix41(L=BPal13[attr|(*T2&0x30)>>4]);
				SeqPix41(L);
				SeqPix41(L=BPal13[attr|(*T2&0x0C)>>2]);
				SeqPix41(L);
				SeqPix41(L=BPal13[attr|(*T2&0x03)   ]);
				SeqPix41(L);
			}
		}
		if (Y%3 != 2) { T1-=16; T2-=16; }
		else if (T1 == VRAM+0x200) T1=VRAM;
	}
}

// RefreshScr13b: N60-BASIC screen 3,4
void MEMORY::RefreshScr13b() /* 128x 64 color */
{
	uint8_t X,Y;
	uint8_t *T1,*T2;
	uint8_t attr;

	T1 = VRAM;		/* attribute data */
	T2 = VRAM+0x0200;	/* graphic data */
	for (Y=0; Y<M1HEIGHT; Y++) {
		SETSCRVARM1(Y);	/* Drawing area */
		for (X=0; X<32; X++,T1++,T2++) {
W=0;
			attr = (*T1&0x02)<<1;
			SeqPix41(BPal13[attr|(*T2&0xC0)>>6]);
			SeqPix41(BPal13[attr|(*T2&0x30)>>4]);
			SeqPix41(BPal13[attr|(*T2&0x0C)>>2]);
			SeqPix41(BPal13[attr|(*T2&0x03)   ]);
		}
		if (Y%3 != 2) { T1-=32; T2-=32; }
		else if (T1 == VRAM+0x200) T1=VRAM;
	}
}

// RefreshScr13c: N60-BASIC screen 3,4
void MEMORY::RefreshScr13c() /* 128x 96 */
{
	uint8_t X,Y;
	uint8_t *T1,*T2;
	uint8_t attr;

	T1 = VRAM;		/* attribute data */
	T2 = VRAM+0x0200;	/* graphic data */
	for (Y=0; Y<M1HEIGHT; Y++) {
		SETSCRVARM1(Y);	/* Drawing area */
		for (X=0; X<16; X++,T1++,T2++) {
			attr = *T1&0x02;
W=0;
			SeqPix41(BPal14[attr|(*T2&0x80)>>7]);
			SeqPix41(BPal14[attr|(*T2&0x40)>>6]);
			SeqPix41(BPal14[attr|(*T2&0x20)>>5]);
			SeqPix41(BPal14[attr|(*T2&0x10)>>4]);
			SeqPix41(BPal14[attr|(*T2&0x08)>>3]);
			SeqPix41(BPal14[attr|(*T2&0x04)>>2]);
			SeqPix41(BPal14[attr|(*T2&0x02)>>1]);
			SeqPix41(BPal14[attr|(*T2&0x01)   ]);
		}
		if (!(Y&1)) { T1-=16; T2-=16; }
		else if (T1 == VRAM+0x200) T1=VRAM;
	}
}

// RefreshScr13d: N60-BASIC screen 3,4
void MEMORY::RefreshScr13d() /* 128x 96 color */
{
	uint8_t X,Y;
	uint8_t *T1,*T2;
	uint8_t attr;

	T1 = VRAM;		/* attribute data */
	T2 = VRAM+0x0200;	/* graphic data */
	for (Y=0; Y<M1HEIGHT; Y++) {
		SETSCRVARM1(Y);	/* Drawing area */
		for (X=0; X<32; X++,T1++,T2++) {
			attr = (*T1&0x02)<<1;
W=0;
			SeqPix41(BPal13[attr|(*T2&0xC0)>>6]);
			SeqPix41(BPal13[attr|(*T2&0x30)>>4]);
			SeqPix41(BPal13[attr|(*T2&0x0C)>>2]);
			SeqPix41(BPal13[attr|(*T2&0x03)   ]);
		}
		if (!(Y&1)) { T1-=32; T2-=32; }
		else if (T1 == VRAM+0x200) T1=VRAM;
	}
}

// RefreshScr13e: N60-BASIC screen 3,4
void MEMORY::RefreshScr13e() /* 128x192 */
{
	uint8_t X,Y;
	uint8_t *T1,*T2;
	uint8_t attr;

	T1 = VRAM;		/* attribute data */
	T2 = VRAM+0x0200;	/* graphic data */
	for (Y=0; Y<M1HEIGHT; Y++) {
		SETSCRVARM1(Y);	/* Drawing area */
		for (X=0; X<16; X++,T1++,T2++) {
			attr = *T1&0x02;
W=0;
			SeqPix41(BPal14[attr|(*T2&0x80)>>7]);
			SeqPix41(BPal14[attr|(*T2&0x40)>>6]);
			SeqPix41(BPal14[attr|(*T2&0x20)>>5]);
			SeqPix41(BPal14[attr|(*T2&0x10)>>4]);
			SeqPix41(BPal14[attr|(*T2&0x08)>>3]);
			SeqPix41(BPal14[attr|(*T2&0x04)>>2]);
			SeqPix41(BPal14[attr|(*T2&0x02)>>1]);
			SeqPix41(BPal14[attr|(*T2&0x01)   ]);
		}
		if (T1 == VRAM+0x200) T1=VRAM;
	}
}

// RefreshScr51: N60m/66-BASIC screen 1,2
void MEMORY::RefreshScr51()
{
	uint8_t X,Y,K;
	int FC,BC;
	uint8_t *S,*T1,*T2;
	uint8_t *G;

	G = CGROM;		/* CGROM */ 
	T1 = VRAM;		/* attribute data */
	T2 = VRAM+0x0400;	/* ascii/semi-graphic data */
	for(Y=0; Y<M5HEIGHT; Y++) {
		SETSCRVARM5(Y);	/* Drawing area */
		for(X=0; X<40; X++, T1++, T2++) {
			/* get CGROM address and color */
			S = G+(*T2<<4)+(*T1&0x80?0x1000:0);
			FC = BPal[(*T1)&0x0F]; BC = BPal[(((*T1)&0x70)>>4)|CSS2];
			K=*(S+Y%10);
W=0;
			SeqPix21(K&0x80? FC:BC); SeqPix21(K&0x40? FC:BC);
			SeqPix21(K&0x20? FC:BC); SeqPix21(K&0x10? FC:BC);
			SeqPix21(K&0x08? FC:BC); SeqPix21(K&0x04? FC:BC);
			SeqPix21(K&0x02? FC:BC); SeqPix21(K&0x01? FC:BC);
		}
		if (Y%10!=9) { T1-=40; T2-=40; }
	}
}

// RefreshScr53: N60m/66-BASIC screen 3
void MEMORY::RefreshScr53()
{
	uint8_t X,Y;
	uint8_t *T1,*T2;

	T1 = VRAM;		/* attribute data */
	T2 = VRAM+0x2000;	/* graphic data */
	for(Y=0; Y<M5HEIGHT; Y++) {
		SETSCRVARM5(Y);	/* Drawing area */
		for(X=0; X<40; X++) {
W=0;
			SeqPix41(BPal53[CSS3|((*T1)&0xC0)>>6|((*T2)&0xC0)>>4]);
			SeqPix41(BPal53[CSS3|((*T1)&0x30)>>4|((*T2)&0x30)>>2]);
			SeqPix41(BPal53[CSS3|((*T1)&0x0C)>>2|((*T2)&0x0C)   ]);
			SeqPix41(BPal53[CSS3|((*T1)&0x03)   |((*T2)&0x03)<<2]);
			T1++; T2++;
		}
	}
}

// RefreshScr54: N60m/66-BASIC screen 4
void MEMORY::RefreshScr54()
{
	uint8_t X,Y;
	uint8_t *T1,*T2;
	uint8_t cssor;

	T1 = VRAM;		/* attribute data */
	T2 = VRAM+0x2000;	/* graphic data */
	/* CSS OR */
	cssor = CSS3|CSS2|CSS1;
	for(Y=0; Y<M5HEIGHT; Y++) {
		SETSCRVARM5(Y);	/* Drawing area */
		for(X=0; X<40; X++) {
W=0;
			SeqPix21(BPal53[cssor|((*T1)&0x80)>>7|((*T2)&0x80)>>6]);
			SeqPix21(BPal53[cssor|((*T1)&0x40)>>6|((*T2)&0x40)>>5]);
			SeqPix21(BPal53[cssor|((*T1)&0x20)>>5|((*T2)&0x20)>>4]);
			SeqPix21(BPal53[cssor|((*T1)&0x10)>>4|((*T2)&0x10)>>3]);
			SeqPix21(BPal53[cssor|((*T1)&0x08)>>3|((*T2)&0x08)>>2]);
			SeqPix21(BPal53[cssor|((*T1)&0x04)>>2|((*T2)&0x04)>>1]);
			SeqPix21(BPal53[cssor|((*T1)&0x02)>>1|((*T2)&0x02)   ]);
			SeqPix21(BPal53[cssor|((*T1)&0x01)   |((*T2)&0x01)<<1]);
			T1++;T2++;
		}
	}
}

#if defined(_PC6601SR) || defined(_PC6001MK2SR)
// RefreshScr61: N66-SR BASIC screen 1
void MEMORY::RefreshScr61()
{
	uint8_t X,Y,K;
	register int FC,BC;
	uint8_t *S,*T1,*T2;
	uint8_t *G;
	int high;
	int addr;
	int semi_addr;

	G = CGROM;		/* CGROM */
	T1 = TEXTVRAM+1;		/* attribute data */
	T2 = TEXTVRAM;	        	/* ascii/semi-graphic data */
	high= (rows==20)? 10:8;       /* charactor's high   SR BASIC 2002/2/23 */
	addr= (rows==20)? 0x2000: 0x1000; /* CGROM address  SR BASIC 2002/2/23 */
	semi_addr= (rows==20)? 0x3000: 0x4000; /* semi address 2003/7/8 */
	for(Y=0; Y<M5HEIGHT; Y++) {
		SETSCRVARM5(Y);	/* Drawing area */
		for(X=0; X< cols; X++, T1+=2, T2+=2) {
W=0;
			S= G+(*T2<<4)+(*T1&0x80?semi_addr:addr); /*for CGROM6 SR semi graph 2002/2/23*//* 2003/7/8 */
			FC = BPal[ (*T1)&0x0F];					   /* fast Palet (2002/9/27) */
			BC = BPal[ (((*T1)&0x70)>>4) |CSS2];
			K=*(S+Y%high);			/* character high 2002/2/23 */
			SeqPix21(K&0x80? FC:BC); SeqPix21(K&0x40? FC:BC);
			SeqPix21(K&0x20? FC:BC); SeqPix21(K&0x10? FC:BC);
			SeqPix21(K&0x08? FC:BC); SeqPix21(K&0x04? FC:BC);
			SeqPix21(K&0x02? FC:BC); SeqPix21(K&0x01? FC:BC);
		}
		if (Y% high!=high-1) { T1-=cols*2; T2-=cols*2; } /* character high  2002/2/23 */
	}
}

// RefreshScr62  N66-SR BASIC screen 2
void MEMORY::RefreshScr62()
{
	uint8_t X,Y;
	uint8_t *T1,*T2;
	T1 = VRAM+ 0x1a00;
	T2 = VRAM;
	for(Y=0; Y<M6HEIGHT; Y++) {
		SETSCRVARM5(Y); /* Drawing area */
		for(X=0; X< 64; X++) {
W=0;
			SeqPix41(BPal53[ CSS3|((*T1)&0x0f) ]);
			SeqPix41(BPal53[ CSS3|((*T1)&0xf0)>>4 ]);
			T1++;
			SeqPix41(BPal53[ CSS3|((*T1)&0x0f) ]);
			SeqPix41(BPal53[ CSS3|((*T1)&0xf0)>>4 ]);
			T1+=3;
		}
		for(X=64 ;X<80 ; X++) {
W=0;
			SeqPix41(BPal53[ CSS3|((*T2)&0x0f) ]);
			SeqPix41(BPal53[ CSS3|((*T2)&0xf0)>>4 ]);
			T2++;
			SeqPix41(BPal53[ CSS3|((*T2)&0x0f) ]);
			SeqPix41(BPal53[ CSS3|((*T2)&0xf0)>>4 ]);
			T2+=3;
		}
		if( (Y & 1)==0)
			{ T1-=(254);  T2-=(62);}
		else
			{ T1-=2; T2-=2; }
	}
}

// RefreshScr63  N66-SR BASIC screen 3
void MEMORY::RefreshScr63()
{
	uint8_t X,Y;
	uint8_t *T1,*T2;
	uint8_t cssor;

	T1 = VRAM+ 0x1a00;
	T2 = VRAM;
	cssor = CSS3|CSS2|CSS1;
	for(Y=0; Y<M6HEIGHT; Y++) {
		SETSCRVARM5(Y); 	/* Drawing area */
		for(X=0; X< 64; X++) {
W=0;
			SeqPix41(BPal53[cssor|((*T1)&0x80)>>7|((*(T1+1))&0x80)>>6]);
			SeqPix41(BPal53[cssor|((*T1)&0x40)>>6|((*(T1+1))&0x40)>>5]);
			SeqPix41(BPal53[cssor|((*T1)&0x20)>>5|((*(T1+1))&0x20)>>4]);
			SeqPix41(BPal53[cssor|((*T1)&0x10)>>4|((*(T1+1))&0x10)>>3]);
			SeqPix41(BPal53[cssor|((*T1)&0x08)>>3|((*(T1+1))&0x08)>>2]);
			SeqPix41(BPal53[cssor|((*T1)&0x04)>>2|((*(T1+1))&0x04)>>1]);
			SeqPix41(BPal53[cssor|((*T1)&0x02)>>1|((*(T1+1))&0x02)   ]);
			SeqPix41(BPal53[cssor|((*T1)&0x01)   |((*(T1+1))&0x01)<<1]);
			T1+=4;
		}
		for(X=64 ;X<80 ; X++) {
W=0;
			SeqPix41(BPal53[cssor|((*T2)&0x80)>>7|((*(T2+1))&0x80)>>6]);
			SeqPix41(BPal53[cssor|((*T2)&0x40)>>6|((*(T2+1))&0x40)>>5]);
			SeqPix41(BPal53[cssor|((*T2)&0x20)>>5|((*(T2+1))&0x20)>>4]);
			SeqPix41(BPal53[cssor|((*T2)&0x10)>>4|((*(T2+1))&0x10)>>3]);
			SeqPix41(BPal53[cssor|((*T2)&0x08)>>3|((*(T2+1))&0x08)>>2]);
			SeqPix41(BPal53[cssor|((*T2)&0x04)>>2|((*(T2+1))&0x04)>>1]);
			SeqPix41(BPal53[cssor|((*T2)&0x02)>>1|((*(T2+1))&0x02)   ]);
			SeqPix41(BPal53[cssor|((*T2)&0x01)   |((*(T2+1))&0x01)<<1]);
			T2+=4;
		}
		if( (Y & 1)==0)
			{ T1-=(254);  T2-=(62);}
		else
			{ T1-=2; T2-=2; }
	}
}

void MEMORY::do_palet(int dest,int src)
{
	int textpalet2[16]={0,4,1,5,2,6,3,7,8,12,9,13,10,14,11,15}; /*  color code-> VRAM code*/
	// *************** for RefreshScr 53/54/62/63 ***************************
	if ((CSS3 & 0x10) ==0 )	// CSS3 =0
	{
		if(dest>=0 && dest<32 && src>=0 && src<32)
		{
			BPal53[dest]= BPal62[src];
		}
	} else {				// CSS3 =1
		if (dest>=0 && dest<32 && src>=0 && src<32)
		{
			int dest1,dest2;
			switch( dest+1)
			{
			case 16: dest1 =13; dest2 = 5; break;
			case 15: dest1 =10; dest2 = 2; break;
			case 14: dest1 =14; dest2 = 6; break;
			case 13: dest1 = 1; dest2 = 9; break;
			}
			BPal53[16+dest1-1]= BPal62[src];
			BPal53[16+dest2-1]= BPal62[src];
		}
	}
	// ************** for RefreshScr51/61 **************************
	if(dest>=0 && dest<16 && src>=0 && src<16)
		BPal[textpalet2[dest]]= BPal61[ textpalet2[src]];  
}

void MEMORY::make_semigraph(void)
{
	uint8_t *P;
	unsigned int i, j, m1, m2;
	P = CGROM1+0x1000;
	for(i=0; i<64; i++) {
		for(j=0; j<16; j++) {
			switch (j/4) {
			case 0: m1=0x20; m2=0x10; break;
			case 1: m1=0x08; m2=0x04; break;
			case 2: m1=0x02; m2=0x01; break;
			default: m1=m2=0;
			};
			*P++=(i&m1 ? 0xF0: 0) | (i&m2 ? 0x0F: 0);
		}
	}
	P = CGROM1+0x2000;
	for(i=0; i<16; i++) {
		for(j=0; j<16; j++) {
			switch (j/6) {
			case 0: m1=0x08; m2=0x04; break;
			case 1: m1=0x02; m2=0x01; break;
			default: m1=m2=0;
			};
			*P++=(i&m1 ? 0xF0: 0) | (i&m2 ? 0x0F: 0);
		}
	}
	P = CGROM6+0x4000;
	for(i=0; i<256; i++) {
		for(j=0; j<16; j++) {
			switch (j/2) {
			case 0: m1=0x80; m2=0x40; break;
			case 1: m1=0x20; m2=0x10; break;
			case 2: m1=0x08; m2=0x04; break;
			case 3: m1=0x02; m2=0x01; break;
			default: m1=m2=0;
			};
			*P++=(i&m1 ? 0xF0: 0) | (i&m2 ? 0x0F: 0);
		}
	}
}

int MEMORY::chk_gvram(uint32_t A,int flag)
{
	if (port60[ (A>>13)+flag ]==0x00 && bitmap)	// VRAM の先頭かつ、CRTが BITMAP mode
		return 1;
	return 0;
}

uint8_t MEMORY::gvram_read(uint32_t A)
{
	uint8_t* adr;
	uint8_t  ret;
	int x,y,z,w,off;

	x = A & 0x1fff;
	y = portCF*16+portCE;		      /* y座標 */
	if( y >=204) y-=204;		      /* Y座標 204 以上だと 204 引く add 2003/10/22 */
	w = (x <256) ? 256: 64;          /* width:0..255 なら256 / 256..319なら 64にする*/
	off=(x <256) ? 0x1a00: 0x0000;   /* offset: Vram offset address */
	x = (x <256) ? x: x-256;	      /* x:256..319 なら 256を引く　 */
	z = ((y & 1 )==1) ? 2: 0;        /* z:Y座標が奇数なら、2を足す  */
	adr = (VRAM+ (off+ (y>>1)*w + (x&0xffc)+z));
	switch(x & 3)
	{
	case 0: ret=  *(adr);      break;
	case 1: ret=  *(adr)>>4;   break;
	case 2: ret=  *(adr+1);    break;
	case 3: ret=  *(adr+1)>>4; break;
	}
	return (ret);
}

void MEMORY::gvram_write(uint32_t A, uint32_t V)
{
	uint8_t* adr;
	int x,y,z,w,off;

	x = A & 0x1fff;
	y = portCF*16+portCE;           /* y座標 */
	if( y >=204) y-=204;			/* Y座標 204 以上だと 204 引く */
	w = (x <256) ? 256: 64;         /* width:0..255 なら256 / 256..319なら 64にする*/
	off=(x <256) ? 0x1a00: 0x0000;  /* offset: Vram offset address */
	x = (x <256) ? x: x-256;	    /* x:256..319 なら 256を引く　 */
	z = ((y & 1 )==1) ? 2: 0;       /* z:Y座標が奇数なら、2を足す  */
	V&= 0x0f;
	adr = VRAM+(off+ (y>>1)*(w) + (x&0xffc)+z);
	switch(x & 3)
	{
	case 0: *(adr)=(*(adr)  &0xf0)  |V;    break;
	case 1: *(adr)=(*(adr)  &0x0f)  |V<<4; break;
	case 2: *(adr+1)=(*(adr+1)&0xf0)|V;    break;
	case 3: *(adr+1)=(*(adr+1)&0x0f)|V<<4; break;
	}
}
#endif

