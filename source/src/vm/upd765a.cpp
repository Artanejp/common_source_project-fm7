/*
	Skelton for retropc emulator

	Origin : M88
	Author : Takeda.Toshiya
	Date   : 2006.09.17-

	[ uPD765A ]
*/

#include "upd765a.h"
#include "disk.h"
#include "noise.h"

#define EVENT_PHASE	0
#define EVENT_DRQ	1
#define EVENT_LOST	2
#define EVENT_RESULT7	3
#define EVENT_INDEX	4
#define EVENT_SEEK_STEP	5	// 5-8
#define EVENT_SEEK_END	9	// 9-12
#define EVENT_UNLOAD	13	// 13-16

#define PHASE_IDLE	0
#define PHASE_CMD	1
#define PHASE_EXEC	2
#define PHASE_READ	3
#define PHASE_WRITE	4
#define PHASE_SCAN	5
#define PHASE_TC	6
#define PHASE_TIMER	7
#define PHASE_RESULT	8

#define S_D0B	0x01
#define S_D1B	0x02
#define S_D2B	0x04
#define S_D3B	0x08
#define S_CB	0x10
#define S_NDM	0x20
#define S_DIO	0x40
#define S_RQM	0x80

#define ST0_NR	0x000008
#define ST0_EC	0x000010
#define ST0_SE	0x000020
#define ST0_AT	0x000040
#define ST0_IC	0x000080
#define ST0_AI	0x0000c0

#define ST1_MA	0x000100
#define ST1_NW	0x000200
#define ST1_ND	0x000400
#define ST1_OR	0x001000
#define ST1_DE	0x002000
#define ST1_EN	0x008000

#define ST2_MD	0x010000
#define ST2_BC	0x020000
#define ST2_SN	0x040000
#define ST2_SH	0x080000
#define ST2_NC	0x100000
#define ST2_DD	0x200000
#define ST2_CM	0x400000

#define ST3_HD	0x04
#define ST3_TS	0x08
#define ST3_T0	0x10
#define ST3_RY	0x20
#define ST3_WP	0x40
#define ST3_FT	0x80

#define DRIVE_MASK	3

#define REGISTER_PHASE_EVENT(phs, usec) { \
	if(phase_id != -1) { \
		cancel_event(this, phase_id); \
	} \
	event_phase = phs; \
	register_event(this, EVENT_PHASE, 100, false, &phase_id); \
}

#define REGISTER_PHASE_EVENT_NEW(phs, usec) { \
	if(phase_id != -1) { \
		cancel_event(this, phase_id); \
	} \
	event_phase = phs; \
	register_event(this, EVENT_PHASE, usec, false, &phase_id); \
}

#define REGISTER_DRQ_EVENT() { \
	double usec = disk[hdu & DRIVE_MASK]->get_usec_per_bytes(1) - get_passed_usec(prev_drq_clock); \
	if(usec < 4) { \
		usec = 4; \
	} \
	register_event(this, EVENT_DRQ, usec, false, &drq_id); \
}

#define CANCEL_EVENT() { \
	if(phase_id != -1) { \
		cancel_event(this, phase_id); \
		phase_id = -1; \
	} \
	if(drq_id != -1) { \
		cancel_event(this, drq_id); \
		drq_id = -1; \
	} \
	if(lost_id != -1) { \
		cancel_event(this, lost_id); \
		lost_id = -1; \
	} \
	if(result7_id != -1) { \
		cancel_event(this, result7_id); \
		result7_id = -1; \
	} \
	for(int d = 0; d < 4; d++) { \
		if(seek_step_id[d] != -1) { \
			cancel_event(this, seek_step_id[d]); \
			seek_step_id[d] = -1; \
		} \
		if(seek_end_id[d] != -1) { \
			cancel_event(this, seek_end_id[d]); \
			seek_end_id[d] = -1; \
		} \
		if(head_unload_id[d] != -1) { \
			cancel_event(this, head_unload_id[d]); \
			head_unload_id[d] = -1; \
		} \
	} \
}

void UPD765A::initialize()
{
	// initialize d88 handler
	for(int i = 0; i < 4; i++) {
		disk[i] = new DISK(emu);
		disk[i]->set_device_name(_T("%s/Disk #%d"), this_device_name, i + 1);
	}
	
	// initialize noise
	if(d_noise_seek != NULL) {
		d_noise_seek->set_device_name(_T("Noise Player (FDD Seek)"));
		if(!d_noise_seek->load_wav_file(_T("FDDSEEK.WAV"))) {
			if(!d_noise_seek->load_wav_file(_T("FDDSEEK1.WAV"))) {
				d_noise_seek->load_wav_file(_T("SEEK.WAV"));
			}
		}
		d_noise_seek->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_down != NULL) {
		d_noise_head_down->set_device_name(_T("Noise Player (FDD Head Load)"));
		d_noise_head_down->load_wav_file(_T("HEADDOWN.WAV"));
		d_noise_head_down->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_up != NULL) {
		d_noise_head_up->set_device_name(_T("Noise Player (FDD Head Unload)"));
		d_noise_head_up->load_wav_file(_T("HEADUP.WAV"));
		d_noise_head_up->set_mute(!config.sound_noise_fdd);
	}
	
	// initialize fdc
	memset(fdc, 0, sizeof(fdc));
	memset(buffer, 0, sizeof(buffer));
	
	phase = prevphase = PHASE_IDLE;
	status = S_RQM;
	seekstat = 0;
	bufptr = buffer; // temporary
	phase_id = drq_id = lost_id = result7_id = -1;
	for(int i = 0; i < 4; i++) {
		seek_step_id[i] = seek_end_id[i] = head_unload_id[i] = -1;
	}
	step_rate_time = head_unload_time = 0;
	no_dma_mode = false;
	motor_on = false;	// motor off
	reset_signal = true;
	irq_masked = drq_masked = false;
#ifdef UPD765A_DMA_MODE
	dma_data_lost = false;
#endif
	
	set_irq(false);
	set_drq(false);
#ifdef UPD765A_EXT_DRVSEL
	hdu = 0;
#else
	set_hdu(0);
#endif
	
	// index hole event
	if(outputs_index.count) {
		register_event(this, EVENT_INDEX, 4, true, NULL);
		prev_index = false;
	}
}

