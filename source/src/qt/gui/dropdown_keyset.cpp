/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */
#include <QString>
#include <QStringList>
#include <QComboBox>
#include <QWidget>

#include "dropdown_keyset.h"

static const _TCHAR *vk_names[] = {
	_T("VK_$00"),			_T("VK_LBUTTON"),		_T("VK_RBUTTON"),		_T("VK_CANCEL"),		
	_T("VK_MBUTTON"),		_T("VK_XBUTTON1"),		_T("VK_XBUTTON2"),		_T("VK_$07"),			
	_T("VK_BACK"),			_T("VK_TAB"),			_T("VK_$0A"),			_T("VK_$0B"),			
	_T("VK_CLEAR"),			_T("VK_RETURN"),		_T("VK_$0E"),			_T("VK_$0F"),			
	_T("VK_SHIFT"),			_T("VK_CONTROL"),		_T("VK_MENU"),			_T("VK_PAUSE"),			
	_T("VK_CAPITAL"),		_T("VK_KANA"),			_T("VK_$16"),			_T("VK_JUNJA"),			
	_T("VK_FINAL"),			_T("VK_KANJI"),			_T("VK_$1A"),			_T("VK_ESCAPE"),		
	_T("VK_CONVERT"),		_T("VK_NONCONVERT"),		_T("VK_ACCEPT"),		_T("VK_MODECHANGE"),		
	_T("VK_SPACE"),			_T("VK_PRIOR"),			_T("VK_NEXT"),			_T("VK_END"),			
	_T("VK_HOME"),			_T("VK_LEFT"),			_T("VK_UP"),			_T("VK_RIGHT"),			
	_T("VK_DOWN"),			_T("VK_SELECT"),		_T("VK_PRINT"),			_T("VK_EXECUTE"),		
	_T("VK_SNAPSHOT"),		_T("VK_INSERT"),		_T("VK_DELETE"),		_T("VK_HELP"),			
	_T("VK_0"),			_T("VK_1"),			_T("VK_2"),			_T("VK_3"),			
	_T("VK_4"),			_T("VK_5"),			_T("VK_6"),			_T("VK_7"),			
	_T("VK_8"),			_T("VK_9"),			_T("VK_$3A"),			_T("VK_$3B"),			
	_T("VK_$3C"),			_T("VK_$3D"),			_T("VK_$3E"),			_T("VK_$3F"),			
	_T("VK_$40"),			_T("VK_A"),			_T("VK_B"),			_T("VK_C"),			
	_T("VK_D"),			_T("VK_E"),			_T("VK_F"),			_T("VK_G"),			
	_T("VK_H"),			_T("VK_I"),			_T("VK_J"),			_T("VK_K"),			
	_T("VK_L"),			_T("VK_M"),			_T("VK_N"),			_T("VK_O"),			
	_T("VK_P"),			_T("VK_Q"),			_T("VK_R"),			_T("VK_S"),			
	_T("VK_T"),			_T("VK_U"),			_T("VK_V"),			_T("VK_W"),			
	_T("VK_X"),			_T("VK_Y"),			_T("VK_Z"),			_T("VK_LWIN"),			
	_T("VK_RWIN"),			_T("VK_APPS"),			_T("VK_$5E"),			_T("VK_SLEEP"),			
	_T("VK_NUMPAD0"),		_T("VK_NUMPAD1"),		_T("VK_NUMPAD2"),		_T("VK_NUMPAD3"),		
	_T("VK_NUMPAD4"),		_T("VK_NUMPAD5"),		_T("VK_NUMPAD6"),		_T("VK_NUMPAD7"),		
	_T("VK_NUMPAD8"),		_T("VK_NUMPAD9"),		_T("VK_MULTIPLY"),		_T("VK_ADD"),			
	_T("VK_SEPARATOR"),		_T("VK_SUBTRACT"),		_T("VK_DECIMAL"),		_T("VK_DIVIDE"),		
	_T("VK_F1"),			_T("VK_F2"),			_T("VK_F3"),			_T("VK_F4"),			
	_T("VK_F5"),			_T("VK_F6"),			_T("VK_F7"),			_T("VK_F8"),			
	_T("VK_F9"),			_T("VK_F10"),			_T("VK_F11"),			_T("VK_F12"),			
	_T("VK_F13"),			_T("VK_F14"),			_T("VK_F15"),			_T("VK_F16"),			
	_T("VK_F17"),			_T("VK_F18"),			_T("VK_F19"),			_T("VK_F20"),			
	_T("VK_F21"),			_T("VK_F22"),			_T("VK_F23"),			_T("VK_F24"),			
	_T("VK_$88"),			_T("VK_$89"),			_T("VK_$8A"),			_T("VK_$8B"),			
	_T("VK_$8C"),			_T("VK_$8D"),			_T("VK_$8E"),			_T("VK_$8F"),			
	_T("VK_NUMLOCK"),		_T("VK_SCROLL"),		_T("VK_$92"),			_T("VK_$93"),			
	_T("VK_$94"),			_T("VK_$95"),			_T("VK_$96"),			_T("VK_$97"),			
	_T("VK_$98"),			_T("VK_$99"),			_T("VK_$9A"),			_T("VK_$9B"),			
	_T("VK_$9C"),			_T("VK_$9D"),			_T("VK_$9E"),			_T("VK_$9F"),			
	_T("VK_LSHIFT"),		_T("VK_RSHIFT"),		_T("VK_LCONTROL"),		_T("VK_RCONTROL"),		
	_T("VK_LMENU"),			_T("VK_RMENU"),			_T("VK_BROWSER_BACK"),		_T("VK_BROWSER_FORWARD"),	
	_T("VK_BROWSER_REFRESH"),	_T("VK_BROWSER_STOP"),		_T("VK_BROWSER_SEARCH"),	_T("VK_BROWSER_FAVORITES"),	
	_T("VK_BROWSER_HOME"),		_T("VK_VOLUME_MUTE"),		_T("VK_VOLUME_DOWN"),		_T("VK_VOLUME_UP"),		
	_T("VK_MEDIA_NEXT_TRACK"),	_T("VK_MEDIA_PREV_TRACK"),	_T("VK_MEDIA_STOP"),		_T("VK_MEDIA_PLAY_PAUSE"),	
	_T("VK_LAUNCH_MAIL"),		_T("VK_LAUNCH_MEDIA_SELECT"),	_T("VK_LAUNCH_APP1"),		_T("VK_LAUNCH_APP2"),		
	_T("VK_$B8"),			_T("VK_$B9"),			_T("VK_OEM_1"),			_T("VK_OEM_PLUS"),		
	_T("VK_OEM_COMMA"),		_T("VK_OEM_MINUS"),		_T("VK_OEM_PERIOD"),		_T("VK_OEM_2"),			
	_T("VK_OEM_3"),			_T("VK_$C1"),			_T("VK_$C2"),			_T("VK_$C3"),			
	_T("VK_$C4"),			_T("VK_$C5"),			_T("VK_$C6"),			_T("VK_$C7"),			
	_T("VK_$C8"),			_T("VK_$C9"),			_T("VK_$CA"),			_T("VK_$CB"),			
	_T("VK_$CC"),			_T("VK_$CD"),			_T("VK_$CE"),			_T("VK_$CF"),			
	_T("VK_$D0"),			_T("VK_$D1"),			_T("VK_$D2"),			_T("VK_$D3"),			
	_T("VK_$D4"),			_T("VK_$D5"),			_T("VK_$D6"),			_T("VK_$D7"),			
	_T("VK_$D8"),			_T("VK_$D9"),			_T("VK_$DA"),			_T("VK_OEM_4"),			
	_T("VK_OEM_5"),			_T("VK_OEM_6"),			_T("VK_OEM_7"),			_T("VK_OEM_8"),			
	_T("VK_$E0"),			_T("VK_OEM_AX"),		_T("VK_OEM_102"),		_T("VK_ICO_HELP"),		
	_T("VK_ICO_00"),		_T("VK_PROCESSKEY"),		_T("VK_ICO_CLEAR"),		_T("VK_PACKET"),		
	_T("VK_$E8"),			_T("VK_OEM_RESET"),		_T("VK_OEM_JUMP"),		_T("VK_OEM_PA1"),		
	_T("VK_OEM_PA2"),		_T("VK_OEM_PA3"),		_T("VK_OEM_WSCTRL"),		_T("VK_OEM_CUSEL"),		
	_T("VK_OEM_ATTN"),		_T("VK_OEM_FINISH"),		_T("VK_OEM_COPY"),		_T("VK_OEM_AUTO"),		
	_T("VK_OEM_ENLW"),		_T("VK_OEM_BACKTAB"),		_T("VK_ATTN"),			_T("VK_CRSEL"),			
	_T("VK_EXSEL"),			_T("VK_EREOF"),			_T("VK_PLAY"),			_T("VK_ZOOM"),			
	_T("VK_NONAME"),		_T("VK_PA1"),			_T("VK_OEM_CLEAR"),		_T("VK_$FF"),			
};

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

