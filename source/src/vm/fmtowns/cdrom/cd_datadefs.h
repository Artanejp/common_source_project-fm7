/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2025.05.10 -

	[FM-Towns CD-ROM based on SCSI CDROM; Definitions of CD-ROM Sector / SUBQ structures.]
	History:
	  2025-05-10 K.Ohta : Split from vm/fmtowns/cdrom.h .
*/
#pragma once

#include <cstdint>
/*!
 * @brief Belows are CD-ROM sector structuer.
 * @note See https://en.wikipedia.org/wiki/CD-ROM#Sector_structure .
 */
#pragma pack(push, 1)
typedef struct cdrom_msf_t {
	union {
		struct {
			uint8_t m; // Minute (maybe 0 to 75? 80?) or special code.
			uint8_t s; // Seconds (0 to 59)
			uint8_t f; // frame (0 to 75)
		} msf;
		uint8_t byte[3];
	} d;
} cdrom_msf_t;
#pragma pack(pop)

#pragma pack(push, 1)
/*!
 * @brief DATA STRUCTURE of TOC_TABLE #00.
 * From page 119 of IEC-60908 edition 2.0 at 1999.
 */
typedef struct {
	uint8_t total_toc_tables;
	uint8_t volume_info; // Should fixed to $00.
	
	cdrom_msf_t interval_start_msf;
	uint8_t genre[2];
	uint8_t total_tracks;
	uint8_t ean_code[13];
	uint8_t first_track_num;
	uint8_t last_track_num;
	cdrom_msf_t alt_interval_start_msf;
	uint8_t seq_num;
	uint8_t alt_seq_num;
	uint8_t copyright_flags;
	uint8_t reserved[11];
} CDROM_RAW_TOC_Table_00_t;

typedef struct {
	uint8_t toc_num;     // May COUNTDOWN. If $00 indicates last TOC (or above).
	uint8_t track_num;   // If $FF, indicates last TOC (or above).
	cdrom_msf_t start_msf;
	uint8_t genre[2];   // If LAST TOC, must fix to $0000.
	uint8_t alt_seq_num;
	uint8_t isrc[12];
} CDROM_RAW_TOC_TRACK_t;

typedef union {
	CDROM_RAW_TOC_Table_00_t head_toc;
	CDROM_RAW_TOC_TRACK_t track_toc[2];
} CDROM_RAW_TOC_t;

/*!
 * @brief Structure of CD-ROM header. (by "RED BOOK").
 */
typedef struct {
	uint8_t sync[12]; // Should be fix to $00. (if emulates...)
	uint8_t addr_m;
	uint8_t addr_s;
	uint8_t addr_f;
	uint8_t sector_type; //! 1 = MODE1, 2=MODE2
} cd_data_head_t;
#pragma pack(pop)

#pragma pack(push, 1)
/*!
 * @brief Sector structure of CDROM MODE1/DATA.
 *
 * @note ToDo: Still not implement crc32 and ecc.
 * @note 20201116 K.O
 */
typedef struct {
	cd_data_head_t header;
	uint8_t data[2048];
	uint8_t crc32[4]; //! CRC32 checksum.
	uint8_t reserved[8];
	uint8_t ecc[276]; //! ERROR CORRECTIOM DATA; by read solomon code.
} cd_data_mode1_t;
#pragma pack(pop)

#pragma pack(push, 1)
/*!
 * @brief Sector structure of CDROM MODE2/DATA.
 */
typedef struct {
	cd_data_head_t header;
	uint8_t data[2336];
} cd_data_mode2_t;
#pragma pack(pop)

/*!
 * @brief Sector structure of CD AUDIO.
 */
#pragma pack(push, 1)
typedef struct {
	uint8_t data[2352];
} cd_audio_sector_t;
#pragma pack(pop)

#pragma pack(push, 1)
/*!
 * @brief Sector ISO/2048 virtual image.
 *
 * @note ToDo: Add fake header and crc and ecc.
 * @note 20201116 K.O
 */
typedef struct {
	uint8_t data[2048];
} cd_data_iso_t;
#pragma pack(pop)

