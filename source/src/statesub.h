/*
 * State assistance routines.
 * (C) May 16 2018 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2 (or later)
 * Usage:
 * 1. Decl. state entries per device.
 *  #define STATE_VERSION n;
 *    void foo::decl_state(void)
 *    {
 *      state_entry = new csp_saver_entries(STATE_VERSION, this_device_id, _T("NAME")); // Must be.
 *      DECL_STATE_ENTRY(var1)
 *      DECL_STATE_ENTRY_MULTI(array, sizeof(array));
 *    }
 * 2. Call decl_state() from constructor.
 * 3.
 * void foo::save_state(FILEIO *state_fio)
 * {
 *    if(state_entry != NULL) state_entry->save_state(state_fio, NULL); // Arg2 is pointer of initial CRC value.
 * }
 * bool foo::load_state(FILEIO *state_fio)
 * {
 *    if(state_entry != NULL) return state_entry->load_state(state_fio, NULL); // Arg2 is pointer of initial CRC value.
 *    return false;
 * }
 */


#if !defined(_CSP_STATE_SUB_H)
#define _CSP_STATE_SUB_H
#include "common.h"
#include "fileio.h"


#include <string>
#include <list>

template <typename T>
class csp_state_data {
protected:
	T* _dataptr;
	int _len;
	std::string _name;
	const uint32_t CRC_MAGIC_WORD = 0x04C11DB7;
protected:
	// Calculate CRC32
	// Refer to : https://qiita.com/mikecat_mixc/items/e5d236e3a3803ef7d3c5
	uint32_t calc_crc32(uint32_t seed, uint8_t *ptr, int bytes)
	{
		uint32_t crc = seed;
		uint8_t d;
		bool is_overflow;
		for(int i = 0; i < bytes; i++) {
			d = *ptr++;
			for(int bit = 0; bit < 8; bit++) {
				is_overflow = ((crc & 0x1) != 0);
				crc = crc >> 1;
				if((d & 0x01) != 0) crc = crc | 0x80000000;
				if(is_overflow) crc = crc ^ ((uint32_t)~CRC_MAGIC_WORD);
				d >>= 1;
			}
		}
		return crc;
	}
					
	bool regdata(_TCHAR *name, T* val, int64_t len)
	{
		if(val == NULL) return false;
		if(len < 0) return false;
		if((name != NULL) && (strlen(name) > 0)) {
			_name = std::string((const char *)name);
		} else {
			_name = std::string("");
		}
		_len = len;
		_dataptr = val;
		return true;
	}
public:
	csp_state_data()
	{
		_dataptr = NULL;
		_len = 1;
	}
	~csp_state_data()
	{
	}
	
	bool decl_data(_TCHAR *name, T* value, int len = 1)
	{
		return regdata(name, value, len);
	}
	std::string get_name(void)
	{
		return _name;
	}
	int get_length(void)
	{
		return _len;
	}
	int load_data(FILEIO *fio, uint32_t *sumseed)
	{
		T* ptr = _dataptr;
		int _nlen = _len;
		size_t donelen;
	
		if(_nlen <= 0) return -1;
		if(fio != NULL) {
			donelen = fio->Fread((void *)ptr, 1, (size_t)_nlen);
			if(donelen != (size_t)_nlen) return -1;
			if(sumseed != NULL) *sumseed = calc_crc32(*sumseed, (uint8_t *)ptr, _nlen); 
			return donelen;
		}
		return -1;
	}
	int save_data(FILEIO *fio, uint32_t *sumseed)
	{
		T* ptr = _dataptr;
		int _nlen = _len;
		size_t donelen;
		
		if(_nlen <= 0) return -1;
		if(fio != NULL) {
			//printf("SAVE: %s\n", _name.c_str());
			if(sumseed != NULL) *sumseed = calc_crc32(*sumseed, (uint8_t *)ptr, _nlen); 
			donelen = fio->Fwrite((void *)ptr, 1, (size_t)_nlen);
			if(donelen != (size_t)_nlen) return -1;
			return donelen;
		}
		return -1;
	}		
			
};

