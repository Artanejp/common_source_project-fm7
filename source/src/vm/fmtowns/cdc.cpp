/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[FM-Towns CD Controller]
*/

#include "cdc.h"
#include "../../fifo.h"
#include "../scsi_host.h"
#include "../upd71071.h"
#include "towns_cdrom.h"

namespace FMTOWNS {

// SAME AS SCSI_CDROM::
#define CDDA_OFF	0
#define CDDA_PLAYING	1
#define CDDA_PAUSED	2

void CDC::set_context_scsi_host(SCSI_HOST* dev)
{
	d_scsi_host = dev;
	d_scsi_host->set_context_irq(this, SIG_TOWNS_CDC_IRQ, 0xffffffff);
	d_scsi_host->set_context_drq(this, SIG_TOWNS_CDC_DRQ, 0xffffffff);
	d_scsi_host->set_context_bsy(this, SIG_TOWNS_CDC_BSY, 0xffffffff);
	d_scsi_host->set_context_cd(this, SIG_TOWNS_CDC_CD, 0xffffffff);
	d_scsi_host->set_context_io(this, SIG_TOWNS_CDC_IO, 0xffffffff);
	d_scsi_host->set_context_msg(this, SIG_TOWNS_CDC_MSG, 0xffffffff);
	d_scsi_host->set_context_req(this, SIG_TOWNS_CDC_REQ, 0xffffffff);
	d_scsi_host->set_context_ack(this, SIG_TOWNS_CDC_ACK, 0xffffffff);
}


void CDC::set_context_cdrom(TOWNS_CDROM* dev)
{
	d_cdrom = dev;
	dev->set_context_done(this, SIG_TOWNS_CDC_CDROM_DONE, 0xffffffff);
}

void CDC::reset()
{
	param_fifo->clear();
	stat_fifo->clear();

	has_status = false;
	extra_status = 0;
	submpu_ready = true;
	software_transfer_phase = false;

	write_signals(&output_submpu_intr, 0x00000000);
	write_signals(&output_dma_intr, 0x00000000);
	write_signals(&output_dma_line, 0x00000000);
	
	dma_intr = false;
	submpu_intr = false;
	
	dma_transfer = false;
	pio_transfer = true;
	command_type_play = false; // false = status command
	stat_reply_intr   = false;
	req_status        = false;
	
	d_scsi_host->reset();
}

void CDC::initialize()
{
	param_fifo = new FIFO(6); // 
	stat_fifo = new FIFO(4);
	
	submpu_ready = true;

	submpu_intr_mask = false;
	dma_intr_mask = false;
	memset(w_regs, 0x00, sizeof(w_regs));

	dma_transfer = false;
	pio_transfer = true;

	busy_status = false;
	cd_status = false;
	io_status = false;
	msg_status = false;
	req_status = false;
	ack_status = false;
}

void CDC::release()
{
	param_fifo->release();
	stat_fifo->release();
	
	delete param_fifo;
	delete stat_fifo;
}
void CDC::write_io8(uint32_t address, uint32_t data)
{
	/*
	 * 04C0h : Master control register
	 * 04C2h : Command register
	 * 04C4h : Parameter register
	 * 04C6h : Transfer control register.
	 */
	out_debug_log(_T("WRITE I/O: ADDR=%04X DATA=%02X"), address, data);
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
		if(submpu_ready) {
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
		if(submpu_ready) {
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
		if((address & 0x01) == 0) {
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
	switch(address & 0x0f) {
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
							{
								uint32_t msf = d_cdrom->read_signal(SIG_TOWNS_CDROM_START_MSF_AA);
								write_status(0x17, (msf & 0x00ff0000) >> 16, (msf & 0x0000ff00) >> 8, msf & 0x000000ff);
								extra_status++;
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
			val = d_scsi_host->read_dma_io8(0);
		}
		break;
	case 0xc: // Sub code status register
		val = d_cdrom->get_subq_status();
		break;
	case 0xd:
		val = d_cdrom->read_subq();
		break;
	}
//	out_debug_log(_T("READ I/O: ADDR=%04X VAL=%02X"), address, val);
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
	int track = 0;
	if(d_cdrom != NULL) {
		track = d_cdrom->get_track(lba1);
	}
	out_debug_log(_T("READ_CDROM TRACK=%d LBA1=%06X LBA2=%06X F1/S1/M1=%02X/%02X/%02X F2/S2/M2=%02X/%02X/%02X"), track, lba1, lba2, f1, s1, m1, f2, s2, m2);
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
	if(d_cdrom != NULL) {
		d_cdrom->set_cdda_status(CDDA_OFF);
	}
	if(lba1 > lba2) { // NOOP?
		extra_status = 0;
		write_status(0x01, 0x00, 0x00, 0x00);
		return;
	}
	__remain = lba2 - lba1;
	//seek_time = get_seek_time(lba1);
	
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
	submpu_ready = false;
	d_cdrom->start_command();
}	

void CDC::stop_cdda(bool req_reply)
{
	uint8_t* command = d_cdrom->command;
	if(!(d_cdrom->is_device_ready())) {
		if(req_reply) write_status(0x10, 0x00, 0x00, 0x00);
		return;
	}
	command[0] = TOWNS_CDROM_CDDA_STOP;
	command[1] = 0;
	command[2] = 0;
	command[3] = (uint8_t)(param_fifo->read() & 0xff); 
	command[4] = (uint8_t)(param_fifo->read() & 0xff); 
	command[5] = (uint8_t)(param_fifo->read() & 0xff); 
	command[6] = 0;
	command[7] = (uint8_t)(param_fifo->read() & 0xff); 
	command[8] = (uint8_t)(param_fifo->read() & 0xff); 
	command[9] = (uint8_t)(param_fifo->read() & 0xff);
	if(req_reply) {
		extra_status = 1;
		write_status(0x00, 0x00, 0x00, 0x00);
	}
	submpu_ready = false;
	d_cdrom->start_command();
}

void CDC::stop_cdda2(bool req_reply)
{
	uint8_t* command = d_cdrom->command;
	if(!(d_cdrom->is_device_ready())) {
		if(req_reply) write_status(0x10, 0x00, 0x00, 0x00);
		return;
	}
	command[0] = TOWNS_CDROM_CDDA_STOP;
	command[1] = 0;
	command[2] = 0;
	command[3] = (uint8_t)(param_fifo->read() & 0xff); 
	command[4] = (uint8_t)(param_fifo->read() & 0xff); 
	command[5] = (uint8_t)(param_fifo->read() & 0xff); 
	command[6] = 0;
	command[7] = (uint8_t)(param_fifo->read() & 0xff); 
	command[8] = (uint8_t)(param_fifo->read() & 0xff); 
	command[9] = (uint8_t)(param_fifo->read() & 0xff);
	if(req_reply) {
		extra_status = 1;
		write_status(0x00, 0x00, 0x00, 0x00);
	}
	submpu_ready = false;
	d_cdrom->start_command();
}

void CDC::unpause_cdda(bool req_reply)
{
	uint8_t* command = d_cdrom->command;
	if(!(d_cdrom->is_device_ready())) {
		if(req_reply) write_status(0x10, 0x00, 0x00, 0x00);
		return;
	}
	command[0] = TOWNS_CDROM_CDDA_UNPAUSE;
	command[1] = 0;
	command[2] = 0;
	command[3] = (uint8_t)(param_fifo->read() & 0xff); 
	command[4] = (uint8_t)(param_fifo->read() & 0xff); 
	command[5] = (uint8_t)(param_fifo->read() & 0xff); 
	command[6] = 0;
	command[7] = (uint8_t)(param_fifo->read() & 0xff); 
	command[8] = (uint8_t)(param_fifo->read() & 0xff); 
	command[9] = (uint8_t)(param_fifo->read() & 0xff);
	if(req_reply) {
		extra_status = 1;
		write_status(0x00, 0x03, 0x00, 0x00);
	}
	submpu_ready = false;
	d_cdrom->start_command();
}

void CDC::play_cdda(bool req_reply)
{
	uint8_t* command = d_cdrom->command;
	if(!(d_cdrom->is_device_ready())) {
		if(req_reply) write_status(0x10, 0x00, 0x00, 0x00);
		return;
	}
	command[0] = TOWNS_CDROM_CDDA_PLAY;
	command[1] = 0;
	command[2] = 0;
	command[3] = (uint8_t)(param_fifo->read() & 0xff); 
	command[4] = (uint8_t)(param_fifo->read() & 0xff); 
 	command[5] = (uint8_t)(param_fifo->read() & 0xff); 
	command[6] = 0;
	command[7] = (uint8_t)(param_fifo->read() & 0xff); 
	command[8] = (uint8_t)(param_fifo->read() & 0xff); 
	command[9] = (uint8_t)(param_fifo->read() & 0xff);

	
	if(req_reply) {
		extra_status = 1;
		write_status(0x00, 0x03, 0x00, 0x00);
	}
	submpu_ready = false;
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
		submpu_intr = true;
		if(!(submpu_intr_mask)) {
			write_signals(&output_submpu_intr, 0xffffffff);
		}
		submpu_ready = true;
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
			stop_cdda(req_status);
			break;
		case 0x05: // STOP CDDA (Difference from $84?)
			stop_cdda2(req_status);
			break;
		case 0x07: // UNPAUSE CDDA
			unpause_cdda(req_status);
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
	case SIG_TOWNS_CDC_DRQ:
		if((data & mask) != 0) {
			if((dma_transfer) ) {
				software_transfer_phase = false;
//				uint8_t val = d_scsi_host->read_dma_io8(0);
				d_dmac->write_signal(SIG_UPD71071_CH3, data, mask);
//				write_signals(&output_dma_line, val); // Indirect call do_dma().
			} else if((pio_transfer) ) {
				software_transfer_phase = true;
			} else {
				software_transfer_phase = false;
			}
		}
		break;
	case SIG_TOWNS_CDC_CDROM_DONE:
		if((data & mask) != 0) {
			submpu_intr = true;
			if(!(submpu_intr_mask)) {
				write_signals(&output_submpu_intr, 0xffffffff);
			}
			submpu_ready = true;
			out_debug_log("DONE");
		}
		break;
	case SIG_TOWNS_CDC_IRQ:
		dma_intr = ((data & mask) != 0);
		if((dma_intr & dma_intr_mask)) {
			if(stat_reply_intr) {
				write_signals(&output_dma_intr, 0xffffffff);
			}
		} else if(!(dma_intr) && (dma_intr_mask)) {
			write_signals(&output_dma_intr, 0x00000000);
		}
		break;
	case SIG_TOWNS_CDC_BSY:
		busy_status = ((data & mask) != 0);
		break;
	case SIG_TOWNS_CDC_CD:
		cd_status = ((data & mask) != 0);
		if((cd_status) && !(msg_status)) { // SCSI_PHASE_STATUS or SCSI_PHASE_COMMAND
			submpu_ready = true;
		}
		break;
	case SIG_TOWNS_CDC_IO:
		io_status = ((data & mask) != 0);
		break;
	case SIG_TOWNS_CDC_MSG:
		msg_status = ((data & mask) != 0);
		break;
	case SIG_TOWNS_CDC_REQ:
		req_status = ((data & mask) != 0);
		break;
	case SIG_TOWNS_CDC_ACK:
		ack_status = ((data & mask) != 0);
		break;
		
	}
}

uint32_t CDC::read_dma_io8(uint32_t addr)
{
	return (uint32_t)(d_scsi_host->read_dma_io8(addr));
}

uint32_t CDC::read_dma_io16(uint32_t addr)
{
	pair16_t d;
//	d.b.l = d_scsi_host->read_dma_io8(addr);
//	d.b.h = d_scsi_host->read_dma_io8(addr);
	d.w = d_scsi_host->read_dma_io16(addr);
	return (uint32_t)(d.w);
}

void CDC::write_dma_io8(uint32_t addr, uint32_t data)
{
	d_scsi_host->write_dma_io8(addr, data);
}

void CDC::write_dma_io16(uint32_t addr, uint32_t data)
{
	d_scsi_host->write_dma_io16(addr, data);
//	pair32_t _d;
//	_d.d = data;
//	d_scsi_host->write_dma_io8(addr, _d.b.l);
//	d_scsi_host->write_dma_io8(addr, _d.b.h);
}



#define STATE_VERSION	1

bool CDC::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!(param_fifo->process_state((void *)state_fio, loading))) {
		return false;
	}
	if(!(stat_fifo->process_state((void *)state_fio, loading))) {
		return false;
	}

	state_fio->StateValue(has_status);
	state_fio->StateValue(extra_status);
	state_fio->StateValue(submpu_ready);
	state_fio->StateValue(software_transfer_phase);
	state_fio->StateValue(dma_transfer);
	state_fio->StateValue(pio_transfer);

	state_fio->StateValue(dma_intr);
	state_fio->StateValue(submpu_intr);
	state_fio->StateValue(dma_intr_mask);
	state_fio->StateValue(submpu_intr_mask);

	state_fio->StateBuffer(w_regs, sizeof(w_regs), 1);

	state_fio->StateValue(busy_status);
	state_fio->StateValue(cd_status);
	state_fio->StateValue(io_status);
	state_fio->StateValue(msg_status);
	state_fio->StateValue(req_status);
	state_fio->StateValue(ack_status);
	
	return true;
	
}

}
