
#if !defined(_CSP_STATE_DATA_H)
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
	int  save_string_data(const _TCHAR *p, FILEIO *fio, uint32_t *sumseed, int maxlen = -1)
	{
		int locallen;
		if(p == NULL) return -1;
		if(maxlen <= 0) {
			locallen = strlen(p);
		} else {
			locallen = strnlen(p, maxlen);
		}
		if(locallen < 0) return -1;
		locallen += 1; // Include "\0";
		if(sumseed != NULL) {
			*sumseed = calc_crc32(*sumseed, (uint8_t *)p, locallen * sizeof(char));
		}
		for(int cp = 0; cp < locallen; cp++) {
			fio->Fputc((int)p[cp]);
		}
		return locallen;
	}
	int  load_string_data(_TCHAR *p, FILEIO *fio, uint32_t *sumseed, int maxlen)
	{
		int cp;
		int _nlen;
		if(p == NULL) return -1;
		if(maxlen <= 0) return -1;
		memset(p, 0x00, sizeof(_TCHAR) * maxlen);
		for(cp = 0; cp < maxlen; cp++) {
			int _t = fio->Fgetc();
			if((_t == EOF) || (_t == '\0') || (_t == 0x00)) break;
			p[cp] = (_TCHAR)_t;
		}
		if(cp >= maxlen) p[maxlen - 1] = '\0';
		_nlen = strlen(p);
		if((sumseed != NULL) && (_nlen > 1)){
			*sumseed = calc_crc32(*sumseed, (uint8_t *)p, _nlen * sizeof(_TCHAR));
		}
		return _nlen;
	}
	// ALL OF BYNARY VALUES SHOULD BE SAVED BY BIG ENDIAN
	int save_and_change_byteorder_be(FILEIO *fio, uint32_t *sum, void *val, int bytes = 4, int rep = 1)
	{
		//int swapoffset = bytes;
		int members = 0;
		uint8_t *buf;
		uint8_t *srcp = (uint8_t *)val;
		
		if((bytes <= 0) || (rep < 1)) return 0;
		if(val == NULL) return 0;
		buf = (uint8_t *)malloc(bytes); // swap buffer
		if(buf == NULL) return 0;
		for(members = 0; members < rep; members++) {
#if defined(__LITTLE_ENDIAN__)
			int k = 0;
			for(int j = (bytes - 1); j >= 0; j--) {
				buf[j] = srcp[k];
				k++;
			}
#else // BIG_ENDIAN
			memcpy(buf, srcp, bytes);
#endif
			if(fio->Fwrite(buf, bytes, 1) != 1) {
				free(buf);
				return members;
			}
			*sum = calc_crc32(*sum, buf, bytes);
			srcp += bytes;
		}
		free(buf);
		return members;
	}
	int load_and_change_byteorder_be(FILEIO *fio, uint32_t *sum, void *val, int bytes = 4, int rep = 1)
	{
		//int swapoffset = bytes;
		int members = 0;
		uint8_t *buf;
		uint8_t *srcp = (uint8_t *)val;
		
		if((bytes <= 0) || (rep < 1)) return 0;
		if(val == NULL) return 0;
		buf = (uint8_t *)malloc(bytes); // swap buffer
		if(buf == NULL) return 0;
		for(members = 0; members < rep; members++) {
			if(fio->Fread(buf, bytes, 1) != 1) {
				free(buf);
				return members;
			}
			*sum = calc_crc32(*sum, buf, bytes);
#if defined(__LITTLE_ENDIAN__)
			int k = 0;
			for(int j = (bytes - 1); j >= 0; j--) {
				srcp[k] = buf[j];
				k++;
			}
#else // BIG_ENDIAN
			memcpy(srcp, buf, bytes);
#endif
			srcp += bytes;
		}
		free(buf);
		return members;
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

	static const uint8_t data_true = 0x01;
	static const uint8_t data_false = 0x00;
	
	if(_nlen <= 0) return -1;
	if(fio != NULL) {
		uint8_t *buf = (uint8_t *)malloc(_nlen);
		if(buf == NULL) return -1;
		if(fio->Fread(buf, _nlen, 1) != 1) return -1;
		if(sumseed != NULL) {
			*sumseed = calc_crc32(*sumseed, buf, _nlen);
		}
		for(int i = 0; i < _nlen; i++) {
			*ptr++ = (buf[i] == data_true) ? true : false;
		}
		free(buf);
		return _nlen;
	}
	return -1;
}

