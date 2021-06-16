/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns VRAM ]
	History: 2017.01.16 Initial.
*/


#include "common.h"
#include "./towns_common.h"
#include "./crtc.h"
#include "./vram.h"
#include "./planevram.h"

#define _CLEAR_COLOR RGBA_COLOR(0,0,0,0)


namespace FMTOWNS {

void TOWNS_VRAM::initialize()
{
	memset(vram, 0x00, sizeof(vram));
}

void TOWNS_VRAM::reset()
{
	for(int i = 0; i < (sizeof(dirty_flag) / sizeof(bool)); i++) {
		dirty_flag[i] = true;
	}
	packed_pixel_mask_reg.d = 0xffffffff;
	vram_access_reg_addr = 0;
	sprite_busy = false;
	sprite_disp_page = false;
	
	layer_display_flags[0] = layer_display_flags[1] = 0;
}
	
void TOWNS_VRAM::make_dirty_vram(uint32_t addr, int bytes)
{
	uint32_t amask = 0x7ffff;
	uint32_t naddr1 = (addr & amask) >> 3;
	switch(bytes) {
	case 1:
		dirty_flag[naddr1] = true;
		break;
	case 2:
	case 3:
	case 4:
		{
			uint32_t naddr2 = ((addr + bytes - 1) & amask) >> 3;
			dirty_flag[naddr1] = true;
			dirty_flag[naddr2] = true;
		}
		break;
	}
}
	
void TOWNS_VRAM::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	__DECL_ALIGNED(8) uint8_t mask[4];
	packed_pixel_mask_reg.write_4bytes_le_to(mask);
	uint8_t rmask = mask[addr & 3];
	uint32_t naddr = addr & 0x7ffff;
	uint8_t nd = vram[naddr];
	data = data & rmask;
	nd = nd & ~rmask;
	vram[naddr] = nd | data;
}

void TOWNS_VRAM::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	uint32_t naddr1 = addr & 0x7ffff;
	uint32_t naddr2 = (addr + 1) & 0x7ffff;;
	pair16_t nd, md;
	pair16_t xmask;
	switch(naddr1 & 3) {
	case 0:
		xmask.w =  packed_pixel_mask_reg.w.l;
		break;
	case 1:
		xmask.b.l =  packed_pixel_mask_reg.b.h;
		xmask.b.h =  packed_pixel_mask_reg.b.h2;
		break;
	case 2:
		xmask.w =  packed_pixel_mask_reg.w.h;
		break;
	case 3:
		xmask.b.l =  packed_pixel_mask_reg.b.h3;
		xmask.b.h =  packed_pixel_mask_reg.b.l;
//		if((addr == 0x8003ffff) || (addr == 0x8007ffff)) { // Wrap1
//			naddr2 = (addr == 0x8003ffff) ? 0x00000 : 0x40000;
//		} else if(addr == 0x8017ffff) { // wrap 2
//			naddr2 = 0x00000;
//		}
		break;
	}

	if(naddr1 == 0x7ffff) { // Wrap
		nd.w = (uint16_t)data;
		md.b.l = vram[naddr1];
		nd.w = nd.w & xmask.w;
		md.w = md.w & ~(xmask.w);
		md.w = md.w | nd.w;
		vram[naddr1] = md.b.l;
	} else {
		md.read_2bytes_le_from(&(vram[naddr1]));
		
		nd.w = (uint16_t)data;
		
		nd.w = nd.w & xmask.w;
		md.w = md.w & ~(xmask.w);
		md.w = md.w | nd.w;
		
		md.write_2bytes_le_to(&(vram[naddr1]));
	}

	return;
}
	
