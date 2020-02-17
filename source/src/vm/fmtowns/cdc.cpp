/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[FM-Towns CD Controller with PSEUDO MB88505H 4bit MCU]
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

#define EVENT_CDROM_SEL_ON     1
#define EVENT_CDROM_SEL_OFF    2
#define EVENT_CDROM_SEL_OFF2   3
#define EVENT_POLL_CMD         4
#define EVENT_ENQUEUE_CMD      5 
#define EVENT_WAIT_REQ         6 
#define EVENT_POLL_BUS_FREE    7
#define EVENT_CDC_RESET        8
#define EVENT_WAIT_CMD_REQ_OFF 9
	
void CDC::set_context_scsi_host(SCSI_HOST* dev)
{
	d_scsi_host = dev;
	d_scsi_host->set_context_irq(this, SIG_TOWNS_CDC_IRQ, 1 << 0);
	d_scsi_host->set_context_drq(this, SIG_TOWNS_CDC_DRQ, 1 << 0);
	
	d_scsi_host->set_context_bsy(this, SIG_TOWNS_CDC_BSY, 1 << 0);
	d_scsi_host->set_context_cd(this, SIG_TOWNS_CDC_CD, 1 << 0);
	d_scsi_host->set_context_io(this, SIG_TOWNS_CDC_IO, 1 << 0);
	d_scsi_host->set_context_msg(this, SIG_TOWNS_CDC_MSG, 1 << 0);
	d_scsi_host->set_context_req(this, SIG_TOWNS_CDC_REQ, 1 << 0);
//	d_scsi_host->set_context_ack(this, SIG_TOWNS_CDC_ACK, 1 << 0);
}


void CDC::set_context_cdrom(TOWNS_CDROM* dev)
{
	d_cdrom = dev;
//	dev->set_context_done(this, SIG_TOWNS_CDC_CDROM_DONE, 0xffffffff);
}

void CDC::reset()
{
	stat_fifo->clear();
	memset(param_queue, 0x00, sizeof(param_queue));
	param_ptr = 0;
	
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
	scsi_req_status        = false;
	data_reg = 0x00;
	for(int i = 0; i < CDC_COMMAND_QUEUE_LENGTH; i++) {
		memset(cmdqueue[i].command, 0x00, 16);
		cmdqueue[i].cmd_table_size = 0;
		cmdqueue[i].cmd_write_ptr = 0;
	}
	next_cmdqueue = 0;
	current_cmdqueue = 0;
	left_cmdqueue = CDC_COMMAND_QUEUE_LENGTH;

	accept_command = false;
	data_in_status = false;
	
	if(event_cdrom_sel > -1) {
		cancel_event(this, event_cdrom_sel);
		event_cdrom_sel = -1;
	}
	if(event_poll_cmd > -1) {
		cancel_event(this, event_poll_cmd);
		event_poll_cmd = -1;
	}
	if(event_enqueue_cmd > -1) {
		cancel_event(this, event_enqueue_cmd);
		event_enqueue_cmd = -1;
	}
	if(event_wait_req > -1) {
		cancel_event(this, event_wait_req);
		event_wait_req = -1;
	}
//	d_scsi_host->reset();
	d_scsi_host->write_signal(SIG_SCSI_RST, 1, 1);
	d_scsi_host->write_signal(SIG_SCSI_ATN, 0, 1);
	d_scsi_host->write_signal(SIG_SCSI_SEL, 0, 1);
	register_event(this, EVENT_CDC_RESET, 30.0, false, NULL);
}

void CDC::initialize()
{
	stat_fifo = new FIFO(4);
	
	submpu_ready = true;

	submpu_intr_mask = false;
	dma_intr_mask = false;
	memset(w_regs, 0x00, sizeof(w_regs));

	dma_transfer = false;
	pio_transfer = true;
	req_status = false;

	busy_status = false;
	cd_status = false;
	io_status = false;
	msg_status = false;
	scsi_req_status = false;
	ack_status = false;

	event_cdrom_sel = -1;
	event_poll_cmd = -1;
	event_enqueue_cmd = -1;
	event_wait_req = -1;
	event_wait_cmd_req_off = -1;
}

void CDC::release()
{
	stat_fifo->release();
	
	delete stat_fifo;
}

