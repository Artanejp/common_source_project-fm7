
#include <QUdpSocket>
#include "./osd_socket.h"


QUdpSocket2::QUdpSocket2(int channel, QObject *parent) : QUdpSocket(parent)
{
	ch = channel;
}

QUdpSocket2::~QUdpSocket2()
{
}

void QUdpSocket2::do_connected(void)
{
	emit sig_connected(ch);
}

void QUdpSocket2::do_disconnected(void)
{
	emit sig_disconnected(ch);
}

void QUdpSocket2::setChannel(int channel)
{
	ch = channel;
}

int QUdpSocket2::getChannel(void)
{
	return ch;
}