void UPD765A::release()
{
	for(int i = 0; i < 4; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}

void UPD765A::reset()
{
	shift_to_idle();
//	CANCEL_EVENT();
	phase_id = drq_id = lost_id = result7_id = -1;
	for(int i = 0; i < 4; i++) {
		if(seek_step_id[i] != -1) {
			// loop events are not canceled automatically in EVENT::reset()
			cancel_event(this, seek_step_id[i]);
		}
		seek_step_id[i] = seek_end_id[i] = head_unload_id[i] = -1;
	}
	set_irq(false);
	set_drq(false);
}

static const char* get_command_name(uint8_t data)
{
	static char name[16];
	
	switch(data & 0x1f) {
	case 0x02:
		my_sprintf_s(name, 16, _T("RD DIAGNOSTIC"));
		break;
	case 0x03:
		my_sprintf_s(name, 16, _T("SPECIFY      "));
		break;
	case 0x04:
		my_sprintf_s(name, 16, _T("SENCE DEVSTAT"));
		break;
	case 0x05:
	case 0x09:
		my_sprintf_s(name, 16, _T("WRITE DATA   "));
		break;
	case 0x06:
	case 0x0c:
		my_sprintf_s(name, 16, _T("READ DATA    "));
		break;
	case 0x07:
		my_sprintf_s(name, 16, _T("RECALIB      "));
		break;
	case 0x08:
		my_sprintf_s(name, 16, _T("SENCE INTSTAT"));
		break;
	case 0x0a:
		my_sprintf_s(name, 16, _T("READ ID      "));
		break;
	case 0x0d:
		my_sprintf_s(name, 16, _T("WRITE ID     "));
		break;
	case 0x0f:
		my_sprintf_s(name, 16, _T("SEEK         "));
		break;
	case 0x11:
	case 0x19:
	case 0x1d:
		my_sprintf_s(name, 16, _T("SCAN         "));
		break;
	default:
		my_sprintf_s(name, 16, _T("INVALID      "));
		break;
	}
	return name;
}

void UPD765A::write_io8(uint32_t addr, uint32_t data)
{
	if(addr & 1) {
		// fdc data
		if((status & (S_RQM | S_DIO)) == S_RQM) {
			status &= ~S_RQM;
			
			switch(phase) {
			case PHASE_IDLE:
#ifdef _FDC_DEBUG_LOG
				this->out_debug_log(_T("FDC: CMD=%2x %s\n"), data, get_command_name(data));
#endif
				command = data;
				process_cmd(command & 0x1f);
				break;
				
			case PHASE_CMD:
#ifdef _FDC_DEBUG_LOG
				this->force_out_debug_log(_T("FDC: PARAM=%2x\n"), data);
#endif
				*bufptr++ = data;
				if(--count) {
					status |= S_RQM;
				} else {
					process_cmd(command & 0x1f);
				}
				break;
				
			case PHASE_WRITE:
#ifdef _FDC_DEBUG_LOG
				this->force_out_debug_log(_T("FDC: WRITE=%2x\n"), data);
#endif
				*bufptr++ = data;
				set_drq(false);
				if(--count) {
					REGISTER_DRQ_EVENT();
				} else {
					process_cmd(command & 0x1f);
				}
				fdc[hdu & DRIVE_MASK].access = true;
				break;
				
			case PHASE_SCAN:
				if(data != 0xff) {
					if(((command & 0x1f) == 0x11 && *bufptr != data) ||
					   ((command & 0x1f) == 0x19 && *bufptr >  data) ||
					   ((command & 0x1f) == 0x1d && *bufptr <  data)) {
						result &= ~ST2_SH;
					}
				}
				bufptr++;
				set_drq(false);
				if(--count) {
					REGISTER_DRQ_EVENT();
				} else {
					cmd_scan();
				}
				fdc[hdu & DRIVE_MASK].access = true;
				break;
			}
		}
	}
}

uint32_t UPD765A::read_io8(uint32_t addr)
{
	if(addr & 1) {
		// fdc data
		if((status & (S_RQM | S_DIO)) == (S_RQM | S_DIO)) {
			uint8_t data;
			status &= ~S_RQM;
			
			switch(phase) {
			case PHASE_RESULT:
				data = *bufptr++;
#ifdef _FDC_DEBUG_LOG
				this->force_out_debug_log(_T("FDC: RESULT=%2x\n"), data);
#endif
				if(--count) {
					status |= S_RQM;
				} else {
					// EPSON QC-10 CP/M Plus
					bool clear_irq = true;
					if((command & 0x1f) == 0x08) {
						for(int i = 0; i < 4; i++) {
							if(fdc[i].result) {
								clear_irq = false;
								break;
							}
						}
					}
					if(clear_irq) {
						set_irq(false);
					}
					shift_to_idle();
				}
				return data;
				
			case PHASE_READ:
				data = *bufptr++;
#ifdef _FDC_DEBUG_LOG
				this->force_out_debug_log(_T("FDC: READ=%2x\n"), data);
#endif
				set_drq(false);
				if(--count) {
					REGISTER_DRQ_EVENT();
				} else {
					process_cmd(command & 0x1f);
				}
				fdc[hdu & DRIVE_MASK].access = true;
				return data;
			}
		}
		return 0xff;
	} else {
		// FIXME: dirty patch for PC-8801 Kimochi Disk 2
		if(phase_id != -1 && event_phase == PHASE_EXEC) {
			cancel_event(this, phase_id);
			phase_id = -1;
			phase = event_phase;
			process_cmd(command & 0x1f);
		}
		// fdc status
#ifdef _FDC_DEBUG_LOG
//		this->out_debug_log(_T("FDC: STATUS=%2x\n"), seekstat | status);
#endif
		return seekstat | status;
	}
}

void UPD765A::write_dma_io8(uint32_t addr, uint32_t data)
{
#ifdef UPD765A_DMA_MODE
	// EPSON QC-10 CP/M Plus
	dma_data_lost = false;
#endif
	write_io8(1, data);
}

uint32_t UPD765A::read_dma_io8(uint32_t addr)
{
#ifdef UPD765A_DMA_MODE
	// EPSON QC-10 CP/M Plus
	dma_data_lost = false;
#endif
	return read_io8(1);
}

void UPD765A::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_UPD765A_RESET) {
		bool next = ((data & mask) != 0);
		if(!reset_signal && next) {
			reset();
		}
		reset_signal = next;
	} else if(id == SIG_UPD765A_TC) {
		if(phase == PHASE_EXEC || phase == PHASE_READ || phase == PHASE_WRITE || phase == PHASE_SCAN || (phase == PHASE_RESULT && count == 7)) {
			if(data & mask) {
				if((phase == PHASE_READ  && ((command & 0x1f) == 0x06 || (command & 0x1f) == 0x0c) && count > 0) ||
				   (phase == PHASE_WRITE && ((command & 0x1f) == 0x05 || (command & 0x1f) == 0x09) && count > 0)) {
					if(status & S_RQM) {
						if(no_dma_mode) {
							write_signals(&outputs_irq, 0);
						} else {
							write_signals(&outputs_drq, 0);
						}
						status &= ~S_RQM;
					}
					CANCEL_EVENT();
					REGISTER_PHASE_EVENT_NEW(PHASE_TC, disk[hdu & DRIVE_MASK]->get_usec_per_bytes(count));
				} else {
					prevphase = phase;
					phase = PHASE_TC;
					process_cmd(command & 0x1f);
				}
			}
		}
	} else if(id == SIG_UPD765A_MOTOR) {
		motor_on = ((data & mask) != 0);
	} else if(id == SIG_UPD765A_MOTOR_NEG) {
		motor_on = ((data & mask) == 0);
#ifdef UPD765A_EXT_DRVSEL
	} else if(id == SIG_UPD765A_DRVSEL) {
		hdu = (hdu & 4) | (data & DRIVE_MASK);
		write_signals(&outputs_hdu, hdu);
#endif
	} else if(id == SIG_UPD765A_IRQ_MASK) {
		if(!(irq_masked = ((data & mask) != 0))) {
			write_signals(&outputs_irq, 0);
		}
	} else if(id == SIG_UPD765A_DRQ_MASK) {
		if(!(drq_masked = ((data & mask) != 0))) {
			write_signals(&outputs_drq, 0);
		}
	} else if(id == SIG_UPD765A_FREADY) {
		// for NEC PC-98x1 series
		force_ready = ((data & mask) != 0);
	}
}

