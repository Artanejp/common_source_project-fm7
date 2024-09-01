/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

#ifndef _QT_OSD_H_
#define _QT_OSD_H_

#include "osd_base.h"
#include "gui/qt_input.h" // Key code table (VK_foo).
#include "../vm/vm.h"

class GLDrawClass;
class EmuThreadClass;
class DrawThreadClass;
class Ui_MainWindow;
class EMU;
class VM;
class FIFO;
class USING_FLAGS;
class CSP_KeyTables;
class MOVIE_LOADER;
class SOUND_LOADER;
class QTcpSocket2;
class QUdpSocket2;

QT_BEGIN_NAMESPACE	
class CSP_Logger;
class OSD : public OSD_BASE
{
	Q_OBJECT
private:
	GLDrawClass *p_glv;
	MOVIE_LOADER *movie_loader;

	QTcpSocket2 *tcp_socket[SOCKET_MAX];
	QUdpSocket2 *udp_socket[SOCKET_MAX];

	void set_features_machine(void);
	void set_features_cpu(void);
	void set_features_vm(void);
	void set_features_misc(void);
	void set_features_debug(void);
protected:
	
	// console
	// ToDo.
	
	// screen
	void initialize_screen() override;
	void release_screen() override;
	void initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode) override;
	scrntype_t* get_buffer(bitmap_t *p, int y) override;
	int add_video_frames() override;

	// video device
	void initialize_video() override;
	void release_video() override;

	// socket
	void initialize_socket() override;
	void release_socket() override;
	
	// MIDI : Will implement
	
	// Screen.
	int get_screen_width(void) override;
	int get_screen_height(void) override;
	int get_vm_buttons_code(int num) override;
	
	// Messaging.
	// ToDo.
public:
	OSD(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger);
	~OSD();
	
	void initialize(int rate, int samples, int* presented_rate, int* presented_samples) override;
	void release() override;

	// common console
	// ToDo.
	
	// console / telnet
	// ToDo.
	
	// common screen
	int get_window_mode_width(int mode) override;
	int get_window_mode_height(int mode) override;
	double get_window_mode_power(int mode) override;
	
	// common video device
	void get_video_buffer() override;
	bool open_movie_file(const _TCHAR* file_path) override;
	void close_movie_file() override;
	int get_movie_sound_rate() override;	
	uint32_t get_cur_movie_frame() override;
	
	// Socket
	SOCKET get_socket(int ch) override;
	void notify_socket_connected(int ch) override;
	void notify_socket_disconnected(int ch) override;
	void update_socket() override;
	bool initialize_socket_tcp(int ch) override;
	bool initialize_socket_udp(int ch) override;
	bool connect_socket(int ch, uint32_t ipaddr, int port) override;
	void disconnect_socket(int ch) override;
	bool listen_socket(int ch) override;
	void send_socket_data_tcp(int ch) override;
	void send_socket_data_udp(int ch, uint32_t ipaddr, int port) override;
	void send_socket_data(int ch) override;
	void recv_socket_data(int ch) override;

	// common MIDI
	// ToDo.
	
	// Wrapper
	void set_draw_thread(std::shared_ptr<DrawThreadClass> handler) override;
	QString get_vm_config_name(void) override;
	void reset_vm_node(void) override;
	const _TCHAR *get_lib_common_vm_version() override;
	
	// Get #define S to value.You may use inside of VM/ .
	void set_features(void) override;
	// Misc
	double get_vm_current_usec() override;
	uint64_t get_vm_current_clock_uint64() override;

	// Special
	bool set_glview(GLDrawClass *glv) override;
	GLDrawClass *get_gl_view() override { return p_glv; }

public slots:
	int draw_screen() override;
	void do_run_movie_audio_callback(uint8_t *data, long len) override;
	
	void do_decode_movie(int frames) override;

	void do_notify_socket_connected(int ch);
	void do_notify_socket_disconnected(int ch);

};
QT_END_NAMESPACE

#endif
