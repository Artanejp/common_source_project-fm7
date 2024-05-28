#pragma once
#include "common.h"
#include <string>

#include <QString>
#include <QSettings>

namespace StateUtil_Qt {

bool beginSection(QSettings* set, _TCHAR* section)
{
	__UNLIKELY_IF(section == nullptr) {
		return false;
	}
	return beginSection(set, QString::fromUtf8(section));
}

bool beginSection(QSettings* set, std::string section)
{
	return beginSection(set, QString::fromStdString(section));
}

bool beginSection(QSettings* set, QString section)
{
	if(set == nullptr) {
		return false;
	}
	__UNLIKELY_IF(section.isEmpty()) {
		return false;
	}
	set->beginGroup(section);	
	return true;
}

bool endSection(QSettings* set)
{
	if(set == nullptr) {
		return false;
	}
	set->endGroup();
}

bool syncStatus(QSettings* set)
{
	if(set == nullptr) {
		return false;
	}
	set->sync();
}

bool isEntryExists(QSettings* set, _TCHAR* key)
{
	if(key == nullptr) {
		return false;
	}
	return isEntryExists(set, QString::fromUtf8(key));
}

bool isEntryExists(QSettings* set, std::string key)
{
	return isEntryExists(set, QString::fromStdString(key));
}

bool isEntryExists(QSettings* set, QString key)
{
	if(set == nullptr) {
		return false;
	}
	if(key.isEmpty()) {
		return false;
	}
	return set->contains(key);
}

bool eraseEntry(QSettings* set, _TCHAR* key)
{
	if(key == nullptr) {
		return false;
	}
	return eraseEntry(set, QString::fromUtf8(key));
}

bool eraseEntry(QSettings* set, std::string key)
{
	return eraseEntry(set, QString::fromStdString(key));
}

bool eraseEntry(QSettings* set, QString key)
{
	if(set == nullptr) {
		return false;
	}
	if(key.isEmpty()) {
		return false;
	}
	if(!(set->contains(key))) {
		return false; // Entry not exists.
	}
	set->remove(key);
	return true;
}

template <class X>
	bool saveValue(QSettings* set, _TCHAR* key, X value)
{
	__UNLIKELY_IF(key == nullptr) {
		return false;
	}
	return saveValue(set, QString::fromStdString(key), value);
}

template <class X>
	bool saveValue(QSettings* set, std::string key, X value)
{
	return saveValue(set, QString::fromStdString(key), value);
}

template <class X>
	bool saveValue(QSettings* set, QString key, X value)
{
	__UNLIKELY_IF(set == nullptr) {
		return false;
	}
	__UNLIKELY_IF(key.isEmpty()) {
		return false;
	}
	set->setValue(key, QVariant(value));
	return true;
}

bool saveValue(QSettings* set, _TCHAR* key, _TCHAR* value)
{
	__UNLIKELY_IF(key == nullptr) {
		return false;
	}
	__UNLIKELY_IF(value == nullptr) {
		return false;
	}
	return saveValue(set, QString::fromUtf8(key), QString::fromLocal8Bit(value));
}

bool saveValue(QSettings* set, std::string key, _TCHAR* value)
{
	__UNLIKELY_IF(value == nullptr) {
		return false;
	}
	return saveValue(set, QString::fromStdString(key), QString::fromLocal8Bit(value));
}

bool saveValue(QSettings* set, QString key, _TCHAR* value)
{
	__UNLIKELY_IF(value == nullptr) {
		return false;
	}
	return saveValue(set, key, QString::fromLocal8Bit(value));
}


template <class X>
	X loadValue(QSettings* set, _TCHAR* key, X& value)
{
	__UNLIKELY_IF(key == nullptr) {
		return value;
	}
	return loadValue(set, QString::fromUtf8(key), value);
}

template <class X>
	X loadValue(QSettings* set, std::string key, X& value)
{
	return loadValue(set, QString::fromStdString(key), value);
}

template <class X>
	X loadValue(QSettings* set, QString key, X& value)
{
	__UNLIKELY_IF(set == nullptr) {
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
size_t loadString(QSettings* set, _TCHAR* key, _TCHAR* buffer, size_t limit)
{
	__UNLIKELY_IF(key == nullptr) {
		return 0;
	}
	return loadString(set, QString::fromUtf8(key), buffer, limit);
}

size_t loadString(QSettings* set, std::string key, _TCHAR* buffer, size_t limit)
{
	return loadString(set, QString::fromStdString(key), buffer, limit);
}

size_t loadString(QSettings* set, QString key, _TCHAR* buffer, size_t limit)
{
	__UNLIKELY_IF(set == nullptr) {
		return 0;
	}
	__UNLIKELY_IF(buffer == nullptr) {
		return 0;
	}
	__UNLIKELY_IF(limit == 0) {
		return 0;
	}
	if(!(set->contains(key))) {
		return 0;
	}
	QString value = set->value(key).toString();
	if(value.isEmpty()) {
		return 0; // Keep data.
	}
	my_tcscpy_s(buffer, limit, value.toLocal8Bit().constData());
	return strlen(buffer);
}	

bool saveString(QSettings* set, _TCHAR* key, _TCHAR* buffer, size_t limit = 0)
{
	__UNLIKELY_IF(key == nullptr) {
		return false;
	}
	return loadString(set, QString::fromUtf8(key), buffer, limit);
}

bool saveString(QSettings* set, std::string key, _TCHAR* buffer, size_t limit = 0)
{
	return loadString(set, QString::fromStdString(key), buffer, limit);
}

bool saveString(QSettings* set, QString key, _TCHAR* buffer, size_t limit = 0)
{
	__UNLIKELY_IF(set == nullptr) {
		return false;
	}
	__UNLIKELY_IF(buffer == nullptr) {
		return false;
	}
	QString value;
	__UNLIKELY_IF(limit == 0) {
		value = QString::fromLocal8Bit(buffer);
	} else {
		value = QString::fromLocal8Bit(buffer, limit);
	}
	set->setValue(value);
	return true;
}	

// These load/save as UTF8, not as Local locale.
size_t loadUtf8String(QSettings* set, _TCHAR* key, _TCHAR* buffer, size_t limit)
{
	__UNLIKELY_IF(key == nullptr) {
		return 0;
	}
	return loadUtf8String(set, QString::fromUtf8(key), buffer, limit);
}

size_t loadUtf8String(QSettings* set, std::string key, _TCHAR* buffer, size_t limit)
{
	return loadUtf8String(set, QString::fromStdString(key), buffer, limit);
}

size_t loadUtf8String(QSettings* set, QString key, _TCHAR* buffer, size_t limit)
{
	__UNLIKELY_IF(set == nullptr) {
		return false;
	}
	__UNLIKELY_IF(buffer == nullptr) {
		return 0;
	}
	__UNLIKELY_IF(limit == 0) {
		return 0;
	}
	if(!(set->contains(key))) {
		return 0;
	}
	QString value = set->value(key).toString();
	if(value.isEmpty()) {
		return 0; // Keep data.
	}
	my_tcscpy_s(buffer, limit, value.toUtf8().constData());
	return strlen(buffer);
}


bool saveUtf8String(QSettings* set, _TCHAR* key, _TCHAR* buffer, size_t limit = 0)
{
	__UNLIKELY_IF(key == nullptr) {
		return false;
	}
	return saveUtf8String(set, QString::fromUtf8(key), buffer, limit);
}

bool saveUtf8String(QSettings* set, std::string key, _TCHAR* buffer, size_t limit = 0)
{
	return saveUtf8String(set, QString::fromStdString(key), buffer, limit);
}

bool saveUtf8String(QSettings* set, QString key, _TCHAR* buffer, size_t limit = 0)
{
	__UNLIKELY_IF(set == nullptr) {
		return false;
	}
	__UNLIKELY_IF(buffer == nullptr) {
		return false;
	}
	QString value;
	__UNLIKELY_IF(limit == 0) {
		value = QString::fromUtf8(buffer);
	} else {
		value = QString::fromUtf8(buffer, limit);
	}
	set->setValue(value);
	return true;
}	

template <class X>
	QMap<int, X> loadToMap(QSettings* set, QString key_head, ssize_t __size, int begin_val = 1)
{
	QMap<int, X> tmp;
	if(set == nullptr) {
		return tmp;
	}
	if(__size <= 0) {
		return tmp;
	}
	int num = begin;
	for(size_t x = 0; x < __size; x++) {
		QString key = key_head + QString::fromUtf8("%1").arg((int)(begin_val + x));
		if(set->contains(key)) {
			X val = set->value();
			tmp.insert((int)(begin_val + x), X);
		}
	}
	return tmp;
}

template <class X>
	size_t saveFromMap(QSettings* set, QString key_head, QMap<int, X> values, int begin_val = 1)
{
	if(set == nullptr) {
		return 0;
	}
	if(values.isEmpty()) {
		return 0;
	}
	size_t count = 0;
	for(auto p = values.begin(); p != values.end(); ++p) {
		QString key = key_head + QString::fromUtf8("%1").arg((int)((*p).key()));
		X val = (*p).value();
		set->setValue(key, QVariant(val));
		count++;
	}
	return count;
}


}
