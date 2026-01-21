/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[Qt/SDL sound ]
*/
/*
  Note: 20220715 K.Ohta : This will be based on QtMultimedia's audio driver,
		But this still has a lot of delay.Will fix or discard.
  Quote from "Audio Overview" of Qt6.3.1 's documentation;
	https://doc.qt.io/qt-6/audiooverview.html
	Push and Pull
		The low level audio classes can operate in two modes - push and pull.
		In pull mode, the audio device is started by giving it a QIODevice.
		For an output device, the QAudioSink class will pull data from the
		QIODevice (using QIODevice::read()) when more audio data is required.
		Conversely, for pull mode with QAudioSource, when audio data is available
		then the data will be written directly to the QIODevice.

		In push mode, the audio device provides a QIODevice instance that can be
		written or read to as needed.
		Typically, this results in simpler code but more buffering,
		which may affect latency.
*/

#include "../emu.h"
#include "../fileio.h"
#include "../fifo.h"
#include "../types/util_sound.h"
#include "../types/util_endians.h"

#include <SDL.h>

#include "qt_main.h"
#include "gui/menu_flags.h"

#include <QDateTime>
#include <QByteArray>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QApplication>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioSink>
#include <QAudioSource>
#else
/* Qt5.x */
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QAudioInput>
#endif

#include <cstdint>
#include <cmath>

#include "emu_thread_tmpl.h"

