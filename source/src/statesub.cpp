
#include "common.h"
#include "fileio.h"

#include "state_data.h"
#include "statesub.h"

csp_state_data_saver::csp_state_data_saver(FILEIO *_fio)
{
	fio = _fio;
}

csp_state_data_saver::~csp_state_data_saver()
{
}

size_t csp_state_data_saver::save_string_data(const _TCHAR *p, uint32_t *sumseed, int maxlen, bool *__stat)
{
	size_t locallen;
	if(__stat != NULL) *__stat =  false;
	if(p == NULL) return -1;
	if(maxlen <= 0) {
		locallen = strlen(p);
	} else {
		locallen = strnlen(p, maxlen);
	}
	if(locallen < 0) return locallen;
	locallen += 1; // Include "\0";
	if(sumseed != NULL) {
		*sumseed = calc_crc32(*sumseed, (uint8_t *)p, locallen * sizeof(char));
	}
	size_t cp;
	for(cp = 0; cp < locallen; cp++) {
		int _t = (int)(p[cp]);
		int res = fio->Fputc(_t);
		if((_t == '\0') || (_t == 0x00)){
			cp++;
			if(__stat != NULL) *__stat =  true;
			break;
		} else if(_t == EOF) {
			return (size_t)EOF; // ERROR
		}
	}
	return locallen;
}

size_t csp_state_data_saver::load_string_data(_TCHAR *p, uint32_t *sumseed, int maxlen, bool *__stat)
{
	size_t cp;
	size_t _nlen;
	if(__stat != NULL) *__stat =  false;
	if(p == NULL) return -1;
	if(maxlen <= 0) return -1;
	memset(p, 0x00, sizeof(_TCHAR) * maxlen);
	for(cp = 0; cp < maxlen; cp++) {
		int _t = fio->Fgetc();
		if((_t == '\0') || (_t == 0x00)){
			p[cp] = 0x00;
			cp++;
			if(__stat != NULL) *__stat =  true;
			break;
		} else  if(_t == EOF) {
			p[cp] = 0x00;
			cp++;
			if(__stat != NULL) *__stat =  false;
			break;
		} else if((cp + 1) >= maxlen) {
			p[cp] = (_TCHAR)_t;
			if(__stat != NULL) *__stat =  true;
			cp++;
			break;
		}
		p[cp] = (_TCHAR)_t;
	}
	if(cp >= maxlen) {
		p[maxlen - 1] = '\0';
		cp = maxlen;
	}
	_nlen = (size_t)cp;
	if(sumseed != NULL){
		*sumseed = calc_crc32(*sumseed, (uint8_t *)p, _nlen * sizeof(_TCHAR));
	}
	return _nlen;
}
// ALL OF BYNARY VALUES SHOULD BE SAVED BY BIG ENDIAN
int csp_state_data_saver::save_and_change_byteorder_be(uint32_t *sum, void *val, int bytes, int rep)
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

int csp_state_data_saver::load_and_change_byteorder_be(uint32_t *sum, void *val, int bytes, int rep)
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

void csp_state_data_saver::put_byte(uint8_t val, uint32_t *sumseed, bool *__stat)
{
	
	if(fio != NULL) {
		if(fio->IsOpened()) {
			fio->FputUint8(val);
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, &val, 1);
			}
			if(__stat != NULL) *__stat =  true;
			return;
		}
	}
	if(__stat != NULL) *__stat =  false;
	return;
}

uint8_t csp_state_data_saver::get_byte(uint32_t *sumseed, bool *__stat)
{
	if(fio != NULL) {
		if(fio->IsOpened()) {
			uint8_t val = fio->FgetUint8();
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, &val, 1);
			}
			if(__stat != NULL) *__stat =  true;
			return val;
		}
	}
	if(__stat != NULL) *__stat =  false;
	return 0x00000000;
}

void csp_state_data_saver::put_bool(bool val, uint32_t *sumseed, bool *__stat)
{
	uint8_t tv;
	tv = (val == false) ? 0x00 : 0x01;
	if(fio != NULL) {
		if(fio->IsOpened()) {
			fio->FputUint8(tv);
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, &tv, 1);
			}
			if(__stat != NULL) *__stat =  true;
			return;
		}
	}
	if(__stat != NULL) *__stat =  false;
	return;
}

bool csp_state_data_saver::get_bool(uint32_t *sumseed, bool *__stat)
{
	if(fio != NULL) {
		if(fio->IsOpened()) {
			uint8_t val = fio->FgetUint8();
			if(val != 0x00) val = 0x01;
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, &val, 1);
			}
			if(__stat != NULL) *__stat =  true;
			return (val == 0x00) ? false : true;
		}
	}
	if(__stat != NULL) *__stat =  false;
	return false;
}

void csp_state_data_saver::put_word(uint16_t val, uint32_t *sumseed, bool *__stat)
{
	if(fio != NULL) {
		if(fio->IsOpened()) {
			fio->FputUint16_BE(val);
			if(sumseed != NULL) {
				pair_t tval;
				uint8_t buf[2];
				tval.w.l = val;
				tval.write_2bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 2);
			}
			if(__stat != NULL) *__stat =  true;
			return;
		}
	}
	if(__stat != NULL) *__stat =  false;
	return;
}

