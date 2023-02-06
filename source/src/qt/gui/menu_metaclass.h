/*
 * Menu_MetaClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */


#ifndef _CSP_QT_MENU_METACLASSES_H
#define _CSP_QT_MENU_METACLASSES_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QMenu>
#include <QIcon>
#include <QVariant>

#include <memory>

#include "common.h"
#include "config.h"
#include "menu_flags.h"

class EMU_TEMPLATE;

QT_BEGIN_NAMESPACE
class QThread;
class QMenuBar;
class QAction;
class QActionGroup;
class QFileDialog;
class CSP_DiskDialog;

class USING_FLAGS;
class Action_Control;

namespace CSP_Ui_Menu {
	struct DriveIndexPair {
		int drive;
		int index;
	};
}

Q_DECLARE_METATYPE(CSP_Ui_Menu::DriveIndexPair)

class DLL_PREFIX Menu_MetaClass : public QMenu {
	Q_OBJECT
private:

protected:
	std::shared_ptr<USING_FLAGS> using_flags;
	QWidget *p_wid;
	QMenuBar *menu_root;
	config_t *p_config;
	
	QMenu *menu_inner_media;
	QMenu *menu_history;
	QMenu *menu_write_protect;

	QIcon icon_insert;
	QIcon icon_eject;
	QIcon icon_write_protected;
	QIcon icon_write_enabled;
	
	class Action_Control *action_insert;
	class Action_Control *action_eject;
	class Action_Control *action_recent;
	class Action_Control *action_inner_media;
	class Action_Control *action_write_protect_on;
	class Action_Control *action_write_protect_off;
	class Action_Control *action_select_media_list[128];
	class Action_Control *action_recent_list[MAX_HISTORY];
	QList<CSP_DiskDialog*>   dialogs;
	QActionGroup *action_group_recent;
	QActionGroup *action_group_inner_media;
	QActionGroup *action_group_protect;
	
	QString object_desc;
	
	int media_drive;
	int base_drive;
	
	bool use_write_protect;
	bool use_d88_menus;
	
	bool write_protect;
	
	void create_pulldown_menu_sub(void);
	void retranslate_pulldown_menu_sub(void);
	void do_open_dialog_common(CSP_DiskDialog* dlg);
  
	QString window_title;
	QString initial_dir;
	QStringList ext_filter;
	QStringList history;
	QStringList inner_media_list;
public:
	Menu_MetaClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent = 0, int drv = 0, int base_drv = 1);
	~Menu_MetaClass();

	virtual void create_pulldown_menu_device_sub(void);
	virtual void connect_menu_device_sub(void);
	virtual void retranslate_pulldown_menu_device_sub(void);

	void create_pulldown_menu(void);
	void retranslateUi(void);
	//void setTitle(QString);
	void setEmu(EMU_TEMPLATE *p);

	bool getWriteProtect(void) {
		return write_protect;
	}
	//QAction *menuAction(void);
public slots:
	void do_set_write_protect(bool f);
	void do_open_media(int drv, QString name);
	void do_insert_media(void);
	void do_eject_media(void);

	void do_open_inner_media(void);
	void do_open_recent_media(void);
	void do_write_protect_media(void);
	void do_write_unprotect_media(void);
	void do_add_media_extension(QString ext, QString description);
	
	void do_set_initialize_directory(const char *dir);
	virtual void do_open_dialog(void);
	void do_clear_inner_media(void);
	void do_select_inner_media(int num);
	void do_update_inner_media(QStringList lst, int num);
	void do_update_inner_media_bubble(QStringList lst, int num);
	void do_update_histories(QStringList lst);
	void do_insert_history(QString path);

	void do_set_window_title(QString s);
	void do_close_window();
	void do_finish(int i);
	
signals:
	int sig_open_media(int, QString);
	int sig_eject_media(int);
	int sig_write_protect_media(int, bool);
	int sig_set_recent_media(int, int);
	int sig_set_inner_slot(int, int);
	int sig_insert_media(int);
	int sig_update_inner_bubble(int drv, QStringList base, class Action_Control **action_select_media_list,
								QStringList lst, int num, bool use_d88_menus);
	int sig_emu_update_config();
	int sig_show();
};
QT_END_NAMESPACE

#endif
