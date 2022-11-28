/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.03.24 -

	[ multifont rom card ]
*/

#include "mfont.h"
#include "../i8259.h"
#include "../../fifo.h"

#define BIT_IBF	0x80
#define BIT_ERR	2
#define BIT_OBF	1

void MFONT::initialize()
{
	memset(mfont, 0xff, sizeof(mfont));
	
	// load multifont rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("MFONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(mfont, sizeof(mfont), 1);
		fio->Fclose();
	}
	delete fio;
	
	cmd = new FIFO(4);
	res = new FIFO(38);
	
	status = 0;
}

void MFONT::release()
{
	cmd->release();
	delete cmd;
	res->release();
	delete res;
}

void MFONT::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xfc:
		cmd->write(data);
		if(cmd->count() == 3) {
			int mode = cmd->read();
			int code = cmd->read();
			code |= cmd->read() << 8;
			
			if(0x200 <= code && code < 0xc00) {
				int ofs = (code - 0x200) * 36;
				res->clear();
				res->write(0x40);
				for(int i = 0; i < 36; i++) {
					res->write(mfont[ofs + i]);
				}
				status = BIT_IBF | BIT_OBF;
				d_pic->write_signal(SIG_I8259_IR7 | SIG_I8259_CHIP1, 1, 1);
			} else {
				// error
				status = BIT_ERR;
			}
		}
		break;
	case 0xfd:
		// set irq
		d_pic->write_signal(SIG_I8259_IR7 | SIG_I8259_CHIP1, 1, 1);
		break;
	}
}

uint32_t MFONT::read_io8(uint32_t addr)
{
	uint32_t val;
	
	switch(addr & 0xff) {
	case 0xfc:
		val = res->read();
		if(res->empty()) {
			status = 0;
		}
		return val;
	case 0xfd:
		// reset irq
		d_pic->write_signal(SIG_I8259_IR7 | SIG_I8259_CHIP1, 0, 1);
		return status;
	}
	return 0xff;
}

#define STATE_VERSION	1

bool MFONT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(status);
	if(!cmd->process_state((void *)state_fio, loading)) {
		return false;
	}
	if(!res->process_state((void *)state_fio, loading)) {
		return false;
	}
	return true;
}

