
#ifndef _TOWNS_SPRITE_H_
#define _TOWNS_SPRITE_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_FMTOWNS_SPRITE_RENDER 256
#define SIG_FMTOWNS_SPRITE_UPDATE_FRAMEBUFFER 257
#define SIG_FMTOWNS_RENDER_SPRITE_CHANGE_BANK 258
#define SIG_FMTOWNS_RENDER_SPRITE_ENABLED     259
#define SIG_FMTOWNS_RENDER_SPRITE_RESET       260
#define SIG_FMTOWNS_RENDER_SPRITE_SET_LIMIT   261
#define SIG_FMTOWNS_RENDER_SPRITE_SET_NUM     262
#define SIG_FMTOWNS_RENDER_SPRITE_CLEAR_VRAM  263


enum {
	ROT_FMTOWNS_SPRITE_0 = 0,
	ROT_FMTOWNS_SPRITE_90,
	ROT_FMTOWNS_SPRITE_180,
	ROT_FMTOWNS_SPRITE_270
};


class TOWNS_VRAM;

#define TOWNS_SPRITE_CACHE_NUM 512

class TOWNS_SPRITE : public DEVICE
{
private:
	TOWNS_VRAM *vram_head;

	// REGISTERS
	uint8_t reg_addr;
	uint8_t reg_data[8];
	// #0, #1
	bool reg_spen;
	uint16_t reg_index;

	uint16_t reg_voffset;
	uint16_t reg_hoffset;
	bool disp_page0;
	bool disp_page1;
	
	int32_t splite_limit;

	uint16_t index_ram[4096]; // 1024 * 4
	uint16_t color_ram[4096]; // 16 * 256
	uint8_t pattern_ram[(65536 - (4096 * 2)) * 2];

	bool pattern_cached[((65536 - (4096 * 2)) * 2) / (8 * 16)];
	bool color_cached[256];
protected:

	
public:
	TOWNS_SPRITE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("SPRITE"));
		vram_head = NULL;
		framebuffer = NULL;
		
	}
	~TOWNS_SPRITE() {}
	

	void write_data8(uint32_t addr, uint32_t data);
	void write_data16(uint32_t addr, uint32_t data);
	void write_data32(uint32_t addr, uint32_t data);
	
	uint32_t read_data8(uint32_t addr);
	uint32_t read_data16(uint32_t addr);
	uint32_t read_data32(uint32_t addr);

	void write_signal(int id, uint32_t data, uint32_t mask);
	
	void initialize();

	void set_context_vram(TOWNS_VRAM *p)
	{
		vram_head = p;
	}
	void save_state(FILEIO *fio);
	bool load_stste(FILEIO *fio);
};