uint16_t csp_state_data_saver::get_word(uint32_t *sumseed, bool *__stat)
{
	if(fio != NULL) {
		if(fio->IsOpened()) {
			uint16_t val = fio->FgetUint16_BE();
			if(__stat != NULL) *__stat =  true;
			if(sumseed != NULL) {
				pair_t tval;
				uint8_t buf[2];
				tval.w.l = val;
				tval.write_2bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 2);
			}
			return val;
		}
	}
	if(__stat != NULL) *__stat =  false;
	return 0x00000000;
}

void csp_state_data_saver::put_dword(uint32_t val, uint32_t *sumseed, bool *__stat)
{
	
	if(fio != NULL) {
		if(fio->IsOpened()) {
			fio->FputUint32_BE(val);
			if(__stat != NULL) *__stat =  true;
			if(sumseed != NULL) {
				pair_t tval;
				uint8_t buf[4];
				tval.d = val;
				tval.write_4bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 4);
			}
			return;
		}
	}
	if(__stat != NULL) *__stat =  false;
	return;
}

uint32_t csp_state_data_saver::get_dword(uint32_t *sumseed, bool *__stat)
{
	if(fio != NULL) {
		if(fio->IsOpened()) {
			uint32_t val = fio->FgetUint32_BE();
			if(__stat != NULL) *__stat =  true;
			if(sumseed != NULL) {
				pair_t tval;
				uint8_t buf[4];
				tval.d = val;
				tval.write_4bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 4);
			}
			return val;
		}
	}
	if(__stat != NULL) *__stat =  false;
	return 0x00000000;
}

void csp_state_data_saver::put_qword(uint64_t val, uint32_t *sumseed, bool *__stat)
{
	
	if(fio != NULL) {
		if(fio->IsOpened()) {
			fio->FputUint64_BE(val);
			if(__stat != NULL) *__stat =  true;
			if(sumseed != NULL) {
				pair64_sav_t tval;
				uint8_t buf[8];
				tval.u64 = val;
				tval.write_8bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 8);
			}
			return;
		}
	}
	if(__stat != NULL) *__stat =  false;
	return;
}

uint64_t csp_state_data_saver::get_qword(uint32_t *sumseed, bool *__stat)
{
	if(fio != NULL) {
		if(fio->IsOpened()) {
			uint64_t val = fio->FgetUint64_BE();
			if(__stat != NULL) *__stat =  true;
			if(sumseed != NULL) {
				pair64_sav_t tval;
				uint8_t buf[8];
				tval.u64 = val;
				tval.write_8bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 8);
			}
			return val;
		}
	}
	if(__stat != NULL) *__stat =  false;
	return 0x00000000;
}

void csp_state_data_saver::put_int8(int8_t val, uint32_t *sumseed, bool *__stat)
{
	
	if(fio != NULL) {
		if(fio->IsOpened()) {
			fio->FputInt8(val);
			if(__stat != NULL) *__stat =  true;
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, &val, 1);
			}
			return;
		}
	}
	if(__stat != NULL) *__stat = false;
	return;
}

int8_t csp_state_data_saver::get_int8(uint32_t *sumseed, bool *__stat)
{
	if(fio != NULL) {
		if(fio->IsOpened()) {
			int8_t val = fio->FgetInt8();
			if(__stat != NULL) *__stat = true;
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, &val, 1);
			}
			return val;
		}
	}
	if(__stat != NULL) *__stat = false;
	return 0x00000000;
}

void csp_state_data_saver::put_int16(int16_t val, uint32_t *sumseed, bool *__stat)
{
	
	if(fio != NULL) {
		if(fio->IsOpened()) {
			fio->FputInt16_BE(val);
			if(__stat != NULL) *__stat = true;
			if(sumseed != NULL) {
				pair_t tval;
				uint8_t buf[2];
				tval.sw.l = val;
				tval.write_2bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 2);
			}
			return;
		}
	}
	if(__stat != NULL) *__stat = false;
	return;
}

int16_t csp_state_data_saver::get_int16(uint32_t *sumseed, bool *__stat)
{
	if(fio != NULL) {
		if(fio->IsOpened()) {
			int16_t val = fio->FgetInt16_BE();
			if(__stat != NULL) *__stat = true;
			if(sumseed != NULL) {
				pair_t tval;
				uint8_t buf[2];
				tval.sw.l = val;
				tval.write_2bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 2);
			}
			return val;
		}
	}
	if(__stat != NULL) *__stat = false;
	return 0x00000000;
}

void csp_state_data_saver::put_int32(int32_t val, uint32_t *sumseed, bool *__stat)
{
	
	if(fio != NULL) {
		if(fio->IsOpened()) {
			fio->FputInt32_BE(val);
			if(__stat != NULL) *__stat = true;
			if(sumseed != NULL) {
				pair_t tval;
				uint8_t buf[4];
				tval.sd = val;
				tval.write_4bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 4);
			}
			return;
		}
	}
	if(__stat != NULL) *__stat = false;
	return;
}

