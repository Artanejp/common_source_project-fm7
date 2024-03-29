/****************************************************************************

  emu2212.c -- S.C.C. emulator by Mitsutaka Okazaki 2001

  2001 09-30 : Version 1.00
  2001 10-03 : Version 1.01 -- Added SCC_set_quality().
  2002 02-14 : Version 1.10 -- Added SCC_writeReg(), SCC_set_type().
                               Fixed SCC_write().
  2002 02-17 : Version 1.11 -- Fixed SCC_write().
  2002 03-02 : Version 1.12 -- Removed SCC_init & SCC_close.
  2003 09-19 : Version 1.13 -- Added SCC_setMask() and SCC_toggleMask()
  2004 10-21 : Version 1.14 -- Fixed the problem where SCC+ is disabled.
  2015 12-13 : Version 1.15 -- Changed own integer types to C99 stdint.h types.

  Registar map for SCC_writeReg()

  $00-1F : WaveTable CH.A
  $20-3F : WaveTable CH.B
  $40-5F : WaveTable CH.C
  $60-7F : WaveTable CH.D&E(SCC), CH.D(SCC+)
  $80-9F : WaveTable CH.E
 
  $C0    : CH.A Freq(L)
  $C1    : CH.A Freq(H)
  $C2    : CH.B Freq(L)
  $C3    : CH.B Freq(H)
  $C4    : CH.C Freq(L)
  $C5    : CH.C Freq(H)
  $C6    : CH.D Freq(L)
  $C7    : CH.D Freq(H)
  $C8    : CH.E Freq(L)
  $C9    : CH.E Freq(H)

  $D0    : CH.A Volume
  $D1    : CH.B Volume
  $D2    : CH.C Volume
  $D3    : CH.D Volume
  $D4    : CH.E Volume
 
  $E0    : Bit0 = 0:SCC, 1:SCC+
  $E1    : CH mask
  $E2    : Extra Flags

*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scc.h"
#include "../../types/util_sound.h"

#define SCC EMU2212

#define GETA_BITS 22

namespace MSX {

static void
internal_refresh (SCC * scc)
{
  if (scc->quality)
  {
    scc->base_incr = 2 << GETA_BITS;
    scc->realstep = (uint32_t) ((1 << 31) / scc->rate);
    scc->sccstep = (uint32_t) ((1 << 31) / (scc->clk / 2));
    scc->scctime = 0;
  }
  else
  {
    scc->base_incr = (uint32_t) ((double) scc->clk * (1 << GETA_BITS) / scc->rate);
  }
}

uint32_t
SCC_setMask (SCC *scc, uint32_t mask)
{
  uint32_t ret = 0;
  if(scc)
  {
    ret = scc->mask;
    scc->mask = mask;
  }  
  return ret;
}

uint32_t
SCC_toggleMask (SCC *scc, uint32_t mask)
{
  uint32_t ret = 0;
  if(scc)
  {
    ret = scc->mask;
    scc->mask ^= mask;
  }
  return ret;
}

void
SCC_set_quality (SCC * scc, uint32_t q)
{
  scc->quality = q;
  internal_refresh (scc);
}

void
SCC_set_rate (SCC * scc, uint32_t r)
{
  scc->rate = r ? r : 44100;
  internal_refresh (scc);
}

SCC *
SCC_new (uint32_t c, uint32_t r)
{
  SCC *scc;

  scc = (SCC *) malloc (sizeof (SCC));
  if (scc == NULL)
    return NULL;
  memset(scc, 0, sizeof (SCC));

  scc->clk = c;
  scc->rate = r ? r : 44100;
  SCC_set_quality (scc, 0);
  scc->type = SCC_ENHANCED;
  return scc;
}

void
SCC_reset (SCC * scc)
{
  int i, j;

  if (scc == NULL)
    return;

  scc->mode = 0;
  scc->active = 0;
  scc->base_adr = 0x9000;

  for (i = 0; i < 5; i++)
  {
    for (j = 0; j < 5; j++)
      scc->wave[i][j] = 0;
    scc->count[i] = 0;
    scc->freq[i] = 0;
    scc->phase[i] = 0;
    scc->volume[i] = 0;
    scc->offset[i] = 0;
    scc->rotate[i] = 0;
  }

  memset(scc->reg,0,0x100-0xC0);

  scc->mask = 0;

  scc->ch_enable = 0xff;
  scc->ch_enable_next = 0xff;

  scc->cycle_4bit = 0;
  scc->cycle_8bit = 0;
  scc->refresh = 0;

  scc->out = 0;
  scc->prev = 0;
  scc->next = 0;

  return;
}

void
SCC_delete (SCC * scc)
{
  if (scc != NULL)
    free (scc);
}

static inline int16_t
calc (SCC * scc)
{
  int i;
  int32_t mix = 0;

  for (i = 0; i < 5; i++)
  {
    scc->count[i] = (scc->count[i] + scc->incr[i]);

    if (scc->count[i] & (1 << (GETA_BITS + 5)))
    {
      scc->count[i] &= ((1 << (GETA_BITS + 5)) - 1);
      scc->offset[i] = (scc->offset[i] + 31) & scc->rotate[i];
      scc->ch_enable &= ~(1 << i);
      scc->ch_enable |= scc->ch_enable_next & (1 << i);
    }

    if (scc->ch_enable & (1 << i))
    {
      scc->phase[i] = ((scc->count[i] >> (GETA_BITS)) + scc->offset[i]) & 0x1F;
      if(!(scc->mask&SCC_MASK_CH(i)))
        mix += ((((int8_t) (scc->wave[i][scc->phase[i]]) * (int8_t) scc->volume[i]))) >> 4;
    }
  }

  return (int16_t) (mix << 4);
}

int16_t
SCC_calc (SCC * scc)
{
  if (!scc->quality)
    return calc (scc);

  while (scc->realstep > scc->scctime)
  {
    scc->scctime += scc->sccstep;
    scc->prev = scc->next;
    scc->next = calc (scc);
  }

  scc->scctime -= scc->realstep;
  scc->out = (int16_t) (((double) scc->next * (scc->sccstep - scc->scctime) + (double) scc->prev * scc->scctime) / scc->sccstep);

  return (int16_t) (scc->out);
}

uint32_t
SCC_readReg (SCC * scc, uint32_t adr)
{
  if (adr < 0xA0)
    return scc->wave[adr >> 5][adr & 0x1f];
  else if( 0xC0 < adr && adr < 0xF0 )
    return scc->reg[adr-0xC0];
  else
    return 0;
}

void
SCC_writeReg (SCC * scc, uint32_t adr, uint32_t val)
{
  int ch;
  uint32_t freq;

  adr &= 0xFF;

  if (adr < 0xA0)
  {
    ch = (adr & 0xF0) >> 5;
    if (!scc->rotate[ch])
    {
      scc->wave[ch][adr & 0x1F] = (int8_t) val;
      if (scc->mode == 0 && ch == 3)
        scc->wave[4][adr & 0x1F] = (int8_t) val;
    }
  }
  else if (0xC0 <= adr && adr <= 0xC9)
  {
    scc->reg[adr-0xC0] = val;
    ch = (adr & 0x0F) >> 1;
    if (adr & 1)
      scc->freq[ch] = ((val & 0xF) << 8) | (scc->freq[ch] & 0xFF);
    else
      scc->freq[ch] = (scc->freq[ch] & 0xF00) | (val & 0xFF);

    if (scc->refresh)
      scc->count[ch] = 0;
    freq = scc->freq[ch];
    if (scc->cycle_8bit)
      freq &= 0xFF;
    if (scc->cycle_4bit)
      freq >>= 8;
    if (freq <= 8)
      scc->incr[ch] = 0;
    else
      scc->incr[ch] = scc->base_incr / (freq + 1);
  }
  else if (0xD0 <= adr && adr <= 0xD4)
  {
    scc->reg[adr-0xC0] = val;
    scc->volume[adr & 0x0F] = (uint8_t) (val & 0xF);
  }
  else if (adr == 0xE0)
  {
    scc->reg[adr-0xC0] = val;
    scc->mode = (uint8_t) val & 1;
  }
  else if (adr == 0xE1)
  {
    scc->reg[adr-0xC0] = val;
    scc->ch_enable_next = (uint8_t) val & 0x1F;
  }
  else if (adr == 0xE2)
  {
    scc->reg[adr-0xC0] = val;
    scc->cycle_4bit = val & 1;
    scc->cycle_8bit = val & 2;
    scc->refresh = val & 32;
    if (val & 64)
      for (ch = 0; ch < 5; ch++)
        scc->rotate[ch] = 0x1F;
    else
      for (ch = 0; ch < 5; ch++)
        scc->rotate[ch] = 0;
    if (val & 128)
      scc->rotate[3] = scc->rotate[4] = 0x1F;
  }

  return;
}

static inline void
write_standard (SCC * scc, uint32_t adr, uint32_t val)
{
  adr &= 0xFF;

  if (adr < 0x80)               /* wave */
  {
    SCC_writeReg (scc, adr, val);
  }
  else if (adr < 0x8A)          /* freq */
  {
    SCC_writeReg (scc, adr + 0xC0 - 0x80, val);
  }
  else if (adr < 0x8F)          /* volume */
  {
    SCC_writeReg (scc, adr + 0xD0 - 0x8A, val);
  }
  else if (adr == 0x8F)         /* ch enable */
  {
    SCC_writeReg (scc, 0xE1, val);
  }
  else if (0xE0 <= adr)         /* flags */
  {
    SCC_writeReg (scc, 0xE2, val);
  }
}

