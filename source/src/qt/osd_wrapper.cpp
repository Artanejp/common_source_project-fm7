/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ VM/OSD Wrapper ]
*/

#include <string>

#include <QObject>
#include <QWidget>

#include <chrono>
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

#include <QOpenGLContext>

#include <QImage>
#include <QPainter>
#include <QColor>
#include <QPen>
#include <QPoint>
#include <QTextCodec>

#include "osd.h"
#include "../vm/vm.h"
#include "../vm/device.h"
#include "../common.h"

#include "emu.h"

#include "emu_thread.h"
#include "draw_thread.h"
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
#include "avio/movie_loader.h"
#endif
#include "qt_gldraw.h"
#include "csp_logger.h"


int OSD::get_vm_buttons_code(int num)
{
#ifdef ONE_BOARD_MICRO_COMPUTER
	if(num < 0) return 0;
	return vm_buttons[num].code;
#else
	return 0;
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
#if !defined(SCREEN_WIDTH)
	return OSD_BASE::get_screen_width();
#else
	return SCREEN_WIDTH;
#endif
}

int OSD::get_screen_height(void)
{
#if !defined(SCREEN_HEIGHT)
	return OSD_BASE::get_screen_height();
#else
	return SCREEN_HEIGHT;
#endif
}


void OSD::set_draw_thread(std::shared_ptr<DrawThreadClass> handler)
{
	if(handler == nullptr) return;

	m_draw_thread = handler;
	
	connect(this, SIGNAL(sig_update_screen(void *, bool)), m_draw_thread.get(), SLOT(do_update_screen(void *, bool)));
	connect(this, SIGNAL(sig_save_screen(const char *)), p_glv, SLOT(do_save_frame_screen(const char *)));
	connect(this, SIGNAL(sig_resize_vm_screen(QImage *, int, int)), p_glv, SLOT(do_set_texture_size(QImage *, int, int)));
	connect(this, SIGNAL(sig_resize_vm_lines(int)), p_glv, SLOT(do_set_horiz_lines(int)));
	connect(parent_thread, SIGNAL(sig_debugger_input(QString)), this, SLOT(do_set_input_string(QString)));
	connect(parent_thread, SIGNAL(sig_quit_debugger()), this, SLOT(do_close_debugger_thread()));
	connect(this, SIGNAL(sig_move_mouse_to_center()), p_glv, SLOT(do_move_mouse_to_center()));
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

double OSD::get_window_mode_power(int mode)
{
	if(mode + WINDOW_MODE_BASE == 2) {
		return 1.5;
	} else if(mode + WINDOW_MODE_BASE > 2) {
		return mode + WINDOW_MODE_BASE - 1;
	}
	return mode + WINDOW_MODE_BASE;
}


void OSD::initialize_video()
{
	movie_loader = nullptr;
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	movie_loader = new MOVIE_LOADER(this, &config);
	//connect(movie_loader, SIGNAL(sig_send_audio_frame(uint8_t *, long)), this, SLOT(do_run_movie_audio_callback2(uint8_t *, long)));
	connect(this, SIGNAL(sig_movie_open(QString)), movie_loader, SLOT(do_open(QString)));
	connect(movie_loader, SIGNAL(sig_movie_end(bool)), this, SLOT(do_video_movie_end(bool)));
	connect(this, SIGNAL(sig_movie_play(void)), movie_loader, SLOT(do_play()));
	connect(this, SIGNAL(sig_movie_stop(void)), movie_loader, SLOT(do_stop()));
	connect(this, SIGNAL(sig_movie_pause(bool)), movie_loader, SLOT(do_pause(bool)));
	connect(this, SIGNAL(sig_movie_seek_frame(bool, int)), movie_loader, SLOT(do_seek_frame(bool, int)));
	connect(this, SIGNAL(sig_movie_eject()), movie_loader, SLOT(do_eject()));
	connect(this, SIGNAL(sig_movie_quit()), movie_loader, SLOT(do_abort_movie_loader()));
	//connect(this, SIGNAL(sig_movie_mute(bool, bool)), movie_loader, SLOT(do_mute(bool, bool)));
	connect(movie_loader, SIGNAL(sig_decoding_error(int)), this, SLOT(do_video_decoding_error(int)));
#endif
}

void OSD::release_video()
{
	emit sig_movie_quit();
//#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
//	delete movie_loader;
//#endif
//	movie_loader = NULL;
}


bool OSD::open_movie_file(const _TCHAR* file_path)
{
	bool ret = false;
	if(file_path ==nullptr) return false;
	emit sig_movie_open(QString::fromLocal8Bit(file_path));
	return true;
}

void OSD::close_movie_file()
{
	emit sig_movie_eject();
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
		std::lock_guard<std::recursive_timed_mutex>  n(vm_mutex);
		if((vm != NULL) && !(is_will_delete_vm())) {
			vm->movie_sound_callback(data, len);
		}
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

#if defined(USE_SOCKET)
#include <QHostAddress>
#include <QTcpSocket>
#include <QUdpSocket>

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
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);
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
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);
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
		if((p != nullptr) && (vm != nullptr)) {
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
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);

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
	if((vm == nullptr) || (is_will_delete_vm())) return;
	vm->notify_socket_disconnected(ch);