int32_t csp_state_data_saver::get_int32(uint32_t *sumseed, bool *__stat)
{
	if(fio != NULL) {
		if(fio->IsOpened()) {
			int32_t val = fio->FgetInt32_BE();
			if(__stat != NULL) *__stat = true;
			if(sumseed != NULL) {
				pair_t tval;
				uint8_t buf[4];
				tval.sd = val;
				tval.write_4bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 4);
			}
			return val;
		}
	}
	if(__stat != NULL) *__stat = false;
	return 0x00000000;
}

void csp_state_data_saver::put_int64(int64_t val, uint32_t *sumseed, bool *__stat)
{
	
	if(fio != NULL) {
		if(fio->IsOpened()) {
			fio->FputInt64_BE(val);
			if(__stat != NULL) *__stat = true;
			if(sumseed != NULL) {
				pair64_sav_t tval;
				uint8_t buf[8];
				tval.s64 = val;
				tval.write_8bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 8);
			}
			return;
		}
	}
	if(__stat != NULL) *__stat = false;
	return;
}

int64_t csp_state_data_saver::get_int64(uint32_t *sumseed, bool *__stat)
{
	if(fio != NULL) {
		if(fio->IsOpened()) {
			int64_t val = fio->FgetInt64_BE();
			if(__stat != NULL) *__stat = true;
			if(sumseed != NULL) {
				pair64_sav_t tval;
				uint8_t buf[8];
				tval.s64 = val;
				tval.write_8bytes_be_to(buf);
				*sumseed = calc_crc32(*sumseed, buf, 8);
			}
			return val;
		}
	}
	if(__stat != NULL) *__stat = false;
	return 0x00000000;
}

/*
 * For floating values, internal format is differnt by ARCHTECTURE, OS, COMPILER etc.
 * So, saving / loading those values by ascii, not binary.
 * -- 20180520 K.Ohta.
 */
#include <stdexcept>

float csp_state_data_saver::get_float(uint32_t *sumseed, bool *__stat)
{
	size_t _sp;
	std::string _s;
	char tmps[1024]; // OK?
	float _v;
	size_t _nlen = sizeof(tmps) / sizeof(char);
	size_t donelen;
	if(fio != NULL) {
		memset(tmps, 0x00, sizeof(tmps));
		donelen = load_string_data(tmps, sumseed, _nlen - 1); 
		if((donelen <= 0) || (donelen >= _nlen)) {  // 
			if(__stat != NULL) *__stat = false;
			return 0.0f;
		}
		_s = std::string(tmps);
		try {
			_v = std::stof(_s, &_sp);
		} catch(const std::invalid_argument& e) {
			if(__stat != NULL) *__stat = false;
			return 0.0f;
		} catch(const std::out_of_range& e) {
			if(__stat != NULL) *__stat =  false;
			return 0.0f;
		}
		if(__stat != NULL) *__stat =  true;
		if(sumseed != NULL) {
			*sumseed = calc_crc32(*sumseed, tmps, donelen);
		}
		return _v;
	}
	if(__stat != NULL) *__stat =  true;
	return 0.0f;
}

void csp_state_data_saver::put_float(float val, uint32_t *sumseed, bool *__stat)
{
	std::string _s;
	char tmps[1024]; // OK?
	float _v = val;
	int _nlen = sizeof(tmps) / sizeof(char);
	size_t donelen;
	if(fio != NULL) {
		memset(tmps, 0x00, sizeof(tmps));
		_s = std::to_string(_v);
		_s.copy(tmps, _nlen - 1);
		size_t rlen = strnlen(tmps, _nlen);
		if((rlen <= 0) || (rlen >= _nlen)) { //
			if(__stat != NULL) *__stat =  false;
			return;
		}
		donelen = save_string_data(tmps, sumseed, rlen);
		if((donelen <= 0) || (donelen != (rlen + 1))) {
			if(__stat != NULL) *__stat =  false;
			return;
		}			
		if(sumseed != NULL) {
			*sumseed = calc_crc32(*sumseed, tmps, donelen);
		}
		if(__stat != NULL) *__stat =  true;
		return;
	}
	if(__stat != NULL) *__stat =  false;
	return;
}

double csp_state_data_saver::get_double(uint32_t *sumseed, bool *__stat)
{
	std::string _s;
	char tmps[1024]; // OK?
	double _v;
	size_t _nlen = sizeof(tmps) / sizeof(char);
	size_t donelen;
	size_t _sp;
	if(fio != NULL) {
		memset(tmps, 0x00, sizeof(tmps));
		donelen = load_string_data(tmps, sumseed, _nlen - 1); 
		if((donelen <= 0) || (donelen >= _nlen)) {  // 
			if(__stat != NULL) *__stat =  false;
			return 0.0;
		}
		_s = std::string(tmps);
		try {
			_v = std::stod(_s, &_sp);
		} catch(const std::invalid_argument& e) {
			if(__stat != NULL) *__stat =  false;
			return 0.0;
		} catch(const std::out_of_range& e) {
			if(__stat != NULL) *__stat =  false;
			return 0.0;
		}
		if(__stat != NULL) *__stat =  true;
		if(sumseed != NULL) {
			*sumseed = calc_crc32(*sumseed, tmps, donelen);
		}
		return _v;
	}
	if(__stat != NULL) *__stat =  true;
	return 0.0;
}

