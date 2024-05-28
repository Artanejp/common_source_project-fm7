#pragma once
#include "common.h"

#include <string>
#include <memory>

#include <QString>
#include <QSettings>

class USING_FLAGS;
class StateUtil_Qt : public QObject
{
protected:
	std::shared_ptr<QSettings> m_set;
	QString m_recent_section_name;
public:
	StateUtil_Qt(std::shared_ptr<QSettings>set, QObject* parent = nullptr);
	StateUtil_Qt(StateUtil_Qt* another, QObject* parent = nullptr);
	StateUtil_Qt(QObject* parent = nullptr);
	~StateUtil_Qt();

	set_settings(std::shared_ptr<QSettings> set);
	set_settings(std::shared_ptr<USING_FLAGS> p_flags);
	void set_settings(StaheUtil_Qt* another);
	
	std::shared_ptr<QSettings> get_settings();
	
	QString get_recent_section_name();
	void reset_recent_section_name() { m_recent_section_name.clear(); }
	
	bool beginSection(_TCHAR* section);
	bool beginSection(std::string section);
	bool beginSection(QString section);

	bool endSection();
	bool syncStatus();

	bool isEntryExists(_TCHAR* key);
	bool isEntryExists(std::string key);
	bool isEntryExists(QString key);

	bool eraseEntry(QSettings* set, _TCHAR* key);
	bool eraseEntry(QSettings* set, std::string key);

	bool eraseEntry(QSettings* set, QString key);

	template <class X>
		bool saveValue(_TCHAR* key, X value)
	{
		__UNLIKELY_IF(key == nullptr) {
			return false;
		}
		return saveValue(QString::fromStdString(key), value);
	}

	template <class X>
		bool saveValue(std::string key, X value)
	{
		return saveValue(QString::fromStdString(key), value);
	}

	template <class X>
		bool saveValue(QString key, X value)
	{
		std::shared_ptr<QSetting> set = m_set;
	
		__UNLIKELY_IF(set.get() == nullptr) {
			return false;
		}
		__UNLIKELY_IF(key.isEmpty()) {
			return false;
		}
		set->setValue(key, QVariant(value));
		return true;
	}


	template <class X>
		X loadValue(_TCHAR* key, X& value)
	{
		__UNLIKELY_IF(key == nullptr) {
			return value;
		}
		return loadValue(QString::fromUtf8(key), value);
	}

	template <class X>
		X loadValue(std::string key, X& value)
	{
		return loadValue(QString::fromStdString(key), value);
	}

	template <class X>
		X loadValue(QString key, X& value)
	{
		std::shared_ptr<QSetting> set = m_set;
		__UNLIKELY_IF(set.get() == nullptr) {
			return value;
		}
		__UNLIKELY_IF(key.isEmpty()) {
			return value;
		}
		if(!(set->contains(key))) {
			return value;
		}
		value = set->value(key);
		return value;
	}

	// These load/save as Local Locale, not as UTF8.
	size_t loadString(_TCHAR* key, _TCHAR* buffer, size_t limit);
	size_t loadString(std::string key, _TCHAR* buffer, size_t limit);
	size_t loadString(QString key, _TCHAR* buffer, size_t limit);

	bool saveString(_TCHAR* key, _TCHAR* buffer, size_t limit = 0);
	bool saveString(std::string key, _TCHAR* buffer, size_t limit = 0);
	bool saveString(QString key, _TCHAR* buffer, size_t limit = 0);
	
	size_t loadToStringArray(_TCAHR *key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1, bool need_clear = true);
	size_t loadToStringArray(std::string key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1, bool need_clear = true);
	size_t loadToStringArray(QString key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1, bool need_clear = true);
	
	size_t saveFromStringArray(_TCHAR* key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1);
	size_t saveFromStringArray(std::string key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1);
	size_t saveFromStringArray(QString key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1);
	
	// These load/save as UTF8, not as Local locale.
	size_t loadUtf8String(_TCHAR* key, _TCHAR* buffer, size_t limit);
	size_t loadUtf8String(std::string key, _TCHAR* buffer, size_t limit);
	size_t loadUtf8String(QString key, _TCHAR* buffer, size_t limit);

	bool saveUtf8String(_TCHAR* key, _TCHAR* buffer, size_t limit = 0);
	bool saveUtf8String(std::string key, _TCHAR* buffer, size_t limit = 0);
	bool saveUtf8String(QString key, _TCHAR* buffer, size_t limit = 0);

	size_t loadToUtf8StringArray(_TCAHR *key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1, bool need_clear = true);
	size_t loadToUtf8StringArray(std::string key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1, bool need_clear = true);
	size_t loadToUtf8StringArray(QString key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1, bool need_clear = true);
	
	size_t saveFromUtf8StringArray(_TCHAR* key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1);
	size_t saveFromUtf8StringArray(std::string key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1);
	size_t saveFromUtf8StringArray(QString key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val = 1);
	
	// load to QMap<int key, X value>
	template <class X>
		QMap<int, X> loadToMap(QString key_head, ssize_t __size, int begin_val = 1)
	{
		QMap<int, X> tmp;
		std::shared_ptr<QSetting> set = m_set;
		if(set.get() == nullptr) {
			return tmp;
		}
		if(__size <= 0) {
			return tmp;
		}
		for(int x = 0; x < __size; x++) {
			QString key = key_head + QString::fromUtf8("%1").arg(begin_val + x);
			if(set->contains(key)) {
				X val = set->value();
				tmp.insert(begin_val + x, X);
			}
		}
		return tmp;
	}

	// save from QMap<int key, X value>
	template <class X>
		size_t saveFromMap(QString key_head, QMap<int, X> values, int begin_val = 1, int end_val = INT32_MAX, size_t limit = INT32_MAX)
	{
		std::shared_ptr<QSetting> set = m_set;
		if(set.get() == nullptr) {
			return 0;
		}
		if(values.isEmpty()) {
			return 0;
		}
		size_t count = 0;
		for(auto p = values.begin(); p != values.end(); ++p) {
			if(count >= limit) break;
			int n = (*p).key();
			if((n >= begin_val) && (n <= end_val)) {
				QString key = key_head + QString::fromUtf8("%1").arg(n);
				X val = (*p).value();
				set->setValue(key, QVariant(val));
				count++;
			}
		}
		return count;
	}

	// load to array, X[__size] .
	template <class X>
		size_t loadToArray(QString key_head, X* p_array, ssize_t __size, int begin_val = 1)
	{
		std::shared_ptr<QSetting> set = m_set;
		if(set.get() == nullptr) {
			return 0;
		}
		if(__size <= 0) {
			return 0;
		}
		if(p_array == nullptr) {
			return 0;
		}
		int num = begin;
		size_t count = 0;
		for(int x = 0; x < __size; x++) {
			QString key = key_head + QString::fromUtf8("%1").arg(begin_val + x);
			if(set->contains(key)) {
				X val = set->value();
				p_array[x] = val;
				count++;
			}
		}
		return count;
	}
	
	// save from array, X[__size] .
	template <class X>
		size_t saveFromArray(QString key_head, X* p_array, ssize_t __size, int begin_val = 1)
	{
		std::shared_ptr<QSetting> set = m_set;
		if(set.get() == nullptr) {
			return 0;
		}
		if(__size <= 0) {
			return 0;
		}
		if(p_array == nullptr) {
			return 0;
		}
		size_t count = 0;
		for(int x = 0; x < __size; x++) {
			QString key = key_head + QString::fromUtf8("%1").arg(begin_val + x);
			X val = p_array[x];
			set->setValue(key, QVariant(val));
			count++;
		}
		return count;
	}

};