template <>
inline int csp_state_data<bool>::save_data(FILEIO *fio,  uint32_t *sumseed)
{
	bool* ptr = _dataptr;
	int _nlen = _len;
	static const uint8_t data_true = 0x01;
	static const uint8_t data_false = 0x00;
	if(_nlen <= 0) return -1;
	if(fio != NULL) {
		uint8_t *buf = (uint8_t *)malloc(_nlen);
		if(buf == NULL) return -1;
		for(int i = 0; i < _nlen; i++) {
			buf[i] = (*ptr++ == false) ? data_false : data_true;
		}
		if(fio->Fwrite(buf, _nlen, 1) != 1) return -1;
		if(sumseed != NULL) {
			*sumseed = calc_crc32(*sumseed, buf, _nlen);
		}
		free(buf);
		return _nlen;
	}
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
	_nlen = save_and_change_byteorder_be(fio, sumseed, (void *)ptr, sizeof(uint16_t), _nlen);
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
	_nlen = save_and_change_byteorder_be(fio, sumseed, (void *)ptr, sizeof(int16_t), _nlen);
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
	_nlen = save_and_change_byteorder_be(fio, sumseed, (void *)ptr, sizeof(uint32_t), _nlen);
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
	uint32_t *ptr = (uint32_t *)_dataptr;
	pair_t dat;
	
	int _nlen = _len;
	
	if(ptr == NULL) return -1;
	for(int i = 0; i < _nlen; i++) {
		//printf("SAVE: %s\n", _name.c_str());
		dat.d = *ptr++;
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
	uint32_t *ptr = (uint32_t *)_dataptr;
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
		*ptr++ = d.d;
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
	_nlen = save_and_change_byteorder_be(fio, sumseed, (void *)ptr, sizeof(int32_t), _nlen);
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
	_nlen = save_and_change_byteorder_be(fio, sumseed, (void *)ptr, sizeof(uint64_t), _nlen);
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
	_nlen = save_and_change_byteorder_be(fio, sumseed, (void *)ptr, sizeof(int64_t), _nlen);
	return _nlen;
};

/*
 * For floating values, internal format is differnt by ARCHTECTURE, OS, COMPILER etc.
 * So, saving / loading those values by ascii, not binary.
 * -- 20180520 K.Ohta.
 */
template <>
inline int csp_state_data<float>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	std::string _s;
	_TCHAR tmps[1024]; // OK?
	float *ptr = _dataptr;
	float _v;
	int _nlen;
	size_t donelen;

	_nlen = 1023;
	if(ptr == NULL) return -1;
	if(fio != NULL) {
		for(int i = 0; i < _len; i++) {
			donelen = load_string_data(tmps, fio, sumseed, (sizeof(tmps) / sizeof(_TCHAR)));
			if(donelen <= 0) return -1;
			_s = std::string(tmps);
			//try {
				_v = std::stof(_s);
			//} catch(const std::invalid_argument& e) {
			//	return -1;
			//} catch(const std::out_of_range& e) {
			//	return -1;
			//}
			*ptr++ = _v;
		}
		return _len;
	}
	return -1;
};
template <>
inline int csp_state_data<float>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	std::string _s;
	char tmps[1024]; // OK?
	float *ptr = _dataptr;
	float _v;
	int _nlen;
	size_t donelen;

	_nlen = 1023;
	if(ptr == NULL) return -1;
	if(fio != NULL) {
		for(int i = 0; i < _len; i++) {
			_v = *ptr++;
			memset(tmps, 0x00, sizeof(tmps));
			_s = std::to_string(_v);
			_s.copy(tmps, sizeof(tmps) / sizeof(char));
			donelen = strnlen(tmps, _nlen);
			if(donelen <= 0) return -1;
			if(donelen >= _nlen) return -1;
			if(save_string_data(tmps, fio, sumseed, sizeof(tmps) / sizeof(_TCHAR)) <= 0) return i;
		}
		return _len;
	}
	return -1;
};

