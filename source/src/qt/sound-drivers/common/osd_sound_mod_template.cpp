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
	  m_external_fileio(false),
	  m_classname("SOUND_MODULE::OUTPUT::M_BASE"),
	  QObject(qobject_cast<QObject*>(parent))
{

	m_logger.reset();
	m_using_flags.reset();
	m_fileio.reset();
	set_osd(parent);

	if(m_channels.load() <= 1) m_channels = 2;
	recalc_samples(m_rate.load(), m_latency_ms.load(), true, false);
	// Belows are specified buffer function.
#if 1
	bool _reinit = (deviceIO == nullptr) ? true : false;
	if(!(_reinit)) {
		if(deviceIO->isOpen()) {
			deviceIO->close();
		}
		if(deviceIO->resize(m_buffer_bytes.load())) {
			m_external_fileio = true;
			m_fileio.reset(deviceIO);
		} else {
			_reinit = true;
		}
	}
	if((_reinit) && !(m_external_fileio.load())) {
		m_fileio.reset(new SOUND_BUFFER_QT(m_buffer_bytes.load(), this));
		reopen_fileio(true);
	}
	update_driver_fileio();
#endif
	m_loglevel = CSP_LOG_INFO;
	m_logdomain = CSP_LOG_TYPE_SOUND;
	__debug_log_func(_T("Initializing"));

	//initialize_driver(parent);
}

M_BASE::~M_BASE()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
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

bool M_BASE::recalc_samples(int rate, int latency_ms, bool need_update, bool need_resize_fileio)
{
	if(rate < 1000) rate = 1000;
	if(latency_ms < 1) latency_ms = 1;
	int64_t _samples =
		((int64_t)rate * latency_ms) / 1000;
	size_t _chunk_bytes = (size_t)(_samples * m_wordsize.load() * m_channels.load());
	int64_t _buffer_bytes = _chunk_bytes * 4;

	bool _need_restart = false;
	if(need_resize_fileio) {
		bool __reinit = true;
		if(m_fileio.get()  != nullptr) {
			std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;
			if(_buffer_bytes != m_buffer_bytes.load()) {
				bool _is_opened = fio->isOpen();
				if(_is_opened) {
					fio->close();
				}
				__reinit = !(fio->resize(_buffer_bytes));
				update_driver_fileio();
				_need_restart = true;
			} else {
				__reinit = false;
			}
		}
		if(__reinit) {
			std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
			if(!(m_external_fileio.load())) {
				m_fileio.reset(new SOUND_BUFFER_QT(_buffer_bytes, this));
			}
			reopen_fileio(true);
			update_driver_fileio();
			_need_restart = true;
		}
	}

	if((need_update) || (m_chunk_bytes.load() != _chunk_bytes) || (m_buffer_bytes.load() != _buffer_bytes) || (m_samples.load() != _samples)){
		m_chunk_bytes  = _chunk_bytes;
		m_buffer_bytes = _buffer_bytes;
		m_samples = _samples;
	}
	return _need_restart;
}

bool M_BASE::reopen_fileio(bool force_reopen)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	SOUND_BUFFER_QT* fio = m_fileio.get();

	if(fio != nullptr) {
		if(force_reopen) {
			if(fio->isOpen()) {
				fio->close();
			}
		}
		fio->reset();
		if(!(fio->isOpen())) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			fio->open(QIODeviceBase::ReadWrite | QIODeviceBase::Truncate );
#else
			fio->open(QIODevice::ReadWrite | QIODevice::Truncate);
#endif
		}
	} else {
		return false;
	}
	return fio->isOpen();
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


bool M_BASE::update_latency(int latency_ms, bool force)
{
	if(latency_ms <= 0) {
		return false;
	}
	if(!(force) && (m_latency_ms.load() == latency_ms)) return true;

	stop();
	recalc_samples(m_rate, latency_ms, true, true);

	std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;
	if(fio.get() != nullptr) {
		fio->reset();
	}
//	return (start() && (m_fileio != nullptr));
	return (start());
}


bool M_BASE::reconfig_sound(int rate, int channels)
{
	// ToDo
	if((rate != m_rate.load()) || (channels != m_channels.load())) {
		int _latency = m_latency_ms.load();
		bool _b = real_reconfig_sound(rate, channels, _latency);
		m_latency_ms = _latency;
		if(_b) {
			m_rate = rate;
			m_channels = channels;
			m_config_ok = update_latency(m_latency_ms.load(), true);
			return m_config_ok.load();
		}
	}
	return false;
}

bool M_BASE::release_driver_fileio()
{
	if(m_fileio.get() != nullptr) {
		std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;
		std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
		fio->close();
		disconnect(fio.get(), nullptr, this, nullptr);
		disconnect(this, nullptr, fio.get(), nullptr);
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
	int channels = m_channels.load();
	if(real_reconfig_sound(rate, channels, _latency_ms)) {
		m_config_ok = true;
		config_t* p_config = get_config_ptr();
		if(p_config != nullptr) {
			set_volume((int)(p_config->general_sound_level));
		}
		__debug_log_func("Success. Sample rate=%d samples=%d", m_rate.load(), m_samples.load());
	} else {
		m_config_ok = false;
		__debug_log_func("Failed.");
	}
	if(presented_rate != nullptr) {
		*presented_rate = m_rate.load();
	}
	if(presented_samples != nullptr) {
		*presented_samples = m_samples.load();
	}
}

void M_BASE::release_sound()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	m_config_ok = false;
	if(m_fileio.get() != nullptr) {
		std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;
		if(fio->isOpen()) {
			fio->close();
		}
	}
	if(!(m_external_fileio.load())) {
		m_fileio.reset();
	}
//	emit sig_sound_finished();
}

