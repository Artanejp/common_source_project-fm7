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
}

M_QT_MULTIMEDIA::~M_QT_MULTIMEDIA()
{
}

bool M_QT_MULTIMEDIA::initialize_driver(QObject* parent)
{
	connect(this, SIGNAL(sig_start_audio()),  this, SLOT(do_sound_start()));
	connect(this, SIGNAL(sig_stop_audio()),  this, SLOT(do_sound_stop()));
	connect(this, SIGNAL(sig_pause_audio()),  this, SLOT(do_sound_suspend()));
	connect(this, SIGNAL(sig_resume_audio()),  this, SLOT(do_sound_resume()));
	connect(this, SIGNAL(sig_discard_audio()),  this, SLOT(do_discard_sound()));
	connect(this, SIGNAL(sig_set_volume(double)),  this, SLOT(do_sound_volume(double)), Qt::QueuedConnection);
	connect(parent, SIGNAL(sig_set_sound_volume(int)),  this, SLOT(set_volume(int)), Qt::QueuedConnection);
	connect(parent, SIGNAL(sig_set_sound_volume(double)),  this, SLOT(set_volume(double)), Qt::QueuedConnection);
	connect(parent, SIGNAL(sig_set_sound_device(QString)),  this, SLOT(do_set_device_by_name(QString)), Qt::QueuedConnection);
	m_config_ok = initialize_driver_post(parent);
	return m_config_ok.load();
}

bool M_QT_MULTIMEDIA::recalc_samples(int rate, int latency_ms, bool need_update, bool need_resize_fileio)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	bool __need_restart = M_BASE::recalc_samples(rate, latency_ms, need_update, need_resize_fileio);
#if 0
	if((__need_restart)) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
#endif
		if(drv.get() != nullptr) {
			if(drv->state() != QAudio::StoppedState) {
				drv->stop();
			}
			drv->reset();
			if(m_fileio.get() != nullptr) {
				drv->start(m_fileio.get());
			}
			if(!(drv->isNull())) {
				__need_restart |= true;
			}
		}
	}
#endif
	return __need_restart;
}

bool M_QT_MULTIMEDIA::reopen_fileio(bool force_reopen)
{
	bool _stat = M_BASE::reopen_fileio(force_reopen);
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
#endif
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(drv.get() != nullptr) {
		if((force_reopen) || (drv->state() != QAudio::StoppedState)) {
			drv->stop();
		}
		drv->reset();
		bool _ba = false;
		if(m_fileio.get() != nullptr) {
			drv->start(m_fileio.get());
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
			_ba = (drv->isNull()) ? true : false;			
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			_ba = ((drv->error() != QAudio::UnderrunError) && (drv->error() != QAudio::NoError)) ? true : false;
#endif
		}
		return ((_ba) && (_stat)) ? true : false;
	}
	return false;
}


bool M_QT_MULTIMEDIA::release_driver_fileio()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
#endif
	if(drv.get() != nullptr) {
		drv->stop();
	}
	if((m_fileio.get() != nullptr) && !(m_external_fileio.load())) {
		m_fileio.reset();
		return true;
	}
		// Maybe disconnect some signals via m_fileio.
	return false;
}

void M_QT_MULTIMEDIA::release_sound()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	#if 1
	if(m_audioOutputSink.get() != nullptr) {
		if(m_audioOutputSink->state() != QAudio::StoppedState) {
			m_audioOutputSink->stop();
			wait_driver_stopped(1000);
		}
		m_audioOutputSink->disconnect();
		m_audioOutputSink.reset();
	}
	M_BASE::release_sound();
	emit sig_sound_finished();
	#endif
}

void M_QT_MULTIMEDIA::do_release_source(QAudio::State state)
{
	// ToDo
}		
void M_QT_MULTIMEDIA::do_release_sink(QAudio::State state)
{
	if(m_audioOutputSink.get() != nullptr) {
		m_audioOutputSink->stop();
		m_audioOutputSink->reset();	
		m_audioOutputSink.reset();
	}
}

bool M_QT_MULTIMEDIA::is_io_device_exists()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
	if(drv.get() != nullptr) {
		std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
		bool _ba = !(drv->isNull());
		bool _bb = (m_fileio.get() != nullptr) ? true : false;
		return ((_ba) && (_bb));
	}
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
	if(drv.get() != nullptr) {
		std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
		bool _ba = ((drv->error() != QAudio::UnderrunError) && (drv->error() != QAudio::NoError)) ? true : false;
		bool _bb = (m_fileio.get() != nullptr) ? true : false;
		return ((_ba) && (_bb));
	}
#endif
	return false;
}


