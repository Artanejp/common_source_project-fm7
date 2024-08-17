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
	M_QT_MULTIMEDIA::M_QT_MULTIMEDIA(
		OSD_BASE *parent,
		QIODevice* sinkDeviceIO,
		QIODevice* sourceDeviceIO,
		int base_rate,
		int base_latency_ms,
		int base_channels,
		void *extra_config_values,
		int extra_config_bytes )
		:
	M_BASE(
		parent,
		sinkDeviceIO,
		sourceDeviceIO,
		base_rate,
		base_latency_ms,
		base_channels,
		extra_config_values,
		extra_config_bytes )
{
	m_classname = "SOUND_MODULE::M_QT_MULTIMEDIA";

	initialize_sink_devices_list();
	initialize_source_devices_list();
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
			_drv = QString::fromUtf8(_ccp->sound_device_name);
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


	m_config_ok = initialize_driver_post(parent);
	return m_config_ok.load();
}

bool M_QT_MULTIMEDIA::recalc_samples(int rate, int latency_ms, bool need_update, bool need_resize_fileio)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	bool __need_restart = M_BASE::recalc_samples(rate, latency_ms, need_update, need_resize_fileio);
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
			if(m_sink_external_fileio.load()) {
				drv->start(m_sink_fileio.get());
			} else {
				m_sink_fileio.reset(drv->start());
			}
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
			if(!(drv->isNull())) {
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			if((drv->error() != QAudio::UnderrunError) && (drv->error() != QAudio::NoError)) {
#endif
				__need_restart |= true;
			}
		}
	}
	return __need_restart;
}



void M_QT_MULTIMEDIA::release_source()
{
	// ToDo
}		

void M_QT_MULTIMEDIA::release_sink()
{
	{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
#endif
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
	#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		tmps = (*n).description();
	#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		tmps = (*n).deviceName();
	#endif
		if(tmps == name) {
			return true;
		}
	}
	return false;
}

bool M_QT_MULTIMEDIA::is_default_output_device()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	if(m_audioOutputDevice == QMediaDevices::defaultAudioOutput()) {
		return true;
	}
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	if(m_audioOutputDevice == QAudioDeviceInfo::defaultOutputDevice()) {
		return true;
	}
#endif
	return false;
}
		

bool M_QT_MULTIMEDIA::is_sink_io_device_exists()
{
	bool b_sink_fileio = M_BASE::is_sink_io_device_exists();
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
	if(drv.get() != nullptr) {
		std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
		bool _ba = !(drv->isNull());
		bool _bb = (m_fileio.get() != nullptr) ? true : false;
		return ((_ba) && (_bb) && (b_sink_fileio));
	}
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
	if(drv.get() != nullptr) {
		std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
		bool _ba = ((drv->error() != QAudio::UnderrunError) && (drv->error() != QAudio::NoError)) ? true : false;
		bool _bb = (m_fileio.get() != nullptr) ? true : false;
		return ((_ba) && (_bb) && (b_sink_fileio));
	}
#endif
	return b_sink_fileio;
}


