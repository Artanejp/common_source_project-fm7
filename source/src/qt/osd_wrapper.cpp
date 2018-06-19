/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ VM/OSD Wrapper ]
*/

#include <string>

#include <QObject>
#include <QWidget>

#include <QMutex>
#include <QSemaphore>
#include <QPainter>
#include <QElapsedTimer>
#include <QQueue>

#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QString>
#include <QObject>
#include <QThread>
#include <QMutex>

#include <QImage>
#include <QPainter>
#include <QColor>
#include <QPen>
#include <QPoint>
#include <QTextCodec>

#include "osd.h"
#include "../vm/vm.h"
#include "../vm/device.h"

#include "emu.h"

#include "emu_thread.h"
#include "draw_thread.h"
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
#include "avio/movie_loader.h"
#endif
#if defined(USE_SOUND_FILES)
#include "avio/sound_loader.h"
#endif
#include "qt_gldraw.h"
#include "csp_logger.h"

void OSD::vm_draw_screen(void)
{
	vm->draw_screen();
}

double OSD::vm_frame_rate(void)
{
#ifdef SUPPORT_VARIABLE_TIMING
	return vm->get_frame_rate();
#else
	return FRAMES_PER_SEC;
#endif   
}

Sint16* OSD::create_sound(int *extra_frames)
{
	return (Sint16 *)vm->create_sound(extra_frames);
}

#ifdef USE_SOUND_FILES
void OSD::load_sound_file(int id, const _TCHAR *name, int16_t **data, int *dst_size)
{
	int i = 0;
	if(data != NULL) *data = NULL;
	if(dst_size != NULL) *dst_size = 0;
	if(id <= 0) return;

	for(i = 0; i < USE_SOUND_FILES; i++) {
		SOUND_LOADER *p = sound_file_obj[i];
		if(p != NULL) {
			if(p->get_id() == id) break;
		}
	}
	
	if(i >= USE_SOUND_FILES) {
		for(i = 0; i < USE_SOUND_FILES; i++) {
			SOUND_LOADER *p = sound_file_obj[i];
			if(p != NULL) {
				if(p->get_id() < 0) {
					p->set_id(id);
					break;
				}
			}
		}
	}
	if(i >= USE_SOUND_FILES) return;
	SOUND_LOADER *p = sound_file_obj[i];
	if(p != NULL) {
		p->free_sound_buffer(NULL);
		p->set_sound_rate(this->get_sound_rate());
		if(p->open(id, QString::fromUtf8(name))) {
			p->do_decode_frames();
			p->close();
			if(data != NULL) *data = (int16_t *)(p->get_sound_buffer());
			if(dst_size != NULL) *dst_size = p->get_dst_size();
		}
	}
}

void OSD::free_sound_file(int id, int16_t **data)
{
	if(data == NULL) return;
	for(int i = 0; i < USE_SOUND_FILES; i++) {
		SOUND_LOADER *p = sound_file_obj[i];
		if(p != NULL) {
			if(p->get_id() == id) {
				p->free_sound_buffer(*data);
				*data = NULL;
				break;
			}
		}
	}
}

void OSD::init_sound_files()
{
	for(int i = 0; i < USE_SOUND_FILES; i++) {
		sound_file_obj[i] = NULL;
		SOUND_LOADER *p = new SOUND_LOADER((void *)tail_sound_file, p_logger);
		if(p != NULL) {
			sound_file_obj[i] = p;
		}
		tail_sound_file = p;
	}
}

void OSD::release_sound_files()
{
	for(int i = 0; i < USE_SOUND_FILES; i++) {
		SOUND_LOADER *p = sound_file_obj[i];
		if(p != NULL) delete p;
		sound_file_obj[i] = NULL;
	}
}
#endif
bool OSD::get_use_socket(void)
{
#ifdef USE_SOCKET
	return true;
#else
	return false;
#endif
}

bool OSD::get_support_variable_timing(void)
{
#ifdef SUPPORT_VARIABLE_TIMING
	return true;
#else
	return false;
#endif
}

bool OSD::get_notify_key_down(void)
{
#ifdef NOTIFY_KEY_DOWN
	return true;
#else
	return false;
#endif	
}

bool OSD::get_notify_key_down_lr_shift(void)
{
#ifdef NOTIFY_KEY_DOWN_LR_SHIFT
	return true;
#else
	return false;
#endif
}

bool OSD::get_notify_key_down_lr_control(void)
{
#ifdef NOTIFY_KEY_DOWN_LR_CONTROL
	return true;
#else
	return false;
#endif
}

