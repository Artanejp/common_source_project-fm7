/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2013.01.17-

	[ common ]
*/

#if defined(_USE_QT)
	#include <string.h>
	#include <fcntl.h>
	#if !defined(__WIN32) && !defined(__WIN64)
		#include <unistd.h>
	#else
		#include <io.h>
		#include <direct.h>
	#endif
	#include <sys/types.h>
	#include <sys/stat.h>
	#include "csp_logger.h"
	#include <string>
	#include <algorithm>
	#include <cctype>
	#include <QDir>
	#include <QFileInfo>
#elif defined(_WIN32)
	#include <shlwapi.h>
	#pragma comment(lib, "shlwapi.lib")
#else
	#include <time.h>
#endif

#if defined(_WIN32)
	#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <atomic>
#include <mutex>

#include "common.h"
#include "fileio.h"

#if defined(__MINGW32__) || defined(__MINGW64__)
	extern DWORD GetLongPathName(LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD cchBuffer);
#endif
#if defined(_USE_QT)
	std::string DLL_PREFIX cpp_homedir;
	std::string DLL_PREFIX cpp_confdir;
	std::string DLL_PREFIX my_procname;
	std::string DLL_PREFIX sRssDir;
#endif

// for disassedmbler
uint32_t DLL_PREFIX get_relative_address_8bit(uint32_t base, uint32_t mask, int8_t offset)
{
	uint32_t addr = base & mask;
	int32_t off2 = (int32_t)offset;
	return (uint32_t)(addr + off2) & mask;
}

uint32_t DLL_PREFIX get_relative_address_16bit(uint32_t base, uint32_t mask, int16_t offset)
{
	uint32_t addr = base & mask;
	int32_t off2 = (int32_t)offset;
	return (uint32_t)(addr + off2) & mask;
}


uint32_t DLL_PREFIX get_relative_address_32bit(uint32_t base, uint32_t mask, int32_t offset)
{
	uint32_t addr = base & mask;
	return (uint32_t)(addr + offset) & mask;
}

void DLL_PREFIX common_initialize()
{
	// get the initial current path when the software starts
	get_initial_current_path();
}


#ifndef SUPPORT_SECURE_FUNCTIONS
//errno_t my_tfopen_s(FILE** pFile, const _TCHAR *filename, const _TCHAR *mode)
//{
//	if((*pFile = _tfopen(filename, mode)) != NULL) {
//		return 0;
//	} else {
//		return errno;
//	}
//}

errno_t DLL_PREFIX my_tcscat_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource)
{
	__UNLIKELY_IF(strDestination == nullptr) return EINVAL;
	__UNLIKELY_IF(numberOfElements < 1) {
		return EINVAL;
	}
	__UNLIKELY_IF(numberOfElements == 1) {
		strDestination[0] = '\0';
		return ERANGE;
	}
	__UNLIKELY_IF(strSource == nullptr) {
//		strDestination[0] = '\0'; // What?
		return EINVAL;
	}

	std::string _src(strSource);
	std::string _dst(strDestination);
	_dst.append(_src);
	// Sanitize string
	memset(strDestination, 0x00, numberOfElements);
	_dst.copy(strDestination, numberOfElements - 1);
	return 0;
}

errno_t DLL_PREFIX my_strcpy_s(char *strDestination, size_t numberOfElements, const char *strSource)
{
	__UNLIKELY_IF(strDestination == nullptr) return EINVAL;
	__UNLIKELY_IF(numberOfElements < 1) {
		return EINVAL;
	}
	__UNLIKELY_IF(numberOfElements == 1) {
		strDestination[0] = '\0';
		return ERANGE;
	}
	__UNLIKELY_IF(strSource == nullptr) {
		strDestination[0] = '\0'; // What?
		return EINVAL;
	}
	// Sanitize string
	memset(strDestination, 0x00, numberOfElements);

	size_t count = strlen(strSource);
	__UNLIKELY_IF(count == 0) {
		return ERANGE;
	}
	if(count >= numberOfElements) {
		count = numberOfElements - 1;
	}
	strncpy(strDestination, strSource, count);
	return 0;
}

errno_t DLL_PREFIX my_tcscpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource)
{
	__UNLIKELY_IF(strDestination == nullptr) return EINVAL;
	__UNLIKELY_IF(numberOfElements < 1) {
		return EINVAL;
	}
	__UNLIKELY_IF(numberOfElements == 1) {
		strDestination[0] = '\0';
		return ERANGE;
	}
	__UNLIKELY_IF(strSource == nullptr) {
		strDestination[0] = '\0'; // What?
		return EINVAL;
	}
	// Sanitize string
	memset(strDestination, 0x00, numberOfElements);
	size_t count = strlen(strSource);
	__UNLIKELY_IF(count == 0) {
		return ERANGE;
	}
	__UNLIKELY_IF(count >= numberOfElements) {
		count = numberOfElements - 1;
	}
	_tcsncpy(strDestination, strSource,  count);
	return 0;
}

errno_t DLL_PREFIX my_strncpy_s(char *strDestination, size_t numberOfElements, const char *strSource, size_t count)
{
	bool truncated = false;
	__UNLIKELY_IF(strDestination == nullptr) return EINVAL;
	__UNLIKELY_IF(numberOfElements < 1) {
		return EINVAL;
	}
	__UNLIKELY_IF(numberOfElements == 1) {
		strDestination[0] = '\0';
		return ERANGE;
	}
	__UNLIKELY_IF(strSource == nullptr) {
		strDestination[0] = '\0'; // What?
		return EINVAL;
	}

	// Sanitize string
	memset(strDestination, 0x00, numberOfElements);
	__UNLIKELY_IF((numberOfElements <= count) || (count == _TRUNCATE)) {
		count = numberOfElements - 1;
		truncated = true;
	}
	__UNLIKELY_IF(count == 0) {
		return ERANGE;
	}
	strncpy(strDestination, strSource, count);
	return (truncated) ? STRUNCATE : 0;
}

errno_t DLL_PREFIX my_tcsncpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource, size_t count)
{
	bool truncated = false;
	__UNLIKELY_IF(strDestination == nullptr) return EINVAL;
	__UNLIKELY_IF(numberOfElements < 1) {
		return EINVAL;
	}
	__UNLIKELY_IF(numberOfElements == 1) {
		strDestination[0] = '\0';
		return ERANGE;
	}
	__UNLIKELY_IF(strSource == nullptr) {
		strDestination[0] = '\0'; // What?
		return EINVAL;
	}

	// Sanitize string
	memset(strDestination, 0x00, numberOfElements);
	__UNLIKELY_IF((numberOfElements <= count) || (count == _TRUNCATE)) {
		count = numberOfElements - 1;
		truncated = true;
	}
	__UNLIKELY_IF(count == 0) {
		return ERANGE;
	}
	_tcsncpy(strDestination, strSource, count);

	return (truncated) ? STRUNCATE : 0;
}

char *DLL_PREFIX my_strtok_s(char *strToken, const char *strDelimit, char **context)
{
	return strtok(strToken, strDelimit);
}

_TCHAR *DLL_PREFIX my_tcstok_s(_TCHAR *strToken, const char *strDelimit, _TCHAR **context)
{
	return _tcstok(strToken, strDelimit);
}

int DLL_PREFIX my_sprintf_s(char *buffer, size_t sizeOfBuffer, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = vsnprintf(buffer, sizeOfBuffer, format, ap);
	va_end(ap);
	return result;
}

