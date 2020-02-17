/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI base device ]
*/

#include "scsi_dev.h"
#include "../fifo.h"

#define EVENT_SEL	0
#define EVENT_PHASE	1
#define EVENT_REQ	2
//#define _SCSI_DEBUG_LOG

void SCSI_DEV::initialize()
{
	DEVICE::initialize();
	buffer = new FIFO(SCSI_BUFFER_SIZE);
	phase = SCSI_PHASE_BUS_FREE;
	_SCSI_DEV_IMMEDIATE_SELECT = osd->check_feature(_T("SCSI_DEV_IMMEDIATE_SELECT"));
	__SCSI_DEBUG_LOG = osd->check_feature(_T("_SCSI_DEBUG_LOG"));
	_SCSI_HOST_WIDE = osd->check_feature(_T("SCSI_HOST_WIDE"));
	_OUT_DEBUG_LOG = false; // Temporally.
	if(_SCSI_HOST_WIDE) {
		update_signal_mask(&outputs_dat, NULL, 0xffff); // Update to 16bit wide.
	}
}

void SCSI_DEV::out_debug_log(const _TCHAR *format, ...)
{
	if(!(__SCSI_DEBUG_LOG) && !(_OUT_DEBUG_LOG)) return;
	va_list args;
	_TCHAR _tmps[4096] = {0};
	_TCHAR _domain[256] = {0};
	my_sprintf_s(_domain, sizeof(_domain) / sizeof(_TCHAR), _T("[SCSI_DEV:ID=%d]"), scsi_id);
	va_start(args, format);
	vsnprintf(_tmps, sizeof(_tmps) / sizeof(_TCHAR), format, args);
	va_end(args);
	DEVICE::out_debug_log(_T("%s %s"), _domain, _tmps);
}

void SCSI_DEV::release()
{
	buffer->release();
	delete buffer;
}

void SCSI_DEV::reset()
{
	data_bus = 0;
	sel_status = atn_status = ack_status = rst_status = false;
	selected = atn_pending = false;
	
	event_sel = event_phase = event_req = -1;
	set_phase(SCSI_PHASE_BUS_FREE);
	set_sense_code(SCSI_SENSE_NOSENSE);
}

void SCSI_DEV::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SCSI_DAT:
		data_bus = data & mask;
