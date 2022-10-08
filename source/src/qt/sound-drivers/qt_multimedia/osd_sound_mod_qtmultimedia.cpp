#include "../../../config.h"
#include "../../gui/menu_flags.h"
#include "../../osd_base.h"

#include "../sound_buffer_qt.h"

#include "./osd_sound_mod_qtmultimedia.h"

#include <algorithm>

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#include <QMediaDevices>
#endif

namespace SOUND_MODULE {
/* SOUND_MODULE */
	
	namespace OUTPUT {
	/* SOUND_MODULE */
	M_QT_MULTIMEDIA::M_QT_MULTIMEDIA(
		OSD_BASE *parent,
		SOUND_BUFFER_QT* deviceIO,
		int base_rate,
		int base_latency_ms,
		int base_channels,
		void *extra_config_values,
		int extra_config_bytes )
		:
	M_BASE(
		parent,
		deviceIO,
		base_rate,
		base_latency_ms,
		base_channels,
		extra_config_values,
		extra_config_bytes )
{
	m_classname = "SOUND_MODULE::OUTPUT::M_QT_MULTIMEDIA";
	
	connect(this, SIGNAL(sig_start_audio()),  this, SLOT(do_sound_start()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_stop_audio()),  this, SLOT(do_sound_stop()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_pause_audio()),  this, SLOT(do_sound_suspend()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_resume_audio()),  this, SLOT(do_sound_resume()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_discard_audio()),  this, SLOT(do_discard_sound()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_set_volume(double)),  this, SLOT(do_sound_volume(double)), Qt::QueuedConnection);

	connect(parent, SIGNAL(sig_set_sound_volume(int)),  this, SLOT(set_volume(int)), Qt::QueuedConnection);
	connect(parent, SIGNAL(sig_set_sound_volume(double)),  this, SLOT(set_volume(double)), Qt::QueuedConnection);
	connect(parent, SIGNAL(sig_set_sound_device(QString)),  this, SLOT(do_set_device_by_name(QString)), Qt::QueuedConnection);
	
	initialize_sound_devices_list();
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	m_audioOutputDevice = QMediaDevices::defaultAudioOutput();
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	m_audioOutputDevice = QAudioDeviceInfo::defaultOutputDevice();
#endif		
	m_device_is_default = true;
	m_device_name = "Default";
	
	QString _drv = QString::fromStdString(m_device_name);
	config_t* _ccp = get_config_ptr();
	if(_ccp != nullptr) {
		if(strlen(_ccp->sound_device_name) > 0) {
			_drv = QString::fromUtf8(_ccp->sound_device_name);
		}
	}
	auto _match = std::find(devices_name_list.begin(), devices_name_list.end(), _drv.toLocal8Bit().toStdString());
	if(_match != devices_name_list.end()) {
		m_device_name = (*_match);
	}
	m_config_ok = initialize_driver();
}

M_QT_MULTIMEDIA::~M_QT_MULTIMEDIA()
{
}

void M_QT_MULTIMEDIA::driver_state_changed(QAudio::State newState)
{
	switch(newState) {
	case QAudio::ActiveState:
		__debug_log_func(_T("AUDIO:ACTIVE"));
		break;
	case QAudio::IdleState:
		__debug_log_func(_T("AUDIO:IDLE"));
		//if(m_audioOutputSink != nullptr) {
		//	m_audioOutputSink->stop();
		//}
		break;
	case QAudio::StoppedState:
		__debug_log_func(_T("AUDIO:STOP"));
		break;
	case QAudio::SuspendedState:
		__debug_log_func(_T("AUDIO:SUSPEND"));
		break;
	#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	case QAudio::InterruptedState:
		__debug_log_func(_T("AUDIO:INTERRUPTED"));
		break;
	#endif
	
	}
}


void M_QT_MULTIMEDIA::update_driver_fileio()
{
	m_driver_fileio = m_fileio;
}



#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
void M_QT_MULTIMEDIA::set_audio_format(QAudioDevice dest_device, QAudioFormat& desired, int& channels, int& rate)
{
	int _channels = channels;
	if(dest_device.minimumChannelCount() > _channels) {
		_channels = dest_device.minimumChannelCount();
	} else if(dest_device.maximumChannelCount() < _channels) {
		_channels = dest_device.maximumChannelCount();
	}
	if(dest_device.minimumSampleRate() > rate) {
		rate = dest_device.minimumSampleRate();
	} else if(dest_device.maximumSampleRate() < rate) {
		rate = dest_device.maximumSampleRate();
	}
	if((rate <= 0)) {
		return;
	}
	if(_channels > 0) {
		channels = _channels; // Workaround 20221008 K.O
	}
	desired.setSampleRate(rate);

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
	desired.setChannelCount(channels);	
}
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void M_QT_MULTIMEDIA::set_audio_format(QAudioDeviceInfo dest_device, QAudioFormat& desired, int& channels, int& rate)
{
	int _channels = channels;
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
		rate = _min_rate;
	} else if(_max_rate < rate) {
		rate = _max_rate;
	}
	if((rate <= 0)) {
		return;
	}
	if(_channels > 0) {
		channels = _channels; // Workaround 20221008 K.O
	}
	
