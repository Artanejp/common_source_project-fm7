/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt screen ]
*/

#include <QImage>
#include "qt_gldraw.h"
#include "osd.h"

#define REC_VIDEO_SUCCESS	1
#define REC_VIDEO_FULL		2
#define REC_VIDEO_ERROR		3

void OSD::initialize_screen()
{
	host_window_width = base_window_width = WINDOW_WIDTH;
	host_window_height = base_window_height = WINDOW_HEIGHT;
	host_window_mode = true;
	
	vm_screen_width = SCREEN_WIDTH;
	vm_screen_height = SCREEN_HEIGHT;
	vm_screen_width_aspect = SCREEN_WIDTH_ASPECT;
	vm_screen_height_aspect = SCREEN_HEIGHT_ASPECT;
	
	//memset(&vm_screen_buffer, 0, sizeof(screen_buffer_t));
	vm_screen_buffer.width = SCREEN_WIDTH;
	vm_screen_buffer.height = SCREEN_HEIGHT;
	vm_screen_buffer.pImage = QImage(SCREEN_WIDTH, SCREEN_HEIGHT, QImage::Format_ARGB32);
#ifdef USE_CRT_FILTER
	filtered_screen_buffer.width = SCREEN_WIDTH;
	filtered_screen_buffer.height = SCREEN_HEIGHT;
	filtered_screen_buffer.pImage = QImage(SCREEN_WIDTH_ASPECT, SCREEN_HEIGHT_ASPECT, QImage::Format_ARGB32);
#endif
	
	now_rec_video = false;
	//pAVIStream = NULL;
	//pAVICompressed = NULL;
	//pAVIFile = NULL;
	
	first_draw_screen = false;
	first_invalidate = true;
	self_invalidate = false;
}

void OSD::release_screen()
{
	stop_rec_video();
	
	//release_d3d9();
	//if(vm_screen_buffer.pImage != NULL) delete vm_screen_buffer.pImage;
	release_screen_buffer(&vm_screen_buffer);
#ifdef USE_CRT_FILTER
	//if(filtered_screen_buffer.pImage != NULL) delete filtered_screen_buffer.pImage;
	release_screen_buffer(&filtered_screen_buffer);
#endif
}

int OSD::get_window_width(int mode)
{
#ifdef USE_SCREEN_ROTATE
	if(config.rotate_type) {
		return base_window_height + vm_screen_height_aspect * mode;
	}
#endif
	return base_window_width + vm_screen_width_aspect * mode;
}

int OSD::get_window_height(int mode)
{
#ifdef USE_SCREEN_ROTATE
	if(config.rotate_type) {
		return base_window_width + vm_screen_width_aspect * mode;
	}
#endif
	return base_window_height + vm_screen_height_aspect * mode;
}

void OSD::set_window_size(int window_width, int window_height, bool window_mode)
{
	if(window_width != -1) {
		host_window_width = window_width;
	}
	if(window_height != -1) {
		host_window_height = window_height;
	}
	host_window_mode = window_mode;
	
	first_draw_screen = false;
	first_invalidate = true;
}

void OSD::set_vm_screen_size(int width, int height, int width_aspect, int height_aspect, int window_width, int window_height)
{
	if(vm_screen_width != width || vm_screen_height != height) {
		if(width_aspect == -1) {
			width_aspect = width;
		}
		if(height_aspect == -1) {
			height_aspect = height;
		}
		int wold = vm_screen_width;
		int hold = vm_screen_height;
		vm_screen_width = width;
		vm_screen_height = height;
		vm_screen_width_aspect = width_aspect;
		vm_screen_height_aspect = height_aspect;
		base_window_width = window_width;
		base_window_height = window_height;
		// change the window size
		//emit sig_resize_vm_screen(vm_screen_width, vm_screen_height);
	}
}

scrntype* OSD::get_vm_screen_buffer(int y)
{
	return get_buffer(&vm_screen_buffer, y);
}

scrntype* OSD::get_buffer(screen_buffer_t *p, int y)
{
	if((y >= p->pImage.height()) || (y < 0) || (y >= p->height)) {
		return NULL;
	}
	return (scrntype *)p->pImage.scanLine(y);
}