//		out_debug_log(_T("DATA=%04X"), data_bus);
		break;
		
	case SIG_SCSI_SEL:
		{
			bool prev_status = sel_status;
			sel_status = ((data & mask) != 0);
			
			if(phase != SCSI_PHASE_BUS_FREE) {
				// this device is already selected
			} else if(!prev_status && sel_status) {
				// L -> H
				if(_SCSI_DEV_IMMEDIATE_SELECT) {
					event_callback(EVENT_SEL, 0);
				} else {
					if(event_sel != -1) {
						cancel_event(this, event_sel);
					}
					register_event(this, EVENT_SEL, 20.0, false, &event_sel);
				}
			} else if(prev_status && !sel_status) {
				// H -> L
				if(event_sel != -1) {
					cancel_event(this, event_sel);
					event_sel = -1;
				}
				if(selected) {
					if(atn_status) {
						// change to message out phase
						buffer->clear();
						set_phase_delay(SCSI_PHASE_MESSAGE_OUT, 10.0);
//						set_phase(SCSI_PHASE_MESSAGE_OUT);
					} else {
						// change to command phase
						memset(command, 0, sizeof(command));
						command_index = 0;
						set_phase_delay(SCSI_PHASE_COMMAND, 10.0);
//						set_phase(SCSI_PHASE_COMMAND);
					}
				}
			}
		}
		break;
		
	case SIG_SCSI_ATN:
		{
			bool prev_status = atn_status;
			atn_status = ((data & mask) != 0);
			
			if(phase == SCSI_PHASE_BUS_FREE) {
				// this device is not selected
			} else if(!prev_status && atn_status) {
				// L -> H
				out_debug_log(_T("ATN signal raised\n"));
				if(ack_status) {
					// wait until ack=off
					atn_pending = true;
				} else {
					// change to message out phase
					atn_pending = false;
					buffer->clear();
					set_phase_delay(SCSI_PHASE_MESSAGE_OUT, 10.0);
				}
			}
		}
		break;
		
	case SIG_SCSI_ACK:
		{
/*
			initiator --->	device

			wait req=on
					set req=on
					wait ack=on
			write data
			set ack=on
			wait req=off
					read data
					set req=off
					wait ack=off
			set ack=off

			initiator <---	device

			wait req=on
					write data
					set req=on
					wait ack=on
			read data
			set ack=on
			wait req=off
					set req=off
					wait ack=off
			set ack=off
*/
			bool prev_status = ack_status;
			ack_status = ((data & mask) != 0);
//			out_debug_log(_T("ACK=%s"), (ack_status) ? _T("ON") : _T("OFF"));
			if(phase == SCSI_PHASE_BUS_FREE) {
				// this device is not selected
			} else if(!prev_status & ack_status) {
				// L -> H
				switch(phase) {
				case SCSI_PHASE_DATA_OUT:
					buffer->write(data_bus);
					
					// check defect list length in format unit data
					if(command[0] == SCSI_CMD_FORMAT) {
						if(buffer->count() == 4) {
							remain += buffer->read_not_remove(2) * 256 + buffer->read_not_remove(3);
						}
					}
					break;
					
				case SCSI_PHASE_COMMAND:
					command[command_index++] = data_bus;
					break;
					
				case SCSI_PHASE_MESSAGE_OUT:
					buffer->write(data_bus);
					break;
				}
				set_req_delay(0, 0.1);
			} else if(prev_status && !ack_status) {
				// H -> L
				if(atn_pending) {
					// change to message out phase
					atn_pending = false;
					buffer->clear();
					set_phase_delay(SCSI_PHASE_MESSAGE_OUT, 10.0);
				} else {
					switch(phase) {
					case SCSI_PHASE_DATA_OUT:
						if(--remain > 0) {
							// flush buffer
							if(buffer->full()) {
								if(!write_buffer(buffer->count())) {
									// change to status phase
									set_dat(SCSI_STATUS_CHKCOND);
									set_phase_delay(SCSI_PHASE_STATUS, 10.0);
									break;
								}
								buffer->clear(); // just in case
							}
							switch(command[0]) {
							case SCSI_CMD_WRITE6:
							case SCSI_CMD_WRITE10:
							case SCSI_CMD_WRITE12:
								// request to write next data
								{
									next_req_usec += 1000000.0 / bytes_per_sec;
									double usec = next_req_usec - get_passed_usec(first_req_clock);
									set_req_delay(1, (usec > 1.0) ? usec : 1.0);
								}
								break;
							default:
								// request to write next data
								set_req_delay(1, 1.0);
								break;
							}
						} else {
							// flush buffer
							if(!buffer->empty()) {
								if(!write_buffer(buffer->count())) {
									// change to status phase
									set_dat(SCSI_STATUS_CHKCOND);
									set_phase_delay(SCSI_PHASE_STATUS, 10.0);
									break;
								}
								buffer->clear(); // just in case
							}
							// change to status phase
							set_dat(SCSI_STATUS_GOOD);
							set_sense_code(SCSI_SENSE_NOSENSE);
							set_phase_delay(SCSI_PHASE_STATUS, 10.0);
						}
						break;
						
					case SCSI_PHASE_DATA_IN:
						if(--remain > 0) {
							// update buffer
							if(buffer->count() == 0) {
								int length = remain > SCSI_BUFFER_SIZE ? SCSI_BUFFER_SIZE : (int)remain;
								if(!read_buffer(length)) {
									// change to status phase
									set_dat(SCSI_STATUS_CHKCOND);
									set_phase_delay(SCSI_PHASE_STATUS, 10.0);
									break;
								}
							}
							// request to read next data
							set_dat(buffer->read());
							switch(command[0]) {
							case SCSI_CMD_READ6:
							case SCSI_CMD_READ10:
							case SCSI_CMD_READ12:
								{
									next_req_usec += 1000000.0 / bytes_per_sec;
									double usec = next_req_usec - get_passed_usec(first_req_clock);
									set_req_delay(1, (usec > 1.0) ? usec : 1.0);
									//set_req_delay(1, usec);
								}
								break;
							default:
								set_req_delay(1, 1.0);
								break;
							}
						} else {
							// change to status phase
							set_dat(SCSI_STATUS_GOOD);
							set_sense_code(SCSI_SENSE_NOSENSE);
							set_phase_delay(SCSI_PHASE_STATUS, 10.0);
						}
						break;
						
					case SCSI_PHASE_COMMAND:
						if(command_index < get_command_length(command[0])) {
							// request next command
							set_req_delay(1, 1.0);
						} else {
							// start command
							start_command();
						}
						break;
						
					case SCSI_PHASE_STATUS:
						// create message data table
						remain = 1;
						buffer->clear();
						buffer->write(0x00); // command complete message
						// change to message in phase
						set_dat(buffer->read());
						set_phase_delay(SCSI_PHASE_MESSAGE_IN, 1.0);
						break;
						
					case SCSI_PHASE_MESSAGE_OUT:
						if((buffer->read() & 0xb8) == 0x80) {
							// identify, change to command phase
							memset(command, 0, sizeof(command));
							command_index = 0;
							set_phase_delay(SCSI_PHASE_COMMAND, 10.0);
						} else {
							// abort, change to bus free phase
							set_phase_delay(SCSI_PHASE_BUS_FREE, 10.0);
						}
						break;
						
					case SCSI_PHASE_MESSAGE_IN:
						if(--remain > 0) {
							// request to read next data
							set_dat(buffer->read());
							set_req_delay(1, 1.0);
						} else {
							// change to bus free phase
							set_phase_delay(SCSI_PHASE_BUS_FREE, 1.0);
						}
						break;
					}
				}
			}
		}
		break;
		
	case SIG_SCSI_RST:
		{
			bool prev_status = rst_status;
			rst_status = ((data & mask) != 0);
			
			if(!prev_status & rst_status) {
				// L -> H
				out_debug_log(_T("RST signal raised\n"));
				reset_device();
				set_phase(SCSI_PHASE_BUS_FREE);
			}
		}
		break;
	}
}

