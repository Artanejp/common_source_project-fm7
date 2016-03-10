
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

	FILE* fd = fopen("kanji.rom", "wb");
	int c1;
	while((c1 = fgetc(fs)) != EOF) {
		memset(lpBmp, 0, 32 * 32 * 2);

		if(c1 == 0x0d || c1 == 0x0a) {
			fgetc(fs);
			continue;
		}
		else if((0x81 <= c1 && c1 <= 0x9f) || 0xe0 <= c1) {
			// kanji
			char string[3];
			string[0] = c1;
			string[1] = fgetc(fs);
			string[2] = '\0';
			ExtTextOut(hdcDib, 0, 0, NULL, NULL, string, 2, NULL);
		}
		else {
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
			LPWORD pat = &lpBmp[32 * (31 - y)];
			int d;
			d  = pat[ 0] ? 0x80 : 0;
			d |= pat[ 1] ? 0x40 : 0;
			d |= pat[ 2] ? 0x20 : 0;
			d |= pat[ 3] ? 0x10 : 0;
			d |= pat[ 4] ? 0x08 : 0;
			d |= pat[ 5] ? 0x04 : 0;
			d |= pat[ 6] ? 0x02 : 0;
			d |= pat[ 7] ? 0x01 : 0;
			fputc(d, fd);
			d  = pat[ 8] ? 0x80 : 0;
			d |= pat[ 9] ? 0x40 : 0;
			d |= pat[10] ? 0x20 : 0;
			d |= pat[11] ? 0x10 : 0;
			d |= pat[12] ? 0x08 : 0;
			d |= pat[13] ? 0x04 : 0;
			d |= pat[14] ? 0x02 : 0;
			d |= pat[15] ? 0x01 : 0;
			fputc(d, fd);
		}
	}
	fclose(fd);
	fclose(fs);
	
	// release
	DeleteObject(hFont);
	DeleteDC(hdcDib);
	DeleteObject(hBmp);
	GlobalFree(lpBuf);
}

