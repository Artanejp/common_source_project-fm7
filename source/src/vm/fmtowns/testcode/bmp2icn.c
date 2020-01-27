/*
version 2003.03.04.1
   BMP to TOWNS System Icon Data Converter
   
   by Kasanova
*/

/*
	�V�X�e���A�C�R���쐬�pBMP�̃t�H�[�}�b�g

	�EBMP�̃T�C�Y��512x544(���ɑ傫���Ԃ�ɂ͖��Ȃ�)
	�E256�FBMP�̂ݑΉ�
	�E�F�ԍ�0��1�A����ȊO�̐F�ԍ��͂��ׂ�0��2�l�ϊ������
	�E32x32�h�b�g�̃A�C�R���� ��8x�c16 �Ŕz�u
	�E��ԉ��̗�ɂ�32x32�h�b�g�̋N���p�A�C�R��������8�z�u
	�E[�p�^�[��]��[�}�X�N�p�^�[��]��2�����ɕ��ׂĔz�u���邱��
	�E�N���p�A�C�R���Ƀ}�X�N�p�^�[���͂���Ȃ�
	�E���������ĉ��̂悤�ȍ\���ɂȂ�
	
	----- 64�h�b�g -----
	[�p�^�[��0][�}�X�N0] [�p�^�[��1][�}�X�N1] [�p�^�[��2][�}�X�N2] [�p�^�[��3][�}�X�N3]�E�E�E
	[�p�^�[��8][�}�X�N8] [�p�^�[��9][�}�X�N9] [�p�^�[��a][�}�X�Na] [�p�^�[��b][�}�X�Nb]�E�E�E
	   �E
	   �E
	   �E
	[�p�^�[��0][�p�^�[��1][�p�^�[��2][�p�^�[��3][�p�^�[��4][�p�^�[��5][�p�^�[��6][�p�^�[��7]
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BMPHEADERSIZE    14
#define ICONMAX         128
#define ICONBOOTMAX       8
#define ICONPATTERNSIZE 256
#define ICON_W           32
#define ICON_H           32
#define ICONPITCH       (ICON_W/8)

#define ICONMATRIX_W   8
#define ICONMATRIX_H  (16+1)

char srcfile[128];
char dstfile[]  = "fmt_sys2.icn";
char dstfile2[] = "fmt_sys5.ic2";
char buffer[2048];
char iconbuffer [ ICONPATTERNSIZE*ICONMAX ]; /* 32768 */
char icon2buffer[ (ICONPATTERNSIZE/2)*ICONBOOTMAX ];
FILE *fp, *fpw, *fpw2;


void pack8dot( unsigned int bmpptr, unsigned int iconptr )
{
	int i;
	char c;
	
	c = 0;
	for( i=0 ; i<8 ; i++ )
	{
		c <<= 1;
		c |= buffer[bmpptr+i]==0 ? 1 : 0;
	}
	iconbuffer[iconptr] = c;
}


int do_convert()
{
	int readsiz, w,h, x,y,yy, pitch;
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

	if( buffer[14] != 8 )
	{
		puts( "256�FBMP�ȊO�ɂ͑Ή����Ă��܂���\n" );
		return( 1 );
	}
	
	w = *(int*)(buffer+4);
	h = *(int*)(buffer+8);
	pitch = w;

	if( w < ICON_W*ICONMATRIX_W )
	{
		puts( "BMP�̐����T�C�Y������܂���\n" );
		return( 1 );
	}

	if( h < ICON_H*ICONMATRIX_H )
	{
		puts( "BMP�̐����T�C�Y������܂���\n" );
		return( 1 );
	}

	if( pitch > sizeof(buffer) )
	{
		puts( "BMP�̐����T�C�Y���傫�����܂�\n" );
		return( 1 );
	}

	/* ������s���܂� */
	/* �u�[�g�A�C�R�� */
	for( yy=ICON_H-1 ; yy>=0 ; yy-- )
	{
		if( fread( buffer, 1, pitch, fp) != pitch )
		{
			puts( "�ǂݍ��ݎ��s\n" );
			return( 1 );
		}

		for( x=0 ; x<ICONBOOTMAX ; x++ )
		{
			ptr = ((ICONPATTERNSIZE/2)*x) + (yy*ICONPITCH);

			pack8dot( x*ICON_W   , ptr );
			pack8dot( x*ICON_W+8 , ptr+1 );
			pack8dot( x*ICON_W+16, ptr+2 );
			pack8dot( x*ICON_W+24, ptr+3 );
		}
	}
	memcpy( icon2buffer, iconbuffer, sizeof(icon2buffer) );

	/* �V�X�e���A�C�R�� */
	for( y=ICONMATRIX_H-2 ; y>=0 ; y-- )
	{
		for( yy=ICON_H-1 ; yy>=0 ; yy-- )
		{
			if( fread( buffer, 1, pitch, fp) != pitch )
			{
				puts( "�ǂݍ��ݎ��s\n" );
				return( 1 );
			}

			for( x=0 ; x<ICONMATRIX_W ; x++ )
			{
				ptr = (ICONPATTERNSIZE*(x+(y*ICONMATRIX_W)))
					 + (yy*ICONPITCH);

				/* �A�C�R�� */
				pack8dot( x*ICON_W*2   , ptr );
				pack8dot( x*ICON_W*2+8 , ptr+1 );
				pack8dot( x*ICON_W*2+16, ptr+2 );
				pack8dot( x*ICON_W*2+24, ptr+3 );

				/* �}�X�N */
				pack8dot( x*ICON_W*2+32, ptr+128 );
				pack8dot( x*ICON_W*2+40, ptr+129 );
				pack8dot( x*ICON_W*2+48, ptr+130 );
				pack8dot( x*ICON_W*2+56, ptr+131 );
			}
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

	if( do_convert() ) puts( "�ϊ��ł��܂���ł���\n" );
	else
	{
		/* �V�X�e���A�C�R���������� */
		if( (fpw=fopen(dstfile,"wb")) != NULL )
		{
			if( fwrite( iconbuffer, 1, sizeof(iconbuffer), fpw) != sizeof(iconbuffer) )
			{
				puts( "�������݂Ɏ��s���܂���(�V�X�e���A�C�R��)\n" );
			}
			fclose( fpw );
		} else
		{
			puts( "�������݃t�@�C���̃I�[�v���Ɏ��s���܂���(�V�X�e���A�C�R��)\n" );
		}

		/* �u�[�g�A�C�R���������� */
		if( (fpw2=fopen(dstfile2,"wb")) != NULL )
		{
			if( fwrite( icon2buffer, 1, sizeof(icon2buffer), fpw2) == sizeof(icon2buffer) )
			{
				puts( "����I��\n" );
			} else
			{
				puts( "�������݂Ɏ��s���܂���(�u�[�g�A�C�R��)\n" );
			}
			fclose( fpw2 );
		} else
		{
			puts( "�������݃t�@�C���̃I�[�v���Ɏ��s���܂���(�u�[�g�A�C�R��)\n" );
		}
	}
	fclose( fp );

	return( 0 );
}

