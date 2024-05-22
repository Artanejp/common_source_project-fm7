#pragma once

#include <QObject>
#include <memory>

#include "../../common.h"

QT_BEGIN_NAMESPACE

class DLL_PREFIX HouseKeeperClass : public QObject {
	Q_OBJECT
private:
	std::atomic<uint32_t> m_tick;
	std::atomic<int>      m_timer_id;
protected:
	virtual void timerEvent(QTimerEvent *event) override;
public:
	HouseKeeperClass(QObject *parent = nullptr);
	~HouseKeeperClass();

public slots:
	void do_set_interval(uint32_t msec);
};
QT_END_NAMESPACE

