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
	std::shared_ptr<USING_FLAGS> m_using_flags;
	std::shared_ptr<CSP_Logger>  m_logger;

	std::atomic_bool    m_config_ok;
	std::atomic<void *> m_extconfig;
	std::atomic<int>	m_rate;
	std::atomic<int>	m_channels;
	std::atomic<int>	m_latency_ms;
	std::recursive_mutex             m_locker;
	std::atomic<int>	m_loglevel;
	std::atomic<int>	m_logdomain;
	std::string			m_device_name;
public:
	SOUND_OUTPUT_MODULE_BASE(
		OSD_BASE *parent,
		_TCHAR* device_name,
		const std::shared_ptr<CSP_Logger> logger,
		const std::shared_ptr<USING_FLAGS> pflags,
		int base_rate = 48000,
		int base_latency_ms = 100,
		int base_channels = 2,
		void *extra_config_values = nullptr);
	~SOUND_OUTPUT_MODULE_BASE();

	virtual bool initialize_driver() { return true; }
	virtual bool release_driver() { return true; }
	int get_sound_rate()
	{
		return m_rate.load();
	}
	int get_latency()
	{
		return m_latency_ms.load();
	}
	int get_channels()
	{
		return m_channels.load();
	}
	virtual bool real_reconfig_sound(int& rate,int& channels,int& latency_ms)
	{
		return true;
	}
	template <class... Args>
		bool debug_log(Args... args)
	{
		_TCHAR buf[512] = {0};
		my_sprintf_s(buf, sizeof(buf) - 1, args);
		return do_send_log(m_loglevel.load(), m_logdomain.load(), (const _TCHAR*)buf, (sizeof(buf) / sizeof(_TCHAR)) - 1);
	}
	template <class... Args>
		bool debug_log(imt level, int domain, Args... args)
	{
		_TCHAR buf[512] = {0};
		my_sprintf_s(buf, sizeof(buf) - 1, args);
		return do_send_log(level, domain, (const _TCHAR*)buf, (sizeof(buf) / sizeof(_TCHAR)) - 1);
	}
	bool config_ok()
	{
		return m_config_ok.load();
	}
	virtual std::list<std::string> get_sound_devices_list()
	{
		static std::list<std::string> dummy_list;
		return dummy_list;
	}
	const _TCHAR* get_current_device_name()
	{
		return (const _TCHAR*)(m_device_name.c_str());
	}
	virtual void set_logger(const std::shared_ptr<CSP_Logger> logger);
	virtual void set_system_flags(const std::shared_ptr<USING_FLAGS> p);

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
		if((rate != m_rate.load()) || (channels != m_channels.load()) || (latency_ms != m_latency_ms.load())) {
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
	virtual const std::string set_device_sound(const _TCHAR* driver_name, int& rate,int& channels,int& latency_ms)
	{
		return std::string(_T("Empty Device"));
	}
	virtual bool do_send_log(imt level, int domain, const _TCHAR* str, int maxlen);
	virtual void do_set_device_by_name(QString) {};
	virtual void do_set_device_by_number(int) {};
	virtual void do_set_device_from_sender_object(void) {};
	
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
	
	virtual void set_osd(OSD_BASE* p);
	virtual void update_extra_config(void* p);
	virtual int result_opening_external_file();
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
	// update device list for ui
	void sig_clear_sound_device();
	// add/update device, number and name. If number < 0 append.
	void sig_add_sound_device_name(int, QString);
	// Send current device number and name to UI. 
	void sig_send_current_device_description(int, QString);
	// Notify updating devices list to UI. size, current_device_number, need_to_reconfig 
	void sig_notify_update_devices_list(int, int, bool);
	// notify device changed status, true = success.
	void sig_notify_changed_device_status(bool)
};

QT_END_NAMESPACE

