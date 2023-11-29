/*
 * Templates of FIFO and RING-BUFFER.
 * (C) 2022-07-16 K.Ohta <whatisthis.sowhat@gmail.com>
 * LICENSE: GPLv2
 *
 * Description:
 *  This presents reference FIFO and RING-BUFFER,
 *  both
 *    unmutexed (faster; using from same thread)
 *  and
 *    mutexed (slower;  using from another threads, i.e: mailbox)
 *  This aims to be skelton of FIFO:: class and RINGBUFFER:: class.

 * ToDo:
 *  * Will support alignment for inrernal buffer.
 */

#pragma once

//#include "fileio.h"
#include <mutex>
#include <limits.h>
#include "./common.h"


namespace FIFO_BASE {
	template <class T >
	class FIFO_INTERFACE {
	private:
		std::atomic<ssize_t> m_bufSize;
	protected:
		std::shared_ptr<T> m_buf;
		std::atomic<ssize_t> m_low_warning;
		std::atomic<ssize_t> m_high_warning;
		size_t m_rptr;
		size_t m_wptr;

		std::atomic<bool> m_is_ringbuffer;
		std::atomic<ssize_t> m_dataCount;
		// Use only for LOCKED_FOO, not using for UNLOCKED_FOO .
		std::recursive_mutex m_locker;

		inline bool is_ringbuffer() const {
			return m_is_ringbuffer.load();
		}
		inline ssize_t get_buffer_size() {
			return m_bufSize.load();
		}
		ssize_t realloc_buffer_size(size_t _count, bool force = false) {
			__UNLIKELY_IF(_count == 0) {
				if(force) {
					m_buf.reset();
					m_bufSize = 0;
				}
			} else {
				__UNLIKELY_IF(_count > SSIZE_MAX) {
					_count = SSIZE_MAX;
				}
				if((_count != m_bufSize.load()) || (force)) {
					T* p = nullptr;
					try {
						p = new T(_count);
					} catch (std::bad_alloc& e) {
						_count = 0;
					}
					if((p == nullptr) && (force)) {
						m_buf.reset();
						m_bufSize = 0;
					} else if((p != nullptr) && (_count != 0)) {
						m_buf.reset(p);
						m_bufSize = _count;
					} else {
						// DO NOTHING
					}
				}
			}
			return m_bufSize.load();
		}
		void reset_buffer() {
			m_buf.reset();
			m_bufSize = 0;
		}
		constexpr bool check_data_available(size_t _count = 1) {
			__LIKELY_IF(check_buffer_available()) {
				__UNLIKELY_IF((_count > SSIZE_MAX) || (_count == 0)) {
					return false;
				}
				return (m_dataCount.load() >= ((ssize_t)_count)) ? true : false;
			}
			return false;
		}
		constexpr bool check_buffer_available() {
			bool success = ((m_bufSize.get() <= 0) || (m_buf.get() == nullptr)) ? false : true;
			return success;
		}
		constexpr bool check_data_writable(size_t _count = 1) {
			__UNLIKELY_IF((_count == 0) || (_count > SSIZE_MAX)) {
				return false;
			}
			__LIKELY_IF(check_buffer_available()) {
				ssize_t _dcount = m_dataCount.load();
				__UNLIKELY_IF(_dcount <= 0) {
					m_dataCount = 0;
					_dcount = 0;
				}
				ssize_t _size = m_bufSize.load();
				if(is_ringbuffer()) {
					__UNLIKELY_IF(_size <= 0) {
						return false;
					}
					return (_count <= (size_t)_size) ? true : false;
				} else {
					return ((_dcount + _count) <= _size) ? true : false;
				}
			}
			return false;
		}

