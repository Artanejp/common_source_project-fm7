// ---------------------------------------------------------------------------
//	PSG Sound Implementation
//	Copyright (C) cisc 1997, 1999.
// ---------------------------------------------------------------------------
//	$Id: psg.cpp,v 1.10 2002/05/15 21:38:01 cisc Exp $

#include "headers.h"
#include "misc.h"
#include "psg.h"

#include "../../fileio.h"

// ---------------------------------------------------------------------------
//	コンストラクタ・デストラクタ
//
PSG::PSG()
{
	// テーブル初期化
	for(int i = 0; i < noisetablesize; i++) {
		noisetable[i] = 0;
	}
	for(int i = 0; i < 32; i++) {
		EmitTableL[i] = EmitTableR[i] = -1;
	}
	for(int i = 0; i < 16; i++) {
		for(int j = 0; j < 64; j++) {
			enveloptable_l[i][j] = enveloptable_r[i][j] = 0;
		}
	}
	SetVolume(0, 0, false);
	MakeNoiseTable();
	Reset();
	mask = 0x3f;
	envelop_l = enveloptable_l[0]; // temporary fix
	envelop_r = enveloptable_r[0]; // temporary fix
}

PSG::~PSG()
{

}

// ---------------------------------------------------------------------------
//	PSG を初期化する(RESET) 
//
void PSG::Reset()
{
	for (int i=0; i<14; i++)
		SetReg(i, 0);
	SetReg(7, 0xff);
	SetReg(14, 0xff);
	SetReg(15, 0xff);
}

// ---------------------------------------------------------------------------
//	クロック周波数の設定
//
void PSG::SetClock(int clock, int rate)
{
	tperiodbase = int((1 << toneshift ) / 4.0 * clock / rate);
	eperiodbase = int((1 << envshift  ) / 4.0 * clock / rate);
	nperiodbase = int((1 << noiseshift) / 4.0 * clock / rate);
	
	// 各データの更新
	int tmp;
	tmp = ((reg[0] + reg[1] * 256) & 0xfff);
	speriod[0] = tmp ? tperiodbase / tmp : tperiodbase;
	tmp = ((reg[2] + reg[3] * 256) & 0xfff);
	speriod[1] = tmp ? tperiodbase / tmp : tperiodbase;
	tmp = ((reg[4] + reg[5] * 256) & 0xfff);
	speriod[2] = tmp ? tperiodbase / tmp : tperiodbase;
	tmp = reg[6] & 0x1f;
	nperiod = tmp ? nperiodbase / tmp / 2 : nperiodbase / 2;
	tmp = ((reg[11] + reg[12] * 256) & 0xffff);
	eperiod = tmp ? eperiodbase / tmp : eperiodbase * 2;
}

// ---------------------------------------------------------------------------
//	ノイズテーブルを作成する
//
void PSG::MakeNoiseTable()
{
	if (!noisetable[0])
	{
		int noise = 14321;
		for (int i=0; i<noisetablesize; i++)
		{
			int n = 0;
			for (int j=0; j<32; j++)
			{
				n = n * 2 + (noise & 1);
				noise = (noise >> 1) | (((noise << 14) ^ (noise << 16)) & 0x10000);
			}
			noisetable[i] = n;
		}
	}
}

// ---------------------------------------------------------------------------
//	出力テーブルを作成
//	素直にテーブルで持ったほうが省スペース。
//
void PSG::SetVolume(int volume_l, int volume_r, bool is_ay3_891x)
{
	double base_l = 0x4000 / 3.0 * pow(10.0, volume_l / 40.0);
	double base_r = 0x4000 / 3.0 * pow(10.0, volume_r / 40.0);
	
	if(is_ay3_891x)
	{
		// AY-3-8190/8192 (PSG): 16step
		for (int i=31; i>=3; i-=2)
		{
			EmitTableL[i] = EmitTableL[i-1] = int(base_l);
			EmitTableR[i] = EmitTableR[i-1] = int(base_r);
			base_l /= 1.189207115;
			base_l /= 1.189207115;
			base_r /= 1.189207115;
			base_r /= 1.189207115;
		}
	}
	else
	{
		// YM2203 (SSG): 32step
		for (int i=31; i>=2; i--)
		{
			EmitTableL[i] = int(base_l);
			EmitTableR[i] = int(base_r);
			base_l /= 1.189207115;
			base_r /= 1.189207115;
		}
	}
	EmitTableL[1] = 0;
	EmitTableL[0] = 0;
	EmitTableR[1] = 0;
	EmitTableR[0] = 0;
	MakeEnvelopTable();

	SetChannelMask(~mask);
}