template <>
inline int csp_state_data<bool>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	bool* ptr = _dataptr;
	int _nlen = _len;
	size_t donelen;
	
	if(_nlen <= 0) return -1;
	//if(fio != NULL) {
		for(int i = 0; i < _nlen; i++) {
			const uint8_t data_true = 0x01;
			const uint8_t data_false = 0x00;
			uint8_t dat;
			dat = fio->FgetUint8();
			if((dat != data_true) && (dat != data_false)) return -1;
			*ptr++ = (dat == data_true) ? true : false;
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, &dat, 1);
			}
			
		}
		return _nlen;
		//}
	return -1;
}

template <>
inline int csp_state_data<bool>::save_data(FILEIO *fio,  uint32_t *sumseed)
{
	bool* ptr = _dataptr;
	int _nlen = _len;
	size_t donelen;
	
	if(_nlen <= 0) return -1;
	//if(fio != NULL) {
			for(int i = 0; i < _nlen; i++) {
				const uint8_t data_true = 0x01;
				const uint8_t data_false = 0x00;
				uint8_t dat = (*ptr++) ? data_true : data_false;
				//printf("SAVE: %s\n", _name.c_str());
				fio->FputUint8(dat);
				if(sumseed != NULL) {
					*sumseed = calc_crc32(*sumseed, &dat, 1);
				}
			}
			return _nlen;
			//	}
			return -1;
}
	
template <>
inline int csp_state_data<uint8_t>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	uint8_t *ptr = _dataptr;
	int _nlen = _len;
	uint8_t dat;
	uint8_t _n;
	
	if(_nlen <= 0) return -1;
	//if(fio != NULL) {
		for(int i = 0; i < _nlen; i++) {
			dat = fio->FgetUint8();
			*ptr++ = dat;
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, &dat, 1);
			}
		}
		return _nlen;
		//}
	return -1;
}

template <>
inline int csp_state_data<uint8_t>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	uint8_t *ptr = _dataptr;
	int _nlen = _len;
	uint8_t dat;
	uint8_t _n;
	if(_nlen <= 0) return -1;
	//if(fio != NULL) {
		for(int i = 0; i < _nlen; i++) {
			dat = *ptr++;	
			//printf("SAVE: %s\n", _name.c_str());
			fio->FputUint8(dat);
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, &dat, 1);
			}
		}
		return _nlen;
		//}
	return -1;
};		

template <>
inline int csp_state_data<int8_t>::load_data(FILEIO *fio,  uint32_t *sumseed)
{
	int8_t *ptr = _dataptr;
	int8_t dat;
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	if(_nlen <= 0) return -1;
	//if(fio != NULL) {
		for(int i = 0; i < _nlen; i++) {
			dat = fio->FgetInt8();
			*ptr++ = dat;
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, &dat, 1);
			}
		}
		return _nlen;
		//}
	return -1;
};

template <>
inline int csp_state_data<int8_t>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	int8_t *ptr = _dataptr;
	int8_t dat;
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	if(_nlen <= 0) return -1;
	//if(fio != NULL) {
		for(int i = 0; i < _nlen; i++) {
			dat = *ptr++;	
			//printf("SAVE: %s\n", _name.c_str());
			fio->FputInt8(dat);
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, &dat, 1);
			}
		}
		return _nlen;
		//}
	return -1;
};		

/*
 * Saving data should be BIG ENDIAN to keep compatibilities around machines.
 */
#define __read_data_u(_f, _s, _d, _l)			\
	_d = _f->FgetUint8();						\
	if(_s != NULL) {							\
		*_s = calc_crc32(*_s, &_d, _l);			\
	} 

#define __read_data_s(_f, _s, _d, _l)			\
	_d = _f->FgetInt8();						\
	if(_s != NULL) {							\
		*_s = calc_crc32(*_s, &_d, _l);			\
	} 

#define __write_data_u(_f, _s, _d, _l)				\
			if(_s != NULL) {						\
				*_s = calc_crc32(*_s, &_d, _l);		\
			}										\
			_f->FputUint8(_d);						\


#define __write_data_s(_f, _s, _d, _l) 				\
		if(_s != NULL) {							\
			*_s = calc_crc32(*_s, &_d, _l);			\
		}											\
		_f->FputInt8(_d);							\


