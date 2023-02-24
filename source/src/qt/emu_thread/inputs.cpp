/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2023.02.24 Split from emu_thread_tmpl.cpp
	Note: This class must be compiled per VM, must not integrate shared units.
	[ win32 main ] -> [ Qt main ] -> [Emu Thread] -> [Input around KEYBOARD/MOUSE// ]
// */

// #include <QString>
// #include <QWidget>
// #include <QMouseEvent>
#include <QTextCodec>

#include "config.h"
#include "emu_thread_tmpl.h"
#include "mainwidget_base.h"
#include "common.h"
#include "osd_base.h"
#include "../../osdcall_types.h"

#include "virtualfileslist.h"
#include "menu_metaclass.h"

#include "menu_flags.h"


void EmuThreadClassBase::do_move_mouse(double x, double y, double globalx, double globaly)
{
	if(p_osd == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_one_board_computer() || (p->get_max_button() > 0)) {
		mouse_x = x;
		mouse_y = y;
//		bool flag = p_osd->is_mouse_enabled();
//		if(!flag) return;
//		printf("Mouse Moved: %g, %g\n", x, y);
//		p_osd->set_mouse_pointer(floor(x), floor(y));
	} else if(p->is_use_mouse()) {
//		double factor = (double)(p_config->mouse_sensitivity & ((1 << 16) - 1));
//		mouse_x = (int)(floor((globalx * factor) / 8192.0));
//		mouse_y = (int)(floor((globaly * factor) / 8192.0));
		mouse_x = globalx;
		mouse_y = globaly;
		//printf("Moved Mouse %d, %d\n", x, y);
		bool flag = p_osd->is_mouse_enabled();
		if(!flag) return;
		//printf("Mouse Moved: %d, %d\n", x, y);
		p_osd->set_mouse_pointer(mouse_x, mouse_y);
	}
}

void EmuThreadClassBase::button_pressed_mouse_sub(Qt::MouseButton button)
{

	if(p_osd == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_one_board_computer() || p->is_use_mouse() || (p->get_max_button() > 0)) {
		int stat = p_osd->get_mouse_button();
		bool flag = (p_osd->is_mouse_enabled() || p->is_use_one_board_computer() || (p->get_max_button() > 0));
		switch(button) {
		case Qt::LeftButton:
			stat |= 0x01;
			break;
		case Qt::RightButton:
			stat |= 0x02;
			break;
		case Qt::MiddleButton:
			flag = !flag;
			emit sig_mouse_enable(flag);
			return;
			break;
		default:
			break;
		}
		if(!flag) return;
		p_osd->set_mouse_button(stat);
	}
}

void EmuThreadClassBase::button_released_mouse_sub(Qt::MouseButton button)
{

	if(p_osd == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_one_board_computer() || p->is_use_mouse() || (p->get_max_button() > 0)) {
		int stat = p_osd->get_mouse_button();
		switch(button) {
		case Qt::LeftButton:
			stat &= 0x7ffffffe;
			break;
		case Qt::RightButton:
			stat &= 0x7ffffffd;
			break;
		case Qt::MiddleButton:
			//emit sig_mouse_enable(false);
			break;
		default:
			break;
		}
		p_osd->set_mouse_button(stat);
	}
}

void EmuThreadClassBase::do_press_button_mouse(Qt::MouseButton button)
{
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() == nullptr) return;
	if(up->is_use_mouse()) {
		button_pressed_mouse_sub(button);
	} else {
		if(up->get_max_button() > 0) {
			button_desc_t *vm_buttons_d = up->get_vm_buttons();
			if(vm_buttons_d == NULL) return;
			int _x = (int)rint(mouse_x);
			int _y = (int)rint(mouse_y);
			switch(button) {
			case Qt::LeftButton:
//			case Qt::RightButton:
				for(int i = 0; i < up->get_max_button(); i++) {
					if((_x >= vm_buttons_d[i].x) &&
					   (_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((_y >= vm_buttons_d[i].y) &&
						   (_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
							if(vm_buttons_d[i].code != 0x00) {
								key_queue_t sp;
								sp.code = vm_buttons_d[i].code;
								sp.mod = key_mod;
								sp.repeat = false;
								enqueue_key_down(sp);
							} else {
								bResetReq = true;
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

void EmuThreadClassBase::do_release_button_mouse(Qt::MouseButton button)
{
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() == nullptr) return;
	if(up->is_use_mouse()) {
		button_released_mouse_sub(button);
	} else {
		if(up->get_max_button() > 0) {
			button_desc_t *vm_buttons_d = up->get_vm_buttons();
			if(vm_buttons_d == NULL) return;
			int _x = (int)rint(mouse_x);
			int _y = (int)rint(mouse_y);
			switch(button) {
			case Qt::LeftButton:
//			case Qt::RightButton:
				for(int i = 0; i < up->get_max_button(); i++) {
					if((_x >= vm_buttons_d[i].x) &&
					   (_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((_y >= vm_buttons_d[i].y) &&
						   (_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
							if(vm_buttons_d[i].code != 0x00) {
								key_queue_t sp;
								sp.code = vm_buttons_d[i].code;
								sp.mod = key_mod;
								sp.repeat = false;
								enqueue_key_up(sp);
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

// New UI
void EmuThreadClassBase::do_key_down(uint32_t vk, uint32_t mod, bool repeat)
{
	key_queue_t sp;
	sp.code = vk;
	sp.mod = mod;
	sp.repeat = repeat;
	//key_changed = true;
	enqueue_key_down(sp);
	key_mod = mod;
}

void EmuThreadClassBase::do_key_up(uint32_t vk, uint32_t mod)
{
	key_queue_t sp;
	sp.code = vk;
	sp.mod = mod;
	sp.repeat = false;
	enqueue_key_up(sp);
	key_mod = mod;
}

void EmuThreadClassBase::do_start_auto_key(QString ctext)
{
	//QMutexLocker _locker(&uiMutex);
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_auto_key()) {
		QTextCodec *codec = QTextCodec::codecForName("Shift-Jis");
		QByteArray array;
		QVector<uint> ucs4_src = ctext.toUcs4();
		QString dst;
		dst.clear();
		uint32_t pool[8] = {0};
		for(auto itr = ucs4_src.constBegin(); itr != ucs4_src.constEnd(); ++itr) {
			uint val = (*itr);
			int chrs = ucs4_kana_zenkaku_to_hankaku((const uint32_t)val, pool, sizeof(pool) / sizeof(uint32_t));
			if(chrs > 0) {
		#if QT_VERSION >= 0x060000
				dst.append(QString::fromUcs4((char32_t*)pool, chrs));
		#else
				dst.append(QString::fromUcs4((uint*)pool, chrs));
		#endif
			}
		}
		clipBoardText = dst;
		//printf("%s\n", clipBoardText.toLocal8Bit().constData());
		array = codec->fromUnicode(clipBoardText);
		//printf("Array is:");
		//for(int l = 0; l < array.size(); l++) {
		//	printf("%02X ", array.at(l));
		//}
		//printf("\n");
		if(clipBoardText.size() > 0) {
			int size = array.size();
			const char *buf = (char *)(array.constData());
			p_emu->stop_auto_key();
			p_emu->set_auto_key_list((char *)buf, size);
			p_emu->start_auto_key();
		}
	}

}

void EmuThreadClassBase::do_stop_auto_key(void)
{
	//QMutexLocker _locker(&uiMutex);
	//csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL,
	//					  "AutoKey: stop\n");
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_auto_key()) {
		p_emu->stop_auto_key();
	}
}
