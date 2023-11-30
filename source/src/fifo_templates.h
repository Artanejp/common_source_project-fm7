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

	protected:
		std::atomic<ssize_t> m_bufSize;
		std::shared_ptr<T>   m_buf;
		std::atomic<ssize_t> m_low_warning;
		std::atomic<ssize_t> m_high_warning;
		std::atomic<size_t>  m_rptr;
		std::atomic<size_t>  m_wptr;

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
		virtual ssize_t realloc_buffer_size(size_t _count, bool force = false)
		{
			__UNLIKELY_IF(_count == 0) {
				if(force) {
					m_buf.reset();
					m_bufSize = 0;
				}
			} else {
				__UNLIKELY_IF(_count > INT_MAX) {
					_count = INT_MAX;
				}
				if((_count != m_bufSize.load()) || (force)) {
					T* p = nullptr;
					try {
						m_buf.reset(new T(_count));
						p = m_buf.get();
					} catch (std::bad_alloc& e) {
						_count = 0;
					}
					if((p == nullptr) && (force)) {
						m_buf.reset();
						m_bufSize = 0;
					} else if((p != nullptr) && (_count != 0)) {
						//m_buf.reset(p);
						m_bufSize = _count;
					} else {
						// DO NOTHING
					}
				}
			}
			return m_bufSize.load();
		}
		virtual void reset_buffer()
		{
			m_buf.reset();
			m_bufSize = 0;
		}
		constexpr bool check_data_available(size_t _count = 1)
		{
			__LIKELY_IF(check_buffer_available()) {
				__UNLIKELY_IF((_count > INT_MAX) || (_count == 0)) {
					return false;
				}
				return (m_dataCount.load() >= ((ssize_t)_count)) ? true : false;
			}
			return false;
		}
		constexpr bool check_buffer_available()
		{
			bool success = ((m_bufSize.load() <= 0) || (m_buf.get() == nullptr)) ? false : true;
			return success;
		}
		constexpr bool check_data_writable(size_t _count = 1)
		{
			__UNLIKELY_IF((_count == 0) || (_count > INT_MAX)) {
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

		inline void __FASTCALL check_offset(size_t& offset)
		{
			ssize_t bufsize = get_buffer_size();
			__UNLIKELY_IF(bufsize <= 0) {
				offset = 0;
			} else {
				__UNLIKELY_IF(offset >= bufsize) {
					offset = offset % bufsize;
				}
			}
		}
		inline bool __FASTCALL check_readable_data_count(T* dst, size_t _count)
		{
			__UNLIKELY_IF(dst == nullptr) {
				return false;
			}
			return check_data_available(_count);
		}
		virtual T __FASTCALL unlocked_read_base(bool& success)
		{
			success = check_data_available();
			return (T)0;
		}
		virtual T __FASTCALL unlocked_read_not_remove_base(size_t offset, bool& success)
		{
			success = check_buffer_available();
			check_offset(offset);
			return (T)0;
		}
		virtual size_t __FASTCALL unlocked_read_to_buffer_base(T* dst, size_t _count, bool& success)
		{
				success = check_readable_data_count(dst, _count);
				__UNLIKELY_IF(!(success)) {
					return 0;
				}
				return 0; // ToDo
		}
		virtual bool __FASTCALL unlocked_write_base(T data)
		{
			bool success = check_data_writable();
			return success;
		}
		virtual bool __FASTCALL unlocked_write_not_push_base(size_t offset, T data)
		{
			bool success = check_buffer_available();
			check_offset(offset);
			return success;
		}
		virtual size_t __FASTCALL unlocked_write_from_buffer_base(T* src, size_t _count, bool& success)
		{
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

		virtual inline T read_base(void)
		{
			bool dummy;
			return unlocked_read_base(dummy);
		}
		virtual inline T read_base(bool &success)
		{
			return unlocked_read_base(success);
		}
		virtual inline T read_not_remove_base(size_t offset, bool& success)
		{
			return unlocked_read_not_remove_base(offset, success);
		}
		virtual inline T read_not_remove_base(size_t offset)
		{
			bool dummy;
			return unlocked_read_not_remove_base(offset, dummy);
		}
		virtual inline size_t read_to_buffer_base(T* dst, size_t _count, bool& success)
		{
			return unlocked_read_to_buffer_base(dst, _count, success);
		}
		virtual inline bool write_base(T data)
		{
			return unlocked_write_base(data);
		}
		virtual inline bool write_not_push_base(size_t offset, T data)
		{
			return unlocked_write_not_push_base(offset, data);
		}
		virtual inline size_t write_from_buffer_base(T* src, size_t _count, bool& success)
		{
			return unlocked_write_from_buffer_base(src, _count, success);
		}
		virtual inline size_t write_from_buffer_base(T* src, size_t _count)
		{
			bool dummy;
			return unlocked_write_from_buffer_base(src, _count, dummy);
		}
		void clear_base()
		{
			m_rptr = 0;
			m_wptr = 0;
			m_dataCount = 0;
			__UNLIKELY_IF(!(check_buffer_available())) {
				return;
			}
			T* p = m_buf.get();
			__LIKELY_IF(p != nullptr) {
				for(int i = 0; i < m_bufSize.load(); i++) {
					p[i] = (T)0;
				}
			}
		}
		virtual bool resize_base(size_t _size, bool force = false, ssize_t _low_warn = INT_MIN + 1, ssize_t _high_warn = INT_MAX - 1)
		{
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

	public:
		FIFO_INTERFACE(size_t _size) :
			m_rptr(0), m_wptr(0), m_dataCount(0),
			m_high_warning(INT_MAX - 1), m_low_warning(INT_MIN + 1),
			m_is_ringbuffer(false)
		{
			if(_size == 0) {
				return;
			}
			bool is_legal = true;
			m_buf.reset(new T(_size));
			m_bufSize = _size;
			//ssize_t bsize = realloc_buffer_size(_size, true);
			//if(/*(bsize <= 0) || (bsize != _size) ||*/ (m_buf.get() == nullptr)) {
			//	is_legal = false;
				m_bufSize = 0;
			//}
			if(!(is_legal)) {
				m_high_warning = INT_MIN;
				m_low_warning = INT_MAX;
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
		virtual void clear()
		{
			clear_base();
		}
		virtual T __FASTCALL read(bool& success)
		{
			return read_base(success);
		}
		virtual T read(void)
		{
			return read_base();
		}
		virtual T __FASTCALL read_not_remove(size_t offset, bool& success)
		{
			return read_not_remove_base(offset, success);
		}
		virtual T __FASTCALL read_not_remove(size_t offset)
		{
			return read_not_remove_base(offset);
		}
		virtual size_t __FASTCALL read_to_buffer(T* dst, size_t _count, bool& success)
		{
			return read_to_buffer_base(dst, _count, success);
		}
		virtual size_t __FASTCALL read_to_buffer(T* dst, size_t _count)
		{
			bool dummy;
			return read_to_buffer_base(dst, _count, dummy);
		}

		virtual bool __FASTCALL write(T data)
		{
			return write_base(data);
		}
		virtual bool __FASTCALL write_not_push(size_t offset, T data)
		{
			return write_not_push_base(offset, data);
		}
		virtual size_t __FASTCALL write_from_buffer(T* src, size_t _count, bool& success)
		{
			return write_from_buffer_base(src, _count, success);

		}
		virtual size_t __FASTCALL write_from_buffer(T* src, size_t _count)
		{
			bool dummy;
			return write_from_buffer_base(src, _count, dummy);
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
		virtual bool __FASTCALL write_ready(size_t _count = 1)
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
			__UNLIKELY_IF(_size > INT_MAX) {
				return INT_MAX;
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
				__UNLIKELY_IF(_size == INT_MAX) {
					return INT_MAX;
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
		void __FASTCALL set_high_warn_value(ssize_t val = INT_MAX - 1)
		{
			m_high_warning = val;
		}
		void __FASTCALL set_low_warn_value(ssize_t val = INT_MIN + 1)
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
		virtual bool resize(size_t _size, bool force = false, ssize_t _low_warn = INT_MIN + 1, ssize_t _high_warn = INT_MAX - 1)
		{
			return resize_base(_size, force, _low_warn, _high_warn);
		}
	};
	template <class T >
	class LOCKED_INTERFACE : public FIFO_INTERFACE<T> {
		using FIFO_INTERFACE<T>::m_bufSize;
		using FIFO_INTERFACE<T>::m_buf;
		using FIFO_INTERFACE<T>::m_low_warning;
		using FIFO_INTERFACE<T>::m_high_warning;
		using FIFO_INTERFACE<T>::m_rptr;
		using FIFO_INTERFACE<T>::m_wptr;
		using FIFO_INTERFACE<T>::m_is_ringbuffer;
		using FIFO_INTERFACE<T>::m_dataCount;
		using FIFO_INTERFACE<T>::m_locker;

		using FIFO_INTERFACE<T>::is_ringbuffer;
		using FIFO_INTERFACE<T>::get_buffer_size;

		using FIFO_INTERFACE<T>::reset_buffer;

		using FIFO_INTERFACE<T>::check_buffer_available;
		using FIFO_INTERFACE<T>::check_data_available;
		using FIFO_INTERFACE<T>::check_data_writable;
		using FIFO_INTERFACE<T>::check_offset;
		using FIFO_INTERFACE<T>::check_readable_data_count;
		using FIFO_INTERFACE<T>::clear_base;
		using FIFO_INTERFACE<T>::resize_base;

	protected:
		ssize_t realloc_buffer_size(size_t _count, bool force = false) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return FIFO_INTERFACE<T>::realloc_buffer_size(_count, force);
		}
		virtual void reset_buffer() override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			FIFO_INTERFACE<T>::reset_buffer();
		}

		virtual inline T read_base(void) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return FIFO_INTERFACE<T>::read_base();
		}
		virtual inline T read_base(bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return FIFO_INTERFACE<T>::read_base(success);
		}
		virtual inline T read_not_remove_base(size_t offset) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return FIFO_INTERFACE<T>::read_not_remove_base(offset);
		}
		virtual inline T read_not_remove_base(size_t offset, bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return FIFO_INTERFACE<T>::read_not_remove_base(offset, success);
		}
		virtual inline size_t read_to_buffer_base(T* dst, size_t _count, bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return FIFO_INTERFACE<T>::read_to_buffer_base(dst, _count, success);
		}
		virtual inline bool write_base(T data) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return FIFO_INTERFACE<T>::write_base(data);
		}
		virtual inline bool write_not_push_base(size_t offset, T data)	override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return FIFO_INTERFACE<T>::write_not_push_base(offset, data);
		}
		virtual inline size_t write_from_buffer_base(T* src, size_t _count, bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return FIFO_INTERFACE<T>::write_from_buffer_base(src, _count, success);
		}
	public:
		LOCKED_INTERFACE<T>(size_t _size) : FIFO_INTERFACE<T>(_size)
		{
		}
		~LOCKED_INTERFACE<T>() {}

		virtual void clear() override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			clear_base();
		}
		virtual bool resize(size_t _size, bool force = false, ssize_t _low_warn = INT_MIN + 1, ssize_t _high_warn = INT_MAX - 1) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return resize_base(_size, force, _low_warn, _high_warn);
		}

	};

}


