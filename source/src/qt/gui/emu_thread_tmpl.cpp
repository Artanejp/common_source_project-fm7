/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2015.11.10 Split from qt_main.cpp
	Note: This class must be compiled per VM, must not integrate shared units.
	[ win32 main ] -> [ Qt main ] -> [Emu Thread] -> [Independed from VMs]
*/

#include <QString>
#include <QTextCodec>
#include <QWaitCondition>

#include <SDL.h>

#include "emu_thread_tmpl.h"

#include "qt_gldraw.h"
#include "../../romakana.h"

//#include "csp_logger.h"
#include "../common/menu_flags.h"
const romakana_convert_t romakana_table_1[] = {
	{'1', KANA_NA + 2, false}, // NU
	{'2', KANA_HA + 2, false}, // HU
	{'3', KANA_A  + 0, false}, // A
	{'4', KANA_A  + 2, false}, // U
	{'5', KANA_A  + 3, false}, // E
	{'6', KANA_A  + 4, false}, // O
	{'7', KANA_YA + 0, false}, // YA
	{'8', KANA_YA + 1, false}, // YU
	{'9', KANA_YA + 2, false}, // YO
	{'0', KANA_WA + 0, false}, // WA
	{'-', KANA_HA + 4, false}, // HO
	{'^', KANA_HA + 3, false}, // HE
	{VK_OEM_5, KANA_ONBIKI, false}, // -

	{'3', KANA_SMALL_A,      true}, // A
	{'4', KANA_SMALL_U,      true}, // U
	{'5', KANA_SMALL_E,      true}, // E
	{'6', KANA_SMALL_O,      true}, // O
	{'7', KANA_SMALL_YA + 0, true}, // YA
	{'8', KANA_SMALL_YA + 1, true}, // YU
	{'9', KANA_SMALL_YA + 2, true}, // YO
	{'0', KANA_WO,           true}, // WO

	{'Q', KANA_TA + 0, false}, // TA
	{'W', KANA_TA + 3, false}, // TE
	{'E', KANA_A  + 1, false}, // I
	{'R', KANA_SA + 2, false}, // SU
	{'T', KANA_KA + 0, false}, // KA
	{'Y', KANA_NN    , false}, // NN
	{'U', KANA_NA + 0, false}, // NA
	{'I', KANA_NA + 1, false}, // NI
	{'O', KANA_RA + 0, false}, // RA
	{'P', KANA_SA + 3, false}, // SE
	{'Q', KANA_TA + 0, false}, // TA
	{VK_OEM_3, KANA_DAKUON, false}, // DAKUTEN
	{VK_OEM_4, KANA_HANDAKUON, false}, // HANDAKUTEN
	{VK_OEM_4, KANA_UPPER_KAKKO, true}, // [
	
	{'A', KANA_TA + 1, false}, // TI
	{'S', KANA_TA + 4, false}, // TO
	{'D', KANA_SA + 1, false}, // SI
	{'F', KANA_HA + 0, false}, // HA
	{'G', KANA_KA + 1, false}, // KI
	{'H', KANA_KA + 2, false}, // KU
	{'J', KANA_MA + 0, false}, // MA
	{'K', KANA_NA + 4, false}, // NO
	{'L', KANA_RA + 1, false}, // RI
	{VK_OEM_PLUS, KANA_RA + 3, false}, // RE
	{VK_OEM_1, KANA_KA + 3, false}, // KE
	{VK_OEM_6, KANA_MA + 2, false}, // MU
	{VK_OEM_6, KANA_DOWNER_KAKKO, true}, // ]

	{'Z', KANA_TA + 2, false}, // TU
	{'X', KANA_SA + 0, false}, // SA
	{'C', KANA_SA + 4, false}, // SO
	{'V', KANA_HA + 1, false}, // HI
	{'B', KANA_KA + 4, false}, // KO
	{'N', KANA_MA + 1, false}, // MI
	{'M', KANA_MA + 4, false}, // MO
	{VK_OEM_COMMA,  KANA_NA + 3, false}, // NE
	{VK_OEM_PERIOD, KANA_RA + 2, false}, // RU
	{VK_OEM_2,   KANA_MA + 3, false}, // ME
	{VK_OEM_102, KANA_RA + 4, false}, // RO

	{VK_OEM_COMMA,  KANA_COMMA, true}, // 
	{VK_OEM_PERIOD, KANA_MARU,  true}, // 
	{VK_OEM_2,   KANA_NAKAGURO, true}, // 
	//{VK_OEM_102, KANA_RA + 4, false}, // RO
	{0xffff, 0xffffffff, false}
};