inline void TOWNS_SPRITE::render_32768_x1_x1(int num, uint16_t* dst, int rot_type, bool is_mirror, int stride)
{
	uint16_t* qp = (uint16_t*)(sprite_table[num].pixels);
	uint16_t *cache_pixel = &(cache_pixels[last_put_cache_num][0]);
	uint16_t *cache_mask = cache_masks[last_put_cache_num][0];
	int xbegin;
	int xend;
	int xstep;
	int ybegin;
	int yend;
	int ystep;
	int addr_base;
	int addr_inc;
	bool xreverse;
	bool yreverse;
	bool rotate = false;
	switch(rot_type) {
	case ROT_FMTOWNS_SPRITE_0:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = false;
			yreverse = false;
			rotate = false;
			if(is_mirror) {
				xreverse = true;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_90:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = false;
			yreverse = true;
			rotate = true;
			if(is_mirror) {
				yreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_180:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = true;
			yreverse = true;
			rotate = false;
			if(is_mirror) {
				xreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_270:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = true;
			yreverse = false;
			rotate = true;
			if(is_mirror) {
				yreverse = true;
			}
		}			
		break;
	}
	uint16_t pixels[16];
	uint16_t masks[16];
	uint16_t tpixels[16];
	uint16_t* cp;
	uint16_t* cm;
	uint16* dp;
	uint16_t* yaddr;
	uint16_t*p;
	for(y = ybegin; y != yend; y += ystep) {
		cp = &(cache_pixel[y << 5]);
		cm = &(cache_mask[y << 5]);
		dp =  &(dst[stride * y]);
		if(!rotate) {
			if(yreverse) {
				yaddr = &(qp[(y - 15) << 5]); // 1 line is 4 bytes (16pixels)
			} else {
				yaddr = &(qp[y << 5]); // 1 line is 4 bytes (16pixels)
			}
			p = &(yaddr[0]);
			if(xreverse) {
__DECL_VECTORIZED_LOOP
				for(int i = 15; i >= 0; i--) {
					pixels[i] = p[15 - i];
				}
			} else {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i <= 15; i++) {
					pixels[i] = p[i];
				}
			}
		} else {
			// Rotate: Swap x, y
			if(yreverse) {
				yaddr = &(qp[15 - y]);
			} else {
				yaddr = &(qp[y]);
			}			
			if(xreverse) {
__DECL_VECTORIZED_LOOP				
				for(int x = 15; x >= 0; x--) {
					p = &(yaddr[x << 5]);
					pixels[x] = p[15 - y];
				}
			} else {
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x <= 15; x++) {
					p = &(yaddr[x << 5]);
					pixels[x] = p[y];
				}
			}
		}
__DECL_VECTORIZED_LOOP		
		for(int x = 0; x < 16; x++) {
			masks[x] = ((pixels[x] & 0x8000) == 0) ? 0xffff : 0x0000;
			cp[x] = pixels[x];
			cm[x] = masks[x];
			// Draw to buffer
			tpixels[x] = dp[x] & masks[x];
			masks[x] = ~masks[x];
			pixels[x] = pixels[x] & masks[x];
			dp[x] = pixels[x] | tpixels[x];
		}
	}
}

inline void TOWNS_SPRITE::render_32768_x05_x1(int num, uint16_t* dst, int rot_type, bool is_mirror, int stride)
{
	uint16_t* qp = (uint16_t*)(sprite_table[num].pixels);
	uint16_t *cache_pixel = &(cache_pixels[last_put_cache_num][0]);
	uint16_t *cache_mask = cache_masks[last_put_cache_num][0];
	int xbegin;
	int xend;
	int xstep;
	int ybegin;
	int yend;
	int ystep;
	int addr_base;
	int addr_inc;
	bool xreverse;
	bool yreverse;
	bool rotate = false;
	switch(rot_type) {
	case ROT_FMTOWNS_SPRITE_0:
			xbegin = 0;
			xend = 16;
			xstep = 2;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = false;
			yreverse = false;
			rotate = false;
			if(is_mirror) {
				xreverse = true;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_90:
			xbegin = 0;
			xend = 16;
			xstep = 2;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = false;
			yreverse = true;
			rotate = true;
			if(is_mirror) {
				yreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_180:
			xbegin = 0;
			xend = 16;
			xstep = 2;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = true;
			yreverse = true;
			rotate = false;
			if(is_mirror) {
				xreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_270:
			xbegin = 0;
			xend = 16;
			xstep = 2;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = true;
			yreverse = false;
			rotate = true;
			if(is_mirror) {
				yreverse = true;
			}
		}			
		break;
	}
	uint16_t pixels[16];
	uint16_t masks[16];
	uint16_t tpixels[16];
	uint16_t* cp;
	uint16_t* cm;
	uint16* dp;
	uint16_t* yaddr;
	uint16_t*p;
	for(y = ybegin; y != yend; y += ystep) {
		cp = &(cache_pixel[y << 5]);
		cm = &(cache_mask[y << 5]);
		dp =  &(dst[stride * y]);
		if(!rotate) {
			if(yreverse) {
				yaddr = &(qp[(y - 15) << 5]); // 1 line is 4 bytes (16pixels)
			} else {
				yaddr = &(qp[y << 5]); // 1 line is 4 bytes (16pixels)
			}
			p = &(yaddr[0]);
			if(xreverse) {
__DECL_VECTORIZED_LOOP
				for(int i = 7; i >= 0; i--) {
					pixels[i] = p[15 - (i << 1)];
				}
			} else {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i <= 7; i++) {
					pixels[i] = p[i << 1];
				}
			}
		} else {
			// Rotate: Swap x, y
			if(yreverse) {
				yaddr = &(qp[15 - y]);
			} else {
				yaddr = &(qp[y]);
			}			
			if(xreverse) {
__DECL_VECTORIZED_LOOP				
				for(int x = 7; x >= 0; x--) {
					p = &(yaddr[x << 6]);
					pixels[x] = p[15 - y];
				}
			} else {
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x <= 7; x++) {
					p = &(yaddr[x << 6]);
					pixels[x] = p[y];
				}
			}
		}
__DECL_VECTORIZED_LOOP		
		for(int x = 0; x < 8; x++) {
			masks[x] = ((pixels[x] & 0x8000) == 0) ? 0xffff : 0x0000;
			cp[x] = pixels[x];
			cm[x] = masks[x];
			// Draw to buffer
			tpixels[x] = dp[x] & masks[x];
			masks[x] = ~masks[x];
			pixels[x] = pixels[x] & masks[x];
			dp[x] = pixels[x] | tpixels[x];
		}
	}
}


