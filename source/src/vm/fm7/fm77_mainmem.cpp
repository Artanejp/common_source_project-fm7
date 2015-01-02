/*
 * 
 */

int FM77_MAINMEM::getbank(uint32 addr, uint32 *realaddr)
{
   if(realaddr == NULL) return -1; // Not effect.
   
   addr = addr & 0xffff;
   if(window != NULL) {
     if(window->isenabled() && (addr >= 0x7c00) && (addr < 0x8000)) { // Window open
       if(use_extram) {
	 *realaddr = ((addr + window->getwindow() * 256) & 0xffff) + 0x20000;
	 return FM77_MAINMEM_77EXTRAM; // FM77
       }
       mainio->wait();
       mmr_beforeaccess = false; // Really?
       *realaddr = 0;
       return -1;
     }
   }
   if(mmr != NULL) {
      int  mmr_page;
      uint32 mmr_offset;
      if(!mmr->isenabled()) goto _fm7_0;
      mainio->wait(); // 1 wait per 3 access.
      mmr_page = mmr->getpage(addr);
      
      if(addr < 0xfc00) { // MMR Banked
	 mmr_offset = addr & 0x0fff;
	 if(mmr_page < 0x30) {
	       if(use_extram) {
		 *realaddr = mmr_offset + mmr_page * 0x1000;
		 return FM7_MAINMEM_77EXTRAM;
	       }
	       *realaddr = 0;
	       return -1;
	 } else if((mmr_page & 0x30) == 0x30) {
	       if(mmr_page == 0x3f) { // $3f000 - $3ffff
		    if(mmr_offset < 0x0c00) {
		       addr = mmr_offset + 0xf000;
		       goto _fm7_0;
		    }
		    if((mmr_offset < 0x0e00)) {
		       *realaddr = mmr_offset - 0x0c00;
		       if(mainio->get_rommode_fd0f() == true) {
			  return FM7_MAINMEN_77_NULLRAM;
		       }
		       return FM7_MAINMEM_77_SHADOWRAM;
		    }
		    addr = mmr_offset + 0xf000;
		    goto _fm7_0; // 
	       }
	       // $30000 - $3efff
	       addr = mmr_offset + (mmr_page & 0x0f) * 0x1000;
	       goto _fm7_0;
	 }
	 *realaddr = 0;
	 return -1; // Illegal
      }
      goto fm7_0; // With I/O Wait. 
   }

_fm7_0:
   if((addr >= 0xfe00) && (addr < 0xfffe)) {
	*realaddr = addr - 0xfe00;
	if(addr < 0xffe0) mainio->wait();
	if(mainio->get_boot_romram() != true) {
	  return FM7_MAINMEM_BOOTROM_RAM;
	}
   }
   return FM7_MAINMEM::getbank(addr, realaddr);
}

