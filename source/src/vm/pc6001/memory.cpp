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

#define RAM		(MEMORY_BASE + RAM_BASE)
#define BASICROM	(MEMORY_BASE + BASICROM_BASE)
#define EXTROM		(MEMORY_BASE + EXTROM_BASE)
#define CGROM1		(MEMORY_BASE + CGROM1_BASE)
#define EmptyRAM	(MEMORY_BASE + EmptyRAM_BASE)
// PC-6001mkII, PC-6601
#define VOICEROM	(MEMORY_BASE + VOICEROM_BASE)
#define KANJIROM	(MEMORY_BASE + KANJIROM_BASE)
#define CGROM5		(MEMORY_BASE + CGROM5_BASE)
// PC-6001mkIISR, PC-6601SR
#define EXTRAM		(MEMORY_BASE + EXTRAM_BASE)
#define SYSTEMROM1	(MEMORY_BASE + SYSTEMROM1_BASE)
#define SYSTEMROM2	(MEMORY_BASE + SYSTEMROM2_BASE)
#define CGROM6		(MEMORY_BASE + CGROM6_BASE)

void MEMORY::initialize()
{
	FILEIO* fio = new FILEIO();
#if defined(_PC6001)
	if(fio->Fopen(create_local_path(_T("BASICROM.60")), FILEIO_READ_BINARY)) {
		fio->Fread(BASICROM, 0x4000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("CGROM60.60")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM1, 0x1000, 1);
		fio->Fclose();
	}
#elif defined(_PC6001MK2)
	if (fio->Fopen(create_local_path(_T("CGROM62.62")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM5, 0x2000, 1);
		fio->Fclose();
	}
	else if (fio->Fopen(create_local_path(_T("CGROM60m.62")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM5, 0x2000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(create_local_path(_T("BASICROM.62")), FILEIO_READ_BINARY)) {
		fio->Fread(BASICROM, 0x8000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(create_local_path(_T("CGROM60.62")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM1, 0x2000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(create_local_path(_T("KANJIROM.62")), FILEIO_READ_BINARY)) {
		fio->Fread(KANJIROM, 0x8000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(create_local_path(_T("VOICEROM.62")), FILEIO_READ_BINARY)) {
		fio->Fread(VOICEROM, 0x4000, 1);
		fio->Fclose();
	}
#elif defined(_PC6601)
	if (fio->Fopen(create_local_path(_T("CGROM66.66")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM5, 0x2000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(create_local_path(_T("BASICROM.66")), FILEIO_READ_BINARY)) {
		fio->Fread(BASICROM, 0x8000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(create_local_path(_T("CGROM60.66")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM1, 0x2000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(create_local_path(_T("KANJIROM.66")), FILEIO_READ_BINARY)) {
		fio->Fread(KANJIROM, 0x8000, 1);
		fio->Fclose();
	}
	if (fio->Fopen(create_local_path(_T("VOICEROM.66")), FILEIO_READ_BINARY)) {
		fio->Fread(VOICEROM, 0x4000, 1);
		fio->Fclose();
	}
#elif defined(_PC6601SR) || defined(_PC6001MK2SR)
	if (fio->Fopen(create_local_path(_T("CGROM68.68")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM6, 0x4000, 1);
		fio->Fclose();
	}
	memcpy(CGROM1, CGROM6, 0x2400);
	memcpy(CGROM5, CGROM6+0x2000, 0x2000);
	if (fio->Fopen(create_local_path(_T("SYSTEMROM1.68")), FILEIO_READ_BINARY)) {
		fio->Fread(SYSTEMROM1, 0x10000, 1);
		fio->Fclose();
	}
	memcpy(BASICROM, SYSTEMROM1, 0x8000);
	if (fio->Fopen(create_local_path(_T("SYSTEMROM2.68")), FILEIO_READ_BINARY)) {
		fio->Fread(SYSTEMROM2, 0x10000, 1);
		fio->Fclose();
	}
	memcpy(VOICEROM, SYSTEMROM2+0x4000, 0x4000);
	memcpy(KANJIROM, SYSTEMROM2+0x8000, 0x8000);
#endif
	delete fio;
	
#ifndef _PC6001
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
	palette_pc[ 0] = RGB_COLOR(0x14,0x14,0x14); // COL065			= 141414			;mk2Å` ìßñæ(çï)
	palette_pc[ 1] = RGB_COLOR(0xFF,0xAC,0x00); // COL066			= FFAC00			;mk2Å` ûÚ
	palette_pc[ 2] = RGB_COLOR(0x00,0xFF,0xAC); // COL067			= 00FFAC			;mk2Å` ê¬óŒ
	palette_pc[ 3] = RGB_COLOR(0xAC,0xFF,0x00); // COL068			= ACFF00			;mk2Å` â©óŒ
	palette_pc[ 4] = RGB_COLOR(0xAC,0x00,0xFF); // COL069			= AC00FF			;mk2Å` ê¬éá
	palette_pc[ 5] = RGB_COLOR(0xFF,0x00,0xAC); // COL070			= FF00AC			;mk2Å` ê‘éá
	palette_pc[ 6] = RGB_COLOR(0x00,0xAC,0xFF); // COL071			= 00ACFF			;mk2Å` ãÛêF
	palette_pc[ 7] = RGB_COLOR(0xAC,0xAC,0xAC); // COL072			= ACACAC			;mk2Å` äDêF
	palette_pc[ 8] = RGB_COLOR(0x14,0x14,0x14); // COL073			= 141414			;mk2Å` çï
	palette_pc[ 9] = RGB_COLOR(0xFF,0x00,0x00); // COL074			= FF0000			;mk2Å` ê‘
	palette_pc[10] = RGB_COLOR(0x00,0xFF,0x00); // COL075			= 00FF00			;mk2Å` óŒ
	palette_pc[11] = RGB_COLOR(0xFF,0xFF,0x00); // COL076			= FFFF00			;mk2Å` â©
	palette_pc[12] = RGB_COLOR(0x00,0x00,0xFF); // COL077			= 0000FF			;mk2Å` ê¬
	palette_pc[13] = RGB_COLOR(0xFF,0x00,0xFF); // COL078			= FF00FF			;mk2Å` É}É[ÉìÉ^
	palette_pc[14] = RGB_COLOR(0x00,0xFF,0xFF); // COL079			= 00FFFF			;mk2Å` ÉVÉAÉì
	palette_pc[15] = RGB_COLOR(0xFF,0xFF,0xFF); // COL080			= FFFFFF			;mk2Å` îí
	
	// register event
	register_vline_event(this);
#endif
}

void MEMORY::reset()
{
#ifdef _PC6001
	int J;
	if (!inserted) {
///		EXTROM1 = EXTROM2 = EmptyRAM;
		EXTROM1 = RAM + 0x4000;
		EXTROM2 = RAM + 0x6000;
		FILEIO* fio = new FILEIO();
		if (fio->Fopen(create_local_path(_T("EXTROM.60")), FILEIO_READ_BINARY)) {
			fio->Fread(EXTROM, 0x4000, 1);
			fio->Fclose();
			EXTROM1 = EXTROM;
			EXTROM2 = EXTROM + 0x2000;
			inserted = true;
		}
		delete fio;
	}
	memset(RAM ,0,0x10000);
	memset(EmptyRAM, 0, 0x2000);
	CGROM = CGROM1;
	CGSW93 = 0;
	VRAM = RAM;
	for(J=0;J<4;J++) {RdMem[J]=BASICROM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	RdMem[2] = EXTROM1; RdMem[3] = EXTROM2;
	for(J=4;J<8;J++) {RdMem[J]=RAM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	EnWrite[0]=0; EnWrite[1]=EnWrite[2]=EnWrite[3]=1;
#else
	int I, J;
	uint8_t *addr=RAM;
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
#if defined(_PC6001MK2) || defined(_PC6601)
	static_cast<VM *>(vm)->sr_mode=0;
	CGROM = CGROM1;
	VRAM = RAM+0xE000;
	for (I=0; I<0x200; I++ ) *(VRAM+I)=0xde;
	for(J=0;J<4;J++) {RdMem[J]=BASICROM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	for(J=4;J<8;J++) {RdMem[J]=RAM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	EnWrite[0]=EnWrite[1]=0; EnWrite[2]=EnWrite[3]=1;
#elif defined(_PC6601SR) || defined(_PC6001MK2SR)
	static_cast<VM *>(vm)->sr_mode=1;
	bitmap=1;
	cols=40;
	rows=20;
	lines=200;
	memset(EXTRAM ,0,0x10000);
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
	CurKANJIROM = KANJIROM;
#endif
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	/* Graphics Vram Write (SR basic) */
	if(static_cast<VM *>(vm)->sr_mode && chk_gvram(addr ,8)) 
		gvram_write(addr, data);
	else
#endif
	/* normal memory write */
	if(EnWrite[addr >> 14]) 
		WrMem[addr >> 13][addr & 0x1FFF] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	/* Graphics Vram Read (SR basic) */
	if(static_cast<VM *>(vm)->sr_mode && chk_gvram(addr, 0))
		return(gvram_read(addr));
#endif
	return(RdMem[addr >> 13][addr & 0x1FFF]);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	unsigned int VRAMHead[2][4] = {
		{ 0xc000, 0xe000, 0x8000, 0xa000 },
		{ 0x8000, 0xc000, 0x0000, 0x4000 }
	};
	uint16_t port=(addr & 0x00ff);
	uint8_t Value=data;
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
		if (static_cast<VM *>(vm)->sr_mode) {
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
		if (static_cast<VM *>(vm)->sr_mode)
			lines=(Value&0x01) ? 200 : 204;
		if (static_cast<VM *>(vm)->sr_mode)
			CGROM = CGROM6;    // N66SR BASIC use CGROM6
		else
			CGROM = ((CRTMode1 == 0) ? CGROM1 : CGROM5);
		if (static_cast<VM *>(vm)->sr_mode) {
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
		if (static_cast<VM *>(vm)->sr_mode) return;	/* sr_mode do nothing! */
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
		static_cast<VM *>(vm)->sr_mode = ((Value & 1)==1) ? 0 : 1;
		if (bitmap && static_cast<VM *>(vm)->sr_mode)
		{
			VRAM = (Value & 0x10) ? RAM+0x8000:RAM+0x0000;
		}
		if (static_cast<VM *>(vm)->sr_mode) {
			CGROM=CGROM6; 
			portF0=0x11;
		}
		break;	
	case 0xC9:
		if (static_cast<VM *>(vm)->sr_mode && !bitmap ) 
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
		if (static_cast<VM *>(vm)->sr_mode) return;	/* sr_mode do nothing! */
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
		if (static_cast<VM *>(vm)->sr_mode) return;	/* sr_mode do nothing! */
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
		if (static_cast<VM *>(vm)->sr_mode) return;	/* sr_mode do nothing! */
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
#endif
	}
	return;
}

#ifndef _PC6001
uint32_t MEMORY::read_io8(uint32_t addr)
{
	uint16_t port=(addr & 0x00ff);
	uint8_t Value=0xff;

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
	case 0xF0: if (!static_cast<VM *>(vm)->sr_mode) Value=portF0;break;
	case 0xF1: if (!static_cast<VM *>(vm)->sr_mode) Value=portF1;break;
	}
	return(Value);
}

#define EVENT_HBLANK	1

void MEMORY::event_vline(int v, int clock)
{
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	if(static_cast<VM *>(vm)->sr_mode) {
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

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
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
			CGSW93=0; if (!static_cast<VM *>(vm)->sr_mode) write_io8(0xf0, portF0);
		} else {
			CGSW93=1; RdMem[3]=CGROM;
		}
		CRTKILL = (data & 2) ? 0 : 1;
#endif
	}
}

void MEMORY::open_cart(const _TCHAR* file_path)
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

#define STATE_VERSION	1

#include "../../statesub.h"

void MEMORY::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_1D_ARRAY(&(MEMORY_BASE[RAM_BASE]), RAM_SIZE);
//	state_fio->FputInt32((int)(CGROM - MEMORY_BASE));
//	state_fio->FputInt32((int)(EXTROM1 - MEMORY_BASE));
//	state_fio->FputInt32((int)(EXTROM2 - MEMORY_BASE));
//	for(int i = 0; i < 8; i++) {
//		state_fio->FputInt32((int)(RdMem[i] - MEMORY_BASE));
//		state_fio->FputInt32((int)(WrMem[i] - MEMORY_BASE));
//	}
//	state_fio->FputInt32((int)(VRAM - MEMORY_BASE));
	DECL_STATE_ENTRY_1D_ARRAY(EnWrite, sizeof(EnWrite));
	DECL_STATE_ENTRY_UINT8(CGSW93);
	DECL_STATE_ENTRY_BOOL(inserted);
#ifndef _PC6001
	DECL_STATE_ENTRY_UINT8(CRTKILL);
//	DECL_STATE_ENTRY_INT32((int)(CurKANJIROM - MEMORY_BASE));
	DECL_STATE_ENTRY_UINT8(CRTMode1);
	DECL_STATE_ENTRY_UINT8(CRTMode2);
	DECL_STATE_ENTRY_UINT8(CRTMode3);
	DECL_STATE_ENTRY_UINT8(CSS1);
	DECL_STATE_ENTRY_UINT8(CSS2);
	DECL_STATE_ENTRY_UINT8(CSS3);
	DECL_STATE_ENTRY_UINT8(portF0);
	DECL_STATE_ENTRY_UINT8(portF1);
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	DECL_STATE_ENTRY_INT32(bitmap);
	DECL_STATE_ENTRY_INT32(cols);
	DECL_STATE_ENTRY_INT32(rows);
	DECL_STATE_ENTRY_INT32(lines);
//	state_fio->FputInt32((int)(TEXTVRAM - MEMORY_BASE));
//	state_fio->FputInt32((int)(SYSROM2 - MEMORY_BASE));
	DECL_STATE_ENTRY_1D_ARRAY(&(MEMORY_BASE[EXTRAM_BASE]), EXTRAM_SIZE);
	DECL_STATE_ENTRY_1D_ARRAY(port60, sizeof(port60));
	DECL_STATE_ENTRY_UINT8(portC1);
	DECL_STATE_ENTRY_UINT8(portC8);
	DECL_STATE_ENTRY_UINT8(portCA);
	DECL_STATE_ENTRY_UINT8(portCB);
	DECL_STATE_ENTRY_UINT8(portCC);
	DECL_STATE_ENTRY_UINT8(portCE);
	DECL_STATE_ENTRY_UINT8(portCF);
	DECL_STATE_ENTRY_1D_ARRAY(palet, sizeof(palet) / sizeof(int));
#endif
#endif
	// Pointer Values
	DECL_STATE_ENTRY_INT32(tmp_cgrom_ptr);
	DECL_STATE_ENTRY_INT32(tmp_extrom1_ptr);
	DECL_STATE_ENTRY_INT32(tmp_extrom2_ptr);
	{
		DECL_STATE_ENTRY_1D_ARRAY(tmp_rdmem_ptr, 8);
		DECL_STATE_ENTRY_1D_ARRAY(tmp_wrmem_ptr, 8);
	}
	DECL_STATE_ENTRY_INT32(tmp_vram_ptr);
#ifndef _PC6001
	DECL_STATE_ENTRY_INT32(tmp_kanjirom_ptr);
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	DECL_STATE_ENTRY_INT32(tmp_textvram_ptr);
	DECL_STATE_ENTRY_INT32(tmp_sysrom2_ptr);
#endif
#endif

	leave_decl_state();
}

void MEMORY::save_state(FILEIO* state_fio)
{
	tmp_cgrom_ptr = (int)(CGROM - MEMORY_BASE);
	tmp_extrom1_ptr = (int)(EXTROM1 - MEMORY_BASE);
	tmp_extrom2_ptr = (int)(EXTROM2 - MEMORY_BASE);
	for(int i = 0; i < 8; i++) {
		tmp_rdmem_ptr[i] = (int)(RdMem[i] - MEMORY_BASE);
		tmp_wrmem_ptr[i] = (int)(WrMem[i] - MEMORY_BASE);
	}
	tmp_vram_ptr =(int)(VRAM - MEMORY_BASE);
#ifndef _PC6001
	tmp_kanjirom_ptr = (int)(CurKANJIROM - MEMORY_BASE);
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	tmp_textvram_ptr = (int)(TEXTVRAM - MEMORY_BASE);
	tmp_sysrom2_ptr = (int)(SYSROM2 - MEMORY_BASE);
#endif
#endif

	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->Fwrite(RAM, RAM_SIZE, 1);
//	state_fio->FputInt32((int)(CGROM - MEMORY_BASE));
//	state_fio->FputInt32((int)(EXTROM1 - MEMORY_BASE));
//	state_fio->FputInt32((int)(EXTROM2 - MEMORY_BASE));
//	for(int i = 0; i < 8; i++) {
//		state_fio->FputInt32((int)(RdMem[i] - MEMORY_BASE));
//		state_fio->FputInt32((int)(WrMem[i] - MEMORY_BASE));
//	}
//	state_fio->FputInt32((int)(VRAM - MEMORY_BASE));
//	state_fio->Fwrite(EnWrite, sizeof(EnWrite), 1);
//	state_fio->FputUint8(CGSW93);
//	state_fio->FputBool(inserted);
#ifndef _PC6001
//	state_fio->FputUint8(CRTKILL);
//	state_fio->FputInt32((int)(CurKANJIROM - MEMORY_BASE));
//	state_fio->FputUint8(CRTMode1);
//	state_fio->FputUint8(CRTMode2);
//	state_fio->FputUint8(CRTMode3);
//	state_fio->FputUint8(CSS1);
//	state_fio->FputUint8(CSS2);
//	state_fio->FputUint8(CSS3);
//	state_fio->FputUint8(portF0);
//	state_fio->FputUint8(portF1);
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
//	state_fio->FputInt32(bitmap);
//	state_fio->FputInt32(cols);
//	state_fio->FputInt32(rows);
//	state_fio->FputInt32(lines);
//	state_fio->FputInt32((int)(TEXTVRAM - MEMORY_BASE));
//	state_fio->FputInt32((int)(SYSROM2 - MEMORY_BASE));
//	state_fio->Fwrite(EXTRAM, EXTRAM_SIZE, 1);
//	state_fio->Fwrite(port60, sizeof(port60), 1);
//	state_fio->FputUint8(portC1);
//	state_fio->FputUint8(portC8);
//	state_fio->FputUint8(portCA);
//	state_fio->FputUint8(portCB);
//	state_fio->FputUint8(portCC);
//	state_fio->FputUint8(portCE);
//	state_fio->FputUint8(portCF);
//	state_fio->Fwrite(palet, sizeof(palet), 1);
#endif
#endif
}

bool MEMORY::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		return false;
	}
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	state_fio->Fread(RAM, RAM_SIZE, 1);
//	CGROM = MEMORY_BASE + state_fio->FgetInt32();
//	EXTROM1 = MEMORY_BASE + state_fio->FgetInt32();
//	EXTROM2 = MEMORY_BASE + state_fio->FgetInt32();
//	for(int i = 0; i < 8; i++) {
//		RdMem[i] = MEMORY_BASE + state_fio->FgetInt32();
//		WrMem[i] = MEMORY_BASE + state_fio->FgetInt32();
//	}
//	VRAM = MEMORY_BASE + state_fio->FgetInt32();
//	state_fio->Fread(EnWrite, sizeof(EnWrite), 1);
//	CGSW93 = state_fio->FgetUint8();
//	inserted = state_fio->FgetBool();
//#ifndef _PC6001
//	CRTKILL = state_fio->FgetUint8();
//	CurKANJIROM = MEMORY_BASE + state_fio->FgetInt32();
//	CRTMode1 = state_fio->FgetUint8();
//	CRTMode2 = state_fio->FgetUint8();
//	CRTMode3 = state_fio->FgetUint8();
//	CSS1 = state_fio->FgetUint8();
//	CSS2 = state_fio->FgetUint8();
//	CSS3 = state_fio->FgetUint8();
//	portF0 = state_fio->FgetUint8();
//	portF1 = state_fio->FgetUint8();
//#if defined(_PC6601SR) || defined(_PC6001MK2SR)
//	bitmap = state_fio->FgetInt32();
//	cols = state_fio->FgetInt32();
//	rows = state_fio->FgetInt32();
//	lines = state_fio->FgetInt32();
//	TEXTVRAM = MEMORY_BASE + state_fio->FgetInt32();
//	SYSROM2 = MEMORY_BASE + state_fio->FgetInt32();
//	state_fio->Fread(EXTRAM, EXTRAM_SIZE, 1);
//	state_fio->Fread(port60, sizeof(port60), 1);
//	portC1 = state_fio->FgetUint8();
//	portC8 = state_fio->FgetUint8();
//	portCA = state_fio->FgetUint8();
//	portCB = state_fio->FgetUint8();
//	portCC = state_fio->FgetUint8();
//	portCE = state_fio->FgetUint8();
//	portCF = state_fio->FgetUint8();
//	state_fio->Fread(palet, sizeof(palet), 1);
//#endif
//#endif

	// Post Process
	CGROM = MEMORY_BASE + tmp_cgrom_ptr;
	EXTROM1 = MEMORY_BASE + tmp_extrom1_ptr;
	EXTROM2 = MEMORY_BASE + tmp_extrom2_ptr;
	for(int i = 0; i < 8; i++) {
		RdMem[i] = MEMORY_BASE + tmp_rdmem_ptr[i];
		WrMem[i] = MEMORY_BASE + tmp_wrmem_ptr[i];
	}
	VRAM = MEMORY_BASE + tmp_vram_ptr;
#ifndef _PC6001
	CurKANJIROM = MEMORY_BASE + tmp_kanjirom_ptr;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	TEXTVRAM = MEMORY_BASE + tmp_textvram_ptr;
	SYSROM2 = MEMORY_BASE + tmp_sysrom2_ptr;
#endif
#endif
	return true;
}

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateBuffer(RAM, RAM_SIZE, 1);
	if(loading) {
		CGROM = MEMORY_BASE + state_fio->FgetInt32_LE();
		EXTROM1 = MEMORY_BASE + state_fio->FgetInt32_LE();
		EXTROM2 = MEMORY_BASE + state_fio->FgetInt32_LE();
		for(int i = 0; i < 8; i++) {
			RdMem[i] = MEMORY_BASE + state_fio->FgetInt32_LE();
			WrMem[i] = MEMORY_BASE + state_fio->FgetInt32_LE();
		}
		VRAM = MEMORY_BASE + state_fio->FgetInt32_LE();
	} else {
		state_fio->FputInt32_LE((int)(CGROM - MEMORY_BASE));
		state_fio->FputInt32_LE((int)(EXTROM1 - MEMORY_BASE));
		state_fio->FputInt32_LE((int)(EXTROM2 - MEMORY_BASE));
		for(int i = 0; i < 8; i++) {
			state_fio->FputInt32_LE((int)(RdMem[i] - MEMORY_BASE));
			state_fio->FputInt32_LE((int)(WrMem[i] - MEMORY_BASE));
		}
		state_fio->FputInt32_LE((int)(VRAM - MEMORY_BASE));
	}
	state_fio->StateBuffer(EnWrite, sizeof(EnWrite), 1);
	state_fio->StateUint8(CGSW93);
	state_fio->StateBool(inserted);
#ifndef _PC6001
	state_fio->StateUint8(CRTKILL);
	if(loading) {
		CurKANJIROM = MEMORY_BASE + state_fio->FgetInt32_LE();
	} else {
		state_fio->FputInt32_LE((int)(CurKANJIROM - MEMORY_BASE));
	}
	state_fio->StateUint8(CRTMode1);
	state_fio->StateUint8(CRTMode2);
	state_fio->StateUint8(CRTMode3);
	state_fio->StateUint8(CSS1);
	state_fio->StateUint8(CSS2);
	state_fio->StateUint8(CSS3);
	state_fio->StateUint8(portF0);
	state_fio->StateUint8(portF1);
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	state_fio->StateInt32(bitmap);
	state_fio->StateInt32(cols);
	state_fio->StateInt32(rows);
	state_fio->StateInt32(lines);
	if(loading) {
		TEXTVRAM = MEMORY_BASE + state_fio->FgetInt32_LE();
		SYSROM2 = MEMORY_BASE + state_fio->FgetInt32_LE();
	} else {
		state_fio->FputInt32_LE((int)(TEXTVRAM - MEMORY_BASE));
		state_fio->FputInt32_LE((int)(SYSROM2 - MEMORY_BASE));
	}
	state_fio->StateBuffer(EXTRAM, EXTRAM_SIZE, 1);
	state_fio->StateBuffer(port60, sizeof(port60), 1);
	state_fio->StateUint8(portC1);
	state_fio->StateUint8(portC8);
	state_fio->StateUint8(portCA);
	state_fio->StateUint8(portCB);
	state_fio->StateUint8(portCC);
	state_fio->StateUint8(portCE);
	state_fio->StateUint8(portCF);
	state_fio->StateBuffer(palet, sizeof(palet), 1);
#endif
#endif
	return true;
}