		inline void check_offset(size_t& offset)
		{
			__UNLIKELY_IF(m_bufSize <= 0) {
				offset = 0;
			} else {
				offset = offset % m_bufSize;
			}
		}
		inline bool check_readable_data_count(T* dst, size_t _count) {
			__UNLIKELY_IF(dst == nullptr) {
				return false;
			}
			return check_data_available(_count);
		}
		virtual T unlocked_read_base(void) {
			return (T)0;
		}
		virtual T unlocked_read_base(bool& success) {
			success = check_data_available();
			return unlocked_read_base();
		}
		virtual T locked_read_base(void) {
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return unlocked_read_base();
		}
		virtual T locked_read_base(bool& success) {
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return unlocked_read_base(success);
		}
		virtual T unlocked_read_not_remove_base(size_t offset) {
			check_offset(offset);
			return (T)0;
		}
		virtual T unlocked_read_not_remove_base(size_t offset, bool& success) {
			success = check_buffer_available();
			return unlocked_read_not_remove_base(offset);
		}
		virtual T locked_read_not_remove_base(size_t offset) {
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return unlocked_read_not_remove_base(offset);
		}
		virtual T locked_read_not_remove_base(size_t offset, bool& success) {

			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return unlocked_read_not_remove_base(offset, success);
		}
		virtual size_t unlocked_read_to_buffer_base(T* dst, size_t _count, bool& success) {
				success = check_readable_data_count(dst, _count);
				__UNLIKELY_IF(!(success)) {
					return 0;
				}
				return 0; // ToDo
		}
		virtual size_t locked_read_to_buffer_base(T* dst, size_t _count, bool& success) {
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return unlocked_read_to_buffer_base(dst, _count, success);
		}
		virtual bool unlocked_write_base(T data) {
			bool success = check_data_writable();
			return success;
		}
		virtual bool locked_write_base(T data) {
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return unlocked_write_base(data);
		}
		virtual bool unlocked_write_not_push_base(size_t offset, T data)	{
			bool success = check_buffer_available();
			check_offset(offset);
			return success;
		}
		virtual bool locked_write_not_push_base(size_t offset, T data)	{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return unlocked_write_not_push_base(offset, data);
		}
		virtual size_t unlocked_write_from_buffer_base(T* src, size_t _count, bool& success) {
			__UNLIKELY_IF(src == nullptr) {
				success = false;
				return 0;
			}
			success = check_data_writable(_count);
			__UNLIKELY_IF(!(success)) {
				return 0;
			}
			return _count;
		}
		virtual size_t locked_write_from_buffer_base(T* src, size_t _count, bool& success) {
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return unlocked_write_from_buffer_base(src, _count, success);
		}
	public:
		FIFO_INTERFACE(size_t _size) :
			m_rptr(0), m_wptr(0), m_dataCount(0),
			m_high_warning(SSIZE_MAX - 1), m_low_warning(SSIZE_MIN + 1),
			m_is_ringbuffer(false)
		{
			bool is_legal = true;
			m_buf.reset();
			ssize_t bsize = realloc_buffer_size(_size, true);
			if((bsize <= 0) || (bsize != _size) || (m.buf.get() == nullptr)) {
				is_legal = false;
			}
			if(!(is_legal)) {
				m_high_warning = SSIZE_MIN;
				m_low_warning = SSIZE_MAX;
				reset_buffer();
			}
			initialize();
		}
		~FIFO_INTERFACE()
		{
			release();
		}
		virtual void initialize() {}
		virtual void release() {}
		virtual void clear() {
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			m_rptr = 0;
			m_wptr = 0;
			m_dataCount = 0;
			__UNLIKELY_IF(!(check_buffer_available())) {
				return;
			}
			for(int i = 0; i < m_bufSize; i++) {
				m_buf[i] = (T)0;
			}
		}
		virtual T read(bool& success) {
			return unlocked_read_base(success);
		}
		virtual T read(void) {
			return unlocked_read_base();
		}
		virtual T read_not_remove(size_t offset, bool& success) {
			return unlocked_read_not_remove_base(offset, success);
		}
		virtual T read_not_remove(size_t offset) {
			return unlocked_read_not_remove_base(offset);
		}
		virtual size_t read_to_buffer(T* dst, size_t _count, bool& success)
		{
			return unlocked_read_to_buffer_base(dst, _count, success);
		}
		virtual size_t read_to_buffer(T* dst, size_t _count) {
			bool dummy;
			return read_to_buffer(dst, _count, dummy);
		}

		virtual bool write(T data) {
			return unlocked_write_base(data);
		}
		virtual bool write_not_push(int offset, T data)	{
			return unlocked_write_not_push_base(offset, data);
		}
		virtual size_t write_from_buffer(T* src, size_t _count, bool& success) {
			return unlocked_write_from_buffer_base(src, _count, success);

		}
		virtual size_t write_from_buffer(T* src, size_t _count) {
			bool dummy;
			return write_from_buffer(src, _count, dummy);
		}
		bool available()
		{
			return check_buffer_available();
		}
		bool empty()
		{
			return (check_data_available()) ? false : true;
		}
		bool read_ready(size_t _count = 1)
		{
			return check_data_available(_count);
		}
		virtual bool write_ready(size_t _count = 1)
		{
			return check_data_writable(_count);
		}
		bool full()
		{
			bool result = ((is_ringbuffer()) || !(check_data_writable()));
			return result;
		}