void TOWNS_VRAM::write_memory_mapped_io32(uint32_t addr, uint32_t data)
{

	__DECL_ALIGNED(8) uint8_t mask[4];
	uint32_t naddr = addr & 0x7ffff;
	pair32_t nd, md;
	pair32_t xmask;
	
	nd.d = data;
	switch(naddr & 3) {
	case 0:
		xmask.d =  packed_pixel_mask_reg.d;
		break;
	case 1:
		xmask.b.l  =  packed_pixel_mask_reg.b.h;
		xmask.b.h  =  packed_pixel_mask_reg.b.h2;
		xmask.b.h2 =  packed_pixel_mask_reg.b.h3;
		xmask.b.h3 =  packed_pixel_mask_reg.b.l;
		break;
	case 2:
		xmask.b.l  =  packed_pixel_mask_reg.b.h2;
		xmask.b.h  =  packed_pixel_mask_reg.b.h3;
		xmask.b.h2 =  packed_pixel_mask_reg.b.l;
		xmask.b.h3 =  packed_pixel_mask_reg.b.h;
		break;
	case 3:
		xmask.b.l  =  packed_pixel_mask_reg.b.h3;
		xmask.b.h  =  packed_pixel_mask_reg.b.l;
		xmask.b.h2 =  packed_pixel_mask_reg.b.h;
		xmask.b.h3 =  packed_pixel_mask_reg.b.h2;
		break;
	}
	
	if((addr & 3) != 0) {
		if(naddr <= 0x7fffc) {
			md.read_4bytes_le_from(&(vram[naddr]));
			nd.d = nd.d & xmask.d;
			md.d = md.d & ~(xmask.d);
			md.d = md.d | nd.d;
			md.write_4bytes_le_to(&(vram[naddr]));
		} else {
			switch(naddr & 3) {
			case 1: // 7fffd
				md.b.l  = vram[naddr];
				md.b.h  = vram[naddr + 1];
				md.b.h2 = vram[naddr + 2];
				break;
			case 2: // 7fffe
				md.b.l  = vram[naddr];
				md.b.h  = vram[naddr + 1];
				break;
			case 3: // 7fffe
				md.b.l  = vram[naddr];
				break;
			}
			nd.d = nd.d & xmask.d;
			md.d = md.d & ~(xmask.d);
			md.d = md.d | nd.d;
			switch(naddr & 3) {
			case 1: // 7fffd
				vram[naddr] = md.b.l;
				vram[naddr + 1] = md.b.h;
				vram[naddr + 2] = md.b.h2;
				break;
			case 2: // 7fffe
				vram[naddr] = md.b.l;
				vram[naddr + 1] = md.b.h;
				break;
			case 3: // 7fffe
				vram[naddr] = md.b.l;
				break;
			}	
		}
//		uint32_t xaddr = addr & 0x0017ffff;
//		uint32_t naddr2 = naddr + 1;
//		uint32_t naddr3 = naddr + 2;
//		uint32_t naddr4 = naddr + 3;
//		uint32_t offmask;
//		if((addr & 0x3ffff) < 0x3fffd) {
//			// Maybe not wrapped.
//			md.read_4bytes_le_from(&(vram[naddr]));
//			
//			nd.d = nd.d & xmask.d;
//			md.d = md.d & ~(xmask.d);
//			md.d = md.d | nd.d;
//			
//			md.write_4bytes_le_to(&(vram[naddr]));
//		} else {
			// Maybe not wrapped.
//			switch(xaddr) {
//			case 0x17fffd:
//			case 0x17fffe:
//			case 0x17ffff:
//				naddr2 = naddr2 & 0x7ffff;
//				naddr3 = naddr3 & 0x7ffff;
//				naddr4 = naddr4 & 0x7ffff;
///				
//				md.b.l   = vram[naddr];
//				md.b.h   = vram[naddr2];
//				md.b.h2  = vram[naddr3];
//				md.b.h3  = vram[naddr4];
				
//				nd.d = nd.d & xmask.d;
//				md.d = md.d & ~(xmask.d);
//				md.d = md.d | nd.d;
			
//				vram[naddr] = md.b.l;
//				vram[naddr2] = md.b.h;
//				vram[naddr3] = md.b.h2;
//				vram[naddr4] = md.b.h3;
//				break;
//			case 0x03fffd:
//			case 0x03fffe:
//			case 0x03ffff:
//			case 0x07fffd:
//			case 0x07fffe:
//			case 0x07ffff:
//				offmask = ((addr & 0x00040000) != 0) ? 0x40000 : 0x00000;
//				naddr2 = (naddr2 & 0x3ffff) + offmask;
//				naddr3 = (naddr3 & 0x3ffff) + offmask;
//				naddr4 = (naddr4 & 0x3ffff) + offmask;
//			
//				md.b.l   = vram[naddr];
//				md.b.h   = vram[naddr2];
//				md.b.h2  = vram[naddr3];
//				md.b.h3  = vram[naddr4];

//				nd.d = nd.d & xmask.d;
//				md.d = md.d & ~(xmask.d);
//				md.d = md.d | nd.d;
			
//				vram[naddr]  = md.b.l;
//				vram[naddr2] = md.b.h;
//				vram[naddr3] = md.b.h2;
//				vram[naddr4] = md.b.h3;
//				break;
//			default:
//				md.read_4bytes_le_from(&(vram[naddr]));
//			
//				nd.d = nd.d & xmask.d;
//				md.d = md.d & ~(xmask.d);
//				md.d = md.d | nd.d;
//			
//				md.write_4bytes_le_to(&(vram[naddr]));
//				break;
//			}
//		}
	} else {
		// Aligned
#ifdef __LITTLE_ENDIAN__
		md.d = *((uint32_t*)(&(vram[naddr])));
#else			
		md.read_4bytes_le_from(&(vram[naddr]));
#endif		
		nd.d = nd.d & xmask.d;
		md.d = md.d & (~(xmask.d));
		md.d = md.d | nd.d;
		
#ifdef __LITTLE_ENDIAN__
		*((uint32_t*)(&(vram[naddr]))) = md.d;
#else
		md.write_4bytes_le_to(&(vram[naddr]));
#endif
	}
	return;
}

