
#include "towns_cdrom.h"

namespace FMTOWNS {

void CDC::reset()
{
	dma_fifo->clear();
	param_fifo->clear();
	stat_fifo->clear();

	has_status = false;
	extra_status = 0;
	submpu_ready = true;
	software_transfer_phase = false;

	write_signals(&output_submpu_intr, 0x00000000);
	write_signals(&output_dma_intr, 0x00000000);
	
	dma_intr = false;
	submpu_intr = false;

}

void CDC::initialize()
{
	dma_fifo->clear();
	param_fifo->clear();
	stat_fifo->clear();
	
	subq_fifo->clear();
	submpu_ready = true;

	submpu_intr_mask = false;
	dma_intr_mask = false;
	memset(w_regs, 0x00, sizeof(w_regs));
	
}

void CDC::write_io8(uint32_t address, uint32_t data)
{
	/*
	 * 04C0h : Master control register
	 * 04C2h : Command register
	 * 04C4h : Parameter register
	 * 04C6h : Transfer control register.
	 */
	
	switch(address & 0x0f) {
	case 0x00: // Master control register
		{
			if((data & 0x80) != 0) {
				if(submpu_intr) write_signals(&output_submpu_intr, 0x00000000);
				submpu_intr = false;
			}
			if((data & 0x40) != 0) {
				if(dma_intr) write_signals(&output_dma_intr, 0x00000000);
				dma_intr = false;
			}
			if((data & 0x04) != 0) this->reset();
			submpu_intr_mask = ((data & 0x02) != 0) ? true : false;
			dma_intr_mask    = ((data & 0x01) != 0) ? true : false;
			w_regs[address & 0x0f] = data;
		}
		break;
	case 0x02: // Command register
		{
			command_type_play = ((data & 0x80) != 0) ? false : true; // false = status command
			stat_reply_intr   = ((data & 0x40) != 0) ? true : false;
			req_status        = ((data & 0x20) != 0) ? true : false;
			if(command_type_play) {
				enqueue_command_play(data);
			} else {
				enqueue_command_status(data);
			}
			w_regs[address & 0x0f] = data;
		}
		break;
	case 0x04: // Parameter register
		{
			if(param_fifo->full()) {
				param_fifo->read(); // Dummy read
			}
			param_fifo->write((int)(data & 0xff));
			w_regs[address & 0x0f] = data;
		}
		break;
	case 0x06:
		{
			dma_transfer = ((data & 0x10) != 0) ? true : false;
			pio_transfer = ((data & 0x08) != 0) ? true : false;
			w_regs[address & 0x0f] = data;
		}			
		break;
	default:
		if((addr & 0x01) == 0) {
			w_regs[address & 0x0f] = data;
		}
		break;
	}
}
	
uint32_t CDC::read_io8(uint32_t address)
{
	/*
	 * 04C0h : Master status register
	 */
	uint32_t val = 0xff;
	switch(addr & 0x0f) {
	case 0x0: //Master status
		{
			val = 0x00;
			val = val | ((submpu_intr) ?             0x80 : 0x00);
			val = val | ((dma_intr) ?                0x40 : 0x00);
			val = val | ((software_transfer_phase) ? 0x20 : 0x00);
			val = val | ((d_dmac->read_signal(SIG_UPD71071_IS_TRANSFERING + 3) !=0) ? 0x10 : 0x00); // USING DMAC ch.3
			val = val | ((has_status)              ? 0x02 : 0x00);
			val = val | ((submpu_ready)            ? 0x01 : 0x00);
		}
		break;
	case 0x2: // Status register
		val = (uint32_t)(stat_fifo->read() & 0xff);
		if(stat_fifo->empty()) {
			has_status = false;
			if(extra_status != 0) {
				uint8_t cmd = w_regs[0x02];
				switch(cmd & 0x9f) {
				case 0x00: // Seek
					write_status(0x04, 0x00, 0x00, 0x00);
					extra_status = 0;
					break;
				case 0x02: // Read
					if(extra_status == 2) {
						write_status(0x22, 0x00, 0x00, 0x00);
					}
					extra_status = 0;
					break;
				case 0x04: // PLAY CDDA
					write_status(0x07, 0x00, 0x00, 0x00);
					has_status = false;
					extra_status = 0;
					break;
				case 0x05:
					{
						switch(extra_status) {
						case 1:
							write_status(0x16, 0x00, 0xa0, 0x00);
							extra_status++;
							break;
						case 2: // st1 = first_track_number
							write_status(0x17, TO_BCD(0x01), 0x00, 0x00);
							extra_status++;
							break;
						case 3:
							write_status(0x16, 0x00, 0xa1, 0x00);
							extra_status++;
							break;
						case 4: 
							write_status(0x17, d_cdrom->read_signal(SIG_TOWNS_CDROM_MAX_TRACK), 0x00, 0x00);
							extra_status++;
							break;
						case 5:
							write_status(0x16, 0x00, 0xa2, 0x00);
							extra_status++;
							break;
						case 6:
							uint32_t msf = d_cdrom->read_signal
							{
								uint32_t msf = d_cdrom->read_signal(SIG_TOWNS_CDROM_START_MSF_AA);
								write_status(0x17, (msf & 0x00ff0000) >> 16, (msf & 0x0000ff00) >> 8, msf & 0x000000ff);
								exra_status++;
							}
							break;
						default:
							if(extra_status == 7) {
								d_cdrom->write_signal(SIG_TOWNS_CDROM_SET_STAT_TRACK, 0x01, 0x01);
							}
							if((extra_status & 0x01) != 0) {
								uint32_t adr_control = d_cdrom->read_signal(SIG_TOWNS_CDROM_GET_ADR);
								write_status(0x16, ((adr_control & 0x0f) << 4) | ((adr_control >> 4) & 0x0f), TO_BCD((extra_status / 2) - 2), 0x00);
								extra_status++;
							} else {
								uint32_t msf = d_cdrom->read_signal(SIG_TOWNS_CDROM_START_MSF);
								write_status(0x17, (msf & 0x00ff0000) >> 16, (msf & 0x0000ff00) >> 8, msf & 0x000000ff);
								if(d_cdrom->read_signal(SIG_TOWNS_CDROM_REACHED_MAX_TRACK) == 0){
									extra_status++;
								} else {
									extra_status = 0;
								}
							}
							break;
						}
					}
				case 0x06: // CDDA status
					{
						switch(extra_status) {
						case 1: // Get current track
							write_status(0x18, 0x00, d_cdrom->read_signal(SIG_TOWNS_CDROM_CURRENT_TRACK), 0x00);
							extra_status++;
							break;
						case 2: // Get current position
							{
								uint32_t msf = d_cdrom->read_signal(SIG_TOWNS_CDROM_RELATIVE_MSF);
								write_status(0x19, (msf >> 16) & 0xff, (msf >> 8) & 0xff, msf & 0xff);
								extra_status++;
							}
							break;
						case 3: // Current_msf
							{
								uint32_t msf = d_cdrom->read_signal(SIG_TOWNS_CDROM_ABSOLUTE_MSF);
								write_status(0x19, 0x00, (msf >> 16) & 0xff, (msf >> 8) & 0xff);
								extra_status++;
							}
							break;
						case 4:
							{
								uint32_t msf = d_cdrom->read_signal(SIG_TOWNS_CDROM_ABSOLUTE_MSF);
								write_status(0x19, msf & 0xff, 0x00, 0x00);
								extra_status = 0;
							}
							break;
						}
						break;
		
						}
				case 0x84:
					write_status(0x11, 0x00, 0x00, 0x00);
					extra_status = 0;
					break;
				}
			}
		}
		break;
	case 0x4: //
		if(pio_transfer) {
			val = data_reg;
		}
		break;
	case 0xc: // Sub code status register
		val = 0x00;
		val = val | (subq_fifo->empty()) ? 0x00 : 0x01;
		val = val | ((d_cdrom->read_signal(SIG_TOWNS_CDROM_SUBQ_OVERRUN) != 0) ? 0x02 : 0x00);
		break;
	case 0xd:
		val = (uint32_t)(subq_fifo->read() & 0xff);
		break;
	}
	return val;
}

void CDC::read_cdrom(bool req_reply)
{
	uint8_t* command = d_cdrom->command;
	extra_status = 0;
	if(!(d_cdrom->is_device_ready())) {
		if(req_reply) write_status(0x10, 0x00, 0x00, 0x00);
		return;
	}
	if(param_fifo->count() < 6) {
		// Error
		return;
	}

	uint8_t m1, s1, f1;
	uint8_t m2, s2, f2;
	f2 = (uint8_t)(param_fifo->read() & 0xff);
	s2 = (uint8_t)(param_fifo->read() & 0xff);
	m2 = (uint8_t)(param_fifo->read() & 0xff);

	f1 = (uint8_t)(param_fifo->read() & 0xff);
	s1 = (uint8_t)(param_fifo->read() & 0xff);
	m1 = (uint8_t)(param_fifo->read() & 0xff);

	uint32_t lba1 = ((uint32_t)m1 & 0x1f) * 0x10000 + ((uint32_t)s1) * 0x100 + (uint32_t)f1;
	uint32_t lba2 = ((uint32_t)m2 & 0x1f) * 0x10000 + ((uint32_t)s2) * 0x100 + (uint32_t)f2;
	uint32_t __remain;
	int track = get_track(lba1);
	if(track < 2) {
		if(lba1 >= 150) {
			lba1 = lba1 - 150;
		} else {
			lba1 = 0;
		}
		if(lba2 >= 150) {
			lba2 = lba2 - 150;
		} else {
			lba2 = 0;
		}
	}
	set_cdda_status(CDDA_OFF);
	if(lba1 > lba2) { // NOOP?
		extra_status = 0;
		write_status(0x01, 0x00, 0x00, 0x00);
		return;
	}
	__remain = lba2 - lba1;
	seek_time = get_seek_time(lba1);
	
	command[0] = SCSI_CMD_READ12;
	command[1] = 0; // LUN = 0
	command[2] = 0; 
	command[3] = m2 & 0x1f;
	command[4] = s2;
	command[5] = f2;
	
	command[6] = 0;
	command[7] = (uint8_t)((__remain / 0x10000) & 0xff);
	command[8] = (uint8_t)((__remain / 0x100) & 0xff);
	command[9] = (uint8_t) (__remain % 0x100);

	if(req_reply) {
		extra_status = 2;
		write_status(0x00, 0x00, 0x00, 0x00);
	} else {
		extra_status = 0;
		if(pio_transfer) {
			write_status(0x21, 0x00, 0x00, 0x00);
		} else {
			write_status(0x22, 0x00, 0x00, 0x00);
		}
	}
	dma_fifo->clear(); // OK?
	d_cdrom->start_command();
}	

void CDC::play_cdda(bool req_reply)
{
	uint8_t* command = d_cdrom->command;
	if(!(d_cdrom->is_device_ready())) {
		if(req_reply) write_status(0x10, 0x00, 0x00, 0x00);
		return;
	}
	if(param_fifo->count() < 6) {
		// Error
		return;
	}
	command[0] = TOWNS_CDROM_CDDA_PLAY;
	command[1] = 0;
	command[2] = 0;
	command[3] = (uint8_t)(command_queue->read() & 0xff); 
	command[4] = (uint8_t)(command_queue->read() & 0xff); 
    commadn[5] = (uint8_t)(command_queue->read() & 0xff); 
	command[6] = 0;
	command[7] = (uint8_t)(command_queue->read() & 0xff); 
	command[8] = (uint8_t)(command_queue->read() & 0xff); 
	command[9] = (uint8_t)(command_queue->read() & 0xff);

	
	if(req_reply) {
		extra_status = 1;
		write_status(0x00, 0x03, 0x00, 0x00);
	}
	dma_fifo->clear(); // OK?
	d_cdrom->start_command();

}

void CDC::write_status(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	has_status = true;
	stat_fifo->clear();
	stat_fifo->write(a);
	stat_fifo->write(b);
	stat_fifo->write(c);
	stat_fifo->write(d);
	if(stat_reply_intr) {
		if(!(submpu_intr_mask)) {
			write_signals(&output_submpu_intr, 0xffffffff);
		}
		submpu_intr = true;
	}
}

void CDC::enqueue_command_play(uint8_t cmd)
{
	//write_signals(&output_submpu_intr, 0x00000000);
	if((d_cdrom->read_signal(SIG_TOWNS_CDROM_IS_MEDIA_INSERTED) == 0x00000000) && (cmd != 0xa0)) { // Not Inserted
		if(req_status) {
			extra_status = 0;
			write_status(0x10, 0x00, 0x00, 0x00);
		}
	} else {
		has_status = false;
		switch(cmd & 0x1f) {
		case 0x00: // SEEK
			if(req_status) {
				extra_status = 1;
				write_status(0x00, 0x00, 0x00, 0x00);
			}
			// ToDo: REAL SEEK
			break;
		case 0x01: // Unknown (from MAME)
			if(req_status) {
				extra_status = 0;
				write_status(0x00, 0xff, 0xff, 0xff);
			}
			break;
		case 0x02: // READ (Mode1)
			read_cdrom(req_status);
			break;
		case 0x04: // PLAY CDDA
			play_cdda(req_status);
			break;
		case 0x05: // Read TOC
			if(req_status) {
				extra_status = 1;
				write_status(0x00, 0x00, 0x00, 0x00);
			} else {
				extra_status = 2;
				write_status(0x16, 0x00, 0xa0, 0x00);
			}
			break;
		case 0x06: // CD-DA Stats (?)
			extra_status = 1;
			write_status(0x00, 0x00, 0x00, 0x00);
			break;
		case 0x1f: // ??
			extra_status = 0;
			write_status(0x00, 0x00, 0x00, 0x00);
			break;
		default: // Illegal command
			extra_status = 0;
			write_status(0x10, 0x00, 0x00, 0x00);
			break;
		}
	}
}

void CDC::enqueue_command_status(uint8_t cmd)
{
	//write_signals(&output_submpu_intr, 0x00000000);
	if((d_cdrom->read_signal(SIG_TOWNS_CDROM_IS_MEDIA_INSERTED) == 0x00000000) && (cmd != 0xa0)) { // Not Inserted
		if(req_status) {
			extra_status = 0;
			write_status(0x10, 0x00, 0x00, 0x00);
		}
	} else {
		has_status = false;
		switch(cmd & 0x1f) {
		case 0x00: // set state
			if(req_status) {
				extra_status = 0;
				if(d_cdrom->read_signal(SIG_SCSI_CDROM_PLAYING) != 0) { // Active() && !(paused)
					write_status(0x00, 0x03, 0x00, 0x00);
				} else {
					write_status(0x00, 0x01, 0x00, 0x00);
				}
			}
			break;
		case 0x01: // set state (CDDASET)
			if(req_status) {
				extra_status = 0;
				write_status(0x00, 0x00, 0x00, 0x00);
			}
			break;
		case 0x04: // STOP CDDA
			if(req_status) {
				extra_status = 0;
				write_status(0x00, 0x00, 0x00, 0x00);
			}
			d_cdrom->write_signal(SIG_SCSI_CDROM_CDDA_STOP, 0xffffffff, 0xffffffff);
			break;
		case 0x05: // STOP CDDA (Difference from $84?)
			if(req_status) {
				extra_status = 0;
				write_status(0x00, 0x00, 0x00, 0x00);
			}
			d_cdrom->write_signal(SIG_SCSI_CDROM_CDDA_PAUSE, 0xffffffff, 0xffffffff);
			break;
		case 0x07: // UNPAUSE CDDA
			if(req_status) {
				extra_status = 0;
				write_status(0x00, 0x03, 0x00, 0x00);
			}
			d_cdrom->write_signal(SIG_SCSI_CDROM_CDDA_PAUSE, 0x00000000, 0xffffffff);
			break;
		default: // Illegal command
			extra_status = 0;
			write_status(0x10, 0x00, 0x00, 0x00);
			break;
		}
	}
}

void CDC::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_TOWNS_CDC_SET_DATA:
		data_reg = data;
		if(dma_fifo->full()) {
			dma_fifo->read();
		}
		dma_fifo->write(data & 0xff);
		break;
	case SIG_TOWNS_CDC_DMA_DONE:
		dma_intr = (data & mask) != 0) ? true : false;
		if(!(dma_intr_mask)) {
			write_signals(&output_dma_intr, (dma_intr) ? 0xffffffff : 0x00000000);
		}
		break;
	case SIG_TOWNS_CDC_RESET_FIFO:
		dma_fifo->reset();
		break;
	case SIG_TOWNS_CDC_SET_SUBQ:
		if(subq_fifo->full()) {
			subq_fifo->read();
		}
		subq_fifo->write(data & 0xff);
		break;
	case SIG_TOWNS_CDC_CLEAR_SUBQ:
		subq_fifo->clear();
		break;
	}
}

uint32_t CDC::read_dma_io8(uint32_t addr)
{
	if((addr & 0x01) == 0) {
		return (uint32_t)(dma_fifo->read() & 0xff);
	}
	return 0xff; // Noop
}

uint32_t CDC::read_dma_io16(uint32_t addr)
{
	if((addr & 0x01) == 0) {
		pair16_t d;
		d.b.l = dma_fifo->read() & 0xff;
		d.b.h = dma_fifo->read() & 0xff;
		return (uint32_t)(d.u16);
	}
	return 0xffff; // Noop
}


}
