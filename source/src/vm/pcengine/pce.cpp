/*
	NEC-HE PC Engine Emulator 'ePCEngine'
	SHARP X1twin Emulator 'eX1twin'

	Origin : Ootake (joypad/cdrom)
	       : xpce (psg)
	       : MESS (vdc/vce/vpc/cdrom)
	Author : Takeda.Toshiya
	Date   : 2009.03.11-

	[ PC-Engine ]
*/

#include <math.h>
#include "pce.h"
#include "../huc6280.h"
#ifdef SUPPORT_CDROM
#include "../msm5205.h"
#include "../scsi_host.h"
#include "../scsi_cdrom.h"
#endif

#define STATE_VSW		0
#define STATE_VDS		1
#define STATE_VDW		2
#define STATE_VCR		3

/* Bits in the VDC status register */

#define VDC_BSY         0x40    /* Set when the VDC accesses VRAM */
#define VDC_VD          0x20    /* Set when in the vertical blanking period */
#define VDC_DV          0x10    /* Set when a VRAM > VRAM DMA transfer is done */
#define VDC_DS          0x08    /* Set when a VRAM > SATB DMA transfer is done */
#define VDC_RR          0x04    /* Set when the current scanline equals the RCR register */
#define VDC_OR          0x02    /* Set when there are more than 16 sprites on a line */
#define VDC_CR          0x01    /* Set when sprite #0 overlaps with another sprite */

/* Bits in the CR register */

#define CR_BB           0x80    /* Background blanking */
#define CR_SB           0x40    /* Object blanking */
#define CR_VR           0x08    /* Interrupt on vertical blank enable */
#define CR_RC           0x04    /* Interrupt on line compare enable */
#define CR_OV           0x02    /* Interrupt on sprite overflow enable */
#define CR_CC           0x01    /* Interrupt on sprite #0 collision enable */

/* Bits in the DCR regsiter */

#define DCR_DSR         0x10    /* VRAM > SATB auto-transfer enable */
#define DCR_DID         0x08    /* Destination diretion */
#define DCR_SID         0x04    /* Source direction */
#define DCR_DVC         0x02    /* VRAM > VRAM EOT interrupt enable */
#define DCR_DSC         0x01    /* VRAM > SATB EOT interrupt enable */

/* just to keep things simple... */
enum vdc_regs {MAWR = 0, MARR, VxR, reg3, reg4, CR, RCR, BXR, BYR, MWR, HSR, HDR, VPR, VDW, VCR, DCR, SOUR, DESR, LENR, DVSSR };

enum line_state
{
	CLEAR_LINE = 0,				// clear (a fired or held) line
	ASSERT_LINE,				// assert an interrupt immediately
	HOLD_LINE,				// hold interrupt line until acknowledged
	PULSE_LINE				// pulse interrupt line instantaneously (only for NMI, RESET)
};

enum
{
	INPUT_LINE_IRQ1 = 0,
	INPUT_LINE_IRQ2 = 1,
	INPUT_LINE_TIRQ = 2,
	INPUT_LINE_NMI
};

#ifndef SUPPORT_CDROM
#define backup_locked	false
#endif

void PCE::initialize()
{
	// get context
	joy_stat = emu->get_joy_buffer();
	
	// register event
	register_vline_event(this);
	
#ifdef SUPPORT_BACKUP_RAM
	static const uint8_t image[8] = {0x48, 0x55, 0x42, 0x4d, 0x00, 0x88, 0x10, 0x80};
	memset(backup, 0, sizeof(backup));
	memcpy(backup, image, sizeof(image));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("BACKUP.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(backup, sizeof(backup), 1);
		fio->Fclose();
	}
	delete fio;
	
	backup_crc32 = get_crc32(backup, sizeof(backup));
#endif
#ifdef SUPPORT_CDROM
	cdrom_initialize();
#endif
	inserted = false;
}

void PCE::release()
{
#ifdef SUPPORT_BACKUP_RAM
	if(backup_crc32 != get_crc32(backup, sizeof(backup))) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("BACKUP.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(backup, sizeof(backup), 1);
			fio->Fclose();
		}
		delete fio;
	}
#endif
}

void PCE::reset()
{
	// reset memory bus
	memset(ram, 0, sizeof(ram));
	bank = 0x80000;
	buffer = 0xff;	// ???
	
	// reset devices
	vdc_reset();
	psg_reset();
	joy_reset();
#ifdef SUPPORT_CDROM
	cdrom_reset();
#endif
	
	prev_width = -1;
}

void PCE::event_vline(int v, int clock)
{
#ifdef SUPPORT_SUPER_GFX
	if(support_sgfx) {
		sgx_interrupt();
	} else
#endif
	pce_interrupt();
}

void PCE::write_data8(uint32_t addr, uint32_t data)
{
	uint8_t mpr = (addr >> 13) & 0xff;
	uint16_t ofs = addr & 0x1fff;
	
#ifdef SUPPORT_CDROM
	if(support_cdrom && mpr >= 0x68 && mpr <= 0x87) {
		cdrom_ram[addr & 0x3ffff] = data;
		return;
	}
#endif
	switch(mpr) {
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
		// populous
		cart[addr & 0xfffff] = data;
		return;
#ifdef SUPPORT_BACKUP_RAM
	case 0xf7:
		if(!backup_locked) {
			backup[ofs] = data;
		}
		return;
#endif
	case 0xf8:
		ram[ofs] = data;
		return;
#ifdef SUPPORT_SUPER_GFX
	case 0xf9:
	case 0xfa:
	case 0xfb:
		if(support_sgfx) {
			ram[addr & 0x7fff] = data;
		}
		return;
#endif
	case 0xff:
		switch(addr & 0x1c00) {
		case 0x0000:	// vdc
#ifdef SUPPORT_SUPER_GFX
			if(support_sgfx) {
				switch(addr & 0x18) {
				case 0x00:
					vdc_w(0, addr, data);
					break;
				case 0x08:
					vpc_w(addr, data);
					break;
				case 0x10:
					vdc_w(1, addr, data);
					break;
				}
			} else
#endif
			vdc_w(0, addr, data);
			break;
		case 0x0400:	// vce
			vce_w(addr, data);
			break;
		case 0x0800:	// psg
			buffer = data;
			psg_write(addr, data);
			break;
		case 0x0c00:	// timer
			buffer = data;
			d_cpu->timer_w(addr, data);
			break;
		case 0x1000:	// joypad
			buffer = data;
			joy_write(addr, data);
			break;
		case 0x1400:	// interrupt control
			buffer = data;
			d_cpu->irq_status_w(addr, data);
			break;
#ifdef SUPPORT_CDROM
		case 0x1800:
			if(support_cdrom && (addr & 0x1e00) != 0x1a00) {
				cdrom_write(addr, data);
				break;
			}
#endif
		}
		return;
	}
	// bank switch for sf2d
	if((addr & 0x1ffc) == 0x1ff0) {
		bank = 0x80000 * ((addr & 3) + 1);
	}
}

uint32_t PCE::read_data8(uint32_t addr)
{
	uint8_t mpr = (addr >> 13) & 0xff;
	uint16_t ofs = addr & 0x1fff;
	
	if(mpr <= 0x3f) {
		return cart[addr & 0x7ffff];
	}
#ifdef SUPPORT_CDROM
	if(support_cdrom && mpr >= 0x68 && mpr <= 0x87) {
		return cdrom_ram[addr & 0x3ffff];
	}
#endif
	if(mpr <= 0x7f) {
		return cart[bank | (addr & 0x7ffff)];
	}
	switch(mpr) {
#ifdef SUPPORT_BACKUP_RAM
	case 0xf7:
		return backup[ofs];
#endif
	case 0xf8:
		return ram[ofs];
#ifdef SUPPORT_SUPER_GFX
	case 0xf9:
	case 0xfa:
	case 0xfb:
		if(support_sgfx) {
			return ram[addr & 0x7fff];
		}
		return 0xff;
#endif
	case 0xff:
		switch (addr & 0x1c00) {
		case 0x0000: // vdc
#ifdef SUPPORT_SUPER_GFX
			if(support_sgfx) {
				switch(addr & 0x18) {
				case 0x00:
					return vdc_r(0, addr);
				case 0x08:
					return vpc_r(addr);
				case 0x10:
					return vdc_r(1, addr);
				}
				return 0xff;
			} else
#endif
			return vdc_r(0, addr);
		case 0x0400: // vce
			return vce_r(addr);
		case 0x0800: // psg
//			return psg_read(addr);
			return buffer;
		case 0x0c00: // timer
			buffer = (buffer & 0x80) | (d_cpu->timer_r(addr) & 0x7f);
			return buffer;
		case 0x1000: // joypad
			buffer = (buffer & 0xb0) | (joy_read(addr) & 0x0f);
			return buffer;
		case 0x1400: // interrupt control
			if(addr & 2) {
				buffer = (buffer & 0xf8) | (d_cpu->irq_status_r(addr) & 0x07);
			}
			return buffer;
#ifdef SUPPORT_CDROM
		case 0x1800:
			if(support_cdrom && (addr & 0x1e00) != 0x1a00) {
				return cdrom_read(addr);
			}
#endif
		}
		break;
	}
	return 0xff;
}

void PCE::write_io8(uint32_t addr, uint32_t data)
{
#ifdef SUPPORT_SUPER_GFX
	if(support_sgfx) {
		sgx_vdc_w(addr, data);
	} else
#endif
	vdc_w(0, addr, data);
}

uint32_t PCE::read_io8(uint32_t addr)
{
#ifdef SUPPORT_SUPER_GFX
	if(support_sgfx) {
		return sgx_vdc_r(addr);
	} else
#endif
	return vdc_r(0, addr);
}

void PCE::draw_screen()
{
	if(emu->now_waiting_in_debugger) {
		// store regs
		vdc_t tmp_vdc_0 = vdc[0];
#ifdef SUPPORT_SUPER_GFX
		vdc_t tmp_vdc_1 = vdc[1];
#endif
		int tmp_line = vce.current_bitmap_line;
		
		// drive vlines
		for(int v = /*get_cur_vline() + 1*/0; v < get_lines_per_frame(); v++) {
			event_vline(v, 0);
		}
		
		// restore regs
		vdc[0] = tmp_vdc_0;
#ifdef SUPPORT_SUPER_GFX
		vdc[1] = tmp_vdc_1;
#endif
		vce.current_bitmap_line = tmp_line;
	}
	
	int dx = (SCREEN_WIDTH - vdc[0].physical_width) / 2, sx = 0;
	int dy = (SCREEN_HEIGHT - 240) / 2;
	
	if(dx < 0) {
		sx = -dx;
		dx = 0;
	}
#ifndef _X1TWIN
	if(prev_width != vdc[0].physical_width) {
		for(int y = 0; y < SCREEN_HEIGHT; y++) {
			memset(emu->get_screen_buffer(y), 0, sizeof(scrntype_t) * SCREEN_WIDTH);
		}
		prev_width = vdc[0].physical_width;
	}
	emu->set_vm_screen_lines(240);
#endif
	for(int y = 0; y < 240; y++, dy++) {
		scrntype_t* src = &vce.bmp[y + 17][86];
		scrntype_t* dst = emu->get_screen_buffer(dy);
		for(int x = sx, x2 = dx; x < vdc[0].physical_width && x2 < SCREEN_WIDTH; x++, x2++) {
			dst[x2] = src[x];
		}
	}
}

