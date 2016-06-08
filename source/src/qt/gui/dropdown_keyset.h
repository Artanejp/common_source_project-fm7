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
#include <QWidget>

#include "dropdown_keytables.h"

QT_BEGIN_NAMESPACE

const keydef_table_t joystick_define_tbl[] = {
	{0x80000000, 0x00000001, "↓ (Joystick)"},
	{0x80000000, 0x00000002, "↑ (Joystick)"},
	{0x80000000, 0x00000004, "→ (Joystick)"},
	{0x80000000, 0x00000008, "← (Joystick)"},
	{0x80000000, 0x00000010, "△ (Joystick)"},
	{0x80000000, 0x00000020, "○ (Joystick)"},
	{0x80000000, 0x00000040, "× (Joystick)"},
	{0x80000000, 0x00000080, "□ (Joystick)"},
	{0x80000000, 0x00000100, "L2 (Joystick)"},
	{0x80000000, 0x00000200, "R2 (Joystick)"},
	{0x80000000, 0x00000400, "L1 (Joystick)"},
	{0x80000000, 0x00000800, "R1 (Joystick)"},
	{0x80000000, 0x00001000, "SEL(Joystick)"},
	{0x80000000, 0x00002000, "RUN(Joystick)"},
	{0x80000000, 0x00004000, "L3 (Joystick)"},
	{0x80000000, 0x00008000, "R3 (Joystick)"},
};

#define KEYDEF_MAXIMUM 512
class DLL_PREFIX CSP_KeyTables: public QObject {
	Q_OBJECT
protected:
	QObject *p_obj;
	QStringList key_names;
	QStringList vk_names_list;
	const keydef_table_t *base_table;
	int table_size;
	keydef_table_t using_table[KEYDEF_MAXIMUM];
public:
	CSP_KeyTables(QObject *parent, const keydef_table_t *tbl = NULL);
	~CSP_KeyTables();
	int conv_code(uint32_t scan);
	void set_base_table(const keydef_table_t *p);
	QString get_vk_name(uint32_t vk);
	QString get_scan_name(uint32_t scan);
	QString get_default_scan_name(uint32_t scan);
	QStringList *get_scan_name_list(void);
	QStringList *get_vk_name_list(void);
	int get_key_table_size(void);
	uint32_t get_vk_from_index(int index);
	uint32_t get_scan_from_index(int index);
	uint32_t get_default_scan_from_index(int index);
	uint32_t get_scan_from_vk(uint32_t vk);
	uint32_t get_vk_from_scan(uint32_t scan);
	const char *get_vk_name(int index);
	const keydef_table_t *get_default_key_table(void);

	keydef_table_t *get_using_key_table(int index);
public slots:
	void do_set_scan_code(uint32_t vk, uint32_t scan);
	void do_set_key_table(const keydef_table_t *tbl);
};

// Assign scancode to VK tables

QT_END_NAMESPACE


#endif //_CSP_QT_DROPDOWN_KEYSET_H
