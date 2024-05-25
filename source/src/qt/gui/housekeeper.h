#pragma once

#include <QEventLoop>
#include <QThread>
#include <QElapsedTimer>
#include <memory>

#include "../../common.h"

QT_BEGIN_NAMESPACE

class QCoreApplication;
class DLL_PREFIX HouseKeeperClass : public QThread {
	Q_OBJECT
protected:
	std::atomic<qint64> m_tick;
	std::atomic<bool> m_started;
	std::atomic<bool> m_running;
	std::atomic<bool> m_paused;
	
	QElapsedTimer m_elapsed;
	QCoreApplication *m_app;
	QEventLoop m_event_loop;
	
	qint64 calc_remain_ms(qint64 tick);

	void run() override;
public:
	HouseKeeperClass(QCoreApplication* app, QObject* parent = nullptr);
	~HouseKeeperClass();
protected slots:
	virtual void __started();
	virtual void __finished();
	
public slots:
	void do_set_priority(QThread::Priority prio);
	virtual qint64 do_set_interval(uint32_t msec);

	
	void do_start();
	void do_pause();
	void do_unpause();
signals:
	int sig_started();
	int sig_finished();
};
QT_END_NAMESPACE

