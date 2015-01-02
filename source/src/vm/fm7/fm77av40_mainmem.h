/*
 * Main memory emulation of FM77AV40.
 *  MEMORY -> FM7_MAINMEM -> FM77AV_MAINMEM -> FM77AV40_MAINMEM
 *  Author: K.Ohta <whatisthis.sowhat _at_ gmail.com> 
 *  History Jan 02, 2015 : Initial.
 */

#ifndef _FM77AV40_MAINMEM_H_
#odefine _FM77AV40_MAINMEM_H_
#include "fm77av_mainmem.h"

class FM77AV40_MAINMEM : public FM77AV_MAINMEM;
{
   
 protected:
  uint8 mainmem_extram786k[0xc0000];
  bool use_extram = false;

  void patch_bootrom(void) {
    uint8 *p = &mainmem_initrom[0x0b0e];
    char  *s = "401Ma.";
  
  for(i = 0; i < 6; i++) *p++ = *s++;

  /* Patch initiator_rom */
  if(patched_bootrom_p1 != NULL) {
    patched_bootrom_p1[0] = 0x20; // BRN->BRA
    goto _boot_2;
  }
  p = mainmem_initrom;
  for(i = 0; i < 0x0b00; i++) {
    if((p[0] == 0x20) && (p[1] == 0xd7)) {
      p[0] = 0x20; // BRA->BRN
      patched_bootrom_p1 = p;
    }
    p++;
  }
 _boot_2:  
  if(patched_bootrom_p2 != NULL) {
    patched_bootrom_p2[1] = 0x50; // fe00 -> 5000
    patched_bootrom_p2[2] = 0x00;
    goto _boot_3;
  }
  p = mainmem_initrom;
  for(i = 0; i < 0x0b00; i++) {
    // 7e 5000 : jmp $5000 -> jmp $fe00
    if(p[0] == 0x7e) {
      if((p[1] == 0xfe) && (p[2] == 0x00)) {
	p[1] = 0x50;
	p[2] = 0x00;
	patched_bootrom_p2 = p;
      }
    }
    p++;
  }
_boot_3:
  }
  
  int getbank(uint32 addr, uint32 *realaddr);
 public:
  void reset(void);
  FM77AV40_MAINMEM(VM* parent_vm, EMU* parent_emu) : FM77AV_MAINMEM(parent_vm, parent_emu)
  {
	int i;
	i = FM7_MAINMEM_AV_EXTRAM768K;
	memset(mainmem_mainmem_extram786k, 0x00, sizeof(mainmem_extram768k));
	read_table[i].dev = write_table[i].dev = NULL;
	read_table[i].memory = write_table[i].memory = mainmem_extram768k;
  }

  ~FM77AV40_MAINMEM(void) {
  }

  void get_stat_extram(void) {
    return use_extram;
  }
  
  void connect_extram(bool mode){
    if(mode == false){
      if(use_extram != false){
	use_extram = false;
	reset();
      }
    } else {
      if(use_extram == false) {
	use_extram = true;
	reset();
      }
    }
  }
  
}

#endif
