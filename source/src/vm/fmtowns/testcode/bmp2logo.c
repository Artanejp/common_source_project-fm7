/*
version 2003.03.04.1
   BMP to TOWNS System Logo Converter
   
   by Kasanova

	�E16�F�r�b�g�}�b�v��RGB�v���[�������ɕϊ����܂�
	�E�r�b�g�}�b�v�̃T�C�Y�� 264x115 �ɂ��Ă�������
	�E�p���b�g���ۑ�����܂�
	�E�����̐F�����������Ȃ�Ȃ��悤�Ƀp���b�g�͒��ӂ���
	  ����U���Ă�������
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BMPHEADERSIZE     14

#define LOGO_USEPLANES     4
#define LOGO_W           264
#define LOGO_H           115
#define LOGO_PITCH       ((LOGO_W/8)*LOGO_USEPLANES)
#define LOGO_PITCH_PLANE (LOGO_W/8)

#if LOGO_USEPLANES==4
#define PAL_OFFSET 0x3b80
#else
#define PAL_OFFSET 0x2f00
#endif

char srcfile[128];
char dstfile[] = "fmt_sys4.lgo";
char buffer[2048];
char logobuffer[ (0x1000*4)-0x400 ];
FILE *fp, *fpw;


void pack8dot( unsigned int bmpptr, unsigned int brgptr )
{
	int j;
	char b,r,g;

#if LOGO_USEPLANES==4
	char i;
	i = 0;
#endif

	b = 0;
	r = 0;
	g = 0;

	for( j=0 ; j<4 ; j++ )
	{
		b <<= 1;
		r <<= 1;
		g <<= 1;
		b |= buffer[bmpptr+j]&0x10 ? 1 : 0;
		r |= buffer[bmpptr+j]&0x20 ? 1 : 0;
		g |= buffer[bmpptr+j]&0x40 ? 1 : 0;
		b <<= 1;
		r <<= 1;
		g <<= 1;
		b |= buffer[bmpptr+j]&0x1 ? 1 : 0;
		r |= buffer[bmpptr+j]&0x2 ? 1 : 0;
		g |= buffer[bmpptr+j]&0x4 ? 1 : 0;

#if LOGO_USEPLANES==4
		i <<= 1;
		i |= buffer[bmpptr+j]&0x80 ? 1 : 0;
		i <<= 1;
		i |= buffer[bmpptr+j]&0x8 ? 1 : 0;
#endif

	}

	logobuffer[4+ brgptr] = b;
	logobuffer[4+ brgptr+LOGO_PITCH_PLANE] = r;
	logobuffer[4+ brgptr+LOGO_PITCH_PLANE*2] = g;

#if LOGO_USEPLANES==4
	logobuffer[4+ brgptr+LOGO_PITCH_PLANE*3] = i;
#endif

}


int do_convert()
{
	int readsiz, w,h, x,y, pitch;
	unsigned int ptr;

	if( fread( buffer, 1, BMPHEADERSIZE, fp) != BMPHEADERSIZE )
	{
		puts( "�w�b�_�ǂݍ��ݎ��s(1)\n" );
		return( 1 );
	}

	if( (buffer[0]!='B')||(buffer[1]!='M') )
	{
		puts( "BMP�t�@�C���ł͂���܂���\n" );
		return( 1 );
	}

	/* BMPINFOHEADER */
	readsiz = *(int*)(buffer+10);
	readsiz -= BMPHEADERSIZE;
	if( fread( buffer, 1, readsiz, fp) != readsiz )
	{
		puts( "�w�b�_�ǂݍ��ݎ��s(2)\n" );
		return( 1 );
	}

	if( buffer[14] != 4 )
	{
		puts( "16�FBMP�ȊO�ɂ͑Ή����Ă��܂���\n" );
		return( 1 );
	}
	
	w = *(int*)(buffer+4);
	h = *(int*)(buffer+8);
	pitch = w/2;

	if( w != LOGO_W )
	{
		puts( "BMP�̐����T�C�Y���s���ł�\n" );
		return( 1 );
	}

	if( h != LOGO_H )
	{
		puts( "BMP�̐����T�C�Y���s���ł�\n" );
		return( 1 );
	}

	/* �摜�T�C�Y���R�s�[ */
	*(int*)logobuffer     = w;
	*(int*)(logobuffer+2) = h;

	/* �p���b�g���R�s�[(���@�ɂ͑��݂��܂���) */
	ptr = *(unsigned int*)buffer;
	for( y=0 ; y<16 ; y++ )
	{
		logobuffer[PAL_OFFSET+y*3  ] = buffer[ptr  ]; /* B */
		logobuffer[PAL_OFFSET+y*3+1] = buffer[ptr+2]; /* R */
		logobuffer[PAL_OFFSET+y*3+2] = buffer[ptr+1]; /* G */
		ptr += 4;
	}

	/* ������s���܂� */
	for( y=LOGO_H-1 ; y>=0 ; y-- )
	{
		if( fread( buffer, 1, pitch, fp) != pitch )
		{
			puts( "�ǂݍ��ݎ��s\n" );
			return( 1 );
		}

		ptr = LOGO_PITCH*y;
		for( x=0 ; x<pitch ; x+=4 )
		{
			pack8dot( x, ptr );
			ptr++;
		}
	}

	return( 0 );
}	


int main( int argc, char *argv[] )
{
	if( argc < 2 )
	{
		puts( "BMP�t�@�C�����w�肵�Ă�������\n" );
		return( 0 );
	}
	
	strcpy( srcfile, argv[1] );

	if( (fp=fopen(srcfile,"rb")) == NULL )
	{
		puts( "�ǂݍ��݃t�@�C���̃I�[�v���Ɏ��s���܂���\n" );
		return( 0 );
	}

	memset( logobuffer, 0xff, sizeof(logobuffer) );
	if( do_convert() ) puts( "�ϊ��ł��܂���ł���\n" );
	else 
	{
		if( (fpw=fopen(dstfile,"wb")) != NULL )
		{
			if( fwrite( logobuffer, 1, sizeof(logobuffer), fpw) == sizeof(logobuffer) )
			{
				puts( "����I��\n" );
			} else
			{
				puts( "�������݂Ɏ��s���܂���\n" );
			}
			fclose( fpw );
		} else
		{
			puts( "�������݃t�@�C���̃I�[�v���Ɏ��s���܂���\n" );
		}
	}
	
	fclose( fp );

	return( 0 );
}

