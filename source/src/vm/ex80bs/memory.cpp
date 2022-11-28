/*
	TOSHIBA EX-80 Emulator 'eEX-80'

	Author : Takeda.Toshiya
	Date   : 2015.12.14-

	[ memory ]
*/

#include "memory.h"
#include "../i8080.h"
#include "../i8255.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 10, eb = (e) >> 10; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x400 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x400 * (i - sb); \
		} \
	} \
}

static const int table[256] = {
	  -1,	// 00 
	  -1,	// 01 
	  -1,	// 02 
	  -1,	// 03 
	  -1,	// 04 
	  -1,	// 05 
	  -1,	// 06 
	  -1,	// 07 
	0x08,	// 08 Back	BS
	  -1,	// 09 Tab
	  -1,	// 0A 
	  -1,	// 0B 
	  -1,	// 0C 
	0x0d,	// 0D Enter	CR
	  -1,	// 0E 
	  -1,	// 0F 
	  -1,	// 10 Shift
	  -1,	// 11 Ctrl
	  -1,	// 12 Alt
	  -1,	// 13 Pause
	  -1,	// 14 Caps
	  -1,	// 15 Kana
	  -1,	// 16 
	  -1,	// 17 
	  -1,	// 18 
	  -1,	// 19 Kanji
	  -1,	// 1A 
	0x03,	// 1B Escape	ETX
	  -1,	// 1C Convert
	  -1,	// 1D NonConv
	  -1,	// 1E 
	  -1,	// 1F 
	0x20,	// 20 Space	SPACE
	0x18,	// 21 PgUp	CAN
	0x0a,	// 22 PgDwn	LF
	  -1,	// 23 End
	  -1,	// 24 Home
	  -1,	// 25 Left
	  -1,	// 26 Up
	  -1,	// 27 Right
	  -1,	// 28 Down
	  -1,	// 29 
	  -1,	// 2A 
	  -1,	// 2B 
	  -1,	// 2C 
	  -1,	// 2D Ins
	0x7f,	// 2E Del	DEL
	  -1,	// 2F 
	0x30,	// 30 0		0
	0x31,	// 31 1		1
	0x32,	// 32 2		2
	0x33,	// 33 3		3
	0x34,	// 34 4		4
	0x35,	// 35 5		5
	0x36,	// 36 6		6
	0x37,	// 37 7		7
	0x38,	// 38 8		8
	0x39,	// 39 9		9
	  -1,	// 3A 
	  -1,	// 3B 
	  -1,	// 3C 
	  -1,	// 3D 
	  -1,	// 3E 
	  -1,	// 3F 
	  -1,	// 40 
	0x41,	// 41 A		A
	0x42,	// 42 B		B
	0x43,	// 43 C		C
	0x44,	// 44 D		D
	0x45,	// 45 E		E
	0x46,	// 46 F		F
	0x47,	// 47 G		G
	0x48,	// 48 H		H
	0x49,	// 49 I		I
	0x4a,	// 4A J		J
	0x4b,	// 4B K		K
	0x4c,	// 4C L		L
	0x4d,	// 4D M		M
	0x4e,	// 4E N		N
	0x4f,	// 4F O		O
	0x50,	// 50 P		P
	0x51,	// 51 Q		Q
	0x52,	// 52 R		R
	0x53,	// 53 S		S
	0x54,	// 54 T		T
	0x55,	// 55 U		U
	0x56,	// 56 V		V
	0x57,	// 57 W		W
	0x58,	// 58 X		X
	0x59,	// 59 Y		Y
	0x5a,	// 5A Z		Z
	  -1,	// 5B 
	  -1,	// 5C 
	  -1,	// 5D 
	  -1,	// 5E 
	  -1,	// 5F 
	0x30,	// 60 NUM 0	0
	0x31,	// 61 NUM 1	1
	0x32,	// 62 NUM 2	2
	0x33,	// 63 NUM 3	3
	0x34,	// 64 NUM 4	4
	0x35,	// 65 NUM 5	5
	0x36,	// 66 NUM 6	6
	0x37,	// 67 NUM 7	7
	0x38,	// 68 NUM 8	8
	0x39,	// 69 NUM 9	9
	0x2a,	// 6A NUM *	*
	0x2b,	// 6B NUM +	+
	0x0d,	// 6C NUM Ent	CR
	0x2d,	// 6D NUM -	-
	0x2e,	// 6E NUM .	.
	0x2f,	// 6F NUM /	/
	  -1,	// 70 F1
	  -1,	// 71 F2
	  -1,	// 72 F3
	  -1,	// 73 F4
	  -1,	// 74 F5
	  -1,	// 75 F6
	  -1,	// 76 F7
	  -1,	// 77 F8
	  -1,	// 78 F9
	  -1,	// 79 F10
	  -1,	// 7A F11
	  -1,	// 7B F12
	  -1,	// 7C F13
	  -1,	// 7D F14
	  -1,	// 7E F15
	  -1,	// 7F F16
	  -1,	// 80 F17
	  -1,	// 81 F18
	  -1,	// 82 F19
	  -1,	// 83 F20
	  -1,	// 84 F21
	  -1,	// 85 F22
	  -1,	// 86 F23
	  -1,	// 87 F24
	  -1,	// 88 
	  -1,	// 89 
	  -1,	// 8A 
	  -1,	// 8B 
	  -1,	// 8C 
	  -1,	// 8D 
	  -1,	// 8E 
	  -1,	// 8F 
	  -1,	// 90 
	  -1,	// 91 ScrLk
	  -1,	// 92 
	  -1,	// 93 
	  -1,	// 94 
	  -1,	// 95 
	  -1,	// 96 
	  -1,	// 97 
	  -1,	// 98 
	  -1,	// 99 
	  -1,	// 9A 
	  -1,	// 9B 
	  -1,	// 9C 
	  -1,	// 9D 
	  -1,	// 9E 
	  -1,	// 9F 
	  -1,	// A0 L Shift
	  -1,	// A1 R Shift
	  -1,	// A2 L Ctrl
	  -1,	// A3 R Ctrl
	  -1,	// A4 L Alt
	  -1,	// A5 R Alt
	  -1,	// A6 
	  -1,	// A7 
	  -1,	// A8 
	  -1,	// A9 
	  -1,	// AA 
	  -1,	// AB 
	  -1,	// AC 
	  -1,	// AD 
	  -1,	// AE 
	  -1,	// AF 
	  -1,	// B0 
	  -1,	// B1 
	  -1,	// B2 
	  -1,	// B3 
	  -1,	// B4 
	  -1,	// B5 
	  -1,	// B6 
	  -1,	// B7 
	  -1,	// B8 
	  -1,	// B9 
	0x3a,	// BA :		:
	0x3b,	// BB ;		;
	0x2c,	// BC ,		,
	0x2d,	// BD -		-
	0x2e,	// BE .		.
	0x2f,	// BF /		/
	0x40,	// C0 @		@
	  -1,	// C1 
	  -1,	// C2 
	  -1,	// C3 
	  -1,	// C4 
	  -1,	// C5 
	  -1,	// C6 
	  -1,	// C7 
	  -1,	// C8 
	  -1,	// C9 
	  -1,	// CA 
	  -1,	// CB 
	  -1,	// CC 
	  -1,	// CD 
	  -1,	// CE 
	  -1,	// CF 
	  -1,	// D0 
	  -1,	// D1 
	  -1,	// D2 
	  -1,	// D3 
	  -1,	// D4 
	  -1,	// D5 
	  -1,	// D6 
	  -1,	// D7 
	  -1,	// D8 
	  -1,	// D9 
	  -1,	// DA 
	0x5b,	// DB [		[
	0x5c,	// DC Yen	Yen
	0x5d,	// DD ]		]
	0x5e,	// DE ^		^
	  -1,	// DF 
	  -1,	// E0 
	  -1,	// E1 
	  -1,	// E2 _
	  -1,	// E3 
	  -1,	// E4 
	  -1,	// E5 
	  -1,	// E6 
	  -1,	// E7 
	  -1,	// E8 
	  -1,	// E9 
	  -1,	// EA 
	  -1,	// EB 
	  -1,	// EC 
	  -1,	// ED 
	  -1,	// EE 
	  -1,	// EF 
	  -1,	// F0 
	  -1,	// F1 
	  -1,	// F2 
	  -1,	// F3 
	  -1,	// F4 
	  -1,	// F5 
	  -1,	// F6 
	  -1,	// F7 
	  -1,	// F8 
	  -1,	// F9 
	  -1,	// FA 
	  -1,	// FB 
	  -1,	// FC 
	  -1,	// FD 
	  -1,	// FE 
	  -1,	// FF 
};

