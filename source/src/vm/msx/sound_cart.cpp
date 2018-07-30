/*
	Common Source Code Project
	MSX Series (experimental)

	Author : umaiboux
	Date   : 2016.03.xx-

	[ Sound Manager for Cartridge ]
*/

#include "sound_cart.h"

SOUND_CART::SOUND_CART(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	d_chip[SOUND_CHIP_SCC] = new SCC(parent_vm, parent_emu);
	enable_chip[SOUND_CHIP_SCC] = false;
	set_device_name(_T("Sound Cartridge"));
}

void SOUND_CART::disable_all()
{
	int i;
	for(i=0; i<=SOUND_CHIP_MAX; i++) {
		this->enable_c(i, false);
	}
}

void SOUND_CART::enable_c(int chip, bool enable)
{
	if (enable_chip[chip] != enable) {
		d_chip[chip]->reset();
		enable_chip[chip] = enable;
	}
}

void SOUND_CART::write_io8_c(int chip, uint32_t addr, uint32_t data)
{
	d_chip[chip]->write_io8(addr, data);
}

uint32_t SOUND_CART::read_io8_c(int chip, uint32_t addr)
{
	return d_chip[chip]->read_io8(addr);
}

void SOUND_CART::write_data8_c(int chip, uint32_t addr, uint32_t data)
{
	d_chip[chip]->write_data8(addr, data);
}

uint32_t SOUND_CART::read_data8_c(int chip, uint32_t addr)
{
	return d_chip[chip]->read_data8(addr);
}

void SOUND_CART::initialize()
{
	/*int i;
	for(i=0; i<=SOUND_CHIP_MAX; i++) {
		d_chip[i]->initialize();
	}*/
}

void SOUND_CART::release()
{
	/*int i;
	for(i=0; i<=SOUND_CHIP_MAX; i++) {
		d_chip[i]->release();
		enable_chip[i] = false;
	}*/
}

void SOUND_CART::reset()
{
	/*int i;
	for(i=0; i<=SOUND_CHIP_MAX; i++) {
		d_chip[i]->reset();
	}*/
}

void SOUND_CART::mix(int32_t* buffer, int cnt)
{
	int i;
	for(i=0; i<=SOUND_CHIP_MAX; i++) {
		if (enable_chip[i]) {
			d_chip[i]->mix(buffer, cnt);
		}
	}
}

void SOUND_CART::initialize_sound(int rate, int clock, int samples)
{
	((SCC*)d_chip[SOUND_CHIP_SCC])->initialize_sound(rate, clock, samples);
}

void SOUND_CART::set_volume(int ch, int decibel_l, int decibel_r)
{
	int i;
	for(i=0; i<=SOUND_CHIP_MAX; i++) {
		if (enable_chip[i]) {
			d_chip[i]->set_volume(1, decibel_l, decibel_r);
		}
	}
}