	desired.setSampleRate(rate);
	desired.setSampleSize(16);
	desired.setSampleType(QAudioFormat::SignedInt);
	#if Q_BYTE_ORDER == Q_BIG_ENDIAN	
	desired.setByteOrder(QAudioFormat::BigEndian);
	#else
	desired.setByteOrder(QAudioFormat::LittleEndian);
	#endif

	desired.setChannelCount(channels);	
}
#endif		

bool M_QT_MULTIMEDIA::initialize_driver()
{
	bool result = false;

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QAudioDevice tmp_output_device;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QAudioDeviceInfo tmp_output_device;
#endif
	tmp_output_device = get_device_by_name(QString::fromStdString(m_device_name));
	QAudioFormat tmp_output_format = tmp_output_device.preferredFormat();
	
	int _channels = m_channels;
	int _rate = m_rate;
	set_audio_format(tmp_output_device, tmp_output_format, _channels, _rate);
	if((_channels > 0) && (_rate > 0)) {
		m_channels = _channels;
		m_rate = _rate;
	} else {
		tmp_output_format = tmp_output_device.preferredFormat();
		_channels = tmp_output_format.channelCount();
		_rate     = tmp_output_format.sampleRate();
		if((_rate <= 0) || (_channels <= 0)) {
			return false; // None devices.
		}
	}
	m_audioOutputDevice = tmp_output_device;
	m_audioOutputFormat = tmp_output_format;
	
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	m_audioOutputSink.reset(new QAudioSink(m_audioOutputDevice, m_audioOutputFormat, this));
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	m_audioOutputSink.reset(new QAudioOutput(m_audioOutputDevice, m_audioOutputFormat, this));
#endif	
	result = ((m_audioOutputSink.get() != nullptr) /* || (m_audioInputSource.get() != nullptr) */);
	if(result) {
		connect(m_audioOutputSink.get(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(driver_state_changed(QAudio::State)));
		m_channels = m_audioOutputSink->format().channelCount();
		m_rate = m_audioOutputSink->format().sampleRate();
		m_config_ok = true;
	}
	m_samples = ((qint64)m_latency_ms * (qint64)(m_rate)) / 1000;
	if(m_samples <= 0) {
		m_samples = 4800;
	}
	m_fileio.reset(new SOUND_BUFFER_QT(m_samples * (qint64)m_channels * sizeof(int16_t) * 4));
	update_driver_fileio();

	__debug_log_func(_T("status=%s"), (m_config_ok) ? _T("OK") : _T("NG"));
	return result;
}

void M_QT_MULTIMEDIA::initialize_sound_devices_list()
{
	devices_name_list.clear();
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	m_audioOutputsList = QMediaDevices::audioOutputs();
	for(auto i = m_audioOutputsList.begin(); i != m_audioOutputsList.end(); ++i) {
		devices_name_list.push_back((*i).description().toStdString());
	}
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	m_audioOutputsList = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
	for(auto i = m_audioOutputsList.begin(); i != m_audioOutputsList.end(); ++i) {
		devices_name_list.push_back((*i).deviceName().toStdString());
	}
#endif	
}

std::list<std::string> M_QT_MULTIMEDIA::get_sound_devices_list()
{
	return devices_name_list;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
QAudioDevice M_QT_MULTIMEDIA::get_device_by_name(QString driver_name)
#else
QAudioDeviceInfo M_QT_MULTIMEDIA::get_device_by_name(QString driver_name)
#endif
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QAudioDevice dest_device = m_audioOutputDevice;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QAudioDeviceInfo dest_device = m_audioOutputDevice;
#endif
	
	if((driver_name == QString::fromUtf8("Default")) || (driver_name.isEmpty())) {
		m_device_is_default = true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		dest_device = QMediaDevices::defaultAudioOutput();
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		dest_device = QAudioDeviceInfo::defaultOutputDevice();
#endif		
	} else {
		for(auto i = m_audioOutputsList.begin(); i != m_audioOutputsList.end(); ++i) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
			if((*i).description().compare(driver_name) == 0) {
				dest_device = *i;
				m_device_is_default = false;
				break;
			}
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			if((*i).deviceName().compare(driver_name) == 0) {
				dest_device = *i;
				m_device_is_default = false;
				break;
			}
#endif
		}
	}
	QString dest_device_name;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	dest_device_name = dest_device.description();
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	dest_device_name = dest_device.deviceName();
#endif
	
	__debug_log_func(_T("desired_driver=%s using=%s"), driver_name.toLocal8Bit().constData(), dest_device_name.toLocal8Bit().constData());

	return dest_device;
}
	
void M_QT_MULTIMEDIA::do_set_device_by_name(QString driver_name)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QAudioDevice dest_device = get_device_by_name(driver_name);
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QAudioDeviceInfo dest_device = get_device_by_name(driver_name);
#endif
	setup_device(dest_device, m_rate, m_channels, m_latency_ms, true);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
void M_QT_MULTIMEDIA::setup_device(QAudioDevice dest_device, int& rate,int& channels,int& latency_ms, bool force_reinit)
#else
void M_QT_MULTIMEDIA::setup_device(QAudioDeviceInfo dest_device, int& rate,int& channels,int& latency_ms, bool force_reinit)
#endif
{
	if(dest_device.isNull()) return; // None initialize if NULL.
	
	__debug_log_func(_T("Expected: rate=%d channels=%d latency=%dmSec reinit=%d"), rate, channels, latency_ms, force_reinit);
	
	if(!(force_reinit)) {
		// If already initialized and not changed, skip.
		if((m_audioOutputDevice == dest_device)
		   && (rate == m_rate)
		   && (channels == m_channels)
		   && (latency_ms == m_latency_ms)
		   && (m_audioOutputSink.get() != nullptr)
		   && (m_fileio.get() != nullptr)) {
			if(m_fileio->isOpen()) {
				return;
			}
			update_driver_fileio();
			__debug_log_func(_T("Nothing changed.Exit."));

			//real_reconfig_sound(rate, channels, latency_ms);
			emit sig_start_audio();
			return;
		}
	}
	if((m_audioOutputDevice.isNull()) || (m_audioOutputDevice != dest_device)) {
		force_reinit = true;
	}
	bool force_req_reinit = false;
	if(!(force_reinit)) {
		if(m_latency_ms != latency_ms) {
			force_req_reinit = true;
		}
		if(m_audioOutputSink.get() != nullptr) {
			if((m_audioOutputSink->format().channelCount() != channels) ||
			   (m_audioOutputSink->format().sampleRate() != rate)) {
				force_req_reinit = true;
			}
		} else {
			force_reinit = true;
		}
	}

	
	if((force_reinit) || (force_req_reinit)) {
		#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		QString __name = dest_device.description();
		#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QString __name = dest_device.deviceName();
		#endif
		
		QAudioFormat desired = dest_device.preferredFormat();
		int _channels = channels;
		int _rate = rate;
		set_audio_format(dest_device, desired, channels, rate);
		if((channels <= 0) || (rate <= 0)) {
			__debug_log_func(_T("Desired device \"%s\" don't be effective.Make fallback. rate=%d channels=%d"), __name.toLocal8Bit().constData(), rate, channels);
			channels = _channels;
			rate = _rate;
			return;
		}
		
		if(m_audioOutputSink.get() != nullptr) {
			if(m_audioOutputSink->state() != QAudio::StoppedState) {
				m_audioOutputSink->stop();
				wait_driver_stopped(1000);
			}
			m_audioOutputSink->disconnect();
			m_audioOutputSink.reset();
		}
		
		m_audioOutputDevice = dest_device;
		m_audioOutputFormat = desired;
		
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		m_audioOutputSink.reset(new QAudioSink(dest_device, desired, this));
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		m_audioOutputSink.reset(new QAudioOutput(dest_device, desired, this));
#endif
		if(m_audioOutputSink.get() != nullptr) {
			connect(m_audioOutputSink.get(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(driver_state_changed(QAudio::State)));
			channels = m_audioOutputSink->format().channelCount();
			rate = m_audioOutputSink->format().sampleRate();
			QString _tmpname = QString::fromUtf8("Defalut");
			if(!(m_device_is_default)) {
				#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
				_tmpname = m_audioOutputDevice.description();
				#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				_tmpname = m_audioOutputDevice.deviceName();
				#endif
			}
			m_device_name = _tmpname.toLocal8Bit().toStdString();
			config_t* _ccp = get_config_ptr();
			if(_ccp != nullptr) {
				memset(_ccp->sound_device_name, 0x00, sizeof(_ccp->sound_device_name));
				my_tcscpy_s(_ccp->sound_device_name, (sizeof(_ccp->sound_device_name) / sizeof(_TCHAR)) - 1, _tmpname.toUtf8().constData());
			}
			
			int64_t _samples =
				((int64_t)rate * latency_ms) / 1000;
			if(_samples < 100) _samples = 100;
			
			if(m_fileio.get() != nullptr) {
				if(m_fileio->isOpen()) {
					m_fileio->close();
				}
			}
			m_fileio.reset(new SOUND_BUFFER_QT(_samples * (qint64)channels * sizeof(int16_t) * 4));
			m_config_ok = (m_fileio.get() != nullptr);
				
			if(m_config_ok.load()) {
				real_reconfig_sound(rate, channels, latency_ms);
			}
		} else {
			m_device_name.clear();
			m_config_ok = false;
			
			int64_t _samples =
				((int64_t)rate * latency_ms) / 1000;
			if(_samples < 100) _samples = 100;
			if(m_fileio.get() != nullptr) {
				if(m_fileio->isOpen()) {
					m_fileio->close();
				}
				m_fileio.reset();
			}
			m_samples = _samples;
			m_latency_ms = latency_ms;
			m_rate = rate;
			m_channels = channels;
		}
	}
	__debug_log_func(_T("Result: rate=%d channels=%d latency=%dmSec reinit=%d"), m_rate, m_channels, m_latency_ms, force_reinit);
	if(m_audioOutputSink.get() != nullptr) {
		update_driver_fileio();
		emit sig_start_audio();
	}
}

const std::string M_QT_MULTIMEDIA::set_device_sound(const _TCHAR* driver_name, int& rate,int& channels,int& latency_ms)
{
	if(driver_name == nullptr) {
		return (const std::string)(std::string(""));
	}
	if(strlen(driver_name) <= 0) {
		return (const std::string)(std::string(""));
	}
	
	QString _name = QString::fromUtf8(driver_name);
	
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QAudioDevice dest_device = get_device_by_name(_name);
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QAudioDeviceInfo dest_device = get_device_by_name(_name);
#endif
	setup_device(dest_device, rate, channels, latency_ms, false);
	
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	return m_audioOutputDevice.description().toStdString();
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	return m_audioOutputDevice.deviceName().toStdString();
#else
	return (const std::string)(std::string(""));
#endif	
}

bool M_QT_MULTIMEDIA::real_reconfig_sound(int& rate,int& channels,int& latency_ms)
{
	if((rate <= 0) || (channels < 1) || (latency_ms < 10)) {
		return false;
	}

	int64_t _samples = (rate * latency_ms) / 1000;
	if((rate != m_rate) || (_samples != m_samples) || (m_latency_ms != latency_ms)) {
		m_device_name = set_device_sound((const _TCHAR *)(m_device_name.c_str()), rate, channels, latency_ms);
		__debug_log_func(_T("Returned Driver=\"%s\" rate=%dHz channles=%d latency=%dmSec"), m_device_name.c_str(), rate, channels, latency_ms);
		//emit sig_set_sound_device(m_device_name);
	}
//	if(m_config_ok.load()) {
	std::shared_ptr<SOUND_BUFFER_QT> sp = m_fileio;
	if(sp.get() != nullptr) {
		if(!(sp->isOpen())) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			sp->open(QIODeviceBase::ReadWrite | QIODeviceBase::Truncate | QIODeviceBase::Unbuffered);
#else
			sp->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Unbuffered);
#endif
		}
		m_prev_started = m_mute = false;
		sp->reset();
	}
//	}
	
