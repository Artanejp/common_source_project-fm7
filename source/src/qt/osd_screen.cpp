/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt screen ]
*/

#include <QImage>
#include <QPainter>
#include <QColor>
#include <QPen>
#include <QPoint>
#include <QTextCodec>

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
	vm_window_width = WINDOW_WIDTH;
	vm_window_height = WINDOW_HEIGHT;
	vm_window_width_aspect = WINDOW_WIDTH_ASPECT;
	vm_window_height_aspect = WINDOW_HEIGHT_ASPECT;
	
	QColor col(0, 0, 0, 255);
	//memset(&vm_screen_buffer, 0, sizeof(bitmap_t));
	vm_screen_buffer.width = SCREEN_WIDTH;
	vm_screen_buffer.height = SCREEN_HEIGHT;
	vm_screen_buffer.pImage = QImage(SCREEN_WIDTH, SCREEN_HEIGHT, QImage::Format_ARGB32);
	vm_screen_buffer.pImage.fill(col);
	now_record_video = false;
	//pAVIStream = NULL;
	//pAVICompressed = NULL;
	//pAVIFile = NULL;
	
	first_draw_screen = false;
	first_invalidate = true;
	self_invalidate = false;
}

void OSD::release_screen()
{
	stop_record_video();
	
	//release_d3d9();
	//if(vm_screen_buffer.pImage != NULL) delete vm_screen_buffer.pImage;
	release_screen_buffer(&vm_screen_buffer);
}

int OSD::get_window_mode_width(int mode)
{
#ifdef USE_SCREEN_ROTATE
	if(config.rotate_type == 1 || config.rotate_type == 3) {
		return (config.window_stretch_type == 0 ? vm_window_height : vm_window_height_aspect) * (mode + WINDOW_MODE_BASE);
	}
#endif
	return (config.window_stretch_type == 0 ? vm_window_width : vm_window_width_aspect) * (mode + WINDOW_MODE_BASE);
}

int OSD::get_window_mode_height(int mode)
{
#ifdef USE_SCREEN_ROTATE
	if(config.rotate_type == 1 || config.rotate_type == 3) {
		return (config.window_stretch_type == 0 ? vm_window_width : vm_window_width_aspect) * (mode + WINDOW_MODE_BASE);
	}
#endif
	return (config.window_stretch_type == 0 ? vm_window_height : vm_window_height_aspect) * (mode + WINDOW_MODE_BASE);
}

void OSD::set_host_window_size(int window_width, int window_height, bool window_mode)
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

void OSD::set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect)
{
	if(vm_screen_width != screen_width || vm_screen_height != screen_height) {
		if(window_width == -1) {
			window_width = screen_width;
		}
		if(window_height == -1) {
			window_height = screen_height;
		}
		if(window_width_aspect == -1) {
			window_width_aspect = window_width;
		}
		if(window_height_aspect == -1) {
			window_height_aspect = window_height;
		}
		vm_screen_width = screen_width;
		vm_screen_height = screen_height;
		vm_window_width = window_width;
		vm_window_height = window_height;
		vm_window_width_aspect = window_width_aspect;
		vm_window_height_aspect = window_height_aspect;
		
		// change the window size
		//emit sig_resize_vm_screen(&(vm_screen_buffer.pImage), vm_window_width, vm_window_height);
		emit sig_resize_vm_screen(&(vm_screen_buffer.pImage), screen_width, screen_height);
	}
//	if(vm_screen_buffer.width != vm_screen_width || vm_screen_buffer.height != vm_screen_height) {
//		if(now_record_video) {
//			stop_record_video();
////			stop_record_sound();
//		}
//		//initialize_screen_buffer(&vm_screen_buffer, vm_screen_width, vm_screen_height, COLORONCOLOR);
//		initialize_screen_buffer(&vm_screen_buffer, vm_screen_width, vm_screen_height, 0);
//		emit sig_resize_vm_screen(&(vm_screen_buffer.pImage), vm_window_width, vm_window_height);
//	}
}


scrntype_t* OSD::get_vm_screen_buffer(int y)
{
	return get_buffer(&vm_screen_buffer, y);
}

scrntype_t* OSD::get_buffer(bitmap_t *p, int y)
{
	if((y >= p->pImage.height()) || (y < 0) || (y >= p->height)) {
		return NULL;
	}
	return (scrntype_t *)p->pImage.scanLine(y);
}

