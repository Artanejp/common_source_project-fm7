/*
	MICOM MAHJONG Emulator 'eMuCom Mahjong'

	Author : Hiroaki GOTO as GORRY
	Date   : 2020.07.20 -

	[ memory ]
*/


#include "./memory.h"

#define PRG_FILE_NAME	"PRG.ROM"
#define CG_FILE_NAME	"CG.ROM"

void MEMORY::initialize()
{
	memset(prg, 0xff, sizeof(prg));
	memset(ram, 0x00, sizeof(ram));
	memset(vram, 0x00, sizeof(vram));
	memset(cg, 0x00, sizeof(vram));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T(PRG_FILE_NAME)), FILEIO_READ_BINARY)) {
		fio->Fread(prg, sizeof(prg), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T(CG_FILE_NAME)), FILEIO_READ_BINARY)) {
		fio->Fread(cg, sizeof(cg), 1);
		fio->Fclose();
	}
	delete fio;

	register_vline_event(this);
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_KEYDATA) {
		key_data = data;
	}
}

void MEMORY::reset()
{
}

void MEMORY::event_vline(int v, int clock)
{
	// draw one line
	if(v < 200) {
		draw_line(v);
	}
}

void MEMORY::event_callback(int event_id, int err)
{
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if(addr < 0x5000) {
	} else if(addr < 0x5400) {
		ram[addr-0x5000] = data;
	} else if(addr < 0x6000) {
	} else if(addr < 0x6400) {
		vram[addr-0x6000] = data;
	} else if(addr == 0x7002) {
		d_keyboard->write_signal(SIG_KEYBOARD_STROBE, data, 0xff);
	} else if(addr == 0x7004) {
		speaker = data & 1;
		d_pcm->write_signal(SIG_KEYBOARD_STROBE, speaker, 0x01);
	}
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	uint32_t val = 0;
	
	addr &= 0xffff;
	if(addr < 0x4000) {
		return prg[addr];
	} else if(addr < 0x5000) {
		return 0xff;	// pull up ?
	} else if(addr < 0x5400) {
		return ram[addr-0x5000];
	} else if(addr < 0x6000) {
		return 0xff;	// pull up ?
	} else if(addr < 0x6400) {
		return ram[addr-0x6000];
	} else if(addr == 0x7001) {
		return key_data;
	}
	return 0xff;	// pull up ?
}

void MEMORY::draw_line(int v)
{
	int ptr = 32 * (v >> 3);
	
	for(int x = 0; x < 256; x += 8) {
		uint16_t code = vram[ptr] << 3;
		uint8_t pat_t = cg[code | (v & 7)];
		uint8_t* dest = &screen[v][x];
		
		dest[0] = (pat_t & 0x80) ? 1 : 0;
		dest[1] = (pat_t & 0x40) ? 1 : 0;
		dest[2] = (pat_t & 0x20) ? 1 : 0;
		dest[3] = (pat_t & 0x10) ? 1 : 0;
		dest[4] = (pat_t & 0x08) ? 1 : 0;
		dest[5] = (pat_t & 0x04) ? 1 : 0;
		dest[6] = (pat_t & 0x02) ? 1 : 0;
		dest[7] = (pat_t & 0x01) ? 1 : 0;
		ptr++;
	}
}

void MEMORY::draw_screen()
{
	if(emu->now_waiting_in_debugger) {
		// draw lines
		for(int v = 0; v < 200; v++) {
			draw_line(v);
		}
	}
	
	// copy to real screen
	emu->set_vm_screen_lines(200);
	for(int y = 0; y < 200; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(2 * y);
		scrntype_t* dest1 = emu->get_screen_buffer(2 * y + 1);
		uint8_t* src = screen[y];
		
		for(int x = 0, x2 = 0; x < 256; x++, x2 += 2) {
			const scrntype_t col[2] = {0, RGB_COLOR(255,255,255)};
			dest0[x2] = dest0[x2 + 1] = col[src[x]];
		}
		if(!config.scan_line) {
			my_memcpy(dest1, dest0, 512 * sizeof(scrntype_t));
		} else {
			memset(dest1, 0, 512 * sizeof(scrntype_t));
		}
	}
	emu->screen_skip_line(true);
}

#define STATE_VERSION	1

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateValue(key_strobe);
	state_fio->StateValue(key_data);
	state_fio->StateValue(speaker);
	return true;
}

