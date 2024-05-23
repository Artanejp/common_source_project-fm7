#include "housekeeper.h"
#include "mainwidget_base.h"

#include <QCoreApplication>
#include <QTimer>

HouseKeeperClass::HouseKeeperClass(QObject* parent)
	: QThread(parent) , m_tick(25), m_timer(nullptr)
{
}

HouseKeeperClass::~HouseKeeperClass()
{
	if(m_timer != nullptr) {
		if(m_timer->isActive()) {
			m_timer->stop();
		}
		delete m_timer;
	}
}

void HouseKeeperClass::do_start()
{
	if(m_timer == nullptr) {
		m_timer = new QTimer(this);
	}
	if(m_timer != nullptr) {
		int _t = (int)(do_set_interval(0, false));
		m_timer->setTimerType(Qt::CoarseTimer);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(do_housekeep()));
		connect(this, SIGNAL(sig_timer_start(int)), m_timer, SLOT(start()), Qt::QueuedConnection);
		connect(this, SIGNAL(sig_timer_stop()), m_timer, SLOT(stop()), Qt::QueuedConnection);
		m_timer->start(_t);
	}
	start(QThread::NormalPriority);
}

void HouseKeeperClass::do_set_priority(QThread::Priority prio)
{
	setPriority(prio);
}

void HouseKeeperClass::do_start_timer(int msec)
{
	if(msec < 0) {
		msec = 0;
	}
	do_set_interval((uint32_t)msec, true);
}

void HouseKeeperClass::do_stop_timer()
{
	emit sig_timer_stop();
}
int HouseKeeperClass::do_set_interval(uint32_t msec, bool is_timer_reset)
{
	if(msec == 0) {
		msec = m_tick.load();
	}
	if((msec >= INT32_MAX) || (msec < 10)){
		msec = 10;
	}
	m_tick = msec;
	if((is_timer_reset) && (m_timer != nullptr)) {
		emit sig_timer_start((int)msec);
	}
	return msec;
}

void HouseKeeperClass::do_housekeep()
{
	int64_t msec = (int64_t)(m_tick.load());
	if(msec > INT32_MAX) {
		msec = INT32_MAX;
	}
	msec = (msec * 75) / 100; // Overhead should be upto 75%
	QCoreApplication::processEvents(QEventLoop::AllEvents, msec);
	emit sig_req_housekeeping();
}