bool OSD::get_notify_key_down_lr_menu(void)
{
#ifdef NOTIFY_KEY_DOWN_LR_MEHU
	return true;
#else
	return false;
#endif
}

bool OSD::get_use_shift_numpad_key(void)
{
#ifdef USE_SHIFT_NUMPAD_KEY
	return true;
#else
	return false;
#endif
}

bool OSD::get_use_auto_key(void)
{
#ifdef USE_AUTO_KEY
	return true;
#else
	return false;
#endif
}

bool OSD::get_dont_keeep_key_pressed(void)
{
#ifdef DONT_KEEEP_KEY_PRESSED
	return true;
#else
	return false;
#endif
}

bool OSD::get_one_board_micro_computer(void)
{
#ifdef ONE_BOARD_MICRO_COMPUTER
	return true;
#else
	return false;
#endif
}

bool OSD::get_use_screen_rotate(void)
{
#ifdef USE_SCREEN_ROTATE
	return true;
#else
	return false;
#endif
}

bool OSD::get_use_movie_player(void)
{
#ifdef USE_MOVIE_PLAYER
	return true;
#else
	return false;
#endif
}

bool OSD::get_use_video_capture(void)
{
#ifdef USE_VIDEO_CAPTURE
	return true;
#else
	return false;
#endif
}

void OSD::vm_key_down(int code, bool flag)
{
#ifdef NOTIFY_KEY_DOWN
	vm->key_down(code, flag);
#endif
}

void OSD::vm_key_up(int code)
{
#ifdef NOTIFY_KEY_DOWN
	vm->key_up(code);
#endif
}

void OSD::vm_reset(void)
{
	vm->reset();
}

int OSD::get_vm_buttons_code(int num)
{
#ifdef ONE_BOARD_MICRO_COMPUTER
	if(num < 0) return 0;
	return vm_buttons[num].code;
#else
	return 0;
#endif
}	

void OSD::update_buttons(bool press_flag, bool release_flag)
{
#if defined(MAX_BUTTONS)
	if(!press_flag && !release_flag) {
		int ii;
		ii = 0;
		for(ii = 0; vm_buttons[ii].code != 0x00; ii++) { 
			if((mouse_ptrx >= vm_buttons[ii].x) && (mouse_ptrx < (vm_buttons[ii].x + vm_buttons[ii].width))) {
				if((mouse_ptry >= vm_buttons[ii].y) && (mouse_ptry < (vm_buttons[ii].y + vm_buttons[ii].height))) {
					if((key_status[vm_buttons[ii].code] & 0x7f) == 0) this->press_button(ii);
				}
			}
		}
		if((mouse_ptrx >= vm_buttons[ii].x) && (mouse_ptrx < (vm_buttons[ii].x + vm_buttons[ii].width))) {
			if((mouse_ptry >= vm_buttons[ii].y) && (mouse_ptry < (vm_buttons[ii].y + vm_buttons[ii].height))) {
				this->press_button(ii);
			}
		}
		mouse_ptrx = mouse_ptry = 0;
	}
	//return;
#endif			
}	

QString OSD::get_vm_config_name(void)
{
#if defined(CONFIG_NAME)
	return QString::fromUtf8(CONFIG_NAME);
#else
	return QString::fromUtf8(" ");
#endif
}

int OSD::get_screen_width(void)
{
	return SCREEN_WIDTH;
}

int OSD::get_screen_height(void)
{
	return SCREEN_HEIGHT;
}

void OSD::lock_vm(void)
{
	locked_vm = true;
	vm_mutex->lock();
	//if(parent_thread != NULL) { 
		//if(!parent_thread->now_debugging()) VMSemaphore->acquire(1);
		//VMSemaphore->acquire(1);
	//} else {
	//	VMSemaphore->acquire(1);
	//}
}

void OSD::unlock_vm(void)
{
	vm_mutex->unlock();
	//if(parent_thread != NULL) { 
	//	//if(!parent_thread->now_debugging()) VMSemaphore->release(1);
	//	VMSemaphore->release(1);
	//} else {
	//	VMSemaphore->release(1);
	//}
	locked_vm = false;
}


bool OSD::is_vm_locked(void)
{
	return locked_vm;
}

void OSD::force_unlock_vm(void)
{
	vm_mutex->unlock();
	locked_vm = false;
}