int DLL_PREFIX my_swprintf_s(wchar_t *buffer, size_t sizeOfBuffer, const wchar_t *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = vswprintf(buffer, sizeOfBuffer, format, ap);
	va_end(ap);
	return result;
}

int DLL_PREFIX my_stprintf_s(_TCHAR *buffer, size_t sizeOfBuffer, const _TCHAR *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = vsnprintf(buffer, sizeOfBuffer, format, ap);
	va_end(ap);
	return result;
}

int DLL_PREFIX my_vsprintf_s(char *buffer, size_t numberOfElements, const char *format, va_list argptr)
{
	return vsnprintf(buffer, numberOfElements * sizeof(char), format, argptr);
}

int DLL_PREFIX my_vstprintf_s(_TCHAR *buffer, size_t numberOfElements, const _TCHAR *format, va_list argptr)
{
	return vsnprintf(buffer, numberOfElements * sizeof(_TCHAR), format, argptr);
}
#endif


#include "./types/util_configwrapper.h"

#ifndef _WIN32
BOOL DLL_PREFIX MyWritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
	BOOL result = FALSE;
	FILEIO* fio_i = new FILEIO();
	if(fio_i->Fopen(lpFileName, FILEIO_READ_ASCII)) {
		char tmp_path[_MAX_PATH];
		my_sprintf_s(tmp_path, _MAX_PATH, "%s.$$$", lpFileName);
		FILEIO* fio_o = new FILEIO();
		if(fio_o->Fopen(tmp_path, FILEIO_WRITE_ASCII)) {
			bool in_section = false;
			char section[1024], line[1024], *equal;
			my_sprintf_s(section, 1024, "[%s]", lpAppName);
			while(fio_i->Fgets(line, 1024) != NULL && strlen(line) > 0) {
				if(line[strlen(line) - 1] == '\n') {
					line[strlen(line) - 1] = '\0';
				}
				if(!result) {
					if(line[0] == '[') {
						if(in_section) {
							fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
							result = TRUE;
						} else if(strcmp(line, section) == 0) {
							in_section = true;
						}
					} else if(in_section && (equal = strstr(line, "=")) != NULL) {
						*equal = '\0';
						if(strcmp(line, lpKeyName) == 0) {
							fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
							result = TRUE;
							continue;
						}
						*equal = '=';
					}
				}
				fio_o->Fprintf("%s\n", line);
			}
			if(!result) {
				if(!in_section) {
					fio_o->Fprintf("[%s]\n", lpAppName);
				}
				fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
				result = TRUE;
			}
			fio_o->Fclose();
		}
		delete fio_o;
		fio_i->Fclose();
		if(result) {
			if(!(FILEIO::RemoveFile(lpFileName) && FILEIO::RenameFile(tmp_path, lpFileName))) {
				result = FALSE;
			}
		}
	} else {
		FILEIO* fio_o = new FILEIO();
		if(fio_o->Fopen(lpFileName, FILEIO_WRITE_ASCII)) {
			fio_o->Fprintf("[%s]\n", lpAppName);
			fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
			fio_o->Fclose();
		}
		delete fio_o;
	}
	delete fio_i;
	return result;
}


DWORD DLL_PREFIX MyGetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	_TCHAR *lpp = (_TCHAR *)lpReturnedString;
	if(lpDefault != NULL) {
		my_strcpy_s(lpp, nSize, lpDefault);
	} else {
		lpp[0] = '\0';
	}
	FILEIO* fio = new FILEIO();
	if(!(fio->IsFileExisting(lpFileName))) return 0;
	if(fio->Fopen(lpFileName, FILEIO_READ_ASCII)) {
		bool in_section = false;
		char section[1024], line[1024], *equal;
		my_sprintf_s(section, 1024, "[%s]", lpAppName);
		while(fio->Fgets(line, 1024) != NULL && strlen(line) > 0) {
			if(line[strlen(line) - 1] == '\n') {
				line[strlen(line) - 1] = '\0';
			}
			if(line[0] == '[') {
				if(in_section) {
					break;
				} else if(strcmp(line, section) == 0) {
					in_section = true;
				}
			} else if(in_section && (equal = strstr(line, "=")) != NULL) {
				*equal = '\0';
				if(strcmp(line, lpKeyName) == 0) {
					my_strcpy_s(lpp, nSize, equal + 1);
					break;
				}
			}
		}
		fio->Fclose();
	}
	delete fio;
	//csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Try App: %s Key: %s", lpAppName, lpKeyName);
	return strlen(lpp);
}

UINT DLL_PREFIX MyGetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName)
{
	int i;
	char sstr[128];
	char sval[128];
	std::string s;
	memset(sstr, 0x00, sizeof(sstr));
	memset(sval, 0x00, sizeof(sval));
	snprintf(sval, 128, "%d", nDefault);
	MyGetPrivateProfileString(lpAppName,lpKeyName, sval, sstr, 128, lpFileName);
	s = sstr;

	if(s.empty()) {
		i = nDefault;
	} else {
		i = strtol(s.c_str(), NULL, 10);
	}
	//csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Got Int: %d\n", i);
	return i;
}
#endif



#ifndef _MSC_VER
struct to_upper {  // Refer from documentation of libstdc++, GCC5.
	char operator() (char c) const { return std::toupper(c); }
};
#endif

#if defined(_USE_QT)
static void _my_mkdir(std::string t_dir)
{
	struct stat st;
//#if !defined(__WIN32) && !defined(__WIN64)
//	if(fstatat(AT_FDCWD, csppath.c_str(), &st, 0) != 0) {
//		mkdirat(AT_FDCWD, t_dir.c_str(), 0700); // Not found
//	}
#if defined(_USE_QT)
	if(stat(t_dir.c_str(), &st) != 0) {
		QDir dir = QDir::current();
		dir.mkdir(QString::fromStdString(t_dir));
		//dir.mkpath(QString::fromUtf8(app_path));
	}
#else
	if(stat(csppath.c_str(), &st) != 0) {
		_mkdir(t_dir.c_str()); // Not found
	}
#endif
}
#endif

static std::mutex string_lock;

const _TCHAR *DLL_PREFIX get_application_path()
{
	std::lock_guard<std::mutex> locker(string_lock);
	static _TCHAR app_path[_MAX_PATH] = {0};
	static bool initialized = false;

	if(!initialized) {
#if defined(_WIN32) && !defined(_USE_QT)
		_TCHAR tmp_path[_MAX_PATH], *ptr = NULL;
		if(GetModuleFileName(NULL, tmp_path, _MAX_PATH) != 0 && GetFullPathName(tmp_path, _MAX_PATH, app_path, &ptr) != 0 && ptr != NULL) {
			*ptr = _T('\0');
		} else {
			my_tcscpy_s(app_path, _MAX_PATH, _T(".\\"));
		}
#else
#if defined(Q_OS_WIN)
		std::string delim = "\\";
#else
		std::string delim = "/";
#endif
		std::string csppath = cpp_homedir + "CommonSourceCodeProject" + delim ;
		_my_mkdir(csppath);

		std::string cpath = csppath + my_procname + delim;
		_my_mkdir(cpath);
		strncpy(app_path, cpath.c_str(), _MAX_PATH - 1);
#endif
		initialized = true;
	}
	return (const _TCHAR *)app_path;
}