void SCSI_DEV::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_SEL:
		event_sel = -1;
		if((data_bus & 0x7f) == (1 << scsi_id)) {
			if(is_device_existing()) {
				// this device is selected!
				out_debug_log(_T("This device is selected\n"));
				set_bsy(true);
				selected = true;
			}
		}
		break;
		
	case EVENT_PHASE:
		event_phase = -1;
		set_phase(next_phase);
		break;
		
	case EVENT_REQ:
		event_req = -1;
		set_req(next_req);
		break;
	}
}

void SCSI_DEV::set_phase(int value)
{
	out_debug_log(_T("Phase %s -> %s\n"), scsi_phase_name[phase], scsi_phase_name[value]);
	if(event_phase != -1) {
		cancel_event(this, event_phase);
		event_phase = -1;
	}
	set_io (value & 1);
	set_msg(value & 2);
	set_cd (value & 4);
	
	if(value == SCSI_PHASE_BUS_FREE) {
		set_bsy(false);
		set_req(0);
		selected = false;
	} else {
		first_req_clock = 0;
//		set_bsy(true);
		set_req_delay(1, 10.0);
	}
	phase = value;
}

void SCSI_DEV::set_phase_delay(int value, double usec)
{
	if(usec <= 0.0) {
		set_phase(value);
	} else {
		if(event_phase != -1) {
			cancel_event(this, event_phase);
			event_phase = -1;
		}
		register_event(this, EVENT_PHASE, usec, false, &event_phase);
		next_phase = value;
	}
}

void SCSI_DEV::set_dat(int value)
{
	if(__SCSI_DEBUG_LOG) {
//		emu->force_out_debug_log(_T("[SCSI_DEV:ID=%d] DATA = %02x\n"), scsi_id, value);
	}
	write_signals(&outputs_dat, value);
}

