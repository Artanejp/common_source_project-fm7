#include <windows.h>
#include <stdio.h>
#include <memory.h>

#define NARROW(p)	(6 <= (p) && (p) <= 14)
#define WIDE(p)		(16 <= (p) && (p) <= 24)

int fgetc2(FILE* fp)
{
	int dat;
	while((dat = fgetc(fp)) != EOF) {
		if(dat == 'N' || dat == 'w')
			return dat;
	}
	return EOF;
}

void fputc2(int dat, FILE* fp)
{
	static int head = 0, cnt = 0, sum = 0, total = 0;
	
	if(head++ < 6)
		return;
	if(sum) {
		sum--;
		return;
	}
	if(cnt++ < 255) {
		if(total++ < 0x8000)
			fputc(dat, fp);
	}
	else {
		sum = 2;
		cnt = 0;
	}
}

void main(int argc, char **argv)
{
	if(argc < 1)
		return;
	
	// get path
	int dat, l = 0, h = 0, w = 0, n = 0;
	bool prev = false;
	FILE *fs, *fd;

	char root[_MAX_PATH], file[_MAX_PATH];
	char tmp1[_MAX_PATH], tmp2[_MAX_PATH], dest[_MAX_PATH];
	GetModuleFileName(NULL, root, _MAX_PATH);
	int p = strlen(root);
	while(root[p] != '\\')
		p--;
	root[p] = '\0';

	strcpy(tmp1, argv[1]);
	p = strlen(tmp1);
	while(p != 0 && tmp1[p] != '\\')
		p--;
	if(p)
		strcpy(file, &tmp1[p + 1]);
	else
		strcpy(file, argv[1]);

	sprintf(tmp1, "%s\\tmp1.txt", root);
	sprintf(tmp2, "%s\\tmp2.txt", root);
	sprintf(dest, "%s\\dest.rom", root);
	
	if(strcmp(file, "tmp1.txt") == 0) {
		printf("skip wav->pulse\n");
		goto label1;
	}
	if(strcmp(file, "tmp2.txt") == 0) {
		printf("skip wav->pulse\n");
		printf("skip pulse->bit\n");
		goto label2;
	}
	
	// convert #1 (wav -> pulse)
	fs = fopen(argv[1], "rb");
	fd = fopen(tmp1, "w");
	
	while((dat = fgetc(fs)) != EOF) {
		bool current = dat & 0x80 ? false : true;
		if(prev && !current) {
			if(NARROW(l) && (NARROW(h) || WIDE(h)))
				fputc('N', fd);
			else if(WIDE(l) && (WIDE(h) || NARROW(h)))
				fputc('w', fd);
			else
				fprintf(fd, "[%d,%d]", l, h);
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
	
label1:
	
	// convert #2 (pulse -> bit)
	fs = fopen(tmp1, "rb");
	fd = fopen(tmp2, "w");
	prev = true;
	
	while((dat = fgetc(fs)) != EOF) {
		if(dat != 'w' && dat != 'N') {
			fputc(dat, fd);
			continue;
		}
		bool current = (dat == 'w') ? true : false;
		if(prev != current) {
			if(w)
				fprintf(fd, "[W%d]", w);
			if(n)
				fprintf(fd, "[n%d]", n);
			w = n = 0;
		}
		prev = current;
		
		if(current)
			w++;
		else
			n++;
		if(w == 2) {
			fputc('w', fd);
			w = n = 0;
		}
		if(n == 4) {
			fputc('N', fd);
			w = n = 0;
		}
	}
	fclose(fd);
	fclose(fs);
	
label2:
	
	// convert #3 (bit -> byte)
	fs = fopen(tmp2, "rb");
	fd = fopen(dest, "wb");
	
	while((dat = fgetc2(fs)) != EOF) {
		if(dat != 'N')
			continue;
		if(fgetc2(fs) != 'N')
			continue;
label3:
		dat = fgetc2(fs);
		if(dat == EOF)
			break;
		if(dat != 'w')
			goto label3;
		int d = 0;
		d |= (fgetc2(fs) == 'N') ? 0x01 : 0;
		d |= (fgetc2(fs) == 'N') ? 0x02 : 0;
		d |= (fgetc2(fs) == 'N') ? 0x04 : 0;
		d |= (fgetc2(fs) == 'N') ? 0x08 : 0;
		d |= (fgetc2(fs) == 'N') ? 0x10 : 0;
		d |= (fgetc2(fs) == 'N') ? 0x20 : 0;
		d |= (fgetc2(fs) == 'N') ? 0x40 : 0;
		d |= (fgetc2(fs) == 'N') ? 0x80 : 0;
		fputc2(d, fd);
	}
	fclose(fd);
	fclose(fs);
}