namespace FIFO_BASE {
	template <class T >
	class UNLOCKED_FIFO : public FIFO_INTERFACE<T> {
		using FIFO_INTERFACE<T>::m_bufSize;
		using FIFO_INTERFACE<T>::m_buf;
		using FIFO_INTERFACE<T>::m_low_warning;
		using FIFO_INTERFACE<T>::m_high_warning;
		using FIFO_INTERFACE<T>::m_rptr;
		using FIFO_INTERFACE<T>::m_wptr;
		using FIFO_INTERFACE<T>::m_is_ringbuffer;
		using FIFO_INTERFACE<T>::m_dataCount;
		using FIFO_INTERFACE<T>::m_locker;

		using FIFO_INTERFACE<T>::is_ringbuffer;
		using FIFO_INTERFACE<T>::get_buffer_size;

		using FIFO_INTERFACE<T>::reset_buffer;

		using FIFO_INTERFACE<T>::check_buffer_available;
		using FIFO_INTERFACE<T>::check_data_available;
		using FIFO_INTERFACE<T>::check_data_writable;
		using FIFO_INTERFACE<T>::check_offset;
		using FIFO_INTERFACE<T>::check_readable_data_count;
		using FIFO_INTERFACE<T>::clear_base;
		using FIFO_INTERFACE<T>::resize_base;

