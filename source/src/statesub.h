/*
 * State assistance routines.
 * (C) May 16 2018 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2 (or later)
 * Usage:
 * 1. Decl. state entries per device.
 *  #define STATE_VERSION n;
 *    void foo::decl_state(void)
 *    {
 *      state_entry = new csp_state_utils(STATE_VERSION, this_device_id, _T("NAME")); // Must be.
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
#include "fifo.h"

#include <string>
#include <list>
#include <typeindex>
#include <map>
#include "state_data.h"
#if defined(_USE_QT)
#include <QObject>
#include <QString>
#endif
typedef enum csp_saver_type_t {
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
	csp_saver_entry_void,
	csp_saver_entry_string,
	csp_saver_entry_cmt_recording,
	csp_saver_entry_any,
	
	csp_saver_entry_fifo,
	csp_saver_entry_cur_time_t,
	csp_saver_entry_device,
	
	csp_saver_entry_custom0 = 256,
	csp_saver_entry_custom1 = 257,
	csp_saver_entry_custom2 = 258,
	csp_saver_entry_custom3 = 259,
	
	// Below are special value
	csp_saver_entry_vararray = 0x10000,
	csp_saver_entry_const = 0x20000,
};


class CSP_Logger;

#if defined(_USE_QT)
  QT_BEGIN_NAMESPACE
class DLL_PREFIX csp_state_utils : public QObject {
#else
class DLL_PREFIX csp_state_utils {
#endif
#if defined(_USE_QT)
	Q_OBJECT
#endif
protected:
	typedef union  {
		int64_t s;
		uint64_t u;
		void *p;
	} union64_t;
	CSP_Logger *logger;
	const uint32_t CRC_MAGIC_WORD = 0x04C11DB7;
	struct __list_t {
		int type_id;
		int local_num;
		bool assume_byte;
		int len;
		int atomlen;
		std::string name;
		void *ptr;
		int *datalenptr;
		
		void *recv_ptr;
		_TCHAR *path_ptr;
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

	std::map <std::type_index, int> typeid_map= {
		{ typeid(int),  csp_saver_entry_int },
		{ typeid(pair_t), csp_saver_entry_pair },
		{ typeid(float), csp_saver_entry_float },
		{ typeid(double), csp_saver_entry_double },
		{ typeid(long double), csp_saver_entry_long_double },
		{ typeid(int8_t), csp_saver_entry_int8 },
		{ typeid(uint8_t), csp_saver_entry_uint8 },
		{ typeid(int16_t), csp_saver_entry_int16 },
		{ typeid(uint16_t), csp_saver_entry_uint16 },
		{ typeid(int32_t), csp_saver_entry_int32 },
		{ typeid(uint32_t), csp_saver_entry_uint32 },
		{ typeid(int64_t), csp_saver_entry_int64 },
		{ typeid(uint64_t), csp_saver_entry_uint64 },
		{ typeid(bool), csp_saver_entry_bool },
		{ typeid(void), csp_saver_entry_void },
		{ typeid(FIFO), csp_saver_entry_fifo },
		{ typeid(cur_time_t), csp_saver_entry_cur_time_t }
	};
public:
	csp_state_utils(int _version = 1, int device_id = 1, const _TCHAR *classname = NULL, CSP_Logger* p_logger = NULL);
	~csp_state_utils();
	std::list<std::string> get_entries_list(void);

	template <class T>
	void add_entry(const _TCHAR *__name, T *p, int _len = 1, int __num = -1, bool is_const = false)
	{
		__list_t _l;
		std::string _name = std::string(__name);
		if(__num >= 0) _name = _name + std::string("_#[") +std::to_string(__num) + std::string("]");
		_l.ptr = (void *)p;
		_l.type_id = typeid_map[typeid(T)];
		_l.len = _len;
		_l.atomlen = sizeof(T);
		_l.name = _name;
		_l.datalenptr = NULL;
		_l.local_num = __num;
		_l.assume_byte = false;
		_l.recv_ptr = 0;
		out_debug_log("ADD ENTRY: NAME=%s TYPE=%s len=%d atomlen=%d", _name.c_str(), typeid(T).name(), _len, _l.atomlen);
		if(is_const) _l.type_id = _l.type_id | csp_saver_entry_const;
		listptr.push_back(_l);
	};

	template <class T>
	void add_entry_vararray(const _TCHAR *__name, T **p, void *datalen, bool assume_byte = false, int __num = -1)
	{
		__list_t _l;
		
		if(datalen == NULL) {
			add_entry(__name, p, 1);
			return;
		}
		std::string _name = std::string(__name);
		if(__num >= 0) _name = _name + std::string("_#[") +std::to_string(__num) + std::string("]");
		
		_l.ptr = (void *)p;
		_l.type_id = typeid_map[typeid(T)];;
		_l.len = 0;
		_l.atomlen = sizeof(T);
		_l.name = _name;
		_l.local_num = __num;
		_l.datalenptr = (int *) datalen;
		_l.assume_byte = assume_byte;
		_l.type_id = _l.type_id | csp_saver_entry_vararray;
		_l.recv_ptr = 0;
		out_debug_log("ADD ENTRY(VARARRAY): NAME=%s TYPE=%s atomlen=%d linked len=%08x", __name, typeid(T).name(), _l.atomlen, datalen);
		listptr.push_back(_l);
	};

	void add_entry_fifo(const _TCHAR *__name, FIFO **p, int _len = 1, int __num = -1);
	void add_entry_cur_time_t(const _TCHAR *__name, cur_time_t *p, int _len = 1, int __num = -1);
	void add_entry_string(const _TCHAR *__name, _TCHAR *p, int _len = 1, int __num = -1, bool is_const = false);
	void add_entry_cmt_recording(const _TCHAR *__name, FILEIO **__fio, bool* __flag, _TCHAR *__path); 
	
	uint32_t get_crc_value(void);
	void get_class_name(_TCHAR *buf, int len);
	bool save_state(FILEIO *__fio, uint32_t *pcrc = NULL);
	bool load_state(FILEIO *__fio, uint32_t *pcrc = NULL);
#if defined(_USE_QT)
signals:
	int sig_debug_log(int, int, QString);
#endif
};
#if defined(_USE_QT)
    QT_END_NAMESPACE
#endif
			
#define DECL_STATE_ENTRY0(_n_name, __list) {				  \
		__list->add_entry((const _TCHAR *)_T(#_n_name), &_n_name);		  \
	}

#define DECL_STATE_ENTRY1(_n_name, __list, __len) {				\
		__list->add_entry((const _TCHAR *)_T(#_n_name), _n_name, __len);		\
	}

#define DECL_STATE_ENTRY2(_n_name, __list, __len, __n) {					\
		__list->add_entry((const _TCHAR *)_T(#_n_name), &_n_name, __len, __n); \
	}

#define DECL_STATE_ENTRY_MULTI0(__type, _n_name, __list, __size) {		\
		__list->add_entry((const _TCHAR *)_T(#_n_name), (uint8_t *)_n_name, __size * sizeof(__type)); \
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

#define DECL_STATE_ENTRY_UINT8_MEMBER(___name, __num) DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_INT8_MEMBER(___name, __num)  DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_UINT16_MEMBER(___name, __num) DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_INT16_MEMBER(___name, __num)  DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_UINT32_MEMBER(___name, __num) DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_INT32_MEMBER(___name, __num)  DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_UINT64_MEMBER(___name, __num) DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_INT64_MEMBER(___name, __num)  DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_BOOL_MEMBER(___name, __num) DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_PAIR_MEMBER(___name, __num)  DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_UINT8_MEMBER(___name, __num) DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_FLOAT_MEMBER(___name, __num)  DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_DOUBLE_MEMBER(___name, __num) DECL_STATE_ENTRY2(___name, state_entry, 1, __num)
#define DECL_STATE_ENTRY_LONG_DOUBLE_MEMBER(___name, __num)  DECL_STATE_ENTRY2(___name, state_entry, 1, __num)



#define DECL_STATE_ENTRY_MULTI(_n_type, ___name, ___size) DECL_STATE_ENTRY_MULTI0(_n_type, ___name, state_entry, ___size)

#define DECL_STATE_ENTRY_STRING(___name, __len) { \
			state_entry->add_entry_string((_TCHAR *)(_T(#___name)), ___name, __len); \
	}

#define DECL_STATE_ENTRY_STRING_MEMBER(___name, __len, __num) {				\
			state_entry->add_entry_string((_TCHAR *)(_T(#___name)), ___name, __len, __num); \
	}

#define DECL_STATE_ENTRY_1D_ARRAY(___name, ___lenvar) { \
		state_entry->add_entry((const _TCHAR *)(_T(#___name)), ___name, ___lenvar); \
	}

#define DECL_STATE_ENTRY_2D_ARRAY(___name, __len1, __len2) {		\
		int __tmplen = ((int)__len1 * (int)__len2);					\
		state_entry->add_entry((const _TCHAR *)(_T(#___name)), &(___name[0][0]), __tmplen); \
	}

#define DECL_STATE_ENTRY_3D_ARRAY(___name, __len1, __len2, __len3) {	\
		int __tmplen = ((int)__len1 * (int)__len2 * (int)__len3);		\
		state_entry->add_entry((const _TCHAR *)_T(#___name), &(___name[0][0][0]), __tmplen); \
	}

#define DECL_STATE_ENTRY_4D_ARRAY(___name, __len1, __len2, __len3, __len4) {	\
		int __tmplen = ((int)__len1 * (int)__len2 * (int)__len3 * (int)__len4); \
		state_entry->add_entry((const _TCHAR *)_T(#___name), &(___name[0][0][0][0]), __tmplen); \
	}

#define DECL_STATE_ENTRY_1D_ARRAY_MEMBER(___name, ___lenvar, __num) {	\
		state_entry->add_entry((const _TCHAR *)(_T(#___name)), ___name, ___lenvar, __num); \
	}

#define DECL_STATE_ENTRY_2D_ARRAY_MEMBER(___name, __len1, __len2, __num) {	\
		int __tmplen = ((int)__len1 * (int)__len2);					\
		state_entry->add_entry((const _TCHAR *)(_T(#___name)), &(___name[0][0]), __tmplen, __num); \
	}

#define DECL_STATE_ENTRY_3D_ARRAY_MEMBER(___name, __len1, __len2, __len3, __num) { \
		int __tmplen = ((int)__len1 * (int)__len2 * (int)__len3);		\
		state_entry->add_entry((const _TCHAR *)_T(#___name), &(___name[0][0][0]), __tmplen, __num); \
	}

#define DECL_STATE_ENTRY_VARARRAY_VAR(_n_name, __sizevar) {				\
		state_entry->add_entry_vararray((const _TCHAR *)_T(#_n_name), &_n_name, (void *)(&__sizevar)); \
	}

#define DECL_STATE_ENTRY_VARARRAY_MEMBER(_n_name, __sizevar, __n) {			\
		state_entry->add_entry_vararray((const _TCHAR *)_T(#_n_name), &_n_name, (void *)(&__sizevar), false, __n); \
	}

#define DECL_STATE_ENTRY_VARARRAY_BYTES(_n_name, __sizevar) {				\
		state_entry->add_entry_vararray((const _TCHAR *)_T(#_n_name), &_n_name, (void *)(&__sizevar), true); \
	}

#define DECL_STATE_ENTRY_VARARRAY_BYTES_MEMBER(_n_name, __sizevar, __n) {	\
		state_entry->add_entry_vararray((const _TCHAR *)_T(#_n_name), &_n_name, (void *)(&__sizevar), true, __n); \
		}

#define DECL_STATE_ENTRY_CMT_RECORDING(__fio, __flag, __path) {				\
			state_entry->add_entry_cmt_recording((const _TCHAR *)_T(#__fio), &__fio, &__flag, (_TCHAR *)(&(__path[0]))); \
		}
		
#define DECL_STATE_ENTRY_SINGLE(___name) { \
		state_entry->add_entry((const _TCHAR *)_T(#___name) , &___name); \
	}

#define DECL_STATE_ENTRY_SINGLE_MEMBER(___name, __num) {						\
		state_entry->add_entry((const _TCHAR *)_T(#___name) , &___name, 1, __num); \
	}

#define DECL_STATE_ENTRY_SINGLE_ARRAY(___name, __len) {						\
		state_entry->add_entry((const _TCHAR *)_T(#___name) , &___name, __len); \
	}
#define DECL_STATE_ENTRY_SINGLE_ARRAY_MEMBER(___name, __len, __num) {			\
		state_entry->add_entry((const _TCHAR *)_T(#___name) , &___name, __len, __num); \
	}

#define DECL_STATE_ENTRY_FIFO(_n_name) {								\
		state_entry->add_entry_fifo((const _TCHAR *)_T(#_n_name), &_n_name, 1); \
	}

#define DECL_STATE_ENTRY_FIFO_ARRAY(_n_name, __len) {					\
		state_entry->add_entry_fifo((const _TCHAR *)_T(#_n_name), &_n_name, __len); \
	}

#define DECL_STATE_ENTRY_FIFO_MEMBER(_n_name, __n) {						\
		state_entry->add_entry_fifo((const _TCHAR *)_T(#_n_name), &_n_name, 1, __n); \
	}

#define DECL_STATE_ENTRY_FIFO_ARRAY_MEMBER(_n_name, __len, __n) {			\
		state_entry->add_entry_fifo((const _TCHAR *)_T(#_n_name), &_n_name, __len, __n); \
	}
   
#define DECL_STATE_ENTRY_CUR_TIME_T(_n_name) {							\
		state_entry->add_entry_cur_time_t((const _TCHAR *)_T(#_n_name), &_n_name, 1); \
	}

#define DECL_STATE_ENTRY_CUR_TIME_T_ARRAY(_n_name, __len) {					\
		state_entry->add_entry_cur_time_t((const _TCHAR *)_T(#_n_name), &_n_name, __len); \
	}

#define DECL_STATE_ENTRY_CUR_TIME_T_MEMBER(_n_name, __n) {				\
		state_entry->add_entry_cur_time_t((const _TCHAR *)_T(#_n_name), &_n_name, 1, __n); \
	}

#define DECL_STATE_ENTRY_CUR_TIME_T_ARRAY_MEMBER(_n_name, __len, __n) {	\
		state_entry->add_entry_cur_time_t((const _TCHAR *)_T(#_n_name), &_n_name, __len, __n); \
	}

#endif /* _CSP_STATE_SUB_H */