uint32_t UPD765A::read_signal(int ch)
{
	// get access status
	uint32_t stat = 0;
	for(int i = 0; i < 4; i++) {
		if(fdc[i].access) {
			stat |= 1 << i;
		}
		fdc[i].access = false;
	}
	return stat;
}

void UPD765A::event_callback(int event_id, int err)
{
	if(event_id == EVENT_PHASE) {
		phase_id = -1;
		prevphase = phase;
		phase = event_phase;
		process_cmd(command & 0x1f);
	} else if(event_id == EVENT_DRQ) {
		drq_id = -1;
		status |= S_RQM;
		
		int drv = hdu & DRIVE_MASK;
		fdc[drv].cur_position = (fdc[drv].cur_position + 1) % disk[drv]->get_track_size();
		fdc[drv].prev_clock = prev_drq_clock = get_current_clock();
		set_drq(true);
	} else if(event_id == EVENT_LOST) {
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: DATA LOST\n"));
#endif
		lost_id = -1;
		result = ST1_OR;
		set_drq(false);
		shift_to_result7();
	} else if(event_id == EVENT_RESULT7) {
		result7_id = -1;
		shift_to_result7_event();
	} else if(event_id == EVENT_INDEX) {
		// index hole signal width is 5msec (thanks Mr.Sato)
		int drv = hdu & DRIVE_MASK;
		bool now_index = (disk[drv]->inserted && get_cur_position(drv) < disk[drv]->get_bytes_per_usec(5000));
		if(prev_index != now_index) {
			write_signals(&outputs_index, now_index ? 0xffffffff : 0);
			prev_index = now_index;
		}
	} else if(event_id >= EVENT_SEEK_STEP && event_id < EVENT_SEEK_STEP + 4) {
		int drv = event_id - EVENT_SEEK_STEP;
		if(fdc[drv].cur_track < fdc[drv].track) {
			fdc[drv].cur_track++;
			if(d_noise_seek != NULL) d_noise_seek->play();
		} else if(fdc[drv].cur_track > fdc[drv].track) {
			fdc[drv].cur_track--;
			if(d_noise_seek != NULL) d_noise_seek->play();
		}
		if(fdc[drv].cur_track == fdc[drv].track) {
			cancel_event(this, seek_step_id[drv]);
			seek_step_id[drv] = -1;
		}
	} else if(event_id >= EVENT_SEEK_END && event_id < EVENT_SEEK_END + 4) {
		int drv = event_id - EVENT_SEEK_END;
		if(seek_step_id[drv] != -1) {
			// to make sure...
			cancel_event(this, seek_step_id[drv]);
			seek_step_id[drv] = -1;
		}
		seek_end_id[drv] = -1;
		seek_event(drv);
	} else if(event_id >= EVENT_UNLOAD && event_id < EVENT_UNLOAD + 4) {
		int drv = event_id - EVENT_UNLOAD;
		if(fdc[drv].head_load) {
			if(d_noise_head_up != NULL) d_noise_head_up->play();
			fdc[drv].head_load = false;
		}
		head_unload_id[drv] = -1;
	}
}

void UPD765A::set_irq(bool val)
{
#ifdef _FDC_DEBUG_LOG
//	this->out_debug_log(_T("FDC: IRQ=%d\n"), val ? 1 : 0);
#endif
	write_signals(&outputs_irq, (val && !irq_masked) ? 0xffffffff : 0);
}

void UPD765A::set_drq(bool val)
{
#ifdef _FDC_DEBUG_LOG
//	this->out_debug_log(_T("FDC: DRQ=%d\n"), val ? 1 : 0);
#endif
	// cancel next drq and data lost events
	if(drq_id != -1) {
		cancel_event(this, drq_id);
	}
	if(lost_id != -1) {
		cancel_event(this, lost_id);
	}
	drq_id = lost_id = -1;
	// register data lost event if data exists
	if(val) {
#ifdef UPD765A_DMA_MODE
		// EPSON QC-10 CP/M Plus
		dma_data_lost = true;
#else
		if((command & 0x1f) != 0x0d) {
			register_event(this, EVENT_LOST, disk[hdu & DRIVE_MASK]->get_usec_per_bytes(1), false, &lost_id);
		} else {
			// FIXME: write id
			register_event(this, EVENT_LOST, 30000, false, &lost_id);
		}
#endif
	}
	if(no_dma_mode) {
		write_signals(&outputs_irq, (val && !irq_masked) ? 0xffffffff : 0);
	} else {
		write_signals(&outputs_drq, (val && !drq_masked) ? 0xffffffff : 0);
#ifdef UPD765A_DMA_MODE
		// EPSON QC-10 CP/M Plus
		if(val && dma_data_lost) {
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC: DATA LOST (DMA)\n"));
#endif
			result = ST1_OR;
			write_signals(&outputs_drq, 0);
			shift_to_result7();
		}
#endif
	}
}

void UPD765A::set_hdu(uint8_t val)
{
#ifdef UPD765A_EXT_DRVSEL
	hdu = (hdu & 3) | (val & 4);
#else
	hdu = val;
#endif
	write_signals(&outputs_hdu, hdu);
}

// ----------------------------------------------------------------------------
// command
// ----------------------------------------------------------------------------

void UPD765A::process_cmd(int cmd)
{
	switch(cmd & 0x1f) {
	case 0x02:
		cmd_read_diagnostic();
		break;
	case 0x03:
		cmd_specify();
		break;
	case 0x04:
		cmd_sence_devstat();
		break;
	case 0x05:
	case 0x09:
		cmd_write_data();
		break;
	case 0x06:
	case 0x0c:
		cmd_read_data();
		break;
	case 0x07:
		cmd_recalib();
		break;
	case 0x08:
		cmd_sence_intstat();
		break;
	case 0x0a:
		cmd_read_id();
		break;
	case 0x0d:
		cmd_write_id();
		break;
	case 0x0f:
		cmd_seek();
		break;
	case 0x11:
	case 0x19:
	case 0x1d:
		cmd_scan();
		break;
	default:
		cmd_invalid();
		break;
	}
}

void UPD765A::cmd_sence_devstat()
{
	switch(phase) {
	case PHASE_IDLE:
		shift_to_cmd(1);
		break;
	case PHASE_CMD:
		set_hdu(buffer[0]);
		buffer[0] = get_devstat(buffer[0] & DRIVE_MASK);
		shift_to_result(1);
		break;
	}
}

void UPD765A::cmd_sence_intstat()
{
	for(int i = 0; i < 4; i++) {
		if(fdc[i].result) {
			buffer[0] = (uint8_t)fdc[i].result;
			buffer[1] = (uint8_t)fdc[i].track;
			fdc[i].result = 0;
			shift_to_result(2);
			return;
		}
	}
#ifdef UPD765A_SENCE_INTSTAT_RESULT
	// IBM PC/JX
	buffer[0] = (uint8_t)ST0_AI;
#else
	buffer[0] = (uint8_t)ST0_IC;
#endif
	shift_to_result(1);
//	status &= ~S_CB;
}

