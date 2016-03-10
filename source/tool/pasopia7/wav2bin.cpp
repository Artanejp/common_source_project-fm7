#include <windows.h>
#include <stdio.h>
#include <memory.h>

#define NARROW(p)	(6 <= (p) && (p) <= 14)
#define WIDE(p)		(16 <= (p) && (p) <= 80)

int fgetc2(FILE* fp)
{
	int dat;
	while((dat = fgetc(fp)) != EOF) {
		if(dat == 'N' || dat == 'w' || dat == ',')
			return dat;
	}
	return EOF;
}

void main(int argc, char **argv)
{
	if(argc < 1)
		return;
	
	// get path
	int dat, l = 0, h = 0, w = 0, n = 0;
	bool prev = true;
	FILE *fs, *fd;

	char root[_MAX_PATH], file[_MAX_PATH];
	char tmp[_MAX_PATH], dest[_MAX_PATH];
	GetModuleFileName(NULL, root, _MAX_PATH);
	int p = strlen(root);
	while(root[p] != '\\')
		p--;
	root[p] = '\0';

	strcpy(tmp, argv[1]);
	p = strlen(tmp);
	while(p != 0 && tmp[p] != '\\')
		p--;
	if(p)
		strcpy(file, &tmp[p + 1]);
	else
		strcpy(file, argv[1]);

	sprintf(tmp, "%s\\tmp.txt", root);
	sprintf(dest, "%s\\dest.bin", root);
	
	if(strcmp(file, "tmp.txt") == 0) {
		printf("skip wav->bit\n");
		goto label;
	}
	
	// convert #1 (wav -> bit)
	fs = fopen(argv[1], "rb");
	fd = fopen(tmp, "w");
	
	while((dat = fgetc(fs)) != EOF) {
		bool current = dat & 0x80 ? true : false;
		if(!prev && current) {
			if(NARROW(h) && (NARROW(l) || WIDE(l)))
				fputc('N', fd);
			else if(WIDE(h) && (WIDE(l) || NARROW(l)))
				fputc('w', fd);
			else
				fprintf(fd, "[%d,%d]", h, l);
			l = h = 0;
		}
		if(current)
			h++;
		else
			l++;
		prev = current;
	}
	fclose(fd);
	fclose(fs);
	
label:
	// convert #2 (bit -> byte)
	fs = fopen(tmp, "rb");
	fd = fopen(dest, "wb");
	
	while((dat = fgetc2(fs)) != EOF) {
		if(dat != 'N')
			continue;
		int d = 0;
		d |= (fgetc2(fs) == 'w') ? 0x01 : 0;
		d |= (fgetc2(fs) == 'w') ? 0x02 : 0;
		d |= (fgetc2(fs) == 'w') ? 0x04 : 0;
		d |= (fgetc2(fs) == 'w') ? 0x08 : 0;
		d |= (fgetc2(fs) == 'w') ? 0x10 : 0;
		d |= (fgetc2(fs) == 'w') ? 0x20 : 0;
		d |= (fgetc2(fs) == 'w') ? 0x40 : 0;
		d |= (fgetc2(fs) == 'w') ? 0x80 : 0;
		if(fgetc2(fs) != 'w')
			continue;
		fputc(d, fd);
	}
	fclose(fd);
	fclose(fs);
}