void PCE::open_cart(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	
	support_6btn_pad = ((config.joystick_type & 1) != 0);
	support_multi_tap = ((config.joystick_type & 2) != 0);
#ifdef SUPPORT_SUPER_GFX
	support_sgfx = false;
#endif
#ifdef SUPPORT_CDROM
	support_cdrom = false;
#endif
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(cart, 0xff, sizeof(cart));
		fio->Fseek(0, FILEIO_SEEK_END);
		int size = fio->Ftell();
		int head = size % 1024;
		size -= head;
		fio->Fseek(head, FILEIO_SEEK_SET);
		fio->Fread(cart, size, 1);
		fio->Fclose();
		
		if(size == 512 * 1024) {
			bool overdump = true;
			for(int i = 0x40000; i < 0x60000; i++) {
				if(cart[i] != cart[i + 0x20000]) {
					overdump = false;
					break;
				}
			}
			if(overdump) {
				size = 384 * 1024;
			}
		}
		if(size == 384 * 1024) {
			memcpy(cart + 0x060000, cart + 0x040000, 0x020000);	/* Set up 060000 - 07FFFF mirror */
			memcpy(cart + 0x080000, cart + 0x040000, 0x040000);	/* Set up 080000 - 0BFFFF region */
			memcpy(cart + 0x0C0000, cart + 0x040000, 0x040000);	/* Set up 0C0000 - 0FFFFF region */
			memcpy(cart + 0x040000, cart, 0x040000);		/* Set up 040000 - 07FFFF region */
		} else {
			/* mirror 256KB rom data */
			if (size <= 0x040000)
				memcpy(cart + 0x040000, cart, 0x040000);
			/* mirror 512KB rom data */
			if (size <= 0x080000)
				memcpy(cart + 0x080000, cart, 0x080000);
		}
		uint32_t cart_crc32 = get_crc32(cart, size);
#ifdef SUPPORT_SUPER_GFX
		if((size == 0x100000 && cart_crc32 == 0x8c4588e2) ||	// 1941 Counter Attack
		   (size == 0x100000 && cart_crc32 == 0x4c2126b0) ||	// Aldynes
		   (size == 0x080000 && cart_crc32 == 0x3b13af61) ||	// Battle Ace
		   (size == 0x100000 && cart_crc32 == 0xb486a8ed) ||	// Daimakaimura
		   (size == 0x0c0000 && cart_crc32 == 0xbebfe042) ||	// Darius Plus
		   (size == 0x100000 && cart_crc32 == 0x1e1d0319) ||	// Darius Plus (1024K)
		   (size == 0x080000 && cart_crc32 == 0x1f041166)) {	// Grandzort
			support_sgfx = true;
		}
#endif
#ifdef SUPPORT_CDROM
		if(size >= 0x40000 && memcmp(cart + 0x3ffb6, "PC Engine CD-ROM SYSTEM", 23) == 0) {
			support_cdrom = true;
		}
#endif
		if((size == 0x280000 && cart_crc32 == 0xd15cb6bb) ||	// Street Fighter II Champion Edition
		   (size == 0x100000 && cart_crc32 == 0xd6fc51ce)) {	// Strip Figher II
			support_6btn_pad = true;
		}
		if(size == 0x40000 && cart_crc32 == 0x80c3f824) {	// Yokai Dochu Ki
			support_multi_tap = false;
		}
		inserted = true;
	}
	delete fio;
}

void PCE::close_cart()
{
	memset(cart, 0xff, sizeof(cart));
	inserted = false;
}

// vdc

void PCE::pce_interrupt()
{
	/* Draw the last scanline */
	if ( vce.current_bitmap_line >= 14 && vce.current_bitmap_line < 14 + 242 )
	{
		/* We are in the active display area */
		/* First fill the line with the overscan color */
		draw_overscan_line(vce.current_bitmap_line );

		/* Check if we need to draw more just the overscan color */
		if ( vdc[0].current_segment == STATE_VDW )
		{
			/* 0 - no sprite and background pixels drawn
			   1 - background pixel drawn
			   otherwise is 2 + sprite# */
			uint8_t drawn[VDC_WPF];
			/* our line buffer */
			scrntype_t *line_buffer = &vce.bmp[vce.current_bitmap_line][86];

			/* clear our priority/sprite collision detection buffer. */
			memset(drawn, 0, VDC_WPF);

			vdc[0].y_scroll = ( vdc[0].current_segment_line == 0 ) ? vdc[0].vdc_data[BYR].w.l : ( vdc[0].y_scroll + 1 );

			/* Draw VDC #0 background layer */
			pce_refresh_line(0, vdc[0].current_segment_line, 0, drawn, line_buffer);

			/* Draw VDC #0 sprite layer */
			if(vdc[0].vdc_data[CR].w.l & CR_SB)
			{
				pce_refresh_sprites(0, vdc[0].current_segment_line, drawn, line_buffer);
			}
		}
	}
	else
	{
		/* We are in one of the blanking areas */
		draw_black_line(vce.current_bitmap_line );
	}

	/* bump current scanline */
	vce.current_bitmap_line = ( vce.current_bitmap_line + 1 ) % VDC_LPF;
	vdc_advance_line(0);
}

#ifdef SUPPORT_SUPER_GFX
void PCE::sgx_interrupt()
{
	/* Draw the last scanline */
	if ( vce.current_bitmap_line >= 14 && vce.current_bitmap_line < 14 + 242 )
	{
		/* We are in the active display area */
		/* First fill the line with the overscan color */
		draw_sgx_overscan_line(vce.current_bitmap_line );

		/* Check if we need to draw more just the overscan color */
		if ( vdc[0].current_segment == STATE_VDW )
		{
			/* 0 - no sprite and background pixels drawn
			   1 - background pixel drawn
			   otherwise is 2 + sprite# */
			uint8_t drawn[2][512];
			scrntype_t *line_buffer;
			scrntype_t temp_buffer[2][512];
			int i;

			/* clear our priority/sprite collision detection buffer. */
			memset( drawn, 0, sizeof(drawn) );

			vdc[0].y_scroll = ( vdc[0].current_segment_line == 0 ) ? vdc[0].vdc_data[BYR].w.l : ( vdc[0].y_scroll + 1 );
			vdc[1].y_scroll = ( vdc[1].current_segment_line == 0 ) ? vdc[1].vdc_data[BYR].w.l : ( vdc[1].y_scroll + 1 );

			/* Draw VDC #0 background layer */
			pce_refresh_line( 0, vdc[0].current_segment_line, 0, drawn[0], temp_buffer[0]);

			/* Draw VDC #0 sprite layer */
			if(vdc[0].vdc_data[CR].w.l & CR_SB)
			{
				pce_refresh_sprites(0, vdc[0].current_segment_line, drawn[0], temp_buffer[0]);
			}

			/* Draw VDC #1 background layer */
			pce_refresh_line( 1, vdc[1].current_segment_line, 1, drawn[1], temp_buffer[1]);

			/* Draw VDC #1 sprite layer */
			if ( vdc[1].vdc_data[CR].w.l & CR_SB )
			{
				pce_refresh_sprites(1, vdc[1].current_segment_line, drawn[1], temp_buffer[1]);
			}

			line_buffer = &vce.bmp[vce.current_bitmap_line][86];
			/* Combine the output of both VDCs */
			for( i = 0; i < 512; i++ )
			{
				int cur_prio = vpc.prio_map[i];

				if ( vpc.vpc_prio[cur_prio].vdc0_enabled )
				{
					if ( vpc.vpc_prio[cur_prio].vdc1_enabled )
					{
						switch( vpc.vpc_prio[cur_prio].prio )
						{
						case 0:	/* BG1 SP1 BG0 SP0 */
							if ( drawn[0][i] )
							{
								line_buffer[i] = temp_buffer[0][i];
							}
							else if ( drawn[1][i] )
							{
								line_buffer[i] = temp_buffer[1][i];
							}
							break;
						case 1:	/* BG1 BG0 SP1 SP0 */
							if ( drawn[0][i] )
							{
								if ( drawn[0][i] > 1 )
								{
									line_buffer[i] = temp_buffer[0][i];
								}
								else
								{
									if ( drawn[1][i] > 1 )
									{
										line_buffer[i] = temp_buffer[1][i];
									}
									else
									{
										line_buffer[i] = temp_buffer[0][i];
									}
								}
							}
							else if ( drawn[1][i] )
							{
								line_buffer[i] = temp_buffer[1][i];
							}
							break;
						case 2:	/* BG1 + SP1 => SP1
							   BG0 + SP1 => BG0
							   BG0 + BG1 => BG0
							   BG0 + SP0 => SP0
							   BG1 + SP0 => BG1
							   SP0 + SP1 => SP0 */
							if ( drawn[0][i] )
							{
								if ( drawn[0][i] > 1 )
								{
									if ( drawn[1][i] == 1 )
									{
										line_buffer[i] = temp_buffer[1][i];
									}
									else
									{
										line_buffer[i] = temp_buffer[0][i];
									}
								}
								else
								{
									line_buffer[i] = temp_buffer[0][i];
								}
							}
							else if ( drawn[1][i] )
							{
								line_buffer[i] = temp_buffer[1][i];
							}
							break;
						}
					}
					else
					{
						if ( drawn[0][i] )
						{
							line_buffer[i] = temp_buffer[0][i];
						}
					}
				}
				else
				{
					if ( vpc.vpc_prio[cur_prio].vdc1_enabled )
					{
						if ( drawn[1][i] )
						{
							line_buffer[i] = temp_buffer[1][i];
						}
					}
				}
			}
		}
	}
	else
	{
		/* We are in one of the blanking areas */
		draw_black_line(vce.current_bitmap_line );
	}

	/* bump current scanline */
	vce.current_bitmap_line = ( vce.current_bitmap_line + 1 ) % VDC_LPF;
	vdc_advance_line(0);
	vdc_advance_line(1);
}
#endif

