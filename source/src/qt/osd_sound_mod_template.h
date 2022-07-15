/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2022.07.15-

	[ OSD / Sound driver / Template ]
*/

#pragma once

#include <QObject>
#include <mutex>

#include "../common.h"

QT_BEGIN_NAMESPACE

namespace OSD_SOUND {
	template <class T >
		class RINGBUFFER {
		T* m_buf;
		int64_t m_bufSize;
		int64_t m_rptr;
		int64_t m_wptr;
		int64_t m_dataCount;
		std::recursive_mutex m_locker;
	public:
		RINGBUFFER(int64_t size = 4096) :
		m_bufSize(size), m_rptr(0), m_wptr(0), m_dataCount(0)
		{
			if(size <= 0) {
				m_buf = nullptr;
			} else {
				try {
					m_buf = new T[size];
				} catch (std::bad_alloc& e) {
					m_buf = nullptr;
				}
			}
		}
		~RINGBUFFER()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			if(m_buf != nullptr) {
				delete[] m_buf;
			}
		}
		//!< Read one data
		T read(bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			if((m_buf == nullptr) || (m_dataCount < 1) || (m_bufSize <= 0)) {
				success = false;
				return (T)0;
			}
			T tmpval;
			if(m_dataCount > m_bufSize) m_dataCount = m_bufSize; // OK?
			if(m_rptr >= m_bufSize) {
				m_rptr = (m_rptr - m_bufSize) % m_bufSize;
			} else if(m_rptr < 0) {
				m_rptr = (m_bufSize + m_rptr) % m_bufSize;
			}
			tmpval = m_buf[m_rptr++];
			m_dataCount--;
			success = true;
			return tmpval;
		}
		T read(void)
		{
			bool dummy;
			return read(dummy);
		}
		int64_t read(T* dst, int64_t count, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			if((dst == nullptr) || (count <= 0) || (m_buf == nullptr) || (m_bufSize <= 0)){
				success = false;
				return 0;
			}
			if(m_dataCount > m_bufSize) m_dataCount = m_bufSize; // OK?
			if(count > m_bufSize) {
				count = m_bufSize;
			}
			if(count > m_dataCount) {
				count = m_dataCount;
			}
			if(count <= 0) {
				success = false;
				return 0;
			}
			if(m_rptr >= m_bufSize) {
				m_rptr = (m_rptr - m_bufSize) % m_bufSize;
			} else if(m_rptr < 0) {
				m_rptr = (m_bufSize + m_rptr) % m_bufSize;
			}
			// OK, Transfer
			int64_t xptr = m_rptr;
			if((xptr + count) >= m_bufSize) {
				int64_t count2 = (xptr + count) - m_bufSize;
				int64_t count1 = count - count2;
				for(int64_t i = 0; i < count1; i++) {
					dst[i] = m_buf[xptr++];
				}
				xptr = 0;
				for(int64_t i = 0; i < count2; i++) {
					dst[i] = m_buf[xptr++];
				}
				m_rptr = xptr;
				m_dataCount -= count;

			} else {
				// Inside buffer
				for(int64_t i = 0; i < count; i++) {
					dst[i] = m_buf[xptr++];
				}
				m_dataCount -= count;
				m_rptr = xptr;
			}
			success = true;
			return count;
		}
		bool write(T data)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			if((m_buf == nullptr) || (m_dataCount >= m_bufSize) || (m_bufSize <= 0)) {
				return false;
			}
			if(m_dataCount < 0) m_dataCount = 0; // OK?
			if(m_wptr >= m_bufSize) {
				m_wptr = (m_wptr - m_bufSize) % m_bufSize;
			} else if(m_wptr < 0) {
				m_wptr = (m_bufSize + -m_wptr) % m_bufSize;
			}
			m_buf[m_wptr++] = data;
			m_dataCount++;
			return true;
		}
		int64_t write(T* src, int64_t count, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			if((src == nullptr) || (count <= 0) || (m_buf == nullptr) || (m_bufSize <= 0)){
				success = false;
				return 0;
			}
			if(m_dataCount < 0) m_dataCount = 0; // OK?
			if(count > m_bufSize) {
				count = m_bufSize;
			}
			if((count + m_dataCount) >= m_bufSize) {
				count = m_bufSize - m_dataCount;
			}
			if(count <= 0) {
				success = false;
				return 0;
			}
			if(m_wptr >= m_bufSize) {
				m_wptr = (m_wptr - m_bufSize) % m_bufSize;
			} else if(m_wptr < 0) {
				m_wptr = (m_bufSize + m_wptr) % m_bufSize;
			}
			// OK, Transfer
			int64_t xptr = m_wptr;
			if((xptr + count) >= m_bufSize) {
				int64_t count2 = (xptr + count) - m_bufSize;
				int64_t count1 = count - count2;
				for(int64_t i = 0; i < count1; i++) {
					m_buf[xptr++] = src[i];
				}
				xptr = 0;
				for(int64_t i = 0; i < count2; i++) {
					m_buf[xptr++] = src[i];
				}
				m_wptr = xptr;
				m_dataCount += count;

			} else {
				// Inside buffer
				for(int64_t i = 0; i < count; i++) {
					m_buf[xptr++] = src[i];
				}
				m_dataCount += count;
				m_wptr = xptr;
			}
			success = true;
			return count;
		}

		
		bool available()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return ((m_buf != nullptr) && (m_bufSize > 0));
		}
		bool read_ready()
		{
			bool f = available();
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return ((f) && (m_dataCount > 0));
		}
		bool write_ready()
		{
			bool f = available();
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return ((f) && (m_dataCount < m_bufSize));
		}
		
		int64_t count()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return (m_dataCount > 0) ? m_dataCount : 0;
		}
		int64_t size()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return (m_bufSize > 0) ? m_bufSize : 0;
		}
		bool resize(int64_t size)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			if(size <= 0) retrun false;
			try {
				T *tmpptr = new T[size];
			} catch (std::bad_alloc& e) {
				return false;
			}
			if(m_buf != nullptr) {
				delete[] m_buf;
			}
			m_buf = tmpptr;
			return true;
		}
	};
}

