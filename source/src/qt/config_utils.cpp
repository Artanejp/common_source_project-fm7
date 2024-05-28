#include "./config_utils.h"

StateUtil_Qt::StateUtil_Qt(std::shared_ptr<QSettings>set, QObject* parent)
	: QObject(parent)
{
	m_recent_section.reset();
	m_set = set;
}

StateUtil_Qt::StateUtil_Qt(StateUtil_Qt* another, QObject* parent)
	: QObject(parent)
{
	m_recent_section.reset();
	if(another != nullptr) {
		m_set = another->get_settings();
		m_recent_section_name = another->get_recent_section_name();
	}
}


StateUtil_Qt::StateUtil_Qt(QObject* parent)
	: QObject(parent)
{
	m_set.reset();
	m_recent_section.reset();
}

StateUtil_Qt::StateUtil_Qt(std::shared_ptr<USING_FLAGS> p_flags, QObject* parent)
	: QObject(parent)
{
	m_recent_section.reset();
	if(p_flags.get() != nullptr) {
		m_set = p_flags->get_settings();
	} else {
		m_set.reset();
	}
}

StateUtil_Qt::~StateUtil_Qt();
{
}

void StateUtil_Qt::set_settings(std::shared_ptr<QSettings> set)
{
	m_set = set;
	return m_set.get();
}

void StateUtil_Qt::set_settings(StaheUtil_Qt* another)
{
	if(another != nullptr) {
		m_set = another->get_settings();
		m_recent_section_name = another->get_recent_section_name();
	}
	return m_set.get();
}


void StateUtil_Qt::set_settings(std::shared_ptr<USING_FLAGS> p_flags)
{
	if(p_flags.get() != nullptr) {
		m_set = p_flags->get_settings();
	}
	return m_set.get();
}

std::shared_ptr<QSettings> StateUtil_Qt::get_settings()
{
	return m_ptr;
}

QString StateUtil_Qt::get_recent_section_name()
{
	return m_recent_section_name;
}

bool StateUtil_Qt::beginSection(_TCHAR* section)
{
	__UNLIKELY_IF(section == nullptr) {
		return false;
	}
	return beginSection(QString::fromUtf8(section));
}

bool StateUtil_Qt::beginSection(std::string section)
{
	return beginSection(QString::fromStdString(section));
}

bool StateUtil_Qt::beginSection(QString section)
{
	std::shared_ptr<QSetting> set = m_set;
	if(set.get() == nullptr) {
		return false;
	}
	__UNLIKELY_IF(section.isEmpty()) {
		return false;
	}
	set->beginGroup(section);	
	return true;
}

bool StateUtil_Qt::endSection()
{
	std::shared_ptr<QSetting> set = m_set;
	if(set.get() == nullptr) {
		return false;
	}
	set->endGroup();
}

bool StateUtil_Qt::syncStatus()
{
	std::shared_ptr<QSetting> set = m_set;
	if(set.get() == nullptr) {
		return false;
	}
	set->sync();
}

bool StateUtil_Qt::isEntryExists(_TCHAR* key)
{
	if(key == nullptr) {
		return false;
	}
	return isEntryExists(QString::fromUtf8(key));
}

bool StateUtil_Qt::isEntryExists(std::string key)
{
	return isEntryExists(QString::fromStdString(key));
}

bool StateUtil_Qt::isEntryExists(QString key)
{
	std::shared_ptr<QSetting> set = m_set;
	if(set.get() == nullptr) {
		return false;
	}
	if(key.isEmpty()) {
		return false;
	}
	return set->contains(key);
}

bool StateUtil_Qt::eraseEntry(_TCHAR* key)
{
	if(key == nullptr) {
		return false;
	}
	return eraseEntry(QString::fromUtf8(key));
}

bool StateUtil_Qt::eraseEntry(std::string key)
{
	return eraseEntry(QString::fromStdString(key));
}

