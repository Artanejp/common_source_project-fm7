/*
 * FM7 -> VM
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *   Feb 27, 2015 : Initial
 */


void VM::VM(EMU* parent_emu) : emu(parent_emu)
{

	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	maincpu = new MC6809(this, emu);
	subcpu = new MC6809(this, emu);
#ifdef WITH_Z80
	z80cpu = new Z80(this, emu);
#endif
	// basic devices
	mainmem = new FM7_MAINMEM(this, emu);
	mainio  = new FM7_MAINIO(this, emu);
	
	submem  = new FM7_SUBMEM(this, emu);
	display = new DISPLAY(this, emu);

	// I/Os
	drec = new DATAREC(this, emu);
	keyboard = new FM7_KEYBOARD(this, emu);
	beep = new BEEP(this, emu);
	fdc  = new MB8877(this, emu);
	
	opn[0] = new YM2203(this, emu); // OPN
	opn[1] = new YM2203(this, emu); // WHG
	opn[2] = new YM2203(this, emu); // THG
#if !defined(_FM77AV_VARIANTS)
	psg = new YM2203(this, emu);
#else
	psg = NULL;
#endif
	kanjiclass1 = new KANJIROM(this, emu, false);
#ifdef CAPABLE_KANJI_CLASS2
	kanjiclass1 = new KANJIROM(this, emu, true);
#endif
  
}
void VM::initialize(void)
{
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
#ifdef WITH_Z80
	event->set_context_cpu(z80cpu,  4000000);
	z80cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
#endif
        
	keyboard->set_context_mainio(mainio);
	keyboard->set_context_subio(subio);
  
	mainmem->set_context_submem(submem);
	maincpu->set_context_mem(mainmem);
   
	subcpu->set_context_mem(submem);
 
	mainio->set_context_maincpu(maincpu);
	mainio->set_context_display(display);
        mainio->set_context_kanji1(kanjiclass1);
        mainio->set_context_kanji2(kanjiclass2);
	drec->set_context_out(mainio, 
  
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
	event->set_context_sound(beep);
	
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
	  if(psg != NULL) event->set_context_sound(psg);
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

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
	//update_dipswitch();
}


void VM::update_dipswitch()
{
	// bit0		0=High 1=Standard
	// bit2		0=5"2D 1=5"2HD
  //	io->set_iovalue_single_r(0x1ff0, (config.monitor_type & 1) | ((config.drive_type & 1) << 2));
}

