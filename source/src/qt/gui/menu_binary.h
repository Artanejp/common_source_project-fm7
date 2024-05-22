/*
 * Menu_MetaClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */


#ifndef _CSP_QT_MENU_BINARY_CLASSES_H
#define _CSP_QT_MENU_BINARY_CLASSES_H

#include "menu_metaclass.h"

QT_BEGIN_NAMESPACE

class DLL_PREFIX Menu_BinaryClass: public Menu_MetaClass {
	Q_OBJECT
protected:
	QActionGroup *action_group_save_recent;
	QAction *action_recent_save_list[MAX_HISTORY];
	QAction *action_saving;

public:
	Menu_BinaryClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent = 0, int drv = 0, int base_drv = 1);
	~Menu_BinaryClass();
	void create_pulldown_menu_device_sub() override;
	void connect_menu_device_sub(void) override;
	void retranslate_pulldown_menu_device_sub(void) override;
public slots:
	void do_update_histories(QStringList lst) override;
signals:
};

QT_END_NAMESPACE

#endif