void M_QT_MULTIMEDIA::sink_state_changed(QAudio::State newState)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
#endif
	std::shared_ptr<QIODevice> fio = m_sink_fileio;
	bool checkstat = false;
	switch(newState) {
	case QAudio::ActiveState:
		if(!(m_prev_sink_started.load())) {
			m_prev_sink_started = true;
			do_discard_sound();
			m_before_rendered = 0;
			__debug_log_func("GO. driver=%0llx fileio=%0llx", drv.get(), fio.get());
		}
		if(drv.get() != nullptr) {
			drv->setVolume(m_sink_volume.load());
		}
		emit sig_sink_started();
		//__debug_log_func(_T("AUDIO:ACTIVE"));
		break;
	case QAudio::IdleState:
		emit sig_sink_empty();
		break;
	case QAudio::StoppedState:
		__debug_log_func(_T("AUDIO:STOP"));
		if(m_prev_sink_started.load()) {
			m_prev_sink_started = false;
		}
		do_discard_sink();
		m_before_rendered = 0;
		m_sink_volume = std::nan("1");
		emit sig_sink_stopped();
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


void M_QT_MULTIMEDIA::update_sink_driver_fileio()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
#endif
	if(m_sink_external_fileio.load()) {
		std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
		if(drv->state() != QAudio::StoppedState) {
			drv->stop();
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

bool M_QT_MULTIMEDIA::initialize_sink_driver_post(QObject* parent)
{
	bool result = false;

	// ToDo: Input
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QAudioDevice tmp_output_device;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QAudioDeviceInfo tmp_output_device;
#endif
	tmp_output_device = get_output_device_by_name(QString::fromStdString(m_sink_device_name));
	if(tmp_output_device.isNull()) {
		return false;
	}
	
	QAudioFormat tmp_output_format = tmp_output_device.preferredFormat();

	int _channels = m_sink_channels.load();
	int _rate = m_sink_rate.load();
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
	m_audioOutputSink.reset(new QAudioSink(m_audioOutputDevice, m_audioOutputFormat, this));
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	m_audioOutputSink.reset(new QAudioOutput(m_audioOutputDevice, m_audioOutputFormat, this));
#endif
	result = ((m_audioOutputSink.get() != nullptr) /* || (m_audioInputSource.get() != nullptr) */);
	if(result) {
		connect(m_audioOutputSink.get(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(sink_state_changed(QAudio::State)));
		m_sink_channels = m_audioOutputSink->format().channelCount();
		m_sink_rate = m_audioOutputSink->format().sampleRate();
		m_config_ok = true; // ToDo.
	}
	m_sink_samples = ((qint64)m_sink_latency_ms.load() * (qint64)(m_sink_rate.load())) / 1000;
	if(m_sink_samples.load() <= 0) {
		m_sink_samples = 1;
	}
	m_sink_chunk_bytes = m_sink_samples.load() * m_sink_wordsize.load() * m_sink_channels.load();
	m_sink_buffer_bytes = m_sink_chunk_bytes.load() * 4;
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	m_audioOutputsList = QMediaDevices::audioOutputs();
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	m_audioOutputsList = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
#endif
}

void M_QT_MULTIMEDIA::initialize_source_sound_devices_list()
{
	// ToDo.	
}

std::list<std::string> M_QT_MULTIMEDIA::get_sound_devices_list()
{
	std::list<std::string> _l;
	QString tmps;
	for(auto n = m_audioOutputsList.begin(); n != m_audioOutputsList.end(); ++n) {
	#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		tmps = (*n).description();
	#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		tmps = (*n).deviceName();
	#endif
		_l.push_back(tmps.toStdString());
	}
		
	return _l;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
QAudioDevice M_QT_MULTIMEDIA::get_output_device_by_name(QString driver_name)
#else
QAudioDeviceInfo M_QT_MULTIMEDIA::get_output_device_by_name(QString driver_name)
#endif
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QAudioDevice dest_device;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QAudioDeviceInfo dest_device;
#endif

	if((driver_name == QString::fromUtf8("Default")) || (driver_name.isEmpty())) {
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
				break;
			}
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			if((*i).deviceName().compare(driver_name) == 0) {
				dest_device = *i;
				break;
			}
#endif
		}
	}
	if(dest_device.isNull()) {
		__debug_log_func(_T("E:desired_driver=%s but not found."), driver_name.toLocal8Bit().constData());
		return dest_device;
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

void M_QT_MULTIMEDIA::do_set_output_by_name(QString driver_name)
{
//	if(m_sink_device_name == driver_name.toLocal8Bit().toStdString()) {
//		return;
//	}
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QAudioDevice dest_device = get_output_device_by_name(driver_name);
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QAudioDeviceInfo dest_device = get_output_device_by_name(driver_name);
#endif
	int _rate = m_sink_rate.load();
	int _channels = m_sink_channels.load();
	int _latency = m_sink_latency_ms.load();
	setup_output_device(dest_device, _rate, _channels, _latency, true);
	m_sink_rate = _rate;
	m_sink_channels = _channels;
	m_sink_latency_ms = _latency;
}

void M_QT_MULTIMEDIA::do_set_input_by_name(QString driver_name)
{
	// ToDo.
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
void M_QT_MULTIMEDIA::setup_output_device(QAudioDevice dest_device, int& rate,int& channels,int& latency_ms, bool force_reinit)
#else
void M_QT_MULTIMEDIA::setup_output_device(QAudioDeviceInfo dest_device, int& rate,int& channels,int& latency_ms, bool force_reinit)
#endif
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
		   && (m_sink_fileio.get() != nullptr)) {
			if(m_sink_external_fileio.load()) {
				std::shared_ptr<QIODevice> fio = m_sink_fileio;
				if(fio->isOpen()) {
					return;
				}
			}
			//update_sink_driver_fileio();
			__debug_log_func(_T("Nothing changed.Exit."));

			//real_reconfig_sound(rate, channels, latency_ms);
			emit sig_start_sink();
			//emit sig_start_audio();
			return;
		}
	}
	if((m_audioOutputDevice.isNull()) || (m_audioOutputDevice != dest_device)) {
		force_reinit = true;
	}
	bool force_req_reinit = false;
	if(!(force_reinit)) {
		if(m_sink_latency_ms.load() != latency_ms) {
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
			m_sink_channels = _channels;
			m_sink_rate = _rate;
			return;
		}
		#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
		#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
		#endif
		if(drv.get() != nullptr) {
			if(drv->state() != QAudio::StoppedState) {
				drv->stop();
				wait_driver_stopped(1000);
			}
			m_audioOutputSink->disconnect();
			m_audioOutputSink.reset();
		}
		do_discard_sink();

		m_audioOutputDevice = dest_device;
		m_audioOutputFormat = desired;

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		m_audioOutputSink.reset(new QAudioSink(dest_device, desired, this));
		std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		m_audioOutputSink.reset(new QAudioOutput(dest_device, desired, this));
		std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
#endif
		m_sink_prev_started = false;
		m_before_rendered = 0;
		m_sink_channels = channels;
		bool prepared = M_BASE::recalc_samples(rate, latency_ms, true, true);
		if(drv.get() != nullptr) {
			connect(drv.get(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(sink_state_changed(QAudio::State)));
			channels = drv->format().channelCount();
			rate = drv->format().sampleRate();
			QString _tmpname = QString::fromUtf8("Default");
			if(!(is_default_output_device())) {
				#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
				_tmpname = m_audioOutputDevice.description();
				#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				_tmpname = m_audioOutputDevice.deviceName();
				#endif
			}
			m_sink_device_name = _tmpname.toLocal8Bit().toStdString();
			config_t* _ccp = get_config_ptr();
			if(_ccp != nullptr) {
				memset(_ccp->sound_device_name, 0x00, sizeof(_ccp->sound_device_name));
				my_tcscpy_s(_ccp->sound_device_name, (sizeof(_ccp->sound_device_name) / sizeof(_TCHAR)) - 1, _tmpname.toUtf8().constData());
			}
			m_sink_channels = channels;
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
			m_sink_device_name.clear();
			m_config_ok = false;
		}
	}
	__debug_log_func(_T("Result: rate=%d channels=%d latency=%dmSec reinit=%d"), m_rate.load(), m_channels.load(), m_latency_ms.load(), force_reinit);
	
	start_sink();
}

const std::string M_QT_MULTIMEDIA::set_sink_device_sound(const _TCHAR* driver_name, int& rate,int& channels,int& latency_ms)
{
	if(driver_name == nullptr) {
		return (const std::string)(std::string(""));
	}
	if(strlen(driver_name) <= 0) {
		return (const std::string)(std::string(""));
	}

	QString _name = QString::fromUtf8(driver_name);

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QAudioDevice dest_device = get_output_device_by_name(_name);
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QAudioDeviceInfo dest_device = get_output_device_by_name(_name);
#endif
	setup_output_device(dest_device, rate, channels, latency_ms, false);

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
		m_sink_device_name = set_sink_device_sound((const _TCHAR *)(m_sink_device_name.c_str()), rate, channels, latency_ms);
		__debug_log_func(_T("Returned Driver=\"%s\" rate=%dHz channles=%d latency=%dmSec"), m_sink_device_name.c_str(), rate, channels, latency_ms);
		//emit sig_set_sound_device(m_sink_device_name);
//	}
	if((rate <= 0) || (latency_ms <= 0)) {
		rate = 48000;
		latency_ms = 100;
		channels = 2;
		m_config_ok = false;
	}
	m_sink_channels = channels;
	if(recalc_samples(rate, latency_ms, true, false)) {
		m_prev_sink_started = m_mute = false;
	}
	return m_config_ok.load();
}


bool M_QT_MULTIMEDIA::wait_start_sink(int64_t msec)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	__UNLIKELY_IF(p.get() == nullptr) {
		return false;
	}
	bool _b = (p->state() != QAudio::StoppedState);
	bool _infinite = false;
	int64_t loop_count;
	__UNLIKELY_IF((msec < 0) || (msec == INT64_MAX)) {
		_infinite = true;
	}
	loop_count = _infinite / 2; // Per 2mSec.
	while(!(_b)) {
		_b = (p->state() != QAudio::StoppedState);
		if(_b) {
			break;
		}
		if(!(_infinite)) {
			loop_count--;
			if(loop_count <= 0) {
				break;
			}
		}
		QThread::msleep(2);
	}
	return _b;
}

bool M_QT_MULTIMEDIA::do_start_sink()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	__UNLIKELY_IF(p.get() == nullptr) {
		return false;
	}
	if((p->state() != QAudio::StoppedState) && (m_prev_sink_started.load())) {
		return false;
	}
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	if(m_sink_external_fileio.load()) {
		bool _s = M_BASE::start_sink();
		if(_s) {
			std::shared_ptr<QIODevice> fio = m_sink_fileio;
			__LIKELY_IF(fio.get() != nullptr) {
				p->start(fio.get());
			}
		}
	} else {
		m_sink_fileio.reset(p->start());
	}
	bool _stat = wait_start_sink(1);
	return _stat;
}

void M_QT_MULTIMEDIA::do_stop_sink()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	if(p.get() != nullptr) {
		p->stop();
	}
	do_discard_sink();
	m_before_rendered = 0;
	m_prev_sink_started = false;
	m_sink_volume = std::nan("1");
}


