/*
 * Common source code project -> FM-7/77/AV -> Kanji rom
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *  Feb 11, 2015 : Initial
 */

#include "vm.h"
#include "../../fileio.h"
#include "emu.h"
#include "fm7_common.h"
#include "kanjirom.h"

KANJIROM::KANJIROM(VM *parent_vm, EMU* parent_emu, bool type_2std): DEVICE(parent_vm, parent_emu)
{
	FILEIO *fio;
	read_ok = false;
	
	fio = new FILEIO();
	memset(data_table, 0xff, 0x20000); 
	//	read_table[0].memory = data_table;
	p_emu = parent_emu;

#if !defined(_FM77AV_VARIANTS)
	jis78_emulation = ((config.dipswitch & FM7_DIPSW_JIS78EMU_ON) != 0);
#else
	jis78_emulation = false;
#endif
	if(type_2std) {
		class2 = true;
		if(fio->Fopen(create_local_path(_T(ROM_KANJI_CLASS2)), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
			jis78_emulation = false;
		}
	} else {
		class2 = false;
		if(fio->Fopen(create_local_path(_T(ROM_KANJI_CLASS1)), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
		} else if(fio->Fopen(create_local_path(_T(ROM_KANJI_CLASS1_FALLBACK)), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
		}
		
	}
	if(class2) {
		set_device_name(_T("FM7_KANJI_CLASS2"));
	} else {
		set_device_name(_T("FM7_KANJI_CLASS1"));
	}
	if(class2) {
		set_device_name(_T("FM7_KANJI_CLASS2"));
	} else {
		set_device_name(_T("FM7_KANJI_CLASS1"));
	}
	this->out_debug_log(_T("KANJIROM READ %s."), read_ok ? "OK" : "FAILED");
	
	if(jis78_emulation && read_ok) {
		if(patch_jis78()) {
			this->out_debug_log(_T("PATCHED KANJIROM to emulate JIS78 feature."));
		} else {
			this->out_debug_log(_T("FAILED TO PATCH KANJIROM."));
		}				
	}
	kanjiaddr.d = 0;
	delete fio;
	return;
}

KANJIROM::~KANJIROM()
{
}

void KANJIROM::reset(void)
{
	kanjiaddr.d = 0;
}

bool KANJIROM::patch_jis78(void)
{
	if(class2) return true;
	// JIS78 Undefined table (Refer from FM77AV)
	// Made by Ryu Takegami.From XM7 v3.4 L77a
	static const uint32_t jis78_table[] = {
		0xffffffff, 0x00000001, 0xffff8001, 0xfc00ffff,
		0x00000001, 0x00000001, 0xfe000001, 0x00000001,
		0xffffffff, 0x80000000, 0xffffffff, 0xf8000001,
		0xfff00000, 0xff800000, 0xffffffff, 0xfffc0000,
		0xffffffff, 0x00000000, 0xffffffff, 0xf8000001,
		0x00000000, 0x00000000, 0xfe000001, 0x0001fffc,
	};
	static const uint32_t convert_table[][2] = {
		/* 第1水準code addr		  第2水準code addr */
		{/*鯵 0x3033*/0x4130,	/*鰺 0x724D*/0xE4D0},
		{/*鴬 0x3229*/0x4490,	/*鶯 0x7274*/0xD540},
		{/*蛎 0x3342*/0x6620,	/*蠣 0x695A*/0x93A0},
		{/*撹 0x3349*/0x6690,	/*攪 0x5978*/0x5380},
		{/*竃 0x3376*/0x8760,	/*竈 0x635E*/0x87E0},
		{/*潅 0x3443*/0x6830,	/*灌 0x5E75*/0x5D50},
		{/*諌 0x3452*/0x6920,	/*諫 0x6B5D*/0x97D0},
		{/*頚 0x375B*/0x6FB0,	/*頸 0x7074*/0xD140},
		{/*砿 0x395C*/0x73C0,	/*礦 0x6268*/0xA480},
		{/*蕊 0x3C49*/0x7890,	/*蘂 0x6922*/0x7220},
		{/*靭 0x3F59*/0x7F90,	/*靱 0x7057*/0xE170},
		{/*賎 0x4128*/0xA280,	/*賤 0x6C4D*/0x98D0},
		{/*壷 0x445B*/0xC9B0,	/*壺 0x5464*/0x4840},
		{/*砺 0x4557*/0xCB70,	/*礪 0x626A*/0xA4A0},
		{/*梼 0x456E*/0xEAE0,	/*檮 0x5B6D*/0x56D0},
		{/*涛 0x4573*/0xEB30,	/*濤 0x5E39*/0x1D90},
		{/*迩 0x4676*/0xED60,	/*邇 0x6D6E*/0xBAE0},
		{/*蝿 0x4768*/0xEE80,	/*蠅 0x6A24*/0x7440},
		{/*桧 0x4930*/0xB300,	/*檜 0x5B58*/0x3780},
		{/*侭 0x4B79*/0xF790,	/*儘 0x5056*/0x2160},
		{/*薮 0x4C79*/0xF990,	/*藪 0x692E*/0x72E0},
		{/*篭 0x4F36*/0xBF60,	/*籠 0x6446*/0x8860},
		{/*尭 0x3646*/0x6C60,	/*堯 0x7421*/0xC810},
		{/*槙 0x4B6A*/0xF6A0,	/*槇 0x7422*/0xC820},
		{/*遥 0x4D5A*/0xDBA0,	/*遙 0x7423*/0xC830},
		{0, 0},
	};

	for(uint32_t patchaddr = 0; patchaddr < 0x6000; patchaddr += 32) {
		if((jis78_table[patchaddr >> 10] & (1 << (patchaddr >> 5))) != 0) {
			for(uint32_t offset = 0; offset < 32; offset++) {
				data_table[patchaddr + offset] = (uint8_t)(offset & 1);
			}
		}
	}
	// Patch ROM to JIS83 by Ryu Takegami.From XM7 v3.4L77a.
	uint8_t *tmpbuf = (uint8_t *)malloc(0x20000);
	if(tmpbuf == NULL) return false;
	
	FILEIO *fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T(ROM_KANJI_CLASS2)), FILEIO_READ_BINARY)) {
		fio->Fread(tmpbuf, 0x20000, 1);
		fio->Fclose();
	} else {
		delete fio;
		free(tmpbuf);
		return false;
	}

	for (uint32_t patchaddr = 0; convert_table[patchaddr][0] != 0; patchaddr++) {
		for (uint32_t offset = 0; offset < 32; offset++) {
			data_table[convert_table[patchaddr][0] * 2 + offset] =
				tmpbuf[convert_table[patchaddr][1] * 2 + offset];
		}
	}
	
	free(tmpbuf);
	return true;
}

