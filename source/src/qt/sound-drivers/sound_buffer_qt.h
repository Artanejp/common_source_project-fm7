#pragma once

#include <memory>
#include <QIODevice>
#include "../../fifo_templates.h"

QT_BEGIN_NAMESPACE


class DLL_PREFIX SOUND_BUFFER_QT : public QIODevice
{

    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	using QIODeviceBase::OpenMode;
	using QIODeviceBase::OpenModeFlag;
#else
	using QIODevice::OpenMode;
	using QIODevice::OpenModeFlag;
#endif
	using BUFFER_TYPE = FIFO_BASE::LOCKED_RINGBUFFER<uint8_t>;
	//using BUFFER_TYPE = FIFO_BASE::LOCKED_FIFO<uint8_t>;
protected:
	std::shared_ptr<BUFFER_TYPE>m_buffer;
public:
	SOUND_BUFFER_QT(uint64_t depth = 0, QObject *parent = nullptr);
	~SOUND_BUFFER_QT();

	virtual bool isSequential() const override;
	virtual bool open(OpenMode flags) override;
	virtual void close() override;

	virtual qint64 size() const override;

	virtual qint64 bytesToWrite() const override;
	virtual qint64 bytesAvailable() const override;

	virtual qint64 pos() const override;
	virtual bool seek(qint64 pos) override;

	virtual bool   atEnd() const override;
	virtual bool   reset() override;
	// Internal Functions.
protected:
	virtual qint64 readData(char *data, qint64 len) override;
	virtual qint64 writeData(const char *data, qint64 len) override;

	// For thread safing calling.
	std::atomic<qint64> wroteFromBefore;
	std::atomic<bool>   is_emitted;

	// Unique functions
public:
	virtual bool resize(qint64 sz);
	virtual qint64 real_buffer_size();
	virtual qint64 read_from_buffer(char *data, qint64 len);
	virtual qint64 write_to_buffer(const char *data, qint64 len);
public slots:
	void   _emit_for_write();
signals:
	int   _signal_for_write();
};

QT_END_NAMESPACE