void csp_state_data_saver::put_double(double val, uint32_t *sumseed, bool *__stat)
{
	std::string _s;
	char tmps[1024]; // OK?
	double _v = val;
	int _nlen = sizeof(tmps) / sizeof(char);
	size_t donelen;
	if(fio != NULL) {
		memset(tmps, 0x00, sizeof(tmps));
		_s = std::to_string(_v);
		_s.copy(tmps, _nlen - 1);
		size_t rlen = strnlen(tmps, _nlen);
		if((rlen <= 0) || (rlen >= _nlen)) { //
			if(__stat != NULL) *__stat =  false;
			return;
		}
		donelen = save_string_data(tmps, sumseed, rlen);
		if((donelen <= 0) || (donelen != (rlen + 1))) {
			if(__stat != NULL) *__stat =  false;
			return;
		}			
		if(sumseed != NULL) {
			*sumseed = calc_crc32(*sumseed, tmps, donelen);
		}
		if(__stat != NULL) *__stat =  true;
		return;
	}
	if(__stat != NULL) *__stat =  false;
	return;
}

long double csp_state_data_saver::get_long_double(uint32_t *sumseed, bool *__stat)
{
	size_t _sp;
	std::string _s;
	char tmps[1024]; // OK?
	long double _v;
	size_t _nlen = sizeof(tmps) / sizeof(char);
	size_t donelen;
	if(fio != NULL) {
		memset(tmps, 0x00, sizeof(tmps));
		donelen = load_string_data(tmps, sumseed, _nlen - 1); 
		if((donelen <= 0) || (donelen >= _nlen)) {  // 
			if(__stat != NULL) *__stat =  false;
			return 0.0;
		}
		_s = std::string(tmps);
		try {
			_v = std::stold(_s, &_sp);
		} catch(const std::invalid_argument& e) {
			if(__stat != NULL) *__stat =  false;
			return 0.0;
		} catch(const std::out_of_range& e) {
			if(__stat != NULL) *__stat =  false;
			return 0.0;
		}
		if(__stat != NULL) *__stat =  true;
		if(sumseed != NULL) {
			*sumseed = calc_crc32(*sumseed, tmps, donelen);
		}
		return _v;
	}
	if(__stat != NULL) *__stat =  true;
	return 0.0;
}

void csp_state_data_saver::put_long_double(long double val, uint32_t *sumseed, bool *__stat)
{
	std::string _s;
	char tmps[1024]; // OK?
	long double _v = val;
	int _nlen = sizeof(tmps) / sizeof(char);
	size_t donelen;
	if(fio != NULL) {
		memset(tmps, 0x00, sizeof(tmps));
		_s = std::to_string(_v);
		_s.copy(tmps, _nlen - 1);
		size_t rlen = strnlen(tmps, _nlen);
		if((rlen <= 0) || (rlen >= _nlen)) { //
			if(__stat != NULL) *__stat =  false;
			return;
		}
		donelen = save_string_data(tmps, sumseed, rlen);
		if((donelen <= 0) || (donelen != (rlen + 1))) {
			if(__stat != NULL) *__stat =  false;
			return;
		}			
		if(sumseed != NULL) {
			*sumseed = calc_crc32(*sumseed, tmps, donelen);
		}
		if(__stat != NULL) *__stat =  true;
		return;
	}
	if(__stat != NULL) *__stat =  false;
	return;
}

size_t csp_state_data_saver::get_byte_array(uint8_t *p, size_t len, size_t repeat, uint32_t *sumseed, bool *__stat)
{
	size_t donelen = 0;
	bool _nstat = true;
	if((len <= 0) || (repeat <= 0) || (p == NULL)) {
		_nstat = false;
	}
	if(fio == NULL) _nstat = false;
	if(_nstat) {
		donelen = fio->Fread(p, len, repeat);
		if(donelen != repeat) {
			_nstat = false;
		} else {
			// Data OK.
			int calcsize = len * donelen;
			if((calcsize > 0) && (sumseed != NULL)) {
				*sumseed = calc_crc32(*sumseed, p, calcsize);
			}
			_nstat = true;
		}
	}
	if(__stat != NULL) *__stat =  _nstat;
	if(_nstat) {
		return len * donelen; // RETURN BYTES;
	} else {
		return -1;
	}
}

size_t csp_state_data_saver::put_byte_array(uint8_t *p, size_t len, size_t repeat, uint32_t *sumseed, bool *__stat)
{
	size_t donelen = 0;
	bool _nstat = true;
	if((len <= 0) || (repeat <= 0) || (p == NULL)) {
		_nstat = false;
	}
	if(fio == NULL) _nstat = false;
	if(_nstat) {
		donelen = fio->Fwrite(p, len, repeat);
		if(donelen != repeat) {
			_nstat = false;
		} else {
			// Data OK.
			int calcsize = len * donelen;
			if((calcsize > 0) && (sumseed != NULL)) {
				*sumseed = calc_crc32(*sumseed, p, calcsize);
			}
			_nstat = true;
		}
	}
	if(__stat != NULL) *__stat =  _nstat;
	if(_nstat) {
		return len * donelen; // RETURN BYTES;
	} else {
		return -1;
	}
}


