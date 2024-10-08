#include "common.h"
#include "menu_flags.h"
#include "osd_base.h"
#include <QSettings>

USING_FLAGS::USING_FLAGS(config_t *cfg, QSettings* set) :
	override_sound_frequency_48000hz(48000),
	p_osd(nullptr),
	p_emu(nullptr)
   
{
	p_settings = set;

	use_alt_f10_key = false;
	use_auto_key = use_auto_key_us = use_auto_key_caps = false;
	use_auto_key_no_caps = use_auto_key_release =
	use_auto_key_shift = use_binary_file = false;

	max_binary = 0;
	use_bitmap = false;
	use_boot_mode = 0;

	use_bubble = false;
	max_bubble =  max_b77_banks = 0;

	use_cart = false;
	max_cart = 0;

	use_compact_disc = false;
	max_compact_disc = 0;

	use_fd = false;
	max_drive = max_d88_banks = 0;
	floppy_type_bit = 0x00000000;
	use_drive_type = 0;

	use_hd = false;
	max_hd = 0;

	use_laser_disc = false;
	max_laser_disc = 0;

	use_qd = false;
	max_qd = 0;

	use_tape = use_tape_baud = use_tape_button = use_tape_ptr = false;
	max_tape = 0;

	base_binary_num = 1;
	base_bubble_num = 1;
	base_cart_num = 1;
	base_cd_num = 1;
	base_fd_num = 1;
	base_hd_num = 1;
	base_ld_num = 1;
	base_qd_num = 1;

	use_cpu_type = 0;

	use_debugger = false;
	use_device_type = 0;
	use_mouse_type = -1;
	use_joystick_type = -1;
	use_keyboard_type = -1;
	use_dipswitch = false;
	use_machine_features = 0;

	max_draw_ranges = 0;

	use_ram_size = false;
	max_ram_size = 1;
	min_ram_size = 0;
	ram_size_order = 1024 * 1024;

	use_joystick = use_joy_button_captions = false;
	num_joy_button_captions = 0;

	use_key_locked = false;
	use_led_devices = 0;
	independent_caps_kana_led = false;

	use_movie_player = false;
	use_video_capture = false;	
	use_notify_power_off = false;

	use_one_board_computer = false;

	use_serial = false;
	use_serial_type = 0;

	use_scanline = use_scanline_auto = use_screen_rotate = false;
	screen_mode_num = 1;
	custom_screen_zoom_factor = 0.0;

	special_reset_num = 0;

	use_vm_auto_key_table = false;
	support_tv_render = false;

	use_alt_f10_key = false;
	use_auto_key = use_auto_key_us = use_auto_key_caps = false;
	use_auto_key_no_caps = use_auto_key_release =
	use_auto_key_shift = use_binary_file = false;
	dont_keeep_key_pressed = false;

	max_binary = 0;
	base_binary_num = 0;

	use_bitmap = false;
	use_boot_mode = 0;

	use_bubble = false;
	max_bubble =  max_b77_banks = 0;

	use_cart = false;
	max_cart = 0;

	base_cart_num = 0;
	base_fd_num = 1;
	base_qd_num = 1;
	base_hd_num = 1;
	base_cd_num = 1;
	base_ld_num = 1;
	base_binary_num = 1;
	base_bubble_num = 1;

	use_cpu_type = 0;

	use_compact_disc = use_debugger = false;
	max_compact_disc = 0;
	base_cd_num = 0;

	use_device_type = 0;
	use_dipswitch = false;

	use_drive_type = 0;

	use_fd = false;
	base_fd_num = 1;
	max_drive = max_d88_banks = 0;

	max_draw_ranges = 0;

	use_hd = false;
	max_hd = 0;
	base_hd_num = 1;

	use_joystick = use_joy_button_captions = false;
	num_joy_button_captions = 0;

	use_laser_disc = false;
	max_laser_disc = 0;
	base_ld_num = 0;

	max_memcard = 0;
	use_minimum_rendering = use_dig_resolution = false;
	use_monitor_type = 0;
	use_mouse = false;
	
	use_printer = false;
	use_printer_type = 0;
	
	use_socket = false;

	use_qd = false;
	max_qd = 0;
	base_qd_num = 1;

	use_sound_device_type = 0;
	use_sound_volume = 0;
	without_sound = false;
	use_sound_files_fdd = false;
	use_sound_files_relay = false;

	use_special_reset = false;
	use_state = false;

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 400
	real_screen_width  = SCREEN_WIDTH;
	real_screen_height = SCREEN_HEIGHT;

	screen_width = SCREEN_WIDTH;
	screen_height = SCREEN_HEIGHT;

	screen_x_zoom = 1.0f;
	screen_y_zoom = 1.0f;
#define _WINDOW_WIDTH_ASPECT 640
#define _WINDOW_HEIGHT_ASPECT 480
	screen_width_aspect = _WINDOW_WIDTH_ASPECT;
	screen_height_aspect = _WINDOW_HEIGHT_ASPECT;

	max_button = 0;
	vm_buttons_d = nullptr;
	max_ranges = 0;
	vm_ranges_d = nullptr;

	use_vertical_pixel_lines = false;
	tape_binary_only = false;

	device_name = QString::fromUtf8("");
	config_name = QString::fromUtf8("");

	machine_pasopia_variants = false;
	machine_basicmaster_variants = false;
	machine_tk80_series  = false;
	machine_cmt_mz_series  = false;
	machine_pc6001_variants  = false;
	machine_pc8001_variants = false;
	machine_pc8801_variants = false;
	machine_pc8801sr_variants = false;

	machine_mz80a_variants = false;
	machine_mz80b_variants = false;
	machine_mz2500 = false;
	machine_x1_series = false;
	machine_fm7_series = false;
	machine_gamegear = false;
	machine_mastersystem = false;
	machine_has_pcengine = false;
	machine_sc3000 = false;
	machine_z80tvgame = false;
	p_config = cfg;
}