// ToDo: Implement for QAudio::State.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void OSD_BASE::do_sound_output_state_changed(QtAudio::State state)
#else
void OSD_BASE::do_sound_output_state_changed(QAudio::State state)
#endif
{
	std::shared_ptr<AudioSink>   drv;
	drv = m_sound_sink;
	
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QtAudio::State oldState = m_sound_sink_state.load();
	#else
	QAudio::State oldState = m_sound_sink_state.load();
	#endif
	bool need_update_elapsed = false;
	bool need_update_processed = false;
	bool need_check_data_empty = false;
	
	switch(state) {
	case QtAudio::ActiveState:
		if(oldState != state) {
			switch(oldState) {
			case QtAudio::StoppedState: // Stop -> Active
				need_update_elapsed = true;
				need_update_processed = true;
				m_sound_sink_empty = true;
				break;
			case QtAudio::SuspendedState: // Suspend -> Active
				need_update_elapsed = true;
				//need_update_processed = true;
				need_check_data_empty = true;
				break;
			case QtAudio::IdleState: // Some data was in.
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			case QAudio::InterruptedState:
		#endif
				if(!(m_sound_sink_started.load())) { // Failsafe
					need_update_elapsed = true;
					need_update_processed = true;
				}
				m_sound_sink_empty = false;
				break;
			default: /* Failsafe */
				need_update_elapsed = true;
				need_update_processed = true;
				need_check_data_empty = true;
				break;
			}
			m_sound_sink_suspended = false;
		}
		break;
	case QtAudio::SuspendedState:
		/* ToDo: Sample counting. */
		if(oldState != QtAudio::StoppedState) {
			m_sound_sink_suspended = true;
		} else {
			m_sound_sink_started = false;
		}
		break;
	case QtAudio::StoppedState:
		m_sound_sink_started   = false;
		m_sound_sink_empty     = true;
		m_sound_sink_suspended = false;
		need_update_elapsed = true;
		need_update_processed = true;
		break;
	case QtAudio::IdleState: /* Maybe Data empty */
		m_sound_sink_empty = true;
		break;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	case QtAudio::InterruptedState:
		break;
#endif
	default:
		return; /* Don't update if undefined behavior. */
		break;
	}
		
	if(need_check_data_empty) {
		m_sound_sink_empty = check_sound_empty(false);
	}
	if(need_update_elapsed) {
		m_sink_prev_elapsed_usec = get_sound_elapsed_usecs(false);
	}
	if(need_update_processed) {
		m_sink_prev_processed_usec = get_sound_processed_usecs(false);
	}
	sound_debug_log(_T("Output State Changed from %s to %s."),
					   get_sound_state_name(oldState),
					   get_sound_state_name(state));
			
	m_sound_sink_state = state;
	return;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void OSD_BASE::do_sound_capture_state_changed(QtAudio::State state)
#else
void OSD_BASE::do_sound_capture_state_changed(QAudio::State state)
#endif	
{
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QtAudio::State oldState = m_sound_source_state.load();
	#else
	QAudio::State oldState = m_sound_source_state.load();
	#endif
	std::shared_ptr<AudioSource>   drv;
	drv = m_sound_source;
	
	bool need_update_elapsed = false;
	bool need_update_processed = false;
	bool need_check_data_empty = false;
	
	switch(state) {
	case QtAudio::ActiveState:
		if(oldState != state) {
			switch(oldState) {
			case QtAudio::StoppedState: // Stop -> Active
				need_update_elapsed = true;
				need_update_processed = true;
				m_sound_source_empty = true;
				break;
			case QtAudio::SuspendedState: // Suspend -> Active
				need_update_elapsed = true;
				//need_update_processed = true;
				need_check_data_empty = true;
				break;
			case QtAudio::IdleState: // Some data was in.
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			case QAudio::InterruptedState:
		#endif
				if(!(m_sound_source_started.load())) { // Failsafe
					need_update_elapsed = true;
					need_update_processed = true;
				}
				m_sound_source_empty = false;
				break;
			default: /* Failsafe */
				need_update_elapsed = true;
				need_update_processed = true;
				need_check_data_empty = true;
				break;
			}
			m_sound_source_started = true;
			m_sound_source_suspended = false;
		}
		break;
	case QtAudio::SuspendedState:
		/* ToDo: Sample counting. */
		if(oldState != QtAudio::StoppedState) {
			m_sound_source_suspended = true;
		} else {
			m_sound_source_suspended = false;
			m_sound_source_started = false;
		}
		break;
	case QtAudio::StoppedState:
		m_sound_source_started   = false;
		m_sound_source_empty     = true;
		m_sound_source_suspended = false;
		need_update_elapsed = true;
		need_update_processed = true;
		break;
	case QtAudio::IdleState: /* Maybe Data empty */
		m_sound_source_empty = true;
		break;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	case QAudio::InterruptedState:
		break;
#endif
	default:
		return; /* Don't update if undefined behavior. */
		break;
	}
		
	if(need_check_data_empty) {
		m_sound_source_empty = check_sound_empty(true);
	}
	if(need_update_elapsed) {
		m_source_prev_elapsed_usec = get_sound_elapsed_usecs(true);
	}
	if(need_update_processed) {
		m_source_prev_processed_usec = get_sound_processed_usecs(true);
	}
	sound_debug_log(_T("Input State Changed from %s to %s."),
					   get_sound_state_name(oldState),
					   get_sound_state_name(state));
	m_sound_source_state = state;
	return;
}

bool OSD_BASE::check_sound_empty(bool is_in)
{
	bool _result = true;
	size_t __size = 0;
	size_t __free = 0;
	if(is_in) {
		std::lock_guard<std::recursive_timed_mutex> _locker(m_sound_source_mutex);
		std::shared_ptr<AudioSource>   drv;
		drv = m_sound_source;
		if(drv.get() != nullptr) {
			__size = drv->bytesAvailable();
		}
		_result = (__size == 0) ? true : false;
	} else {
		std::lock_guard<std::recursive_timed_mutex> _locker(m_sound_sink_mutex);
		std::shared_ptr<AudioSink>    drv;
		drv = m_sound_sink;
		if(drv.get() != nullptr) {
			__free = drv->bytesFree();
			__size = drv->bufferSize();
		}
		_result = (__free >= __size) ? true : false;
	}
	return _result;
}

bool OSD_BASE::check_sound_full(bool is_in)
{
	bool _result = true;
	size_t __free = 0;
	size_t __size = 0;
	if(is_in) {
		std::lock_guard<std::recursive_timed_mutex> _locker(m_sound_source_mutex);
		std::shared_ptr<AudioSource>   drv;
		drv = m_sound_source;
		if(drv.get() != nullptr) {
			__free = drv->bytesAvailable();
			__size = drv->bufferSize();
		}
		_result = (__free >= __size) ? true : false;
	} else {
		std::lock_guard<std::recursive_timed_mutex> _locker(m_sound_sink_mutex);
		std::shared_ptr<AudioSink>    drv;
		drv = m_sound_sink;
		if(drv.get() != nullptr) {
			__free = drv->bytesFree();
			__size = drv->bufferSize();
		}
		_result = ((__free == 0) || (__size == 0)) ? true : false;
	}
	return _result;
}

int64_t OSD_BASE::get_sound_elapsed_usecs(bool is_in)
{
	if(is_in) {
		std::shared_ptr<AudioSource>   drv;
		drv = m_sound_source;
		if(drv.get() != nullptr) {
			return (int64_t)(drv->elapsedUSecs());
		}
	} else {
		std::shared_ptr<AudioSink>    drv;
		drv = m_sound_sink;
		if(drv.get() != nullptr) {
			return (int64_t)(drv->elapsedUSecs());
		}
	}
	return 0;
}

int64_t OSD_BASE::get_sound_processed_usecs(bool is_in)
{
	if(is_in) {
		std::shared_ptr<AudioSource>   drv;
		drv = m_sound_source;
		if(drv.get() != nullptr) {
			return (int64_t)(drv->processedUSecs());
		}
	} else {
		std::shared_ptr<AudioSink>    drv;
		drv = m_sound_sink;
		if(drv.get() != nullptr) {
			return (int64_t)(drv->processedUSecs());
		}
	}
	return 0;
}

void OSD_BASE::sound_debug_log(const char *fmt, ...)
{
	if(m_sound_debug.load()) {
		char strbuf[4096];
		strbuf[0] = '\0';
		va_list ap;
		
		va_start(ap, fmt);
		vsnprintf(strbuf, 4095, fmt, ap);
		va_end(ap);
		QString tmps = QString::fromLocal8Bit(strbuf);
		emit sig_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_SOUND, tmps);
	}
}

