#include "../../../config.h"
#include "../../gui/menu_flags.h"
#include "../../osd_base.h"

#include "../sound_buffer_qt.h"

#include "./osd_sound_mod_qtmultimedia.h"

#include <algorithm>
#include <QElapsedTimer>

//#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
//#include <QAudioInput>
//#include <QAudioOutput>
//#endif

namespace SOUND_MODULE {
	
/* SOUND_MODULE */
	
M_QT_MULTIMEDIA::M_QT_MULTIMEDIA(
		QObject *parent,
		OSD_BASE *osd,
		QIODevice* sinkDeviceIO,
		QIODevice* sourceDeviceIO,
		size_t base_rate,
		size_t base_latency_ms,
		size_t base_channels,
		void *extra_config_values,
		size_t extra_config_bytes )
		:
	M_BASE(
		parent,
		osd,
		sinkDeviceIO,
		sourceDeviceIO,
		base_rate,
		base_latency_ms,
		base_channels,
		extra_config_values,
		extra_config_bytes )
{
	m_classname = "SOUND_MODULE::M_QT_MULTIMEDIA";
	m_prev_sink_state = QAudio::StoppedState;
	m_prev_source_state = QAudio::StoppedState;
	
	initialize_sink_sound_devices_list();
	initialize_source_sound_devices_list();
	/*
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	m_audioOutputDevice = QMediaDevices::defaultAudioOutput();
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	m_audioOutputDevice = QAudioDeviceInfo::defaultOutputDevice();
#endif
	*/
	m_sink_device_name = "Default";

	QString _drv = QString::fromStdString(m_sink_device_name);
	config_t* _ccp = get_config_ptr();
	if(_ccp != nullptr) {
		if(strlen(_ccp->sound_device_name) > 0) {
			_drv = QString::fromLocal8Bit(_ccp->sound_device_name);
			m_sink_device_name = _drv.toStdString();
		}
	}
	// ToDo: Source Name.
	m_source_device_name = "";
}

M_QT_MULTIMEDIA::~M_QT_MULTIMEDIA()
{
}

bool M_QT_MULTIMEDIA::initialize_driver(QObject* parent)
{
	connect(this, SIGNAL(sig_start_sink()),  this, SLOT(do_start_sink()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_stop_sink()),  this, SLOT(do_stop_sink()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_mute_sink()),  this, SLOT(do_mute_sink()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_unmute_sink()),  this, SLOT(do_unmute_sink()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_discard_sink()),  this, SLOT(do_discard_sink()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_set_sink_volume(double)),  this, SLOT(do_sink_volume(double)), Qt::QueuedConnection);
	
	initialize_sink_sound_devices_list();
	initialize_source_sound_devices_list();

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	connect(&m_Root, SIGNAL(audioOutputsChanged()), this, SLOT(do_reload_sink_sound_devices()));
	connect(&m_Root, SIGNAL(audioInputsChanged()), this, SLOT(do_reload_source_sound_devices()));
#endif
	
	return true;
}

// ToDo: Connect SINGAL(FILEIO::bytesWritten(bytes)) to OSD's SLOT.
// ToDo: Independent latancy_ms when sink_driver->start() (start() without  QIODevice).
// - 20240818 K.O
bool M_QT_MULTIMEDIA::recalc_sink_buffer(int rate, int latency_ms, const bool force)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	bool __need_restart = M_BASE::recalc_samples(rate, latency_ms, false);

	if((__need_restart) || (force)) {
		std::shared_ptr<OutputSinkType> drv = m_audioOutputSink;
		bool _b = m_prev_sink_started.load();
		size_t _buf_size = m_sink_buffer_bytes.load();
		if(drv.get() != nullptr) {
			if(m_sink_external_fileio.load()) {
				// ToDo: reseize buffer of m_sink_fileio . - 20240821 K.O
				QIODevice* fio = m_sink_fileio;
				if(fio != nullptr) {
					size_t _now_size = fio->size(); // OK?
					if((_buf_size != _now_size) && (_buf_size != 0)) {
						stop_sink();
						// ToDo: RE OPEN as need buffer size.
						wait_stop_sink(-1);
						if(_b) {
							start_sink();
						}
						return true;
					}
				}
			} else {
				return true;
			}
		}
		return false;
	}
	return false;
}



