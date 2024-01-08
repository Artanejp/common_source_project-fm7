/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from emu_input.cpp
	[ win32 main ] -> [ Qt main ] -> [Joy Stick]
*/
#include <Qt>
#include <QApplication>
#include <SDL.h>
#include "emu_template.h"
#include "osd_base.h"
#include "fifo.h"
#include "fileio.h"
#include "qt_input.h"
#include "qt_gldraw.h"
#include "qt_main.h"
#include "csp_logger.h"

#include "joy_thread.h"
#include "../emu_thread/emu_thread_tmpl.h"

#include "../../fileio.h"

JoyThreadClass::JoyThreadClass(std::shared_ptr<EmuThreadClassBase> p, std::shared_ptr<USING_FLAGS> pflags, config_t *cfg, QObject *parent) : QThread(parent)
{
	//int i, j;
	int i;
	int n;
	p_osd = NULL;
	p_emu_thread.reset();
	p_config = cfg;
	using_flags = pflags;
	csp_logger.reset();
	joydb.clear();
	SetEmu(p);

	char tmp_string[2048] = {0};
	my_tcscat_s(tmp_string, 2047, "a:b0,b:b1,x:b2,y:b3,start:b9,guide:b8,");
	my_tcscat_s(tmp_string, 2047, "rightshoulder:b7,leftshoulder:b6,leftstick:b5,rightstick:b4,");
	my_tcscat_s(tmp_string, 2047, "dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,");
	my_tcscat_s(tmp_string, 2047, "leftx:a0,lefty:a1,rightx:a2,righty:a3");
	default_assign = QString::fromLocal8Bit(tmp_string);

	if(using_flags->is_use_joystick()) {
# if defined(USE_SDL2)
		int result = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_EVENTS);
		read_joydb();
		//int result = 0;
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "Joystick/Game controller subsystem was %s.", (result == 0) ? "initialized" : "not initialized");
		for(i = 0; i < 16; i++) {
			controller_table[i] = NULL;
		}
# endif
		n = SDL_NumJoysticks();
		for(i = 0; i < 16; i++) {
			joyhandle[i] = NULL;
			names[i] = QString::fromUtf8("");
			joy_num[i] = i;
			joy_assign[i] = p_config->assigned_joystick_num[i];
			is_controller[i] = false;
		}
		for(i = 0; i < 4; i++) {
			emulate_dpad[i] = p_config->emulated_joystick_dpad[i];
		}
		if(n > 0) {
			if(n >= 16) n = 16;
# if !defined(USE_SDL2)
			for(i = 0; i < n; i++) {
				joystick_plugged(i);
			}
# endif
			debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "JoyThread : Start.");
		} else {
			debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "JoyThread : Any joysticks were not connected.");
		}
		bRunThread = (result == 0) ? true : false;
	} else {
		for(i = 0; i < 16; i++) {
			joyhandle[i] = NULL;
			names[i] = QString::fromUtf8("None");
		}
	    debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "JoyThread : None launched because this VM has not supported joystick.");
		bRunThread = false;
	}
}


JoyThreadClass::~JoyThreadClass()
{
	int i;
	if(using_flags->is_use_joystick()) {
# if defined(USE_SDL2)
		for(i = 0; i < 16; i++) {
			if(controller_table[i] != NULL) {
				SDL_GameControllerClose(controller_table[i]);
			}
			controller_table[i] = NULL;
		}
# else
		for(i = 0; i < 16; i++) {
			SDL_JoystickClose(joyhandle[i]);
			joyhandle[i] = NULL;
		}
# endif
		//if(!(joydb.isEmpty())) {
		//	write_joydb();
		//}
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "JoyThread : EXIT");
	}
}

