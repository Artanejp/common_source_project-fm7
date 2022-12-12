/*
 * Main memory MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *  License: GPLv2
 *
 */
#include "../vm.h"
#include "../../emu.h"
#include "fm7_mainmem.h"
#include "fm7_mainio.h"
#include "fm7_display.h"
#if defined(CAPABLE_DICTROM)
#include "kanjirom.h"
#endif
FM7_MAINMEM::FM7_MAINMEM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
#if !defined(_FM77AV_VARIANTS)
	for(int i = 0; i < 8; i++) fm7_bootroms[i] = (uint8_t *)malloc(0x200);
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
	cpu_clocks = CPU_CLOCKS;
	// Initialize table
	set_device_name(_T("MAIN MEMORY"));
}

FM7_MAINMEM::~FM7_MAINMEM()
{
#if !defined(_FM77AV_VARIANTS)
	for(int i = 0; i < 8; i++) if(fm7_bootroms[i] != NULL) free(fm7_bootroms[i]);
#endif
}

void FM7_MAINMEM::reset()
{
   	waitfactor = 0;
	waitcount = 0;
	mem_waitfactor = 0;
	mem_waitcount = 0;
	ioaccess_wait = false;
	sub_halted = (display->read_signal(SIG_DISPLAY_HALT) == 0) ? false : true;
	//sub_halted = false;
	memset(fm7_mainmem_bootrom_vector, 0x00, 0x10); // Clear without vector

#if defined(_FM77AV_VARIANTS)
	memset(fm7_bootram, 0x00, 0x1f0);
	initiator_enabled = true;
	boot_ram_write = true;
#elif defined(_FM77_VARIANTS)
	boot_ram_write = false;
#endif
#if defined(_FM77_VARIANTS) || defined(_FM8)
	bootmode = config.boot_mode & 7;
#else
	bootmode = config.boot_mode & 3;
#endif
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
#  if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		// Thanks to Ryu Takegami, around DRAM refresh.
		// DRAM refresh makes halting MAIN MPU per 13.02uS.
		if(!mmr_fast && !window_fast) { // SLOW
			if(refresh_fast) {
				clock = MAINCLOCK_FAST_MMR - ((100000000 / 1302) * 1);  // Fast Refresh: 1wait
			} else {
				clock = MAINCLOCK_FAST_MMR - ((100000000 / 1302) * 3);  // Slow Refresh: 3Wait(!)
			}				
			if(mmr_enabled || window_enabled) {
				clock = (uint32_t)((double)clock * 0.87);
			}					
		} else {
			clock = MAINCLOCK_FAST_MMR;
			//if(!(mmr_enabled) && !(window_enabled)) clock = MAINCLOCK_NORMAL;
		}
#  else
		if(mmr_enabled || window_enabled) {
			clock = MAINCLOCK_MMR;
		} else {
			clock = MAINCLOCK_NORMAL;
		}
#  endif
#else
		clock = MAINCLOCK_NORMAL;
#endif				
	}
	//mem_waitcount = 0;
	uint32_t before_waitfactor = mem_waitfactor;
	if(CPU_CLOCKS > clock) {
		mem_waitfactor = (uint32_t)(65536.0 * ((1.0 - (double)clock / (double)CPU_CLOCKS)));
		//out_debug_log(_T("CLOCK=%d WAIT FACTOR=%d"), clock, mem_waitfactor);
	} else {
		mem_waitfactor = 0;
		//out_debug_log(_T("CLOCK=%d WAIT FACTOR=%d"), clock, mem_waitfactor);
	}
	cpu_clocks = clock;
	// Below is ugly hack cause of CPU#0 cannot modify clock.
	if(before_waitfactor != mem_waitfactor) maincpu->write_signal(SIG_CPU_WAIT_FACTOR, mem_waitfactor, 0xffffffff);
}
		

void FM7_MAINMEM::iowait()
{
	int _waitfactor = 0;
	if(config.cpu_type == 1) return; // SLOW
#ifdef HAS_MMR
	if((window_enabled) || (mmr_enabled)) {
		if(!ioaccess_wait) {
			_waitfactor = 2;
		} else { // Not MMR, TWR or enabled FAST MMR mode
			_waitfactor = 3; // If(MMR or TWR) and NOT FAST MMR factor = 3, else factor = 2
			if(mmr_fast) _waitfactor = 2;
		}
	} else {
		_waitfactor = 2;
	}
#else
	_waitfactor = 2;
#endif	  
	if(_waitfactor <= 0) return;
	waitcount++;
	if(waitcount >= _waitfactor) {
		maincpu->set_extra_clock(1);
		waitcount = 0;
		ioaccess_wait = !ioaccess_wait;
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
	case FM7_MAINIO_MEM_REFRESH_FAST:
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
		case FM7_MAINIO_MEM_REFRESH_FAST:
			refresh_fast = flag;
			setclock(config.cpu_type);
			break;
#endif			
	}
}