uint8_t UPD765A::get_devstat(int drv)
{
	if(drv >= MAX_DRIVE) {
		return ST3_FT | drv;
	}
	// XM8 version 1.20
	if(force_ready && !disk[drv]->inserted) {
		return drv;
	}
//	return drv | ((fdc[drv].track & 1) ? ST3_HD : 0) | (disk[drv]->inserted && disk[drv]->two_side ? ST3_TS : 0) | (fdc[drv].track ? 0 : ST3_T0) | (force_ready || disk[drv]->inserted ? ST3_RY : 0) | (disk[drv]->write_protected ? ST3_WP : 0);
	return drv | ((fdc[drv].track & 1) ? ST3_HD : 0) | ST3_TS | (fdc[drv].track ? 0 : ST3_T0) | (force_ready || disk[drv]->inserted ? ST3_RY : 0) | (disk[drv]->write_protected ? ST3_WP : 0);
}

void UPD765A::cmd_seek()
{
	switch(phase) {
	case PHASE_IDLE:
		shift_to_cmd(2);
		break;
	case PHASE_CMD:
		seek(buffer[0] & DRIVE_MASK, buffer[1]);
		shift_to_idle();
		break;
	}
}

void UPD765A::cmd_recalib()
{
	switch(phase) {
	case PHASE_IDLE:
		shift_to_cmd(1);
		break;
	case PHASE_CMD:
		seek(buffer[0] & DRIVE_MASK, 0);
		shift_to_idle();
		break;
	}
}

void UPD765A::seek(int drv, int trk)
{
	// get distance
	int steptime = (32 - 2 * step_rate_time) * 1000; // msec -> usec
	if(disk[drv]->drive_type == DRIVE_TYPE_2HD) {
		steptime /= 2;
	}
	int seektime = (trk == fdc[drv].track) ? 120 : steptime * abs(trk - fdc[drv].track) + 500; // usec
	
	if(drv >= MAX_DRIVE) {
		// invalid drive number
		fdc[drv].result = (drv & DRIVE_MASK) | ST0_SE | ST0_NR | ST0_AT;
		set_irq(true);
	} else {
		fdc[drv].cur_track = fdc[drv].track;
		fdc[drv].track = trk;
#ifdef UPD765A_DONT_WAIT_SEEK
		if(fdc[drv].cur_track != fdc[drv].track) {
			if(d_noise_seek != NULL) d_noise_seek->play();
		}
		seek_event(drv);
#else
		if(seek_step_id[drv] != -1) {
			cancel_event(this, seek_step_id[drv]);
		}
		if(seek_end_id[drv] != -1) {
			cancel_event(this, seek_end_id[drv]);
		}
		if(fdc[drv].cur_track != fdc[drv].track) {
			register_event(this, EVENT_SEEK_STEP + drv, steptime, true, &seek_step_id[drv]);
		} else {
			seek_step_id[drv] = -1;
		}
		register_event(this, EVENT_SEEK_END + drv, seektime, false, &seek_end_id[drv]);
		seekstat |= 1 << drv;
#endif
	}
}

void UPD765A::seek_event(int drv)
{
	int trk = fdc[drv].track;
	
	if(drv >= MAX_DRIVE) {
		fdc[drv].result = (drv & DRIVE_MASK) | ST0_SE | ST0_NR | ST0_AT;
	} else if(force_ready || disk[drv]->inserted) {
		fdc[drv].result = (drv & DRIVE_MASK) | ST0_SE;
	} else {
#ifdef UPD765A_NO_ST0_AT_FOR_SEEK
		// for NEC PC-100
		fdc[drv].result = (drv & DRIVE_MASK) | ST0_SE | ST0_NR;
#else
		fdc[drv].result = (drv & DRIVE_MASK) | ST0_SE | ST0_NR | ST0_AT;
#endif
	}
	set_irq(true);
	seekstat &= ~(1 << drv);
	
	// reset dsch flag
	disk[drv]->changed = false;
}

void UPD765A::cmd_read_data()
{
	switch(phase) {
	case PHASE_IDLE:
		shift_to_cmd(8);
		break;
	case PHASE_CMD:
		get_sector_params();
		start_transfer();
		REGISTER_PHASE_EVENT_NEW(PHASE_EXEC, get_usec_to_exec_phase());
		break;
	case PHASE_EXEC:
		// XM8 version 1.20
		if(force_ready && !disk[hdu & DRIVE_MASK]->inserted) {
			REGISTER_PHASE_EVENT(PHASE_EXEC, 1000000);
			break;
		}
		read_data((command & 0x1f) == 12, false);
		break;
	case PHASE_READ:
		if(result) {
			shift_to_result7();
			break;
		}
		if(!id_incr()) {
			REGISTER_PHASE_EVENT(PHASE_TIMER, 2000);
			break;
		}
		REGISTER_PHASE_EVENT_NEW(PHASE_EXEC, get_usec_to_exec_phase());
		break;
	case PHASE_TC:
		CANCEL_EVENT();
		shift_to_result7();
		break;
	case PHASE_TIMER:
//		result = ST0_AT | ST1_EN;
		result = ST1_EN;
		shift_to_result7();
		break;
	}
}

void UPD765A::cmd_write_data()
{
	switch(phase) {
	case PHASE_IDLE:
		shift_to_cmd(8);
		break;
	case PHASE_CMD:
		get_sector_params();
		start_transfer();
		REGISTER_PHASE_EVENT_NEW(PHASE_EXEC, get_usec_to_exec_phase());
		break;
	case PHASE_EXEC:
		// XM8 version 1.20
		if(force_ready && !disk[hdu & DRIVE_MASK]->inserted) {
			REGISTER_PHASE_EVENT(PHASE_EXEC, 1000000);
			break;
		}
		result = check_cond(true);
		if(result & ST1_MA) {
			REGISTER_PHASE_EVENT(PHASE_EXEC, 1000000);	// retry
			break;
		}
		if(!result) {
			result = find_id();
		}
		if(result) {
			shift_to_result7();
		} else {
			int length = 0x80 << min(id[3], 7);
			if(id[3] == 0) {
				length = min(dtl, 0x80);
				memset(buffer + length, 0, 0x80 - length);
			}
			shift_to_write(length);
		}
		break;
	case PHASE_WRITE:
		write_data((command & 0x1f) == 9);
		if(result) {
			shift_to_result7();
			break;
		}
		phase = PHASE_EXEC;
		if(!id_incr()) {
			REGISTER_PHASE_EVENT(PHASE_TIMER, 2000);
			break;
		}
		REGISTER_PHASE_EVENT_NEW(PHASE_EXEC, get_usec_to_exec_phase());
		break;
	case PHASE_TIMER:
//		result = ST0_AT | ST1_EN;
		result = ST1_EN;
		shift_to_result7();
		break;
	case PHASE_TC:
		CANCEL_EVENT();
		if(prevphase == PHASE_WRITE && bufptr != buffer) {
			// terminate while transfer ?
			memset(bufptr, 0, count);
			write_data((command & 0x1f) == 9);
		}
		shift_to_result7();
		break;
	}
}

void UPD765A::cmd_scan()
{
	switch(phase) {
	case PHASE_IDLE:
		shift_to_cmd(9);
		break;
	case PHASE_CMD:
		get_sector_params();
		dtl = dtl | 0x100;
		start_transfer();
		REGISTER_PHASE_EVENT_NEW(PHASE_EXEC, get_usec_to_exec_phase());
		break;
	case PHASE_EXEC:
		// XM8 version 1.20
		if(force_ready && !disk[hdu & DRIVE_MASK]->inserted) {
			REGISTER_PHASE_EVENT(PHASE_EXEC, 1000000);
			break;
		}
		read_data(false, true);
		break;
	case PHASE_SCAN:
		if(result) {
			shift_to_result7();
			break;
		}
		phase = PHASE_EXEC;
		if(!id_incr()) {
			REGISTER_PHASE_EVENT(PHASE_TIMER, 2000);
			break;
		}
		REGISTER_PHASE_EVENT_NEW(PHASE_EXEC, get_usec_to_exec_phase());
		break;
	case PHASE_TC:
		CANCEL_EVENT();
		shift_to_result7();
		break;
	case PHASE_TIMER:
//		result = ST0_AT | ST1_EN;
		result = ST1_EN;
		shift_to_result7();
		break;
	}
}