static const int table_shift[256] = {
	  -1,	// 00 
	  -1,	// 01 
	  -1,	// 02 
	  -1,	// 03 
	  -1,	// 04 
	  -1,	// 05 
	  -1,	// 06 
	  -1,	// 07 
	0x08,	// 08 Back	BS
	  -1,	// 09 Tab
	  -1,	// 0A 
	  -1,	// 0B 
	  -1,	// 0C 
	0x0d,	// 0D Enter	CR
	  -1,	// 0E 
	  -1,	// 0F 
	  -1,	// 10 Shift
	  -1,	// 11 Ctrl
	  -1,	// 12 Alt
	  -1,	// 13 Pause
	  -1,	// 14 Caps
	  -1,	// 15 Kana
	  -1,	// 16 
	  -1,	// 17 
	  -1,	// 18 
	  -1,	// 19 Kanji
	  -1,	// 1A 
	0x03,	// 1B Escape	ETX
	  -1,	// 1C Convert
	  -1,	// 1D NonConv
	  -1,	// 1E 
	  -1,	// 1F 
	0x20,	// 20 Space	SPACE
	0x18,	// 21 PgUp	CAN
	0x0a,	// 22 PgDwn	LF
	  -1,	// 23 End
	  -1,	// 24 Home
	  -1,	// 25 Left
	  -1,	// 26 Up
	  -1,	// 27 Right
	  -1,	// 28 Down
	  -1,	// 29 
	  -1,	// 2A 
	  -1,	// 2B 
	  -1,	// 2C 
	  -1,	// 2D Ins
	0x7f,	// 2E Del	DEL
	  -1,	// 2F 
	  -1,	// 30 0
	0x21,	// 31 1		!
	0x22,	// 32 2		"
	0x23,	// 33 3		#
	0x24,	// 34 4		$
	0x25,	// 35 5		%
	0x26,	// 36 6		&
	0x27,	// 37 7		'
	0x28,	// 38 8		(
	0x29,	// 39 9		)
	  -1,	// 3A 
	  -1,	// 3B 
	  -1,	// 3C 
	  -1,	// 3D 
	  -1,	// 3E 
	  -1,	// 3F 
	  -1,	// 40 
	  -1,	// 41 A
	  -1,	// 42 B
	  -1,	// 43 C
	  -1,	// 44 D
	  -1,	// 45 E
	  -1,	// 46 F
	  -1,	// 47 G
	  -1,	// 48 H
	  -1,	// 49 I
	  -1,	// 4A J
	  -1,	// 4B K
	  -1,	// 4C L
	  -1,	// 4D M
	  -1,	// 4E N
	  -1,	// 4F O
	  -1,	// 50 P
	  -1,	// 51 Q
	  -1,	// 52 R
	  -1,	// 53 S
	  -1,	// 54 T
	  -1,	// 55 U
	  -1,	// 56 V
	  -1,	// 57 W
	  -1,	// 58 X
	  -1,	// 59 Y
	  -1,	// 5A Z
	  -1,	// 5B 
	  -1,	// 5C 
	  -1,	// 5D 
	  -1,	// 5E 
	  -1,	// 5F 
	0x30,	// 60 NUM 0	0
	0x31,	// 61 NUM 1	1
	0x32,	// 62 NUM 2	2
	0x33,	// 63 NUM 3	3
	0x34,	// 64 NUM 4	4
	0x35,	// 65 NUM 5	5
	0x36,	// 66 NUM 6	6
	0x37,	// 67 NUM 7	7
	0x38,	// 68 NUM 8	8
	0x39,	// 69 NUM 9	9
	0x2a,	// 6A NUM *	*
	0x2b,	// 6B NUM +	+
	0x0d,	// 6C NUM Ent	CR
	0x2d,	// 6D NUM -	-
	0x2e,	// 6E NUM .	.
	0x2f,	// 6F NUM /	/
	  -1,	// 70 F1
	  -1,	// 71 F2
	  -1,	// 72 F3
	  -1,	// 73 F4
	  -1,	// 74 F5
	  -1,	// 75 F6
	  -1,	// 76 F7
	  -1,	// 77 F8
	  -1,	// 78 F9
	  -1,	// 79 F10
	  -1,	// 7A F11
	  -1,	// 7B F12
	  -1,	// 7C F13
	  -1,	// 7D F14
	  -1,	// 7E F15
	  -1,	// 7F F16
	  -1,	// 80 F17
	  -1,	// 81 F18
	  -1,	// 82 F19
	  -1,	// 83 F20
	  -1,	// 84 F21
	  -1,	// 85 F22
	  -1,	// 86 F23
	  -1,	// 87 F24
	  -1,	// 88 
	  -1,	// 89 
	  -1,	// 8A 
	  -1,	// 8B 
	  -1,	// 8C 
	  -1,	// 8D 
	  -1,	// 8E 
	  -1,	// 8F 
	  -1,	// 90 
	  -1,	// 91 ScrLk
	  -1,	// 92 
	  -1,	// 93 
	  -1,	// 94 
	  -1,	// 95 
	  -1,	// 96 
	  -1,	// 97 
	  -1,	// 98 
	  -1,	// 99 
	  -1,	// 9A 
	  -1,	// 9B 
	  -1,	// 9C 
	  -1,	// 9D 
	  -1,	// 9E 
	  -1,	// 9F 
	  -1,	// A0 L Shift
	  -1,	// A1 R Shift
	  -1,	// A2 L Ctrl
	  -1,	// A3 R Ctrl
	  -1,	// A4 L Alt
	  -1,	// A5 R Alt
	  -1,	// A6 
	  -1,	// A7 
	  -1,	// A8 
	  -1,	// A9 
	  -1,	// AA 
	  -1,	// AB 
	  -1,	// AC 
	  -1,	// AD 
	  -1,	// AE 
	  -1,	// AF 
	  -1,	// B0 
	  -1,	// B1 
	  -1,	// B2 
	  -1,	// B3 
	  -1,	// B4 
	  -1,	// B5 
	  -1,	// B6 
	  -1,	// B7 
	  -1,	// B8 
	  -1,	// B9 
	0x2a,	// BA :		*
	0x2b,	// BB ;		+
	0x3c,	// BC ,		<
	0x3d,	// BD -		=
	0x3e,	// BE .		>
	0x3f,	// BF /		?
	0x60,	// C0 @		`
	  -1,	// C1 
	  -1,	// C2 
	  -1,	// C3 
	  -1,	// C4 
	  -1,	// C5 
	  -1,	// C6 
	  -1,	// C7 
	  -1,	// C8 
	  -1,	// C9 
	  -1,	// CA 
	  -1,	// CB 
	  -1,	// CC 
	  -1,	// CD 
	  -1,	// CE 
	  -1,	// CF 
	  -1,	// D0 
	  -1,	// D1 
	  -1,	// D2 
	  -1,	// D3 
	  -1,	// D4 
	  -1,	// D5 
	  -1,	// D6 
	  -1,	// D7 
	  -1,	// D8 
	  -1,	// D9 
	  -1,	// DA 
	0x7b,	// DB [		{
	0x7c,	// DC Yen	|
	0x7d,	// DD ]		}
	0x7e,	// DE ^		~
	  -1,	// DF 
	  -1,	// E0 
	  -1,	// E1 
	  -1,	// E2 _
	  -1,	// E3 
	  -1,	// E4 
	  -1,	// E5 
	  -1,	// E6 
	  -1,	// E7 
	  -1,	// E8 
	  -1,	// E9 
	  -1,	// EA 
	  -1,	// EB 
	  -1,	// EC 
	  -1,	// ED 
	  -1,	// EE 
	  -1,	// EF 
	  -1,	// F0 
	  -1,	// F1 
	  -1,	// F2 
	  -1,	// F3 
	  -1,	// F4 
	  -1,	// F5 
	  -1,	// F6 
	  -1,	// F7 
	  -1,	// F8 
	  -1,	// F9 
	  -1,	// FA 
	  -1,	// FB 
	  -1,	// FC 
	  -1,	// FD 
	  -1,	// FE 
	  -1,	// FF 
};