template <>
inline int csp_state_data<uint16_t>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	uint16_t *ptr = _dataptr;
	pair_t d;
	uint8_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.l = dat;
		*ptr++ = d.w.l;
	}
	return _nlen;
}

template <>
inline int csp_state_data<uint16_t>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	uint16_t *ptr = _dataptr;
	pair_t d;
	uint8_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		//printf("SAVE: %s\n", _name.c_str());
		d.w.l = *ptr++;
		dat = d.b.h; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.l; __write_data_u(fio, sumseed, dat, 1);
	}
	return _nlen;
}

template <>
inline int csp_state_data<int16_t>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	int16_t *ptr = _dataptr;
	pair_t d;
	uint8_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.l = dat;
		*ptr++ = d.sw.l;
	}
	return _nlen;
};


template <>
inline int csp_state_data<int16_t>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	int16_t *ptr = _dataptr;
	pair_t d;
	uint8_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		//printf("SAVE: %s\n", _name.c_str());
		d.sw.l = *ptr++;
		dat = d.b.h; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.l; __write_data_u(fio, sumseed, dat, 1);
	}
	return _nlen;
};


template <>
inline int csp_state_data<uint32_t>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	uint32_t *ptr = _dataptr;
	pair_t d;
	uint8_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		//printf("SAVE: %s\n", _name.c_str());
		d.d = *ptr++;
		dat = d.b.h3; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h2; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h;  __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.l;  __write_data_u(fio, sumseed, dat, 1);
	}
	return _nlen;
};

template <>
inline int csp_state_data<uint32_t>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	uint32_t *ptr = _dataptr;
	pair_t d;
	uint8_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h3 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h2 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.l = dat;
		*ptr++ = d.d;
	}
	return _nlen;
};

template <>
inline int csp_state_data<pair_t>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	pair_t *ptr = _dataptr;
	pair_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		//printf("SAVE: %s\n", _name.c_str());
		dat = *ptr++;
	    __write_data_u(fio, sumseed, dat.b.h3, 1);
		__write_data_u(fio, sumseed, dat.b.h2, 1);
		__write_data_u(fio, sumseed, dat.b.h, 1);
		__write_data_u(fio, sumseed, dat.b.l, 1);
	}
	return _nlen;
};

template <>
inline int csp_state_data<pair_t>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	pair_t *ptr = _dataptr;
	uint8_t dat;
	pair_t d;
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h3 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h2 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.l = dat;
		*ptr++ = d;
	}
	return _nlen;
};


template <>
inline int csp_state_data<int32_t>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	int32_t *ptr = _dataptr;
	pair_t d;
	uint8_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h3 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h2 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.l = dat;
		*ptr++ = d.sd;
	}
	return _nlen;
};

template <>
inline int csp_state_data<int32_t>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	int32_t *ptr = _dataptr;
	pair_t d;
	uint8_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		//printf("SAVE: %s\n", _name.c_str());
		d.sd = *ptr++;
		dat = d.b.h3; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h2; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h;  __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.l;  __write_data_u(fio, sumseed, dat, 1);
	}
	return _nlen;
};

typedef union {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h7, h6, h5, h4, h3, h2, h, l;
#else
		uint8_t l, h, h2, h3, h4, h5, h6, h7;
#endif
	} b;
	uint64_t u64;
	int64_t s64;
} pair64_sav_t;

template <>
inline int csp_state_data<uint64_t>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	uint64_t *ptr = _dataptr;
	pair64_sav_t d;
	uint8_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h7 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h6 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h5 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h4 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h3 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h2 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.l = dat;
		*ptr++ = d.u64;
	}
	return _nlen;
};

template <>
inline int csp_state_data<uint64_t>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	uint64_t *ptr = _dataptr;
	pair64_sav_t d;
	uint8_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		//printf("SAVE: %s\n", _name.c_str());
		d.u64 = *ptr++;
		dat = d.b.h7; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h6; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h5; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h4; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h2; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h;  __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.l;  __write_data_u(fio, sumseed, dat, 1);
	}
	return _nlen;
};

