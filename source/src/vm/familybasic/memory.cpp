/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ memory ]
*/

#include "memory.h"
#include "ppu.h"
#include "../datarec.h"
#include "../ym2413.h"

#define EVENT_DMA_DONE	0

void MEMORY::initialize()
{
	key_stat = emu->get_key_buffer();
	joy_stat = emu->get_joy_buffer();
	
	// register event
	register_vline_event(this);
}

void MEMORY::load_rom_image(const _TCHAR *file_name)
{
	FILEIO* fio = new FILEIO();
	
	memset(save_ram, 0, sizeof(save_ram));
	
	if(fio->Fopen(create_local_path(file_name), FILEIO_READ_BINARY)) {
		// create save file name
		_TCHAR tmp_file_name[_MAX_PATH];
		my_tcscpy_s(tmp_file_name, _MAX_PATH, file_name);
		_TCHAR *dot = _tcsstr(tmp_file_name, _T("."));
		if(dot != NULL) dot[0] = _T('\0');
		my_stprintf_s(save_file_name, _MAX_PATH, _T("%s.SAV"), tmp_file_name);
	} else {
		// for compatibility
		fio->Fopen(create_local_path(_T("BASIC.NES")), FILEIO_READ_BINARY);
		my_tcscpy_s(save_file_name, _MAX_PATH, _T("BACKUP.BIN"));
	}
	if(fio->IsOpened()) {
		// read header
		fio->Fread(&header, sizeof(header), 1);
		// read program rom
		rom_size = 0x2000 * header.num_8k_rom_banks();
		for(uint32_t bit = 0x80000; bit != 0; bit >>= 1) {
			if(rom_size & bit) {
				if(rom_size & (bit - 1)) {
					rom_size = (rom_size | (bit - 1)) + 1;
				}
				break;
			}
		}
		rom = (uint8_t *)calloc(rom_size, 1);
		fio->Fread(rom, 0x2000 * header.num_8k_rom_banks(), 1);
		fio->Fclose();
	} else {
		memset(&header, 0, sizeof(header));
		header.dummy = 2; // 32KB
		rom_size = 0x2000 * header.num_8k_rom_banks();
		rom = (uint8_t *)calloc(rom_size, 1);
		memset(rom, 0xff, 0x2000 * header.num_8k_rom_banks());
	}
	if(fio->Fopen(create_local_path(save_file_name), FILEIO_READ_BINARY)) {
		fio->Fread(save_ram, sizeof(save_ram), 1);
		fio->Fclose();
	}
	delete fio;
	
	rom_mask = (rom_size / 0x2000) - 1;
	save_ram_crc32 = get_crc32(save_ram, sizeof(save_ram));
}

