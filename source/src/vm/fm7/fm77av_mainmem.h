/*
 * Main memory emulation of FM77AV.
 *  MEMORY -> FM7_MAINMEM -> FM77AV_MAINMEM
 *  Author: K.Ohta <whatisthis.sowhat _at_ gmail.com> 
 *  History Jan 02, 2015 : Initial.
 */

#ifndef _FM77AV_MAINMEM_H_
#odefine _FM77AV_MAINMEM_H_
#include "fm7_mainmem.h"


class FM77AV_MAINMEM : public FM7_MAINMEM;
{
   
 protected:
  uint8 mainmem_page0[0x10000];
  uint8 mainmem_jcard[0x20000];
  uint8 mainmem_initrom[0x2000];
  uint8 *patched_bootrom_p1;
  uint8 *patched_bootrom_p2;
  bool diag_load_initrom = false;

  virtual void patch_bootrom(void) {
    uint8 *p = &initiator_rom[0x0b0e];
    for(i = 0; i < 6; i++) *p++ = 0xff;

  /* Patch initiator_rom */
    if(patched_bootrom_p1 != NULL) {
      patched_bootrom_p1[0] = 0x21; // BRA->BRN
      goto _boot_2;
    }
    p = initiator_rom;
    for(i = 0; i < 0x0b00; i++) {
      if((p[0] == 0x20) && (p[1] == 0xd7)) {
	p[0] = 0x21; // BRA->BRN
	patched_bootrom_p1 = p;
      }
      p++;
    }
  _boot_2:  
    if(patched_bootrom_p2 != NULL) {
      patched_bootrom_p2[1] = 0xfe; // 5000 -> fe00
      patched_bootrom_p2[2] = 0x00;
      goto _boot_3;
    }
    p = initiator_rom;
    for(i = 0; i < 0x0b00; i++) {
      // 7e 5000 : jmp $5000 -> jmp $fe00
      if(p[0] == 0x7e) {
	if((p[1] == 0x50) && (p[2] == 0x00)) {
	  p[1] = 0xfe;
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
  FM77AV_MAINMEM(VM* parent_vm, EMU* parent_emu) : FM7_MAINMEM(parent_vm, parent_emu)
  {
	int i;
	i = FM7_MAINMEM_AV_PAGE0;
	memset(mainmem_page0, 0x00, sizeof(mainmem_page0));
	read_table[i].dev = write_table[i].dev = NULL;
	read_table[i].memory = write_table[i].memory = mainmem_page0;
	
	i = FM7_MAINMEM_AV_JCARD;
	memset(mainmem_jcard, 0x00, sizeof(mainmem_jcard));
	read_table[i].dev = write_table[i].dev = NULL;
	read_table[i].memory = write_table[i].memory = mainmem_jcard;

	i = FM7_MAINMEM_AV_INITROM;
	memset(mainmem_initrom, 0x00, sizeof(mainmem_initrom));
	if(read_bios("INITIATE.ROM", mainmem_initrom, 0x2000) == 0x2000) diag_load_initrom = true;
	read_table[i].dev = write_table[i].dev = NULL;
	read_table[i].memory = write_table[i].memory = mainmem_initrom;

  }

  ~FM77AV_MAINMEM(void) {
  }

  void mem_reset(); 
  bool get_loadstat_initrom(void){
    return diag_loadstat_initrom;
  }
}

#endif
