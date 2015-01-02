#ifndef _FM7_DIPSW_H_
#define _FM7_DIPSW_H_

#include "../device.h"

class DEVICE;
class FM7_DIPSW : public DEVICE
{
 protected:
  uint8 boot_sw = 0;
  bool cycle_steal = false;
  bool fast_clock = false;
 public:
 FM7_DIPSW(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
    {
    }
 ~FM7_DIPSW() {}

 uint32 get_boot_mode(void) {
 case (boot_sw & 3) {
 case 0:
   return FM7_BOOTMODE_BASIC;
   break;
 case 1:
   return FM7_BOOTMODE_DOS;
   break;
 case 2:
   return FM7_BOOTMODE_ROM3;
   break;
 case 3:
   return FM7_BOOTMODE_ROM4;
   break;
 }
 }

 void set_boot_mode(uint32 mode){
   boot_sw = mode & 3;
 }
 bool get_cyclesteal(void){
   return cycle_steal;
 }
 void set_cyclesteal(bool mode){
   cycle_steal =  mode;
 }
 
 bool get_fastclock(void){
   return fast_clock;
 }
 void set_fastclock(bool mode){
   fast_clock =  mode;
 }
}
#endif
