
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

typedef struct {
	uint16_t num;
	uint8_t rotate_type;
	uint16_t attribute;
	uint16_t color;
	
	bool is_halfx;
	bool is_halfy;
	bool enable_offset;
	bool is_32768;
	bool is_impose;
	bool is_disp;
} sprite_table_t;

typedef struct {
	bool is_use;
	uint16_t num;
	uint16_t attribute;
	uint8_t rotate_type;
	bool is_32768;
	bool is_halfx;
	bool is_halfy;
	uint16_t* pixels;
	uint16_t* masks;
	uint16_t color;
} sprite_cache_t;	

class TOWNS_VRAM;
class TOWNS_SPRITE : public DEVICE
{

protected:
	TOWNS_VRAM *vram_head;
	uint16_t* vram_buffer; // uint16_t[256][256]
	uint16_t* mask_buffer; // uint16_t[256][256]
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
	uint8_t display_page;
	uint8_t write_page;
	int render_num;
	int render_mod;
	int render_lines;
	bool split_rendering;
	
	uint16_t index_ram[4096]; // 1024 * 4
	uint16_t color_ram[4096]; // 16 * 256
	uint8_t pattern_ram[(65536 - (4096 * 2)) * 2];
	
	bool pattern_cached[((65536 - (4096 * 2)) * 2) / (8 * 16)];
	bool color_cached[256];
	
	bool sprite_enabled;
	bool use_cache;
	int32_t last_put_cache_num;
	
	sprite_table_t sprite_table[1024];
	sprite_cache_t cache_index[TOWNS_SPRITE_CACHE_NUM];
	uint16_t cache_pixels[TOWNS_SPRITE_CACHE_NUM][16 * 16];
	uint16_t cache_masks[TOWNS_SPRITE_CACHE_NUM][16 * 16];
	
	inline void take_data_32768(uint16_t* src, uint16_t* dst, uint16_t* mask);
	inline void take_data_32768_mirror(uint16_t* src, uint16_t* dst, uint16_t* mask);
	inline void take_data_16(uint8_t* src, uint16_t* color_table, uint16_t* dst, uint16_t* mask);
	inline void take_data_16_mirror(uint8_t* src, uint16_t* color_table, uint16_t* dst, uint16_t* mask);
	inline void zoom_data(uint16_t* cache, uint16_t* maskcache, bool is_halfx, bool is_halfy, uint16_t* dstcache, uint16_t* dstmask);

	void rotate_data_0(uint16_t* src, bool is_mirror, uint16_t* color_table, uint16_t* dstcache, uint16_t* mask, bool is_32768, bool is_halfx, bool is_halfy);
	void rotate_data_90(uint16_t* src, bool is_mirror, uint16_t* color_table, uint16_t* dstcache, uint16_t* mask, bool is_32768, bool is_halfx, bool is_halfy);
	void rotate_data_180(uint16_t* src, bool is_mirror, uint16_t* color_table, uint16_t* dstcache, uint16_t* mask, bool is_32768, bool is_halfx, bool is_halfy);
	void rotate_data_270(uint16_t* src, bool is_mirror, uint16_t* color_table, uint16_t* dstcache, uint16_t* mask, bool is_32768, bool is_halfx, bool is_halfy);

	void clear_cache(int num);
	void build_sprite_table(void);
	
	void set_sprite_attribute(int table_num, uint16_t num_attr);
	void set_sprite_color(int table_num, uint16_t color_table_num);
	bool check_cache(int num, sprite_cache_t** p);
	void render_zoomed_pixels(int x, int y, int uint16_t* pixels, uint16_t* masks, bool is_halfx, bool is_halfy, uint16_t* dst_pixel, uint16_t* dst_mask);
	void render_sprite(int num, uint16* dst_pixel, uint16_t* dst_mask, int x, int y);
	void render(uint16_t *buffer, uint16_t* mask);

public:
	TOWNS_SPRITE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("SPRITE"));
		vram_head = NULL;
		framebuffer = NULL;
		
	}
	~TOWNS_SPRITE() {}

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	void write_data8(uint32_t addr, uint32_t data);
	void write_data16(uint32_t addr, uint32_t data);
	void write_data32(uint32_t addr, uint32_t data);
	
	uint32_t read_data8(uint32_t addr);
	uint32_t read_data16(uint32_t addr);
	uint32_t read_data32(uint32_t addr);

	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void initialize();
	void event_frame();
	void event_vline(int v, int clock);
	
	void set_context_vram(TOWNS_VRAM *p)
	{
		vram_head = p;
	}
	void save_state(FILEIO *fio);
	bool load_stste(FILEIO *fio);
};

