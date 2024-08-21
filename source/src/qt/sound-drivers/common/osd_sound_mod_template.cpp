#include <QMetaMethod>
#include <QAction>

#include "../../gui/csp_logger.h"
#include "../../osd_base.h"
#include "../../gui/menu_flags.h"

#include "../sound_buffer_qt.h"
#include "../osd_sound_mod_template.h"

#include "../osd_sound_mod_consts.h"
#include "../osd_sound_mod_utils.h"
#incluse <cmath>

namespace SOUND_MODULE {
/* SOUND_MODULE */

	M_BASE::M_BASE(OSD_BASE *parent,
				   QIODevice* sinkDeviceIO,
				   QIODevice* sourceDeviceIO,
				   int base_rate,
				   int base_latency_ms,
				   int base_channels,
				   void *extra_config_values,
				   int extra_config_bytes)
	:
	  m_config_ok(false),
	  m_sink_rate(base_rate),
	  m_sink_latency_ms(base_latency_ms),
	  m_sink_channels(base_channels),
	  m_sink_samples(0),
	  m_sink_wordsize(sizeof(int16_t)),
	  m_sink_volume(std::nan("1")),
	  
	  m_source_rate(base_rate),
	  m_source_latency_ms(base_latency_ms),
	  m_source_channels(base_channels),
	  m_source_samples(0),
	  m_source_wordsize(sizeof(int16_t)),
	  m_source_volume(std::nan("1")),
	  
	  m_extconfig_ptr(extra_config_values),
	  m_extconfig_bytes(extra_config_bytes),
	  
	  m_prev_sink_started(false),
	  m_prev_source_started(false),

