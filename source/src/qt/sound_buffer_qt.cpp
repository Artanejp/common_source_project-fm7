
#include "./sound_buffer_qt.h"

SOUND_BUFFER_QT::SOUND_BUFFER_QT(uint64_t depth, QObject *parent) : QIODevice(parent)
{
//	printf("SOUND_BUFFER_QT(%d)\n", depth);
	if((depth > 0) && (depth < INT_MAX)) {
		m_buffer.reset(new FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>((int)depth));
	}
}

SOUND_BUFFER_QT::~SOUND_BUFFER_QT()
{
		
}

bool SOUND_BUFFER_QT::open(QIODeviceBase::OpenMode flags)
{
//	printf("open() flags=%08x\n", flags);
    if ((flags & (QIODeviceBase::Append | QIODeviceBase::Truncate)) != 0)
        flags |= QIODeviceBase::WriteOnly;
    if ((flags & (QIODeviceBase::ReadOnly | QIODeviceBase::WriteOnly)) == 0) {
        qWarning("QBuffer::open: Buffer access not specified");
        return false;
    }

    if ((flags & QIODeviceBase::Truncate) == QIODeviceBase::Truncate) {
		std::shared_ptr<FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>> p = m_buffer;
		p->clear();
	}
    return QIODevice::open(flags | QIODevice::Unbuffered);
    //return QIODevice::open(flags);
}

void SOUND_BUFFER_QT::close()
{
//	//printf("close()\n");
	std::shared_ptr<FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>> p = m_buffer;
	if(p) {
		p->clear();
	}
	QIODevice::close();
}

bool SOUND_BUFFER_QT::resize(qint64 sz)
{
//	printf("resize()\n");
	std::shared_ptr<FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>> p = m_buffer;
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
	std::shared_ptr<FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>> p = m_buffer;
	if(p) {
//		printf("size() = %d\n", p->fifo_size());
		return (qint64)(p->fifo_size());
	}
//	printf("size() = 0\n");
    return (qint64)0;
    //return QIODevice::pos();
}

qint64 SOUND_BUFFER_QT::bytesToWrite() const
{
	std::shared_ptr<FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>> p = m_buffer;
	qint64 _n = (qint64)0;
	if(p) {
		_n = (qint64)(p->fifo_size() - p->count());		
	}
	//printf("bytesToWrite() = %lld\n", _n);
	return (qint64)0;
}

qint64 SOUND_BUFFER_QT::bytesAvailable() const
{
	qint64 _size = QIODevice::bytesAvailable();
	std::shared_ptr<FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>> p = m_buffer;
	if(p) {
		_size += p->count();
	}
//	printf("bytesAvailable() is %lld\n", _size);
	return _size;
}

qint64 SOUND_BUFFER_QT::pos() const
{
	qint64 _pos = (qint64)0;
	std::shared_ptr<FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>> p = m_buffer;
	if(p) {
		_pos = (p->count()) % (p->fifo_size());
	}
    return (qint64)_pos;
    //return QIODevice::pos();
}


bool SOUND_BUFFER_QT::atEnd() const
{
//	printf("atEnd()\n");
    const bool result = isOpen();
	std::shared_ptr<FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>> p = m_buffer;
	if(p) {
		return (!(result) || (p->empty()));
	}
	return result;
}

bool SOUND_BUFFER_QT::reset()
{
	//printf("reset()\n");
	std::shared_ptr<FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>> p = m_buffer;
	if(p) {
		p->clear();
		return true;
	}
	return false;
}

qint64 SOUND_BUFFER_QT::readData(char *data, qint64 len)
{
	//printf("readData() called len=%lld\n", len);
	if(!(isReadable()) || !(isOpen())) return qint64(0);
	std::shared_ptr<FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>> p = m_buffer;

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
	return qint64(0);
}

qint64 SOUND_BUFFER_QT::writeData(const char *data, qint64 len)
{
	if(!(isWritable()) || !(isOpen())) return qint64(0);
	//printf("writeData() called len=%lld\n", len);
	std::shared_ptr<FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>> p = m_buffer;
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
	return qint64(0);
}


