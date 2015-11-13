/*
 * Menu_CartClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History: 2015-11-13: Initial
 */


#ifndef _CSP_QT_MENU_CART_CLASSES_H
#define _CSP_QT_MENU_CART_CLASSES_H

#include "menu_metaclass.h"

QT_BEGIN_NAMESPACE

class Menu_CartClass: public Menu_MetaClass {
	Q_OBJECT
protected:
public:
	Menu_CartClass(EMU *ep, QMenuBar *root_entry, QString desc, QWidget *parent = 0, int drv = 0);
	~Menu_CartClass();
	void create_pulldown_menu_device_sub();
	void connect_menu_device_sub(void);
	void retranslate_pulldown_menu_device_sub(void);
};

QT_END_NAMESPACE

#endif