void OSD_BASE::reset_sound()
{
	if(m_sound_exit.load()) {
		return;
	}
	std::shared_ptr<AudioSink>     _out_drv;
	std::shared_ptr<AudioSource>   _in_drv;
	_out_drv = m_sound_sink;
	_in_drv  = m_sound_source;

	bool _out_noinit = true;
	bool _in_noinit = true;
	if(m_sound_initialized.load()) {
		if(_out_drv.get() != nullptr) {
			std::lock_guard<std::recursive_timed_mutex> _locker(m_sound_sink_mutex);
			_out_drv->reset(); // Temporally stop.
			m_sound_sink_empty = check_sound_empty(false);
			m_sink_prev_elapsed_usec = get_sound_elapsed_usecs(false);
			m_sink_prev_processed_usec = get_sound_elapsed_usecs(false);
			m_sound_sink_state = _out_drv->state();
			_out_noinit = false;
		}
		if(_in_drv.get() != nullptr) {
			std::lock_guard<std::recursive_timed_mutex> _locker(m_sound_source_mutex);
			_in_drv->reset(); // Temporally stop.
			m_sound_source_empty = check_sound_empty(true);
			m_source_prev_elapsed_usec = get_sound_elapsed_usecs(true);
			m_source_prev_processed_usec = get_sound_elapsed_usecs(true);
			m_sound_source_state = _in_drv->state();
			_in_noinit = false;
		}
	}
	if(_out_noinit) {
		m_sound_sink_state = QtAudio::StoppedState;
		m_sound_sink_empty = true;
		m_sound_sink_started = false;
	}
	if(_in_noinit) {
		m_sound_source_state = QtAudio::StoppedState;
		m_sound_source_empty = true;
		m_sound_source_started = false;
	}
	m_sound_vm_local_usec = 0;
}

	
void OSD_BASE::update_sound(int* extra_frames)
{
	__LIKELY_IF(extra_frames != nullptr) {
		*extra_frames = 0;
	}
	if(m_sound_exit.load()) {
		return;
	}

	std::shared_ptr<AudioSink>   sink_drv = m_sound_sink;
	m_now_mute = false;
	if(m_sound_initialized.load()) {
		// Get sound driver
		int64_t _samples = m_sound_sink_samples.load();
		int64_t _rate    = m_sound_sink_rate.load();
		__UNLIKELY_IF((_rate <= 0) || (_samples <= 0)) {
			return; 
		}
		QtAudio::State _sink_state = QtAudio::StoppedState;
		// Check sound buffer remains
		// Check sound buffer is enough to write.
		double _frame_rate = vm_frame_rate();
		__UNLIKELY_IF(_frame_rate > 500.0) { // !!
			_frame_rate = 500.0;
		}
		__UNLIKELY_IF(_frame_rate < 1.0) {
			_frame_rate = 1.0;
		}
		int64_t _frame_usec = (int64_t)(std::llrint(1.0e6 / _frame_rate));
		int64_t _sample_usec = (_rate * 1000 * 1000) / _samples;
		int64_t _minimum_usec = _sample_usec - _frame_usec; // OK?
		__UNLIKELY_IF(_minimum_usec >= _sample_usec) {
			_minimum_usec = _sample_usec;
		}
		__UNLIKELY_IF(_minimum_usec <= _frame_usec) {
			_minimum_usec = _frame_usec;
		}
		m_sound_vm_local_usec += _frame_usec;
		
		if(sink_drv.get() != nullptr) {
			_sink_state = sink_drv->state();
			if((_sink_state == QtAudio::StoppedState) && (m_sound_sink_started.load())) {
				std::lock_guard<std::recursive_timed_mutex> _locker(m_sound_sink_mutex);
				// Stopped, but initialize completed.
				m_sound_sink_io = sink_drv->start();
				m_sound_sink_started = true;
				return;
			}
		}
		if(m_sound_vm_local_usec.load() <= _minimum_usec) {
			return; // NOP
		}
		
		bool emergency_alloced = false;
		size_t _buffer_bytes = ((size_t)_samples) * 2 * sizeof(int16_t);
		int __extra_frames = 0;
		// Check remain sink buffer enough to send emulated data.
		if((_sink_state != QtAudio::StoppedState) && (sink_drv.get() != nullptr)) {
			if(_buffer_bytes > sink_drv->bytesFree()) {
				return; // lol.
			}
		}
		// Mixing.
		int16_t* sound_buffer = (int16_t*)create_sound(&__extra_frames);
		if(sound_buffer == nullptr) {
			// Render failed.
			sound_buffer = new int16_t[_samples * 2];
			if(sound_buffer == nullptr) {
				return; // Even allocate failed.
			}
			memset(sound_buffer, 0x00, _buffer_bytes);
			emergency_alloced = true;
		}
		if(__extra_frames > 0) {
			_frame_rate = vm_frame_rate();
			_frame_usec = (int64_t)(std::llrint(1.0e6 / _frame_rate));
			int64_t _tmp_usec = ((int64_t)__extra_frames) * _frame_usec;
			m_sound_vm_local_usec += _tmp_usec;
		}
		__LIKELY_IF(extra_frames != NULL) {
			*extra_frames = __extra_frames;
		}
		m_sound_vm_local_usec -= _sample_usec;

		//sound_debug_log(_T("Render %d Samples , Extra frames = %d"), m_sound_samples, __extra_frames);
		if((now_record_sound || now_record_video) && (sound_buffer != nullptr) && (_buffer_bytes > 0)) {
			if(now_record_video) {
				// ToDo: Change endian for Video?
				int16_t* vwp = sound_buffer;
				emit sig_enqueue_audio(vwp, _buffer_bytes);
			}
			if((now_record_sound) && (rec_sound_fio != nullptr)) {
				if(rec_sound_fio->IsOpened()) {
					#if defined(__LITTLE_ENDIAN__)
					int16_t* awp = sound_buffer;
					#else
					// Swap endianness; BIG to LITTLE.
					int16_t awp[_samples * 2];

					for(int i = 0; i < (_samples * 2); i++) {
						pair16_t _tmp;
						_tmp.w = sound_buffer[i];
						_tmp.write_2bytes_le_to((uint8_t*)(&(awp[i])));
					}
					#endif
					if(rec_sound_fio->Fwrite(awp, _buffer_bytes, 1) != 1) {
						// Write error ??
						stop_record_sound();
					} else {
						// OK.
						rec_sound_bytes += _buffer_bytes;
					}
				}
			}
		}
		// ToDo: Convert sound format.
		if(sink_drv.get() != nullptr) {
			std::lock_guard<std::recursive_timed_mutex> _locker(m_sound_sink_mutex);
			QIODevice* wp = m_sound_sink_io;
			_sink_state = sink_drv->state();
			qint64 _result = 0;
			if((_sink_state != QtAudio::StoppedState) && (wp != nullptr)) {
				_result = wp->write((const char *)sound_buffer, _buffer_bytes);
			}
		}
		if(emergency_alloced) {
			delete sound_buffer;
		}
	}
}

