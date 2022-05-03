
#include "./planevram.h"
#include "./vram.h"

#include "./crtc.h"
#include "./sprite.h"

#include "../../fileio.h"

namespace FMTOWNS {

void PLANEVRAM::initialize()
{
	DEVICE::initialize();
}

void PLANEVRAM::reset()
{
	mix_reg = 0xff;
	r50_readplane = 0x0; // OK?
	r50_ramsel = 0x0; // OK?
	r50_gvramsel = 0x00000000; // OK?
}

void PLANEVRAM::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0xff80:
		mix_reg = data & 0x28;
		break;
	case 0xff81:
//		out_debug_log(_T("0xCFF81=%02X"), data & 0xff);
		r50_readplane = (data & 0xc0) >> 6;
		r50_ramsel = data & 0x0f;
		break;
	case 0xff82:
		__LIKELY_IF(d_crtc != NULL) {
			d_crtc->write_signal(SIG_TOWNS_CRTC_MMIO_CFF82H, data, 0xffffffff);
//			out_debug_log(_T("WRITE CFF82h <- %02X"), data);
		}
		break;
	case 0xff83:
//		out_debug_log(_T("0xCFF83=%02X"), data & 0xff);
		r50_gvramsel = ((data & 0x10) != 0) ? 0x20000 : 0x00000;
		break;
	case 0xff86:
		break;
	case 0xffa0:
		break;
	default:
		__LIKELY_IF(d_sprite != NULL) {
			d_sprite->write_data8(addr & 0x7fff, data);
		}
		break;
	}
}

uint32_t PLANEVRAM::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0xff80:
		return mix_reg;
		break;
	case 0xff81:
		return ((r50_readplane << 6) | r50_ramsel);
		break;
	case 0xff82:
		__LIKELY_IF(d_crtc != NULL) {
			return d_crtc->read_signal(SIG_TOWNS_CRTC_MMIO_CFF82H);
		}
		break;
	case 0xff83:
		return ((r50_gvramsel != 0x00000) ? 0x10 : 0x00);
		break;
	case 0xff84:
		return 0x7f; // Reserve.FIRQ
		break;
	case 0xff86:
		__LIKELY_IF(d_crtc != NULL) {
			return d_crtc->read_signal(SIG_TOWNS_CRTC_MMIO_CFF86H);
		}
		break;
	case 0xffa0:
		{
			uint8_t val;
			val = 0xff;
			val = val & 0x7f;
			return val;
		}
		break;
	default:
		__LIKELY_IF(d_sprite != NULL) {
			return d_sprite->read_data8(addr & 0x7fff);
		}
		break;
	}
	return 0xff;
}

uint32_t PLANEVRAM::read_memory_mapped_io8(uint32_t addr)
{
	// Plane Access
	uint32_t x_addr = r50_gvramsel;
	// ToDo: Writing plane.
	addr = (addr & 0x7fff) << 2;
	__UNLIKELY_IF(d_vram == NULL) return 0xff;
	
	uint8_t *p = d_vram->get_vram_address(x_addr + addr);
	__UNLIKELY_IF(p == NULL) return 0xff;
//	p = &(p[x_addr + addr]); 
	
	// 8bit -> 32bit
	uint8_t val = 0;
	const uint8_t lmask = 1 << (r50_readplane & 3);
	const uint8_t hmask = lmask << 4;
	
	d_vram->lock();
	__DECL_ALIGNED(8) uint8_t extra_p[8];
	__DECL_ALIGNED(8) uint8_t extra_mask[8];
	const uint8_t extra_value[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	__DECL_ALIGNED(8) uint8_t extra_bool[8];
	__DECL_ALIGNED(8) uint8_t cache[4];
	
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		cache[i] = p[i];
	}
	
__DECL_VECTORIZED_LOOP
	for(int i = 0, j = 0; i < 8; i += 2, j++) {
		extra_p[i    ] = cache[j];
		extra_p[i + 1] = cache[j];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i += 2) {
		extra_mask[i    ] = hmask;
		extra_mask[i + 1] = lmask;
	}
	
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		extra_bool[i] = ((extra_mask[i] & extra_p[i]) != 0) ? 0xff : 0x00;
	}

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		extra_bool[i] &= extra_value[i];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		val |= extra_bool[i];
	}
	d_vram->unlock();
	
	return val;
}


void PLANEVRAM::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	// Plane Access
	uint32_t x_addr = r50_gvramsel;

	// ToDo: Writing plane.
	addr = (addr & 0x7fff) << 2;

	__UNLIKELY_IF(d_vram == NULL) return;
	
	uint8_t *p = d_vram->get_vram_address(x_addr + addr);
	__UNLIKELY_IF(p == NULL) return;
	
	// 8bit -> 32bit
	uint32_t tmp = 0;
	uint32_t tmp_d = data;
	uint8_t ntmp = r50_ramsel/* & 0x0f*/;
#ifdef __LITTLE_ENDIAN__
//	uint32_t tmp_m1 = 0xf0000000/* & write_plane_mask*/;
//	uint32_t tmp_m2 = 0x0f000000/* & write_plane_mask*/;
	const uint32_t tmp_m1 = (((uint32_t)ntmp) << 28);
	const uint32_t tmp_m2 = (((uint32_t)ntmp) << 24);
#else
//	uint32_t tmp_m1 = 0x0000000f/* & write_plane_mask*/;
//	uint32_t tmp_m2 = 0x000000f0/* & write_plane_mask*/;
	const uint32_t tmp_m1 = (((uint32_t)ntmp) << 0);
	const uint32_t tmp_m2 = (((uint32_t)ntmp) << 4);
#endif	
	pair32_t tmp_r1;
	//uint32_t tmp_r2;
	uint32_t mask = 0;

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
#ifdef __LITTLE_ENDIAN__
		tmp = tmp >> 8;
		mask = mask >> 8;
#else
		tmp = tmp << 8;
		mask = mask << 8;
#endif
		tmp = tmp | (((tmp_d & 0x80) != 0) ? tmp_m2 : 0x00);
		tmp = tmp | (((tmp_d & 0x40) != 0) ? tmp_m1 : 0x00);
		mask = mask | (tmp_m1 | tmp_m2);
		tmp_d <<= 2;
	}
	d_vram->lock();

	tmp_r1.read_4bytes_le_from(p);
//	tmp_r2 = tmp_r1.d;
	tmp_r1.d = tmp_r1.d & ~mask;
	tmp_r1.d = tmp | tmp_r1.d;

//	if(tmp_r2 != tmp_r1.d) {
		tmp_r1.write_4bytes_le_to(p);
//		d_vram->make_dirty_vram(x_addr + addr, 4);
//	}
	d_vram->unlock();

}

#define STATE_VERSION	2

bool PLANEVRAM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	
	state_fio->StateValue(mix_reg);
	state_fio->StateValue(r50_readplane);
	state_fio->StateValue(r50_ramsel);
	state_fio->StateValue(r50_gvramsel);
	
	return true;
}

}