uint32_t TOWNS_VRAM::read_memory_mapped_io8(uint32_t addr)
{
	return vram[addr & 0x7ffff];
}

uint32_t TOWNS_VRAM::read_memory_mapped_io16(uint32_t addr)
{
	pair16_t a;

	uint32_t naddr = addr & 0x7ffff;
	if((addr & 1) == 0) { // Aligned
		a.read_2bytes_le_from(&(vram[naddr]));
	} else {
		/*
		switch(addr) {
		case 0x8003ffff:
			a.b.l = vram[0x3ffff];
			a.b.h = vram[0x00000];
			break;
		case 0x8007ffff:
			a.b.l = vram[0x7ffff];
			a.b.h = vram[0x40000];
			break;
		case 0x8017ffff:
			a.b.l = vram[0x7ffff];
			a.b.h = vram[0x00000];
			break;
		default:
			a.read_2bytes_le_from(&(vram[naddr]));
			break;
		}
		*/
		if(naddr == 0x7ffff) {
			a.b.l = vram[naddr];
			a.b.h = 0xff;
		} else {
			a.b.l = vram[naddr];
			a.b.h = vram[naddr + 1];
		}
	}
	return (uint32_t)(a.w);
}

uint32_t TOWNS_VRAM::read_memory_mapped_io32(uint32_t addr)
{
	pair32_t a;

	uint32_t naddr = addr & 0x7ffff;
	if((addr & 3) == 0) { // Aligned
#ifdef __LITTLE_ENDIAN__
		a.d = *((uint32_t*)(&(vram[naddr])));
#else
		a.read_4bytes_le_from(&(vram[naddr]));
#endif
	} else { // Unaligned
		if((addr & 0x7ffff) < 0x7fffd) {
			// Maybe not wrapped.
			a.read_4bytes_le_from(&(vram[naddr]));
		} else	{
			a.d = 0xffffffff;
			switch(naddr & 3) {
			case 1:
				a.b.l  = vram[naddr];
				a.b.h  = vram[naddr + 1];
				a.b.h2 = vram[naddr + 2];
				break;
			case 2:
				a.b.l  = vram[naddr];
				a.b.h  = vram[naddr + 1];
				break;
			case 3:
				a.b.l  = vram[naddr];
				break;
				
			}
			/*
			uint32_t xaddr = addr & 0x0017ffff;
			uint32_t naddr2 = naddr + 1;
			uint32_t naddr3 = naddr + 2;
			uint32_t naddr4 = naddr + 3;
			uint32_t offmask;
			switch(xaddr) {
			case 0x17fffd:
			case 0x17fffe:
			case 0x17ffff:
				naddr2 = naddr2 & 0x7ffff;
				naddr3 = naddr3 & 0x7ffff;
				naddr4 = naddr4 & 0x7ffff;
				
				a.b.l   = vram[naddr];
				a.b.h   = vram[naddr2];
				a.b.h2  = vram[naddr3];
				a.b.h3  = vram[naddr4];
				break;
			case 0x03fffd:
			case 0x03fffe:
			case 0x03ffff:
			case 0x07fffd:
			case 0x07fffe:
			case 0x07ffff:
				offmask = ((addr & 0x00040000) != 0) ? 0x40000 : 0x00000;
				naddr2 = (naddr2 & 0x3ffff) + offmask;
				naddr3 = (naddr3 & 0x3ffff) + offmask;
				naddr4 = (naddr4 & 0x3ffff) + offmask;
			
				a.b.l   = vram[naddr];
				a.b.h   = vram[naddr2];
				a.b.h2  = vram[naddr3];
				a.b.h3  = vram[naddr4];
				break;
			default:
			// Maybe not wrapped.
				a.read_4bytes_le_from(&(vram[naddr]));
				break;
			}
			*/
		}
	}
	return a.d;
}