static const int table_symbol[256] = {
	  -1,	// 00 
	  -1,	// 01 
	  -1,	// 02 
	  -1,	// 03 
	  -1,	// 04 
	  -1,	// 05 
	  -1,	// 06 
	  -1,	// 07 
	0x08,	// 08 Back	BS
	  -1,	// 09 Tab
	  -1,	// 0A 
	  -1,	// 0B 
	  -1,	// 0C 
	0x0d,	// 0D Enter	CR
	  -1,	// 0E 
	  -1,	// 0F 
	  -1,	// 10 Shift
	  -1,	// 11 Ctrl
	  -1,	// 12 Alt
	  -1,	// 13 Pause
	  -1,	// 14 Caps
	  -1,	// 15 Kana
	  -1,	// 16 
	  -1,	// 17 
	  -1,	// 18 
	  -1,	// 19 Kanji
	  -1,	// 1A 
	0x03,	// 1B Escape	ETX
	  -1,	// 1C Convert
	  -1,	// 1D NonConv
	  -1,	// 1E 
	  -1,	// 1F 
	0x20,	// 20 Space	SPACE
	0x18,	// 21 PgUp	CAN
	0x0a,	// 22 PgDwn	LF
	  -1,	// 23 End
	  -1,	// 24 Home
	  -1,	// 25 Left
	  -1,	// 26 Up
	  -1,	// 27 Right
	  -1,	// 28 Down
	  -1,	// 29 
	  -1,	// 2A 
	  -1,	// 2B 
	  -1,	// 2C 
	  -1,	// 2D Ins
	0x7f,	// 2E Del	DEL
	  -1,	// 2F 
	0xa6,	// 30 0		WO
	  -1,	// 31 1
	  -1,	// 32 2
	0xa7,	// 33 3		XA
	0xa9,	// 34 4		XU
	0xaa,	// 35 5		XE
	0xab,	// 36 6		XO
	0xac,	// 37 7		XYA
	0xad,	// 38 8		XYU
	0xae,	// 39 9		XYO
	  -1,	// 3A 
	  -1,	// 3B 
	  -1,	// 3C 
	  -1,	// 3D 
	  -1,	// 3E 
	  -1,	// 3F 
	  -1,	// 40 
	  -1,	// 41 A
	  -1,	// 42 B
	  -1,	// 43 C
	  -1,	// 44 D
	0xa8,	// 45 E		XI
	  -1,	// 46 F
	  -1,	// 47 G
	  -1,	// 48 H
	  -1,	// 49 I
	  -1,	// 4A J
	  -1,	// 4B K
	  -1,	// 4C L
	  -1,	// 4D M
	  -1,	// 4E N
	  -1,	// 4F O
	  -1,	// 50 P
	  -1,	// 51 Q
	  -1,	// 52 R
	  -1,	// 53 S
	  -1,	// 54 T
	  -1,	// 55 U
	  -1,	// 56 V
	  -1,	// 57 W
	  -1,	// 58 X
	  -1,	// 59 Y
	0xaf,	// 5A Z		XTSU
	  -1,	// 5B 
	  -1,	// 5C 
	  -1,	// 5D 
	  -1,	// 5E 
	  -1,	// 5F 
	0x30,	// 60 NUM 0	0
	0x31,	// 61 NUM 1	1
	0x32,	// 62 NUM 2	2
	0x33,	// 63 NUM 3	3
	0x34,	// 64 NUM 4	4
	0x35,	// 65 NUM 5	5
	0x36,	// 66 NUM 6	6
	0x37,	// 67 NUM 7	7
	0x38,	// 68 NUM 8	8
	0x39,	// 69 NUM 9	9
	0x2a,	// 6A NUM *	*
	0x2b,	// 6B NUM +	+
	0x0d,	// 6C NUM Ent	CR
	0x2d,	// 6D NUM -	-
	0x2e,	// 6E NUM .	.
	0x2f,	// 6F NUM /	/
	  -1,	// 70 F1
	  -1,	// 71 F2
	  -1,	// 72 F3
	  -1,	// 73 F4
	  -1,	// 74 F5
	  -1,	// 75 F6
	  -1,	// 76 F7
	  -1,	// 77 F8
	  -1,	// 78 F9
	  -1,	// 79 F10
	  -1,	// 7A F11
	  -1,	// 7B F12
	  -1,	// 7C F13
	  -1,	// 7D F14
	  -1,	// 7E F15
	  -1,	// 7F F16
	  -1,	// 80 F17
	  -1,	// 81 F18
	  -1,	// 82 F19
	  -1,	// 83 F20
	  -1,	// 84 F21
	  -1,	// 85 F22
	  -1,	// 86 F23
	  -1,	// 87 F24
	  -1,	// 88 
	  -1,	// 89 
	  -1,	// 8A 
	  -1,	// 8B 
	  -1,	// 8C 
	  -1,	// 8D 
	  -1,	// 8E 
	  -1,	// 8F 
	  -1,	// 90 
	  -1,	// 91 ScrLk
	  -1,	// 92 
	  -1,	// 93 
	  -1,	// 94 
	  -1,	// 95 
	  -1,	// 96 
	  -1,	// 97 
	  -1,	// 98 
	  -1,	// 99 
	  -1,	// 9A 
	  -1,	// 9B 
	  -1,	// 9C 
	  -1,	// 9D 
	  -1,	// 9E 
	  -1,	// 9F 
	  -1,	// A0 L Shift
	  -1,	// A1 R Shift
	  -1,	// A2 L Ctrl
	  -1,	// A3 R Ctrl
	  -1,	// A4 L Alt
	  -1,	// A5 R Alt
	  -1,	// A6 
	  -1,	// A7 
	  -1,	// A8 
	  -1,	// A9 
	  -1,	// AA 
	  -1,	// AB 
	  -1,	// AC 
	  -1,	// AD 
	  -1,	// AE 
	  -1,	// AF 
	  -1,	// B0 
	  -1,	// B1 
	  -1,	// B2 
	  -1,	// B3 
	  -1,	// B4 
	  -1,	// B5 
	  -1,	// B6 
	  -1,	// B7 
	  -1,	// B8 
	  -1,	// B9 
	  -1,	// BA :
	  -1,	// BB ;
	0xa4,	// BC ,		TOUTEN
	  -1,	// BD -
	0xa1,	// BE .		KUTEN
	0xa5,	// BF /		CHUUTEN
	  -1,	// C0 @
	  -1,	// C1 
	  -1,	// C2 
	  -1,	// C3 
	  -1,	// C4 
	  -1,	// C5 
	  -1,	// C6 
	  -1,	// C7 
	  -1,	// C8 
	  -1,	// C9 
	  -1,	// CA 
	  -1,	// CB 
	  -1,	// CC 
	  -1,	// CD 
	  -1,	// CE 
	  -1,	// CF 
	  -1,	// D0 
	  -1,	// D1 
	  -1,	// D2 
	  -1,	// D3 
	  -1,	// D4 
	  -1,	// D5 
	  -1,	// D6 
	  -1,	// D7 
	  -1,	// D8 
	  -1,	// D9 
	  -1,	// DA 
	0xa2,	// DB [		KAGIKAKKO
	  -1,	// DC Yen
	0xa3,	// DD ]		KAGIKAKKO TOJI
	  -1,	// DE ^
	  -1,	// DF 
	  -1,	// E0 
	  -1,	// E1 
	  -1,	// E2 _
	  -1,	// E3 
	  -1,	// E4 
	  -1,	// E5 
	  -1,	// E6 
	  -1,	// E7 
	  -1,	// E8 
	  -1,	// E9 
	  -1,	// EA 
	  -1,	// EB 
	  -1,	// EC 
	  -1,	// ED 
	  -1,	// EE 
	  -1,	// EF 
	  -1,	// F0 
	  -1,	// F1 
	  -1,	// F2 
	  -1,	// F3 
	  -1,	// F4 
	  -1,	// F5 
	  -1,	// F6 
	  -1,	// F7 
	  -1,	// F8 
	  -1,	// F9 
	  -1,	// FA 
	  -1,	// FB 
	  -1,	// FC 
	  -1,	// FD 
	  -1,	// FE 
	  -1,	// FF 
};