void JoyThreadClass::SetEmu(std::shared_ptr<EmuThreadClassBase> p)
{
	if(p.get() != nullptr) {
		p_emu_thread = p;
		if(csp_logger.get() != nullptr) {
			disconnect(this, NULL, csp_logger.get(), NULL);
		}
		EMU_TEMPLATE* p_emu = p_emu_thread->get_emu();
		if(p_emu == nullptr) return;
		
		p_osd = p_emu->get_osd();
		if(p_osd != NULL) {
			csp_logger = p_osd->get_logger();
			connect(this, SIGNAL(sig_debug_log(int, int, QString)),
					csp_logger.get(), SLOT(do_debug_log(int, int, QString)));
		}
	}
}

void JoyThreadClass::debug_log(int level, int domain_num, QString msg)
{
	emit sig_debug_log(level, domain_num, msg);
}

int JoyThreadClass::read_joydb()
{
	QString app_path2 = QString::fromUtf8("");
	if(using_flags.get() != nullptr) {
		app_path2 = using_flags->get_config_directory();
	}
	app_path2 = app_path2 + QString::fromUtf8(JOY_DB_NAME);

	if(FILEIO::IsFileExisting((const _TCHAR *)(app_path2.toLocal8Bit().constData()))) {
		return 0;
	}
	FILEIO *fp = new FILEIO();
	int count = 0;

	if(fp != NULL) {
		if(fp->Fopen(app_path2.toLocal8Bit().constData(), FILEIO_READ_ASCII)) {
			char tmpline[2048] = {0};
			while(fp->Fgets(tmpline, 2047) != (char *)0) {
				QString n = QString::fromLocal8Bit(tmpline);
				if(!(n.isEmpty())) {
					replace_joydb(n);
					count++;
				}
			}
			fp->Fclose();
		}
		delete fp;
	}
	joydb.removeDuplicates();
	return count;
}

int JoyThreadClass::write_joydb()
{
	FILEIO *fp = new FILEIO();
	QString app_path2 = QString::fromUtf8("");
	if(using_flags.get() != nullptr) {
		app_path2 = using_flags->get_config_directory();
	}
	app_path2 = app_path2 + QString::fromUtf8(JOY_DB_NAME);
	int count = 0;
	if(fp != NULL) {
		if(fp->Fopen((const _TCHAR *)(app_path2.toLocal8Bit().constData()), FILEIO_WRITE_ASCII)) {
			for(auto n = joydb.begin(); n != joydb.end(); ++n) {
				if(fp->Fprintf("%s\n", (*n).toLocal8Bit().constData()) <= 0) break;
				count++;
			}
			fp->Fclose();
		}
		delete fp;
	}
	return count;
}

bool JoyThreadClass::search_joydb(QString str, QString& answer)
{
	return search_joydb_by_guid(str.left(32), answer);
}

bool JoyThreadClass::search_joydb_by_guid(QString guid, QString& answer)
{
	if(guid.size() >= 32) {
		if(joydb.contains(guid.left(32))) {
			int i = joydb.indexOf(guid.left(32));
			if(i >= 0) {
				answer = joydb.at(i);
				return true;
			}
		}
	}
	return false;
}

bool JoyThreadClass::replace_joydb(QString after)
{
	QString guid = after.left(32);
	return replace_joydb_by_guid(guid, after);
}

bool JoyThreadClass::replace_joydb_by_guid(QString guid, QString after)
{
	if(guid.size() >= 32) {
		QString n = after;
		QString m = n.remove("\n");
		QString l = m.remove("\r");
		if(joydb.contains(guid.left(32))) {
			int i = joydb.indexOf(guid.left(32));
			if(i >= 0) {
				joydb.replace(i, l);
				joydb.removeDuplicates();
				return true;
			}
		}
		joydb.append(l);
		joydb.removeDuplicates();
		return true;
	}
	return false;
}

void JoyThreadClass::debug_log(int level, int domain_num, const char *fmt, ...)
{
	char strbuf[4096] = {0};

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(strbuf, 4095, fmt, ap);
	va_end(ap);

	emit sig_debug_log(level, domain_num, QString::fromUtf8(strbuf));
}

QString JoyThreadClass::default_joyassign()
{
	return default_assign;
}