		size_t count()
		{
			ssize_t _count = m_dataCount.load();
			__UNLIKELY_IF(_count < 0) {
				return 0;
			}
			return (size_t)_count;
		}
		size_t fifo_size()
		{
			ssize_t _size = m_bufSize.load();
			__UNLIKELY_IF(_size < 0) {
				return 0;
			}
			__UNLIKELY_IF(_size > SSIZE_MAX) {
				return SSIZE_MAX;
			}
			return (size_t)_size;
		}
		size_t left()
		{
			__UNLIKELY_IF(!(check_buffer_available())) {
				return 0;
			}
			ssize_t _size = get_buffer_size();
			if(is_ringbuffer()) {
				__UNLIKELY_IF(_size == SSIZE_MAX) {
					return SSIZE_MAX;
				}
			} else {
				ssize_t _count = m_dataCount.load();
				_size = _size - _count;
			}
			__UNLIKELY_IF(_size < 0) {
				return 0;
			}
			return _size;
		}
		void set_high_warn_value(ssize_t val = SSIZE_MAX - 1)
		{
			m_high_warning = val;
		}
		void set_low_warn_value(ssize_t val = SSIZE_MIN + 1)
		{
			m_low_warning = val;
		}
		bool high_warn()
		{
			return (m_high_warning.load() < m_dataCount.load()) ? true : false;
		}
		bool low_warn()	{
			return (m_low_warning.load() > m_dataCount.load()) ? true : false;
		}
		bool resize(size_t _size, bool force = false, ssize_t _low_warn = SSIZE_MIN + 1, ssize_t _high_warn = SSIZE_MAX - 1) {
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			ssize_t realsize = realloc_buffer_size(_size, force);
			if(realsize <= 0) {
				set_high_warn_value();
				set_low_warn_value();
				return false;
			}
			if((size_t)realsize != _size) {
				set_high_warn_value();
				set_low_warn_value();
				return false;
			}
			set_high_warn_value(_high_warn);
			set_low_warn_value(_low_warn);
			return true;
		}
	};