inline void TOWNS_SPRITE::render_32768_x1_x05(int num, uint16_t* dst, int rot_type, bool is_mirror, int stride)
{
	uint16_t* qp = (uint16_t*)(sprite_table[num].pixels);
	uint16_t *cache_pixel = &(cache_pixels[last_put_cache_num][0]);
	uint16_t *cache_mask = cache_masks[last_put_cache_num][0];
	int xbegin;
	int xend;
	int xstep;
	int ybegin;
	int yend;
	int ystep;
	int addr_base;
	int addr_inc;
	bool xreverse;
	bool yreverse;
	bool rotate = false;
	switch(rot_type) {
	case ROT_FMTOWNS_SPRITE_0:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = false;
			yreverse = false;
			rotate = false;
			if(is_mirror) {
				xreverse = true;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_90:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = false;
			yreverse = true;
			rotate = true;
			if(is_mirror) {
				yreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_180:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = true;
			yreverse = true;
			rotate = false;
			if(is_mirror) {
				xreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_270:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = true;
			yreverse = false;
			rotate = true;
			if(is_mirror) {
				yreverse = true;
			}
		}			
		break;
	}
	uint16_t pixels[16];
	uint16_t masks[16];
	uint16_t tpixels[16];
	uint16_t* cp;
	uint16_t* cm;
	uint16* dp;
	uint16_t* yaddr;
	uint16_t*p;
	for(y = ybegin; y != yend; y += ystep) {
		cp = &(cache_pixel[y << 5]);
		cm = &(cache_mask[y << 5]);
		dp =  &(dst[stride * y]);
		if(!rotate) {
			if(yreverse) {
				yaddr = &(qp[(7 - y) << 6]); // 1 line is 4 bytes (16pixels)
			} else {
				yaddr = &(qp[y << 6]); // 1 line is 4 bytes (16pixels)
			}
			p = &(yaddr[0]);
			if(xreverse) {
__DECL_VECTORIZED_LOOP
				for(int i = 15; i >= 0; i--) {
					pixels[i] = p[15 - i];
				}
			} else {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i <= 15; i++) {
					pixels[i] = p[i];
				}
			}
		} else {
			// Rotate: Swap x, y
			if(yreverse) {
				yaddr = &(qp[15 - (y << 1)]);
			} else {
				yaddr = &(qp[y << 1]);
			}			
			if(xreverse) {
__DECL_VECTORIZED_LOOP				
				for(int x = 7; x >= 0; x--) {
					p = &(yaddr[x << 6]);
					pixels[x] = p[7 - y];
				}
			} else {
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x <= 7; x++) {
					p = &(yaddr[x << 6]);
					pixels[x] = p[y];
				}
			}
		}
__DECL_VECTORIZED_LOOP		
		for(int x = 0; x < 8; x++) {
			masks[x] = ((pixels[x] & 0x8000) == 0) ? 0xffff : 0x0000;
			cp[x] = pixels[x];
			cm[x] = masks[x];
			// Draw to buffer
			tpixels[x] = dp[x] & masks[x];
			masks[x] = ~masks[x];
			pixels[x] = pixels[x] & masks[x];
			dp[x] = pixels[x] | tpixels[x];
		}
	}
}

