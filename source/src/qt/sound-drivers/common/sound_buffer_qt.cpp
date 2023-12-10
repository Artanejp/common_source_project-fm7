
#include "../sound_buffer_qt.h"

SOUND_BUFFER_QT::SOUND_BUFFER_QT(uint64_t depth, QObject *parent) : QIODevice(parent)
{
//	printf("SOUND_BUFFER_QT(%d)\n", depth);
	if((depth > 0) && (depth < INT_MAX)) {
		//m_buffer.reset(new FIFO_BASE::LOCKED_FIFO<uint8_t>((int)depth));
		m_buffer.reset(new FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>((int)depth));
	}
}

SOUND_BUFFER_QT::~SOUND_BUFFER_QT()
{
		
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool SOUND_BUFFER_QT::open(QIODeviceBase::OpenMode flags)
{
//	printf("open() flags=%08x\n", flags);
    if ((flags & (QIODeviceBase::Append | QIODeviceBase::Truncate)) != 0)
        flags |= QIODeviceBase::WriteOnly;
    if ((flags & (QIODeviceBase::ReadOnly | QIODeviceBase::WriteOnly)) == 0) {
        qWarning("SOUND_BUFFER_QT::open: Buffer access not specified");
        return false;
    }

    if ((flags & QIODeviceBase::Truncate) == QIODeviceBase::Truncate) {
		std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;
		p->clear();
	}
    //return QIODevice::open(flags | QIODevice::Unbuffered);
    return QIODevice::open(flags);
}
#else
bool SOUND_BUFFER_QT::open(QIODevice::OpenMode flags)
{
//	printf("open() flags=%08x\n", flags);
    if ((flags & (QIODevice::Append | QIODevice::Truncate)) != 0)
        flags |= QIODevice::WriteOnly;
    if ((flags & (QIODevice::ReadOnly | QIODevice::WriteOnly)) == 0) {
        qWarning("SOUND_BUFFER_QT::open: Buffer access not specified");
        return false;
    }

    if ((flags & QIODevice::Truncate) == QIODevice::Truncate) {
		std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;
		p->clear();
	}
    //return QIODevice::open(flags | QIODevice::Unbuffered);
    return QIODevice::open(flags);
}
#endif

void SOUND_BUFFER_QT::close()
{
//	//printf("close()\n");
	std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;
	if(p) {
		p->clear();
	}
	QIODevice::close();
}

bool SOUND_BUFFER_QT::resize(qint64 sz)
{
//	printf("resize()\n");
	std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;
	if((sz <= 0) || (sz >= INT_MAX) || !(p)) {
		return false;
	}
	return p->resize((int)sz);
}

bool SOUND_BUFFER_QT::isSequential() const
{
	return true;
}

qint64 SOUND_BUFFER_QT::size() const
{
	return bytesAvailable();
}

qint64 SOUND_BUFFER_QT::bytesToWrite() const
{
	qint64 _n = (qint64)0;

	std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	if((p) && ((openMode() & QIODeviceBase::Unbuffered) == 0)) {
#else
	if((p) && ((openMode() & QIODevice::Unbuffered) == 0)) {
#endif
		//_n = (qint64)(p->fifo_size() - p->count());		
		_n = (qint64)(p->count());		
	}

	//printf("bytesToWrite() is %lld\n", _n);
	return (qint64)_n;
}

qint64 SOUND_BUFFER_QT::bytesAvailable() const
{
	qint64 _size = QIODevice::bytesAvailable();
	std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;
	if(p) {
		_size += p->count();
		//_size += (qint64)(p->fifo_size() - p->count());		
	}
	//printf("bytesAvailable() is %lld\n", _size);
	return _size;
}

qint64 SOUND_BUFFER_QT::pos() const
{
	qint64 _pos = (qint64)0;
#if 0	
	std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;
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
	std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;
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
	std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;
	if(p) {
		return (!(result) || (p->empty()));
	}
	return result;
}


bool SOUND_BUFFER_QT::reset()
{
	//printf("reset()\n");
	std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;
	if(p) {
		p->clear();
		return true;
	}
	return false;
}

qint64 SOUND_BUFFER_QT::readData(char *data, qint64 len)
{
	//printf("readData() called len=%lld\n", len);
	if(!(isReadable()) || !(isOpen())) return qint64(-1);
	std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;

	if(p) {
	    if ((len = qMin(len, qint64(p->count()))) <= 0) {
			return qint64(0);
		}
		bool _success;
		len = p->read_to_buffer((uint8_t *)data, (int)len, _success);
		if((len > (qint64)0) && (_success)) {
			//printf("readData() ok len=%lld\n", len);
			return len;
		}
	}
	return qint64(-1);
}

qint64 SOUND_BUFFER_QT::writeData(const char *data, qint64 len)
{
	if(!(isWritable()) || !(isOpen())) return qint64(-1);
	//printf("writeData() called len=%lld\n", len);
	std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>> p = m_buffer;
	if(p) {
	    if ((len = qMin(len, qint64(p->left()))) <= 0) {
			return qint64(0);
		}
		bool _success;
		len = p->write_from_buffer((uint8_t *)data, (int)len, _success);
		//printf("writeData() ok len=%lld\n", len);
		if((len > (qint64)0) && (_success)) {
			emit bytesWritten(len);
			emit readyRead();
			return len;
		}
	}
	return qint64(-1);
}


