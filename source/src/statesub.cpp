
#include "common.h"
#include "fileio.h"

//#include "state_data.h"
#include "statesub.h"

csp_state_utils::csp_state_utils(int _version, int device_id, _TCHAR *classname)
{
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

csp_state_utils::~csp_state_utils()
{
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
				_vname = _vname + ((csp_state_data<float> *)pp)->get_name();
				break;
			case csp_saver_entry_double:
				_tname = _T("TYPE: double");
				_vname = _vname + ((csp_state_data<double> *)pp)->get_name();
				break;
			case csp_saver_entry_long_double:
				_tname = _T("TYPE: long double");
				_vname = _vname + ((csp_state_data<long double> *)pp)->get_name();
				break;
			case csp_saver_entry_pair:
				_tname = _T("TYPE: pair_t");
				_vname = _vname + ((csp_state_data<pair_t> *)pp)->get_name();
				break;
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
uint32_t csp_state_utils::get_crc_value(void)
{
	return crc_value;
}
void csp_state_utils::get_class_name(_TCHAR *buf, int len)
{
	strncpy(buf, __classname_bak, ((size_t)len > sizeof(__classname_bak)) ? sizeof(__classname_bak) : len);
}
//template <typename T>
bool csp_state_utils::save_state(FILEIO *fio, uint32_t *pcrc)
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
			case csp_saver_entry_float:
				retval = ((csp_state_data<float> *)pp)->save_data(fio, &crc_value);
				break;
			case csp_saver_entry_double:
				retval = ((csp_state_data<double> *)pp)->save_data(fio, &crc_value);
				break;
			case csp_saver_entry_long_double:
				retval = ((csp_state_data<long double> *)pp)->save_data(fio, &crc_value);
				break;
			case csp_saver_entry_pair:
				retval = ((csp_state_data<pair_t> *)pp)->save_data(fio, &crc_value);
				break;
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
bool csp_state_utils::load_state(FILEIO *fio, uint32_t *pcrc)
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
			case csp_saver_entry_float:
				retval = ((csp_state_data<float> *)pp)->load_data(fio, &crc_value);
				break;
			case csp_saver_entry_double:
				retval = ((csp_state_data<double> *)pp)->load_data(fio, &crc_value);
				break;
			case csp_saver_entry_long_double:
				retval = ((csp_state_data<long double> *)pp)->load_data(fio, &crc_value);
				break;
			case csp_saver_entry_pair:
				retval = ((csp_state_data<pair_t> *)pp)->load_data(fio, &crc_value);
				break;
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