inline void TOWNS_SPRITE::render_32768_x05_x05(int num, uint16_t* dst, int rot_type, bool is_mirror, int stride)
{
	uint16_t* qp = (uint16_t*)(sprite_table[num].pixels);
	uint16_t *cache_pixel = &(cache_pixels[last_put_cache_num][0]);
	uint16_t *cache_mask = cache_masks[last_put_cache_num][0];
	int xbegin;
	int xend;
	int xstep;
	int ybegin;
	int yend;
	int ystep;
	int addr_base;
	int addr_inc;
	bool xreverse;
	bool yreverse;
	bool rotate = false;
	switch(rot_type) {
	case ROT_FMTOWNS_SPRITE_0:
			xbegin = 0;
			xend = 8;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = false;
			yreverse = false;
			rotate = false;
			if(is_mirror) {
				xreverse = true;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_90:
			xbegin = 0;
			xend = 8;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = false;
			yreverse = true;
			rotate = true;
			if(is_mirror) {
				yreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_180:
			xbegin = 0;
			xend = 8;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = true;
			yreverse = true;
			rotate = false;
			if(is_mirror) {
				xreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_270:
			xbegin = 0;
			xend = 8;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = true;
			yreverse = false;
			rotate = true;
			if(is_mirror) {
				yreverse = true;
			}
		}			
		break;
	}
	uint16_t pixels[16];
	uint16_t masks[16];
	uint16_t tpixels[16];
	uint16_t* cp;
	uint16_t* cm;
	uint16* dp;
	uint16_t* yaddr;
	uint16_t*p;
	for(y = ybegin; y != yend; y += ystep) {
		cp = &(cache_pixel[y << 5]);
		cm = &(cache_mask[y << 5]);
		dp =  &(dst[stride * y]);
		if(!rotate) {
			if(yreverse) {
				yaddr = &(qp[(7 - y) << 6]); // 1 line is 8 bytes (16pixels)
			} else {
				yaddr = &(qp[y << 6]); // 1 line is 8 bytes (16pixels)
			}
			p = &(yaddr[0]);
			if(xreverse) {
__DECL_VECTORIZED_LOOP
				for(int i = 7; i >= 0; i--) {
					pixels[i] = p[15 - (i << 1)];
				}
			} else {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i <= 7; i++) {
					pixels[i] = p[i << 1];
				}
			}
		} else {
			// Rotate: Swap x, y
			if(yreverse) {
				yaddr = &(qp[15 - (y << 1)]);
			} else {
				yaddr = &(qp[y << 1]);
			}			
			if(xreverse) {
__DECL_VECTORIZED_LOOP				
				for(int x = 7; x >= 0; x--) {
					p = &(yaddr[x << 6]);
					pixels[x] = p[7 - (y >> 1)];
				}
			} else {
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x <= 7; x++) {
					p = &(yaddr[x << 6]);
					pixels[x] = p[y];
				}
			}
		}
__DECL_VECTORIZED_LOOP		
		for(int x = 0; x < 8; x++) {
			masks[x] = ((pixels[x] & 0x8000) == 0) ? 0xffff : 0x0000;
			cp[x] = pixels[x];
			cm[x] = masks[x];
			// Draw to buffer
			tpixels[x] = dp[x] & masks[x];
			masks[x] = ~masks[x];
			pixels[x] = pixels[x] & masks[x];
			dp[x] = pixels[x] | tpixels[x];
		}
	}
}


