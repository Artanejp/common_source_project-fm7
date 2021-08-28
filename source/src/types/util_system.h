/*!
  @todo will move to another directory.
*/

#pragma once

#define array_length(array) (sizeof(array) / sizeof(array[0]))

#define FROM_BCD(v)	(((v) & 0x0f) + (((v) >> 4) & 0x0f) * 10)
#define TO_BCD(v)	((int)(((v) % 100) / 10) << 4) | ((v) % 10)
#define TO_BCD_LO(v)	((v) % 10)
#define TO_BCD_HI(v)	(int)(((v) % 100) / 10)

// file path
const _TCHAR *DLL_PREFIX get_application_path();
const _TCHAR *DLL_PREFIX get_initial_current_path();
const _TCHAR *DLL_PREFIX create_local_path(const _TCHAR *format, ...);
void DLL_PREFIX create_local_path(_TCHAR *file_path, int length, const _TCHAR *format, ...);
const _TCHAR *DLL_PREFIX create_absolute_path(const _TCHAR *format, ...);
void DLL_PREFIX create_absolute_path(_TCHAR *file_path, int length, const _TCHAR *format, ...);
bool DLL_PREFIX is_absolute_path(const _TCHAR *file_path);
const _TCHAR *DLL_PREFIX create_date_file_path(const _TCHAR *extension);
void DLL_PREFIX create_date_file_path(_TCHAR *file_path, int length, const _TCHAR *extension);
const _TCHAR *DLL_PREFIX create_date_file_name(const _TCHAR *extension);
void DLL_PREFIX create_date_file_name(_TCHAR *file_path, int length, const _TCHAR *extension);
bool DLL_PREFIX check_file_extension(const _TCHAR *file_path, const _TCHAR *ext);
const _TCHAR *DLL_PREFIX get_file_path_without_extensiton(const _TCHAR *file_path);
void DLL_PREFIX get_long_full_path_name(const _TCHAR* src, _TCHAR* dst, size_t dst_len);
const _TCHAR* DLL_PREFIX get_parent_dir(const _TCHAR* file);


// misc
void DLL_PREFIX common_initialize();

int32_t DLL_PREFIX  __FASTCALL muldiv_s32(int32_t nNumber, int32_t nNumerator, int32_t nDenominator);
uint32_t DLL_PREFIX __FASTCALL muldiv_u32(uint32_t nNumber, uint32_t nNumerator, uint32_t nDenominator);

uint32_t DLL_PREFIX  get_crc32(uint8_t data[], int size);
uint32_t DLL_PREFIX  calc_crc32(uint32_t seed, uint8_t data[], int size);

inline uint16_t __FASTCALL jis_to_sjis(uint16_t jis)
{
	pair32_t tmp;
	
	tmp.w.l = jis - 0x2121;
	if(tmp.w.l & 0x100) {
		tmp.w.l += 0x9e;
	} else {
		tmp.w.l += 0x40;
	}
	if(tmp.b.l > 0x7f) {
		tmp.w.l += 0x01;
	}
	tmp.b.h = (tmp.b.h >> 1) + 0x81;
	if(tmp.w.l >= 0xa000) {
		tmp.w.l += 0x4000;
	}
	return tmp.w.l;
}

const _TCHAR DLL_PREFIX *get_lib_common_version();

