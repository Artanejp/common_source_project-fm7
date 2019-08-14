#include <QKeyEvent>
#include "qt_lineeditplus.h"

QLineEditPlus::QLineEditPlus(const QString &contents, QWidget *parent) : QLineEdit(contents, parent)
{
	list.clear();
	pointer = 0;
	connect(this, SIGNAL(editingFinished()), this, SLOT(redirectEditString2()));
}

QLineEditPlus::~QLineEditPlus()
{
}

void QLineEditPlus::keyPressEvent(QKeyEvent *event)
{
	int keyCode = event->key();
	QString _text;
	switch(keyCode) {
	case Qt::Key_Up:
		if(pointer < list.count()) {
			_text = list.at(pointer);
			if(!(_text.isEmpty())) {
				setText(_text);
			}
			pointer++;
			if(pointer >= list.count()) pointer = list.count() - 1;
			return;
		}
	case Qt::Key_Down:
		if(pointer > 0) {
			pointer--;
			_text = list.at(pointer);
			if(!(_text.isEmpty())) {
				setText(_text);
			}
			return;
		}
		break;
	}
	QLineEdit::keyPressEvent(event);
}

void QLineEditPlus::clearHistory(void)
{
	list.clear();
	pointer = 0;
}

int QLineEditPlus::historyCount(void)
{
	return list.count();
}

QStringList &QLineEditPlus::getList(void)
{
	return list;
}

int QLineEditPlus::maxCount(void)
{
	return QLINEEDITPLUS_MAX_HISTORY;
}


void QLineEditPlus::redirectEditString2(void)
{
	if(this->text().isEmpty()) return;
	QString _text = this->text();
	QString _first = QString::fromUtf8("");
	if(!(list.isEmpty())) {
		_first = list.first();
	}
	if(_text != _first) {
		list.prepend(_text);
		if(list.count() >= QLINEEDITPLUS_MAX_HISTORY) {
			list.removeLast();
		}
		pointer = 0; // Pointer must be tail.
	}
	emit editingFinished2();
}
