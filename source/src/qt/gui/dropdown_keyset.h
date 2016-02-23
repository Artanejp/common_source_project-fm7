/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#ifndef _CSP_QT_DROPDOWN_KEYSET_H
#define _CSP_QT_DROPDOWN_KEYSET_H

#include <QString>
#include <QStringList>
#include "dropdown_keytables.h"

Q_BEGIN_NAMESPACE

const keydet_table_t joystick_define[] = {
	{0x80000000, 0x80000008, "Left (Joystick)"},
	{0x80000000, 0x80000004, "Right (Joystick)"},
	{0x80000000, 0x80000002, "Up (Joystick)"},
	{0x80000000, 0x80000001, "Down (Joystick)"},
	{0x80000000, 0x80000010, "Button1  (Joystick)"},
	{0x80000000, 0x80000020, "Button2  (Joystick)"},
	{0x80000000, 0x80000040, "Button3  (Joystick)"},
	{0x80000000, 0x80000080, "Button4  (Joystick)"},
	{0x80000000, 0x80000100, "Button5  (Joystick)"},
	{0x80000000, 0x80000200, "Button6  (Joystick)"},
	{0x80000000, 0x80000400, "Button7  (Joystick)"},
	{0x80000000, 0x80000800, "Button8  (Joystick)"},
	{0x80000000, 0x80001000, "Button9  (Joystick)"},
	{0x80000000, 0x80002000, "Button10 (Joystick)"},
	{0x80000000, 0x80004000, "Button11 (Joystick)"},
	{0x80000000, 0x80008000, "Button12 (Joystick)"},
};

#define KEYDEF_MAXIMUM 512
class CSP_KeyTables: public QObject {
	Q_OBJECT
protected:
	QObject *p_obj;
	QStringList key_names;
	QStringList vk_names;
	const keydef_table_t *base_table;
	int table_size;
	keydef_table_t using_table[KEYDEF_MAXIMUM];
public:
	CSP_KeyTables(QObject *parent, const keydef_table_t *tbl = NULL);
	~CSP_KeyDropDown();
	int conv_code(uint32 scan);
	void set_base_table(const keydef_table_t *p);
	QString get_vk_name(uint32 vk);
	QString get_scan_name(uint32 scan);
	QString get_default_scan_name(uint32 scan);
	QStringList get_scan_name_list(void);
	QStringList get_vk_name_list(void);
	int get_key_table_size(void);
	uint32 get_vk_from_index(int index);
	uint32 get_scan_from_index(int index);
	uint32 get_default_scan_from_index(int index);
	
public slots:
	void set_scan_code(uint32 vk, uint32 scan);
	void set_key_table(keydef_table_t *tbl);
};

// Assign scancode to VK tables
class CSP_DropDownKeys: public QWidget {
	Q_OBJECT
protected:
	QWidget *p_wid;
	QComboBox s_name_src;
	QStringList s_name_dst;
	QString src_name;

	int index_src;
public:
	CSP_DropDownKeys(QWidget *parent);
public slots:
	void do_set_table(keydef_table_t *tbl);
signals:
	int sig_update_vk_scan(int vk, int scan);
};

Q_END_NAMESPACE


#endif //_CSP_QT_DROPDOWN_KEYSET_H