void M_QT_MULTIMEDIA::release_source()
{
	// ToDo
}		

void M_QT_MULTIMEDIA::release_sink()
{
	{
		std::shared_ptr<OutputSinkType> drv = m_audioOutputSink;
		if(drv.get() != nullptr) {
			m_audioOutputSink->stop();
			m_audioOutputSink->reset();	
		}
	}
	while(m_audioOutputSink.use_count() != 0) {
		__debug_log_func(_T("Sink: used remains %d"), m_audioOutputSink.use_count());
		QThread::msleep(15);
	}
	m_audioOutputSink.reset();
}

bool M_QT_MULTIMEDIA::has_output_device(QString name)
{
	QString tmps;
	for(auto n = m_audioOutputsList.begin(); n != m_audioOutputsList.end(); ++n) {
		tmps = get_audio_device_name(*n);
		if(tmps == name) {
			return true;
		}
	}
	return false;
}

bool M_QT_MULTIMEDIA::is_default_output_device()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	if(m_audioOutputDevice == QMediaDevices::defaultAudioOutput()) {
		return true;
	}
#else
	if(m_audioOutputDevice == QAudioDeviceInfo::defaultOutputDevice()) {
		return true;
	}
#endif
	return false;
}
		

bool M_QT_MULTIMEDIA::is_sink_io_device_exists()
{
	bool b_sink_fileio = M_BASE::is_sink_io_device_exists();
	std::shared_ptr<OutputSinkType> drv = m_audioOutputSink;
	
	if(drv.get() != nullptr) {
		std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
		// OK?
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		bool _ba = !(drv->isNull());
	#else
		bool _ba = ((drv->error() != QAudio::UnderrunError) && (drv->error() != QAudio::NoError)) ? true : false;
	#endif
		bool _bb = (m_sink_fileio != nullptr) ? true : false;
		return ((_ba) && (_bb) && (b_sink_fileio));
	}
	return b_sink_fileio;
}


void M_QT_MULTIMEDIA::sink_state_changed(QAudio::State newState)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<OutputSinkType> drv = m_audioOutputSink;
	QIODevice* fio = m_sink_fileio;

	QAudio::State oldStat = m_prev_sink_state.load();
	switch(newState) {
	case QAudio::ActiveState:
		if((oldStat == QAudio::SuspendedState) || (oldStat == QAudio::StoppedState)) {
			do_discard_sink();
			m_sink_before_rendered = 0;
			__debug_log_func("GO. driver=%0llx fileio=%0llx", drv.get(), fio);
		}
		if(drv.get() != nullptr) {
			drv->setVolume(m_sink_volume.load());
		}
		m_prev_sink_started = true;
		if(oldStat == QAudio::StoppedState) {
			emit sig_sink_started(); // Notify to start.
		} else if (oldStat == QAudio::SuspendedState) {
			emit sig_sink_resumed();
		}
		//__debug_log_func(_T("AUDIO:ACTIVE"));
		break;
	case QAudio::IdleState:
		emit sig_sink_empty(); // Notify to buffer empty.
		break;
	case QAudio::StoppedState:
		m_prev_sink_started = false;
		if(oldStat != QAudio::StoppedState) {
			__debug_log_func(_T("AUDIO:STOP"));
			if(!(m_sink_external_fileio.load())) {
				m_sink_fileio = nullptr;
			}
			m_sink_before_rendered = 0;
			m_sink_volume = std::nan("1");
			emit sig_sink_stopped(); // Notify to stopped.
		}
		break;
	case QAudio::SuspendedState:
		if((oldStat != QAudio::SuspendedState) && (oldStat != QAudio::StoppedState)) {
			__debug_log_func(_T("AUDIO:SUSPEND"));
			emit sig_sink_suspended();  // Notify to be accepted suspend.
		}
		break;
	#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	case QAudio::InterruptedState:
		__debug_log_func(_T("AUDIO:INTERRUPTED"));
		emit sig_sink_interruput_accpted();
		break;
	#endif

	}
	m_prev_sink_state = newState;
}


