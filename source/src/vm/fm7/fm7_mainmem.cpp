/*
 * Main memory MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *  License: GPLv2
 *
 */
#include "vm.h"
#include "emu.h"
#include "fm7_mainmem.h"

FM7_MAINMEM::FM7_MAINMEM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	p_vm = parent_vm;
	p_emu = parent_emu;
#if !defined(_FM77AV_VARIANTS)
	for(int i = 0; i < 4; i++) fm7_bootroms[i] = (uint8_t *)malloc(0x200);
#endif	
	mainio = NULL;
	display = NULL;
	maincpu = NULL;
#if defined(CAPABLE_DICTROM)
	kanjiclass1 = NULL;
#endif	
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)  || \
    defined(_FM77_VARIANTS)
	fm7_mainmem_extram = NULL;
#endif
	// Initialize table
	set_device_name(_T("MAIN MEMORY"));
}

FM7_MAINMEM::~FM7_MAINMEM()
{
}

void FM7_MAINMEM::reset()
{
   	waitfactor = 0;
	waitcount = 0;
	ioaccess_wait = false;
	//sub_halted = (display->read_signal(SIG_DISPLAY_HALT) == 0) ? false : true;
	sub_halted = false;
	memset(fm7_mainmem_bootrom_vector, 0x00, 0x10); // Clear without vector

#if defined(_FM77AV_VARIANTS)
	memset(fm7_bootram, 0x00, 0x1f0);
	initiator_enabled = true;
	boot_ram_write = true;
#elif defined(_FM77_VARIANTS)
	boot_ram_write = false;
#endif	
	bootmode = config.boot_mode & 3;
#if defined(HAS_MMR)
	if((config.dipswitch & FM7_DIPSW_EXTRAM) != 0) {
		extram_connected = true;
	} else {
		extram_connected = false;
	}
#endif
#if defined(_FM77AV_VARIANTS)
	if(dictrom_connected) {
		use_page2_extram = true;
	} else {
		use_page2_extram = ((config.dipswitch & FM7_DIPSW_EXTRAM_AV) != 0) ? true : false;
	}
#endif   
#ifdef HAS_MMR
	mmr_extend = false;
	mmr_segment = 0;
	window_offset = 0;
	mmr_enabled = false;
	mmr_fast = false;
	window_enabled = false;
	window_fast = false;
	refresh_fast = false;
#endif
	if((bootmode & 0x03) == 0) { // IF BASIC BOOT THEN ROM
		basicrom_fd0f = true;
	} else { // ELSE RAM
		basicrom_fd0f = false;
	}
	clockmode = (config.cpu_type == 0) ? true : false;
	is_basicrom = ((bootmode & 0x03) == 0) ? true : false;
	setclock(clockmode ? 0 : 1);
	init_data_table();
	update_all_mmr_jumptable();
	maincpu->reset();
}

void FM7_MAINMEM::setclock(int mode)
{
	uint32_t clock = MAINCLOCK_SLOW;
	if(mode == 1) { // SLOW
		clock = MAINCLOCK_SLOW; // Temporally
#if defined(HAS_MMR)		
		if(!mmr_fast && !window_fast) {
			if(refresh_fast) {
				if(mmr_enabled || window_enabled) {
					clock = (uint32_t)((double)clock * 1.089);
				} else {
					clock = (uint32_t)((double)clock * 1.086);
				}					
			}
		}
#endif		
	} else {
#if defined(HAS_MMR)
		if(window_enabled) {
			if(window_fast) {
				clock = MAINCLOCK_FAST_MMR;
			} else {
				clock = MAINCLOCK_MMR;
			}
		} else if(mmr_enabled) {
			if(mmr_fast) {
				clock = MAINCLOCK_FAST_MMR;
			} else {
				clock = MAINCLOCK_MMR;
			}
		} else {
			// fix by Ryu Takegami
			if(mmr_fast) {
				clock = MAINCLOCK_FAST_MMR;
			} else {
				clock = MAINCLOCK_NORMAL;
			}
		}
		if(!mmr_fast && !window_fast) {
			if(refresh_fast) {
				if(mmr_enabled || window_enabled) {
					clock = (uint32_t)((double)clock * 1.089);
				} else {
					clock = (uint32_t)((double)clock * 1.086);
				}					
			}
		}
#else
		clock = MAINCLOCK_NORMAL;
#endif				
	}
	p_vm->set_cpu_clock(this->maincpu, clock);
}
		

