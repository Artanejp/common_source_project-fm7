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
 * 2. Call decl_state() from VM's decl_state().
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
	csp_saver_entry_any,
	// Below are special value
	csp_saver_entry_vararray = 65536,
};

class csp_state_utils {
protected:
	const uint32_t CRC_MAGIC_WORD = 0x04C11DB7;
	struct __list_t {
		int type_id;
		int len;
		int atomlen;
		std::string name;
		void *ptr;
		int *datalenptr;
	};
	std::list<__list_t>listptr;
	_TCHAR __classname[128];
	_TCHAR __classname_bak[128];
	
	uint32_t crc_value;
	_TCHAR magic[16];
	_TCHAR magic_bak[16];
	int class_version;
	int class_version_bak;
	int internal_version;
	int internal_version_bak;
	int this_device_id;
	int this_device_id_bak;
	csp_state_data_saver *fio;

	void out_debug_log(const char *fmt, ...);
public:
	csp_state_utils(int _version = 1, int device_id = 1, _TCHAR *classname = NULL);
	~csp_state_utils();
	std::list<std::string> get_entries_list(void);

	template <class T>
		void add_entry(_TCHAR *__name, T *p, int _len = 1)
	{
		__list_t _l;
		
		_l.ptr = (void *)p;
		_l.type_id = csp_saver_entry_any;
		_l.len = _len;
		_l.atomlen = sizeof(T);
		_l.name = std::string(__name);
		_l.datalenptr = NULL;
		out_debug_log("ADD ENTRY: NAME=%s TYPE=%s len=%d atomlen=%d", __name, typeid(T).name(), _len, _l.atomlen);
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
		}			
		listptr.push_back(_l);
	};
	template <class T>
		void add_entry_vararray(_TCHAR *__name, T *p, void *datalen)
	{
		__list_t _l;
		
		if(datalen == NULL) {
			add_entry(__name, p, 1);
			return;
		}
		_l.ptr = (void *)p;
		_l.type_id = 0;
		_l.len = 0;
		_l.atomlen = sizeof(T);
		_l.name = std::string(__name);
		_l.datalenptr = (int *) datalen;
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
		}
		_l.type_id = _l.type_id | csp_saver_entry_vararray;
		out_debug_log("ADD ENTRY(VARARRAY): NAME=%s TYPE=%s atomlen=%d linked len=%08x", __name, typeid(T).name(), _l.atomlen, datalen);
		listptr.push_back(_l);
	};
	void add_entry_tchar(_TCHAR *__name, _TCHAR *p, int _len = 1)
	{
		__list_t _l;
		if(p == NULL) return;
		_l.ptr = (void *)p;
		_l.type_id = csp_saver_entry_tchar;
		_l.len = _len;
		_l.atomlen = sizeof(_TCHAR);
		_l.name = std::string(__name);
		out_debug_log("ADD ENTRY: NAME=%s TYPE=_TCHAR* VAL=%s len=%d atomlen=%d HEAD=%08x", __name, p, _len, _l.atomlen, p);
		listptr.push_back(_l);
	};
	
	uint32_t get_crc_value(void);
	void get_class_name(_TCHAR *buf, int len);
	bool save_state(FILEIO *__fio, uint32_t *pcrc = NULL);
	bool load_state(FILEIO *__fio, uint32_t *pcrc = NULL);
};
	
			
#define DECL_STATE_ENTRY0(_n_name, __list) {				  \
		__list->add_entry(_T(#_n_name), &_n_name);		  \
	}

#define DECL_STATE_ENTRY1(_n_name, __list, __len) {				\
		__list->add_entry(_T(#_n_name), _n_name, __len);		\
	}

#define DECL_STATE_ENTRY_MULTI0(__type, _n_name, __list, __size) {		\
		__list->add_entry(_T(#_n_name), (uint8_t *)_n_name, __size * sizeof(__type)); \
	}

#define DECL_STATE_ENTRY_INT(___name) DECL_STATE_ENTRY0(___name, state_entry)

#define DECL_STATE_ENTRY_UINT8(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_INT8(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_UINT16(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_INT16(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_UINT32(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_INT32(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_UINT64(___name) DECL_STATE_ENTRY0( ___name, state_entry)
#define DECL_STATE_ENTRY_INT64(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_PAIR(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_BOOL(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_TCHAR(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_FLOAT(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_DOUBLE(___name) DECL_STATE_ENTRY0(___name, state_entry)
#define DECL_STATE_ENTRY_LONG_DOUBLE(___name) DECL_STATE_ENTRY0(___name, state_entry)

#define DECL_STATE_ENTRY_UINT8_ARRAY(___name, __len) DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_INT8_ARRAY(___name, __len)  DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_UINT16_ARRAY(___name, __len) DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_INT16_ARRAY(___name, __len)  DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_UINT32_ARRAY(___name, __len) DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_INT32_ARRAY(___name, __len) DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_UINT64_ARRAY(___name, __len) DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_INT64_ARRAY(___name, __len) DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_BOOL_ARRAY(___name, __len) DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_PAIR_ARRAY(___name, __len) DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_FLOAT_ARRAY(___name, __len) DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_DOUBLE_ARRAY(___name, __len) DECL_STATE_ENTRY1(___name, state_entry, __len)
#define DECL_STATE_ENTRY_LONG_DOUBLE_ARRAY(___name, __len) DECL_STATE_ENTRY1(___name, state_entry, __len)

#define DECL_STATE_ENTRY_MULTI(_n_type, ___name, ___size) DECL_STATE_ENTRY_MULTI0(_n_type, ___name, state_entry, ___size)

#define DECL_STATE_ENTRY_2D_ARRAY(___name, __len1, __len2) {		\
		int __tmplen = ((int)__len1 * (int)__len2);					\
		state_entry->add_entry(_T(#___name), &(___name[0][0]), __tmplen); \
	}

#define DECL_STATE_ENTRY_3D_ARRAY(___name, __len1, __len2, __len3) {	\
		int __tmplen = ((int)__len1 * (int)__len2 * (int)__len3);		\
		state_entry->add_entry(_T(#___name), &(___name[0][0][0]), __tmplen); \
	}



#define DECL_STATE_ENTRY_VARARRAY_VAR(_n_name, __sizevar) {				\
		state_entry->add_entry_vararray(_T(#_n_name), _n_name, (void *)(&__sizevar)); \
	}

#endif /* _CSP_STATE_SUB_H */
