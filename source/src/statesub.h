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
#include "state_data.h"

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
	csp_saver_entry_pair,
	csp_saver_entry_float,
	csp_saver_entry_double,
	csp_saver_entry_long_double,
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
					
public:
	csp_state_utils(int _version = 1, int device_id = 1, _TCHAR *classname = NULL);
	~csp_state_utils();
	std::list<std::string> get_entries_list(void);

	template <class T>
	void add_entry(csp_state_data<T> *p)
	{
		__list_t _l;
		_l.ptr = (void *)p;
		_l.type_id = csp_saver_entry_any;
		if(typeid(T) == typeid(int)) {
			_l.type_id = csp_saver_entry_int;
		} else if(typeid(T) == typeid(pair_t)) {
			_l.type_id = csp_saver_entry_pair;
		} else if(typeid(T) == typeid(float)) {
			_l.type_id = csp_saver_entry_float;
		} else if(typeid(T) == typeid(double)) {
			_l.type_id = csp_saver_entry_double;
		} else if(typeid(T) == typeid(long double)) {
			_l.type_id = csp_saver_entry_long_double;
		} else if(typeid(T) == typeid(int8_t)) {
			_l.type_id = csp_saver_entry_int8;
		} else if(typeid(T) == typeid(uint8_t)) {
			_l.type_id = csp_saver_entry_uint8;
		} else if(typeid(T) == typeid(int16_t)) {
			_l.type_id = csp_saver_entry_int16;
		} else if(typeid(T) == typeid(uint16_t)) {
			_l.type_id = csp_saver_entry_uint16;
		} else if(typeid(T) == typeid(int32_t)) {
			_l.type_id = csp_saver_entry_int32;
		} else if(typeid(T) == typeid(uint32_t)) {
			_l.type_id = csp_saver_entry_uint32;
		} else if(typeid(T) == typeid(int64_t)) {
			_l.type_id = csp_saver_entry_int64;
		} else if(typeid(T) == typeid(uint64_t)) {
			_l.type_id = csp_saver_entry_uint64;
		} else if(typeid(T) == typeid(bool)) {
			_l.type_id = csp_saver_entry_bool;
		} else if(typeid(T) == typeid(_TCHAR)) {
			_l.type_id = csp_saver_entry_tchar;
		}			
		listptr.push_back(_l);
	};
	uint32_t get_crc_value(void);
	void get_class_name(_TCHAR *buf, int len);
	bool save_state(FILEIO *fio, uint32_t *pcrc = NULL);
	bool load_state(FILEIO *fio, uint32_t *pcrc = NULL);
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
#define DECL_STATE_ENTRY_FLOAT(___name) DECL_STATE_ENTRY0(float, ___name, state_entry)
#define DECL_STATE_ENTRY_DOUBLE(___name) DECL_STATE_ENTRY0(double, ___name, state_entry)
#define DECL_STATE_ENTRY_LONG_DOUBLE(___name) DECL_STATE_ENTRY0(long double, ___name, state_entry)

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
#define DECL_STATE_ENTRY_FLOAT_ARRAY(___name, __len) DECL_STATE_ENTRY1(float, ___name, state_entry, __len)
#define DECL_STATE_ENTRY_DOUBLE_ARRAY(___name, __len) DECL_STATE_ENTRY1(double, ___name, state_entry, __len)
#define DECL_STATE_ENTRY_LONG_DOUBLE_ARRAY(___name, __len) DECL_STATE_ENTRY1(long double, ___name, state_entry, __len)

//#define DECL_STATE_ENTRY_MULTI(___name, ___size) DECL_STATE_ENTRY_MULTI0(uint8_t, ___name, state_entry, ___size)
#define DECL_STATE_ENTRY_MULTI(_n_type, ___name, ___size) DECL_STATE_ENTRY_MULTI0(_n_type, ___name, state_entry, ___size)


#endif /* _CSP_STATE_SUB_H */
