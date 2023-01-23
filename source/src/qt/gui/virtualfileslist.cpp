#include <QAction>
#include "./virtualfileslist.h"

VirtualFilesList::VirtualFilesList(_TCHAR* listptr,
								   unsigned int pathlen,
								   unsigned int list_size,
								   QObject *parent) :
		m_dstptr(listptr),
		m_length(pathlen),
		m_list_size(list_size),
		QObject(parent)
{
	m_list.clear();
	updateListFromConfig();
}

VirtualFilesList::~VirtualFilesList()
{
}

bool VirtualFilesList::setConfigList(_TCHAR* listptr, const unsigned int pathlen, const unsigned int list_size)
{
	if((listptr == nullptr) || (pathlen == 0) || (list_size == 0)) {
		return false;
	}
	m_length = pathlen;
	m_list_size = list_size;
	m_dstptr = listptr;
	if(updateListFromConfig() > 0) {
		return true;
	}
	return false;
}

QString VirtualFilesList::getFromList(size_t num)
{
	if((m_list_size == 0) || (num == 0)) return QString("");
	if((m_list_size <= num) || (m_list.size() <= num)) return QString("");
	return m_list.at(num);
}

QStringList VirtualFilesList::getList() const
{
	QStringList l(m_list);
	return l;
}
int VirtualFilesList::count()
{
	return m_list.count();
}
ssize_t VirtualFilesList::search(QString s)
{
	return (ssize_t)(m_list.indexOf(s));
}
	

size_t VirtualFilesList::updateListFromConfig()
{
	if(m_dstptr == nullptr) return 0;
	if(m_length == 0)       return 0;
	if(m_list_size == 0)    return 0;
	_TCHAR* p = m_dstptr;
	_TCHAR* tmps = new _TCHAR[m_length];
	size_t ncount = 0;

	for(unsigned int i = 0; i < m_list_size; i++) {
		// For security
		memset(tmps, 0x00, m_length);
		my_strcpy_s(tmps, m_length - 1, p);
		if(strlen(tmps) > 0) {
			QString s = QString::fromLocal8Bit(tmps);
			m_list.append(s);
			ncount++;
		}
		p = &(p[m_length]);
	}
	delete [] tmps;
	return m_list.count();
}
void VirtualFilesList::resetUiList()
{
	if(m_list.size() == 0) return;
	emit sig_clear_ui_list();
	size_t ncount = 0;
	for(auto l = m_list.begin(); l != m_list.end(); ++l) {
		if(!((*l).isEmpty())) {
			emit sig_set_ui_list(ncount, (*l));
			ncount++;
		}
		if(ncount >= m_list_size) break;
	}
}

size_t VirtualFilesList::updateToConfigFromList()
{
	if(m_dstptr == nullptr) return 0;
	if(m_length == 0)       return 0;
	if(m_list_size == 0)    return 0;
	_TCHAR* p = m_dstptr;
	size_t ncount = 0;
	for(auto i = m_list.begin(); i != m_list.end(); ++i) {
		QString s = *i;
		if(!(s.isEmpty())) {
			memset(p, 0x00, m_length);
			my_strcpy_s(p, m_length - 1, s.toLocal8Bit().constData());
			p = &(p[m_length]);
			ncount++;
		}
		if(ncount >= m_list_size) break;
	}
	return ncount;
}

// SLOTS
void VirtualFilesList::do_replace_list_from_ui()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	QStringList l = cp->data().value<QStringList>();
	
	do_replace_list(l);
}
	
void VirtualFilesList::do_select_item_from_ui()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	QString s = cp->data().value<QString>();
	
	do_update_list_and_ui(s);
}
void VirtualFilesList::do_update_list(QString s)
{
	if(s.isEmpty()) return; // Noop
	ssize_t npos = search(s);
	if((npos >= 0) && (npos < m_list.size())) {
		m_list.removeAt(npos);
	}
	m_list.push_front(s);
	if(m_list.size() > m_list_size) {
		size_t __last = m_list.size();
		for(size_t i = m_list_size; i < __last; i++) {
			m_list.removeAt(i);
		}
	}
	updateToConfigFromList();
}

void VirtualFilesList::do_update_list_and_ui(QString s)
{
	do_update_list(s);
	resetUiList();
}
		
// Update from UI
void VirtualFilesList::do_replace_list(QStringList l)
{
	m_list.clear();
	size_t ncount = 0;
	for(auto p = l.begin(); p != l.end(); ++p) {
		if(!((*p).isEmpty())) {
			m_list.append(*p);
			ncount++;
		}
		if(ncount >= m_list_size) break;
	}
	if(ncount > 0) {
		updateToConfigFromList();
	}
}
void VirtualFilesList::do_update_config_list()
{
	updateToConfigFromList();
}
void VirtualFilesList::do_update_from_config()
{
	updateListFromConfig();
	resetUiList();
}
void VirtualFilesList::do_reset_ui_list()
{
	resetUiList();
}
void VirtualFilesList::do_clear_list()
{
	m_list.clear();
	resetUiList();
}
		