void PSG::SetChannelMask(int c)
{ 
	mask = ~c;
	for (int i=0; i<3; i++)
	{
		olevel_l[i] = mask & (1 << i) ? EmitTableL[(reg[8+i] & 15) * 2 + 1] : 0;
		olevel_r[i] = mask & (1 << i) ? EmitTableR[(reg[8+i] & 15) * 2 + 1] : 0;
	}
}

// ---------------------------------------------------------------------------
//	エンベロープ波形テーブル
//
void PSG::MakeEnvelopTable()
{
	// 0 lo  1 up 2 down 3 hi
	static uint8 table1[16*2] =
	{
		2,0, 2,0, 2,0, 2,0, 1,0, 1,0, 1,0, 1,0,
		2,2, 2,0, 2,1, 2,3, 1,1, 1,3, 1,2, 1,0,
	};
	static uint8 table2[4] = {  0,  0, 31, 31 };
	static uint8 table3[4] = {  0,  1, 255,  0 }; // 255 = -1

	uint* ptr_l = enveloptable_l[0];
	uint* ptr_r = enveloptable_r[0];

	for (int i=0; i<16*2; i++)
	{
		uint8 v = table2[table1[i]];
		
		for (int j=0; j<32; j++)
		{
			*ptr_l++ = EmitTableL[v];
			*ptr_r++ = EmitTableR[v];
			v += table3[table1[i]];
		}
	}
}

// ---------------------------------------------------------------------------
//	PSG のレジスタに値をセットする
//	regnum		レジスタの番号 (0 - 15)
//	data		セットする値
//
void PSG::SetReg(uint regnum, uint8 data)
{
	if (regnum < 0x10)
	{
		reg[regnum] = data;
		switch (regnum)
		{
			int tmp;

		case 0:		// ChA Fine Tune
		case 1:		// ChA Coarse Tune
			tmp = ((reg[0] + reg[1] * 256) & 0xfff);
			speriod[0] = tmp ? tperiodbase / tmp : tperiodbase;
			break;
		
		case 2:		// ChB Fine Tune
		case 3:		// ChB Coarse Tune
			tmp = ((reg[2] + reg[3] * 256) & 0xfff);
			speriod[1] = tmp ? tperiodbase / tmp : tperiodbase;
			break;
		
		case 4:		// ChC Fine Tune
		case 5:		// ChC Coarse Tune
			tmp = ((reg[4] + reg[5] * 256) & 0xfff);
			speriod[2] = tmp ? tperiodbase / tmp : tperiodbase;
			break;

		case 6:		// Noise generator control
			data &= 0x1f;
			nperiod = data ? nperiodbase / data : nperiodbase;
			break;

		case 8:
			olevel_l[0] = mask & 1 ? EmitTableL[(data & 15) * 2 + 1] : 0;
			olevel_r[0] = mask & 1 ? EmitTableR[(data & 15) * 2 + 1] : 0;
			break;

		case 9:
			olevel_l[1] = mask & 2 ? EmitTableL[(data & 15) * 2 + 1] : 0;
			olevel_r[1] = mask & 2 ? EmitTableR[(data & 15) * 2 + 1] : 0;
			break;
		
		case 10:
			olevel_l[2] = mask & 4 ? EmitTableL[(data & 15) * 2 + 1] : 0;
			olevel_r[2] = mask & 4 ? EmitTableR[(data & 15) * 2 + 1] : 0;
			break;

		case 11:	// Envelop period
		case 12:
			tmp = ((reg[11] + reg[12] * 256) & 0xffff);
			eperiod = tmp ? eperiodbase / tmp : eperiodbase * 2;
			break;

		case 13:	// Envelop shape
			ecount = 0;
			envelop_l = enveloptable_l[data & 15];
			envelop_r = enveloptable_r[data & 15];
			break;
		}
	}
}

// ---------------------------------------------------------------------------
//
//
inline void PSG::StoreSample(Sample& dest, int32 data)
{
	if (sizeof(Sample) == 2)
		dest = (Sample) Limit(dest + data, 0x7fff, -0x8000);
	else
		dest += data;
}

