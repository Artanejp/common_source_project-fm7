
#include "./osd_sound_mod_qtmultimedia.h"

SOUND_OUTPUT_MODULE_QTMULTIMEDIA::SOUND_OUTPUT_MODULE_QTMULTIMEDIA(
		OSD_BASE *parent,
		_TCHAR* device_name,
		int base_rate,
		int base_latency_ms,
		int base_channels,
		void *extra_configvalues)
		:
	SOUND_OUTPUT_MODULE_QTMULTIMEDIA(
		parent,
		device_name,
		base_rate,
		base_latency_ms,
		base_channels,
		extra_configvalues)
{

	if(initialize_driver()) {
		m_device_name = set_device_sound((const _TCHAR*)m_device_name.c_str(), m_rate, m_channels, m_latency_ms);
		m_config_ok = true;
	}

}

SOUND_OUTPUT_MODULE_QTMULTIMEDIA::~SOUND_OUTPUT_MODULE_QTMULTIMEDIA()
{
}

void SOUND_OUTPUT_MODULE_QTMULTIMEDIA::set_audio_format(QAudioDevice dest_device, QAudioFormat& desired, int& channels, int& rate)
{
	if(dest_device.minimumChannelCount() > channels) {
		channels = dest_device.minimumChannelCount();
	} else if(dest_device.maximumChannelCount() < channels) {
		channels = dest_device.maximumChannelCount();
	}
	if(dest_device.minimumSampleRate() > rate) {
		rate = dest_device.minimumSampleRate();
	} else if(dest_device.maximumSampleRate() < rate) {
		rate = dest_device.maximumSampleRate();
	}
	desired.setSampleRate(rate);
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
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
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	desired.setSampleSize(16);
	desired.setSampleType(QAudioFormat::SignedInt);
	#if Q_BYTE_ORDER == Q_BIG_ENDIAN	
	desired.setByteOrder(QAudioFormat::BigEndian);
	#else
	desired.setByteOrder(QAudioFormat::LittleEndian);
	#endif
#endif
	desired.setChannelCount(channels);	
}

bool SOUND_OUTPUT_MODULE_QTMULTIMEDIA::initialize_driver()
{
	bool result = false;

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	m_audioOutputDevice = QMediaDevices::defaultAudioOutput();
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	m_audioOutputDevice = QAudioDeviceInfo::defaultOutputDevice();
#endif
	m_audioOutputFormat = m_audioOutputDevice.preferredFormat();
	int _channels = m_channels.load();
	int _rate = m_rate.load();
	set_audio_format(m_audioOutputDevice, m_audioOutputFormat, _channels, _rate);
	m_channels = _channels;
	m_rate = _rate;
	
	
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	m_audioOutputSink.reset(new QAudioSink(m_audioOutputDevice, m_audioOutputFormat, this));
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	m_audioOutputSink.reset(new QAudioOutput(m_audioOutputDevice, m_audioOutputFormat, this));
#endif	
	result = ((m_audioOutputSink.get() != nullptr) /* || (m_audioInputSource.get() != nullptr) */);
	if(result) {
		m_channels = m_audioOutputDevice.channelsCount();
		m_rate = m_audioOutputDevice.sampleRate();
		m_config_ok = true;
	}
	m_samples = ((qint64)(m_latency_ms.load()) * (qint64)(m_rate.load())) / 1000;
	if(m_samples.load() <= 0) {
		m_samples = 4800;
	}
	m_audioOutput.reset(new SOUND_BUFFER_QT(m_samples.load() * (qint64)(m_channels.load()) * sizeof(int16_t) * 4));
	
	return result;
}

