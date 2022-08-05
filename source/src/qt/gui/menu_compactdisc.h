/*
 * Menu_MetaClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */


#ifndef _CSP_QT_MENU_CDROM_CLASSES_H
#define _CSP_QT_MENU_CDROM_CLASSES_H

#include "menu_metaclass.h"

QT_BEGIN_NAMESPACE

class DLL_PREFIX Menu_CompactDiscClass: public Menu_MetaClass {
	Q_OBJECT
protected:
	Action_Control *action_swap_byteorder;
public:
	Menu_CompactDiscClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent = 0, int drv = 0, int base_drv = 1);
	~Menu_CompactDiscClass();
	void create_pulldown_menu_device_sub();
	void connect_menu_device_sub(void);
	void retranslate_pulldown_menu_device_sub(void);
	
public slots:
	void do_swap_cdaudio_byteorder(bool flag);
signals:
	int sig_swap_audio_byteorder(int, bool);
};

QT_END_NAMESPACE

#endif
