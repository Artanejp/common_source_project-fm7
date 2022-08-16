#include <./osd_sound_mod_template.h>


SOUND_OUTPUT_MODULE_BASE::SOUND_OUTPUT_MODULE_BASE(OSD_BASE *parent,
												   SOUND_BUFFER_QT* deviceIO,
												   int base_rate,
												   int base_latency_ms,
												   int base_channels,
												   void *extra_config_values)
	: 
	  m_config_ok(false),
	  m_rate(base_rate),
	  m_latency_ms(base_latency_ms),
	  m_channels(base_channels),
	  m_extconfig(extra_config_values),
	  m_wordsize(sizeof(int16_t)),
	  QObject(qobject_cast<QObject*>parent)
{
	m_device.clear();

	m_logger.reset();
	m_using_flags.reset();
	set_osd(parent);

	if(deviceIO != nullptr) {
		m_fileio.reset(deviceIO);
		m_buffer_bytes = deviceIO->size(); 
		m_chunk_bytes =  m_buffer_bytes / 4;
	} else {
		if(m_channels < 1) m_channels = 1;
		if(m_rate < 1000) m_rate = 1000;
		m_chunk_bytes = ((qint64)(m_channels * m_wordsize * latency_ms) * (quint64)m_rate) / 1000;
		m_buffer_bytes = m_chunk_bytes * 4;
		m_fileio.reset(new SOUND_BUFFER_QT(m_buffer_bytes, this));
	}
	
	m_loglevel = CSP_LOG_INFO;
	m_logdomain = CSP_LOG_TYPE_SOUND;
	m_device_name.clear();
	
	initialize_driver();
}

SOUND_OUTPUT_MODULE_BASE::~SOUND_OUTPUT_MODULE_BASE()
{
	if(m_config_ok.load()) {
		release_driver();
	}
	m_fileio.reset();
	
	m_config_ok = false;
}


void SOUND_OUTPUT_MODULE_BASE::request_to_release()
{
	if(m_config_ok.load()) {
		m_config_ok = !(release_driver());
	}
	emit sig_released(!(m_config_ok.load()));
}

std::shared_ptr<QIODevice> SOUND_OUTPUT_MODULE_BASE::set_io_device(QIODevice *p)
{
	
	bool _f = is_running_sound();
	if(m_fileio.get() != nullptr) {
		_f &= m_fileio->isOpen();
	}
	stop();
	if(p == nullptr) {
		m_fileio.reset(new SOUND_BUFFER_QT(m_chunk_bytes * 4, this));
	} else {
		m_fileio.reset(p);
	}
	update_driver_fileio();
	if(_f) {
		start();
	}
	return m_fileio;
}

std::shared_ptr<QIODevice> SOUND_OUTPUT_MODULE_BASE::set_io_device(std::shared_ptr<QIODevice> ps)
{
	bool _f = is_running_sound();
	if(m_fileio.get() != nullptr) {
		_f &= m_fileio->isOpen();
	}
	stop();
	m_fileio = ps;
	update_driver_fileio();
	if(_f) {
		start();
	}
	return m_fileio;
}


bool SOUND_OUTPUT_MODULE_BASE::update_latency(int latency_ms, bool force)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(latency_ms <= 0) {
		return false;
	}
	if(!(force) && (m_latency_ms == latency_ms)) return true;
	
	m_latency_ms = latency_ms;
	m_chunk_bytes = ((qint64)(m_channels * ((int)m_wordsize) * latency_ms) * (quint64)m_rate) / 1000;
	m_buffer_bytes = m_chunk_bytes * 4;
	
	stop();
	
	std::shared_ptr<SOUND_BUFFER_QT> q = m_fileio;
	if(q.get() != nullptr) {
		q->reset();
		if(!(q->resize(m_buffer_bytes))) {
			q->reset();
			m_buffer_bytes = (int64_t)(q->size());
		}
	} else {
		m_fileio.reset(new SOUND_BUFFER_QT(m_buffer_bytes, this));
	}
	update_driver_fileio();
	return (start() && (m_fileio.get() != nullptr));
}