template <>
inline int csp_state_data<double>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	std::string _s;
	_TCHAR tmps[1024]; // OK?
	double *ptr = _dataptr;
	double _v;
	int _nlen;
	size_t donelen;

	_nlen = 1023;
	if(ptr == NULL) return -1;
	if(fio != NULL) {
		for(int i = 0; i < _len; i++) {
			donelen = load_string_data(tmps, fio, sumseed, (sizeof(tmps) / sizeof(_TCHAR)));
			if(donelen <= 0) return -1;
			_s = std::string(tmps);
			//try {
				_v = std::stod(_s);
			//} catch(const std::invalid_argument& e) {
			//	return -1;
			//} catch(const std::out_of_range& e) {
			//	return -1;
			//}
			*ptr++ = _v;
		}
		return _len;
	}
	return -1;
};
template <>
inline int csp_state_data<double>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	std::string _s;
	_TCHAR tmps[1024]; // OK?
	double *ptr = _dataptr;
	double _v;
	int _nlen;
	size_t donelen;

	_nlen = 1023;
	if(ptr == NULL) return -1;
	if(fio != NULL) {
		for(int i = 0; i < _len; i++) {
			_v = *ptr++;
			memset(tmps, 0x00, sizeof(tmps));
			_s = std::to_string(_v);
			_s.copy(tmps, sizeof(tmps) / sizeof(_TCHAR));
			donelen = strnlen(tmps, _nlen);
			if(donelen <= 0) return -1;
			if(donelen >= _nlen) return -1;
			if(save_string_data(tmps, fio, sumseed, sizeof(tmps) / sizeof(_TCHAR)) <= 0) return i;
		}
		return _len;
	}
	return -1;
};

template <>
inline int csp_state_data<long double>::load_data(FILEIO *fio, uint32_t *sumseed)
{
	std::string _s;
	_TCHAR tmps[2048]; // OK?
	long double *ptr = _dataptr;
	long double _v;
	int _nlen;
	size_t donelen;

	_nlen = 2047;
	if(ptr == NULL) return -1;
	if(fio != NULL) {
		for(int i = 0; i < _len; i++) {
			donelen = load_string_data(tmps, fio, sumseed, (sizeof(tmps) / sizeof(_TCHAR)));
			if(donelen <= 0) return -1;

			_s = std::string(tmps);
			//try {
				_v = std::stold(_s);
			//} catch(const std::invalid_argument& e) {
			//	return -1;
			//} catch(const std::out_of_range& e) {
			//	return -1;
			//}
			*ptr++ = _v;
		}
		return _len;
	}
	return -1;
};
template <>
inline int csp_state_data<long double>::save_data(FILEIO *fio, uint32_t *sumseed)
{
	std::string _s;
	_TCHAR tmps[2048]; // OK?
	long double *ptr = _dataptr;
	long double _v;
	int _nlen;
	size_t donelen;

	_nlen = 2047;
	if(ptr == NULL) return -1;
	if(fio != NULL) {
		for(int i = 0; i < _len; i++) {
			_v = *ptr++;
			memset(tmps, 0x00, sizeof(tmps));
			_s = std::to_string(_v);
			_s.copy(tmps, sizeof(tmps) / sizeof(_TCHAR));
			donelen = strnlen(tmps, _nlen);
			if(donelen <= 0) return -1;
			if(donelen >= _nlen) return -1;
			if(save_string_data(tmps, fio, sumseed, sizeof(tmps) / sizeof(_TCHAR)) <= 0) return i;
		}
		return _len;
	}
	return -1;
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
#endif /* _CSP_STATE_DATA_H */
