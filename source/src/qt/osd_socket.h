/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.09.06 Split from qt/osd.h

	[ Qt dependent / Socket]
*/
#ifndef _QT_OSD_SOCKET_H_
#define _QT_OSD_SOCKET_H_

#include <QObject>
#include <QString>
#include <QImage>

#include <SDL.h>

#include "osd_base.h"

#include <QTcpSocket>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
class DLL_PREFIX QTcpSocket2 : public QTcpSocket
{
	Q_OBJECT
protected:
	int ch;
public:
	QTcpSocket2(int channel = 0, QObject *parent = 0);
	~QTcpSocket2();
	void setChannel(int channel);
	int getChannel(void);
public slots:
	void do_connected(void);
	void do_disconnected(void);
signals:
	int sig_connected(int);
	int sig_disconnected(int);
};

class DLL_PREFIX QUdpSocket2 : public QUdpSocket
{
	Q_OBJECT
protected:
	int ch;
public:
	QUdpSocket2(int channel = 0, QObject *parent = 0);
	~QUdpSocket2();
	void setChannel(int channel);
	int getChannel(void);
public slots:
	void do_connected(void);
	void do_disconnected(void);
signals:
	int sig_connected(int);
	int sig_disconnected(int);
};
QT_END_NAMESPACE

#endif