void SCSI_DEV::set_bsy(int value)
{
	out_debug_log(_T("BUSY = %d\n"), value ? 1 : 0);
	write_signals(&outputs_bsy, value ? 0xffffffff : 0);
}

void SCSI_DEV::set_cd(int value)
{
	out_debug_log(_T("C/D = %d\n"), value ? 1 : 0);
	write_signals(&outputs_cd,  value ? 0xffffffff : 0);
}

void SCSI_DEV::set_io(int value)
{
	out_debug_log(_T("I/O = %d\n"), value ? 1 : 0);
	write_signals(&outputs_io,  value ? 0xffffffff : 0);
}

void SCSI_DEV::set_msg(int value)
{
	out_debug_log(_T("MSG = %d\n"), value ? 1 : 0);
	write_signals(&outputs_msg, value ? 0xffffffff : 0);
}

void SCSI_DEV::set_req(int value)
{
//	out_debug_log(_T("REQ = %d\n"), value ? 1 : 0);
	if(event_req != -1) {
		cancel_event(this, event_req);
		event_req = -1;
	}
	if(value && first_req_clock == 0) {
		first_req_clock = get_current_clock();
		next_req_usec = 0.0;
	}
	write_signals(&outputs_req, value ? 0xffffffff : 0);
}

void SCSI_DEV::set_req_delay(int value, double usec)
{
	if(usec <= 0.0) {
		set_req(value);
	} else {
		if(event_req != -1) {
			cancel_event(this, event_req);
			event_req = -1;
		}
		register_event(this, EVENT_REQ, usec, false, &event_req);
		next_req = value;
	}
}

int SCSI_DEV::get_command_length(int value)
{
	switch((value >> 5) & 7) {
	case 0:
	case 3:
	case 6:
	case 7:
		return 6;
	case 1:
	case 2:
		return 10;
	case 5:
		return 12;
	}
	return 6;
}

