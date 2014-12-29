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

	[ memory ]
*/

#include "memory.h"
#include "timer.h"
#include "../../fileio.h"

#ifndef _PC6001
void MEMORY::initialize()
{
	int i;
	// for mkII/66
	int Pal11[ 4] = { 15, 8,10, 8 };
	int Pal12[ 8] = { 10,11,12, 9,15,14,13, 1 };
	int Pal13[ 8] = { 10,11,12, 9,15,14,13, 1 };
	int Pal14[ 4] = {  8,10, 8,15 };
	int Pal15[ 8] = {  8,9,11,14, 8,9,14,15 };
	int Pal53[32] = {  0, 4, 1, 5, 2, 6, 3, 7, 8,12, 9,13,10,14,11,15,
		10,11,12, 9,15,14,13, 1,10,11,12, 9,15,14,13, 1 };
	
	for(i=0;i<32;i++) {
		BPal53[i]=Pal53[i];
		if (i>15) continue;
		BPal[i]=i;
		if (i>7) continue;
		BPal12[i]=Pal12[i];
		BPal13[i]=Pal13[i];
		BPal15[i]=Pal15[i];
		if (i>3) continue;
		BPal11[i]=Pal11[i];
		BPal14[i]=Pal14[i];
	}
	for (i=0;i<32;i++) BPal62[i] = BPal53[i];	// for RefreshScr62/63
	for (i=0;i<16;i++) BPal61[i] = BPal[i];		// for RefreshScr61

	// mk2Å` palette
	palette[ 0] = RGB_COLOR(0x14,0x14,0x14); // COL065			= 141414			;mk2Å` ìßñæ(çï)
	palette[ 1] = RGB_COLOR(0xFF,0xAC,0x00); // COL066			= FFAC00			;mk2Å` ûÚ
	palette[ 2] = RGB_COLOR(0x00,0xFF,0xAC); // COL067			= 00FFAC			;mk2Å` ê¬óŒ
	palette[ 3] = RGB_COLOR(0xAC,0xFF,0x00); // COL068			= ACFF00			;mk2Å` â©óŒ
	palette[ 4] = RGB_COLOR(0xAC,0x00,0xFF); // COL069			= AC00FF			;mk2Å` ê¬éá
	palette[ 5] = RGB_COLOR(0xFF,0x00,0xAC); // COL070			= FF00AC			;mk2Å` ê‘éá
	palette[ 6] = RGB_COLOR(0x00,0xAC,0xFF); // COL071			= 00ACFF			;mk2Å` ãÛêF
	palette[ 7] = RGB_COLOR(0xAC,0xAC,0xAC); // COL072			= ACACAC			;mk2Å` äDêF
	palette[ 8] = RGB_COLOR(0x14,0x14,0x14); // COL073			= 141414			;mk2Å` çï
	palette[ 9] = RGB_COLOR(0xFF,0x00,0x00); // COL074			= FF0000			;mk2Å` ê‘
	palette[10] = RGB_COLOR(0x00,0xFF,0x00); // COL075			= 00FF00			;mk2Å` óŒ
	palette[11] = RGB_COLOR(0xFF,0xFF,0x00); // COL076			= FFFF00			;mk2Å` â©
	palette[12] = RGB_COLOR(0x00,0x00,0xFF); // COL077			= 0000FF			;mk2Å` ê¬
	palette[13] = RGB_COLOR(0xFF,0x00,0xFF); // COL078			= FF00FF			;mk2Å` É}É[ÉìÉ^
	palette[14] = RGB_COLOR(0x00,0xFF,0xFF); // COL079			= 00FFFF			;mk2Å` ÉVÉAÉì
	palette[15] = RGB_COLOR(0xFF,0xFF,0xFF); // COL080			= FFFFFF			;mk2Å` îí
	
	// register event
	register_vline_event(this);
}

#define EVENT_HBLANK	1

void MEMORY::event_vline(int v, int clock)
{
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	if(vm->sr_mode) {
		if(v == (CRTMode1 ? 200 : 192)) {
			d_timer->write_signal(SIG_TIMER_IRQ_VRTC, 1, 1);
		}
		if(!CRTKILL) {
			// SRÉÇÅ[ÉhÇÃBUSRQÇ…Ç¬Ç¢ÇƒÇÕÅAÇ¶Ç—Ç∑ólÇÃèÓïÒë“Çø
		}
	} else
#endif
	{
		if(!CRTKILL) {
			if(v < (CRTMode1 ? 200 : 192)) {
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				register_event_by_clock(this, EVENT_HBLANK, CPU_CLOCKS /  FRAMES_PER_SEC / LINES_PER_FRAME * 368 / 456, false, NULL);
			}
		}
	}
}

void MEMORY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_HBLANK) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
	}
}
#endif