bool SOUND_OUTPUT_MODULE_BASE::reconfig_sound(int rate, int channels)
{
	// ToDo
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if((rate != m_rate) || (channels != m_channels)) {
		if(real_reconfig_sound(rate, channels)) {
			m_rate = rate;
			m_channels = channels;
			m_config_ok = update_latency(m_latency_ms, true);
			return m_config_ok.load();
		}
	}
	return false;
}

int64_t SOUND_OUTPUT_MODULE_BASE::update_sound(void* datasrc, int samples)
{
	std::shared_ptr<SOUND_BUFFER_QT>q = m_fileio;	

	if(q.get() == nullptr) return -1;
	
	if(samples > 0) {
		qint64 _size = (qint64)(samples * m_channels) * (qint64)m_wordsize;
		return (int64_t)q->write((const char *)datasrc, _size);
	} else if(samples < 0) {
		return (int64_t)q->write((const char *)datasrc, m_chunk_bytes);
	}
	return -1;
}

bool SOUND_OUTPUT_MODULE_BASE::start()
{
	std::shared_ptr<SOUND_BUFFER_QT>q = m_fileio;
	if(is_running_sound()) { // ToDo: STOP
		stop();
	}
	bool _stat = false;
	
	if(q.get() != nullptr) {
		_stat = q->open(QIODeviceBase::Write | QIODeviceBase::Unbuffered);
		update_driver_fileio();
	}	
	if(_stat) {
		QMetaMethod _sig = QMetaMethod::fromSignal(SIGNAL(sig_start_audio()));
		if(isSignalConnected(_sig)) {
			emit sig_start_audio();
		}
	}
	return _stat;
}

bool SOUND_OUTPUT_MODULE_BASE::pause()
{
	QMetaMethod _sig = QMetaMethod::fromSignal(SIGNAL(sig_pause_audio()));
	if(isSignalConnected(_sig)) {
		emit sig_pause_audio();
		return true;
	}
	return false;
}

bool SOUND_OUTPUT_MODULE_BASE::resume()
{
	QMetaMethod _sig = QMetaMethod::fromSignal(SIGNAL(sig_resume_audio()));
	if(isSignalConnected(_sig)) {
		emit sig_resume_audio();
		return true;
	}
	return false;
}

bool SOUND_OUTPUT_MODULE_BASE::stop()
{
	bool _stat = false;
	QMetaMethod _sig = QMetaMethod::fromSignal(SIGNAL(sig_close_audio()));
	if(isSignalConnected(_sig)) {
		emit sig_close_audio();
		_stat = true;
	}
	std::shared_ptr<SOUND_BUFFER_QT>q = m_fileio;

	if(q.get() != nullptr) {
		if(q->isOpen()) {
			q->close();
		}
		return _stat;
	}
	return false;
}

bool SOUND_OUTPUT_MODULE_BASE::discard()
{
//	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
//	std::shared_ptr<SOUND_BUFFER_QT> q = m_fileio;
//	if(q.get() != nullptr) {
		QMetaMethod _sig = QMetaMethod::fromSignal(SIGNAL(sig_discard_audio()));
		if(isSignalConnected(_sig)) {
			emit sig_discard_audio();
			return true;
		}
//	}
	return false;
}

void SOUND_OUTPUT_MODULE_BASE::do_set_device_by_name(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	QString _id = cp->data().value<QString>();
	do_set_device_by_name(_id);
}

void SOUND_OUTPUT_MODULE_BASE::do_set_device_by_number(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int _id = cp->data().value<int>();
	do_set_device_by_number(_id);
}


bool SOUND_OUTPUT_MODULE_BASE::do_send_log(int level, int domain, const _TCHAR* _str, int maxlen)
{
	__UNLIKELY_IF((_str == nullptr) || (maxlen <= 0)) return false;
	__UNLIKELY_IF(strlen(_str) <= 0) return false;
	
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	__LIKELY_IF(isSignalConnected(SIGNAL(sig_send_log(int, int, const _TCHAR*, int)))) {
		emit sig_send_log(level, domain, _str, maxlen);
		return true;
	}
	
	QString s = QString::fromUtf8(_str, maxlen);
	
	__LIKELY_IF(isSignalConnected(SIGNAL(sig_send_log(int, int, QString)))) {
		emit sig_send_log(level, domain, s);
		return true;
	}
	return false;
}