void JoyThreadClass::do_replace_default_assign(QString new_assign)
{
	default_assign = new_assign;
}


QString JoyThreadClass::make_guid(SDL_JoystickGUID guid)
{
	char guid_string[2048] = {0};
	char tmpstr[1024];
	for(int i = 0; i < 16; i++) {
		memset(tmpstr, 0x00, sizeof(tmpstr));
		my_stprintf_s(tmpstr, 128, _T("%02x"), guid.data[i]);
		my_tcscat_s(guid_string, 2047, tmpstr);
	}
	QString r = QString::fromUtf8(guid_string);
	return r;
}

QString JoyThreadClass::joystick_guid(int num)
{
	SDL_JoystickGUID guid;
	guid = SDL_JoystickGetDeviceGUID(num);
	QString guid_str = make_guid(guid);
	return guid_str;
}

QString JoyThreadClass::joystick_name(int num)
{
	const char *p = SDL_JoystickNameForIndex(num);
	if(p != NULL) {
		return QString::fromLocal8Bit(p);
	}

	return QString::fromUtf8("");
}

void JoyThreadClass::joystick_plugged(int num)
{
	//int i,j;
	int i;
	//bool found = false;

# if defined(USE_SDL2)
	SDL_JoystickGUID guid;
	guid = SDL_JoystickGetDeviceGUID(num);

	QString guid_str = make_guid(guid);
	QString answer;
	if(!(search_joydb_by_guid(guid_str, answer))) {
		const char *p = SDL_JoystickNameForIndex(num);
		char _n[1024] = {0};
		if(p != NULL) {
			my_tcscpy_s(_n, 1023, p);
		}
		answer = guid_str +
			QString::fromUtf8(",") +
			QString::fromLocal8Bit(_n) +
			QString::fromUtf8(",") +
			default_joyassign();
	}
	if(!(answer.isEmpty())) {
		if(SDL_GameControllerAddMapping(answer.toLocal8Bit().constData()) >= 0) {
			debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "MAP JOYSTICK%d to: %s", num, answer.toLocal8Bit().constData());
			replace_joydb(answer);
		}
	}

	if(SDL_IsGameController(num) == SDL_TRUE) {
		//	if(controller_table[num] != NULL) return;
		controller_table[num] = SDL_GameControllerOpen(num);
		joyhandle[num] = SDL_GameControllerGetJoystick(controller_table[num]);
		if(controller_table[num] != NULL) {
			names[num] = QString::fromLocal8Bit(SDL_GameControllerNameForIndex(num));
			debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "JoyThread : Controller %d : %s : is plugged.", num, names[num].toUtf8().constData());
			strncpy(p_config->assigned_joystick_name[num], names[num].toUtf8().constData(),
					(sizeof(p_config->assigned_joystick_name[num])  / sizeof(char)) - 1);
			joy_num[num] = num;
			if((num >= 0) && (num < 16)) {
				is_controller[num] = true;
				joy_assign[num] = p_config->assigned_joystick_num[num];
				if((joy_assign[num] >= 0) && (joy_assign[num] < 4)) {
					emulate_dpad[joy_assign[num]] = p_config->emulated_joystick_dpad[joy_assign[num]];
					emit sig_state_dpad(num, emulate_dpad[joy_assign[num]]);
				}
			}
		}
	} else