void CDC::enqueue_cmdqueue(int size, uint8_t data[])
{
	out_debug_log(_T("ENQUEUE COMMAND SIZE=%d LEFT=%d"), size, left_cmdqueue);
	if(left_cmdqueue > 0) {
		uint8_t n = next_cmdqueue;
		if(size > 16) size = 16;
		if(size < 0) size = 0;
		if(size > 0) {
			for(int i = 0; i < size; i++) {
				cmdqueue[n].command[i] = data[i];
			}
			cmdqueue[n].cmd_table_size = size;
			cmdqueue[n].cmd_write_ptr = 0;
			next_cmdqueue++;
			left_cmdqueue--;
			start_poll_bus_free(0); // CDROM's pseudo SCSI ID is 0.
		}
		next_cmdqueue = next_cmdqueue & (CDC_COMMAND_QUEUE_LENGTH - 1);
	}
	return NULL;
}

void CDC::start_poll_bus_free(int unit)
{
	unit = unit & 7;
	out_debug_log("POLLING BUS FREE");
	// SET SCSI ID to DATA BUS before RISING UP SEL.
	d_scsi_host->write_dma_io8(0, (0x80 | (1 << unit)));
	//	d_scsi_host->write_signal(SIG_SCSI_SEL, 1, 1);
	register_event(this, EVENT_CDROM_SEL_ON, 15.0, true, &event_cdrom_sel); 
}

void CDC::start_poll_cmd_phase()
{
	if(event_poll_cmd < 0) {
		// 6.0us is tempolarry valuie.
		register_event(this, EVENT_POLL_CMD, 6.0, true, &event_poll_cmd); 
	}
}

void CDC::start_enqueue_command()
{
	// Note: command must be SCSI command, to cmd_table[], size is cmd_table_size.
	if(event_enqueue_cmd < 0) {
		// 3.0us is tempolarry valuie.
		register_event(this, EVENT_ENQUEUE_CMD, 3.0, true, &event_enqueue_cmd); 
	}
}

bool CDC::check_bus_free()
{
	if(!(busy_status) && !(scsi_req_status)) {
		return (!(cd_status) && !(msg_status) && !(io_status));
	}
	return false;
}

bool CDC::check_command_phase()
{
	return ((cd_status) && !(msg_status) && !(io_status));
}

bool CDC::check_data_in()
{
	return 	(!(cd_status) && !(msg_status) && (io_status));
}

void CDC::select_unit_on()
{
	out_debug_log("BUS FREE->SEL ON");
	d_scsi_host->write_signal(SIG_SCSI_SEL, 1, 1);
	//d_scsi_host->write_dma_io8(0, 0x81); // Write SCSI ADDRESS 0
	if(event_cdrom_sel > -1) {
		cancel_event(this, event_cdrom_sel);
		event_cdrom_sel = -1;
	}
	register_event(this, EVENT_CDROM_SEL_OFF, 800.0, false, &event_cdrom_sel); 
}

void CDC::select_unit_off()
{
//	d_scsi_host->write_dma_io8(0, 0x00); // Write SCSI ADDRESS 0
	register_event(this, EVENT_CDROM_SEL_OFF2, 100.0, false, &event_cdrom_sel); 
}

void CDC::select_unit_off2()
{
	out_debug_log("SEL ON ->SEL OFF");
	event_cdrom_sel = -1;
	d_scsi_host->write_signal(SIG_SCSI_SEL, 0, 1);
	event_cdrom_sel = -1;
	start_poll_cmd_phase();
}


void CDC::prologue_command_phase()
{
	if(check_command_phase()) {
		out_debug_log("COMMAND PHASE");
		accept_command = true;
		cancel_event(this, event_poll_cmd);
		event_poll_cmd = -1;
		if(event_enqueue_cmd > -1) {
			cancel_event(this, event_enqueue_cmd);
		}
		if(left_cmdqueue < CDC_COMMAND_QUEUE_LENGTH) {
//				d_scsi_host->write_signal(SIG_SCSI_SEL, 1, 1);
			start_enqueue_command();
		}

	}
}