void PCE::vdc_advance_line(int which)
{
	int ret = 0;

	vdc[which].curline += 1;
	vdc[which].current_segment_line += 1;
	vdc[which].raster_count += 1;

	if ( vdc[which].satb_countdown )
	{
		vdc[which].satb_countdown -= 1;
		if ( vdc[which].satb_countdown == 0 )
		{
			if ( vdc[which].vdc_data[DCR].w.l & DCR_DSC )
			{
				vdc[which].status |= VDC_DS;	/* set satb done flag */
				ret = 1;
			}
		}
	}

	if ( vce.current_bitmap_line == 0 )
	{
		vdc[which].current_segment = STATE_VSW;
		vdc[which].current_segment_line = 0;
		vdc[which].vblank_triggered = 0;
		vdc[which].curline = 0;
	}

	if ( STATE_VSW == vdc[which].current_segment && vdc[which].current_segment_line >= ( vdc[which].vdc_data[VPR].b.l & 0x1F ) )
	{
		vdc[which].current_segment = STATE_VDS;
		vdc[which].current_segment_line = 0;
	}

	if ( STATE_VDS == vdc[which].current_segment && vdc[which].current_segment_line >= vdc[which].vdc_data[VPR].b.h )
	{
		vdc[which].current_segment = STATE_VDW;
		vdc[which].current_segment_line = 0;
		vdc[which].raster_count = 0x40;
	}

	if ( STATE_VDW == vdc[which].current_segment && vdc[which].current_segment_line > ( vdc[which].vdc_data[VDW].w.l & 0x01FF ) )
	{
		vdc[which].current_segment = STATE_VCR;
		vdc[which].current_segment_line = 0;

		/* Generate VBlank interrupt, sprite DMA */
		vdc[which].vblank_triggered = 1;
		if ( vdc[which].vdc_data[CR].w.l & CR_VR )
		{
			vdc[which].status |= VDC_VD;
			ret = 1;
		}

		/* do VRAM > SATB DMA if the enable bit is set or the DVSSR reg. was written to */
		if( ( vdc[which].vdc_data[DCR].w.l & DCR_DSR ) || vdc[which].dvssr_write )
		{
			int i;

			vdc[which].dvssr_write = 0;

			for( i = 0; i < 256; i++ )
			{
				vdc[which].sprite_ram[i] = ( vdc[which].vram[ ( vdc[which].vdc_data[DVSSR].w.l << 1 ) + i * 2 + 1 ] << 8 ) | vdc[which].vram[ ( vdc[which].vdc_data[DVSSR].w.l << 1 ) + i * 2 ];
			}

			/* generate interrupt if needed */
			if ( vdc[which].vdc_data[DCR].w.l & DCR_DSC )
			{
				vdc[which].satb_countdown = 4;
			}
		}
	}

	if ( STATE_VCR == vdc[which].current_segment )
	{
		if ( vdc[which].current_segment_line >= 3 && vdc[which].current_segment_line >= vdc[which].vdc_data[VCR].b.l )
		{
			vdc[which].current_segment = STATE_VSW;
			vdc[which].current_segment_line = 0;
			vdc[which].curline = 0;
		}
	}

	/* generate interrupt on line compare if necessary */
	if ( vdc[which].raster_count == vdc[which].vdc_data[RCR].w.l && vdc[which].vdc_data[CR].w.l & CR_RC )
	{
		vdc[which].status |= VDC_RR;
		ret = 1;
	}

	/* handle frame events */
	if(vdc[which].curline == 261 && ! vdc[which].vblank_triggered )
	{

		vdc[which].vblank_triggered = 1;
		if(vdc[which].vdc_data[CR].w.l & CR_VR)
		{	/* generate IRQ1 if enabled */
			vdc[which].status |= VDC_VD;	/* set vblank flag */
			ret = 1;
		}

		/* do VRAM > SATB DMA if the enable bit is set or the DVSSR reg. was written to */
		if ( ( vdc[which].vdc_data[DCR].w.l & DCR_DSR ) || vdc[which].dvssr_write )
		{
			int i;

			vdc[which].dvssr_write = 0;
			for( i = 0; i < 256; i++ )
			{
				vdc[which].sprite_ram[i] = ( vdc[which].vram[ ( vdc[which].vdc_data[DVSSR].w.l << 1 ) + i * 2 + 1 ] << 8 ) | vdc[which].vram[ ( vdc[which].vdc_data[DVSSR].w.l << 1 ) + i * 2 ];
			}

			/* generate interrupt if needed */
			if(vdc[which].vdc_data[DCR].w.l & DCR_DSC)
			{
				vdc[which].satb_countdown = 4;
			}
		}
	}

	if (ret) {
		if(!emu->now_waiting_in_debugger) {
			d_cpu->write_signal(INPUT_LINE_IRQ1, HOLD_LINE, 0);
		}
	}
}