const _TCHAR *DLL_PREFIX get_initial_current_path()
{
	std::lock_guard<std::mutex> locker(string_lock);
	static _TCHAR current_path[_MAX_PATH] = {0};
	static bool initialized = false;

	if(!initialized) {
#if defined(_WIN32) && !defined(_USE_QT)
		GetCurrentDirectory(_MAX_PATH, current_path);
#else
		getcwd(current_path, _MAX_PATH);
#endif
		size_t len = _tcslen(current_path);
		if(current_path[len - 1] != '\\' && current_path[len - 1] != '/') {
#if defined(_WIN32) || defined(Q_OS_WIN)
			current_path[len] = '\\';
#else
			current_path[len] = '/';
#endif
			current_path[len + 1] = '\0';
		}

		initialized = true;
	}
	return (const _TCHAR *)current_path;
}

// Make re-entrant
// 20250923 K.O 
static void __create_local_path(_TCHAR *file_path, int length, const _TCHAR *format, va_list ap)
{
	__LIKELY_IF((file_path != NULL) && (format != NULL) && (length > 0)) {
		_TCHAR file_name[_MAX_PATH];
		file_name[0] = '\0';
		my_vstprintf_s(file_name, _MAX_PATH, format, ap);
		my_stprintf_s(file_path, min(length, (int)_MAX_PATH), _T("%s%s"), get_application_path(), file_name);
		return;
	}
	if((file_path != NULL) && (length > 0)) {
		// No Format
		file_path[0] = '\0';
	}
}

const _TCHAR *DLL_PREFIX create_local_path(const _TCHAR *format, ...)
{
	static _TCHAR file_path[16][_MAX_PATH] = {0};
	static std::atomic<unsigned int> table_index = 0;
	unsigned int output_index = (table_index++) & 15;
	_TCHAR file_name[_MAX_PATH];
	//printf("%d %d\n", table_index, output_index);
	va_list ap;

	va_start(ap, format);
	__create_local_path(&(file_path[output_index][0]), _MAX_PATH, format, ap);
	va_end(ap);
	return (const _TCHAR *)file_path[output_index];
}

void DLL_PREFIX create_local_path(_TCHAR *file_path, int length, const _TCHAR *format, ...)
{
	va_list ap;

	va_start(ap, format);
	__create_local_path(file_path, _MAX_PATH, format, ap);
	va_end(ap);
}

static void __create_absolute_path(_TCHAR *file_path, int length, const _TCHAR *format, va_list ap)
{
	__LIKELY_IF((file_path != NULL) && (format != NULL) && (length > 0)) {
		_TCHAR file_name[_MAX_PATH];
		file_name[0] = '\0';
		my_vstprintf_s(file_name, _MAX_PATH, format, ap);
		if(is_absolute_path(file_name)) {
			my_tcscpy_s(file_path, min(length, (int)_MAX_PATH), file_name);
		} else {
			my_stprintf_s(file_path, min(length, (int)_MAX_PATH), _T("%s%s"), get_initial_current_path(), file_name);
		}
		return;
	}
	if((file_path != NULL) && (length > 0)) {
		// No Format
		file_path[0] = '\0';
	}
}
		
const _TCHAR *DLL_PREFIX create_absolute_path(const _TCHAR *format, ...)
{
	static _TCHAR file_path[16][_MAX_PATH] = {0};
	static std::atomic<unsigned int> table_index = 0;
	unsigned int output_index = (table_index++) & 15;
	_TCHAR file_name[_MAX_PATH];
	va_list ap;

	va_start(ap, format);
	__create_absolute_path(&(file_path[output_index][0]), _MAX_PATH, format, ap);
	va_end(ap);

	return (const _TCHAR *)file_path[output_index];
}

void DLL_PREFIX create_absolute_path(_TCHAR *file_path, int length, const _TCHAR *format, ...)
{
	va_list ap;

	va_start(ap, format);
	__create_absolute_path(file_path, length, format, ap);
	va_end(ap);
}

bool DLL_PREFIX is_absolute_path(const _TCHAR *file_path)
{
#ifdef _WIN32
	if(_tcslen(file_path) > 2 && ((file_path[0] >= _T('A') && file_path[0] <= _T('Z')) || (file_path[0] >= _T('a') && file_path[0] <= _T('z'))) && file_path[1] == _T(':')) {
		return true;
	}
#endif
	return (_tcslen(file_path) > 1 && (file_path[0] == _T('/') || file_path[0] == _T('\\')));
}