void M_QT_MULTIMEDIA::update_sink_driver_fileio()
{
	std::shared_ptr<OutputSinkType> drv = m_audioOutputSink;
	bool _b = m_prev_sink_started.load();
	
	if(m_sink_external_fileio.load()) {
		std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
		if(!(_b)) {
			drv->stop();
			wait_stop_sink(-1);
		}
		if(_b) {
			start_sink();
		}
	}
}

void M_QT_MULTIMEDIA::source_state_changed(QAudio::State newState)
{
	// ToDo:
}

void M_QT_MULTIMEDIA::set_audio_format(DeviceInfoType dest_device, QAudioFormat& desired, size_t& channels, size_t& rate)
{
	int _channels = (int)channels;
	int _rate = (int)rate;
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	if(dest_device.minimumChannelCount() > _channels) {
		_channels = dest_device.minimumChannelCount();
	} else if(dest_device.maximumChannelCount() < _channels) {
		_channels = dest_device.maximumChannelCount();
	}
	if(dest_device.minimumSampleRate() > _rate) {
		_rate = dest_device.minimumSampleRate();
	} else if(dest_device.maximumSampleRate() < _rate) {
		_rate = dest_device.maximumSampleRate();
	}
	QList<QAudioFormat::SampleFormat> _al = dest_device.supportedSampleFormats();
	if(_al.contains(QAudioFormat::Int16)) {
		desired.setSampleFormat(QAudioFormat::Int16);
	} else if(_al.contains(QAudioFormat::Int32)) {
		desired.setSampleFormat(QAudioFormat::Int32);
	} else {
		desired.setSampleFormat(QAudioFormat::Unknown);
	}

	switch(channels) {
	case 1:
		channels = 1;
		desired.setChannelConfig(QAudioFormat::ChannelConfigMono);
		break;
	case 2:
		desired.setChannelConfig(QAudioFormat::ChannelConfigStereo);
		break;
	case 3:
		desired.setChannelConfig(QAudioFormat::ChannelConfig2Dot1);
		break;
	case 5:
		desired.setChannelConfig(QAudioFormat::ChannelConfigSurround5Dot0);
		break;
	case 6:
		desired.setChannelConfig(QAudioFormat::ChannelConfigSurround5Dot1);
		break;
	case 7:
		desired.setChannelConfig(QAudioFormat::ChannelConfigSurround7Dot0);
		break;
	case 8:
		desired.setChannelConfig(QAudioFormat::ChannelConfigSurround7Dot1);
		break;
	default:
		channels = 2;
		desired.setChannelConfig(QAudioFormat::ChannelConfigStereo);
		break;
	}
	#else
	QList<int> channelsList = dest_device.supportedChannelCounts();
	QList<int> ratesList    = dest_device.supportedSampleRates();

	int _min_channels = INT_MAX;
	int _max_channels = 0;
	for(auto i = channelsList.begin() ; i != channelsList.end(); ++i) {
		if((*i) < _min_channels) _min_channels = (*i);
		if((*i) > _max_channels) _max_channels = (*i);
	}
	if(_min_channels > _channels) {
		_channels = _min_channels;
	} else if(_max_channels < _channels) {
		_channels = _max_channels;
	}
	int _min_rate = INT_MAX;
	int _max_rate = 0;
	for(auto i = ratesList.begin() ; i != ratesList.end(); ++i) {
		if((*i) < _min_rate) _min_rate = (*i);
		if((*i) > _max_rate) _max_rate = (*i);
	}
	if(_min_rate > rate) {
		_rate = _min_rate;
	} else if(_max_rate < rate) {
		_rate = _max_rate;
	}
	#endif	
	//if(_rate > 0) {
	//}
	if(_rate <= 0) {
		return;
	}
	rate = (size_t)_rate; // Workaround 20221018 K.O
	if(_channels > 0) {
		channels = (size_t)_channels; // Workaround 20221008 K.O
	}
	desired.setSampleRate(_rate);
	desired.setChannelCount(_channels);
	#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	desired.setSampleSize(16);
	desired.setSampleType(QAudioFormat::SignedInt);
		#if Q_BYTE_ORDER == Q_BIG_ENDIAN
	desired.setByteOrder(QAudioFormat::BigEndian);
		#else
	desired.setByteOrder(QAudioFormat::LittleEndian);
		#endif
	#endif
}