uint32_t FM7_MAINMEM::read_io8(uint32_t addr)
{
	return mainio->read_io8(addr);
}

void FM7_MAINMEM::write_io8(uint32_t addr, uint32_t data)
{
	return mainio->write_io8(addr, data);
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
	write_data8_main(addr & 0xffff, data, true);
#else
	write_data8(addr & 0xffff, data);
#endif	
}

void FM7_MAINMEM::write_dma_io8(uint32_t addr, uint32_t data)
{
#if defined(HAS_MMR)
	write_data8_main(addr & 0xffff, data, true);
#else
	write_data8(addr & 0xffff, data);
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

uint32_t FM7_MAINMEM::read_data16w(uint32_t addr, int *wait)
{
	return read_data16(addr);
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

uint32_t FM7_MAINMEM::read_data32w(uint32_t addr, int *wait)
{
	return read_data32(addr);
}

void FM7_MAINMEM::write_data16(uint32_t addr, uint32_t data)
{
	uint32_t d = data;
   
	write_data8(addr + 1, d & 0xff);
	d = d / 256;
	write_data8(addr + 0, d & 0xff);
}

void FM7_MAINMEM::write_data16w(uint32_t addr, uint32_t data, int *wait)
{
	write_data16(addr, data);
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

void FM7_MAINMEM::write_data32w(uint32_t addr, uint32_t data, int *wait)
{
	write_data32(addr, data);
}


void FM7_MAINMEM::update_config()
{
	setclock(config.cpu_type);
}

#define STATE_VERSION 8

bool FM7_MAINMEM::decl_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	
	state_fio->StateValue(ioaccess_wait);
	state_fio->StateValue(waitfactor);
	state_fio->StateValue(waitcount);
	state_fio->StateValue(sub_halted);
	
	state_fio->StateValue(diag_load_basicrom);
	state_fio->StateValue(diag_load_bootrom_bas);
	state_fio->StateValue(diag_load_bootrom_dos);
	state_fio->StateValue(diag_load_bootrom_mmr);
	state_fio->StateValue(diag_load_bootrom_bubble);
	state_fio->StateValue(diag_load_bootrom_bubble_128k);
	state_fio->StateValue(diag_load_bootrom_sfd8);
	state_fio->StateValue(diag_load_bootrom_2hd);

	state_fio->StateArray(fm7_mainmem_omote, sizeof(fm7_mainmem_omote), 1);
	state_fio->StateArray(fm7_mainmem_ura, sizeof(fm7_mainmem_ura), 1);
	state_fio->StateArray(fm7_mainmem_basicrom, sizeof(fm7_mainmem_basicrom), 1);
	state_fio->StateArray(fm7_mainmem_bioswork, sizeof(fm7_mainmem_bioswork), 1);
	state_fio->StateArray(fm7_mainmem_bootrom_vector, sizeof(fm7_mainmem_bootrom_vector), 1);
	state_fio->StateArray(fm7_mainmem_reset_vector, sizeof(fm7_mainmem_reset_vector), 1);
	
	state_fio->StateArray(fm7_mainmem_null, sizeof(fm7_mainmem_null), 1);

#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	state_fio->StateArray(fm7_bootram, sizeof(fm7_bootram), 1);
#endif	
#if defined(_FM77_VARIANTS) || defined(_FM8)
	for(int i = 0; i < 8; i++) state_fio->StateArray(fm7_bootroms[i], 0x200, 1);
#elif defined(_FM7) || defined(_FMNEW7)
	for(int i = 0; i < 4; i++) state_fio->StateArray(fm7_bootroms[i], 0x200, 1);
#endif	

#if defined(_FM8)
	state_fio->StateValue(diag_load_sm11_14);
	state_fio->StateValue(diag_load_sm11_15);
#elif defined(_FM77_VARIANTS)
	state_fio->StateValue(diag_load_wb11_12);
#elif defined(_FM7) || defined(_FMNEW7)
	state_fio->StateValue(diag_load_tl11_11);
#  if defined(_FMNEW7)
	state_fio->StateValue(diag_load_tl11_12);
#  endif	
#elif defined(_FM77AV_VARIANTS)
	state_fio->StateValue(dictrom_connected);
	state_fio->StateValue(use_page2_extram);
	
	state_fio->StateValue(diag_load_initrom);
	state_fio->StateValue(diag_load_dictrom);
	state_fio->StateValue(diag_load_learndata);
	state_fio->StateArray(fm7_mainmem_initrom, sizeof(fm7_mainmem_initrom), 1);
	state_fio->StateArray(fm77av_hidden_bootmmr, sizeof(fm77av_hidden_bootmmr), 1);
	
	state_fio->StateArray(fm7_mainmem_mmrbank_0, sizeof(fm7_mainmem_mmrbank_0), 1);
	state_fio->StateArray(fm7_mainmem_mmrbank_2, sizeof(fm7_mainmem_mmrbank_2), 1);
	
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	state_fio->StateValue(diag_load_extrarom);
	state_fio->StateArray(fm7_mainmem_extrarom, sizeof(fm7_mainmem_extrarom), 1);
# endif
# if defined(CAPABLE_DICTROM)
	state_fio->StateArray(fm7_mainmem_dictrom, sizeof(fm7_mainmem_dictrom), 1);
	state_fio->StateArray(fm7_mainmem_learndata, sizeof(fm7_mainmem_learndata), 1);
# endif
#endif

#ifdef HAS_MMR
	state_fio->StateValue(extram_connected);
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	 defined(_FM77_VARIANTS)
	state_fio->StateValue(extram_pages);
	state_fio->StateArray(fm7_mainmem_extram, extram_size, 1);
#  if defined(_FM77_VARIANTS)
	state_fio->StateArray(fm77_shadowram, sizeof(fm77_shadowram), 1);
#  endif
# endif
#endif
							  
	{ // V2;
		state_fio->StateValue(is_basicrom);
		state_fio->StateValue(clockmode);
		state_fio->StateValue(basicrom_fd0f);
		state_fio->StateValue(bootmode);
#if defined(_FM77AV_VARIANTS)
		state_fio->StateValue(extcard_bank);
		state_fio->StateValue(extrom_bank);
		state_fio->StateValue(initiator_enabled);
		state_fio->StateValue(dictrom_enabled);
		state_fio->StateValue(dictram_enabled);
#endif
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
		state_fio->StateValue(boot_ram_write);
#endif		
#if defined(HAS_MMR)
		state_fio->StateValue(window_enabled);
		state_fio->StateValue(mmr_enabled);
		state_fio->StateValue(mmr_fast);
		state_fio->StateValue(mmr_extend);
		
		state_fio->StateValue(window_offset);
		state_fio->StateValue(window_fast);
		state_fio->StateValue(refresh_fast);
		state_fio->StateValue(mmr_segment);
		state_fio->StateArray(mmr_map_data, sizeof(mmr_map_data), 1);
#endif
	}
	state_fio->StateValue(mem_waitfactor); // OK?
	state_fio->StateValue(mem_waitcount); // OK?

	state_fio->StateValue(cpu_clocks); // OK?

	return true;
}

void FM7_MAINMEM::save_state(FILEIO *state_fio)
{
	decl_state(state_fio, false);
#if defined(HAS_MMR)
#  if defined(_FM77_VARIANTS)
	if(extram_pages > 3) extram_pages = 3;
#  elif defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) 
	if(extram_pages > 12) extram_pages = 12;
#  endif
#endif
	//extram_size = extram_pages * 0x10000;
}

bool FM7_MAINMEM::load_state(FILEIO *state_fio)
{
	bool mb = decl_state(state_fio, true);
	this->out_debug_log(_T("Load State: MAINIO: id=%d stat=%s\n"), this_device_id, (mb) ? _T("OK") : _T("NG"));
	if(!mb) return false;
	
#if defined(HAS_MMR)
#  if defined(_FM77_VARIANTS)
	if(extram_pages > 3) extram_pages = 3;
#  elif defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) 
	if(extram_pages > 12) extram_pages = 12;
#  endif
#endif
	//extram_size = extram_pages * 0x10000;
	init_data_table();
	update_all_mmr_jumptable();
	return true;
}