inline void TOWNS_SPRITE::render_16_x1_x1(int num, uint16_t* dst, int rot_type, bool is_mirror, int stride)
{
	uint8_t* qp = (uint8_t*)(sprite_table[num].pixels);
	uint16_t *cache_pixel = &(cache_pixels[last_put_cache_num][0]);
	uint16_t *cache_mask = cache_masks[last_put_cache_num][0];
	int xbegin;
	int xend;
	int xstep;
	int ybegin;
	int yend;
	int ystep;
	int addr_base;
	int addr_inc;
	bool xreverse;
	bool yreverse;
	bool rotate = false;
	switch(rot_type) {
	case ROT_FMTOWNS_SPRITE_0:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = false;
			yreverse = false;
			rotate = false;
			if(is_mirror) {
				xreverse = true;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_90:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = false;
			yreverse = true;
			rotate = true;
			if(is_mirror) {
				yreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_180:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = true;
			yreverse = true;
			rotate = false;
			if(is_mirror) {
				xreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_270:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = true;
			yreverse = false;
			rotate = true;
			if(is_mirror) {
				yreverse = true;
			}
		}			
		break;
	}
	uint8_t pixels[16];
	uint16_t masks[16];
	uint16_t tpixels[16];
	uint16_t tpixels2[16];
	uint16_t* cp;
	uint16_t* cm;
	uint16* dp;
	uint8_t* yaddr;
	uint8_t* p;
	for(y = ybegin; y != yend; y += ystep) {
		cp = &(cache_pixel[y << 5]);
		cm = &(cache_mask[y << 5]);
		dp =  &(dst[stride * y]);
		if(!rotate) {
			if(yreverse) {
				yaddr = &(qp[(y - 15) << 3]); // 1 line is 4 bytes (16pixels)
			} else {
				yaddr = &(qp[y << 3]); // 1 line is 4 bytes (16pixels)
			}
			p = &(yaddr[0]);
			if(xreverse) {
__DECL_VECTORIZED_LOOP
				for(int i = 15; i >= 0; i--) {
					pixels[i] = p[(15 - i) >> 1];
				}
			} else {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i <= 15; i++) {
					pixels[i] = p[i];
				}
			}
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 16; i+= 2) {
				pixels[i] = pixels[i] >> 4;
			}
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 16; i++) {
				pixels[i] = pixels[i] & 0x0f;
			}
		} else {
			// Rotate: Swap x, y
			if(yreverse) {
				yaddr = &(qp[(15 - y) >> 1]);
			} else {
				yaddr = &(qp[(y >> 1)]);
			}			
			if(xreverse) {
__DECL_VECTORIZED_LOOP				
				for(int x = 15; x >= 0; x--) {
					p = &(yaddr[x << 3]);
					pixels[x] = p[(15 - y) >> 1];
				}
			} else {
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x <= 15; x++) {
					p = &(yaddr[x << 3]);
					pixels[x] = p[y >> 1];
				}
			}
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 16; i++) {
				pixels[i] = pixels[i] >> 4;
				pixels[i] = pixels[i] & 0x0f;
			}
			
		}
__DECL_VECTORIZED_LOOP		
		for(int x = 0; x < 16; x++) {
			masks[x] = (pixels[x] == 0) ? 0xffff : 0x0000
			tpixel[x] = color_index[pixels[x]];
			cp[x] = tpixel;
			cm[x] = masks[x];
			// Draw to buffer
			tpixels2[x] = dp[x] & masks[x];
			masks[x] = ~masks[x];
			tpixels[x] = tpixels[x] & masks[x];
			dp[x] = tpixels[x] | tpixels2[x];
		}
	}
}