#endif
	{
		bool matched = false;
		QString tmps;
		if(!matched) {
			for(i = 0; i < 16; i++) {
				if(joyhandle[i] == NULL) {
					joyhandle[i] = SDL_JoystickOpen(num);
					joy_num[i] = SDL_JoystickInstanceID(joyhandle[i]);
					if((joy_num[i] >= 0) && (joy_num[i] < 16)) {
						is_controller[joy_num[i]] = false;
						joy_assign[joy_num[i]] = p_config->assigned_joystick_num[joy_num[i]];
						if((joy_assign[num] >= 0) && (joy_assign[num] < 4)) {
							emulate_dpad[joy_assign[joy_num[i]]] = true; // If as Joystick (not Game Controller), must emulate digital pad.
							p_config->emulated_joystick_dpad[joy_assign[joy_num[i]]] = true;
							emit sig_state_dpad(joy_num[i], emulate_dpad[joy_assign[joy_num[i]]]);
						}
					}
					names[i] = QString::fromUtf8(SDL_JoystickNameForIndex(num));
					debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "JoyThread : Joystick %d : %s : is plugged.", num, names[i].toUtf8().data());
					strncpy(p_config->assigned_joystick_name[num], names[num].toUtf8().constData(),
							(sizeof(p_config->assigned_joystick_name[num])  / sizeof(char)) - 1);
					break;
				}
			}
		}
	}

}

void JoyThreadClass::joystick_unplugged(int num)
{
	//int i, j;
	if(num < 0) return;
# if defined(USE_SDL2)
	if(controller_table[num] != NULL) {
		SDL_GameControllerClose(controller_table[num]);
		controller_table[num] = NULL;
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "JoyThread : Controller %d : %s : is removed.", num, names[num].toUtf8().data());
		joy_num[num] = -1;
		is_controller[num] = false;
		joyhandle[num] = NULL;
	} else
# endif
	{
		if(joyhandle[num] != NULL) {
			controller_table[num] = NULL;
			is_controller[joy_num[num]] = false;
			SDL_JoystickClose(joyhandle[num]);
			joyhandle[num] = NULL;
			debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "JoyThread : Joystick %d : %s : is removed.", num, names[num].toUtf8().data());
			joy_num[num] = -1;
		}
	}
	names[num] = QString::fromUtf8("");
	memset(p_config->assigned_joystick_name[num], 0x00, 255);
}

enum {
	JS_AXIS_TYPE_LEFT,
	JS_AXIS_TYPE_RIGHT
};

void JoyThreadClass::x_axis_changed(int idx, int type, int value)
{
	if(p_osd == NULL) return;
	if((idx < 0) || (idx >= 16)) return;
	int true_index = get_joy_num(idx);

	if((true_index < 0) || (true_index >= 4)) return;

	uint32_t *joy_status = (uint32_t *)(p_osd->get_joy_buffer());
	//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "X AXIS Changed #%d/%d, TYPE=%d VAL=%d", idx, true_index, type, value);
	if(joy_status != NULL) {
		switch(type) {
		case JS_AXIS_TYPE_LEFT:
			joy_status[(true_index * 2) + 4] = value + 32768;
			if((emulate_dpad[true_index]) || !(is_controller[idx])) {
				if(value < -8192) { // left
					joy_status[true_index] |= 0x04; joy_status[true_index] &= ~0x08;
				} else if(value > 8192)  { // right
					joy_status[true_index] |= 0x08; joy_status[true_index] &= ~0x04;
				}  else { // center
					joy_status[true_index] &= ~0x0c;
				}
			}
			break;
		case JS_AXIS_TYPE_RIGHT:
			joy_status[(true_index * 2) + 12] = value + 32768;
			break;
		}
	}
	p_osd->release_joy_buffer(joy_status);

}

void JoyThreadClass::y_axis_changed(int idx, int type, int value)
{
	if(p_osd == NULL) return;
	if((idx < 0) || (idx >= 16)) return;
	int true_index = get_joy_num(idx);

	if((true_index < 0) || (true_index >= 4)) return;

	uint32_t *joy_status = p_osd->get_joy_buffer();
	//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "Y AXIS Changed #%d/%d, TYPE=%d VAL=%d", idx, true_index, type, value);
	if(joy_status != NULL) {
		switch(type) {
		case JS_AXIS_TYPE_LEFT:
			joy_status[(true_index * 2) + 4 + 1] = value + 32768;
			if((emulate_dpad[true_index]) || !(is_controller[idx])) {
				if(value < -8192) {// up
					joy_status[true_index] |= 0x01; joy_status[true_index] &= ~0x02;
				} else if(value > 8192)  {// down
					joy_status[true_index] |= 0x02; joy_status[true_index] &= ~0x01;
				} else {
					joy_status[true_index] &= ~0x03;
				}
			}
			break;
		case JS_AXIS_TYPE_RIGHT:
			joy_status[(true_index * 2) + 12 + 1] = value + 32768;
			break;
		}
	}
	p_osd->release_joy_buffer(joy_status);
}