#endif
}

bool OSD::listen_socket(int ch)
{
#ifdef USE_SOCKET
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);

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
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);

	if((vm == nullptr) || (is_will_delete_vm())) return;
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
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);

	QHostAddress addr = QHostAddress((quint32)ipaddr);
	if((vm == nullptr) || (is_will_delete_vm())) return;
	if(!(is_tcp[ch])) {
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

SOCKET OSD::get_socket(int ch)
{
#ifdef USE_SOCKET
	if(is_tcp[ch]) {
		if(tcp_socket[ch] == NULL) return (SOCKET)0;
		return (SOCKET)tcp_socket[ch];
	} else {
		if(udp_socket[ch] == NULL) return (SOCKET)0;
		return (SOCKET)udp_socket[ch];
	}
#endif
	return (SOCKET)0;
}

//

// Screen
scrntype_t DLL_PREFIX *bitmap_s::get_buffer(int y)
{
	if((is_mapped) && (glv != NULL)) {
		scrntype_t *p = NULL;
			p = glv->get_screen_buffer(y);
			if(p != NULL) return p;
		}
		return (scrntype_t *)pImage.scanLine(y);
}

scrntype_t* OSD::get_buffer(bitmap_t *p, int y)
{
	if(p_glv->is_ready_to_map_vram_texture()) {
		if(p == &vm_screen_buffer) {
			return (scrntype_t *)p->get_buffer(y);
		}
	}
	if((y >= p->pImage.height()) || (y < 0) || (y >= p->height)) {
		return NULL;
	}
	return (scrntype_t *)p->pImage.scanLine(y);
}


int OSD::draw_screen()
{
	// draw screen
	std::lock_guard<std::recursive_timed_mutex> Locker_S(screen_mutex);
	bool mapped = false;
	//QMutexLocker Locker_VM(&vm_mutex);
	#if 0
	if(vm_screen_buffer.width != vm_screen_width || vm_screen_buffer.height != vm_screen_height) {
		//emit sig_movie_set_width(vm_screen_width);
		//emit sig_movie_set_height(vm_screen_height);
		initialize_screen_buffer(&vm_screen_buffer, vm_screen_width, vm_screen_height, 0);
	}
	#endif
	#if 1
	if(p_glv->is_ready_to_map_vram_texture()) {
		vm_screen_buffer.is_mapped = true;
		mapped = true;
		vm_screen_buffer.glv = p_glv;
	} else {
		vm_screen_buffer.is_mapped = false;
	}
	#else
			vm_screen_buffer.is_mapped = false;
	#endif
	this->vm_draw_screen();
	// screen size was changed in vm->draw_screen()
	if(vm_screen_buffer.width != vm_screen_width || vm_screen_buffer.height != vm_screen_height) {
		return 0;
	}
	draw_screen_buffer = &vm_screen_buffer;

	// calculate screen size
	// invalidate window
	// ToDo: Support MAX_DRAW_RANGES. 20221212 K.O
	//emit sig_update_screen((void *)draw_screen_buffer, mapped);
	// Direct call to DrawThread, because this function called from DrawThread. 20240212 K.O
	__LIKELY_IF(m_draw_thread.get() != nullptr) {
		m_draw_thread->do_update_screen((void *)draw_screen_buffer, mapped);
	}
	first_draw_screen = self_invalidate = true;

	// record avi file
	if(now_record_video) {
		add_video_frames();
	}
	return 1;
}

void OSD::initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode)
{
	std::lock_guard<std::recursive_timed_mutex> Locker_S(screen_mutex);
	OSD_BASE::initialize_screen_buffer(buffer, width, height, mode);
	buffer->glv = p_glv;
	//emit sig_movie_set_width(width);
	//emit sig_movie_set_height(height);
	emit sig_resize_vm_screen(&(buffer->pImage), width, height);
}