USING_FLAGS::~USING_FLAGS()
{
	if(p_settings != nullptr) {
		p_settings->sync();
	}
}

const _TCHAR *USING_FLAGS::get_joy_button_captions(int num)
{
	return "";
}

const _TCHAR *USING_FLAGS::get_sound_device_caption(int num)
{
	return "";
}


void USING_FLAGS::set_osd(OSD_BASE *p)
{
	p_osd = p;
}

OSD_BASE *USING_FLAGS::get_osd(void)
{
	return p_osd;
}

config_t *USING_FLAGS::get_config_ptr(void)
{
	return p_config;
}


const int USING_FLAGS::get_vm_node_size(void)
{
	if(p_osd == NULL) return 0;
	return p_osd->get_vm_node_size();
}

void USING_FLAGS::set_vm_node_name(int id, const _TCHAR *name)
{
	if(p_osd == NULL) return;
	p_osd->set_vm_node(id, name);
}

const _TCHAR *USING_FLAGS::get_vm_node_name(int id)
{
	if(p_osd == nullptr) return _T("NODE");

	return (const _TCHAR *)(p_osd->get_vm_node_name(id));
}

void USING_FLAGS::set_emu(EMU_TEMPLATE *p)
{
	p_emu = p;
}

EMU_TEMPLATE *USING_FLAGS::get_emu(void)
{
	return p_emu;
}

const _TCHAR *USING_FLAGS::get_sound_device_name(int num)
{
	if(p_osd == nullptr) return NULL;
	return (const _TCHAR *)(p_osd->get_sound_device_name(num));
}

const _TCHAR *USING_FLAGS::get_sound_device_name()
{
	__UNLIKELY_IF(p_osd == nullptr) return NULL;
	return (const _TCHAR *)(p_osd->get_sound_device_name(-1));
}


const int USING_FLAGS::get_sound_sample_rate(int num)
{
	__UNLIKELY_IF((num < 0) || (num >= (sizeof(sound_frequency_table) / sizeof(int)))) {
			return override_sound_frequency_48000hz;
	}
	const int f = sound_frequency_table[num];
	if(f <= 0) {
		return (const int)override_sound_frequency_48000hz;
	}
	return f;
}

const double USING_FLAGS::get_sound_latency(int num)
{
	__UNLIKELY_IF((num < 0) || (num >= 5)) {
		num = 1;
	}
	return sound_latency_table[num];
}