template <>
inline int csp_state_data<int64_t>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	int64_t *ptr = _dataptr;
	pair64_sav_t d;
	uint8_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h7 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h6 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h5 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h4 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h3 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h2 = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.h = dat;
		__read_data_u(fio, sumseed, dat, 1);
		d.b.l = dat;
		*ptr++ = d.s64;
	}
	return _nlen;
};

template <>
inline int csp_state_data<int64_t>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	int64_t *ptr = _dataptr;
	pair64_sav_t d;
	uint8_t dat;
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		//printf("SAVE: %s\n", _name.c_str());
		d.s64 = *ptr++;
		dat = d.b.h7; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h6; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h5; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h4; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h2; __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.h;  __write_data_u(fio, sumseed, dat, 1);
		dat = d.b.l;  __write_data_u(fio, sumseed, dat, 1);
	}
	return _nlen;
};


template <>
inline int csp_state_data<_TCHAR>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	_TCHAR tmps[1024]; // OK?
	_TCHAR *_np;
	_TCHAR *ptr = _dataptr;
	int _nlen;
	size_t donelen;

	memset(tmps, 0x00, sizeof(tmps));
	if(ptr == NULL) return -1;
	_nlen = strlen(ptr) + 1; 
	if(_nlen <= 0) return -1;
	if(_nlen >= 1024) _nlen = 1024;
	if(fio != NULL) {
		donelen = fio->Fread((void *)tmps, 1, _nlen);
		if(donelen != (size_t)_nlen) return -1;
		if(strncmp(tmps, ptr, donelen) != 0) return -1;
		if(sumseed != NULL) *sumseed = calc_crc32(*sumseed, (uint8_t *)ptr, donelen); 
		return donelen;
	}
	return -1;
};

template <>
inline int csp_state_data<_TCHAR>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	_TCHAR *ptr = _dataptr;
	int _nlen;
	size_t donelen;
	uint8_t ndat = 0x00;
	
	if(ptr == NULL) return -1;
	_nlen = strlen(ptr) + 1; 
	if(_nlen <= 0) return -1;
	if(_nlen >= 1024) _nlen = 1024;
	
	if(fio != NULL) {
		if(sumseed != NULL) *sumseed = calc_crc32(*sumseed, (uint8_t *)ptr, _nlen); 
		donelen = fio->Fwrite((void *)ptr, 1, (size_t)_nlen);
		if(donelen != (size_t)_nlen) return -1;
		return donelen;
	}
	return -1;
};		

enum {
	csp_saver_entry_int = 0,
	csp_saver_entry_uint8,
	csp_saver_entry_int8,
	csp_saver_entry_uint16,
	csp_saver_entry_int16,
	csp_saver_entry_uint32,
	csp_saver_entry_int32,
	csp_saver_entry_uint64,
	csp_saver_entry_int64,
	csp_saver_entry_bool,
	csp_saver_entry_tchar,
	csp_saver_entry_any
};

