/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.10.23-

	[ Romaji -> Kana conversion ]
*/


// Note: This routine require at least C/C++99.
// Because this uses unicode string.
#include <wchar.h>

#include "common.h"
#include "romakana.h"

wchar_t detect_shiin(const char src, bool *b_dakuon, bool *b_handakuon, bool *b_chi, bool *b_kigou)
{
	wchar_t base_code;
	*b_dakuon = false;
	*b_handakuon = false;
	*b_chi = false;
	*b_kigou = false;
	switch(src) {
	case 'X':
	case 'x':
		base_code = KANA_SMALL_A;
		break;
	case 'K':
	case 'k':
		base_code = KANA_KA; // KA
		break;
	case 'S':
	case 's':
		base_code = KANA_SA; // SA
		break;
	case 'T':
	case 't':
		base_code = KANA_TA; // TA
		break;
	case 'N':
	case 'n':
		base_code = KANA_NA; // NA
		break;
	case 'H':
	case 'h':
		base_code = KANA_HA; // HA
		break;
	case 'M':
	case 'm':
		base_code = KANA_MA; // MA
		break;
	case 'Y':
	case 'y':
		base_code = KANA_YA; // YA
		break;
	case 'R':
	case 'r':
		base_code = KANA_RA; // RA
		break;
	case 'W':
	case 'w':
		base_code = KANA_WA; // WA
		break;
	case 'G':
	case 'g':
		base_code = KANA_KA;
		*b_dakuon = true;
		break;
	case 'Z':
	case 'z':
	case 'J':
	case 'j':
		base_code = KANA_SA;
		*b_dakuon = true;
		break;
	case 'D':
	case 'd':
		base_code = KANA_TA;
		*b_dakuon = true;
		break;
	case 'B':
	case 'b':
		base_code = KANA_HA;
		*b_dakuon = true;
		break;
	case 'P':
	case 'p':
		base_code = KANA_HA;
		*b_handakuon = true;
		break;
	case 'C':
	case 'c':
		base_code = (wchar_t)0;
		*b_chi = true;
		break;
	case '-':
		base_code = KANA_ONBIKI;
		*b_kigou = true;
		break;
	case '`':
		base_code = KANA_DAKUON;
		*b_kigou = true;
		break;
	case '{':
		base_code = KANA_HANDAKUON;
		*b_kigou = true;
		break;
	case '[':
		base_code = KANA_UPPER_KAKKO;
		*b_kigou = true;
		break;
	case ']':
		base_code = KANA_DOWNER_KAKKO;
		*b_kigou = true;
		break;
	case ',':
		base_code = KANA_COMMA;
		*b_kigou = true;
		break;
	case '.':
		base_code = KANA_MARU;
		*b_kigou = true;
		break;
	case '/':
		base_code = KANA_NAKAGURO;
		*b_kigou = true;
		break;
	default:
		base_code = (wchar_t)0;
		break;
	}
	return base_code;
}

int detect_boin(const char src, wchar_t base_code, bool *b_xya, bool *b_tsu)
{
	int code = -1;
	*b_xya = false;
	*b_tsu = false;
	switch(src)
	{
	case 'A':
	case 'a':
		code = 0;
		break;
	case 'I':
	case 'i':
		code = 1;
		break;
	case 'U':
	case 'u':
		code = 2;
		break;
	case 'E':
	case 'e':
		code = 3;
		break;
	case 'O':
	case 'o':
		code = 4;
		break;
	case 'Y':
	case 'y':
		*b_xya = true;
		code = -1;
		break;
	case 'S': // TSU
	case 's':
		if(base_code  == L'ﾀ') {
			code = -2;
			*b_tsu = true;
		}
		break;
	}
	return code;
}

wchar_t detect_xya(const char c, wchar_t base_code)
{
	bool b_dummy1;
	bool b_dummy2;
	int r_code = detect_boin(c, base_code, &b_dummy1, &b_dummy2);
	switch(r_code) {
	case 0: // Kya
		return KANA_SMALL_YA;
		break;
	case 1: // Kyi
		return KANA_SMALL_I;
		break;
	case 2: // Kyu
		return KANA_SMALL_YU;
		break;
	case 3: // Kye
		return KANA_SMALL_E;
		break;
	case 4: // Kyo
		return KANA_SMALL_YO;
		break;
	default:
		return (wchar_t)0;
		break;
	}
	return (wchar_t)0;
}


