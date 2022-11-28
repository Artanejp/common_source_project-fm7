/*
 * FM-7 -> FM-8 Main I/O [fm8_mainio.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jan 03, 2015 : Initial
 *   Jan 25, 2018 : Move some routines from fm7_mainio.h
 *
 */

#include "fm7.h"
#include "fm8_mainio.h"

#include "../mc6809.h"
#include "../z80.h"

#include "../datarec.h"
#include "../i8251.h"
#if defined(HAS_DMA)
#include "hd6844.h"
#endif
#include "../ym2203.h"
#include "../ay_3_891x.h"
#include "../pcm1bit.h"

#include "./fm7_mainmem.h"
#include "bubblecasette.h"

FM8_MAINIO::FM8_MAINIO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : FM7_MAINIO(parent_vm, parent_emu)
{
    psg = NULL;
	bubble_casette[0] = NULL;
	bubble_casette[1] = NULL;

	// FD15/ FD46 / FD51
	connect_psg = false;
	{
		opn_address[0] = 0x00;
		opn_data[0] = 0x00;
		opn_cmdreg[0] = 0;
	}
#if defined(HAS_DMA)
	dmac = NULL;
#endif	
	set_device_name(_T("FM-8 MAIN I/O"));
	
}

FM8_MAINIO::~FM8_MAINIO()
{
}


void FM8_MAINIO::initialize()
{
	FM7_MAINIO::initialize();
}

void FM8_MAINIO::reset()
{
	FM7_MAINIO::reset();
	cancel_event(this, event_timerirq);
}

uint8_t FM8_MAINIO::get_port_fd00(void)
{
	uint8_t ret = 0xfe;
	if(clock_fast) ret |= 0x01; //0b00000001;
	return ret;
}

void FM8_MAINIO::set_port_fd02(uint8_t val)
{
	return;
}

uint8_t FM8_MAINIO::get_irqstat_fd03(void)
{
	return 0xff;
}

uint8_t FM8_MAINIO::get_extirq_fd17(void)
{
	uint8_t val = 0xff;
	return val;
}

void FM8_MAINIO::set_ext_fd17(uint8_t data)
{
}

void FM8_MAINIO::set_irq_syndet(bool flag)
{
	bool backup = intstat_syndet;
	irqreq_syndet = flag;
	intstat_syndet = flag;
	if(backup != intstat_syndet) do_irq();
}

void FM8_MAINIO::set_irq_rxrdy(bool flag)
{
	bool backup = intstat_rxrdy;
	irqreq_rxrdy = flag;
	intstat_rxrdy = flag;
	if(backup != intstat_rxrdy) do_irq();
}

void FM8_MAINIO::set_irq_txrdy(bool flag)
{
	bool backup = intstat_txrdy;
	irqreq_txrdy = flag;
	intstat_txrdy = flag;
	if(backup != intstat_txrdy) do_irq();
}


void FM8_MAINIO::set_irq_timer(bool flag)
{
}

void FM8_MAINIO::set_irq_printer(bool flag)
{
}

void FM8_MAINIO::set_irq_mfd(bool flag)
{
	//bool backup = irqstat_fdc;
	if(!connect_fdc) return;
	if(flag) {
		irqreg_fdc |= 0x40; //0b01000000;
	} else {
		irqreg_fdc &= 0xbf; //0b10111111;
	}
	// With FM8, $FD1F is alive and not do_irq(), Thanks to Anna_Wu.
}

void FM8_MAINIO::do_irq(void)
{
	bool intstat;
	uint32_t nval;
   	intstat = intstat_txrdy | intstat_rxrdy | intstat_syndet;
	nval = (intstat) ? 0xffffffff : 0;
	write_signals(&irq_bus, nval);
}

void FM8_MAINIO::write_fd0f(void)
{
	if((config.dipswitch & FM7_DIPSW_FM8_PROTECT_FD0F) != 0) {
		return;
	}
	bootmode = 1; // DOS : Where BUBBLE?
	if(config.boot_mode >= 4) {
		bootmode = 5;
	}
	mainmem->write_signal(FM7_MAINIO_BOOTMODE, bootmode, 0xffffffff);
	mainmem->write_signal(FM7_MAINIO_IS_BASICROM, 0, 0xffffffff);
	mainmem->write_signal(FM7_MAINIO_PUSH_FD0F, 0, 0xffffffff);
}

