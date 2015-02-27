/*
 * FM7 -> VM
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *   Feb 27, 2015 : Initial
 */

void VM::initialize(void)
{
	drec = new DATAREC;
	keyboard = new FM7_KEYBOARD;
	beep = new BEEP; 
	opn[0] = new YM2203; // OPN
	opn[1] = new YM2203; // WHG
	opn[2] = new YM2203; // THG
#if !defined(_FM77AV_VARIANTS)
	psg = new YM2203;
#else
	psg = NULL;
#endif
}


void VM::connect_bus(void)
{
	int i;
	
	/*
	 * CLASS CONSTRUCTION
	 *
	 * VM 
	 *  |-> MAINCPU -> MAINMEM -> MAINIO -> MAIN DEVICES
	 *  |             |        |      
	 *  | -> SUBCPU  -> SUBMEM  -> SUBIO -> SUB DEVICES
	 *  | -> DISPLAY
	 *  | -> KEYBOARD
	 *
	 *  MAINMEM can access SUBMEM/IO, when SUBCPU is halted.
	 *  MAINMEM and SUBMEM can access DISPLAY and KEYBOARD with exclusive.
	 *  MAINCPU can access MAINMEM.
	 *  SUBCPU  can access SUBMEM.
	 *  DISPLAY : R/W from MAINCPU and SUBCPU.
	 *  KEYBOARD : R/W
	 *
	 */                     
#if defined(_FM8)
	event->set_context_cpu(maincpu, 1095000);
	event->set_context_cpu(subcpu,   999000);
#else
	event->set_context_cpu(maincpu, 1794000);
	event->set_context_cpu(subcpu,  2000000);
#endif
   
	event->set_context_sound(beep);
	keyboard->set_context_mainio(mainio);
	keyboard->set_context_subio(subio);
  
	mainmem->set_context_submem(submem);
	maincpu->set_context_mem(mainmem);
   
	subcpu->set_context_mem(submem);
 
	mainio->set_context_maincpu(maincpu);
	mainio->set_context_display(display);
        mainio->set_context_kanji1(kanjiclass1);
        mainio->set_context_kanji2(kanjiclass2);
  
	display->set_context_mainio(mainio);
	display->set_context_cpu(subcpu);
	display->set_context_mem(submem);
        display->set_context_kanji1(kanjiclass1);
        display->set_context_kanji2(kanjiclass2);
   
	// Palette, VSYNC, HSYNC, Multi-page, display mode. 
	mainio->set_context_display(display);
	mainio->set_context_cmt(drec);

	//FDC
	fdc->set_context_irq(mainio, FM7_MAINIO_FDC_IRQ, 0xffffffff);
	fdc->set_context_drq(mainio, FM7_MAINIO_FDC_DRQ, 0xffffffff);
	mainio->set_context_fdc(fdc);
	// SOUND
	mainio->set_context_beep(beep);
	if(opn_connected) {
		opn[0]->set_context_irq(mainio, FM7_MAINIO_OPN_IRQ, 0xffffffff);
		opn[0]->set_context_port_a(mainio, SIG_FM7_OPN_JOY_A, 0xff);
		opn[0]->set_context_port_b(mainio, SIG_FM7_OPN_JOY_B, 0xff);
		mainio->set_context_opn(opn[0], 0);
	}
	if(whg_connected) {
		opn[1]->set_context_irq(mainio, FM7_MAINIO_WHG_IRQ, 0xffffffff);
		mainio->set_context_opn(opn[1], 1);
	}
   
	if(thg_connected) {
		opn[2]->set_context_irq(mainio, FM7_MAINIO_THG_IRQ, 0xffffffff);
		mainio->set_context_opn(opn[2], 2);
	}
   
	if((!opn_psg_77av) || (!opn_connected)) {
		event->set_context_sound(psg);
	}
	if(opn_connected) {
		event->set_context_sound(opn[0]);
	}
	if(whg_connected) {
    		event->set_context_sound(opn[1]);
	}
	if(thg_connected) {
		event->set_context_sound(opn[2]);
	}
#ifdef DATAREC_SOUND
	event->set_context_sound(drec);
#endif
}  