void OSD::set_draw_thread(DrawThreadClass *handler)
{
	//this->moveToThread(handler);
	connect(this, SIGNAL(sig_update_screen(bitmap_t *)), handler, SLOT(do_update_screen(bitmap_t *)));
	connect(this, SIGNAL(sig_save_screen(const char *)), glv, SLOT(do_save_frame_screen(const char *)));
	connect(this, SIGNAL(sig_resize_vm_screen(QImage *, int, int)), glv, SLOT(do_set_texture_size(QImage *, int, int)));
	connect(this, SIGNAL(sig_resize_vm_lines(int)), glv, SLOT(do_set_horiz_lines(int)));
	connect(parent_thread, SIGNAL(sig_debugger_input(QString)), this, SLOT(do_set_input_string(QString)));
	connect(parent_thread, SIGNAL(sig_quit_debugger()), this, SLOT(do_close_debugger_thread()));
	connect(this, SIGNAL(sig_move_mouse_to_center()), glv, SLOT(do_move_mouse_to_center()));
	connect(this, SIGNAL(sig_close_window()), parent_thread, SLOT(doExit()));
}

void OSD::initialize_screen()
{
	host_window_width = base_window_width = WINDOW_WIDTH;
	host_window_height = base_window_height = WINDOW_HEIGHT;
	host_window_mode = true;
	
	vm_screen_width = SCREEN_WIDTH;
	vm_screen_height = SCREEN_HEIGHT;
	vm_window_width = WINDOW_WIDTH;
	vm_window_height = WINDOW_HEIGHT;
	vm_window_width_aspect = WINDOW_WIDTH_ASPECT;
	vm_window_height_aspect = WINDOW_HEIGHT_ASPECT;
	
	QColor col(0, 0, 0, 255);

	vm_screen_buffer.width = SCREEN_WIDTH;
	vm_screen_buffer.height = SCREEN_HEIGHT;
	vm_screen_buffer.pImage = QImage(SCREEN_WIDTH, SCREEN_HEIGHT, QImage::Format_ARGB32);
	vm_screen_buffer.pImage.fill(col);
	now_record_video = false;
	
	first_draw_screen = false;
	first_invalidate = true;
	self_invalidate = false;
}

void OSD::release_screen()
{
	stop_record_video();
	release_screen_buffer(&vm_screen_buffer);
}

int OSD::get_window_mode_width(int mode)
{
	if(get_use_screen_rotate()) {
		if(p_config->rotate_type == 1 || p_config->rotate_type == 3) {
			return (p_config->window_stretch_type == 0 ? vm_window_height : vm_window_height_aspect) * (mode + WINDOW_MODE_BASE);
		}
	}
	return (p_config->window_stretch_type == 0 ? vm_window_width : vm_window_width_aspect) * (mode + WINDOW_MODE_BASE);
}

int OSD::get_window_mode_height(int mode)
{
	if(get_use_screen_rotate()) {
		if(p_config->rotate_type == 1 || p_config->rotate_type == 3) {
			return (p_config->window_stretch_type == 0 ? vm_window_width : vm_window_width_aspect) * (mode + WINDOW_MODE_BASE);
		}
	}
	return (p_config->window_stretch_type == 0 ? vm_window_height : vm_window_height_aspect) * (mode + WINDOW_MODE_BASE);
}

void OSD::initialize_video()
{
	movie_loader = NULL;
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	movie_loader = new MOVIE_LOADER(this, &config);
	//connect(movie_loader, SIGNAL(sig_send_audio_frame(uint8_t *, long)), this, SLOT(do_run_movie_audio_callback2(uint8_t *, long)));
	connect(movie_loader, SIGNAL(sig_movie_end(bool)), this, SLOT(do_video_movie_end(bool)));
	connect(this, SIGNAL(sig_movie_play(void)), movie_loader, SLOT(do_play()));
	connect(this, SIGNAL(sig_movie_stop(void)), movie_loader, SLOT(do_stop()));
	connect(this, SIGNAL(sig_movie_pause(bool)), movie_loader, SLOT(do_pause(bool)));
	connect(this, SIGNAL(sig_movie_seek_frame(bool, int)), movie_loader, SLOT(do_seek_frame(bool, int)));
	//connect(this, SIGNAL(sig_movie_mute(bool, bool)), movie_loader, SLOT(do_mute(bool, bool)));
	connect(movie_loader, SIGNAL(sig_decoding_error(int)), this, SLOT(do_video_decoding_error(int)));
#endif	
}

void OSD::release_video()
{
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	delete movie_loader;
#endif	
	movie_loader = NULL;
}


