
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

class TOWNS_VRAM;

class SPRITE_CACHE {
private:
	int begin_num;
	bool blank;
	bool is_16colors; // 32768colors = false
	bool used;
	
	uint16_t palette[16];
	bool cached_x10x10[8];
	bool cached_x05x10[8];
	bool cached_x10x05[8];
	bool cached_x05x05[8];
	uint16_t data_x10x10[8][16 * 16];
	uint16_t data_x05x10[8][8 * 16];
	uint16_t data_x10x05[8][16 * 8];
	uint16_t data_x05x05[8][8 * 8];
	uint16_t color_table[16];
public:
	SPRITE_CACHE(bool is_16c);
	~SPRITE_CACHE();

	uint16_t *get_cache_address(bool condense_x, bool condense_y, uint8_t rotate, uint32_t *size);
	bool set_cache(bool condense_x, bool condense_y, uint8_t rotate, uint16_t *srcdata);
	
	int get_cached_num(void);
	bool query_cached_range(int pos, bool is_16c);
	bool get_cached_is_16c(void);
	
	bool is_cached(bool is_16c, bool condense_x, bool condense_y, uint8_t rotate, uint16_t *palette);
	bool is_blank(void);
	bool is_used(void);
	void clean(void);
	void set_num(int num, uint16_t *palette);
};

#define TOWNS_SPRITE_CACHE_NUM 512

class TOWNS_SPRITE : public DEVICE
{
private:
	TOWNS_VRAM *vram_head;
	
	int cache_size_16c;
	int cache_size_32768c;

	// NEED ATOMIC witn  MULTI-THREADED SPRITE
	bool dirty_cache_16c[TOWNS_SPRITE_CACHE_NUM];
	bool dirty_cache_32768c[TOWNS_SPRITE_CACHE_NUM];	
	SPRITE_CACHE *cached_16c[TOWNS_SPRITE_CACHE_NUM]; // ToDo
	SPRITE_CACHE *cached_32768c[TOWNS_SPRITE_CACHE_NUM]; // ToDo

	bool dirty_color_table[256];
	bool dirty_pattern_table[1024];
	
	int splite_limit;
	int render_count;
	bool bank0;

	uint16_t palettes[256][16];

	// Put address(16bit)
	int xaddress[1024];
	int yaddress[1024];

	// Attribute(16bit)
	bool check_offset[1024];
	bool suy[1024];
	bool sux[1024];
	uint8_t rotate[1024];
	uint32_t pattern_num[1024];

	// Color Table(16bit)
	bool is_16c[1024];
	bool is_impose[1024];
	bool is_disp[1024];
	uint16_t color_table_num[1024];

	uint8_t index_table[2 * 4 * 1024]; 
	uint8_t color_table[2 * 16 * 256];
	uint8_t pattern_table[2 * 16 * 16 * (256 - 32)];
protected:
	void do_put_sprite(int num);
	
public:
	TOWNS_SPRITE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
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

#endif /* _TOWNS_SPRITE_H */
