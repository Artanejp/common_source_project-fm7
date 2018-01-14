/*
 * Emulation of Fujitsu Japanese Communication Card.
 * (C) 2018 K.Ohta.
 * Note:
 * Based on XM7 L70 , with permittion from Ryu Takegami. 
 */

#include "vm.h"
#include "../../fileio.h"
#include "emu.h"

#include "fm7_common.h"

#include "../mc6809.h"
#include "./jcommcard.h"

FM7_JCOMMCARD::FM7_JCOMMCARD(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	n_bank = 0;
	rcb_address = 0;
	halted = false;
	kanji_address = 0x00000;
	jis78_emulation = false;
		
	memset(prog_rom, 0xff, sizeof(prog_rom));
	memset(dict_rom, 0xff, sizeof(dict_rom));
	memset(kanji_rom, 0xff, sizeof(kanji_rom));
	memset(backup_ram, 0x00, sizeof(backup_ram));
	cpu = NULL;
	modified = true;
	firmware_ok = false;
}

FM7_JCOMMCARD::~FM7_JCOMMCARD()
{
}

void FM7_JCOMMCARD::initialize(void)
{
	FILEIO *fio = new FILEIO();
	bool b_stat = false;
	
	if(fio->Fopen(create_local_path(_T(ROM_JCOMM_FIRMWARE)), FILEIO_READ_BINARY)) { // 20180114
		fio->Fread(prog_rom, sizeof(prog_rom), 1);
		fio->Fclose();
		firmware_ok = true;
	}

	/* Patch from XM7/VM/jsubsys.c */
	if(prog_rom[0x000d] == 0x8f) {
		prog_rom[0x000d] = 0x88;
	}
	/* Change: DICT.ROM to JSUBDICT.ROM */
	if(fio->Fopen(create_local_path(_T(ROM_JCOMM_DICTIONARY)), FILEIO_READ_BINARY)) {
		fio->Fread(dict_rom, sizeof(dict_rom), 1);
		fio->Fclose();
	}
	/* KANJI ROM */
	if(fio->Fopen(create_local_path(_T(ROM_JCOMM_KANJI)), FILEIO_READ_BINARY)) {
		fio->Fread(kanji_rom, sizeof(kanji_rom), 1);
		fio->Fclose();
		b_stat = true;
	} else if(fio->Fopen(create_local_path(_T(ROM_KANJI_CLASS1)), FILEIO_READ_BINARY)) {
		fio->Fread(kanji_rom, sizeof(kanji_rom), 1);
		fio->Fclose();
		b_stat = true;
	} else if(fio->Fopen(create_local_path(_T(ROM_KANJI_CLASS1_FALLBACK)), FILEIO_READ_BINARY)) {
		fio->Fread(kanji_rom, sizeof(kanji_rom), 1);
		fio->Fclose();
		b_stat = true;
	}
	this->out_debug_log(_T("KANJIROM READ %s."), b_stat ? "OK" : "FAILED");

	if(fio->Fopen(create_local_path(_T(RAM_JCOMM_BACKUP)), FILEIO_READ_BINARY)) {
		fio->Fread(backup_ram, sizeof(backup_ram), 1);
		fio->Fclose();
		modified = false;
	}
	delete fio;

#if defined(_FM77AV_VARIANTS)
	jis78_emulation = false;
#else
	jis78_emulation = ((config.dipswitch & FM7_DIPSW_JIS78EMU_ON) != 0);
#endif
	if(jis78_emulation && b_stat) {
		patch_jis78();
	}
}

bool FM7_JCOMMCARD::patch_jis78(void)
{
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
				kanji_rom[patchaddr + offset] = (uint8_t)(offset & 1);
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
			kanji_rom[convert_table[patchaddr][0] * 2 + offset] =
				tmpbuf[convert_table[patchaddr][1] * 2 + offset];
		}
	}
	
	free(tmpbuf);
	return true;
}

void FM7_JCOMMCARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool b = ((data & mask) != 0);
	switch(id) {
	case FM7_JCOMMCARD_BUS_HALT:
		halted = b;
		break;
	}
}

uint32_t FM7_JCOMMCARD::read_io8(uint32_t address)
{
	uint32_t data = 0xff;
	switch(address & 3) {
	case 0:
		/* HALT STATUS */
		if(halted) {
			data &= 0x7f;
		}
		break;
	case 1:
		/* RCB DATA (Auto increment address) */
		if(halted) {
			data = backup_ram[0x1f00 | rcb_address];
			rcb_address++;
		}
		break;
	case 2:
	case 3:
		/* Kanji Data */
		if((jis78_emulation) && (kanji_address >= 0x3000) && (kanji_address < 0x4000)) {
			/* JIS78 */
			data =  address & 1;
		} else {
			data = kanji_rom[(kanji_address << 1) + (address & 1)];
		}
		break;
	}
	return data;
}