int OSD::draw_screen()
{
	// check avi file recording timing
	if(now_record_video && rec_video_run_frames <= 0) {
		return 0;
	}
	
	// draw screen
	lock_vm();
	if(vm_screen_buffer.width != vm_screen_width || vm_screen_buffer.height != vm_screen_height) {
		if(now_record_video) {
			stop_record_video();
//			stop_record_sound();
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
	if(now_record_video) {
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

void OSD::initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode)
{
	release_screen_buffer(buffer);
	buffer->width = width;
	buffer->height = height;
	if((width > buffer->pImage.width()) || (height > buffer->pImage.height())) {
		QColor col(0, 0, 0, 255);
		buffer->pImage = QImage(width, height, QImage::Format_ARGB32);
		buffer->pImage.fill(col);
	}
	//printf("%dx%d NULL=%d\n", buffer->pImage.width(), buffer->pImage.height(), buffer->pImage.isNull() ? 1 : 0);
	QColor fillcolor;
	fillcolor.setRgb(0, 0, 0, 255);
	buffer->pImage.fill(fillcolor);
	emit sig_resize_vm_screen(&(buffer->pImage), width, height);
}

void OSD::release_screen_buffer(bitmap_t *buffer)
{
	if(!(buffer->width == 0 && buffer->height == 0)) {
		//if(buffer->hPainter == NULL) delete buffer->hPainter;
	}
	buffer->width = 0;
	buffer->height = 0;
	//memset(buffer, 0, sizeof(bitmap_t));
}


void OSD::rotate_screen_buffer(bitmap_t *source, bitmap_t *dest)
{
}

void OSD::stretch_screen_buffer(bitmap_t *source, bitmap_t *dest)
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

bool OSD::start_record_video(int fps)
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
		stop_record_video();
		return false;
	}
	
	// compression
	AVICOMPRESSOPTIONS FAR * pOpts[1];
	pOpts[0] = &AVIOpts;
	if(show_dialog && !AVISaveOptions(main_window_handle, ICMF_CHOOSE_KEYFRAME | ICMF_CHOOSE_DATARATE, 1, &pAVIStream, (LPAVICOMPRESSOPTIONS FAR *)&pOpts)) {
		AVISaveOptionsFree(1, (LPAVICOMPRESSOPTIONS FAR *)&pOpts);
		stop_record_video();
		return false;
	}
	if(AVIMakeCompressedStream(&pAVICompressed, pAVIStream, &AVIOpts, NULL) != AVIERR_OK) {
		stop_record_video();
		return false;
	}
	if(AVIStreamSetFormat(pAVICompressed, 0, &video_screen_buffer.lpDib->bmiHeader, video_screen_buffer.lpDib->bmiHeader.biSize + video_screen_buffer.lpDib->bmiHeader.biClrUsed * sizeof(RGBQUAD)) != AVIERR_OK) {
		stop_record_video();
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
	now_record_video = true;
	return true;
}

void OSD::stop_record_video()
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
	if(now_record_video) {
		FILE* fp = NULL;
		if((fp = _tfopen(bios_path(video_file_name), _T("r+b"))) != NULL) {
			// copy fccHandler
			uint8_t buf[4];
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
	now_record_video = false;
}

void OSD::restart_record_video()
{
	bool tmp = now_record_video;
	stop_record_video();
	if(tmp) {
		start_record_video(-1);
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
				stop_record_video();
				if(!start_record_video(-1)) {
					return 0;
				}
			} else if(rec_video_thread_param.result == REC_VIDEO_ERROR) {
				stop_record_video();
				return 0;
			}
		}
//		BitBlt(vm_screen_buffer.hdcDib, 0, 0, vm_screen_buffer.width, vm_screen_buffer.height, video_screen_buffer.hdcDib, 0, 0, SRCCOPY);
		memcpy(video_screen_buffer.lpBmp, vm_screen_buffer.lpBmp, sizeof(scrntype_t) * vm_screen_buffer.width * vm_screen_buffer.height);
		
		rec_video_thread_param.frames += counter;
		rec_video_thread_param.result = 0;
		if((hVideoThread = (HANDLE)_beginthreadex(NULL, 0, rec_video_thread, &rec_video_thread_param, 0, NULL)) == (HANDLE)0) {
			stop_record_video();
			return 0;
		}
	}
#endif	
	return counter;
}

//#ifdef USE_PRINTER
void OSD::create_bitmap(bitmap_t *bitmap, int width, int height)
{
	QRect rect;
	QColor col = QColor(0, 0, 0, 255);
	initialize_screen_buffer(bitmap, width, height, 0); // HALFTONE
	bitmap->hPainter.begin(&(bitmap->pImage));
	bitmap->hPainter.fillRect(0, 0, width, height, col);
	bitmap->hPainter.end();
	AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Create bitmap: %08x %d x %d", bitmap, width, height);
}

void OSD::release_bitmap(bitmap_t *bitmap)
{
	release_screen_buffer(bitmap);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Release bitmap: %08x", bitmap);
}

void OSD::create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic)
{
	QString fontName;
	fontName = QString::fromUtf8(family);
	if(fontName == QString::fromUtf8("Gothic")) {
		fontName = QString::fromUtf8("Sans Serif");
	} else if(fontName == QString::fromUtf8("Mincho")) {
		fontName = QString::fromUtf8("Serif");
	} else {
		//fontName = QString::fromUtf8("Sans Serif");
		fontName = QString::fromUtf8(family);
	}
	font->hFont = QFont(fontName);
	font->hFont.setPixelSize(height);
	//font->hFont.setFixedPitch(true);
	font->hFont.setItalic(italic);
	font->hFont.setBold(bold);	
	QFontMetrics metric(font->hFont);
	font->hFont.setStretch((width * 10000) / (metric.width("F") * 100));
	font->rotate = rotate;
	font->init_flag = true;
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Create Font: Family=%s WIDTH=%d HEIGHT=%d",fontName.toUtf8().constData(), width, height);
	// Qt uses substitution font if not found.
}