inline void TOWNS_SPRITE::render_16_x05_x1(int num, uint16_t* dst, int rot_type, bool is_mirror, int stride)
{
	uint16_t* qp = (uint16_t*)(sprite_table[num].pixels);
	uint16_t *cache_pixel = &(cache_pixels[last_put_cache_num][0]);
	uint16_t *cache_mask = cache_masks[last_put_cache_num][0];
	int xbegin;
	int xend;
	int xstep;
	int ybegin;
	int yend;
	int ystep;
	int addr_base;
	int addr_inc;
	bool xreverse;
	bool yreverse;
	bool rotate = false;
	switch(rot_type) {
	case ROT_FMTOWNS_SPRITE_0:
			xbegin = 0;
			xend = 16;
			xstep = 2;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = false;
			yreverse = false;
			rotate = false;
			if(is_mirror) {
				xreverse = true;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_90:
			xbegin = 0;
			xend = 16;
			xstep = 2;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = false;
			yreverse = true;
			rotate = true;
			if(is_mirror) {
				yreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_180:
			xbegin = 0;
			xend = 16;
			xstep = 2;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = true;
			yreverse = true;
			rotate = false;
			if(is_mirror) {
				xreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_270:
			xbegin = 0;
			xend = 16;
			xstep = 2;
			ybegin = 0;
			yend = 16;
			ystep = 1;
			xreverse = true;
			yreverse = false;
			rotate = true;
			if(is_mirror) {
				yreverse = true;
			}
		}			
		break;
	}
	uint16_t pixels[16];
	uint16_t masks[16];
	uint16_t tpixels[16];
	uint16_t* cp;
	uint16_t* cm;
	uint16* dp;
	uint8_t* yaddr;
	uint8_t*p;
	for(y = ybegin; y != yend; y += ystep) {
		cp = &(cache_pixel[y << 5]);
		cm = &(cache_mask[y << 5]);
		dp =  &(dst[stride * y]);
		if(!rotate) {
			if(yreverse) {
				yaddr = &(qp[(y - 15) << 3]); // 1 line is 4 bytes (16pixels)
			} else {
				yaddr = &(qp[y << 3]); // 1 line is 4 bytes (16pixels)
			}
			p = &(yaddr[0]);
			if(xreverse) {
__DECL_VECTORIZED_LOOP
				for(int i = 7; i >= 0; i--) {
					pixels[i] = p[7 - i];
				}
			} else {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i <= 7; i++) {
					pixels[i] = p[i];
				}
			}
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i+= 2) {
				pixels[i] = pixels[i] >> 4;
				pixels[i] = pixels[i] & 0x0f;
			}
		} else {
			// Rotate: Swap x, y
			if(yreverse) {
				yaddr = &(qp[(15 - y) >> 1]);
			} else {
				yaddr = &(qp[y >> 1]);
			}			
			if(xreverse) {
__DECL_VECTORIZED_LOOP				
				for(int x = 7; x >= 0; x--) {
					p = &(yaddr[x << 4]);
					pixels[x] = p[15 - y];
				}
			} else {
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x <= 7; x++) {
					p = &(yaddr[x << 4]);
					pixels[x] = p[y];
				}
			}
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				pixels[i] = pixels[i] >> 4;
				pixels[i] = pixels[i] & 0x0f;
			}
		}
__DECL_VECTORIZED_LOOP		
		for(int x = 0; x < 8; x++) {
			masks[x] = ((pixels[x] & 0x8000) == 0) ? 0xffff : 0x0000;
			cp[x] = pixels[x];
			cm[x] = masks[x];
			// Draw to buffer
			tpixels[x] = dp[x] & masks[x];
			masks[x] = ~masks[x];
			pixels[x] = pixels[x] & masks[x];
			dp[x] = pixels[x] | tpixels[x];
		}
	}
}