void FM7_MAINMEM::wait()
{
	int waitfactor; // If MMR of TWR enabled, factor = 3.
			    // If memory, factor = 2?
	if(!clockmode) return; // SLOW
#ifdef HAS_MMR
    //if(!mmr_fast && !window_fast && (window_enabled || mmr_enabled)) waitfactor = 2;
	if(!ioaccess_wait) {
		waitfactor = 2;
		ioaccess_wait = true;
	} else { // Not MMR, TWR or enabled FAST MMR mode
		waitfactor = 3; // If(MMR or TWR) and NOT FAST MMR factor = 3, else factor = 2
		if(mmr_fast) waitfactor = 2;
		ioaccess_wait = false;
	} 
#else
	waitfactor = 2;
#endif	  
	if(waitfactor <= 0) return;
	waitcount++;
	if(waitcount >= waitfactor) {
		if(maincpu != NULL) maincpu->set_extra_clock(1);
		waitcount = 0;
	}
}



int FM7_MAINMEM::check_extrom(uint32_t raddr, uint32_t *realaddr)
{
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(extrom_bank) { // Extra ROM selected.
		uint32_t dbank = extcard_bank & 0x3f;
		if(dbank < 0x20) { // KANJI
			if((dbank == 0x07) || (dbank == 0x06)) {
				// NOT KANJI AS IS.Thanks Ryu.
				*realaddr = raddr & 0x01;
				return FM7_MAINMEM_KANJI_DUMMYADDR;
			}
			*realaddr = (dbank << 12) | raddr;
			return FM7_MAINMEM_KANJI_LEVEL1;
		} else if(dbank < 0x2c) {
			raddr = ((dbank << 12) - 0x20000) | raddr;
			*realaddr = raddr;
			return FM7_MAINMEM_77AV40_EXTRAROM;
		} else if(dbank < 0x30) {
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		} else {
			raddr = ((dbank << 12) - 0x30000) | raddr;
			if((raddr >= 0x8000)  && (raddr < 0xfc00)) {
				*realaddr = raddr - 0x8000;
				return FM7_MAINMEM_BASICROM;
			} else if((raddr >= 0xfe00) && (raddr < 0xffe0)) {
				*realaddr = raddr - 0xfe00;
				return FM7_MAINMEM_BOOTROM_MMR;
			} else if(raddr >= 0xfffe) {
				*realaddr = raddr - 0xfffe;
				return FM7_MAINMEM_RESET_VECTOR;
			}
			//*realaddr = raddr + 0x10000;
			//return FM7_MAINMEM_77AV40_EXTRAROM;
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		}
	}
#endif	
	return -1;
}


uint32_t FM7_MAINMEM::read_signal(int sigid)
{
	uint32_t value = 0x00000000;
	switch(sigid) {
	case FM7_MAINIO_PUSH_FD0F:
		value = (basicrom_fd0f) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_IS_BASICROM:
		value = (is_basicrom) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_CLOCKMODE:
		value = (clockmode) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_BOOTMODE:
		value = (uint32_t)bootmode & 0x07;
		break;
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	case FM7_MAINIO_BOOTRAM_RW:
		value = (boot_ram_write) ? 0xffffffff : 0x00000000;
		break;
#endif			
#ifdef HAS_MMR			
	case FM7_MAINIO_WINDOW_ENABLED:
		value = (window_enabled) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_WINDOW_FAST:
		value = (window_fast) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_FASTMMR_ENABLED:
		value = (mmr_fast) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_MMR_ENABLED:
		value = (mmr_enabled) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_MMR_EXTENDED:
		value = (mmr_extend) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINMEM_REFRESH_FAST:
		value = (refresh_fast) ? 0xffffffff : 0x00000000;
		break;
#endif			
#if defined(_FM77AV_VARIANTS)
	case FM7_MAINIO_INITROM_ENABLED:
		value = (initiator_enabled) ? 0xffffffff: 0x00000000;
		break;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)	   
	case FM7_MAINIO_EXTROM:
		value = (extrom_bank) ? 0xffffffff: 0x00000000;
		break;
# endif	   
	case FM7_MAINIO_EXTBANK:
		value = extcard_bank & 0x3f;
		value |= (dictram_enabled) ? 0x80 : 0;
		value |= (dictrom_enabled) ? 0x40 : 0;
		break;
#endif
	}
	return value;
}