// ---------------------------------------------------------------------------
//	PCM データを吐き出す(2ch)
//	dest		PCM データを展開するポインタ
//	nsamples	展開する PCM のサンプル数
//
void PSG::Mix(Sample* dest, int nsamples)
{
	uint8 chenable[3], nenable[3];
	uint8 r7 = ~reg[7];

	if ((r7 & 0x3f) | ((reg[8] | reg[9] | reg[10]) & 0x1f))
	{
		chenable[0] = (r7 & 0x01) && (speriod[0] <= (1 << toneshift));
		chenable[1] = (r7 & 0x02) && (speriod[1] <= (1 << toneshift));
		chenable[2] = (r7 & 0x04) && (speriod[2] <= (1 << toneshift));
		nenable[0]  = (r7 >> 3) & 1;
		nenable[1]  = (r7 >> 4) & 1;
		nenable[2]  = (r7 >> 5) & 1;
		
		int noise, sample_l, sample_r;
		uint env_l;
		uint env_r;
		uint* p1_l = ((mask & 1) && (reg[ 8] & 0x10)) ? &env_l : &olevel_l[0];
		uint* p1_r = ((mask & 1) && (reg[ 8] & 0x10)) ? &env_r : &olevel_r[0];
		uint* p2_l = ((mask & 2) && (reg[ 9] & 0x10)) ? &env_l : &olevel_l[1];
		uint* p2_r = ((mask & 2) && (reg[ 9] & 0x10)) ? &env_r : &olevel_r[1];
		uint* p3_l = ((mask & 4) && (reg[10] & 0x10)) ? &env_l : &olevel_l[2];
		uint* p3_r = ((mask & 4) && (reg[10] & 0x10)) ? &env_r : &olevel_r[2];
		
		#define SCOUNT(ch)	(scount[ch] >> (toneshift+oversampling))
		
		if (p1_l != &env_l && p2_l != &env_l && p3_l != &env_l)
		{
			// エンベロープ無し
			if ((r7 & 0x38) == 0)
			{
				// ノイズ無し
				for (int i=0; i<nsamples; i++)
				{
					sample_l = 0;
					sample_r = 0;
					for (int j=0; j < (1 << oversampling); j++)
					{
						int x, y, z;
						x = (SCOUNT(0) & chenable[0]) - 1;
						sample_l += (olevel_l[0] + x) ^ x;
						sample_r += (olevel_r[0] + x) ^ x;
						scount[0] += speriod[0];
						y = (SCOUNT(1) & chenable[1]) - 1;
						sample_l += (olevel_l[1] + y) ^ y;
						sample_r += (olevel_r[1] + y) ^ y;
						scount[1] += speriod[1];
						z = (SCOUNT(2) & chenable[2]) - 1;
						sample_l += (olevel_l[2] + z) ^ z;
						sample_r += (olevel_r[2] + z) ^ z;
						scount[2] += speriod[2];
					}
					sample_l /= (1 << oversampling);
					sample_r /= (1 << oversampling);
					StoreSample(dest[0], sample_l);
					StoreSample(dest[1], sample_r);
					dest += 2;
				}
			}
			else
			{
				// ノイズ有り
				for (int i=0; i<nsamples; i++)
				{
					sample_l = 0;
					sample_r = 0;
					for (int j=0; j < (1 << oversampling); j++)
					{
#ifdef _M_IX86
						noise = noisetable[(ncount >> (noiseshift+oversampling+6)) & (noisetablesize-1)] 
							>> (ncount >> (noiseshift+oversampling+1));
#else
						noise = noisetable[(ncount >> (noiseshift+oversampling+6)) & (noisetablesize-1)] 
							>> (ncount >> (noiseshift+oversampling+1) & 31);
#endif
						ncount += nperiod;

						int x, y, z;
						x = ((SCOUNT(0) & chenable[0]) | (nenable[0] & noise)) - 1;		// 0 or -1
						sample_l += (olevel_l[0] + x) ^ x;
						sample_r += (olevel_r[0] + x) ^ x;
						scount[0] += speriod[0];
						y = ((SCOUNT(1) & chenable[1]) | (nenable[1] & noise)) - 1;
						sample_l += (olevel_l[1] + y) ^ y;
						sample_r += (olevel_r[1] + y) ^ y;
						scount[1] += speriod[1];
						z = ((SCOUNT(2) & chenable[2]) | (nenable[2] & noise)) - 1;
						sample_l += (olevel_l[2] + z) ^ z;
						sample_r += (olevel_r[2] + z) ^ z;
						scount[2] += speriod[2];
					}
					sample_l /= (1 << oversampling);
					sample_r /= (1 << oversampling);
					StoreSample(dest[0], sample_l);
					StoreSample(dest[1], sample_r);
					dest += 2;
				}
			}

			// エンベロープの計算をさぼった帳尻あわせ
			ecount = (ecount >> 8) + (eperiod >> (8-oversampling)) * nsamples;
			if (ecount >= (1 << (envshift+6+oversampling-8)))
			{
				if ((reg[0x0d] & 0x0b) != 0x0a)
					ecount |= (1 << (envshift+5+oversampling-8));
				ecount &= (1 << (envshift+6+oversampling-8)) - 1;
			}
			ecount <<= 8;
		}
		else
		{
			// エンベロープあり
			for (int i=0; i<nsamples; i++)
			{
				sample_l = 0;
				sample_r = 0;
				for (int j=0; j < (1 << oversampling); j++)
				{
					env_l = envelop_l[ecount >> (envshift+oversampling)];
					env_r = envelop_r[ecount >> (envshift+oversampling)];
					ecount += eperiod;
					if (ecount >= (1 << (envshift+6+oversampling)))
					{
						if ((reg[0x0d] & 0x0b) != 0x0a)
							ecount |= (1 << (envshift+5+oversampling));
						ecount &= (1 << (envshift+6+oversampling)) - 1;
					}
#ifdef _M_IX86
					noise = noisetable[(ncount >> (noiseshift+oversampling+6)) & (noisetablesize-1)] 
						>> (ncount >> (noiseshift+oversampling+1));
#else
					noise = noisetable[(ncount >> (noiseshift+oversampling+6)) & (noisetablesize-1)] 
						>> (ncount >> (noiseshift+oversampling+1) & 31);
#endif
					ncount += nperiod;

					int x, y, z;
					x = ((SCOUNT(0) & chenable[0]) | (nenable[0] & noise)) - 1;		// 0 or -1
					sample_l += (*p1_l + x) ^ x;
					sample_r += (*p1_r + x) ^ x;
					scount[0] += speriod[0];
					y = ((SCOUNT(1) & chenable[1]) | (nenable[1] & noise)) - 1;
					sample_l += (*p2_l + y) ^ y;
					sample_r += (*p2_r + y) ^ y;
					scount[1] += speriod[1];
					z = ((SCOUNT(2) & chenable[2]) | (nenable[2] & noise)) - 1;
					sample_l += (*p3_l + z) ^ z;
					sample_r += (*p3_r + z) ^ z;
					scount[2] += speriod[2];
				}
				sample_l /= (1 << oversampling);
				sample_r /= (1 << oversampling);
				StoreSample(dest[0], sample_l);
				StoreSample(dest[1], sample_r);
				dest += 2;
			}
		}
	}
}

