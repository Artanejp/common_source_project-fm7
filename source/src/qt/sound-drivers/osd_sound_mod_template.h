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

#include "../../common.h"
#include "../../config.h"
#include "../osd_types.h"


#if !defined(__debug_log_func)
#define __debug_log_func(...) debug_log_func(__func__, __VA_ARGS__)
#endif



QT_BEGIN_NAMESPACE


class QIODevice;
class OSD_BASE;
class USING_FLAGS;
class CSP_Logger;

namespace SOUND_MODULE {
/* SOUND_MODULE */
	enum class __FORMAT;
	enum class __BYTEORDER;

class DLL_PREFIX M_BASE : public QObject
{
	Q_OBJECT
protected:
	OSD_BASE*						m_OSD;
	std::string						m_classname;
	std::shared_ptr<USING_FLAGS>	m_using_flags;
	std::shared_ptr<CSP_Logger>		m_logger;

	QIODevice*						m_sink_fileio;
	QIODevice*						m_source_fileio;
	std::atomic<bool>				m_sink_external_fileio;
	std::atomic<bool>				m_source_external_fileio;
	
	std::atomic<int>				m_loglevel;
	std::atomic<int>				m_logdomain;

	std::atomic<bool>				m_config_ok;
	
	std::atomic<bool>				m_prev_sink_started;
	std::atomic<bool>				m_prev_source_started;
	
	std::atomic<bool>				m_mute;

	std::atomic<size_t>				m_sink_chunk_bytes;
	std::atomic<size_t>				m_sink_buffer_bytes;
	std::atomic<size_t>				m_sink_before_rendered;
	std::atomic<size_t>				m_sink_samples;
	std::atomic<size_t>				m_sink_rate;
	std::atomic<size_t>				m_sink_latency_ms;
	std::atomic<size_t>				m_sink_channels;
	std::atomic<size_t>				m_sink_wordsize;
	std::atomic<double>				m_sink_volume;
	
	std::atomic<size_t>				m_source_chunk_bytes;
	std::atomic<size_t>				m_source_buffer_bytes;
	std::atomic<size_t>				m_source_before_rendered;
	std::atomic<size_t>				m_source_samples;
	std::atomic<size_t>				m_source_rate;
	std::atomic<size_t>				m_source_latency_ms;
	std::atomic<size_t>				m_source_channels;
	std::atomic<size_t>				m_source_wordsize;
	std::atomic<double>				m_source_volume;
	
	std::atomic<void*>				m_extconfig_ptr;
	std::atomic<size_t>				m_extconfig_bytes;
	
	
	std::string						m_sink_device_name;
	std::string						m_source_device_name;
	virtual void update_sink_driver_fileio()
	{
		// Update driver side of fileio by m_sink_fileio and m_source_fileio;
		//connect(m_sink_fileio.get(), SIGNAL(bytesWritten(qint64)), real_driver, SLOT, QObject::DirectConnection);
		//connect(m_sink_fileio.get(), SIGNAL(aboutToClose()), real_driver, SLOT, QObject::DirectConnection);
		// Optional:
		// connect(m_sink_fileio.get(), SIGNAL(readyRead()), real_driver, SLOT, QObject::DirectConnection);
	}
	virtual void update_source_driver_fileio()
	{
		// Update driver side of fileio by m_source_fileio and m_source_fileio;
	}
	// Maybe disconnect some signals via m_sink_fileio and m_source_fileio.
	virtual bool release_driver_fileio();
	virtual bool real_reconfig_sound(size_t& rate, size_t& channels, size_t& latency_ms, const bool force);
	
	virtual bool has_output_device(QString name) { return false; }
	virtual bool is_default_output_device() { return false; }
	virtual bool has_input_device(QString name) { return false; }
	virtual bool is_default_input_device() { return false; }

	virtual bool recalc_samples(size_t rate, size_t latency_ms, bool force = false);
	
	virtual bool reopen_sink_fileio(bool force_reopen = false);
	virtual bool reopen_source_fileio(bool force_reopen = false);

	bool debug_log_func(const _TCHAR *_funcname, const _TCHAR *_fmt, ...);
	bool debug_log(const _TCHAR *_fmt, ...);
	config_t*					 get_config_ptr();

public:
	M_BASE(QObject *parent,
		   OSD_BASE *osd,
		   QIODevice* sinkDeviceIO = nullptr,
		   QIODevice* sourceDeviceIO = nullptr,
		   size_t base_rate = 48000,
		   size_t base_latency_ms = 100,
		   size_t base_channels = 2,
		   void   *extra_config_values = nullptr,
		   size_t extra_config_bytes = 0);

	~M_BASE();

	std::recursive_timed_mutex				m_locker;

	virtual bool is_output_driver_started();
	virtual bool is_output_driver_stopped();
	virtual bool is_capture_driver_started();
	virtual bool is_capture_driver_stopped();
	
	virtual bool initialize_driver(QObject *parent)
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
	virtual void release_sink();
	virtual void release_source();

	virtual int64_t update_sound(void* datasrc, int samples);

	QIODevice* get_sink_io_device()
	{
		return m_sink_fileio;
	}
	QIODevice* get_source_io_device()
	{
		return m_source_fileio;
	}
	virtual bool is_sink_io_device_exists();
	virtual bool is_source_io_device_exists();


	bool config_ok()
	{
		return m_config_ok.load();
	}

	virtual size_t get_sink_buffer_bytes();
	virtual size_t get_sink_chunk_bytes()
	{
		return m_sink_chunk_bytes.load();
	}
	inline size_t get_sink_sample_count()
	{
		return m_sink_samples.load();
	}
	inline size_t get_sink_latency_ms()
	{
		return m_sink_latency_ms.load();
	}
	inline size_t get_sink_channels()
	{
		return m_sink_channels.load();
	}
	inline size_t get_sink_sample_rate()
	{
		return m_sink_rate.load();
	}
	inline size_t get_sink_word_size()
	{
		return m_sink_wordsize.load();
	}
	