static inline void
write_enhanced (SCC * scc, uint32_t adr, uint32_t val)
{
  adr &= 0xFF;

  if (adr < 0xA0)               /* wave */
  {
    SCC_writeReg (scc, adr, val);
  }
  else if (adr < 0xAA)          /* freq */
  {
    SCC_writeReg (scc, adr + 0xC0 - 0xA0, val);
  }
  else if (adr < 0xAF)          /* volume */
  {
    SCC_writeReg (scc, adr + 0xD0 - 0xAA, val);
  }
  else if (adr == 0xAF)         /* ch enable */
  {
    SCC_writeReg (scc, 0xE1, val);
  }
  else if (0xC0 <= adr && adr <= 0xDF)  /* flags */
  {
    SCC_writeReg (scc, 0xE2, val);
  }
}

static inline uint32_t 
read_enhanced (SCC * scc, uint32_t adr)
{
  adr &= 0xFF;
  if (adr < 0xA0)
    return SCC_readReg (scc, adr);
  else if (adr < 0xAA)
    return SCC_readReg (scc, adr + 0xC0 - 0xA0);
  else if (adr < 0xAF)
    return SCC_readReg (scc, adr + 0xD0 - 0xAA);
  else if (adr == 0xAF)
    return SCC_readReg (scc, 0xE1);
  else if (0xC0 <= adr && adr <= 0xDF)
    return SCC_readReg (scc, 0xE2);
  else
    return 0;
}