	if((rate <= 0) || (latency_ms <= 0)) {
		rate = 48000;
		latency_ms = 100;
		channels = 2;
		m_config_ok = false;
	}
	m_rate = rate;
	m_latency_ms = latency_ms;
	m_channels = channels;
	m_samples = ((int64_t)rate * (int64_t)latency_ms) / 1000;
	return m_config_ok.load();
}


void M_QT_MULTIMEDIA::release_sound()
{
//	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	
	m_audioOutputSink->disconnect();

	if(m_audioOutputSink.get() != nullptr) {
		m_audioOutputSink->stop();
	}
	m_audioOutputSink.reset();

	M_BASE::release_sound();
}

bool M_QT_MULTIMEDIA::release_driver()
{
	emit sig_stop_audio();
	if(!(wait_driver_stopped(1000))) return false;
	return release_driver_fileio();
}
	
void M_QT_MULTIMEDIA::do_sound_start()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	if(m_driver_fileio.get() != nullptr) {
		m_driver_fileio->reset();
	}
	if(p.get() != nullptr) {
		p->start(m_driver_fileio.get());
		__debug_log_func("GO. fileio=%0llx", m_driver_fileio.get());
	}
	m_prev_started = true;
}

void M_QT_MULTIMEDIA::do_sound_stop()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	if(p.get() != nullptr) {
		p->stop();
	}
	do_discard_sound();
	m_prev_started = false;
}

