/*
 * Menu_MetaClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */


#ifndef _CSP_QT_MENU_METACLASSES_H
#define _CSP_QT_MENU_METACLASSES_H

#include <QString>
#include <QStringList>
#include <QWidget>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QActionGroup>

#include "common.h"
#include "config.h"
#include "emu.h"
#include "vm.h"
class EMU;

QT_BEGIN_NAMESPACE

class Menu_MetaClass : public QMenu {
	Q_OBJECT
private:

protected:
	QWidget *p_wid;
	QMenuBar *menu_root;
	EMU *p_emu;

	QMenu *menu_inner_media;
	QMenu *menu_history;
	QMenu *menu_write_protect;

	class Action_Control *action_insert;
	class Action_Control *action_eject;
	class Action_Control *action_recent;
	class Action_Control *action_inner_media;
	class Action_Control *action_write_protect_on;
	class Action_Control *action_write_protect_off;
#if defined(USE_FD1)
	class Action_Control *action_select_media_list[MAX_D88_BANKS];
#endif   
	class Action_Control *action_recent_list[MAX_HISTORY];

	QActionGroup *action_group_recent;
	QActionGroup *action_group_inner_media;
	QActionGroup *action_group_protect;
	
	QString object_desc;
	
	int media_drive;
	bool use_write_protect;
	bool use_d88_menus;
	
	bool write_protect;
	
	void create_pulldown_menu_sub(void);
	void retranslate_pulldown_menu_sub(void);
  
	QString window_title;
	QString initial_dir;
	QStringList ext_filter;
	QStringList history;
	QStringList inner_media_list;
public:
	Menu_MetaClass(EMU *ep, QMenuBar *root_entry, QString desc, QWidget *parent = 0, int drv = 0);
	~Menu_MetaClass();

	virtual void create_pulldown_menu_device_sub(void);
	virtual void connect_menu_device_sub(void);
	virtual void retranslate_pulldown_menu_device_sub(void);

	void create_pulldown_menu(void);
	void retranslateUi(void);
	//void setTitle(QString);
	void setEmu(EMU *p);

	bool getWriteProtect(void) {
		return write_protect;
	}
	//QAction *menuAction(void);
public slots:
	void do_set_write_protect(bool f);
	void do_open_media(int drv, QString name);
	void do_insert_media(void);
	void do_eject_media(void);
	void do_open_inner_media(int drv, int s_num);
	void do_open_recent_media(int drv, int s_num);
	void do_write_protect_media(void);
	void do_write_unprotect_media(void);
	void do_add_media_extension(QString ext, QString description);
	void do_set_initialize_directory(const char *dir);
	void do_open_dialog(void);
	void do_clear_inner_media(void);
	void do_select_inner_media(int num);
#if defined(USE_FD1)
	void do_update_inner_media(QStringList lst, int num);
#endif   
	void do_update_histories(QStringList lst);
	void do_set_window_title(QString s);
signals:
	int sig_open_media(int, QString);
	int sig_eject_media(int);
	int sig_write_protect_media(int, bool);
	int sig_set_recent_media(int, int);
	int sig_set_inner_slot(int, int);
	int sig_insert_media(int);
};
QT_END_NAMESPACE

#endif
