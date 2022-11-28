// ---------------------------------------------------------------------------
//	OPM-like Sound Generator
//	Copyright (C) cisc 1998, 2003.
// ---------------------------------------------------------------------------
//	$Id: opm.h,v 1.14 2003/06/07 08:25:53 cisc Exp $

#ifndef FM_OPM_H
#define FM_OPM_H

#include "fmgen.h"
#include "fmtimer.h"
#include "psg.h"

// ---------------------------------------------------------------------------
//	class OPM
//	OPM �ɗǂ�����(?)���𐶐����鉹�����j�b�g
//	
//	interface:
//	bool Init(uint clock, uint rate, bool);
//		�������D���̃N���X���g�p����O�ɂ��Ȃ炸�Ă�ł������ƁD
//		����: ���`�⊮���[�h�͔p�~����܂���
//
//		clock:	OPM �̃N���b�N���g��(Hz)
//
//		rate:	�������� PCM �̕W�{���g��(Hz)
//
//				
//		�Ԓl	�������ɐ�������� true
//
//	bool SetRate(uint clock, uint rate, bool)
//		�N���b�N�� PCM ���[�g��ύX����
//		�������� Init �Ɠ��l�D
//	
//	void Mix(Sample* dest, int nsamples)
//		Stereo PCM �f�[�^�� nsamples ���������C dest �Ŏn�܂�z���
//		������(���Z����)
//		�Edest �ɂ� sample*2 ���̗̈悪�K�v
//		�E�i�[�`���� L, R, L, R... �ƂȂ�D
//		�E�����܂ŉ��Z�Ȃ̂ŁC���炩���ߔz����[���N���A����K�v������
//		�EFM_SAMPLETYPE �� short �^�̏ꍇ�N���b�s���O���s����.
//		�E���̊֐��͉��������̃^�C�}�[�Ƃ͓Ɨ����Ă���D
//		  Timer �� Count �� GetNextEvent �ő��삷��K�v������D
//	
//	void Reset()
//		���������Z�b�g(������)����
//
//	void SetReg(uint reg, uint data)
//		�����̃��W�X�^ reg �� data ����������
//	
//	uint ReadStatus()
//		�����̃X�e�[�^�X���W�X�^��ǂݏo��
//		busy �t���O�͏�� 0
//	
//	bool ReadIRQ()
//		IRQ �o�͂�ǂݏo��
//	
//	bool Count(uint32 t)
//		�����̃^�C�}�[�� t [clock] �i�߂�D
//		�����̓�����Ԃɕω�����������(timer �I�[�o�[�t���[)
//		true ��Ԃ�
//
//	uint32 GetNextEvent()
//		�����̃^�C�}�[�̂ǂ��炩���I�[�o�[�t���[����܂łɕK�v��
//		����[clock]��Ԃ�
//		�^�C�}�[����~���Ă���ꍇ�� 0 ��Ԃ��D
//	
//	void SetVolume(int db_l, int db_r)
//		�e�����̉��ʂ��{�|�����ɒ��߂���D�W���l�� 0.
//		�P�ʂ͖� 1/2 dB�C�L���͈͂̏���� 20 (10dB)
//
namespace FM
{
	//	YM2151(OPM) ----------------------------------------------------
	class OPM : public Timer
	{
	public:
		OPM();
		~OPM() {}

		bool	Init(uint c, uint r, bool=false);
		bool	SetRate(uint c, uint r, bool);
		void	SetLPFCutoff(uint freq);
		void	Reset();
		bool	ReadIRQ();
		
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);
		uint	ReadStatus() { return status & 0x03; }
		
		void 	Mix(Sample* buffer, int nsamples);
		
		void	SetVolume(int db_l, int db_r);
		void	SetChannelMask(uint mask);
		
		bool ProcessState(void *f, bool loading);
		
	private:
		void	Intr(bool value);
	
	private:
		enum
		{
			OPM_LFOENTS = 512,
		};
		
		void	SetStatus(uint bit);
		void	ResetStatus(uint bit);
		void	SetParameter(uint addr, uint data);
		void	TimerA();
		void	RebuildTimeTable();
		void	MixSub(int activech, ISample**);
		void	MixSubL(int activech, ISample**);
		void	LFO();
		uint	Noise();
		
		int		fmvolume_l;
		int		fmvolume_r;

		uint	clock;
		uint	rate;
		uint	pcmrate;

		uint	pmd;
		uint	amd;
		uint	lfocount;
		uint	lfodcount;

		uint	lfo_count_;
		uint	lfo_count_diff_;
		uint	lfo_step_;
		uint	lfo_count_prev_;

		uint	lfowaveform;
		uint	rateratio;
		uint	noise;
		int32	noisecount;
		uint32	noisedelta;
		
		bool	interpolation;
		uint8	lfofreq;
		uint8	status;
		bool	interrupt;
		uint8	reg01;

		uint8	kc[8];
		uint8	kf[8];
		uint8	pan[8];

		Channel4 ch[8];
		Chip	chip;

		static void	BuildLFOTable();
		static int amtable[4][OPM_LFOENTS];
		static int pmtable[4][OPM_LFOENTS];

	public:
		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }

	};
}

#endif // FM_OPM_H