const std::string SOUND_OUTPUT_MODULE_QTMULTIMEDIA::set_device_sound(const _TCHAR* driver_name, int& rate,int& channels,int& latency_ms)
{
	if(m_audioOutputSink.get() == nullptr) {
		return (const std::string)(std::string(""));
	}
	QString _name = QString::fromUtf8(driver_name);
	QList<QAudioDevice> _list = QMediaDevices::audioOutputs();
	QAudioDevice dest_device = m_audioOutputDevice;
	if(_name == QString::fromUtf8("Default")) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		dest_device = QMediaDevices::defaultAudioOutput();
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		dest_device = QAudioDeviceInfo::defaultOutputDevice();
#endif		
	} else {
		for(auto i = _list.begin(); i != _list.end(); ++i) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
			if((*i).description().compare(_name) == 0) {
				dest_device = *i;
				break;
			}
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			if((*i).deviceName().compare(_name) == 0) {
				dest_device = *i;
				break;
			}
#endif
		}
	}
	bool req_reinit = false;
	if(dest_device != m_audioOutputDevice) {
		req_reinit = true;
	} else {
		if((m_audioOutputSink->format().channelCount() != channels) ||
		   (m_audioOutputSink->format().sampleRate() != rate)) {
			req_reinit = true;
		}
	}
	bool req_restart = false;
	if(req_reinit) {
		if(m_audioOutputSink->state() != QAudio::StoppedState) {
			m_audioOutputSink->stop();
			req_restart = true;
		}
		QAudioFormat desired = dest_device.preferredFormat();
		set_audio_format(dest_device, desired, channels, rate);
		
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		m_audioOutputSink.reset(new QAudioSink(dest_device, desired, this));
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		m_audioOutputSink.reset(new QAudioOutput(dest_device, desired, this));
#endif
		if(m_audioOutputSink.get() != nullptr) {
			m_audioOutputDevice = dest_device;
			m_audioOutputFormat = desired;
			m_channels = dest_device.channelsCount();
			m_rate = dest_device.sampleRate();
			m_config_ok = true;
		} else {
			m_config_ok = false;
		}
	} else if(m_audioOutputSink->state() != QAudio::StoppedState) {
		if(m_latency_ms.load() != latency_ms) {
			m_audioOutputSink->stop();
			req_restart = true;
		}
	}

	if((req_reinit) || (m_latency_ms.load() != latency_ms)) {
		int64_t _samples =
			((int64_t)m_rate * m_latency_ms.load()) / 1000;
		if(m_audioOutput.get() == nullptr) {
			m_audioOutput.reset(new SOUND_BUFFER_QT(m_samples.load() * (qint64)(m_channels.load()) * sizeof(int16_t) * 4));
			m_config_ok = (m_audioOutput.get() != nullptr);
		} else {
			if(m_audioOutput->isOpen()) {
				m_audioOutput->close();
			}
			m_config_ok =  m_audioOutput->resize(samples * channels * sizeof(int16_t) * 4);
		}
		if(m_config_ok.load()) {
			m_samples = _samples;
			m_latency_ms = latency_ms;
		}
	}
	if((req_restart) && (m_audioOutputSink.get() != nullptr)) {
		m_audioOutputSink->start(m_audioOutput.get());		
	}
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	return m_audioOutputDevice.description().toStdString();
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	return m_audioOutputDevice.deviceName().toStdString();
#else
	return (const std::string)(std::string(""));
#endif	
}

bool SOUND_OUTPUT_MODULE_QTMULTIMEDIA::real_reconfig_sound(int& rate,int& channels,int& latency_ms)
{
	if((rate <= 0) || (channels < 1) || (latency_ms < 10)) {
		return false;
	}

	int64_t _rate = rate;
	int64_t _samples;
	_samples = (rate * latency_ms) / 1000;
	m_channels = channels;
	initialize_sound(_rate, _samples, &_rate, &_samples);
	if((_rate <= 0) || (_samples <= 0)) {
		rate = 48000;
		samples = 0;
		m_channels = 1;
		m_config_ok = false;
		return false;
	}
	rate = _rate;
	samples = _samples;
	m_config_ok = true;
	return true;
}