void CDC::event_callback(int id, int error)
{
	switch(id) {
	case EVENT_CDC_RESET:
		d_scsi_host->write_signal(SIG_SCSI_RST, 0, 1);
		break;
	case EVENT_CDROM_SEL_ON:
		if(check_bus_free()) {
			select_unit_on();
		}
		break;
	case EVENT_CDROM_SEL_OFF:
		select_unit_off();
		break;
	case EVENT_CDROM_SEL_OFF2:
		select_unit_off2();
		break;
	case EVENT_POLL_CMD:
		if((scsi_req_status)) {
			prologue_command_phase();
		}
		break;
	case EVENT_ENQUEUE_CMD:
		if((scsi_req_status) && (cd_status) && !(msg_status) && !(io_status)) {
		if((accept_command) && (left_cmdqueue < CDC_COMMAND_QUEUE_LENGTH)) {
			data_in_status = false;
			uint8_t val = cmdqueue[current_cmdqueue].command[cmdqueue[current_cmdqueue].cmd_write_ptr];
			d_scsi_host->write_dma_io8(0, val);
			d_scsi_host->write_signal(SIG_SCSI_ACK, 0, 1);
			cmdqueue[current_cmdqueue].cmd_write_ptr++;
			if(event_wait_cmd_req_off > -1) {
				cancel_event(this, event_wait_cmd_req_off);
				event_wait_cmd_req_off = -1;
			}				
			register_event(this, EVENT_WAIT_CMD_REQ_OFF, 6.0, true, &event_wait_cmd_req_off); 
		}
		}
		break;
	case EVENT_WAIT_CMD_REQ_OFF:
		if(!(scsi_req_status)) {
			if(event_wait_cmd_req_off > -1) {
				cancel_event(this, event_wait_cmd_req_off);
				event_wait_cmd_req_off = -1;
			}
//			d_scsi_host->write_signal(SIG_SCSI_ACK, 0, 1);
			if(cmdqueue[current_cmdqueue].cmd_write_ptr >= cmdqueue[current_cmdqueue].cmd_table_size) { // Write all data.
				accept_command = false;
				cancel_event(this, event_enqueue_cmd);
				event_enqueue_cmd = -1;
				if(event_wait_req > -1) {
					cancel_event(this, event_wait_req);
				}
				out_debug_log(_T("PUSH COMMAND SIZE=%d\nCMDs="), cmdqueue[current_cmdqueue].cmd_table_size);
				_TCHAR tmps[128] = {0};
				for(int ii = 0; ii < cmdqueue[current_cmdqueue].cmd_table_size; ii++) {
					_TCHAR tmps2[8] = {0};
					my_stprintf_s(tmps2, 8, _T("%02X "), cmdqueue[current_cmdqueue].command[ii]);
					my_tcscat_s(tmps, 128, tmps2);
				}
				out_debug_log(_T(" %s"), tmps);
				left_cmdqueue++;
				if(left_cmdqueue > CDC_COMMAND_QUEUE_LENGTH) {
					left_cmdqueue = CDC_COMMAND_QUEUE_LENGTH;
				}
				current_cmdqueue++;
				current_cmdqueue = current_cmdqueue & (CDC_COMMAND_QUEUE_LENGTH - 1);
				d_scsi_host->write_signal(SIG_SCSI_ACK, 0, 1);
				register_event(this, EVENT_WAIT_REQ, 1.0, true, &event_wait_req); 
			} else {
				// Continue
				if(event_enqueue_cmd > -1) {
					cancel_event(this, event_enqueue_cmd);
					event_enqueue_cmd = -1;
				}
				start_enqueue_command();
			}
		}
	case EVENT_WAIT_REQ:
		if((scsi_req_status)) {
#if 0
			if(!(cd_status) && !(msg_status) && (io_status)) { // DATA IN
				out_debug_log("DATA IN");
				data_in_status = true;
				if(!(pio_transfer) && (dma_transfer)) {
					// WAIT FOR DRQ, NOT EVENT.
					cancel_event(this, event_wait_req);
					event_wait_req = -1;
				} else if((pio_transfer) && !(dma_transfer)) {
					data_reg = d_scsi_host->read_dma_io8(0);
//					data_in_status = false;
					// WAIT FOR DRQ, NOT EVENT.
//					cancel_event(this, event_wait_req);
//					event_wait_req = -1;
				}
			} else
#endif
				if(((cd_status) && !(msg_status) && (io_status)) ||// STATUS
					  ((cd_status) && (msg_status) && (io_status))) { // MSG IN
				data_in_status = true;
				if(!(pio_transfer) && (dma_transfer)) {
					//uint8_t val = d_scsi_host->read_dma_io8(0);
					//out_debug_log(_T("STATUS DATA=%02X"), val);
					d_dmac->write_signal(SIG_UPD71071_CH3, 0xff, 0xff);
					//data_reg = val;
					// WAIT FOR DRQ, NOT EVENT.
					cancel_event(this, event_wait_req);
					event_wait_req = -1;
				} else if((pio_transfer) && !(dma_transfer)) {
					data_reg = d_scsi_host->read_dma_io8(0);
					out_debug_log(_T("STATUS DATA=%02X"), data_reg);
					d_scsi_host->write_signal(SIG_SCSI_ACK, 1, 1);
					// WAIT FOR DRQ, NOT EVENT.
//					cancel_event(this, event_wait_req);
//					event_wait_req = -1;
				}
			} else { // ToDo: implement DATA OUT, MSG OUT and COMMAND
			}
		} else {
			// BUS FREE
//			d_scsi_host->write_signal(SIG_SCSI_SEL, 0, 1);
//			cancel_event(this, event_wait_req);
//			event_wait_req = -1;
			// EOL
		}
		break;
	}
}
void CDC::write_io8(uint32_t address, uint32_t data)
{
	/*
	 * 04C0h : Master control register
	 * 04C2h : Command register
	 * 04C4h : Parameter register
	 * 04C6h : Transfer control register.
	 */
//	out_debug_log(_T("WRITE I/O: ADDR=%04X DATA=%02X"), address, data);
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
			if((data & 0x04) != 0) {
				reset();
			}
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
			w_regs[address & 0x0f] = data;
//			readptr = 0;
			if(command_type_play) {
				enqueue_command_play(data);
			} else {
				enqueue_command_status(data);
			}
		}
		param_ptr = 0;
		