	  m_before_rendered(0),
	  m_mute(false),
	  m_sink_external_fileio(false),
	  m_source_external_fileio(false),
	  m_classname("SOUND_MODULE::M_BASE")/*,
	   QObject(qobject_cast<QObject*>(parent))*/
{

	m_logger.reset();
	m_using_flags.reset();
	m_sink_fileio.reset();
	m_source_fileio.reset();
	set_osd(parent);

	if(m_channels.load() <= 1) m_channels = 2;
	recalc_samples(m_rate.load(), m_latency_ms.load(), true);
	// Belows are specified buffer function.

	bool sink_reinit = (sinkDeviceIO == nullptr) ? true : false;
	if(!(sink_reinit)) {
		if(sinkDeviceIO->isOpen()) {
			sinkDeviceIO->close();
		}
		m_sink_external_fileio = true;
		m_sink_fileio.reset(sinkDeviceIO);
	}
//	if((sink_reinit) && !(m_sink_external_fileio.load())) {
//		m_sink_fileio.reset(new SOUND_BUFFER_QT(m_sink_buffer_bytes.load(), this));
//		reopen_sink_fileio(true);
//	}
	update_sink_driver_fileio();
	
#if 0 // Temporally unavaiable.
	bool source_reinit = (sourceDeviceIO == nullptr) ? true : false;
	if(!(source_reinit)) {
		if(sourceDeviceIO->isOpen()) {
			sourceDeviceIO->close();
		}
		if(sourceDeviceIO->resize(m_source_buffer_bytes.load())) {
			m_source_external_fileio = true;
			m_source_fileio.reset(sourceDeviceIO);
		} else {
			sink_reinit = true;
		}
	}
	if((source_reinit) && !(m_source_external_fileio.load())) {
		m_source_fileio.reset(new SOUND_BUFFER_QT(m_source_buffer_bytes.load(), this));
		reopen_source_fileio(true);
	}
#endif	
	update_source_driver_fileio();

	m_loglevel = CSP_LOG_INFO;
	m_logdomain = CSP_LOG_TYPE_SOUND;
	__debug_log_func(_T("Initializing"));
	//initialize_driver(parent);
}

M_BASE::~M_BASE()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	release_sound();
	m_sink_fileio.reset(); // Force to clear FILEIO, even using external this.
	m_source_fileio.reset(); // Force to clear FILEIO, even using external this.
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

bool M_BASE::recalc_samples(int rate, int latency_ms, bool force)
{
	if(rate < 1000) rate = 1000;
	if(latency_ms < 1) latency_ms = 1;
	int64_t _samples =
		((int64_t)rate * latency_ms) / 1000;
	size_t _chunk_bytes = (size_t)(_samples * m_sink_wordsize.load() * m_sink_channels.load());
	int64_t _buffer_bytes = _chunk_bytes * 4;

	if((m_rate.load() == rate) && (m_latency_ms.load() == latency_ms) && !(force)) {
		return false;
	}
	
	m_sink_chunk_bytes  = _chunk_bytes;
	m_sink_buffer_bytes = _buffer_bytes;
	m_sink_samples = _samples;
	return true;
}

bool M_BASE::reopen_sink_fileio(bool force_reopen)
{
	std::shared_ptr<QIODevice> fio = m_sink_fileio;

	if(fio.get() != nullptr) {
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

bool M_BASE::reopen_source_fileio(bool force_reopen)
{
	std::shared_ptr<QIODevice> fio = m_source_fileio;

	if(fio.get() != nullptr) {
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

void M_BASE::do_about_to_quit()
{
	release_sound();
	emit sig_released(!(m_config_ok.load()));
}

__FORMAT M_BASE::get_sink_sound_format()
{
	return __FORMAT::Signed_Int;
}

__FORMAT M_BASE::get_source_sound_format()
{
	return __FORMAT::Signed_Int;
}






bool M_BASE::update_latency(int latency_ms, bool force)
{
	if(latency_ms <= 0) {
		return false;
	}
	if(!(force) && (m_latency_ms.load() == latency_ms)) return true;

	stop_sink();
	recalc_samples(m_rate.load(), latency_ms, true);

	if(m_sink_external_fileio.load()) {
		std::shared_ptr<QIODevice> fio = m_sink_fileio;
		if(fio.get() != nullptr) {
			fio->reset();
		}
	}
//	return (start() && (m_fileio != nullptr));
	return (start_sink());
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
	bool result_sink = false;
	if(m_sink_external_fileio.load()) {
		std::shared_ptr<QIODevice> fio_sink = m_sink_fileio;
		if(fio_sink.get() != nullptr) {
			fio_sink->close();
			disconnect(fio_sink.get(), nullptr, this, nullptr);
			disconnect(this, nullptr, fio_sink.get(), nullptr);
			result_sink = true;
		}
	} else {
		result_sink = true;
	}
	bool result_source = false;
	if(m_source_external_fileio.load()) {
		std::shared_ptr<QIODevice> fio_source = m_source_fileio;
		if(fio_source.get() != nullptr) {
			fio_source->close();
			disconnect(fio_source.get(), nullptr, this, nullptr);
			disconnect(this, nullptr, fio_source.get(), nullptr);
			result_source = true;
		}
	} else {
		result_source = true;
	}

	return ((result_source) || (result_sink));
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
			set_sink_volume((int)(p_config->general_sound_level));
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

void M_BASE::release_source()
{
	// Write driver's own sequence.
}

void M_BASE::release_sink()
{
	// Write driver's own sequence.
}
		
void M_BASE::release_sound()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker); // Lockout file I/O.
	release_source();
	release_sink();
	
	bool _r = release_driver_fileio();
	
	if(!(m_sink_external_fileio.load())) {
		m_sink_fileio.reset();
	}
	if(!(m_source_external_fileio.load())) {
		m_source_fileio.reset();
	}
	m_config_ok = false;
}

bool M_BASE::is_output_driver_started()
{
	bool _b = m_config_ok.load();
	_b &= m_prev_sink_started.load();
	return _b;
}

bool M_BASE::is_capture_driver_started()
{
	bool _b = m_config_ok.load();
	_b &= m_prev_source_started.load();
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


bool M_BASE::check_enough_to_render()
{
	int64_t _left = get_bytes_left();
	return ((m_chunk_bytes.load() < _left) ? true : false);
}

// Below APIs can call directry from OSD:: .
int64_t M_BASE::update_sound(void* datasrc, int samples)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<QIODevice> q = m_sink_fileio;
	//__debug_log_func(_T("SRC=%0llx  samples=%d fileio=%0llx"), (uintptr_t)datasrc, samples, (uintptr_t)(q.get()));
	__UNLIKELY_IF(q.get() == nullptr) return -1;
	__UNLIKELY_IF(datasrc == nullptr) return -1;
	__UNLIKELY_IF(samples < 0) return -1;
	qint64 bytes_per_sample = (qint64)(m_sink_channels.load() * m_sink_wordsize.load());
	__UNLIKELY_IF(bytes_per_sample <= 0) {
		retrun -1;
	}
	if(!(is_driver_started()) ||  !(q->isOpen())) {
		return -1;
	}
	int64_t _result = -1;
	qint64 _size = m_sink_chunk_bytes.load();
	__LIKELY_IF(samples > 0) {
		_size = (qint64)samples * bytes_per_sample;
	}
	__LIKELY_IF(_size > 0) {
		_result = (int64_t)q->write((const char *)datasrc, _size);
	}
	__LIKELY_IF(_result >= (int64_t)bytes_per_sample) {
		_result = _result / (int64_t)bytes_per_sample;
	} else {
		_result = -1;
	}
	return _result;
}


bool M_BASE::start_sink()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(m_sink_external_fileio.load()) {
		_stat = reopen_sink_fileio(false);
		if(_stat) {
			update_sink_driver_fileio();
		}
	}
	emit sig_start_sink();

	return true;
}


bool M_BASE::stop_sink()
{
	emit sig_stop_sink();
	if(m_sink_external_fileio.load()) {
		std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
		std::shared_ptr<QIODevice> q = m_sink_fileio;
		if(q.get() != nullptr) {
			if(q->isOpen()) {
				q->close();
			}
		}
	}
	m_sink_volume = std::nan("1");
	return true;
}

bool M_BASE::discard_sink()
{
	if(m_sink_external_fileio.load()) {
		std::shared_ptr<QIODevice> fio = m_sink_fileio;
		if(fio.get() != nullptr) {
			fio->reset();
		}
	}
	emit sig_discard_sink();
	return true;
}

void M_BASE::set_sink_volume(double level)
{
	double xlevel = m_sink_volume.load();
	if((std::isNan(xlevel)) || (level != xlevel)) {
		emit sig_set_sink_volume(level);
	}
}

void M_BASE::set_sink_volume(int level)
{
	level = std::min(std::max(level, (int)INT16_MIN), (int)INT16_MAX);
	double xlevel = ((double)(level + INT16_MAX)) / ((double)UINT16_MAX);
	set_sink_volume(xlevel);
}

void M_BASE::mute_sink()
{
	if(m_sink_external_fileio.load()) {
		std::shared_ptr<QIODevice> fio = m_sink_fileio;
		if(fio.get() != nullptr) {
			fio->reset();
		}
	}
	emit sig_mute_sink();
}

void M_BASE::unmute_sink()
{
	/*
	if(m_sink_external_fileio.load()) {
		// ToDo.
	}
	*/
	emit sig_unmute_sink();
}

// END OF: Below APIs can call directry from OSD:: .

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
		connect(this, SIGNAL(sig_sink_started()), p, SLOT(do_sink_started()), Qt::DirectConnection);
		connect(this, SIGNAL(sig_sink_stopped()), p, SLOT(do_sink_stopped()), Qt::DirectConnection);
		connect(this, SIGNAL(sig_sink_empty()), p, SLOT(do_sink_empty()), Qt::DirectConnection);
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

bool M_BASE::is_sink_io_device_exists()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(m_sink_fileio.get() != nullptr) {
		return true;
	}
	return false;
}

bool M_BASE::is_source_io_device_exists()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(m_source_fileio.get() != nullptr) {
		return true;
	}
	return false;
}


void M_BASE::get_sink_parameters(int& channels, int& rate,
													 int& latency_ms, size_t& word_size,
													 int& chunk_bytes, int& buffer_bytes)
{
	channels = m_sink_channels.load();
	rate = m_sink_rate.load();
	latency_ms = m_sink_latency_ms.load();
	word_size = m_sink_wordsize.load();
	chunk_bytes = m_sink_chunk_bytes.load();
	buffer_bytes = m_sink_buffer_bytes.load();
}

int64_t M_BASE::get_sink_bytes_size()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<QIODevice> q = m_sink_fileio;
	if(q.get() != nullptr) {
		int64_t n =  (int64_t)(q->size());
		if(n < 0) n = 0;
		return n;
	}
	return 0;
}
	
