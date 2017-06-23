
//
//
//
#pragma	once

#include <windows.h>

//#include "fmdll.h"

#define	SUPPORT_FM_1		0x00000001
#define	SUPPORT_FM_2		0x00000002
#define	SUPPORT_FM_3		0x00000004
#define	SUPPORT_FM_4		0x00000008
#define	SUPPORT_FM_5		0x00000010
#define	SUPPORT_FM_6		0x00000020
#define	SUPPORT_FM_7		0x00000040
#define	SUPPORT_FM_8		0x00000080
#define	SUPPORT_FM_A		0x00000007
#define	SUPPORT_FM_B		0x00000038
#define	SUPPORT_FM_C		0x000000c0	// OPM�p
#define	SUPPORT_FM			(SUPPORT_FM_A+SUPPORT_FM_B+SUPPORT_FM_C)
#define	SUPPORT_PSG_1		0x00000100
#define	SUPPORT_PSG_2		0x00000200
#define	SUPPORT_PSG_3		0x00000400
#define	SUPPORT_PSG_NOISE	0x00000800	// SN76489
#define	SUPPORT_PSG			0x00000700
#define	SUPPORT_SN76489		0x00000f00
#define	SUPPORT_RHYTHM		0x00001000
#define	SUPPORT_ADPCM_A		0x00002000	// YM2610(OPNB)/6ch
#define	SUPPORT_ADPCM_B		0x00004000
#define	SUPPORT_DAC			0x00008000
#define	SUPPORT_PCM86		0x00010000
#define	SUPPORT_WSS			0x00020000
#define	SUPPORT_MSM6258		0x00040000	// X68
#define	SUPPORT_RF5C68		0x00080000	// TOWNS
#define	SUPPORT_PSGPCM		0x00100000
#define	SUPPORT_BEEPPCM		0x00200000

#define	SUPPORT_CSM			0x00800000	// DLL����CSM���������邩
#define	SUPPORT_TIMER		0x01000000	// �^�C�}�[A/B
#define	SUPPORT_STATUS		0x02000000	// BUSY/TIMER/ID
#define	SUPPORT_REG			0x04000000	// GetReg
#define	SUPPORT_INFO		0x08000000	// fm_getver/fm_getauthor��
#define	SUPPORT_MULTIPLE	0x10000000	// ����chip


#define	SUPPORT_FAILED		0x80000000

#define	SUPPORT_YM2203		(SUPPORT_FM_A+SUPPORT_PSG)
#define	SUPPORT_OPN			SUPPORT_YM2203
#define	SUPPORT_YM2608		(SUPPORT_FM_A+SUPPORT_FM_B+SUPPORT_PSG+SUPPORT_RHYTHM+SUPPORT_ADPCM_B)
#define	SUPPORT_OPNA		SUPPORT_YM2608
#define	SUPPORT_YM2610		(SUPPORT_FM_A+SUPPORT_FM_4+SUPPORT_PSG+SUPPORT_PSG_NOISE+SUPPORT_ADPCM_A+SUPPORT_ADPCM_B)
#define	SUPPORT_YM2610B		(SUPPORT_FM_A+SUPPORT_FM_B+SUPPORT_PSG+SUPPORT_PSG_NOISE+SUPPORT_ADPCM_A+SUPPORT_ADPCM_B)
#define	SUPPORT_OPNB		SUPPORT_YM2610
#define	SUPPORT_YM2612		(SUPPORT_FM_A+SUPPORT_FM_B+SUPPORT_DAC)
#define	SUPPORT_OPN2		SUPPORT_YM2612
#define	SUPPORT_YM2151		SUPPORT_FM
#define	SUPPORT_OPM			SUPPORT_YM2151

#define	FMDLL_VER			100

typedef	DWORD (__cdecl *DLLFUNC0)(void);
typedef	DWORD (__cdecl *DLLFUNC1)(LPVOID);
typedef	DWORD (__cdecl *DLLFUNC2)(LPVOID, DWORD);
typedef	DWORD (__cdecl *DLLFUNC3)(LPVOID, DWORD, DWORD);

//	DLL���ł�CriticalSection���Ă���
//	#define USE_CS
#ifdef USE_CS
#define	CLI()	EnterCriticalSection(&cs)
#define	STI()	LeaveCriticalSection(&cs)
#else
#define	CLI()
#define	STI()
#endif

