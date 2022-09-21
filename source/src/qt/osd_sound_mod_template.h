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
#include <string>

#include "../common.h"
#include "./config.h"

QT_BEGIN_NAMESPACE

class SOUND_BUFFER_QT;
class OSD_BASE;
class USING_FLAGS;
class CSP_Logger;

#define __debug_log_func(...) debug_log_func(__func__, __VA_ARGS__)

namespace SOUND_OUTPUT_MODULE {
class DLL_PREFIX M_BASE : public QObject
{
	Q_OBJECT
protected:
	OSD_BASE*							m_OSD;
	std::string							m_classname;
	
	std::shared_ptr<SOUND_BUFFER_QT>	m_fileio;
	std::shared_ptr<SOUND_BUFFER_QT>	m_driver_fileio;
	std::shared_ptr<USING_FLAGS>		m_using_flags;
	std::shared_ptr<CSP_Logger>			m_logger;
	
	std::atomic<bool>					m_config_ok;
	std::atomic<bool>					m_prev_started;
	std::atomic<bool>					m_mute;

	int64_t								m_chunk_bytes;
	int64_t								m_buffer_bytes;
	int64_t								m_before_rendered;
	int									m_samples;
	
	int									m_rate;
	int									m_latency_ms;
	int									m_channels;
	size_t								m_wordsize;
	std::atomic<void*>					m_extconfig_ptr;
	std::atomic<int>					m_extconfig_bytes;
	std::atomic<int>					m_loglevel;
	std::atomic<int>					m_logdomain;

	virtual void update_driver_fileio()
	{
		// Update driver side of fileio by m_fileio;
		//connect(m_fileio.get(), SIGNAL(bytesWritten(qint64)), real_driver, SLOT, QObject::DirectConnection);
		//connect(m_fileio.get(), SIGNAL(aboutToClose()), real_driver, SLOT, QObject::DirectConnection);
		// Optional:
		// connect(m_fileio.get(), SIGNAL(readyRead()), real_driver, SLOT, QObject::DirectConnection);
	}

	// Maybe disconnect some signals via m_fileio.
	virtual bool release_driver_fileio();
	virtual bool real_reconfig_sound(int& rate,int& channels,int& latency_ms);

	config_t*					 get_config_ptr();
	
	bool debug_log_func(const _TCHAR *_funcname, const _TCHAR *_fmt, ...);
	bool debug_log(const _TCHAR *_fmt, ...);
	
public:
	M_BASE(OSD_BASE *parent,
							 SOUND_BUFFER_QT* deviceIO = nullptr,
							 int base_rate = 48000,
							 int base_latency_ms = 100,
							 int base_channels = 2,
							 void *extra_config_values = nullptr,
							 int extra_config_bytes = 0);
	
	~M_BASE();

	std::recursive_timed_mutex				m_locker;

	virtual bool wait_driver_started(int64_t timeout_msec = INT64_MIN);
	virtual bool wait_driver_stopped(int64_t timeout_msec = INT64_MIN);
	virtual bool initialize_driver()
	{
		// AT LEAST:
		// connect(this, SIGNAL(sig_start_audio()), ..., QObject::QueuedConnection);
		// connect(this, SIGNAL(sig_pause_audio()), ..., QObject::QueuedConnection);
		// connect(this, SIGNAL(sig_resume_audio()), ..., QObject::QueuedConnection);
		// connect(this, SIGNAL(sig_close_audio()), ..., QObject::QueuedConnection);
		// connect(this, SIGNAL(sig_discard_audio()), ..., QObject::QueuedConnection);
		// connect(this, SIGNAL(sig_released(bool)), ..., QObject::QueuedConnection);
		// connect(this, SIGNAL(sig_req_open_sound(int, int, QString)), ..., QObject::QueuedConnection);

		// For Logging
		// connect(real_driver, SIGNAL(sig_log(QString)), this, SLOT(do_debug_log(QString)), QObject::QueuedConnection);
		// connect(real_driver, SIGNAL(sig_log(int, int, QString)), this, SLOT(do_debug_log(int, int, QString)), QObject::QueuedConnection);
		return true;
	}
	virtual bool release_driver()
	{
		// Maybe You should:
		// Stop driver,
		// then, m_fileio.reset() @ driver (not this).
		return true;
	}
	
	virtual int64_t update_sound(void* datasrc, int samples);
	
	std::shared_ptr<SOUND_BUFFER_QT> set_io_device(SOUND_BUFFER_QT *p);
	std::shared_ptr<SOUND_BUFFER_QT> set_io_device(std::shared_ptr<SOUND_BUFFER_QT> ps);
	std::shared_ptr<SOUND_BUFFER_QT> get_io_device()
	{
		return m_fileio;
	}
	bool is_io_device_exists();
	