int OSD::draw_screen()
{
	// check avi file recording timing
	if(now_rec_video && rec_video_run_frames <= 0) {
		return 0;
	}
	
	// draw screen
	lock_vm();
	if(vm_screen_buffer.width != vm_screen_width || vm_screen_buffer.height != vm_screen_height) {
		if(now_rec_video) {
			stop_rec_video();
//			stop_rec_sound();
		}
		initialize_screen_buffer(&vm_screen_buffer, vm_screen_width, vm_screen_height, 0);
	}
	vm->draw_screen();
	unlock_vm();
	// screen size was changed in vm->draw_screen()
	if(vm_screen_buffer.width != vm_screen_width || vm_screen_buffer.height != vm_screen_height) {
		return 0;
	}
	draw_screen_buffer = &vm_screen_buffer;
	
	// calculate screen size
	// invalidate window
	emit sig_update_screen(draw_screen_buffer);

	first_draw_screen = self_invalidate = true;
	
	// record avi file
	if(now_rec_video) {
		return add_video_frames();
	} else {
		return 1;
	}
}

void OSD::update_screen()
{
	// UpdateScreen
	// -> qt_gldraw.cpp , qt_glutil.cpp and qt_glevents.cpp within src/qt/common/ .
	first_invalidate = self_invalidate = false;
}

void OSD::initialize_screen_buffer(screen_buffer_t *buffer, int width, int height, int mode)
{
	release_screen_buffer(buffer);
	buffer->width = width;
	buffer->height = height;
	if((width > buffer->pImage.width()) || (height > buffer->pImage.height())) {
		buffer->pImage = QImage(width, height, QImage::Format_ARGB32);
	}
	//printf("%dx%d NULL=%d\n", buffer->pImage.width(), buffer->pImage.height(), buffer->pImage.isNull() ? 1 : 0);
	QColor fillcolor;
	fillcolor.setRgb(0, 0, 0, 255);
	buffer->pImage.fill(fillcolor);
	emit sig_resize_vm_screen(&(buffer->pImage), width, height);
}

void OSD::release_screen_buffer(screen_buffer_t *buffer)
{
	if(!(buffer->width == 0 && buffer->height == 0)) {
		//delete buffer->pImage;
	}
	buffer->width = 0;
	buffer->height = 0;
	//memset(buffer, 0, sizeof(screen_buffer_t));
}

#ifdef USE_CRT_FILTER
#define _3_8(v) (((((v) * 3) >> 3) * 180) >> 8)
#define _5_8(v) (((((v) * 3) >> 3) * 180) >> 8)
#define _8_8(v) (((v) * 180) >> 8)

static uint8 r0[2048], g0[2048], b0[2048], t0[2048];
static uint8 r1[2048], g1[2048], b1[2048];

void OSD::apply_crt_fileter_to_screen_buffer(screen_buffer_t *source, screen_buffer_t *dest)
{
	if(source->width * 3 == dest->width && source->height * 3 == dest->height) {
		apply_crt_filter_x3_y3(source, dest);
	} else if(source->width * 3 == dest->width && source->height * 2 == dest->height) {
		apply_crt_filter_x3_y2(source, dest);
	} else if(source->width * 2 == dest->width && source->height * 3 == dest->height) {
		apply_crt_filter_x2_y3(source, dest);
	} else if(source->width * 2 == dest->width && source->height * 2 == dest->height) {
		apply_crt_filter_x2_y2(source, dest);
	} else if(source->width != dest->width || source->height != dest->height) {
		apply_crt_filter_x1_y1(source, source);
		stretch_screen_buffer(source, dest);
	} else {
		apply_crt_filter_x1_y1(source, dest);
	}
}