class CFMDLL
{
private:
	HMODULE	hDll;

#ifdef USE_CS
	CRITICAL_SECTION	cs;
#endif
	DLLFUNC3	fm_create;
	DLLFUNC1	fm_reset;
	DLLFUNC3	fm_setrate;
	DLLFUNC2	fm_setvolfm;
	DLLFUNC2	fm_setvolpsg;
	DLLFUNC2	fm_setvolrhythmtotal;
	DLLFUNC3	fm_setvolrhythm;
	DLLFUNC2	fm_setvoladpcm;
	DLLFUNC2	fm_setchannelmask;
	DLLFUNC3	fm_getpcm;
//	DLLFUNC2	fm_getpcmbuf;
	DLLFUNC3	fm_setreg;
	DLLFUNC2	fm_getreg;
	DLLFUNC2	fm_updatetimer;
	DLLFUNC1	fm_getnextevent;
	DLLFUNC1	fm_getadpcmbuf;
	DLLFUNC1	fm_release;
	DLLFUNC2	fm_getstatus;
	DLLFUNC1	fm_getcaps;
	DLLFUNC1	fm_getdllver;
	
public:
	CFMDLL(LPCTSTR lpFilename) {
		hDll = LoadLibrary(lpFilename);
		if (hDll != NULL) {
			fm_create = (DLLFUNC3) GetProcAddress(hDll, "fm_create");
			fm_setrate = (DLLFUNC3) GetProcAddress(hDll, "fm_setrate");
			fm_reset = (DLLFUNC1) GetProcAddress(hDll, "fm_reset");
			fm_setvolfm = (DLLFUNC2) GetProcAddress(hDll, "fm_setvolfm");
			fm_setvolpsg = (DLLFUNC2) GetProcAddress(hDll, "fm_setvolpsg");
			fm_setvolrhythmtotal = (DLLFUNC2) GetProcAddress(hDll, "fm_setvolrhythmtotal");
			fm_setvolrhythm = (DLLFUNC3) GetProcAddress(hDll, "fm_setvolrhythm");
			fm_setvoladpcm = (DLLFUNC2) GetProcAddress(hDll, "fm_setvoladpcm");
			
			fm_setchannelmask = (DLLFUNC2) GetProcAddress(hDll, "fm_setchannelmask");
			fm_setreg = (DLLFUNC3) GetProcAddress(hDll, "fm_setreg");
			fm_getreg = (DLLFUNC2) GetProcAddress(hDll, "fm_getreg");
			fm_updatetimer = (DLLFUNC2) GetProcAddress(hDll, "fm_updatetimer");
			fm_getnextevent = (DLLFUNC1) GetProcAddress(hDll, "fm_getnextevent");
			fm_getadpcmbuf = (DLLFUNC1) GetProcAddress(hDll, "fm_getadpcmbuf");
			fm_getpcm = (DLLFUNC3) GetProcAddress(hDll, "fm_getpcm");
//			fm_getpcmbuf = (DLLFUNC2) GetProcAddress(hDll, "fm_getpcmbuf");
			fm_release = (DLLFUNC1) GetProcAddress(hDll, "fm_release");
			fm_getstatus = (DLLFUNC2) GetProcAddress(hDll, "fm_getstatus");
			fm_getcaps = (DLLFUNC1) GetProcAddress(hDll, "fm_getcaps");
			fm_getdllver = (DLLFUNC1) GetProcAddress(hDll, "fm_getdllver");
			
			//	���̊֐�������̂�V1�ȍ~
			if (fm_getdllver == NULL) {
				FreeLibrary(hDll);
				hDll = NULL;
			}
#ifdef USE_CS
			else {
				InitializeCriticalSection(&cs);
			}
#endif
		}
	}
	
	~CFMDLL() {
		if (hDll != NULL) {
#ifdef USE_CS
			DeleteCriticalSection(&cs);
#endif
			FreeLibrary(hDll);
			hDll = NULL;
		}
	}

	//	DLL�̑��݃t���O
	BOOL IsDll(void) {
		return (hDll != NULL ? TRUE : FALSE);
	}

	//	�����쐬(&chip�Ȃ̂ɒ���)
	//	chip�ɂ�NULL�����Ă�����
	BOOL Create(LPVOID *chip, int clock, int rate) {
		BOOL	bRet = FALSE;
		if (hDll != NULL && fm_create != NULL) {
			CLI();
			bRet = fm_create(chip, clock, rate);
//			LPVOID	lpv;
//			bRet = fm_create(&lpv, clock, rate);
//			*chip = lpv;
#if 0
			//	chip�̓��e�Ŕ��f
			if (!bRet) {
				FreeLibrary(hDll);
				hDll = NULL;
			}
#endif
			STI();
		}
		return bRet;
	}
		
	//	�������Z�b�g
	void Reset(LPVOID chip) {
		if (hDll != NULL && fm_reset != NULL) {
			CLI();
			fm_reset(chip);
			STI();
		}
	}
	
	//	�������[�g�ݒ�
	void SetRate(LPVOID chip, int clock, int rate) {
		if (hDll != NULL && fm_setrate != NULL) {
			CLI();
			fm_setrate(chip, clock, rate);
			STI();
		}
	}
	
	//	���W�X�^�ݒ�
	void SetReg(LPVOID chip, UINT adr, BYTE data) {
		if (hDll != NULL && fm_setreg != NULL) {
			CLI();
			fm_setreg(chip, adr, data);
			STI();
		}
	}
	
