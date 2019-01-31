
#include "towns_cdrom.h"

namespace FMTOWNS {

void CDC::initialize()
{
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
				if(submpu_intr) output_signals(&output_submpu_intr, 0x00000000);
				submpu_intr = false;
			}
			if((data & 0x40) != 0) {
				if(dma_intr) output_signals(&output_dma_intr, 0x00000000);
				dma_intr = false;
			}
			submpu_intr_mask = ((data & 0x02) != 0) ? true : false;
			dma_intr_mask    = ((data & 0x01) != 0) ? true : false;
			if((data & 0x04) != 0) this->reset();
			w_regs[address & 0x0f] = data;
		}
		break;
	case 0x02: // Command register
		{
			command_type_play = ((data & 0x80) != 0) ? true : false; // false = status command
			stat_reply_intr   = ((data & 0x40) != 0) ? true : false;
			req_status        = ((data & 0x20) != 0) ? true : false;
			if(command_type_play) {
				enqueue_command_play(data & 0x1f);
			} else {
				enqueue_command_status(data & 0x1f);
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
	d_cdrom->start_command();

}

void CDC::write_status(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	status_fifo->clear();
	status_fifo->write(a);
	status_fifo->write(b);
	status_fifo->write(c);
	status_fifo->write(d);
	if(stat_reply_intr) {
		if(!(submpu_intr_mask)) {
			output_signals(&output_submpu_intr, 0xffffffff);
		}
		submpu_intr = true;
	}
}