void SCSI_DEV::start_command()
{
	switch(command[0]) {
	case SCSI_CMD_TST_U_RDY:
		out_debug_log(_T("Command: Test Unit Ready\n"));
		// change to status phase
		if(!is_device_ready()) {
			set_dat(SCSI_STATUS_CHKCOND);
			set_sense_code(SCSI_SENSE_NOTREADY);
		} else {
			set_dat(SCSI_STATUS_GOOD);
			set_sense_code(SCSI_SENSE_NOSENSE);
		}
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		break;
		
	case SCSI_CMD_REQ_SENSE:
		out_debug_log(_T("Command: Request Sense\n"));
		// start position
		position = (command[1] & 0x1f) * 0x10000 + command[2] * 0x100 + command[3];
		position *= physical_block_size();
		// transfer length
//		remain = 16;
		remain = command[4];
		// create sense data table
		buffer->clear();
		for(int i = 0; i < remain; i++) {
			int value = 0;
			switch(i) {
			case 0: value = SCSI_SERROR_CURRENT; break;
			case 2: value = is_device_ready() ? SCSI_KEY_NOSENSE : SCSI_KEY_UNITATT; break;
			case 7: value = 0x08; break;
			}
			buffer->write(value);
		}
		// change to data in phase
		set_dat(buffer->read());
		set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		break;
		
	case SCSI_CMD_INQUIRY:
		out_debug_log(_T("Command: Inquiry\n"));
		// start position
		position = (command[1] & 0x1f) * 0x10000 + command[2] * 0x100 + command[3];
		position *= physical_block_size();
		// transfer length
		remain = 32;
		// create inquiry data table
		buffer->clear();
		buffer->write(device_type);
		buffer->write(is_removable ? 0x80 : 0x00);
		buffer->write(0x02); // ANSI SCSI2
		buffer->write(0x01); // ANSI-CCS
		buffer->write(0x10);
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x18);
		for(int i = 0; i < (int)strlen(vendor_id) && i < 8; i++) {
			buffer->write(vendor_id[i]);
		}
		for(int i = strlen(vendor_id); i < 8; i++) {
			buffer->write(0x20);
		}
		for(int i = 0; i < (int)strlen(product_id) && i < 16; i++) {
			buffer->write(vendor_id[i]);
		}
		for(int i = strlen(product_id); i < 16; i++) {
			buffer->write(0x20);
		}
		// change to data in phase
		set_dat(buffer->read());
		set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		break;
		
	case SCSI_CMD_RD_CAPAC:
		out_debug_log(_T("Command: Read Capacity\n"));
		// start position
		position = command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5];
		position *= physical_block_size();
		// transfer length
		remain = 8;
		// create capacity data table
		buffer->clear();
		buffer->write((max_logical_block_addr() >> 24) & 0xff);
		buffer->write((max_logical_block_addr() >> 16) & 0xff);
		buffer->write((max_logical_block_addr() >>  8) & 0xff);
		buffer->write((max_logical_block_addr() >>  0) & 0xff);
		buffer->write((    logical_block_size() >> 24) & 0xff);
		buffer->write((    logical_block_size() >> 16) & 0xff);
		buffer->write((    logical_block_size() >>  8) & 0xff);
		buffer->write((    logical_block_size() >>  0) & 0xff);
		// change to data in phase
		set_dat(buffer->read());
		set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		break;
		
	case SCSI_CMD_FORMAT:
		out_debug_log(_T("Command: Format Unit\n"));
		if(command[1] & 0x10) {
			// change to data out phase for extra bytes
			remain = 4;
			set_phase_delay(SCSI_PHASE_DATA_OUT, 10.0);
		} else {
			// no extra bytes, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_RD_DEFECT:
		out_debug_log(_T("Command: Read Defect Data\n"));
		// transfer length
		remain = 4;
		// create detect data table
		buffer->clear();
		buffer->write(0x00);
		buffer->write(command[2]);
		buffer->write(0x00); // msb of defect list length
		buffer->write(0x00); // lsb of defect list length
		// change to data in phase
		set_dat(buffer->read());
		set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		break;
		
	case SCSI_CMD_READ6:
		// start position
		position = (command[1] & 0x1f) * 0x10000 + command[2] * 0x100 + command[3];
		out_debug_log(_T("Command: Read 6-byte LBA=%d BLOCKS=%d\n"), position, command[4]);
		position *= physical_block_size();
		// transfer length
		remain = command[4] * logical_block_size();
		if(remain != 0) {
			// read data buffer
			buffer->clear();
			int length = remain > SCSI_BUFFER_SIZE ? SCSI_BUFFER_SIZE : (int)remain;
			if(!read_buffer(length)) {
				// change to status phase
				set_dat(SCSI_STATUS_CHKCOND);
				set_phase_delay(SCSI_PHASE_STATUS, 10.0);
				break;
			}
			// change to data in phase
			set_dat(buffer->read());
			set_phase_delay(SCSI_PHASE_DATA_IN, seek_time);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_sense_code(SCSI_SENSE_NOSENSE);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_WRITE6:
		// start position
		position = (command[1] & 0x1f) * 0x10000 + command[2] * 0x100 + command[3];
		out_debug_log(_T("Command: Write 6-byte LBA=%d BLOCKS=%d\n"), position, command[4]);
		position *= physical_block_size();
		// transfer length
		remain = command[4] * logical_block_size();
		if(remain != 0) {
			// clear data buffer
			buffer->clear();
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_OUT, seek_time);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_sense_code(SCSI_SENSE_NOSENSE);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_READ10:
		// start position
		position = command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5];
		out_debug_log(_T("Command: Read 10-byte LBA=%d BLOCKS=%d\n"), position, command[7] * 0x100 + command[8]);
		position *= physical_block_size();
		// transfer length
		remain = command[7] * 0x100 + command[8];
		remain *= logical_block_size();
		if(remain != 0) {
			// read data buffer
			buffer->clear();
			int length = remain > SCSI_BUFFER_SIZE ? SCSI_BUFFER_SIZE : (int)remain;
			if(!read_buffer(length)) {
				// change to status phase
				set_dat(SCSI_STATUS_CHKCOND);
				set_phase_delay(SCSI_PHASE_STATUS, 10.0);
				break;
			}
			// change to data in phase
			set_dat(buffer->read());
			set_phase_delay(SCSI_PHASE_DATA_IN, seek_time);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_sense_code(SCSI_SENSE_NOSENSE);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_WRITE10:
		goto WRITE10;
	case SCSI_CMD_WRT_VERIFY:
	WRITE10:
		// start position
		position = command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5];
		out_debug_log(_T("Command: %s LBA=%d BLOCKS=%d\n"), (command[0] == SCSI_CMD_WRT_VERIFY) ? _T("Write and Verify") : _T("Write 10-byte"), position, command[7] * 0x100 + command[8]);
		position *= physical_block_size();
		// transfer length
		remain = command[7] * 0x100 + command[8];
		remain *= logical_block_size();
		if(remain != 0) {
			// clear data buffer
			buffer->clear();
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_OUT, seek_time);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_sense_code(SCSI_SENSE_NOSENSE);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_READ12:
		// start position
		position = command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5];
		out_debug_log(_T("Command: Read 12-byte LBA=%d BLOCKS=%d\n"), position, command[6] * 0x1000000 + command[7] * 0x10000 + command[8] * 0x100 + command[9]);
		position *= physical_block_size();
		// transfer length
		remain = command[6] * 0x1000000 + command[7] * 0x10000 + command[8] * 0x100 + command[9];
		remain *= logical_block_size();
		if(remain != 0) {
			// read data buffer
			buffer->clear();
			int length = remain > SCSI_BUFFER_SIZE ? SCSI_BUFFER_SIZE : (int)remain;
			if(!read_buffer(length)) {
				// change to status phase
				set_dat(SCSI_STATUS_CHKCOND);
				set_phase_delay(SCSI_PHASE_STATUS, 10.0);
				break;
			}
			// change to data in phase
			set_dat(buffer->read());
			set_phase_delay(SCSI_PHASE_DATA_IN, seek_time);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_sense_code(SCSI_SENSE_NOSENSE);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_WRITE12:
		// start position
		position = command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5];
		out_debug_log(_T("Command: Write 12-byte LBA=%d BLOCKS=%d\n"), position, command[6] * 0x1000000 + command[7] * 0x10000 + command[8] * 0x100 + command[9]);
		position *= physical_block_size();
		// transfer length
		remain = command[6] * 0x1000000 + command[7] * 0x10000 + command[8] * 0x100 + command[9];
		remain *= logical_block_size();
		if(remain != 0) {
			// clear data buffer
			buffer->clear();
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_OUT, seek_time);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_sense_code(SCSI_SENSE_NOSENSE);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	default:
		out_debug_log(_T("Command: Unknown %02X\n"), command[0]);
		set_dat(SCSI_STATUS_GOOD);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
	}
}