	virtual uint64_t wrote_data_to()
	{
		return 0;
	}
	virtual int64_t driver_elapsed_usec()
	{
		return 0;
	}
	virtual int64_t driver_processed_usec()
	{
		return 0;
	}
	virtual bool check_elapsed_to_render();
	virtual void update_render_point_usec();
	virtual bool check_enough_to_render();
	
	bool config_ok()
	{
		return m_config_ok.load();
	}
	
	int64_t get_buffer_bytes();
	int64_t get_chunk_bytes();
	int get_sample_count() { return m_samples; }
	int get_latency_ms();
	int get_channels();
	int get_sample_rate();
	size_t get_word_size();
	void get_buffer_parameters(int& channels, int& rate, int& latency_ms,
							   size_t& word_size, int& chunk_bytes, int& buffer_bytes);
	virtual int64_t get_bytes_available();
	virtual int64_t get_bytes_left();
	
	virtual M_BASE* get_real_driver()
	{
		return dynamic_cast<SOUND_OUTPUT_MODULE::M_BASE*>(this);
	}

	virtual std::list<std::string> get_sound_devices_list()
	{
		static std::list<std::string> dummy_list;
		return dummy_list;
	}
	
	virtual const _TCHAR* get_sound_device_name(int num)
	{
		return (const _TCHAR*)nullptr;
	}
	virtual const _TCHAR* get_current_device_name()
	{
		return (const _TCHAR*)(_T("Empty"));
	}
	
	virtual void set_logger(const std::shared_ptr<CSP_Logger> logger);
	virtual void set_system_flags(const std::shared_ptr<USING_FLAGS> p);
	void* get_extra_config_ptr()
	{
		return m_extconfig_ptr.load();
	}
	int get_extra_config_bytes()
	{
		return m_extconfig_bytes.load();
	}
	virtual bool set_extra_config(void* p, int bytes);
	virtual bool modify_extra_config(void* p, int& bytes);
public slots:
	
	virtual void initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples);
	virtual void release_sound();
	virtual void mute_sound();
	virtual void stop_sound();
	
	virtual void update_config() {}
	virtual void update_extra_config() {}
	
	bool start();
	bool pause();
	bool resume();
	bool stop();
	bool discard();

	virtual void reset_to_defalut() {} 
	virtual void set_volume(double level);
	virtual void set_volume(int level);
	virtual bool is_running_sound()
	{
		return true;
	}
	bool update_rate(int rate)
	{
		return reconfig_sound(rate, m_channels);
	}
	bool update_channels(int channels)
	{
		return reconfig_sound(m_rate, channels);
	}
	bool update_latency(int latency_ms, bool fortce = false);
	bool reconfig_sound(int rate, int channels);
	void request_to_release();
	
	virtual bool do_send_log(int level, int domain, QString _str);
	virtual bool do_send_log(int level, int domain, const _TCHAR* _str, int maxlen);
	virtual bool do_send_log(const _TCHAR* _str, int maxlen)
	{
		return do_send_log(m_loglevel.load(), m_logdomain.load(), _str, maxlen);
	}
	virtual bool do_send_log(const QString _str)
	{
		return do_send_log(m_loglevel.load(), m_logdomain.load(), _str);
	}
	
	virtual void do_set_device_by_name(QString name) {};
	virtual void do_set_device_by_name(const _TCHAR *name)
	{
		if(name != nullptr) {
			do_set_device_by_name(QString::fromUtf8(name));
		}
	}
	virtual void do_set_device_by_name(const _TCHAR *name, int maxlen)
	{
		if((name != nullptr) && (maxlen > 0)) {
			do_set_device_by_name(QString::fromUtf8(name, maxlen));
		}
	}
	virtual void do_set_device_by_number(int) {};
	
	// This set device by device-name having QAction (as QObject).
	virtual void do_set_device_by_name(void);
	virtual void do_set_device_by_number(void);

	// From real driver: notify to update sound devices list.
	virtual void do_update_device_list() {}
	
	virtual void set_osd(OSD_BASE* p);

signals:
	// loglevel, logdomain, message
	void sig_send_log(int, int, QString);
	void sig_send_log(int, int, const _TCHAR*, int);
	// rate, channels, path
	void sig_req_open_sound(int, int, QString);
	//
	void sig_start_audio();
	void sig_stop_audio();
	void sig_pause_audio();
	void sig_resume_audio();
	void sig_close_audio();
	void sig_discard_audio();

	void sig_set_volume(double);
	// 
	// notify completed to release sound driver.
	void sig_released(bool);
	// To UI: notify reset sound device list.
	void sig_reset_sound_device_list();
	// To UI: notify update sound device list #arg1 to #arg2.
	void sig_set_sound_device(int, QString);
	// To UI: notify adding sound device list #arg1.
	void sig_add_sound_device(QString);
};
}
QT_END_NAMESPACE
	