static inline uint32_t
read_standard (SCC * scc, uint32_t adr)
{
  adr &= 0xFF;
  if(adr<0x80)
    return SCC_readReg (scc, adr);
  else if (0xA0<=adr&&adr<=0xBF)
    return SCC_readReg (scc, 0x80+(adr&0x1F));
  else if (adr < 0x8A)          
    return SCC_readReg (scc, adr + 0xC0 - 0x80);
  else if (adr < 0x8F)          
    return SCC_readReg (scc, adr + 0xD0 - 0x8A);
  else if (adr == 0x8F)         
    return SCC_readReg (scc, 0xE1);
  else if (0xE0 <= adr)         
    return SCC_readReg (scc, 0xE2);
  else return 0;
}

uint32_t
SCC_read (SCC * scc, uint32_t adr)
{
  if( scc->type == SCC_ENHANCED && (adr&0xFFFE) == 0xBFFE ) 
    return (scc->base_adr>>8)&0x20;
  
  if( adr < scc->base_adr ) return 0;
  adr -= scc->base_adr;
  
  if( adr == 0 ) 
  {
    if(scc->mode) return 0x80; else return 0x3F;
  }

  if(!scc->active||adr<0x800||0x8FF<adr) return 0;

  switch (scc->type) 
  {
  case SCC_STANDARD:
      return read_standard (scc, adr);
    break;
  case SCC_ENHANCED:
    if(!scc->mode)
      return read_standard (scc, adr);
    else 
      return read_enhanced (scc, adr);
    break;
  default:
    break;
  }

  return 0;
}

void
SCC_write (SCC * scc, uint32_t adr, uint32_t val)
{
  val = val & 0xFF;

  if( scc->type == SCC_ENHANCED && (adr&0xFFFE) == 0xBFFE ) 
  {
    scc->base_adr = 0x9000 | ((val&0x20)<<8);
    return;
  }
  
  if( adr < scc->base_adr ) return;
  adr -= scc->base_adr;

  if(adr == 0) 
  {
    if( val == 0x3F ) 
    {
      scc->mode = 0;
      scc->active = 1;
    }
    else if( val&0x80 && scc->type == SCC_ENHANCED)
    {
      scc->mode = 1;
      scc->active = 1;
    }
    else
    {
      scc->mode = 0;
      scc->active = 0;
    }
    return;
  }
  
  if(!scc->active||adr<0x800||0x8FF<adr) return;

  switch (scc->type) 
  {
  case SCC_STANDARD:
      write_standard (scc, adr, val);
    break;
  case SCC_ENHANCED:
    if(scc->mode)
      write_enhanced (scc, adr, val);
    else 
      write_standard (scc, adr, val);
  default:
    break;
  }

  return;
}