// ---------------------------------------------------------------------------
//	テーブル
//
//uint	PSG::noisetable[noisetablesize] = { 0, };
//int	PSG::EmitTableL[0x20] = { -1, };
//int	PSG::EmitTableR[0x20] = { -1, };
//uint	PSG::enveloptable_l[16][64] = { 0, };
//uint	PSG::enveloptable_r[16][64] = { 0, };

// ---------------------------------------------------------------------------
//	ステートセーブ
//
#define PSG_STATE_VERSION	2

bool PSG::ProcessState(void *f, bool loading)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	if(!state_fio->StateCheckUint32(PSG_STATE_VERSION)) {
		return false;
	}
	state_fio->StateArray(reg, sizeof(reg), 1);
	if(loading) {
		int offset = state_fio->FgetInt32_LE();
		envelop_l = &enveloptable_l[0][0] + offset;
	} else {
		state_fio->FputInt32_LE((int)(envelop_l - &enveloptable_l[0][0]));
	}
	state_fio->StateArray(olevel_l, sizeof(olevel_l), 1);
	state_fio->StateArray(olevel_r, sizeof(olevel_r), 1);
	state_fio->StateArray(scount, sizeof(scount), 1);
	state_fio->StateArray(speriod, sizeof(speriod), 1);
	state_fio->StateValue(ecount);
	state_fio->StateValue(eperiod);
	state_fio->StateValue(ncount);
	state_fio->StateValue(nperiod);
	state_fio->StateValue(tperiodbase);
	state_fio->StateValue(eperiodbase);
	state_fio->StateValue(nperiodbase);
	state_fio->StateValue(mask);
	return true;
}