void KANJIROM::write_data8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case KANJIROM_ADDR_HI:
		kanjiaddr.b.h = data & 0xff;
		break;
	case KANJIROM_ADDR_LO:
		kanjiaddr.b.l = data & 0xff;
		break;
	}		
	return;
}

uint32_t KANJIROM::read_data8(uint32_t addr)
{
	if(addr == KANJIROM_DATA_HI) {
#if !defined(_FM77AV_VARIANTS)
		if((jis78_emulation) && (kanjiaddr.d >= 0x3000) && (kanjiaddr.d < 0x4000)) {
			return 0;
		}
#endif
		return data_table[(kanjiaddr.d << 1) & 0x1ffff];
	} else if(addr == KANJIROM_DATA_LO) {
#if !defined(_FM77AV_VARIANTS)
		if((jis78_emulation) && (kanjiaddr.d >= 0x3000) && (kanjiaddr.d < 0x4000)) {
			return 1;
		}
#endif
		return data_table[((kanjiaddr.d << 1) & 0x1ffff) + 1];
	} else if(addr == KANJIROM_READSTAT) {
		return (read_ok) ? 0xffffffff : 0x00000000;
	} else if((addr >= KANJIROM_DIRECTADDR) && (addr < (KANJIROM_DIRECTADDR + 0x20000))) {
		return data_table[addr - KANJIROM_DIRECTADDR];
	}
	return 0x00000000;
}

bool KANJIROM::get_readstat(void)
{
	return read_ok;
}

void KANJIROM::release()
{
}

#define STATE_VERSION 3
void KANJIROM::save_state(FILEIO *state_fio)
{
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);
	this->out_debug_log(_T("Save State: KANJIROM: id=%d ver=%d\n"), this_device_id, STATE_VERSION);

	state_fio->FputBool(class2);
	state_fio->FputBool(read_ok);
	state_fio->Fwrite(data_table, sizeof(data_table), 1);
	state_fio->FputUint16_BE(kanjiaddr.w.l);

#if !defined(_FM77AV_VARIANTS)
	state_fio->FputBool(jis78_emulation);
#endif
}

bool KANJIROM::load_state(FILEIO *state_fio)
{
	uint32_t version;
	version = state_fio->FgetUint32_BE();
	if(this_device_id != state_fio->FgetInt32_BE()) return false;
	this->out_debug_log(_T("Load State: KANJIROM: id=%d ver=%d\n"), this_device_id, version);

	if(version >= 1) {
		class2 = state_fio->FgetBool();
		read_ok = state_fio->FgetBool();
		state_fio->Fread(data_table, sizeof(data_table), 1);
		if(version == 1) return true;
	}
	if(version >= 2) {
		kanjiaddr.d = 0;
		kanjiaddr.w.l = state_fio->FgetUint16_BE();
		if(version == 2) return true;
	}
#if !defined(_FM77AV_VARIANTS)
	if(version >= 3) {
		jis78_emulation = state_fio->FgetBool();
	}
#else
	jis78_emulation = false;
#endif
	return false;
}