	virtual size_t get_source_buffer_bytes();
	virtual size_t get_source_chunk_bytes()
	{
		return m_source_chunk_bytes.load();
	}
	inline size_t get_source_sample_count()
	{
		return m_source_samples.load();
	}
	inline size_t get_source_latency_ms()
	{
		return m_source_latency_ms.load();
	}
	inline size_t get_source_channels()
	{
		return m_source_channels.load();
	}
	inline size_t get_source_sample_rate()
	{
		return m_source_rate.load();
	}
	inline size_t get_source_word_size()
	{
		return m_source_wordsize.load();
	}
	
	virtual __FORMAT get_sink_sound_format();
	virtual __FORMAT get_source_sound_format();

	
	void get_sink_parameters(int& channels, int& rate, int& latency_ms,
							   size_t& word_size, int& chunk_bytes, int& buffer_bytes);
	
	void get_source_parameters(int& channels, int& rate, int& latency_ms,
							   size_t& word_size, int& chunk_bytes, int& buffer_bytes);
	
	virtual int64_t get_sink_bytes_left();
	virtual int64_t get_source_bytes_left();
	virtual int64_t get_sink_bytes_size();
	virtual int64_t get_source_bytes_size();

	virtual M_BASE* get_real_driver()
	{
		return dynamic_cast<SOUND_MODULE::M_BASE*>(this);
	}

	virtual std::list<std::string> get_sound_sink_devices_list()
	{
		static std::list<std::string> dummy_list;
		return dummy_list;
	}
	virtual std::list<std::string> get_sound_source_devices_list()
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
	virtual void do_about_to_quit();
	
	virtual void update_config() {}
	virtual void update_extra_config() {}
	virtual void release_sound();

	virtual void reset_to_defalut() {}
	virtual bool is_running_sound()
	{
		return true;
	}
	bool update_rate(size_t rate)
	{
		return reconfig_sound(rate, m_sink_channels.load());
	}
	bool update_channels(size_t channels)
	{
		return reconfig_sound(m_sink_rate.load(), channels);
	}
	virtual bool update_latency(size_t latency_ms, bool fortce = false);
	bool reconfig_sound(size_t rate, size_t channels);

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

	virtual void do_set_output_by_name(QString name) {};
	virtual void do_set_output_by_name(const _TCHAR *name)
	{
		if(name != nullptr) {
			do_set_output_by_name(QString::fromUtf8(name));
		}
	}
	virtual void do_set_output_by_name(const _TCHAR *name, int maxlen)
	{
		if((name != nullptr) && (maxlen > 0)) {
			do_set_output_by_name(QString::fromUtf8(name, maxlen));
		}
	}
	virtual void do_set_output_by_number(int) {};

	virtual void do_set_input_by_name(QString name) {};
	virtual void do_set_input_by_name(const _TCHAR *name)
	{
		if(name != nullptr) {
			do_set_input_by_name(QString::fromUtf8(name));
		}
	}
	virtual void do_set_input_by_name(const _TCHAR *name, int maxlen)
	{
		if((name != nullptr) && (maxlen > 0)) {
			do_set_input_by_name(QString::fromUtf8(name, maxlen));
		}
	}
	virtual void do_set_input_by_number(int) {};
	
	// This set device by device-name having QAction (as QObject).
	virtual void do_set_output_by_name(void);
	virtual void do_set_output_by_number(void);

	virtual void do_set_input_by_name(void);
	virtual void do_set_input_by_number(void);
	// From real driver: notify to update sound devices list.
	
	virtual void do_update_output_devices_list() {}
	virtual void do_update_input_devices_list() {}

	// Below SLOTs can call directry from OSD:: .
	virtual void set_osd(OSD_BASE* p);
	
	void set_sink_volume(double level);
	void set_sink_volume(int level);
	bool start_sink();
	void mute_sink();
	void unmute_sink();
	bool stop_sink();
	bool discard_sink();

signals:
	void sig_sound_finished();

	// loglevel, logdomain, message
	void sig_send_log(int, int, QString);
	void sig_send_log(int, int, const _TCHAR*, int);
	// rate, channels, path
	void sig_req_open_sound(int, int, QString);
	
	// To real drivers (SINK/OUTPUT)
	void sig_start_sink();
	void sig_stop_sink();
	void sig_mute_sink();
	void sig_unmute_sink();
	void sig_discard_sink();
	void sig_set_sink_volume(double);
	
	// To real drivers (SOURCE/INPUT)
	void sig_start_source();
	void sig_stop_source();
	void sig_mute_source();
	void sig_unmute_source();
	void sig_discard_source();
	void sig_set_source_volume(double);

	// To notify to OSD:: .
	void sig_sink_started();
	void sig_sink_stopped();
	void sig_sink_suspended();
	void sig_sink_resumed();
	
	void sig_output_devices_list_changed();
	void sig_input_devices_list_changed();

	void sig_sink_interruput_accpted();
	//void sig_sink_full();
	void sig_sink_empty();
	
	void sig_source_started();
	void sig_source_stopped();
	void sig_source_suspended();
	void sig_source_resumed();
	void sig_source_full();
	//void sig_source_empty();
	void sig_source_got_data(size_t bytes);
	//
	// notify completed to release sound driver.
	void sig_released(bool);
	// To UI: notify reset sound device list.
	void sig_update_output_devices_list();
	void sig_update_input_devices_list();
};
/* SOUND_MODULE */
}

QT_END_NAMESPACE