//		memset(param_queue, 0x00, sizeof(param_queue));
		break;
	case 0x04: // Parameter register
		param_queue[param_ptr] = data;
		param_ptr = (param_ptr + 1) & 0x07;
		break;
	case 0x06:
		if((data & 0x08) != 0) {
			dma_transfer = false;
			pio_transfer = true;
		}
		if((data & 0x10) != 0) {
			dma_transfer = true;
			pio_transfer = false;
		}
		w_regs[address & 0x0f] = data;
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
		val = data_reg;
		if((pio_transfer)) {
			data_reg = d_scsi_host->read_dma_io8(0);
//			out_debug_log(_T("PIO READ DATA=%02X"), val);
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
	extra_status = 0;
	if(!(d_cdrom->is_device_ready())) {
		out_debug_log(_T("DEVICE NOT READY"));
		if(req_reply) write_status(0x10, 0x00, 0x00, 0x00);
		return;
	}

	uint8_t m1, s1, f1;
	uint8_t m2, s2, f2;
//	uint8_t pad1, dcmd;
	
	m1 = FROM_BCD(param_queue[0]);
	s1 = FROM_BCD(param_queue[1]);
	f1 = FROM_BCD(param_queue[2]);

	m2 = FROM_BCD(param_queue[3]);
	s2 = FROM_BCD(param_queue[4]);
	f2 = FROM_BCD(param_queue[5]);
	uint8_t pad1 = param_queue[6];
	uint8_t dcmd = param_queue[7];

	int32_t lba1 = ((m1 * (60 * 75)) + (s1 * 75) + f1) - 150;
	int32_t lba2 = ((m2 * (60 * 75)) + (s2 * 75) + f2) - 150;
	
	uint32_t __remain;
	int track = 0;
	if(d_cdrom != NULL) {
		track = d_cdrom->get_track(lba1);
	}
	out_debug_log(_T("READ_CDROM TRACK=%d LBA1=%06X LBA2=%06X F1/S1/M1=%02X/%02X/%02X F2/S2/M2=%02X/%02X/%02X PAD=%02X DCMD=%02X"), track, lba1, lba2, f1, s1, m1, f2, s2, m2, pad1, dcmd);
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
	uint8_t command[16] = {0};
	command[0] = SCSI_CMD_READ12;
	command[1] = 0;
	command[2] = 0;
	command[3] = m1;
	command[4] = s1;
	command[5] = f1;
	
	command[6] = 0;
	command[7] = (uint8_t)((__remain / 0x10000) & 0xff);
	command[8] = (uint8_t)((__remain / 0x100) & 0xff);
	command[9] = (uint8_t) (__remain % 0x100);

	enqueue_cmdqueue(12, command);
	
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
//	submpu_ready = false;
}	