uint8_t FM8_MAINIO::read_fd0f(void)
{
	if((config.dipswitch & FM7_DIPSW_FM8_PROTECT_FD0F) != 0) {
		return 0xff;
	}
	bootmode = 0; // BASIC
	if(config.boot_mode >= 4) {
		bootmode = 4;
	}
	mainmem->write_signal(FM7_MAINIO_BOOTMODE, bootmode, 0xffffffff);
	mainmem->write_signal(FM7_MAINIO_IS_BASICROM, 0xffffffff, 0xffffffff);
	mainmem->write_signal(FM7_MAINIO_PUSH_FD0F, 0xffffffff, 0xffffffff);
	return 0xff;
}

void FM8_MAINIO::reset_sound(void)
{
	if(psg != NULL) {
		psg->reset();
	}
	connect_psg = false;
	if(config.sound_type == 0) {
		connect_psg = false;
	} else {
		connect_psg = true;
	}
# if defined(USE_AY_3_8910_AS_PSG)
	psg->write_signal(SIG_AY_3_891X_MUTE, !connect_psg ? 0xffffffff : 0x00000000, 0xffffffff);
# else	
	psg->write_signal(SIG_YM2203_MUTE, !connect_psg ? 0xffffffff : 0x00000000, 0xffffffff);
# endif
	pcm1bit->write_signal(SIG_PCM1BIT_MUTE, 0x01, 0x01);
	pcm1bit->write_signal(SIG_PCM1BIT_ON, 0x00, 0x01);

}

void FM8_MAINIO::set_psg(uint8_t val)
{
	if(connect_psg) return set_opn(0, val);
}

uint8_t FM8_MAINIO::get_psg(void)
{
	if(connect_psg) return get_opn(0);
	return 0xff;
}
/*
 * $fd0d : After 77AV, this is OPN.
 */
void FM8_MAINIO::set_psg_cmd(uint8_t cmd)
{
	cmd = cmd & 0x03;
	if(connect_psg) set_opn_cmd(0, cmd);
	return;
}
// OPN
// Write to FD16, same as 
void FM8_MAINIO::write_opn_reg(int index, uint32_t addr, uint32_t data)
{
	if(connect_psg) {
		opn_regs[0][addr] = data;
		psg->write_io8(0, addr & 0x0f);
		psg->write_io8(1, data);
		return;
	}
}

void FM8_MAINIO::set_opn(int index, uint8_t val)
{
	if(!connect_psg) {
		return;
	}
	if(index != 0) return;
	if(psg == NULL) return;
	opn_data[index] = val;
	switch(opn_cmdreg[index]){
		case 0: // High inpedance
		case 1: // Read Data
			break;
		case 2: // Write Data
			write_opn_reg(index, opn_address[index], opn_data[index]);
			break;
		case 3: // Register address
			if(index == 3) {
				opn_address[index] = val & 0x0f;
			} else {
				opn_address[index] = val;
			}
			break;
		default:
			break;
	}
}

uint8_t FM8_MAINIO::get_opn(int index)
{
	uint8_t val = 0xff;
	if(!connect_psg) return val;
	if(index != 0) return val;
	if(psg == NULL) return val;
	switch(opn_cmdreg[index]) {
		case 0:
		case 1:
		case 2:
		case 3:
			val = opn_data[index];
			break;
		default:
	 		break;
		}
		return val;
}
  /*
   * $fd16?
   */