const _TCHAR *DLL_PREFIX create_date_file_path(const _TCHAR *extension)
{
	cur_time_t cur_time;

	get_host_time(&cur_time);
	return create_local_path(_T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.%s"), cur_time.year, cur_time.month, cur_time.day, cur_time.hour, cur_time.minute, cur_time.second, extension);
}

void DLL_PREFIX create_date_file_path(_TCHAR *file_path, int length, const _TCHAR *extension)
{
	my_tcscpy_s(file_path, length, create_date_file_path(extension));
}

const _TCHAR *DLL_PREFIX create_date_file_name(const _TCHAR *extension)
{
	static _TCHAR file_name[16][_MAX_PATH] = {0};
	static std::atomic<unsigned int> table_index = 0;
	unsigned int output_index = (table_index++) & 15;
	cur_time_t cur_time;

	get_host_time(&cur_time);
	my_stprintf_s(file_name[output_index], _MAX_PATH, _T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.%s"), cur_time.year, cur_time.month, cur_time.day, cur_time.hour, cur_time.minute, cur_time.second, extension);
	return (const _TCHAR *)file_name[output_index];
}

void DLL_PREFIX create_date_file_name(_TCHAR *file_path, int length, const _TCHAR *extension)
{
	my_tcscpy_s(file_path, length, create_date_file_name(extension));
}

bool DLL_PREFIX check_file_extension(const _TCHAR *file_path, const _TCHAR *ext)
{
#if defined(_USE_QT)
	std::string s_fpath = file_path;
	std::string s_ext = ext;
	//bool f = false;
	int pos;
	std::transform(s_fpath.begin(), s_fpath.end(), s_fpath.begin(), to_upper());
	std::transform(s_ext.begin(), s_ext.end(), s_ext.begin(), to_upper());
	if(s_fpath.length() < s_ext.length()) return false;
	pos = s_fpath.rfind(s_ext.c_str(), s_fpath.length());
	if((pos != (int)std::string::npos) && (pos >= ((int)s_fpath.length() - (int)s_ext.length()))) return true;
	return false;
#else
	size_t nam_len = _tcslen(file_path);
	size_t ext_len = _tcslen(ext);

	return (nam_len >= ext_len && _tcsncicmp(&file_path[nam_len - ext_len], ext, ext_len) == 0);
#endif
}

const _TCHAR *DLL_PREFIX get_file_path_without_extensiton(const _TCHAR *file_path)
{
	static _TCHAR path[32][_MAX_PATH] = {0};
	static std::atomic<unsigned int> table_index = 0;
	unsigned int output_index = (table_index++) & 31;

	my_tcscpy_s(path[output_index], _MAX_PATH, file_path);
#if defined(_WIN32) && defined(_MSC_VER)
	PathRemoveExtension(path[output_index]);
#elif defined(_USE_QT)
	QString delim;
	delim = QString::fromUtf8(".");
	QString tmp_path = QString::fromLocal8Bit(file_path);
	int n = tmp_path.lastIndexOf(delim);
	if(n > 0) {
		tmp_path = tmp_path.left(n);
	}
	//printf("%s\n", tmp_path.toUtf8().constData());
	memset(path[output_index], 0x00, sizeof(_TCHAR) * _MAX_PATH);
	strncpy(path[output_index], tmp_path.toLocal8Bit().constData(), _MAX_PATH - 1);
#else
#endif
	return (const _TCHAR *)path[output_index];
}

void DLL_PREFIX get_long_full_path_name(const _TCHAR* src, _TCHAR* dst, size_t dst_len)
{
#ifdef _WIN32
	_TCHAR tmp[_MAX_PATH];
	if(GetFullPathName(src, _MAX_PATH, tmp, NULL) == 0) {
		my_tcscpy_s(dst, dst_len, src);
	} else if(GetLongPathName(tmp, dst, _MAX_PATH) == 0) {
		my_tcscpy_s(dst, dst_len, tmp);
	}
#elif defined(_USE_QT)
	QString tmp_path = QString::fromLocal8Bit(src);
//	QString tmp_path = QString::fromUtf8(src);
	QFileInfo info(tmp_path);
//	strncpy(dst, info.absoluteFilePath().toLocal8Bit().constData(), dst_len);
	my_tcscpy_s(dst, dst_len, info.absoluteFilePath().toLocal8Bit().constData());
//	my_tcscpy_s(dst, dst_len, info.absoluteFilePath().toUtf8().constData());
#else
	// write code for your environment

#endif
}

const _TCHAR *DLL_PREFIX get_parent_dir(const _TCHAR* file)
{
	static _TCHAR path[32][_MAX_PATH] = {0};
	static std::atomic<unsigned int> table_index = 0;
	unsigned int output_index = (table_index++) & 31;

#ifdef _WIN32
	_TCHAR *ptr;
	GetFullPathName(file, _MAX_PATH, path[output_index], &ptr);
	if(ptr != NULL) {
		*ptr = _T('\0');
	}
#elif defined(_USE_QT)
	QString delim;
#if defined(Q_OS_WIN)
	delim = QString::fromUtf8("\\");
#else
	delim = QString::fromUtf8("/");
#endif
	QString tmp_path = QString::fromLocal8Bit(file);
	int n = tmp_path.lastIndexOf(delim);
	if(n > 0) {
		tmp_path = tmp_path.left(n);
		tmp_path.append(delim);
	}
	//printf("%s\n", tmp_path.toUtf8().constData());
	memset(path[output_index], 0x00, sizeof(_TCHAR) * _MAX_PATH);
	strncpy(path[output_index], tmp_path.toLocal8Bit().constData(), _MAX_PATH - 1);
#else
	// write code for your environment
#endif
	return path[output_index];
}

const wchar_t *DLL_PREFIX char_to_wchar(const char *cs)
{
	// char to wchar_t
	__UNLIKELY_IF(cs == NULL) {
		return (const wchar_t *)("");
	}
	size_t _len = strlen(cs);
	__UNLIKELY_IF(_len == 0) {
		return (const wchar_t *)("");
	}
	static wchar_t ws[32][4096] = {0};
	static std::atomic<unsigned int> table_index = 0;
	unsigned int output_index = (table_index++) & 31;

#if defined(_WIN32) || defined(_USE_QT)
	mbstowcs(&(ws[output_index][0]), cs, min(_len, (size_t)(4096 - 1)));
#else
	// write code for your environment
#endif
	return (wchar_t *)(&(ws[output_index][0]));
}

const char *DLL_PREFIX wchar_to_char(const wchar_t *ws)
{
	// wchar_t to char
	__UNLIKELY_IF(ws == NULL) {
		return (const char *)("");
	}
	size_t _len = wcslen(ws);
	__UNLIKELY_IF(_len == 0) {
		return (const char *)("");
	}
	static char cs[32][4096] = {0};
	static std::atomic<unsigned int> table_index = 0;
	unsigned int output_index = (table_index++) & 31;

#if defined(_WIN32) || defined(_USE_QT)
	wcstombs(&(cs[output_index][0]), ws, min(_len, (size_t)(4096 - 1)));
#else
	// write code for your environment
#endif
	return &(cs[output_index][0]);
}

const _TCHAR *DLL_PREFIX char_to_tchar(const char *cs)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// char to wchar_t
	return char_to_wchar(cs);
#else
	// char to char
	return cs;
#endif
}

const char *DLL_PREFIX tchar_to_char(const _TCHAR *ts)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// wchar_t to char
	return wchar_to_char(ts);
#else
	// char to char
	return ts;
#endif
}

const _TCHAR *DLL_PREFIX wchar_to_tchar(const wchar_t *ws)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// wchar_t to wchar_t
	return ws;
#else
	// wchar_t to char
	return wchar_to_char(ws);
#endif
}

const wchar_t *DLL_PREFIX tchar_to_wchar(const _TCHAR *ts)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// wchar_t to wchar_t
	return ts;
#else
	// char to wchar_t
	return char_to_wchar(ts);
#endif
}