AudioDevice OSD_BASE::search_sound_sink(const _TCHAR *name, bool& found)
{
	const size_t max_compare_length = 64;	
	
	found = false;
	QString _name = QString::fromLocal8Bit("");
	__UNLIKELY_IF(name != nullptr) {
		_name = QString::fromLocal8Bit(name).left(max_compare_length);
	}
	AudioDevice default_result;
	
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	default_result = QMediaDevices::defaultAudioOutput();
	#else /* Qt 5.x */
	default_result = QAudioDeviceInfo::defaultOutputDevice();
	#endif
	
	if(_name == QString::fromUtf8("Default")) {
		found = true;
		return QMediaDevices::defaultAudioOutput();
	}
	
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QList<AudioDevice> sink_list = QMediaDevices::audioOutputs();
	#else
	QList<AudioDevice> sink_list = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
	#endif
	
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	for(auto n = sink_list.begin(); n != sink_list.end(); ++n) {
		if((*n).description().left(max_compare_length) == _name) {
			found = true;
			return (*n);
		}
	}
	#else
	for(auto n = sink_list.begin(); n != sink_list.end(); ++n) {
		if((*n).deviceName().left(max_compare_length) == _name) {
			found = true;
			return (*n);
		}
	}
	#endif
	return default_result;
}

std::list<std::string> OSD_BASE::load_sound_output_devices_list()
{
	std::list<std::string> _list;
	_list.clear();
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QList<QAudioDevice> sink_list = QMediaDevices::audioOutputs();
	#else
	QList<QAudioDeviceInfo> sink_list = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
	#endif
	
	for(auto n = sink_list.begin(); n != sink_list.end(); ++n) {
		#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		_list.push_back((*n).description().toStdString());
		#else
		_list.push_back((*n).deviceName().toStdString());
		#endif
	}
	return _list;
}



AudioDevice OSD_BASE::search_sound_source(const _TCHAR *name, bool& found)
{
	const size_t max_compare_length = 64;	
	found = false;
	QString _name = QString::fromLocal8Bit("");
	__UNLIKELY_IF(name != nullptr) {
		_name = QString::fromLocal8Bit(name).left(max_compare_length);
	}
	
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	if(_name == QString::fromUtf8("Default")) {
		found = true;
		return QMediaDevices::defaultAudioInput();
	}
	QList<QAudioDevice> source_list  = QMediaDevices::audioInputs();
	for(auto n = source_list.begin(); n != source_list.end(); ++n) {
		if((*n).description().left(max_compare_length) == _name) {
			found = true;
			return (*n);
		}
	}
	return QMediaDevices::defaultAudioInput();
	
	#else
	
	if(_name == QStrung::fromUtf8("Default")) {
		found = true;
		return QAudioDeviceInfo::defaultInputDevice();
	}
	QList<QAudioDeviceInfo> source_list = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	for(auto n = source_list.begin(); n != source_list.end(); ++n) {
		if((*n).deviceName().left(max_compare_length) == _name) {
			found = true;
			return (*n);
		}
	}
	return QAudioDeviceInfo::defaultInputDevice();
	#endif
}

std::list<std::string> OSD_BASE::load_sound_capture_devices_list()
{
	std::list<std::string> _list;
	_list.clear();
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QList<QAudioDevice> source_list = QMediaDevices::audioInputs();
	#else
	QList<QAudioDeviceInfo> source_list = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	#endif
	
	for(auto n = source_list.begin(); n != source_list.end(); ++n) {
		#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		_list.push_back((*n).description().toStdString());
		#else
		_list.push_back((*n).deviceName().toStdString());
		#endif
	}
	return _list;
}


