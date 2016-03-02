/*
	SEGA GAME GEAR Emulator 'yaGAME GEAR'

	Author : tanam
	Date   : 2013.08.24-

	[ system port ]
*/

#include "system.h"
#include "keyboard.h"
#include "../event.h"
#include "../z80.h"

void SYSTEM::write_io8(uint32_t addr, uint32_t data)
{
	if ((addr & 0x000000ff)==0x80) {
		tenkey=true;
	}
	if ((addr & 0x000000ff)==0xc0) {
		tenkey=false;
	}
	return;
}
uint32_t SYSTEM::read_io8(uint32_t addr)
{
	// Controller 1
	if ((addr & 0x000000ff)==0xfc) {
		const uint8_t *ten=emu->get_key_buffer();
		const uint32_t *joy=emu->get_joy_buffer();
		uint8_t button=0xf0;
		if (joy[0] & 0x10) button=0xb0;				// B2
	//	Bit 7:	0iŠg’£’[q‚P‚Oƒsƒ“‚Ì“à—ej
	//	Bit 6:	1=Off/0=On	ƒgƒŠƒK[‚Pi¶j
	//	Bit 5:	1i‚i‚n‚x’[q‚Vƒsƒ“@–¢g—pj
	//	Bit 4:	1i‚i‚n‚x’[q‚Xƒsƒ“@–¢g—pj
	//	Bit 3:	1=Off/0=On	LEFT
	//	Bit 2:	1=Off/0=On	DOWN
	//	Bit 1:	1=Off/0=On	RIGHT
	//	Bit 0:	1=Off/0=On	UP
		if (!tenkey) {
			uint32_t joystick=0xff;
			if (joy[0] & 0x01) joystick &= 0xfe;	// U
			if (joy[0] & 0x02) joystick &= 0xfb;	// D
			if (joy[0] & 0x04) joystick &= 0xf7;	// L
			if (joy[0] & 0x08) joystick &= 0xfd;	// R
			if (joy[0] & 0x20) joystick &= 0xbf;	// B1
			return joystick;
		}
	//	ƒL[ƒpƒbƒhƒ}ƒgƒŠƒbƒNƒXi‚j‚l‚R|‚O‚Ì‚S‚a‚‰‚”‚È‚Ì‚Å‚O|‚ej
	// 0=–¢g—p	1='8'	2='4'	3='5'
	// 4=–¢g—p	5='7'	6='#'	7='2'
	// 8=–¢g—p	9='*'	A='0'	B='9'
	// C='3'	D='1'	E='6'	F=‰½‚à‰Ÿ‚³‚ê‚Ä‚¢‚È‚¢
		if (ten[0x31] & 0x80)
			return (button | 0x0d); // 1
		if (ten[0x32] & 0x80)
			return (button | 0x07); // 2
		if (ten[0x33] & 0x80)
			return (button | 0x0c); // 3
		if (ten[0x34] & 0x80)
			return (button | 0x02); // 4
		if (ten[0x35] & 0x80)
			return (button | 0x03); // 5
		if (ten[0x36] & 0x80)
			return (button | 0x0e); // 6
		if (ten[0x37] & 0x80)
			return (button | 0x05); // 7
		if (ten[0x38] & 0x80)
			return (button | 0x01); // 8
		if (ten[0x39] & 0x80)
			return (button | 0x0b); // 9
		if (ten[0xe2] & 0x80)
			return (button | 0x06); // #
		if (ten[0xde] & 0x80)
			return (button | 0x08); // *
		return (button | 0x0f);
	}
	if (((KEYBOARD *)d_key)->is_start()) {
		return 0x80; /// 1000 0000
	}
	return 0x00;
}