void CDC::stop_cdda(bool req_reply)
{
	if(!(d_cdrom->is_device_ready())) {
		if(req_reply) write_status(0x10, 0x00, 0x00, 0x00);
		return;
	}

	uint8_t m1, s1, f1;
	uint8_t m2, s2, f2;
	
	m1 = FROM_BCD(param_queue[0]);
	s1 = FROM_BCD(param_queue[1]);
	f1 = FROM_BCD(param_queue[2]);

	m2 = FROM_BCD(param_queue[3]);
	s2 = FROM_BCD(param_queue[4]);
	f2 = FROM_BCD(param_queue[5]);
	uint8_t pad1 = param_queue[6];
	uint8_t dcmd = param_queue[7];

	uint8_t command[10] = {0};
	command[0] = TOWNS_CDROM_CDDA_STOP;
	command[1] = 0;
	command[2] = 0;
	command[3] = f1;
	command[4] = s1;
	command[5] = m1;
	command[6] = 0;
	command[7] = f2;
	command[8] = s2;
	command[9] = m2;
	enqueue_cmdqueue(10, command);
	
	if(req_reply) {
		extra_status = 1;
		write_status(0x00, 0x00, 0x00, 0x00);
	}
//	submpu_ready = false;
}

void CDC::stop_cdda2(bool req_reply)
{
	if(!(d_cdrom->is_device_ready())) {
		if(req_reply) write_status(0x10, 0x00, 0x00, 0x00);
		return;
	}
	uint8_t m1, s1, f1;
	uint8_t m2, s2, f2;
	
	m1 = FROM_BCD(param_queue[0]);
	s1 = FROM_BCD(param_queue[1]);
	f1 = FROM_BCD(param_queue[2]);

	m2 = FROM_BCD(param_queue[3]);
	s2 = FROM_BCD(param_queue[4]);
	f2 = FROM_BCD(param_queue[5]);
	uint8_t pad1 = param_queue[6];
	uint8_t dcmd = param_queue[7];

	uint8_t command[10] = {0};
	command[0] = TOWNS_CDROM_CDDA_STOP;
	command[1] = 0;
	command[2] = 0;
	command[3] = f1;
	command[4] = m1;
	command[5] = s1;
	command[6] = 0;
	command[7] = f2;
	command[8] = m2;
	command[9] = s2;
	enqueue_cmdqueue(10, command);

	if(req_reply) {
		extra_status = 1;
		write_status(0x00, 0x00, 0x00, 0x00);
	}
//	submpu_ready = false;
}

void CDC::unpause_cdda(bool req_reply)
{

	if(!(d_cdrom->is_device_ready())) {
		if(req_reply) write_status(0x10, 0x00, 0x00, 0x00);
		return;
	}
	uint8_t m1, s1, f1;
	uint8_t m2, s2, f2;
	
	m1 = FROM_BCD(param_queue[0]);
	s1 = FROM_BCD(param_queue[1]);
	f1 = FROM_BCD(param_queue[2]);

	m2 = FROM_BCD(param_queue[3]);
	s2 = FROM_BCD(param_queue[4]);
	f2 = FROM_BCD(param_queue[5]);
	uint8_t pad1 = param_queue[6];
	uint8_t dcmd = param_queue[7];

	uint8_t command[10] = {0};
	
	command[0] = TOWNS_CDROM_CDDA_UNPAUSE;
	command[1] = 0;
	command[2] = 0;
	command[3] = f1;
	command[4] = m1;
	command[5] = s1;
	command[6] = 0;
	command[7] = f2;
	command[8] = m2;
	command[9] = s2;
	enqueue_cmdqueue(10, command);

	if(req_reply) {
		extra_status = 1;
		write_status(0x00, 0x03, 0x00, 0x00);
	}
//	submpu_ready = false;
}