bool SOUND_OUTPUT_MODULE_BASE::do_send_log(int level, int domain, const QString _str)
{
	__UNLIKELY_IF(str.isEmpty()) return false;
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	
	__LIKELY_IF(isSignalConnected(SIGNAL(sig_send_log(int, int, QString)))) {
		emit sig_send_log(level, domain, _str);
		return true;
	}
	return false;
}


void SOUND_OUTPUT_MODULE_BASE::set_logger(const std::shared_ptr<CSP_Logger> logger)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(m_logger.get() != nullptr) {
		disconnect(this, nullptr, m_logger.get(), nullptr);
	}
	m_logger = logger;
	if(m_logger.get() != nullptr) {
		connect(this, SIGNAL(sig_send_log(int, int, QString)),
				m_logger.get(), SLOT(do_send_log(int, int, QString)),
				Qt::QueuedConnection);
	}
}

void SOUND_OUTPUT_MODULE_BASE::set_osd(OSD_BASE* p)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(p != nullptr) {
		m_OSD = p;
		set_logger(p->get_logger());
		set_system_flags(p->get_config_flags());
	} else {
		m_OSD = nullptr;
		if(m_logger.get() != nullptr) {
			disconnect(this, nullptr, m_logger.get(), nullptr);
		}
		m_logger.reset();
		m_using_flags.reset();
	}
}

void SOUND_OUTPUT_MODULE_BASE::set_system_flags(const std::shared_ptr<USING_FLAGS> p)
{
	m_using_flags = p;
	update_config();
}

bool SOUND_OUTPUT_MODULE_BASE::set_extra_config(void* p, int bytes)
{
	if((p == nullptr) || (bytes <= 0)) {
		return false;
	}
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	m_extconfig_ptr   = p;
	m_extconfig_bytes = bytes;
	update_extra_config();
	return true;
}

bool SOUND_OUTPUT_MODULE_BASE::modify_extra_config(void* p, int& bytes)
{
	if((p == nullptr) || (bytes <= 0)) {
		return false;
	}
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	uint8_t* q = (uint8_t*)(m_extconfig_ptr.load());
	if(q == nullptr) {
		return false;
	}
	bytes = std::min(bytes, m_extconfig_bytes.load());
	memcpy(q, p, bytes);
	update_extra_config();
	return true;
}

bool SOUND_OUTPUT_MODULE_BASE::is_io_device_exists()
{
	std::shared_ptr<QIODevice> p = m_fileio;
	if(p.get() != nullptr) {
		return true;
	}
	return false;
}

int64_t SOUND_OUTPUT_MODULE_BASE::get_buffer_bytes()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_buffer_bytes;
}

int64_t SOUND_OUTPUT_MODULE_BASE::get_chunk_bytes()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_chunk_bytes;
}

int SOUND_OUTPUT_MODULE_BASE::get_latency_ms()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_latency_ms;
}
	
int SOUND_OUTPUT_MODULE_BASE::get_channels()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_channels;
}

int SOUND_OUTPUT_MODULE_BASE::get_sample_rate()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_rate;
}

size_t SOUND_OUTPUT_MODULE_BASE::get_word_size()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_wordsize;
}

void SOUND_OUTPUT_MODULE_BASE::get_buffer_parameters(int& channels, int& rate,
													 int& latency_ms, size_t& word_size,
													 int& chunk_bytes, int& buffer_bytes)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	channels = m_channels;
	rate = m_rate;
	latency_ms = m_latency_ms;
	word_size = m_wordsize;
	chunk_bytes = m_chunk_bytes;
	buffer_bytes = m_buffer_bytes;
}

int64_t SOUND_OUTPUT_MODULE_BASE::get_bytes_available()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<SOUND_BUFFER_QT> p = m_fileio;
	if(p.get() != nullptr) {
		return p->bytesAvailable();
	}
	return 0;
}

int64_t SOUND_OUTPUT_MODULE_BASE::get_bytes_left()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<SOUND_BUFFER_QT> p = m_fileio;
	
	if(p.get() != nullptr) {
		int64_t n = m_buffer_bytes - p->bytesAvailable();
		if(n < 0) n = 0;
		return n;
	}
	return 0;
}