inline void TOWNS_SPRITE::render_16_x1_x05(int num, uint16_t* dst, int rot_type, bool is_mirror, int stride)
{
	uint16_t* qp = (uint16_t*)(sprite_table[num].pixels);
	uint16_t *cache_pixel = &(cache_pixels[last_put_cache_num][0]);
	uint16_t *cache_mask = cache_masks[last_put_cache_num][0];
	int xbegin;
	int xend;
	int xstep;
	int ybegin;
	int yend;
	int ystep;
	int addr_base;
	int addr_inc;
	bool xreverse;
	bool yreverse;
	bool rotate = false;
	switch(rot_type) {
	case ROT_FMTOWNS_SPRITE_0:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = false;
			yreverse = false;
			rotate = false;
			if(is_mirror) {
				xreverse = true;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_90:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = false;
			yreverse = true;
			rotate = true;
			if(is_mirror) {
				yreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_180:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = true;
			yreverse = true;
			rotate = false;
			if(is_mirror) {
				xreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_270:
			xbegin = 0;
			xend = 16;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = true;
			yreverse = false;
			rotate = true;
			if(is_mirror) {
				yreverse = true;
			}
		}			
		break;
	}
	uint16_t pixels[16];
	uint16_t masks[16];
	uint16_t tpixels[16];
	uint16_t* cp;
	uint16_t* cm;
	uint16* dp;
	uint16_t* yaddr;
	uint16_t*p;
	for(y = ybegin; y != yend; y += ystep) {
		cp = &(cache_pixel[y << 5]);
		cm = &(cache_mask[y << 5]);
		dp =  &(dst[stride * y]);
		if(!rotate) {
			if(yreverse) {
				yaddr = &(qp[(7 - y) << 6]); // 1 line is 4 bytes (16pixels)
			} else {
				yaddr = &(qp[y << 6]); // 1 line is 4 bytes (16pixels)
			}
			p = &(yaddr[0]);
			if(xreverse) {
__DECL_VECTORIZED_LOOP
				for(int i = 15; i >= 0; i--) {
					pixels[i] = p[15 - i];
				}
			} else {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i <= 15; i++) {
					pixels[i] = p[i];
				}
			}
		} else {
			// Rotate: Swap x, y
			if(yreverse) {
				yaddr = &(qp[15 - (y << 1)]);
			} else {
				yaddr = &(qp[y << 1]);
			}			
			if(xreverse) {
__DECL_VECTORIZED_LOOP				
				for(int x = 7; x >= 0; x--) {
					p = &(yaddr[x << 6]);
					pixels[x] = p[7 - y];
				}
			} else {
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x <= 7; x++) {
					p = &(yaddr[x << 6]);
					pixels[x] = p[y];
				}
			}
		}
__DECL_VECTORIZED_LOOP		
		for(int x = 0; x < 8; x++) {
			masks[x] = ((pixels[x] & 0x8000) == 0) ? 0xffff : 0x0000;
			cp[x] = pixels[x];
			cm[x] = masks[x];
			// Draw to buffer
			tpixels[x] = dp[x] & masks[x];
			masks[x] = ~masks[x];
			pixels[x] = pixels[x] & masks[x];
			dp[x] = pixels[x] | tpixels[x];
		}
	}
}

inline void TOWNS_SPRITE::render_16_x05_x05(int num, uint16_t* dst, int rot_type, bool is_mirror, int stride)
{
	uint16_t* qp = (uint16_t*)(sprite_table[num].pixels);
	uint16_t *cache_pixel = &(cache_pixels[last_put_cache_num][0]);
	uint16_t *cache_mask = cache_masks[last_put_cache_num][0];
	int xbegin;
	int xend;
	int xstep;
	int ybegin;
	int yend;
	int ystep;
	int addr_base;
	int addr_inc;
	bool xreverse;
	bool yreverse;
	bool rotate = false;
	switch(rot_type) {
	case ROT_FMTOWNS_SPRITE_0:
			xbegin = 0;
			xend = 8;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = false;
			yreverse = false;
			rotate = false;
			if(is_mirror) {
				xreverse = true;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_90:
			xbegin = 0;
			xend = 8;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = false;
			yreverse = true;
			rotate = true;
			if(is_mirror) {
				yreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_180:
			xbegin = 0;
			xend = 8;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = true;
			yreverse = true;
			rotate = false;
			if(is_mirror) {
				xreverse = false;
			}
		}			
		break;
	case ROT_FMTOWNS_SPRITE_270:
			xbegin = 0;
			xend = 8;
			xstep = 1;
			ybegin = 0;
			yend = 8;
			ystep = 1;
			xreverse = true;
			yreverse = false;
			rotate = true;
			if(is_mirror) {
				yreverse = true;
			}
		}			
		break;
	}
	uint16_t pixels[16];
	uint16_t masks[16];
	uint16_t tpixels[16];
	uint16_t* cp;
	uint16_t* cm;
	uint16* dp;
	uint16_t* yaddr;
	uint16_t*p;
	for(y = ybegin; y != yend; y += ystep) {
		cp = &(cache_pixel[y << 5]);
		cm = &(cache_mask[y << 5]);
		dp =  &(dst[stride * y]);
		if(!rotate) {
			if(yreverse) {
				yaddr = &(qp[(7 - y) << 6]); // 1 line is 4 bytes (16pixels)
			} else {
				yaddr = &(qp[y << 6]); // 1 line is 4 bytes (16pixels)
			}
			p = &(yaddr[0]);
			if(xreverse) {
__DECL_VECTORIZED_LOOP
				for(int i = 7; i >= 0; i--) {
					pixels[i] = p[15 - (i << 1)];
				}
			} else {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i <= 7; i++) {
					pixels[i] = p[i << 1];
				}
			}
		} else {
			// Rotate: Swap x, y
			if(yreverse) {
				yaddr = &(qp[15 - (y << 1)]);
			} else {
				yaddr = &(qp[y << 1]);
			}			
			if(xreverse) {
__DECL_VECTORIZED_LOOP				
				for(int x = 7; x >= 0; x--) {
					p = &(yaddr[x << 6]);
					pixels[x] = p[7 - x];
				}
			} else {
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x <= 7; x++) {
					p = &(yaddr[x << 6]);
					pixels[x] = p[x];
				}
			}
		}
