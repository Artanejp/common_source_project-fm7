/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.10.23-

	[ Romaji -> Kana conversion ]
*/

#if defined(_USE_QT)
#include "qt/gui/qt_input.h"
#endif
#include "common.h"
#include "romakana.h"

_TCHAR detect_shiin(const _TCHAR src, bool *b_dakuon, bool *b_handakuon, bool *b_chi, bool *b_kigou)
{
	_TCHAR base_code;
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
		base_code = 0;
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
		base_code = 0;
		break;
	}
	return base_code;
}

int detect_boin(_TCHAR src, _TCHAR base_code, bool *b_xya, bool *b_tsu)
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
		if((base_code & 0x00ff) == 0x00c0) {
			code = -2;
			*b_tsu = true;
		}
		break;
	}
	return code;
}

_TCHAR detect_xya(const _TCHAR c, _TCHAR base_code)
{
	bool b_dummy1;
	bool b_dummy2;
	int r_code = detect_boin(c, base_code, &b_dummy1, &b_dummy2);
	switch(r_code) {
	case 0: // Kya
		return KANA_SMALL_YA;
		break;
	case 2: // Kyu
		return KANA_SMALL_YU;
		break;
	case 4: // Kyo
		return KANA_SMALL_YO;
		break;
	default:
		return 0;
		break;
	}
	return 0;
}


_TCHAR detect_w(const _TCHAR c, _TCHAR base_code)
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
		return 0;
		break;
	}
	return 0;
}