int DLL_PREFIX ucs4_kana_zenkaku_to_hankaku(const uint32_t in, uint32_t *buf, int bufchars)
{
	uint32_t out = (uint32_t)in;
	int letters = 1;
	if((buf != NULL) && (bufchars >= 4)) {
		memset(buf, 0x00, sizeof(uint32_t) * bufchars);
	} else {
		return -1;
	}
	// U+FF61-U+FF9F = HANKAKU KANA
	// U+FF01-U+FF5E = ZENKAKU ALPHABET
	// U+3041-U+309E = ZENKAKU HIRAGANA
	// U+30A1-U+30FE = ZENKAKU KATAKANA
	static const uint32_t kanacnvtable[] = {
		// AIUEO
		0xff67, 0xff71, 0xff68, 0xff72, 0xff69, 0xff73, 0xff6a, 0xff74, 0xff6b, 0xff75,
		// KA/GA
		0xff76, 0xff76, 0xff77, 0xff77, 0xff78, 0xff78, 0xff79, 0xff79, 0xff7a, 0xff7a,
		// SA/ZA
		0xff7b, 0xff7b, 0xff7c, 0xff7c, 0xff7d, 0xff7d, 0xff7e, 0xff7e, 0xff7f, 0xff7f,
		// TA/DA
		0xff80, 0xff80, 0xff81, 0xff81, 0xff6f, 0xff82, 0xff82, 0xff83, 0xff83, 0xff84, 0xff84,
		// NA
		0xff85, 0xff86, 0xff87, 0xff88, 0xff89,
		// HA/BA/PA
		0xff8a, 0xff8a, 0xff8a, 0xff8b, 0xff8b, 0xff8b, 0xff8c, 0xff8c, 0xff8c,
		0xff8d, 0xff8d, 0xff8d, 0xff8e, 0xff8e, 0xff8e,
		// MA
		0xff8f, 0xff90, 0xff91, 0xff92, 0xff93,
		// YAYUYO
		0xff6c, 0xff94, 0xff6d, 0xff95, 0xff6e, 0xff96,
		// RA
		0xff97, 0xff98, 0xff99, 0xff9a, 0xff9b,
		// WA
		0xff9c, 0xff9c, 0xff68, 0xff69, 0xff66, 0xff9d, 0xff73, 0xff76, 0xff79, 0xff9e, 0xff9f,
		0x00000000,
	};

	enum {
		L_NORMAL,
		L_SMALL,
		L_DAKUON,
		L_HANDAKUON,
		L_SMALL_WA,
		L_WI,
		L_WO,
		L_WE,
		L_VU,
		L_XKA,
		L_XKE
	};
	static const int attr_tbl[] = {
		// AIUEO
			L_SMALL, L_NORMAL, L_SMALL, L_NORMAL, L_SMALL, L_NORMAL, L_SMALL, L_NORMAL, L_SMALL, L_NORMAL,
		// Kx
		L_NORMAL, L_DAKUON, L_NORMAL, L_DAKUON, L_NORMAL, L_DAKUON, L_NORMAL, L_DAKUON, L_NORMAL, L_DAKUON,
		// Sx
		L_NORMAL, L_DAKUON, L_NORMAL, L_DAKUON, L_NORMAL, L_DAKUON, L_NORMAL, L_DAKUON, L_NORMAL, L_DAKUON,
		// Tx
		L_NORMAL, L_DAKUON, L_NORMAL, L_DAKUON, L_NORMAL, L_DAKUON, L_SMALL, L_NORMAL, L_DAKUON, L_NORMAL, L_DAKUON,
		// Nx
		L_NORMAL,   L_NORMAL,  L_NORMAL,   L_NORMAL,   L_NORMAL,
		// Hx
		L_NORMAL,   L_DAKUON,  L_HANDAKUON,  L_NORMAL,   L_DAKUON,  L_HANDAKUON,  L_NORMAL,   L_DAKUON,  L_HANDAKUON,  L_NORMAL,   L_DAKUON,  L_HANDAKUON,  L_NORMAL,   L_DAKUON,  L_HANDAKUON,
		// Mx
		L_NORMAL,   L_NORMAL,  L_NORMAL,   L_NORMAL,   L_NORMAL,
		// Yx
		L_SMALL, L_NORMAL, L_SMALL, L_NORMAL, L_SMALL, L_NORMAL,
		// Rx
		L_NORMAL,   L_NORMAL,  L_NORMAL,   L_NORMAL,   L_NORMAL,
		// Wx
		L_SMALL_WA, L_NORMAL, L_WI, L_WE, L_NORMAL, L_NORMAL, L_VU, L_XKA, L_XKE
	};

	uint32_t tmp = 0;
	if((in < 0xFF5F) && (in > 0xFF00)) { // ALPHABET
		out = in - 0xff00 + 0x20; // Zenkaku alphabet to Hankaku alphabet
		buf[0] = out;
		return 1;
	} else if((in < 0x3095) && (in > 0x3040)) { // Hiragana
		tmp = in - 0x3041;
	} else if((in < 0x30f5) && (in > 0x30a0)) { // Katakana
		tmp = in - 0x30a1;
	} else if((in == 0x3099) || (in == 0x309b)) { // DAKUON
		buf[0] = 0xff9e;
		return 1;
	} else if((in == 0x309a) || (in == 0x309c)) { // HAN DAKUON
		buf[0] = 0xff9f;
		return 1;
	} else if(in == 0x3002) {// MARU
		buf[0] = 0xff61;
		return 1;
	} else if(in == 0x30fb) {// NAKAGURO
		buf[0] = 0xff65;
		return 1;
	} else if(in == 0x30fc) {// ONBIKI
		buf[0] = 0xff70;
		return 1;
	} else if(in == 0x3001) {// Ten
		buf[0] = 0xff64;
		return 1;
	} else if((in == 0x300c) || (in == 0x300d)){// KagiKakko
		buf[0] = 0xff62 + (in - 0x300c);
		return 1;
	} else {
		buf[0] = in;
		return 1;
	}
	uint32_t kana = kanacnvtable[tmp];
	int attr = attr_tbl[tmp];
	switch(attr) {
	case L_NORMAL:
	case L_SMALL:
		buf[0] = kana;
		break;
	case L_DAKUON:
		buf[0] = kana;
		buf[1] = 0xff9e; // DAKUON
		letters = 2;
		break;
	case L_HANDAKUON:
		buf[0] = kana;
		buf[1] = 0xff9f; // DAKUON
		letters = 2;
		break;
	case L_SMALL_WA:
		buf[0] = kana;
		break;
	case L_WI:
		buf[0] = kana; // WIP
		break;
	case L_WE:
		buf[0] = kana; // WIP
		break;
	case L_VU:
		buf[0] = 0xff73; // U
		buf[1] = 0xff9f; // DAKUON
		letters = 2;
		break;
	case L_XKA:
		buf[0] = 0xff76; // KA
		break;
	case L_XKE:
		buf[0] = 0xff79; // KE
		break;
	default:
		buf[0] = kana;
		break;
	}
	return letters;
}

const _TCHAR *DLL_PREFIX  create_string(const _TCHAR* format, ...)
{
	static _TCHAR buffer[64][1024] = {0};
	static std::atomic<unsigned int> table_index = 0;
	unsigned int output_index = (table_index++) & 63;
	va_list ap;

	va_start(ap, format);
	my_vstprintf_s(buffer[output_index], 1024, format, ap);
	va_end(ap);
	return (const _TCHAR *)buffer[output_index];
}

int32_t DLL_PREFIX  __FASTCALL muldiv_s32(int32_t nNumber, int32_t nNumerator, int32_t nDenominator)
{
	try {
		int64_t tmp;
		tmp  = (int64_t)nNumber;
		tmp *= (int64_t)nNumerator;
		tmp /= (int64_t)nDenominator;
		return (int32_t)tmp;
	} catch(...) {
		double tmp;
		tmp  = (double)nNumber;
		tmp *= (double)nNumerator;
		tmp /= (double)nDenominator;
		if(tmp < 0) {
			return (int32_t)(tmp - 0.5);
		} else {
			return (int32_t)(tmp + 0.5);
		}
	}
}

uint32_t DLL_PREFIX  __FASTCALL muldiv_u32(uint32_t nNumber, uint32_t nNumerator, uint32_t nDenominator)
{
	try {
		uint64_t tmp;
		tmp  = (uint64_t)nNumber;
		tmp *= (uint64_t)nNumerator;
		tmp /= (uint64_t)nDenominator;
		return (uint32_t)tmp;
	} catch(...) {
		double tmp;
		tmp  = (double)nNumber;
		tmp *= (double)nNumerator;
		tmp /= (double)nDenominator;
		return (uint32_t)(tmp + 0.5);
	}
}

static bool _crc_initialized = false;
static uint32_t _crc_table[256] = {0};
static void init_crc32_table(void)
{
	for(int i = 0; i < 256; i++) {
		uint32_t c = i;
		for(int j = 0; j < 8; j++) {
			if(c & 1) {
				c = (c >> 1) ^ 0xedb88320;
			} else {
				c >>= 1;
			}
		}
		_crc_table[i] = c;
	}
	_crc_initialized = true;
}

uint32_t DLL_PREFIX  get_crc32(uint8_t data[], int size)
{
	const uint32_t *table = (const uint32_t *)_crc_table;
	if(!_crc_initialized) {
		init_crc32_table();
	}

	uint32_t c = ~0;
	for(int i = 0; i < size; i++) {
		c = table[(c ^ data[i]) & 0xff] ^ (c >> 8);
	}
	return ~c;
}

uint32_t DLL_PREFIX  calc_crc32(uint32_t seed, uint8_t data[], int size)
{
#if 0
	if(!_crc_initialized) {
		init_crc32_table();
	}
	const uint32_t *table = (const uint32_t *)_crc_table;

	uint32_t c = ~seed;
	for(int i = 0; i < size; i++) {
		c = table[(c ^ data[i]) & 0xff] ^ (c >> 8);
	}
	return ~c;
#else
	// Calculate CRC32
	// Refer to : https://qiita.com/mikecat_mixc/items/e5d236e3a3803ef7d3c5
	static const uint32_t CRC_MAGIC_WORD = 0x04C11DB7;
	uint32_t crc = seed;
	uint8_t *ptr = data;
	uint8_t d;
	int bytes = size;
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
#endif
}


