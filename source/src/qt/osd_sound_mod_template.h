/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2022.07.15-

	[ OSD / Sound driver / Template ]
*/

#pragma once

#include <QObject>
#include <mutex>
#include <memory>

#include "../common.h"
#include "./sound_buffer_qt.h"

QT_BEGIN_NAMESPACE

class OSD_BASE;
class USING_FLAGS;
class CSP_Logger;

class DLL_PREFIX SOUND_OUTPUT_MODULE_BASE : public QObject
{
	Q_OBJECT
protected:
	OSD_BASE    *m_OSD;
	USING_FLAGS *m_using_flags;
	CSP_Logger  *m_logger;

	std::atomic_bool     m_config_ok;
	std::atomic<void *>  m_extconfig;
	int	m_rate;
	int m_channels;
	int	m_latency_ms;
	std::recursive_mutex             m_locker;
	std::atomic_int	m_loglevel;
	std::atomic_int m_logdomain;
public:
	SOUND_OUTPUT_MODULE_BASE(
		OSD_BASE *parent,
		USING_FLAGS *pflags,
		CSP_Logger *logger,
		int base_rate = 48000,
		int base_latency_ms = 100,
		int base_channels = 2,
		void *extra_configvalues = nullptr)
		: m_OSD(parent), 
		m_using_flags(pflags),
		m_logger(logger),
		QObject(qobject_cast<QObject*>parent),
	{
		if(m_logger != nullptr) {
			QObject::connect(this, SIGNAL(sig_send_log(int, int, QString)),
							 m_logger, SLOT(do_debug_log(int, int, QString)),
							 Qt::QueueedConnection);
		}
		m_rate = base_rate;
		m_latency_ms = base_latency_ms;
		m_channels = base_channels;
		m_extconfig = configvalues;
		m_config_ok = real_reconfig_sound(m_rate, m_channels, m_latency_ms);
		m_loglevel = CSP_LOG_INFO;
		m_logdomain = CSP_LOG_TYPE_SOUND;
	}
	~SOUND_OUTPUT_MODULE_BASE()
	{
	}
	
	int get_sound_rate()
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker);
		return m_rate;
	}
	int get_latency()
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker);
		return m_latency_ms;
	}
	int get_channels()
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker);
		return m_channels;
	}
	virtual bool real_reconfig_sound(int& rate,int& channels,int& latency_ms) {
		return true;
	}	
	template <class... Args>
		bool debug_log(Args... args)
	{
		_TCHAR buf[512] = {0};
		my_sprintf_s(buf, sizeof(buf) - 1, args); 
		QString s = QString::fromUtf8(buf);
		emit sig_send_log(m_loglevel, m_logdomain, s);
		return true;
	}
	template <class... Args>
		bool debug_log(imt level, int domain, Args... args)
	{
		_TCHAR buf[512] = {0};
		my_sprintf_s(buf, sizeof(buf) - 1, args); 
		QString s = QString::fromUtf8(buf);
		emit sig_send_log(level, domain, s);
		return true;
	}
	bool config_ok()
	{
		return m_config_ok.load();
	}
public slot:
	bool update_rate(int& rate)
	{
		return reconfig_sound(rate, m_channels, m_latency_ms);
	}
	bool update_latency(int& latency_ms)
	{
		return reconfig_sound(m_rate, m_channels, latency_ms);
	}
	bool update_channels(int& channels)
	{
		return reconfig_sound(rate, m_channels, m_latency_ms);
	}
	bool reconfig_sound(int& rate, int& channels, int& latency_ms)
	{
		// ToDo
		std::lock_guard<std::recursive_mutex> locker(m_locker);
		if((rate != m_rate) || (channels != m_channels) || (latency_ms != m_latency_ms)) {
			if(real_reconfig_sound(rate, channels, latency_ms)) {
				m_rate = rate;
				m_channels = channels;
				m_latency_ms = latency_ms;
				m_config_ok = true;
				return true;
			}
		}
		return false;
	}

	virtual void initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples) {}
	virtual void release_sound() {}

	virtual void update_sound(int* extra_frames) {}
	virtual void mute_sound() {}
	virtual void stop_sound() {}

	// *PURE* SLOTS
	virtual void do_start_recording_sound() {}
	virtual void do_stop_recording_sound() {}
	virtual void do_restart_recording_sound() {}
	virtual void do_request_capture_sound(int ch) {}
	
	virtual void set_logger(CSP_Logger* logger)
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker);
		if(m_logger != nullptr) {
			QObject::disconnect(this, SIGNAL(sig_send_log(int, int, QString)),
								m_logger, SLOT(do_debug_log(int, int, QString)));
		}
		m_logger = logger;
		if(m_logger != nullptr) {
			QObject::connect(this, SIGNAL(sig_send_log(int, int, QString)),
							 m_logger, SLOT(do_debug_log(int, int, QString)),
							 Qt::QueueedConnection);
		}
	}
	virtual void set_osd(OSD_BASE* p)
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker);
		m_OSD = p;
	}

	virtual void set_system_flags(USING_FLAGS* p)
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker);
		m_using_flags = p;
	}
	virtual void update_extra_config(void* p)
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker);
		m_extconfig = p;
		// more lock via m_locker_outqueue etc, if needs.
	}
	virtual int result_opening_external_file()
	{
		return 0;
	}
	virtual int64_t wrote_data_to()
	{
		return 0;
	}
	virtual bool result_closing_external_file()
	{
		return true;
	}
signals:
	// loglevel, logdomain, message
	void sig_send_log(int, int, QString);
	// rate, channels, path
	void sig_req_open_sound(int, int, QString);
	// 
	void sig_req_close_sound();
	// samples, channel, data
	void sig_send_output_sound_data(int64_t, int, int16_t[]);
};

QT_END_NAMESPACE

