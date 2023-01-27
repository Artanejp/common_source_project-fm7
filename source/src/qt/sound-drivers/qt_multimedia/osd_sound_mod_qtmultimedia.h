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
#include <QAudioDevice>
#include <QAudioSource>
#include <QAudioSink>
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#endif	

#include "../osd_sound_mod_template.h"

QT_BEGIN_NAMESPACE


namespace SOUND_MODULE {
/* SOUND_MODULE */
	namespace OUTPUT {
	/* SOUND_MODULE::OUTPUT */
class DLL_PREFIX M_QT_MULTIMEDIA
	: public M_BASE
{
	Q_OBJECT
protected:
	QAudioFormat						m_audioOutputFormat;
	std::string							m_device_name;
	std::list<std::string>				devices_name_list;
	bool								m_device_is_default;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink>			m_audioOutputSink;
	QAudioDevice						m_audioOutputDevice;
	QList<QAudioDevice>					m_audioOutputsList;
	virtual void set_audio_format(QAudioDevice dest_device, QAudioFormat& desired, int& channels, int& rate);
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput>		m_audioOutputSink;
	QAudioDeviceInfo					m_audioOutputDevice;
	QList<QAudioDeviceInfo>				m_audioOutputsList;
	
	virtual void set_audio_format(QAudioDeviceInfo dest_device, QAudioFormat& desired, int& channels, int& rate);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	QAudioDevice get_device_by_name(QString driver_name);
	void setup_device(QAudioDevice dest_device, int& rate, int& channels, int& latency_ms, bool force_reinit = false);
#else
	QAudioDeviceInfo get_device_by_name(QString driver_name);
	void setup_device(QAudioDeviceInfo dest_device, int& rate,int& channels,int& latency_ms, bool force_reinit = false);
#endif
	virtual void initialize_sound_devices_list();
	virtual bool real_reconfig_sound(int& rate,int& channels,int& latency_ms) override;
	virtual void update_driver_fileio() override;
	virtual const std::string set_device_sound(const _TCHAR* driver_name, int& rate,int& channels,int& latency_ms);
	
public:
	M_QT_MULTIMEDIA(
		OSD_BASE *parent,
		SOUND_BUFFER_QT* deviceIO = nullptr,
		int base_rate = 48000,
		int base_latency_ms = 100,
		int base_channels = 2,
		void *extra_configvalues = nullptr,
		int extra_config_bytes = 0);
	
	~M_QT_MULTIMEDIA();
	
	virtual bool initialize_driver() override;
	virtual bool release_driver() override;
	
	virtual int64_t driver_elapsed_usec() override;
	virtual int64_t driver_processed_usec() override;
	virtual bool is_driver_started() override;
	
	virtual std::list<std::string> get_sound_devices_list() override;
																	
public slots:
	virtual void release_sound() override;

	virtual void mute_sound() override;
	virtual void unmute_sound() override;
	virtual void stop_sound() override;
	
	virtual void driver_state_changed(QAudio::State newState);
	
	virtual void do_sound_start();
	virtual void do_sound_stop();
	virtual void do_sound_resume();
	virtual void do_sound_suspend();
	virtual void do_discard_sound();
	virtual void do_sound_volume(double level);
	virtual void do_set_device_by_name(QString driver_name) override;

};

/* SOUND_MODULE::OUTPUT */
	}
/* SOUND_MODULE */
}