void OSD::apply_crt_filter_x3_y3(screen_buffer_t *source, screen_buffer_t *dest)
{
	if(!screen_skip_line) {
		for(int y = 0, yy = 0; y < source->height; y++, yy += 3) {
			scrntype* src = get_buffer(source, y);
			scrntype* out1 = get_buffer(dest, yy + 0);
			scrntype* out2 = get_buffer(dest, yy + 1);
			scrntype* out3 = get_buffer(dest, yy + 2);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 3) {
				uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = (32 + _8_8(r)) << 16;
				out1[xx + 1] = out2[xx + 1] = (32 + _8_8(g)) << 8;
				out1[xx + 2] = out2[xx + 2] = (32 + _8_8(b));
				if(t0[x]) {
					out3[xx    ] = (32 + _8_8(r)) << 16;
					out3[xx + 1] = (32 + _8_8(g)) << 8;
					out3[xx + 2] = (32 + _8_8(b));
				} else {
					out3[xx    ] = (32 + _5_8(r)) << 16;
					out3[xx + 1] = (32 + _5_8(g)) << 8;
					out3[xx + 2] = (32 + _5_8(b));
				}
			}
		}
	} else {
		for(int y = 0, yy = 0; y < source->height; y += 2, yy += 6) {
			scrntype* src = get_buffer(source, y);
			scrntype* out1 = get_buffer(dest, yy + 0);
			scrntype* out2 = get_buffer(dest, yy + 1);
			scrntype* out3 = get_buffer(dest, yy + 2);
			scrntype* out4 = get_buffer(dest, yy + 3);
			scrntype* out5 = get_buffer(dest, yy + 4);
			scrntype* out6 = get_buffer(dest, yy + 5);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 3) {
				uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = out3[xx    ] = out4[xx    ] = (32 + _8_8(r)) << 16;
				out1[xx + 1] = out2[xx + 1] = out3[xx + 1] = out4[xx + 1] = (32 + _8_8(g)) << 8;
				out1[xx + 2] = out2[xx + 2] = out3[xx + 2] = out4[xx + 2] = (32 + _8_8(b));
				if(t0[x]) {
					out5[xx    ] = out6[xx    ] = (32 + _8_8(r)) << 16;
					out5[xx + 1] = out6[xx + 1] = (32 + _8_8(g)) << 8;
					out5[xx + 2] = out6[xx + 2] = (32 + _8_8(b));
				} else {
					out5[xx    ] = out6[xx    ] = (32 + _5_8(r)) << 16;
					out5[xx + 1] = out6[xx + 1] = (32 + _5_8(g)) << 8;
					out5[xx + 2] = out6[xx + 2] = (32 + _5_8(b));
				}
			}
		}
	}
}

void OSD::apply_crt_filter_x3_y2(screen_buffer_t *source, screen_buffer_t *dest)
{
	if(!screen_skip_line) {
		for(int y = 0, yy = 0; y < source->height; y++, yy += 2) {
			scrntype* src = get_buffer(source, y);
			scrntype* out1 = get_buffer(dest, yy + 0);
			scrntype* out2 = get_buffer(dest, yy + 1);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 3) {
				uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = (32 + _8_8(r)) << 16;
				out1[xx + 1] = (32 + _8_8(g)) << 8;
				out1[xx + 2] = (32 + _8_8(b));
				if(t0[x]) {
					out2[xx    ] = (32 + _8_8(r)) << 16;
					out2[xx + 1] = (32 + _8_8(g)) << 8;
					out2[xx + 2] = (32 + _8_8(b));
				} else {
					out2[xx    ] = (32 + _5_8(r)) << 16;
					out2[xx + 1] = (32 + _5_8(g)) << 8;
					out2[xx + 2] = (32 + _5_8(b));
				}
			}
		}
	} else {
		for(int y = 0, yy = 0; y < source->height; y += 2, yy += 4) {
			scrntype* src = get_buffer(source, y);
			scrntype* out1 = get_buffer(dest, yy + 0);
			scrntype* out2 = get_buffer(dest, yy + 1);
			scrntype* out3 = get_buffer(dest, yy + 2);
			scrntype* out4 = get_buffer(dest, yy + 3);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 3) {
				uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = out3[xx    ] = (32 + _8_8(r)) << 16;
				out1[xx + 1] = out2[xx + 1] = out3[xx + 1] = (32 + _8_8(g)) << 8;
				out1[xx + 2] = out2[xx + 2] = out3[xx + 2] = (32 + _8_8(b));
				if(t0[x]) {
					out4[xx    ] = (32 + _8_8(r)) << 16;
					out4[xx + 1] = (32 + _8_8(g)) << 8;
					out4[xx + 2] = (32 + _8_8(b));
				} else {
					out4[xx    ] = (32 + _5_8(r)) << 16;
					out4[xx + 1] = (32 + _5_8(g)) << 8;
					out4[xx + 2] = (32 + _5_8(b));
				}
			}
		}
	}
}

