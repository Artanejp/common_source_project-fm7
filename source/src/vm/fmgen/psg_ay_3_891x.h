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
//	PSG に良く似た音を生成する音源ユニット
//	
//	interface:
//	bool SetClock(uint clock, uint rate)
//		初期化．このクラスを使用する前にかならず呼んでおくこと．
//		PSG のクロックや PCM レートを設定する
//
//		clock:	PSG の動作クロック
//		rate:	生成する PCM のレート
//		retval	初期化に成功すれば true
//
//	void Mix(Sample* dest, int nsamples)
//		PCM を nsamples 分合成し， dest で始まる配列に加える(加算する)
//		あくまで加算なので，最初に配列をゼロクリアする必要がある
//	
//	void Reset()
//		リセットする
//
//	void SetReg(uint reg, uint8 data)
//		レジスタ reg に data を書き込む
//	
//	uint GetReg(uint reg)
//		レジスタ reg の内容を読み出す
//	
//	void SetVolume(int db_l, int db_r)
//		各音源の音量を調節する
//		単位は約 1/2 dB
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