bool M_QT_MULTIMEDIA::initialize_sink_driver_post(QObject* parent)
{
	bool result = false;

	// ToDo: Input
	DeviceInfoType tmp_output_device;
	
	tmp_output_device = get_output_device_by_name(QString::fromStdString(m_sink_device_name));
	if(tmp_output_device.isNull()) {
		return false;
	}
	
	QAudioFormat tmp_output_format = tmp_output_device.preferredFormat();

	size_t _channels = m_sink_channels.load();
	size_t _rate = m_sink_rate.load();
	set_audio_format(tmp_output_device, tmp_output_format, _channels, _rate);
	if((_channels <= 0) || (_rate <= 0)) {
		tmp_output_format = tmp_output_device.preferredFormat();
		_channels = (size_t)(tmp_output_format.channelCount());
		_rate     = (size_t)(tmp_output_format.sampleRate());
		if((_rate == 0) || (_channels == 0)) {
			return false; // None devices.
		}
	}
	
	result = set_new_output_device(tmp_output_device, tmp_output_format);
	update_sink_driver_fileio();

	__debug_log_func(_T("Sink status=%s"), (m_config_ok) ? _T("OK") : _T("NG"));
	return result;
}
bool M_QT_MULTIMEDIA::initialize_source_driver_post(QObject* parent)
{
	return true; // Dummy
}

void M_QT_MULTIMEDIA::initialize_sink_sound_devices_list()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	m_audioOutputsList = QMediaDevices::audioOutputs();
#else
	m_audioOutputsList = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
#endif
}

void M_QT_MULTIMEDIA::initialize_source_sound_devices_list()
{
	// ToDo.	
}

std::list<std::string> M_QT_MULTIMEDIA::get_sound_sink_devices_list()
{
	std::list<std::string> _l;
	QString tmps;
	for(auto n = m_audioOutputsList.begin(); n != m_audioOutputsList.end(); ++n) {
		tmps = get_audio_device_name(*n);
		_l.push_back(tmps.toStdString());
	}
		
	return _l;
}

bool M_QT_MULTIMEDIA::is_output_driver_stopped()
{
	std::shared_ptr<OutputSinkType> drv = m_audioOutputSink;
	if(drv.get() != nullptr) {
		return ((drv->state() == QAudio::StoppedState) ? true : false);
	}
	return true;
}

bool M_QT_MULTIMEDIA::is_capture_driver_stopped()
{
	std::shared_ptr<InputSourceType> drv = m_audioInputSource;
	if(drv.get() != nullptr) {
		return ((drv->state() == QAudio::StoppedState) ? true : false);
	}
	return true;
}


M_QT_MULTIMEDIA::DeviceInfoType M_QT_MULTIMEDIA::get_output_device_by_name(QString driver_name)
{
	DeviceInfoType dest_device;
	if((driver_name == QString::fromUtf8("Default")) || (driver_name.isEmpty())) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		dest_device = QMediaDevices::defaultAudioOutput();
#else
		dest_device = QAudioDeviceInfo::defaultOutputDevice();
#endif
	} else {
		for(auto i = m_audioOutputsList.begin(); i != m_audioOutputsList.end(); ++i) {
			QString _s = get_audio_device_name(*i);
			__UNLIKELY_IF(_s.compare(driver_name) ==0) {
				dest_device = *i;
				break;
			}
		}
	}
	if(dest_device.isNull()) {
		__debug_log_func(_T("E:desired_driver=%s but not found."), driver_name.toLocal8Bit().constData());
		return dest_device;
	}
	QString dest_device_name = get_audio_device_name(dest_device);

	__debug_log_func(_T("desired_driver=%s using=%s"), driver_name.toLocal8Bit().constData(), dest_device_name.toLocal8Bit().constData());

	return dest_device;
}