#ifndef _PC6001
#define SETSCRVARM1(y1)	dest = &screen[y1][0];W=0;
#define SETSCRVARM5(y1)	dest = &screen[y1][0];W=0;
#define M1HEIGHT 192
#define M5HEIGHT 200
#define M6HEIGHT 204
#define SeqPix21(c) dest[X*8+W]=c;W++;
#define SeqPix41(c) dest[X*8+W]=c;W++;dest[X*8+W]=c;W++;

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
	byte X,Y,K;
	int FC,BC;
	byte *S,*T1,*T2;
	byte *G;

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
	byte X,Y;
	byte *T1,*T2;
	byte attr;

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
	byte X,Y;
	byte *T1,*T2;
	byte attr;
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
	byte X,Y;
	byte *T1,*T2;
	byte attr;

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
	byte X,Y;
	byte *T1,*T2;
	byte attr;

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
	byte X,Y;
	byte *T1,*T2;
	byte attr;

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
	byte X,Y;
	byte *T1,*T2;
	byte attr;

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
	byte X,Y,K;
	int FC,BC;
	byte *S,*T1,*T2;
	byte *G;

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
	byte X,Y;
	byte *T1,*T2;

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
	byte X,Y;
	byte *T1,*T2;
	byte cssor;

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
	byte X,Y,K;
	register int FC,BC;
	byte *S,*T1,*T2;
	byte *G;
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
	byte X,Y;
	byte *T1,*T2;
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
	byte X,Y;
	byte *T1,*T2;
	byte cssor;

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

void MEMORY::draw_screen()
{
	if (CRTKILL) {
		for(int y = 0; y < 400; y++) {
			scrntype* dest = emu->screen_buffer(y);
			memset(dest, 0, 640 * sizeof(scrntype));
		}
	} else if (vm->sr_mode) {
		if (CRTMode2) {
			if (CRTMode3) RefreshScr63();
			else RefreshScr62();
		} else RefreshScr61();
		// copy to screen
		if (bitmap) {
			for(int y = 0; y < 200; y++) {
				scrntype* dest = emu->screen_buffer(y*2);
				scrntype* dest1 = emu->screen_buffer(y*2+1);
				for(int x = 0; x < 320; x++) {
					dest[x*2] = dest[x*2+1] = palette[screen[y][x*2]];
				}
				if(config.scan_line) {
					memset(dest1, 0, 640 * sizeof(scrntype));
				} else {
					memcpy(dest1, dest, 640 * sizeof(scrntype));
				}
			}
		} else if (cols==40) {
			for(int y = 0; y < 200; y++) {
				scrntype* dest = emu->screen_buffer(y*2);
				scrntype* dest1 = emu->screen_buffer(y*2+1);
				for(int x = 0; x < 320; x++) {
					dest[x*2] = dest[x*2+1] = palette[screen[y][x]];
				}
				if(config.scan_line) {
					memset(dest1, 0, 640 * sizeof(scrntype));
				} else {
					memcpy(dest1, dest, 640 * sizeof(scrntype));
				}
			}
		} else {
			for(int y = 0; y < 200; y++) {
				scrntype* dest = emu->screen_buffer(y*2);
				scrntype* dest1 = emu->screen_buffer(y*2+1);
				for(int x = 0; x < 640; x++) {
					dest[x] = palette[screen[y][x]];
				}
				if(config.scan_line) {
					memset(dest1, 0, 640 * sizeof(scrntype));
				} else {
					memcpy(dest1, dest, 640 * sizeof(scrntype));
				}
			}
		}
	} else {
		if (CRTMode1) {
			if (CRTMode2)
				if (CRTMode3) RefreshScr54();
				else RefreshScr53();
			else RefreshScr51();
			// copy to screen
			for(int y = 0; y < 200; y++) {
				scrntype* dest = emu->screen_buffer(y*2);
				scrntype* dest1 = emu->screen_buffer(y*2+1);
				for(int x = 0; x < 320; x++) {
					dest[x*2] = dest[x*2+1] = palette[screen[y][x]];
				}
				if(config.scan_line) {
					memset(dest1, 0, 640 * sizeof(scrntype));
				} else {
					memcpy(dest1, dest, 640 * sizeof(scrntype));
				}
			}
		} else {
			RefreshScr10();
			// copy to screen
			for(int y = 0; y < 200; y++) {
				scrntype* dest = emu->screen_buffer(y*2);
				scrntype* dest1 = emu->screen_buffer(y*2+1);
				for(int x = 0; x < 320; x++) {
					if (x >= 32 && x < 288 && y >=4 && y < 196) {
						dest[x*2] = dest[x*2+1] = palette[screen[y-4][x-32]];
					} else {
						dest[x*2] = dest[x*2+1] = palette[8];
					}
					if(config.scan_line) {
						memset(dest1, 0, 640 * sizeof(scrntype));
					} else {
						memcpy(dest1, dest, 640 * sizeof(scrntype));
					}
				}
			}
		}
	}
	emu->screen_skip_line = true;
}
#else
void MEMORY::draw_screen()
{
	if (CRTKILL) {
		for(int y = 0; y < 400; y++) {
			scrntype* dest = emu->screen_buffer(y);
			memset(dest, 0, 640 * sizeof(scrntype));
		}
	} else if (CRTMode1) {
		if (CRTMode2)
			if (CRTMode3) RefreshScr54();
			else RefreshScr53();
		else RefreshScr51();
		// copy to screen
		for(int y = 0; y < 200; y++) {
			scrntype* dest = emu->screen_buffer(y*2);
			scrntype* dest1 = emu->screen_buffer(y*2+1);
			for(int x = 0; x < 320; x++) {
				dest[x*2] = dest[x*2+1] = palette[screen[y][x]];
			}
			if(config.scan_line) {
				memset(dest1, 0, 640 * sizeof(scrntype));
			} else {
				memcpy(dest1, dest, 640 * sizeof(scrntype));
			}
		}
	} else {
		RefreshScr10();
		// copy to screen
		for(int y = 0; y < 200; y++) {
			scrntype* dest = emu->screen_buffer(y*2);
			scrntype* dest1 = emu->screen_buffer(y*2+1);
			for(int x = 0; x < 320; x++) {
				if (x >= 32 && x < 288 && y >=4 && y < 196) dest[x*2] = dest[x*2+1] = palette[screen[y-4][x-32]];
				else dest[x*2] = dest[x*2+1] = palette[8];
			}
			if(config.scan_line) {
				memset(dest1, 0, 640 * sizeof(scrntype));
			} else {
				memcpy(dest1, dest, 640 * sizeof(scrntype));
			}
		}
	}
	emu->screen_skip_line = true;
}
#endif
#endif

