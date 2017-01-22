/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.22 -

	[ Sprite ]
*/

#include "common.h"
#include "./towns_sprite.h"

void TOWNS_SPRITE::initialize(void)
{
	for(i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
		dirty_cache_16c[i] = false;
		dirty_cache_32768c[i] = false;
		cached_16c[i] = NULL;
		cached_32768c[i] = NULL;
		cached_16c[i] = new SPRITE_CACHE(true);
		cached_256c[i] = new SPRITE_CACHE(false);
	}
	
}

void TOWNS_SPRITE::clear_cache(int num, bool n_16c)
{
	if(num >= TOWNS_SPRITE_CACHE_NUM) return;
	if(num < 0) return;
	if(n_16c) {
		if(cached_16c[num] != NULL) {
			cached_16c[NUM]->clean();
		}
		dirty_cache_16c[num] = false;
	} else {
		if(cached_32768c[num] != NULL) {
			cached_32768c[NUM]->clean();
		}
		dirty_cache_32768c[num] = false;
	}		
}
void TOWNS_SPRITE::do_put_sprite(int num)
{
	if(num < 0) return;
	if(num > 1023) return;
	
	int _size;
	uint16_t buffer[16 * 16];
	uint16_t tmpbuf[16 * 16];
	
	if(is_16c[num]) {
		if((num < 128) || (num > 1023)) return;
		if((color_table_num[num] < 256) || (color_table_num[num] > 511)) return;
		
		memset(buffer, 0x00, sizeof(uint16_t) * 16 * 16);
		
		for(int i = 0; i < [TOWNS_SPRITE_CACHE_NUM]; i++) {
			if(dirty_cache_16c[i]) {
				if(cached_16c[i] != NULL) {
					if(cached_16c[i]->query_cached_range(num, true)) {
						if(cached_16c[i]->is_cached(true,
													sux[i],
													suy[i],
													rotate[i],
													&(color_table[color_table_num[num] * 2 * 16]))) { // Hit
							uint16_t *p = cached_16c[i]->get_cache_address(sux[i], suy[i], rotate[i], &_size);
							if(p != NULL) {
								transfer_cache(i, num);
								return;
							}
						}
					}
				}
			}
			// Not hit
			uint16_t *pp = (uint16_t *)(&pattern_table[2 * 4 * 16 * (num - 128)]);
			_size = make_sprite(buffer, pp); // to, from
			if(_size > 0) {
				if(transfer_sprite_16(num, buffer, tmpbuf)) { // Put to VRAM
					int cnum = try_register_cache(num, buffer); // TRY
					if(cnum >= 0) {
						if(cached_16c[cnum] != NULL) {
							cached_16c[cnum]->set_cache(sux[num], suy[num], rotate[num], tmpbuf);
						}
					}
				}
			}
		}	
	} else { // 32768c
		memset(buffer, 0x00, sizeof(uint16_t) * 16 * 16);

	}
}

#define STATE_VERSION	1

bool TOWNS_SPRITE::save_state(FILEIO *state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
}

bool TOWNS_SPRITE::load_state(FILEIO *state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	return true;
}
