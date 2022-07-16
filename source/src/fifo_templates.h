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

namespace FIFO_BASE {
	
	template <class T >
	class UNLOCKED_FIFO {
	protected:
		T* m_buf;
		int m_bufSize;
		int m_rptr;
		int m_wptr;
		int m_dataCount;
	public:
		UNLOCKED_FIFO(int size) :
		m_bufSize(size), m_rptr(0), m_wptr(0), m_dataCount(0)
		{
			if(_size <= 0) {
				m_buf = nullptr;
			} else {
				try {
					m_buf = new T[_size];
				} catch (std::bad_alloc& e) {
					m_buf = nullptr;
				}
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
			}
		}
		virtual void clear()
		{
			m_rptr = 0;
			m_wptr = 0;
			m_dataCount = 0;
			if((m_buf == nullptr) || (m_bufSize <= 0)) {
				return;
			}
			for(int i = 0; i < m_bufSize; i++) {
				m_buf[i] = (T)0;
			}
		}
		
		virtual T read(bool& success)
		{
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

		virtual T read_not_remove(bool& success)
		{
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
			tmpval = m_buf[m_rptr];
			//m_dataCount--;
			success = true;
			return tmpval;
		}
		
		virtual T read(void)
		{
			bool dummy;
			return read(dummy);
		}
		
		virtual int read_to_buffer(T* dst, int count, bool& success)
		{
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
			int xptr = m_rptr;
			if((xptr + count) >= m_bufSize) {
				int count2 = (xptr + count) - m_bufSize;
				int count1 = count - count2;
				for(int i = 0; i < count1; i++) {
					dst[i] = m_buf[xptr++];
				}
				xptr = 0;
				for(int i = 0; i < count2; i++) {
					dst[i] = m_buf[xptr++];
				}
				m_rptr = xptr;
				m_dataCount -= count;

			} else {
				// Inside buffer
				for(int i = 0; i < count; i++) {
					dst[i] = m_buf[xptr++];
				}
				m_dataCount -= count;
				m_rptr = xptr;
			}
			success = true;
			return count;
		}
		virtual bool write(T data)
		{
			if((m_buf == nullptr) || (m_dataCount >= m_bufSize) || (m_bufSize <= 0)) {
				return false;
			}
			if(m_dataCount < 0) m_dataCount = 0; // OK?
			if(m_wptr >= m_bufSize) {
				m_wptr = (m_wptr - m_bufSize) % m_bufSize;
			} else if(m_wptr < 0) {
				m_wptr = (m_bufSize + m_wptr) % m_bufSize;
			}
			m_buf[m_wptr++] = data;
			m_dataCount++;
			return true;
		}
		virtual int write_from_buffer(T* src, int count, bool& success)
		{
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
			int xptr = m_wptr;
			if((xptr + count) >= m_bufSize) {
				int count2 = (xptr + count) - m_bufSize;
				int count1 = count - count2;
				for(int i = 0; i < count1; i++) {
					m_buf[xptr++] = src[i];
				}
				xptr = 0;
				for(int i = 0; i < count2; i++) {
					m_buf[xptr++] = src[i];
				}
				m_wptr = xptr;
				m_dataCount += count;

			} else {
				// Inside buffer
				for(int i = 0; i < count; i++) {
					m_buf[xptr++] = src[i];
				}
				m_dataCount += count;
				m_wptr = xptr;
			}
			success = true;
			return count;
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
			return ((f) && (m_dataCount < m_bufSize));
		}
		virtual bool full()
		{
			bool f = available();
			return (!(f) || (m_dataCount >= m_bufSize));
		}
		
		virtual int count()
		{
			return (m_dataCount > 0) ? m_dataCount : 0;
		}
		
		virtual int size()
		{
			return (m_bufSize > 0) ? m_bufSize : 0;
		}
		virtual bool resize(int _size)
		{
			if(_size <= 0) retrun false;
			try {
				T *tmpptr = new T[_size];
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

	template <class T >
		class LOCKED_FIFO : public UNLOCKED_FIFO {
	protected:
		std::recursive_mutex m_locker;
	public:
		LOCKED_FIFO(int _size) :
		UNLOCKED_FIFO(_size)
		{
		}
		~FIFO()
		{
		}

		virtual void initialize()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
		}
		virtual void release()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			UNLOCKED_FIFO::release();
		}
		virtual void clear()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			UNLOCKED_FIFO::clear();
		}
		virtual T read(bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::read(success);
		}
		virtual T read_not_remove(bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::read_not_remove(success);
		}
		virtual int read_to_buffer(T* dst, int count, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::read_to_buffer(dst, count, success);
		}
		virtual bool write(T data)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::write(data);
		}
		virtual int write_from_buffer(T* src, int count, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::write_from_buffer(src, count, success);
		}
		virtual bool available()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::available();
		}
		virtual bool empty()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::empty();
		}
		virtual bool read_ready()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::read_ready();
		}
		virtual bool write_ready()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::write_ready();
		}

		virtual bool full()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::full();
		}
		virtual int count()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::count();
		}
		virtual int size()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::size();
		}
		virtual bool resize(int _size)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_FIFO::resize(_size);
		}
	};
	
	template <class T >
		class UNLOCKED_RINGBUFFER : public UNLOCKED_FIFO {
	public:
		UNLOCKED_RINGBUFFER(int _size) :
		UNLOCKED_FIFO(_size)
		{
		}
		~UNLOCKED_RINGBUFFER()
		{
			release();
		}
		
		virtual T read(void)
		{
			bool dummy;
			return read(dummy);
		}
		// RINGBUFFER :  Even write to buffer when full.
		virtual bool write(T data)
		{
			if((m_buf == nullptr) || (m_bufSize <= 0)) {
				return false;
			}
			if(m_dataCount < 0) m_dataCount = 0; // OK?
			if(m_wptr >= m_bufSize) {
				m_wptr = (m_wptr - m_bufSize) % m_bufSize;
			} else if(m_wptr < 0) {
				m_wptr = (m_bufSize + m_wptr) % m_bufSize;
			}
			m_buf[m_wptr++] = data;
			m_dataCount++;
			if(m_dataCount > m_bufSize) m_dataCount = m_bufSize;
			return true;
		}
		virtual int write_from_buffer(T* src, int count, bool& success)
		{
			if((src == nullptr) || (count <= 0) || (m_buf == nullptr) || (m_bufSize <= 0)){
				success = false;
				return 0;
			}
			if(m_dataCount < 0) m_dataCount = 0; // OK?
			if(count > m_bufSize) {
				count = m_bufSize;
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
			int xptr = m_wptr;
			if((xptr + count) >= m_bufSize) {
				int count2 = (xptr + count) - m_bufSize;
				int count1 = count - count2;
				for(int i = 0; i < count1; i++) {
					m_buf[xptr++] = src[i];
				}
				xptr = 0;
				for(int i = 0; i < count2; i++) {
					m_buf[xptr++] = src[i];
				}
				m_wptr = xptr;
				m_dataCount += count;

			} else {
				// Inside buffer
				for(int i = 0; i < count; i++) {
					m_buf[xptr++] = src[i];
				}
				m_dataCount += count;
				m_wptr = xptr;
			}
			success = true;
			if(m_dataCount >= m_bufSize) m_dataCount = m_bufSize;
			return count;
		}
		virtual bool full()
		{
			return false; // OK?
		}
	};
	
	template <class T >
		class LOCKED_RINGBUFFER : public UNLOCKED_RINGBUFFER {
	protected:
		std::recursive_mutex m_locker;
	public:
		LOCKED_RINGBUFFER(int _size) :
		UNLOCKED_RINGBUFFER(_size)
		{
		}
		~LOCKED_RINGBUFFER()
		{
			release();
		}
		virtual void initialize()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			UNLOCKED_RINGBUFFER::initialize();
		}
		virtual void release()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			UNLOCKED_RINGBUFFER::release();
		}
		virtual void clear()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			UNLOCKED_RINGBUFFER::clear();
		}
		
		virtual T read(bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::read(success);
		}
		virtual T read_not_remove(bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::read_not_remove(success);
		}
		virtual T read(void)
		{
			bool dummy;
			return read(dummy);
		}
		virtual int read_to_buffer(T* dst, int count, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::read_to_buffer(dst, count, success);
		}
		
		virtual bool write(T data)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::write(data);
		}
		virtual int write_from_buffer(T* src, int count, bool& success)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::write_from_buffer(src, count, success);
		}
		virtual bool available()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::available();
		}
		virtual bool empty()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::empty();
		}
		virtual bool read_ready()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::read_ready();
		}
		virtual bool write_ready()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::write_ready();
		}
		virtual int count()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::count();
		}
		virtual int size()
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::size();
		}
		virtual bool resize(int _size)
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return UNLOCKED_RINGBUFFER::resize(_size);
		}
	};
}