bool StateUtil_Qt::eraseEntry(QString key)
{
	std::shared_ptr<QSetting> set = m_set;
	if(set.get() == nullptr) {
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

// These load/save as Local Locale, not as UTF8.
size_t StateUtil_Qt::loadString(_TCHAR* key, _TCHAR* buffer, size_t limit)
{
	__UNLIKELY_IF(key == nullptr) {
		return 0;
	}
	return loadString(QString::fromUtf8(key), buffer, limit);
}

size_t StateUtil_Qt::loadString(std::string key, _TCHAR* buffer, size_t limit)
{
	return loadString(QString::fromStdString(key), buffer, limit);
}

size_t StateUtil_Qt::loadString(QString key, _TCHAR* buffer, size_t limit)
{
	std::shared_ptr<QSettings> set = m_set;
	__UNLIKELY_IF(set.get() == nullptr) {
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

bool StateUtil_Qt::saveString(_TCHAR* key, _TCHAR* buffer, size_t limit)
{
	__UNLIKELY_IF(key == nullptr) {
		return false;
	}
	return saveString(QString::fromUtf8(key), buffer, limit);
}

bool StateUtil_Qt::saveString(std::string key, _TCHAR* buffer, size_t limit)
{
	return saveString(QString::fromStdString(key), buffer, limit);
}

bool StateUtil_Qt::saveString(QString key, _TCHAR* buffer, size_t limit)
{
	std::shared_ptr<QSettings> set = m_set;
	__UNLIKELY_IF(set.get() == nullptr) {
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
size_t StateUtil_Qt::loadUtf8String(_TCHAR* key, _TCHAR* buffer, size_t limit)
{
	__UNLIKELY_IF(key == nullptr) {
		return 0;
	}
	return loadUtf8String(QString::fromUtf8(key), buffer, limit);
}

size_t StateUtil_Qt::loadUtf8String(std::string key, _TCHAR* buffer, size_t limit)
{
	return loadUtf8String(QString::fromStdString(key), buffer, limit);
}

size_t StateUtil_Qt::loadUtf8String(QString key, _TCHAR* buffer, size_t limit)
{
	std::shared_ptr<QSettings> set = m_set;
	__UNLIKELY_IF(set.get() == nullptr) {
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


bool StateUtil_Qt::saveUtf8String(_TCHAR* key, _TCHAR* buffer, size_t limit = 0)
{
	__UNLIKELY_IF(key == nullptr) {
		return false;
	}
	return saveUtf8String(QString::fromUtf8(key), buffer, limit);
}

bool StateUtil_Qt::saveUtf8String(QSettings* set, std::string key, _TCHAR* buffer, size_t limit = 0)
{
	return saveUtf8String(QString::fromStdString(key), buffer, limit);
}

bool StateUtil_Qt::saveUtf8String(QString key, _TCHAR* buffer, size_t limit = 0)
{
	std::shared_ptr<QSettings> set = m_set;
	__UNLIKELY_IF(set.get() == nullptr) {
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
	set->setValue(key, value);
	return true;
}	

size_t StateUtil_Qt::loadToStringArray(_TCAHR *key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val, bool need_clear)
{
	if(key == nullptr) {
		return 0;
	}
	return loadToStringArray(QString::fromUtf8(key), buffer, _array_size, string_size, begin_val, need_clear);
}

size_t StateUtil_Qt::loadToStringArray(std::string key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val, bool need_clear)
{
	return loadToStringArray(QString::fromStdString(key), buffer, _array_size, string_size, begin_val, need_clear);
}

size_t StateUtil_Qt::loadToStringArray(QString key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val, bool need_clear)
{
	if(key.isEmpty()) {
		return 0;
	}
	if(((void*)buffer) == nullptr) {
		return 0;
	}
	if((_array_size == 0) || (_string_size == 0)) {
		return 0;
	}
	std::shared_ptr<QSettings> set = m_set;
	__UNLIKELY_IF(set.get() == nullptr) {
		return 0;
	}
	size_t begin_ptr = 0;
	size_t count = 0;
	_TCHAR* p = buffer;
	for(int x = 0; x < _array_size; x++) {
		QString key_name = key + QString::fromUtf8("%1").arg(x + begin_val);
		if(need_clear) {
			memset(p, 0x00, _string_size * sizeof(_TCHAR));
		}
		if(set->contains(key_name)) {
			// ToDo: Clear if empty?
			QString tmps = set->value(key_name);
			my_tcscpy_s(p, _string_size, tmps.toLocal8Bit().constData());
		}
		p = &(p[_string_size]);
		count++;
	}
	return count;
}

size_t StateUtil_Qt::saveFromStringArray(_TCHAR* key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val)
{
	if(key == nullptr) {
		return 0;
	}
	return saveFromStringArray(QString::fromUtf8(key), buffer, _array_size, _siring_size);
}

size_t StateUtil_Qt::saveFromStringArray(std::string key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val)
{
	return saveFromStringArray(QString::fromStdString(key), buffer, _array_size, _siring_size);
}

size_t StateUtil_Qt::saveFromStringArray(QString key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val)
{
	if(key.isEmpty()) {
		return 0;
	}
	if(((void*)buffer) == nullptr) {
		return 0;
	}
	if((_array_size == 0) || (_string_size == 0)) {
		return 0;
	}
	std::shared_ptr<QSettings> set = m_set;
	__UNLIKELY_IF(set.get() == nullptr) {
		return 0;
	}
	size_t begin_ptr = 0;
	size_t count = 0;
	_TCHAR* p = buffer;
	for(int x = 0; x < _array_size; x++) {
		QString tmps = QString::fromLocal8Bit(p, _string_size);
		QString key_name = key + QString::fromUtf8("%1").arg(x + begin_val);
		// ToDo: Need check as empty?
		set->setValue(key_name, tmps);
		p = &(p[_string_size]);
		count++;
	}
	return count;
}

size_t StateUtil_Qt::loadToUtf8StringArray(_TCAHR *key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val, bool need_clear)
{
	if(key == nullptr) {
		return 0;
	}
	return loadToUtf8StringArray(QString::fromUtf8(key), buffer, _array_size, string_size, begin_val, need_clear);
}

size_t StateUtil_Qt::loadToUtf8StringArray(std::string key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val, bool need_clear)
{
	return loadToUtf8StringArray(QString::fromStdString(key), buffer, _array_size, string_size, begin_val, need_clear);
}

size_t StateUtil_Qt::loadToUtf8StringArray(QString key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val, bool need_clear)
{
	if(key.isEmpty()) {
		return 0;
	}
	if(((void*)buffer) == nullptr) {
		return 0;
	}
	if((_array_size == 0) || (_string_size == 0)) {
		return 0;
	}
	std::shared_ptr<QSettings> set = m_set;
	__UNLIKELY_IF(set.get() == nullptr) {
		return 0;
	}
	size_t begin_ptr = 0;
	size_t count = 0;
	_TCHAR* p = buffer;
	for(int x = 0; x < _array_size; x++) {
		QString key_name = key + QString::fromUtf8("%1").arg(x + begin_val);
		if(need_clear) {
			memset(p, 0x00, _string_size * sizeof(_TCHAR));
		}
		if(set->contains(key_name)) {
			// ToDo: Clear if empty?
			QString tmps = set->value(key_name);
			my_tcscpy_s(p, _string_size, tmps.toUtf8().constData());
		}
		p = &(p[_string_size]);
		count++;
	}
	return count;
}

size_t StateUtil_Qt::saveFromUtf8StringArray(_TCHAR* key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val)
{
	if(key == nullptr) {
		return 0;
	}
	return saveFromStringArray(QString::fromUtf8(key), buffer, _array_size, _siring_size);
}

size_t StateUtil_Qt::saveFromUtf8StringArray(std::string key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val)
{
	return saveFromStringArray(QString::fromStdString(key), buffer, _array_size, _siring_size);
}

size_t StateUtil_Qt::saveFromUtf8StringArray(QString key, _TCHAR* buffer, size_t _array_size, size_t _string_size, int begin_val)
{
	if(key.isEmpty()) {
		return 0;
	}
	if(((void*)buffer) == nullptr) {
		return 0;
	}
	if((_array_size == 0) || (_string_size == 0)) {
		return 0;
	}
	std::shared_ptr<QSettings> set = m_set;
	__UNLIKELY_IF(set.get() == nullptr) {
		return 0;
	}
	size_t begin_ptr = 0;
	size_t count = 0;
	_TCHAR* p = buffer;
	for(int x = 0; x < _array_size; x++) {
		QString tmps = QString::fromUtf8(p, _string_size);
		QString key_name = key + QString::fromUtf8("%1").arg(x + begin_val);
		// ToDo: Need check as empty?
		set->setValue(key_name, tmps);
		p = &(p[_string_size]);
		count++;
	}
	return count;
}