bool OSD::set_glview(GLDrawClass *glv)
{
	if(glv == NULL) return false;
	if(glContext != NULL) {
		if(glContext->isValid()) return true;
		return false;
	}
	p_glv = glv;

	glContext = new QOpenGLContext();
	if(glContext != NULL) {
		glContext->setShareContext(glv->context());
		glContext->create();
	}
	if(glContext->isValid()) {
		is_glcontext_shared = true;
		return true;
	}
	return false;
}

int OSD::add_video_frames()
{
	//static double frames = 0;
	//static int prev_video_fps = -1;
	int counter = 0;
	//static double prev_vm_fps = -1;
	double vm_fps = vm_frame_rate();
	int delta_ns = (int)(1.0e9 / vm_fps);
	//if(rec_video_fps_nsec >= delta_ns) {
	if(delta_ns == rec_video_fps_nsec) {
		rec_video_nsec += delta_ns;
		if(rec_video_nsec > (rec_video_fps_nsec * 2)) {
			rec_video_nsec -= rec_video_fps_nsec;
		} else if(rec_video_nsec < (rec_video_fps_nsec * -2)) {
			rec_video_nsec += rec_video_fps_nsec;
		}
		while(rec_video_nsec > rec_video_fps_nsec) {
			rec_video_nsec -= rec_video_fps_nsec;
			counter++;
		}
	} else { // Will branch whether rec_video_fps_nsec >= delta_ns ?
		rec_video_nsec += delta_ns;
		if(rec_video_nsec > (rec_video_fps_nsec * 2)) {
			rec_video_nsec -= rec_video_fps_nsec;
		} else if(rec_video_nsec < (rec_video_fps_nsec * -2)) {
			rec_video_nsec += rec_video_fps_nsec;
		}
		while(rec_video_nsec >= rec_video_fps_nsec) {
			rec_video_nsec -= rec_video_fps_nsec;
			counter++;
		}
	}

	if(using_flags->is_use_one_board_computer()) {
		//int size = vm_screen_buffer.pImage.byteCount();
		int i = counter;
		rec_image_buffer = background_image.rgbSwapped();
		if(p_glv->is_ready_to_map_vram_texture()) {
			vm_screen_buffer.is_mapped = true;
			vm_screen_buffer.glv = p_glv;
			for(int y = 0; y < vm_screen_buffer.pImage.height(); y++) {
				scrntype_t *p = vm_screen_buffer.get_buffer(y);
				if(p != NULL) {
					if(p != (scrntype_t*)(vm_screen_buffer.pImage.scanLine(y))) {
						memcpy(vm_screen_buffer.pImage.scanLine(y), p, vm_screen_buffer.pImage.width() * sizeof(scrntype_t));
					}
				} else {
					if(vm_screen_buffer.pImage.scanLine(y) != NULL) {
						memset(vm_screen_buffer.pImage.scanLine(y), 0x00, vm_screen_buffer.pImage.width() * sizeof(scrntype_t));
					}
				}
			}
		}
		QImage video_result = QImage(vm_screen_buffer.pImage);

		QRgb pixel;
		int ww = video_result.width();
		int hh = video_result.height();
		//printf("%d x %d\n", ww, hh);
		for(int yy = 0; yy < hh; yy++) {
			for(int xx = 0; xx < ww; xx++) {
				pixel = video_result.pixel(xx, yy);
#if defined(__LITTLE_ENDIAN__)
				pixel |= 0xff000000;
				if(pixel != 0xff000000) {
					rec_image_buffer.setPixel(xx, yy, pixel);
				}
#else
				pixel |= 0x000000ff;
				if(pixel != 0x000000ff) {
					rec_image_buffer.setPixel(xx, yy, pixel);
				}
#endif
			}
		}
		if(i > 0) {
			// Enqueue to frame.
			emit sig_enqueue_video(i, background_image.width(), background_image.height(), &rec_image_buffer);
			//i--;
		}
	} else {
		//int size = vm_screen_buffer.pImage.byteCount();
		int i = counter;
		QImage video_result;
		video_result = QImage(vm_screen_buffer.pImage);
		// Rescaling
		if(i > 0) {
			// Enqueue to frame.
			emit sig_enqueue_video(i, vm_screen_width, vm_screen_height, &video_result);
			//i--;
		}
		// _TCHAR __tmps1[128] = {0};
		// my_stprintf_s(__tmps1, sizeof(__tmps1) - 1, "Push Video %d frames\n", counter);
		//emit sig_debug_log(CSP_LOG_DEBUG2, CSP_LOG_TYPE_SCREEN, QString::fromUtf8(__tmps1) );
	}
	return counter;
}