bool OSD::open_movie_file(const _TCHAR* file_path)
{
	bool ret = false;
	if(file_path == NULL) return ret;
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	if(movie_loader != NULL) {
		ret = movie_loader->open(QString::fromUtf8(file_path));
	}
#endif	
	return ret;
}

void OSD::close_movie_file()
{
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	if(movie_loader != NULL) {
		movie_loader->close();
	}
#endif
	now_movie_play = false;
	now_movie_pause = false;
}

#include <limits.h>
uint32_t OSD::get_cur_movie_frame()
{
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	if(movie_loader != NULL) {
		uint64_t pos;
		pos = movie_loader->get_current_frame();
		if(pos > UINT_MAX) {
			return UINT_MAX;
		}
		return (uint32_t)pos;
	}
#endif	
	return 0;
}

void OSD::do_run_movie_audio_callback(uint8_t *data, long len)
{
	if(data == NULL) return;
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
#if defined(_PX7)
	{
		lock_vm();
		this->vm->movie_sound_callback(data, len);
		unlock_vm();
	}
#endif
#endif
	free(data);
}

void OSD::do_decode_movie(int frames)
{
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	//movie_loader->do_decode_frames(frames, SCREEN_WIDTH, SCREEN_HEIGHT);
	movie_loader->do_decode_frames(frames, vm_window_width_aspect, vm_window_height_aspect);
#endif	
}

void OSD::get_video_buffer()
{
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	//movie_loader->do_decode_frames(1, this->get_vm_window_width(), this->get_vm_window_height());
	movie_loader->get_video_frame();
	//printf("**\n");
#endif
}

int OSD::get_movie_sound_rate()
{
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	return movie_loader->get_movie_sound_rate();
#endif
	return 44100;
}

void OSD::reset_vm_node()
{
	device_node_t sp;
	device_node_list.clear();
	p_logger->reset();
	max_vm_nodes = 0;
	for(DEVICE *p = vm->first_device; p != NULL; p = p->next_device) {
		sp.id = p->this_device_id;
		sp.name = p->this_device_name;
		p_logger->set_device_name(sp.id, sp.name);
		p_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL,  "Device %d :%s", sp.id, sp.name);
		device_node_list.append(sp);
		if(max_vm_nodes <= p->this_device_id) max_vm_nodes = p->this_device_id + 1;
	}
	for(DEVICE *p = vm->first_device; p != NULL; p = p->next_device) {
		emit sig_update_device_node_name(p->this_device_id, p->this_device_name);
	}
}

#if defined(USE_SOCKET)
#include <QHostAddress>
#include "osd_socket.h"
#endif
// Socket
void OSD::initialize_socket()
{
	for(int i = 0; i < SOCKET_MAX; i++) {
		tcp_socket[i] = NULL;
		udp_socket[i] = NULL;
		is_tcp[i] = false;
		socket_delay[i] = 0;
		host_mode[i] = false;
	}
}

void OSD::release_socket()
{
	// release sockets
#ifdef USE_SOCKET
	for(int i = 0; i < SOCKET_MAX; i++) {
		if(tcp_socket[i] != NULL) {
			if(tcp_socket[i]->isOpen()) tcp_socket[i]->close();
			delete tcp_socket[i];
			tcp_socket[i] = NULL;
		}
		if(udp_socket[i] != NULL) {
			if(udp_socket[i]->isOpen()) udp_socket[i]->close();
			delete udp_socket[i];
			udp_socket[i] = NULL;
		}
	}
#endif	
}


void OSD::notify_socket_connected(int ch)
{
	do_notify_socket_connected(ch);
}

void OSD::do_notify_socket_connected(int ch)
{
#ifdef USE_SOCKET
	vm->notify_socket_connected(ch);
#endif	
}


void OSD::notify_socket_disconnected(int ch)
{
	do_notify_socket_disconnected(ch);
}


void OSD::do_notify_socket_disconnected(int ch)
{
	if(!socket_delay[ch]) {
		socket_delay[ch] = 1;//56;
	}
}

