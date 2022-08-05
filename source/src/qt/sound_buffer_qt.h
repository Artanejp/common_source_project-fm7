#pragma once

#include <memory>
#include <QIODevice>
#include "../fifo_templates.h"

QT_BEGIN_NAMESPACE

class DLL_PREFIX SOUND_BUFFER_QT : public QIODevice
{
    Q_OBJECT
	
protected:	
	//std::shared_ptr<FIFO_BASE::LOCKED_FIFO<uint8_t>>m_buffer;
	std::shared_ptr<FIFO_BASE::UNLOCKED_FIFO<uint8_t>>m_buffer;
public:
	SOUND_BUFFER_QT(uint64_t depth = 0, QObject *parent = nullptr);
	~SOUND_BUFFER_QT();
	
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	virtual bool open(QIODeviceBase::OpenMode flags) override;
#else
	virtual bool open(QIODevice::OpenMode flags) override;
#endif
	virtual void close() override;
	virtual bool resize(qint64 sz);
	virtual bool isSequential() const override;
	virtual qint64 size() const override;

	virtual qint64 bytesToWrite() const override;
	virtual qint64 bytesAvailable() const override;

	virtual qint64 pos() const override;
	virtual bool seek(qint64 pos) override;

	virtual bool   atEnd() const override;
	virtual bool   reset() override;

protected:
	virtual qint64 readData(char *data, qint64 len) override;
	virtual qint64 writeData(const char *data, qint64 len) override;
};

QT_END_NAMESPACE