double OSD::get_vm_current_usec()
{
	if(log_mutex.try_lock_for(std::chrono::milliseconds(100))) {
		if(vm == nullptr) {
			log_mutex.unlock();
			return 0.0;
		}
		double _d = vm->get_current_usec();
		log_mutex.unlock();
		return _d;
	}
	return 0.0;
}

uint64_t OSD::get_vm_current_clock_uint64()
{
	if(log_mutex.try_lock_for(std::chrono::milliseconds(100))) {
		if(vm == nullptr) {
			log_mutex.unlock();
			return 0;
		}
		uint64_t _n = vm->get_current_clock_uint64();
		log_mutex.unlock();
		return _n;
	}
	return 0;
}

const _TCHAR *OSD::get_lib_common_vm_version()
{
	// ToDo: Really need to lock? 20221011 K.O
//	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);

	if(vm->first_device != NULL) {
		return vm->first_device->get_lib_common_vm_version();
	} else {
		return (const _TCHAR *)"\0";
	}
}

void OSD::reset_vm_node(void)
{
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);

	device_node_t sp;
	device_node_list.clear();
	emit sig_logger_reset();

	max_vm_nodes = 0;
	if((vm == nullptr) || (is_will_delete_vm())) return;

	for(DEVICE *p = vm->first_device; p != NULL; p = p->next_device) {
		sp.id = p->this_device_id;
		sp.name = p->this_device_name;
		emit sig_logger_set_device_name(sp.id, QString::fromUtf8(sp.name));

		_TCHAR tmps2[512] = {0};
		my_stprintf_s(tmps2, sizeof(tmps2) - 1, "Device %d :%s", sp.id, sp.name);
		emit sig_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, QString::fromUtf8(tmps2));

		device_node_list.append(sp);
		if(max_vm_nodes <= p->this_device_id) max_vm_nodes = p->this_device_id + 1;
	}
	for(DEVICE *p = vm->first_device; p != NULL; p = p->next_device) {
		emit sig_update_device_node_name(p->this_device_id, p->this_device_name);
	}
}
