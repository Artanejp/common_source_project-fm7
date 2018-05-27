/*
 * Menu_QDClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */


#ifndef _CSP_QT_MENU_QD_CLASSES_H
#define _CSP_QT_MENU_QD_CLASSES_H

#include "menu_metaclass.h"

QT_BEGIN_NAMESPACE

class DLL_PREFIX Menu_QDClass: public Menu_MetaClass {
	Q_OBJECT
protected:
public:
	Menu_QDClass(QMenuBar *root_entry, QString desc, USING_FLAGS *p, QWidget *parent = 0, int drv = 0, int base_drv = 1);
	~Menu_QDClass();
	void create_pulldown_menu_device_sub();
	void connect_menu_device_sub(void);
	void retranslate_pulldown_menu_device_sub(void);
};

QT_END_NAMESPACE

#endif