	template <class T >
	class UNLOCKED_FIFO : publc FIFO_INTERFACE<T> {
	protected:
		virtual T unlocked_read_base() override
		{
			bool dummy = false;
			return unlocked_read_base(dummy);
		}
		virtual T unlocked_read_base(bool& success) override
		{
			T tmpval = (T)0;
			success = false;
			__LIKELY_IF(check_data_available()) {
				size_t buf_size = (size_t)get_buffer_size();
				T* p = m_buf.get();
				__LIKELY_IF(m_rptr.load() < buf_size) {
					tmpval = p[m_rptr++];
					m_dataCount--;
					success = true;
					__UNLIKELY_IF(m_rptr.load() >= buf_size) {
						m_rptr = 0;
					}
				}
			}
			return tmpval;
		}
		virtual T unlocked_read_not_remove_base(size_t offset, bool& success) override
		{
			check_offset(offset);
			T tmpval = (T)0;
			success = check_buffer_available();
			__LIKELY_IF(success) {
				size_t rptr = m_rptr.load();
				rptr = optr + offset;
				ssize_t bufsize = get_buffer_size();
				__UNLIKELY_IF(bufsize <= 0) {
					success = false;
					return tmpval;
				}
				__UNLIKELY_IF(rptr >= bufsize) {
					rptr = rptr % bufsize;
				}
				T* p = m_buf.get();
				__UNLIKELY_IF(p == nullptr) {
					success = false;
				} else {
					tmpval = p[rptr];
					success = true;
				}
			}
			return tmpval;
		}
		virtual T unlocked_read_not_remove_base(size_t offset) override
		{
			bool dummy;
			return unlocked_read_not_remove_base(offset, dummy);
		}
		virtual size_t unlocked_read_to_buffer_base(T* dst, size_t _count, bool& success) override
		{
			success = false;
			__UNLIKELY_IF(dst == nullptr) {
				return 0;
			}
			success = check_buffer_available();
			__LIKELY_IF(success) {
				T* p = m_buf.get();
				size_t words = 0;
				for(; words < _count; words++) {
					__UNLIKELY_IF(m_dataCount.load() <= 0) {
						break;
					}
					__UNLIKELY_IF(m_rptr.load() >= buf_size) {
						m_rptr = 0;
					}
					dst[words] = p[m_rptr++];
					m_dataCount--;
				}
				__UNLIKELY_IF(words <= 0) {
					success = false;
					return 0;
				}
				return words;
			}
			return 0;
		}
		virtual size_t unlocked_read_to_buffer_base(T* dst, size_t _count) override
		{
			bool dummy;
			return unlocked_read_to_buffer_base(dst, _count, dummy);
		}
	public:
		UNLOCKED_FIFO(size_t _size) : FIFO_INTERFACE<T>
		{
		}
		~UNLOCKED_FIFO()
		{
		}
		//!< Read one data
		virtual bool write(T data)
		{
			__UNLIKELY_IF((m_buf == nullptr) || (m_dataCount >= (int)m_bufSize)
						  || (m_bufSize == 0)) {
				return false;
			}
			__UNLIKELY_IF(m_dataCount < 0) {
				m_dataCount = 0; // OK?
			}
			m_buf[m_wptr++] = data;
			m_dataCount++;
			__UNLIKELY_IF(m_wptr >= m_bufSize) {
				m_wptr = 0;
			}
			__UNLIKELY_IF(m_dataCount >= (int)m_bufSize) {
				m_dataCount = (int)m_bufSize;
			}
			return true;
		}
		virtual bool write_not_push(int offset, T data)
		{
			__UNLIKELY_IF((m_buf == nullptr) ||
						  (m_bufSize == 0) || (offset < 0)) {
				return false;
			}
			unsigned int wp = m_wptr + offset;
			__UNLIKELY_IF(wp >= (int)m_bufSize) {
				wp = wp % m_bufSize;
			}
			m_buf[wp] = data;
			return true;
		}
		virtual int write_from_buffer(T* src, int _count, bool& success)
		{
			__UNLIKELY_IF((src == nullptr) || (_count <= 0) ||
						  (m_buf == nullptr) || (m_bufSize == 0) ||
						  (m_dataCount >= (int)m_bufSize)) {
				success = false;
				return 0;
			}
			__UNLIKELY_IF(m_dataCount < 0) {
				m_dataCount = 0; // OK?
			}
			__UNLIKELY_IF(_count > (int)m_bufSize) {
				_count = (int)m_bufSize;
			}
			__UNLIKELY_IF((_count + m_dataCount) >= (int)m_bufSize) {
				_count = (int)m_bufSize - m_dataCount;
				if(_count <= 0) {
					success = false;
					return 0;
				}
			}
			__UNLIKELY_IF(m_wptr >= m_bufSize) {
				m_wptr = m_wptr % m_bufSize;
			}
			// OK, Transfer
			int xptr = m_wptr;
			if((xptr + (unsigned int)_count) > m_bufSize) {
				int count1 = (int)(m_bufSize - (xptr % m_bufSize));
				int count2 = _count - count1;
				int rp = 0;
				for(int i = 0; i < count1; i++) {
					m_buf[xptr++] = src[rp++];
				}
				xptr = 0;
				for(int i = 0; i < count2; i++) {
					m_buf[xptr++] = src[rp++];
				}
			} else {
				// Inside buffer
				for(int i = 0; i < _count; i++) {
					m_buf[xptr++] = src[i];
				}
			}
			m_wptr = xptr % m_bufSize;
			m_dataCount += _count;
			__UNLIKELY_IF(m_dataCount > (int)m_bufSize) {
				m_dataCount = (int)m_bufSize;
			}
			success = true;
			return _count;
		}

	};

