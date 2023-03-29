// ---------------------------------------------------------------------------
//	OPN/A/B interface with ADPCM support
//	Copyright (C) cisc 1998, 2003.
// ---------------------------------------------------------------------------
//	$Id: opna.h,v 1.33 2003/06/12 13:14:37 cisc Exp $

#ifndef FM_OPNA_H
#define FM_OPNA_H

#include "../../common.h"
#include "fmgen.h"
#include "fmtimer.h"
#include "psg.h"

// ---------------------------------------------------------------------------
//	class OPN/OPNA
//	OPN/OPNA に良く似た音を生成する音源ユニット
//
//	interface:
//	bool Init(uint clock, uint rate, bool, const _TCHAR* path);
//		初期化．このクラスを使用する前にかならず呼んでおくこと．
//		OPNA の場合はこの関数でリズムサンプルを読み込む
//
//		clock:	OPN/OPNA/OPNB のクロック周波数(Hz)
//
//		rate:	生成する PCM の標本周波数(Hz)
//
//		path:	リズムサンプルのパス(OPNA のみ有効)
//				省略時はカレントディレクトリから読み込む
//				文字列の末尾には '\' や '/' などをつけること
//
//		返り値	初期化に成功すれば true
//
//	bool LoadRhythmSample(const _TCHAR* path)
//		(OPNA ONLY)
//		Rhythm サンプルを読み直す．
//		path は Init の path と同じ．
//
//	bool SetRate(uint clock, uint rate, bool)
//		クロックや PCM レートを変更する
//		引数等は Init を参照のこと．
//
//	void Mix(FM_SAMPLETYPE* dest, int nsamples)
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
//	uint GetReg(uint reg)
//		音源のレジスタ reg の内容を読み出す
//		読み込むことが出来るレジスタは PSG, ADPCM の一部，ID(0xff) とか
//
//	uint ReadStatus()/ReadStatusEx()
//		音源のステータスレジスタを読み出す
//		ReadStatusEx は拡張ステータスレジスタの読み出し(OPNA)
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
//		タイマーが停止している場合は ULONG_MAX を返す… と思う
//
//	void SetVolumeFM(int db_l, int db_r)/SetVolumePSG(int db_l, int db_r) ...
//		各音源の音量を＋−方向に調節する．標準値は 0.
//		単位は約 1/2 dB，有効範囲の上限は 20 (10dB)
//
class CSP_Logger;
namespace FM
{
	//	OPN Base -------------------------------------------------------
	class DLL_PREFIX OPNBase : public Timer
	{
	protected:
		int chip_num;
	public:
		OPNBase();
		virtual ~OPNBase() {}

		bool	Init(uint c, uint r);
		virtual void Reset();
		bool	ReadIRQ();

		void	SetVolumeFM(int db_l, int db_r);
		void	SetVolumePSG(int db_l, int db_r);
		void	SetLPFCutoff(uint freq) {}	// obsolete
		uint	GetPrescaler() { return prescale; }
		bool	is_ay3_891x;

	protected:
		void	SetParameter(Channel4* ch, uint addr, uint data);
		virtual void	SetPrescaler(uint p);
		void	RebuildTimeTable();
		void	Intr(bool value);

		bool ProcessState(void *f, bool loading);

		int		fmvolume_l;
		int		fmvolume_r;

		uint	clock;				// OPN クロック
		uint	rate;				// FM 音源合成レート
		uint	psgrate;			// FMGen  出力レート
		uint	status;
		bool	interrupt;
		Channel4* csmch;


		__DECL_ALIGNED(16) static  uint32 lfotable[8];
	private:
		void	TimerA();

	protected:
		uint8	prescale;
		Chip	chip;
		PSG		psg;
	};

	//	OPNA Base ------------------------------------------------------
	class DLL_PREFIX OPNABase : public OPNBase
	{
	public:
		OPNABase();
		~OPNABase();

		uint	ReadStatus() { return status & 0x03; }
		uint	ReadStatusEx();
		void	SetChannelMask(uint mask);

	private:
		void	MakeTable2();

	protected:
		bool	Init(uint c, uint r, bool);
		bool	SetRate(uint c, uint r, bool);

		void	Reset();
		void 	SetReg(uint addr, uint data);
		void	SetADPCMBReg(uint reg, uint data);
		uint	GetReg(uint addr);

		bool ProcessState(void *f, bool loading);

	protected:
		void	FMMix(Sample* buffer, int nsamples);
		void 	Mix6(Sample* buffer, int nsamples, int activech);

		void	MixSubS(int activech, ISample**);
		void	MixSubSL(int activech, ISample**);

		void	SetStatus(uint bit);
		void	ResetStatus(uint bit);
		void	UpdateStatus();
		void	LFO();

		void	DecodeADPCMB();
		void	ADPCMBMix(Sample* dest, uint count);

		void	WriteRAM(uint data);
		uint	ReadRAM();
		int		ReadRAMN();
		int		DecodeADPCMBSample(uint);

	// FM 音源関係
		__DECL_ALIGNED(16) uint8	pan[6];
		__DECL_ALIGNED(16) uint8	fnum2[9];