void FM7_MAINMEM::write_signal(int sigid, uint32_t data, uint32_t mask)
{
	bool flag = ((data & mask) != 0);
	switch(sigid) {
		case SIG_FM7_SUB_HALT:
			sub_halted = flag;
			break;
		case FM7_MAINIO_IS_BASICROM:
			is_basicrom = flag;
			break;
		case FM7_MAINIO_PUSH_FD0F:
			basicrom_fd0f = flag;
			break;
		case FM7_MAINIO_CLOCKMODE:
			clockmode = flag;
			setclock(clockmode ? 0 : 1);
			break;
		case FM7_MAINIO_BOOTMODE:
			bootmode = data & 0x07;
			break;
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
		case FM7_MAINIO_BOOTRAM_RW:
			boot_ram_write = flag;
			break;
#endif			
#ifdef _FM77AV_VARIANTS
		case FM7_MAINIO_INITROM_ENABLED:
			initiator_enabled = flag;
			break;
		case FM7_MAINIO_EXTBANK:
			extcard_bank = data & 0x3f;
			dictram_enabled = ((data & 0x80) != 0) ? true : false;
			dictrom_enabled = ((data & 0x40) != 0) ? true : false;
			break;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)	   
		case FM7_MAINIO_EXTROM:
			extrom_bank = flag;
	   		break;
# endif	   
#endif			
#ifdef HAS_MMR			
		case FM7_MAINIO_WINDOW_ENABLED:
			window_enabled = flag;
			setclock(config.cpu_type);
			break;
		case FM7_MAINIO_WINDOW_FAST:
			window_fast = flag;
			setclock(config.cpu_type);
			break;
		case FM7_MAINIO_FASTMMR_ENABLED:
			mmr_fast = flag;
			setclock(config.cpu_type);
			break;
		case FM7_MAINIO_MMR_ENABLED:
			mmr_enabled = flag;
			setclock(config.cpu_type);
			break;
		case FM7_MAINIO_MMR_EXTENDED:
			mmr_extend = flag;
			break;
		case FM7_MAINMEM_REFRESH_FAST:
			refresh_fast = flag;
			setclock(config.cpu_type);
			break;
#endif			
	}
}


uint32_t FM7_MAINMEM::read_dma_data8(uint32_t addr)
{
#if defined(HAS_MMR)	
	uint32_t val;
	val = this->read_data8_main(addr & 0xffff, true);
	return val;
#else
	return this->read_data8(addr & 0xffff);
#endif	
}

uint32_t FM7_MAINMEM::read_dma_io8(uint32_t addr)
{
#if defined(HAS_MMR)	
	uint32_t val;
	val = this->read_data8_main(addr & 0xffff, true);
	return val;
#else
	return this->read_data8(addr & 0xffff);
#endif	
}

uint32_t FM7_MAINMEM::read_data8(uint32_t addr)
{
#if defined(HAS_MMR)   
	if(addr >= FM7_MAINIO_WINDOW_OFFSET) {
		switch(addr) {
		case FM7_MAINIO_WINDOW_OFFSET:
			return (uint32_t)window_offset;
			break;
		case FM7_MAINIO_MMR_SEGMENT:
			return (uint32_t)mmr_segment;
			break;
		default:
			if((addr >= FM7_MAINIO_MMR_BANK) && (addr < (FM7_MAINIO_MMR_BANK + 0x80))){
				return mmr_map_data[addr - FM7_MAINIO_MMR_BANK];
			}
			break;
		}
		return 0xff;
	}
#endif   
	return read_data8_main(addr, false);
}

void FM7_MAINMEM::write_dma_data8(uint32_t addr, uint32_t data)
{
#if defined(HAS_MMR)
	this->write_data8_main(addr & 0xffff, data, true);
#else
	this->write_data8(addr & 0xffff, data);
#endif	
}

void FM7_MAINMEM::write_dma_io8(uint32_t addr, uint32_t data)
{
#if defined(HAS_MMR)
	this->write_data8_main(addr & 0xffff, data, true);
#else
	this->write_data8(addr & 0xffff, data);
#endif	
}

