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
	class UNLOCKED_FIFO {
	protected:
		T* m_buf;
		unsigned int m_bufSize;
		unsigned int m_rptr;
		unsigned int m_wptr;
		int m_low_warning;
		int m_high_warning;
		int m_dataCount;
	public:
		UNLOCKED_FIFO(int _size) :
			m_rptr(0), m_wptr(0), m_dataCount(0),
			m_high_warning(INT_MAX - 1), m_low_warning(INT_MIN + 1)
		{
			bool is_legal = true;
			m_buf = nullptr;
			if((_size <= 0) || (_size == INT_MAX)) {
				is_legal = false;
			} else {
				m_bufSize = _size;
				try {
					m_buf = new T[_size];
				} catch (std::bad_alloc& e) {
					m_buf = nullptr;
					m_bufSize = 0;
					is_legal = false;
				}
			}
			if(!(is_legal)) {
				m_buf = nullptr;
				m_high_warning = INT_MIN;
				m_low_warning = INT_MAX;
				m_bufSize = 0;
			}
		}
		~UNLOCKED_FIFO()
		{
			release();
		}
		//!< Read one data
		virtual void initialize()
		{
		}
		virtual void release()
		{
			if(m_buf != nullptr) {
				delete[] m_buf;
				m_buf = nullptr;
			}
		}
		virtual void clear()
		{
			m_rptr = 0;
			m_wptr = 0;
			m_dataCount = 0;
			__UNLIKELY_IF((m_buf == nullptr) || (m_bufSize == 0)) {
				return;
			}
			for(int i = 0; i < m_bufSize; i++) {
				m_buf[i] = (T)0;
			}
		}

		virtual T read(bool& success)
		{
			__UNLIKELY_IF((m_buf == nullptr) || (m_dataCount < 1)
						  || (m_bufSize == 0)) {
				success = false;
				return (T)0;
			}
			T tmpval;
			tmpval = m_buf[m_rptr++];
			__UNLIKELY_IF(m_rptr >= m_bufSize) {
				m_rptr = 0;
			}
			m_dataCount--;
			__UNLIKELY_IF(m_dataCount < 0) {
				m_dataCount = 0;
			}
			success = true;
			return tmpval;
		}
		virtual T read(void)
		{
			bool dummy;
			return read(dummy);
		}

		virtual T read_not_remove(int offset, bool& success)
		{
			__UNLIKELY_IF((m_buf == nullptr) ||
						  (m_bufSize == 0) || (offset < 0)) {
				success = false;
				return (T)0;
			}
			T tmpval;
			unsigned int real_offset = offset + m_rptr;
			if(real_offset >= m_bufSize) {
				real_offset = real_offset % m_bufSize;
			}
			tmpval = m_buf[real_offset];
			success = true;
			return tmpval;
		}
		virtual T read_not_remove(int offset)
		{
			bool dummy;
			return read_not_remove(offset, dummy);
		}

		virtual int read_to_buffer(T* dst, int _count, bool& success)
		{
			__UNLIKELY_IF((dst == nullptr) || (m_buf == nullptr) || (m_bufSize == 0)) {
				success = false;
				return 0;
			}
			if(_count > m_dataCount) {
				_count = m_dataCount;
			}
			if(_count > (int)m_bufSize) {
				_count = (int)m_bufSize;
			}
			__UNLIKELY_IF(_count <= 0) {
				success = false;
				return 0;
			}
			// OK, Transfer
			unsigned int xptr = m_rptr;
			if((xptr + (unsigned int)_count) >= m_bufSize) {
				int count1 = (int)(m_bufSize - (xptr % m_bufSize));
				int count2 = _count - count1;
				int wp = 0;
				for(int i = 0; i < count1; i++) {
					dst[wp++] = m_buf[xptr++];
				}
				xptr = 0;
				for(int i = 0; i < count2; i++) {
					dst[wp++] = m_buf[xptr++];
				}
			} else {
				// Inside buffer
				for(int i = 0; i < _count; i++) {
					dst[i] = m_buf[xptr++];
				}
			}
			m_rptr = xptr % m_bufSize;
			m_dataCount -= _count;
			__UNLIKELY_IF(m_dataCount < 0) {
				m_dataCount = 0;
			}
			__UNLIKELY_IF(m_dataCount > (int)m_bufSize) {
				m_dataCount = (int)m_bufSize;
			}
			__UNLIKELY_IF(m_rptr >= m_bufSize) {
				m_rptr = m_rptr % m_bufSize;
			}
			success = true;
			return _count;
		}
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

		virtual bool available()
		{
			return ((m_buf != nullptr) && (m_bufSize > 0));
		}
		virtual bool empty()
		{
			bool f = available();
			return (!(f) || (m_dataCount <= 0));
		}
		virtual bool read_ready()
		{
			bool f = available();
			return ((f) && (m_dataCount > 0));
		}
		virtual bool write_ready()
		{
			bool f = available();
			return ((f) && (m_dataCount < (int)m_bufSize));
		}
		virtual bool full()
		{
			bool f = available();
			return (!(f) || (m_dataCount >= (int)m_bufSize));
		}

		virtual int count()
		{
			return (m_dataCount > 0) ? m_dataCount : 0;
		}

		virtual int fifo_size()
		{
			return (m_bufSize > 0) ? m_bufSize : 0;
		}

		virtual int left()
		{
			__UNLIKELY_IF((m_bufSize == 0) || (m_buf == nullptr)) {
				return 0;
			}
			__UNLIKELY_IF(m_dataCount < 0) {
				return (int)m_bufSize;
			}
			__UNLIKELY_IF(((unsigned int)m_dataCount) > m_bufSize) {
				return 0;
			}
			return (int)(m_bufSize - (unsigned int)m_dataCount);
		}
		virtual void set_high_warn_value(int val = INT_MAX - 1)
		{
			m_high_warning = val;
		}
		virtual void set_low_warn_value(int val = INT_MIN + 1)
		{
			m_low_warning = val;
		}
		virtual bool high_warn()
		{
			return (m_high_warning < m_dataCount) ? true : false;
		}
		virtual bool low_warn()
		{
			return (m_low_warning > m_dataCount) ? true : false;
		}
		virtual bool resize(int _size, int _low_warn = INT_MIN + 1, int _high_warn = INT_MAX - 1)
		{
			__UNLIKELY_IF((_size <= 0) || (_size >= INT_MAX)) {
				return false;
			}
			try {
				T *tmpptr = new T[_size];
				if(m_buf != nullptr) {
					delete[] m_buf;
				}
				m_buf = tmpptr;
			} catch (std::bad_alloc& e) {
				return false;
			}
			m_bufSize = (unsigned int)_size;
			m_low_warning = _low_warn;
			m_high_warning = _high_warn;
			return true;
		}
	};

	template <class T >
	class LOCKED_FIFO : public UNLOCKED_FIFO<T> {
	protected:
		std::recursive_mutex m_locker;
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