void M_QT_MULTIMEDIA::do_set_output_by_name(QString driver_name)
{
	DeviceInfoType dest_device = get_output_device_by_name(driver_name);
	
	bool force = false;
	if(!(m_config_ok.load())) {
		force = true;
	}
	size_t _rate = m_sink_rate.load();
	size_t _channels = m_sink_channels.load();
	size_t _latency = m_sink_latency_ms.load();
	setup_output_device(dest_device, _rate, _channels, _latency, force);
	m_sink_rate = _rate;
	m_sink_channels = _channels;
	m_sink_latency_ms = _latency;
	
	config_t* _ccp = get_config_ptr();
	if(_ccp != nullptr) {
		set_sink_volume((int)(_ccp->general_sound_level));
	}
}

void M_QT_MULTIMEDIA::do_set_input_by_name(QString driver_name)
{
	// ToDo.
}

bool M_QT_MULTIMEDIA::set_new_output_device(DeviceInfoType dest_device, QAudioFormat dest_format)
{
	m_config_ok = false;
	if((dest_device.isNull()) || !(dest_format.isValid())) {
		return false; // Failed
	}
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(!(is_output_driver_stopped())) {
		stop_sink();
		wait_stop_sink(1000);
	}
	__LIKELY_IF(m_audioOutputSink.get() != nullptr) {
		m_audioOutputSink->disconnect();
	}
	
	int rate = (int)(m_sink_rate.load());
	int latency_ms = (int)(m_sink_latency_ms.load());
	
	QString __name = get_audio_device_name(dest_device);
	recalc_sink_buffer(rate, latency_ms, true);
	
	m_audioOutputSink.reset(new OutputSinkType(dest_device, dest_format, this));
	if(m_audioOutputSink.get() != nullptr) { // OK.
		connect(m_audioOutputSink.get(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(sink_state_changed(QAudio::State)));
		m_prev_sink_started = false;
		m_sink_before_rendered = 0;
		QAudioFormat fmt = m_audioOutputSink->format();
		m_sink_channels = fmt.channelCount();
		m_sink_rate = fmt.sampleRate();
		#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		m_sink_wordsize = (size_t)(fmt.bytesPerSample());
		#else
		m_sink_wordsize = (size_t)(fmt.sampleSize()) >> 3;
		#endif

		M_BASE::recalc_samples(m_sink_rate.load(), m_sink_latency_ms.load(), true);
		if(m_sink_external_fileio.load()) {
			QIODevice* fio = m_sink_fileio;
			if(fio != nullptr) {
				fio->close();
				// ToDo: Resize fio.
			}
			M_BASE::reopen_sink_fileio(true);
		} else {
			m_audioOutputSink->setBufferSize(m_sink_buffer_bytes.load());
		}
		m_audioOutputDevice = dest_device;
		m_audioOutputFormat = fmt;
		m_sink_device_name = __name.toLocal8Bit().toStdString();		
		__debug_log_func(_T("Desired device \"%s\" temporally preferred format. rate=%d channels=%d"), __name.toLocal8Bit().constData(), fmt.sampleRate(), fmt.channelCount());

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		return true;
#else
		return ((m_audioOutputSink->error() != QAudio::FatalError) ? true : false);
#endif
	} else {
		__debug_log_func(_T("Failed to set desired device \"%s\""), __name.toLocal8Bit().constData());
	}
	return false;
}


void M_QT_MULTIMEDIA::setup_output_device(DeviceInfoType dest_device, size_t& rate, size_t& channels, size_t& latency_ms, bool force_reinit)
{
	if(dest_device.isNull()) return;

	__debug_log_func(_T("Expected: rate=%d channels=%d latency=%dmSec reinit=%d"), rate, channels, latency_ms, force_reinit);
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);

	if(!(force_reinit)) {
		// If already initialized and not changed, skip.
		if((m_audioOutputDevice == dest_device)
		   && (rate == m_sink_rate.load())
		   && (channels == m_sink_channels.load())
		   && (latency_ms == m_sink_latency_ms.load())
		   && (m_audioOutputSink.get() != nullptr)
		   && (m_sink_fileio != nullptr)) {
			if(m_sink_external_fileio.load()) {
				QIODevice* fio = m_sink_fileio;
				if(fio->isOpen()) {
					return;
				}
				if(is_output_driver_stopped()) {
					// Reopen fio;
				}
			}
			start_sink();
			__debug_log_func(_T("Nothing changed. But restart device"));
			return;
		}
	}
	
	bool req_reinit = force_reinit;
	bool req_device_reset = false;
	if((m_audioOutputDevice.isNull()) || (m_audioOutputDevice != dest_device) || (m_audioOutputSink.get() == nullptr)) {
		req_device_reset = true;
	}
	if(m_sink_rate.load() != rate) {
		req_reinit = true;
	}
	if(m_sink_channels.load() != channels) {
		req_reinit = true;
	}
	if(m_sink_latency_ms.load() != latency_ms) {
		req_reinit = true;
	}
	
	if(!(req_device_reset) && (m_audioOutputSink.get() != nullptr)) {
		if((m_audioOutputSink->format().channelCount() != channels) ||
		   (m_audioOutputSink->format().sampleRate() != rate)) {
			req_reinit = true;
		}
	}

	if((req_reinit) || (req_device_reset)) {
		QAudioFormat desired = dest_device.preferredFormat();
		set_audio_format(dest_device, desired, channels, rate);
		if((channels <= 0) || (rate <= 0)) {
			desired = dest_device.preferredFormat(); // Re-Reset.
			rate = desired.sampleRate();
			channels = desired.channelCount();
			QString __name = get_audio_device_name(dest_device);
			__debug_log_func(_T("Desired device \"%s\" don't be effective.Make fallback. rate=%d channels=%d"), __name.toLocal8Bit().constData(), rate, channels);
		}
		m_sink_latency_ms = latency_ms;
		
		bool result = set_new_output_device(dest_device, desired);
		
		if(result) {
			if(is_default_output_device()) {
				m_sink_device_name = "Default";
			}
			config_t* _ccp = get_config_ptr();
			if(_ccp != nullptr) {
				memset(_ccp->sound_device_name, 0x00, sizeof(_ccp->sound_device_name));
				my_tcscpy_s(_ccp->sound_device_name, (sizeof(_ccp->sound_device_name) / sizeof(_TCHAR)) - 1, m_sink_device_name.c_str());
			}
		} else {
			m_sink_device_name.clear();
		}
	}
	__debug_log_func(_T("Result: rate=%d channels=%d latency=%dmSec reinit=%d"), m_sink_rate.load(), m_sink_channels.load(), m_sink_latency_ms.load(), force_reinit);
	
	start_sink();
}

