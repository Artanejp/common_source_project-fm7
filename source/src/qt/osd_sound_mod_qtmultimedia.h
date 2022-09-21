/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2022.07.25-

	[ OSD / Sound driver / St Multimedia ]
*/

#pragma once

#include "./osd_sound_mod_template.h"

#include <string>
#include <QAudioFormat>
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#include <QAudioDevice>
#include <QAudioSink>
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#endif	

QT_BEGIN_NAMESPACE

class SOUND_BUFFER_QT;
namespace SOUND_OUTPUT_MODULE {
class DLL_PREFIX M_QT_MULTIMEDIA
	: public M_BASE
{
	Q_OBJECT
protected:
	QAudioFormat						m_audioOutputFormat;
	std::string							m_device_name;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink>			m_audioOutputSink;
	QAudioDevice m_audioOutputDevice;
	virtual void set_audio_format(QAudioDevice dest_device, QAudioFormat& desired, int& channels, int& rate);
	
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput>		m_audioOutputSink;
	QAudioDeviceInfo m_audioOutputDevice;
	virtual void set_audio_format(QAudioDeviceInfo dest_device, QAudioFormat& desired, int& channels, int& rate);
#endif

	
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
										  
public slots:
	virtual void release_sound() override;

	virtual void mute_sound() override;
	virtual void stop_sound() override;
	
	virtual void driver_state_changed(QAudio::State newState);
	
	virtual void do_sound_start();
	virtual void do_sound_stop();
	virtual void do_sound_resume();
	virtual void do_sound_suspend();
	virtual void do_discard_sound();
	virtual void do_sound_volume(double level);
	
};
}