void DLL_PREFIX get_host_time(cur_time_t* cur_time)
{
#ifdef _WIN32
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);
	cur_time->year = sTime.wYear;
	cur_time->month = sTime.wMonth;
	cur_time->day = sTime.wDay;
	cur_time->day_of_week = sTime.wDayOfWeek;
	cur_time->hour = sTime.wHour;
	cur_time->minute = sTime.wMinute;
	cur_time->second = sTime.wSecond;
#else
	time_t timer = time(NULL);
	struct tm *local = localtime(&timer);
	cur_time->year = local->tm_year + 1900;
	cur_time->month = local->tm_mon + 1;
	cur_time->day = local->tm_mday;
	cur_time->day_of_week = local->tm_wday;
	cur_time->hour = local->tm_hour;
	cur_time->minute = local->tm_min;
	cur_time->second = local->tm_sec;
#endif
}



void DLL_PREFIX cur_time_t::increment()
{
	if(++second >= 60) {
		second = 0;
		if(++minute >= 60) {
			minute = 0;
			if(++hour >= 24) {
				hour = 0;
				// days in this month
				int days = 31;
				if(month == 2) {
					days = LEAP_YEAR(year) ? 29 : 28;
				} else if(month == 4 || month == 6 || month == 9 || month == 11) {
					days = 30;
				}
				if(++day > days) {
					day = 1;
					if(++month > 12) {
						month = 1;
						year++;
					}
				}
				if(++day_of_week >= 7) {
					day_of_week = 0;
				}
			}
		}
	}
}

void DLL_PREFIX cur_time_t::update_year()
{
	// 1970-2069
	if(year < 70) {
		year += 2000;
	} else if(year < 100) {
		year += 1900;
	}
}

void DLL_PREFIX cur_time_t::update_day_of_week()
{
	static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	int y = year - (month < 3);
	day_of_week = (y + y / 4 - y / 100 + y / 400 + t[month - 1] + day) % 7;
}

#define STATE_VERSION	1


bool DLL_PREFIX cur_time_t::process_state(void *f, bool loading)
{
	FILEIO *state_fio = (FILEIO *)f;

	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	state_fio->StateValue(year);
	state_fio->StateValue(month);
	state_fio->StateValue(day);
	state_fio->StateValue(day_of_week);
	state_fio->StateValue(hour);
	state_fio->StateValue(minute);
	state_fio->StateValue(second);
	state_fio->StateValue(initialized);
	return true;
}

const _TCHAR *DLL_PREFIX get_symbol(symbol_t *first_symbol, uint32_t addr)
{
	static _TCHAR name[64][1024] = {0};
	static std::atomic<unsigned int> table_index = 0;
	unsigned int output_index = (table_index++) & 63;

	if(first_symbol != NULL) {
		for(symbol_t* symbol = first_symbol; symbol; symbol = symbol->next_symbol) {
			if(symbol->addr == addr) {
				my_tcscpy_s(name[output_index], 1024, symbol->name);
				return name[output_index];
			}
		}
	}
	return NULL;
}

const _TCHAR *DLL_PREFIX get_value_or_symbol(symbol_t *first_symbol, const _TCHAR *format, uint32_t addr)
{
	static _TCHAR name[64][1024] = {0};
	static std::atomic<unsigned int> table_index = 0;
	unsigned int output_index = (table_index++) & 63;

	if(first_symbol != NULL) {
		for(symbol_t* symbol = first_symbol; symbol; symbol = symbol->next_symbol) {
			if(symbol->addr == addr) {
				my_tcscpy_s(name[output_index], 1024, symbol->name);
				return name[output_index];
			}
 		}
 	}
	my_stprintf_s(name[output_index], 1024, format, addr);
	return name[output_index];
}

const _TCHAR *DLL_PREFIX get_value_and_symbol(symbol_t *first_symbol, const _TCHAR *format, uint32_t addr)
{
	static _TCHAR name[64][1024] = {0};
	static std::atomic<unsigned int> table_index = 0;
	unsigned int output_index = (table_index++) & 63;

	my_stprintf_s(name[output_index], 1024, format, addr);

	if(first_symbol != NULL) {
		for(symbol_t* symbol = first_symbol; symbol; symbol = symbol->next_symbol) {
			if(symbol->addr == addr) {
				_TCHAR temp[1024];
//				my_stprintf_s(temp, 1024, _T(" (%s)"), symbol->name);
				my_stprintf_s(temp, 1024, _T(";%s"), symbol->name);
				my_tcscat_s(name[output_index], 1024, temp);
				return name[output_index];
			}
		}
	}
	return name[output_index];
}