const std::string M_QT_MULTIMEDIA::set_sink_device_sound(std::string driver_name, size_t& rate, size_t& channels, size_t& latency_ms)
{
	if(driver_name.empty()) {
		return (const std::string)(std::string(""));
	}

	QString _name = QString::fromStdString(driver_name);

	DeviceInfoType dest_device = get_output_device_by_name(_name);
	setup_output_device(dest_device, rate, channels, latency_ms, false);
	return (const std::string)m_sink_device_name;
}

bool M_QT_MULTIMEDIA::real_reconfig_sound(size_t& rate,size_t& channels,size_t& latency_ms, const bool force)
{
	if((rate <= 0) || (channels < 1) || (latency_ms < 10)) {
		return false;
	}
	size_t __rate = m_sink_rate.load();
	size_t __latency_ms = m_sink_latency_ms.load();
	bool prev_started = m_prev_sink_started.load();
	size_t __channels = m_sink_channels.load();
	bool is_reset_sink = false;
	if((force) || (rate != __rate) || (channels != __channels) /* || (latency_ms != __latanecy_ms) */) {
		set_sink_device_sound(m_sink_device_name, rate, channels, latency_ms);
		is_reset_sink = true;
	}
	__debug_log_func(_T("Returned Driver=\"%s\" rate=%dHz channles=%d latency=%dmSec"), m_sink_device_name.c_str(), rate, channels, latency_ms);
	
	if((rate == 0) || (latency_ms <= 0)) {
		m_config_ok = false;
		return m_config_ok.load();
	}
	if(!(is_reset_sink)) {
		// Sink don't change, but latency has changed.
		recalc_sink_buffer((int)__rate, (int)__latency_ms, false);
	} else {
		// Restart 
		m_sink_channels = channels;
		m_sink_rate = rate;
		m_sink_latency_ms = latency_ms;
		recalc_sink_buffer(rate, latency_ms, true);
	}
	
	if(prev_started) {
		//m_mute = false;
		start_sink();
	}
	return m_config_ok.load();
}