void OSD::release_font(font_t *font)
{
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Release Font");
}

void OSD::create_pen(pen_t *pen, int width, uint8_t r, uint8_t g, uint8_t b)
{
	pen->r = r;
	pen->g = g;
	pen->b = b;
	pen->width = width;
	pen->hPen = QPen(Qt::SolidLine);
	QColor color = QColor(r, g, b, 255);
	pen->hPen.setColor(color);
	pen->hPen.setWidth(width);
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Create PEN %08x to width=%d (RGB)=(%d,%d,%d)\n", pen, width, r, g, b);
}

void OSD::release_pen(pen_t *pen)
{
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Release Pen %08x", pen);
}

void OSD::clear_bitmap(bitmap_t *bitmap, uint8_t r, uint8_t g, uint8_t b)
{
	//lock_vm();
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Clear bitmap %08x", bitmap);
	draw_rectangle_to_bitmap(bitmap, 0, 0, bitmap->width, bitmap->height, r, g, b);
	//unlock_vm();
}

int OSD::get_text_width(bitmap_t *bitmap, font_t *font, const char *text)
{
	QFontMetrics fm(font->hFont);
	QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
	QString s = codec->toUnicode(text);
	return fm.width(s);
}

void OSD::draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const _TCHAR *text, uint8_t r, uint8_t g, uint8_t b)
{
	QColor col(r, g, b, 255);
	QPoint loc = QPoint(x, y);
	
	QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
	QString str = codec->toUnicode(text);
	bitmap->hPainter.begin(&(bitmap->pImage));
	//bitmap->hPainter.setBackgroundMode(Qt::OpaqueMode);
	bitmap->hPainter.setPen(col);
	bitmap->hPainter.rotate((qreal)font->rotate);

	bitmap->hPainter.setFont(font->hFont);
	bitmap->hPainter.drawText(loc, str);
	bitmap->hPainter.end();
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Draw Text to BITMAP %08x : (%d,%d) %s : Color(%d,%d,%d)", bitmap, x, y, str.toUtf8().constData(), r, g, b);
}

void OSD::draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey)
{
	QLine line(sx, sy, ex, ey);
	bitmap->hPainter.begin(&(bitmap->pImage));
	//bitmap->hPainter.setBackgroundMode(Qt::TransparentMode);
	bitmap->hPainter.setPen(pen->hPen);
	bitmap->hPainter.drawLine(line);
	bitmap->hPainter.end();
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Draw Line from (%d,%d) to (%d,%d) to BITMAP %08x", sx, sy, ex, ey, bitmap);
}

void OSD::draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	QPoint point(x, y);
	QColor d_col(r, g, b);
	QPen d_pen(d_col);
	bitmap->hPainter.begin(&(bitmap->pImage));
	//bitmap->hPainter.setBackgroundMode(Qt::OpaqueMode);
	bitmap->hPainter.setPen(d_pen);
	bitmap->hPainter.drawPoint(point);
	bitmap->hPainter.end();
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Draw Point to BITMAP %08x :(%d,%d) Color=(%d,%d,%d)", bitmap, x, y, r, g, b);
}

void OSD::draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b)
{
	QColor d_col(r, g, b, 255);
	QBrush d_brush(d_col);
	bitmap->hPainter.begin(&(bitmap->pImage));
	//bitmap->hPainter.setBackgroundMode(Qt::OpaqueMode);
	bitmap->hPainter.fillRect(x, y, width, height, d_brush);
	bitmap->hPainter.end();
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Draw Rect BITMAP %08x :from (%d,%d) size=(%d,%d) Color=(%d,%d,%d)", bitmap, x, y, width, height, r, g, b);
}

void OSD::stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t *source, int source_x, int source_y, int source_width, int source_height)
{
	QRect src_r(source_x, source_y, source_width, source_height);
	QRect dest_r(dest_x, dest_y, dest_width, dest_height);
	dest->hPainter.begin(&(dest->pImage));
	//dest->hPainter.setBackgroundMode(Qt::OpaqueMode);
	dest->hPainter.drawImage(dest_r, source->pImage, src_r);
	dest->hPainter.end();
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Scale (%d,%d, %d, %d) to (%d,%d, %d, %d)", source_x, source_y, source_width, source_height,
	//	     dest_x, dest_y, dest_width, dest_height);
}
//#endif


void OSD::write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path)
{
	int comp_quality = 0;
	bitmap->pImage.save(QString::fromUtf8(file_path), "PNG", comp_quality);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "PRINTER: Save bitmap %08x to %s", bitmap, file_path);
}
