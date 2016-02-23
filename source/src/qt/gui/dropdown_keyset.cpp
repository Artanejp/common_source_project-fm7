/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#include "dropdown_keyset.h"

CSP_KeyTables::CSP_KeyTables(QObject *parent, const keydef_table_t *tbl) : QObject(parent)
{
	int i;
	QString tmps;
	vk_names_list.clear();
	for(i = 0; i < 256; i++) {
		tmps = QString::fromUtf8(vk_names[i]);
		vk_names_list.append(tmps);
	}
	memset(using_table, 0x00, KEYDEF_MAXIMUM * sizeof(keydef_table_t));
	base_table = NULL;
	table_size = 0;
	do_set_key_table(tbl);
}

CSP_KeyTables::~CSP_KeyTables()
{
}

void CSP_KeyTables::do_set_key_table(keydef_table_t *tbl)
{
	int i;
	
	if(tbl != NULL) {
		key_names.clear();
		base_table = tbl;
		memset(using_table, 0x00, KEYDEF_MAXIMUM * sizeof(keydef_table_t));
		table_size = 0;
		for(i = 0; i < KEYDEF_MAXIMUM; i++) {
			if(tbl[i].vk == 0xffffffff) break;
			using_table[i].vk = tbl[i].vk;
			using_table[i].scan = tbl[i].scan;
			using_table[i].name = tbl[i].name;
			key_names.append(QString::fromUtf8(tbl[i].name));
		}
		table_size = i;
	}
}

void CSP_KeyTables::do_set_scan_code(uint32 vk, uint32 scan)
{
	int i;
	if(scan >= 0x80000000) return;
	for(i = 0; i < KEYDEF_MAXIMUM; i++) {
		if(using_table[i].vk == 0xffffffff) break;
		if(using_table[i].vk == vk) {
			using_table[i].scan = scan;
			break;
		}
	}
}

QString CSP_KeyTables::get_default_scan_name(uint32 scan)
{
	int i;
	QString s = QString::fromUtf8("");
	if(base_table == NULL) return s;
	for(i = 0; i < KEYDEF_MAXIMUM; i++) {
		if(base_table[i].vk == 0xffffffff) break;
		if(base_table[i].scan == scan) {
			s = QString::fromUtf8(base_table[i].name);
			break;
		}
	}
	return s;
}

QString CSP_KeyTables::get_scan_name(uint32 scan)
{
	int i;
	QString s = QString::fromUtf8("");
	for(i = 0; i < KEYDEF_MAXIMUM; i++) {
		if(using_table[i].vk == 0xffffffff) break;
		if(using_table[i].scan == scan) {
			s = QString::fromUtf8(using_table[i].name);
			break;
		}
	}
	return s;
}

QString CSP_KeyTables::get_vk_name(uint32 vk)
{
	QString s = QString::fromUtf8("");
	if(vk >= 256) return s;
	s = vk_names_list.at((int)vk);
	return s;
}

QStringList *CSP_KeyTables::get_scan_name_list(void)
{
	return &key_names;
}

QStringList *CSP_KeyTables::get_vk_name_list(void)
{
	return &vk_names_list;
}

int CSP_KeyTables::get_key_table_size(void)
{
	return table_size;
};

uint32 CSP_KeyTables::get_vk_from_index(int index)
{
	if((index < 0) || (index > 255)) return 0xffffffff;
	return (uint32)index;
}

uint32 CSP_KeyTables::get_scan_from_index(int index)
{
	int i;
	if((index < 0) || (index > table_size)) return 0;
	if(using_table[index].vk == 0xffffffff) return 0;
	return using_table[index].scan;
}

uint32 CSP_KeyTables::get_default_scan_from_index(int index)
{
	int i;
	if((index < 0) || (index > KEYDEF_MAXIMUM)) return 0;
	if(base_table == NULL) return 0;
	for(i = 0; i < KEYDEF_MAXIMUM; i++) {
		if(base_table[i].vk == 0xffffffff) break;
	}
	if(i < index) return 0;
	return base_table[index].scan;
}

uint32 CSP_KeyTables::get_scan_from_vk(uint32 vk)
{
	int i;
	if(vk >= 256) return 0xffffffff;
	for(i = 0; i < table_size; i++) {
		if(using_table[i].vk == 0xffffffff) return 0xffffffff;
		if(using_table[i].vk == vk) {
			return using_table[i].scan;
		}
	}
}

uint32 CSP_KeyTables::get_vk_from_scan(uint32 scan)
{
	int i;

	for(i = 0; i < table_size; i++) {
		if(using_table[i].vk == 0xffffffff) return 0xffffffff;
		if(using_table[i].scan == scan) {
			return using_table[i].vk;
		}
	}
}

keydef_table_t *CSP_KeyTables::get_using_key_table(int index)
{
	if(index < 0) return NULL;
	if(index >= table_size) return NULL;
	if(index >= KEYDEF_MAXIMUM) return NULL;
	return &(using_table[index]);
}