void CSP_KeyTables::do_set_key_table(const keydef_table_t *tbl)
{
	int i;
	
	if(tbl != NULL) {
		key_names.clear();
		base_table = tbl;
		memset(using_table, 0x00, KEYDEF_MAXIMUM * sizeof(keydef_table_t));
		table_size = 0;
		//bool authorised_vk[256];
		//for(i = 0; i < 256; i++) authorised_vk[i] = false;
		for(i = 0; i < KEYDEF_MAXIMUM; i++) {
			if(tbl[i].vk == 0xffffffff) break;
			if(tbl[i].vk >= 256) continue;
			using_table[i].vk = tbl[i].vk;
			using_table[i].scan = tbl[i].scan;
			using_table[i].name = tbl[i].name;
			key_names.append(QString::fromUtf8(tbl[i].name));
			//authorised_vk[tbl[i].vk] = true;
		}
		table_size = i;
	}
}

void CSP_KeyTables::do_set_scan_code(uint32_t vk, uint32_t scan)
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

QString CSP_KeyTables::get_default_scan_name(uint32_t scan)
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

QString CSP_KeyTables::get_scan_name(uint32_t scan)
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

QString CSP_KeyTables::get_vk_name(uint32_t vk)
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

uint32_t CSP_KeyTables::get_vk_from_index(int index)
{
	if((index < 0) || (index > 255)) return 0xffffffff;
	return (uint32_t)index;
}

