
#include "./osd_sound_mod_qtmultimedia.h"

SOUND_OUTPUT_MODULE_QTMULTIMEDIA::SOUND_OUTPUT_MODULE_QTMULTIMEDIA(
		OSD_BASE *parent,
		USING_FLAGS *pflags,
		CSP_Logger *logger,
		int base_rate,
		int base_latency_ms,
		int base_channels,
		void *extra_configvalues)
		:
	SOUND_OUTPUT_MODULE_QTMULTIMEDIA(
		fileio_ptr, parent, pflags, logger,
		base_rate, base_latency_ms, base_channels,
		extra_configvalues)
{
	m_audioOutput = nullptr;
	m_samples = 0;
	reconfig_sound(base_rate, base_channels, base_latency_ms);
}

SOUND_OUTPUT_MODULE_QTMULTIMEDIA::~SOUND_OUTPUT_MODULE_QTMULTIMEDIA()
{
	release_sound();
}

bool SOUND_OUTPUT_MODULE_QTMULTIMEDIA::real_reconfig_sound(int& rate,int& channels,int& latency_ms)
{
	if((rate <= 0) || (channels < 1) || (latency_ms < 10)) {
		return false;
	}
	m_samples = (rate * latency_ms) / 1000;
	initialize_sound(rate, m_samples, &m_rate, &m_samples);
	if((m_rate <= 0) || (m_samples <= 0)) {
		return false;
	}
	return m_config_ok;
}

void SOUND_OUTPUT_MODULE_QTMULTIMEDIA::initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples)
{
	QAudioFormat desired;
	if(m_audioOutputSink != nullptr) {
		m_audioOutputSink->stop();
		delete m_audioOutputSink;
		m_audioOutputSink = nullptr;
		sound_us_before_rendered = 0;
	}
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	desired.setChannelCount(m_channels);
	desired.setSampleRate(m_rate);
	desired.setSampleFormat(QAudioFormat::Int16);
	// ToDo
	desired.setChannelConfig(QAudioFormat::ChannelConfigStereo);
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	desired.setChannelCount(m_channels);
	desired.setSampleRate(m_rate);
	desired.setSampleSize(16);
	desired.setSampleType(QAudioFormat::SignedInt);
	#if Q_BYTE_ORDER == Q_BIG_ENDIAN	
	desired.setByteOrder(QAudioFormat::BigEndian);
	#else
	desired.setByteOrder(QAudioFormat::LittleEndian);
	#endif	
#endif
//	if(!(m_audioOutputDevice.isFormatSupported(desired))) {
//		desired = m_audioOutputDevice.preferredFormat();
//	}
	m_audioOutputFormat = desired;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	m_audioOutputSink = new QAudioSink(m_audioOutputDevice, m_audioOutputFormat, this);
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	m_audioOutputSink = new QAudioOutput(m_audioOutputDevice, m_audioOutputFormat, this);
#endif
	if(samples <= 0) return;

	rate = m_audioOutputFormat.sampleRate();
	//int channels = m_audioOutputFormat.sampleRate();
	if(rate <= 0) rate = 8000;
	
	sound_us_before_rendered = 0;
	
	if(m_audioOutput != nullptr)
	{
		m_config_ok =  m_audioOutput->resize(samples * channels * sizeof(int16_t) * 4);
	} else {
		m_audioOutput = new SOUND_BUFFER_QT(samples * channels * sizeof(int16_t) * 4);
		m_config_ok =  (m_audioOutput != nullptr);
	}
	if(!(m_config_ok)) return;
	
	m_samples = samples;

	if(m_audioOutput->isOpen()) {
		m_audioOutput->close();
	}
	if(m_audioOutput != nullptr) {
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		m_audioOutput->open(QIODeviceBase::ReadWrite | QIODeviceBase::Truncate | QIODeviceBase::Unbuffered);
	#else
		m_audioOutput->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Unbuffered);
	#endif
		m_audioOutput->reset();
		if(m_audioOutputSink != nullptr) {
			#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			m_audioOutputSink->start(m_audioOutput);
			#endif
			sound_us_before_rendered = 0;
		}
	}
	
	m_samples = samples;
	m_rate = rate;
	
	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,"Sample rate=%d samples=%d\n", m_rate, m_samples);

	sound_ok = sound_started = now_mute = false;
	if(presented_rate != nullptr) {
		*presented_rate = m_rate;
	}
	if(presented_samples != nullptr) {
		*presented_samples = m_samples;
	}
	
	if(m_audioOutputSink != nullptr) {
		sound_ok = true;
		sound_initialized = true;
		if(p_config != nullptr) {
			double _ll = (double)(p_config->general_sound_level + INT16_MAX) / 65535.0;
			m_audioOutputSink->setVolume(_ll);
		}
		connect(m_audioOutputSink, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleAudioOutputStateChanged(QAudio::State)));
	} else {
		sound_ok = false;
		sound_initialized = false;
		m_audioOutput = nullptr;
		sound_us_before_rendered = 0;
	}
	if(sound_ok) {
		connect(this, SIGNAL(sig_sound_start()),  this, SLOT(do_sound_start()), Qt::QueuedConnection);
		connect(this, SIGNAL(sig_sound_resume()),  m_audioOutputSink, SLOT(resume()), Qt::QueuedConnection);
		connect(this, SIGNAL(sig_sound_suspend()), m_audioOutputSink, SLOT(suspend()), Qt::QueuedConnection);
		connect(this, SIGNAL(sig_sound_stop()),    m_audioOutputSink, SLOT(stop()), Qt::QueuedConnection);
		
	}
	
//	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,
//						  "Sound OK: BufSize = %d", outbuffer_length);


}


void SOUND_OUTPUT_MODULE_QTMULTIMEDIA::release_sound()
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	sound_exit = true;
	sound_ok = false;
	sound_initialized = false;
	if(m_audioOutputSink != nullptr) {
		delete m_audioOutputSink;
		m_audioOutputSink = nullptr;
	}
	if(m_audioOutput != nullptr) {
		if(m_audioOutput->isOpen()) {
			m_audioOutput->close();
		}
		delete m_audioOutput;
		m_audioOutput = nullptr;
	}
}

void SOUND_OUTPUT_MODULE_QTMULTIMEDIA::do_sound_start()
{
	if(m_audioOutput != nullptr) {
		m_audioOutput->reset();
		if(m_audioOutputSink != nullptr) {
			m_audioOutputSink->start(m_audioOutput);
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