// Called per 1 frame.
void OSD::update_socket()
{
#ifdef USE_SOCKET
	qint64 bytes;
	for(int i = 0; i < SOCKET_MAX; i++) {
		QIODevice *p = NULL;
		if(is_tcp[i]) {
			if(tcp_socket[i] != NULL) {
				if(tcp_socket[i]->isOpen()) {
					p = tcp_socket[i];
				}
			}
		} else {
			if(udp_socket[i] != NULL) {
				if(udp_socket[i]->isOpen()) {
					p = udp_socket[i];
				}
			}
		}
		if(p != NULL) {	
			// recv
			bytes = p->bytesAvailable();
			if(bytes > 0) {
				int size0, size1;
				uint8_t* buf0 = vm->get_socket_recv_buffer0(i, &size0, &size1);
				uint8_t* buf1 = vm->get_socket_recv_buffer1(i);
				qint64 size;
				
				if(bytes > (qint64)(size0 + size1)) {
					bytes = (qint64)(size0 + size1);
				}
				QByteArray src = p->read(bytes);

				size = src.size();
				uint8_t *pp = (uint8_t *)(src.constData());
				if(size <= (qint64)size0) {
					memcpy(buf0, pp, size);
				} else {
					memcpy(buf0, pp, size0);
					memcpy(buf1, pp + size0, (int)size - size0);
				}
				vm->inc_socket_recv_buffer_ptr(i, (int)size);
			} else if(socket_delay[i] != 0) {
				if(--socket_delay[i] == 0) {
					vm->notify_socket_disconnected(i);
				}
			}
		}
	}
#endif	
}

bool OSD::initialize_socket_tcp(int ch)
{
#ifdef USE_SOCKET
	if(udp_socket[ch] != NULL) {
		if(udp_socket[ch]->isOpen()) {
			udp_socket[ch]->close();
		}
		delete udp_socket[ch];
		udp_socket[ch] = NULL;
	}
	if(tcp_socket[ch] != NULL) {
		if(tcp_socket[ch]->isOpen()) tcp_socket[ch]->close();
		delete tcp_socket[ch];
	}
	is_tcp[ch] = true;
	tcp_socket[ch] = new QTcpSocket2(ch);
	if(tcp_socket[ch] == NULL) return false;
	tcp_socket[ch]->setChannel(ch);
	connect(tcp_socket[ch], SIGNAL(connected()), tcp_socket[ch], SLOT(do_connected()));
	connect(tcp_socket[ch], SIGNAL(sig_connected(int)), this, SLOT(do_notify_socket_connected(int)));
	connect(tcp_socket[ch], SIGNAL(disconnected()), tcp_socket[ch], SLOT(do_disconnected()));
	connect(tcp_socket[ch], SIGNAL(sig_disconnected(int)), this, SLOT(do_notify_socket_disconnected(int)));
#endif	
	return true;
}

bool OSD::initialize_socket_udp(int ch)
{
#ifdef USE_SOCKET
	if(tcp_socket[ch] != NULL) {
		if(tcp_socket[ch]->isOpen()) {
			tcp_socket[ch]->close();
		}
		delete tcp_socket[ch];
		tcp_socket[ch] = NULL;
	}
	if(udp_socket[ch] != NULL) {
		if(udp_socket[ch]->isOpen()) udp_socket[ch]->close();
		delete udp_socket[ch];
	}
	is_tcp[ch] = false;
	udp_socket[ch] = new QUdpSocket2(ch);
	if(udp_socket[ch] == NULL) return false;
	connect(udp_socket[ch], SIGNAL(connected()), udp_socket[ch], SLOT(do_connected()));
	connect(udp_socket[ch], SIGNAL(sig_connected(int)), this, SLOT(do_notify_socket_connected(int)));
	connect(udp_socket[ch], SIGNAL(disconnected()), udp_socket[ch], SLOT(do_disconnected()));
	connect(udp_socket[ch], SIGNAL(sig_disconnected(int)), this, SLOT(do_notify_socket_disconnected(int)));
#endif	
	return true;
}

bool OSD::connect_socket(int ch, uint32_t ipaddr, int port)
{
#ifdef USE_SOCKET
	QHostAddress addr = QHostAddress((quint32)ipaddr);
	if(is_tcp[ch]) {
		if(tcp_socket[ch] != NULL) {
			tcp_socket[ch]->connectToHost(addr, (quint16)port);
		} else {
			return false;
		}
	} else {
		if(udp_socket[ch] != NULL) {
			udp_socket[ch]->connectToHost(addr, (quint16)port);
		} else {
			return false;
		}
	}
	host_mode[ch] = false;
#endif
	return true;
}

