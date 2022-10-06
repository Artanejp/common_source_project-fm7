#include <QMetaMethod>
#include <QAction>

#include "../../gui/csp_logger.h"
#include "../../osd_base.h"
#include "../../gui/menu_flags.h"

#include "../sound_buffer_qt.h"
#include "../osd_sound_mod_template.h"

#include "../osd_sound_mod_consts.h"
#include "../osd_sound_mod_utils.h"

namespace SOUND_MODULE {
/* SOUND_MODULE */

	namespace OUTPUT {
	/* SOUND_MODULE::OUTPUT */

	M_BASE::M_BASE(OSD_BASE *parent,
				   SOUND_BUFFER_QT* deviceIO,
				   int base_rate,
				   int base_latency_ms,
				   int base_channels,
				   void *extra_config_values,
				   int extra_config_bytes)
	: 
	  m_config_ok(false),
	  m_rate(base_rate),
	  m_latency_ms(base_latency_ms),
	  m_channels(base_channels),
	  m_extconfig_ptr(extra_config_values),
	  m_extconfig_bytes(extra_config_bytes),
	  m_wordsize(sizeof(int16_t)),
	  m_prev_started(false),
	  m_before_rendered(0),
	  m_samples(0),
	  m_mute(false),
	  m_classname("SOUND_MODULE::OUTPUT::M_BASE"),
	  QObject(qobject_cast<QObject*>(parent))
{

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
		m_chunk_bytes = ((qint64)(m_channels * m_wordsize * m_latency_ms) * (quint64)m_rate) / 1000;
		m_buffer_bytes = m_chunk_bytes * 4;
		m_fileio.reset(new SOUND_BUFFER_QT(m_buffer_bytes, this));
	}
	
	m_loglevel = CSP_LOG_INFO;
	m_logdomain = CSP_LOG_TYPE_SOUND;
	__debug_log_func(_T("Initializing"));
	
	initialize_driver();
}

M_BASE::~M_BASE()
{
	if(m_config_ok.load()) {
		release_driver();
	}
	m_fileio.reset();
	
	m_config_ok = false;
}

bool M_BASE::debug_log_func(const _TCHAR *_funcname, const _TCHAR *_fmt, ...)
{
	_TCHAR buf[768] = {0};
	va_list ap;
	va_start(ap, _fmt);
	int result = vsnprintf(buf, (sizeof(buf) / sizeof(_TCHAR)), _fmt, ap);
	va_end(ap);

	QString _tmps;
	bool _stat = (_funcname != nullptr);
	if(_stat) {
		_stat = strlen(_funcname);
	}
	__LIKELY_IF(_stat) {
		_tmps = QString::fromUtf8("::") + QString::fromUtf8(_funcname) + QString::fromUtf8("(): ");
		_tmps = QString::fromStdString(m_classname) + _tmps + QString::fromUtf8(buf, sizeof(buf));
	} else {
		_tmps = QString::fromUtf8(buf, sizeof(buf));
	}
	return do_send_log(m_loglevel.load(), m_logdomain.load(), _tmps);
}

bool M_BASE::debug_log(const _TCHAR *_fmt, ...)
{
	_TCHAR buf[768] = {0};
	va_list ap;
	va_start(ap, _fmt);
	int result = vsnprintf(buf, (sizeof(buf) / sizeof(_TCHAR)), _fmt, ap);
	va_end(ap);

	QString _tmps;
	_tmps = QString::fromUtf8(buf, sizeof(buf));
	return do_send_log(m_loglevel.load(), m_logdomain.load(), _tmps);
}
	
void M_BASE::request_to_release()
{
	if(m_config_ok.load()) {
		m_config_ok = !(release_driver());
	}
	emit sig_released(!(m_config_ok.load()));
}

__FORMAT M_BASE::get_sound_format()
{
	return __FORMAT::Signed_Int;
}
		
		
bool M_BASE::wait_driver_started(int64_t timeout_msec)
{
	bool _r = m_prev_started.load();
	if(_r) {
		return true;
	}
	bool _infinite = (timeout_msec == INT64_MIN);
	
	while((timeout_msec >= 4) || (_infinite)) {
		if(!(m_prev_started.is_lock_free())) {
			QThread::msleep(4);
		} else {
			_r = m_prev_started.load();
			if(_r) {
				return true;
			}
			QThread::msleep(4);
		}
		timeout_msec -= 4;
	}
	return _r;
}

bool M_BASE::wait_driver_stopped(int64_t timeout_msec)
{
	bool _r = m_prev_started.load();
	if(!(_r)) {
		return true;
	}
	bool _infinite = (timeout_msec == INT64_MIN);
	
	while((timeout_msec >= 4) || (_infinite)) {
		if(!(m_prev_started.is_lock_free())) {
			QThread::msleep(4);
		} else {
			_r = m_prev_started.load();
			if(!(_r)) {
				return true;
			}
			QThread::msleep(4);
		}
		timeout_msec -= 4;
	}
	return !(_r);
}

std::shared_ptr<SOUND_BUFFER_QT> M_BASE::set_io_device(SOUND_BUFFER_QT *p)
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

std::shared_ptr<SOUND_BUFFER_QT> M_BASE::set_io_device(std::shared_ptr<SOUND_BUFFER_QT> ps)
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


bool M_BASE::update_latency(int latency_ms, bool force)
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


bool M_BASE::reconfig_sound(int rate, int channels)
{
	// ToDo
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if((rate != m_rate) || (channels != m_channels)) {
		if(real_reconfig_sound(rate, channels, m_latency_ms)) {
			m_rate = rate;
			m_channels = channels;
			m_config_ok = update_latency(m_latency_ms, true);
			return m_config_ok.load();
		}
	}
	return false;
}

bool M_BASE::release_driver_fileio()
{
	if(m_fileio.get() != nullptr) {
		m_fileio->close();
		disconnect(m_fileio.get(), nullptr, this, nullptr);
		disconnect(this, nullptr, m_fileio.get(), nullptr);
		return true;
	}
		// Maybe disconnect some signals via m_fileio.
	return false;
}

	
bool M_BASE::real_reconfig_sound(int& rate,int& channels,int& latency_ms)
{
	if((rate <= 0) || (channels < 1) || (latency_ms < 10)) {
		return false;
	}
	return true;
}

	
void M_BASE::initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(samples <= 0) return;
	if(rate <= 0) rate = 8000; // OK?
	