void csp_state_data_saver::put_pair32(pair_t val, uint32_t *sumseed, bool *__stat)
{
	
	if(fio != NULL) {
		if(fio->IsOpened()) {
			uint8_t buf[4];
			val.write_4bytes_be_to(buf);
			size_t memb = fio->Fwrite(buf, 1, 4);
			if(memb != 4) {
				if(__stat != NULL) *__stat =  false;
				return;
			}
			if(__stat != NULL) *__stat = true;
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, buf, 4);
			}
			return;
		}
	}
	if(__stat != NULL) *__stat = false;
	return;
}

pair_t csp_state_data_saver::get_pair32(uint32_t *sumseed, bool *__stat)
{
	pair_t val;
	val.d = 0;
	if(fio != NULL) {
		if(fio->IsOpened()) {
			uint8_t buf[4];
			size_t memb = fio->Fread(buf, 1, 4);
			if(memb != 4) {
				if(__stat != NULL) *__stat = false;
				return val;
			}
			val.read_4bytes_be_from(buf);
			if(__stat != NULL) *__stat = true;
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, buf, 4);
			}
			return val;
		}
	}
	if(__stat != NULL) *__stat = false;
	return val;
}

pair64_sav_t csp_state_data_saver::get_pair64(uint32_t *sumseed, bool *__stat)
{
	pair64_sav_t val;
	val.u64 = 0;
	if(fio != NULL) {
		if(fio->IsOpened()) {
			uint8_t buf[8];
			size_t memb = fio->Fread(buf, 1, 8);
			if(memb != 8) {
				if(__stat != NULL) *__stat = false;
				return val;
			}
			val.read_8bytes_be_from(buf);
			if(__stat != NULL) *__stat = true;
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, buf, 8);
			}
			return val;
		}
	}
	if(__stat != NULL) *__stat = false;
	return val;
}

void csp_state_data_saver::put_pair64(pair64_sav_t val, uint32_t *sumseed, bool *__stat)
{
	if(fio != NULL) {
		if(fio->IsOpened()) {
			uint8_t buf[8];
			val.write_8bytes_be_to(buf);
			size_t memb = fio->Fwrite(buf, 1, 8);
			if(memb != 8) {
				if(__stat != NULL) *__stat = false;
				return;
			}
			if(__stat != NULL) *__stat = true;
			if(sumseed != NULL) {
				*sumseed = calc_crc32(*sumseed, buf, 8);
			}
			return;
		}
	}
	if(__stat != NULL) *__stat = false;
	return;
}

csp_state_utils::csp_state_utils(int _version, int device_id, _TCHAR *classname)
{
	listptr.clear();
	crc_value = 0;
	listptr.clear();
		
	this_device_id = device_id;
	this_device_id_bak = this_device_id;
		
	memset(__classname, 0x00, sizeof(__classname));
	memset(__classname_bak, 0x00, sizeof(__classname_bak));
	memset(magic, 0x00, sizeof(magic));
	memset(magic_bak, 0x00, sizeof(magic_bak));
	
	strncpy(magic, _T("CSP_SAVE"), 16);
	strncpy(magic_bak, magic, 16);
	
	if(classname != NULL) {
		strncpy(__classname, classname, sizeof(__classname) - 1); 
	} else {
		strncpy(__classname, "Unknown Class", sizeof(__classname) - 1); 
	}
	strncpy(__classname_bak, __classname, sizeof(__classname_bak) - 1);
		
	class_version = _version;
	class_version_bak = _version;

	internal_version = 1;
	internal_version_bak = internal_version;
	out_debug_log("NEW SAVE STATE: NAME=%s DEVID=%d VER=%d", classname, device_id, _version);
	
	add_entry_tchar(_T("HEADER"), magic_bak, strlen(magic_bak) + 1);
	add_entry_tchar(_T("CLASSNAME"), __classname_bak, strlen(__classname_bak) + 1);
	add_entry(_T("DEVICE_ID"), &this_device_id_bak);
	add_entry(_T("STAVE_VER"), &class_version_bak);	
	add_entry(_T("INTERNAL_VER"), &internal_version_bak);	

	fio = NULL;
}

csp_state_utils::~csp_state_utils()
{
}

#include "config.h"
#include "csp_logger.h"
#if defined(_USE_QT)
#	include <QtGlobal>
#endif
extern CSP_Logger DLL_PREFIX_I *csp_logger;

void csp_state_utils::out_debug_log(const char *fmt, ...)
{
	char strbuf[8192];
	
	va_list ap;
	
	va_start(ap, fmt);	
	vsnprintf(strbuf, 8192, fmt, ap);
	//csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_VM_STATE, strbuf);
#if defined(_USE_QT)
	if((config.state_log_to_console) || (config.state_log_to_syslog) 
	   || (config.state_log_to_recording)) {
		qInfo("[STATE] %s", strbuf);
	}
#endif
	va_end(ap);
}