void OSD::disconnect_socket(int ch)
{
//	soc[ch] = -1;
#ifdef USE_SOCKET
	if(host_mode[ch]) {
		if(is_tcp[ch]) {
			if(tcp_socket[ch] != NULL) {
				if(tcp_socket[ch]->isOpen()) tcp_socket[ch]->close();
			}
		} else {
			if(udp_socket[ch] != NULL) {
				if(udp_socket[ch]->isOpen()) udp_socket[ch]->close();
			}
		}
	} else {
		if(is_tcp[ch]) {
			if(tcp_socket[ch] != NULL) {
				udp_socket[ch]->disconnectFromHost();
			}
		} else {
			if(udp_socket[ch] != NULL) {
				udp_socket[ch]->disconnectFromHost();
			}
		}
	}		
	vm->notify_socket_disconnected(ch);
#endif	
}

bool OSD::listen_socket(int ch)
{
#ifdef USE_SOCKET
	//QHostAddress addr = QHostAddress(QHostAddress::AnyIPv4); // OK?
	// This unit is dummy?
	//connect(udp_socket[ch], SIGNAL(connected()), udp_socket[ch], SLOT(do_connected()));
	//connect(udp_socket[ch], SIGNAL(sig_connected(int)), this, SLOT(do_notify_socket_connected(int)));
	//connect(udp_socket[ch], SIGNAL(disconnected()), udp_socket[ch], SLOT(do_disconnected()));
	//connect(udp_socket[ch], SIGNAL(sig_disconnected(int)), this, SLOT(do_notify_socket_disconnected(int)));
#endif	
	return false;
}

void OSD::send_socket_data_tcp(int ch)
{
#ifdef USE_SOCKET
	if(is_tcp[ch]) {
		while(1) {
			int size;
			uint8_t *buf = vm->get_socket_send_buffer(ch, &size);
			if(size <= 0) {
				return;
			}
			qint64 size2 = 0;
			if(tcp_socket[ch] != NULL) {
				if(tcp_socket[ch]->isWritable()) {
					size2 = tcp_socket[ch]->write((const char *)buf, (qint64)size);
					if(size2 < 0) {
						disconnect_socket(ch);
						notify_socket_disconnected(ch);
						return;
					}
				}
			} else {
				return;
			}
			vm->inc_socket_send_buffer_ptr(ch, (int)size2);
		}
	}
#endif	
}

void OSD::send_socket_data_udp(int ch, uint32_t ipaddr, int port)
{
#ifdef USE_SOCKET
	QHostAddress addr = QHostAddress((quint32)ipaddr);
	if(!is_tcp[ch]) {
		while(1) {
			int size;
			uint8_t *buf = vm->get_socket_send_buffer(ch, &size);
			if(size <= 0) {
				return;
			}
			qint64 size2 = 0;
			
			if(udp_socket[ch] != NULL) {
				size2 = udp_socket[ch]->writeDatagram((const char *)buf, (qint64)size, addr, (quint16)port);
				if(size2 < 0) {
					disconnect_socket(ch);
					notify_socket_disconnected(ch);
					return;
				}
			} else {
				return;
			}
			vm->inc_socket_send_buffer_ptr(ch, (int)size2);
		}
	}
#endif	
}

void OSD::send_socket_data(int ch)
{
	// This is dummy.
}

void OSD::recv_socket_data(int ch)
{
	// This is dummy.
}

int OSD::get_socket(int ch)
{
#ifdef USE_SOCKET
	if(is_tcp[ch]) {
		if(tcp_socket[ch] == NULL) return -1;
	} else {
		if(udp_socket[ch] == NULL) return -1;
	}
#endif	
	return ch;
}

//
#if defined(USE_SOCKET)
QTcpSocket2::QTcpSocket2(int channel, QObject *parent) : QTcpSocket(parent)
{
	ch = channel;
}

QTcpSocket2::~QTcpSocket2()
{
}

void QTcpSocket2::do_connected(void)
{
	emit sig_connected(ch);
}

void QTcpSocket2::do_disconnected(void)
{
	emit sig_disconnected(ch);
}

void QTcpSocket2::setChannel(int channel)
{
	ch = channel;
}

int QTcpSocket2::getChannel(void)
{
	return ch;
}

QUdpSocket2::QUdpSocket2(int channel, QObject *parent) : QUdpSocket(parent)
{
	ch = channel;
}

QUdpSocket2::~QUdpSocket2()
{
}

void QUdpSocket2::do_connected(void)
{
	emit sig_connected(ch);
}

void QUdpSocket2::do_disconnected(void)
{
	emit sig_disconnected(ch);
}

void QUdpSocket2::setChannel(int channel)
{
	ch = channel;
}

int QUdpSocket2::getChannel(void)
{
	return ch;
}
#endif