inline void TOWNS_SPRITE::take_data_32768(uint16_t* src, uint16_t* dst, uint16_t* mask)
{
	uint16_t* p = src;
	uint16_t* q = dst;
	uint16_t* r = mask;
	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x++) {
			q[x] = p[x];
		}
		for(int x = 0; x < 16; x++) {
			r[x] = ((q[x] & 0x8000) != 0) ? 0 : 0xffff;
		}
		r += 16;
		q += 16;
		p += 16;
	}
}

inline void TOWNS_SPRITE::take_data_32768_mirror(uint16_t* src, uint16_t* dst, uint16_t* mask)
{
	uint16_t* p = src;
	uint16_t* q = dst;
	uint16_t* r = mask;
	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x++) {
			q[x] = p[15 - x];
		}
		for(int x = 0; x < 16; x++) {
			r[x] = ((q[x] & 0x8000) != 0) ? 0 : 0xffff;
		}
		r += 16;
		q += 16;
		p += 16;
	}
}


inline void TOWNS_SPRITE::take_data_16(uint8_t* src, uint16_t* color_table, uint16_t* dst, uint16_t* mask)
{
	uint8_t* p = src;
	uint16_t* q = dst;
	uint16_t* r = mask;
	uint8_t cache[16];
	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x += 2) {
			cache[x] = p[x >> 1];
			cache[x + 1] = cache[x];
		}
		for(int x = 0; x < 16; x += 2) {
			cache[x] = cache[x] >> 4;
		}
		for(int x = 0; x < 16; x++) {
			cache[x] = cache[x] & 0x0f;
			r[x] = (cache[x] == 0) ? 0x0000 : 0xffff;
			q[x] = color_table[cache[x]];
		}
		r += 16;
		q += 16;
		p += 8;
	}
}

inline void TOWNS_SPRITE::take_data_16_mirror(uint8_t* src, uint16_t* color_table, uint16_t* dst, uint16_t* mask)
{
	uint8_t* p = src;
	uint16_t* q = dst;
	uint16_t* r = mask;
	uint8_t cache[16];
	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x += 2) {
			cache[x] = p[(15 - x) >> 1];
			cache[x + 1] = cache[x];
		}
		for(int x = 1; x < 16; x += 2) {
			cache[x] = cache[x] >> 4;
		}
		for(int x = 0; x < 16; x++) {
			cache[x] = cache[x] & 0x0f;
			r[x] = (cache[x] == 0) ? 0x0000 : 0xffff;
			q[x] = color_table[cache[x]];
		}
		r += 16;
		q += 16;
		p += 8;
	}
}


void TOWNS_SPRITE::rotate_data_0(uint16_t* src, bool is_mirror, uint16_t* color_table, uint16_t* dstcache, uint16_t* mask, bool is_32768, bool is_halfx, bool is_halfy)
{
	uint16_t cache[16 * 16];
	if(!is_32768) {
		if(is_mirror) {
			take_data_16_mirror(src, color_table, cache, mask);
		} else {
			take_data_16(src, color_table, cache, mask);
		}
		
	} else {
		if(is_mirror) {
			take_data_32768_mirror((uint16_t*)src, cache, mask);
		} else {
			take_data_16((uint16_t*)src, cache, mask);
		}
	}
	// Rotate
	// Zoom
	uint16_t maskcache[16 * 16];
	memcpy(maskcache, mask, sizeof(maskcache));
	zoom_data(cache, maskcache, is_halfx, is_halfy, dstcache, mask);
}