bool M_QT_MULTIMEDIA::wait_start_sink(int64_t msec)
{
	bool _infinite = false;
	__UNLIKELY_IF((msec < 0) || (msec == INT64_MAX)) {
		_infinite = true;
	}
	QElapsedTimer _timer;
	if(_infinite) {
		do {
			if(is_output_driver_started()) {
				return true;
			}
			QThread::msleep(5);
		} while(1);
	} else {
		_timer.start();
		do {
			if(is_output_driver_started()) {
				_timer.invalidate();
				return true;
			}
			if(msec > _timer.elapsed()) {
				_timer.invalidate();
				return false;
			}
			QThread::msleep(5);
		} while(1);
	}
	return false;
}

bool M_QT_MULTIMEDIA::wait_stop_sink(int64_t msec)
{
	bool _infinite = false;
	__UNLIKELY_IF((msec < 0) || (msec == INT64_MAX)) {
		_infinite = true;
	}
	QElapsedTimer _timer;
	if(_infinite) {
		do {
			if(is_output_driver_stopped()) {
				return true;
			}
			QThread::msleep(5);
		} while(1);
	} else {
		_timer.start();
		do {
			if(is_output_driver_stopped()) {
				_timer.invalidate();
				return true;
			}
			if(msec > _timer.elapsed()) {
				_timer.invalidate();
				return false;
			}
			QThread::msleep(5);
		} while(1);
	}
	return false;
}

// ToDo: Connect SINGAL(FILEIO::bytesWritten(bytes)) to OSD's SLOT.
// - 20240818 K.O
void M_QT_MULTIMEDIA::do_start_sink()
{
	std::shared_ptr<OutputSinkType> p = m_audioOutputSink;
	__UNLIKELY_IF(p.get() == nullptr) {
		return;
	}
	if(p->state() == QAudio::StoppedState) {
		if(p->bufferSize() != m_sink_buffer_bytes.load()) {
			p->setBufferSize(m_sink_buffer_bytes.load()); // Resize buffer
		}
	} else {
		return; // Already started.
	}
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if((m_sink_external_fileio.load()) && (m_sink_fileio != nullptr)) {
		p->start(m_sink_fileio);
	} else {
		m_sink_external_fileio = false; // OK?
		m_sink_fileio = p->start();
	}
	return;
}

void M_QT_MULTIMEDIA::do_stop_sink()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<OutputSinkType> p = m_audioOutputSink;
	if(p.get() != nullptr) {
		p->stop();
	}
	do_discard_sink();
	if(!(m_sink_external_fileio.load())) {
		m_sink_fileio = nullptr;
	}
	m_sink_before_rendered = 0;
	m_prev_sink_started = false;
	m_sink_volume = std::nan("1");
}

void M_QT_MULTIMEDIA::do_sink_volume(double level)
{
	std::shared_ptr<OutputSinkType> p = m_audioOutputSink;
	if(p.get() != nullptr) {
		p->setVolume(level);
	}
	m_sink_volume = level;
}

bool M_QT_MULTIMEDIA::is_output_driver_started()
{
	std::shared_ptr<OutputSinkType> p = m_audioOutputSink;
	if(p.get() == nullptr) {
		return true;
	}
	if(p->state() == QAudio::StoppedState) {
		return false;
	}
	return true;
}