__DECL_VECTORIZED_LOOP		
		for(int x = 0; x < 8; x++) {
			masks[x] = ((pixels[x] & 0x8000) == 0) ? 0xffff : 0x0000;
			cp[x] = pixels[x];
			cm[x] = masks[x];
			// Draw to buffer
			tpixels[x] = dp[x] & masks[x];
			masks[x] = ~masks[x];
			pixels[x] = pixels[x] & masks[x];
			dp[x] = pixels[x] | tpixels[x];
		}
	}
}


inline void TOWNS_SPRITE::render_base(int num, uint16* dst_pixel, int width, int height, int stride)
{
	int half_type = 0;
	int rot_type;
	half_type = half_type | ((cache_index[num].is_halfx) ? 1 : 0);
	half_type = half_type | ((cache_index[num].is_halfy) ? 2 : 0);

	switch((sprite_table[num].rotate) & 7) {
	case 0:
		rot_type = ROT_FMTOWNS_SPRITE_0;
		is_mirror = false;
		break;
	case 1:
		rot_type = ROT_FMTOWNS_SPRITE_180;
		is_mirror = true;
		break;
	case 2:
		rot_type = ROT_FMTOWNS_SPRITE_180;
		is_mirror = false;
		break;
	case 3:
		rot_type = ROT_FMTOWNS_SPRITE_0;
		is_mirror = true;
		break;
	case 4:
		rot_type = ROT_FMTOWNS_SPRITE_270;
		is_mirror = true;
		break;
	case 5:
		rot_type = ROT_FMTOWNS_SPRITE_90;
		is_mirror = false;
		break;
	case 6:
		rotate = false;
		rot_type = ROT_FMTOWNS_SPRITE_270;
		is_mirror = false;
		break;
	case 7:
		rot_type = ROT_FMTOWNS_SPRITE_90;
		is_mirror = true;
		break;
	}
	if(sprite_table[num].is_32768) {
	switch(half_type & 3) {
	case 0: // not rotate
		render_32768_x1_x1(num, dst, rot_type, mirror, stride);
		break;
	case 1:
		render_32768_x05_x1(num, dst, rot_type, mirror, stride);
		break;
	case 2:
		render_32768_x1_x05(num, dst, rot_type, mirror, stride);
		break;
	case 3:
		render_32768_x05_x05(num, dst, rot_type, mirror, stride);
		break;
	}
	} else {
	switch(half_type & 3) {
	case 0: // not rotate
		render_16_x1_x1(num, dst, rot_type, mirror, stride);
		break;
	case 1:
		render_16_x05_x1(num, dst, rot_type, mirror, stride);
		break;
	case 2:
		render_16_x1_x05(num, dst, rot_type, mirror, stride);
		break;
	case 3:
		render_16_x05_x05(num, dst, rot_type, mirror, stride);
		break;
	}
	}
}

#endif /* _TOWNS_SPRITE_H */
