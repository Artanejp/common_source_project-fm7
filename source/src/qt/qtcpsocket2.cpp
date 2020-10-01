
#include <QTcpSocket>
#include "./osd_socket.h"

QTcpSocket2::QTcpSocket2(int channel, QObject *parent) : QTcpSocket(parent)
{
	ch = channel;
}

QTcpSocket2::~QTcpSocket2()
{
}

void QTcpSocket2::do_connected(void)
{
	emit sig_connected(ch);
}

void QTcpSocket2::do_disconnected(void)
{
	emit sig_disconnected(ch);
}

void QTcpSocket2::setChannel(int channel)
{
	ch = channel;
}

int QTcpSocket2::getChannel(void)
{
	return ch;
}


