#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include "../../common.h"
#include "../../config.h"

QT_BEGIN_NAMESPACE

class DLL_PREFIX VirtualFilesList : public QObject {
	Q_OBJECT
protected:
	QStringList m_list;
	unsigned int m_length;
	unsigned int m_list_size;
	_TCHAR* m_dstptr;
public:
	VirtualFilesList(_TCHAR* listptr = nullptr,
					 unsigned int pathlen = _MAX_PATH,
					 unsigned int list_size = MAX_HISTORY,
					 QObject *parent = nullptr);
	~VirtualFilesList();
	
	bool setConfigList(_TCHAR* listptr, const unsigned int pathlen, const unsigned int list_size);
	
	QString getFromList(size_t num);
	QStringList getList() const;
	int count();
	ssize_t search(QString s);
	
	size_t updateListFromConfig();
	void resetUiList();
	size_t updateToConfigFromList();
public slots:
	// Maybe Update from UI (or EMU_THREAD)
	void do_replace_list_from_ui();
	void do_select_item_from_ui();
	void do_update_list(QString s);
	void do_update_list_and_ui(QString s);
	// Update from UI
	void do_replace_list(QStringList l);
	void do_update_config_list();
	void do_update_from_config();
	void do_reset_ui_list();
	void do_clear_list();
		
signals:
	// Update TO UI
	int sig_clear_ui_list();
	int sig_set_ui_list(size_t num, QString s);
	int sig_replace_ui_list(QStringList s);
};

class DLL_PREFIX VirtualBanksList : public QObject {
	Q_OBJECT
protected:
	int m_bank_num;
	int m_cur_bank;
	_TCHAR* m_path_pos;
	int m_path_len;
	int m_element_size;
	int m_list_size;
	QStringList m_bank_list;
public:
	VirtualBanksList(int drive = 0,
					 _TCHAR* path = nullptr,
					 unsigned int path_len = _MAX_PATH,
					 unsigned int name_len = 128,
					 unsigned int list_size = 64,
					 QObject* parent = nullptr
		) :
	m_cur_bank(0),
	m_bank_num(0),
	m_list_size(list_size),
	m_element_size(name_len),
	QObject(parent)
	{
		m_bank_list.clear();
	}
	~VirtualBanksList() {}
	int get_bank_num()
	{
		return m_bank_num;
	}
	int get_cur_bank()
	{
		return m_cur_bank;
	}
	void reset(const _TCHAR* path = nullptr,
			  const int pathlen = 0,
			  const _TCHAR **listptr = nullptr,
			  const int bank_num = 0,
			  const int cur_bank = 0);
	void set_cur_bank(int num);
	// Aliases
	int pos()
	{
		return get_cur_bank();
	}
	int size()
	{
		return get_bank_num();
	}
	int element_length()
	{
		return m_element_size;
	}
};

QT_END_NAMESPACE
