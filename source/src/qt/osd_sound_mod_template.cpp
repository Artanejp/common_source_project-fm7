#include <./osd_sound_mod_template.h>


SOUND_OUTPUT_MODULE_BASE::SOUND_OUTPUT_MODULE_BASE(OSD_BASE *parent,
												   _TCHAR* device_name,
												   int base_rate,
												   int base_latency_ms,
												   int base_channels,
												   void *extra_config_values)
	: m_OSD(parent),
	  m_config_ok(false),
	  m_rate(base_rate),
	  m_latency_ms(base_latency_ms),
	  m_channels(base_channels),
	  m_extconfig(extra_config_values),
	  QObject(qobject_cast<QObject*>parent)
{
	m_device.clear();
	if(m_OSD != nullptr) {
		m_logger = m_OSD->get_logger();
		m_using_flags = m_OSD->get_config_flags();
	}
	if(m_logger.get() != nullptr) {
		QObject::connect(this, SIGNAL(sig_send_log(int, int, QString)),
						 m_logger.get(), SLOT(do_debug_log(int, int, QString)),
						 Qt::QueueedConnection);
	}
	m_loglevel = CSP_LOG_INFO;
	m_logdomain = CSP_LOG_TYPE_SOUND;
	
	if(device_name == nullptr) {
		m_device_name = std::string(_T("Default"));
	} else {
		QString tmpname = QString::fromUtf8(device_name);
		m_device_name = tmpname.toStdString();
	}
	if(initialize_driver()) {
		m_device_name = set_device_sound((const _TCHAR*)m_device_name.c_str(), m_rate, m_channels, m_latency_ms);
	}
}

SOUND_OUTPUT_MODULE_BASE::~SOUND_OUTPUT_MODULE_BASE()
{
	release_driver();
}

bool SOUND_OUTPUT_MODULE_BASE::do_send_log(imt level, int domain, const _TCHAR* str, int maxlen)
{
	__UNLIKELY_IF((str == nullptr) || (maxlen <= 0)) return false;
	__UNLIKELY_IF(strlen(str) <= 0) return false;
	QString s = QString::fromUtf8(buf);
	emit sig_send_log(level, domain, s);
	return true;
}

void SOUND_OUTPUT_MODULE_BASE::set_logger(const std::shared_ptr<CSP_Logger> logger)
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	if(m_logger.get() != nullptr) {
		QObject::disconnect(this, SIGNAL(sig_send_log(int, int, QString)),
							m_logger.get(), SLOT(do_debug_log(int, int, QString)));
	}
	m_logger = logger;
	if(m_logger.get() != nullptr) {
		QObject::connect(this, SIGNAL(sig_send_log(int, int, QString)),
						 m_logger.get(), SLOT(do_debug_log(int, int, QString)),
						 Qt::QueueedConnection);
	}
}
void SOUND_OUTPUT_MODULE_BASE::set_osd(OSD_BASE* p)
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	m_OSD = p;
}

void SOUND_OUTPUT_MODULE_BASE::set_system_flags(const std::shared_ptr<USING_FLAGS> p)
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	m_using_flags = p;
}

void SOUND_OUTPUT_MODULE_BASE::update_extra_config(void* p)
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	m_extconfig = p;
	// more lock via m_locker_outqueue etc, if needs.
}