void FM7_MAINMEM::write_data8(uint32_t addr, uint32_t data)
{
#if defined(HAS_MMR)   
	if(addr >= FM7_MAINIO_WINDOW_OFFSET) {
		switch(addr) {
		case FM7_MAINIO_WINDOW_OFFSET:
			window_offset = data;
			break;
		case FM7_MAINIO_MMR_SEGMENT:
			if(mmr_extend) {
				mmr_segment = data & 0x07;
			} else {
				mmr_segment = data & 0x03;
			}
			break;
		default:
			if((addr >= FM7_MAINIO_MMR_BANK) && (addr < (FM7_MAINIO_MMR_BANK + 0x80))){
				mmr_map_data[addr - FM7_MAINIO_MMR_BANK] = (uint8_t)data;
				update_mmr_jumptable(addr - FM7_MAINIO_MMR_BANK);
			}
			break;
		}
		return;
	}
#endif
	write_data8_main(addr, data, false);
}

// Read / Write data(s) as big endian.
uint32_t FM7_MAINMEM::read_data16(uint32_t addr)
{
	uint32_t hi, lo;
	uint32_t val;
   
	hi = read_data8(addr) & 0xff;
	lo = read_data8(addr + 1) & 0xff;
   
	val = hi * 256 + lo;
	return val;
}

uint32_t FM7_MAINMEM::read_data32(uint32_t addr)
{
	uint32_t ah, a2, a3, al;
	uint32_t val;
   
	ah = read_data8(addr) & 0xff;
	a2 = read_data8(addr + 1) & 0xff;
	a3 = read_data8(addr + 2) & 0xff;
	al = read_data8(addr + 3) & 0xff;
   
	val = ah * (65536 * 256) + a2 * 65536 + a3 * 256 + al;
	return val;
}

void FM7_MAINMEM::write_data16(uint32_t addr, uint32_t data)
{
	uint32_t d = data;
   
	write_data8(addr + 1, d & 0xff);
	d = d / 256;
	write_data8(addr + 0, d & 0xff);
}

void FM7_MAINMEM::write_data32(uint32_t addr, uint32_t data)
{
	uint32_t d = data;
   
	write_data8(addr + 3, d & 0xff);
	d = d / 256;
	write_data8(addr + 2, d & 0xff);
	d = d / 256;
	write_data8(addr + 1, d & 0xff);
	d = d / 256;
	write_data8(addr + 0, d & 0xff);
}


void FM7_MAINMEM::update_config()
{
	//setclock(config.cpu_type);
}

#define STATE_VERSION 2
void FM7_MAINMEM::save_state(FILEIO *state_fio)
{
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);
	this->out_debug_log(_T("Save State: MAINMEM: id=%d ver=%d\n"), this_device_id, STATE_VERSION);

	// V1
	state_fio->FputBool(ioaccess_wait);
	state_fio->FputInt32_BE(waitfactor);
	state_fio->FputInt32_BE(waitcount);

	state_fio->FputBool(sub_halted);
	
	state_fio->FputBool(diag_load_basicrom);
	state_fio->FputBool(diag_load_bootrom_bas);
	state_fio->FputBool(diag_load_bootrom_dos);
	state_fio->FputBool(diag_load_bootrom_mmr);
	state_fio->Fwrite(fm7_mainmem_omote, sizeof(fm7_mainmem_omote), 1);
	state_fio->Fwrite(fm7_mainmem_ura, sizeof(fm7_mainmem_ura), 1);
	state_fio->Fwrite(fm7_mainmem_basicrom, sizeof(fm7_mainmem_basicrom), 1);
	state_fio->Fwrite(fm7_mainmem_bioswork, sizeof(fm7_mainmem_bioswork), 1);
	state_fio->Fwrite(fm7_mainmem_bootrom_vector, sizeof(fm7_mainmem_bootrom_vector), 1);
	state_fio->Fwrite(fm7_mainmem_reset_vector, sizeof(fm7_mainmem_reset_vector), 1);
	
	state_fio->Fwrite(fm7_mainmem_null, sizeof(fm7_mainmem_null), 1);
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	state_fio->Fwrite(fm7_bootram, sizeof(fm7_bootram), 1);
#endif	
#if !defined(_FM77AV_VARIANTS)
	int addr;
	for(addr = 0; addr < 4; addr++) state_fio->Fwrite(fm7_bootroms[addr], sizeof(0x200), 1);
