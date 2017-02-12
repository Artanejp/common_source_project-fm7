// ---------------------------------------------------------------------------
//	PSG-like sound generator
//	Copyright (C) cisc 1997, 1999.
// ---------------------------------------------------------------------------
//	$Id: psg.h,v 1.8 2003/04/22 13:12:53 cisc Exp $

#ifndef PSG_AY_3_891X_H
#define PSG_AY_3_891X_H

#include "types.h"
#include "psg.h"

#define PSG_SAMPLETYPE		int32		// int32 or int16

// ---------------------------------------------------------------------------
//	class PSG
//	PSG �ɗǂ��������𐶐����鉹�����j�b�g
//	
//	interface:
//	bool SetClock(uint clock, uint rate)
//		�������D���̃N���X���g�p����O�ɂ��Ȃ炸�Ă�ł������ƁD
//		PSG �̃N���b�N�� PCM ���[�g��ݒ肷��
//
//		clock:	PSG �̓���N���b�N
//		rate:	�������� PCM �̃��[�g
//		retval	�������ɐ�������� true
//
//	void Mix(Sample* dest, int nsamples)
//		PCM �� nsamples ���������C dest �Ŏn�܂�z��ɉ�����(���Z����)
//		�����܂ŉ��Z�Ȃ̂ŁC�ŏ��ɔz����[���N���A����K�v������
//	
//	void Reset()
//		���Z�b�g����
//
//	void SetReg(uint reg, uint8 data)
//		���W�X�^ reg �� data ����������
//	
//	uint GetReg(uint reg)
//		���W�X�^ reg �̓��e��ǂݏo��
//	
//	void SetVolume(int db_l, int db_r)
//		�e�����̉��ʂ𒲐߂���
//		�P�ʂ͖� 1/2 dB
//
class DLL_PREFIX PSG_AY_3_891X : public PSG
{
protected:
	uint32_t clock;
	uint32_t psgrate;
	int prescale;
public:
	PSG_AY_3_891X();
	~PSG_AY_3_891X();
	bool Init(uint c, uint r);
	void SetPrescaler(int factor);
	void SetVolume(int vol_l, int vol_r);
};

#endif // PSG_AY_3_891X_H