void FM7_JCOMMCARD::write_io8(uint32_t address, uint32_t data)
{
	switch(address & 3) {
	case 0:
		/* Kanji Address High */
		kanji_address = (kanji_address & 0x0000ff) | ((data & 0x000000ff) << 8);
		break;
	case 1:
		/* Kanji Address Low */
		kanji_address = (kanji_address & 0x00ff00) | (data & 0x000000ff);
		break;
	case 2:
		/* REQUEST TO HALT */
		if((data & 0x80) != 0) {
			if(cpu != NULL) cpu->write_signal(SIG_CPU_HALTREQ, 0x00000000, 0xffffffff);
			//haltreq = false;
			//halted = false;
		} else {
			if(cpu != NULL) cpu->write_signal(SIG_CPU_HALTREQ, 0xffffffff, 0xffffffff);
			//haltreq = true;
			//halted = true;
			rcb_address = 0;
		}
		break;
	case 3:
		/* RCB DATA (Auto increment address) */
		if(halted) {
			backup_ram[0x1f00 | rcb_address] = (uint8_t)data;
			rcb_address++;
		}
		break;
	}
}

uint32_t FM7_JCOMMCARD::read_data8(uint32_t address)
{
	/*
	 * $8000-$9FFE : SRAM
	 * $9FFF       : SYNC/BANK REG
	 * $A000-$AFFF : DICT
	 * $C000-$FFFF : SUB SYSTEM
	 */
	if(address < 0x8000) return 0xff; /* NOOP */
	if(address <= 0x9ffe) { /* SRAM */
		return (uint32_t)backup_ram[address & 0x1fff];
	} else if(address == 0x9fff) { /* RCB BANK REGISTER */
		return (uint32_t)n_bank;
	} else if(address < 0xb000) { /* DICT ROM */
		return (uint32_t)(dict_rom[(address & 0x0fff) | (((uint32_t)n_bank) << 12)]);
	} else if(address < 0xc000) {
		return 0xff;
	} else if(address < 0x10000) {
		return (uint32_t)prog_rom[address & 0x3fff];
	}
	return 0xff;
}

void FM7_JCOMMCARD::write_data8(uint32_t address, uint32_t data)
{
	if(address < 0x8000) return;
	if(address >= 0xa000) return;
	if(address == 0x9fff) {
		if(cpu != NULL) cpu->write_signal(SIG_CPU_HALTREQ, ((data & 0x80) == 0) ? 0xffffffff : 0, 0xffffffff);
		//halted = ((data & 0x80) == 0);
		n_bank = (uint8_t)(data & 0x3f); 
	} else if(address < 0x9fff) {
		modified = true;
		backup_ram[address & 0x1fff] = (uint8_t)data;
	}
}

void FM7_JCOMMCARD::release(void)
{
	FILEIO *fio = new FILEIO();
	if(modified) {
		if(fio->Fopen(create_local_path(_T(RAM_JCOMM_BACKUP)), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(backup_ram, sizeof(backup_ram), 1);
			fio->Fclose();
			modified = false;
		}
	}
}
	
void FM7_JCOMMCARD::reset(void)
{
	rcb_address = 0x00;
	kanji_address = 0x00000;
	if(cpu != NULL) cpu->write_signal(SIG_CPU_HALTREQ, 0, 0xffffffff);
#if defined(_FM77AV_VARIANTS)
	jis78_emulation = false;
#else
	jis78_emulation = ((config.dipswitch & FM7_DIPSW_JIS78EMU_ON) != 0);
#endif
	if(cpu != NULL) cpu->reset();
}

#define STATE_VERSION 2

void FM7_JCOMMCARD::save_state(FILEIO *state_fio)
{
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);
	this->out_debug_log(_T("Save State: JCOMM CARD: id=%d ver=%d\n"), this_device_id, STATE_VERSION);

	state_fio->FputUint8(n_bank & 0x3f);
	state_fio->FputUint8(rcb_address);
	state_fio->FputUint32_BE(kanji_address);

	state_fio->FputBool(halted);

	state_fio->Fwrite(prog_rom, sizeof(prog_rom), 1);
	state_fio->Fwrite(dict_rom, sizeof(dict_rom), 1);
	state_fio->Fwrite(kanji_rom, sizeof(kanji_rom), 1);
	state_fio->Fwrite(backup_ram, sizeof(backup_ram), 1);
	state_fio->FputBool(firmware_ok);

#if !defined(_FM77AV_VARIANTS)
	state_fio->FputBool(jis78_emulation);
#endif
}

bool FM7_JCOMMCARD::load_state(FILEIO *state_fio)
{
	uint32_t version;
	version = state_fio->FgetUint32_BE();
	if(this_device_id != state_fio->FgetInt32_BE()) return false;
	this->out_debug_log(_T("Load State: JCOMM CARD: id=%d ver=%d\n"), this_device_id, STATE_VERSION);

	if(version >= 1) {
		n_bank = state_fio->FgetUint8() & 0x3f;
		rcb_address = state_fio->FgetUint8();
		kanji_address = state_fio->FgetUint32_BE();

		halted = state_fio->FgetBool();

		state_fio->Fread(prog_rom, sizeof(prog_rom), 1);
		state_fio->Fread(dict_rom, sizeof(dict_rom), 1);
		state_fio->Fread(kanji_rom, sizeof(kanji_rom), 1);
		state_fio->Fread(backup_ram, sizeof(backup_ram), 1);
		firmware_ok = state_fio->FgetBool();
		modified = true;

#if !defined(_FM77AV_VARIANTS)
		jis78_emulation = state_fio->FgetBool();
#else
		jis78_emulation = false;
#endif
	}
	return true;
}