void UPD765A::cmd_read_diagnostic()
{
	switch(phase) {
	case PHASE_IDLE:
		shift_to_cmd(8);
		break;
	case PHASE_CMD:
		get_sector_params();
		start_transfer();
		REGISTER_PHASE_EVENT_NEW(PHASE_EXEC, get_usec_to_exec_phase());
		break;
	case PHASE_EXEC:
		// XM8 version 1.20
		if(force_ready && !disk[hdu & DRIVE_MASK]->inserted) {
			REGISTER_PHASE_EVENT(PHASE_EXEC, 1000000);
			break;
		}
		read_diagnostic();
		break;
	case PHASE_READ:
		if(result & ~ST1_ND) {
			shift_to_result7();
			break;
		}
		if(!id_incr()) {
			REGISTER_PHASE_EVENT(PHASE_TIMER, 2000);
			break;
		}
		REGISTER_PHASE_EVENT_NEW(PHASE_EXEC, get_usec_to_exec_phase());
		break;
	case PHASE_TC:
		CANCEL_EVENT();
		shift_to_result7();
		break;
	case PHASE_TIMER:
//		result |= ST0_AT | ST1_EN;
		result |= ST1_EN;
		shift_to_result7();
		break;
	}
}

void UPD765A::read_data(bool deleted, bool scan)
{
	result = check_cond(false);
	if(result & ST1_MA) {
		REGISTER_PHASE_EVENT(PHASE_EXEC, 10000);
		return;
	}
	if(result) {
		shift_to_result7();
		return;
	}
	result = read_sector();
	if(deleted) {
		result ^= ST2_CM;
	}
	if((result & ~ST2_CM) && !(result & ST2_DD)) {
		shift_to_result7();
		return;
	}
	if((result & ST2_CM) && (command & 0x20)) {
		REGISTER_PHASE_EVENT(PHASE_TIMER, 100000);
		return;
	}
	int length = (id[3] != 0) ? (0x80 << min(id[3], 7)) : (min(dtl, 0x80));
	if(!scan) {
		shift_to_read(length);
	} else {
		shift_to_scan(length);
	}
	return;
}

void UPD765A::write_data(bool deleted)
{
	if((result = check_cond(true)) != 0) {
		shift_to_result7();
		return;
	}
	result = write_sector(deleted);
	return;
}

