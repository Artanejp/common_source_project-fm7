#ifndef _CSP_QT_DOCKWIDGET_BASE_H
#define _CSP_QT_DOCKWIDGET_BASE_H

#include <QObject>
#include <QWidget>
//#include <QToolBar>
//#include <QDockWidget>
#include <QIcon>
#include <QString>
#include <QStringList>
#include <QPixmap>

class QLabel;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;
class USING_FLAGS;
enum {
	CSP_DockDisks_Domain_Binary = 0,
	CSP_DockDisks_Domain_Bubble,
	CSP_DockDisks_Domain_Cart,
	CSP_DockDisks_Domain_CMT,
	CSP_DockDisks_Domain_CD,
	CSP_DockDisks_Domain_FD,
	CSP_DockDisks_Domain_HD,
	CSP_DockDisks_Domain_LD,
	CSP_DockDisks_Domain_QD,
};
	
QT_BEGIN_NAMESPACE

class CSP_LabelVirtualDevice : public QWidget {
	Q_OBJECT
private:
	QHBoxLayout *HBox;
	QLabel *Indicator;
	QLabel *Message;

	QString sDesc;
	QString sMES;

	QString sStat;
	
	int _height;
	int _width;
	float _base_pt;

	int _now_width;
	int _now_height;
	float _now_pt;
	int local_num;
public:
	CSP_LabelVirtualDevice(QWidget *parent = 0,
							int width = 6, float point = 12.0f,
							QString baseName = QString::fromUtf8("DMY"), int num = 0);
	~CSP_LabelVirtualDevice();

	QString getMessage(void) { return sMES; }
	QString getLabel(void)   { return sDesc; }
	int getDeviceNum(void)   { return local_num; }
public slots:
	void setDeviceNum(int n);
	void setLabel(QString s);
	void setMessage(QString s);
	void setIndicatorStatus(QString s);
	void setVisibleIndicator(bool f);
	void setVisibleMessage(bool f);
	void setScreenWidth(int width, int basewidth);
};

class CSP_DockDisks : public QWidget {
	Q_OBJECT
private:
	USING_FLAGS *using_flags;
	//QHBoxLayout *HBox;
	QGridLayout *HVBox;
	CSP_LabelVirtualDevice *pBinary[8];
	CSP_LabelVirtualDevice *pBubble[8];
	CSP_LabelVirtualDevice *pCart[8];
	CSP_LabelVirtualDevice *pCMT[2];
	CSP_LabelVirtualDevice *pCompactDisc[2];
	CSP_LabelVirtualDevice *pFloppyDisk[8];
	CSP_LabelVirtualDevice *pHardDisk[8];
	CSP_LabelVirtualDevice *pLaserDisc[2];
	CSP_LabelVirtualDevice *pQuickDisk[2];

	bool two_rows;
	int initial_width;
	int initial_height;
	int base_width;
public:
	CSP_DockDisks(QWidget *parent, USING_FLAGS *p);
	~CSP_DockDisks();

public slots:
	void updateLabel(int dom, int localnum, QString str);
	void updateMessage(int dom, int localnum, QString str);
	void setVisibleLabel(int dom, int localNum, bool enabled);
	void setPixmap(int dom, int localNum, const QPixmap &);
	void setOrientation(int loc);
	void setScreenWidth(int width);
};
QT_END_NAMESPACE

#endif
	
