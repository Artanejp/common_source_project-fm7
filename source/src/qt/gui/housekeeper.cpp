#include "housekeeper.h"
#include <QCoreApplication>

HouseKeeperClass::HouseKeeperClass(QObject* parent)
	: QObject(parent) , m_tick(25)
{
	setObjectName(QString::fromUtf8("HouseKeeper"));
	m_timer_id = startTimer((int)(m_tick.load()));
}

HouseKeeperClass::~HouseKeeperClass()
{
	int _id = m_timer_id.load();
	if(_id != 0) {
		killTimer(_id);
	}
}

void HouseKeeperClass::do_set_interval(uint32_t msec)
{
	if(msec > 0) {
		m_tick = (int)msec;
	}
}

void HouseKeeperClass::timerEvent(QTimerEvent *event)
{
	if(event == nullptr) return;
	int _id = m_timer_id.load();
	int msec = (int)(m_tick.load());
	if((_id != 0) && (event->timerId() == _id)) {
		if(msec > 5) {
			msec = msec - 5;
		}
		if(msec > 0) {
			QCoreApplication::processEvents(QEventLoop::AllEvents, msec);
		}
	}
}