bool SCSI_DEV::read_buffer(int length)
{
	for(int i = 0; i < length; i++) {
		buffer->write(0);
		position++;
	}
	set_sense_code(SCSI_SENSE_NOSENSE);
	return true;
}

bool SCSI_DEV::write_buffer(int length)
{
	for(int i = 0; i < length; i++) {
		buffer->read();
		position++;
	}
	set_sense_code(SCSI_SENSE_NOSENSE);
	return true;
}

#define STATE_VERSION	2

bool SCSI_DEV::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(data_bus);
	state_fio->StateValue(sel_status);
	state_fio->StateValue(atn_status);
	state_fio->StateValue(ack_status);
	state_fio->StateValue(rst_status);
	state_fio->StateValue(selected);
	state_fio->StateValue(atn_pending);
	state_fio->StateValue(phase);
	state_fio->StateValue(next_phase);
	state_fio->StateValue(next_req);
	state_fio->StateValue(event_sel);
	state_fio->StateValue(event_phase);
	state_fio->StateValue(event_req);
	state_fio->StateValue(first_req_clock);
	state_fio->StateValue(next_req_usec);
	state_fio->StateArray(command, sizeof(command), 1);
	state_fio->StateValue(command_index);
	if(!buffer->process_state((void *)state_fio, loading)) {
 		return false;
 	}
	state_fio->StateValue(position);
	state_fio->StateValue(remain);
	state_fio->StateValue(sense_code);
 	return true;
}
