#pragma once

#include <QThread>
#include <memory>

#include "../../common.h"

QT_BEGIN_NAMESPACE

class QTimer;
class DLL_PREFIX HouseKeeperClass : public QThread {
	Q_OBJECT
private:
	std::atomic<uint32_t> m_tick;
	QTimer* m_timer;
public:
	HouseKeeperClass(QObject* parent = nullptr);
	~HouseKeeperClass();

public slots:
	void do_start();
	void do_housekeep();
	void do_set_priority(QThread::Priority prio);
	
	void do_start_timer(int msec);
	void do_stop_timer();
	int do_set_interval(uint32_t msec, bool is_timer_reset = false);
signals:
	int sig_timer_start(int);
	int sig_timer_stop();
	int sig_req_housekeeping();
};
QT_END_NAMESPACE