#endif	
#ifdef _FM77AV_VARIANTS
	state_fio->FputBool(dictrom_connected);
	state_fio->FputBool(use_page2_extram);
	
	state_fio->FputBool(diag_load_initrom);
	state_fio->FputBool(diag_load_dictrom);
	state_fio->FputBool(diag_load_learndata);
	state_fio->Fwrite(fm7_mainmem_initrom, sizeof(fm7_mainmem_initrom), 1);
	state_fio->Fwrite(fm77av_hidden_bootmmr, sizeof(fm77av_hidden_bootmmr), 1);
	
	state_fio->Fwrite(fm7_mainmem_mmrbank_0, sizeof(fm7_mainmem_mmrbank_0), 1);
	state_fio->Fwrite(fm7_mainmem_mmrbank_2, sizeof(fm7_mainmem_mmrbank_2), 1);
	
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	state_fio->FputBool(diag_load_extrarom);
	state_fio->Fwrite(fm7_mainmem_extrarom, sizeof(fm7_mainmem_extrarom), 1);
# endif
# if defined(CAPABLE_DICTROM)
	state_fio->Fwrite(fm7_mainmem_dictrom, sizeof(fm7_mainmem_dictrom), 1);
	state_fio->Fwrite(fm7_mainmem_learndata, sizeof(fm7_mainmem_learndata), 1);
# endif
#endif
	
#ifdef HAS_MMR
	state_fio->FputBool(extram_connected);
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	 defined(_FM77_VARIANTS)
	int pages;
	state_fio->FputInt32_BE(extram_pages);
	pages = extram_pages;
#  if defined(_FM77_VARIANTS)
	if(pages > 3) pages = 3;
#  else
	if(pages > 12) pages = 12;
#  endif	
	if(pages > 0) state_fio->Fwrite(fm7_mainmem_extram, pages * 0x10000, 1);
#  if defined(_FM77_VARIANTS)
	state_fio->Fwrite(fm77_shadowram, sizeof(fm77_shadowram), 1);
#  endif
# endif
#endif
	
	{ // V2;
		state_fio->FputBool(is_basicrom);
		state_fio->FputBool(clockmode);
		state_fio->FputBool(basicrom_fd0f);
		state_fio->FputUint32_BE(bootmode);
#if defined(_FM77AV_VARIANTS)
		state_fio->FputUint32_BE(extcard_bank);
		state_fio->FputBool(extrom_bank);
		state_fio->FputBool(initiator_enabled);
		state_fio->FputBool(dictrom_enabled);
		state_fio->FputBool(dictram_enabled);
#endif
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
		state_fio->FputBool(boot_ram_write);
#endif		
#if defined(HAS_MMR)
		state_fio->FputBool(window_enabled);
		state_fio->FputBool(mmr_enabled);
		state_fio->FputBool(mmr_fast);
		state_fio->FputBool(mmr_extend);
		
		state_fio->FputUint16_BE(window_offset);
		state_fio->FputBool(window_fast);
		state_fio->FputBool(refresh_fast);
		state_fio->FputUint8(mmr_segment);
		state_fio->Fwrite(mmr_map_data, sizeof(mmr_map_data), 1);
#endif
	}
}

