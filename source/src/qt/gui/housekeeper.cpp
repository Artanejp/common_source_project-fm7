#include "housekeeper.h"
#include "mainwidget_base.h"

#include <QCoreApplication>
#include <QElapsedTimer>

HouseKeeperClass::HouseKeeperClass(QCoreApplication* app, QObject* parent)
	: QThread(parent) , m_tick(25), m_started(false), m_running(true), m_paused(false), m_event_loop(this)
{
	m_app = app;
	connect(this, SIGNAL(started()), this, SLOT(__started()));
	connect(this, SIGNAL(finished()), this, SLOT(___finished()));
}

HouseKeeperClass::~HouseKeeperClass()
{
}

void HouseKeeperClass::do_start()
{
//	m_event_loop.wakeUp();
	start(QThread::NormalPriority);
}

void HouseKeeperClass::__started()
{
	// Started signal met.
//	m_event_loop.moveToThread(this->thread());
	m_started = true;
	emit sig_started();
}

void HouseKeeperClass::__finished()
{
	// Started signal met.
	m_running = false;
	m_started = false;
	emit sig_finished();
}

void HouseKeeperClass::do_set_priority(QThread::Priority prio)
{
	setPriority(prio);
}

void HouseKeeperClass::do_pause()
{
	m_paused = true;
}

void HouseKeeperClass::do_unpause()
{
	m_paused = false;
}

qint64 HouseKeeperClass::do_set_interval(uint32_t msec)
{
	if((msec > 0) && (msec < INT32_MAX)) {
		m_tick = (qint64)msec;
		return m_tick.load();
	}
	return 0;
}


qint64 HouseKeeperClass::calc_remain_ms(qint64 tick)
{
	__UNLIKELY_IF(m_elapsed.isValid()) {
		return 0;
	}
	qint64 elapsed = m_elapsed.elapsed();
	if(tick < elapsed) {
		return 0;
	}
	return (tick - elapsed);
}

void HouseKeeperClass::run()
{
	qint64 msec = 0;
	QAbstractEventDispatcher *dispatcher = eventDispatcher();
	while(!(m_running.load())) {
		msec = 0;
		m_elapsed.start();
		qint64 tick = m_tick.load();
		qint64 msec2 = (qint64)tick;
		msec2 = (msec2 * 75) / 100; // Overhead should be upto 75%
		if(msec2 > 0) {
			m_event_loop.processEvents(QEventLoop::AllEvents, msec2);
		}
		if(!(m_started.load()) && !(m_running.load())) {
			break; // Exit Loop.
		}
		qint64 diff = 0;
		if(!(m_paused.load())) {
//			diff = calc_remain_ms(tick);
//			if(diff > 0) {
			QCoreApplication::processEvents(QEventLoop::AllEvents, tick);
//			}
		}
		diff = calc_remain_ms(tick);
		if(diff > 0) {
			QThread::msleep(diff);
		}
		QThread::yieldCurrentThread();
	}
}