bool OSD_BASE::setup_sound_sink(QString device_name, int rate, int samples, int& presented_rate, int& presented_samples, bool force)
{
	if(device_name.isEmpty()) {
		device_name = QString::fromUtf8("Default");
	}
	QAudioFormat _fmt;
	_fmt.setSampleRate(rate);
	_fmt.setChannelCount(2);
	_fmt.setChannelConfig(QAudioFormat::ChannelConfigStereo);
	_fmt.setSampleFormat(QAudioFormat::Int16);

	presented_rate = rate;
	presented_samples = samples;
	
	bool found = false;
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QAudioDevice _sink_dev = search_sound_sink((const _TCHAR*)(m_sound_default_sink_name.toLocal8Bit().constData()), found);
	
	if(!(_sink_dev.isFormatSupported(_fmt))) {
		// ToDo: Not support for Int16.
		if(presented_rate > _sink_dev.maximumSampleRate()) {
			presented_rate = _sink_dev.maximumSampleRate();
		}
		if(presented_rate < _sink_dev.minimumSampleRate()) {
			presented_rate = _sink_dev.minimumSampleRate();
		}
	}
	#else
	QAudioDeviceInfo _sink_dev = search_sound_sink((const _TCHAR*)(m_sound_default_sink_name.toLocal8Bit().constData()), found);
	if(!(_sink_dev.isFormatSupported(_fmt))) {
		// ToDo: Not support for Int16.
		_fmt = _sink_dev.nearestFormat(_fmt);
		presented_rate = _fmt.sampleRate();
	}
	#endif
	
	if(device_name != m_sound_default_sink_name) {
			force = true;
	}
	if(m_sound_sink.get() == nullptr) {
		force = true;
	} else {
		if(m_sound_sink->format().sampleRate() != presented_rate) {
			force = true;
		}
	}
	if(presented_rate != rate) {
		int64_t __samples = presented_samples;
		int64_t __rate = rate * 10000;
		__rate = __rate / ((int64_t)presented_rate);
		__samples = (__samples * __rate) / 10000;
		presented_samples = (int)__samples;
		force = true;
	}
	if(force) {
		std::lock_guard<std::recursive_timed_mutex> _locker(m_sound_sink_mutex);
		_fmt.setSampleRate(presented_rate);
		if(m_sound_sink.get() != nullptr) {
			m_sound_sink->stop();
		}
		m_sound_sink.reset(new AudioSink(_sink_dev, _fmt));
		if(m_sound_sink.get() != nullptr) {
			m_sound_initialized = true;

			m_sound_sink->setBufferSize((((size_t)presented_samples) * 2 * sizeof(int16_t)) * 2);
			#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			connect(m_sound_sink.get(), SIGNAL(stateChanged(QtAudio:State)), this, SLOT(do_sound_output_state_changed(QtAudio::State)));
			#else
			connect(m_sound_sink.get(), SIGNAL(stateChanged(QAudio:State)), this, SLOT(do_sound_output_state_changed(QAudio::State)));
			#endif
			connect(this, SIGNAL(sig_sound_sink_finished()), m_sound_sink.get(), SLOT(deleteLater()));
			
			m_sound_sink_io = m_sound_sink->start(); // GO!
			m_sound_sink_started = true;
			m_sound_default_sink_name = device_name;
		} else {
			m_sound_initialized = false;
			presented_rate = 0;
			presented_samples = 0;
			m_sound_sink_started = false;
			m_sound_default_sink_name.clear();
		}

	}
	return ((force) && (m_sound_sink.get() != nullptr));
}

bool OSD_BASE::setup_sound_source(QString device_name, int rate, int samples, int& presented_rate, int& presented_samples, bool force)
{
	if(device_name.isEmpty()) {
		device_name = QString::fromUtf8("Default");
	}
	return false; // ToDo.
}

void OSD_BASE::initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples)
{
	// If sound driver hasn't initialized, initialize.
	m_sound_exit = false;
		
	if((m_sound_sink.get() == nullptr)  ||
	   (m_sound_sink_rate.load() != rate) ||
	   (m_sound_sink_samples.load() != samples)) {
		//m_sound_ok = false;
		reset_sound();
		if(m_sound_sink.get() != nullptr) {
			std::lock_guard<std::recursive_timed_mutex> _locker(m_sound_sink_mutex);
			m_sound_sink->stop();
			m_sound_sink->deleteLater();
			m_sound_sink.reset();
			m_sound_sink_io = nullptr;
		}

		m_sound_sink_started = false;
		m_sound_source_started = false;
		m_sound_initialized = false;
		m_sound_sink_started = false;
		m_sound_sink_empty = true; // OK?
		m_sound_sink_suspended = false; // OK?
		
		m_sound_source_started = false;
		m_sound_source_empty = false; // OK?
		m_sound_source_suspended = false; // OK?

		// Read
		int prate, psamples;
		setup_sound_sink(m_sound_default_sink_name, rate, samples, prate, psamples, true);
		
		emit sig_update_sound_outputs_list();
		
		if(presented_rate != nullptr) {
			*presented_rate = prate;
		}
		if(presented_samples != nullptr) {
			*presented_samples = psamples;
		}
		m_sound_sink_rate = prate;
		m_sound_sink_samples = psamples;
		m_sink_prev_elapsed_usec = get_sound_elapsed_usecs(false);
		m_sink_prev_processed_usec = get_sound_processed_usecs(false);
		sound_debug_log(_T("OSD::%s rate=%d samples=%d m_sound_driver=%llx"), __func__, prate, samples, (uintptr_t)(m_sound_sink.get()));
	}
}

void OSD_BASE::release_sound()
{
	std::lock_guard<std::recursive_timed_mutex> _olocker(m_sound_sink_mutex);
	std::lock_guard<std::recursive_timed_mutex> _ilocker(m_sound_source_mutex);
	// ToDo: Sound Input
	// release Qt Multimedia sound
	m_sound_exit = true;
	m_sound_initialized = false;
	m_sound_sink_started = false;
	m_sound_sink_empty = true; // OK?
	m_sound_sink_suspended = false; // OK?
	
	m_sound_source_started = false;
	m_sound_source_empty = false; // OK?
	m_sound_source_suspended = false; // OK?

	if(m_sound_sink.get() != nullptr) {
		m_sound_sink->stop();
	}
	if(m_sound_source.get() != nullptr) {
		m_sound_source->stop();
	}
	emit sig_sound_sink_finished();

	m_sound_sink.reset();
	m_sound_source.reset();
	m_sound_sink_io = nullptr;
	m_sound_source_io = nullptr;
	m_sound_default_sink_name.clear();
	m_sound_default_source_name.clear();
}