void PCE::vdc_reset()
{
	/* clear context */
	memset(&vdc, 0, sizeof(vdc));
	memset(&vce, 0, sizeof(vce));
	memset(&vpc, 0, sizeof(vpc));

	vdc[0].inc = 1;
	vdc[1].inc = 1;

	/* initialize palette */
	int i;

	for( i = 0; i < 512; i++ )
	{
		int r = (( i >> 3) & 7) << 5;
		int g = (( i >> 6) & 7) << 5;
		int b = (( i     ) & 7) << 5;
		int y = ( (  66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
		vce.palette[i] = RGB_COLOR(r, g, b);
		vce.palette[512+i] = RGB_COLOR(y, y, y);
	}

	vpc_w( 0, 0x11 );
	vpc_w( 1, 0x11 );
	vpc.window1.w.l = 0;
	vpc.window2.w.l = 0;
	vpc.vdc_select = 0;
}

void PCE::draw_black_line(int line)
{
	int i;

	/* our line buffer */
	scrntype_t *line_buffer = vce.bmp[line];

	for( i=0; i< VDC_WPF; i++ )
		line_buffer[i] = 0;
}

void PCE::draw_overscan_line(int line)
{
	int i;

	/* Are we in greyscale mode or in color mode? */
	scrntype_t *color_base = vce.palette + (vce.vce_control & 0x80 ? 512 : 0);

	/* our line buffer */
	scrntype_t *line_buffer = vce.bmp[line];

	for ( i = 0; i < VDC_WPF; i++ )
		line_buffer[i] = color_base[vce.vce_data[0x100].w.l];
}

#ifdef SUPPORT_SUPER_GFX
void PCE::draw_sgx_overscan_line(int line)
{
	int i;

	/* Are we in greyscale mode or in color mode? */
	scrntype_t *color_base = vce.palette + (vce.vce_control & 0x80 ? 512 : 0);

	/* our line buffer */
	scrntype_t *line_buffer = vce.bmp[line];

	for ( i = 0; i < VDC_WPF; i++ )
		line_buffer[i] = color_base[vce.vce_data[0].w.l];
}
#endif

void PCE::vram_write(int which, uint32_t offset, uint8_t data)
{
	if(offset & 0x10000)
	{
		return;
	}
	else
	{
		vdc[which].vram[offset] = data;
	}
}

uint8_t PCE::vram_read(int which, uint32_t offset)
{
	uint8_t temp;

	if(offset & 0x10000)
	{
		temp = vdc[which].vram[offset & 0xFFFF];
	}
	else
	{
		temp = vdc[which].vram[offset];
	}

	return temp;
}

void PCE::vdc_w(int which, uint16_t offset, uint8_t data)
{
	switch(offset&3)
	{
		case 0x00:	/* VDC register select */
			vdc[which].vdc_register = (data & 0x1F);
			break;

		case 0x02:	/* VDC data (LSB) */
			vdc[which].vdc_data[vdc[which].vdc_register].b.l = data;
			switch(vdc[which].vdc_register)
			{
				case VxR:	/* LSB of data to write to VRAM */
					vdc[which].vdc_latch = data;
					break;

				case BYR:
					vdc[which].y_scroll=vdc[which].vdc_data[BYR].w.l;
					break;

				case HDR:
					vdc[which].physical_width = ((data & 0x003F) + 1) << 3;
					break;

				case VDW:
					vdc[which].physical_height &= 0xFF00;
					vdc[which].physical_height |= (data & 0xFF);
					vdc[which].physical_height &= 0x01FF;
					break;

				case LENR:
					break;
				case SOUR:
					break;
				case DESR:
					break;
			}
			break;

		case 0x03:	/* VDC data (MSB) */
			vdc[which].vdc_data[vdc[which].vdc_register].b.h = data;
			switch(vdc[which].vdc_register)
			{
				case VxR:	/* MSB of data to write to VRAM */
					vram_write(which, vdc[which].vdc_data[MAWR].w.l*2+0, vdc[which].vdc_latch);
					vram_write(which, vdc[which].vdc_data[MAWR].w.l*2+1, data);
					vdc[which].vdc_data[MAWR].w.l += vdc[which].inc;
					break;

				case CR:
					{
						static const unsigned char inctab[] = {1, 32, 64, 128};
						vdc[which].inc = inctab[(data >> 3) & 3];
					}
					break;

				case VDW:
					vdc[which].physical_height &= 0x00FF;
					vdc[which].physical_height |= (data << 8);
					vdc[which].physical_height &= 0x01FF;
					break;

				case DVSSR:
					/* Force VRAM <> SATB DMA for this frame */
					vdc[which].dvssr_write = 1;
					break;

				case BYR:
					vdc[which].y_scroll=vdc[which].vdc_data[BYR].w.l;
					break;

				case LENR:
					vdc_do_dma(which);
					break;
				case SOUR:
					break;
				case DESR:
					break;
			}
			break;
	}
}

uint8_t PCE::vdc_r(int which, uint16_t offset)
{
	int temp = 0;
	switch(offset & 3)
	{
		case 0x00:
			temp = vdc[which].status;
			vdc[which].status &= ~(VDC_VD | VDC_DV | VDC_DS | VDC_RR | VDC_OR | VDC_CR);
			d_cpu->write_signal(INPUT_LINE_IRQ1, CLEAR_LINE, 0);
			break;

		case 0x02:
			temp = vram_read(which, vdc[which].vdc_data[MARR].w.l * 2 + 0);
			break;

		case 0x03:
			temp = vram_read(which, vdc[which].vdc_data[MARR].w.l * 2 + 1);
			if ( vdc[which].vdc_register == VxR )
			{
				vdc[which].vdc_data[MARR].w.l += vdc[which].inc;
			}
			break;
	}
	return (temp);
}

uint8_t PCE::vce_r(uint16_t offset)
{
	int temp = 0xFF;
	switch(offset & 7)
	{
		case 0x04:	/* color table data (LSB) */
			temp = vce.vce_data[vce.vce_address.w.l].b.l;
			break;

		case 0x05:	/* color table data (MSB) */
			temp = vce.vce_data[vce.vce_address.w.l].b.h;
			temp |= 0xFE;
			vce.vce_address.w.l = (vce.vce_address.w.l + 1) & 0x01FF;
			break;
	}
	return (temp);
}

void PCE::vce_w(uint16_t offset, uint8_t data)
{
	switch(offset & 7)
	{
		case 0x00:	/* control reg. */
			vce.vce_control = data;
			break;

		case 0x02:	/* color table address (LSB) */
			vce.vce_address.b.l = data;
			vce.vce_address.w.l &= 0x1FF;
			break;

		case 0x03:	/* color table address (MSB) */
			vce.vce_address.b.h = data;
			vce.vce_address.w.l &= 0x1FF;
			break;

		case 0x04:	/* color table data (LSB) */
			vce.vce_data[vce.vce_address.w.l].b.l = data;
			break;

		case 0x05:	/* color table data (MSB) */
			vce.vce_data[vce.vce_address.w.l].b.h = data & 0x01;

			/* bump internal address */
			vce.vce_address.w.l = (vce.vce_address.w.l + 1) & 0x01FF;
			break;
	}
}

void PCE::pce_refresh_line(int which, int line, int external_input, uint8_t *drawn, scrntype_t *line_buffer)
{
	static const int width_table[4] = {5, 6, 7, 7};

	int scroll_y = ( vdc[which].y_scroll & 0x01FF);
	int scroll_x = (vdc[which].vdc_data[BXR].w.l & 0x03FF);
	int nt_index;

	/* is virtual map 32 or 64 characters tall ? (256 or 512 pixels) */
	int v_line = (scroll_y) & (vdc[which].vdc_data[MWR].w.l & 0x0040 ? 0x1FF : 0x0FF);

	/* row within character */
	int v_row = (v_line & 7);

	/* row of characters in BAT */
	int nt_row = (v_line >> 3);

	/* virtual X size (# bits to shift) */
	int v_width = width_table[(vdc[which].vdc_data[MWR].w.l >> 4) & 3];

	/* pointer to the name table (Background Attribute Table) in VRAM */
	uint8_t *bat = &(vdc[which].vram[nt_row << (v_width+1)]);

	/* Are we in greyscale mode or in color mode? */
	scrntype_t *color_base = vce.palette + (vce.vce_control & 0x80 ? 512 : 0);

	int b0, b1, b2, b3;
	int i0, i1, i2, i3;
	int cell_pattern_index;
	int cell_palette;
	int x, c, i;

	/* character blanking bit */
	if(!(vdc[which].vdc_data[CR].w.l & CR_BB))
	{
		return;
	}
	else
	{
		int	pixel = 0;
		int phys_x = - ( scroll_x & 0x07 );

		for(i=0;i<(vdc[which].physical_width >> 3) + 1;i++)
		{
			nt_index = (i + (scroll_x >> 3)) & ((2 << (v_width-1))-1);
			nt_index *= 2;

			/* get name table data: */

			/* palette # = index from 0-15 */
			cell_palette = ( bat[nt_index + 1] >> 4 ) & 0x0F;

			/* This is the 'character number', from 0-0x0FFF         */
			/* then it is shifted left 4 bits to form a VRAM address */
			/* and one more bit to convert VRAM word offset to a     */
			/* byte-offset within the VRAM space                     */
			cell_pattern_index = ( ( ( bat[nt_index + 1] << 8 ) | bat[nt_index] ) & 0x0FFF) << 5;

			b0 = vram_read(which, (cell_pattern_index) + (v_row << 1) + 0x00);
			b1 = vram_read(which, (cell_pattern_index) + (v_row << 1) + 0x01);
			b2 = vram_read(which, (cell_pattern_index) + (v_row << 1) + 0x10);
			b3 = vram_read(which, (cell_pattern_index) + (v_row << 1) + 0x11);

			for(x=0;x<8;x++)
			{
				i0 = (b0 >> (7-x)) & 1;
				i1 = (b1 >> (7-x)) & 1;
				i2 = (b2 >> (7-x)) & 1;
				i3 = (b3 >> (7-x)) & 1;
				c = (cell_palette << 4 | i3 << 3 | i2 << 2 | i1 << 1 | i0);

				/* colour #0 always comes from palette #0 */
				if ( ! ( c & 0x0F ) )
					c &= 0x0F;

				if ( phys_x >= 0 && phys_x < vdc[which].physical_width )
				{
					drawn[ pixel ] = c ? 1 : 0;
					if ( c || ! external_input )
						line_buffer[ pixel ] = color_base[vce.vce_data[c].w.l];
					pixel++;
//					if ( vdc[which].physical_width != 512 )
//					{
//						while ( pixel < ( ( ( phys_x + 1 ) * 512 ) / vdc[which].physical_width ) )
//						{
//							drawn[ pixel ] = c ? 1 : 0;
//							if ( c || ! external_input )
//							line_buffer[ pixel ] = color_base[vce.vce_data[c].w.l];
//							pixel++;
//						}
//					}
				}
				phys_x += 1;
			}
		}
	}
}

void PCE::conv_obj(int which, int i, int l, int hf, int vf, char *buf)
{
	int b0, b1, b2, b3, i0, i1, i2, i3, x;
	int xi;
	int tmp;

	l &= 0x0F;
	if(vf) l = (15 - l);

	tmp = l + ( i << 5);

	b0 = vram_read(which, (tmp + 0x00)<<1);
	b0 |= vram_read(which, ((tmp + 0x00)<<1)+1)<<8;
	b1 = vram_read(which, (tmp + 0x10)<<1);
	b1 |= vram_read(which, ((tmp + 0x10)<<1)+1)<<8;
	b2 = vram_read(which, (tmp + 0x20)<<1);
	b2 |= vram_read(which, ((tmp + 0x20)<<1)+1)<<8;
	b3 = vram_read(which, (tmp + 0x30)<<1);
	b3 |= vram_read(which, ((tmp + 0x30)<<1)+1)<<8;

	for(x=0;x<16;x++)
	{
		if(hf) xi = x; else xi = (15 - x);
		i0 = (b0 >> xi) & 1;
		i1 = (b1 >> xi) & 1;
		i2 = (b2 >> xi) & 1;
		i3 = (b3 >> xi) & 1;
		buf[x] = (i3 << 3 | i2 << 2 | i1 << 1 | i0);
	}
}

void PCE::pce_refresh_sprites(int which, int line, uint8_t *drawn, scrntype_t *line_buffer)
{
	int i;
	uint8_t sprites_drawn = 0;

	/* Are we in greyscale mode or in color mode? */
	scrntype_t *color_base = vce.palette + (vce.vce_control & 0x80 ? 512 : 0);

	/* count up: Highest priority is Sprite 0 */
	for(i = 0; i < 64; i++)
	{
		static const int cgy_table[] = {16, 32, 64, 64};

		int obj_y = (vdc[which].sprite_ram[(i << 2) + 0] & 0x03FF) - 64;
		int obj_x = (vdc[which].sprite_ram[(i << 2) + 1] & 0x03FF) - 32;
		int obj_i = (vdc[which].sprite_ram[(i << 2) + 2] & 0x07FE);
		int obj_a = (vdc[which].sprite_ram[(i << 2) + 3]);
		int cgx   = (obj_a >> 8) & 1;   /* sprite width */
		int cgy   = (obj_a >> 12) & 3;  /* sprite height */
		int hf    = (obj_a >> 11) & 1;  /* horizontal flip */
		int vf    = (obj_a >> 15) & 1;  /* vertical flip */
		int palette = (obj_a & 0x000F);
		int priority = (obj_a >> 7) & 1;
		int obj_h = cgy_table[cgy];
		int obj_l = (line - obj_y);
		int cgypos;
		char buf[16];

		if ((obj_y == -64) || (obj_y > line)) continue;
		if ((obj_x == -32) || (obj_x >= vdc[which].physical_width)) continue;

		/* no need to draw an object that's ABOVE where we are. */
		if((obj_y + obj_h) < line) continue;

		/* If CGX is set, bit 0 of sprite pattern index is forced to 0 */
		if ( cgx )
			obj_i &= ~2;

		/* If CGY is set to 1, bit 1 of the sprite pattern index is forced to 0. */
		if ( cgy & 1 )
			obj_i &= ~4;

		/* If CGY is set to 2 or 3, bit 1 and 2 of the sprite pattern index are forced to 0. */
		if ( cgy & 2 )
			obj_i &= ~12;

		if (obj_l < obj_h)
		{

			sprites_drawn++;
			if(sprites_drawn > 16)
			{
				if(vdc[which].vdc_data[CR].w.l & CR_OV)
				{
					/* note: flag is set only if irq is taken, Mizubaku Daibouken relies on this behaviour */
					vdc[which].status |= VDC_OR;
					if(!emu->now_waiting_in_debugger) {
						d_cpu->write_signal(INPUT_LINE_IRQ1, ASSERT_LINE, 0);
					}
				}
				continue;  /* Should cause an interrupt */
			}

			cgypos = (obj_l >> 4);
			if(vf) cgypos = ((obj_h - 1) >> 4) - cgypos;

			if(cgx == 0)
			{
				int x;
				int pixel_x = obj_x;//( ( obj_x * 512 ) / vdc[which].physical_width );

				conv_obj(which, obj_i + (cgypos << 2), obj_l, hf, vf, buf);

				for(x = 0; x < 16; x++)
				{
					if(((obj_x + x) < (vdc[which].physical_width)) && ((obj_x + x) >= 0))
					{
						if ( buf[x] )
						{
							if( drawn[pixel_x] < 2 )
							{
								if( priority || drawn[pixel_x] == 0 )
								{
									line_buffer[pixel_x] = color_base[vce.vce_data[0x100 + (palette << 4) + buf[x]].w.l];
//									if ( vdc[which].physical_width != 512 )
//									{
//										int dp = 1;
//										while ( pixel_x + dp < ( ( ( obj_x + x + 1 ) * 512 ) / vdc[which].physical_width ) )
//										{
//											drawn[pixel_x + dp] = i + 2;
//											line_buffer[pixel_x + dp] = color_base[vce.vce_data[0x100 + (palette << 4) + buf[x]].w.l];
//											dp++;
//										}
//									}
								}
								drawn[pixel_x] = i + 2;
							}
							/* Check for sprite #0 collision */
							else if (drawn[pixel_x] == 2)
							{
								if(vdc[which].vdc_data[CR].w.l & CR_CC) {
									if(!emu->now_waiting_in_debugger) {
										d_cpu->write_signal(INPUT_LINE_IRQ1, ASSERT_LINE, 0);
									}
								}
								vdc[which].status |= VDC_CR;
							}
						}
					}
//					if ( vdc[which].physical_width != 512 )
//					{
//						pixel_x = ( ( obj_x + x + 1 ) * 512 ) / vdc[which].physical_width;
//					}
//					else
//					{
						pixel_x += 1;
//					}
				}
			}
			else
			{
				int x;
				int pixel_x = obj_x;//( ( obj_x * 512 ) / vdc[which].physical_width );

				conv_obj(which, obj_i + (cgypos << 2) + (hf ? 2 : 0), obj_l, hf, vf, buf);

				for(x = 0; x < 16; x++)
				{
					if(((obj_x + x) < (vdc[which].physical_width)) && ((obj_x + x) >= 0))
					{
						if ( buf[x] )
						{
							if( drawn[pixel_x] < 2 )
							{
								if ( priority || drawn[pixel_x] == 0 )
								{
									line_buffer[pixel_x] = color_base[vce.vce_data[0x100 + (palette << 4) + buf[x]].w.l];
//									if ( vdc[which].physical_width != 512 )
//									{
//										int dp = 1;
//										while ( pixel_x + dp < ( ( ( obj_x + x + 1 ) * 512 ) / vdc[which].physical_width ) )
//										{
//											drawn[pixel_x + dp] = i + 2;
//											line_buffer[pixel_x + dp] = color_base[vce.vce_data[0x100 + (palette << 4) + buf[x]].w.l];
//											dp++;
//										}
//									}
								}
								drawn[pixel_x] = i + 2;
							}
							/* Check for sprite #0 collision */
							else if ( drawn[pixel_x] == 2 )
							{
								if(vdc[which].vdc_data[CR].w.l & CR_CC) {
									if(!emu->now_waiting_in_debugger) {
										d_cpu->write_signal(INPUT_LINE_IRQ1, ASSERT_LINE, 0);
									}
								}
								vdc[which].status |= VDC_CR;
							}
						}
					}
//					if ( vdc[which].physical_width != 512 )
//					{
//						pixel_x = ( ( obj_x + x + 1 ) * 512 ) / vdc[which].physical_width;
//					}
//					else
//					{
						pixel_x += 1;
//					}
				}

				/* 32 pixel wide sprites are counted as 2 sprites and the right half
				   is only drawn if there are 2 open slots.
				*/
				sprites_drawn++;
				if( sprites_drawn > 16 )
				{
					if(vdc[which].vdc_data[CR].w.l&CR_OV)
					{
						/* note: flag is set only if irq is taken, Mizubaku Daibouken relies on this behaviour */
						vdc[which].status |= VDC_OR;
						if(!emu->now_waiting_in_debugger) {
							d_cpu->write_signal(INPUT_LINE_IRQ1, ASSERT_LINE, 0);
						}
					}
				}
				else
				{
					conv_obj(which, obj_i + (cgypos << 2) + (hf ? 0 : 2), obj_l, hf, vf, buf);
					for(x = 0; x < 16; x++)
					{
						if(((obj_x + 0x10 + x) < (vdc[which].physical_width)) && ((obj_x + 0x10 + x) >= 0))
						{
							if ( buf[x] )
							{
								if( drawn[pixel_x] < 2 )
								{
									if( priority || drawn[pixel_x] == 0 )
									{
										line_buffer[pixel_x] = color_base[vce.vce_data[0x100 + (palette << 4) + buf[x]].w.l];
//										if ( vdc[which].physical_width != 512 )
//										{
//											int dp = 1;
//											while ( pixel_x + dp < ( ( ( obj_x + x + 17 ) * 512 ) / vdc[which].physical_width ) )
//											{
//												drawn[pixel_x + dp] = i + 2;
//												line_buffer[pixel_x + dp] = color_base[vce.vce_data[0x100 + (palette << 4) + buf[x]].w.l];
//												dp++;
//											}
//										}
									}
									drawn[pixel_x] = i + 2;
								}
								/* Check for sprite #0 collision */
								else if ( drawn[pixel_x] == 2 )
								{
									if(vdc[which].vdc_data[CR].w.l & CR_CC) {
										if(!emu->now_waiting_in_debugger) {
											d_cpu->write_signal(INPUT_LINE_IRQ1, ASSERT_LINE, 0);
										}
									}
									vdc[which].status |= VDC_CR;
								}
							}
						}
//						if ( vdc[which].physical_width != 512 )
//						{
//							pixel_x = ( ( obj_x + x + 17 ) * 512 ) / vdc[which].physical_width;
//						}
//						else
//						{
							pixel_x += 1;
//						}
					}
				}
			}
		}
	}
}

void PCE::vdc_do_dma(int which)
{
	int src = vdc[which].vdc_data[SOUR].w.l;
	int dst = vdc[which].vdc_data[DESR].w.l;
	int len = vdc[which].vdc_data[LENR].w.l;

	int did = (vdc[which].vdc_data[DCR].w.l >> 3) & 1;
	int sid = (vdc[which].vdc_data[DCR].w.l >> 2) & 1;
	int dvc = (vdc[which].vdc_data[DCR].w.l >> 1) & 1;

	do {
		uint8_t l, h;

		l = vram_read(which, src<<1);
		h = vram_read(which, (src<<1) + 1);

		vram_write(which, dst<<1,l);
		vram_write(which, 1+(dst<<1),h);

		if(sid) src = (src - 1) & 0xFFFF;
		else	src = (src + 1) & 0xFFFF;

		if(did) dst = (dst - 1) & 0xFFFF;
		else	dst = (dst + 1) & 0xFFFF;

		len = (len - 1) & 0xFFFF;

	} while (len != 0xFFFF);

	vdc[which].status |= VDC_DV;
	vdc[which].vdc_data[SOUR].w.l = src;
	vdc[which].vdc_data[DESR].w.l = dst;
	vdc[which].vdc_data[LENR].w.l = len;
	if(dvc)
	{
		d_cpu->write_signal(INPUT_LINE_IRQ1, ASSERT_LINE, 0);
	}

}

void PCE::vpc_update_prio_map()
{
	int i;

	for( i = 0; i < 512; i++ )
	{
		vpc.prio_map[i] = 0;
		if ( vpc.window1.w.l < 0x40 || i > vpc.window1.w.l )
		{
			vpc.prio_map[i] |= 1;
		}
		if ( vpc.window2.w.l < 0x40 || i > vpc.window2.w.l )
		{
			vpc.prio_map[i] |= 2;
		}
	}
}

void PCE::vpc_w(uint16_t offset, uint8_t data)
{
	switch( offset & 0x07 )
	{
	case 0x00:	/* Priority register #0 */
		vpc.priority.b.l = data;
		vpc.vpc_prio[0].prio = ( data >> 2 ) & 3;
		vpc.vpc_prio[0].vdc0_enabled = data & 1;
		vpc.vpc_prio[0].vdc1_enabled = data & 2;
		vpc.vpc_prio[1].prio = ( data >> 6 ) & 3;
		vpc.vpc_prio[1].vdc0_enabled = data & 0x10;
		vpc.vpc_prio[1].vdc1_enabled = data & 0x20;
		break;
	case 0x01:	/* Priority register #1 */
		vpc.priority.b.h = data;
		vpc.vpc_prio[2].prio = ( data >> 2 ) & 3;
		vpc.vpc_prio[2].vdc0_enabled = data & 1;
		vpc.vpc_prio[2].vdc1_enabled = data & 2;
		vpc.vpc_prio[3].prio = ( data >> 6 ) & 3;
		vpc.vpc_prio[3].vdc0_enabled = data & 0x10;
		vpc.vpc_prio[3].vdc1_enabled = data & 0x20;
		break;
	case 0x02:	/* Window 1 LSB */
		vpc.window1.b.l = data;
		vpc_update_prio_map();
		break;
	case 0x03:	/* Window 1 MSB */
		vpc.window1.b.h = data & 3;
		vpc_update_prio_map();
		break;
	case 0x04:	/* Window 2 LSB */
		vpc.window2.b.l = data;
		vpc_update_prio_map();
		break;
	case 0x05:	/* Window 2 MSB */
		vpc.window2.b.h = data & 3;
		vpc_update_prio_map();
		break;
	case 0x06:	/* VDC I/O select */
		vpc.vdc_select = data & 1;
		break;
	}
}

uint8_t PCE::vpc_r(uint16_t offset)
{
	uint8_t data = 0;
	switch( offset & 0x07 )
	{
	case 0x00:  /* Priority register #0 */
		data = vpc.priority.b.l;
		break;
	case 0x01:  /* Priority register #1 */
		data = vpc.priority.b.h;
		break;
	case 0x02:  /* Window 1 LSB */
		data = vpc.window1.b.l;
		break;
	case 0x03:  /* Window 1 MSB; high bits are 0 or 1? */
		data = vpc.window1.b.h;
		break;
	case 0x04:  /* Window 2 LSB */
		data = vpc.window2.b.l;
		break;
	case 0x05:  /* Window 2 MSB; high bits are 0 or 1? */
		data = vpc.window2.b.h;
		break;
	}
	return data;
}

#ifdef SUPPORT_SUPER_GFX
void PCE::sgx_vdc_w(uint16_t offset, uint8_t data)
{
	if ( vpc.vdc_select )
	{
		vdc_w( 1, offset, data );
	}
	else
	{
		vdc_w( 0, offset, data );
	}
}

uint8_t PCE::sgx_vdc_r(uint16_t offset)
{
	return ( vpc.vdc_select ) ? vdc_r( 1, offset ) : vdc_r( 0, offset );
}
#endif

// psg

void PCE::psg_reset()
{
	touch_sound();
	memset(psg, 0, sizeof(psg));
	for (int i = 0; i < 6; i++) {
		psg[i].regs[4] = 0x80;
	}
	psg[4].randval = psg[5].randval = 0x51f631e4;
	
	psg_ch = 0;
	psg_vol = psg_lfo_freq = psg_lfo_ctrl = 0;
}

void PCE::psg_write(uint16_t addr, uint8_t data)
{
	switch(addr & 0x1f) {
	case 0:
		touch_sound();
		psg_ch = data & 7;
		break;
	case 1:
		touch_sound();
		psg_vol = data;
		break;
	case 2:
		touch_sound();
		psg[psg_ch].regs[2] = data;
		break;
	case 3:
		touch_sound();
//		psg[psg_ch].regs[3] = data & 0x1f;
		psg[psg_ch].regs[3] = data & 0xf;
		break;
	case 4:
		touch_sound();
		psg[psg_ch].regs[4] = data;
		break;
	case 5:
		touch_sound();
		psg[psg_ch].regs[5] = data;
		break;
	case 6:
		touch_sound();
		if(psg[psg_ch].regs[4] & 0x40) {
			psg[psg_ch].wav[0] =data & 0x1f;
		}
		else {
			psg[psg_ch].wav[psg[psg_ch].wavptr] = data & 0x1f;
			psg[psg_ch].wavptr = (psg[psg_ch].wavptr + 1) & 0x1f;
		}
		break;
	case 7:
		touch_sound();
		psg[psg_ch].regs[7] = data;
		break;
	case 8:
		touch_sound();
		psg_lfo_freq = data;
		break;
	case 9:
		touch_sound();
		psg_lfo_ctrl = data;
		break;
	}
}

uint8_t PCE::psg_read(uint16_t addr)
{
	int ptr;
	
	switch(addr & 0x1f) {
	case 0:
		return psg_ch;
	case 1:
		return psg_vol;
	case 2:
		return psg[psg_ch].regs[2];
	case 3:
		return psg[psg_ch].regs[3];
	case 4:
		return psg[psg_ch].regs[4];
	case 5:
		return psg[psg_ch].regs[5];
	case 6:
		ptr = psg[psg_ch].wavptr;
		psg[psg_ch].wavptr = (psg[psg_ch].wavptr + 1) & 0x1f;
		return psg[psg_ch].wav[ptr];
	case 7:
		return psg[psg_ch].regs[7];
	case 8:
		return psg_lfo_freq;
	case 9:
		return psg_lfo_ctrl;
	}
	return 0xff;
}

void PCE::mix(int32_t* buffer, int cnt)
{
	int vol_tbl[32] = {
		 100, 451, 508, 573, 646, 728, 821, 925,1043,1175,1325, 1493, 1683, 1898, 2139, 2411,
		2718,3064,3454,3893,4388,4947,5576,6285,7085,7986,9002,10148,11439,12894,14535,16384
	};
	
	if(!inserted) {
		return;
	}
	for(int ch = 0; ch < 6; ch++) {
		if(!(psg[ch].regs[4] & 0x80)) {
			// mute
			psg[ch].genptr = psg[ch].remain = 0;
		}
		else if(psg[ch].regs[4] & 0x40) {
			// dda
			int32_t wav = ((int32_t)psg[ch].wav[0] - 16) * 702;
			int32_t vol = max((psg_vol >> 3) & 0x1e, (psg_vol << 1) & 0x1e) + (psg[ch].regs[4] & 0x1f) + max((psg[ch].regs[5] >> 3) & 0x1e, (psg[ch].regs[5] << 1) & 0x1e) - 60;
			vol = (vol < 0) ? 0 : (vol > 31) ? 31 : vol;
			int32_t outvol = wav * vol_tbl[vol] / 16384;
			for(int i = 0, j = 0; i < cnt; i++, j += 2) {
				buffer[j    ] += apply_volume(outvol, volume_l); // L
				buffer[j + 1] += apply_volume(outvol, volume_r); // R
			}
		}
		else if(ch >= 4 && (psg[ch].regs[7] & 0x80)) {
			// noise
			uint16_t freq = (psg[ch].regs[7] & 0x1f);
			int32_t vol = max((psg_vol >> 3) & 0x1e, (psg_vol << 1) & 0x1e) + (psg[ch].regs[4] & 0x1f) + max((psg[ch].regs[5] >> 3) & 0x1e, (psg[ch].regs[5] << 1) & 0x1e) - 60;
			vol = (vol < 0) ? 0 : (vol > 31) ? 31 : vol;
			vol = vol_tbl[vol];
			for(int i = 0, j = 0; i < cnt; i++, j += 2) {
				psg[ch].remain += 3000 + freq * 512;
				uint32_t t = psg[ch].remain / sample_rate;
				if(t >= 1) {
					if(psg[ch].randval & 0x80000) {
						psg[ch].randval = ((psg[ch].randval ^ 4) << 1) + 1;
						psg[ch].noise = true;
					}
					else {
						psg[ch].randval <<= 1;
						psg[ch].noise = false;
					}
					psg[ch].remain -= sample_rate * t;
				}
				int32_t outvol = (int32_t)((psg[ch].noise ? 10 * 702 : -10 * 702) * vol / 16384);
				buffer[j    ] += apply_volume(outvol, volume_l); // L
				buffer[j + 1] += apply_volume(outvol, volume_r); // R
			}
		}
		else {
			int32_t wav[32];
			for(int i = 0; i < 32; i++) {
				wav[i] = ((int32_t)psg[ch].wav[i] - 16) * 702;
			}
			uint32_t freq = psg[ch].regs[2] + ((uint32_t)psg[ch].regs[3] << 8);
			if(freq) {
				int32_t vol = max((psg_vol >> 3) & 0x1e, (psg_vol << 1) & 0x1e) + (psg[ch].regs[4] & 0x1f) + max((psg[ch].regs[5] >> 3) & 0x1e, (psg[ch].regs[5] << 1) & 0x1e) - 60;
				vol = (vol < 0) ? 0 : (vol > 31) ? 31 : vol;
				vol = vol_tbl[vol];
				for(int i = 0, j = 0; i < cnt; i++, j += 2) {
					int32_t outvol = wav[psg[ch].genptr] * vol / 16384;
					buffer[j    ] += apply_volume(outvol, volume_l); // L
					buffer[j + 1] += apply_volume(outvol, volume_r); // R
					psg[ch].remain += 32 * 1118608 / freq;
					uint32_t t = psg[ch].remain / (10 * sample_rate);
					psg[ch].genptr = (psg[ch].genptr + t) & 0x1f;
					psg[ch].remain -= 10 * sample_rate * t;
				}
			}
		}
	}
#ifdef SUPPORT_CDROM
	if(support_cdrom) {
		if(!msm_idle) {
			d_msm->mix(buffer, cnt);
		}
		d_scsi_cdrom->mix(buffer, cnt);
	}
#endif
}

void PCE::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

// joypad (non multipad)

void PCE::joy_reset()
{
	joy_counter = 0;
	joy_high_nibble = joy_second_byte = false;
}

void PCE::joy_write(uint16_t addr, uint8_t data)
{
	joy_high_nibble = ((data & 1) != 0);
	
	if(data & 2) {
		joy_counter = 0;
		joy_high_nibble = false;
		joy_second_byte = !joy_second_byte;
	}
}

uint8_t PCE::joy_read(uint16_t addr)
{
	uint8_t index;
	
	if(joy_high_nibble) {
		if(++joy_counter == 16) {
			joy_counter = 0;
		}
	}
	if(joy_counter == 0) {
		return 0x00;
	}
	if(support_multi_tap) {
		if(joy_counter > 4) {
			return 0x0f;
		}
		index = joy_counter;
	} else {
		index = 1;
	}
	if(support_6btn_pad) {
		return joy_6btn_pad_r(index);
	} else {
		return joy_2btn_pad_r(index);
	}
}

uint8_t PCE::joy_2btn_pad_r(uint8_t index)
{
	uint8_t data = 0x0f;
	
	if(joy_high_nibble) {
		if(joy_stat[index - 1] & 0x001) data &= ~0x01;	// Up
		if(joy_stat[index - 1] & 0x008) data &= ~0x02;	// Right
		if(joy_stat[index - 1] & 0x002) data &= ~0x04;	// Down
		if(joy_stat[index - 1] & 0x004) data &= ~0x08;	// Left
	} else {
		if(joy_stat[index - 1] & 0x010) data &= ~0x01;	// Button #1
		if(joy_stat[index - 1] & 0x020) data &= ~0x02;	// Button #2
		if(joy_stat[index - 1] & 0x040) data &= ~0x04;	// Select
		if(joy_stat[index - 1] & 0x080) data &= ~0x08;	// Run
	}
	return data;
}

uint8_t PCE::joy_6btn_pad_r(uint8_t index)
{
	uint8_t data = 0x0f;
	
	if(joy_second_byte) {
		if(joy_high_nibble) {
			if(joy_stat[index - 1] & 0x001) data &= ~0x01;	// Up
			if(joy_stat[index - 1] & 0x008) data &= ~0x02;	// Right
			if(joy_stat[index - 1] & 0x002) data &= ~0x04;	// Down
			if(joy_stat[index - 1] & 0x004) data &= ~0x08;	// Left
		} else {
			if(joy_stat[index - 1] & 0x010) data &= ~0x01;	// Button #1
			if(joy_stat[index - 1] & 0x020) data &= ~0x02;	// Button #2
			if(joy_stat[index - 1] & 0x040) data &= ~0x04;	// Select
			if(joy_stat[index - 1] & 0x080) data &= ~0x08;	// Run
		}
	} else {
		if(joy_high_nibble) {
			return 0x00;
		} else {
			if(joy_stat[index - 1] & 0x100) data &= ~0x01;	// Button #3
			if(joy_stat[index - 1] & 0x200) data &= ~0x02;	// Button #4
			if(joy_stat[index - 1] & 0x400) data &= ~0x04;	// Button #5
			if(joy_stat[index - 1] & 0x800) data &= ~0x08;	// Button #6
		}
	}
	if(!support_multi_tap) {
		if(joy_counter == 5 && !joy_high_nibble) {
			joy_second_byte = false;
		}
	}
	return data;
}

// CD-ROM^2

#ifdef SUPPORT_CDROM
#define PCE_CD_IRQ_TRANSFER_READY	0x40
#define PCE_CD_IRQ_TRANSFER_DONE	0x20
#define PCE_CD_IRQ_BRAM			0x10 /* ??? */
#define PCE_CD_IRQ_SAMPLE_FULL_PLAY	0x08
#define PCE_CD_IRQ_SAMPLE_HALF_PLAY	0x04

#define PCE_CD_ADPCM_PLAY_FLAG		0x08
#define PCE_CD_ADPCM_STOP_FLAG		0x01

#define EVENT_CDDA_FADE_IN		0
#define EVENT_CDDA_FADE_OUT		1
#define EVENT_ADPCM_FADE_IN		2
#define EVENT_ADPCM_FADE_OUT		3

void PCE::cdrom_initialize()
{
	adpcm_clock_divider = 1;
	backup_locked = true;
	event_cdda_fader = event_adpcm_fader = -1;
}

void PCE::cdrom_reset()
{
	touch_sound();
	memset(cdrom_regs, 0, sizeof(cdrom_regs));
	cdrom_regs[0x0c] |= PCE_CD_ADPCM_STOP_FLAG;
	cdrom_regs[0x0c] &= ~PCE_CD_ADPCM_PLAY_FLAG;
	
	irq_status = drq_status = false;
	
	adpcm_read_ptr = adpcm_write_ptr = 0;
	adpcm_read_buf = adpcm_write_buf = 0;
	adpcm_dma_enabled = false;
	msm_idle = 1;
	
	if(event_cdda_fader != -1) {
		cancel_event(this, event_cdda_fader);
	}
	if(event_adpcm_fader != -1) {
		cancel_event(this, event_adpcm_fader);
	}
	cdda_volume = adpcm_volume = 100.0;
	event_cdda_fader = event_adpcm_fader = -1;
	
	d_scsi_cdrom->set_volume((int)cdda_volume);
	d_msm->set_volume((int)adpcm_volume);
}

void PCE::cdrom_write(uint16_t addr, uint8_t data)
{
	touch_sound();
	switch(addr & 0x0f) {
	case 0x00:  /* CDC status */
		d_scsi_host->write_signal(SIG_SCSI_SEL, 1, 1);
		d_scsi_host->write_signal(SIG_SCSI_SEL, 0, 1);
		adpcm_dma_enabled = false;
		set_cdrom_irq_line(0x70, CLEAR_LINE);
		break;
		
	case 0x01:  /* CDC command / status / data */
		write_cdrom_data(data);
		break;
		
	case 0x02:  /* ADPCM / CD control / IRQ enable/disable */
		/* bit 6 - transfer ready irq */
		/* bit 5 - transfer done irq */
		/* bit 4 - BRAM irq? */
		/* bit 3 - ADPCM FULL irq */
		/* bit 2 - ADPCM HALF irq */
		if(data & 0x80) {
			set_ack();
		} else {
			clear_ack();
		}
		/* Update mask register now otherwise it won't catch the irq enable/disable change */
		cdrom_regs[0x02] = data;
		/* Don't set or reset any irq lines, but just verify the current state */
		set_cdrom_irq_line(0, 0);
		break;
		
	case 0x03:  /* BRAM lock / CD status / IRQ - Read Only register */
		break;
		
	case 0x04:  /* CD reset */
		if(data & 0x02) {
			// Reset ADPCM hardware
			reset_adpcm();
			set_cdrom_irq_line(0x70, CLEAR_LINE);
		}
		d_scsi_host->write_signal(SIG_SCSI_RST, data, 0x02);
		break;
		
	case 0x05:  /* Convert PCM data / PCM data */
	case 0x06:  /* PCM data */
		break;
		
	case 0x07:  /* BRAM unlock / CD status */
		if(data & 0x80) {
			backup_locked = false;
		}
		break;
		
	case 0x08:  /* ADPCM address (LSB) / CD data */
	case 0x09:  /* ADPCM address (MSB) */
		break;
		
	case 0x0a:  /* ADPCM RAM data port */
		if(adpcm_write_buf > 0) {
			adpcm_write_buf--;
		} else {
			write_adpcm_ram(data);
		}
		break;
		
	case 0x0b:  /* ADPCM DMA control */
		if(data & 3) {
			/* Start CD to ADPCM transfer */
			adpcm_dma_enabled = true;
			
			if(d_scsi_cdrom->get_cur_command() == SCSI_CMD_READ6 &&
			   d_scsi_host->read_signal(SIG_SCSI_BSY) != 0 &&
			   d_scsi_host->read_signal(SIG_SCSI_REQ) != 0 &&
			   d_scsi_host->read_signal(SIG_SCSI_CD ) == 0 &&
			   d_scsi_host->read_signal(SIG_SCSI_MSG) == 0 &&
			   d_scsi_host->read_signal(SIG_SCSI_IO ) != 0) {
				// already data is received, read first byte
				adpcm_do_dma();
			} else {
				cdrom_regs[0x0c] |= 0x04;
			}
		}
		break;
		
	case 0x0c:  /* ADPCM status */
		break;
		
	case 0x0d:  /* ADPCM address control */
		if((cdrom_regs[0x0d] & 0x80) && !(data & 0x80)) {
			// Reset ADPCM hardware
			reset_adpcm();
		}
		if(data & 0x02) {
			// ADPCM set write address
			adpcm_write_ptr = (cdrom_regs[0x09] << 8) | cdrom_regs[0x08];
			adpcm_write_buf = data & 1;
			adpcm_written = 0;
		}
		if(data & 0x08) {
			// ADPCM set read address
			adpcm_read_ptr = (cdrom_regs[0x09] << 8) | cdrom_regs[0x08];
			adpcm_read_buf = 2;
		}
		if(data & 0x10) {
			// ADPCM set length
			adpcm_length = (cdrom_regs[0x09] << 8) | cdrom_regs[0x08];
		}
		if((data & 0x40) && ((cdrom_regs[0x0D] & 0x40) == 0)) {
			// ADPCM play
			msm_start_addr = (adpcm_read_ptr) & 0xffff;
			msm_end_addr = (adpcm_read_ptr + adpcm_length) & 0xffff;
			msm_half_addr = (adpcm_read_ptr + (adpcm_length / 2)) & 0xffff;
			adpcm_write_ptr &= 0xffff;
			msm_nibble = 0;
			adpcm_play();
			d_msm->reset_w(0);
		} else if ((data & 0x40) == 0) {
			// used by Buster Bros to cancel an in-flight sample
			// if repeat flag (bit5) is high, ADPCM should be fully played (from Ootake)
			if(!(data & 0x20)) {
				set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_HALF_PLAY, CLEAR_LINE);
				set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, CLEAR_LINE);
				adpcm_stop();
				d_msm->reset_w(1);
			}
		}
		break;
		
	case 0x0e:  /* ADPCM playback rate */
		adpcm_clock_divider = 0x10 - (data & 0x0f);
		d_msm->change_clock_w((ADPCM_CLOCK / 6) / adpcm_clock_divider);
		break;
		
	case 0x0f:  /* ADPCM and CD audio fade timer */
		if(cdrom_regs[0x0f] != data) {
			switch(data & 0x0f) {
			case 0x00: // CD-DA / ADPCM enable (100 msecs)
				cdda_fade_in(100);
				adpcm_fade_in(100);
				break;
			case 0x01: // CD-DA enable (100 msecs)
				cdda_fade_in(100);
				break;
			case 0x08: // CD-DA short (1500 msecs) fade out / ADPCM enable
			case 0x0c: // CD-DA short (1500 msecs) fade out / ADPCM enable
				cdda_fade_out(1500);
				adpcm_fade_in(100);
				break;
			case 0x09: // CD-DA long (5000 msecs) fade out
				cdda_fade_out(5000);
				break;
			case 0x0a: // ADPCM long (5000 msecs) fade out
				adpcm_fade_out(5000);
				break;
			case 0x0d: // CD-DA short (1500 msecs) fade out
				cdda_fade_out(1500);
				break;
			case 0x0e: // ADPCM short (1500 msecs) fade out
				adpcm_fade_out(1500);
				break;
			}
		}
		break;
	}
	cdrom_regs[addr & 0x0f] = data;
}