	protected:
		virtual T __FASTCALL unlocked_read_base(bool& success) override
		{
			T tmpval = (T)0;
			success = false;
			__LIKELY_IF(check_data_available()) {
				size_t bufsize = (size_t)get_buffer_size();
				T* p = m_buf.get();
				__UNLIKELY_IF(m_rptr.load() >= bufsize) {
					m_rptr = m_rptr.load() % bufsize;
				}
				tmpval = p[m_rptr++];
				m_dataCount--;
				success = true;
				__UNLIKELY_IF(m_rptr.load() >= bufsize) {
					m_rptr = 0;
				}
			}
			return tmpval;
		}
		virtual T __FASTCALL unlocked_read_not_remove_base(size_t offset, bool& success) override
		{
			check_offset(offset);
			T tmpval = (T)0;
			success = check_buffer_available();
			__LIKELY_IF(success) {
				size_t rptr = m_rptr.load();
				rptr = rptr + offset;
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
		virtual size_t __FASTCALL unlocked_read_to_buffer_base(T* dst, size_t _count, bool& success) override
		{
			success = false;
			__UNLIKELY_IF(dst == nullptr) {
				return 0;
			}
			success = check_buffer_available();
			__LIKELY_IF(success) {
				T* p = m_buf.get();
				size_t words = 0;

				ssize_t bufsize = get_buffer_size();
				__UNLIKELY_IF(m_rptr.load() >= bufsize) {
					m_rptr = m_rptr.load() % bufsize;
				}
				for(; words < _count; words++) {
					__UNLIKELY_IF(m_dataCount.load() <= 0) {
						break;
					}
					__UNLIKELY_IF(m_rptr.load() >= bufsize) {
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
		virtual bool __FASTCALL unlocked_write_base(T data) override
		{
			__UNLIKELY_IF(!(check_data_writable())) {
				return false;
			}
			size_t bufsize = get_buffer_size();
			__UNLIKELY_IF(m_wptr >= m_bufSize) {
				m_wptr = m_wptr % bufsize;
			}
			__UNLIKELY_IF(m_dataCount < 0) {
				m_dataCount = 0; // OK?
			}
			T* p = m_buf.get();
			__UNLIKELY_IF(p == nullptr) {
				return false;
			}
			p[m_wptr++] = data;
			m_dataCount++;
			__UNLIKELY_IF(m_wptr.load() >= bufsize) {
				m_wptr = 0;
			}
			return true;
		}
		virtual bool __FASTCALL unlocked_write_not_push_base(size_t offset, T data) override
		{
			__UNLIKELY_IF(!(check_data_available())) {
				return false;
			}
			T* p = m_buf.get();
			__UNLIKELY_IF(p == nullptr) {
				return false;
			}
			check_offset(offset);
			size_t wp = m_wptr.load() + offset;
			check_offset(wp);

			p[wp] = data;
			return true;
		}
		virtual size_t __FASTCALL unlocked_write_from_buffer_base(T* src, size_t _count, bool& success) override
		{
			success = false;
			__UNLIKELY_IF(src == nullptr) {
				return 0;
			}
			success = check_buffer_available();
			__LIKELY_IF(success) {
				T* p = m_buf.get();
				size_t words = 0;

				ssize_t bufsize = get_buffer_size();
				__UNLIKELY_IF(m_wptr.load() >= bufsize) {
					m_wptr = m_wptr.load() % bufsize;
				}
				for(; words < _count; words++) {
					__UNLIKELY_IF(m_dataCount.load() >= bufsize) {
						break;
					}
					__UNLIKELY_IF(m_wptr.load() >= bufsize) {
						m_wptr = 0;
					}
					p[m_wptr++] = src[words];
					m_dataCount++;
				}
				__UNLIKELY_IF(words <= 0) {
					success = false;
					return 0;
				}
				return words;
			}
			return 0;
		}

	public:
		UNLOCKED_FIFO<T>(size_t _size) : FIFO_INTERFACE<T>(_size)
		{
		}
		~UNLOCKED_FIFO<T>()
		{
		}
		//!< Read one data

	};

	template <class T >
	class LOCKED_FIFO : public UNLOCKED_FIFO<T> {
		using FIFO_INTERFACE<T>::m_bufSize;
		using FIFO_INTERFACE<T>::m_buf;
		using FIFO_INTERFACE<T>::m_low_warning;
		using FIFO_INTERFACE<T>::m_high_warning;
		using FIFO_INTERFACE<T>::m_rptr;
		using FIFO_INTERFACE<T>::m_wptr;
		using FIFO_INTERFACE<T>::m_is_ringbuffer;
		using FIFO_INTERFACE<T>::m_dataCount;
		using FIFO_INTERFACE<T>::m_locker;

		using FIFO_INTERFACE<T>::is_ringbuffer;
		using FIFO_INTERFACE<T>::get_buffer_size;

		using FIFO_INTERFACE<T>::reset_buffer;

		using FIFO_INTERFACE<T>::check_buffer_available;
		using FIFO_INTERFACE<T>::check_data_available;
		using FIFO_INTERFACE<T>::check_data_writable;
		using FIFO_INTERFACE<T>::check_offset;
		using FIFO_INTERFACE<T>::check_readable_data_count;
		using FIFO_INTERFACE<T>::clear_base;
		using FIFO_INTERFACE<T>::resize_base;

		using UNLOCKED_FIFO<T>::unlocked_read_base;
		using UNLOCKED_FIFO<T>::unlocked_read_not_remove_base;
		using UNLOCKED_FIFO<T>::unlocked_read_to_buffer_base;
		using UNLOCKED_FIFO<T>::unlocked_write_base;
		using UNLOCKED_FIFO<T>::unlocked_write_not_push_base;
		using UNLOCKED_FIFO<T>::unlocked_write_from_buffer_base;

		using UNLOCKED_FIFO<T>::read_base;
		using UNLOCKED_FIFO<T>::read_not_remove_base;
		using UNLOCKED_FIFO<T>::read_to_buffer_base;
		using UNLOCKED_FIFO<T>::write_base;
		using UNLOCKED_FIFO<T>::write_not_push_base;
		using UNLOCKED_FIFO<T>::write_from_buffer_base;

	public:
		LOCKED_FIFO<T>(size_t _size) :
		UNLOCKED_FIFO<T>(_size)
		{
		}
		~LOCKED_FIFO<T>()
		{
		}
		virtual T __FASTCALL read(bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return read_base(success);
		}
		virtual T read(void) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			bool dummy;
			return read_base(dummy);
		}

		virtual T __FASTCALL read_not_remove(size_t offset, bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return read_not_remove_base(offset, success);
		}
		virtual T __FASTCALL read_not_remove(size_t offset) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			bool dummy;
			return read_not_remove_base(offset, dummy);
		}
		virtual size_t __FASTCALL read_to_buffer(T* dst, size_t _count, bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return read_to_buffer_base(dst, _count, success);
		}
		virtual size_t __FASTCALL read_to_buffer(T* dst, size_t _count) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			bool dummy;
			return read_to_buffer_base(dst, _count, dummy);
		}
		virtual bool __FASTCALL write(T data) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return write_base(data);
		}
		virtual bool __FASTCALL write_not_push(size_t offset, T data) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return write_not_push_base(offset, data);
		}
		virtual size_t __FASTCALL write_from_buffer(T* src, size_t _count, bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return write_from_buffer_base(src, _count, success);
		}
		virtual size_t __FASTCALL write_from_buffer(T* src, size_t _count) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return write_from_buffer_base(src, _count);
		}
		virtual void clear() override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			FIFO_INTERFACE<T>::clear();
		}
		virtual bool resize(size_t _size, bool force = false, ssize_t _low_warn = INT_MIN + 1, ssize_t _high_warn = INT_MAX - 1) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return FIFO_INTERFACE<T>::resize(_size, force, _low_warn, _high_warn);
		}
	};

	template <class T >
	class UNLOCKED_RINGBUFFER : public FIFO_INTERFACE<T> {
	protected:
		using FIFO_INTERFACE<T>::m_bufSize;
		using FIFO_INTERFACE<T>::m_buf;
		using FIFO_INTERFACE<T>::m_low_warning;
		using FIFO_INTERFACE<T>::m_high_warning;
		using FIFO_INTERFACE<T>::m_rptr;
		using FIFO_INTERFACE<T>::m_wptr;
		using FIFO_INTERFACE<T>::m_is_ringbuffer;
		using FIFO_INTERFACE<T>::m_dataCount;
		using FIFO_INTERFACE<T>::m_locker;

		using FIFO_INTERFACE<T>::is_ringbuffer;
		using FIFO_INTERFACE<T>::get_buffer_size;

		using FIFO_INTERFACE<T>::reset_buffer;

		using FIFO_INTERFACE<T>::check_buffer_available;
		using FIFO_INTERFACE<T>::check_data_available;
		using FIFO_INTERFACE<T>::check_data_writable;
		using FIFO_INTERFACE<T>::check_offset;
		using FIFO_INTERFACE<T>::check_readable_data_count;
		using FIFO_INTERFACE<T>::clear_base;
		using FIFO_INTERFACE<T>::resize_base;

		virtual T __FASTCALL unlocked_read_base(bool& success) override
		{
			success = false;
			__UNLIKELY_IF(!(check_data_available())) {
				return 0;
			}
			size_t bufsize = get_buffer_size();
			__UNLIKELY_IF(m_rptr.load() >= bufsize) {
				m_rptr = m_rptr.load() % bufsize;
			}
			T* p = m_buf.get();
			__UNLIKELY_IF(p == nullptr) {
				return 0;
			}
			T data = p[m_rptr++];
			__UNLIKELY_IF(m_rptr.load() >= bufsize) {
				m_rptr = 0;
			}
			m_dataCount--;
			__UNLIKELY_IF(m_dataCount.load() <= 0) {
				m_dataCount = bufsize;
			}
			success = true;
			return data;
		}
		virtual T __FASTCALL unlocked_read_not_remove_base(size_t offset, bool& success) override
		{
			success = false;
			__UNLIKELY_IF(!(check_buffer_available())) {
				return 0;
			}
			size_t bufsize = get_buffer_size();
			__UNLIKELY_IF(offset >= bufsize) {
				offset = offset % bufsize;
			}
			T* p = m_buf.get();
			__UNLIKELY_IF(p == nullptr) {
				return 0;
			}
			T data = p[offset];
			success = true;
			return data;
		}
		virtual size_t __FASTCALL unlocked_read_to_buffer_base(T* dst, size_t _count, bool& success) override
		{
			success = false;
			__UNLIKELY_IF((dst == nullptr) || (_count == 0)) {
				return 0;
			}
			__UNLIKELY_IF(!(check_data_available(1))) {
				return 0;
			}
			ssize_t bufsize = get_buffer_size();
			__UNLIKELY_IF(m_dataCount.load() > bufsize) {
				m_dataCount = bufsize;
			}
			if(_count > m_dataCount.load()) {
				_count = m_dataCount.load();
			}
			T* p = m_buf.get();
			size_t words = 0;
			__UNLIKELY_IF(m_rptr.load() >= bufsize) {
				m_rptr = m_rptr.load() % bufsize;
			}
			for(; words < _count; words++) {
				__UNLIKELY_IF(m_rptr.load() >= bufsize) {
					m_rptr = 0;
				}
				__UNLIKELY_IF(m_dataCount.load() <= 0) {
					break;
				}
				dst[words] = p[m_rptr++];
				m_dataCount--;
			}
			__UNLIKELY_IF(m_dataCount.load() <= 0) {
				m_dataCount = 0;
			}
			__UNLIKELY_IF(words <= 0) {
				return 0;
			}
			success = true;
			return words;
		}
		virtual bool __FASTCALL unlocked_write_base(T data) override
		{
			__UNLIKELY_IF(!(check_buffer_available())) {
				return false;
			}
			size_t bufsize = get_buffer_size();
			__UNLIKELY_IF(m_wptr.load() >= bufsize) {
				m_wptr = m_wptr.load() % bufsize;
			}
			__UNLIKELY_IF(m_dataCount.load() < 0) {
				m_dataCount = 0;
			}
			T* p = m_buf.get();
			__UNLIKELY_IF(p == nullptr) {
				return false;
			}
			p[m_wptr++] = data;
			__UNLIKELY_IF(m_wptr.load() >= bufsize) {
				m_wptr = 0;
			}
			m_dataCount++;
			__UNLIKELY_IF(m_dataCount.load() >= bufsize) {
				m_dataCount = bufsize;
			}
			return true;
		}
		virtual bool __FASTCALL unlocked_write_not_push_base(size_t offset, T data) override
		{
			__UNLIKELY_IF(!(check_buffer_available())) {
				return false;
			}
			size_t bufsize = get_buffer_size();
			__UNLIKELY_IF(offset >= bufsize) {
				offset = offset % bufsize;
			}
			T* p = m_buf.get();
			__UNLIKELY_IF(p == nullptr) {
				return false;
			}
			p[offset] = data;
			return true;
		}
		virtual size_t __FASTCALL unlocked_write_from_buffer_base(T* src, size_t _count, bool& success) override
		{
			success = false;
			__UNLIKELY_IF(!(check_buffer_available())) {
				return 0;
			}
			__UNLIKELY_IF(m_dataCount < 0) {
				m_dataCount = 0; // OK?
			}
			ssize_t bufsize = get_buffer_size();
			if(_count > bufsize) {
				_count = bufsize;
			}
			T* p = m_buf.get();
			size_t words = 0;
			__UNLIKELY_IF(m_wptr.load() >= bufsize) {
				m_wptr = m_wptr.load() % bufsize;
			}
			for(; words < _count; words++) {
				__UNLIKELY_IF(m_wptr.load() >= bufsize) {
					m_wptr = 0;
				}
				p[m_wptr++] = src[words];
				m_dataCount++;
			}
			__UNLIKELY_IF(m_dataCount.load() > bufsize) {
				m_dataCount = bufsize;
			}
			__UNLIKELY_IF(words <= 0) {
				return 0;
			}
			success = true;
			return words;
		}

	public:
		UNLOCKED_RINGBUFFER<T>(size_t _size) :
			FIFO_INTERFACE<T>(_size)
		{
			m_is_ringbuffer = true;
		}
		~UNLOCKED_RINGBUFFER<T>()
		{
		}
		// RINGBUFFER :  Even write to buffer when full.
	};

	template <class T >
	class LOCKED_RINGBUFFER : public UNLOCKED_RINGBUFFER<T> {
		using FIFO_INTERFACE<T>::m_bufSize;
		using FIFO_INTERFACE<T>::m_buf;
		using FIFO_INTERFACE<T>::m_low_warning;
		using FIFO_INTERFACE<T>::m_high_warning;
		using FIFO_INTERFACE<T>::m_rptr;
		using FIFO_INTERFACE<T>::m_wptr;
		using FIFO_INTERFACE<T>::m_is_ringbuffer;
		using FIFO_INTERFACE<T>::m_dataCount;
		using FIFO_INTERFACE<T>::m_locker;

		using FIFO_INTERFACE<T>::is_ringbuffer;
		using FIFO_INTERFACE<T>::get_buffer_size;

		using FIFO_INTERFACE<T>::reset_buffer;

		using FIFO_INTERFACE<T>::check_buffer_available;
		using FIFO_INTERFACE<T>::check_data_available;
		using FIFO_INTERFACE<T>::check_data_writable;
		using FIFO_INTERFACE<T>::check_offset;
		using FIFO_INTERFACE<T>::check_readable_data_count;
		using FIFO_INTERFACE<T>::clear_base;
		using FIFO_INTERFACE<T>::resize_base;

		using UNLOCKED_RINGBUFFER<T>::unlocked_read_base;
		using UNLOCKED_RINGBUFFER<T>::unlocked_read_not_remove_base;
		using UNLOCKED_RINGBUFFER<T>::unlocked_read_to_buffer_base;
		using UNLOCKED_RINGBUFFER<T>::unlocked_write_base;
		using UNLOCKED_RINGBUFFER<T>::unlocked_write_not_push_base;
		using UNLOCKED_RINGBUFFER<T>::unlocked_write_from_buffer_base;

		using UNLOCKED_RINGBUFFER<T>::read_base;
		using UNLOCKED_RINGBUFFER<T>::read_not_remove_base;
		using UNLOCKED_RINGBUFFER<T>::read_to_buffer_base;
		using UNLOCKED_RINGBUFFER<T>::write_base;
		using UNLOCKED_RINGBUFFER<T>::write_not_push_base;
		using UNLOCKED_RINGBUFFER<T>::write_from_buffer_base;
	public:
		LOCKED_RINGBUFFER<T>(size_t _size) :
		UNLOCKED_RINGBUFFER<T>(_size)
		{
		}
		~LOCKED_RINGBUFFER<T>()
		{
		}
		virtual T __FASTCALL read(bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return read_base(success);
		}
		virtual T read(void) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			bool dummy;
			return read_base(dummy);
		}

		virtual T __FASTCALL read_not_remove(size_t offset, bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return read_not_remove_base(offset, success);
		}
		virtual T __FASTCALL read_not_remove(size_t offset) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			bool dummy;
			return read_not_remove_base(offset, dummy);
		}
		virtual size_t __FASTCALL read_to_buffer(T* dst, size_t _count, bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return read_to_buffer_base(dst, _count, success);
		}
		virtual size_t __FASTCALL read_to_buffer(T* dst, size_t _count) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			bool dummy;
			return read_to_buffer_base(dst, _count, dummy);
		}
		virtual bool __FASTCALL write(T data) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return write_base(data);
		}
		virtual bool __FASTCALL write_not_push(size_t offset, T data) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return write_not_push_base(offset, data);
		}
		virtual size_t __FASTCALL write_from_buffer(T* src, size_t _count, bool& success) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return write_from_buffer_base(src, _count, success);
		}
		virtual size_t __FASTCALL write_from_buffer(T* src, size_t _count) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return write_from_buffer_base(src, _count);
		}
		virtual void clear() override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			clear_base();
		}
		virtual bool resize(size_t _size, bool force = false, ssize_t _low_warn = INT_MIN + 1, ssize_t _high_warn = INT_MAX - 1) override
		{
			std::lock_guard<std::recursive_mutex> locker(m_locker);
			return resize_base(_size, force, _low_warn, _high_warn);
		}
	};
}
