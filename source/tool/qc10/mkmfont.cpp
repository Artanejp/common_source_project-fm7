
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#if (defined(_MSC_VER) && (_MSC_VER == 1200)) || defined(_WIN32_WCE)
#define for if(0);else for
#endif

static const int table[64][3] = {
	{0xC5, 0xC5, 0}, {0xC4, 0xC4, 0}, {0xE5, 0xE5, 0}, {0xE3, 0xE3, 0},
	{0xE4, 0xE4, 0}, {0xE1, 0xE1, 0}, {0xE0, 0xE0, 0}, {0xE2, 0xE2, 0},
	{0xAA, 0xAA, 0}, {0xEF, 0xEF, 0}, {0xED, 0xED, 0}, {0xEC, 0xEC, 0},
	{0xEE, 0xEE, 0}, {0xDC, 0xDC, 0}, {0xF9, 0xF9, 0}, {0xFC, 0xFC, 0},
	{0xFA, 0xFA, 0}, {0xF9, 0xF9, 0}, {0xFB, 0xFB, 0}, {0xC9, 0xC9, 0},
	{0xEB, 0xEB, 0}, {0xE9, 0xE9, 0}, {0xE9, 0xE9, 0}, {0xE8, 0xE8, 0},
	{0xEA, 0xEA, 0}, {0xD6, 0xD6, 0}, {0xF6, 0xF6, 0}, {0xF3, 0xF3, 0},
	{0xF2, 0xF2, 0}, {0xF4, 0xF4, 0}, {0xBA, 0xBA, 0}, {0xD1, 0xD1, 0},
	{0xF1, 0xF1, 0}, {0xA2, 0xA2, 0}, {0xA1, 0xA1, 0}, {0xBF, 0xBF, 0},
	{0xFF, 0xFF, 0}, {0xDF, 0xDF, 0}, {0xD8, 0xD8, 0}, {0xF8, 0xF8, 0},
	{0xA4, 0xA4, 0}, {0xC6, 0xC6, 0}, {0xE6, 0xE6, 0}, {0x46, 0x45, 0},
	{0x49, 0x4A, 0}, {0x69, 0x6A, 0}, {0x50, 0x74, 0}, {0xA7, 0xA7, 0},
	{0xC7, 0xC7, 0}, {0xE7, 0xE7, 0}, {0xBD, 0xBD, 0}, {0xBC, 0xBC, 0},
	{0x66, 0x66, 0}, {0xB0, 0xB0, 0}, {0xB7, 0xB7, 0}, {0xA8, 0xA8, 0},
	{0xA5, 0xA5, 0}, {0xA2, 0xA2, 0}, {0xA3, 0xA3, 0}, {0xA3, 0xA3, 0},
	{0xAC, 0xAC, 1}, {0xAC, 0xAC, 0}, {0xAB, 0xAB, 0}, {0xBB, 0xBB, 0},
};

LPWORD lpBmp;
HDC hdcDib;
HFONT hFont[16];
HFONT hFontNarrow;