uint8_t PCE::cdrom_read(uint16_t addr)
{
	// System 3 Card header handling
	if((addr & 0xc0) == 0xc0) {
		switch(addr & 0xcf) {
		case 0xc1: return 0xaa;
		case 0xc2: return 0x55;
		case 0xc3: return 0x00;
		case 0xc5: return 0xaa;
		case 0xc6: return 0x55;
		case 0xc7: return 0x03;
		}
	}
	uint8_t data = cdrom_regs[addr & 0x0f];
	
	switch(addr & 0x0f) {
	case 0x00:  /* CDC status */
		data = 0;
#ifdef _PCENGINE
		if(d_cpu->get_pc() == 0xf34b) {
			// XXX: Hack to wait the CD-DA will be finished for the Manhole
			data |= d_scsi_cdrom->read_signal(SIG_SCSI_CDROM_PLAYING) ? 0x80 : 0;
		}
#endif
		data |= d_scsi_host->read_signal(SIG_SCSI_BSY) ? 0x80 : 0;
		data |= d_scsi_host->read_signal(SIG_SCSI_REQ) ? 0x40 : 0;
		data |= d_scsi_host->read_signal(SIG_SCSI_MSG) ? 0x20 : 0;
		data |= d_scsi_host->read_signal(SIG_SCSI_CD ) ? 0x10 : 0;
		data |= d_scsi_host->read_signal(SIG_SCSI_IO ) ? 0x08 : 0;
		break;
		
	case 0x01:  /* CDC command / status / data */
	case 0x08:  /* ADPCM address (LSB) / CD data */
		{
			bool read6_data_in = false;
			if(d_scsi_cdrom->get_cur_command() == SCSI_CMD_READ6 &&
			   d_scsi_host->read_signal(SIG_SCSI_BSY) != 0 &&
			   d_scsi_host->read_signal(SIG_SCSI_REQ) != 0 &&
			   d_scsi_host->read_signal(SIG_SCSI_CD ) == 0 &&
			   d_scsi_host->read_signal(SIG_SCSI_MSG) == 0 &&
			   d_scsi_host->read_signal(SIG_SCSI_IO ) != 0) {
				// read6 command, data in phase
				read6_data_in = true;
			}
			data = read_cdrom_data();
			
			if(read6_data_in) {
				// set ack automatically and immediately for correct transfer speed
				set_ack();
				
				// XXX: Hack to wait until next REQ signal is raised
				// because PCE does not check REQ signal before reads next byte
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
			}
		}
		break;
		
	case 0x02:  /* ADPCM / CD control */
		break;
		
	case 0x03:  /* BRAM lock / CD status */
		// from Ootake
		backup_locked = true;
		data |= PCE_CD_IRQ_BRAM;
		cdrom_regs[3] ^= 0x02;
		if(cdrom_regs[2] == 0) {
			cdrom_regs[3] &= 0x02;
		}
		set_cdrom_irq_line(0, 0);
		break;
		
	case 0x04:  /* CD reset */
		break;
		
	case 0x05:  /* Convert PCM data / PCM data */
		data = (d_scsi_cdrom->read_signal((cdrom_regs[3] & 0x02) ? SIG_SCSI_CDROM_SAMPLE_L : SIG_SCSI_CDROM_SAMPLE_R) >> 0) & 0xff;
		break;
		
	case 0x06:  /* PCM data */
		data = (d_scsi_cdrom->read_signal((cdrom_regs[3] & 0x02) ? SIG_SCSI_CDROM_SAMPLE_L : SIG_SCSI_CDROM_SAMPLE_R) >> 8) & 0xff;
		break;
		
	case 0x07:  /* BRAM unlock / CD status */
		data = (backup_locked ? (data & 0x7f) : (data | 0x80));
		break;
		
	case 0x0a:  /* ADPCM RAM data port */
		if(adpcm_read_buf > 0) {
			adpcm_read_buf--;
			data = 0x00;
		} else {
			data = read_adpcm_ram();
		}
		break;
		
	case 0x0b:  /* ADPCM DMA control */
		break;
		
	case 0x0c:  /* ADPCM status */
		break;
		
	case 0x09:  /* ADPCM address (MSB) */
	case 0x0d:  /* ADPCM address control */
	case 0x0e:  /* ADPCM playback rate */
	case 0x0f:  /* ADPCM and CD audio fade timer */
		data = 0;
		break;
	}
	return data;
}