bool FM7_MAINMEM::load_state(FILEIO *state_fio)
{
	uint32_t version;
	version = state_fio->FgetUint32_BE();
	if(this_device_id != state_fio->FgetInt32_BE()) return false;
	this->out_debug_log(_T("Load State: MAINMEM: id=%d ver=%d\n"), this_device_id, version);
	if(version >= 1) {
		// V1
		ioaccess_wait = state_fio->FgetBool();
		waitfactor = state_fio->FgetInt32_BE();
		waitcount = state_fio->FgetInt32_BE();

		sub_halted = state_fio->FgetBool();
	
		diag_load_basicrom = state_fio->FgetBool();
		diag_load_bootrom_bas = state_fio->FgetBool();
		diag_load_bootrom_dos = state_fio->FgetBool();
		diag_load_bootrom_mmr = state_fio->FgetBool();
		
		state_fio->Fread(fm7_mainmem_omote, sizeof(fm7_mainmem_omote), 1);
		state_fio->Fread(fm7_mainmem_ura, sizeof(fm7_mainmem_ura), 1);
		state_fio->Fread(fm7_mainmem_basicrom, sizeof(fm7_mainmem_basicrom), 1);
		state_fio->Fread(fm7_mainmem_bioswork, sizeof(fm7_mainmem_bioswork), 1);
		state_fio->Fread(fm7_mainmem_bootrom_vector, sizeof(fm7_mainmem_bootrom_vector), 1);
		state_fio->Fread(fm7_mainmem_reset_vector, sizeof(fm7_mainmem_reset_vector), 1);
	
		state_fio->Fread(fm7_mainmem_null, sizeof(fm7_mainmem_null), 1);
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
		state_fio->Fread(fm7_bootram, sizeof(fm7_bootram), 1);
#endif	
#if !defined(_FM77AV_VARIANTS)
		int addr;
		for(addr = 0; addr < 4; addr++) state_fio->Fread(fm7_bootroms[addr], sizeof(0x200), 1);
#endif	
#ifdef _FM77AV_VARIANTS
		dictrom_connected = state_fio->FgetBool();
		use_page2_extram = state_fio->FgetBool();
	
		diag_load_initrom = state_fio->FgetBool();
		diag_load_dictrom = state_fio->FgetBool();
		diag_load_learndata = state_fio->FgetBool();
		state_fio->Fread(fm7_mainmem_initrom, sizeof(fm7_mainmem_initrom), 1);
		state_fio->Fread(fm77av_hidden_bootmmr, sizeof(fm77av_hidden_bootmmr), 1);
		
		state_fio->Fread(fm7_mainmem_mmrbank_0, sizeof(fm7_mainmem_mmrbank_0), 1);
		state_fio->Fread(fm7_mainmem_mmrbank_2, sizeof(fm7_mainmem_mmrbank_2), 1);
	
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		diag_load_extrarom = state_fio->FgetBool();
		state_fio->Fread(fm7_mainmem_extrarom, sizeof(fm7_mainmem_extrarom), 1);
# endif		
# if defined(CAPABLE_DICTROM)
		state_fio->Fread(fm7_mainmem_dictrom, sizeof(fm7_mainmem_dictrom), 1);
		state_fio->Fread(fm7_mainmem_learndata, sizeof(fm7_mainmem_learndata), 1);
# endif
#endif
	
#ifdef HAS_MMR
		extram_connected = state_fio->FgetBool();
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	 defined(_FM77_VARIANTS)
		int pages;
		extram_pages = state_fio->FgetInt32_BE();
		pages = extram_pages;
#  if defined(_FM77_VARIANTS)
		if(pages > 3) pages = 3;
#  else
		if(pages > 12) pages = 12;
#  endif	
		if(fm7_mainmem_extram != NULL) {
			free(fm7_mainmem_extram);
			fm7_mainmem_extram = NULL;
		}
		if(pages > 0) {
			fm7_mainmem_extram = (uint8_t *)malloc(pages * 0x10000);
			state_fio->Fread(fm7_mainmem_extram, pages * 0x10000, 1);
		}
#  if defined(_FM77_VARIANTS)
		state_fio->Fread(fm77_shadowram, sizeof(fm77_shadowram), 1);
#  endif
# endif
#endif
		if(version == 1) return true;
	}
	{ // V2;
		is_basicrom = state_fio->FgetBool();
		clockmode = state_fio->FgetBool();
		basicrom_fd0f = state_fio->FgetBool();
		bootmode = state_fio->FgetUint32_BE();
#if defined(_FM77AV_VARIANTS)
		extcard_bank = state_fio->FgetUint32_BE();
		extrom_bank = state_fio->FgetBool();
		initiator_enabled = state_fio->FgetBool();
		dictrom_enabled = state_fio->FgetBool();
		dictram_enabled = state_fio->FgetBool();
#endif
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
		boot_ram_write = state_fio->FgetBool();
#endif		
#if defined(HAS_MMR)
		window_enabled = state_fio->FgetBool();
		mmr_enabled = state_fio->FgetBool();
		mmr_fast = state_fio->FgetBool();
		mmr_extend = state_fio->FgetBool();
		
		window_offset = state_fio->FgetUint16_BE();
		window_fast = state_fio->FgetBool();
		refresh_fast = state_fio->FgetBool();
		mmr_segment = state_fio->FgetUint8();
		state_fio->Fread(mmr_map_data, sizeof(mmr_map_data), 1);
#endif
	}
	init_data_table();
	update_all_mmr_jumptable();
	if(version != STATE_VERSION) return false;
	return true;
}