void M_QT_MULTIMEDIA::do_sound_resume()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	if(p.get() != nullptr) {
		p->resume();
	}
}

void M_QT_MULTIMEDIA::do_sound_suspend()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	if(p.get() != nullptr) {
		p->suspend();
	}
}

void M_QT_MULTIMEDIA::do_sound_volume(double level)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	if(p.get() != nullptr) {
		p->setVolume(level);
	}
}

int64_t M_QT_MULTIMEDIA::driver_elapsed_usec()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	if(p.get() != nullptr) {
		return (int64_t)(p->elapsedUSecs());
	}
	return 0;
}

int64_t M_QT_MULTIMEDIA::driver_processed_usec()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	if(p.get() != nullptr) {
		return (int64_t)(p->processedUSecs());
	}
	return 0;
}


void M_QT_MULTIMEDIA::mute_sound()
{
	if(!(m_mute.load()) && (m_config_ok.load())) {
		#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		std::shared_ptr<QAudioSink> p = m_audioOutputSink;
		#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
		#endif
		
		if(p.get() != nullptr) {
			switch(p->state()) {
			case QAudio::ActiveState:
			case QAudio::IdleState:
				emit sig_pause_audio();
				emit sig_discard_audio();
				break;
			default:
				break;
			}
		}
	}
	m_mute = true;
}

void M_QT_MULTIMEDIA::do_discard_sound()
{
	std::shared_ptr<SOUND_BUFFER_QT> q = m_fileio;
	if(q.get() != nullptr) {
		q->reset();
	}
}

void M_QT_MULTIMEDIA::stop_sound()
{
	if((m_config_ok.load()) && (m_prev_started)) {
		#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		std::shared_ptr<QAudioSink> p = m_audioOutputSink;
		#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
		#endif
		if(p.get() != nullptr) {
			switch(p->state()) {
			case QAudio::ActiveState:
			case QAudio::IdleState:
			case QAudio::SuspendedState:
				emit sig_stop_audio();
				break;
			default:
				break;
			}
		}
	}
}

	/* SOUND_MODULE::OUTPUT */
	}
/* SOUND_MODULE */

}

