/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2022.07.25-

	[ OSD / Sound driver / St Multimedia ]
*/

#pragma once

#include <string>
#include <list>


#include <QAudioFormat>
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
// Qt 6.x
#include <QMediaDevices>
#include <QAudioDevice>

#include <QAudioSource>
#include <QAudioSink>
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
// Qt 5.x
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#else
#error "This version of Qt don't supports Qt Multimedia. Please remove to build this"."
#endif

#include "../osd_sound_mod_template.h"



QT_BEGIN_NAMESPACE

//#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
//class QAudioInput;
//class QAudioOutput;
//#endif

namespace SOUND_MODULE {
/* SOUND_MODULE */
class DLL_PREFIX M_QT_MULTIMEDIA
	: public M_BASE
{
	Q_OBJECT
protected:
	QAudioFormat						m_audioOutputFormat;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using OutputSinkType  = QAudioSink;
using InputSourceType = QAudioSource;
using DeviceInfoType  = QAudioDevice;
#else
using OutputSinkType  = QAudioOutput;
using InputSourceType = QAudioInput;
using DeviceInfoType  = QAudioDeviceInfo;
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QMediaDevices						m_Root;
	//QAudioOutput						*m_Output;
	//QAudioInput						*m_Input;
#endif
	
	std::shared_ptr<OutputSinkType>		m_audioOutputSink;
	DeviceInfoType						m_audioOutputDevice;
	QList<DeviceInfoType>				m_audioOutputsList;

	std::shared_ptr<InputSourceType>	m_audioInputSource;
	DeviceInfoType						m_audiInputDevice;
	QList<DeviceInfoType>				m_audioInputssList;
	
	std::atomic<QAudio::State>			m_prev_sink_state;
	std::atomic<QAudio::State>			m_prev_source_state;

	virtual void set_audio_format(DeviceInfoType dest_device, QAudioFormat& desired, int& channels, int& rate);
	DeviceInfoType get_output_device_by_name(QString driver_name);
	void setup_output_device(DeviceInfoType dest_device, int& rate, int& channels, int& latency_ms, bool force_reinit = false);
	
	virtual bool real_reconfig_sound(int& rate,int& channels,int& latency_ms, const bool force) override;
	virtual void update_sink_driver_fileio() override;
	
	virtual const std::string set_sink_device_sound(const _TCHAR* driver_name, int& rate,int& channels,int& latency_ms);
	virtual bool initialize_driver_post(QObject *parent);

	bool has_output_device(QString name) override;
	bool is_default_output_device() override;
	
	bool has_input_device(QString name) override;
	bool is_default_input_device() override;
	
	bool is_output_stopped();
	bool is_input_stopped();
	
	virtual bool recalc_sink_buffer(int rate, int latency_ms, const bool force);
	virtual bool reopen_sink_fileio(bool force_reopen = false) override;


	// Will use? May be unused?
	virtual bool initialize_sink_driver_post(QObject* parent);
	virtual bool initialize_source_driver_post(QObject* parent);

	inline QString get_audio_device_name(DeviceInfoType x)
	{
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		return x.description();
	#else
		return x.deviceName();
	#endif
	}
public:
	M_QT_MULTIMEDIA(
		OSD_BASE *parent,
		QIODevice* sinkDeviceIO = nullptr,
		QIODevice* sourceDeviceIO = nullptr,
		size_t base_rate = 48000,
		size_t base_latency_ms = 100,
		size_t base_channels = 2,
		void *extra_configvalues = nullptr,
		size_t extra_config_bytes = 0);

	~M_QT_MULTIMEDIA();

	virtual bool initialize_driver(QObject *parent) override;
	virtual void initialize_sink_sound_devices_list();
	virtual void initialize_source_sound_devices_list();
	
	virtual void release_sink() override;
	virtual void release_source() override;

	virtual bool is_output_driver_started() override;
	virtual int64_t update_sound(void* datasrc, int samples) override;

	virtual std::list<std::string> get_sound_sink_devices_list() override;
	virtual bool is_sink_io_device_exists() override;

public slots:
	// Common SLOTs.
	virtual void release_sound() override;
	virtual void do_set_output_by_name(QString driver_name) override;
	virtual void do_set_input_by_name(QString name) override;

	// Unique SLOTS.
	virtual void sink_state_changed(QAudio::State newState);
	virtual void source_state_changed(QAudio::State newState);

	virtual void do_sound_start();
	virtual void do_sound_stop();
	virtual void do_sound_resume();
	virtual void do_sound_suspend();
	virtual void do_discard_sound();
	virtual void do_sound_volume(double level);

	void do_reload_sink_sound_devices();
	void do_reload_source_sound_devices();
};

/* SOUND_MODULE */
}