std::list<std::string> csp_state_utils::get_entries_list(void)
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
			case csp_saver_entry_float:
				_tname = _T("TYPE: float");
				break;
			case csp_saver_entry_double:
				_tname = _T("TYPE: double");
				break;
			case csp_saver_entry_long_double:
				_tname = _T("TYPE: long double");
				break;
			case csp_saver_entry_pair:
				_tname = _T("TYPE: pair_t");
				break;
			case csp_saver_entry_int:
				_tname = _T("TYPE: int");
				break;
			case csp_saver_entry_uint8:
				_tname = _T("TYPE: uint8_t");
				break;
			case csp_saver_entry_int8:
				_tname = _T("TYPE: int8_t");
				break;
			case csp_saver_entry_uint16:
				_tname = _T("TYPE: uint16_t");
				break;
			case csp_saver_entry_int16:
				_tname = _T("TYPE: int16_t");
				break;
			case csp_saver_entry_uint32:
				_tname = _T("TYPE: uint32_t");
				break;
			case csp_saver_entry_int32:
				_tname = _T("TYPE: int32_t");
				break;
			case csp_saver_entry_uint64:
				_tname = _T("TYPE: uint64_t");
				break;
			case csp_saver_entry_int64:
				_tname = _T("TYPE: int64_t");
				break;
			case csp_saver_entry_bool:
				_tname = _T("TYPE: bool");
				break;
			case csp_saver_entry_tchar:
				_tname = _T("TYPE: _TCHAR");
				break;
			case csp_saver_entry_any:
				_tname = _T("TYPE: ANY");
				break;
			default:
				_tname = _T("TYPE: ANY(UNKNOWN)");
				break;
			}
			_vname = _vname + (*p).name;
			_rlist.push_back(_vname);
			_rlist.push_back(_tname);
		}
	}
	return _rlist;
}
uint32_t csp_state_utils::get_crc_value(void)
{
	return crc_value;
}
void csp_state_utils::get_class_name(_TCHAR *buf, int len)
{
	strncpy(buf, __classname_bak, ((size_t)len > sizeof(__classname_bak)) ? sizeof(__classname_bak) : len);
}