bool M_BASE::check_elapsed_to_render()
{
	//const int64_t sound_us_now = driver_processed_usec();
	if(m_rate.load() <= 0) return false;
	if(!(is_driver_started())) {
		return false;
	}
	const int64_t sound_us_now = driver_elapsed_usec();
	const int64_t  _period_usec = m_latency_ms.load() * 1000;
	int64_t _diff = sound_us_now - m_before_rendered.load();
	if((_diff < 0) && ((INT64_MAX - m_before_rendered.load()) <= _period_usec))  {
			// For uS overflow
		_diff = sound_us_now + (INT64_MAX - m_before_rendered.load());
	}
	if(_diff < 0) {
		_diff = 0;
	}
	if(_diff < (_period_usec - 2000)) {
		return false;
	}
//	if(_diff < _period_usec) {
//		return false;
//	}
	return true;
}

bool M_BASE::is_driver_started()
{
	bool _b = m_config_ok.load();
	_b &= m_prev_started.load();
	return _b;
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
	m_before_rendered = driver_elapsed_usec();
	//m_before_rendered = driver_processed_usec();
}

bool M_BASE::check_enough_to_render()
{
	int64_t _left = get_bytes_left();
	return ((m_chunk_bytes.load() < _left) ? true : false);
}

int64_t M_BASE::update_sound(void* datasrc, int samples)
{
	std::shared_ptr<SOUND_BUFFER_QT> q = m_fileio;
	//__debug_log_func(_T("SRC=%0llx  samples=%d fileio=%0llx"), (uintptr_t)datasrc, samples, (uintptr_t)(q.get()));
	if(q.get() == nullptr) return -1;
	if(datasrc == nullptr) return -1;

	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(!(is_driver_started()) ||  !(q->isOpen())) {
		return -1;
	}
	int64_t _result = -1;
	qint64 _size = m_chunk_bytes.load();
	if(samples > 0) {
		_size = (qint64)samples * (qint64)(m_channels.load() * m_wordsize.load());
	} else if(samples == 0) {
		return _result;
	}

	if(_size > 0) {
		_result = (int64_t)q->write((const char *)datasrc, _size);
	}
	if(_result > 0) {
		_result = _result / (qint64)(m_channels.load() * m_wordsize.load());

	}
	return _result;
}


bool M_BASE::start()
{
//	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
//	QIODevice *q = m_fileio;
//	if(is_running_sound()) { // ToDo: STOP
//		stop();
//		wait_driver_stopped(1000);
//	}

	bool _stat = reopen_fileio(false);
	update_driver_fileio();

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
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<SOUND_BUFFER_QT> q = m_fileio;
	if(q.get() != nullptr) {
		if(q->isOpen()) {
			q->close();
		}
	}
	return _stat;
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
	std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;
	if(fio.get() != nullptr) {
		fio->reset();
	}
}

void M_BASE::unmute_sound()
{
}

void M_BASE::stop_sound()
{
	std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;
	if(fio.get() != nullptr) {
		fio->reset();
	}
}

void M_BASE::do_set_output_by_name(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	QString _id = cp->data().value<QString>();
	do_set_output_by_name(_id);
}

void M_BASE::do_set_input_by_name(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	QString _id = cp->data().value<QString>();
	do_set_input_by_name(_id);
}

void M_BASE::do_set_output_by_number(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int _id = cp->data().value<int>();
	do_set_output_by_number(_id);
}

void M_BASE::do_set_input_by_number(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int _id = cp->data().value<int>();
	do_set_input_by_number(_id);
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
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(m_fileio.get() != nullptr) {
		return true;
	}
	return false;
}

int64_t M_BASE::get_buffer_bytes()
{
	return m_buffer_bytes.load();
}

int64_t M_BASE::get_chunk_bytes()
{
	return m_chunk_bytes.load();
}

int M_BASE::get_latency_ms()
{
	return m_latency_ms.load();
}

int M_BASE::get_channels()
{
	return m_channels.load();
}

int M_BASE::get_sample_rate()
{
	return m_rate.load();
}

size_t M_BASE::get_word_size()
{
	return m_wordsize.load();
}

void M_BASE::get_buffer_parameters(int& channels, int& rate,
													 int& latency_ms, size_t& word_size,
													 int& chunk_bytes, int& buffer_bytes)
{
	channels = m_channels.load();
	rate = m_rate.load();
	latency_ms = m_latency_ms.load();
	word_size = m_wordsize.load();
	chunk_bytes = m_chunk_bytes.load();
	buffer_bytes = m_buffer_bytes.load();
}

int64_t M_BASE::get_bytes_available()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<SOUND_BUFFER_QT> q = m_fileio;
	if(q.get() != nullptr) {
		return q->bytesAvailable();
	}
	return 0;
}

int64_t M_BASE::get_bytes_left()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<SOUND_BUFFER_QT> q = m_fileio;
	if(q.get() != nullptr) {
		int64_t n =  q->bytesToWrite() - q->bytesAvailable();
		if(n < 0) n = 0;
		return n;
	}
	return 0;
}
	/* SOUND_MODULE::OUTPUT */
	}
	/* SOUND_MODULE */
}
