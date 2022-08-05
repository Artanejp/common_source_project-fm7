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
	class Action_Control *action_recent_save_list[MAX_HISTORY];
	class Action_Control *action_saving;

	QMenu *menu_history_save;
public:
	Menu_BinaryClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent = 0, int drv = 0, int base_drv = 1);
	~Menu_BinaryClass();
	void create_pulldown_menu_device_sub();
	void connect_menu_device_sub(void);
	void retranslate_pulldown_menu_device_sub(void);
public slots:
	void do_open_recent_media_save(int drv, int slot);
	void do_open_media_save(int drv, QString name);
	void do_open_save_dialog();
	void do_update_histories(QStringList lst);
signals:
	void sig_open_media_save(int, QString);
	void sig_set_recent_media_save(int, int);
};

QT_END_NAMESPACE

#endif