void PCE::write_cdrom_data(uint8_t data)
{
	d_scsi_host->write_dma_io8(0, data);
}

uint8_t PCE::read_cdrom_data()
{
	return d_scsi_host->read_dma_io8(0);
}

void PCE::reset_adpcm()
{
	// reset ADPCM hardware
	adpcm_read_ptr = adpcm_write_ptr = 0;
	msm_start_addr = msm_end_addr = msm_half_addr = 0;
	msm_nibble = 0;
	adpcm_stop();
	d_msm->reset_w(1);
	
	// stop ADPCM dma
	adpcm_dma_enabled = false;
}

void PCE::write_adpcm_ram(uint8_t data)
{
	adpcm_ram[(adpcm_write_ptr++) & 0xffff] = data;
}

uint8_t PCE::read_adpcm_ram()
{
	return adpcm_ram[(adpcm_read_ptr++) & 0xffff];
}

void PCE::adpcm_do_dma()
{
	write_adpcm_ram(read_cdrom_data());
	adpcm_written++;
	set_ack();
	cdrom_regs[0x0c] &= ~0x04;
}

void PCE::adpcm_play()
{
	touch_sound();
	cdrom_regs[0x0c] &= ~PCE_CD_ADPCM_STOP_FLAG;
	cdrom_regs[0x0c] |= PCE_CD_ADPCM_PLAY_FLAG;
	set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, CLEAR_LINE);
	cdrom_regs[0x03] &= ~0x0c;
	msm_idle = 0;
}

