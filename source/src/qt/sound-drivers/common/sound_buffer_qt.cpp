#include "../sound_buffer_qt.h"

SOUND_BUFFER_QT::SOUND_BUFFER_QT(uint64_t depth, QObject *parent)
	: QIODevice(parent),
	  wroteFromBefore(0),
	  is_emitted(false)
{

	if((depth > 0) && (depth < INT_MAX)) {
		//m_buffer.reset(new FIFO_BASE::LOCKED_FIFO<uint8_t>((int)depth));
		m_buffer.reset(new BUFFER_TYPE((int)depth));
	}
	connect(this, SIGNAL(_signal_for_write()), this, SLOT(_emit_for_write()), Qt::QueuedConnection);
}

SOUND_BUFFER_QT::~SOUND_BUFFER_QT()
{
	m_buffer.reset();
}

bool SOUND_BUFFER_QT::open(OpenMode flags)
{
//	printf("open() flags=%08x\n", flags);
    if ((flags & (Append | Truncate)) != 0)
		flags |= WriteOnly;

    if ((flags & (ReadOnly | WriteOnly)) == 0) {
        qWarning("SOUND_BUFFER_QT::open: Buffer access not specified");
        return false;
    }

    if ((flags & Truncate) == Truncate) {
		std::shared_ptr<BUFFER_TYPE> p = m_buffer;
		if(p) {
			p->clear();
		}
	}
	is_emitted = false;
    return QIODevice::open(flags);
}

void SOUND_BUFFER_QT::close()
{
//	//printf("close()\n");
	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if(p) {
		p->clear();
	}
	QIODevice::close();
}

bool SOUND_BUFFER_QT::isSequential() const
{
	return true;
}

qint64 SOUND_BUFFER_QT::size() const
{
	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if(p) {
		return (qint64)(p->count());
	}
	return 0;
}

qint64 SOUND_BUFFER_QT::bytesToWrite() const
{
	qint64 _n = (qint64)0;

	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if(p) {
		_n = (qint64)(p->fifo_size());
	}
	return (qint64)_n;
}

qint64 SOUND_BUFFER_QT::bytesAvailable() const
{
	#if 0
	qint64 _size = QIODevice::bytesAvailable();
	#else
	qint64 _size = 0;
	#endif
	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if(p) {
		_size += (qint64)(p->count());
		//_size += p->fifo_size();
	}
	//printf("bytesAvailable() is %lld\n", _size);
	return _size;
}

qint64 SOUND_BUFFER_QT::pos() const
{
	qint64 _pos = (qint64)0;
#if 0
	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if(p) {
		_pos = (p->count()) % (p->fifo_size());
	}
#endif
    return (qint64)_pos;
    //return QIODevice::pos();
}

bool SOUND_BUFFER_QT::seek(qint64 pos)
{
#if 0
	if(pos < 0) {
		return false;
	}
	if(pos == 0) {
		return true;
	}
	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if(p) {
		if(pos < p->count()) {
			uint8_t* buf = new uint8_t[pos];
			bool _success;
			p->read_to_buffer(buf, pos, _success);
			delete[] buf;
			return true;
		} else if(pos == p->count()) {
			p->clear();
			return true;
		}
	}
#endif
	return false;
}

bool SOUND_BUFFER_QT::atEnd() const
{
//	printf("atEnd()\n");
    const bool result = isOpen();
	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if(p) {
		return (!(result) || (p->empty()));
	}
	return result;
}


bool SOUND_BUFFER_QT::reset()
{
	//printf("reset()\n");
	wroteFromBefore = 0;
	is_emitted = false;
	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if(p) {
		p->clear();
		return true;
	}
	return false;
}

qint64 SOUND_BUFFER_QT::read_from_buffer(char *data, qint64 len)
{
	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if((p) && (data != nullptr)) {
	    if ((len = qMin(len, qint64(p->count()))) <= 0) {
			return qint64(0);
		}
		bool _success;
		len = p->read_to_buffer((uint8_t *)data, (int)len, _success);
		if((len > (qint64)0) /*&& (_success)*/) {
			//printf("readData() ok len=%lld\n", len);
			return len;
		}
	}
	return qint64(-1);
}

qint64 SOUND_BUFFER_QT::readData(char *data, qint64 len)
{
	//printf("readData() called len=%lld\n", len);
	if(!(isReadable()) || !(isOpen())) return qint64(-1);
	return read_from_buffer(data, len);
}

qint64 SOUND_BUFFER_QT::write_to_buffer(const char *data, qint64 len)
{
	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if((p) && (data != nullptr)) {
	    if ((len = qMin(len, qint64(p->left()))) <= 0) {
			return qint64(0);
		}
		bool _success;
		len = p->write_from_buffer((uint8_t *)data, (int)len, _success);
		//printf("writeData() ok len=%lld\n", len);
		if((len > (qint64)0) /*&& (_success)*/) {
			wroteFromBefore += len;
			if(isOpen() && (receivers(SIGNAL(readyRead())) > 0)) {
				if(!(is_emitted.exchange(true))) {
					emit _signal_for_write();
				}
			}
			return len;
		}
	}
	return qint64(-1);
}

qint64 SOUND_BUFFER_QT::writeData(const char *data, qint64 len)
{
	if(!(isWritable()) || !(isOpen())) return qint64(-1);
	//printf("writeData() called len=%lld\n", len);
	return write_to_buffer(data, len);
}

// For asynchronous emitting.
void SOUND_BUFFER_QT::_emit_for_write()
{
	qint64 len = wroteFromBefore.exchange(0);
	is_emitted = false;
	emit bytesWritten(len);
	emit readyRead();
}

// unique functions

bool SOUND_BUFFER_QT::resize(qint64 sz)
{
//	printf("resize()\n");
	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if((sz <= 0) || (sz >= INT_MAX) || !(p)) {
		return false;
	}
	is_emitted = false;
	wroteFromBefore = 0;
	return p->resize((int)sz);
}

qint64 SOUND_BUFFER_QT::real_buffer_size()
{
	std::shared_ptr<BUFFER_TYPE> p = m_buffer;
	if(p) {
		int64_t sz = (int64_t)(p->fifo_size());
		return (sz > 0) ? sz : 0;
	}
	return 0;
}
