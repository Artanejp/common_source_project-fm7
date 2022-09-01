/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2022.07.25-

	[ OSD / Sound driver / St Multimedia ]
*/

#pragma once

#include "./osd_sound_mod_template.h"

#include <QAudioFormat>
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#include <QAudioDevice>
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAudioDeviceInfo>
#endif	

QT_BEGIN_NAMESPACE

namespace SOUND_OUTPUT_MODULE {
class DLL_PREFIX M_QT_MULTIMEDIA
	: public M_BASE
{
	Q_OBJECT
protected:
	QAudioFormat m_audioOutputFormat;
	std::shared_ptr<SOUND_BUFFER_QT>	m_audioOutput;
	std::atomic<int64_t>				m_samples;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
	std::shared_ptr<QAudioSink>			m_audioOutputSink;
	QAudioDevice m_audioOutputDevice;
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	std::shared_ptr<QAudioOutput>		m_audioOutputSink;
	QAudioDeviceInfo m_audioOutputDevice;
#endif	
	
public:
	M_QT_MULTIMEDIA(
		OSD_BASE *parent,
		USING_FLAGS *pflags,
		CSP_Logger *logger,
		int base_rate = 48000,
		int base_latency_ms = 100,
		int base_channels = 2,
		void *extra_configvalues = nullptr);

	~SOUND_OUTPUT_MODULE_QTMULTIMEDIA();

	virtual bool real_reconfig_sound(int& rate,int& channels,int& latency_ms) override;
	
public slots:
	virtual bool set_fileio(std::shared_ptr<SOUND_BUFFER_QT> fileio_ptr) override;
	virtual void initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples) override;
	virtual void release_sound() override;

	virtual void update_sound(int* extra_frames) override;
	virtual void mute_sound() override;
	virtual void stop_sound() override;

	virtual int result_opening_external_file() override;
	virtual int64_t wrote_data_to() override;
	virtual bool result_closing_external_file() override;
	
};