void UPD765A::read_diagnostic()
{
	int drv = hdu & DRIVE_MASK;
	int trk = fdc[drv].track;
	int side = (hdu >> 2) & 1;
	
	result = check_cond(false);
	if(result & ST1_MA) {
		REGISTER_PHASE_EVENT(PHASE_EXEC, 10000);
		return;
	}
	if(result) {
		shift_to_result7();
		return;
	}
	if(!disk[drv]->make_track(trk, side)) {
//		result = ST1_ND;
		result = ST0_AT | ST1_MA;
		shift_to_result7();
		return;
	}
	if((command & 0x40) != (disk[drv]->track_mfm ? 0x40 : 0)) {
//		result = ST1_ND;
		result = ST0_AT | ST1_MA;
		shift_to_result7();
		return;
	}
	if(disk[drv]->get_sector(trk, side, 0)) {
#if 0
		if(disk[drv]->id[0] != id[0] || disk[drv]->id[1] != id[1] || disk[drv]->id[2] != id[2] /*|| disk[drv]->id[3] != id[3]*/) {
#else
		if(disk[drv]->id[0] != id[0] || disk[drv]->id[1] != id[1] || disk[drv]->id[2] != id[2] || disk[drv]->id[3] != id[3]) {
#endif
			result = ST1_ND;
		}
	}
	
	// FIXME: we need to consider the case that the first sector does not have a data field
	// start reading at the first sector data
	memcpy(buffer, disk[drv]->track + disk[drv]->data_position[0], disk[drv]->get_track_size() - disk[drv]->data_position[0]);
	memcpy(buffer + disk[drv]->get_track_size() - disk[drv]->data_position[0], disk[drv]->track, disk[drv]->data_position[0]);
	fdc[drv].next_trans_position = disk[drv]->data_position[0];
	
	shift_to_read(0x80 << min(id[3], 7));
	return;
}

uint32_t UPD765A::read_sector()
{
	int drv = hdu & DRIVE_MASK;
	int trk = fdc[drv].track;
	int side = (hdu >> 2) & 1;
	
	// get sector counts in the current track
	if(!disk[drv]->make_track(trk, side)) {
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: TRACK NOT FOUND (TRK=%d SIDE=%d)\n"), trk, side);
#endif
		return ST0_AT | ST1_MA;
	}
	if((command & 0x40) != (disk[drv]->track_mfm ? 0x40 : 0)) {
		return ST0_AT | ST1_MA;
	}
	int secnum = disk[drv]->sector_num.sd;
	if(!secnum) {
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: NO SECTORS IN TRACK (TRK=%d SIDE=%d)\n"), trk, side);
#endif
		return ST0_AT | ST1_MA;
	}
	int cy = -1;
	for(int i = 0; i < secnum; i++) {
		if(!disk[drv]->get_sector(trk, side, i)) {
			continue;
		}
		cy = disk[drv]->id[0];
#if 0
		if(disk[drv]->id[0] != id[0] || disk[drv]->id[1] != id[1] || disk[drv]->id[2] != id[2] /*|| disk[drv]->id[3] != id[3]*/) {
#else
		if(disk[drv]->id[0] != id[0] || disk[drv]->id[1] != id[1] || disk[drv]->id[2] != id[2] || disk[drv]->id[3] != id[3]) {
#endif
			continue;
		}
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: SECTOR FOUND (TRK=%d SIDE=%d ID=%2x,%2x,%2x,%2x)\n"), trk, side, id[0], id[1], id[2], id[3]);
#endif
		if(disk[drv]->sector_size.sd == 0) {
			continue;
		}
		// sector number is matched
		if(disk[drv]->invalid_format) {
			memset(buffer, disk[drv]->drive_mfm ? 0x4e : 0xff, sizeof(buffer));
			memcpy(buffer, disk[drv]->sector, disk[drv]->sector_size.sd);
		} else {
			memcpy(buffer, disk[drv]->track + disk[drv]->data_position[i], disk[drv]->get_track_size() - disk[drv]->data_position[i]);
			memcpy(buffer + disk[drv]->get_track_size() - disk[drv]->data_position[i], disk[drv]->track, disk[drv]->data_position[i]);
		}
		fdc[drv].next_trans_position = disk[drv]->data_position[i];
		
		if((disk[drv]->addr_crc_error || disk[drv]->data_crc_error) && !disk[drv]->ignore_crc()) {
			return ST0_AT | ST1_DE | (disk[drv]->data_crc_error ? ST2_DD : 0);
		}
		if(disk[drv]->deleted) {
			return ST2_CM;
		}
		return 0;
	}
#ifdef _FDC_DEBUG_LOG
	this->out_debug_log(_T("FDC: SECTOR NOT FOUND (TRK=%d SIDE=%d ID=%2x,%2x,%2x,%2x)\n"), trk, side, id[0], id[1], id[2], id[3]);
#endif
	if(cy != id[0] && cy != -1) {
		if(cy == 0xff) {
			return ST0_AT | ST1_ND | ST2_BC;
		} else {
			return ST0_AT | ST1_ND | ST2_NC;
		}
	}
	return ST0_AT | ST1_ND;
}

uint32_t UPD765A::write_sector(bool deleted)
{
	int drv = hdu & DRIVE_MASK;
	int trk = fdc[drv].track;
	int side = (hdu >> 2) & 1;
	
	if(disk[drv]->write_protected) {
		return ST0_AT | ST1_NW;
	}
	// get sector counts in the current track
	if(!disk[drv]->get_track(trk, side)) {
		return ST0_AT | ST1_MA;
	}
	int secnum = disk[drv]->sector_num.sd;
	if(!secnum) {
		return ST0_AT | ST1_MA;
	}
	int cy = -1;
	for(int i = 0; i < secnum; i++) {
		if(!disk[drv]->get_sector(trk, side, i)) {
			continue;
		}
		cy = disk[drv]->id[0];
		if(disk[drv]->id[0] != id[0] || disk[drv]->id[1] != id[1] || disk[drv]->id[2] != id[2] /*|| disk[drv]->id[3] != id[3]*/) {
			continue;
		}
		if(disk[drv]->sector_size.sd == 0) {
			continue;
		}
		// sector number is matched
		int size = 0x80 << min(id[3], 7);
		memcpy(disk[drv]->sector, buffer, min(size, disk[drv]->sector_size.sd));
		disk[drv]->set_deleted(deleted);
		return 0;
	}
	if(cy != id[0] && cy != -1) {
		if(cy == 0xff) {
			return ST0_AT | ST1_ND | ST2_BC;
		} else {
			return ST0_AT | ST1_ND | ST2_NC;
		}
	}
	return ST0_AT | ST1_ND;
}

uint32_t UPD765A::find_id()
{
	int drv = hdu & DRIVE_MASK;
	int trk = fdc[drv].track;
	int side = (hdu >> 2) & 1;
	
	// get sector counts in the current track
	if(!disk[drv]->get_track(trk, side)) {
		return ST0_AT | ST1_MA;
	}
	if((command & 0x40) != (disk[drv]->track_mfm ? 0x40 : 0)) {
		return ST0_AT | ST1_MA;
	}
	int secnum = disk[drv]->sector_num.sd;
	if(!secnum) {
		return ST0_AT | ST1_MA;
	}
	int cy = -1;
	for(int i = 0; i < secnum; i++) {
		if(!disk[drv]->get_sector(trk, side, i)) {
			continue;
		}
		cy = disk[drv]->id[0];
		if(disk[drv]->id[0] != id[0] || disk[drv]->id[1] != id[1] || disk[drv]->id[2] != id[2] /*|| disk[drv]->id[3] != id[3]*/) {
			continue;
		}
		if(disk[drv]->sector_size.sd == 0) {
			continue;
		}
		// sector number is matched
		fdc[drv].next_trans_position = disk[drv]->data_position[i];
		return 0;
	}
	if(cy != id[0] && cy != -1) {
		if(cy == 0xff) {
			return ST0_AT | ST1_ND | ST2_BC;
		} else {
			return ST0_AT | ST1_ND | ST2_NC;
		}
	}
	return ST0_AT | ST1_ND;
}

uint32_t UPD765A::check_cond(bool write)
{
	int drv = hdu & DRIVE_MASK;
	hdue = hdu;
	if(drv >= MAX_DRIVE) {
		return ST0_AT | ST0_NR;
	}
	if(!disk[drv]->inserted) {
		return ST0_AT | ST1_MA;
	}
	return 0;
}

void UPD765A::get_sector_params()
{
	set_hdu(buffer[0]);
	hdue = buffer[0];
	id[0] = buffer[1];
	id[1] = buffer[2];
	id[2] = buffer[3];
	id[3] = buffer[4];
	eot = buffer[5];
	gpl = buffer[6];
	dtl = buffer[7];
}

bool UPD765A::id_incr()
{
	if((command & 19) == 17) {
		// scan equal
		if((dtl & 0xff) == 0x02) {
			id[2]++;
		}
	}
	if(id[2]++ != eot) {
		return true;
	}
	id[2] = 1;
	if(command & 0x80) {
		set_hdu(hdu ^ 4);
		id[1] ^= 1;
		if(id[1] & 1) {
			return true;
		}
	}
	id[0]++;
	return false;
}

void UPD765A::cmd_read_id()
{
	switch(phase) {
	case PHASE_IDLE:
		shift_to_cmd(1);
		break;
	case PHASE_CMD:
		set_hdu(buffer[0]);
		start_transfer();
//		break;
	case PHASE_EXEC:
		// XM8 version 1.20
		if(force_ready && !disk[hdu & DRIVE_MASK]->inserted) {
			REGISTER_PHASE_EVENT(PHASE_EXEC, 1000000);
			break;
		}
		if(check_cond(false) & ST1_MA) {
//			REGISTER_PHASE_EVENT(PHASE_EXEC, 1000000);
//			break;
		}
		if((result = read_id()) == 0) {
			int drv = hdu & DRIVE_MASK;
			int bytes = fdc[drv].next_trans_position - get_cur_position(drv);
			if(bytes < 0) {
				bytes += disk[drv]->get_track_size();
			}
			REGISTER_PHASE_EVENT_NEW(PHASE_TIMER, disk[drv]->get_usec_per_bytes(bytes));
		} else {
			REGISTER_PHASE_EVENT(PHASE_TIMER, 5000);
		}
		break;
	case PHASE_TIMER:
		shift_to_result7();
		break;
	}
}

void UPD765A::cmd_write_id()
{
	switch(phase) {
	case PHASE_IDLE:
		shift_to_cmd(5);
		break;
	case PHASE_CMD:
		set_hdu(buffer[0]);
		id[3] = buffer[1];
		eot = buffer[2];
		gpl = buffer[3];
		dtl = buffer[4]; // temporary
		if(!eot) {
			REGISTER_PHASE_EVENT(PHASE_TIMER, 1000000);
			break;
		}
		start_transfer();
		fdc[hdu & DRIVE_MASK].next_trans_position = get_cur_position(hdu & DRIVE_MASK);
		shift_to_write(4 * eot);
		break;
	case PHASE_WRITE:
		REGISTER_PHASE_EVENT(PHASE_TIMER, 4000000);
		break;
	case PHASE_TC:
#if 1
		if((result = check_cond(true)) == 0) {
			if(disk[hdu & DRIVE_MASK]->write_protected) {
				result = ST0_AT | ST1_NW;
			}
		}
		CANCEL_EVENT();
		shift_to_result7();
#else
		REGISTER_PHASE_EVENT(PHASE_TIMER, 4000000);
#endif
		break;
	case PHASE_TIMER:
		// XM8 version 1.20
		if(force_ready && !disk[hdu & DRIVE_MASK]->inserted) {
			REGISTER_PHASE_EVENT(PHASE_TIMER, 1000000);
			break;
		}
		result = write_id();
		shift_to_result7();
		break;
	}
}

uint32_t UPD765A::read_id()
{
	int drv = hdu & DRIVE_MASK;
	int trk = fdc[drv].track;
	int side = (hdu >> 2) & 1;
	
	// get sector counts in the current track
	if(!disk[drv]->get_track(trk, side)) {
		return ST0_AT | ST1_MA;
	}
	if((command & 0x40) != (disk[drv]->track_mfm ? 0x40 : 0)) {
		return ST0_AT | ST1_MA;
	}
	int secnum = disk[drv]->sector_num.sd;
	if(!secnum) {
		return ST0_AT | ST1_MA;
	}
	
	// first found sector
	int position = get_cur_position(drv), first_sector = 0;
	if(position > disk[drv]->sync_position[secnum - 1]) {
		position -= disk[drv]->get_track_size();
	}
	for(int i = 0; i < secnum; i++) {
		if(position < disk[drv]->sync_position[i]) {
			first_sector = i;
			break;
		}
	}
	for(int i = 0; i < secnum; i++) {
		int index = (first_sector + i) % secnum;
		if(disk[drv]->get_sector(trk, side, index)) {
			id[0] = disk[drv]->id[0];
			id[1] = disk[drv]->id[1];
			id[2] = disk[drv]->id[2];
			id[3] = disk[drv]->id[3];
			fdc[drv].next_trans_position = disk[drv]->id_position[index] + 6;
			return 0;
		}
	}
	return ST0_AT | ST1_ND;
}

uint32_t UPD765A::write_id()
{
	int drv = hdu & DRIVE_MASK;
	int trk = fdc[drv].track;
	int side = (hdu >> 2) & 1;
	int length = 0x80 << min(id[3], 7);
	
	if((result = check_cond(true)) != 0) {
		return result;
	}
	if(disk[drv]->write_protected) {
		return ST0_AT | ST1_NW;
	}
	
	disk[drv]->format_track(trk, side);
	disk[drv]->track_mfm = ((command & 0x40) != 0);
	
	for(int i = 0; i < eot && i < 256; i++) {
		for(int j = 0; j < 4; j++) {
			id[j] = buffer[4 * i + j];
		}
		disk[drv]->insert_sector(id[0], id[1], id[2], id[3], false, false, dtl, length);
	}
	return 0;
}

void UPD765A::cmd_specify()
{
	switch(phase) {
	case PHASE_IDLE:
		shift_to_cmd(2);
		break;
	case PHASE_CMD:
		step_rate_time = buffer[0] >> 4;
		head_unload_time = buffer[1] >> 1;
		no_dma_mode = ((buffer[1] & 1) != 0);
		shift_to_idle();
		status = 0x80;//0xff;
		break;
	}
}

void UPD765A::cmd_invalid()
{
	buffer[0] = (uint8_t)ST0_IC;
	shift_to_result(1);
}

void UPD765A::shift_to_idle()
{
	phase = PHASE_IDLE;
	status = S_RQM;
}

void UPD765A::shift_to_cmd(int length)
{
	phase = PHASE_CMD;
	status = S_RQM | S_CB;
	bufptr = buffer;
	count = length;
}

void UPD765A::shift_to_exec()
{
	phase = PHASE_EXEC;
	process_cmd(command & 0x1f);
}

void UPD765A::shift_to_read(int length)
{
	phase = PHASE_READ;
	status = S_RQM | S_DIO | S_NDM | S_CB;
	bufptr = buffer;
	count = length;
	
	int drv = hdu & DRIVE_MASK;
	fdc[drv].cur_position = fdc[drv].next_trans_position;
	fdc[drv].prev_clock = prev_drq_clock = get_current_clock();
	set_drq(true);
}

void UPD765A::shift_to_write(int length)
{
	phase = PHASE_WRITE;
	status = S_RQM | S_NDM | S_CB;
	bufptr = buffer;
	count = length;
	
	int drv = hdu & DRIVE_MASK;
	fdc[drv].cur_position = fdc[drv].next_trans_position;
	fdc[drv].prev_clock = prev_drq_clock = get_current_clock();
	set_drq(true);
}

void UPD765A::shift_to_scan(int length)
{
	phase = PHASE_SCAN;
	status = S_RQM | S_NDM | S_CB;
	result = ST2_SH;
	bufptr = buffer;
	count = length;
	
	int drv = hdu & DRIVE_MASK;
	fdc[drv].cur_position = fdc[drv].next_trans_position;
	fdc[drv].prev_clock = prev_drq_clock = get_current_clock();
	set_drq(true);
}

void UPD765A::shift_to_result(int length)
{
	phase = PHASE_RESULT;
	status = S_RQM | S_CB | S_DIO;
	bufptr = buffer;
	count = length;
}

void UPD765A::shift_to_result7()
{
#ifdef UPD765A_WAIT_RESULT7
	if(result7_id != -1) {
		cancel_event(this, result7_id);
		result7_id = -1;
	}
	if(phase != PHASE_TIMER) {
		register_event(this, EVENT_RESULT7, 100, false, &result7_id);
	} else
#endif
	shift_to_result7_event();
	finish_transfer();
}

void UPD765A::shift_to_result7_event()
{
#ifdef UPD765A_NO_ST1_EN_OR_FOR_RESULT7
	// for NEC PC-9801 (XANADU)
	result &= ~(ST1_EN | ST1_OR);
#endif
	buffer[0] = (result & 0xf8) | (hdue & 7);
	buffer[1] = uint8_t(result >>  8);
	buffer[2] = uint8_t(result >> 16);
	buffer[3] = id[0];
	buffer[4] = id[1];
	buffer[5] = id[2];
	buffer[6] = id[3];
	set_irq(true);
	shift_to_result(7);
}

void UPD765A::start_transfer()
{
	int drv = hdu & DRIVE_MASK;
	
	if(head_unload_id[drv] != -1) {
		cancel_event(this, head_unload_id[drv]);
		head_unload_id[drv] = -1;
	}
	if(!fdc[drv].head_load) {
		if(d_noise_head_down != NULL) d_noise_head_down->play();
		fdc[drv].head_load = true;
	}
}

void UPD765A::finish_transfer()
{
	int drv = hdu & DRIVE_MASK;
	
	if(fdc[drv].head_load) {
		if(head_unload_id[drv] != -1) {
			cancel_event(this, head_unload_id[drv]);
		}
		int time = (16 * (head_unload_time + 1)) * 1000; // msec -> usec
		if(disk[drv]->drive_type == DRIVE_TYPE_2HD) {
			time /= 2;
		}
		register_event(this, EVENT_UNLOAD + drv, time, false, &head_unload_id[drv]);
	}
}

// ----------------------------------------------------------------------------
// timing
// ----------------------------------------------------------------------------

int UPD765A::get_cur_position(int drv)
{
	return (fdc[drv].cur_position + disk[drv]->get_bytes_per_usec(get_passed_usec(fdc[drv].prev_clock))) % disk[drv]->get_track_size();
}

double UPD765A::get_usec_to_exec_phase()
{
	int drv = hdu & DRIVE_MASK;
	int trk = fdc[drv].track;
	int side = (hdu >> 2) & 1;
	
	if(/*disk[drv]->no_skew &&*/ !disk[drv]->correct_timing()) {
		// XXX: this image may be a standard image or coverted from a standard image and skew may be incorrect,
		// so use the constant period to go to exec phase
		return 100;
	}
	
	// search target sector
	int position = get_cur_position(drv);
	int trans_position = -1, sync_position;
	
	if(disk[drv]->get_track(trk, side) && disk[drv]->sector_num.sd != 0) {
		if((command & 0x1f) == 0x02) {
			// read diagnotics
			trans_position = disk[drv]->data_position[0];
			sync_position = disk[drv]->sync_position[0];
		} else {
			for(int i = 0; i < disk[drv]->sector_num.sd; i++) {
				if(!disk[drv]->get_sector(trk, side, i)) {
					continue;
				}
				if(disk[drv]->id[0] != id[0] || disk[drv]->id[1] != id[1] || disk[drv]->id[2] != id[2] /*|| disk[drv]->id[3] != id[3]*/) {
					continue;
				}
				// sector number is matched
				trans_position = disk[drv]->data_position[i];
				sync_position = disk[drv]->sync_position[i];
				break;
			}
		}
	}
	if(trans_position == -1) {
		// sector not found
		return 100;
	}
	
	// get current position
	int bytes = trans_position - position;
	if(sync_position < position) {
		bytes += disk[drv]->get_track_size();
	}
	return disk[drv]->get_usec_per_bytes(bytes);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void UPD765A::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->open(file_path, bank);
		if(disk[drv]->changed) {
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC: Disk Changed (Drive=%d)\n"), drv);
#endif
			if(raise_irq_when_media_changed) {
				fdc[drv].result = (drv & DRIVE_MASK) | ST0_AI | ST0_NR;
				set_irq(true);
			}
		}
	}
}

void UPD765A::close_disk(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]->inserted) {
		if(fdc[drv].head_load) {
			if(d_noise_head_up != NULL) d_noise_head_up->play();
			fdc[drv].head_load = false;
		}
		disk[drv]->close();
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: Disk Ejected (Drive=%d)\n"), drv);
#endif
		if(raise_irq_when_media_changed) {
			fdc[drv].result = (drv & DRIVE_MASK) | ST0_AI;
			set_irq(true);
		}
	}
}