#pragma pack(push, 1)
/*!
 * @brief Buffer definition of CD-ROM sector buffer (excepts ISO/2048).
 *
 * @note 20201116 K.O
 */
typedef union cdimage_buffer_s {
	uint8_t rawdata[2352]; //!< @note OK?
	cd_data_mode1_t mode1;
	cd_data_mode2_t mode2;
	cd_audio_sector_t audio;
} cdimage_buffer_t;
#pragma pack(pop)

/*!
 * @brief Define SUBQ Channel structure.
 * From page 37 of IEC-60908 edition 2.0 at 1999.
 */

enum {
	CDROM_SUBCH_SYNC_S0 = 0x0801; // 0b00100000000001; -> Note: Big endian.
	CDROM_SUBCH_SYNC_S1 = 0x0012; // 0b00000000010010; -> Note: Big endian.
} CDROM_SUBChannel_SYNC_t;


#pragma pack(push, 1)	
typedef struct {
	uint8_t is_not_audio:1; // 1 = START, 0 = IN AUDIO. 
} CDROM_SUBP_bitslice_t;
#pragma pack(pop)	

#pragma pack(push, 1)	
typedef struct {
	uint8_t zero_data[9]; // Should be filled by $00.
} CDROM_SUBQ_MODE0_DATA_t;

typedef struct {
	uint8_t track_num;
	uint8_t index;
	cdrom_msf_t rel_msf;
	uint8_t zero_data;
	cdrom_msf_t abs_msf;
} CDROM_SUBQ_MODE1_DATA_t;

typedef struct {
	uint8_t n1:4; // 4bits.
	uint8_t n2_n13[6]; // 4bits x 12.
	uint8_t dummy_zero:4;
	uint8_t zero_pad; // 8bit.
	uint8_t aframe;   // 8bit. See 17.5.2 of IEC-60908 / 1999 (Edition 2).
}  CDROM_SUBQ_MODE2_DATA_t;

/*
 * See page 43, "17.5.3 Mode 3 for DATA-Q" of
 * IEC-60908 Edition 2 (1999) for BCD encoding protocol.
 */
typedef struct {
	uint8_t I1:6;         // I1, I2 : Country CODE.
	uint8_t I2:6;
	uint8_t I3:6;         // I3 - I5 : OWNER CODE
	uint8_t I4:6;
	uint8_t I5:6;
	uint8_t dummy_zero:2; // Should be fixed to 00.
	union {                 // I6 - I7
		struct {
			uint8_t I6:4;
			uint8_t I7:4;
		} I;
		struct {
			uint8_t hi:4;
			uint8_t lo:4;
		} year;
		uint8_t year_byte;
	} I67;
	union {          // I8 -I12 : Serial code.
		struct {
			uint8_t I8:4;
			uint8_t I9:4;
			uint8_t I10:4;
			uint8_t I11:4;
			uint8_t I12:4;
			uint8_t zero_pad:4;    // 4bit.
		} bits;
		uint8_t bytes[3];
	} I8_12;
	uint8_t aframe;        // 8bit. "F" of Absolute MSF.
} CDROM_SUBQ_MODE3_DATA_t;
#pragma pack(pop)

#pragma pack(push, 1)	
typedef struct {
	//uint8_t sync:2; // OMIT SYNC BIT.
	union {
		struct {
			uint8_t control:4;
			uint8_t adr:4; // This indicate MODE.
			// Data field may be 72bits.
			union {
				uint8_t                 data[9];     // 72 bits.
				CDROM_SUBQ_MODE0_DATA_t mode0;
				CDROM_SUBQ_MODE1_DATA_t mode1;
				CDROM_SUBQ_MODE2_DATA_t mode2;
				CDROM_SUBQ_MODE3_DATA_t mode3;
			} data;
			uint8_t crc[2];                         // 16 bits.
		} bits; // 96bit
		uint8_t byte[12];
	} data;
} CDROM_SUBQ_bitslice_t;
#pragma pack(pop)