#include "./types/util_sound.h"
// Use this before writing wav_data.
bool DLL_PREFIX write_dummy_wav_header(void *__fio)
{
	if(__fio == NULL) return false;

	FILEIO *fio = (FILEIO *)__fio;
	uint8_t dummy[sizeof(wav_header_t) + sizeof(wav_chunk_t)];

	if(!fio->IsOpened()) return false;

	memset(dummy, 0, sizeof(dummy));
	fio->Fwrite(dummy, sizeof(dummy), 1);
	return true;
}
// Use this after writing wav_data.
bool DLL_PREFIX set_wav_header(wav_header_t *header, wav_chunk_t *first_chunk, uint16_t channels, uint32_t rate,
							   uint16_t bits, size_t file_length)
{
	uint32_t length = (uint32_t) file_length;

	if(header == NULL) return false;
	if(first_chunk == NULL) return false;

	pair32_t __riff_chunk_size;
	pair32_t __fmt_chunk_size;
	pair32_t __wav_chunk_size;
	pair16_t __fmt_id;
	pair16_t __channels;
	pair32_t __sample_rate;
	pair32_t __data_speed;
	pair16_t __block_size;
	pair16_t __sample_bits;

	__riff_chunk_size.d = length - 8;
	__fmt_chunk_size.d = 16;
	__fmt_id.u16 = 1;
	__channels.u16 = channels;
	__sample_rate.d = rate;
	__block_size.u16 = (uint16_t)((channels * bits) / 8);
	__sample_bits.u16 = bits;
	__data_speed.d = rate * (uint32_t)(__block_size.u16);

	memcpy(&(header->riff_chunk.id), "RIFF", 4);
	header->riff_chunk.size = __riff_chunk_size.get_4bytes_le_to();

	memcpy(&(header->wave), "WAVE", 4);
	memcpy(&(header->fmt_chunk.id), "fmt ", 4);
	header->fmt_chunk.size = __fmt_chunk_size.get_4bytes_le_to();
	header->format_id = __fmt_id.get_2bytes_le_to();
	header->channels = __channels.get_2bytes_le_to();
	header->sample_rate = __sample_rate.get_4bytes_le_to();
	header->data_speed =  __data_speed.get_4bytes_le_to();
	header->block_size = __block_size.get_2bytes_le_to();
	header->sample_bits = __sample_bits.get_2bytes_le_to();

	memcpy(&(first_chunk->id), "data", 4);
	__wav_chunk_size.d = length - sizeof(wav_header_t) - sizeof(wav_chunk_t);
	first_chunk->size = __wav_chunk_size.get_4bytes_le_to();

	return true;
}
// Note: buffers are allocated by this, You should free() within user class.
bool DLL_PREFIX load_wav_to_stereo(void *__fio, int16_t **left_buf, int16_t **right_buf, uint32_t *rate, int *got_samples)
{

	if(__fio == NULL) return false;
	if(left_buf == NULL) return false;
	if(right_buf == NULL) return false;
	if(rate == NULL) return false;
	if(got_samples == NULL) return false;
	//if((bits != 8) && (bits != 16) && (bits != 32)) return false;

	FILEIO *fio = (FILEIO *)__fio;
	if(!fio->IsOpened()) return false;


	int16_t *left_buffer = NULL;
	int16_t *right_buffer = NULL;
	size_t samples = 0;
	uint32_t sample_rate = 0;

	wav_header_t header;
	wav_chunk_t  chunk;

	pair16_t __fmt_id;
	pair16_t __sample_bits;
	pair16_t __channels;
	pair32_t __sample_rate;
	pair32_t __chunk_size;

	fio->Fread(&header, sizeof(header), 1);
	__fmt_id.set_2bytes_le_from(header.format_id);
	__sample_bits.set_2bytes_le_from(header.sample_bits);
	__chunk_size.set_4bytes_le_from(header.fmt_chunk.size);
	__channels.set_2bytes_le_from(header.channels);
	__sample_rate.set_4bytes_le_from(header.sample_rate);

	if((__fmt_id.u16 == 1) && ((__sample_bits.u16 == 8) || (__sample_bits.u16 == 16) || (__sample_bits.u16 == 32))) {
		fio->Fseek(__chunk_size.d - 16, FILEIO_SEEK_CUR);
		bool is_eof = false;
		while(1) {
			if(fio->Fread(&chunk, sizeof(chunk), 1) != 1) {
				is_eof = true;
				break;
			}
			__chunk_size.set_4bytes_le_from(chunk.size);
			if(strncmp(chunk.id, "data", 4) == 0) {
				break;
			}
			fio->Fseek(__chunk_size.d, FILEIO_SEEK_CUR);
		}
		__chunk_size.set_4bytes_le_from(chunk.size);
		if(is_eof) {
			fio->Fclose();
			delete fio;
			return false;
		}

		samples = (size_t)(__chunk_size.d / __channels.u16);
		int16_t data_l, data_r;
		union {
			int16_t s16;
			struct {
				uint8_t l, h;
			} b;
		} pair16;
		union {
			int32_t s32;
			struct {
				uint8_t l, h, h2, h3;
			} b;
		} pair32;

		if(samples > 0) {
			if(__sample_bits.u16 == 16) {
				samples /= 2;
			} else if(__sample_bits.u16 == 32) {
				samples /= 4;
			}
			if(samples == 0) return false;
			sample_rate = __sample_rate.d;

			left_buffer = (int16_t *)malloc(samples * sizeof(int16_t));
			right_buffer = (int16_t *)malloc(samples * sizeof(int16_t));
			if(left_buffer == NULL) {
				if(right_buffer != NULL) free(right_buffer);
				return false;
			}
			if(right_buffer == NULL) {
				if(left_buffer != NULL) free(left_buffer);
				return false;
			}
			switch(__sample_bits.u16) {
			case 8:
				if(__channels.s16 == 1) {
					for(int i = 0; i < samples; i++) {
						data_l = (int16_t)(fio->FgetUint8());
						data_l = (data_l - 128) * 256;
						left_buffer[i] = data_l;
						right_buffer[i] = data_l;
					}
				} else if(__channels.s16 == 2) {
					for(int i = 0; i < samples; i++) {
						data_l = (int16_t)(fio->FgetUint8());
						data_l = (data_l - 128) * 256;
						data_r = (int16_t)(fio->FgetUint8());
						data_r = (data_r - 128) * 256;
						left_buffer[i] = data_l;
						right_buffer[i] = data_r;
					}
				}
				break;
			case 16:
				if(__channels.s16 == 1) {
					for(int i = 0; i < samples; i++) {
						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_l = pair16.s16;

						left_buffer[i] = data_l;
						right_buffer[i] = data_l;
					}
				} else if(__channels.s16 == 2) {
					for(int i = 0; i < samples; i++) {
						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_l = pair16.s16;

						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_r = pair16.s16;
						left_buffer[i] = data_l;
						right_buffer[i] = data_r;
					}
				}
				break;
			case 32:
				if(__channels.s16 == 1) {
					for(int i = 0; i < samples; i++) {
						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data_l = (int16_t)(pair32.s32 / 65536);

						left_buffer[i] = data_l;
						right_buffer[i] = data_l;
					}
				} else if(__channels.s16 == 2) {
					for(int i = 0; i < samples; i++) {
						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data_l = (int16_t)(pair32.s32 / 65536);

						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data_r = (int16_t)(pair32.s32 / 65536);

						left_buffer[i] = data_l;
						right_buffer[i] = data_r;
					}
				}
				break;
			default:
				break;
			}
		}
	} else {
		return false;
	}
	*left_buf = left_buffer;
	*right_buf = right_buffer;
	*rate = sample_rate;
	*got_samples = (int)samples;
	return true;
}

bool DLL_PREFIX load_wav_to_monoral(void *__fio, int16_t **buffer, uint32_t *rate, int *got_samples)
{

	if(__fio == NULL) return false;
	if(buffer == NULL) return false;
	if(rate == NULL) return false;
	if(got_samples == NULL) return false;
	//if((bits != 8) && (bits != 16) && (bits != 32)) return false;

	FILEIO *fio = (FILEIO *)__fio;
	if(!fio->IsOpened()) return false;


	int16_t *left_buffer = NULL;
	size_t samples = 0;
	uint32_t sample_rate = 0;

	wav_header_t header;
	wav_chunk_t  chunk;

	pair16_t __fmt_id;
	pair16_t __sample_bits;
	pair16_t __channels;
	pair32_t __sample_rate;
	pair32_t __chunk_size;

	fio->Fread(&header, sizeof(header), 1);
	__fmt_id.set_2bytes_le_from(header.format_id);
	__sample_bits.set_2bytes_le_from(header.sample_bits);
	__chunk_size.set_4bytes_le_from(header.fmt_chunk.size);
	__channels.set_2bytes_le_from(header.channels);
	__sample_rate.set_4bytes_le_from(header.sample_rate);

	if((__fmt_id.u16 == 1) && ((__sample_bits.u16 == 8) || (__sample_bits.u16 == 16) || (__sample_bits.u16 == 32))) {
		fio->Fseek(__chunk_size.d - 16, FILEIO_SEEK_CUR);
		bool is_eof = false;
		while(1) {
			if(fio->Fread(&chunk, sizeof(chunk), 1) != 1) {
				is_eof = true;
				break;
			}
			__chunk_size.set_4bytes_le_from(chunk.size);
			if(strncmp(chunk.id, "data", 4) == 0) {
				break;
			}
			fio->Fseek(__chunk_size.d, FILEIO_SEEK_CUR);
		}
		__chunk_size.set_4bytes_le_from(chunk.size);
		if(is_eof) {
			fio->Fclose();
			delete fio;
			return false;
		}

		samples = (size_t)(__chunk_size.d / __channels.u16);
		int16_t data_l, data_r;
		int32_t data32_l, data32_r;
		union {
			int16_t s16;
			struct {
				uint8_t l, h;
			} b;
		} pair16;
		union {
			int32_t s32;
			struct {
				uint8_t l, h, h2, h3;
			} b;
		} pair32;

		if(samples > 0) {
			if(__sample_bits.u16 == 16) {
				samples /= 2;
			} else if(__sample_bits.u16 == 32) {
				samples /= 4;
			}
			if(samples == 0) return false;
			sample_rate = __sample_rate.d;

			left_buffer = (int16_t *)malloc(samples * sizeof(int16_t));
			if(left_buffer == NULL) {
				return false;
			}
			switch(__sample_bits.u16) {
			case 8:
				if(__channels.s16 == 1) {
					for(int i = 0; i < samples; i++) {
						data_l = (int16_t)(fio->FgetUint8());
						data_l = (data_l - 128) * 256;
						left_buffer[i] = data_l;
					}
				} else if(__channels.s16 == 2) {
					for(int i = 0; i < samples; i++) {
						data_l = (int16_t)(fio->FgetUint8());
						data_l = (data_l - 128) * 256;
						data_r = (int16_t)(fio->FgetUint8());
						data_r = (data_r - 128) * 256;
						left_buffer[i] = (data_l + data_r) / 2;
					}
				}
				break;
			case 16:
				if(__channels.s16 == 1) {
					for(int i = 0; i < samples; i++) {
						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_l = pair16.s16;

						left_buffer[i] = data_l;
					}
				} else if(__channels.s16 == 2) {
					for(int i = 0; i < samples; i++) {
						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_l = pair16.s16;

						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_r = pair16.s16;
						left_buffer[i] = (data_l + data_r) / 2;
					}
				}
				break;
			case 32:
				if(__channels.s16 == 1) {
					for(int i = 0; i < samples; i++) {
						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data_l = (int16_t)(pair32.s32 / 65536);

						left_buffer[i] = data_l;
					}
				} else if(__channels.s16 == 2) {
					for(int i = 0; i < samples; i++) {
						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data32_l = pair32.s32 / 65536;

						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data32_r = pair32.s32 / 65536;

						left_buffer[i] = (int16_t)((data32_l + data32_r) / 2);
					}
				}
				break;
			default:
				break;
			}
		}
	} else {
		return false;
	}
	*buffer = left_buffer;
	*rate = sample_rate;
	*got_samples = (int)samples;
	return true;
}

