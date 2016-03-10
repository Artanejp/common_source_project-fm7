
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>

void sjis2jis(int *p1, int *p2) {
	int c = _mbcjmstojis(*p1 * 256 + *p2);
	*p1 = (c >> 8) & 0xff;
	*p2 = c & 0xff;
}

void main()
{
	unsigned char buf[0x20000];
	memset(buf, 0, sizeof(buf));
	
	FILE* fp = fopen("kanji.txt", "rb");
	if(fp == NULL) {
		printf("kanji.txt not found.\n");
		return;
	}
	
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
	
	int c1, c2, ofs;
	
	while((c1 = fgetc(fp)) != EOF) {
		c2 = fgetc(fp);
		if(!(c1 == 0x20 && c2 == 0x20) && !(c1 == 0x0d && c2 == 0x0a)) {
			// draw font
			memset(lpBmp, 0, 32 * 32 * 2);
			char string[3];
			string[0] = c1;
			string[1] = c2;
			string[2] = '\0';
			ExtTextOut(hdcDib, 0, 0, NULL, NULL, string, 2, NULL);
			
			// sjis to jis
			sjis2jis(&c1, &c2);
			printf("%2x %2x\n", c1, c2);
			
			// rom address
			if(0x20 <= c1 && c1 < 0x28) {
				//0010 0BBB 0CCD DDDD
				//00CC BBBD DDDD RRRR
				int b = c1 & 7;
				int c = (c2 >> 5) & 3;
				int d = c2 & 0x1f;
				ofs = (c << 13) | (b << 10) | (d << 5);
			}
			else {
				//0AAB BBBB 0CCD DDDD
				//CCBB BBBD DDDD RRRR
				int b = c1 & 0x1f;
				int c = (c2 >> 5) & 3;
				int d = c2 & 0x1f;
				ofs = (c << 15) | (b << 10) | (d << 5);
			}
			for(int y = 0; y < 16; y++) {
				LPWORD pat = &lpBmp[32 * (31 - y)];
				int d;
				d  = pat[ 0] ? 0x01 : 0;
				d |= pat[ 1] ? 0x02 : 0;
				d |= pat[ 2] ? 0x04 : 0;
				d |= pat[ 3] ? 0x08 : 0;
				d |= pat[ 4] ? 0x10 : 0;
				d |= pat[ 5] ? 0x20 : 0;
				d |= pat[ 6] ? 0x40 : 0;
				d |= pat[ 7] ? 0x80 : 0;
				buf[ofs++] = d;
				d  = pat[ 8] ? 0x01 : 0;
				d |= pat[ 9] ? 0x02 : 0;
				d |= pat[10] ? 0x04 : 0;
				d |= pat[11] ? 0x08 : 0;
				d |= pat[12] ? 0x10 : 0;
				d |= pat[13] ? 0x20 : 0;
				d |= pat[14] ? 0x40 : 0;
				d |= pat[15] ? 0x80 : 0;
				buf[ofs++] = d;
			}
		}
	}
	fclose(fp);
	
	// output
	fp = fopen("kanji.rom", "wb");
	fwrite(buf, sizeof(buf), 1, fp);
	fclose(fp);
	
	// release
	DeleteObject(hFont);
	DeleteDC(hdcDib);
	DeleteObject(hBmp);
	GlobalFree(lpBuf);
}

