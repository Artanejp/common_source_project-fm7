
#if !defined(_CSP_STATE_DATA_H)
#define _CSP_STATE_DATA_H
#include "common.h"
#include "fileio.h"


#include <string>
#include <list>


class DLL_PREFIX csp_state_data_saver {
protected:
	FILEIO *fio;
public:
	csp_state_data_saver(FILEIO *_fio);
	~csp_state_data_saver();
	
	bool pre_proc_saving(uint32_t *sumseed, bool *__stat);
	bool pre_proc_loading(uint32_t *sumseed, bool *__stat);
	bool post_proc_saving(uint32_t *sumseed, bool *__stat);
	bool post_proc_loading(uint32_t *sumseed, bool *__stat);
	
	size_t save_string_data(const _TCHAR *p, uint32_t *sumseed, int maxlen = -1, bool *__stat = NULL);
	size_t  load_string_data(_TCHAR *p, uint32_t *sumseed, int maxlen, bool *__stat = NULL);
	// ALL OF BYNARY VALUES SHOULD BE SAVED BY BIG ENDIAN
	int save_and_change_byteorder_be(uint32_t *sum, void *val, int bytes = 4, int rep = 1);
	int load_and_change_byteorder_be(uint32_t *sum, void *val, int bytes = 4, int rep = 1);
	
	void put_byte(uint8_t val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	uint8_t get_byte(uint32_t *sumseed = NULL, bool *__stat = NULL);
	
	void put_bool(bool val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	bool get_bool(uint32_t *sumseed = NULL, bool *__stat = NULL);
	
	void put_word(uint16_t val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	uint16_t get_word(uint32_t *sumseed = NULL, bool *__stat = NULL);
	
	void put_dword(uint32_t val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	uint32_t get_dword(uint32_t *sumseed = NULL, bool *__stat = NULL);
	
	void put_qword(uint64_t val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	uint64_t get_qword(uint32_t *sumseed = NULL, bool *__stat = NULL);

	void put_int8(int8_t val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	int8_t get_int8(uint32_t *sumseed = NULL, bool *__stat = NULL);
	
	void put_int16(int16_t val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	int16_t get_int16(uint32_t *sumseed = NULL, bool *__stat = NULL);
	
	void put_int32(int32_t val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	int32_t get_int32(uint32_t *sumseed = NULL, bool *__stat = NULL);
	
	void put_int64(int64_t val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	int64_t get_int64(uint32_t *sumseed = NULL, bool *__stat = NULL);

	void put_float(float val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	float get_float(uint32_t *sumseed = NULL, bool *__stat = NULL);

	void put_double(double val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	double get_double(uint32_t *sumseed = NULL, bool *__stat = NULL);

	void put_long_double(long double val, uint32_t *sumseed = NULL, bool *__stat = NULL);
	long double get_long_double(uint32_t *sumseed = NULL, bool *__stat = NULL);

	size_t get_byte_array(uint8_t *p, size_t len, size_t repeat, uint32_t *sumseed = NULL, bool *__stat = NULL);
	size_t put_byte_array(uint8_t *p, size_t len, size_t repeat, uint32_t *sumseed = NULL, bool *__stat = NULL);
	
	pair_t get_pair32(uint32_t *sumseed = NULL, bool *__stat = NULL);
	void put_pair32(pair_t val, uint32_t *sumseed = NULL, bool *__stat = NULL);

	pair64_t get_pair64(uint32_t *sumseed = NULL, bool *__stat = NULL);
	void put_pair64(pair64_t val, uint32_t *sumseed = NULL, bool *__stat = NULL);

};


#endif /* _CSP_STATE_DATA_H */
