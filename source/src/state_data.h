
#if !defined(_CSP_STATE_DATA_H)
#define _CSP_STATE_DATA_H
#include "common.h"
#include "fileio.h"


#include <string>
#include <list>

typedef union {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h7, h6, h5, h4, h3, h2, h, l;
#else
		uint8_t l, h, h2, h3, h4, h5, h6, h7;
#endif
	} b;
	struct {
#ifdef __BIG_ENDIAN__
		int8_t h7, h6, h5, h4, h3, h2, h, l;
#else
		int8_t l, h, h2, h3, h4, h5, h6, h7;
#endif
	} sb;
	struct {
#ifdef __BIG_ENDIAN__
		uint16_t h3, h2, h, l;
#else
		uint16_t l, h, h2, h3;
#endif
	} w;
	struct {
#ifdef __BIG_ENDIAN__
		int16_t h3, h2, h, l;
#else
		int16_t l, h, h2, h3;
#endif
	} sw;
	struct {
#ifdef __BIG_ENDIAN__
		uint32_t h, l;
#else
		uint32_t l, h;
#endif
	} d;
	struct {
#ifdef __BIG_ENDIAN__
		int32_t h, l;
#else
		int32_t l, h;
#endif
	} sd;
	struct {
#ifdef __BIG_ENDIAN__
		pair_t h, l;
#else
		pair_t l, h;
#endif
	} dwpair;
	uint64_t u64;
	int64_t s64;
	inline void clear(void)
	{
		u64 = 0;
	}
	inline void read_2bytes_le_from(uint8_t *t)
	{
		b.l = t[0]; b.h = t[1]; b.h2 = b.h3 = 0;
	}
	inline void write_2bytes_le_to(uint8_t *t)
	{
		t[0] = b.l; t[1] = b.h;
	}
	inline void read_2bytes_be_from(uint8_t *t)
	{
		b.h3 = b.h2 = 0; b.h = t[0]; b.l = t[1];
	}
	inline void write_2bytes_be_to(uint8_t *t)
	{
		t[0] = b.h; t[1] = b.l;
	}
	inline void read_4bytes_le_from(uint8_t *t)
	{
		b.l = t[0]; b.h = t[1]; b.h2 = t[2]; b.h3 = t[3];
	}
	inline void write_4bytes_le_to(uint8_t *t)
	{
		t[0] = b.l; t[1] = b.h; t[2] = b.h2; t[3] = b.h3;
	}
	inline void read_4bytes_be_from(uint8_t *t)
	{
		b.h3 = t[0]; b.h2 = t[1]; b.h = t[2]; b.l = t[3];
	}
	inline void write_4bytes_be_to(uint8_t *t)
	{
		t[0] = b.h3; t[1] = b.h2; t[2] = b.h; t[3] = b.l;
	}
	inline void read_8bytes_le_from(uint8_t *t)
	{
		b.l  = t[0]; b.h  = t[1]; b.h2 = t[2]; b.h3 = t[3];
		b.h4 = t[4]; b.h5 = t[5]; b.h6 = t[6]; b.h7 = t[7];
	}
	inline void write_8bytes_le_to(uint8_t *t)
	{
		t[0] = b.l;  t[1] = b.h;  t[2] = b.h2; t[3] = b.h3;
		t[4] = b.h4; t[5] = b.h5; t[6] = b.h6; t[7] = b.h7;
	}
	inline void read_8bytes_be_from(uint8_t *t)
	{
		b.h7 = t[0]; b.h6 = t[1]; b.h5 = t[2]; b.h4 = t[3];
		b.h3 = t[4]; b.h2 = t[5]; b.h  = t[6];  b.l = t[7];
	}
	inline void write_8bytes_be_to(uint8_t *t)
	{
		t[0] = b.h7; t[1] = b.h6; t[2] = b.h5; t[3] = b.h4;
		t[4] = b.h3; t[5] = b.h2; t[6] = b.h;  t[7] = b.l;
	}
} pair64_sav_t;

class csp_state_data_saver {
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

	pair64_sav_t get_pair64(uint32_t *sumseed = NULL, bool *__stat = NULL);
	void put_pair64(pair64_sav_t val, uint32_t *sumseed = NULL, bool *__stat = NULL);

};


#endif /* _CSP_STATE_DATA_H */