static const int table_kana[256] = {
	  -1,	// 00 
	  -1,	// 01 
	  -1,	// 02 
	  -1,	// 03 
	  -1,	// 04 
	  -1,	// 05 
	  -1,	// 06 
	  -1,	// 07 
	0x08,	// 08 Back	BS
	  -1,	// 09 Tab
	  -1,	// 0A 
	  -1,	// 0B 
	  -1,	// 0C 
	0x0d,	// 0D Enter	CR
	  -1,	// 0E 
	  -1,	// 0F 
	  -1,	// 10 Shift
	  -1,	// 11 Ctrl
	  -1,	// 12 Alt
	  -1,	// 13 Pause
	  -1,	// 14 Caps
	  -1,	// 15 Kana
	  -1,	// 16 
	  -1,	// 17 
	  -1,	// 18 
	  -1,	// 19 Kanji
	  -1,	// 1A 
	0x03,	// 1B Escape	ETX
	  -1,	// 1C Convert
	  -1,	// 1D NonConv
	  -1,	// 1E 
	  -1,	// 1F 
	0x20,	// 20 Space	SPACE
	0x18,	// 21 PgUp	CAN
	0x0a,	// 22 PgDwn	LF
	  -1,	// 23 End
	  -1,	// 24 Home
	  -1,	// 25 Left
	  -1,	// 26 Up
	  -1,	// 27 Right
	  -1,	// 28 Down
	  -1,	// 29 
	  -1,	// 2A 
	  -1,	// 2B 
	  -1,	// 2C 
	  -1,	// 2D Ins
	0x7f,	// 2E Del	DEL
	  -1,	// 2F 
	0xdc,	// 30 0		WA
	0xc7,	// 31 1		NU
	0xcc,	// 32 2		FU
	0xb1,	// 33 3		A
	0xb3,	// 34 4		U
	0xb4,	// 35 5		E
	0xb5,	// 36 6		O
	0xd4,	// 37 7		YA
	0xd5,	// 38 8		YU
	0xd6,	// 39 9		YO
	  -1,	// 3A 
	  -1,	// 3B 
	  -1,	// 3C 
	  -1,	// 3D 
	  -1,	// 3E 
	  -1,	// 3F 
	  -1,	// 40 
	0xc1,	// 41 A		CHI
	0xba,	// 42 B		KO
	0xbf,	// 43 C		SO
	0xbc,	// 44 D		SHI
	0xb2,	// 45 E		I
	0xca,	// 46 F		HA
	0xb7,	// 47 G		KI
	0xb8,	// 48 H		KU
	0xc6,	// 49 I		NI
	0xcf,	// 4A J		MA
	0xc9,	// 4B K		NO
	0xd8,	// 4C L		RI
	0xd3,	// 4D M		MO
	0xd0,	// 4E N		MI
	0xd7,	// 4F O		RA
	0xbe,	// 50 P		SE
	0xc0,	// 51 Q		TA
	0xbd,	// 52 R		SU
	0xc4,	// 53 S		TO
	0xb6,	// 54 T		KA
	0xc5,	// 55 U		NA
	0xcb,	// 56 V		HI
	0xc3,	// 57 W		TE
	0xbb,	// 58 X		SA
	0xdd,	// 59 Y		N
	0xc2,	// 5A Z		TSU
	  -1,	// 5B 
	  -1,	// 5C 
	  -1,	// 5D 
	  -1,	// 5E 
	  -1,	// 5F 
	0x30,	// 60 NUM 0	0
	0x31,	// 61 NUM 1	1
	0x32,	// 62 NUM 2	2
	0x33,	// 63 NUM 3	3
	0x34,	// 64 NUM 4	4
	0x35,	// 65 NUM 5	5
	0x36,	// 66 NUM 6	6
	0x37,	// 67 NUM 7	7
	0x38,	// 68 NUM 8	8
	0x39,	// 69 NUM 9	9
	0x2a,	// 6A NUM *	*
	0x2b,	// 6B NUM +	+
	0x0d,	// 6C NUM Ent	CR
	0x2d,	// 6D NUM -	-
	0x2e,	// 6E NUM .	.
	0x2f,	// 6F NUM /	/
	  -1,	// 70 F1
	  -1,	// 71 F2
	  -1,	// 72 F3
	  -1,	// 73 F4
	  -1,	// 74 F5
	  -1,	// 75 F6
	  -1,	// 76 F7
	  -1,	// 77 F8
	  -1,	// 78 F9
	  -1,	// 79 F10
	  -1,	// 7A F11
	  -1,	// 7B F12
	  -1,	// 7C F13
	  -1,	// 7D F14
	  -1,	// 7E F15
	  -1,	// 7F F16
	  -1,	// 80 F17
	  -1,	// 81 F18
	  -1,	// 82 F19
	  -1,	// 83 F20
	  -1,	// 84 F21
	  -1,	// 85 F22
	  -1,	// 86 F23
	  -1,	// 87 F24
	  -1,	// 88 
	  -1,	// 89 
	  -1,	// 8A 
	  -1,	// 8B 
	  -1,	// 8C 
	  -1,	// 8D 
	  -1,	// 8E 
	  -1,	// 8F 
	  -1,	// 90 
	  -1,	// 91 ScrLk
	  -1,	// 92 
	  -1,	// 93 
	  -1,	// 94 
	  -1,	// 95 
	  -1,	// 96 
	  -1,	// 97 
	  -1,	// 98 
	  -1,	// 99 
	  -1,	// 9A 
	  -1,	// 9B 
	  -1,	// 9C 
	  -1,	// 9D 
	  -1,	// 9E 
	  -1,	// 9F 
	  -1,	// A0 L Shift
	  -1,	// A1 R Shift
	  -1,	// A2 L Ctrl
	  -1,	// A3 R Ctrl
	  -1,	// A4 L Alt
	  -1,	// A5 R Alt
	  -1,	// A6 
	  -1,	// A7 
	  -1,	// A8 
	  -1,	// A9 
	  -1,	// AA 
	  -1,	// AB 
	  -1,	// AC 
	  -1,	// AD 
	  -1,	// AE 
	  -1,	// AF 
	  -1,	// B0 
	  -1,	// B1 
	  -1,	// B2 
	  -1,	// B3 
	  -1,	// B4 
	  -1,	// B5 
	  -1,	// B6 
	  -1,	// B7 
	  -1,	// B8 
	  -1,	// B9 
	0xb9,	// BA :		KE
	0xda,	// BB ;		RE
	0xc8,	// BC ,		NE
	0xce,	// BD -		HO
	0xd9,	// BE .		RU
	0xd2,	// BF /		ME
	0xde,	// C0 @		DAKUON
	  -1,	// C1 
	  -1,	// C2 
	  -1,	// C3 
	  -1,	// C4 
	  -1,	// C5 
	  -1,	// C6 
	  -1,	// C7 
	  -1,	// C8 
	  -1,	// C9 
	  -1,	// CA 
	  -1,	// CB 
	  -1,	// CC 
	  -1,	// CD 
	  -1,	// CE 
	  -1,	// CF 
	  -1,	// D0 
	  -1,	// D1 
	  -1,	// D2 
	  -1,	// D3 
	  -1,	// D4 
	  -1,	// D5 
	  -1,	// D6 
	  -1,	// D7 
	  -1,	// D8 
	  -1,	// D9 
	  -1,	// DA 
	0xdf,	// DB [		HAN DAKUON
	0xb0,	// DC Yen	CHOUON
	0xd1,	// DD ]		MU
	0xcd,	// DE ^		HE
	  -1,	// DF 
	  -1,	// E0 
	  -1,	// E1 
	0xdb,	// E2 _		RO
	  -1,	// E3 
	  -1,	// E4 
	  -1,	// E5 
	  -1,	// E6 
	  -1,	// E7 
	  -1,	// E8 
	  -1,	// E9 
	  -1,	// EA 
	  -1,	// EB 
	  -1,	// EC 
	  -1,	// ED 
	  -1,	// EE 
	  -1,	// EF 
	  -1,	// F0 
	  -1,	// F1 
	  -1,	// F2 
	  -1,	// F3 
	  -1,	// F4 
	  -1,	// F5 
	  -1,	// F6 
	  -1,	// F7 
	  -1,	// F8 
	  -1,	// F9 
	  -1,	// FA 
	  -1,	// FB 
	  -1,	// FC 
	  -1,	// FD 
	  -1,	// FE 
	  -1,	// FF 
};