void JoyThreadClass::button_down(int idx, unsigned int button)
{
	if(p_osd == NULL) return;
	if((idx < 0) || (idx >= 16)) return;
	int true_index = get_joy_num(idx);

	if((true_index < 0) || (true_index >= 4)) return;
	if(button >= SDL_CONTROLLER_BUTTON_MAX) return;

	uint32_t *joy_status = p_osd->get_joy_buffer();

	if(joy_status != NULL) {
		switch(button) {
		default:
			if(button < 24) {
				joy_status[true_index] |= (1 << (button + 4));
				//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "BUTTON DOWN #%d/%d, NUM=%d", idx, true_index, button);
			}
			break;
		}
	}
	p_osd->release_joy_buffer(joy_status);

}

void JoyThreadClass::controller_button_down(int idx, unsigned int button)
{
	if(p_osd == NULL) return;
	if((idx < 0) || (idx >= 16)) return;
	int true_index = get_joy_num(idx);

	if((true_index < 0) || (true_index >= 4)) return;
	if(button >= SDL_CONTROLLER_BUTTON_MAX) return;
	uint32_t *joy_status = p_osd->get_joy_buffer();

	if(joy_status != NULL) {
		switch(button) {
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			joy_status[true_index] |= 0x01;
			joy_status[true_index + 20] |= 0x01; // DPAD DIR
			//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "DPAD UP #%d", true_index);
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			joy_status[true_index] |= 0x02;
			joy_status[true_index + 20] |= 0x02; // DPAD DIR
			//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "DPAD DOWN #%d", true_index);
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			joy_status[true_index] |= 0x04;
			joy_status[true_index + 20] |= 0x04; // DPAD DIR
			//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "DPAD LEFT #%d", true_index);
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			joy_status[true_index] |= 0x08;
			joy_status[true_index + 20] |= 0x08; // DPAD DIR
			//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "DPAD RIGHT #%d", true_index);
			break;
		case SDL_CONTROLLER_BUTTON_A:
			joy_status[true_index] |= 0x10;
			break;
		case SDL_CONTROLLER_BUTTON_B:
			joy_status[true_index] |= 0x20;
			break;
		case SDL_CONTROLLER_BUTTON_X:
			joy_status[true_index] |= 0x40;
			break;
		case SDL_CONTROLLER_BUTTON_Y:
			joy_status[true_index] |= 0x80;
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK:
			joy_status[true_index] |= 0x100;
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			joy_status[true_index] |= 0x200;
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			joy_status[true_index] |= 0x400;
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			joy_status[true_index] |= 0x800;
			break;
		case SDL_CONTROLLER_BUTTON_GUIDE:
			joy_status[true_index] |= 0x1000;
			break;
		case SDL_CONTROLLER_BUTTON_START:
			joy_status[true_index] |= 0x2000;
			break;
		default:
			if(button < 24) {
				joy_status[true_index] |= (1 << (button + 4));
				//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "BUTTON DOWN #%d, NUM=%d", true_index, button);
			}
			break;
		}
	}
	p_osd->release_joy_buffer(joy_status);

}

void JoyThreadClass::button_up(int idx, unsigned int button)
{
	if(p_osd == NULL) return;
	if((idx < 0) || (idx >= 16)) return;
	int true_index = get_joy_num(idx);

	if((true_index < 0) || (true_index >= 4)) return;
	if(button >= 16) return;

	uint32_t *joy_status = p_osd->get_joy_buffer();
	if(joy_status != NULL) {
		switch(button) {
		default:
			if(button < 24) {
				joy_status[true_index] &= ~(1 << (button + 4));
			}
			break;
		}
	}
	p_osd->release_joy_buffer(joy_status);

}