void M_QT_MULTIMEDIA::do_sink_volume(double level)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> p = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
#endif
	if(p.get() != nullptr) {
		p->setVolume(level);
	}
	m_sink_volume = level;
}

bool M_QT_MULTIMEDIA::is_output_driver_started()
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	bool _b = M_BASE::is_output_driver_started();
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

void M_QT_MULTIMEDIA::do_mute_sink()
{
	if(!(m_mute.load()) && (m_config_ok.load())) {
	#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		std::shared_ptr<QAudioSink> p = m_audioOutputSink;
	#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
	#endif
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
	#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		std::shared_ptr<QAudioSink> p = m_audioOutputSink;
	#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
	#endif
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> q = m_audioOutputSink;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> q = m_audioOutputSink;
#endif
	if(q.get() != nullptr) {
		q->reset();
	}
	emit sig_sink_empty();
}

void M_QT_MULTIMEDIA::do_stop_sink()
{
	if((m_config_ok.load()) && (m_prev_sink_started)) {
		#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
		std::shared_ptr<QAudioSink> p = m_audioOutputSink;
		#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		std::shared_ptr<QAudioOutput> p = m_audioOutputSink;
		#endif
		//std::shared_ptr<QIODevice> fio = m_sink_fileio;
		if(p.get() != nullptr) {
			if(p->state() != QAudio::StoppedState) {
				p->stop();
			}
		}
	}
	m_sink_volume = std::nan("1");
}


int64_t M_QT_MULTIMEDIA::update_sound(void* datasrc, int samples)
{
	std::lock_guard<std::recursive_timed_mutex> locker(m_locker);
	#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink> drv = m_audioOutputSink;
	#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput> drv = m_audioOutputSink;
	#endif
	std::shared_ptr<QIODevice> q = m_sink_fileio;
	int64_t sample_bytes = ((int64_t)samples) * ((int64_t)(m_sink_channels.load() * m_sink_wordsize.load()));

	if(sample_bytes <= 0) {
		return 0;
	}
	__UNLIKELY_IF(drv.get() == nullptr) {
		return samples;
	}
	__UNLIKELY_IF((drv->state() == QAudio::StoppedState) || (drv->state() == QAudio::SuspendedState)) { // OK?
		return samples; // OK?
	}
	if(q.get() != nullptr) {
		int64_t wrote = 0;
		wrote = q->write_to_buffer((const char *)datasrc, sample_bytes);
		if(wrote <= 0) {
			return 0;
		}
		wrote = wrote / sample_bytes;
		return wrote;
	}
	return 0;
}

/* SOUND_MODULE */

}
