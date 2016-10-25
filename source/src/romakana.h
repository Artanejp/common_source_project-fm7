/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.10.23-

	[ Romaji -> Kana conversion ]
*/

#ifndef _ROMAKANA_H
#define _ROMAKANA_H

#define KANA_MARU			0x00a1
#define KANA_UPPER_KAKKO	0x00a2
#define KANA_DOWNER_KAKKO	0x00a3
#define KANA_COMMA			0x00a4
#define KANA_NAKAGURO		0x00a5
#define KANA_WO				0x00a6
#define KANA_SMALL_A		0x00a7
#define KANA_SMALL_I		0x00a8
#define KANA_SMALL_U		0x00a9
#define KANA_SMALL_E		0x00aa
#define KANA_SMALL_O		0x00ab
#define KANA_SMALL_YA		0x00ac
#define KANA_SMALL_YU		0x00ad
#define KANA_SMALL_YO		0x00ae
#define KANA_SMALL_TU		0x00af
#define KANA_ONBIKI			0x00b0
#define KANA_A				0x00b1
#define KANA_KA				0x00b6
#define KANA_SA				0x00bb
#define KANA_TA				0x00c0
#define KANA_NA				0x00c5
#define KANA_HA				0x00ca
#define KANA_MA				0x00cf
#define KANA_YA				0x00d4
#define KANA_RA				0x00d7
#define KANA_WA				0x00dc
#define KANA_NN				0x00dd
#define KANA_DAKUON			0x00de
#define KANA_HANDAKUON		0x00df

typedef struct {
	uint32_t vk;
	uint32_t code;
	bool shift;
} romakana_convert_t;

extern "C" {
	extern const romakana_convert_t romakana_table_1[];
// Convert romaji -> kana.
// ARG:
// src : src string (ASCII)
// dst : dst string (KANA = iso2022-jp + 0x80) see http://charset.7jp.net/jis.html .
// 
// -1 : Illegal
// 0  : Not converted
// 1 : convert
// 2 : convert, but, another candicate exiusts.
	extern int alphabet_to_kana(const _TCHAR *src, _TCHAR *dst, int *dstlen);
}

#endif