EmuThreadClassBase::EmuThreadClassBase(META_MainWindow *rootWindow, EMU *pp_emu, USING_FLAGS *p, QObject *parent) : QThread(parent) {
	MainWindow = rootWindow;
	p_emu = pp_emu;
	using_flags = p;
	p_config = p->get_config_ptr();
	
	bRunThread = true;
	prev_skip = false;
	tick_timer.start();
	update_fps_time = tick_timer.elapsed();
	next_time = update_fps_time;
	total_frames = 0;
	draw_frames = 0;
	skip_frames = 0;
	calc_message = true;
	mouse_flag = false;

	drawCond = new QWaitCondition();
	mouse_x = 0;
	mouse_y = 0;
	if(using_flags->is_use_tape() && !using_flags->is_tape_binary_only()) {
		tape_play_flag = false;
		tape_rec_flag = false;
		tape_pos = 0;
	}

	if(using_flags->get_use_sound_volume() > 0) {
		for(int i = 0; i < using_flags->get_use_sound_volume(); i++) {
			bUpdateVolumeReq[i] = true;
			volume_avg[i] = (using_flags->get_config_ptr()->sound_volume_l[i] +
							 using_flags->get_config_ptr()->sound_volume_r[i]) / 2;
			volume_balance[i] = (using_flags->get_config_ptr()->sound_volume_r[i] -
								 using_flags->get_config_ptr()->sound_volume_l[i]) / 2;
		}
	}
	memset(roma_kana_buffer, 0x00, sizeof(roma_kana_buffer));
	roma_kana_ptr = 0;
	roma_kana_down_queue.clear();
	roma_kana_up_queue.clear();
	roma_kana_conv = false;
};

EmuThreadClassBase::~EmuThreadClassBase() {
	delete drawCond;
};

void EmuThreadClassBase::calc_volume_from_balance(int num, int balance)
{
	int level = volume_avg[num];
	int right;
	int left;
	volume_balance[num] = balance;
	right = level + balance;
	left  = level - balance;
	using_flags->get_config_ptr()->sound_volume_l[num] = left;	
	using_flags->get_config_ptr()->sound_volume_r[num] = right;
}

void EmuThreadClassBase::calc_volume_from_level(int num, int level)
{
	int balance = volume_balance[num];
	int right,left;
	volume_avg[num] = level;
	right = level + balance;
	left  = level - balance;
	using_flags->get_config_ptr()->sound_volume_l[num] = left;	
	using_flags->get_config_ptr()->sound_volume_r[num] = right;
}

void EmuThreadClassBase::doExit(void)
{
	int status;
	bRunThread = false;
}