void CDC::play_cdda(bool req_reply)
{
	if(!(d_cdrom->is_device_ready())) {
		if(req_reply) write_status(0x10, 0x00, 0x00, 0x00);
		return;
	}
	uint8_t m1, s1, f1;
	uint8_t m2, s2, f2;
	uint8_t command[12] = {0};
	m1 = FROM_BCD(param_queue[0]);
	s1 = FROM_BCD(param_queue[1]);
	f1 = FROM_BCD(param_queue[2]);

	m2 = FROM_BCD(param_queue[3]);
	s2 = FROM_BCD(param_queue[4]);
	f2 = FROM_BCD(param_queue[5]);
	uint8_t pad1 = param_queue[6];
	uint8_t dcmd = param_queue[7];
	
	command[0] = TOWNS_CDROM_CDDA_PLAY;
	command[1] = 0;
	command[2] = 0;
	command[3] = f1;
	command[4] = m1;
	command[5] = s1;
	command[6] = 0;
	command[7] = f2;
	command[8] = m2;
	command[9] = s2;
	enqueue_cmdqueue(10, command);
	
	if(req_reply) {
		extra_status = 1;
		write_status(0x00, 0x03, 0x00, 0x00);
	}
//	submpu_ready = false;
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
		out_debug_log(_T("CMD (%02X) BUT DISC NOT ACTIVE"), cmd);
		if(req_status) {
			extra_status = 0;
			write_status(0x10, 0x00, 0x00, 0x00);
		}
	} else {
		has_status = false;
//		d_scsi_host->write_dma_io8(0, 0x81); // SELECT SCSI 0
		switch(cmd & 0x1f) {
		case 0x00: // SEEK
			out_debug_log(_T("CMD SEEK (%02X)"), cmd);
			if(req_status) {
				extra_status = 1;
				write_status(0x00, 0x00, 0x00, 0x00);
			}
			// ToDo: REAL SEEK
			break;
		case 0x01: // Unknown (from MAME)
			out_debug_log(_T("CMD UNKNOWN (%02X)"), cmd);
			if(req_status) {
				extra_status = 0;
				write_status(0x00, 0xff, 0xff, 0xff);
			}
			break;
		case 0x02: // READ (Mode1)
			out_debug_log(_T("CMD READ MODE1 (%02X)"), cmd);
			read_cdrom(req_status);
			break;
		case 0x04: // PLAY CDDA
			out_debug_log(_T("CMD PLAY CDDA (%02X)"), cmd);
			play_cdda(req_status);
			break;
		case 0x05: // Read TOC
			out_debug_log(_T("CMD READ TOC (%02X)"), cmd);
			if(req_status) {
				extra_status = 1;
				write_status(0x00, 0x00, 0x00, 0x00);
			} else {
				extra_status = 2;
				write_status(0x16, 0x00, 0xa0, 0x00);
			}
			break;
		case 0x06: // CD-DA Stats (?)
			out_debug_log(_T("CMD READ CD-DA STATUS (%02X)"), cmd);
			extra_status = 1;
			write_status(0x00, 0x00, 0x00, 0x00);
			break;
		case 0x1f: // ??
			out_debug_log(_T("CMD UNKNOWN (%02X)"), cmd);
			extra_status = 0;
			write_status(0x00, 0x00, 0x00, 0x00);
			break;
		default: // Illegal command
			out_debug_log(_T("CMD Illegal(%02X)"), cmd);
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
		out_debug_log(_T("CMD (%02X) BUT DISC NOT ACTIVE"), cmd);
		if(req_status) {
			extra_status = 0;
			write_status(0x10, 0x00, 0x00, 0x00);
		}
	} else {
		has_status = false;
//		d_scsi_host->write_dma_io8(0, 0x81); // SELECT SCSI 0
		switch(cmd & 0x1f) {
		case 0x00: // set state
			out_debug_log(_T("CMD SET CDDA STATE (%02X)"), cmd);
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
			out_debug_log(_T("CMD SET CDDASET STATE (%02X)"), cmd);
			if(req_status) {
				extra_status = 0;
				write_status(0x00, 0x00, 0x00, 0x00);
			}
			break;
		case 0x04: // STOP CDDA
			out_debug_log(_T("CMD PAUSE CDDA $84(%02X)"), cmd);
			stop_cdda(req_status);
			break;
		case 0x05: // STOP CDDA (Difference from $84?)
			out_debug_log(_T("CMD PAUSE CDDA $85(%02X)"), cmd);
			stop_cdda2(req_status);
			break;
		case 0x07: // UNPAUSE CDDA
			out_debug_log(_T("CMD RESUME CDDA(%02X)"), cmd);
			unpause_cdda(req_status);
			break;
		default: // Illegal command
			out_debug_log(_T("CMD Illegal(%02X)"), cmd);
			extra_status = 0;
			write_status(0x10, 0x00, 0x00, 0x00);
			break;
		}
//		d_scsi_host->write_dma_io8(0, 0x81); // SELECT SCSI 0
	}
}