void OSD_BASE::do_update_master_volume(int level)
{
	level = std::min(std::max(level, (int)INT16_MIN), (int)INT16_MAX);
	if(p_config != nullptr) {
		p_config->general_sound_level = level;
	}
	std::shared_ptr<AudioSink>   sink_drv = m_sound_sink;
	if(sink_drv.get() != nullptr) {
		double xlevel;
		__UNLIKELY_IF(level <= (INT16_MIN + 128)) {
			xlevel = 0.0;
		} else if(level >= (INT16_MAX - 128)) {
			xlevel = 1.0;
		} else {
			level += INT16_MAX;
			level >>= 7;
			xlevel = ((double)level) / ((double)(UINT16_MAX >> 7));
		}
		sink_drv->setVolume(xlevel);
	}
}

void OSD_BASE::do_set_host_sound_output_device(QString device_name)
{
	if(device_name.isEmpty()) return;
	bool found = false;
	AudioDevice drv = search_sound_sink((const _TCHAR*)(device_name.toLocal8Bit().constData()), found);
	int rate = m_sound_sink_rate.load();
	int samples = m_sound_sink_samples.load();
	int prate = rate;
	int psamples = samples;
	// ToDo: Need conversion.
	setup_sound_sink(device_name, rate, samples, prate, psamples, false);
	if((prate != rate) || (psamples != samples)) {
		// ToDo: Force to reinitialize VM or stretch.
		
	}
	m_sound_sink_rate = prate;
	m_sound_sink_samples = psamples;
	
}

const _TCHAR *OSD_BASE::get_sound_device_name(int num)
{
	if(num <= 0) { // Special
		return _T("Default");
	}
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QList<QAudioDevice> _l = QMediaDevices::audioOutputs();
	if(_l.size() >= num) {
		return (const _TCHAR *)(_l.at(num - 1).description().toLocal8Bit().constData());
	}
	#else /* Qt 5.x */
	QList<QAudioDeviceInfo> _l = QAudioDeviceInfo::availableDevices(QAudio::AudioOutputName);
	if(_l.size() >= num) {
		return (const _TCHAR *)(_l.at(num - 1).deviceName().toLocal8Bit().constData());
	}
	#endif
	return (const _TCHAR *)nullptr;
}

void OSD_BASE::init_sound_device_list()
{
	sound_capture_devices_list.clear();
	sound_output_devices_list.clear();
	emit sig_clear_sound_outputs_list();
	//emit sig_clear_sound_inputs_list();
	
	
	do_update_sound_output_devices_list();
	//do_update_sound_capture_devices_list();
	
	// ToDo: Capturing (SOURCE)
}

void OSD_BASE::do_update_sound_output_devices_list()
{
	sound_output_devices_list.clear();
	std::list<std::string> _l = load_sound_output_devices_list();
	int _xi = 1;
	for(auto s = _l.begin(); s != _l.end(); ++s) {
		if(!((*s).empty())) {
			sound_output_devices_list.push_back((*s));
			sound_debug_log("SOUND OUTPUT DEVICE#%03d %s", _xi, (*s).c_str());
			_xi++;
		}
	}
	emit sig_update_sound_outputs_list();
}

void OSD_BASE::do_update_sound_capture_devices_list()
{
	sound_capture_devices_list.clear();
	std::list<std::string> _l = load_sound_capture_devices_list();
	int _xi = 1;
	for(auto s = _l.begin(); s != _l.end(); ++s) {
		if(!((*s).empty())) {
			sound_capture_devices_list.push_back((*s));
			sound_debug_log("SOUND CAPTURE DEVICE#%03d %s", _xi, (*s).c_str());
			_xi++;
		}
	}
	emit sig_update_sound_inputs_list();
}


void OSD_BASE::unmute_sound()
{
	if((m_now_mute.load()) && (m_sound_initialized.load())) {
		std::shared_ptr<AudioSink>   sink_drv;
		sink_drv = m_sound_sink;
		if(sink_drv.get() != nullptr) {
			sink_drv->resume();
		}
	}
	m_now_mute = false; // OK?
}

void OSD_BASE::mute_sound()
{
	if(!(m_now_mute.load()) && (m_sound_initialized.load())) {
		std::shared_ptr<AudioSink>   sink_drv;
		sink_drv = m_sound_sink;
		if(sink_drv.get() != nullptr) {
			sink_drv->suspend();
		}
	}
	m_now_mute = true;
	
}

void OSD_BASE::stop_sound()
{
	//m_sound_ok = false;
	std::shared_ptr<AudioSink>   sink_drv;
	sink_drv = m_sound_sink;
	if(sink_drv.get() != nullptr) {
		std::lock_guard<std::recursive_timed_mutex> _locker(m_sound_sink_mutex);
		sink_drv->stop();
		m_sound_sink_io = nullptr;
	}
//	m_sound_initialized = false;
}

//void OSD_BASE::do_acknowledge_sound_started()
//{
	//m_sound_ok = true;
//	elapsed_us_before_rendered = sound_drv->driver_elapsed_usec();
//}

int OSD_BASE::get_sound_rate()
{
	__LIKELY_IF(m_sound_sink_rate.load() > 0) {
		return m_sound_sink_rate.load();
	}
	return 0;
}
/* End Note: */