bool USING_FLAGS::is_support_phy_key_name()
{
	return false;
}

bool USING_FLAGS::check_feature(const _TCHAR* key)
{
	__UNLIKELY_IF(p_osd == nullptr) return false;
	return p_osd->check_feature(key);
}

bool USING_FLAGS::check_feature(const QString key)
{
	__UNLIKELY_IF(p_osd == nullptr) return false;
	if(key.isEmpty()) return false;

	const _TCHAR* p_key = (const _TCHAR *)(key.toUtf8().constData());
	return p_osd->check_feature(p_key);
}

bool USING_FLAGS::check_vm_name(const QString name)
{
	return check_feature(name);
}

void USING_FLAGS::set_config_directory(std::string confdir)
{
	set_config_directory(QString::fromStdString(confdir));
}

void USING_FLAGS::set_config_directory(QString confdir)
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	cpp_confdir = confdir;
}

QString USING_FLAGS::get_config_directory()
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	return cpp_confdir;
}


size_t USING_FLAGS::get_config_directory(_TCHAR* str, size_t maxlen)
{
	__UNLIKELY_IF(maxlen <= 0) return 0;
	__UNLIKELY_IF(str == nullptr) return 0;
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	_TCHAR* p = (_TCHAR*)(cpp_confdir.toLocal8Bit().constData());
	__UNLIKELY_IF(p == nullptr) return 0;
	size_t len = _tcslen(p);
	if(len >= maxlen) {
		len = maxlen;
	}
	my_tcscpy_s(str, len, p);
	return len;
}


void USING_FLAGS::set_home_directory(std::string homedir)
{
	set_home_directory(QString::fromStdString(homedir));
}

void USING_FLAGS::set_home_directory(QString homedir)
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	cpp_homedir = homedir;
}

QString USING_FLAGS::get_home_directory()
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	return cpp_homedir;
}

size_t USING_FLAGS::get_home_directory(_TCHAR* str, size_t maxlen)
{
	__UNLIKELY_IF(maxlen <= 0) return 0;
	__UNLIKELY_IF(str == nullptr) return 0;
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	_TCHAR* p = (_TCHAR*)(cpp_homedir.toLocal8Bit().constData());
	__UNLIKELY_IF(p == nullptr) return 0;
	size_t len = _tcslen(p);
	if(len >= maxlen) {
		len = maxlen;
	}
	my_tcscpy_s(str, len, p);
	return len;
}

void USING_FLAGS::set_proc_name(std::string procname)
{
	set_proc_name(QString::fromStdString(procname));
}

void USING_FLAGS::set_proc_name(QString procname)
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	my_procname = procname;
}

QString USING_FLAGS::get_proc_name()
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	return my_procname;
}

size_t USING_FLAGS::get_proc_name(_TCHAR* str, size_t maxlen)
{
	__UNLIKELY_IF(maxlen <= 0) return 0;
	__UNLIKELY_IF(str == nullptr) return 0;
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	_TCHAR* p = (_TCHAR*)(my_procname.toUtf8().constData());
	__UNLIKELY_IF(p == nullptr) return 0;
	size_t len = _tcslen(p);
	if(len >= maxlen) {
		len = maxlen;
	}
	my_tcscpy_s(str, len, p);
	return len;
}

void USING_FLAGS::set_resource_directory(std::string rssdir)
{
	set_resource_directory(QString::fromStdString(rssdir));
}

void USING_FLAGS::set_resource_directory(QString rssdir)
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	resource_directory = rssdir;
}

QString USING_FLAGS::get_resource_directory()
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	return resource_directory;
}

size_t USING_FLAGS::get_resource_directory(_TCHAR* str, size_t maxlen)
{
	__UNLIKELY_IF(maxlen <= 0) return 0;
	__UNLIKELY_IF(str == nullptr) return 0;
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	_TCHAR* p = (_TCHAR*)(resource_directory.toLocal8Bit().constData());
	__UNLIKELY_IF(p == nullptr) return 0;
	size_t len = _tcslen(p);
	if(len >= maxlen) {
		len = maxlen;
	}
	my_tcscpy_s(str, len, p);
	return len;
}