uint32_t CSP_KeyTables::get_scan_from_index(int index)
{
	if((index < 0) || (index > table_size)) return 0;
	if(using_table[index].vk == 0xffffffff) return 0;
	return using_table[index].scan;
}

uint32_t CSP_KeyTables::get_default_scan_from_index(int index)
{
	int i;
	if((index < 0) || (index > KEYDEF_MAXIMUM)) return 0;
	if(base_table == NULL) return 0;
	for(i = 0; i < KEYDEF_MAXIMUM; i++) {
		if(base_table[i].vk == 0xffffffff) return 0xffffffff;
	}
	if(i < index) return 0;
	return base_table[index].scan;
}

uint32_t CSP_KeyTables::get_scan_from_vk(uint32_t vk)
{
	int i;
	if(vk >= 256) return 0xffffffff;
	for(i = 0; i < table_size; i++) {
		if(using_table[i].vk == 0xffffffff) return 0xffffffff;
		if(using_table[i].vk == vk) {
			return using_table[i].scan;
		}
	}
	return 0xffffffff;
}

uint32_t CSP_KeyTables::get_vk_from_scan(uint32_t scan)
{
	int i;

	//printf("%d\n", scan);
	for(i = 0; i < table_size; i++) {
		if(using_table[i].vk == 0xffffffff) return 0xffffffff;
		if(using_table[i].scan == scan) {
			return using_table[i].vk;
		}
	}
	return 0xffffffff;
}

keydef_table_t *CSP_KeyTables::get_using_key_table(int index)
{
	if(index < 0) return NULL;
	if(index >= table_size) return NULL;
	if(index >= KEYDEF_MAXIMUM) return NULL;
	return &(using_table[index]);
}

const char *CSP_KeyTables::get_vk_name(int index)
{
	if((index >= 256) && (index < 0)) return NULL;
	return vk_names[index];
}

const keydef_table_t *CSP_KeyTables::get_default_key_table()
{
	return base_table;
}