void OSD_BASE::start_record_sound()
{
	__UNLIKELY_IF(!(m_sound_initialized.load())) {
		return;
	}

	if(!now_record_sound) {
		//LockVM();
		__UNLIKELY_IF(rec_sound_fio != nullptr) {
			if(rec_sound_fio->IsOpened()) { // Fail safe.
				rec_sound_fio->Fclose();
			}
			delete rec_sound_fio;
		}
		rec_sound_fio = nullptr;
		__UNLIKELY_IF(m_sound_sink_rate.load() <= 0) {
			return; // Rate don't set.
		}
		QDateTime nowTime = QDateTime::currentDateTime();
		QString tmps = QString::fromUtf8("Sound_Save_emu");
		tmps = tmps + get_vm_config_name();
		tmps = tmps + QString::fromUtf8("_");
		tmps = tmps + nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz"));
		tmps = tmps + QString::fromUtf8(".wav");
		strncpy((char *)sound_file_name, tmps.toLocal8Bit().constData(), sizeof(sound_file_name) - 1);
		// create wave file
		rec_sound_fio = new FILEIO();
		if(rec_sound_fio != nullptr) {
			if(rec_sound_fio->Fopen(bios_path(sound_file_name), FILEIO_WRITE_BINARY)) {
				// write dummy wave header
				write_dummy_wav_header((void *)rec_sound_fio);

				rec_sound_bytes = 0;
				rec_sound_buffer_ptr = 0;
				now_record_sound = true;
			} else {
				// failed to open the wave file
				delete rec_sound_fio;
			}
		}
		//UnlockVM();
	}
}

void OSD_BASE::stop_record_sound()
{
	if(now_record_sound) {
		int __rate = m_sound_sink_rate.load();
		if((rec_sound_bytes == 0) || (__rate <= 0)) {
			if(rec_sound_fio != nullptr) {
				if(rec_sound_fio->IsOpened()) {
					rec_sound_fio->Fclose();
					rec_sound_fio->RemoveFile(sound_file_name);
				}
			}
		} else {
			// update wave header
			wav_header_t wav_header;
			wav_chunk_t wav_chunk;
			size_t head_size = (size_t)rec_sound_bytes + sizeof(wav_header) + sizeof(wav_chunk);
			if(!set_wav_header(&wav_header, &wav_chunk, 2, (uint32_t)__rate, 16, head_size)) {
				if(rec_sound_fio != nullptr) {
					if(rec_sound_fio->IsOpened()) {
						rec_sound_fio->Fclose();
						rec_sound_fio->RemoveFile(sound_file_name);
					}
				}
			} else {
				if(rec_sound_fio != nullptr) {
					if(rec_sound_fio->IsOpened()) {
						rec_sound_fio->Fseek(0, FILEIO_SEEK_SET); // Start to head.
						rec_sound_fio->Fwrite(&wav_header, sizeof(wav_header_t), 1);
						rec_sound_fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
						rec_sound_fio->Fclose();
					}
				}
			}
		}
		if(rec_sound_fio != nullptr) {
			delete rec_sound_fio;
		}
		rec_sound_fio = nullptr; // Must clear.
		now_record_sound = false;
		//UnlockVM();
	}
}

void OSD_BASE::restart_record_sound()
{
	bool tmp = now_record_sound;
	stop_record_sound();
	if(tmp) {
		start_record_sound();
	}
}


void OSD_BASE::close_capture_sound_emu(int ch)
{
	if(ch < 0) return;
	if(ch >= MAX_CAPTURE_SOUNDS) return;
	if(sound_capture_desc[ch].out_buffer != NULL) {
		free(sound_capture_desc[ch].out_buffer);
	}
	sound_capture_desc[ch].out_buffer = NULL;
	sound_capturing_emu[ch] = false;
}

void *OSD_BASE::get_capture_sound_buffer(int ch)
{
	if(ch < 0) return NULL;
	if(ch >= MAX_CAPTURE_SOUNDS) return NULL;
	return sound_capture_desc[ch].out_buffer;
}

bool OSD_BASE::is_capture_sound_buffer(int ch)
{
	if(ch < 0) return false;
	if(ch >= MAX_CAPTURE_SOUNDS) return false;
	if(sound_capture_desc[ch].out_buffer == NULL) return false;
	return sound_capturing_emu[ch];
}
void *OSD_BASE::open_capture_sound_emu(int ch, int rate, int channels, int sample_type, int samples, int physical_device_num)
{
	if(ch < 0) return nullptr;
	if(ch >= MAX_CAPTURE_SOUNDS) return nullptr;

	void *p = nullptr;
#if 0
	close_capture_sound_emu(ch);
	sound_capture_desc[ch].rate = rate;
	sound_capture_desc[ch].channels = channels;
	sound_capture_desc[ch].samples = samples;
	sound_capture_desc[ch].sample_type = sample_type;
	sound_capture_desc[ch].physical_dev = physical_device_num;
	bool stat = false;
	if((physical_device_num >= 0) && (physical_device_num < sound_capture_devices_list.count()) && (physical_device_num < MAX_SOUND_CAPTURE_DEVICES)) {
		if(!(capturing_sound[physical_device_num])) {
			stat = open_sound_capture_device(physical_device_num, (rate < 44100) ? 44100 : rate, (channels > 2) ? channels : 2);
		}
	}


	if(stat) {
		switch(sample_type) {
		case SAMPLE_TYPE_UINT8:
		case SAMPLE_TYPE_SINT8:
			p = malloc(sizeof(uint8_t) * channels * (samples + 100));
			break;
		case SAMPLE_TYPE_UINT16_BE:
		case SAMPLE_TYPE_SINT16_BE:
		case SAMPLE_TYPE_UINT16_LE:
		case SAMPLE_TYPE_SINT16_LE:
			p = malloc(sizeof(uint16_t) * channels * (samples + 100));
			break;
		case SAMPLE_TYPE_UINT32_BE:
		case SAMPLE_TYPE_SINT32_BE:
		case SAMPLE_TYPE_UINT32_LE:
		case SAMPLE_TYPE_SINT32_LE:
		p = malloc(sizeof(uint32_t) * channels * (samples + 100));
		break;
		case SAMPLE_TYPE_FLOAT_BE:
		case SAMPLE_TYPE_FLOAT_LE:
			p = malloc(sizeof(float) * channels * (samples + 100));
			break;
		}
	}
	sound_capture_desc[ch].out_buffer = (uint8_t *)p;
	sound_capturing_emu[ch] = true;
#endif
	return p;
}