bool UPD765A::is_disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

bool UPD765A::is_disk_inserted()
{
	int drv = hdu & DRIVE_MASK;
	return is_disk_inserted(drv);
}

bool UPD765A::disk_ejected(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->ejected;
	}
	return false;
}

bool UPD765A::disk_ejected()
{
	int drv = hdu & DRIVE_MASK;
	return disk_ejected(drv);
}

void UPD765A::is_disk_protected(int drv, bool value)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->write_protected = value;
	}
}

bool UPD765A::is_disk_protected(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->write_protected;
	}
	return false;
}

uint8_t UPD765A::get_media_type(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]->inserted) {
		return disk[drv]->media_type;
	}
	return MEDIA_TYPE_UNK;
}

void UPD765A::set_drive_type(int drv, uint8_t type)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_type = type;
	}
}

uint8_t UPD765A::get_drive_type(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->drive_type;
	}
	return DRIVE_TYPE_UNK;
}

void UPD765A::set_drive_rpm(int drv, int rpm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_rpm = rpm;
	}
}

void UPD765A::set_drive_mfm(int drv, bool mfm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_mfm = mfm;
	}
}

void UPD765A::update_config()
{
	if(d_noise_seek != NULL) {
		d_noise_seek->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_down != NULL) {
		d_noise_head_down->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_up != NULL) {
		d_noise_head_up->set_mute(!config.sound_noise_fdd);
	}
}