void M_QT_MULTIMEDIA::driver_state_changed(QAudio::State newState)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
#endif
	std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	bool checkstat = false;
	switch(newState) {
	case QAudio::ActiveState:
		//__debug_log_func(_T("AUDIO:ACTIVE"));
		break;
	case QAudio::IdleState:
		break;
	case QAudio::StoppedState:
		__debug_log_func(_T("AUDIO:STOP"));
		#if 1
		if(drv.get() != nullptr) {
			drv->reset();
		}
		__LIKELY_IF(fio.get() != nullptr) {
			fio->reset();
		}
		#endif
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
#endif
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	bool need_to_reset = false;
	int64_t _expected = m_buffer_bytes.load();
	if(_expected > 0) {
		std::shared_ptr<SOUND_BUFFER_QT> p = m_fileio;
		if(p.get() != nullptr) {
			int64_t _size = p->real_buffer_size();
			if(_size <= 0) {
				need_to_reset = true;
			} else if(_expected != _size) {
				need_to_reset = !(p->resize(_expected));
			}
		} else {
			need_to_reset = true;
		}
	}
	if((need_to_reset) && !(m_external_fileio.load())) {
		bool need_reload = false;
		QAudio::State oldState = QAudio::StoppedState;
		if(drv.get() != nullptr) {
			oldState = drv->state();
			if(oldState != QAudio::StoppedState) {
				need_reload = true;
				drv->stop();
			}
		}
		if(_expected > 0) {
			if(_expected >= INT_MAX) _expected = INT_MAX - 1;
			m_fileio.reset(new SOUND_BUFFER_QT((uint64_t)_expected, this));
		}
		if((need_reload) && (oldState != QAudio::StoppedState) && (m_fileio.get() != nullptr)) {
			if(drv) {
				drv->start(m_fileio.get());
				if(oldState == QAudio::SuspendedState) {
					drv->suspend();
				}
			}
		}
	}
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
	int _rate = rate;
	if(dest_device.minimumSampleRate() > _rate) {
		_rate = dest_device.minimumSampleRate();
	} else if(dest_device.maximumSampleRate() < _rate) {
		_rate = dest_device.maximumSampleRate();
	}
	//if(_rate > 0) {
	rate = _rate; // Workaround 20221018 K.O
	//}
	if(_rate <= 0) {
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

bool M_QT_MULTIMEDIA::initialize_driver_post(QObject* parent)
{
	bool result = false;

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QAudioDevice tmp_output_device;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QAudioDeviceInfo tmp_output_device;
#endif
	tmp_output_device = get_device_by_name(QString::fromStdString(m_device_name));
	QAudioFormat tmp_output_format = tmp_output_device.preferredFormat();

	int _channels = m_channels.load();
	int _rate = m_rate.load();
	set_audio_format(tmp_output_device, tmp_output_format, _channels, _rate);
	if((_channels <= 0) || (_rate <= 0)) {
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
	m_audioOutputSink.reset(new QAudioSink(m_audioOutputDevice, m_audioOutputFormat));
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
	m_samples = ((qint64)m_latency_ms.load() * (qint64)(m_rate.load())) / 1000;
	if(m_samples.load() <= 0) {
		m_samples = 1;
	}
	m_chunk_bytes = m_samples.load() * m_wordsize.load() * m_channels.load();
	m_buffer_bytes = m_chunk_bytes.load() * 4;
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
	if(m_device_name == driver_name.toLocal8Bit().toStdString()) {
		return;
	}
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QAudioDevice dest_device = get_device_by_name(driver_name);
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QAudioDeviceInfo dest_device = get_device_by_name(driver_name);
#endif
	int _rate = m_rate.load();
	int _channels = m_channels.load();
	int _latency = m_latency_ms.load();
	setup_device(dest_device, _rate, _channels, _latency, true);
	m_rate = _rate;
	m_channels = _channels;
	m_latency_ms = _latency;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
void M_QT_MULTIMEDIA::setup_device(QAudioDevice dest_device, int& rate,int& channels,int& latency_ms, bool force_reinit)
#else
void M_QT_MULTIMEDIA::setup_device(QAudioDeviceInfo dest_device, int& rate,int& channels,int& latency_ms, bool force_reinit)
#endif
{
	if(dest_device.isNull()) return;

	__debug_log_func(_T("Expected: rate=%d channels=%d latency=%dmSec reinit=%d"), rate, channels, latency_ms, force_reinit);

	if(!(force_reinit)) {
		// If already initialized and not changed, skip.
		std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
		if((m_audioOutputDevice == dest_device)
		   && (rate == m_rate.load())
		   && (channels == m_channels.load())
		   && (latency_ms == m_latency_ms.load())
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
		if(m_latency_ms.load() != latency_ms) {
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
		std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		m_audioOutputSink.reset(new QAudioOutput(dest_device, desired, this));
		std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
#endif
		do_discard_sound();
		m_prev_started = false;
		m_before_rendered = 0;
		m_channels = channels;
		bool prepared = M_BASE::recalc_samples(rate, latency_ms, true, true);
		if(drv.get() != nullptr) {
			connect(drv.get(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(driver_state_changed(QAudio::State)));
			channels = drv->format().channelCount();
			rate = drv->format().sampleRate();
			QString _tmpname = QString::fromUtf8("Default");
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
			m_channels = channels;
			if(drv->state() != QAudio::StoppedState) {
				drv->stop();
			}
			recalc_samples(rate, latency_ms, true, true);
			//drv->setBufferSize(m_chunk_bytes.load());
			std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
			m_config_ok = !(drv->isNull());
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			m_config_ok = (drv->error() != QAudio::FatalError);
#endif
			if(m_config_ok.load()) {
				real_reconfig_sound(rate, channels, latency_ms);
			}
		} else {
			m_device_name.clear();
			m_config_ok = false;
		}
	}
	__debug_log_func(_T("Result: rate=%d channels=%d latency=%dmSec reinit=%d"), m_rate.load(), m_channels.load(), m_latency_ms.load(), force_reinit);
	if(m_audioOutputSink.get() != nullptr) {
		update_driver_fileio();
		emit sig_start_audio();
		//update_render_point_usec();
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
//	if((rate != m_rate) || (_samples != m_samples) || (m_latency_ms != latency_ms)) {
		m_device_name = set_device_sound((const _TCHAR *)(m_device_name.c_str()), rate, channels, latency_ms);
		__debug_log_func(_T("Returned Driver=\"%s\" rate=%dHz channles=%d latency=%dmSec"), m_device_name.c_str(), rate, channels, latency_ms);
		//emit sig_set_sound_device(m_device_name);
//	}
	if((rate <= 0) || (latency_ms <= 0)) {
		rate = 48000;
		latency_ms = 100;
		channels = 2;
		m_config_ok = false;
	}
	m_channels = channels;
	if(recalc_samples(rate, latency_ms, true, false)) {
		//m_prev_started = m_mute = false;
	}
	return m_config_ok.load();
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
	if(p.get() == nullptr) {
		return;
	}
	if((p->state() != QAudio::StoppedState) && (m_prev_started)) {
//		update_render_point_usec();
		return;
	}
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;
	if(fio.get() != nullptr) {
		fio->reset();
	}

	p->reset();
	//p->setBufferSize(m_chunk_bytes.load());
	if(fio.get() != nullptr) {
		p->start(fio.get());
	}
	update_render_point_usec();
	__debug_log_func("GO. fileio=%0llx buffer size=%ld", m_fileio.get(), p->bufferSize());

	//update_render_point_usec();
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
	m_before_rendered = 0;
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
		update_render_point_usec();
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
	std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;
	if(fio.get() != nullptr) {
		fio->reset();
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
		//return (int64_t)(p->processedUSecs());
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

bool M_QT_MULTIMEDIA::is_driver_started()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	bool _b = M_BASE::is_driver_started();
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	if(p.get() == nullptr) {
		return false;
	}
	if(p->state() == QAudio::StoppedState) {
		return false;
	}
	return _b;
}

void M_QT_MULTIMEDIA::mute_sound()
{
	if(!(m_mute.load()) && (m_config_ok.load())) {
	#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		std::shared_ptr<QAudioSink> p = m_audioOutputSink;
	#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
	#endif
		std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;

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

void M_QT_MULTIMEDIA::unmute_sound()
{
	if((m_mute.load()) && (m_config_ok.load())) {
	#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		std::shared_ptr<QAudioSink> p = m_audioOutputSink;
	#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
	#endif
		if(p.get() != nullptr) {
			switch(p->state()) {
			case QAudio::SuspendedState:
				emit sig_discard_audio();
				emit sig_resume_audio();
				break;
			default:
				break;
			}
		}
	}
	m_mute = false;
}


void M_QT_MULTIMEDIA::do_discard_sound()
{
//	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> q = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> q = m_audioOutputSink;
#endif
	std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;
	if(fio.get() != nullptr) {
		fio->reset();
	}
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
		std::shared_ptr<SOUND_BUFFER_QT> fio = m_fileio;
		if(fio.get() != nullptr) {
			fio->reset();
		}
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


int64_t M_QT_MULTIMEDIA::update_sound(void* datasrc, int samples)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
	#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
	#endif
	std::shared_ptr<SOUND_BUFFER_QT> q = m_fileio;
	int64_t sample_bytes = ((int64_t)samples) * ((int64_t)(m_channels.load() * m_wordsize.load()));

	if(sample_bytes <= 0) {
		return 0;
	}
//	if(drv.get() == nullptr) {
//		return samples;
//	}
//	if((drv->state() == QAudio::StoppedState) || (drv->state() == QAudio::SuspendedState)) { // OK?
//		return samples; // OK?
//	}
	if(q.get() != nullptr) {
		int64_t wrote = 0;
		wrote = q->write_to_buffer((const char *)datasrc, sample_bytes);
		if(wrote <= 0) {
			return 0;
		}
		wrote = wrote / ((int64_t)(m_channels.load() * m_wordsize.load()));
		return wrote;
	}
	return 0;
}

	/* SOUND_MODULE::OUTPUT */
	}
/* SOUND_MODULE */

}