void TOWNS_SPRITE::rotate_data_90(uint16_t* src, bool is_mirror, uint16_t* color_table, uint16_t* dstcache, uint16_t* mask, bool is_32768, bool is_halfx, bool is_halfy)
{
	uint16_t cache[16 * 16];
	if(!is_32768) {
		if(is_mirror) {
			take_data_16_mirror(src, color_table, cache, mask);
		} else {
			take_data_16(src, color_table, cache, mask);
		}
		
	} else {
		if(is_mirror) {
			take_data_32768_mirror((uint16_t*)src, cache, mask);
		} else {
			take_data_16((uint16_t*)src, cache, mask);
		}
	}
	// Rotate
	uint16_t maskcache[16][16];
	uint16_t cache2[16][16];
	if(is_mirror) {
		// q[x][y] = p[15 - y][15 - x]
__DECL_VECTORIZED_LOOP				
		for(y = 0; y < 16; y++) {
			uint16_t* p = &(cache[15 - y]); 
			uint16_t* q = &(mask[15 - y]); 
			for(x = 0; x < 16; x++) {
				cache2[y][x] = p[(15 - x) << 4];
				maskcache[y][x] = q[(15 - x) << 4];
			}
		}			
	} else {
		// q[x][y] = p[15 - y][x]
		for(y = 0; y < 16; y++) {
			uint16_t* p = &(cache[15 - y]);
			uint16_t* q = &(mask[15 - y]);
__DECL_VECTORIZED_LOOP					
			for(x = 0; x < 16; x++) {
				cache2[y][x] = p[x << 4];
				maskcache[y][x] = q[x << 4];
			}
		}			
	}	
	zoom_data((uint16_t*)(&(cache2[0][0])), (uint16_t*)(&(maskcache[0][0])), is_halfx, is_halfy, dstcache, mask);
}

void TOWNS_SPRITE::rotate_data_180(uint16_t* src, bool is_mirror, uint16_t* color_table, uint16_t* dstcache, uint16_t* mask, bool is_32768, bool is_halfx, bool is_halfy)
{
	uint16_t cache[16 * 16];
	if(!is_32768) {
		if(is_mirror) {
			take_data_16_mirror(src, color_table, cache, mask);
		} else {
			take_data_16(src, color_table, cache, mask);
		}
		
	} else {
		if(is_mirror) {
			take_data_32768_mirror((uint16_t*)src, cache, mask);
		} else {
			take_data_16((uint16_t*)src, cache, mask);
		}
	}
	// Rotate
	uint16_t maskcache[16][16];
	uint16_t cache2[16][16];
	if(is_mirror) {
		// q[x][y] = p[x][15 - y]
		for(y = 0; y < 16; y++) {
			uint16_t* p = &(cache[(15 - y) << 4]); 
			uint16_t* q = &(mask[(15 - y) << 4]);
__DECL_VECTORIZED_LOOP					
			for(x = 0; x < 16; x++) {
				cache2[y][x] = p[x];
				maskcache[y][x] = q[x];
			}
		}			
	} else {
		// q[x][y] = p[15 - x][15 - y]
		for(y = 0; y < 16; y++) {
			uint16_t* p = &(cache[15 - y] << 4); 
			uint16_t* q = &(mask[15 - y] << 4);
__DECL_VECTORIZED_LOOP
			for(x = 0; x < 16; x++) {
				cache2[y][x] = p[15 - x];
				maskcache[y][x] = q[15 - x];
			}
		}			
	}	
	zoom_data((uint16_t*)(&(cache2[0][0])), (uint16_t*)(&(maskcache[0][0])), is_halfx, is_halfy, dstcache, mask);
}