bool OSD_BASE::open_sound_capture_device(int num, int req_rate, int req_channels)
{
	if(num < 0) return false;
	if(num >= MAX_SOUND_CAPTURE_DEVICES) return false;
	if(sound_capture_devices_list.size() <= num) return false;
#if 0
	SDL_AudioSpec req;
	SDL_AudioSpec desired;
	req.freq = req_rate;
	req.channels = req_channels;
	req.silence = 0;
	req.format = AUDIO_S16SYS;
	req.samples = (sizeof(sound_capture_buffer[num]) / sizeof(int16_t)) / req_channels;
	req.callback = &(this->audio_capture_callback);
	req.userdata = (void *)(&(sound_capture_dev_desc[num].userdata));

	if(!(capturing_sound[num])) {
		sound_capture_desc[num].physical_dev = SDL_OpenAudioDevice((const char *)sound_capture_devices_list.value(num).toUtf8().constData(), 1, &req, &desired, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
		if(sound_capture_desc[num].physical_dev <= 0) {
			debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,"Failed to initialize sound capture device \"%s\"\n", (const char *)sound_capture_devices_list.value(num).toUtf8().constData());
			sound_capture_desc[num].physical_dev = -1;
			return false;
		}
		// Device OK
		capturing_sound[num] = true;
		sound_capture_dev_desc[num].format = desired.format;
		sound_capture_dev_desc[num].sample_rate = desired.freq;
		sound_capture_dev_desc[num].channels = desired.channels;
		sound_capture_dev_desc[num].buffer_samples = desired.samples;
		sound_capture_dev_desc[num].size = desired.size;
		sound_capture_dev_desc[num].callback = desired.callback;
		sound_capture_dev_desc[num].silence = desired.silence;
		int buflen = desired.samples * desired.channels;
		switch(desired.format) {
		case AUDIO_S8:
		case AUDIO_U8:
			buflen = buflen * sizeof(int8_t);
			break;
		case AUDIO_S16LSB:
		case AUDIO_S16MSB:
		case AUDIO_U16LSB:
		case AUDIO_U16MSB:
			buflen = buflen * sizeof(int16_t);
			break;
		case AUDIO_S32LSB:
		case AUDIO_S32MSB:
			buflen = buflen * sizeof(int32_t);
			break;
		case AUDIO_F32LSB:
		case AUDIO_F32MSB:
			buflen = buflen * sizeof(float);
			break;
		default:
			break;
		}

		sound_capture_dev_desc[num].userdata.buffer_size = buflen;
		sound_capture_dev_desc[num].userdata.format = desired.format;
		sound_capture_dev_desc[num].userdata.readlen = 0;
		sound_capture_dev_desc[num].userdata.writelen = 0;
		sound_capture_dev_desc[num].userdata.readpos = 0;
		sound_capture_dev_desc[num].userdata.writepos = 0;
		sound_capture_dev_desc[num].userdata.read_buffer_ptr = &(sound_capture_buffer[num][0]);
		memset(&(sound_capture_buffer[num][0]), 0x00, buflen);

		for(int ch = 0; ch < MAX_SOUND_CAPTURE_DEVICES; ch++) {
			if(sound_capture_desc[ch].physical_dev == num) {
				sound_capture_desc[ch].read_format = desired.format;
				sound_capture_desc[ch].read_rate = desired.freq;
				sound_capture_desc[ch].read_silence = desired.silence;
				sound_capture_desc[ch].read_size = desired.size;
				sound_capture_desc[ch].read_channels = desired.channels;
				sound_capture_desc[ch].read_samples = desired.samples;
				sound_capture_desc[ch].read_callback = desired.callback;
				sound_capture_desc[ch].read_userdata = desired.userdata;

				sound_capture_desc[ch].read_pos = 0;
				sound_capture_desc[ch].read_data_len = 0;
				sound_capture_desc[ch].read_buffer_len = buflen;
				sound_capture_desc[ch].read_buffer_ptr = (uint8_t *)(&(sound_capture_buffer[num][0]));


			}
		}
	}
#endif
	return true;
}

bool OSD_BASE::close_sound_capture_device(int num, bool force)
{
	// ToDo: Check capturing entries
#if 0
	if((capturing_sound[num]) && (sound_capture_desc[num].physical_dev > 0)) {
		SDL_CloseAudioDevice(sound_capture_desc[num].physical_dev);
	}
#endif
	return true;
}