#ifdef USE_DEBUGGER
bool UPD765A::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	int drv = hdu & DRIVE_MASK;
	int side = (hdu >> 2) & 1;
	int position = get_cur_position(drv);
	
	my_stprintf_s(buffer, buffer_len,
	_T("CMD=%02X (%s) HDU=%02X C=%02X H=%02X R=%02X N=%02X EOT=%02X GPL=%02X DTL=%02X\n")
	_T("UNIT: DRIVE=%d TRACK=%2d(%2d) SIDE=%d POSITION=%5d/%d"),
	command, get_command_name(command), hdu,id[0], id[1], id[2], id[3], eot, gpl, dtl,
	drv, fdc[drv].track, fdc[drv].cur_track, side,
	position, disk[drv]->get_track_size());
	
	for(int i = 0; i < disk[drv]->sector_num.sd; i++) {
		uint8_t c, h, r, n;
		int length;
		if(disk[drv]->get_sector_info(-1, -1, i, &c, &h, &r, &n, &length)) {
			my_tcscat_s(buffer, buffer_len,
			create_string(_T("\nSECTOR %2d: C=%02X H=%02X R=%02X N=%02X SIZE=%4d AM1=%5d DATA=%5d"), i + 1, c, h, r, n, length, disk[drv]->am1_position[i], disk[drv]->data_position[i]));
			if(position >= disk[drv]->am1_position[i] && position < disk[drv]->data_position[i] + length) {
				my_tcscat_s(buffer, buffer_len, _T(" <==="));
			}
		}
	}
	return true;
}
#endif

#define STATE_VERSION	3

bool UPD765A::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < array_length(fdc); i++) {
		state_fio->StateValue(fdc[i].track);
		state_fio->StateValue(fdc[i].cur_track);
		state_fio->StateValue(fdc[i].result);
		state_fio->StateValue(fdc[i].access);
		state_fio->StateValue(fdc[i].head_load);
		state_fio->StateValue(fdc[i].cur_position);
		state_fio->StateValue(fdc[i].next_trans_position);
		state_fio->StateValue(fdc[i].prev_clock);
	}
	for(int i = 0; i < array_length(disk); i++) {
		if(!disk[i]->process_state(state_fio, loading)) {
			return false;
		}
	}
	state_fio->StateValue(hdu);
	state_fio->StateValue(hdue);
	state_fio->StateArray(id, sizeof(id), 1);
	state_fio->StateValue(eot);
	state_fio->StateValue(gpl);
	state_fio->StateValue(dtl);
	state_fio->StateValue(phase);
	state_fio->StateValue(prevphase);
	state_fio->StateValue(status);
	state_fio->StateValue(seekstat);
	state_fio->StateValue(command);
	state_fio->StateValue(result);
	state_fio->StateValue(step_rate_time);
	state_fio->StateValue(head_unload_time);
	state_fio->StateValue(no_dma_mode);
	state_fio->StateValue(motor_on);
#ifdef UPD765A_DMA_MODE
	state_fio->StateValue(dma_data_lost);
#endif
	state_fio->StateValue(irq_masked);
	state_fio->StateValue(drq_masked);
	if(loading) {
		bufptr = buffer + state_fio->FgetInt32_LE();
	} else {
		state_fio->FputInt32_LE((int)(bufptr - buffer));
	}
	state_fio->StateArray(buffer, sizeof(buffer), 1);
	state_fio->StateValue(count);
	state_fio->StateValue(event_phase);
	state_fio->StateValue(phase_id);
	state_fio->StateValue(drq_id);
	state_fio->StateValue(lost_id);
	state_fio->StateValue(result7_id);
	state_fio->StateArray(seek_step_id, sizeof(seek_step_id), 1);
	state_fio->StateArray(seek_end_id, sizeof(seek_end_id), 1);
	state_fio->StateArray(head_unload_id, sizeof(head_unload_id), 1);
	state_fio->StateValue(force_ready);
	state_fio->StateValue(reset_signal);
	state_fio->StateValue(prev_index);
	state_fio->StateValue(prev_drq_clock);
	return true;
}