void SOUND_OUTPUT_MODULE_QTMULTIMEDIA::initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples)
{
	if(samples <= 0) return;
	if(rate <= 0) rate = 8000;
	int _latency_ms = (samples * 1000) / rate;
	int _channels = m_channels.load();	
	if((rate != m_rate.load()) || (samples != m_samples.load())) {
		m_device_name = set_device_sound((const _TCHAR *)(m_device_name.c_str()), rate, _channels, _latency_ms);
	}
	
	if(!(m_config_ok)) return;
	
	if(m_audioOutput.get() != nullptr) {
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		m_audioOutput->open(QIODeviceBase::ReadWrite | QIODeviceBase::Truncate | QIODeviceBase::Unbuffered);
	#else
		m_audioOutput->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Unbuffered);
	#endif
		sound_ok = sound_started = now_mute = false;
		m_audioOutput->reset();
		if(m_audioOutputSink.get() != nullptr) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			m_audioOutputSink->start(m_audioOutput);
			sound_started = true;
#endif
			sound_us_before_rendered = 0;
		}
	}
	if(presented_rate != nullptr) {
		*presented_rate = m_rate.load();
	}
	if(presented_samples != nullptr) {
		*presented_samples = m_samples.load();
	}
	
	sound_ok = true;
	sound_initialized = true;
	
	if(p_config != nullptr) {
		double _ll = (double)(p_config->general_sound_level + INT16_MAX) / 65535.0;
		m_audioOutputSink->setVolume(_ll);
	}
	connect(m_audioOutputSink, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleAudioOutputStateChanged(QAudio::State)));

	connect(this, SIGNAL(sig_sound_start()),  this, SLOT(do_sound_start()));
	connect(this, SIGNAL(sig_sound_resume()),  m_audioOutputSink, SLOT(resume()));
	connect(this, SIGNAL(sig_sound_suspend()), m_audioOutputSink, SLOT(suspend()));
	connect(this, SIGNAL(sig_sound_stop()),    m_audioOutputSink, SLOT(stop()));

	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,"Sample rate=%d samples=%d\n", m_rate, m_samples);	
}


void SOUND_OUTPUT_MODULE_QTMULTIMEDIA::release_sound()
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	sound_exit = true;
	sound_ok = false;
	sound_initialized = false;
	m_audioOutputSink.reset();

	if(m_audioOutput.get() != nullptr) {
		if(m_audioOutput->isOpen()) {
			m_audioOutput->close();
		}
	}
	m_audioOutput.reset();
}

void SOUND_OUTPUT_MODULE_QTMULTIMEDIA::do_sound_start()
{
	if(m_audioOutput.get() != nullptr) {
		m_audioOutput->reset();
		if(m_audioOutputSink.get() != nullptr) {
			m_audioOutputSink->start(m_audioOutput.get());
		}
	}
}