void PCE::adpcm_stop()
{
	touch_sound();
	cdrom_regs[0x0c] |= PCE_CD_ADPCM_STOP_FLAG;
	cdrom_regs[0x0c] &= ~PCE_CD_ADPCM_PLAY_FLAG;
	cdrom_regs[0x0d] &= ~0x60;
	msm_idle = 1;
}

void PCE::set_ack()
{
	d_scsi_host->write_signal(SIG_SCSI_ACK, 1, 1);
}

void PCE::clear_ack()
{
	if(d_scsi_host->read_signal(SIG_SCSI_CD) != 0) {
		cdrom_regs[0x0b] &= 0xfc;
	}
	d_scsi_host->write_signal(SIG_SCSI_ACK, 0, 0);
}

void PCE::set_cdrom_irq_line(int num, int state)
{
	if (state == ASSERT_LINE) {
		cdrom_regs[0x03] |= num;
	} else {
		cdrom_regs[0x03] &= ~num;
	}
	if (cdrom_regs[0x02] & cdrom_regs[0x03] & 0x7c) {
		d_cpu->write_signal(INPUT_LINE_IRQ2, ASSERT_LINE, 0);
	} else {
		d_cpu->write_signal(INPUT_LINE_IRQ2, CLEAR_LINE, 0);
	}
}

void PCE::cdda_fade_in(int time)
{
	if(event_cdda_fader != -1) {
		cancel_event(this, event_cdda_fader);
	}
	register_event(this, EVENT_CDDA_FADE_IN, time, true, &event_cdda_fader);
	d_scsi_cdrom->set_volume((int)(cdda_volume = 0.0));
}

void PCE::cdda_fade_out(int time)
{
	if(event_cdda_fader != -1) {
		cancel_event(this, event_cdda_fader);
	}
	register_event(this, EVENT_CDDA_FADE_OUT, time, true, &event_cdda_fader);
	d_scsi_cdrom->set_volume((int)(cdda_volume = 100.0));
}

void PCE::adpcm_fade_in(int time)
{
	if(event_adpcm_fader != -1) {
		cancel_event(this, event_adpcm_fader);
	}
	register_event(this, EVENT_ADPCM_FADE_IN, time, true, &event_adpcm_fader);
	d_msm->set_volume((int)(adpcm_volume = 0.0));
}

void PCE::adpcm_fade_out(int time)
{
	if(event_adpcm_fader != -1) {
		cancel_event(this, event_adpcm_fader);
	}
	register_event(this, EVENT_ADPCM_FADE_OUT, time, true, &event_adpcm_fader);
	d_msm->set_volume((int)(adpcm_volume = 100.0));
}