typedef enum {
	DEFAULT_VALUE_CH_R = 0x00; // 96bits of '0'.
	DEFAULT_VALUE_CH_S = 0x00; // 96bits of '0'.
	DEFAULT_VALUE_CH_T = 0x00; // 96bits of '0'.
	DEFAULT_VALUE_CH_U = 0x00; // 96bits of '0'.
	DEFAULT_VALUE_CH_V = 0x00; // 96bits of '0'.
	DEFAULT_VALUE_CH_W = 0x00; // 96bits of '0'.
} CDROM_SUBChannel_defalt_values_t;

static inline const  uint8_t char_to_cdrom_mode3_6bit(char c)
{
	__UNLIKELY_IF((c < '0') && (c > 'Z')) {
		return 0;
	}
	if((c >= '0') && (c <= '9')) {
		return (uint8_t)(c - '0');
	}
	if((c >= 'A') && (c <= 'Z')) {
		return (uint8_t)((c - 'A') + 0x11); // Char starts 0b01001 .
	}
	return 0x00; // Undefined.
}

static inline const char cdrom_mode3_6bit_to_char(uint8_t n)
{
	n = n & 63; // Sanitize.
	__UNLIKELY_IF(n > 0x2a) { // 0b101010
		return '\0'; // UNDEFINED.
	}
	__LIKELY_IF(n < 10) { // 0b101010
		return (char)(n + '0');
	}
	__LIKELY_IF(n >= 0x11) { // 0b010001
		return (char)(n - 0x11 + 'A' );
	}
	return '\0'; // UNDEFINED.
}

#pragma pack(push, 1)
/*!
 * @brief Union definition of SUBQ bitslice.
 *
 * @note 20201116 K.O
 */
typedef union SUBC_u {
	struct {
		uint8_t P:1;
		uint8_t Q:1;
		uint8_t R:1;
		uint8_t S:1;
		uint8_t T:1;
		uint8_t U:1;
		uint8_t V:1;
		uint8_t W:1;
	} bit;
	uint8_t byte;
} SUBC_t;
#pragma pack(pop)

/*!
 * @note this function don't care of SYNC field.
 */
static inline const SUBC_t get_subchannel_bitslice_without_sync(CDROM_SUBQ_bitslice_t subq_src, const bool is_not_audio, const unsigned int bitpos)
{
	unsigned int _bitpos = bitpos % 96;
	SUBC_t val;
	val.byte = 0x00;

	CDROM_SUBP_bitslice_t _P;
	_P.is_not_audio = (is_not_audio) ? 1 : 0;
	
	uint8_t nibble = _bitpos & 7;
	uint8_t pbyte  = _bitpos >> 3;
	
	uint8_t _tmpb = subq_src.data.byte[pbyte];
	uint8_t _bit = (_tmpb >> (7 - nibble)) & 1;
	val.bit.P = _P;
	val.bit.Q = _bit;
	// ToDo: Make SUBR - SUBW
	return val;
}

static inline cdrom_msf_t make_msf_from_m_s_f(uint8_t m, uint8_t s, uint8_t f)
{
	cdrom_msf_t val;
	val.m = m;
	val.s = s;
	val.f = f;
	return val;
}
	
static inline cdrom_msf_t make_msf_from_lba(size_t lba)
{
	cdrom_msf_t val;
	size_t m = lba / (60 * 75);
	size_t s = (lba / 75) % 60;
	size_t f = lba % 75;
	if(m > 255) m = 255;
	val.m = (uint8_t)m;
	val.s = (uint8_t)s;
	val.f = (uint8_t)f;
	return val;
}

static inline size_t make_lba_from_msf(cdrom_msf_t msf)
{
	size_t val;
	val = ((size_t)(msf.m) * (75 * 60)) + ((size_t)(msf.s % 60) * 75) + (size_t)(msf.f % 75);
	return val;
}

static inline ssize_t make_relative_lba_from_absolute_msf(cdrom_msf_t msf, size_t start_lba)
{
	size_t val = make_lba_from_msf(msf);
	__UNLIKELY_IF(val < start_lba) {
		return (ssize_t)(-(start_lba - val));
	}
	val -= start_lba;
	__UNLIKELY_IF(val >= SSIZE_MAX) {
		val = SSIZE_MAX;
	}
	return (ssize_t)val;
}