void MEMORY::save_backup()
{
	if(save_ram_crc32 != get_crc32(save_ram, sizeof(save_ram))) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(save_file_name), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(save_ram, sizeof(save_ram), 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void MEMORY::release()
{
	if(rom != NULL) {
		free(rom);
		rom = NULL;
	}
	save_backup();
}

void MEMORY::reset()
{
	memset(ram, 0, sizeof(ram));
	
	for(int i = 4; i < 8; i++) {
		set_rom_bank(i, i);
	}
	bank_ptr[3] = save_ram;
	
	dma_addr = 0x80;
	frame_irq_enabled = 0xff;
	
	pad_strobe = false;
	pad1_bits = pad2_bits = 0xff;
	
	kb_out = false;
	kb_scan = 0;
	
	mmc5_reset();
	vrc7_reset();
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	
	if(addr < 0x2000) {
		ram[addr & 0x7ff] = data;
	} else if(addr < 0x4000) {
		d_ppu->write_data8(addr, data);
	} else if(addr == 0x4014) {
		// stop cpu
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		register_event_by_clock(this, EVENT_DMA_DONE, 514, false, NULL);
		// start dma
		dma_addr = data << 8;
		for(int i = 0; i < 256; i++) {
			spr_ram[i] = read_data8(dma_addr | i);
		}
	} else if(addr == 0x4016) {
		if(data & 1) {
			pad_strobe = true;
		} else if(pad_strobe) {
			pad_strobe = false;
			// joypad #1
			pad1_bits = 0;
			if(joy_stat[0] & 0x10) pad1_bits |= 0x01;	// A
			if(joy_stat[0] & 0x20) pad1_bits |= 0x02;	// B
			if(joy_stat[0] & 0x40) pad1_bits |= 0x04;	// SEL
			if(joy_stat[0] & 0x80) pad1_bits |= 0x08;	// START
			if(joy_stat[0] & 0x01) pad1_bits |= 0x10;	// UP
			if(joy_stat[0] & 0x02) pad1_bits |= 0x20;	// DOWN
			if(joy_stat[0] & 0x04) pad1_bits |= 0x40;	// LEFT
			if(joy_stat[0] & 0x08) pad1_bits |= 0x80;	// RIGHT
			// joypad #2
			pad2_bits = 0;
			if(joy_stat[1] & 0x10) pad2_bits |= 0x01;	// A
			if(joy_stat[1] & 0x20) pad2_bits |= 0x02;	// B
			if(joy_stat[1] & 0x01) pad2_bits |= 0x10;	// UP
			if(joy_stat[1] & 0x02) pad2_bits |= 0x20;	// DOWN
			if(joy_stat[1] & 0x04) pad2_bits |= 0x40;	// LEFT
			if(joy_stat[1] & 0x08) pad2_bits |= 0x80;	// RIGHT
		}
		// keyboard
		if((data & 0x07) == 0x04) {
			if(++kb_scan > 9) {
				kb_scan = 0;
			}
			kb_out = !kb_out;
		} else if((data & 0x07) == 0x05) {
			kb_out = false;
			kb_scan = 0;
		} else if((data & 0x07) == 0x06) {
			kb_out = !kb_out;
		}
		// data recorder (thanks MESS)
		if((data & 0x07) == 0x07) {
			d_drec->write_signal(SIG_DATAREC_MIC, 1, 1);
		} else {
			d_drec->write_signal(SIG_DATAREC_MIC, 0, 0);
		}
	} else if(addr < 0x4018) {
		if(addr == 0x4017) {
			frame_irq_enabled = data;
		}
		d_apu->write_data8(addr, data);
	} else if(addr < 0x6000) {
		if(header.mapper() == 5) {
			mmc5_lo_write(addr, data);
//		} else if(header.mapper() == 85) {
//			vrc7_lo_write(addr, data);
		}
	} else if(addr < 0x8000) {
		if(header.mapper() == 5) {
			mmc5_save_write(addr, data);
//		} else if(header.mapper() == 85) {
//			vrc7_save_write(addr, data);
		} else {
			bank_ptr[3][addr & 0x1fff] = data;
		}
	} else {
		if(header.mapper() == 5) {
			mmc5_hi_write(addr, data);
		} else if(header.mapper() == 85) {
			vrc7_hi_write(addr, data);
		}
	}
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	
	if(addr < 0x2000) {
		return ram[addr & 0x7ff];
	} else if(addr < 0x4000) {
		return d_ppu->read_data8(addr);
	} else if(addr == 0x4014) {
		return dma_addr >> 8;
	} else if(addr < 0x4016) {
		uint32_t val = d_apu->read_data8(addr);
		if(addr == 0x4015 && !(frame_irq_enabled & 0xc0)) {
			val |= 0x40;
		}
		return val;
	} else if(addr == 0x4016) {
		// joypad #1
		uint32_t val = pad1_bits & 1;
		pad1_bits >>= 1;
		// data recorder
		val |= d_drec->read_signal(0) ? 2 : 0;
		// mic
		val |= key_stat[0x7b] ? 4 : 0;	// F12
		return val;
	} else if(addr == 0x4017) {
		// joypad #2
		uint32_t val = 0xfe | (pad2_bits & 1);
		pad2_bits >>= 1;
		// keyboard
		if(kb_out) {
			switch(kb_scan) {
			case 1:
				if(key_stat[0x77]) val &= ~0x02;	// F8
				if(key_stat[0x0d]) val &= ~0x04;	// RETURN
				if(key_stat[0xdb]) val &= ~0x08;	// [
				if(key_stat[0xdd]) val &= ~0x10;	// ]
				break;
			case 2:
				if(key_stat[0x76]) val &= ~0x02;	// F7
				if(key_stat[0xc0]) val &= ~0x04;	// @
				if(key_stat[0xba]) val &= ~0x08;	// :
				if(key_stat[0xbb]) val &= ~0x10;	// ;
				break;
			case 3:
				if(key_stat[0x75]) val &= ~0x02;	// F6
				if(key_stat[0x4f]) val &= ~0x04;	// O
				if(key_stat[0x4c]) val &= ~0x08;	// L
				if(key_stat[0x4b]) val &= ~0x10;	// K
				break;
			case 4:
				if(key_stat[0x74]) val &= ~0x02;	// F5
				if(key_stat[0x49]) val &= ~0x04;	// I
				if(key_stat[0x55]) val &= ~0x08;	// U
				if(key_stat[0x4a]) val &= ~0x10;	// J
				break;
			case 5:
				if(key_stat[0x73]) val &= ~0x02;	// F4
				if(key_stat[0x59]) val &= ~0x04;	// Y
				if(key_stat[0x47]) val &= ~0x08;	// G
				if(key_stat[0x48]) val &= ~0x10;	// H
				break;
			case 6:
				if(key_stat[0x72]) val &= ~0x02;	// F3
				if(key_stat[0x54]) val &= ~0x04;	// T
				if(key_stat[0x52]) val &= ~0x08;	// R
				if(key_stat[0x44]) val &= ~0x10;	// D
				break;
			case 7:
				if(key_stat[0x71]) val &= ~0x02;	// F2
				if(key_stat[0x57]) val &= ~0x04;	// W
				if(key_stat[0x53]) val &= ~0x08;	// S
				if(key_stat[0x41]) val &= ~0x10;	// A
				break;
			case 8:
				if(key_stat[0x70]) val &= ~0x02;	// F1
				if(key_stat[0x1b]) val &= ~0x04;	// ESC
				if(key_stat[0x51]) val &= ~0x08;	// Q
				if(key_stat[0x11]) val &= ~0x10;	// CTRL
				break;
			case 9:
				if(key_stat[0x24]) val &= ~0x02;	// CLS
				if(key_stat[0x26]) val &= ~0x04;	// UP
				if(key_stat[0x27]) val &= ~0x08;	// RIGHT
				if(key_stat[0x25]) val &= ~0x10;	// LEFT
				break;
			}
		} else {
			switch(kb_scan) {
			case 1:
				if(key_stat[0x15]) val &= ~0x02;	// KANA
//				if(key_stat[0x10]) val &= ~0x04;	// RSHIFT
				if(key_stat[0xdc]) val &= ~0x08;	// '\\'
				if(key_stat[0x23]) val &= ~0x10;	// STOP
				break;
			case 2:
				if(key_stat[0xe2]) val &= ~0x02;	// _
				if(key_stat[0xbf]) val &= ~0x04;	// /
				if(key_stat[0xbd]) val &= ~0x08;	// -
				if(key_stat[0xde]) val &= ~0x10;	// ^
				break;
			case 3:
				if(key_stat[0xbe]) val &= ~0x02;	// .
				if(key_stat[0xbc]) val &= ~0x04;	// ,
				if(key_stat[0x50]) val &= ~0x08;	// P
				if(key_stat[0x30]) val &= ~0x10;	// 0
				break;
			case 4:
				if(key_stat[0x4d]) val &= ~0x02;	// M
				if(key_stat[0x4e]) val &= ~0x04;	// N
				if(key_stat[0x39]) val &= ~0x08;	// 9
				if(key_stat[0x38]) val &= ~0x10;	// 8
				break;
			case 5:
				if(key_stat[0x42]) val &= ~0x02;	// B
				if(key_stat[0x56]) val &= ~0x04;	// V
				if(key_stat[0x37]) val &= ~0x08;	// 7
				if(key_stat[0x36]) val &= ~0x10;	// 6
				break;
			case 6:
				if(key_stat[0x46]) val &= ~0x02;	// F
				if(key_stat[0x43]) val &= ~0x04;	// C
				if(key_stat[0x35]) val &= ~0x08;	// 5
				if(key_stat[0x34]) val &= ~0x10;	// 4
				break;
			case 7:
				if(key_stat[0x58]) val &= ~0x02;	// X
				if(key_stat[0x5a]) val &= ~0x04;	// Z
				if(key_stat[0x45]) val &= ~0x08;	// E
				if(key_stat[0x33]) val &= ~0x10;	// 3
				break;
			case 8:
				if(key_stat[0x10]) val &= ~0x02;	// LSHIFT
				if(key_stat[0x12]) val &= ~0x04;	// GRAPH
				if(key_stat[0x31]) val &= ~0x08;	// 1
				if(key_stat[0x32]) val &= ~0x10;	// 2
				break;
			case 9:
				if(key_stat[0x28]) val &= ~0x02;	// DOWN
				if(key_stat[0x20]) val &= ~0x04;	// SPACE
				if(key_stat[0x2e]) val &= ~0x08;	// DEL
				if(key_stat[0x2d]) val &= ~0x10;	// INS
				break;
			}
		}
		return val;
	} else if(addr < 0x6000) {
		if(header.mapper() == 5) {
			return mmc5_lo_read(addr);
//		} else if(header.mapper() == 85) {
//			return vrc7_lo_read(addr);
		} else {
			return 0xff;
		}
	} else if(addr < 0x8000) {
//		if(header.mapper() == 5) {
//			return mmc5_save_read(addr);
//		} else if(header.mapper() == 85) {
//			return vrc7_save_read(addr);
//		} else {
			return bank_ptr[3][addr & 0x1fff];
//		}
	} else if(addr < 0xa000) {
		return bank_ptr[4][addr & 0x1fff];
	} else if(addr < 0xc000) {
		return bank_ptr[5][addr & 0x1fff];
	} else if(addr < 0xe000) {
		return bank_ptr[6][addr & 0x1fff];
	} else {
		return bank_ptr[7][addr & 0x1fff];
	}
}

void MEMORY::event_vline(int v, int clock)
{
	// 525 -> 262.5
	if(v & 1) {
		return;
	}
	v >>= 1;
	
	// fram irq
	if(v == 240 && !(frame_irq_enabled & 0xc0)) {
		// pending
		d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
	}
	
	// mapper irq
	if(header.mapper() == 5) {
		mmc5_hsync(v);
	} else if(header.mapper() == 85) {
		vrc7_hsync(v);
	}
}

void MEMORY::event_callback(int event_id, int err)
{
	// dma done
	d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
}

void MEMORY::set_rom_bank(uint8_t bank, uint32_t bank_num)
{
	bank_ptr[bank] = rom + 0x2000 * (bank_num & rom_mask);
	banks[bank] = bank_num;
	
	// mmc5
	mmc5_wram_bank[bank] = 8;
}

// mmc5

void MEMORY::mmc5_reset()
{
	if(header.mapper() == 5) {
		mmc5_set_wram_bank(3, 0);
		set_rom_bank(4, header.num_8k_rom_banks() - 1);
		set_rom_bank(5, header.num_8k_rom_banks() - 1);
		set_rom_bank(6, header.num_8k_rom_banks() - 1);
		set_rom_bank(7, header.num_8k_rom_banks() - 1);
		
		for(int i = 0; i < 8; i++) {
			mmc5_chr_reg[i][0] = i;
			mmc5_chr_reg[i][1] = (i & 0x03) + 4;
		}
		mmc5_wram_protect0 = 0x02;
		mmc5_wram_protect1 = 0x01;
		mmc5_prg_size = 3;
		mmc5_chr_size = 3;
		mmc5_gfx_mode = 0;
//		mmc5_split_control = 0;
//		mmc5_split_bank = 0;
		mmc5_irq_enabled = 0;
		mmc5_irq_status = 0;
		mmc5_irq_line = 0;
	}
}

uint32_t MEMORY::mmc5_lo_read(uint32_t addr)
{
	uint8_t data = (uint8_t)(addr >> 8);
	
	if(addr == 0x5204) {
		data = mmc5_irq_status;
		mmc5_irq_status &= ~0x80;
	} else if(addr == 0x5205) {
		data = (uint8_t)(((mmc5_value0 * mmc5_value1) & 0x00ff) >> 0);
	} else if(addr == 0x5206) {
		data = (uint8_t)(((mmc5_value0 * mmc5_value1) & 0xff00) >> 8);
	} else if(addr >= 0x5c00 && addr < 0x6000) {
		if(mmc5_gfx_mode == 2 || mmc5_gfx_mode == 3) {
			data = (d_ppu->get_name_tables() + 0x800)[addr & 0x3ff];
		}
	}
	return data;
}

void MEMORY::mmc5_lo_write(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x5100:
		mmc5_prg_size = data & 0x03;
		break;
	case 0x5101:
		mmc5_chr_size = data & 0x03;
		break;
	case 0x5102:
		mmc5_wram_protect0 = data & 0x03;
		break;
	case 0x5103:
		mmc5_wram_protect1 = data & 0x03;
		break;
	case 0x5104:
		mmc5_gfx_mode = data & 0x03;
		break;
	case 0x5105:
		for(int i = 0; i < 4; i++) {
			d_ppu->set_ppu_bank(8 + i, data & 0x03);
			data >>= 2;
		}
		break;
	case 0x5106:
		for(int i = 0; i < 0x3c0; i++) {
			(d_ppu->get_name_tables() + 0xc00)[i] = data;
		}
		break;
	case 0x5107:
		for(int i = 0x3c0; i < 0x400; i++) {
			(d_ppu->get_name_tables() + 0xc00)[i] = 0x55 * (data & 3);
		}
		break;
	case 0x5113:
		mmc5_set_wram_bank(3, data & 0x07);
		break;
	case 0x5114:
	case 0x5115:
	case 0x5116:
	case 0x5117:
		mmc5_set_cpu_bank(addr & 0x07, data);
		break;
	case 0x5120:
	case 0x5121:
	case 0x5122:
	case 0x5123:
	case 0x5124:
	case 0x5125:
	case 0x5126:
	case 0x5127:
		mmc5_chr_reg[addr & 0x07][0] = data;
		mmc5_set_ppu_bank(0);
		break;
	case 0x5128:
	case 0x5129:
	case 0x512a:
	case 0x512b:
		mmc5_chr_reg[(addr & 0x03) + 0][1] = data;
		mmc5_chr_reg[(addr & 0x03) + 4][1] = data;
		break;
	case 0x5200:
//		mmc5_split_control = data;
		break;
	case 0x5201:
//		mmc5_split_scroll = data;
		break;
	case 0x5202:
//		mmc5_split_bank = data & 0x3f;
		break;
	case 0x5203:
		mmc5_irq_line = data;
		break;
	case 0x5204:
		mmc5_irq_enabled = data;
		break;
	case 0x5205:
		mmc5_value0 = data;
		break;
	case 0x5206:
		mmc5_value1 = data;
		break;
	default:
		if(addr >= 0x5000 && addr <= 0x5015) {
//			d_mmc5->write_io8(addr, data);
		} else if(addr >= 0x5c00 && addr <= 0x5fff) {
			if(mmc5_gfx_mode != 3) {
				(d_ppu->get_name_tables() + 0x800)[addr & 0x3ff] = data; //(mmc5_irq_status & 0) ? data : 0x40;
			}
		}
		break;
	}
}

//uint32_t MEMORY::mmc5_save_read(uint32_t addr)
//{
//	return bank_ptr[3][addr & 0x1fff];
//}

void MEMORY::mmc5_save_write(uint32_t addr, uint32_t data)
{
	if(mmc5_wram_protect0 == 0x02 && mmc5_wram_protect1 == 0x01) {
		if(mmc5_wram_bank[3] < 8) {
			bank_ptr[3][addr & 0x1fff] = data;
		}
	}
}

void MEMORY::mmc5_hi_write(uint32_t addr, uint32_t data)
{
	if(mmc5_wram_protect0 == 0x02 && mmc5_wram_protect1 == 0x01) {
		if(addr < 0xa000) {
			if(mmc5_wram_bank[4] < 8) {
				bank_ptr[4][addr & 0x1fff] = data;
			}
		} else if(addr < 0xc000) {
			if(mmc5_wram_bank[5] < 8) {
				bank_ptr[5][addr & 0x1fff] = data;
			}
		} else if(addr < 0xe000) {
			if(mmc5_wram_bank[6] < 8) {
				bank_ptr[6][addr & 0x1fff] = data;
			}
		} else {
			if(mmc5_wram_bank[7] < 8) {
				bank_ptr[7][addr & 0x1fff] = data;
			}
		}
	}
}

void MEMORY::mmc5_hsync(int v)
{
	if(v <= 240) {
		if(v == mmc5_irq_line) {
			if(d_ppu->spr_enabled() && d_ppu->bg_enabled()) {
				mmc5_irq_status |= 0x80;
			}
		}
		if((mmc5_irq_status & 0x80) && (mmc5_irq_enabled & 0x80)) {
			d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
		}
	} else {
		mmc5_irq_status |= 0x40;
	}
}

void MEMORY::mmc5_set_cpu_bank(uint8_t bank, uint32_t bank_num)
{
	if(bank_num & 0x80) {
		if(mmc5_prg_size == 0) {
			if(bank == 7) {
				set_rom_bank(4, (bank_num & 0x7c) + 0);
				set_rom_bank(5, (bank_num & 0x7c) + 1);
				set_rom_bank(6, (bank_num & 0x7c) + 2);
				set_rom_bank(7, (bank_num & 0x7c) + 3);
			}
		} else if(mmc5_prg_size == 1) {
			if(bank == 5) {
				set_rom_bank(4, (bank_num & 0x7e) + 0);
				set_rom_bank(5, (bank_num & 0x7e) + 1);
			} else if(bank == 7) {
				set_rom_bank(6, (bank_num & 0x7e) + 0);
				set_rom_bank(7, (bank_num & 0x7e) + 1);
			}
		} else if(mmc5_prg_size == 2) {
			if(bank == 5) {
				set_rom_bank(4, (bank_num & 0x7e) + 0);
				set_rom_bank(5, (bank_num & 0x7e) + 1);
			} else if(bank == 6) {
				set_rom_bank(6, bank_num & 0x7f);
			} else if(bank == 7) {
				set_rom_bank(7, bank_num & 0x7f);
			}
		} else if(mmc5_prg_size == 3) {
			if(bank == 4) {
				set_rom_bank(4, bank_num & 0x7f);
			} else if(bank == 5) {
				set_rom_bank(5, bank_num & 0x7f);
			} else if(bank == 6) {
				set_rom_bank(6, bank_num & 0x7f);
			} else if(bank == 7) {
				set_rom_bank(7, bank_num & 0x7f);
			}
		}
	} else {
		if(mmc5_prg_size == 1) {
			if(bank == 5) {
				mmc5_set_wram_bank(4, (bank_num & 0x06)+0);
				mmc5_set_wram_bank(5, (bank_num & 0x06)+1);
			}
		} else if(mmc5_prg_size == 2) {
			if(bank == 5) {
				mmc5_set_wram_bank(4, (bank_num & 0x06)+0);
				mmc5_set_wram_bank(5, (bank_num & 0x06)+1);
			} else  if(bank == 6)
			{
				mmc5_set_wram_bank(6, bank_num & 0x07);
			}
		} else if(mmc5_prg_size == 3) {
			if(bank == 4) {
				mmc5_set_wram_bank(4, bank_num & 0x07);
			} else if(bank == 5) {
				mmc5_set_wram_bank(5, bank_num & 0x07);
			} else if(bank == 6) {
				mmc5_set_wram_bank(6, bank_num & 0x07);
			}
		}
	}
}

void MEMORY::mmc5_set_wram_bank(uint8_t bank, uint32_t bank_num)
{
	if(bank_num < 8) {
		bank_ptr[bank] = save_ram + 0x2000 * bank_num;
		mmc5_wram_bank[bank] = bank_num;
	} else {
		set_rom_bank(bank, banks[bank]);
	}
}

void MEMORY::mmc5_set_ppu_bank(uint8_t mode)
{
	if(mmc5_chr_size == 0) {
		d_ppu->set_ppu_bank(0, mmc5_chr_reg[7][mode] * 8 + 0);
		d_ppu->set_ppu_bank(1, mmc5_chr_reg[7][mode] * 8 + 1);
		d_ppu->set_ppu_bank(2, mmc5_chr_reg[7][mode] * 8 + 2);
		d_ppu->set_ppu_bank(3, mmc5_chr_reg[7][mode] * 8 + 3);
		d_ppu->set_ppu_bank(4, mmc5_chr_reg[7][mode] * 8 + 4);
		d_ppu->set_ppu_bank(5, mmc5_chr_reg[7][mode] * 8 + 5);
		d_ppu->set_ppu_bank(6, mmc5_chr_reg[7][mode] * 8 + 6);
		d_ppu->set_ppu_bank(7, mmc5_chr_reg[7][mode] * 8 + 7);
	} else if(mmc5_chr_size == 1) {
		d_ppu->set_ppu_bank(0, mmc5_chr_reg[3][mode] * 4 + 0);
		d_ppu->set_ppu_bank(1, mmc5_chr_reg[3][mode] * 4 + 1);
		d_ppu->set_ppu_bank(2, mmc5_chr_reg[3][mode] * 4 + 2);
		d_ppu->set_ppu_bank(3, mmc5_chr_reg[3][mode] * 4 + 3);
		d_ppu->set_ppu_bank(4, mmc5_chr_reg[7][mode] * 4 + 0);
		d_ppu->set_ppu_bank(5, mmc5_chr_reg[7][mode] * 4 + 1);
		d_ppu->set_ppu_bank(6, mmc5_chr_reg[7][mode] * 4 + 2);
		d_ppu->set_ppu_bank(7, mmc5_chr_reg[7][mode] * 4 + 3);
	} else if(mmc5_chr_size == 2) {
		d_ppu->set_ppu_bank(0, mmc5_chr_reg[1][mode] * 2 + 0);
		d_ppu->set_ppu_bank(1, mmc5_chr_reg[1][mode] * 2 + 1);
		d_ppu->set_ppu_bank(2, mmc5_chr_reg[3][mode] * 2 + 0);
		d_ppu->set_ppu_bank(3, mmc5_chr_reg[3][mode] * 2 + 1);
		d_ppu->set_ppu_bank(4, mmc5_chr_reg[5][mode] * 2 + 0);
		d_ppu->set_ppu_bank(5, mmc5_chr_reg[5][mode] * 2 + 1);
		d_ppu->set_ppu_bank(6, mmc5_chr_reg[7][mode] * 2 + 0);
		d_ppu->set_ppu_bank(7, mmc5_chr_reg[7][mode] * 2 + 1);
	} else {
		d_ppu->set_ppu_bank(0, mmc5_chr_reg[0][mode]);
		d_ppu->set_ppu_bank(1, mmc5_chr_reg[1][mode]);
		d_ppu->set_ppu_bank(2, mmc5_chr_reg[2][mode]);
		d_ppu->set_ppu_bank(3, mmc5_chr_reg[3][mode]);
		d_ppu->set_ppu_bank(4, mmc5_chr_reg[4][mode]);
		d_ppu->set_ppu_bank(5, mmc5_chr_reg[5][mode]);
		d_ppu->set_ppu_bank(6, mmc5_chr_reg[6][mode]);
		d_ppu->set_ppu_bank(7, mmc5_chr_reg[7][mode]);
	}
}

uint8_t MEMORY::mmc5_ppu_latch_render(uint8_t mode, uint32_t addr)
{
	uint8_t data = 0;
	
	if(mmc5_gfx_mode == 1 && mode == 1) {
		// ex gfx mode
		uint32_t bank_num = ((d_ppu->get_name_tables() + 0x800)[addr] & 0x3f) << 2;
		d_ppu->set_ppu_bank(0, bank_num + 0);
		d_ppu->set_ppu_bank(1, bank_num + 1);
		d_ppu->set_ppu_bank(2, bank_num + 2);
		d_ppu->set_ppu_bank(3, bank_num + 3);
		d_ppu->set_ppu_bank(4, bank_num + 0);
		d_ppu->set_ppu_bank(5, bank_num + 1);
		d_ppu->set_ppu_bank(6, bank_num + 2);
		d_ppu->set_ppu_bank(7, bank_num + 3);
		data = (((d_ppu->get_name_tables() + 0x800)[addr] & 0xc0) >> 4) | 0x01;
	} else {
		// normal
		mmc5_set_ppu_bank(mode);
	}
	return data;
}

// vrc7

void MEMORY::vrc7_reset()
{
	if(header.mapper() == 85) {
		vrc7_irq_enabled = 0;
		vrc7_irq_counter = 0;
		vrc7_irq_latch = 0;
		d_opll->write_signal(SIG_YM2413_MUTE, 0, 0);
		set_rom_bank(4, 0);
		set_rom_bank(5, 1);
		set_rom_bank(6, header.num_8k_rom_banks() - 2);
		set_rom_bank(7, header.num_8k_rom_banks() - 1);
	} else {
		d_opll->write_signal(SIG_YM2413_MUTE, 1, 1);
	}
}

//uint32_t MEMORY::vrc7_lo_read(uint32_t addr)
//{
//	return 0xff;
//}

//void MEMORY::vrc7_lo_write(uint32_t addr, uint32_t data)
//{
//}

//uint32_t MEMORY::vrc7_save_read(uint32_t addr)
//{
//	return bank_ptr[3][addr & 0x1fff];
//}

//void MEMORY::vrc7_save_write(uint32_t addr, uint32_t data)
//{
//	bank_ptr[3][addr & 0x1fff] = data;
//}

void MEMORY::vrc7_hi_write(uint32_t addr, uint32_t data)
{
	switch(addr & 0xf038) {
	case 0x8000:
		set_rom_bank(4, data);
		break;
	case 0x8008:
	case 0x8010:
		set_rom_bank(5, data);
		break;
	case 0x9000:
		set_rom_bank(6, data);
		break;
	case 0x9010:
	case 0x9030:
		d_opll->write_io8(addr >> 5, data);
		break;
	case 0xa000:
		d_ppu->set_ppu_bank(0, data);
		break;
	case 0xa008:
	case 0xa010:
		d_ppu->set_ppu_bank(1, data);
		break;
	case 0xb000:
		d_ppu->set_ppu_bank(2, data);
		break;
	case 0xb008:
	case 0xb010:
		d_ppu->set_ppu_bank(3, data);
		break;
	case 0xc000:
		d_ppu->set_ppu_bank(4, data);
		break;
	case 0xc008:
	case 0xc010:
		d_ppu->set_ppu_bank(5, data);
		break;
	case 0xd000:
		d_ppu->set_ppu_bank(6, data);
		break;
	case 0xd008:
	case 0xd010:
		d_ppu->set_ppu_bank(7, data);
		break;
	case 0xe000:
		switch(data & 0x03) {
		case 0x00:
			d_ppu->set_mirroring(MIRROR_VERT);
			break;
		case 0x01:
			d_ppu->set_mirroring(MIRROR_HORIZ);
			break;
		case 0x02:
			d_ppu->set_mirroring(0, 0, 0, 0);
			break;
		case 0x03:
			d_ppu->set_mirroring(1, 1, 1, 1);
			break;
		}
		break;
	case 0xe008:
	case 0xe010:
		vrc7_irq_latch = data;
		break;
	case 0xf000:
		if(data & 0x02) {
			vrc7_irq_counter = vrc7_irq_latch;
		}
		vrc7_irq_enabled = data & 0x03;
		break;
	case 0xf008:
	case 0xf010:
		vrc7_irq_enabled = (vrc7_irq_enabled & 0x01) * 3;
		break;
	}
}

void MEMORY::vrc7_hsync(int v)
{
	if(vrc7_irq_enabled & 0x02) {
		if(vrc7_irq_counter == 0xff) {
			d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
			vrc7_irq_counter = vrc7_irq_latch;
		} else {
			vrc7_irq_counter++;
		}
	}
}

#define STATE_VERSION	3

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(save_file_name, sizeof(save_file_name), 1);
	state_fio->StateArray(header.id, sizeof(header.id), 1);
	state_fio->StateValue(header.ctrl_z);
	state_fio->StateValue(header.dummy);
	state_fio->StateValue(header.num_8k_vrom_banks);
	state_fio->StateValue(header.flags_1);
	state_fio->StateValue(header.flags_2);
	state_fio->StateArray(header.reserved, sizeof(header.reserved), 1);
	state_fio->StateValue(rom_size);
//	state_fio->StateValue(rom_mask);
	if(loading) {
		rom_mask = (rom_size / 0x2000) - 1;
		if(rom != NULL) {
			free(rom);
		}
		rom = (uint8_t *)malloc(rom_size);
	}
	state_fio->StateArray(rom, rom_size, 1);
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(save_ram, sizeof(save_ram), 1);
	state_fio->StateValue(save_ram_crc32);
	state_fio->StateArray(banks, sizeof(banks), 1);
	state_fio->StateValue(dma_addr);
	state_fio->StateValue(frame_irq_enabled);
	state_fio->StateArray(mmc5_wram_bank, sizeof(mmc5_wram_bank), 1);
	state_fio->StateArray(&mmc5_chr_reg[0][0], sizeof(mmc5_chr_reg), 1);
	state_fio->StateValue(mmc5_value0);
	state_fio->StateValue(mmc5_value0);
	state_fio->StateValue(mmc5_wram_protect0);
	state_fio->StateValue(mmc5_wram_protect1);
	state_fio->StateValue(mmc5_prg_size);
	state_fio->StateValue(mmc5_chr_size);
	state_fio->StateValue(mmc5_gfx_mode);
//	state_fio->StateValue(mmc5_split_control);
//	state_fio->StateValue(mmc5_split_bank);
	state_fio->StateValue(mmc5_irq_enabled);
	state_fio->StateValue(mmc5_irq_status);
	state_fio->StateValue(mmc5_irq_line);
	state_fio->StateValue(vrc7_irq_enabled);
	state_fio->StateValue(vrc7_irq_counter);
	state_fio->StateValue(vrc7_irq_latch);
	state_fio->StateValue(pad_strobe);
	state_fio->StateValue(pad1_bits);
	state_fio->StateValue(pad2_bits);
	state_fio->StateValue(kb_out);
	state_fio->StateValue(kb_scan);
	
	// post process
	if(loading) {
		if(header.mapper() == 5) {
			for(int i = 3; i < 8; i++) {
				mmc5_set_wram_bank(i, mmc5_wram_bank[i]);
			}
		} else {
			for(int i = 4; i < 8; i++) {
				set_rom_bank(i, banks[i]);
			}
			bank_ptr[3] = save_ram;
		}
	}
	return true;
}