void MEMORY::initialize()
{
	column = 0xff;
	kana = false;
	pressed = 0;
	register_frame_event(this);
	
	memset(rdmy, 0xff, sizeof(rdmy));
	memset(mon, 0xff, sizeof(mon));
	memset(prom1, 0xff, sizeof(prom1));
	memset(prom2, 0xff, sizeof(prom2));
	memset(bas1, 0xff, sizeof(bas1));
	memset(bas2, 0xff, sizeof(bas2));
	memset(ram, 0x00, sizeof(ram));
	memset(vram, 0x00, sizeof(vram));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("MON.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(mon, sizeof(mon), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("PROM1.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(prom1, sizeof(prom1), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("PROM2.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(prom2, sizeof(prom2), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("LV1BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(bas1, sizeof(bas1), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("LV2BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(bas2, sizeof(bas2), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x0000, 0x07ff, wdmy, mon  );
	SET_BANK(0x0800, 0x0bff, wdmy, prom1);
	SET_BANK(0x0c00, 0x0fff, wdmy, prom2);
	SET_BANK(0x1000, 0xffff, wdmy, rdmy );
	
	boot_mode = -1;
}

void MEMORY::reset()
{
	if(boot_mode != config.boot_mode) {
		SET_BANK(0x1000, 0xffff, wdmy, rdmy);
		
		if(config.boot_mode == 2) {
			// EX-80BS LV2 BASIC
			SET_BANK(0x1800, 0x3fff, wdmy, bas2);
			SET_BANK(0x8000, 0xbfff, ram , ram );
			SET_BANK(0xfc00, 0xffff, vram, vram);
		} else if(config.boot_mode == 1) {
			// EX-80BS LV1 BASIC
			SET_BANK(0x3000, 0x3fff, wdmy, bas1);
			SET_BANK(0x8000, 0xbfff, ram , ram );
			SET_BANK(0xfc00, 0xffff, vram, vram);
		} else {
			// EX-80
			SET_BANK(0x8000, 0x87ff, ram , ram );
		}
		boot_mode = config.boot_mode;
	}
	irq_bits = 0;
}

void MEMORY::event_frame()
{
	const uint8_t* key_stat = emu->get_key_buffer();
	uint32_t val = 0xff;
	
	if(!(column & 0x10)) {
		if(key_stat[0x80]) val &= ~0x01;	// 0
		if(key_stat[0x81]) val &= ~0x02;	// 1
		if(key_stat[0x82]) val &= ~0x04;	// 2
		if(key_stat[0x83]) val &= ~0x08;	// 3
		if(key_stat[0x84]) val &= ~0x10;	// 4
		if(key_stat[0x85]) val &= ~0x20;	// 5
		if(key_stat[0x86]) val &= ~0x40;	// 6
		if(key_stat[0x87]) val &= ~0x80;	// 7
	}
	if(!(column & 0x20)) {
		if(key_stat[0x88]) val &= ~0x01;	// 8
		if(key_stat[0x89]) val &= ~0x02;	// 9
		if(key_stat[0x8a]) val &= ~0x04;	// A
		if(key_stat[0x8b]) val &= ~0x08;	// B
		if(key_stat[0x8c]) val &= ~0x10;	// C
		if(key_stat[0x8d]) val &= ~0x20;	// D
		if(key_stat[0x8e]) val &= ~0x40;	// E
		if(key_stat[0x8f]) val &= ~0x80;	// F
	}
	if(!(column & 0x40)) {
		if(key_stat[0x98]) val &= ~0x02;	// RET
		if(key_stat[0x99]) val &= ~0x01;	// RUN
		if(key_stat[0x9a]) val &= ~0x40;	// SDA
		if(key_stat[0x9b]) val &= ~0x80;	// LDA
		if(key_stat[0x9c]) val &= ~0x04;	// ADR
		if(key_stat[0x9d]) val &= ~0x10;	// RIC
		if(key_stat[0x9e]) val &= ~0x08;	// RDC
		if(key_stat[0x9f]) val &= ~0x20;	// WIC
	}
	d_pio->write_signal(SIG_I8255_PORT_A, val, 0xff);
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	column = data & mask;
	event_frame();
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	wbank[addr >> 10][addr & 0x3ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 10][addr & 0x3ff];
}

uint32_t MEMORY::fetch_op(uint32_t addr, int *wait)
{
	if((config.dipswitch & 1) && d_cpu->read_signal(SIG_I8080_INTE)) {
		irq_bits |= 1;
		d_cpu->write_signal(SIG_I8080_INTR, 1, 1);
	}
	*wait = 0;
	return read_data8(addr);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xaf:
		if((irq_bits &= ~(data & 0xfa)) == 0) {
			d_cpu->write_signal(SIG_I8080_INTR, 0, 1);
		}
		break;
	}
}

uint32_t MEMORY::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xef:
		return pressed;
	}
	return 0xff;
}

uint32_t MEMORY::get_intr_ack()
{
	if(irq_bits & 0x80) {
		return 0xc7; // RST 0
	}
	if(irq_bits & 0x40) {
		return 0xcf; // RST 1
	}
	if(irq_bits & 0x20) {
		return 0xd7; // RST 2
	}
	if(irq_bits & 0x10) {
		return 0xdf; // RST 3
	}
	if(irq_bits & 0x08) {
		return 0xe7; // RST 4
	}
	if(irq_bits & 0x04) {
		irq_bits &= ~0x04;
		return 0xef; // RST 5
	}
	if(irq_bits & 0x02) {
		return 0xf7; // RST 6
	}
	if(irq_bits & 0x01) {
		irq_bits &= ~0x01;
		return 0xff; // RST 7
	}
	return 0x00; // NOP
}

void MEMORY::key_down(int code)
{
	if(code == 0x15) {
		kana = !kana;
		return;
	}
	bool shift  = (emu->get_key_buffer()[0x10] != 0);
	bool symbol = (emu->get_key_buffer()[0x12] != 0);
	
	if(shift) {
		code = table_shift[code & 0xff];
	} else if(symbol) {
		code = table_symbol[code & 0xff];
	} else if(kana) {
		code = table_kana[code & 0xff];
	} else {
		code = table[code & 0xff];
	}
	if(code != -1) {
		pressed = code;
		irq_bits |= 4;
		d_cpu->write_signal(SIG_I8080_INTR, 1, 1);
	}
}

void MEMORY::load_binary(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::save_binary(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
}

#define STATE_VERSION	2

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(column);
	state_fio->StateValue(kana);
	state_fio->StateValue(pressed);
	state_fio->StateValue(irq_bits);
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateValue(boot_mode);
	return true;
}

