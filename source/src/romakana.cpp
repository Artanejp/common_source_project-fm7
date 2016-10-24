/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.10.23-

	[ Romaji -> Kana conversion ]
*/

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

int detect_boin(_TCHAR src, _TCHAR base_code, bool *b_xya, bool *b_tsu, bool *b_nn)
{
	int code = -1;
	*b_xya = false;
	*b_tsu = false;
	*b_nn = false;
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
	case 'N': // NN
	case 'n':
		code = -3;
		*b_nn = true;
		break;
	}
	return code;
}

_TCHAR detect_xya(const _TCHAR c, _TCHAR base_code)
{
	bool b_dummy1;
	bool b_dummy2;
	bool b_dummy3;
	int r_code = detect_boin(c, base_code, &b_dummy1, &b_dummy2, &b_dummy3);
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
	bool b_dummy3;
	int r_code = detect_boin(c, base_code, &b_dummy1, &b_dummy2, &b_dummy3);
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
	bool b_nn = false;
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
				// TT
				if(j < dlen) {
					dst[j++] = KANA_SMALL_TU;
				}
				i+= 1;
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
				c_code = detect_boin((const _TCHAR)src[i + 1], base_code, &b_xya, &b_tsu, &b_nn);
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
				int code = detect_boin((const _TCHAR)src[i + 1], base_code, &b_xya, &b_tsu, &b_nn);
				if(b_nn) {
					if(j < dlen) {
						dst[j++] = KANA_NN;
					}
					i += 2;
				} else if(code >= 0) {
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
						int code = detect_boin((const _TCHAR)src[i + 2], base_code, &b_xya, &b_tsu, &b_nn);
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
				int code = detect_boin((const _TCHAR)src[i], base_code, &b_xya, &b_tsu, &b_nn);
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