void TOWNS_VRAM::write_signal(int id, uint32_t data, uint32_t mask)
{
	// ToDo
}
// Renderers

void TOWNS_VRAM::write_io8(uint32_t address,  uint32_t data)
{
	switch(address & 0xffff) {
	case 0x0458:
		vram_access_reg_addr = data & 3;
//		out_debug_log(_T("VRAM ACCESS(0458h)=%02X"), data);
		break;
	case 0x045a:
		switch(vram_access_reg_addr) {
		case 0:
			packed_pixel_mask_reg.b.l = data;
			break;
		case 1:
			packed_pixel_mask_reg.b.h2 = data;
			break;
		}			
//		out_debug_log(_T("VRAM MASK(045Ah)=%08X"), packed_pixel_mask_reg.d);
		break;
	case 0x045b:
		switch(vram_access_reg_addr) {
		case 0:
			packed_pixel_mask_reg.b.h = data;
			break;
		case 1:
			packed_pixel_mask_reg.b.h3 = data;
			break;
		}			
//		out_debug_log(_T("VRAM MASK(045Bh)=%08X"), packed_pixel_mask_reg.d);
		break;
	}
}

void TOWNS_VRAM::write_io16(uint32_t address,  uint32_t data)
{
	pair32_t d;
	d.d = data;
	switch(address & 0xffff) {
	case 0x0458:
		vram_access_reg_addr = data & 3;
//		out_debug_log(_T("VRAM ACCESS(0458h)=%02X"), data);
		break;
	case 0x045a:
		switch(vram_access_reg_addr) {
		case 0:
			packed_pixel_mask_reg.w.l = d.w.l;
			break;
		case 1:
			packed_pixel_mask_reg.w.h = d.w.l;
			break;
		}			
//		out_debug_log(_T("VRAM MASK(045Ah)=%08X"), packed_pixel_mask_reg.d);
		break;
	}
}

uint32_t TOWNS_VRAM::read_io8(uint32_t address)
{
	switch(address & 0xffff) {
	case 0x0458:
		return vram_access_reg_addr;
		break;
	case 0x045a:
		switch(vram_access_reg_addr) {
		case 0:
			return packed_pixel_mask_reg.b.l;
			break;
		case 1:
			return packed_pixel_mask_reg.b.h2;
			break;
		}			
		break;
	case 0x045b:
		switch(vram_access_reg_addr) {
		case 0:
			return packed_pixel_mask_reg.b.h;
			break;
		case 1:
			return packed_pixel_mask_reg.b.h3;
			break;
		}			
		break;
	}
	return 0xff;
}

uint32_t TOWNS_VRAM::read_io16(uint32_t address)
{
	switch(address & 0xffff) {
	case 0x0458:
		return vram_access_reg_addr;
		break;
	case 0x045a:
		switch(vram_access_reg_addr) {
		case 0:
			return packed_pixel_mask_reg.w.l;
			break;
		case 1:
			return packed_pixel_mask_reg.w.h;
			break;
		}			
		break;
	}
	return 0xffff;
}

#define STATE_VERSION	1

bool TOWNS_VRAM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}

	state_fio->StateValue(access_page1);
	state_fio->StateArray(dirty_flag, sizeof(dirty_flag), 1);
	state_fio->StateArray(layer_display_flags, sizeof(layer_display_flags), 1);

	state_fio->StateValue(sprite_busy);
	state_fio->StateValue(sprite_disp_page);

	state_fio->StateValue(vram_access_reg_addr);
	state_fio->StateValue(packed_pixel_mask_reg);

	state_fio->StateArray(vram, sizeof(vram), 1);
	
	return true;
}


#undef _CLEAR_COLOR
}