	int _latency_ms = (samples * 1000) / rate;
	int channels = m_channels;
	if(real_reconfig_sound(rate, channels, _latency_ms)) {
		m_config_ok = true;
		config_t* p_config = get_config_ptr();
		if(p_config != nullptr) {
			set_volume((int)(p_config->general_sound_level));
		}
		__debug_log_func("Success. Sample rate=%d samples=%d", m_rate, m_samples);
	} else {
		m_config_ok = false;
		__debug_log_func("Failed.");
	}
	if(presented_rate != nullptr) {
		*presented_rate = m_rate;
	}
	if(presented_samples != nullptr) {
		*presented_samples = m_samples;
	}
}
	
void M_BASE::release_sound()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	m_config_ok = false;
	if(m_fileio.get() != nullptr) {
		if(m_fileio->isOpen()) {
			m_fileio->close();
		}
	}
	m_driver_fileio.reset();
	m_fileio.reset();
}

bool M_BASE::check_elapsed_to_render()
{
	const int64_t sound_us_now = driver_elapsed_usec();
	if(m_rate <= 0) return false;
	
	const int64_t  _period_usec = m_latency_ms * 1000;
	int64_t _diff = sound_us_now - m_before_rendered;
	if((_diff < 0) && ((INT64_MAX - m_before_rendered) <= _period_usec))  {
			// For uS overflow
		_diff = sound_us_now + (INT64_MAX - m_before_rendered);
	}
	if(_diff < 0) {
		_diff = 0;
	}
	if(_diff < (_period_usec - 1000)) {
		return false;
	}
	//if(_diff < _period_usec) {
	//	return false;
	//}
	return true;
}

config_t* M_BASE::get_config_ptr()
{
	config_t* _np = nullptr;
	std::shared_ptr<USING_FLAGS> _cp = m_using_flags;
	if(_cp.get() != nullptr) {
		_np = _cp->get_config_ptr();
	}
	return _np;
}

void M_BASE::update_render_point_usec()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	m_before_rendered = driver_elapsed_usec();
}

bool M_BASE::check_enough_to_render()
{
	int64_t _left = get_bytes_left();
	return ((m_chunk_bytes < _left) ? true : false);
}

int64_t M_BASE::update_sound(void* datasrc, int samples)
{
	std::shared_ptr<SOUND_BUFFER_QT>q = m_fileio;	
	//__debug_log_func(_T("SRC=%0llx  samples=%d fileio=%0llx"), (uintptr_t)datasrc, samples, (uintptr_t)(q.get()));
	if(q.get() == nullptr) return -1;
	
	if(samples > 0) {
		qint64 _size = (qint64)(samples * m_channels) * (qint64)m_wordsize;
		return (int64_t)q->write((const char *)datasrc, _size);
	} else if(samples < 0) {
		return (int64_t)q->write((const char *)datasrc, m_chunk_bytes);
	}
	return -1;
}


