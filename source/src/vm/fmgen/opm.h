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

class CSP_Logger;
// ---------------------------------------------------------------------------
//	class OPM
//	OPM に良く似た(?)音を生成する音源ユニット
//	
//	interface:
//	bool Init(uint clock, uint rate, bool);
//		初期化．このクラスを使用する前にかならず呼んでおくこと．
//		注意: 線形補完モードは廃止されました
//
//		clock:	OPM のクロック周波数(Hz)
//
//		rate:	生成する PCM の標本周波数(Hz)
//
//				
//		返値	初期化に成功すれば true
//
//	bool SetRate(uint clock, uint rate, bool)
//		クロックや PCM レートを変更する
//		引数等は Init と同様．
//	
//	void Mix(Sample* dest, int nsamples)
//		Stereo PCM データを nsamples 分合成し， dest で始まる配列に
//		加える(加算する)
//		・dest には sample*2 個分の領域が必要
//		・格納形式は L, R, L, R... となる．
//		・あくまで加算なので，あらかじめ配列をゼロクリアする必要がある
//		・FM_SAMPLETYPE が short 型の場合クリッピングが行われる.
//		・この関数は音源内部のタイマーとは独立している．
//		  Timer は Count と GetNextEvent で操作する必要がある．
//	
//	void Reset()
//		音源をリセット(初期化)する
//
//	void SetReg(uint reg, uint data)
//		音源のレジスタ reg に data を書き込む
//	
//	uint ReadStatus()
//		音源のステータスレジスタを読み出す
//		busy フラグは常に 0
//	
//	bool ReadIRQ()
//		IRQ 出力を読み出す
//	
//	bool Count(uint32 t)
//		音源のタイマーを t [clock] 進める．
//		音源の内部状態に変化があった時(timer オーバーフロー)
//		true を返す
//
//	uint32 GetNextEvent()
//		音源のタイマーのどちらかがオーバーフローするまでに必要な
//		時間[clock]を返す
//		タイマーが停止している場合は 0 を返す．
//	
//	void SetVolume(int db_l, int db_r)
//		各音源の音量を＋−方向に調節する．標準値は 0.
//		単位は約 1/2 dB，有効範囲の上限は 20 (10dB)
//
namespace FM
{
	//	YM2151(OPM) ----------------------------------------------------
	class DLL_PREFIX  OPM : public Timer
	{
	protected:
		int chip_num;
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

		bool	ProcessState(void *f, bool loading);
		
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

		__DECL_ALIGNED(16) uint8	kc[8];
		__DECL_ALIGNED(16) uint8	kf[8];
		__DECL_ALIGNED(16) uint8	pan[8];

		__DECL_ALIGNED(16) Channel4 ch[8];
		Chip	chip;

		static void	BuildLFOTable();
		__DECL_ALIGNED(16) static int amtable[4][OPM_LFOENTS];
		__DECL_ALIGNED(16) static int pmtable[4][OPM_LFOENTS];

	public:
		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }

	};
}

#endif // FM_OPM_H
