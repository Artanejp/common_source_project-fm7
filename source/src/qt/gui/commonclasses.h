/*
 * Qt -> GUI -> CommonClasses
 * commonclasses.h
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Licence : GPLv2
 * History : Jan 13 2015 : Split from x1turboz / menuclasses.h
 */

#ifndef _CSP_QT_GUI_COMMONCLASSES_H
#define _CSP_QT_GUI_COMMONCLASSES_H

#include <QVariant>
#include <QObject>
#include <QAction>
#include <QString>

#include "simd_types.h"
#include "common.h"
#include "config.h"
#include "menu_flags.h"

QT_BEGIN_NAMESPACE
class DLL_PREFIX Action_Control: public QAction {
	Q_OBJECT
protected:
	//    virtual void addedTo ( QWidget * actionWidget, QWidget * container ){}
	//   virtual void addedTo ( int index, QPopupMenu * menu ){}
	QString bindString;
public:
	//Object_Menu_Control *binds;
	Action_Control (QObject *parent, USING_FLAGS *p) : QAction(parent) {
		//binds = new Object_Menu_Control(parent, p);
		bindString.clear();
	}
	~Action_Control() {
		//delete binds;
	}
public slots:
	void do_set_window_focus_type(bool flag);
	
signals:
	int quit_emu_thread(void);
	int sig_set_dev_log_to_console(int, bool);
	int sig_set_dev_log_to_syslog(int, bool);
	int sig_set_window_focus_type(bool);
};
QT_END_NAMESPACE

#endif


 