	template <class T >
	class LOCKED_FIFO : public UNLOCKED_FIFO<T> {
	protected:
	public:
		LOCKED_FIFO(int _size) :
		UNLOCKED_FIFO<T>(_size)
		{
		}
		~LOCKED_FIFO()
		{
		}
		virtual void initialize()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
		}
		virtual void release()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			UNLOCKED_FIFO<T>::release();
		}
		virtual void clear()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			UNLOCKED_FIFO<T>::clear();
		}
		virtual T read(bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::read(success);
		}
		virtual T read(void)
		{
			bool success;
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::read(success);
		}

		virtual T read_not_remove(int offset, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::read_not_remove(offset, success);
		}
		virtual T read_not_remove(int offset)
		{
			bool success;
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::read_not_remove(offset, success);
		}
		virtual int read_to_buffer(T* dst, int _count, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::read_to_buffer(dst, _count, success);
		}
		virtual bool write(T data)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::write(data);
		}
		virtual bool write_not_push(int offset, T data)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::write_not_push(offset, data);
		}
		virtual int write_from_buffer(T* src, int _count, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::write_from_buffer(src, _count, success);
		}
		virtual bool available()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::available();
		}
		virtual bool empty()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::empty();
		}
		virtual bool read_ready()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::read_ready();
		}
		virtual bool write_ready()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::write_ready();
		}

		virtual bool full()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::full();
		}
		virtual int count()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::count();
		}
		virtual int fifo_size()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::fifo_size();
		}
		virtual int left()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::left();
		}
		virtual void set_high_warn_value(int val = INT_MAX - 1)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::set_high_warn_value(val);
		}
		virtual void set_low_warn_value(int val = INT_MIN + 1)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::set_low_warn_value(val);
		}
		virtual bool high_warn()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::high_warn();
		}
		virtual bool low_warn()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::low_warn();
		}
		virtual bool resize(int _size, int _low_warn = INT_MIN + 1, int _high_warn = INT_MAX - 1)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO<T>::resize(_size, _low_warn, _high_warn);
		}
	};

	template <class T >
	class UNLOCKED_RINGBUFFER : public UNLOCKED_FIFO<T> {
	public:
		UNLOCKED_RINGBUFFER(int _size) :
		UNLOCKED_FIFO<T>(_size)
		{
		}
		~UNLOCKED_RINGBUFFER()
		{
		}
		virtual void initialize()
		{
			UNLOCKED_FIFO<T>::initialize();
		}
		virtual void release()
		{
			UNLOCKED_FIFO<T>::release();
		}
		// RINGBUFFER :  Even write to buffer when full.
		virtual bool write(T data)
		{
			__UNLIKELY_IF((UNLOCKED_FIFO<T>::m_buf == nullptr) || (UNLOCKED_FIFO<T>::m_bufSize == 0)) {
				return false;
			}

			__UNLIKELY_IF(UNLOCKED_FIFO<T>::m_dataCount < 0) {
				UNLOCKED_FIFO<T>::m_dataCount = 0; // OK?
			}
			UNLOCKED_FIFO<T>::m_buf[UNLOCKED_FIFO<T>::m_wptr++] = data;
			UNLOCKED_FIFO<T>::m_dataCount++;
			__UNLIKELY_IF(UNLOCKED_FIFO<T>::m_wptr >= UNLOCKED_FIFO<T>::m_bufSize) {
				UNLOCKED_FIFO<T>::m_wptr = UNLOCKED_FIFO<T>::m_wptr % UNLOCKED_FIFO<T>::m_bufSize;
			}
			__UNLIKELY_IF(UNLOCKED_FIFO<T>::m_dataCount > (int)UNLOCKED_FIFO<T>::m_bufSize) {
				UNLOCKED_FIFO<T>::m_dataCount = (int)UNLOCKED_FIFO<T>::m_bufSize;
			}
			return true;
		}
		virtual bool write_not_push(int offset, T data)
		{
			__UNLIKELY_IF((UNLOCKED_FIFO<T>::m_buf == nullptr) ||
						  (UNLOCKED_FIFO<T>::m_bufSize == 0) || (offset < 0)) {
				return false;
			}
			unsigned int wp = UNLOCKED_FIFO<T>::m_wptr + offset;
			__UNLIKELY_IF(wp >= (int)UNLOCKED_FIFO<T>::m_bufSize) {
				wp = wp % UNLOCKED_FIFO<T>::m_bufSize;
			}
			UNLOCKED_FIFO<T>::m_buf[wp] = data;
			return true;
		}
		virtual int write_from_buffer(T* src, int _count, bool& success)
		{
			__UNLIKELY_IF((src == nullptr) || (_count <= 0) ||
			   (UNLOCKED_FIFO<T>::m_buf == nullptr) || (UNLOCKED_FIFO<T>::m_bufSize == 0)) {
				success = false;
				return 0;
			}
			__UNLIKELY_IF(UNLOCKED_FIFO<T>::m_dataCount < 0) {
				UNLOCKED_FIFO<T>::m_dataCount = 0; // OK?
			}
			__UNLIKELY_IF(_count > (int)UNLOCKED_FIFO<T>::m_bufSize) {
				_count = (int)UNLOCKED_FIFO<T>::m_bufSize;
				__UNLIKELY_IF(_count <= 0) {
					success = false;
					return 0;
				}
			}
			__UNLIKELY_IF(UNLOCKED_FIFO<T>::m_wptr >= UNLOCKED_FIFO<T>::m_bufSize) {
				UNLOCKED_FIFO<T>::m_wptr = UNLOCKED_FIFO<T>::m_wptr % UNLOCKED_FIFO<T>::m_bufSize;
			}
			// OK, Transfer
			unsigned int xptr = UNLOCKED_FIFO<T>::m_wptr;
			if((xptr + (unsigned int)_count) >= UNLOCKED_FIFO<T>::m_bufSize) {
				int count1 = (int)(UNLOCKED_FIFO<T>::m_bufSize - xptr);
				int count2 = _count - count1;
				int rp = 0;
				for(int i = 0; i < count1; i++) {
					UNLOCKED_FIFO<T>::m_buf[xptr++] = src[rp++];
				}
				xptr = 0;
				for(int i = 0; i < count2; i++) {
					UNLOCKED_FIFO<T>::m_buf[xptr++] = src[rp++];
				}
			} else {
				// Inside buffer
				for(int i = 0; i < _count; i++) {
					UNLOCKED_FIFO<T>::m_buf[xptr++] = src[i];
				}
			}
			UNLOCKED_FIFO<T>::m_dataCount += _count;
			UNLOCKED_FIFO<T>::m_wptr = (xptr % UNLOCKED_FIFO<T>::m_bufSize);
			__UNLIKELY_IF(UNLOCKED_FIFO<T>::m_dataCount >= (int)UNLOCKED_FIFO<T>::m_bufSize) {
				UNLOCKED_FIFO<T>::m_dataCount = UNLOCKED_FIFO<T>::m_bufSize;
			}
			success = true;
			return _count;
		}
		virtual bool write_ready()
		{
			bool f = UNLOCKED_FIFO<T>::available();
			return f; // OK?
		}
		virtual bool full()
		{
			return false; // OK?
		}
		virtual int left()
		{
			__UNLIKELY_IF((UNLOCKED_FIFO<T>::m_bufSize == 0) || (UNLOCKED_FIFO<T>::m_buf == nullptr)) {
				return 0;
			}
			return (int)UNLOCKED_FIFO<T>::m_bufSize;
		}
	};

	template <class T >
	class LOCKED_RINGBUFFER : public UNLOCKED_RINGBUFFER<T> {
	protected:
		std::recursive_mutex m_locker;
	public:
		LOCKED_RINGBUFFER(int _size) :
		UNLOCKED_RINGBUFFER<T>(_size)
		{
		}
		~LOCKED_RINGBUFFER()
		{
		}
		virtual void initialize()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			UNLOCKED_RINGBUFFER<T>::initialize();
		}
		virtual void release()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			UNLOCKED_RINGBUFFER<T>::release();
		}
		virtual void clear()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			UNLOCKED_RINGBUFFER<T>::clear();
		}

		virtual T read(bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::read(success);
		}
		virtual T read(void)
		{
			bool success;
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::read(success);
		}
		virtual T read_not_remove(int offset, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::read_not_remove(offset, success);
		}
		virtual T read_not_remove(int offset)
		{
			bool success;
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::read_not_remove(offset, success);
		}

		virtual int read_to_buffer(T* dst, int _count, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::read_to_buffer(dst, _count, success);
		}

		virtual bool write(T data)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::write(data);
		}
		virtual bool write_not_push(int offset, T data)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::write_not_push(offset, data);
		}
		virtual int write_from_buffer(T* src, int _count, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::write_from_buffer(src, _count, success);
		}
		virtual bool available()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::available();
		}
		virtual bool empty()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::empty();
		}
		virtual bool read_ready()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::read_ready();
		}
		virtual bool write_ready()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::write_ready();
		}
		virtual bool full()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::full();
		}
		virtual int count()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::count();
		}
		virtual int fifo_size()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::fifo_size();
		}
		virtual int left()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::left();
		}

		virtual void set_high_warn_value(int val = INT_MAX - 1)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::set_high_warn_value(val);
		}
		virtual void set_low_warn_value(int val = INT_MIN + 1)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::set_low_warn_value(val);
		}
		virtual bool high_warn()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::high_warn();
		}
		virtual bool low_warn()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::low_warn();
		}
		virtual bool resize(int _size, int _low_warn = INT_MIN + 1, int _high_warn = INT_MAX - 1)
 		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER<T>::resize(_size, _low_warn, _high_warn);
		}
	};
}
