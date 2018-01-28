
/*
 * Menu_MetaClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */


#ifndef _CSP_QT_MENU_FD_CLASSES_H
#define _CSP_QT_MENU_FD_CLASSES_H

#include "menu_metaclass.h"

QT_BEGIN_NAMESPACE

class DLL_PREFIX Menu_FDClass: public Menu_MetaClass {
	Q_OBJECT
protected:
	class Action_Control *action_ignore_crc_error;
	class Action_Control *action_correct_timing;
	class Action_Control *action_count_immediate;
	QIcon icon_floppy;
public:
	Menu_FDClass(QMenuBar *root_entry, QString desc, USING_FLAGS *p, QWidget *parent = 0, int drv = 0);
	~Menu_FDClass();
	void create_pulldown_menu_device_sub();
	void connect_menu_device_sub(void);
	void retranslate_pulldown_menu_device_sub(void);
};

QT_END_NAMESPACE

#endif