class OSD_BASE;
class USING_FLAGS;
class CSP_Logger;
class DLL_PREFIX SOUND_MODULE_BASE : public QObject
{
	Q_OBJECT
protected:
	OSD_BASE    *m_OSD;
	USING_FLAGS *m_using_flags;
	CSP_Logger  *m_logger;
	void        *m_extconfig;
	OSD_SOUND::RINGBUFFER<int16_t> *m_queue;
	std::recursive_mutex m_locker;
	std::recursive_mutex m_locker_outqueue;
public:
	SOUND_MODULE_BASE(OSD_BASE *parent, USING_FLAGS *pflags, CSP_Logger *logger, int64_t buffer_size = 4096, void *configvalues = nullptr)
		: m_OSD(parent), m_using_flags(pflags), m_logger(logger),
		m_extconfig(configvalues), 
		QObject(qobject_cast<QObject*>parent)
	{
		m_queue = new OSD_SOUND::RINGBUFFER<int16_t>(buffer_size);
	}
	~SOUND_MODULE_BASE()
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker);
		std::lock_guard<std::recursive_mutex> locker(m_locker_outqueue);
		if(m_queue != nullptr) {
			delete m_queue;
		}
	}
	virtual int get_sound_rate(bool is_input = false, int dev_channel = 0)
	{
		return 44100;
	}
	
	virtual int64_t enqueueOutputSink(int16_t* p, int64_t size)
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker_outqueue);
		if(m_queue != nullptr) {
			bool flag;
			int64_t wsize = m_queue->write(p, size, flag);
			if((wsize > size) || (wsize <= 0) || !(flag)) {
				return 0;
			}
			return wsize;
		}
		return 0;
	}
	
	virtual int64_t dequeueOutputSink(int16_t* p, int64_t size)
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker_outqueue);
		if(m_queue != nullptr) {
			bool flag;
			int64_t rsize = m_queue->read(p, size, flag);
			if((rsize > size) || (rsize <= 0) || !(flag)) {
				return 0;
			}
			return rsize;
		}
		return 0;
	}
	virtual int64_t enqueueInputSource(int device_ch, int16_t* p, int64_t size)
	{
		return 0;
	}
	virtual int64_t dequeueInputSource(int device_ch, int16_t* p, int64_t size)
	{
		return 0;
	}
	
	virtual int64_t outputSinkQueueSize()
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker_outqueue);
		if(m_queue != nullptr) {
			return m_queue->size();
		}
		return 0;
	}
	virtual int64_t outputSinkDataCount()
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker_outqueue);
		if(m_queue != nullptr) {
			return m_queue->count();
		}
		return 0;
	}
	virtual bool outputSinkQueueAvailable()
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker_outqueue);
		if(m_queue != nullptr) {
			return m_queue->available();
		}
		return false;
	}
	virtual bool outputSinkQueueReadReady()
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker_outqueue);
		if(m_queue != nullptr) {
			return m_queue->read_ready();
		}
		return false;
	}
	virtual bool outputSinkQueueWriteReady()
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker_outqueue);
		if(m_queue != nullptr) {
			return m_queue->write_ready();
		}
		return false;
	}


	virtual int64_t inputSourceQueueSize(int input_ch = 0)
	{
		return 0;
	}
	virtual int64_t inputSourceDataCount(int input_ch = 0)
	{
		return 0;
	}
	virtual bool inputSourceQueueAvailable(int input_ch = 0)
	{
		return false;
	}
	virtual bool inputSourceQueueReadReady(int input_ch = 0)
	{
		return false;
	}
	virtual bool inputSourceQueueWriteReady(int input_ch = 0)
	{
		return false;
	}
	
	virtual bool initalizeSoundOutputDevice(int channels, int sample_rate, int& samples_per_chunk, int& chunks, std::string device_name)
	{
		return true;
	}

	virtual bool initalizeSoundOutputDevice(int channels, int sample_rate, int& samples_per_chunk, int& chunks, int device_num)
	{
		return true;
	}
	virtual bool detachSoundOutputDevice()
	{
		return true;
	}
	virtual bool isSoundOutputDeviceReady()
	{
		return false;
	}

	virtual bool initalizeSoundInputDevice(int channels, int sample_rate, int& samples_per_chunk, int& chunks, std::string device_name)
	{
		return true;
	}

	virtual bool initalizeSoundInputDevice(int input_channel, int channels, int sample_rate, int& samples_per_chunk, int& chunks, int device_num)
	{
		return true;
	}
	virtual void detachSoundInputDevice(int input_channel = 0)
	{
		return true;
	}
	virtual bool isSoundInputDeviceReady(int input_channel = 0)
	{
		return false;
	}
	
	virtual void soundOutHandler(int64_t& req_size, void* userdata = nullptr)
	{
	}
	virtual void soundInHandler(int64_t &req_size, void* userdata = nullptr)
	{
	}
	// Kick sound out handler
	virtual bool soundOutReq()
	{
		return true;
	}
	// Kick sound in handler
	virtual bool soundInReq(int input_ch)
	{
		return true;
	}