void JoyThreadClass::controller_button_up(int idx, unsigned int button)
{
	if(p_osd == NULL) return;
	if((idx < 0) || (idx >= 16)) return;

	int true_index = get_joy_num(idx);

	if((true_index < 0) || (true_index >= 4)) return;
	if(button >= 12) return;

	uint32_t *joy_status = p_osd->get_joy_buffer();
	if(joy_status != NULL) {
		switch(button) {
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			joy_status[true_index] &= ~0x01;
			joy_status[true_index + 20] &= ~0x01; // DPAD DIR
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			joy_status[true_index] &= ~0x02;
			joy_status[true_index + 20] &= ~0x02; // DPAD DIR
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			joy_status[true_index] &= ~0x04;
			joy_status[true_index + 20] &= ~0x04; // DPAD DIR
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			joy_status[true_index] &= ~0x08;
			joy_status[true_index + 20] &= ~0x08; // DPAD DIR
			break;
		case SDL_CONTROLLER_BUTTON_A:
			joy_status[true_index] &= ~0x10;
			break;
		case SDL_CONTROLLER_BUTTON_B:
			joy_status[true_index] &= ~0x20;
			break;
		case SDL_CONTROLLER_BUTTON_X:
			joy_status[true_index] &= ~0x40;
			break;
		case SDL_CONTROLLER_BUTTON_Y:
			joy_status[true_index] &= ~0x80;
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK:
			joy_status[true_index] &= ~0x100;
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			joy_status[true_index] &= ~0x200;
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			joy_status[true_index] &= ~0x400;
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			joy_status[true_index] &= ~0x800;
			break;
		case SDL_CONTROLLER_BUTTON_GUIDE:
			joy_status[true_index] &= ~0x1000;
			break;
		case SDL_CONTROLLER_BUTTON_START:
			joy_status[true_index] &= ~0x2000;
			break;
		default:
			if(button < 24) {
				joy_status[true_index] &= ~(1 << (button + 4));
			}
			break;
		}
	}
	p_osd->release_joy_buffer(joy_status);
}

#if defined(USE_SDL2)
int JoyThreadClass::get_joyid_from_instanceID(SDL_JoystickID id)
{
	int i;
	SDL_Joystick *js;
	for(i = 0; i < 16; i++) {
		if(controller_table[i] == NULL) continue;
		js = SDL_GameControllerGetJoystick(controller_table[i]);
		if(js == NULL) continue;
		if(id == SDL_JoystickInstanceID(js)) return i;
	}
	return -1;
}
#endif
int JoyThreadClass::get_joy_num(int id)
{
	int i;
	for(i = 0; i < 16; i++) {
		if(joy_assign[i] == id) return i;
	}
	return -1;
}