wchar_t detect_w(const char c, wchar_t base_code)
{
	bool b_dummy1;
	bool b_dummy2;
	int r_code = detect_boin(c, base_code, &b_dummy1, &b_dummy2);
	switch(r_code) {
	case 0: // Wa
		return KANA_WA;
		break;
	case 4: // Wo
		return KANA_WO;
		break;
	default:
		return (wchar_t)0;
		break;
	}
	return (wchar_t)0;
}

extern "C" {
// Roma-Kana Conversion table for JIS keyboard (Maybe Host is 106 keyboard. Some keyboards needs another table). 
int alphabet_to_kana(const _TCHAR *src, wchar_t *dst, int *dstlen)
{
	int srclen;
	//bool b_boin = false;
	//bool b_shiin = false;
	//bool b_x = false;
	bool b_dakuon = false;
	bool b_handakuon = false;
	bool b_xya = false;
	bool b_tsu = false;
	bool b_chi = false;
	bool b_kigou = false;
	//int dstp = 0;
	int dlen;
	int i, j;
	wchar_t base_code[4];
	const wchar_t *tmptt_a[5] = {L"ｱ", L"ｲ", L"ｳ", L"ｴ", L"ｵ"};
	const wchar_t *tmptt_ka[5] = {L"ｶ", L"ｷ", L"ｸ", L"ｹ", L"ｺ"};
	const wchar_t *tmptt_sa[5] = {L"ｻ", L"ｼ", L"ｽ", L"ｾ", L"ｿ"};
	const wchar_t *tmptt_ta[5] = {L"ﾀ", L"ﾁ", L"ﾂ", L"ﾃ", L"ﾄ"};
	const wchar_t *tmptt_na[5] = {L"ﾅ", L"ﾆ", L"ﾇ", L"ﾈ", L"ﾉ"};
	const wchar_t *tmptt_ha[5] = {L"ﾊ", L"ﾋ", L"ﾌ", L"ﾍ", L"ﾎ"};
	const wchar_t *tmptt_ma[5] = {L"ﾏ", L"ﾐ", L"ﾑ", L"ﾒ", L"ﾓ"};
	const wchar_t *tmptt_ra[5] = {L"ﾗ", L"ﾘ", L"ﾙ", L"ﾚ", L"ﾛ"};
	const wchar_t *tbl_tt[5] = {L"ｧ", L"ｨ", L"ｩ", L"ｪ", L"ｫ"};
	
	if((src == NULL) || (dst == NULL) || (dstlen == NULL)) return -1;
	srclen = strlen(src);
	dlen = *dstlen;
	if((dlen <= 0) || (srclen <= 0)) return -1;
	i = 0;
	j = 0;
	do {
		memset(base_code, 0x00, sizeof(base_code));
		base_code[0] = detect_shiin((const char)src[i], &b_dakuon, &b_handakuon, &b_chi, &b_kigou);
		if(b_kigou) {
			if(wcslen(base_code) != (size_t)0) {
				if(j < dlen) {
					wcsncat(dst, base_code, 1);
					j++;
					//dst[j++] = base_code;
				}
			}
			i++;
		} else if(wcslen(base_code) != (size_t)0) {
			if((i + 1) >= srclen) {
				*dstlen = 0;
				return 0;
			}
			if(src[i] == src[i + 1]) {
				if(((src[i] & 0xff) == 'N') || ((src[i] & 0xff) == 'n')) {
					if(j < dlen) {
						wcscat(dst, L"ﾝ");
						j++;
						//dst[j++] = (uint32_t)KANA_NN;
					}
					i += 2;
				} else {
					// TT
					if(j < dlen) {
						wcscat(dst, L"ｯ");
						j++;
						//dst[j++] = KANA_SMALL_TU;
					}
					i += 1;
				}
			} else if(wcsncmp(base_code, L"ﾜ", 1) == 0) {
				wchar_t c_code[2] = {0};
				c_code[0] = detect_w((const char)src[i + 1], base_code[0]);
				if(wcslen(c_code) != (size_t)0) {
					if(j < dlen) {
						wcsncat(dst, c_code, 1);
						j++;
						//dst[j++] = c_code;
					}
					i += 2;
				} else {
					wchar_t tmps[4];
					if(j < dlen) {
						swprintf(tmps, 1, L"%c", src[i]);
						wcsncat(dst, tmps, 1);
						j++;
						//dst[j++] = src[i];
					}
					if(j < dlen) {
						swprintf(tmps, 1, L"%c", src[i + 1]);
						wcsncat(dst, tmps, 1);
						j++;
						//dst[j++] = src[i + 1];
					}
					i += 2;
				}
			} else if(wcsncmp(base_code, L"ｧ", 1) == 0) {
				int c_code = 0;
				c_code = detect_boin((const _TCHAR)src[i + 1], base_code[0], &b_xya, &b_tsu);
				if(c_code >= 0) {
					if(j < dlen) {
						if(c_code < 5) wcsncat(dst, tbl_tt[c_code], 1);
						j++;
						//dst[j++] = KANA_SMALL_A + c_code;
					}
					i += 2;
				} else {
					if(b_xya) {
						if((i + 2) < srclen) {
							wchar_t rr[2] = {0};
							rr[0] = detect_xya((const char)src[i + 2], base_code[0]);
							if(wcslen(rr) != 0) {
								if(j < dlen) {
									wcsncat(dst, rr, 1);
									j++;
									//dst[j++] = rr;
								}
							} else {
								wchar_t tmps[4];
								
								if(j < dlen) {
									swprintf(tmps, 1, L"%c", src[i]);
									wcsncat(dst, tmps, 1);
									j++;
									//dst[j++] = src[i];
								}
								if(j < dlen) {
									swprintf(tmps, 1, L"%c", src[i + 1]);
									wcsncat(dst, tmps, 1);
									j++;
									//dst[j++] = src[i];
								}
								if(j < dlen) {
									swprintf(tmps, 1, L"%c", src[i + 2]);
									wcsncat(dst, tmps, 1);
									j++;
									//dst[j++] = src[i];
								}
							}
							i += 3;
						} else {
							*dstlen = 0;
							return 0;
						}
					} else {
						if((i + 2) < srclen) {
							wchar_t tmps[4] = {0};
							if(j < dlen) {
								swprintf(tmps, 1, L"%c", src[i]);
								wcsncat(dst, tmps, 1);
								j++;
								//dst[j++] = src[i];
							}
							if(j < dlen) {
								swprintf(tmps, 1, L"%c", src[i + 1]);
								wcsncat(dst, tmps, 1);
								j++;
								//dst[j++] = src[i];
							}
							i += 2;
						} else {
							*dstlen = 0;
							return 0;
						}
					}
				}
			} else {
				int code = detect_boin((const char)src[i + 1], base_code[0], &b_xya, &b_tsu);
				wchar_t tmps[4] = {0};
				if(code >= 0) {
					if(j < dlen) {
						if(wcsncmp(base_code, L"ﾔ", 1) == 0) {
							switch(code) {
							case 0:
								wcsncat(dst, L"ﾔ", 1);
								j++;
								break;
							case 2:
								wcsncat(dst, L"ﾕ", 1);
								j++;
								break;
							case 4:
								wcsncat(dst, L"ﾖ", 1);
								j++;
								break;
							default:
								swprintf(tmps, 1, L"%c", src[i]);
								wcsncat(dst, tmps, 1);
								j++;
								if(j < dlen) {
									swprintf(tmps, 1, L"%c", src[i + 1]);
									wcsncat(dst, tmps, 1);
									j++;
								}
								break;
							}
						} else {
							const wchar_t **base_addr = NULL;
							if((wcsncmp(base_code, L"ｱ", 1) == 0) && (code < 5)){
								base_addr = tmptt_a;
							} else if((wcsncmp(base_code, L"ｶ", 1) == 0) && (code < 5)){
								base_addr = tmptt_ka;
							} else if((wcsncmp(base_code, L"ｻ", 1) == 0) && (code < 5)){
								base_addr = tmptt_sa;
							} else if((wcsncmp(base_code, L"ﾀ", 1) == 0) && (code < 5)){
								base_addr = tmptt_ta;
							} else if((wcsncmp(base_code, L"ﾅ", 1) == 0) && (code < 5)){
								base_addr = tmptt_na;
							} else if((wcsncmp(base_code, L"ﾊ", 1) == 0) && (code < 5)){
								base_addr = tmptt_ha;
							} else if((wcsncmp(base_code, L"ﾏ", 1) == 0) && (code < 5)){
								base_addr = tmptt_ma;
							} else if((wcsncmp(base_code, L"ﾗ", 1) == 0) && (code < 5)){
								base_addr = tmptt_ra;
							}
							if(base_addr != NULL) {
								wcsncat(dst, base_addr[code], 1);
								j++;
							}
							//dst[j++] = base_code + code;
						}
					}
					if(b_dakuon) {
						if(j < dlen) {
							wcsncat(dst, L"ﾞ", 1);
							j++;
							//dst[j++] = KANA_DAKUON;
						}
					} else if(b_handakuon) {
						if(j < dlen) {
							wcsncat(dst, L"ﾟ", 1);
							j++;
							//dst[j++] = KANA_HANDAKUON;
						}
					}
					i += 2;
				} else {
					wchar_t next_code[4] = {0};
					if(b_xya) {
						if((i + 2) >= srclen) {
							*dstlen = 0;
							return 0;
						}
						if((base_code[0] == L'ｶ') ||
						   (base_code[0] == L'ｻ') ||
						   (base_code[0] == L'ﾀ') ||
						   (base_code[0] == L'ﾅ') ||
						   (base_code[0] == L'ﾊ') ||
						   (base_code[0] == L'ﾏ') ||
						   (base_code[0] == L'ﾗ')) {
							// Ky*, Sy*, Ty*, Ny*, My*, Hy*, Ry*, Zy*, Dy*, By*, Py*
							next_code[0] = detect_xya((const char)src[i + 2], base_code[0]);
							if(wcslen(next_code) != 0) {
								if(j < dlen) {
									if((wcsncmp(base_code, L"ｶ", 1) == 0)){
										wcsncat(dst, L"ｷ", 1);
									} else if((wcsncmp(base_code, L"ｻ", 1) == 0)){
										wcsncat(dst, L"ｼ", 1);
									} else if((wcsncmp(base_code, L"ﾀ", 1) == 0)){
										wcsncat(dst, L"ﾁ", 1);
									} else if((wcsncmp(base_code, L"ﾅ", 1) == 0)){
										wcsncat(dst, L"ﾆ", 1);
									} else if((wcsncmp(base_code, L"ﾊ", 1) == 0)){
										wcsncat(dst, L"ﾋ", 1);
									} else if((wcsncmp(base_code, L"ﾏ", 1) == 0)){
										wcsncat(dst, L"ﾐ", 1);
									} else if((wcsncmp(base_code, L"ﾗ", 1) == 0)){
										wcsncat(dst, L"ﾘ", 1);
									}
									j++;
									i += 2;
								}
								if(b_dakuon) {
									if(j < dlen) {
										wcsncat(dst, L"ﾞ", 1);
										j++;
									}
								} else if(b_handakuon) {
									if(j < dlen) {
										wcsncat(dst, L"ﾟ", 1);
										j++;
								}
								}								
								if(j < dlen) {
									wcsncat(dst, next_code, 1);
									j++;
									i++;
								}
							} else {
								i += 2;
							}
						} else {
							wchar_t tmps[4] = {0};
							if(j < dlen) {
								swprintf(tmps, 1, L"%c", src[i]);
								wcsncat(dst, tmps, 1);
								j++;
								//dst[j++] = src[i];
							}
							if(j < dlen) {
								swprintf(tmps, 1, L"%c", src[i + 1]);
								wcsncat(dst, tmps, 1);
								j++;
								//dst[j++] = src[i];
							}
							if(j < dlen) {
								swprintf(tmps, 1, L"%c", src[i + 2]);
								wcsncat(dst, tmps, 1);
								j++;
								//dst[j++] = src[i];
							}
							i += 3;
						}
					} else  if(b_tsu) {
						if((i + 2) >= srclen) {
							*dstlen = 0;
							return 0;
						}
						if((src[i + 2] == 'u') || (src[i + 2] == 'U')) {
							if(j < dlen) {
								wcsncat(dst, L"ﾂ", 1);
								j++;
							}
							i += 3;
						} else {
							wchar_t tmps[4] = {0};
							if(j < dlen) {
								swprintf(tmps, 1, L"%c", src[i]);
								wcsncat(dst, tmps, 1);
								j++;
								//dst[j++] = src[i];
							}
							if(j < dlen) {
								swprintf(tmps, 1, L"%c", src[i + 1]);
								wcsncat(dst, tmps, 1);
								j++;
								//dst[j++] = src[i];
							}
							if(j < dlen) {
								swprintf(tmps, 1, L"%c", src[i + 2]);
								wcsncat(dst, tmps, 1);
								j++;
								//dst[j++] = src[i];
							}
							i += 3;
						}
					} else {
						wchar_t tmps[4] = {0};
						if(j < dlen) {
							swprintf(tmps, 1, L"%c", src[i]);
							wcsncat(dst, tmps, 1);
							j++;
							//dst[j++] = src[i];
						}
						if(j < dlen) {
							swprintf(tmps, 1, L"%c", src[i + 1]);
							wcsncat(dst, tmps, 1);
							j++;
							//dst[j++] = src[i];
						}
						i += 2;
					}
				}
			}
		} else { // Not shiin
			if(b_chi) {
				if((i + 2) < srclen) {
					if((src[i + 1] == 'H') || (src[i + 1] == 'h')) {
						int code = detect_boin((const _TCHAR)src[i + 2], base_code[0], &b_xya, &b_tsu);
						if(code >= 0) {
							if(code == 1) {
								wchar_t tmps[4] = {0};
								if(j < dlen) {
									swprintf(tmps, 1, L"%c", src[i]);
									wcsncat(dst, tmps, 1);
									j++;
									//dst[j++] = src[i];
								}
								if(j < dlen) {
									swprintf(tmps, 1, L"%c", src[i + 1]);
									wcsncat(dst, tmps, 1);
									j++;
									//dst[j++] = src[i];
								}
								if(j < dlen) {
									swprintf(tmps, 1, L"%c", src[i + 2]);
									wcsncat(dst, tmps, 1);
									j++;
									//dst[j++] = src[i];
								}
							} else {
								if((j + 2) < dlen) {
									switch(code) {
									case 0: // Cha
										wcsncat(dst, L"ﾁｬ", 1);
										j += 2;
										break;
									case 2: // Chu
										wcsncat(dst, L"ﾁｭ", 1);
										j += 2;
										break;
									case 3: // Chu
										wcsncat(dst, L"ﾁｪ", 1);
										j += 2;
										break;
									case 4: // Cho
										wcsncat(dst, L"ﾁｮ", 1);
										j += 2;
										break;
									}
								}
							}
							i += 3;
						} else {
							wchar_t tmps[4] = {0};
							if(j < dlen) {
								swprintf(tmps, 1, L"%c", src[i]);
								wcsncat(dst, tmps, 1);
								j++;
								//dst[j++] = src[i];
							}
							if(j < dlen) {
								swprintf(tmps, 1, L"%c", src[i + 1]);
								wcsncat(dst, tmps, 1);
								j++;
								//dst[j++] = src[i];
							}
							i += 2;
						}
					} else { // Not cha-cho
						wchar_t tmps[4] = {0};
						if(j < dlen) {
							swprintf(tmps, 1, L"%c", src[i]);
							wcsncat(dst, tmps, 1);
							j++;
							//dst[j++] = src[i];
						}
						i++;
					}					
				} else {
					*dstlen = 0;
					return 0;
				}					
			} else { // Not chi
				if(wcslen(base_code) != 0) {
					if(j < dlen) {
						wchar_t tmps[4] = {0};
						if(j < dlen) {
							swprintf(tmps, 1, L"%c", src[i]);
							wcsncat(dst, tmps, 1);
							j++;
							//dst[j++] = src[i];
						}
						i++;
					}						
					break; // over srclen.
				}
				int code = detect_boin((const char)src[i], base_code[0], &b_xya, &b_tsu);
				if(code >= 0) {
					if(j < dlen) {
						if(code < 5) {
							wcsncat(dst, tmptt_a[code], 1);
							j++;
						}
					}
				} else {
					if(j < dlen) {
						wchar_t tmps[4] = {0};
						if(j < dlen) {
							swprintf(tmps, 1, L"%c", src[i]);
							wcsncat(dst, tmps, 1);
							j++;
							//dst[j++] = src[i];
						}
					}
				}
				i++;
			}
		}
	} while((i <= srclen) && (j < dlen));
	*dstlen = j;
	if(j <= 0) return 0;
	return i;
}
}