public slot:
	virtual void initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples)
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker_outqueue);
		// more lock via m_locker etc, if needs.
		if(presented_rate != nullptr) {
			*presenyted_rate = rate;
		}
		if(presented_samples != nullptr) {
			*presenyted_samples = samples;
		}
	}
	virtual void update_sound(int* extra_frames) {}
	virtual void mute_sound() {}
	virtual void stop_sound() {}

	// *PURE* SLOTS
	virtual void do_start_recording_sound() {}
	virtual void do_stop_recording_sound() {}
	virtual void do_restart_recording_sound() {}
	virtual void do_request_capture_sound(int ch) {}
	virtual void do_resize_output_buffer(int count, int channels) {}
	virtual void do_resize_capture_buffer(int ch, int count, int channels) {}
	virtual void do_receive_external_sound(int count, int channels, int16_t* data) {}

	virtual void set_logger(CSP_Logger* logger)
	{
		std::lock_guard<std::recursive_mutex> locker(m_locker);
		m_logger = logger;
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
	
signals:
	void sig_send_log(QString);
	void sig_send_log_with_class(int, int, QString);
	void sig_req_input(int64_t);
	void sig_complete_output(int64_t);
	void sig_send_captured_sound_data(int, int64_t, int, int16_t[]);
	void sig_send_output_sound_data(int64_t, int, int16_t[]);
};

QT_END_NAMESPACE