		uint8	reg22;
		uint	reg29;		// OPNA only?

		uint	stmask;
		uint	statusnext;

		uint32	lfocount;
		uint32	lfodcount;

		__DECL_ALIGNED(16) uint	fnum[6];
		__DECL_ALIGNED(16) uint	fnum3[3];

	// ADPCM 関係
		uint8*	adpcmbuf;		// ADPCM RAM
		uint	adpcmmask;		// メモリアドレスに対するビットマスク
		uint	adpcmnotice;	// ADPCM 再生終了時にたつビット
		uint	startaddr;		// Start address
		uint	stopaddr;		// Stop address
		uint	memaddr;		// 再生中アドレス
		uint	limitaddr;		// Limit address/mask
		int		adpcmlevel;		// ADPCM 音量
		int		adpcmvolume_l;
		int		adpcmvolume_r;
		int		adpcmvol_l;
		int		adpcmvol_r;
		uint	deltan;			// N
		int		adplc;			// 周波数変換用変数
		int		adpld;			// 周波数変換用変数差分値
		uint	adplbase;		// adpld の元
		int		adpcmx;			// ADPCM 合成用 x
		int		adpcmd;			// ADPCM 合成用
		int		adpcmout_l;		// ADPCM 合成後の出力
		int		adpcmout_r;		// ADPCM 合成後の出力
		int		apout0_l;			// out(t-2)+out(t-1)
		int		apout0_r;			// out(t-2)+out(t-1)
		int		apout1_l;			// out(t-1)+out(t)
		int		apout1_r;			// out(t-1)+out(t)

		uint	adpcmreadbuf;	// ADPCM リード用バッファ
		bool	adpcmplay;		// ADPCM 再生中
		int8	granuality;
		bool	adpcmmask_;

		uint8	control1;		// ADPCM コントロールレジスタ１
		uint8	control2;		// ADPCM コントロールレジスタ２
		__DECL_ALIGNED(16) uint8	adpcmreg[8];	// ADPCM レジスタの一部分

		int		rhythmmask_;

		__DECL_ALIGNED(16) Channel4 ch[6];

		static void	BuildLFOTable();
		__DECL_ALIGNED(16) static int amtable[FM_LFOENTS];
		__DECL_ALIGNED(16) static int pmtable[FM_LFOENTS];
		__DECL_ALIGNED(16) static int32 tltable[FM_TLENTS+FM_TLPOS];
		static bool	tablehasmade;
	};

	//	OPN2 Base ------------------------------------------------------
	class DLL_PREFIX OPN2Base : public OPNBase
	{
	public:
		OPN2Base();
		~OPN2Base();

		uint	ReadStatus() { return status & 0x7f; }
		uint	ReadStatusEx()  { return 0xff; }
		void	SetChannelMask(uint mask);

	private:
		void	MakeTable2();

	protected:
		bool	Init(uint c, uint r, bool);
		bool	SetRate(uint c, uint r, bool);
		virtual void SetPrescaler(uint p);

		void	Reset();
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);

		bool ProcessState(void *f, bool loading);

	protected:
		void	FMMix(Sample* buffer, int nsamples);
		void 	Mix6(Sample* buffer, int nsamples, int activech);

		void	MixSubS(int activech, ISample**);
		void	MixSubSL(int activech, ISample**);

		void	SetStatus(uint bit);
		void	ResetStatus(uint bit);
		void	UpdateStatus();
		void	LFO();

	// FM 音源関係
		__DECL_ALIGNED(16) uint8	pan[6];
		__DECL_ALIGNED(16) uint8	fnum2[9];

		uint8	reg22;
		uint	reg29;		// OPNA only?

		uint	stmask;

		uint32	lfocount;
		uint32	lfodcount;

		__DECL_ALIGNED(16) uint	fnum[6];
		__DECL_ALIGNED(16) uint	fnum3[3];

		__DECL_ALIGNED(16) Channel4 ch[6];
		int32   dac_data;
		bool    dac_enabled;