void FM8_MAINIO::set_opn_cmd(int index, uint8_t cmd)
{
	if(!connect_psg) return;
	if(index != 0) return;
	if(psg == NULL) return;
	uint32_t mask[16] = { // Parameter is related by XM7. Thanks Ryu.
		0xff, 0x0f, 0xff, 0x0f,
		0xff, 0x0f, 0x1f, 0xff,
		0x1f, 0x1f, 0x1f, 0xff,
		0xff, 0x0f, 0xff, 0xff
	};
	opn_cmdreg[index] = cmd & 0x0f;
	uint8_t val = opn_data[index];
        switch(opn_cmdreg[index]) {
		case 0:
			break;
		case 1:
			if(index == 0) {
				psg->write_io8(0, opn_address[index]);
				opn_data[index] = psg->read_io8(1);
			}
			if(opn_address[index] <= 0x0f) {
				opn_data[index] &= mask[opn_address[index]];
			}
			break;
		case 2:
			write_opn_reg(index, opn_address[index], opn_data[index]);
	 		break;
		case 3: // Register address
			if(index == 3) {
				opn_address[index] = val & 0x0f;
			} else {
				opn_address[index] = val;
			}

			break;
	 	default:
	   		break;
	}
	return;
}
uint8_t FM8_MAINIO::get_extirq_whg(void)
{
	uint8_t val = 0xff;
	return val;
}
uint8_t FM8_MAINIO::get_extirq_thg(void)
{
	uint8_t val = 0xff;
	return val;
}
void FM8_MAINIO::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool val_b = ((data & mask) != 0);
	switch(id) {
		case FM7_MAINIO_OPN_IRQ:
			return;
			break;
		case FM7_MAINIO_WHG_IRQ:
			return;
			break;
		case FM7_MAINIO_THG_IRQ:
			return;
			break;
		case FM7_MAINIO_FDC_IRQ:
			set_irq_mfd(val_b);
			return;
			break;
		default:
			break;
	}
	FM7_MAINIO::write_signal(id, data, mask);
}

uint32_t FM8_MAINIO::read_data8(uint32_t addr)
{
	uint32_t retval = 0xff;
	switch(addr) {
	case 0x00:
		retval = (uint32_t)get_port_fd00();
		return retval;
		break;
	case 0x01: // FD01
		retval = 0xff;
		return retval;
		break;
	case 0x03: // FD03
		retval = 0xff;
		return retval;
		break;
	case 0x0e: // PSG DATA
		retval = (uint32_t) get_psg();
		//printf("PSG DATA READ val=%02x\n", retval);
		return retval;
		break;
	case 0x0f: // FD0F
		read_fd0f();
		retval = 0xff;
		break;
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
		retval = bubble_casette[0]->read_data8(addr);
		return retval;
		break;
	case 0x37: // Multi page
		retval = 0xff;
		return retval;
		break;
	default:
		break;
	}
	if((addr < 0x40) && (addr >= 0x38)) return 0xff; // Palette
	if((addr < 0x54) && (addr >= 0x45)) return 0xff; // WHG, THG
	if((addr >= 0x40) && (addr < 0x100)) return 0xff; // Another devices.
		
	return FM7_MAINIO::read_data8(addr);
}

void FM8_MAINIO::write_data8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x02:
		return set_port_fd02(data);
		break;
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
		bubble_casette[0]->write_data8(addr, data);
		return;
		break;
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
		return;
		break;
	case 0x37: // Multi page
		return;
		break;
	default:
		break;
	}
	if((addr < 0x40) && (addr >= 0x38)) return; // palette
	if((addr < 0x54) && (addr >= 0x45)) return; // WHG, THG
	if((addr < 0x100) && (addr >= 0x40)) return;
	FM7_MAINIO::write_data8(addr, data);
}

void FM8_MAINIO::event_callback(int event_id, int err)
{
	if(event_id == EVENT_TIMERIRQ_ON) {
		return;
	}
	FM7_MAINIO::event_callback(event_id, err);
}

void FM8_MAINIO::update_config()
{
	FM7_MAINIO::update_config();
	// BASIC
	if(config.boot_mode == 0) {
		mainmem->write_signal(FM7_MAINIO_IS_BASICROM, 0xffffffff, 0xffffffff);
	} else {
		mainmem->write_signal(FM7_MAINIO_IS_BASICROM, 0, 0xffffffff);
	}
	mainmem->write_signal(FM7_MAINIO_PUSH_FD0F, ((config.boot_mode & 3) == 0) ? 1 : 0, 0x01);
	mainmem->write_signal(FM7_MAINIO_BOOTMODE, bootmode, 0xffffffff);
}

bool FM8_MAINIO::decl_state(FILEIO *state_fio, bool loading)
{
	if(!FM7_MAINIO::decl_state(state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(connect_psg);

	return true;
}

void FM8_MAINIO::save_state(FILEIO *state_fio)
{
	decl_state(state_fio, false);
}

bool FM8_MAINIO::load_state(FILEIO *state_fio)
{
	bool mb = decl_state(state_fio, true);
	return mb;
}