bool M_BASE::start()
{
	std::shared_ptr<SOUND_BUFFER_QT>q = m_fileio;
	if(is_running_sound()) { // ToDo: STOP
		stop();
		wait_driver_stopped(1000);
	}
	
	bool _stat = false;
	
	if(q.get() != nullptr) {
		#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		_stat = q->open(QIODeviceBase::ReadWrite | QIODeviceBase::Truncate | QIODeviceBase::Unbuffered);
		#else
		_stat = q->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Unbuffered);
		#endif		
		update_driver_fileio();
	}	
	if(_stat) {
		emit sig_start_audio();
		return wait_driver_started(1000);
	}
	return _stat;
}

bool M_BASE::pause()
{
	emit sig_pause_audio();
	return true;
}

bool M_BASE::resume()
{
	emit sig_resume_audio();
	return true;
}

bool M_BASE::stop()
{
	bool _stat = false;
	emit sig_stop_audio();
	_stat = wait_driver_stopped(1000);
	
	std::shared_ptr<SOUND_BUFFER_QT>q = m_fileio;
	if(q.get() != nullptr) {
		if(q->isOpen()) {
			q->close();
		}
		return _stat;
	}
	return false;
}

bool M_BASE::discard()
{
	emit sig_discard_audio();
	return true;
}

void M_BASE::set_volume(double level)
{
	emit sig_set_volume(level);
}

void M_BASE::set_volume(int level)
{

	level = std::min(std::max(level, (int)INT16_MIN), (int)INT16_MAX);
	double xlevel = ((double)(level + INT16_MAX)) / ((double)UINT16_MAX);
	emit sig_set_volume(xlevel);
}
	
void M_BASE::mute_sound()
{
}

void M_BASE::stop_sound()
{
}
	
void M_BASE::do_set_device_by_name(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	QString _id = cp->data().value<QString>();
	do_set_device_by_name(_id);
}

void M_BASE::do_set_device_by_number(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int _id = cp->data().value<int>();
	do_set_device_by_number(_id);
}


bool M_BASE::do_send_log(int level, int domain, const _TCHAR* _str, int maxlen)
{
	__UNLIKELY_IF((_str == nullptr) || (maxlen <= 0)) return false;
	__UNLIKELY_IF(strlen(_str) <= 0) return false;
	
	QString s = QString::fromUtf8(_str, maxlen);
	emit sig_send_log(level, domain, s);
	return true;
}

bool M_BASE::do_send_log(int level, int domain, const QString _str)
{
	__UNLIKELY_IF(_str.isEmpty()) return false;
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	
	emit sig_send_log(level, domain, _str);
	return true;
}


void M_BASE::set_logger(const std::shared_ptr<CSP_Logger> logger)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(m_logger.get() != nullptr) {
		disconnect(this, nullptr, m_logger.get(), nullptr);
	}
	m_logger = logger;
	if(m_logger.get() != nullptr) {
		connect(this, SIGNAL(sig_send_log(int, int, QString)),
				m_logger.get(), SLOT(do_debug_log(int, int, QString))
				,Qt::QueuedConnection);
	}
}

void M_BASE::set_osd(OSD_BASE* p)
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

void M_BASE::set_system_flags(const std::shared_ptr<USING_FLAGS> p)
{
	m_using_flags = p;
	update_config();
}

bool M_BASE::set_extra_config(void* p, int bytes)
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

bool M_BASE::modify_extra_config(void* p, int& bytes)
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

bool M_BASE::is_io_device_exists()
{
	std::shared_ptr<SOUND_BUFFER_QT> p = m_fileio;
	if(p.get() != nullptr) {
		return true;
	}
	return false;
}

int64_t M_BASE::get_buffer_bytes()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_buffer_bytes;
}

int64_t M_BASE::get_chunk_bytes()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_chunk_bytes;
}

int M_BASE::get_latency_ms()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_latency_ms;
}
	
int M_BASE::get_channels()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_channels;
}

int M_BASE::get_sample_rate()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_rate;
}

size_t M_BASE::get_word_size()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	return m_wordsize;
}

void M_BASE::get_buffer_parameters(int& channels, int& rate,
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

int64_t M_BASE::get_bytes_available()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<SOUND_BUFFER_QT> p = m_fileio;
	if(p.get() != nullptr) {
		return p->bytesAvailable();
	}
	return 0;
}

int64_t M_BASE::get_bytes_left()
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

	/* SOUND_MODULE::OUTPUT */
	}
	/* SOUND_MODULE */
}