void OSD::apply_crt_filter_x2_y3(screen_buffer_t *source, screen_buffer_t *dest)
{
	if(!screen_skip_line) {
		for(int y = 0, yy = 0; y < source->height; y++, yy += 3) {
			scrntype* src = get_buffer(source, y);
			scrntype* out1 = get_buffer(dest, yy + 0);
			scrntype* out2 = get_buffer(dest, yy + 1);
			scrntype* out3 = get_buffer(dest, yy + 2);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 2) {
				uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				out1[xx + 1] = out2[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				if(t0[x]) {
					out3[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
					out3[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				} else {
					out3[xx    ] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
					out3[xx + 1] = RGB_COLOR(16 + _3_8(r), 16 + _3_8(g), 16 + _3_8(b));
				}
			}
		}
	} else {
		for(int y = 0, yy = 0; y < source->height; y += 2, yy += 6) {
			scrntype* src = get_buffer(source, y);
			scrntype* out1 = get_buffer(dest, yy + 0);
			scrntype* out2 = get_buffer(dest, yy + 1);
			scrntype* out3 = get_buffer(dest, yy + 2);
			scrntype* out4 = get_buffer(dest, yy + 3);
			scrntype* out5 = get_buffer(dest, yy + 4);
			scrntype* out6 = get_buffer(dest, yy + 5);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 2) {
				uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = out3[xx    ] = out4[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				out1[xx + 1] = out2[xx + 1] = out3[xx + 1] = out4[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				if(t0[x]) {
					out5[xx    ] = out6[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
					out5[xx + 1] = out6[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				} else {
					out5[xx    ] = out6[xx    ] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
					out5[xx + 1] = out6[xx + 1] = RGB_COLOR(16 + _3_8(r), 16 + _3_8(g), 16 + _3_8(b));
				}
			}
		}
	}
}

void OSD::apply_crt_filter_x2_y2(screen_buffer_t *source, screen_buffer_t *dest)
{
	if(!screen_skip_line) {
		for(int y = 0, yy = 0; y < source->height; y++, yy += 2) {
			scrntype* src = get_buffer(source, y);
			scrntype* out1 = get_buffer(dest, yy + 0);
			scrntype* out2 = get_buffer(dest, yy + 1);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 2) {
				uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				out1[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				if(t0[x]) {
					out2[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
					out2[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				} else {
					out2[xx    ] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
					out2[xx + 1] = RGB_COLOR(16 + _3_8(r), 16 + _3_8(g), 16 + _3_8(b));
				}
			}
		}
	} else {
		for(int y = 0, yy = 0; y < source->height; y += 2, yy += 4) {
			scrntype* src = get_buffer(source, y);
			scrntype* out1 = get_buffer(dest, yy + 0);
			scrntype* out2 = get_buffer(dest, yy + 1);
			scrntype* out3 = get_buffer(dest, yy + 2);
			scrntype* out4 = get_buffer(dest, yy + 3);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 2) {
				uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = out3[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				out1[xx + 1] = out2[xx + 1] = out3[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				if(t0[x]) {
					out4[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
					out4[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				} else {
					out4[xx    ] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
					out4[xx + 1] = RGB_COLOR(16 + _3_8(r), 16 + _3_8(g), 16 + _3_8(b));
				}
			}
		}
	}
}

void OSD::apply_crt_filter_x1_y1(screen_buffer_t *source, screen_buffer_t *dest)
{
	if(!screen_skip_line) {
		for(int y = 0; y < source->height; y++) {
			scrntype* src = get_buffer(source, y);
			scrntype* out1 = get_buffer(dest, y + 0);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype c = src[x - 1];
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1; x <= source->width; x++) {
				uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[x - 1] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
			}
		}
	} else {
		for(int y = 0; y < source->height; y += 2) {
			scrntype* src = get_buffer(source, y);
			scrntype* out1 = get_buffer(dest, y + 0);
			scrntype* out2 = get_buffer(dest, y + 1);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1; x <= source->width; x++) {
				uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[x - 1] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				if(t0[x]) {
					out2[x - 1] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				} else {
					out2[x - 1] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
				}
			}
		}
	}
}
#endif

#ifdef USE_SCREEN_ROTATE
void OSD::rotate_screen_buffer(screen_buffer_t *source, screen_buffer_t *dest)
{
}
#endif

void OSD::stretch_screen_buffer(screen_buffer_t *source, screen_buffer_t *dest)
{
}

void OSD::capture_screen()
{
	// create file name
	char file_name[_MAX_PATH];
	memset(file_name, 0x00, sizeof(file_name));
	create_date_file_name((_TCHAR *)file_name, _MAX_PATH, _T("png"));
	emit sig_save_screen((const char *)file_name); 
	// create bitmap
}

bool OSD::start_rec_video(int fps)
{
	if(fps > 0) {
		rec_video_fps = fps;
		rec_video_run_frames = rec_video_frames = 0;
	}
#if 0	
	bool show_dialog = (fps > 0);
	
	// initialize vfw
	create_date_file_name(video_file_name, _MAX_PATH, _T("avi"));

	AVIFileInit();
	if(AVIFileOpen(&pAVIFile, bios_path(video_file_name), OF_WRITE | OF_CREATE, NULL) != AVIERR_OK) {
		return false;
	}
	if(video_screen_buffer.width != vm_screen_buffer.width || video_screen_buffer.height != vm_screen_buffer.height) {
		initialize_screen_buffer(&video_screen_buffer, vm_screen_buffer.width, vm_screen_buffer.height, 0);
	}
	
	// stream header
	AVISTREAMINFO strhdr;
	memset(&strhdr, 0, sizeof(strhdr));
	strhdr.fccType = streamtypeVIDEO;	// vids
	strhdr.fccHandler = 0;
	strhdr.dwScale = 1;
	strhdr.dwRate = rec_video_fps;
	strhdr.dwSuggestedBufferSize = video_screen_buffer.lpDib->bmiHeader.biSizeImage;
	SetRect(&strhdr.rcFrame, 0, 0, video_screen_buffer.width, video_screen_buffer.height);
	if(AVIFileCreateStream(pAVIFile, &pAVIStream, &strhdr) != AVIERR_OK) {
		stop_rec_video();
		return false;
	}
	
	// compression
	AVICOMPRESSOPTIONS FAR * pOpts[1];
	pOpts[0] = &AVIOpts;
	if(show_dialog && !AVISaveOptions(main_window_handle, ICMF_CHOOSE_KEYFRAME | ICMF_CHOOSE_DATARATE, 1, &pAVIStream, (LPAVICOMPRESSOPTIONS FAR *)&pOpts)) {
		AVISaveOptionsFree(1, (LPAVICOMPRESSOPTIONS FAR *)&pOpts);
		stop_rec_video();
		return false;
	}
	if(AVIMakeCompressedStream(&pAVICompressed, pAVIStream, &AVIOpts, NULL) != AVIERR_OK) {
		stop_rec_video();
		return false;
	}
	if(AVIStreamSetFormat(pAVICompressed, 0, &video_screen_buffer.lpDib->bmiHeader, video_screen_buffer.lpDib->bmiHeader.biSize + video_screen_buffer.lpDib->bmiHeader.biClrUsed * sizeof(RGBQUAD)) != AVIERR_OK) {
		stop_rec_video();
		return false;
	}
	dwAVIFileSize = 0;
	lAVIFrames = 0;
	
	hVideoThread = (HANDLE)0;
	rec_video_thread_param.pAVICompressed = pAVICompressed;
	rec_video_thread_param.lpBmp = video_screen_buffer.lpBmp;
	rec_video_thread_param.pbmInfoHeader = &video_screen_buffer.lpDib->bmiHeader;
	rec_video_thread_param.dwAVIFileSize = 0;
	rec_video_thread_param.lAVIFrames = 0;
	rec_video_thread_param.frames = 0;
	rec_video_thread_param.result = 0;
#endif	
	now_rec_video = true;
	return true;
}

void OSD::stop_rec_video()
{
#if 0
	// release thread
	if(hVideoThread != (HANDLE)0) {
		WaitForSingleObject(hVideoThread, INFINITE);
		hVideoThread = (HANDLE)0;
	}
	
	// release vfw
	if(pAVIStream) {
		AVIStreamClose(pAVIStream);
	}
	if(pAVICompressed) {
		AVIStreamClose(pAVICompressed);
	}
	if(pAVIFile) {
		AVIFileClose(pAVIFile);
		AVIFileExit();
	}
	pAVIStream = NULL;
	pAVICompressed = NULL;
	pAVIFile = NULL;
	
	// repair header
	if(now_rec_video) {
		FILE* fp = NULL;
		if((fp = _tfopen(bios_path(video_file_name), _T("r+b"))) != NULL) {
			// copy fccHandler
			uint8 buf[4];
			fseek(fp, 0xbc, SEEK_SET);
			if(ftell(fp) == 0xbc) {
				fread(buf, 4, 1, fp);
				fseek(fp, 0x70, SEEK_SET);
				fwrite(buf, 4, 1, fp);
			}
			fclose(fp);
		}
	}
#endif	
	now_rec_video = false;
}

void OSD::restart_rec_video()
{
	bool tmp = now_rec_video;
	stop_rec_video();
	if(tmp) {
		start_rec_video(-1);
	}
}

void OSD::add_extra_frames(int extra_frames)
{
	rec_video_run_frames += extra_frames;
}

int OSD::add_video_frames()
{
	static double frames = 0;
	static int prev_video_fps = -1;
	int counter = 0;
#if 0	
#ifdef SUPPORT_VARIABLE_TIMING
	static double prev_vm_fps = -1;
	double vm_fps = vm->frame_rate();
	
	if(prev_video_fps != rec_video_fps || prev_vm_fps != vm_fps) {
		prev_video_fps = rec_video_fps;
		prev_vm_fps = vm_fps;
		frames = vm_fps / rec_video_fps;
	}
#else
	if(prev_video_fps != rec_video_fps) {
		prev_video_fps = rec_video_fps;
		frames = FRAMES_PER_SEC / rec_video_fps;
	}
#endif
	while(rec_video_run_frames > 0) {
		rec_video_run_frames -= frames;
		rec_video_frames += frames;
		counter++;
	}
	if(counter != 0) {
		if(hVideoThread != (HANDLE)0) {
			if(rec_video_thread_param.result == 0) {
				WaitForSingleObject(hVideoThread, INFINITE);
			}
			hVideoThread = (HANDLE)0;
			
			if(rec_video_thread_param.result == REC_VIDEO_FULL) {
				stop_rec_video();
				if(!start_rec_video(-1)) {
					return 0;
				}
			} else if(rec_video_thread_param.result == REC_VIDEO_ERROR) {
				stop_rec_video();
				return 0;
			}
		}
//		BitBlt(vm_screen_buffer.hdcDib, 0, 0, vm_screen_buffer.width, vm_screen_buffer.height, video_screen_buffer.hdcDib, 0, 0, SRCCOPY);
		memcpy(video_screen_buffer.lpBmp, vm_screen_buffer.lpBmp, sizeof(scrntype) * vm_screen_buffer.width * vm_screen_buffer.height);
		
		rec_video_thread_param.frames += counter;
		rec_video_thread_param.result = 0;
		if((hVideoThread = (HANDLE)_beginthreadex(NULL, 0, rec_video_thread, &rec_video_thread_param, 0, NULL)) == (HANDLE)0) {
			stop_rec_video();
			return 0;
		}
	}
#endif	
	return counter;
}