void
SCC_set_type (SCC * scc, uint32_t type)
{
  scc->type = type;
}

#undef SCC

/*
	Common Source Code Project
	MSX Series (experimental)

	Origin : emu2212
	modified by umaiboux
	Date   : 2016.04.xx-

	[ SCC ]
*/


SCC::SCC(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	emu2212 = SCC_new(3579545, 48000);
	volume_l = volume_r = 1024;
	set_device_name(_T("SCC"));
}

void SCC::write_data8(uint32_t addr, uint32_t data)
{
	SCC_write(emu2212, addr, data);
}

uint32_t SCC::read_data8(uint32_t addr)
{
	return SCC_read(emu2212, addr);
}

void SCC::initialize()
{
	SCC_set_type(emu2212, SCC_STANDARD);
}

void SCC::release()
{
	SCC_delete(emu2212);
}

void SCC::reset()
{
	SCC_reset(emu2212);
}

void SCC::mix(int32_t* buffer, int cnt)
{
	int i;
	for(i=0; i<cnt; i++) {
		int16_t c = (SCC_calc(emu2212)) << 1;
		buffer[i*2  ] += apply_volume(c, volume_l); // L
		buffer[i*2+1] += apply_volume(c, volume_r); // R
	}
}

void SCC::initialize_sound(int rate, int clock, int samples)
{
	this->release();
	emu2212 = SCC_new(clock, rate);
	this->reset();
	this->initialize();
}

void SCC::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

#define STATE_VERSION	1

bool SCC::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	save_load_state(state_fio, !loading);
	return true;
}

#if 0
void SCC::decl_state_scc()
{
	DECL_STATE_ENTRY_UINT32((emu2212->clk));
    DECL_STATE_ENTRY_UINT32((emu2212->rate));
	DECL_STATE_ENTRY_UINT32((emu2212->base_incr));
	DECL_STATE_ENTRY_UINT32((emu2212->quality));

	DECL_STATE_ENTRY_INT32((emu2212->out));
	DECL_STATE_ENTRY_INT32((emu2212->prev));
	DECL_STATE_ENTRY_INT32((emu2212->next));
	DECL_STATE_ENTRY_UINT32((emu2212->type));
	DECL_STATE_ENTRY_UINT32((emu2212->mode));
	DECL_STATE_ENTRY_UINT32((emu2212->active));
	DECL_STATE_ENTRY_UINT32((emu2212->base_adr));
	DECL_STATE_ENTRY_UINT32((emu2212->mask));
	
	DECL_STATE_ENTRY_UINT32((emu2212->realstep));
	DECL_STATE_ENTRY_UINT32((emu2212->scctime));
	DECL_STATE_ENTRY_UINT32((emu2212->sccstep));
	
	DECL_STATE_ENTRY_1D_ARRAY(emu2212->incr, 5);

	DECL_STATE_ENTRY_2D_ARRAY(emu2212->wave, 5, 32);

	DECL_STATE_ENTRY_1D_ARRAY(emu2212->count, 5);
	DECL_STATE_ENTRY_1D_ARRAY(emu2212->freq, 5);
	DECL_STATE_ENTRY_1D_ARRAY(emu2212->phase, 5);
	DECL_STATE_ENTRY_1D_ARRAY(emu2212->volume, 5);
	DECL_STATE_ENTRY_1D_ARRAY(emu2212->offset, 5);
	DECL_STATE_ENTRY_1D_ARRAY(emu2212->reg, 0x100-0xC0);

	DECL_STATE_ENTRY_INT32((emu2212->ch_enable));
	DECL_STATE_ENTRY_INT32((emu2212->ch_enable_next));

	DECL_STATE_ENTRY_INT32((emu2212->cycle_4bit));
	DECL_STATE_ENTRY_INT32((emu2212->cycle_8bit));
	DECL_STATE_ENTRY_INT32((emu2212->refresh));
	DECL_STATE_ENTRY_1D_ARRAY(emu2212->rotate, 5) ;
}
#endif

// ToDo: Dedicate endianness. 20181017 K.O.
void SCC::save_load_state(FILEIO* state_fio, bool is_save)
{
#define STATE_ENTRY(x) {&(x), sizeof(x)}
	typedef struct {
		void *address;
		size_t size;
	} t_state_table;
	t_state_table state_table[] = {
		STATE_ENTRY(*emu2212),
		{ NULL, 0 }
	};
	int i;
	for(i=0; state_table[i].size>0; i++) {
		if (is_save) {
			state_fio->Fwrite(state_table[i].address, state_table[i].size, 1);
		}
		else {
			state_fio->Fread(state_table[i].address, state_table[i].size, 1);
		}
	}
	return;
}

}
