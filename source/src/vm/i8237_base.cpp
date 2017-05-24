/*
	Skelton for retropc emulator

	Origin : MESS
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ i8237 ]
*/

#include "i8237.h"

void I8237_BASE::initialize()
{
	DEVICE::initialize();
}

void I8237_BASE::reset()
{
	low_high = false;
	cmd = req = tc = 0;
	mask = 0xff;
}

void I8237_BASE::write_io8(uint32_t addr, uint32_t data)
{
	// Dummy function
}

uint32_t I8237_BASE::read_io8(uint32_t addr)
{
	int ch = (addr >> 1) & 3;
	uint32_t val = 0xff;
	
	switch(addr & 0x0f) {
	case 0x00: case 0x02: case 0x04: case 0x06:
		if(low_high) {
			val = dma[ch].areg >> 8;
		} else {
			val = dma[ch].areg & 0xff;
		}
		low_high = !low_high;
		return val;
	case 0x01: case 0x03: case 0x05: case 0x07:
		if(low_high) {
			val = dma[ch].creg >> 8;
		} else {
			val = dma[ch].creg & 0xff;
		}
		low_high = !low_high;
		return val;
	case 0x08:
		// status register
		val = (req << 4) | tc;
		tc = 0;
		return val;
	case 0x0d:
		// temporary register
		return tmp & 0xff;
	}
	return 0xff;
}

void I8237_BASE::write_signal(int id, uint32_t data, uint32_t mask)
{
	// Dummy function
}

// note: if SINGLE_MODE_DMA is defined, do_dma() is called in every machine cycle

void I8237_BASE::do_dma()
{
	// Dummy function
}

void I8237_BASE::write_mem(uint32_t addr, uint32_t data)
{
	if(mode_word) {
		d_mem->write_dma_data16(addr << 1, data);
	} else {
		d_mem->write_dma_data8(addr, data);
	}
}

uint32_t I8237_BASE::read_mem(uint32_t addr)
{
	if(mode_word) {
		return d_mem->read_dma_data16(addr << 1);
	} else {
		return d_mem->read_dma_data8(addr);
	}
}

void I8237_BASE::write_io(int ch, uint32_t data)
{
	if(mode_word) {
		dma[ch].dev->write_dma_io16(0, data);
	} else {
		dma[ch].dev->write_dma_io8(0, data);
	}
}

uint32_t I8237_BASE::read_io(int ch)
{
	if(mode_word) {
		return dma[ch].dev->read_dma_io16(0);
	} else {
		return dma[ch].dev->read_dma_io8(0);
	}
}