class csp_state_utils {
protected:
	const uint32_t CRC_MAGIC_WORD = 0x04C11DB7;
	struct __list_t {
		int type_id;
		void *ptr;
	};
	std::list<__list_t>listptr;
	_TCHAR __classname[128];
	_TCHAR __classname_bak[128];
	csp_state_data<_TCHAR>* __header1;
	csp_state_data<int>* __header2;
	csp_state_data<int>* __header3;
	uint32_t crc_value;
	int class_version;
	int class_version_bak;
	int this_device_id;
	int this_device_id_bak;
	// Calculate CRC32
	// Refer to : https://qiita.com/mikecat_mixc/items/e5d236e3a3803ef7d3c5
	uint32_t calc_crc32(uint32_t seed, uint8_t *ptr, int bytes)
	{
		uint32_t crc = seed;
		uint8_t d;
		bool is_overflow;
		for(int i = 0; i < bytes; i++) {
			d = *ptr++;
			for(int bit = 0; bit < 8; bit++) {
				is_overflow = ((crc & 0x1) != 0);
				crc = crc >> 1;
				if((d & 0x01) != 0) crc = crc | 0x80000000;
				if(is_overflow) crc = crc ^ ((uint32_t)~CRC_MAGIC_WORD);
				d >>= 1;
			}
		}
		return crc;
	}
					
public:
	csp_state_utils(int _version = 1, int device_id = 1, _TCHAR *classname = NULL) {
		listptr.clear();
		crc_value = 0;
		__list_t _l;
		
		this_device_id = device_id;
		this_device_id_bak = this_device_id;
		
		memset(__classname, 0x00, sizeof(__classname));
		memset(__classname_bak, 0x00, sizeof(__classname_bak));
		
		if(classname != NULL) {
			strncpy(__classname, classname, sizeof(__classname) - 1); 
		} else {
			strncpy(__classname, "Unknown Class", sizeof(__classname) - 1); 
		}
		strncpy(__classname_bak, __classname, sizeof(__classname_bak) - 1);
		
		class_version = _version;
		class_version_bak = _version;
		
		std::string __headerstr = std::string("HEADER_");
		__headerstr = __headerstr + __classname;
		__header1 = new csp_state_data<_TCHAR>();
		__header1->decl_data((_TCHAR *)__headerstr.c_str(), __classname, __headerstr.size());

		__header2 = new csp_state_data<int>();
		__header2->decl_data((_TCHAR *)"HEADER_VER", &class_version, 1);
		__header3 = new csp_state_data<int>();
		__header3->decl_data((_TCHAR *)"DEVICE_ID", &this_device_id, 1);

		_l.type_id = csp_saver_entry_tchar;
		_l.ptr = (void *)__header1;
		listptr.push_back(_l);
		
		_l.type_id = csp_saver_entry_int;
		_l.ptr = (void *)__header2;
		listptr.push_back(_l);
		
		_l.type_id = csp_saver_entry_int;
		_l.ptr = (void *)__header3;
		listptr.push_back(_l);

	}
	~csp_state_utils() {
	}
	std::list<std::string> get_entries_list(void)
	{
		__list_t _l;
		std::list<std::string> _rlist;
		std::string _tname, _vname;
		_rlist.clear();
		
		for(auto p = listptr.begin(); p != listptr.end(); ++p) {
			void *pp = (*p).ptr;
			if(pp != NULL) {
				_vname = _T("NAME:");
				switch((*p).type_id) {
				case csp_saver_entry_int:
					_tname = _T("TYPE: int");
					_vname = _vname + ((csp_state_data<int> *)pp)->get_name();
					break;
				case csp_saver_entry_uint8:
					_tname = _T("TYPE: uint8_t");
					_vname = _vname + ((csp_state_data<uint8_t> *)pp)->get_name();
					break;
				case csp_saver_entry_int8:
					_tname = _T("TYPE: int8_t");
					_vname = _vname + ((csp_state_data<int8_t> *)pp)->get_name();
					break;
				case csp_saver_entry_uint16:
					_tname = _T("TYPE: uint16_t");
					_vname = _vname + ((csp_state_data<uint16_t> *)pp)->get_name();
					break;
				case csp_saver_entry_int16:
					_tname = _T("TYPE: int16_t");
					_vname = _vname + ((csp_state_data<int16_t> *)pp)->get_name();
					break;
				case csp_saver_entry_uint32:
					_tname = _T("TYPE: uint32_t");
					_vname = _vname + ((csp_state_data<uint32_t> *)pp)->get_name();
					break;
				case csp_saver_entry_int32:
					_tname = _T("TYPE: int32_t");
					_vname = _vname + ((csp_state_data<int32_t> *)pp)->get_name();
					break;
				case csp_saver_entry_uint64:
					_tname = _T("TYPE: uint64_t");
					_vname = _vname + ((csp_state_data<uint64_t> *)pp)->get_name();
					break;
				case csp_saver_entry_int64:
					_tname = _T("TYPE: int64_t");
					_vname = _vname + ((csp_state_data<int64_t> *)pp)->get_name();
					break;
				case csp_saver_entry_bool:
					_tname = _T("TYPE: bool");
					_vname = _vname + ((csp_state_data<bool> *)pp)->get_name();
					break;
				case csp_saver_entry_tchar:
					_tname = _T("TYPE: _TCHAR");
					_vname = _vname + ((csp_state_data<_TCHAR> *)pp)->get_name();
					break;
				case csp_saver_entry_any:
					_tname = _T("TYPE: ANY");
					_vname = _vname + ((csp_state_data<void> *)pp)->get_name();
					break;
				default:
					_tname = _T("TYPE: ANY(UNKNOWN)");
					_vname = _vname + ((csp_state_data<void> *)pp)->get_name();
					break;
				}
				_rlist.push_back(_tname);
				_rlist.push_back(_vname);
			}
		}
		return _rlist;
	}
	template <class T>
	void add_entry(csp_state_data<T> *p)
	{
		__list_t _l;
		_l.ptr = (void *)p;
		if(typeid(T) == typeid(int)) {
			_l.type_id = csp_saver_entry_int;
		} else if(typeid(T) == typeid(uint8_t)) {
			_l.type_id = csp_saver_entry_uint8;
		} else if(typeid(T) == typeid(int8_t)) {
			_l.type_id = csp_saver_entry_int8;
		} else if(typeid(T) == typeid(uint16_t)) {
			_l.type_id = csp_saver_entry_uint16;
		} else if(typeid(T) == typeid(int16_t)) {
			_l.type_id = csp_saver_entry_int16;
		} else if(typeid(T) == typeid(uint32_t)) {
			_l.type_id = csp_saver_entry_uint32;
		} else if(typeid(T) == typeid(int32_t)) {
			_l.type_id = csp_saver_entry_int32;
		} else if(typeid(T) == typeid(uint64_t)) {
			_l.type_id = csp_saver_entry_uint64;
		} else if(typeid(T) == typeid(int64_t)) {
			_l.type_id = csp_saver_entry_int8;
		} else if(typeid(T) == typeid(bool)) {
			_l.type_id = csp_saver_entry_bool;
		} else if(typeid(T) == typeid(_TCHAR)) {
			_l.type_id = csp_saver_entry_tchar;
		} else {
			_l.type_id = csp_saver_entry_any;
		}
		listptr.push_back(_l);
	}
	uint32_t get_crc_value(void)
	{
		return crc_value;
	}
	void get_class_name(_TCHAR *buf, int len)
	{
		strncpy(buf, __classname_bak, ((size_t)len > sizeof(__classname_bak)) ? sizeof(__classname_bak) : len);
	}
	//template <typename T>
	bool save_state(FILEIO *fio, uint32_t *pcrc = NULL)
	{
		const uint8_t initdata[4] = {0xff, 0xff, 0xff, 0xff};
		const uint8_t taildata[4] = {0x00, 0x00, 0x00, 0x00};
		int retval;
		class_version = class_version_bak;
		this_device_id = this_device_id_bak;
		memset(__classname, 0x00, sizeof(__classname));
		strncpy(__classname, __classname_bak, sizeof(__classname));
		// Initialize crc;
		if(pcrc == NULL) {
			crc_value = 0xffffffff;
		} else {
			crc_value = *pcrc;
		}
		// Write header
		crc_value = calc_crc32(crc_value, initdata, 4);
		for(auto p = listptr.begin(); p != listptr.end(); ++p) {
			void *pp = (*p).ptr;
			if(pp != NULL) {
				switch((*p).type_id) {
				case csp_saver_entry_int:
					retval = ((csp_state_data<int> *)pp)->save_data(fio, &crc_value);
					break;
				case csp_saver_entry_uint8:
					retval = ((csp_state_data<uint8_t> *)pp)->save_data(fio, &crc_value);
					break;
				case csp_saver_entry_int8:
					retval = ((csp_state_data<int8_t> *)pp)->save_data(fio, &crc_value);
					break;
				case csp_saver_entry_uint16:
					retval = ((csp_state_data<uint16_t> *)pp)->save_data(fio, &crc_value);
					break;
				case csp_saver_entry_int16:
					retval = ((csp_state_data<int16_t> *)pp)->save_data(fio, &crc_value);
					break;
				case csp_saver_entry_uint32:
					retval = ((csp_state_data<uint32_t> *)pp)->save_data(fio, &crc_value);
					break;
				case csp_saver_entry_int32:
					retval = ((csp_state_data<int32_t> *)pp)->save_data(fio, &crc_value);
					break;
				case csp_saver_entry_uint64:
					retval = ((csp_state_data<uint64_t> *)pp)->save_data(fio, &crc_value);
					break;
				case csp_saver_entry_int64:
					retval = ((csp_state_data<int64_t> *)pp)->save_data(fio, &crc_value);
					break;
				case csp_saver_entry_bool:
					retval = ((csp_state_data<bool> *)pp)->save_data(fio, &crc_value);
					break;
				case csp_saver_entry_tchar:
					retval = ((csp_state_data<_TCHAR> *)pp)->save_data(fio, &crc_value);
					break;
				case csp_saver_entry_any:
					retval = ((csp_state_data<void> *)pp)->save_data(fio, &crc_value);
					break;
				default:
					retval = 0;
					break;
				}
				if(retval <= 0) return false;
			}
		}
		// embed CRC
		crc_value = calc_crc32(crc_value, taildata, 4);
		fio->FputUint32_BE(crc_value);
		if(pcrc != NULL) *pcrc = crc_value;
		return true;
	}
	bool load_state(FILEIO *fio, uint32_t *pcrc = NULL)
	{
		const uint8_t initdata[4] = {0xff, 0xff, 0xff, 0xff};
		const uint8_t taildata[4] = {0x00, 0x00, 0x00, 0x00};
		int retval;
		// Initialize crc;
		if(pcrc == NULL) {
			crc_value = 0xffffffff;
		} else {
			crc_value = *pcrc;
		}
		crc_value = calc_crc32(crc_value, initdata, 4);
		for(auto p = listptr.begin(); p != listptr.end(); ++p) {
			void *pp = (*p).ptr;
			if(pp != NULL) {
				switch((*p).type_id) {
				case csp_saver_entry_int:
					retval = ((csp_state_data<int> *)pp)->load_data(fio, &crc_value);
					break;
				case csp_saver_entry_uint8:
					retval = ((csp_state_data<uint8_t> *)pp)->load_data(fio, &crc_value);
					break;
				case csp_saver_entry_int8:
					retval = ((csp_state_data<int8_t> *)pp)->load_data(fio, &crc_value);
					break;
				case csp_saver_entry_uint16:
					retval = ((csp_state_data<uint16_t> *)pp)->load_data(fio, &crc_value);
					break;
				case csp_saver_entry_int16:
					retval = ((csp_state_data<int16_t> *)pp)->load_data(fio, &crc_value);
					break;
				case csp_saver_entry_uint32:
					retval = ((csp_state_data<uint32_t> *)pp)->load_data(fio, &crc_value);
					break;
				case csp_saver_entry_int32:
					retval = ((csp_state_data<int32_t> *)pp)->load_data(fio, &crc_value);
					break;
				case csp_saver_entry_uint64:
					retval = ((csp_state_data<uint64_t> *)pp)->load_data(fio, &crc_value);
					break;
				case csp_saver_entry_int64:
					retval = ((csp_state_data<int64_t> *)pp)->load_data(fio, &crc_value);
					break;
				case csp_saver_entry_bool:
					retval = ((csp_state_data<bool> *)pp)->load_data(fio, &crc_value);
					break;
				case csp_saver_entry_tchar:
					retval = ((csp_state_data<_TCHAR> *)pp)->load_data(fio, &crc_value);
					break;
				case csp_saver_entry_any:
					retval = ((csp_state_data<void> *)pp)->load_data(fio, &crc_value);
					break;
				default:
					retval = 0;
					break;
				}
			}
		}
		crc_value = calc_crc32(crc_value, taildata, 4);
		if(pcrc != NULL) *pcrc = crc_value;
		// check CRC
		uint32_t cmpval = fio->FgetUint32_BE();
		if(cmpval != crc_value) return false;
		// Check header
		if(this_device_id != this_device_id_bak) return false;
		if(class_version != class_version_bak) return false;
		if(strncmp(__classname, __classname_bak, sizeof(__classname)) != 0) return false;
		return true;
	}
};
	
			
#define DECL_STATE_ENTRY0(__type, _n_name, __list) {				  \
		csp_state_data<__type> *__p = new csp_state_data<__type>(); \
		__p->decl_data((_TCHAR *) # _n_name, &_n_name, 1);					\
		__list->add_entry(__p);											\
	}