int64_t M_BASE::get_sink_bytes_left()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<QIODevice> q = m_sink_fileio;
	if(q.get() != nullptr) {
		int64_t n =  (int64_t)(q->bytesAvailable());
		if(n < 0) n = 0;
		return n;
	}
	return 0;
}

void M_BASE::get_source_parameters(int& channels, int& rate,
													 int& latency_ms, size_t& word_size,
													 int& chunk_bytes, int& buffer_bytes)
{
	channels = m_source_channels.load();
	rate = m_source_rate.load();
	latency_ms = m_source_latency_ms.load();
	word_size = m_source_wordsize.load();
	chunk_bytes = m_source_chunk_bytes.load();
	buffer_bytes = m_source_buffer_bytes.load();
}

int64_t M_BASE::get_source_bytes_size()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<QIODevice> q = m_source_fileio;
	if(q.get() != nullptr) {
		int64_t n =  (int64_t)(q->size());
		if(n < 0) n = 0;
		return n;
	}
	return 0;
}

int64_t M_BASE::get_source_bytes_left()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<QIODevice> q = m_source_fileio;
	if(q.get() != nullptr) {
		int64_t n =  (int64_t)(q->bytesAvailable());
		if(n < 0) n = 0;
		return n;
	}
	return 0;
}
	/* SOUND_MODULE */
}
