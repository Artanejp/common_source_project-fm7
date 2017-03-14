/*
	Common Source Code Project
	MSX Series (experimental)

	Origin : openmsx-0.12.0.tar.gz
		src\memory\RomTypes.hh
		src\memory\RomFactory.cc
	modified by umaiboux
	Date   : 2016.03.xx-

	[ ROM type enum & auto-detect ]
*/

#ifndef _ROMTYPE1_H_

typedef uint16_t word;

enum RomType {
	// Order doesn't matter (I sorted them alphabetically)
	ROM_ARC,
	ROM_ASCII8,
	ROM_ASCII8_2,
	ROM_ASCII8_8,
	ROM_ASCII16,
	ROM_ASCII16_2,
	ROM_ASCII16_8,
	ROM_CROSS_BLAIM,
	ROM_DOOLY,
	ROM_DRAM,
	ROM_FSA1FM1,
	ROM_FSA1FM2,
	ROM_GAME_MASTER2,
	ROM_GENERIC_8KB,
	ROM_GENERIC_16KB,
	ROM_HALNOTE,
	ROM_HAMARAJANIGHT,
	ROM_HARRY_FOX,
	ROM_HOLY_QURAN,
	ROM_HOLY_QURAN2,
	ROM_KBDMASTER,
	ROM_KOEI_8,
	ROM_KOEI_32,
	ROM_KONAMI,
	ROM_KONAMI_SCC,
	ROM_MAJUTSUSHI,
	ROM_MANBOW2,
	ROM_MANBOW2_2,
	ROM_MATRAINK,
	ROM_MEGAFLASHROMSCC,
	ROM_MEGAFLASHROMSCCPLUS,
	ROM_MEGAFLASHROMSCCPLUSSD,
	ROM_MIRRORED,
	ROM_MITSUBISHIMLTS2,
	ROM_MSXDOS2,
	ROM_MSXTRA,
	ROM_MULTIROM,
	ROM_NATIONAL,
	ROM_NETTOU_YAKYUU,
	ROM_NORMAL,
	ROM_PADIAL8,
	ROM_PADIAL16,
	ROM_PANASONIC,
	ROM_PLAYBALL,
	ROM_R_TYPE,
	ROM_SUPERLODERUNNER,
	ROM_SUPERSWANGI,
	ROM_SYNTHESIZER,
	ROM_WIZARDRY,
	ROM_ZEMINA80IN1,
	ROM_ZEMINA90IN1,
	ROM_ZEMINA126IN1,

	ROM_END_OF_UNORDERED_LIST, // not an actual romtype

	// For these the numeric value does matter
	ROM_PAGE0        = 128 + 1,  // value of lower 4 bits matters
	ROM_PAGE1        = 128 + 2,
	ROM_PAGE01       = 128 + 3,
	ROM_PAGE2        = 128 + 4,
	ROM_PAGE12       = 128 + 6,
	ROM_PAGE012      = 128 + 7,
	ROM_PAGE3        = 128 + 8,
	ROM_PAGE23       = 128 + 12,
	ROM_PAGE123      = 128 + 14,
	ROM_PAGE0123     = 128 + 15,
	ROM_MIRRORED0000 = 144 + 0, // value of lower 3 bits matters
	ROM_MIRRORED4000 = 144 + 2,
	ROM_MIRRORED8000 = 144 + 4,
	ROM_MIRROREDC000 = 144 + 6,
	ROM_NORMAL0000   = 152 + 0, // value of lower 3 bits matters
	ROM_NORMAL4000   = 152 + 2,
	ROM_NORMAL8000   = 152 + 4,
	ROM_NORMALC000   = 152 + 6,

	ROM_UNKNOWN      = 256,
	ROM_ALIAS        = 512, // no other enum value can have this bit set
};

static RomType guessRomType(uint8_t *rom, int size)
{
	if (size == 0) {
		return ROM_NORMAL;
	}
	const uint8_t* data = &rom[0];

	if (size < 0x10000) {
		if ((size <= 0x4000) &&
		           (data[0] == 'A') && (data[1] == 'B')) {
			word initAddr = data[2] + 256 * data[3];
			word textAddr = data[8] + 256 * data[9];
			if ((textAddr & 0xC000) == 0x8000) {
				if ((initAddr == 0) ||
				    (((initAddr & 0xC000) == 0x8000) &&
				     (data[initAddr & (size - 1)] == 0xC9))) {
					return ROM_PAGE2;
				}
			}
		}
		// not correct for Konami-DAC, but does this really need
		// to be correct for _every_ rom?
		return ROM_MIRRORED;
	} else if (size == 0x10000 && !((data[0] == 'A') && (data[1] == 'B'))) {
		// 64 kB ROMs can be plain or memory mapped...
		// check here for plain, if not, try the auto detection
		// (thanks for the hint, hap)
		return ROM_MIRRORED;
	} else {
		//  GameCartridges do their bankswitching by using the Z80
		//  instruction ld(nn),a in the middle of program code. The
		//  adress nn depends upon the GameCartridge mappertype used.
		//  To guess which mapper it is, we will look how much writes
		//  with this instruction to the mapper-registers-addresses
		//  occur.

		unsigned typeGuess[ROM_END_OF_UNORDERED_LIST] = {}; // 0-initialized
		for (int i = 0; i < size - 3; ++i) {
			if (data[i] == 0x32) {
				word value = data[i + 1] + (data[i + 2] << 8);
				switch (value) {
				case 0x5000:
				case 0x9000:
				case 0xb000:
					typeGuess[ROM_KONAMI_SCC]++;
					break;
				case 0x4000:
					typeGuess[ROM_KONAMI]++;
					break;
				case 0x8000:
				case 0xa000:
					typeGuess[ROM_KONAMI]++;
					break;
				case 0x6800:
				case 0x7800:
					typeGuess[ROM_ASCII8]++;
					break;
				case 0x6000:
					typeGuess[ROM_KONAMI]++;
					typeGuess[ROM_ASCII8]++;
					typeGuess[ROM_ASCII16]++;
					break;
				case 0x7000:
					typeGuess[ROM_KONAMI_SCC]++;
					typeGuess[ROM_ASCII8]++;
					typeGuess[ROM_ASCII16]++;
					break;
				case 0x77ff:
					typeGuess[ROM_ASCII16]++;
					break;
				}
			}
		}
		if (typeGuess[ROM_ASCII8]) typeGuess[ROM_ASCII8]--; // -1 -> max_int
		RomType type = ROM_GENERIC_8KB;
		for (int i = 0; i < ROM_END_OF_UNORDERED_LIST; ++i) {
			// debug: fprintf(stderr, "%d: %d\n", i, typeGuess[i]);
			if (typeGuess[i] && (typeGuess[i] >= typeGuess[type])) {
				type = static_cast<RomType>(i);
			}
		}
		// in case of doubt we go for type ROM_GENERIC_8KB
		// in case of even type ROM_ASCII16 and ROM_ASCII8 we would
		// prefer ROM_ASCII16 but we would still prefer ROM_GENERIC_8KB
		// above ROM_ASCII8 or ROM_ASCII16
		if ((type == ROM_ASCII16) &&
		    (typeGuess[ROM_GENERIC_8KB] == typeGuess[ROM_ASCII16])) {
			type = ROM_GENERIC_8KB;
		}
		return type;
	}
}

#define _ROMTYPE1_H_

#endif