void draw(int c1, int c2, int mir, FILE* fo)
{
	if(c1 == c2) {
		for(int i = 14; i >= 7; i--) {
			memset(lpBmp, 0, 32 * 32 * 2);
			SelectObject(hdcDib, hFont[i]);
			char string[2];
			string[0] = c1;
			string[1] = '\0';
			ExtTextOut(hdcDib, 0, 0, NULL, NULL, string, 1, NULL);
			int w = 0;
			for(int y = 0; y < 18; y++) {
				LPWORD pat = &lpBmp[32 * (31 - y)];
				for(int x = 14; x < 32; x++) {
					if(pat[x]) {
						w = 1;
						goto exit;
					}
				}
			}
exit:
			if(!w) break;
		}
	}
	else {
		memset(lpBmp, 0, 32 * 32 * 2);
		SelectObject(hdcDib, hFont[7]);
		char string[2];
		string[0] = c1;
		string[1] = '\0';
		ExtTextOut(hdcDib, 0, 0, NULL, NULL, string, 1, NULL);
		string[0] = c2;
		ExtTextOut(hdcDib, 7, 0, NULL, NULL, string, 1, NULL);
	}
	if(mir) {
		WORD tmp[14];
		for(int y = 0; y < 18; y++) {
			LPWORD pat = &lpBmp[32 * (31 - y)];
			for(int x = 0; x < 14; x++)
				tmp[x] = pat[13 - x];
			for(int x = 0; x < 14; x++)
				pat[x] = tmp[x];
		}
	}
	for(int y = 0; y < 18; y++) {
		LPWORD pat = &lpBmp[32 * (31 - y)];
		int d;
#if 1
		d  = pat[ 0] ? 0x40 : 0;
		d |= pat[ 1] ? 0x20 : 0;
		d |= pat[ 2] ? 0x10 : 0;
		d |= pat[ 3] ? 0x08 : 0;
		d |= pat[ 4] ? 0x04 : 0;
		d |= pat[ 5] ? 0x02 : 0;
		d |= pat[ 6] ? 0x01 : 0;
		fputc(d, fo);
		d  = pat[ 7] ? 0x80 : 0;
		d |= pat[ 8] ? 0x40 : 0;
		d |= pat[ 9] ? 0x20 : 0;
		d |= pat[10] ? 0x10 : 0;
		d |= pat[11] ? 0x08 : 0;
		d |= pat[12] ? 0x04 : 0;
		d |= pat[13] ? 0x02 : 0;
		fputc(d, fo);
#else
		d  = pat[ 0] ? 0x02 : 0;
		d |= pat[ 1] ? 0x04 : 0;
		d |= pat[ 2] ? 0x08 : 0;
		d |= pat[ 3] ? 0x10 : 0;
		d |= pat[ 4] ? 0x20 : 0;
		d |= pat[ 5] ? 0x40 : 0;
		d |= pat[ 6] ? 0x80 : 0;
		fputc(d, fo);
		d  = pat[ 7] ? 0x01 : 0;
		d |= pat[ 8] ? 0x02 : 0;
		d |= pat[ 9] ? 0x04 : 0;
		d |= pat[10] ? 0x08 : 0;
		d |= pat[11] ? 0x10 : 0;
		d |= pat[12] ? 0x20 : 0;
		d |= pat[13] ? 0x40 : 0;
		fputc(d, fo);
#endif
	}
}

void main(int argc, char *argv[])
{
	// create dib section (32x32, 16bpp)
	LPBYTE lpBuf = (LPBYTE)GlobalAlloc(GPTR, sizeof(BITMAPINFO));
	LPBITMAPINFO lpDib = (LPBITMAPINFO)lpBuf;
	lpDib->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpDib->bmiHeader.biWidth = 32;
	lpDib->bmiHeader.biHeight = 32;
	lpDib->bmiHeader.biPlanes = 1;
	lpDib->bmiHeader.biBitCount = 16;
	lpDib->bmiHeader.biCompression = BI_RGB;
	lpDib->bmiHeader.biSizeImage = 0;
	lpDib->bmiHeader.biXPelsPerMeter = 0;
	lpDib->bmiHeader.biYPelsPerMeter = 0;
	lpDib->bmiHeader.biClrUsed = 0;
	lpDib->bmiHeader.biClrImportant = 0;
	HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
	HBITMAP hBmp = CreateDIBSection(hdc, lpDib, DIB_RGB_COLORS, (PVOID*)&lpBmp, NULL, 0);
	hdcDib = CreateCompatibleDC(hdc);
	SelectObject(hdcDib, hBmp);
	
	// create font (ms gothic, 16pt)
	LOGFONT logfont;
	logfont.lfEscapement = 0;
	logfont.lfOrientation = 0;
	logfont.lfWeight = FW_NORMAL;
	logfont.lfItalic = FALSE;
	logfont.lfUnderline = FALSE;
	logfont.lfStrikeOut = FALSE;
	logfont.lfCharSet = DEFAULT_CHARSET;	// EASTEUROPE_CHARSET
	logfont.lfOutPrecision = OUT_TT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = DEFAULT_QUALITY; 
	logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	strcpy(logfont.lfFaceName, argv[1]);
	logfont.lfHeight = 18;
	for(int i = 7; i <= 14; i++) {
		logfont.lfWidth = i;
		hFont[i] = CreateFontIndirect(&logfont);
	}
	SetBkMode(hdcDib, TRANSPARENT);
	SetTextColor(hdcDib, RGB(255, 255, 255));
	
	FILE *fo;
	fo = fopen("out.bin", "wb");
	for(int i = 0x20; i <= 0x7f; i++)
		draw(i, i, 0, fo);
	for(int i = 0; i < 32; i++) {
		int c1 = table[i][0];
		int c2 = table[i][1];
		int mir = table[i][2];
		draw(c1, c2, mir, fo);
	}
	for(int i = 32; i < 64; i++) {
		int c1 = table[i][0];
		int c2 = table[i][1];
		int mir = table[i][2];
		draw(c1, c2, mir, fo);
	}
	fclose(fo);
	
	// release
	for(int i = 7; i <= 14; i++)
		DeleteObject(hFont[i]);
	DeleteDC(hdcDib);
	DeleteObject(hBmp);
	GlobalFree(lpBuf);
}