void OSD_BASE::update_sound(int* extra_frames)
{
	*extra_frames = 0;
	
	now_mute = false;
	if(sound_ok) {
		//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, "Sink->bytesFree() = %d", m_audioOutputSink->bytesFree());
		
		const int64_t sound_us_now = (int64_t)m_audioOutputSink->elapsedUSecs();
		const int64_t  _period_usec = (((int64_t)sound_samples * (int64_t)10000) / (int64_t)sound_rate) * 100;
		int64_t _diff = sound_us_now - (int64_t)sound_us_before_rendered;
		if((_diff < 0) && ((INT64_MAX - (int64_t)sound_us_before_rendered + 1) <= _period_usec)) {
			// For uS overflow
			_diff = sound_us_now + (INT64_MAX - (int64_t)sound_us_before_rendered + 1);
		}
		//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, "update_sound(): _diff=%lld _period_usec=%lld", _diff, _period_usec);
		int now_mixed_ptr = 0;
		if(m_OSD != nullptr) {
			VM_TEMPLATE* p_vm = m_OSD->get_vm();
			if(vm != nullptr) {
				now_mixed_ptr = vm->get_sound_buffer_ptr();
			}
		}
		if(now_mixed_ptr < ((m_samples * 90) / 100)) {
			// Render even emulate 95% of latency when remain seconds is less than 2m Sec.
			return;
		}
		if((sound_started) && (_diff < (_period_usec - 1000))) {
			return;
		}
		qint64 left = 0;
		qint64 buffer_len = m_samples * m_channels * sizeof(int16_t) * 4;
		if(m_audioOutput != nullptr) {
			left = _size - m_audioOutput->bytesAvailable();
		}
		if(left < (sound_samples * 2 * sizeof(int16_t))) {
			return;
		}		   
		// Input
		int16_t* sound_buffer = (int16_t*)create_sound(extra_frames);		
		if(sound_buffer != nullptr) {
			if(!(sound_started)) {
				m_audioOutput->reset();
		#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
				m_audioOutputSink->start(m_audioOutput);
		#else
				emit sig_sound_start();
		#endif

			}
			sound_started = true;
		}
		if(now_record_sound || now_record_video) {
			if(sound_samples > rec_sound_buffer_ptr) {
				int samples = sound_samples - rec_sound_buffer_ptr;
				int length = samples * sizeof(int16_t) * 2; // stereo
				rec_sound_bytes += length;
				if(now_record_video) {
					//AGAR_DebugLog(AGAR_LOG_DEBUG, "Push Sound %d bytes\n", length);
					emit sig_enqueue_audio((int16_t *)(&(sound_buffer[rec_sound_buffer_ptr * 2])), length);
				}
				// record sound
				if(now_record_sound) {
					rec_sound_fio->Fwrite(sound_buffer + rec_sound_buffer_ptr * 2, length, 1);
				}
				//if(now_record_video) {
				//	// sync video recording
				//	static double frames = 0;
				//	static int prev_samples = -1;
				//	static double prev_fps = -1;
				//	double fps = this->vm_frame_rate();
				//	frames = fps * (double)samples / (double)sound_rate;
				//}
				//printf("Wrote %d samples ptr=%d\n", samples, rec_sound_buffer_ptr);
				rec_sound_buffer_ptr += samples;
				if(rec_sound_buffer_ptr >= sound_samples) rec_sound_buffer_ptr = 0;
			}
		}
		//if(sound_initialized) return;
		if(sound_buffer != nullptr) {
			if((m_audioOutput != nullptr) /*&& (m_audioOutputSink != nullptr)*/) {
				// ToDo: Not Int16.
				//qint64 sound_len = sound_samples * sound_rate * 2 * wordsize;
				qint64 sound_len = m_samples * m_channels * sizeof(int16_t);

				qint64 written = 0;
				if(m_using_flags != nullptr) {
					config_t* p_config = m_using_flags->get_config_ptr();
					if(p_config != nullptr) {
						double _ll = (double)(p_config->general_sound_level + INT16_MAX) / 65535.0;
						m_audioOutputSink->setVolume(_ll);
					}
				}
				qint64 _result = m_audioOutput->write((const char *)sound_buffer, sound_len);
				sound_us_before_rendered = m_audioOutputSink->elapsedUSecs();
			}
		}
	}
}

void OSD_BASE::mute_sound()
{
	if(!(now_mute) && (sound_ok)) {
		if(m_audioOutputSink != nullptr) {
			switch(m_audioOutputSink->state()) {
			case QAudio::ActiveState:
			case QAudio::IdleState:
				emit sig_sound_suspend();
				break;
			default:
				break;
			}
			if(m_audioOutput != nullptr) {
				m_audioOutput->reset();
			}
			
		}
	}
	now_mute = true;
}

void OSD_BASE::stop_sound()
{
	if((sound_ok) && (sound_started)) {
		if(m_audioOutputSink != nullptr) {
			switch(m_audioOutputSink->state()) {
			case QAudio::ActiveState:
			case QAudio::IdleState:
			case QAudio::SuspendedState:
				emit sig_stop_sound();
				break;
			default:
				break;
			}
			if(m_audioOutput != nullptr) {
				m_audioOutput->reset();
			}
		}
	}
}