	//	���W�X�^�ݒ�
	DWORD GetReg(LPVOID chip, UINT adr) {
		DWORD	dwRet = 0;
		if (hDll != NULL && fm_getreg != NULL) {
			CLI();
			dwRet = fm_getreg(chip, adr);
			STI();
		}
		return dwRet;
	}
	
	//	FM�����{�����[���ݒ�(fmgen�݊�)
	void SetVolumeFM(LPVOID chip, int vol) {
		if (hDll != NULL && fm_setvolfm != NULL) {
			CLI();
			fm_setvolfm(chip, vol);
			STI();
		}
	}
	
	//	PSG�����{�����[���ݒ�(fmgen�݊�)
	void SetVolumePSG(LPVOID chip, int vol) {
		if (hDll != NULL && fm_setvolpsg != NULL) {
			CLI();
			fm_setvolpsg(chip, vol);
			STI();
		}
	}
	
	//	���Y�������S�̃{�����[���ݒ�(fmgen�݊�)
	void SetVolumeRhythmTotal(LPVOID chip, int vol) {
		if (hDll != NULL && fm_setvolrhythmtotal != NULL) {
			CLI();
			fm_setvolrhythmtotal(chip, vol);
			STI();
		}
	}
	
	//	�e���Y�������{�����[���ݒ�(fmgen�݊�)
	void SetVolumeRhythm(LPVOID chip, int no, int vol) {
		if (hDll != NULL && fm_setvolrhythm != NULL) {
			CLI();
			fm_setvolrhythm(chip, no, vol);
			STI();
		}
	}

	//	ADPCM�{�����[���ݒ�(fmgen�݊�)
	void SetVolumeADPCM(LPVOID chip, int vol) {
		if (hDll != NULL && fm_setvoladpcm != NULL) {
			CLI();
			fm_setvoladpcm(chip, vol);
			STI();
		}
	}
	
	//	�`�����l���}�X�N(fmgen�݊�)
	void SetChannelMask(LPVOID chip, int mask) {
		if (hDll != NULL && fm_setchannelmask != NULL) {
			CLI();
			fm_setchannelmask(chip, mask);
			STI();
		}
	}
	
	//	�^�C�}�X�V(fmgen�݊�)
	void Count(LPVOID chip, DWORD us) {
		if (hDll != NULL && fm_updatetimer != NULL) {
			CLI();
			fm_updatetimer(chip, us);
			STI();
		}
	}
	
	//	���^�C�}�[�C�x���g�܂ł̎��Ԏ擾(fmgen�݊�)
	DWORD GetNextEvent(LPVOID chip) {
		DWORD	dwRet = 0xffffffff;
		if (hDll != NULL && fm_getnextevent != NULL) {
			CLI();
			dwRet = fm_getnextevent(chip);
			STI();
		}
		return dwRet;
	}
	
	//	����
	void Mix(LPVOID chip, int *pcm, int samples) {
		if (hDll != NULL && fm_getpcm != NULL) {
			CLI();
			fm_getpcm(chip, (DWORD) pcm, samples);
			STI();
		}
	}
#if 0
	//	�e�`�����l��PCM�f�[�^�擾(���m����)
	int* GetBuffer(LPVOID chip, int no) {
		int	*p = NULL;
		if (hDll != NULL && fm_getpcmbuf != NULL) {
			CLI();
			p = (int*) fm_getpcmbuf(chip, no);
			STI();
		}
		return p;
	}
#endif
	//	ADPCM�o�b�t�@(256KB)
	//	buf��buffer�����݂��Ă�c
	LPBYTE GetADPCMBuffer(LPVOID chip) {
		LPBYTE	p = NULL;
		if (hDll != NULL && fm_getadpcmbuf != NULL) {
			CLI();
			p = (LPBYTE) fm_getadpcmbuf(chip);
			STI();
		}
		return p;
	}
	//	�J��
	void Release(LPVOID chip) {
		if (hDll != NULL && fm_release != NULL) {
			CLI();
			fm_release(chip);
//			chip = NULL;
			STI();
		}
	}
	
	DWORD GetStatus(LPVOID chip, DWORD adr) {
		DWORD	dwRet = 0xff;
		if (hDll != NULL && fm_getstatus != NULL) {
			CLI();
			dwRet = fm_getstatus(chip, adr);
			STI();
		}
		return dwRet;
	}
	
	//	�T�|�[�g���Ă���@�\��Ԃ�
	DWORD GetCaps(LPVOID chip) {
		DWORD	dwRet = 0;
		if (hDll != NULL && fm_getcaps != NULL) {
			CLI();
			dwRet = fm_getcaps(chip);
			STI();
		}
		return dwRet;
	}

};

#undef	CLI
#undef	STI
