
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
	class Action_Control *action_create_fd;
	class Action_Control *action_ignore_crc_error;
	class Action_Control *action_correct_timing;
	class Action_Control *action_count_immediate;
	QIcon icon_floppy;
	bool type_mask[6];
public:
	Menu_FDClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent = 0, int drv = 0, int base_drv = 1);
	~Menu_FDClass();
	void create_pulldown_menu_device_sub() override;
	void connect_menu_device_sub(void) override;
	void retranslate_pulldown_menu_device_sub(void) override;

public slots:
	void do_set_create_mask(quint8 type, bool flag);
	
	void do_open_dialog(void) override;
	void do_open_dialog_create_fd();
	void do_create_media(quint8 media_type, QString name);
	
	void do_set_disk_count_immediate(bool flag);
	void do_set_ignore_crc_error(bool flag);
	void do_set_correct_disk_timing(bool flag);
	
signals:
	int sig_create_d88_media(int, quint8, QString);
};

QT_END_NAMESPACE

#endif