static inline size_t make_absolute_lba_from_absolute_msf(cdrom_msf_t msf, size_t start_lba)
{
	size_t val = make_lba_from_msf(msf);
	return (val + start_lba);
}

static inline cdrom_msf_t make_relative_msf_from_lba(size_t abs_lba, size_t start_lba, bool &success)
{
	success = false;
	cdrom_msf_t val;
	__UNLIKELY_IF(abs_lba < start_lba) {
		val = make_msf_from_lba(0);
	} else {
		val = make_msf_from_lba(abs_lba - start_lba);
		success = true;
	}
	return val;
}

static inline cdrom_msf_t make_absolute_msf_from_lba(size_t rel_lba, size_t start_lba)
{
	cdrom_msf_t val;
	val = make_msf_from_lba(rel_lba + start_lba);
	return val;
}

static inline size_t calc_with_pregap(size_t lba, size_t pregap = 150)
{
	__UNLIKELY_IF(lba < pregap) {
		return 0;
	}
	return lba - pregap;
}

static inline bool unpack_bcd_to_uint8(uint8_t* src, size_t digits, uint8_t* dst, size_t bytes)
{
	__UNLIKELY_IF(digits == 0) {
		return false;
	}
	__UNLIKELY_IF(src == NULL) {
		return false;
	}
	__UNLIKELY_IF(dst == NULL) {
		return false;
	}
	__UNLIKELY_IF(bytes < (digits * 2)) {
		return false;
	}
	for(int i = 0, j = 0; i < digits; i++) {
		uint8_t hi = (src[i] >> 4) & 0x0f;
		uint8_t lo = src[i] & 0x0f;
		// ToDo check value?
		dst[j + 0] = hi;
		dst[j + 1] = lo;
		j += 2;
	}
	return true;
}

static inline uint8_t make_packed_bcd_from_uint8_value(uint8_t val)
{
	uint8_t ret;
	ret = ((val / 10) & 0x0f) << 4;
	ret |= ((val % 10) & 0x0f);
	return ret;
}

// ToDo: Minus value.
template <typename _T>
	inline size_t make_unpacked_bcd_from_value(uint8_t* dst, size_t max_bytes, _T val, bool &is_overflow)
{
	is_overflow = false;
	__UNLIKELY_IF((dst == NULL) || (max_bytes == 0)) {
		return 0;
	}
	__UNLIKELY_IF(val < ((_T)0)) {
		is_overflow = true; // Value undeflow.
		return 0;
	}
	size_t pos = max_bytes - 1;
	size_t _count = 0;
	for(size_t i = 0; i < max_bytes; i++) {
		dst[pos] = (uint8_t)((int)(val % 10));
		val /= ((_T)10);
		pos--;
		_count++;
		if(val < ((_T)1)) break;
	}
	__LIKELY_IF(val >= ((_T)1)) {
		is_overflow = true;
	}
	return _count;
}

template <typename _T>
	inline _T make_value_from_unpacked_bcd(uint8_t* src, size_t bytes, bool &is_legal)
{
	is_legal = false;
	__UNLIKELY_IF((src == NULL) || (bytes == 0)) {
		return (_T)0;
	}
	_T val = 0;
	for(size_t i = 0; i < bytes; i++) {
		val *= ((_T)10);
		uint8_t tmp = src[i] & 0x0f;
		__UNLIKELY_IF(tmp >= 10) {
			return val;
		}
		val = val + ((_T)tmp);
	}
	is_legal = true;
	return val;
}
static inline bool pack_uint8_to_bcd(uint8_t* src, size_t bytes, uint8_t* dst, size_t digits)
{
	__UNLIKELY_IF(digits == 0) {
		return false;
	}
	__UNLIKELY_IF(src == NULL) {
		return false;
	}
	__UNLIKELY_IF(dst == NULL) {
		return false;
	}
	__UNLIKELY_IF(bytes < (digits * 2)) {
		return false;
	}
	for(int i = 0, j = 0; i < digits; i++) {
		uint8_t hi = (src[j + 0] & 0x0f) << 4;
		uint8_t lo = src[j + 1] & 0x0f;
		// ToDo check value?
		dst[i] = hi | lo;
		j += 2;
	}
	return true;
}

