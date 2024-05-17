
/*
 * Menu_MetaClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */


#ifndef _CSP_QT_MENU_HDD_CLASSES_H
#define _CSP_QT_MENU_HDD_CLASSES_H

#include "menu_metaclass.h"

QT_BEGIN_NAMESPACE

class DLL_PREFIX Menu_HDDClass: public Menu_MetaClass {
	Q_OBJECT
protected:
	class Action_Control *action_create_hdd;
public:
	Menu_HDDClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent = 0, int drv = 0, int base_drv = 1);
	~Menu_HDDClass();
	void create_pulldown_menu_device_sub() override;
	void connect_menu_device_sub(void) override;
	void connect_via_emu_thread(EmuThreadClassBase *p) override;
	void retranslate_pulldown_menu_device_sub(void) override;
public slots:
	void do_open_dialog_create_hd();
	void do_create_hard_disk(int drv, int sector_size, int sectors, int surfaces, int cylinders, QString name);
signals:
	int sig_create_disk(int, int, int, int, int, QString);
};

QT_END_NAMESPACE

#endif