bool  JoyThreadClass::EventSDL(SDL_Event *eventQueue)
{
	//	SDL_Surface *p;
	Sint16 value;
	unsigned int button;
	//int vk;
	//uint32_t sym;
	//uint32_t mod;
# if defined(USE_SDL2)
	SDL_JoystickID id;
	uint8_t cont_button;
# endif
	//int i, j;
	int i;
	if(eventQueue == NULL) return false;
	/*
	 * JoyStickなどはSDLが管理する
	 */
	switch (eventQueue->type){
# if defined(USE_SDL2)
		case SDL_CONTROLLERAXISMOTION:
			value = eventQueue->caxis.value;
			if(eventQueue->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
				id = (int)eventQueue->caxis.which;
				i = get_joyid_from_instanceID(id);
				x_axis_changed(i, JS_AXIS_TYPE_LEFT, value);
			} else if(eventQueue->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
				id = (int)eventQueue->caxis.which;
				i = get_joyid_from_instanceID(id);
				y_axis_changed(i, JS_AXIS_TYPE_LEFT, value);
			} else if(eventQueue->caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
				id = (int)eventQueue->caxis.which;
				i = get_joyid_from_instanceID(id);
				x_axis_changed(i, JS_AXIS_TYPE_RIGHT, value);
			} else if(eventQueue->caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
				id = (int)eventQueue->caxis.which;
				i = get_joyid_from_instanceID(id);
				y_axis_changed(i, JS_AXIS_TYPE_RIGHT, value);
			}
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			cont_button = eventQueue->cbutton.button;
			id = eventQueue->cbutton.which;
			i = get_joyid_from_instanceID(id);
			//button = SDL_GameControllerGetButton(controller_table[i], cont_button);
			//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "Press button: %d", cont_button);
			controller_button_down(i, cont_button);
			break;
		case SDL_CONTROLLERBUTTONUP:
			cont_button = eventQueue->cbutton.button;
			id = eventQueue->cbutton.which;
			i = get_joyid_from_instanceID(id);
			controller_button_up(i, cont_button);
			break;
		case SDL_CONTROLLERDEVICEADDED:
			i = eventQueue->cdevice.which;
			joystick_plugged(i);
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			i = eventQueue->cdevice.which;
//			i = get_joy_num(i);
			joystick_unplugged(i);
			break;
# endif
		case SDL_JOYAXISMOTION:
			value = eventQueue->jaxis.value;
			i = eventQueue->jaxis.which;
//			i = get_joy_num(i);
			if(eventQueue->jaxis.axis == 0) { // X
				x_axis_changed(i, JS_AXIS_TYPE_LEFT, value);
			} else if(eventQueue->jaxis.axis == 1) { // Y
				y_axis_changed(i, JS_AXIS_TYPE_LEFT, value);
			} else if(eventQueue->jaxis.axis == 2) { // X
				x_axis_changed(i, JS_AXIS_TYPE_RIGHT, value);
			} else if(eventQueue->jaxis.axis == 3) { // Y
				y_axis_changed(i, JS_AXIS_TYPE_RIGHT, value);
			}
			//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "AXIS_CHANGED NUM=%d, AXIS=%d VAL=%d", i, eventQueue->jaxis.axis, value);
			break;
		case SDL_JOYBUTTONDOWN:
			button = eventQueue->jbutton.button;
			i = eventQueue->jbutton.which;
//			i = get_joy_num(i);
			button_down(i, button);
			break;
		case SDL_JOYBUTTONUP:
			button = eventQueue->jbutton.button;
			i = eventQueue->jbutton.which;
//			i = get_joy_num(i);
			button_up(i, button);
			break;
		case SDL_JOYDEVICEADDED:
			i = eventQueue->jdevice.which;
			joystick_plugged(i);
			break;
		case SDL_JOYDEVICEREMOVED:
			i = eventQueue->jdevice.which;
//			i = get_joy_num(i);
			joystick_unplugged(i);
			break;
		default:
			break;
	}
	return true;
}

void JoyThreadClass::doWork(const QString &params)
{
	if(using_flags->is_use_joystick()) {
		do {
			if(bRunThread == false) {
				break;
			}
			while(SDL_PollEvent(&event)) {
				EventSDL(&event);
			}
			msleep(10);
		} while(1);
	}
	this->quit();
}


void JoyThreadClass::doExit(void)
{
	bRunThread = false;
	//this->quit();
}

void JoyThreadClass::do_map_joy_num(int num, int assign)
{
	if((num < 0) || (num >= 16)) return;
	if(assign >= 16) return;
	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_JOYSTICK, "ASSIGN %d to %d", assign - 1, num);
	joy_assign[num] = assign - 1;
	p_config->assigned_joystick_num[num] = assign - 1;
}

void JoyThreadClass::do_set_emulate_dpad(int num, bool val)
{
	if((num < 0) || (num >= 4)) return;
	emulate_dpad[num] = val;
	p_config->emulated_joystick_dpad[num] = val;
}
