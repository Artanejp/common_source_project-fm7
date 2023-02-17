/*
 * Menu_QDClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */


#ifndef _CSP_QT_MENU_QD_CLASSES_H
#define _CSP_QT_MENU_QD_CLASSES_H

#include "menu_metaclass.h"

QT_BEGIN_NAMESPACE

class EmuThreadClassBase;
class DLL_PREFIX Menu_QDClass: public Menu_MetaClass {
	Q_OBJECT
public:
	Menu_QDClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent = 0, int drv = 0, int base_drv = 1);
	~Menu_QDClass();
	void create_pulldown_menu_device_sub() override;
	void connect_menu_device_sub(void) override;
	void retranslate_pulldown_menu_device_sub(void) override;
	void connect_via_emu_thread(EmuThreadClassBase *p) override;

};

QT_END_NAMESPACE

#endif