void TOWNS_SPRITE::rotate_data_270(uint16_t* src, bool is_mirror, uint16_t* color_table, uint16_t* dstcache, uint16_t* mask, bool is_32768,bool is_halfx, bool is_halfy)
{
	uint16_t cache[16 * 16];
	if(!is_32768) {
		if(is_mirror) {
			take_data_16_mirror(src, color_table, cache, mask);
		} else {
			take_data_16(src, color_table, cache, mask);
		}
		
	} else {
		if(is_mirror) {
			take_data_32768_mirror((uint16_t*)src, cache, mask);
		} else {
			take_data_16((uint16_t*)src, cache, mask);
		}
	}
	// Rotate
	uint16_t maskcache[16][16];
	uint16_t cache2[16][16];
	if(is_mirror) {
		// q[x][y] = p[y][x]
		
		for(y = 0; y < 16; y++) {
			uint16_t* p = &(cache[y]); 
			uint16_t* q = &(mask[y]);
__DECL_VECTORIZED_LOOP			
			for(x = 0; x < 16; x++) {
				cache2[y][x] = p[x << 4];
				maskcache[y][x] = q[x << 4];
			}
		}			
	} else {
		// q[x][y] = p[y][15 - x]
		for(y = 0; y < 16; y++) {
			uint16_t* p = &(cache[15 - y]); 
			uint16_t* q = &(mask[15 - y]);
__DECL_VECTORIZED_LOOP			
			for(x = 0; x < 16; x++) {
				cache2[y][x] = p[x << 4];
				maskcache[y][x] = q[x << 4];
			}
		}			
	}	
	zoom_data((uint16_t*)(&(cache2[0][0])), (uint16_t*)(&(maskcache[0][0])), is_halfx, is_halfy, dstcache, mask);
}

inline void TOWNS_SPRITE::zoom_data(uint16_t* cache, uint16_t* maskcache, bool is_halfx, bool is_halfy, uint16_t* dstcache, uint16_t* dstmask)
{
	if(is_halfx) {
		if(is_halfy) {
			uint16_t cache2[8][8];
			uint16_t maskcache2[8][8];
			for(int y = 0; y < 16; y += 2) {
				uint16_t cacheline[8];
				uint16_t *pp = &(cache[y << 4]);
				uint16_t maskcacheline[8];
				uint16_t *pq = &(maskcache[y << 4]);
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x < 8; x++) {
					cacheline[x] = pp[x << 1];
					maskcacheline[x] = pq[x << 1];
				}
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x < 8; x++) {
					cache2[y >> 1][x] = cacheline[x];
					maskcache2[y >> 1][x] = maskcacheline[x];
				}
			}
			memcpy(dstcache, &(cache2[0][0]), 8 * 8 * sizeof(uint16_t));
			memcpy(dstmask, &(maskcache2[0][0]), 8 * 8 * sizeof(uint16_t));
		} else { // halfx only, not halfy
			uint16_t cache2[16][8];
			uint16_t maskcache2[16][8];
			for(int y = 0; y < 16; y++) {
				uint16_t cacheline[8];
				uint16_t *pp = &(cache[y << 4]);
				uint16_t maskcacheline[8];
				uint16_t *pq = &(maskcache[y << 4]);
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x < 8; x++) {
					cacheline[x] = pp[x << 1];
					maskcacheline[x] = pq[x << 1];
				}
__DECL_VECTORIZED_LOOP				
				for(int x = 0; x < 8; x++) {
					cache2[y][x] = cacheline[x];
					maskcache2[y][x] = maskcacheline[x];
				}
			}
			memcpy(dstcache, &(cache2[0][0]), 16 * 8 * sizeof(uint16_t));
			memcpy(mask, &(maskcache2[0][0]), 16 * 8 * sizeof(uint16_t));
		}
	} else {
		if(is_halfy) { // halfx only, not halfx
			uint16_t cache2[16][8];
			uint16_t maskcache2[16][8];
			for(int y = 0; y < 16; y += 2) {
				uint16_t cacheline[16];
				uint16_t *pp = &(cache[y << 4]);
				uint16_t maskcacheline[16];
				uint16_t *pq = &(maskcache[y << 4]);
__DECL_VECTORIZED_LOOP
				for(int x = 0; x < 16; x++) {
					cacheline[x] = pp[x];
					maskcacheline[x] = pq[x];
				}
__DECL_VECTORIZED_LOOP
				for(int x = 0; x < 16; x++) {
					cache2[y >> 1][x] = cacheline[x];
					maskcache2[y >> 1][x] = maskcacheline[x];
				}
			}
			memcpy(dstcache, &(cache2[0][0]), 8 * 16 * sizeof(uint16_t));
			memcpy(dstmask, &(maskcache2[0][0]), 8 * 16 * sizeof(uint16_t));
		} else { // 1x1
			memcpy(dstcache, cache, 16 * 16 * sizeof(uint16_t));
			memcpy(dstmask, maskcache, 16 * 16 * sizeof(uint16_t));
		}
	}
}

#endif /* _TOWNS_SPRITE_H */