#define DECL_STATE_ENTRY1(__type, _n_name, __list, __len) {				\
		csp_state_data<__type> *__p = new csp_state_data<__type>(); \
		__p->decl_data((_TCHAR *) # _n_name, _n_name, __len);					\
		__list->add_entry(__p);											\
	}

#define DECL_STATE_ENTRY_MULTI0(__type, _n_name, __list, __size) {	\
	csp_state_data<__type>  *__p = new csp_state_data<__type>(); \
	__p->decl_data((_TCHAR *) # _n_name, _n_name, __size); \
	__list->add_entry(__p); \
	}

#define DECL_STATE_ENTRY_INT(___name) DECL_STATE_ENTRY0(int, ___name, state_entry)

#define DECL_STATE_ENTRY_UINT8(___name) DECL_STATE_ENTRY0(uint8_t, ___name, state_entry)
#define DECL_STATE_ENTRY_INT8(___name) DECL_STATE_ENTRY0(int8_t, ___name, state_entry)
#define DECL_STATE_ENTRY_UINT16(___name) DECL_STATE_ENTRY0(uint16_t, ___name, state_entry)
#define DECL_STATE_ENTRY_INT16(___name) DECL_STATE_ENTRY0(int16_t, ___name, state_entry)
#define DECL_STATE_ENTRY_UINT32(___name) DECL_STATE_ENTRY0(uint32_t, ___name, state_entry)
#define DECL_STATE_ENTRY_INT32(___name) DECL_STATE_ENTRY0(int32_t, ___name, state_entry)
#define DECL_STATE_ENTRY_UINT64(___name) DECL_STATE_ENTRY0(uint64_t, ___name, state_entry)
#define DECL_STATE_ENTRY_INT64(___name) DECL_STATE_ENTRY0(int64_t, ___name, state_entry)
#define DECL_STATE_ENTRY_PAIR(___name) DECL_STATE_ENTRY0(pair_t, ___name, state_entry)
#define DECL_STATE_ENTRY_BOOL(___name) DECL_STATE_ENTRY0(bool, ___name, state_entry)
#define DECL_STATE_ENTRY_TCHAR(___name) DECL_STATE_ENTRY0(_TCHAR, ___name, state_entry)

