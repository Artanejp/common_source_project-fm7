/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.10.23-

	[ Romaji -> Kana conversion ]
*/

#ifndef _ROMAKANA_H
#define _ROMAKANA_H

// Note: This routine require at least C/C++99.
// Because this uses unicode string.
#include <wchar.h>

#define KANA_MARU			L'｡'
#define KANA_UPPER_KAKKO	L'｢'
#define KANA_DOWNER_KAKKO	L'｣'
#define KANA_COMMA			L'､'
#define KANA_NAKAGURO		L'･'
#define KANA_WO				L'ｦ'
#define KANA_SMALL_A		L'ｧ'
#define KANA_SMALL_I		L'ｨ'
#define KANA_SMALL_U		L'ｩ'
#define KANA_SMALL_E		L'ｪ'
#define KANA_SMALL_O		L'ｫ'
#define KANA_SMALL_YA		L'ｬ'
#define KANA_SMALL_YU		L'ｭ'
#define KANA_SMALL_YO		L'ｮ'
#define KANA_SMALL_TU		L'ﾂ'
#define KANA_ONBIKI			L'ｰ'
#define KANA_A				L'ｱ'
#define KANA_KA				L'ｶ'
#define KANA_SA				L'ｻ'
#define KANA_TA				L'ﾀ'
#define KANA_NA				L'ﾅ'
#define KANA_HA				L'ﾊ'
#define KANA_MA				L'ﾏ'
#define KANA_YA				L'ﾔ'
#define KANA_RA				L'ﾗ'
#define KANA_WA				L'ﾜ'
#define KANA_NN				L'ﾝ'
#define KANA_DAKUON			L'ﾞ'
#define KANA_HANDAKUON		L'ﾟ'

extern "C" {
// Convert romaji -> kana.
// ARG:
// src : src string (ASCII)
// dst : dst string (KANA = iso2022-jp + 0x80) see http://charset.7jp.net/jis.html .
// 
// -1 : Illegal
// 0  : Not converted
// 1 : convert
// 2 : convert, but, another candicate exiusts.
	extern int alphabet_to_kana(const _TCHAR *src, wchar_t *dst, int *dstlen);
}

#endif