#ifdef _PC6001
void MEMORY::reset()
{
	int J;
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("BASICROM.60")), FILEIO_READ_BINARY)) {
		fio->Fread(BASICROM, 0x4000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("CGROM60.60")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM1, 0x1000, 1);
		fio->Fclose();
	}
	if (!inserted) {
///		EXTROM1 = EXTROM2 = EmptyRAM;
		EXTROM1 = RAM + 0x4000;
		EXTROM2 = RAM + 0x6000;
		if (fio->Fopen(emu->bios_path(_T("EXTROM.60")), FILEIO_READ_BINARY)) {
			fio->Fread(EXTROM, 0x4000, 1);
			fio->Fclose();
			EXTROM1 = EXTROM;
			EXTROM2 = EXTROM + 0x2000;
			inserted = true;
		}
	}
	memset(RAM ,0,0x10000);
	memset(EmptyRAM, 0, sizeof(EmptyRAM));
	CGROM = CGROM1;
	CGSW93 = 0;
	VRAM = RAM;
	for(J=0;J<4;J++) {RdMem[J]=BASICROM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	RdMem[2] = EXTROM1; RdMem[3] = EXTROM2;
	for(J=4;J<8;J++) {RdMem[J]=RAM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	EnWrite[0]=0; EnWrite[1]=EnWrite[2]=EnWrite[3]=1;
#else
void MEMORY::reset()
{
	int I, J;
	byte *addr=RAM;
	memset(RAM ,0,0x10000);
	memset(EmptyRAM, 0, 0x2000);
	for(I=0; I<256; I++ ){
		for( J=0; J<64; J++ ){
			*addr++ = 0x00;
			*addr++ = 0xff;
		}
		for( J=0; J<64; J++ ){
			*addr++ = 0xff;
			*addr++ = 0x00;
		}
	}
	if (!inserted) {
		EXTROM1 = EXTROM2 = EmptyRAM;
	}
	FILEIO* fio = new FILEIO();
#ifdef _PC6001MK2
	vm->sr_mode=0;
	if (fio->Fopen(emu->bios_path(_T("CGROM62.62")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM5, 0x2000, 1);
		fio->Fclose();
	}
	else if (fio->Fopen(emu->bios_path(_T("CGROM60m.62")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM5, 0x2000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(emu->bios_path(_T("BASICROM.62")), FILEIO_READ_BINARY)) {
		fio->Fread(BASICROM, 0x8000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(emu->bios_path(_T("CGROM60.62")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM1, 0x2000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(emu->bios_path(_T("KANJIROM.62")), FILEIO_READ_BINARY)) {
		fio->Fread(KANJIROM, 0x8000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(emu->bios_path(_T("VOICEROM.62")), FILEIO_READ_BINARY)) {
		fio->Fread(VOICEROM, 0x4000, 1);
		fio->Fclose();
	}
	CGROM = CGROM1;
	VRAM = RAM+0xE000;
	for (I=0; I<0x200; I++ ) *(VRAM+I)=0xde;
	for(J=0;J<4;J++) {RdMem[J]=BASICROM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	for(J=4;J<8;J++) {RdMem[J]=RAM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	EnWrite[0]=EnWrite[1]=0; EnWrite[2]=EnWrite[3]=1;
#endif
#ifdef _PC6601
	vm->sr_mode=0;
	if (fio->Fopen(emu->bios_path(_T("CGROM66.66")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM5, 0x2000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(emu->bios_path(_T("BASICROM.66")), FILEIO_READ_BINARY)) {
		fio->Fread(BASICROM, 0x8000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(emu->bios_path(_T("CGROM60.66")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM1, 0x2000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(emu->bios_path(_T("KANJIROM.66")), FILEIO_READ_BINARY)) {
		fio->Fread(KANJIROM, 0x8000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(emu->bios_path(_T("VOICEROM.66")), FILEIO_READ_BINARY)) {
		fio->Fread(VOICEROM, 0x4000, 1);
		fio->Fclose();
	}
	CGROM = CGROM1;
	VRAM = RAM+0xE000;
	for (I=0; I<0x200; I++ ) *(VRAM+I)=0xde;
	for(J=0;J<4;J++) {RdMem[J]=BASICROM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	for(J=4;J<8;J++) {RdMem[J]=RAM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	EnWrite[0]=EnWrite[1]=0; EnWrite[2]=EnWrite[3]=1;
#endif
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	vm->sr_mode=1;
	bitmap=1;
	cols=40;
	rows=20;
	lines=200;
	memset(EXTRAM ,0,0x10000);
	if (fio->Fopen(emu->bios_path(_T("CGROM68.68")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM6, 0x4000, 1);
		fio->Fclose();
	}
	memcpy(CGROM1, CGROM6, 0x2400);
	memcpy(CGROM5, CGROM6+0x2000, 0x2000);
	if (fio->Fopen(emu->bios_path(_T("SYSTEMROM1.68")), FILEIO_READ_BINARY)) {
		fio->Fread(SYSTEMROM1, 0x10000, 1);
		fio->Fclose();
	}
	memcpy(BASICROM, SYSTEMROM1, 0x8000);
	if (fio->Fopen(emu->bios_path(_T("SYSTEMROM2.68")), FILEIO_READ_BINARY)) {
		fio->Fread(SYSTEMROM2, 0x10000, 1);
		fio->Fclose();
	}
	SYSROM2=(SYSTEMROM2+0x2000);
	memcpy(VOICEROM, SYSTEMROM2+0x4000, 0x4000);
	memcpy(KANJIROM, SYSTEMROM2+0x8000, 0x8000);
	for (int i=0; i<16; i++) palet[i] = i;
	port60[0]= 0xf8; 					//I/O[60..67] READ  MEMORY MAPPING
	for (I=1; I<15; I++) port60[I]=0;	//I/O[68-6f]  WRITE MEMORY MAPPING
	portC1 = 0x00;						//I/O[C1]     CRT CONTROLLER MODE
	portC8 = 0x00;						//I/O[C8]     CRT CONTROLLER TYPE
	portCA = 0x00;						//I/O[CA]     X GEOMETORY low  HARDWARE SCROLL
	portCB = 0x00;						//I/O[CB]     X GEOMETORY high HARDWARE SCROLL
	portCC = 0x00;						//I/O[CC]     Y GEOMETORY      HARDWARE SCROLL
	portCE = 0x00;						//I/O[CE]     LINE SETTING  BITMAP (low) */
	portCF = 0x00;						//I/O[CF]     LINE SETTING  BITMAP (High) */
	CGROM=CGROM6;
	make_semigraph();
	for(J=0;J<4;J++) {RdMem[J]=SYSTEMROM1+0x2000*J+0x8000;WrMem[J]=RAM+0x2000*J;};
	RdMem[2] = EXTROM1; RdMem[3] = EXTROM2;
	for(J=4;J<8;J++) {RdMem[J]=RAM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	EnWrite[0]=EnWrite[1]=0; EnWrite[2]=EnWrite[3]=1;
	VRAM=RAM;
	TEXTVRAM=RAM;
	SYSROM2=EmptyRAM;
#endif
	portF0 = 0x11;
	portF1 = 0xdd;
	CRTMode1 = CRTMode2 = CRTMode3 = 0;
	CSS3=CSS2=CSS1=0;
	CGSW93 = CRTKILL = 0;
	CurKANJIROM = NULL;
#endif
	delete fio;
}

#if defined(_PC6601SR) || defined(_PC6001MK2SR)
void MEMORY::make_semigraph(void)
{
	byte *P;
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

int MEMORY::chk_gvram(uint32 A,int flag)
{
	if (port60[ (A>>13)+flag ]==0x00 && bitmap)	// VRAM ÇÃêÊì™Ç©Ç¬ÅACRTÇ™ BITMAP mode
		return 1;
	return 0;
}

byte MEMORY::gvram_read(uint32 A)
{
	byte* adr;
	byte  ret;
	int x,y,z,w,off;

	x = A & 0x1fff;
	y = portCF*16+portCE;		      /* yç¿ïW */
	if( y >=204) y-=204;		      /* Yç¿ïW 204 à»è„ÇæÇ∆ 204 à¯Ç≠ add 2003/10/22 */
	w = (x <256) ? 256: 64;          /* width:0..255 Ç»ÇÁ256 / 256..319Ç»ÇÁ 64Ç…Ç∑ÇÈ*/
	off=(x <256) ? 0x1a00: 0x0000;   /* offset: Vram offset address */
	x = (x <256) ? x: x-256;	      /* x:256..319 Ç»ÇÁ 256Çà¯Ç≠Å@ */
	z = ((y & 1 )==1) ? 2: 0;        /* z:Yç¿ïWÇ™äÔêîÇ»ÇÁÅA2Çë´Ç∑  */
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

void MEMORY::gvram_write(uint32 A, uint32 V)
{
	byte* adr;
	int x,y,z,w,off;

	x = A & 0x1fff;
	y = portCF*16+portCE;           /* yç¿ïW */
	if( y >=204) y-=204;			/* Yç¿ïW 204 à»è„ÇæÇ∆ 204 à¯Ç≠ */
	w = (x <256) ? 256: 64;         /* width:0..255 Ç»ÇÁ256 / 256..319Ç»ÇÁ 64Ç…Ç∑ÇÈ*/
	off=(x <256) ? 0x1a00: 0x0000;  /* offset: Vram offset address */
	x = (x <256) ? x: x-256;	    /* x:256..319 Ç»ÇÁ 256Çà¯Ç≠Å@ */
	z = ((y & 1 )==1) ? 2: 0;       /* z:Yç¿ïWÇ™äÔêîÇ»ÇÁÅA2Çë´Ç∑  */
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

void MEMORY::write_data8(uint32 addr, uint32 data)
{
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	/* Graphics Vram Write (SR basic) */
	if(vm->sr_mode && chk_gvram(addr ,8)) 
		gvram_write(addr, data);
	else
#endif
	/* normal memory write */
	if(EnWrite[addr >> 14]) 
		WrMem[addr >> 13][addr & 0x1FFF] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	/* Graphics Vram Read (SR basic) */
	if(vm->sr_mode && chk_gvram(addr, 0))
		return(gvram_read(addr));
#endif
	return(RdMem[addr >> 13][addr & 0x1FFF]);
}

void MEMORY::open_cart(_TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(EXTROM, 0x4000, 1);
		fio->Fclose();
		EXTROM1 = EXTROM;
		EXTROM2 = EXTROM + 0x2000;
		EnWrite[1]=0;
		inserted = true;
	} else {
///		EXTROM1 = EXTROM2 = EmptyRAM;
		EXTROM1 = RAM + 0x4000;
		EXTROM2 = RAM + 0x6000;
		EnWrite[1]=1;
		inserted = false;
	}
	delete fio;
}

void MEMORY::close_cart()
{
///	EXTROM1 = EXTROM2 = EmptyRAM;
	EXTROM1 = RAM + 0x4000;
	EXTROM2 = RAM + 0x6000;
	EnWrite[1]=1;
	inserted = false;
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	unsigned int VRAMHead[2][4] = {
		{ 0xc000, 0xe000, 0x8000, 0xa000 },
		{ 0x8000, 0xc000, 0x0000, 0x4000 }
	};
	uint16 port=(addr & 0x00ff);
	byte Value=data;
	switch(port)
	{
#ifdef _PC6001
	/// 64K RAM ///
	case 0x00:
		if (Value & 1) {
			RdMem[0]=RAM;
			RdMem[1]=RAM+0x2000;
			EnWrite[0]=1;
		} else {
			RdMem[0]=BASICROM;
			RdMem[1]=BASICROM+0x2000;
			EnWrite[0]=0;
		}
		break;
	/// CP/M ///
	case 0xf0:
		if (Value ==0xdd) {
			RdMem[0]=RAM;
			RdMem[1]=RAM+0x2000;
			RdMem[2]=RAM+0x4000;
			RdMem[3]=RAM+0x6000;
			EnWrite[0]=EnWrite[1]=1;
		} else {
			RdMem[0]=BASICROM;
			RdMem[1]=BASICROM+0x2000;
			RdMem[2]=EXTROM1;
			RdMem[3]=EXTROM2;
			EnWrite[0]=EnWrite[1]=0;
		}
		break;
	}
	return;
}
#else
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
		int reg,val;
		reg= 15-(port-0x40);
		val= 15-Value;
		palet[ reg]= val;
		do_palet( reg,val);
		break;
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
		int start_adr;
		start_adr= Value & 0xe;
		port60[port-0x60]= Value;
		switch( Value & 0xf0) {
		case 0xf0: RdMem[(port& 0xf)]=SYSTEMROM1+(start_adr)*0x1000;break;
		case 0xe0: RdMem[(port& 0xf)]=SYSTEMROM2+(start_adr)*0x1000;break;
		case 0xd0: RdMem[(port& 0xf)]=    CGROM6+(start_adr)*0x1000;break;
		case 0xc0: RdMem[(port& 0xf)]=   EXTROM2; /*+(start_adr)*0x1000; */break;
		case 0xb0: RdMem[(port& 0xf)]=   EXTROM1; /*+(start_adr)*0x1000; */break;
		case 0x00: RdMem[(port& 0xf)]=       RAM+(start_adr)*0x1000;break;
		case 0x20: if (EXTRAM) RdMem[ port & 0xf]=  EXTRAM+((start_adr)*0x1000); break;
		}
		return;
	case 0x68:
	case 0x69:
	case 0x6a:
	case 0x6b:
	case 0x6c:
	case 0x6d:
	case 0x6e:
	case 0x6f:
		port60[port-0x60]= Value;
		if ((Value & 0xf0)==0x00) {
			WrMem[ (port& 0xf)-8]= RAM+((Value & 0xe)*0x1000);
			EnWrite[ ((port & 0xe)-8)/2 ]= 1;
		}
		if (EXTRAM) {
			if((Value & 0xf0)==0x20) {
				WrMem[ (port& 0xf)-8]= EXTRAM+((Value & 0xe)*0x1000);
			}
		}
		break;
#endif
	case 0xB0:
		if (vm->sr_mode) {
			d_timer->set_portB0(Value);
		} else {
			VRAM=(RAM+VRAMHead[CRTMode1][(data&0x06)>>1]);
			if (CRTMode1 && Value == 6) d_timer->set_portB0(Value | 0x01); /// Colony Oddysey
			else d_timer->set_portB0(Value);
		}
		break;
	case 0xC0: // CSS
		CSS3=(Value&0x04)<<2;CSS2=(Value&0x02)<<2;CSS1=(Value&0x01)<<2;
		break;
	case 0xC1: // CRT controller mode
		CRTMode1=(Value&0x02) ? 0 : 1;
		CRTMode2=(Value&0x04) ? 0 : 1;
		CRTMode3=(Value&0x08) ? 0 : 1;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
		portC1 = Value;
		if (vm->sr_mode)
			lines=(Value&0x01) ? 200 : 204;
		if (vm->sr_mode)
			CGROM = CGROM6;    // N66SR BASIC use CGROM6
		else
			CGROM = ((CRTMode1 == 0) ? CGROM1 : CGROM5);
		if (vm->sr_mode) {
			if (CRTMode1==1 && CRTMode2==0 && !bitmap) { /* width 80 */
				cols=80;
			} else if(CRTMode1==0 && CRTMode2==0 && !bitmap) { /* Width 40  */
				cols=40;
			}
		}
#else
		CGROM = ((CRTMode1 == 0) ? CGROM1 : CGROM5);
#endif
		break;
	case 0xC2: // ROM swtich
		if (vm->sr_mode) return;	/* sr_mode do nothing! */
		if ((Value&0x02)==0x00) CurKANJIROM=KANJIROM;
		else CurKANJIROM=KANJIROM+0x4000;
		if ((Value&0x01)==0x00) {
///			if(RdMem[0]!=BASICROM) RdMem[0]=VOICEROM;
///			if(RdMem[1]!=BASICROM+0x2000) RdMem[1]=VOICEROM+0x2000;
///			if(RdMem[0]!=BASICROM)        RdMem[0]=SYSTEMROM2;
///			if(RdMem[1]!=BASICROM+0x2000) RdMem[1]=SYSTEMROM2+0x2000;
			if(RdMem[2]!=BASICROM+0x4000) RdMem[2]=VOICEROM;
			if(RdMem[3]!=BASICROM+0x6000) RdMem[3]=VOICEROM+0x2000;
		}
		else {
			write_io8(0xF0,portF0); 	
		};
		break;
	case 0xC3: break; // C2H in/out switch
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	case 0xC8:
		portC8  = Value;
		bitmap  = (Value & 8)? 0:1;
		rows    = (Value & 4)? 20:25;
///		busreq  = (Value & 2)? 0:1;
		vm->sr_mode = ((Value & 1)==1) ? 0 : 1;
		if (bitmap && vm->sr_mode)
		{
			VRAM = (Value & 0x10) ? RAM+0x8000:RAM+0x0000;
		}
		if (vm->sr_mode) {
			CGROM=CGROM6; 
			portF0=0x11;
		}
		break;	
	case 0xC9:
		if (vm->sr_mode && !bitmap ) 
		{		
			TEXTVRAM=RAM+(Value & 0xf)*0x1000;
		}
		break;	
	case 0xCA: portCA=Value; break;	// Graphics scroll X low
	case 0xCB: portCB=Value; break;// Graphics scroll X high
	case 0xCC: portCC=Value; break;	// Graphics scroll Y
	case 0xCE: portCE=Value; break; /* Graphics Y zahyou SR-BASIC add 2002/2 */
	case 0xCF: portCF=0; break;
#endif
	case 0xF0: // read block set 
		if (vm->sr_mode) return;	/* sr_mode do nothing! */
		portF0 = Value;
		switch(data & 0x0f)
		{
		case 0x00: RdMem[0]=RdMem[1]=EmptyRAM; break;
		case 0x01: RdMem[0]=BASICROM;RdMem[1]=BASICROM+0x2000; break;
		case 0x02: RdMem[0]=CurKANJIROM;RdMem[1]=CurKANJIROM+0x2000; break;
		case 0x03: RdMem[0]=RdMem[1]=EXTROM2; break;
		case 0x04: RdMem[0]=RdMem[1]=EXTROM1; break;
		case 0x05: RdMem[0]=CurKANJIROM;RdMem[1]=BASICROM+0x2000; break;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
		case 0x06: RdMem[0]=BASICROM;RdMem[1]=(SYSROM2==EmptyRAM ? CurKANJIROM+0x2000 : SYSROM2); break;
#else
		case 0x06: RdMem[0]=BASICROM;RdMem[1]=CurKANJIROM+0x2000;break;
#endif
		case 0x07: RdMem[0]=EXTROM1;RdMem[1]=EXTROM2; break;
		case 0x08: RdMem[0]=EXTROM2;RdMem[1]=EXTROM1; break;
		case 0x09: RdMem[0]=EXTROM2;RdMem[1]=BASICROM+0x2000; break;
		case 0x0a: RdMem[0]=BASICROM;RdMem[1]=EXTROM2; break;
		case 0x0b: RdMem[0]=EXTROM1;RdMem[1]=CurKANJIROM+0x2000; break;
		case 0x0c: RdMem[0]=CurKANJIROM;RdMem[1]=EXTROM1; break;
		case 0x0d: RdMem[0]=RAM;RdMem[1]=RAM+0x2000; break;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
		case 0x0e: if (EXTRAM) {RdMem[0]=EXTRAM; RdMem[1]=EXTRAM+0x2000;break;}
#else
		case 0x0e: RdMem[0]=RdMem[1]=EmptyRAM; break;
#endif
		case 0x0f: RdMem[0]=RdMem[1]=EmptyRAM; break;
		};
		switch(data & 0xf0)
		{
		case 0x00: RdMem[2]=RdMem[3]=EmptyRAM; break;
		case 0x10: RdMem[2]=BASICROM+0x4000;RdMem[3]=BASICROM+0x6000; break;
		case 0x20: RdMem[2]=VOICEROM;RdMem[3]=VOICEROM+0x2000; break;
		case 0x30: RdMem[2]=RdMem[3]=EXTROM2; break;
		case 0x40: RdMem[2]=RdMem[3]=EXTROM1; break;
		case 0x50: RdMem[2]=VOICEROM;RdMem[3]=BASICROM+0x6000; break;
		case 0x60: RdMem[2]=BASICROM+0x4000;RdMem[3]=VOICEROM+0x2000; break;
		case 0x70: RdMem[2]=EXTROM1;RdMem[3]=EXTROM2; break;
		case 0x80: RdMem[2]=EXTROM2;RdMem[3]=EXTROM1; break;
		case 0x90: RdMem[2]=EXTROM2;RdMem[3]=BASICROM+0x6000; break;
		case 0xa0: RdMem[2]=BASICROM+0x4000;RdMem[3]=EXTROM2; break;
		case 0xb0: RdMem[2]=EXTROM1;RdMem[3]=VOICEROM+0x2000; break;
		case 0xc0: RdMem[2]=VOICEROM;RdMem[3]=EXTROM1; break;
		case 0xd0: RdMem[2]=RAM+0x4000;RdMem[3]=RAM+0x6000; break;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
		case 0xe0: if (EXTRAM) {RdMem[2]=EXTRAM+0x4000; RdMem[3]=EXTRAM+0x6000; break;}
#else
		case 0xe0: RdMem[2]=RdMem[3]=EmptyRAM; break;
#endif
		case 0xf0: RdMem[2]=RdMem[3]=EmptyRAM; break;
		};
		if (CGSW93)	RdMem[3] = CGROM;
		break;
	case 0xF1: // read block set
		if (vm->sr_mode) return;	/* sr_mode do nothing! */
		portF1 = Value;
		switch(data & 0x0f)
		{
		case 0x00: RdMem[4]=RdMem[5]=EmptyRAM; break;
		case 0x01: RdMem[4]=BASICROM;RdMem[5]=BASICROM+0x2000; break;
		case 0x02: RdMem[4]=CurKANJIROM;RdMem[5]=CurKANJIROM+0x2000; break;
		case 0x03: RdMem[4]=RdMem[5]=EXTROM2; break;
		case 0x04: RdMem[4]=RdMem[5]=EXTROM1; break;
		case 0x05: RdMem[4]=CurKANJIROM;RdMem[5]=BASICROM+0x2000; break;
		case 0x06: RdMem[4]=BASICROM;RdMem[5]=CurKANJIROM+0x2000; break;
		case 0x07: RdMem[4]=EXTROM1;RdMem[5]=EXTROM2; break;
		case 0x08: RdMem[4]=EXTROM2;RdMem[5]=EXTROM1; break;
		case 0x09: RdMem[4]=EXTROM2;RdMem[5]=BASICROM+0x2000; break;
		case 0x0a: RdMem[4]=BASICROM;RdMem[5]=EXTROM2; break;
		case 0x0b: RdMem[4]=EXTROM1;RdMem[5]=CurKANJIROM+0x2000; break;
		case 0x0c: RdMem[4]=CurKANJIROM;RdMem[5]=EXTROM1; break;
		case 0x0d: RdMem[4]=RAM+0x8000;RdMem[5]=RAM+0xa000; break;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
		case 0x0e: if (EXTRAM) {RdMem[4]=EXTRAM+0x8000; RdMem[5]=EXTRAM+0xa000; break;}
#else
		case 0x0e: RdMem[4]=RdMem[5]=EmptyRAM; break;
#endif
		case 0x0f: RdMem[4]=RdMem[5]=EmptyRAM; break;
		};
		switch(data & 0xf0)
		{
		case 0x00: RdMem[6]=RdMem[7]=EmptyRAM; break;
		case 0x10: RdMem[6]=BASICROM+0x4000;RdMem[7]=BASICROM+0x6000; break;
		case 0x20: RdMem[6]=CurKANJIROM;RdMem[7]=CurKANJIROM+0x2000; break;
		case 0x30: RdMem[6]=RdMem[7]=EXTROM2; break;
		case 0x40: RdMem[6]=RdMem[7]=EXTROM1; break;
		case 0x50: RdMem[6]=CurKANJIROM;RdMem[7]=BASICROM+0x6000; break;
		case 0x60: RdMem[6]=BASICROM+0x4000;RdMem[7]=CurKANJIROM+0x2000; break;
		case 0x70: RdMem[6]=EXTROM1;RdMem[7]=EXTROM2; break;
		case 0x80: RdMem[6]=EXTROM2;RdMem[7]=EXTROM1; break;
		case 0x90: RdMem[6]=EXTROM2;RdMem[7]=BASICROM+0x6000; break;
		case 0xa0: RdMem[6]=BASICROM+0x4000;RdMem[7]=EXTROM2; break;
		case 0xb0: RdMem[6]=EXTROM1;RdMem[7]=CurKANJIROM+0x2000; break;
		case 0xc0: RdMem[6]=CurKANJIROM;RdMem[7]=EXTROM1; break;
		case 0xd0: RdMem[6]=RAM+0xc000;RdMem[7]=RAM+0xe000; break;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
		case 0xe0: if (EXTRAM) {RdMem[6]=EXTRAM+0xc000;RdMem[7]=EXTRAM+0xe000; break;}
#else
		case 0xe0: RdMem[6]=RdMem[7]=EmptyRAM; break;
#endif
		case 0xf0: RdMem[6]=RdMem[7]=EmptyRAM; break;
		};
		break;
	case 0xF2: // write ram block set
		if (vm->sr_mode) return;	/* sr_mode do nothing! */
		if (data & 0x40) {EnWrite[3]=1;WrMem[6]=RAM+0xc000;WrMem[7]=RAM+0xe000;}
		else EnWrite[3]=0;
		if (data & 0x010) {EnWrite[2]=1;WrMem[4]=RAM+0x8000;WrMem[5]=RAM+0xa000;}
		else EnWrite[2]=0;
		if (data & 0x04) {EnWrite[1]=1;WrMem[2]=RAM+0x4000;WrMem[3]=RAM+0x6000;}
		else EnWrite[1]=0;
		if (data & 0x01) {EnWrite[0]=1;WrMem[0]=RAM;WrMem[1]=RAM+0x2000;}
		else EnWrite[0]=0;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
		if (EXTRAM) {
			if (Value&0x80) {EnWrite[3]=2;WrMem[6]=EXTRAM+0xc000;WrMem[7]=EXTRAM+0xe000;}
			if (Value&0x20) {EnWrite[2]=2;WrMem[4]=EXTRAM+0x8000;WrMem[5]=EXTRAM+0xa000;}
			if (Value&0x08) {EnWrite[1]=2;WrMem[2]=EXTRAM+0x4000;WrMem[3]=EXTRAM+0x6000;}
			if (Value&0x02) {EnWrite[0]=2;WrMem[0]=EXTRAM+0x0000;WrMem[1]=EXTRAM+0x2000;}
		}
#endif
		break;
	}
	return;
}

uint32 MEMORY::read_io8(uint32 addr)
{
	uint16 port=(addr & 0x00ff);
	byte Value=0xff;

	switch(port)
	{
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	case 0x60:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:case 0x66:case 0x67:
	case 0x68:case 0x69:case 0x6a:case 0x6b:case 0x6c:case 0x6d:case 0x6e:case 0x6f:
		Value=port60[ port-0x60 ];
		break;
	case 0xC0: Value=0xff;break;
	case 0xC2: Value=0xff;break;
#endif
	case 0xF0: if (!vm->sr_mode) Value=portF0;break;
	case 0xF1: if (!vm->sr_mode) Value=portF1;break;
	}
	return(Value);
}
#endif

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_MEMORY_PIO_PORT_C) {
#ifdef _PC6001
		if(data & 4) {
			CGSW93=0;RdMem[3]=EXTROM2;
		} else {
			CGSW93=1; RdMem[3]=CGROM1;
		}
#else
		if(data & 4) {
			CGSW93=0; if (!vm->sr_mode) write_io8(0xf0, portF0);
		} else {
			CGSW93=1; RdMem[3]=CGROM;
		}
		CRTKILL = (data & 2) ? 0 : 1;
#endif
	}
}