// From https://en.wikipedia.org/wiki/High-pass_filter
#if !defined(M_PI)
	#define M_PI 3.14159265358979323846
#endif
void DLL_PREFIX calc_high_pass_filter(int32_t* dst, int32_t* src, int sample_freq, int hpf_freq, int samples, double quality, bool is_add)
{
	if(src == NULL) return;
	if(dst == NULL) return;
	double ifreq = 1.0 / ((double)hpf_freq * 2.0 * M_PI);
	double isample = 1.0 / (double)sample_freq;
	float alpha = (float)(isample * quality / (ifreq + isample));

	if(alpha >= 1.0f) alpha = 1.0f;
	if(alpha <= 0.0f) alpha = 0.0f;
	float ialpha = 1.0f - alpha;

	#if 0
	__DECL_ALIGNED(16) float tmp_v[samples * 2 + 4]; // 2ch stereo
	__DECL_ALIGNED(16) float tmp_h[samples * 2 + 4];
	for(int i = 0; i < (samples * 2); i ++) {
		tmp_h[i] = (float)(src[i]);
	}
	tmp_v[0] = tmp_h[0];
	tmp_v[1] = tmp_h[1];
	for(int i = 2; i < (samples * 2); i += 2) {
		tmp_v[i + 0] = tmp_h[i + 0] * alpha + tmp_v[i - 2 + 0] * ialpha;
		tmp_v[i + 1] = tmp_h[i + 1] * alpha + tmp_v[i - 2 + 1] * ialpha;
		tmp_v[i + 0] = tmp_h[i + 0] - tmp_v[i + 0];
		tmp_v[i + 1] = tmp_h[i + 1] - tmp_v[i + 1];
	}
	#else
	const size_t n_samples = samples * 2;
	__DECL_ALIGNED(16) std::valarray<float> tmp_h(n_samples + 4);
	__DECL_ALIGNED(16) std::valarray<float> tmp_v(n_samples + 4);
	for(size_t i = 0; i < n_samples; i++) {
		tmp_h[i] = (float)(src[i]);
	}
	tmp_v[0] = tmp_h[0];
	tmp_v[1] = tmp_h[1];

	for(int i = 2; i < n_samples; i++) {
		tmp_v[i] = tmp_h[i] - (tmp_h[i] * alpha + tmp_v[i - 2] * ialpha);
	}
	#endif
	if(is_add) {
		for(int i = 0; i < (samples * 2); i++) {
			dst[i] = dst[i] + (int32_t)(tmp_v[i]);
		}
	} else {
		for(int i = 0; i < (samples * 2); i++) {
			dst[i] = (int32_t)(tmp_v[i]);
		}
	}
	
}

// From https://en.wikipedia.org/wiki/Low-pass_filter
void DLL_PREFIX calc_low_pass_filter(int32_t* dst, int32_t* src, int sample_freq, int lpf_freq, int samples, double quality, bool is_add)
{
	if(dst == NULL) return;
	if(src == NULL) return;

	double ifreq = 1.0 / ((double)lpf_freq * (2.0 * M_PI));
	double isample = 1.0 / (double)sample_freq;
	float alpha = (float)(isample * quality / (ifreq + isample));
	if(alpha >= 1.0f) alpha = 1.0f;
	if(alpha <= 0.0f) alpha = 0.0f;
	float ialpha = 1.0f - alpha;

	#if 0
	__DECL_ALIGNED(16) float tmp_v[samples * 2 + 4]; // 2ch stereo
	__DECL_ALIGNED(16) float tmp_h[samples * 2 + 4];

	for(int i = 0; i < (samples * 2); i++) {
		tmp_h[i] = (float)(src[i]);
	}
	tmp_v[0] = tmp_h[0];
	tmp_v[1] = tmp_h[1];
	for(int i = 2; i < (samples * 2); i += 2) {
		tmp_v[i + 0] = tmp_h[i + 0] * alpha + tmp_v[i - 2 + 0] * ialpha;
		tmp_v[i + 1] = tmp_h[i + 1] * alpha + tmp_v[i - 2 + 1] * ialpha;
	}
	#else
	const size_t n_samples = samples * 2;
	__DECL_ALIGNED(16) std::valarray<float> tmp_h(n_samples + 4);
	__DECL_ALIGNED(16) std::valarray<float> tmp_v(n_samples + 4);
	for(size_t i = 0; i < n_samples; i++) {
		tmp_h[i] = (float)(src[i]);
	}
	tmp_v[0] = tmp_h[0];
	tmp_v[1] = tmp_h[1];

	for(int i = 2; i < n_samples; i++) {
		tmp_v[i] = tmp_h[i] * alpha + tmp_v[i - 2] * ialpha;
	}
	#endif
	if(is_add) {
		for(int i = 0; i < (samples * 2); i++) {
			dst[i] = dst[i] + (int32_t)(tmp_v[i]);
		}
	} else {
		for(int i = 0; i < (samples * 2); i++) {
			dst[i] = (int32_t)(tmp_v[i]);
		}
	}
}


DLL_PREFIX const _TCHAR *get_lib_common_version()
{
#if defined(__LIBEMU_UTIL_VERSION)
	return (const _TCHAR *)__LIBEMU_UTIL_VERSION;
#else
	return (const _TCHAR *)"\0";
#endif
}
