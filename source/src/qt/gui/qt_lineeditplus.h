#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QString>
#include <QStringList>

#define QLINEEDITPLUS_MAX_HISTORY 32
QT_BEGIN_NAMESPACE	

class QCompleter;
class QLineEditPlus : public QLineEdit {
	Q_OBJECT
protected:
	QStringList list;
	QCompleter *completer;
	int pointer;
	virtual void keyPressEvent(QKeyEvent *event);
public:
	QLineEditPlus(const QString &contents, QWidget *parent = nullptr);
	~QLineEditPlus();
	int historyCount(void);
	int maxCount(void);
	QStringList &getList(void);
	QCompleter *getCompleter();
public slots:
	void clearHistory(void);
	void redirectEditString2(void);
	void setCompleteList(QStringList complist);
signals:
	int editingFinished2();
};

QT_END_NAMESPACE	
