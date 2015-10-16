
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

void main()
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
	LPWORD lpBmp;
	HBITMAP hBmp = CreateDIBSection(hdc, lpDib, DIB_RGB_COLORS, (PVOID*)&lpBmp, NULL, 0);
	HDC hdcDib = CreateCompatibleDC(hdc);
	SelectObject(hdcDib, hBmp);
	
	// create font (ms gothic, 16pt)
	LOGFONT logfont;
	logfont.lfEscapement = 0;
	logfont.lfOrientation = 0;
	logfont.lfWeight = FW_NORMAL;
	logfont.lfItalic = FALSE;
	logfont.lfUnderline = FALSE;
	logfont.lfStrikeOut = FALSE;
	logfont.lfCharSet = SHIFTJIS_CHARSET;
	logfont.lfOutPrecision = OUT_TT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = DEFAULT_QUALITY; 
	logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	strcpy(logfont.lfFaceName, "‚l‚r ƒSƒVƒbƒN");
//	strcpy(logfont.lfFaceName, "‚l‚r –¾’©");
	logfont.lfHeight = 16;
	logfont.lfWidth = 8;
	HFONT hFont = CreateFontIndirect(&logfont);
	SelectObject(hdcDib, hFont);
	SetBkMode(hdcDib, TRANSPARENT);
	SetTextColor(hdcDib, RGB(255, 255, 255));
	
	// convert font
	FILE* fs = fopen("kanji.txt", "rb");
	if(fs == NULL) {
		printf("kanji.txt not found.\n");
		return;
	}
	unsigned char lo[0x10000], hi[0x10000];
	int c1, ptr = 0;
	
	while((c1 = fgetc(fs)) != EOF) {
		memset(lpBmp, 0, 32 * 32 * 2);
		if(c1 == 0x0d || c1 == 0x0a) {
			fgetc(fs);
			continue;
		} else if((0x81 <= c1 && c1 <= 0x9f) || 0xe0 <= c1) {
			// kanji
			char string[3];
			string[0] = c1;
			string[1] = fgetc(fs);
			string[2] = '\0';
			ExtTextOut(hdcDib, 0, 0, NULL, NULL, string, 2, NULL);
		} else {
			// 1st
			char string[2];
			string[0] = c1;
			string[1] = '\0';
			ExtTextOut(hdcDib, 0, 0, NULL, NULL, string, 1, NULL);
			
			// 2nd
			string[0] = fgetc(fs);
			ExtTextOut(hdcDib, 8, 0, NULL, NULL, string, 1, NULL);
		}
		for(int y = 0; y < 16; y++) {
			if(ptr >= 0x10000) {
				continue;
			}
			LPWORD pat = &lpBmp[32 * (31 - y)];
			lo[ptr]  = pat[ 0] ? 0x80 : 0;
			lo[ptr] |= pat[ 1] ? 0x40 : 0;
			lo[ptr] |= pat[ 2] ? 0x20 : 0;
			lo[ptr] |= pat[ 3] ? 0x10 : 0;
			lo[ptr] |= pat[ 4] ? 0x08 : 0;
			lo[ptr] |= pat[ 5] ? 0x04 : 0;
			lo[ptr] |= pat[ 6] ? 0x02 : 0;
			lo[ptr] |= pat[ 7] ? 0x01 : 0;
			hi[ptr]  = pat[ 8] ? 0x80 : 0;
			hi[ptr] |= pat[ 9] ? 0x40 : 0;
			hi[ptr] |= pat[10] ? 0x20 : 0;
			hi[ptr] |= pat[11] ? 0x10 : 0;
			hi[ptr] |= pat[12] ? 0x08 : 0;
			hi[ptr] |= pat[13] ? 0x04 : 0;
			hi[ptr] |= pat[14] ? 0x02 : 0;
			hi[ptr] |= pat[15] ? 0x01 : 0;
			ptr++;
		}
	}
	fclose(fs);
	
	FILE* fd = fopen("KANJI.ROM", "wb");
	fwrite(lo + 0x0000, 0x8000, 1, fd);
	fwrite(hi + 0x0000, 0x8000, 1, fd);
	fwrite(lo + 0x8000, 0x8000, 1, fd);
	fwrite(hi + 0x8000, 0x8000, 1, fd);
	fclose(fd);
	
	// release
	DeleteObject(hFont);
	DeleteDC(hdcDib);
	DeleteObject(hBmp);
	GlobalFree(lpBuf);
}