void CDC::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_TOWNS_CDC_DRQ:
//		out_debug_log(_T("SIG_TOWNS_CDC_DRQ"));
		if((data & mask) != 0) {
			if((dma_transfer) ) {
				software_transfer_phase = false;
				out_debug_log(_T("DRQ/DMA"));
				if((scsi_req_status) && (check_data_in())) {
					out_debug_log(_T("SEND DMAREQ to DMA3"));
					d_dmac->write_signal(SIG_UPD71071_CH3, 0xff, 0xff);
				}
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
		out_debug_log(_T("BSY %s"), ((data & mask) != 0) ? _T("ON") : _T("OFF"));
		busy_status = ((data & mask) != 0);
		break;
	case SIG_TOWNS_CDC_CD:
		out_debug_log(_T("CD %s"), ((data & mask) != 0) ? _T("ON") : _T("OFF"));
		cd_status = ((data & mask) != 0);
//		if((cd_status) && !(msg_status)) { // SCSI_PHASE_STATUS or SCSI_PHASE_COMMAND
//			submpu_ready = true;
//		}
		break;
	case SIG_TOWNS_CDC_IO:
		out_debug_log(_T("IO %s"), ((data & mask) != 0) ? _T("ON") : _T("OFF"));
		io_status = ((data & mask) != 0);
		break;
	case SIG_TOWNS_CDC_MSG:
		out_debug_log(_T("MSG %s"), ((data & mask) != 0) ? _T("ON") : _T("OFF"));
		msg_status = ((data & mask) != 0);
		break;
	case SIG_TOWNS_CDC_REQ:
//		out_debug_log(_T("REQ %s"), ((data & mask) != 0) ? _T("ON") : _T("OFF"));
		scsi_req_status = ((data & mask) != 0);
		break;
	case SIG_TOWNS_CDC_ACK:
//		out_debug_log(_T("ACK %s"), ((data & mask) != 0) ? _T("ON") : _T("OFF"));
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
	state_fio->StateValue(req_status);

	state_fio->StateBuffer(w_regs, sizeof(w_regs), 1);

	state_fio->StateValue(busy_status);
	state_fio->StateValue(cd_status);
	state_fio->StateValue(io_status);
	state_fio->StateValue(msg_status);
	state_fio->StateValue(scsi_req_status);
	state_fio->StateValue(ack_status);
	state_fio->StateValue(data_in_status);
	
	state_fio->StateBuffer(param_queue, sizeof(param_queue), 1);
	state_fio->StateValue(param_ptr);
	
	state_fio->StateValue(accept_command);
	state_fio->StateValue(left_cmdqueue);
	state_fio->StateValue(current_cmdqueue);
	state_fio->StateValue(next_cmdqueue);
	for(int i = 0; i < CDC_COMMAND_QUEUE_LENGTH; i++) {
		state_fio->StateBuffer(cmdqueue[i].command, sizeof(uint8_t) * 16, 1);
		state_fio->StateValue(cmdqueue[i].cmd_table_size);
		state_fio->StateValue(cmdqueue[i].cmd_write_ptr);
	}

	state_fio->StateValue(event_cdrom_sel);
	state_fio->StateValue(event_poll_cmd);
	state_fio->StateValue(event_enqueue_cmd);
	state_fio->StateValue(event_wait_req);
	state_fio->StateValue(event_wait_cmd_req_off);
	
	return true;
	
}

}