		static void	BuildLFOTable();
		__DECL_ALIGNED(16) static int amtable[FM_LFOENTS];
		__DECL_ALIGNED(16) static int pmtable[FM_LFOENTS];
		__DECL_ALIGNED(16) static int32 tltable[FM_TLENTS+FM_TLPOS];
		static bool	tablehasmade;
	};
	//	YM2203(OPN) ----------------------------------------------------
	class DLL_PREFIX OPN : public OPNBase
	{
	public:
		OPN();
		virtual ~OPN() {}

		bool	Init(uint c, uint r, bool=false, const char* =0);
		bool	SetRate(uint c, uint r, bool=false);

		void	Reset();
		void 	Mix(Sample* buffer, int nsamples);
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);
		uint	ReadStatus() { return status & 0x03; }
		uint	ReadStatusEx() { return 0xff; }

		void	SetChannelMask(uint mask);

		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		int		dbgGetPGOut(int c, int s) { return ch[c].op[s].dbgpgout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }

		bool ProcessState(void *f, bool loading);

	private:
		void	SetStatus(uint bit);
		void	ResetStatus(uint bit);

		__DECL_ALIGNED(16) uint	fnum[3];
		__DECL_ALIGNED(16) uint	fnum3[3];
		__DECL_ALIGNED(16) uint8	fnum2[6];

		__DECL_ALIGNED(16) Channel4 ch[3];
	};

	//	YM2608(OPNA) ---------------------------------------------------
	class DLL_PREFIX OPNA : public OPNABase
	{
	public:
		OPNA();
		virtual ~OPNA();

		bool	Init(uint c, uint r, bool  = false, const _TCHAR* rhythmpath=0);
		bool	LoadRhythmSample(const _TCHAR*);

		bool	SetRate(uint c, uint r, bool = false);
		void 	Mix(Sample* buffer, int nsamples);

		void	Reset();
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);

		void	SetVolumeADPCM(int db_l, int db_r);
		void	SetVolumeRhythmTotal(int db_l, int db_r);
		void	SetVolumeRhythm(int index, int db_l, int db_r);

		uint8*	GetADPCMBuffer() { return adpcmbuf; }

		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		int		dbgGetPGOut(int c, int s) { return ch[c].op[s].dbgpgout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }

		bool ProcessState(void *f, bool loading);

	private:
		struct Rhythm
		{
			uint8	pan;		// ぱん
			int8	level;		// おんりょう
			int		volume_l;		// おんりょうせってい
			int		volume_r;		// おんりょうせってい
			int16*	sample;		// さんぷる
			uint	size;		// さいず
			uint	pos;		// いち
			uint	step;		// すてっぷち
			uint	rate;		// さんぷるのれーと
		};

		void	RhythmMix(Sample* buffer, uint count);

	// リズム音源関係
		__DECL_ALIGNED(16) Rhythm	rhythm[6];
		int8	rhythmtl;		// リズム全体の音量
		int		rhythmtvol_l;
		int		rhythmtvol_r;
		uint8	rhythmkey;		// リズムのキー
	};

	//	YM2610/B(OPNB) ---------------------------------------------------
	class DLL_PREFIX OPNB : public OPNABase
	{
	public:
		OPNB();
		virtual ~OPNB();

		bool	Init(uint c, uint r, bool = false,
					 uint8 *_adpcma = 0, int _adpcma_size = 0,
					 uint8 *_adpcmb = 0, int _adpcmb_size = 0);

		bool	SetRate(uint c, uint r, bool = false);
		void 	Mix(Sample* buffer, int nsamples);

		void	Reset();
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);
		uint	ReadStatusEx();

		void	SetVolumeADPCMATotal(int db_l, int db_r);
		void	SetVolumeADPCMA(int index, int db_l, int db_r);
		void	SetVolumeADPCMB(int db_l, int db_r);

//		void	SetChannelMask(uint mask);

	private:
		struct ADPCMA
		{
			uint8	pan;		// ぱん
			int8	level;		// おんりょう
			int		volume_l;		// おんりょうせってい
			int		volume_r;		// おんりょうせってい
			uint	pos;		// いち
			uint	step;		// すてっぷち

			uint	start;		// 開始
			uint	stop;		// 終了
			uint	nibble;		// 次の 4 bit
			int		adpcmx;		// 変換用
			int		adpcmd;		// 変換用
		};

		int		DecodeADPCMASample(uint);
		void	ADPCMAMix(Sample* buffer, uint count);
		static void InitADPCMATable();

	// ADPCMA 関係
		uint8*	adpcmabuf;		// ADPCMA ROM
		int		adpcmasize;
		__DECL_ALIGNED(16) ADPCMA	adpcma[6];
		int8	adpcmatl;		// ADPCMA 全体の音量
		int		adpcmatvol_l;
		int		adpcmatvol_r;
		uint8	adpcmakey;		// ADPCMA のキー
		int		adpcmastep;
		__DECL_ALIGNED(16) uint8	adpcmareg[32];

		__DECL_ALIGNED(16) static int jedi_table[(48+1)*16];

		__DECL_ALIGNED(16) Channel4 ch[6];
	};

	//	YM2612/3438(OPN2) ----------------------------------------------------
	class DLL_PREFIX OPN2 : public OPN2Base
	{
	public:
		OPN2();
		virtual ~OPN2();

		bool	Init(uint c, uint r, bool=false, const char* =0);
		bool	SetRate(uint c, uint r, bool);

		void	Reset();
		void 	Mix(Sample* buffer, int nsamples);
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);
		uint	ReadStatus() { return status & 0x03; }
		uint	ReadStatusEx() { return 0xff; }

		bool ProcessState(void *f, bool loading);

	private:
		//void	SetStatus(uint bit);
		//void	ResetStatus(uint bit);
	// 線形補間用ワーク
		int32	mixc, mixc1;
	};
}

// ---------------------------------------------------------------------------

inline void FM::OPNBase::RebuildTimeTable()
{
	int p = prescale;
	prescale = -1;
	SetPrescaler(p);
}

inline void FM::OPNBase::SetVolumePSG(int db_l, int db_r)
{
	psg.SetVolume(db_l, db_r, is_ay3_891x);
}

#endif // FM_OPNA_H