void PCE::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_PCE_SCSI_IRQ:
		if(data & mask) {
			if(!irq_status) {
				irq_status = true;
				
				if(d_scsi_host->read_signal(SIG_SCSI_BSY) != 0 &&
				   d_scsi_host->read_signal(SIG_SCSI_CD ) != 0 &&
				   d_scsi_host->read_signal(SIG_SCSI_MSG) == 0 &&
				   d_scsi_host->read_signal(SIG_SCSI_IO ) != 0) {
					// status phase, command is finished
					set_cdrom_irq_line(PCE_CD_IRQ_TRANSFER_READY, CLEAR_LINE);
					set_cdrom_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE);
				}
				// clear busreq because next REQ signal is raised
				d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
			}
		} else {
			if(irq_status) {
				irq_status = false;
			}
		}
		break;
		
	case SIG_PCE_SCSI_DRQ:
		if(data & mask) {
			if(!drq_status) {
				drq_status = true;
				set_cdrom_irq_line(PCE_CD_IRQ_TRANSFER_READY, ASSERT_LINE);
				
				// clear busreq because next REQ signal is raised
				d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
				
				if(adpcm_dma_enabled) {
					if(!msm_idle && adpcm_write_ptr >= msm_start_addr) {
						// now streaming, wait dma not to overwrite buffer before it is played
					} else {
						adpcm_do_dma();
					}
				}
			}
		} else {
			if(drq_status) {
				drq_status = false;
				
				if(d_scsi_cdrom->get_cur_command() == SCSI_CMD_READ6 &&
				   d_scsi_host->read_signal(SIG_SCSI_BSY) != 0 &&
				   d_scsi_host->read_signal(SIG_SCSI_CD ) == 0 &&
				   d_scsi_host->read_signal(SIG_SCSI_MSG) == 0 &&
				   d_scsi_host->read_signal(SIG_SCSI_IO ) != 0) {
					// clear ack automatically and immediately for correct transfer speed
					clear_ack();
				}
			}
		}
		break;
		
	case SIG_PCE_SCSI_BSY:
		if(!(data & mask)) {
			// bus free
			set_cdrom_irq_line(PCE_CD_IRQ_TRANSFER_READY, CLEAR_LINE);
			set_cdrom_irq_line(PCE_CD_IRQ_TRANSFER_DONE, CLEAR_LINE);
		}
		break;
		
	case SIG_PCE_CDDA_DONE:
		if(data & mask) {
			touch_sound();
			set_cdrom_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE);
		}
		break;
		
	case SIG_PCE_ADPCM_VCLK:
		// Callback for new data from the MSM5205.
		// The PCE cd unit actually divides the clock signal supplied to
		// the MSM5205. Currently we can only use static clocks for the
		// MSM5205.
		if(!msm_idle) {
			uint8_t msm_data = (msm_nibble) ? (adpcm_ram[msm_start_addr & 0xffff] & 0x0f) : ((adpcm_ram[msm_start_addr & 0xffff] & 0xf0) >> 4);
			d_msm->data_w(msm_data);
			msm_nibble ^= 1;
			
			if(msm_nibble == 0) {
				adpcm_written--;
				if(adpcm_dma_enabled && adpcm_written == 0) {
					// finish streaming when all samples are played
					set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_HALF_PLAY, CLEAR_LINE);
					set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, ASSERT_LINE);
					adpcm_stop();
					d_msm->reset_w(1);
				} else if((msm_start_addr & 0xffff) == msm_half_addr) {
					// reached to half address
					set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, CLEAR_LINE);
					set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_HALF_PLAY, ASSERT_LINE);
				} else if((msm_start_addr & 0xffff) == msm_end_addr) {
					// reached to end address
					if(adpcm_dma_enabled && adpcm_length == 0xffff) {
						// restart streaming
						set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_HALF_PLAY, CLEAR_LINE);
						set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, CLEAR_LINE);
					} else {
						// stop playing adpcm
						set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_HALF_PLAY, CLEAR_LINE);
						set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, ASSERT_LINE);
						adpcm_stop();
						d_msm->reset_w(1);
					}
				}
				msm_start_addr++;
				
				if(adpcm_dma_enabled) {
					if(!msm_idle && adpcm_write_ptr < msm_start_addr) {
						if(d_scsi_cdrom->get_cur_command() == SCSI_CMD_READ6 &&
						   d_scsi_host->read_signal(SIG_SCSI_BSY) != 0 &&
						   d_scsi_host->read_signal(SIG_SCSI_REQ) != 0 &&
						   d_scsi_host->read_signal(SIG_SCSI_CD ) == 0 &&
						   d_scsi_host->read_signal(SIG_SCSI_MSG) == 0 &&
						   d_scsi_host->read_signal(SIG_SCSI_IO ) != 0) {
							// already data is received, read next byte
							adpcm_do_dma();
						}
					}
				}
			}
		}
		break;
	}
}

void PCE::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_CDDA_FADE_IN:
		if((cdda_volume += 0.1) >= 100.0) {
			cancel_event(this, event_cdda_fader);
			event_cdda_fader = -1;
			cdda_volume = 100.0;
		}
		d_scsi_cdrom->set_volume((int)cdda_volume);
		break;
		
	case EVENT_CDDA_FADE_OUT:
		if((cdda_volume -= 0.1) <= 0) {
			cancel_event(this, event_cdda_fader);
			event_cdda_fader = -1;
			cdda_volume = 0.0;
		}
		d_scsi_cdrom->set_volume((int)cdda_volume);
		break;
		
	case EVENT_ADPCM_FADE_IN:
		if((adpcm_volume += 0.1) >= 100.0) {
			cancel_event(this, event_adpcm_fader);
			event_adpcm_fader = -1;
			adpcm_volume = 100.0;
		}
		d_msm->set_volume((int)adpcm_volume);
		break;
		
	case EVENT_ADPCM_FADE_OUT:
		if((adpcm_volume -= 0.1) <= 0) {
			cancel_event(this, event_adpcm_fader);
			event_adpcm_fader = -1;
			adpcm_volume = 0.0;
		}
		d_msm->set_volume((int)adpcm_volume);
		break;
	}
}
#endif

#define STATE_VERSION	5

void process_state_vdc(vdc_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->dvssr_write);
	state_fio->StateValue(val->physical_width);
	state_fio->StateValue(val->physical_height);
	state_fio->StateArray(val->sprite_ram, sizeof(val->sprite_ram), 1);
	state_fio->StateValue(val->curline);
	state_fio->StateValue(val->current_segment);
	state_fio->StateValue(val->current_segment_line);
	state_fio->StateValue(val->vblank_triggered);
	state_fio->StateValue(val->raster_count);
	state_fio->StateValue(val->satb_countdown);
	state_fio->StateArray(val->vram, sizeof(val->vram), 1);
	state_fio->StateValue(val->inc);
	state_fio->StateValue(val->vdc_register);
	state_fio->StateValue(val->vdc_latch);
	state_fio->StateArray(val->vdc_data, sizeof(val->vdc_data), 1);
	state_fio->StateValue(val->status);
	state_fio->StateValue(val->y_scroll);
}

void process_state_vce(vce_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->vce_control);
	state_fio->StateValue(val->vce_address);
	state_fio->StateArray(val->vce_data, sizeof(val->vce_data), 1);
	state_fio->StateValue(val->current_bitmap_line);
	state_fio->StateArray(&val->bmp[0][0], sizeof(val->bmp), 1);
	state_fio->StateArray(val->palette, sizeof(val->palette), 1);
}

void process_state_vpc(vpc_t* val, FILEIO* state_fio)
{
	for(int i = 0; i < array_length(val->vpc_prio); i++) {
		state_fio->StateValue(val->vpc_prio[i].prio);
		state_fio->StateValue(val->vpc_prio[i].vdc0_enabled);
		state_fio->StateValue(val->vpc_prio[i].vdc1_enabled);
	}
	state_fio->StateArray(val->prio_map, sizeof(val->prio_map), 1);
	state_fio->StateValue(val->priority);
	state_fio->StateValue(val->window1);
	state_fio->StateValue(val->window2);
	state_fio->StateValue(val->vdc_select);
}

void process_state_psg(psg_t* val, FILEIO* state_fio)
{
	state_fio->StateArray(val->regs, sizeof(val->regs), 1);
	state_fio->StateArray(val->wav, sizeof(val->wav), 1);
	state_fio->StateValue(val->wavptr);
	state_fio->StateValue(val->genptr);
	state_fio->StateValue(val->remain);
	state_fio->StateValue(val->noise);
	state_fio->StateValue(val->randval);
}

bool PCE::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(support_6btn_pad);
	state_fio->StateValue(support_multi_tap);
#ifdef SUPPORT_SUPER_GFX
	state_fio->StateValue(support_sgfx);
#endif
#ifdef SUPPORT_CDROM
	state_fio->StateValue(support_cdrom);
#endif
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(cart + 0x80000, 0x80000, 1);
#ifdef SUPPORT_BACKUP_RAM
	state_fio->StateArray(backup, sizeof(backup), 1);
	state_fio->StateValue(backup_crc32);
#endif
	state_fio->StateValue(bank);
	state_fio->StateValue(buffer);
	state_fio->StateValue(prev_width);
	state_fio->StateValue(inserted);
	for(int i = 0; i < array_length(vdc); i++) {
		process_state_vdc(&vdc[i], state_fio);
	}
	process_state_vce(&vce, state_fio);
	process_state_vpc(&vpc, state_fio);
	for(int i = 0; i < array_length(psg); i++) {
		process_state_psg(&psg[i], state_fio);
	}
	state_fio->StateValue(psg_ch);
	state_fio->StateValue(psg_vol);
	state_fio->StateValue(psg_lfo_freq);
	state_fio->StateValue(psg_lfo_ctrl);
	state_fio->StateValue(joy_counter);
	state_fio->StateValue(joy_high_nibble);
	state_fio->StateValue(joy_second_byte);
#ifdef SUPPORT_CDROM
	state_fio->StateArray(cdrom_ram, sizeof(cdrom_ram), 1);
	state_fio->StateArray(cdrom_regs, sizeof(cdrom_regs), 1);
	state_fio->StateValue(backup_locked);
	state_fio->StateValue(irq_status);
	state_fio->StateValue(drq_status);
	state_fio->StateArray(adpcm_ram, sizeof(adpcm_ram), 1);
	state_fio->StateValue(adpcm_read_ptr);
	state_fio->StateValue(adpcm_write_ptr);
	state_fio->StateValue(adpcm_written);
	state_fio->StateValue(adpcm_length);
	state_fio->StateValue(adpcm_clock_divider);
	state_fio->StateValue(adpcm_read_buf);
	state_fio->StateValue(adpcm_write_buf);
	state_fio->StateValue(adpcm_dma_enabled);
	state_fio->StateValue(msm_start_addr);
	state_fio->StateValue(msm_end_addr);
	state_fio->StateValue(msm_half_addr);
	state_fio->StateValue(msm_nibble);
	state_fio->StateValue(msm_idle);
	state_fio->StateValue(cdda_volume);
	state_fio->StateValue(adpcm_volume);
	state_fio->StateValue(event_cdda_fader);
	state_fio->StateValue(event_adpcm_fader);
#endif
	return true;
}