bool csp_state_utils::save_state(FILEIO *__fio, uint32_t *pcrc)
{
	const uint8_t initdata[4] = {0xff, 0xff, 0xff, 0xff};
	const uint8_t taildata[4] = {0x00, 0x00, 0x00, 0x00};
	int retval;

	class_version_bak = class_version;
	this_device_id_bak = this_device_id;
	memset(__classname_bak, 0x00, sizeof(__classname_bak));
	strncpy(__classname_bak, __classname, sizeof(__classname));
	// Initialize crc;
	if(pcrc == NULL) {
		crc_value = 0xffffffff;
	} else {
		crc_value = *pcrc;
	}
	// Write header
	crc_value = calc_crc32(crc_value, initdata, 4);

	out_debug_log("SAVE STATE: NAME=%s DEVID=%d VER=%d", __classname, this_device_id, class_version);
	
	fio = new csp_state_data_saver(__fio);
	if(fio != NULL) {
		for(auto p = listptr.begin(); p != listptr.end(); ++p) {
			void *pp = NULL;
			int _tid = (*p).type_id;
			int _asize = (*p).atomlen;
			int _len = (*p).len;
			
			bool _stat;
			if((_tid & csp_saver_entry_vararray) != 0) {
				if((*p).datalenptr != NULL) {
					_len = *((*p).datalenptr);
				} else {
					_len = 0;
				}
				(*p).len =_len;
				_tid = _tid & ((int)~csp_saver_entry_vararray);
				void **xp = (void **)((*p).ptr);
				if(*xp != NULL) {
					pp = (void *)(*xp);
				} else {
					pp = NULL;
					_len = 0;
				}
				fio->put_int32(_len, &crc_value, &_stat);
				out_debug_log("SAVE VARARRAY p=%08x len=%d atom=%d CRC=%08x",pp, _len, _asize, crc_value); 
			} else {
				pp = (*p).ptr;
			}
			if((pp != NULL) && (_len > 0)) {
				switch(_tid) {
				case csp_saver_entry_float:
					{
						retval = 0;
						float *px = (float *)pp;
						for(int i = 0; i < _len; i++) {
							fio->put_float(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_double:
					{
						retval = 0;
						double *px = (double *)pp;
						for(int i = 0; i < _len; i++) {
							fio->put_double(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_long_double:
					{
						retval = 0;
						long double *px = (long double *)pp;
						for(int i = 0; i < _len; i++) {
							fio->put_long_double(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_pair:
					{
						retval = 0;
						pair_t *px = (pair_t *)pp;
						for(int i = 0; i < _len; i++) {
							fio->put_pair32(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_int:
					{
						retval = 0;
						int *px = (int *)pp;
						for(int i = 0; i < _len; i++) {
							fio->put_int32(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_uint8:
					{
						retval = 0;
						uint8_t *px = (uint8_t *)pp;
						if(_len > 1) {
							size_t _n = __fio->Fwrite(px, _len, 1);
							if(_n != 1) {
								retval = 0;
								_stat = false;
							} else {
								retval = _len;
								_stat = true;
								crc_value = calc_crc32(crc_value, px, _len);
							}
						} else {
							fio->put_byte(*px, &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_int8:
					{
						retval = 0;
						int8_t *px = (int8_t *)pp;
						if(_len > 1) {
							size_t _n = __fio->Fwrite(px, _len, 1);
							if(_n != 1) {
								retval = 0;
								_stat = false;
							} else {
								retval = _len;
								_stat = true;
								crc_value = calc_crc32(crc_value, px, _len);
							}
						} else {
							fio->put_int8(*px, &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_uint16:
					{
						retval = 0;
						uint16_t *px = (uint16_t *)pp;
						for(int i = 0; i < _len; i++) {
							fio->put_word(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_int16:
					{
						retval = 0;
						int16_t *px = (int16_t *)pp;
						for(int i = 0; i < _len; i++) {
							fio->put_int16(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_uint32:
					{
						retval = 0;
						uint32_t *px = (uint32_t *)pp;
						for(int i = 0; i < _len; i++) {
							fio->put_dword(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_int32:
					{
						retval = 0;
						int32_t *px = (int32_t *)pp;
						for(int i = 0; i < _len; i++) {
							fio->put_int32(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_uint64:
					{
						retval = 0;
						uint64_t *px = (uint64_t *)pp;
						for(int i = 0; i < _len; i++) {
							fio->put_qword(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_int64:
					{
						retval = 0;
						int64_t *px = (int64_t *)pp;
						for(int i = 0; i < _len; i++) {
							fio->put_int64(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_bool:
					{
						retval = 0;
						bool *px = (bool *)pp;
						for(int i = 0; i < _len ; i++) {
							fio->put_bool(px[i], &crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					break;
				case csp_saver_entry_tchar:
					{
						retval = fio->save_string_data((const _TCHAR *)pp, &crc_value, _len, &_stat);
						if(!_stat) retval = -1;
					}
					break;
				case csp_saver_entry_any:
					{
						retval = fio->put_byte_array((uint8_t *)pp, _asize, _len, &crc_value, &_stat);
						if(!_stat) retval = -1;
					}
					break;
				default:
					retval = 0;
					break;
				}
				if((retval <= 0) || (!_stat)) {
					delete fio;
					return false;
				}

			}
			out_debug_log("CRC=%08x", crc_value);
		}
		delete fio;
	}
	fio = NULL;
	// embed CRC
	crc_value = calc_crc32(crc_value, taildata, 4);
	__fio->FputUint32_BE(crc_value);
	out_debug_log("CRC: VAL=%08x", crc_value);
	if(pcrc != NULL) *pcrc = crc_value;
	return true;
}

bool csp_state_utils::load_state(FILEIO *__fio, uint32_t *pcrc)
{
	const uint8_t initdata[4] = {0xff, 0xff, 0xff, 0xff};
	const uint8_t taildata[4] = {0x00, 0x00, 0x00, 0x00};
	int retval;
	// Initialize crc;
	//class_version_bak = class_version;
	//this_device_id_bak = this_device_id;
	memset(__classname_bak, 0x00, sizeof(__classname_bak));
	//strncpy(__classname_bak, __classname, sizeof(__classname));
	if(pcrc == NULL) {
		crc_value = 0xffffffff;
	} else {
		crc_value = *pcrc;
	}
	crc_value = calc_crc32(crc_value, initdata, 4);

	fio = new csp_state_data_saver(__fio);
	out_debug_log("LOAD STATE: NAME=%s DEVID=%d VER=%d", __classname, this_device_id, class_version);
	if(fio != NULL) {
		for(auto p = listptr.begin(); p != listptr.end(); ++p) {
			void *pp = NULL;
			int _tid = (*p).type_id;
			int _asize = (*p).atomlen;
			int _len = (*p).len;
			std::string _name = (*p).name;
			bool _stat;
			
			if((_tid & csp_saver_entry_vararray) != 0) {
				_len = fio->get_int32(&crc_value, &_stat);
				void **xp = (void **)((*p).ptr);
				if(*xp != NULL) {
			   		free(*xp);
				}
				*xp = NULL;
			   	if(_len > 0) {
					*xp = malloc(_asize * _len);
					if(*xp == NULL) _len = 0;
				}
				if((*p).datalenptr != NULL) {
					*((*p).datalenptr) = _len;
				}
				pp = *xp;
				_tid = _tid & ((int)~csp_saver_entry_vararray);
				out_debug_log("LOAD VARARRAY p=%08x len=%d atom=%d CRC=%08x",pp, _len, _asize, crc_value); 
 			} else {
				pp = (*p).ptr;
			}
			if((pp != NULL) && (_len > 0)) {
				switch(_tid) {
				case csp_saver_entry_float:
					{
						retval = 0;
						float *px = (float *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_float(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					out_debug_log("NAME=%s FLOAT: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_double:
					{
						retval = 0;
						double *px = (double *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_double(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					out_debug_log("NAME=%s DOUBLE: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_long_double:
					{
						retval = 0;
						long double *px = (long double *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_long_double(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					out_debug_log("NAME=%s LONG DOUBLE: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_pair:
					{
						retval = 0;
						pair_t *px = (pair_t *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_pair32(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					out_debug_log("NAME=%s PAIR_T: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0,pp);
					break;
				case csp_saver_entry_int:
					{
						retval = 0;
						int *px = (int *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_int32(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					out_debug_log("NAME=%s INT: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_uint8:
					{
						retval = 0;
						uint8_t *px = (uint8_t *)pp;
						if((_len > 1) && (px != NULL)) {
							size_t _n = __fio->Fread(px, _len, 1);
							if(_n != 1) {
								retval = 0;
								_stat = false;
							} else {
								retval = _len;
								_stat = true;
								crc_value = calc_crc32(crc_value, px, _len);
							}
						} else {
							*px = fio->get_byte(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					out_debug_log("NAME=%s UINT8: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_int8:
					{
						retval = 0;
						int8_t *px = (int8_t *)pp;
						if((_len > 1) && (px != NULL)) {
							size_t _n = __fio->Fread(px, _len, 1);
							if(_n != 1) {
								retval = 0;
								_stat = false;
							} else {
								retval = _len;
								_stat = true;
								crc_value = calc_crc32(crc_value, px, _len);
							}
						} else {
							*px = fio->get_int8(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					out_debug_log("NAME=%s INT8: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_uint16:
					{
						retval = 0;
						uint16_t *px = (uint16_t *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_word(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					out_debug_log("NAMEE=%s UINT16: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_int16:
					{
						retval = 0;
						int16_t *px = (int16_t *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_int16(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					out_debug_log("NAME=%s INT16: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_uint32:
					{
						retval = 0;
						uint32_t *px = (uint32_t *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_dword(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
						retval++;
						}
					}
					out_debug_log("NAME=%s UINT32: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_int32:
					{
						retval = 0;
						int32_t *px = (int32_t *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_int32(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
					}
					out_debug_log("NAME=%s INT32: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_uint64:
					{
						retval = 0;
						uint64_t *px = (uint64_t *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_qword(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
							retval++;
						}
						
					}
					out_debug_log("NAME=%s UINT64: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_int64:
					{
						retval = 0;
						int64_t *px = (int64_t *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_int64(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
						retval++;
						}
					}
					out_debug_log("NAME=%s INT64: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_bool:
					{
						retval = 0;
						bool *px = (bool *)pp;
						for(int i = 0; i < _len; i++) {
							px[i] = fio->get_bool(&crc_value, &_stat);
							if(!_stat) {
								retval = -1;
								break;
							}
						retval++;
						}
					}
					out_debug_log("NAME=%s BOOL: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					break;
				case csp_saver_entry_tchar:
					{
						retval = fio->load_string_data((const _TCHAR *)pp, &crc_value, _len, &_stat);
						out_debug_log("NAME=%s STR: LEN=%d STAT=%d S=%s", _name.c_str(), retval, (_stat) ? 1 : 0, (_TCHAR *)pp);
						if(!_stat) retval = -1;
					}
					break;
				case csp_saver_entry_any:
					{
						retval = fio->get_byte_array((uint8_t *)pp, _asize, _len, &crc_value, &_stat);
						if(!_stat) retval = -1;
						out_debug_log("NAME=%s BYTE-ARRAY: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), retval, (_stat) ? 1 : 0, pp);
					}
					break;
				default:
					retval = 0;
					out_debug_log("NAME=%s UNKNOWN TID=%d: LEN=%d STAT=%d HEAD=%08x", _name.c_str(), _tid, retval, (_stat) ? 1 : 0, pp);
					break;
				}
				if((retval <= 0) || (!_stat)) {
					delete fio;
					return false;
				}
			}
			out_debug_log("CRC=%08x", crc_value);
		}
		delete fio;
	}
	fio = NULL;
	crc_value = calc_crc32(crc_value, taildata, 4);
	if(pcrc != NULL) *pcrc = crc_value;
	// check CRC
	uint32_t cmpval = __fio->FgetUint32_BE();
	if(cmpval != crc_value) {
		out_debug_log("CRC_ERROR: VAL=%08x / %08x", cmpval, crc_value);
		return false;
	}
	// Check header
	if(this_device_id != this_device_id_bak) {
		out_debug_log("WRONG device_id %d", this_device_id_bak);
		return false;
	}
	if(class_version != class_version_bak) {
		out_debug_log("WRONG crass_version %d", class_version_bak);
		return false;
	}
	if(strncmp(__classname, __classname_bak, sizeof(__classname)) != 0) {
		out_debug_log("WRONG CRASS NAME %s", __classname_bak);
		return false;
	}
	out_debug_log("OK CRC=%08x", crc_value);
	return true;
}