void EmuThreadClassBase::button_pressed_mouse(Qt::MouseButton button)
{
	if(using_flags->is_use_mouse()) {
		button_pressed_mouse_sub(button);
	} else {		
		if(using_flags->get_max_button() > 0) {
			button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
			if(vm_buttons_d == NULL) return;
			switch(button) {
			case Qt::LeftButton:
			case Qt::RightButton:
				for(int i = 0; i < using_flags->get_max_button(); i++) {
					if((mouse_x >= vm_buttons_d[i].x) &&
					   (mouse_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((mouse_y >= vm_buttons_d[i].y) &&
						   (mouse_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
							if(vm_buttons_d[i].code != 0x00) {
								key_queue_t sp;
								sp.code = vm_buttons_d[i].code;
								sp.mod = key_mod;
								sp.repeat = false;
								key_down_queue.enqueue(sp);
							} else {
								bResetReq = true;
							}
						}
					}
				}
				break;
			}
		}
	}
}

void EmuThreadClassBase::button_released_mouse(Qt::MouseButton button)
{
	if(using_flags->is_use_mouse()) {
	} else {
		if(using_flags->get_max_button() > 0) {
			button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
			if(vm_buttons_d == NULL) return;
			
			switch(button) {
			case Qt::LeftButton:
			case Qt::RightButton:
				for(int i = 0; i < using_flags->get_max_button(); i++) {
					if((mouse_x >= vm_buttons_d[i].x) &&
					   (mouse_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((mouse_y >= vm_buttons_d[i].y) &&
						   (mouse_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
							if(vm_buttons_d[i].code != 0x00) {
								key_queue_t sp;
								sp.code = vm_buttons_d[i].code;
								sp.mod = key_mod;
								sp.repeat = false;
								key_up_queue.enqueue(sp);
							}
						}
					}
				}
				break;
			}
		}
	}
}


void EmuThreadClassBase::do_key_down(uint32_t vk, uint32_t mod, bool repeat)
{
	key_queue_t sp;
	sp.code = vk;
	sp.mod = mod;
	sp.repeat = repeat;
	//key_changed = true;
#if 1
	if(using_flags->is_use_roma_kana_conversion()) {
		if(p_config->roma_kana_conversion) {
			if(roma_kana_ptr < (sizeof(roma_kana_buffer) / sizeof(_TCHAR)) &&
			   (((vk >= 'A') && (vk <= 'z')) ||
				(vk == VK_OEM_4) || (vk == VK_OEM_6) ||
				(vk == VK_OEM_2) || (vk == VK_OEM_COMMA) ||
				(vk == VK_OEM_PERIOD) || (vk == VK_OEM_MINUS))) {
				return;
			} else {
				if(roma_kana_ptr > (sizeof(roma_kana_buffer) / sizeof(_TCHAR))) roma_kana_ptr = sizeof(roma_kana_buffer) / sizeof(_TCHAR);
				key_queue_t ssp;
				for(int i = 0; i < roma_kana_ptr; i++) {
					ssp.code = roma_kana_shadow[i];
					ssp.mod = mod;
					key_down_queue.enqueue(ssp);
					key_up_queue.enqueue(ssp);
				}
				ssp.code = VK_SHIFT;
				ssp.mod = mod & ~(Qt::ShiftModifier);
				ssp.repeat = false;
				//key_down_queue.enqueue(ssp);
				key_up_queue.enqueue(ssp);
				memset(roma_kana_shadow, 0x00, sizeof(roma_kana_shadow));
				memset(roma_kana_buffer, 0x00, sizeof(roma_kana_buffer));
				roma_kana_ptr = 0;
				//roma_kana_updown = false;
				//roma_kana_conv = false;
			}
		}
	}
#endif
	key_down_queue.enqueue(sp);
	key_mod = mod;
}

void EmuThreadClassBase::do_key_up(uint32_t vk, uint32_t mod)
{
	key_queue_t sp;
	sp.code = vk;
	sp.mod = mod;
	sp.repeat = false;
#if 1
	if(using_flags->is_use_roma_kana_conversion()) {
		if(p_config->roma_kana_conversion) {
			if(roma_kana_ptr < (sizeof(roma_kana_buffer) / sizeof(_TCHAR)) &&
			   (((vk >= 'A') && (vk <= 'z')) ||
				(vk == VK_OEM_4) || (vk == VK_OEM_6) ||
				(vk == VK_OEM_2) || (vk == VK_OEM_COMMA) ||
				(vk == VK_OEM_PERIOD) || (vk == VK_OEM_MINUS))) {
				   _TCHAR kana_buffer[6];
				   int kana_len = 6;
				   memset(kana_buffer, 0x00, sizeof(kana_buffer));
				   uint32_t vvk = vk;
				   switch(vk) {
				   case VK_OEM_MINUS:
					   vvk = '-';
					   break;
				   case VK_OEM_4:
					   vvk = '[';
					   break;
				   case VK_OEM_6:
					   vvk = ']';
					   break;
				   case VK_OEM_COMMA:
					   vvk = ',';
					   break;
				   case VK_OEM_PERIOD:
					   vvk = '.';
					   break;
				   case VK_OEM_2:
					   vvk = '/';
					   break;
				   }
				   roma_kana_shadow[roma_kana_ptr] = vk;
				   roma_kana_buffer[roma_kana_ptr++] = (_TCHAR)vvk;
				   if(alphabet_to_kana((const _TCHAR *)roma_kana_buffer, kana_buffer, &kana_len) > 0) {
					   key_queue_t ssp;
					   ssp.code = 0;
					   ssp.mod = 0;
					   ssp.repeat = false;
					   bool before_shift = false;
					   for(int i = 0; i < kana_len; i++) {
						   int j = 0;
						   do {
							   if(romakana_table_1[j].vk == 0xffff) break;
							   if((_TCHAR)romakana_table_1[j].code == kana_buffer[i]) {
								   uint32_t vvvk = romakana_table_1[j].vk;
								   switch(romakana_table_1[j].vk) {
								   case '-':
									   vvvk = VK_OEM_MINUS;
									   break;
								   case '^':
									   vvvk = VK_OEM_7;
									   break;
								   case '@':
									   vvvk = VK_OEM_3;
									   break;
								   case '[':
									   vvvk = VK_OEM_4;
									   break;
								   case ';':
									   vvvk = VK_OEM_PLUS;
									   break;
								   case ':':
									   vvvk = VK_OEM_1;
									   break;
								   case ']':
									   vvvk = VK_OEM_6;
									   break;
								   case ',':
									   vvvk = VK_OEM_COMMA;
									   break;
								   case '.':
									   vvvk = VK_OEM_PERIOD;
									   break;
								   case '/':
									   vvvk = VK_OEM_2;
									   break;
								   }
								   ssp.code = vvvk;
								   ssp.mod = mod;
								   ssp.repeat = false;
#if 1
								   if(romakana_table_1[j].shift) {
									   key_queue_t sss;
									   sss.code = VK_SHIFT;
									   ssp.mod = ssp.mod | Qt::ShiftModifier;
									   sss.mod = ssp.mod;
									   sss.repeat = false;
									   roma_kana_down_queue.enqueue(sss);
								   }
#endif
								   roma_kana_down_queue.enqueue(ssp);
								   roma_kana_up_queue.enqueue(ssp);
								   if(romakana_table_1[j].shift) {
									   key_queue_t sss;
									   sss.code = VK_SHIFT;
									   ssp.mod = ssp.mod & ~(Qt::ShiftModifier);
									   sss.mod = ssp.mod;
									   sss.repeat = false;
									   roma_kana_up_queue.enqueue(sss);
								   }
								   break;
							   }
							   j++;
						   } while(1);
					   }
					   if(kana_len > 0) {
						   roma_kana_ptr = 0;
						   ssp.code = VK_SHIFT;
						   ssp.mod = mod & ~(Qt::ShiftModifier);
						   ssp.repeat = false;
						   roma_kana_down_queue.enqueue(ssp);
						   roma_kana_up_queue.enqueue(ssp);
						   memset(roma_kana_buffer, 0x00, sizeof(roma_kana_buffer));
						   memset(roma_kana_shadow, 0x00, sizeof(roma_kana_shadow));
						   roma_kana_updown = false;
						   roma_kana_conv = true;
					   }
				   }
				   return;
			} else {
			
			}
		}
	}
#endif
	key_up_queue.enqueue(sp);
	key_mod = mod;
}

void EmuThreadClassBase::set_tape_play(bool flag)
{
	tape_play_flag = flag;
}

void EmuThreadClassBase::resize_screen(int screen_width, int screen_height, int stretched_width, int stretched_height)
{
	emit sig_resize_screen(screen_width, screen_height);
}

void EmuThreadClassBase::do_draw_timing(bool f)
{
//	draw_timing = f;
}

void EmuThreadClassBase::sample_access_drv(void)
{
	if(using_flags->is_use_qd()) get_qd_string();
	if(using_flags->is_use_fd()) get_fd_string();
	if(using_flags->is_use_tape() && !using_flags->is_tape_binary_only()) get_tape_string();
	if(using_flags->is_use_compact_disc()) get_cd_string();
	if(using_flags->is_use_bubble()) get_bubble_string();
}

void EmuThreadClassBase::doUpdateConfig()
{
	bUpdateConfigReq = true;
}

void EmuThreadClassBase::doStartRecordSound()
{
	bStartRecordSoundReq = true;
}

void EmuThreadClassBase::doStopRecordSound()
{
	bStopRecordSoundReq = true;
}

void EmuThreadClassBase::doReset()
{
	bResetReq = true;
}

void EmuThreadClassBase::doSpecialReset()
{
	bSpecialResetReq = true;
}

void EmuThreadClassBase::doLoadState()
{
	bLoadStateReq = true;
}

void EmuThreadClassBase::doSaveState()
{
	bSaveStateReq = true;
}

void EmuThreadClassBase::doStartRecordVideo(int fps)
{
	record_fps = fps;
	bStartRecordMovieReq = true;
}

void EmuThreadClassBase::doStopRecordVideo()
{
	bStartRecordMovieReq = false;
}

void EmuThreadClassBase::doUpdateVolumeLevel(int num, int level)
{
	if(using_flags->get_use_sound_volume() > 0) {
		if((num < using_flags->get_use_sound_volume()) && (num >= 0)) {
			calc_volume_from_level(num, level);
			bUpdateVolumeReq[num] = true;
		}
	}
}

void EmuThreadClassBase::doUpdateVolumeBalance(int num, int level)
{
	if(using_flags->get_use_sound_volume() > 0) {
		if((num < using_flags->get_use_sound_volume()) && (num >= 0)) {
			calc_volume_from_balance(num, level);
			bUpdateVolumeReq[num] = true;
		}
	}
}