bool M_QT_MULTIMEDIA::is_capture_driver_started()
{
	std::shared_ptr<InputSourceType> p = m_audioInputSource;
	if(p.get() == nullptr) {
		return true;
	}
	if(p->state() == QAudio::StoppedState) {
		return false;
	}
	return true;
}


void M_QT_MULTIMEDIA::do_mute_sink()
{
	if(!(m_mute.load()) && (m_config_ok.load())) {
		std::shared_ptr<OutputSinkType> p = m_audioOutputSink;
		//std::shared_ptr<QIODevice> fio = m_sink_fileio;

		if(p.get() != nullptr) {
			switch(p->state()) {
			case QAudio::ActiveState:
			case QAudio::IdleState:
				p->suspend();
				do_discard_sink();
				break;
			default:
				break;
			}
		}
	}
	m_mute = true;
}

void M_QT_MULTIMEDIA::do_unmute_sink()
{
	if((m_mute.load()) && (m_config_ok.load())) {
		std::shared_ptr<OutputSinkType> p = m_audioOutputSink;
		if(p.get() != nullptr) {
			switch(p->state()) {
			case QAudio::SuspendedState:
			case QAudio::IdleState:
				do_discard_sink();
				p->resume();
				p->setVolume(m_sink_volume.load());
				break;
			default:
				break;
			}
		}
	}
	m_mute = false;
}


void M_QT_MULTIMEDIA::do_discard_sink()
{
//	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<OutputSinkType> q = m_audioOutputSink;
	if(q.get() != nullptr) {
		q->reset();
	}
}

void M_QT_MULTIMEDIA::do_reload_sink_sound_devices()
{
	initialize_sink_sound_devices_list();
	// Check current device still aveilable.
	// I not, todo?
	emit sig_output_devices_list_changed();
}
void M_QT_MULTIMEDIA::do_reload_source_sound_devices()
{
	initialize_source_sound_devices_list();
	// Check current device still aveilable.
	// I not, todo?
	emit sig_input_devices_list_changed();
}

int64_t M_QT_MULTIMEDIA::update_sound(void* datasrc, int samples)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<OutputSinkType> drv = m_audioOutputSink;
	QIODevice* q = m_sink_fileio;
	qint64 sample_bytes = ((qint64)samples) * ((qint64)(m_sink_channels.load() * m_sink_wordsize.load()));

	if(sample_bytes <= 0) {
		return 0;
	}
	__UNLIKELY_IF(drv.get() == nullptr) {
		return samples;
	}
	__UNLIKELY_IF(drv->state() == QAudio::StoppedState) {
		//emit sig_start_sink();
		return samples;
	} else if(drv->state() == QAudio::SuspendedState) { // OK?
		// ToDo: Wait for unmute.
		return samples;
	}
	
	config_t* _ccp = get_config_ptr();
	if(_ccp != nullptr) {
		set_sink_volume((int)(_ccp->general_sound_level));
	}
	
	if(q != nullptr) {
		qint64 wrote = 0;
		wrote = q->write((const char *)datasrc, (qint64)sample_bytes);
		if(wrote <= 0) {
			return 0;
		}
		wrote = wrote / sample_bytes;
		return (int64_t)wrote;
	}
	return 0;
}

size_t M_QT_MULTIMEDIA::get_sink_buffer_bytes()
{
	if(m_sink_external_fileio.load()) {
		return M_BASE::get_sink_buffer_bytes();
	}
	std::shared_ptr<OutputSinkType> drv = m_audioOutputSink;
	if(drv.get() != nullptr) {
		return (size_t)(drv->bufferSize());
	}
	return 0;
}

size_t M_QT_MULTIMEDIA::get_source_buffer_bytes()
{
	if(m_source_external_fileio.load()) {
		return M_BASE::get_source_buffer_bytes();
	}
	std::shared_ptr<InputSourceType> drv = m_audioInputSource;
	if(drv.get() != nullptr) {
		return (size_t)(drv->bufferSize());
	}
	return 0;
}

/* SOUND_MODULE */

}