static inline void make_subq_mode0(CDROM_SUBQ_bitslice_t &dat,
								   const uint8_t control
	)
{
	dat.bits.control = control;
	dat.bits.adr = 0;
	
	__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 9; i++) {
		dat.data.bits.mode0.zero_data = 0x00;
	}
	// ToDo: Calc CRC.
}

static inline void make_subq_mode1(CDROM_SUBQ_bitslice_t &dat,
								   const uint8_t control,
								   const uint8_t track,
								   const uint8_t index,
								   const cdrom_msf_t rel_msf,
								   const cdrom_msf_t abs_msf
	)
{
	dat.bits.control = control;
	dat.bits.adr = 1;
	dat.data.bits.mode1.track_num = trk;
	dat.data.bits.mode1.index = index;
	dat.data.bits.mode1.rel_msf = rel_msf;
	dat.data.bits.mode1.abs_msf = abs_msf;
	dat.data.bits.mode1.zero_data = 0x00;
	// ToDo: Calc CRC.
}

static inline void make_subq_mode2(CDROM_SUBQ_bitslice_t &dat,
								   const uint8_t control,
								   uint8_t *ean_bcd /* 13 DIGITS of UNPACKED BCD */ , 
								   const uint8_t frame
	)
{
	dat.bits.control = control;
	dat.bits.adr = 2;
	__UNLIKELY_IF(ean_bcd == NULL) {
		dat.data.bits.mode2.n1 = 0;
		for(int i = 0; i < 6; i++) {
			dat.data.bits.mode2.n2_n13[i] = 0;
		}
	} else {
		uint8_t tmp;
		tmp = ean_bcd[0] & 0x0f;
		__UNLIKELY_IF(tmp > 9) {
			tmp = 0; // OK?
		}
		dat.data.bits.mode2.n1 = tmp;
		for(int i = 1; i < 13; i++) {
			int pos = (i - 1) >> 1;
			tmp = ean_bcd[i] & 0x0f;
			__UNLIKELY_IF(tmp > 9) {
				tmp = 0; // OK?
			}
			if((i & 1) != 0) {
				// Higher
				dat.data.bits.mode2.n2_n13[pos] = 0x00;
				tmp = (tmp & 0x0f) << 4;
			} else {
				tmp = (tmp & 0xf0) >> 4;
			}
			dat.data.bits.mode2.n2_n13[pos] |= tmp;
		}
	}
	dat.data.bits.mode2.dummy_zero = 0;
	dat.data.bits.mode2.zero_pad = 0;
	dat.data.bits.mode2.aframe= frame;
	// ToDo: Calc CRC.
}

// ToDo: make_subq_mode3()
static inline void make_subq_mode2(CDROM_SUBQ_bitslice_t &dat,
								   const uint8_t control,
								   const char country_code[2],
								   const char owner_code[3],
								   const uint8_t year,
								   const uint8_t serial[3],
								   const uint8_t frame
	)
{
	dat.bits.control = control;
	dat.bits.adr = 3;
	dat.bits.mode3.I1 = char_to_cdrom_mode3_6bit(country_code[0]):
	dat.bits.mode3.I2 = char_to_cdrom_mode3_6bit(country_code[1]):
	dat.bits.mode3.I3 = char_to_cdrom_mode3_6bit(owner_code[0]):
	dat.bits.mode3.I4 = char_to_cdrom_mode3_6bit(owner_code[1]):
	dat.bits.mode3.I5 = char_to_cdrom_mode3_6bit(owner_code[2]):
	dat.bits.mode3.dummy_zero = 0;

	dat.bits.mode3.year_byte = year % 100; // OK?

	for(int i = 0; i < 3; i++) {
		dat.bits.mode3.I8_12.bytes[i] = serial[i];
	}
	dat.bits.mode3.I8_12.bits.zero_pad = 0;	
	dat.bits.mode3.aframe = frame;	
	// ToDo: Calc CRC.
}