extern "C" {
// Roma-Kana Conversion table for JIS keyboard (Maybe Host is 106 keyboard. Some keyboards needs another table). 
const romakana_convert_t romakana_table_1[] = {
	{'1', KANA_NA + 2, false}, // NU
	{'2', KANA_HA + 2, false}, // HU
	{'3', KANA_A  + 0, false}, // A
	{'4', KANA_A  + 2, false}, // U
	{'5', KANA_A  + 3, false}, // E
	{'6', KANA_A  + 4, false}, // O
	{'7', KANA_YA + 0, false}, // YA
	{'8', KANA_YA + 1, false}, // YU
	{'9', KANA_YA + 2, false}, // YO
	{'0', KANA_WA + 0, false}, // WA
	{'-', KANA_HA + 4, false}, // HO
	{'^', KANA_HA + 3, false}, // HE
	{VK_OEM_5, KANA_ONBIKI, false}, // -

	{'3', KANA_SMALL_A,      true}, // A
	{'4', KANA_SMALL_U,      true}, // U
	{'5', KANA_SMALL_E,      true}, // E
	{'6', KANA_SMALL_O,      true}, // O
	{'7', KANA_SMALL_YA + 0, true}, // YA
	{'8', KANA_SMALL_YA + 1, true}, // YU
	{'9', KANA_SMALL_YA + 2, true}, // YO
	{'0', KANA_WO,           true}, // WO

	{'Q', KANA_TA + 0, false}, // TA
	{'W', KANA_TA + 3, false}, // TE
	{'E', KANA_A  + 1, false}, // I
	{'R', KANA_SA + 2, false}, // SU
	{'T', KANA_KA + 0, false}, // KA
	{'Y', KANA_NN    , false}, // NN
	{'U', KANA_NA + 0, false}, // NA
	{'I', KANA_NA + 1, false}, // NI
	{'O', KANA_RA + 0, false}, // RA
	{'P', KANA_SA + 3, false}, // SE
	{'Q', KANA_TA + 0, false}, // TA
	{VK_OEM_3, KANA_DAKUON, false}, // DAKUTEN
	{VK_OEM_4, KANA_HANDAKUON, false}, // HANDAKUTEN
	{VK_OEM_4, KANA_UPPER_KAKKO, true}, // [
	
	{'A', KANA_TA + 1, false}, // TI
	{'S', KANA_TA + 4, false}, // TO
	{'D', KANA_SA + 1, false}, // SI
	{'F', KANA_HA + 0, false}, // HA
	{'G', KANA_KA + 1, false}, // KI
	{'H', KANA_KA + 2, false}, // KU
	{'J', KANA_MA + 0, false}, // MA
	{'K', KANA_NA + 4, false}, // NO
	{'L', KANA_RA + 1, false}, // RI
	{VK_OEM_PLUS, KANA_RA + 3, false}, // RE
	{VK_OEM_1, KANA_KA + 3, false}, // KE
	{VK_OEM_6, KANA_MA + 2, false}, // MU
	{VK_OEM_6, KANA_DOWNER_KAKKO, true}, // ]

	{'Z', KANA_TA + 2, false}, // TU
	{'Z', KANA_SMALL_TU, true}, // TU (Small)
	{'X', KANA_SA + 0, false}, // SA
	{'C', KANA_SA + 4, false}, // SO
	{'V', KANA_HA + 1, false}, // HI
	{'B', KANA_KA + 4, false}, // KO
	{'N', KANA_MA + 1, false}, // MI
	{'M', KANA_MA + 4, false}, // MO
	{VK_OEM_COMMA,  KANA_NA + 3, false}, // NE
	{VK_OEM_PERIOD, KANA_RA + 2, false}, // RU
	{VK_OEM_2,   KANA_MA + 3, false}, // ME
	{VK_OEM_102, KANA_RA + 4, false}, // RO

	{VK_OEM_COMMA,  KANA_COMMA, true}, // 
	{VK_OEM_PERIOD, KANA_MARU,  true}, // 
	{VK_OEM_2,   KANA_NAKAGURO, true}, // 
	//{VK_OEM_102, KANA_RA + 4, false}, // RO
	{0xffff, 0xffffffff, false}
};

int alphabet_to_kana(const _TCHAR *src, _TCHAR *dst, int *dstlen)
{
	int srclen;
	bool b_boin = false;
	bool b_shiin = false;
	bool b_x = false;
	bool b_dakuon = false;
	bool b_handakuon = false;
	bool b_xya = false;
	bool b_tsu = false;
	bool b_chi = false;
	bool b_kigou = false;
	int dstp = 0;
	int dlen;
	int i, j;
	_TCHAR base_code = 0;
	if((src == NULL) || (dst == NULL) || (dstlen == NULL)) return -1;
	srclen = strlen(src);
	dlen = *dstlen;
	if((dlen <= 0) || (srclen <= 0)) return -1;
	i = 0;
	j = 0;
	do {
		base_code = detect_shiin((const _TCHAR)src[i], &b_dakuon, &b_handakuon, &b_chi, &b_kigou);
		if(b_kigou) {
			if(base_code != 0) {
				if(j < dlen) {
					dst[j++] = base_code;
				}
			}
			i++;
		} else if(base_code != 0) {
			if((i + 1) >= srclen) {
				*dstlen = 0;
				return 0;
			}
			if(src[i] == src[i + 1]) {
				if(((src[i] & 0xff) == 'N') || ((src[i] & 0xff) == 'n')) {
					if(j < dlen) {
						dst[j++] = KANA_NN;
					}
					i += 2;
				} else {
					// TT
					if(j < dlen) {
						dst[j++] = KANA_SMALL_TU;
					}
					i += 1;
				}
			} else if((base_code & 0x00ff) == KANA_WA) {
				_TCHAR c_code = detect_w((const _TCHAR)src[i + 1], base_code);
				if(c_code != 0) {
					if(j < dlen) {
						dst[j++] = c_code;
					}
					i += 2;
				} else {
					if(j < dlen) {
						dst[j++] = src[i];
					}
					if(j < dlen) {
						dst[j++] = src[i + 1];
					}
					i += 2;
				}
			} else if((base_code & 0x00ff) == KANA_SMALL_A) {
				int c_code = 0;
				c_code = detect_boin((const _TCHAR)src[i + 1], base_code, &b_xya, &b_tsu);
				if(c_code >= 0) {
					if(j < dlen) {
						dst[j++] = KANA_SMALL_A + c_code;
					}
					i += 2;
				} else {
					if(b_xya) {
						if((i + 2) < srclen) {
							_TCHAR rr = detect_xya((const _TCHAR)src[i + 2], base_code);
							if(rr != 0) {
								if(j < dlen) {
									dst[j++] = rr;
								}
							} else {
								if(j < dlen) {
									dst[j++] = src[i];
								}
								if(j < dlen) {
									dst[j++] = src[i + 1];
								}
								if(j < dlen) {
									dst[j++] = src[i + 2];
								}
							}
							i += 3;
						} else {
							*dstlen = 0;
							return 0;
						}
					} else {
						if((i + 2) < srclen) {
							if(j < dlen) {
								dst[j++] = src[i];
							}
							if(j < dlen) {
								dst[j++] = src[i + 1];
							}
							i += 2;
						} else {
							*dstlen = 0;
							return 0;
						}
					}
				}
			} else {
				int code = detect_boin((const _TCHAR)src[i + 1], base_code, &b_xya, &b_tsu);
				if(code >= 0) {
					if(j < dlen) {
						if((base_code & 0x00ff) == KANA_YA) {
							switch(code) {
							case 0:
								dst[j++] = base_code;
								break;
							case 2:
								dst[j++] = base_code + 1;
								break;
							case 4:
								dst[j++] = base_code + 2;
								break;
							default:
								dst[j++] = src[i];
								if(j < dlen) {
									dst[j++] = src[i + 1];
								}
								break;
							}
						} else {
							dst[j++] = base_code + code;
						}
					}
					if(b_dakuon) {
						if(j < dlen) {
							dst[j++] = KANA_DAKUON;
						}
					} else if(b_handakuon) {
						if(j < dlen) {
							dst[j++] = KANA_HANDAKUON;
						}
					}
					i += 2;
				} else {
					_TCHAR next_code = 0;
					if(b_xya) {
						if((i + 2) >= srclen) {
							*dstlen = 0;
							return 0;
						}
						uint32_t bc = base_code & 0x00ff;
						if(((bc >= KANA_KA) && (bc <= KANA_MA)) || (bc == KANA_RA)) {
							// Ky*, Sy*, Ty*, Ny*, My*, Hy*, Ry*, Zy*, Dy*, By*, Py*
							next_code = detect_xya((const _TCHAR)src[i + 2], base_code);
							if(next_code != 0) {
								if(j < dlen) {
									dst[j] = base_code + 1;
									j++;
									i += 2;
								}
								if(b_dakuon) {
									if(j < dlen) {
										dst[j] = KANA_DAKUON;
										j++;
									}
								} else if(b_handakuon) {
									if(j < dlen) {
										dst[j] = KANA_HANDAKUON;
										j++;
								}
								}								
								if(j < dlen) {
									dst[j] = next_code;
									j++;
									i++;
								}
							} else {
								i += 2;
							}
						} else {
							if(j < dlen) {
								dst[j++] = src[i];
							}
							if(j < dlen) {
								dst[j++] = src[i + 1];
							}
							if(j < dlen) {
								dst[j++] = src[i + 2];
							}
							i += 3;
						}
					} else  if(b_tsu) {
						if((i + 2) >= srclen) {
							*dstlen = 0;
							return 0;
						}
						uint32_t bc = base_code & 0x00ff;
						if((src[i + 2] == 'u') || (src[i + 2] == 'U')) {
							if(j < dlen) {
								dst[j++] = KANA_TA + 2; // TU
							}
							i += 3;
						} else {
							if(j < dlen) {
								dst[j++] = src[i];
							}
							if(j < dlen) {
								dst[j++] = src[i + 1];
							}
							if(j < dlen) {
								dst[j++] = src[i + 2];
							}
							i += 3;
						}
					} else {
						if(j < dlen) {
						dst[j++] = src[i++];
						}
						if(j < dlen) {
							dst[j++] = src[i++];
						}
					}
				}
			}
		} else { // Not shiin
			if(b_chi) {
				if((i + 2) < srclen) {
					if((src[i + 1] == 'H') || (src[i + 1] == 'h')) {
						int code = detect_boin((const _TCHAR)src[i + 2], base_code, &b_xya, &b_tsu);
						_TCHAR ccode = 0;
						if(code >= 0) {
							switch(code) {
							case 0: // Cha
							case 2: // Chu
							case 3: // Che
							case 4: // Cho
								ccode = KANA_TA + 1;
								break;
							}
							if(ccode != 0) {
								if(j < dlen) {
									dst[j++] = ccode;
								}
								if(j < dlen) {
									if(code == 3) { // Che
										dst[j++] = KANA_SMALL_E;
									} else {
										dst[j++] = (code >> 1) + KANA_SMALL_YA;
									}
								}
							} else {
								if(code == 1) {
									if(j < dlen) {
										dst[j++] = KANA_TA + 1;
									}
								} else {
									if(j < dlen) {
										dst[j++] = src[i];
									}
									if(j < dlen) {
										dst[j++] = src[i + 1];
									}
									if(j < dlen) {
										dst[j++] = src[i + 2];
									}
								}
							}
							i += 3;
						} else {
							if(j < dlen) {
								dst[j++] = src[i];
							}
							if(j < dlen) {
								dst[j++] = src[i + 1];
							}
							i += 2;
						}
					} else { // Not cha-cho
						if(j < dlen) {
							dst[j++] = src[i++];
						}
					}						
				} else {
					*dstlen = 0;
					return 0;
				}					
			} else { // Not chi
				if(base_code != 0) {
					if(j < dlen) {
						dst[j++] = src[i];
					}
					break; // over srclen.
				}
				int code = detect_boin((const _TCHAR)src[i], base_code, &b_xya, &b_tsu);
				if(code >= 0) {
					if(j < dlen) {
						dst[j++] = code + KANA_A;
					}
				} else {
					if(j < dlen) {
						dst[j++] = src[i];
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