#define DECL_STATE_ENTRY_UINT8_ARRAY(___name, __len) DECL_STATE_ENTRY1(uint8_t, ___name, state_entry, __len)
#define DECL_STATE_ENTRY_INT8_ARRAY(___name, __len) DECL_STATE_ENTRY1(int8_t, ___name, state_entry, __len)
#define DECL_STATE_ENTRY_UINT16_ARRAY(___name, __len) DECL_STATE_ENTRY1(uint16_t, ___name, state_entry, __len)
#define DECL_STATE_ENTRY_INT16_ARRAY(___name, __len) DECL_STATE_ENTRY1(int16_t, ___name, state_entry, __len)
#define DECL_STATE_ENTRY_UINT32_ARRAY(___name, __len) DECL_STATE_ENTRY1(uint32_t, ___name, state_entry, __len)
#define DECL_STATE_ENTRY_INT32_ARRAY(___name, __len) DECL_STATE_ENTRY1(int32_t, ___name, state_entry, __len)
#define DECL_STATE_ENTRY_UINT64_ARRAY(___name, __len) DECL_STATE_ENTRY1(uint64_t, ___name, state_entry, __len)
#define DECL_STATE_ENTRY_INT64_ARRAY(___name, __len) DECL_STATE_ENTRY1(int64_t, ___name, state_entry, __len)
#define DECL_STATE_ENTRY_BOOL_ARRAY(___name, __len) DECL_STATE_ENTRY1(bool, ___name, state_entry, __len)
#define DECL_STATE_ENTRY_PAIR_ARRAY(___name, __len) DECL_STATE_ENTRY1(pair_t, ___name, state_entry, __len)


//#define DECL_STATE_ENTRY_MULTI(___name, ___size) DECL_STATE_ENTRY_MULTI0(uint8_t, ___name, state_entry, ___size)
#define DECL_STATE_ENTRY_MULTI(_n_type, ___name, ___size) DECL_STATE_ENTRY_MULTI0(_n_type, ___name, state_entry, ___size)


#endif /* _CSP_STATE_SUB_H */
