/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 main ]
*/

#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <commdlg.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <stdio.h>
#include "res/resource.h"
#include "emu.h"

// emulation core
EMU* emu;

// buttons
#ifdef USE_BUTTON
#define MAX_FONT_SIZE 32
HFONT hFont[MAX_FONT_SIZE];
HWND hButton[MAX_BUTTONS];
WNDPROC buttonWndProc[MAX_BUTTONS];
#endif

// menu
static HMENU hMenu = NULL;
bool now_menuloop = false;

void update_menu(HWND hWnd, HMENU hMenu, int pos);

void show_menu_bar(HWND hWnd)
{
	if(!(hMenu != NULL && IsMenu(hMenu))) {
		hMenu = LoadMenu((HINSTANCE)GetModuleHandle(0), MAKEINTRESOURCE(IDR_MENU1));
		SetMenu(hWnd, hMenu);
	}
}

void hide_menu_bar(HWND hWnd)
{
	if(hMenu != NULL && IsMenu(hMenu)) {
		SetMenu(hWnd, NULL);
		DestroyMenu(hMenu);
		hMenu = NULL;
	}
}

// dialog
#ifdef USE_CART1
void open_cart_dialog(HWND hWnd, int drv);
#endif
#ifdef USE_FD1
void open_disk_dialog(HWND hWnd, int drv);
void open_disk(int drv, _TCHAR* path, int bank);
void close_disk(int drv);
#endif
#ifdef USE_QD1
void open_quickdisk_dialog(HWND hWnd, int drv);
#endif
#ifdef USE_TAPE
void open_tape_dialog(HWND hWnd, bool play);
#endif
#ifdef USE_LASER_DISC
void open_laser_disc_dialog(HWND hWnd);
#endif
#ifdef USE_BINARY_FILE1
void open_binary_dialog(HWND hWnd, int drv, bool load);
#endif

#if defined(USE_CART1) || defined(USE_FD1) || defined(USE_TAPE) || defined(USE_BINARY_FILE1)
#define SUPPORT_DRAG_DROP
#endif
#ifdef SUPPORT_DRAG_DROP
void open_any_file(_TCHAR* path);
#endif

void get_long_full_path_name(_TCHAR* src, _TCHAR* dst)
{
	_TCHAR tmp[_MAX_PATH];
	
	if(GetFullPathName(src, _MAX_PATH, tmp, NULL) == 0) {
		_tcscpy(dst, src);
	} else if(GetLongPathName(tmp, dst, _MAX_PATH) == 0) {
		_tcscpy(dst, tmp);
	}
}

_TCHAR* get_parent_dir(_TCHAR* file)
{
	static _TCHAR path[_MAX_PATH];
	_TCHAR *ptr;
	
	GetFullPathName(file, _MAX_PATH, path, &ptr);
	*ptr = _T('\0');
	return path;
}

_TCHAR* get_open_file_name(HWND hWnd, _TCHAR* filter, _TCHAR* title, _TCHAR* dir)
{
	static _TCHAR path[_MAX_PATH];
	_TCHAR tmp[_MAX_PATH] = _T("");
	OPENFILENAME OpenFileName;
	
	memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = hWnd;
	OpenFileName.lpstrFilter = filter;
	OpenFileName.lpstrFile = tmp;
	OpenFileName.nMaxFile = _MAX_PATH;
	OpenFileName.lpstrTitle = title;
	if(dir[0]) {
		OpenFileName.lpstrInitialDir = dir;
	} else {
		_TCHAR app[_MAX_PATH];
		GetModuleFileName(NULL, app, _MAX_PATH);
		OpenFileName.lpstrInitialDir = get_parent_dir(app);
	}
	if(GetOpenFileName(&OpenFileName)) {
		get_long_full_path_name(OpenFileName.lpstrFile, path);
		_tcscpy(dir, get_parent_dir(path));
		return path;
	}
	return NULL;
}

#define UPDATE_HISTORY(path, recent) { \
	int no = MAX_HISTORY - 1; \
	for(int i = 0; i < MAX_HISTORY; i++) { \
		if(_tcsicmp(recent[i], path) == 0) { \
			no = i; \
			break; \
		} \
	} \
	for(int i = no; i > 0; i--) { \
		_tcscpy(recent[i], recent[i - 1]); \
	} \
	_tcscpy(recent[0], path); \
}

// screen
int desktop_width;
int desktop_height;
int desktop_bpp;
int prev_window_mode = 0;
bool now_fullscreen = false;

#define MAX_WINDOW	8
#define MAX_FULLSCREEN	32

int window_mode_count;
int screen_mode_count;
int screen_mode_width[MAX_FULLSCREEN];
int screen_mode_height[MAX_FULLSCREEN];

void set_window(HWND hWnd, int mode);

// timing control
#define MAX_SKIP_FRAMES 10

int get_interval()
{
	static int accum = 0;
	accum += emu->frame_interval();
	int interval = accum >> 10;
	accum -= interval << 10;
	return interval;
}

// windows main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
#ifdef USE_BUTTON
LRESULT CALLBACK ButtonWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow)
{
	// load config
	load_config();
	
	// create window
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hCursor = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wndclass.lpszClassName = _T("CWINDOW");
	RegisterClass(&wndclass);
	
	// get window position
	RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
	AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, TRUE);
	HDC hdcScr = GetDC(NULL);
	desktop_width = GetDeviceCaps(hdcScr, HORZRES);
	desktop_height = GetDeviceCaps(hdcScr, VERTRES);
	desktop_bpp = GetDeviceCaps(hdcScr, BITSPIXEL);
	ReleaseDC(NULL, hdcScr);
	int dest_x = (int)((desktop_width - (rect.right - rect.left)) / 2);
	int dest_y = (int)((desktop_height - (rect.bottom - rect.top)) / 2);
	//dest_x = (dest_x < 0) ? 0 : dest_x;
	dest_y = (dest_y < 0) ? 0 : dest_y;
	
	// show window
	HWND hWnd = CreateWindow(_T("CWINDOW"), _T(DEVICE_NAME), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
	                         dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);
	
	// show menu
	show_menu_bar(hWnd);
	
	RECT rect_tmp;
	GetClientRect(hWnd, &rect_tmp);
	if(rect_tmp.bottom != WINDOW_HEIGHT) {
		rect.bottom += WINDOW_HEIGHT - rect_tmp.bottom;
		dest_y = (int)((desktop_height - (rect.bottom - rect.top)) / 2);
		dest_y = (dest_y < 0) ? 0 : dest_y;
		SetWindowPos(hWnd, NULL, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
	}
	
	// enumerate screen mode
	screen_mode_count = 0;
	if(WINDOW_WIDTH <= 320 && WINDOW_HEIGHT <= 240) {
		// FIXME: need to check if your video card really supports 320x240 resolution
		screen_mode_width[0] = 320;
		screen_mode_height[0] = 240;
		screen_mode_count = 1;
	}
	for(int i = 0;; i++) {
		DEVMODE dev;
		ZeroMemory(&dev, sizeof(dev));
		dev.dmSize = sizeof(dev);
		if(EnumDisplaySettings(NULL, i, &dev) == 0) {
			break;
		}
		if(dev.dmPelsWidth >= WINDOW_WIDTH && dev.dmPelsHeight >= WINDOW_HEIGHT) {
			if(dev.dmPelsWidth >= 640 && dev.dmPelsHeight >= 480) {
				bool found = false;
				for(int j = 0; j < screen_mode_count; j++) {
					if(screen_mode_width[j] == dev.dmPelsWidth && screen_mode_height[j] == dev.dmPelsHeight) {
						found = true;
						break;
					}
				}
				if(!found) {
					screen_mode_width[screen_mode_count] = dev.dmPelsWidth;
					screen_mode_height[screen_mode_count] = dev.dmPelsHeight;
					if(++screen_mode_count == MAX_FULLSCREEN) {
						break;
					}
				}
			}
		}
	}
	for(int i = 0; i < screen_mode_count - 1; i++) {
		for(int j = i + 1; j < screen_mode_count; j++) {
			if(screen_mode_width[i] > screen_mode_width[j] || (screen_mode_width[i] == screen_mode_width[j] && screen_mode_height[i] > screen_mode_height[j])) {
				int width = screen_mode_width[i];
				screen_mode_width[i] = screen_mode_width[j];
				screen_mode_width[j] = width;
				int height = screen_mode_height[i];
				screen_mode_height[i] = screen_mode_height[j];
				screen_mode_height[j] = height;
			}
		}
	}
	if(screen_mode_count == 0) {
		screen_mode_width[0] = desktop_width;
		screen_mode_height[0] = desktop_height;
		screen_mode_count = 1;
	}
	
	// restore screen mode
	if(config.window_mode >= 0 && config.window_mode < MAX_WINDOW) {
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_WINDOW1 + config.window_mode, 0L);
	} else if(config.window_mode >= MAX_WINDOW && config.window_mode < screen_mode_count + MAX_WINDOW) {
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_FULLSCREEN1 + config.window_mode - MAX_WINDOW, 0L);
	} else {
		config.window_mode = 0;
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_WINDOW1, 0L);
	}
	
	// accelerator
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	
	// disenable ime
	ImmAssociateContext(hWnd, 0);
	
	// initialize emulation core
	emu = new EMU(hWnd, hInstance);
	emu->set_display_size(WINDOW_WIDTH, WINDOW_HEIGHT, true);
	
#ifdef SUPPORT_DRAG_DROP
	// open command line path
	if(szCmdLine[0]) {
		if(szCmdLine[0] == _T('"')) {
			int len = _tcslen(szCmdLine);
			szCmdLine[len - 1] = _T('\0');
			szCmdLine++;
		}
		_TCHAR path[_MAX_PATH];
		get_long_full_path_name(szCmdLine, path);
		open_any_file(path);
	}
#endif
	
	// set priority
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
	
	// main loop
	int total_frames = 0, draw_frames = 0, skip_frames = 0;
	DWORD next_time = 0;
	DWORD update_fps_time = 0;
	bool prev_skip = false;
	MSG msg;
	
	while(1) {
		// check window message
		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if(!GetMessage(&msg, NULL, 0, 0)) {
#ifdef _DEBUG
				_CrtDumpMemoryLeaks();
#endif
				ExitProcess(0);	// trick
				return msg.wParam;
			}
			if(!TranslateAccelerator(hWnd, hAccel, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else if(emu) {
			// drive machine
			int run_frames = emu->run();
			total_frames += run_frames;
			
			// timing controls
			int interval = 0, sleep_period = 0;
//			for(int i = 0; i < run_frames; i++) {
				interval += get_interval();
//			}
			bool now_skip = emu->now_skip() && !emu->now_rec_video;
			
			if((prev_skip && !now_skip) || next_time == 0) {
				next_time = timeGetTime();
			}
			if(!now_skip) {
				next_time += interval;
			}
			prev_skip = now_skip;
			
			if(next_time > timeGetTime()) {
				// update window if enough time
				draw_frames += emu->draw_screen();
				skip_frames = 0;
				
				// sleep 1 frame priod if need
				DWORD current_time = timeGetTime();
				if((int)(next_time - current_time) >= 10) {
					sleep_period = next_time - current_time;
				}
			} else if(++skip_frames > MAX_SKIP_FRAMES) {
				// update window at least once per 10 frames
				draw_frames += emu->draw_screen();
				skip_frames = 0;
				next_time = timeGetTime();
			}
			Sleep(sleep_period);
			
			// calc frame rate
			DWORD current_time = timeGetTime();
			if(update_fps_time <= current_time && update_fps_time != 0) {
				_TCHAR buf[256];
				int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);
				if(emu->message_count > 0) {
					_stprintf(buf, _T("%s - %s"), _T(DEVICE_NAME), emu->message);
					emu->message_count--;
				} else {
					_stprintf(buf, _T("%s - %d fps (%d %%)"), _T(DEVICE_NAME), draw_frames, ratio);
				}
				SetWindowText(hWnd, buf);
				
				update_fps_time += 1000;
				total_frames = draw_frames = 0;
			}
			if(update_fps_time <= current_time) {
				update_fps_time = current_time + 1000;
			}
		}
	}
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	_TCHAR path[_MAX_PATH];
	int no;
	
	switch(iMsg) {
	case WM_CREATE:
#ifdef USE_BUTTON
		memset(hFont, 0, sizeof(hFont));
		for(int i = 0; i < MAX_BUTTONS; i++) {
			hButton[i] = CreateWindow(_T("BUTTON"), buttons[i].caption,
			                          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_MULTILINE,
			                          buttons[i].x, buttons[i].y,
			                          buttons[i].width, buttons[i].height,
			                          hWnd, (HMENU)(ID_BUTTON + i), (HINSTANCE)GetModuleHandle(0), NULL);
			buttonWndProc[i] = (WNDPROC)(LONG_PTR)GetWindowLong(hButton[i], GWL_WNDPROC);
			SetWindowLong(hButton[i], GWL_WNDPROC, (LONG)(LONG_PTR)ButtonWndProc);
			//HFONT hFont = GetWindowFont(hButton[i]);
			if(!hFont[buttons[i].font_size]) {
				LOGFONT logfont;
				logfont.lfEscapement = 0;
				logfont.lfOrientation = 0;
				logfont.lfWeight = FW_NORMAL;
				logfont.lfItalic = FALSE;
				logfont.lfUnderline = FALSE;
				logfont.lfStrikeOut = FALSE;
				logfont.lfCharSet = DEFAULT_CHARSET;
				logfont.lfOutPrecision = OUT_TT_PRECIS;
				logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
				logfont.lfQuality = DEFAULT_QUALITY;
				logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
				_tcscpy(logfont.lfFaceName, _T("Arial"));
				logfont.lfHeight = buttons[i].font_size;
				logfont.lfWidth = buttons[i].font_size >> 1;
				hFont[buttons[i].font_size] = CreateFontIndirect(&logfont);
			}
			SetWindowFont(hButton[i], hFont[buttons[i].font_size], TRUE);
		}
#endif
#ifdef SUPPORT_DRAG_DROP
		DragAcceptFiles(hWnd, TRUE);
#endif
		break;
	case WM_CLOSE:
#ifdef USE_POWER_OFF
		// notify power off
		if(emu) {
			static int notified = 0;
			if(!notified) {
				emu->notify_power_off();
				notified = 1;
				return 0;
			}
		}
#endif
		// release window
		if(now_fullscreen) {
			ChangeDisplaySettings(NULL, 0);
		}
		now_fullscreen = false;
#ifdef USE_BUTTON
		for(int i = 0; i < MAX_FONT_SIZE; i++) {
			if(hFont[i]) {
				DeleteObject(hFont[i]);
			}
		}
#endif
		if(hMenu != NULL && IsMenu(hMenu)) {
			DestroyMenu(hMenu);
		}
		DestroyWindow(hWnd);
		// release emulation core
		if(emu) {
			delete emu;
			emu = NULL;
		}
		save_config();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
#ifdef USE_BITMAP
	case WM_SIZE:
		if(emu) {
			emu->reload_bitmap();
		}
		break;
#endif
	case WM_KILLFOCUS:
		if(emu) {
			emu->key_lost_focus();
		}
		break;
	case WM_PAINT:
		if(emu) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			emu->update_screen(hdc);
			EndPaint(hWnd, &ps);
		}
		return 0;
	case WM_MOVING:
		if(emu) {
			emu->suspend();
		}
		break;
	case WM_KEYDOWN:
		if(emu) {
			bool repeat = ((HIWORD(lParam) & 0x4000) != 0);
			emu->key_down(LOBYTE(wParam), repeat);
		}
		break;
	case WM_KEYUP:
		if(emu) {
			emu->key_up(LOBYTE(wParam));
		}
		break;
	case WM_SYSKEYDOWN:
		if(emu) {
			bool repeat = ((HIWORD(lParam) & 0x4000) != 0);
			emu->key_down(LOBYTE(wParam), repeat);
		}
#ifdef USE_ALT_F10_KEY
		return 0;	// not activate menu when hit ALT/F10
#endif
		break;
	case WM_SYSKEYUP:
		if(emu) {
			emu->key_up(LOBYTE(wParam));
		}
#ifdef USE_ALT_F10_KEY
		return 0;	// not activate menu when hit ALT/F10
#endif
		break;
	case WM_SYSCHAR:
#ifdef USE_ALT_F10_KEY
		return 0;	// not activate menu when hit ALT/F10
#endif
		break;
	case WM_INITMENUPOPUP:
		if(emu) {
			emu->suspend();
		}
		update_menu(hWnd, (HMENU)wParam, LOWORD(lParam));
		break;
	case WM_ENTERMENULOOP:
		now_menuloop = true;
		break;
	case WM_EXITMENULOOP:
		if(now_fullscreen && now_menuloop) {
			hide_menu_bar(hWnd);
		}
		now_menuloop = false;
		break;
	case WM_MOUSEMOVE:
		if(now_fullscreen && !now_menuloop) {
			POINTS p = MAKEPOINTS(lParam);
			if(p.y == 0) {
				show_menu_bar(hWnd);
			} else if(p.y > 32) {
				hide_menu_bar(hWnd);
			}
		}
		break;
	case WM_RESIZE:
		if(emu) {
			if(now_fullscreen) {
				emu->set_display_size(-1, -1, false);
			} else {
				set_window(hWnd, config.window_mode);
			}
		}
		break;
#ifdef SUPPORT_DRAG_DROP
	case WM_DROPFILES:
		if(emu) {
			HDROP hDrop = (HDROP)wParam;
			if(DragQueryFile((HDROP)wParam, 0xFFFFFFFF, NULL, 0) == 1) {
				DragQueryFile(hDrop, 0, path, _MAX_PATH);
				open_any_file(path);
			}
			DragFinish(hDrop);
		}
		break;
#endif
#ifdef USE_SOCKET
	case WM_SOCKET0:
	case WM_SOCKET1:
	case WM_SOCKET2:
	case WM_SOCKET3:
		no = iMsg - WM_SOCKET0;
		if(!emu) {
			break;
		}
		if(WSAGETSELECTERROR(lParam) != 0) {
			emu->disconnect_socket(no);
			emu->socket_disconnected(no);
			break;
		}
		if(emu->get_socket(no) != (int)wParam) {
			break;
		}
		switch(WSAGETSELECTEVENT(lParam)) {
		case FD_CONNECT:
			emu->socket_connected(no);
			break;
		case FD_CLOSE:
			emu->socket_disconnected(no);
			break;
		case FD_WRITE:
			emu->send_data(no);
			break;
		case FD_READ:
			emu->recv_data(no);
			break;
		}
		break;
#endif
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case ID_RESET:
			if(emu) {
				emu->reset();
			}
			break;
#ifdef USE_SPECIAL_RESET
		case ID_SPECIAL_RESET:
			if(emu) {
				emu->special_reset();
			}
			break;
#endif
#ifdef USE_STATE
		case ID_SAVE_STATE:
			if(emu) {
				emu->save_state();
			}
			break;
		case ID_LOAD_STATE:
			if(emu) {
				emu->load_state();
			}
			break;
#endif
#ifdef USE_BOOT_MODE
		case ID_BOOT_MODE0:
		case ID_BOOT_MODE1:
		case ID_BOOT_MODE2:
		case ID_BOOT_MODE3:
		case ID_BOOT_MODE4:
		case ID_BOOT_MODE5:
		case ID_BOOT_MODE6:
		case ID_BOOT_MODE7:
			config.boot_mode = LOWORD(wParam) - ID_BOOT_MODE0;
			if(emu) {
				emu->update_config();
			}
			break;
#endif
#ifdef USE_CPU_TYPE
		case ID_CPU_TYPE0:
		case ID_CPU_TYPE1:
		case ID_CPU_TYPE2:
		case ID_CPU_TYPE3:
		case ID_CPU_TYPE4:
		case ID_CPU_TYPE5:
		case ID_CPU_TYPE6:
		case ID_CPU_TYPE7:
			config.cpu_type = LOWORD(wParam) - ID_CPU_TYPE0;
			// need to recreate vm class instance
//			if(emu) {
//				emu->update_config();
//			}
			break;
#endif
		case ID_CPU_POWER0:
		case ID_CPU_POWER1:
		case ID_CPU_POWER2:
		case ID_CPU_POWER3:
		case ID_CPU_POWER4:
			config.cpu_power = LOWORD(wParam) - ID_CPU_POWER0;
			if(emu) {
				emu->update_config();
			}
			break;
#ifdef USE_DIPSWITCH
		case ID_DIPSWITCH0:
		case ID_DIPSWITCH1:
		case ID_DIPSWITCH2:
		case ID_DIPSWITCH3:
		case ID_DIPSWITCH4:
		case ID_DIPSWITCH5:
		case ID_DIPSWITCH6:
		case ID_DIPSWITCH7:
		case ID_DIPSWITCH8:
		case ID_DIPSWITCH9:
		case ID_DIPSWITCH10:
		case ID_DIPSWITCH11:
		case ID_DIPSWITCH12:
		case ID_DIPSWITCH13:
		case ID_DIPSWITCH14:
		case ID_DIPSWITCH15:
		case ID_DIPSWITCH16:
		case ID_DIPSWITCH17:
		case ID_DIPSWITCH18:
		case ID_DIPSWITCH19:
		case ID_DIPSWITCH20:
		case ID_DIPSWITCH21:
		case ID_DIPSWITCH22:
		case ID_DIPSWITCH23:
		case ID_DIPSWITCH24:
		case ID_DIPSWITCH25:
		case ID_DIPSWITCH26:
		case ID_DIPSWITCH27:
		case ID_DIPSWITCH28:
		case ID_DIPSWITCH29:
		case ID_DIPSWITCH30:
		case ID_DIPSWITCH31:
			config.dipswitch ^= (1 << (LOWORD(wParam) - ID_DIPSWITCH0));
			break;
#endif
#ifdef USE_DEVICE_TYPE
		case ID_DEVICE_TYPE0:
		case ID_DEVICE_TYPE1:
		case ID_DEVICE_TYPE2:
		case ID_DEVICE_TYPE3:
		case ID_DEVICE_TYPE4:
		case ID_DEVICE_TYPE5:
		case ID_DEVICE_TYPE6:
		case ID_DEVICE_TYPE7:
			config.device_type = LOWORD(wParam) - ID_DEVICE_TYPE0;
			break;
#endif
#ifdef USE_AUTO_KEY
		case ID_AUTOKEY_START:
			if(emu) {
				emu->start_auto_key();
			}
			break;
		case ID_AUTOKEY_STOP:
			if(emu) {
				emu->stop_auto_key();
			}
			break;
#endif
#ifdef USE_DEBUGGER
		case ID_OPEN_DEBUGGER0:
		case ID_OPEN_DEBUGGER1:
		case ID_OPEN_DEBUGGER2:
		case ID_OPEN_DEBUGGER3:
			if(emu) {
				no = LOWORD(wParam) - ID_OPEN_DEBUGGER0;
				emu->open_debugger(no);
			}
			break;
		case ID_CLOSE_DEBUGGER:
			if(emu) {
				emu->close_debugger();
			}
			break;
#endif
		case ID_EXIT:
			SendMessage(hWnd, WM_CLOSE, 0, 0L);
			break;
#ifdef USE_CART1
		#define CART_MENU_ITEMS(drv, ID_OPEN_CART, ID_CLOSE_CART, ID_RECENT_CART) \
		case ID_OPEN_CART: \
			if(emu) { \
				open_cart_dialog(hWnd, drv); \
			} \
			break; \
		case ID_CLOSE_CART: \
			if(emu) { \
				emu->close_cart(drv); \
			} \
			break; \
		case ID_RECENT_CART + 0: \
		case ID_RECENT_CART + 1: \
		case ID_RECENT_CART + 2: \
		case ID_RECENT_CART + 3: \
		case ID_RECENT_CART + 4: \
		case ID_RECENT_CART + 5: \
		case ID_RECENT_CART + 6: \
		case ID_RECENT_CART + 7: \
			no = LOWORD(wParam) - ID_RECENT_CART; \
			_tcscpy(path, config.recent_cart_path[drv][no]); \
			for(int i = no; i > 0; i--) { \
				_tcscpy(config.recent_cart_path[drv][i], config.recent_cart_path[drv][i - 1]); \
			} \
			_tcscpy(config.recent_cart_path[drv][0], path); \
			if(emu) { \
				emu->open_cart(drv, path); \
			} \
			break;
		CART_MENU_ITEMS(0, ID_OPEN_CART1, ID_CLOSE_CART1, ID_RECENT_CART1)
#endif
#ifdef USE_CART2
		CART_MENU_ITEMS(1, ID_OPEN_CART2, ID_CLOSE_CART2, ID_RECENT_CART2)
#endif
#ifdef USE_FD1
		#define FD_MENU_ITEMS(drv, ID_OPEN_FD, ID_CLOSE_FD, ID_RECENT_FD, ID_SELECT_D88_BANK) \
		case ID_OPEN_FD: \
			if(emu) { \
				open_disk_dialog(hWnd, drv); \
			} \
			break; \
		case ID_CLOSE_FD: \
			if(emu) { \
				close_disk(drv); \
			} \
			break; \
		case ID_RECENT_FD + 0: \
		case ID_RECENT_FD + 1: \
		case ID_RECENT_FD + 2: \
		case ID_RECENT_FD + 3: \
		case ID_RECENT_FD + 4: \
		case ID_RECENT_FD + 5: \
		case ID_RECENT_FD + 6: \
		case ID_RECENT_FD + 7: \
			no = LOWORD(wParam) - ID_RECENT_FD; \
			_tcscpy(path, config.recent_disk_path[drv][no]); \
			for(int i = no; i > 0; i--) { \
				_tcscpy(config.recent_disk_path[drv][i], config.recent_disk_path[drv][i - 1]); \
			} \
			_tcscpy(config.recent_disk_path[drv][0], path); \
			if(emu) { \
				open_disk(drv, path, 0); \
			} \
			break; \
		case ID_SELECT_D88_BANK +  0: \
		case ID_SELECT_D88_BANK +  1: \
		case ID_SELECT_D88_BANK +  2: \
		case ID_SELECT_D88_BANK +  3: \
		case ID_SELECT_D88_BANK +  4: \
		case ID_SELECT_D88_BANK +  5: \
		case ID_SELECT_D88_BANK +  6: \
		case ID_SELECT_D88_BANK +  7: \
		case ID_SELECT_D88_BANK +  8: \
		case ID_SELECT_D88_BANK +  9: \
		case ID_SELECT_D88_BANK + 10: \
		case ID_SELECT_D88_BANK + 11: \
		case ID_SELECT_D88_BANK + 12: \
		case ID_SELECT_D88_BANK + 13: \
		case ID_SELECT_D88_BANK + 14: \
		case ID_SELECT_D88_BANK + 15: \
		case ID_SELECT_D88_BANK + 16: \
		case ID_SELECT_D88_BANK + 17: \
		case ID_SELECT_D88_BANK + 18: \
		case ID_SELECT_D88_BANK + 19: \
		case ID_SELECT_D88_BANK + 20: \
		case ID_SELECT_D88_BANK + 21: \
		case ID_SELECT_D88_BANK + 22: \
		case ID_SELECT_D88_BANK + 23: \
		case ID_SELECT_D88_BANK + 24: \
		case ID_SELECT_D88_BANK + 25: \
		case ID_SELECT_D88_BANK + 26: \
		case ID_SELECT_D88_BANK + 27: \
		case ID_SELECT_D88_BANK + 28: \
		case ID_SELECT_D88_BANK + 29: \
		case ID_SELECT_D88_BANK + 30: \
		case ID_SELECT_D88_BANK + 31: \
		case ID_SELECT_D88_BANK + 32: \
		case ID_SELECT_D88_BANK + 33: \
		case ID_SELECT_D88_BANK + 34: \
		case ID_SELECT_D88_BANK + 35: \
		case ID_SELECT_D88_BANK + 36: \
		case ID_SELECT_D88_BANK + 37: \
		case ID_SELECT_D88_BANK + 38: \
		case ID_SELECT_D88_BANK + 39: \
		case ID_SELECT_D88_BANK + 40: \
		case ID_SELECT_D88_BANK + 41: \
		case ID_SELECT_D88_BANK + 42: \
		case ID_SELECT_D88_BANK + 43: \
		case ID_SELECT_D88_BANK + 44: \
		case ID_SELECT_D88_BANK + 45: \
		case ID_SELECT_D88_BANK + 46: \
		case ID_SELECT_D88_BANK + 47: \
		case ID_SELECT_D88_BANK + 48: \
		case ID_SELECT_D88_BANK + 49: \
		case ID_SELECT_D88_BANK + 50: \
		case ID_SELECT_D88_BANK + 51: \
		case ID_SELECT_D88_BANK + 52: \
		case ID_SELECT_D88_BANK + 53: \
		case ID_SELECT_D88_BANK + 54: \
		case ID_SELECT_D88_BANK + 55: \
		case ID_SELECT_D88_BANK + 56: \
		case ID_SELECT_D88_BANK + 57: \
		case ID_SELECT_D88_BANK + 58: \
		case ID_SELECT_D88_BANK + 59: \
		case ID_SELECT_D88_BANK + 60: \
		case ID_SELECT_D88_BANK + 61: \
		case ID_SELECT_D88_BANK + 62: \
		case ID_SELECT_D88_BANK + 63: \
			no = LOWORD(wParam) - ID_SELECT_D88_BANK; \
			if(emu && emu->d88_file[drv].cur_bank != no) { \
				emu->open_disk(drv, emu->d88_file[drv].path, emu->d88_file[drv].bank[no].offset); \
				emu->d88_file[drv].cur_bank = no; \
			} \
			break;
		FD_MENU_ITEMS(0, ID_OPEN_FD1, ID_CLOSE_FD1, ID_RECENT_FD1, ID_SELECT_D88_BANK1)
#endif
#ifdef USE_FD2
		FD_MENU_ITEMS(1, ID_OPEN_FD2, ID_CLOSE_FD2, ID_RECENT_FD2, ID_SELECT_D88_BANK2)
#endif
#ifdef USE_FD3
		FD_MENU_ITEMS(2, ID_OPEN_FD3, ID_CLOSE_FD3, ID_RECENT_FD3, ID_SELECT_D88_BANK3)
#endif
#ifdef USE_FD4
		FD_MENU_ITEMS(3, ID_OPEN_FD4, ID_CLOSE_FD4, ID_RECENT_FD4, ID_SELECT_D88_BANK4)
#endif
#ifdef USE_FD5
		FD_MENU_ITEMS(4, ID_OPEN_FD5, ID_CLOSE_FD5, ID_RECENT_FD5, ID_SELECT_D88_BANK5)
#endif
#ifdef USE_FD6
		FD_MENU_ITEMS(5, ID_OPEN_FD6, ID_CLOSE_FD6, ID_RECENT_FD6, ID_SELECT_D88_BANK6)
#endif
#ifdef USE_FD7
		FD_MENU_ITEMS(6, ID_OPEN_FD7, ID_CLOSE_FD7, ID_RECENT_FD7, ID_SELECT_D88_BANK7)
#endif
#ifdef USE_FD8
		FD_MENU_ITEMS(7, ID_OPEN_FD8, ID_CLOSE_FD8, ID_RECENT_FD8, ID_SELECT_D88_BANK8)
#endif
#ifdef USE_QD1
		#define QD_MENU_ITEMS(drv, ID_OPEN_QD, ID_CLOSE_QD, ID_RECENT_QD) \
		case ID_OPEN_QD: \
			if(emu) { \
				open_quickdisk_dialog(hWnd, drv); \
			} \
			break; \
		case ID_CLOSE_QD: \
			if(emu) { \
				emu->close_quickdisk(drv); \
			} \
			break; \
		case ID_RECENT_QD + 0: \
		case ID_RECENT_QD + 1: \
		case ID_RECENT_QD + 2: \
		case ID_RECENT_QD + 3: \
		case ID_RECENT_QD + 4: \
		case ID_RECENT_QD + 5: \
		case ID_RECENT_QD + 6: \
		case ID_RECENT_QD + 7: \
			no = LOWORD(wParam) - ID_RECENT_QD; \
			_tcscpy(path, config.recent_quickdisk_path[drv][no]); \
			for(int i = no; i > 0; i--) { \
				_tcscpy(config.recent_quickdisk_path[drv][i], config.recent_quickdisk_path[drv][i - 1]); \
			} \
			_tcscpy(config.recent_quickdisk_path[drv][0], path); \
			if(emu) { \
				emu->open_quickdisk(drv, path); \
			} \
			break;
		QD_MENU_ITEMS(0, ID_OPEN_QD1, ID_CLOSE_QD1, ID_RECENT_QD1)
#endif
#ifdef USE_QD2
		QD_MENU_ITEMS(1, ID_OPEN_QD2, ID_CLOSE_QD2, ID_RECENT_QD2)
#endif
#ifdef USE_TAPE
		case ID_PLAY_TAPE:
			if(emu) {
				open_tape_dialog(hWnd, true);
			}
			break;
		case ID_REC_TAPE:
			if(emu) {
				open_tape_dialog(hWnd, false);
			}
			break;
		case ID_CLOSE_TAPE:
			if(emu) {
				emu->close_tape();
			}
			break;
		case ID_USE_WAVE_SHAPER:
			config.wave_shaper = !config.wave_shaper;
			break;
		case ID_DIRECT_LOAD_MZT:
			config.direct_load_mzt = !config.direct_load_mzt;
			break;
		case ID_RECENT_TAPE + 0:
		case ID_RECENT_TAPE + 1:
		case ID_RECENT_TAPE + 2:
		case ID_RECENT_TAPE + 3:
		case ID_RECENT_TAPE + 4:
		case ID_RECENT_TAPE + 5:
		case ID_RECENT_TAPE + 6:
		case ID_RECENT_TAPE + 7:
			no = LOWORD(wParam) - ID_RECENT_TAPE;
			_tcscpy(path, config.recent_tape_path[no]);
			for(int i = no; i > 0; i--) {
				_tcscpy(config.recent_tape_path[i], config.recent_tape_path[i - 1]);
			}
			_tcscpy(config.recent_tape_path[0], path);
			if(emu) {
				emu->play_tape(path);
			}
			break;
#endif
#ifdef USE_TAPE_BUTTON
		case ID_PLAY_BUTTON:
			if(emu) {
				emu->push_play();
			}
			break;
		case ID_STOP_BUTTON:
			if(emu) {
				emu->push_stop();
			}
			break;
#endif
#ifdef USE_LASER_DISC
		case ID_OPEN_LASER_DISC:
			if(emu) {
				open_laser_disc_dialog(hWnd);
			}
			break;
		case ID_CLOSE_LASER_DISC:
			if(emu) {
				emu->close_laser_disc();
			}
			break;
		case ID_RECENT_LASER_DISC + 0:
		case ID_RECENT_LASER_DISC + 1:
		case ID_RECENT_LASER_DISC + 2:
		case ID_RECENT_LASER_DISC + 3:
		case ID_RECENT_LASER_DISC + 4:
		case ID_RECENT_LASER_DISC + 5:
		case ID_RECENT_LASER_DISC + 6:
		case ID_RECENT_LASER_DISC + 7:
			no = LOWORD(wParam) - ID_RECENT_LASER_DISC;
			_tcscpy(path, config.recent_laser_disc_path[no]);
			for(int i = no; i > 0; i--) {
				_tcscpy(config.recent_laser_disc_path[i], config.recent_laser_disc_path[i - 1]);
			}
			_tcscpy(config.recent_laser_disc_path[0], path);
			if(emu) {
				emu->open_laser_disc(path);
			}
			break;
#endif
#ifdef USE_BINARY_FILE1
		#define BINARY_MENU_ITEMS(drv, ID_LOAD_BINARY, ID_SAVE_BINARY, ID_RECENT_BINARY) \
		case ID_LOAD_BINARY: \
			if(emu) { \
				open_binary_dialog(hWnd, drv, true); \
			} \
			break; \
		case ID_SAVE_BINARY: \
			if(emu) { \
				open_binary_dialog(hWnd, drv, false); \
			} \
			break; \
		case ID_RECENT_BINARY + 0: \
		case ID_RECENT_BINARY + 1: \
		case ID_RECENT_BINARY + 2: \
		case ID_RECENT_BINARY + 3: \
		case ID_RECENT_BINARY + 4: \
		case ID_RECENT_BINARY + 5: \
		case ID_RECENT_BINARY + 6: \
		case ID_RECENT_BINARY + 7: \
			no = LOWORD(wParam) - ID_RECENT_BINARY; \
			_tcscpy(path, config.recent_binary_path[drv][no]); \
			for(int i = no; i > 0; i--) { \
				_tcscpy(config.recent_binary_path[drv][i], config.recent_binary_path[drv][i - 1]); \
			} \
			_tcscpy(config.recent_binary_path[drv][0], path); \
			if(emu) { \
				emu->load_binary(drv, path); \
			} \
			break;
		BINARY_MENU_ITEMS(0, ID_LOAD_BINARY1, ID_SAVE_BINARY1, ID_RECENT_BINARY1)
#endif
#ifdef USE_BINARY_FILE2
		BINARY_MENU_ITEMS(1, ID_LOAD_BINARY2, ID_SAVE_BINARY2, ID_RECENT_BINARY2)
#endif
		case ID_SCREEN_REC60:
		case ID_SCREEN_REC30:
		case ID_SCREEN_REC15:
			if(emu) {
				static const int fps[3] = {60, 30, 15};
				no = LOWORD(wParam) - ID_SCREEN_REC60;
				emu->start_rec_sound();
				if(!emu->start_rec_video(fps[no])) {
					emu->stop_rec_sound();
				}
			}
			break;
		case ID_SCREEN_STOP:
			if(emu) {
				emu->stop_rec_video();
				emu->stop_rec_sound();
			}
			break;
		case ID_SCREEN_CAPTURE:
			if(emu) {
				emu->capture_screen();
			}
			break;
		case ID_SCREEN_WINDOW1:
		case ID_SCREEN_WINDOW2:
		case ID_SCREEN_WINDOW3:
		case ID_SCREEN_WINDOW4:
		case ID_SCREEN_WINDOW5:
		case ID_SCREEN_WINDOW6:
		case ID_SCREEN_WINDOW7:
		case ID_SCREEN_WINDOW8:
			if(emu) {
				set_window(hWnd, LOWORD(wParam) - ID_SCREEN_WINDOW1);
			}
			break;
		case ID_SCREEN_FULLSCREEN1:
		case ID_SCREEN_FULLSCREEN2:
		case ID_SCREEN_FULLSCREEN3:
		case ID_SCREEN_FULLSCREEN4:
		case ID_SCREEN_FULLSCREEN5:
		case ID_SCREEN_FULLSCREEN6:
		case ID_SCREEN_FULLSCREEN7:
		case ID_SCREEN_FULLSCREEN8:
		case ID_SCREEN_FULLSCREEN9:
		case ID_SCREEN_FULLSCREEN10:
		case ID_SCREEN_FULLSCREEN11:
		case ID_SCREEN_FULLSCREEN12:
		case ID_SCREEN_FULLSCREEN13:
		case ID_SCREEN_FULLSCREEN14:
		case ID_SCREEN_FULLSCREEN15:
		case ID_SCREEN_FULLSCREEN16:
		case ID_SCREEN_FULLSCREEN17:
		case ID_SCREEN_FULLSCREEN18:
		case ID_SCREEN_FULLSCREEN19:
		case ID_SCREEN_FULLSCREEN20:
		case ID_SCREEN_FULLSCREEN21:
		case ID_SCREEN_FULLSCREEN22:
		case ID_SCREEN_FULLSCREEN23:
		case ID_SCREEN_FULLSCREEN24:
		case ID_SCREEN_FULLSCREEN25:
		case ID_SCREEN_FULLSCREEN26:
		case ID_SCREEN_FULLSCREEN27:
		case ID_SCREEN_FULLSCREEN28:
		case ID_SCREEN_FULLSCREEN29:
		case ID_SCREEN_FULLSCREEN30:
		case ID_SCREEN_FULLSCREEN31:
		case ID_SCREEN_FULLSCREEN32:
			if(emu && !now_fullscreen) {
				set_window(hWnd, LOWORD(wParam) - ID_SCREEN_FULLSCREEN1 + MAX_WINDOW);
			}
			break;
		case ID_SCREEN_USE_D3D9:
			config.use_d3d9 = !config.use_d3d9;
			if(emu) {
				emu->set_display_size(-1, -1, !now_fullscreen);
			}
			break;
		case ID_SCREEN_WAIT_VSYNC:
			config.wait_vsync = !config.wait_vsync;
			if(emu) {
				emu->set_display_size(-1, -1, !now_fullscreen);
			}
			break;
		case ID_SCREEN_STRETCH_DOT:
		case ID_SCREEN_STRETCH_ASPECT:
		case ID_SCREEN_STRETCH_FILL:
			config.stretch_type = LOWORD(wParam) - ID_SCREEN_STRETCH_DOT;
			if(emu) {
				emu->set_display_size(-1, -1, !now_fullscreen);
			}
			break;
		// accelerator
		case ID_ACCEL_SCREEN:
			if(emu) {
				emu->suspend();
				set_window(hWnd, now_fullscreen ? prev_window_mode : -1);
			}
			break;
		case ID_ACCEL_MOUSE:
			if(emu) {
				emu->toggle_mouse();
			}
			break;
#ifdef USE_MONITOR_TYPE
		case ID_SCREEN_MONITOR_TYPE0:
		case ID_SCREEN_MONITOR_TYPE1:
		case ID_SCREEN_MONITOR_TYPE2:
		case ID_SCREEN_MONITOR_TYPE3:
		case ID_SCREEN_MONITOR_TYPE4:
		case ID_SCREEN_MONITOR_TYPE5:
		case ID_SCREEN_MONITOR_TYPE6:
		case ID_SCREEN_MONITOR_TYPE7:
			config.monitor_type = LOWORD(wParam) - ID_SCREEN_MONITOR_TYPE0;
			if(emu) {
				emu->update_config();
#ifdef USE_SCREEN_ROTATE
				if(now_fullscreen) {
					emu->set_display_size(-1, -1, false);
				} else {
					set_window(hWnd, prev_window_mode);
				}
#endif
			}
			break;
#endif
#ifdef USE_CRT_FILTER
		case ID_SCREEN_CRT_FILTER:
			config.crt_filter = !config.crt_filter;
			break;
#endif
#ifdef USE_SCANLINE
		case ID_SCREEN_SCANLINE:
			config.scan_line = !config.scan_line;
			if(emu) {
				emu->update_config();
			}
			break;
#endif
		case ID_SOUND_REC:
			if(emu) {
				emu->start_rec_sound();
			}
			break;
		case ID_SOUND_STOP:
			if(emu) {
				emu->stop_rec_sound();
			}
			break;
		case ID_SOUND_FREQ0:
		case ID_SOUND_FREQ1:
		case ID_SOUND_FREQ2:
		case ID_SOUND_FREQ3:
		case ID_SOUND_FREQ4:
		case ID_SOUND_FREQ5:
		case ID_SOUND_FREQ6:
		case ID_SOUND_FREQ7:
			config.sound_frequency = LOWORD(wParam) - ID_SOUND_FREQ0;
			if(emu) {
				emu->update_config();
			}
			break;
		case ID_SOUND_LATE0:
		case ID_SOUND_LATE1:
		case ID_SOUND_LATE2:
		case ID_SOUND_LATE3:
		case ID_SOUND_LATE4:
			config.sound_latency = LOWORD(wParam) - ID_SOUND_LATE0;
			if(emu) {
				emu->update_config();
			}
			break;
#ifdef USE_SOUND_DEVICE_TYPE
		case ID_SOUND_DEVICE_TYPE0:
		case ID_SOUND_DEVICE_TYPE1:
		case ID_SOUND_DEVICE_TYPE2:
		case ID_SOUND_DEVICE_TYPE3:
		case ID_SOUND_DEVICE_TYPE4:
		case ID_SOUND_DEVICE_TYPE5:
		case ID_SOUND_DEVICE_TYPE6:
		case ID_SOUND_DEVICE_TYPE7:
			config.sound_device_type = LOWORD(wParam) - ID_SOUND_DEVICE_TYPE0;
			//if(emu) {
			//	emu->update_config();
			//}
			break;
#endif
#ifdef USE_VIDEO_CAPTURE
		case ID_CAPTURE_FILTER:
			if(emu) {
				emu->show_capture_dev_filter();
			}
			break;
		case ID_CAPTURE_PIN:
			if(emu) {
				emu->show_capture_dev_pin();
			}
			break;
		case ID_CAPTURE_SOURCE:
			if(emu) {
				emu->show_capture_dev_source();
			}
			break;
		case ID_CAPTURE_CLOSE:
			if(emu) {
				emu->close_capture_dev();
			}
			break;
		case ID_CAPTURE_DEVICE1:
		case ID_CAPTURE_DEVICE2:
		case ID_CAPTURE_DEVICE3:
		case ID_CAPTURE_DEVICE4:
		case ID_CAPTURE_DEVICE5:
		case ID_CAPTURE_DEVICE6:
		case ID_CAPTURE_DEVICE7:
		case ID_CAPTURE_DEVICE8:
			no = LOWORD(wParam) - ID_CAPTURE_DEVICE1;
			if(emu) {
				emu->open_capture_dev(no, false);
			}
			break;
#endif
#ifdef USE_BUTTON
		case ID_BUTTON +  0:
		case ID_BUTTON +  1:
		case ID_BUTTON +  2:
		case ID_BUTTON +  3:
		case ID_BUTTON +  4:
		case ID_BUTTON +  5:
		case ID_BUTTON +  6:
		case ID_BUTTON +  7:
		case ID_BUTTON +  8:
		case ID_BUTTON +  9:
		case ID_BUTTON + 10:
		case ID_BUTTON + 11:
		case ID_BUTTON + 12:
		case ID_BUTTON + 13:
		case ID_BUTTON + 14:
		case ID_BUTTON + 15:
		case ID_BUTTON + 16:
		case ID_BUTTON + 17:
		case ID_BUTTON + 18:
		case ID_BUTTON + 19:
		case ID_BUTTON + 20:
		case ID_BUTTON + 21:
		case ID_BUTTON + 22:
		case ID_BUTTON + 23:
		case ID_BUTTON + 24:
		case ID_BUTTON + 25:
		case ID_BUTTON + 26:
		case ID_BUTTON + 27:
		case ID_BUTTON + 28:
		case ID_BUTTON + 29:
		case ID_BUTTON + 30:
		case ID_BUTTON + 31:
			if(emu) {
				emu->press_button(LOWORD(wParam) - ID_BUTTON);
			}
			break;
#endif
		}
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam) ;
}

#ifdef USE_BUTTON
LRESULT CALLBACK ButtonWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	for(int i = 0; i < MAX_BUTTONS; i++) {
		if(hWnd == hButton[i]) {
			switch(iMsg) {
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if(emu) {
					emu->key_down(LOBYTE(wParam), false);
				}
				return 0;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				if(emu) {
					emu->key_up(LOBYTE(wParam));
				}
				return 0;
			}
			return CallWindowProc(buttonWndProc[i], hWnd, iMsg, wParam, lParam);
		}
	}
	return 0;
}
#endif

void update_menu(HWND hWnd, HMENU hMenu, int pos)
{
#ifdef MENU_POS_CONTROL
	if(pos == MENU_POS_CONTROL) {
		// control menu
#ifdef USE_BOOT_MODE
		if(config.boot_mode >= 0 && config.boot_mode < USE_BOOT_MODE) {
			CheckMenuRadioItem(hMenu, ID_BOOT_MODE0, ID_BOOT_MODE0 + USE_BOOT_MODE - 1, ID_BOOT_MODE0 + config.boot_mode, MF_BYCOMMAND);
		}
#endif
#ifdef USE_CPU_TYPE
		if(config.cpu_type >= 0 && config.cpu_type < USE_CPU_TYPE) {
			CheckMenuRadioItem(hMenu, ID_CPU_TYPE0, ID_CPU_TYPE0 + USE_CPU_TYPE - 1, ID_CPU_TYPE0 + config.cpu_type, MF_BYCOMMAND);
		}
#endif
		if(config.cpu_power >= 0 && config.cpu_power < 5) {
			CheckMenuRadioItem(hMenu, ID_CPU_POWER0, ID_CPU_POWER4, ID_CPU_POWER0 + config.cpu_power, MF_BYCOMMAND);
		}
#ifdef USE_DIPSWITCH
		for(int i = 0; i < 32; i++) {
			CheckMenuItem(hMenu, ID_DIPSWITCH0 + i, (config.dipswitch & (1 << i)) ? MF_CHECKED : MF_UNCHECKED);
		}
#endif
#ifdef USE_DEVICE_TYPE
		if(config.device_type >= 0 && config.device_type < USE_DEVICE_TYPE) {
			CheckMenuRadioItem(hMenu, ID_DEVICE_TYPE0, ID_DEVICE_TYPE0 + USE_DEVICE_TYPE - 1, ID_DEVICE_TYPE0 + config.device_type, MF_BYCOMMAND);
		}
#endif
#ifdef USE_AUTO_KEY
		// auto key
		bool now_paste = true, now_stop = true;
		if(emu) {
			now_paste = emu->now_auto_key();
			now_stop = !now_paste;
		}
		EnableMenuItem(hMenu, ID_AUTOKEY_START, now_paste ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_AUTOKEY_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
#endif
#ifdef USE_DEBUGGER
		// debugger
		EnableMenuItem(hMenu, ID_OPEN_DEBUGGER0, emu && !emu->now_debugging && emu->debugger_enabled(0) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_OPEN_DEBUGGER1, emu && !emu->now_debugging && emu->debugger_enabled(1) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_OPEN_DEBUGGER2, emu && !emu->now_debugging && emu->debugger_enabled(2) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_OPEN_DEBUGGER3, emu && !emu->now_debugging && emu->debugger_enabled(3) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_CLOSE_DEBUGGER, emu &&  emu->now_debugging                             ? MF_ENABLED : MF_GRAYED);
#endif
	}
#endif
#ifdef MENU_POS_CART1
	else if(pos == MENU_POS_CART1) {
		#define UPDATE_MENU_CART(drv, ID_RECENT_CART, ID_CLOSE_CART) \
		bool flag = false; \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			DeleteMenu(hMenu, ID_RECENT_CART + i, MF_BYCOMMAND); \
		} \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			if(_tcsicmp(config.recent_cart_path[drv][i], _T(""))) { \
				AppendMenu(hMenu, MF_STRING, ID_RECENT_CART + i, config.recent_cart_path[drv][i]); \
				flag = true; \
			} \
		} \
		if(!flag) { \
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_CART, _T("None")); \
		} \
		EnableMenuItem(hMenu, ID_CLOSE_CART, emu->cart_inserted(drv) ? MF_ENABLED : MF_GRAYED);
		// cart slot #1
		UPDATE_MENU_CART(0, ID_RECENT_CART1, ID_CLOSE_CART1)
	}
#endif
#ifdef MENU_POS_CART2
	else if(pos == MENU_POS_CART2) {
		// cart slot #2
		UPDATE_MENU_CART(1, ID_RECENT_CART2, ID_CLOSE_CART2)
	}
#endif
#ifdef MENU_POS_FD1
	else if(pos == MENU_POS_FD1) {
		#define UPDATE_MENU_FD(drv, ID_RECENT_FD, ID_D88_FILE_PATH, ID_SELECT_D88_BANK, ID_CLOSE_FD) \
		bool flag = false; \
		while(DeleteMenu(hMenu, 3, MF_BYPOSITION) != 0) {} \
		if(emu->d88_file[drv].bank_num > 1) { \
			AppendMenu(hMenu, MF_STRING | MF_DISABLED, ID_D88_FILE_PATH, emu->d88_file[drv].path); \
			for(int i = 0; i < emu->d88_file[drv].bank_num; i++) { \
				_TCHAR tmp[32]; \
				_stprintf(tmp, _T("%d: %s"), i + 1, emu->d88_file[drv].bank[i].name); \
				AppendMenu(hMenu, MF_STRING | (i == emu->d88_file[drv].cur_bank ? MF_CHECKED : 0), ID_SELECT_D88_BANK + i, tmp); \
			} \
			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL); \
		} \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			if(_tcsicmp(config.recent_disk_path[drv][i], _T(""))) { \
				AppendMenu(hMenu, MF_STRING, ID_RECENT_FD + i, config.recent_disk_path[drv][i]); \
				flag = true; \
			} \
		} \
		if(!flag) { \
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_FD, _T("None")); \
		} \
		EnableMenuItem(hMenu, ID_CLOSE_FD, emu->disk_inserted(drv) ? MF_ENABLED : MF_GRAYED);
		// floppy drive #1
		UPDATE_MENU_FD(0, ID_RECENT_FD1, ID_D88_FILE_PATH1, ID_SELECT_D88_BANK1, ID_CLOSE_FD1)
	}
#endif
#ifdef MENU_POS_FD2
	else if(pos == MENU_POS_FD2) {
		// floppy drive #2
		UPDATE_MENU_FD(1, ID_RECENT_FD2, ID_D88_FILE_PATH2, ID_SELECT_D88_BANK2, ID_CLOSE_FD2)
	}
#endif
#ifdef MENU_POS_FD3
	else if(pos == MENU_POS_FD3) {
		// floppy drive #3
		UPDATE_MENU_FD(2, ID_RECENT_FD3, ID_D88_FILE_PATH3, ID_SELECT_D88_BANK3, ID_CLOSE_FD3)
	}
#endif
#ifdef MENU_POS_FD4
	else if(pos == MENU_POS_FD4) {
		// floppy drive #4
		UPDATE_MENU_FD(3, ID_RECENT_FD4, ID_D88_FILE_PATH4, ID_SELECT_D88_BANK4, ID_CLOSE_FD4)
	}
#endif
#ifdef MENU_POS_FD5
	else if(pos == MENU_POS_FD5) {
		// floppy drive #5
		UPDATE_MENU_FD(4, ID_RECENT_FD5, ID_D88_FILE_PATH5, ID_SELECT_D88_BANK5, ID_CLOSE_FD5)
	}
#endif
#ifdef MENU_POS_FD6
	else if(pos == MENU_POS_FD6) {
		// floppy drive #6
		UPDATE_MENU_FD(5, ID_RECENT_FD6, ID_D88_FILE_PATH6, ID_SELECT_D88_BANK6, ID_CLOSE_FD6)
	}
#endif
#ifdef MENU_POS_FD7
	else if(pos == MENU_POS_FD7) {
		// floppy drive #7
		UPDATE_MENU_FD(6, ID_RECENT_FD7, ID_D88_FILE_PATH7, ID_SELECT_D88_BANK7, ID_CLOSE_FD7)
	}
#endif
#ifdef MENU_POS_FD8
	else if(pos == MENU_POS_FD8) {
		// floppy drive #8
		UPDATE_MENU_FD(7, ID_RECENT_FD8, ID_D88_FILE_PATH8, ID_SELECT_D88_BANK8, ID_CLOSE_FD8)
	}
#endif
#ifdef MENU_POS_QD1
	else if(pos == MENU_POS_QD1) {
		#define UPDATE_MENU_QD(drv, ID_RECENT_QD, ID_CLOSE_QD) \
		bool flag = false; \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			DeleteMenu(hMenu, ID_RECENT_QD + i, MF_BYCOMMAND); \
		} \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			if(_tcsicmp(config.recent_quickdisk_path[drv][i], _T(""))) { \
				AppendMenu(hMenu, MF_STRING, ID_RECENT_QD + i, config.recent_quickdisk_path[drv][i]); \
				flag = true; \
			} \
		} \
		if(!flag) { \
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_QD, _T("None")); \
		} \
		EnableMenuItem(hMenu, ID_CLOSE_QD, emu->quickdisk_inserted(drv) ? MF_ENABLED : MF_GRAYED);
		// quick disk drive #1
		UPDATE_MENU_QD(0, ID_RECENT_QD1, ID_CLOSE_QD1)
	}
#endif
#ifdef MENU_POS_QD2
	else if(pos == MENU_POS_QD2) {
		// quick disk drive #2
		UPDATE_MENU_QD(1, ID_RECENT_QD2, ID_CLOSE_QD2)
	}
#endif
#ifdef MENU_POS_TAPE
	else if(pos == MENU_POS_TAPE) {
		// data recorder
		bool flag = false;
		for(int i = 0; i < MAX_HISTORY; i++) {
			DeleteMenu(hMenu, ID_RECENT_TAPE + i, MF_BYCOMMAND);
		}
		for(int i = 0; i < MAX_HISTORY; i++) {
			if(_tcsicmp(config.recent_tape_path[i], _T(""))) {
				AppendMenu(hMenu, MF_STRING, ID_RECENT_TAPE + i, config.recent_tape_path[i]);
				flag = true;
			}
		}
		if(!flag) {
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_TAPE, _T("None"));
		}
		EnableMenuItem(hMenu, ID_CLOSE_TAPE, emu->tape_inserted() ? MF_ENABLED : MF_GRAYED);
#ifdef USE_TAPE_BUTTON
		EnableMenuItem(hMenu, ID_PLAY_BUTTON, emu->tape_inserted() ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_STOP_BUTTON, emu->tape_inserted() ? MF_ENABLED : MF_GRAYED);
#endif
		CheckMenuItem(hMenu, ID_USE_WAVE_SHAPER, config.wave_shaper ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_DIRECT_LOAD_MZT, config.direct_load_mzt ? MF_CHECKED : MF_UNCHECKED);
	}
#endif
#ifdef MENU_POS_LASER_DISC
	else if(pos == MENU_POS_LASER_DISC) {
		// data recorder
		bool flag = false;
		for(int i = 0; i < MAX_HISTORY; i++) {
			DeleteMenu(hMenu, ID_RECENT_LASER_DISC + i, MF_BYCOMMAND);
		}
		for(int i = 0; i < MAX_HISTORY; i++) {
			if(_tcsicmp(config.recent_laser_disc_path[i], _T(""))) {
				AppendMenu(hMenu, MF_STRING, ID_RECENT_LASER_DISC + i, config.recent_laser_disc_path[i]);
				flag = true;
			}
		}
		if(!flag) {
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_LASER_DISC, _T("None"));
		}
		EnableMenuItem(hMenu, ID_CLOSE_LASER_DISC, emu->laser_disc_inserted() ? MF_ENABLED : MF_GRAYED);
	}
#endif
#ifdef MENU_POS_BINARY1
	else if(pos == MENU_POS_BINARY1) {
		// binary #1
		#define UPDATE_MENU_BINARY(drv, ID_RECENT_BINARY) \
		bool flag = false; \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			DeleteMenu(hMenu, ID_RECENT_BINARY + i, MF_BYCOMMAND); \
		} \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			if(_tcsicmp(config.recent_binary_path[drv][i], _T(""))) { \
				AppendMenu(hMenu, MF_STRING, ID_RECENT_BINARY + i, config.recent_binary_path[drv][i]); \
				flag = true; \
			} \
		} \
		if(!flag) { \
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_BINARY, _T("None")); \
		}
		UPDATE_MENU_BINARY(0, ID_RECENT_BINARY1)
	}
#endif
#ifdef MENU_POS_BINARY2
	else if(pos == MENU_POS_BINARY2) {
		// binary #2
		UPDATE_MENU_BINARY(1, ID_RECENT_BINARY2)
	}
#endif
#ifdef MENU_POS_SCREEN
	else if(pos == MENU_POS_SCREEN) {
		// recording
		bool now_rec = true, now_stop = true;
		if(emu) {
			now_rec = emu->now_rec_video;
			now_stop = !now_rec;
		}
		EnableMenuItem(hMenu, ID_SCREEN_REC60, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SCREEN_REC30, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SCREEN_REC15, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SCREEN_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
		
		// screen mode
		UINT last = ID_SCREEN_WINDOW1;
		for(int i = 1; i < MAX_WINDOW; i++) {
			DeleteMenu(hMenu, ID_SCREEN_WINDOW1 + i, MF_BYCOMMAND);
		}
		for(int i = 1; i < MAX_WINDOW; i++) {
			if(emu && emu->get_window_width(i) <= desktop_width && emu->get_window_height(i) <= desktop_height) {
				_TCHAR buf[16];
				_stprintf(buf, _T("Window x%d"), i + 1);
				InsertMenu(hMenu, ID_SCREEN_FULLSCREEN1, MF_BYCOMMAND | MF_STRING, ID_SCREEN_WINDOW1 + i, buf);
				last = ID_SCREEN_WINDOW1 + i;
			}
		}
		for(int i = 0; i < MAX_FULLSCREEN; i++) {
			if(i < screen_mode_count) {
				MENUITEMINFO info;
				ZeroMemory(&info, sizeof(info));
				info.cbSize = sizeof(info);
				_TCHAR buf[64];
				_stprintf(buf, _T("Fullscreen %dx%d"), screen_mode_width[i], screen_mode_height[i]);
				info.fMask = MIIM_TYPE;
				info.fType = MFT_STRING;
				info.dwTypeData = buf;
				SetMenuItemInfo(hMenu, ID_SCREEN_FULLSCREEN1 + i, FALSE, &info);
				EnableMenuItem(hMenu, ID_SCREEN_FULLSCREEN1 + i, now_fullscreen ? MF_GRAYED : MF_ENABLED);
				last = ID_SCREEN_FULLSCREEN1 + i;
			} else {
				DeleteMenu(hMenu, ID_SCREEN_FULLSCREEN1 + i, MF_BYCOMMAND);
			}
		}
		if(config.window_mode >= 0 && config.window_mode < MAX_WINDOW) {
			CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW1, last, ID_SCREEN_WINDOW1 + config.window_mode, MF_BYCOMMAND);
		} else if(config.window_mode >= MAX_WINDOW && config.window_mode < screen_mode_count + MAX_WINDOW) {
			CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW1, last, ID_SCREEN_FULLSCREEN1 + config.window_mode - MAX_WINDOW, MF_BYCOMMAND);
		}
		CheckMenuItem(hMenu, ID_SCREEN_USE_D3D9, config.use_d3d9 ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_SCREEN_WAIT_VSYNC, config.wait_vsync ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuRadioItem(hMenu, ID_SCREEN_STRETCH_DOT, ID_SCREEN_STRETCH_FILL, ID_SCREEN_STRETCH_DOT + config.stretch_type, MF_BYCOMMAND);
		
#ifdef USE_MONITOR_TYPE
		if(config.monitor_type >= 0 && config.monitor_type < USE_MONITOR_TYPE) {
			CheckMenuRadioItem(hMenu, ID_SCREEN_MONITOR_TYPE0, ID_SCREEN_MONITOR_TYPE0 + USE_MONITOR_TYPE - 1, ID_SCREEN_MONITOR_TYPE0 + config.monitor_type, MF_BYCOMMAND);
		}
#endif
#ifdef USE_CRT_FILTER
		CheckMenuItem(hMenu, ID_SCREEN_CRT_FILTER, config.crt_filter ? MF_CHECKED : MF_UNCHECKED);
#endif
#ifdef USE_SCANLINE
		CheckMenuItem(hMenu, ID_SCREEN_SCANLINE, config.scan_line ? MF_CHECKED : MF_UNCHECKED);
#endif
	}
#endif
#ifdef MENU_POS_SOUND
	else if(pos == MENU_POS_SOUND) {
		// sound menu
		bool now_rec = false, now_stop = false;
		if(emu) {
			now_rec = emu->now_rec_sound;
			now_stop = !now_rec;
		}
		EnableMenuItem(hMenu, ID_SOUND_REC, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SOUND_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
		
		if(config.sound_frequency >= 0 && config.sound_frequency < 8) {
			CheckMenuRadioItem(hMenu, ID_SOUND_FREQ0, ID_SOUND_FREQ7, ID_SOUND_FREQ0 + config.sound_frequency, MF_BYCOMMAND);
		}
		if(config.sound_latency >= 0 && config.sound_latency < 5) {
			CheckMenuRadioItem(hMenu, ID_SOUND_LATE0, ID_SOUND_LATE4, ID_SOUND_LATE0 + config.sound_latency, MF_BYCOMMAND);
		}
#ifdef USE_SOUND_DEVICE_TYPE
		if(config.sound_device_type >= 0 && config.sound_device_type < USE_SOUND_DEVICE_TYPE) {
			CheckMenuRadioItem(hMenu, ID_SOUND_DEVICE_TYPE0, ID_SOUND_DEVICE_TYPE0 + USE_SOUND_DEVICE_TYPE - 1, ID_SOUND_DEVICE_TYPE0 + config.sound_device_type, MF_BYCOMMAND);
		}
#endif
	}
#endif
#ifdef MENU_POS_CAPTURE
	else if(pos == MENU_POS_CAPTURE) {
		// video capture menu
		int num_devs = emu->get_num_capture_devs();
		int cur_index = emu->get_cur_capture_dev_index();
		
		for(int i = 0; i < 8; i++) {
			DeleteMenu(hMenu, ID_CAPTURE_DEVICE1 + i, MF_BYCOMMAND);
		}
		for(int i = 0; i < 8; i++) {
			if(num_devs >= i + 1) {
				AppendMenu(hMenu, MF_STRING, ID_CAPTURE_DEVICE1 + i, emu->get_capture_dev_name(i));
			}
		}
		if(num_devs == 0) {
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_CAPTURE_DEVICE1, _T("None"));
		}
		if(cur_index != -1) {
			CheckMenuRadioItem(hMenu, ID_CAPTURE_DEVICE1, ID_CAPTURE_DEVICE1, ID_CAPTURE_DEVICE1 + cur_index, MF_BYCOMMAND);
		}
		EnableMenuItem(hMenu, ID_CAPTURE_FILTER, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_CAPTURE_PIN, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_CAPTURE_SOURCE, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_CAPTURE_CLOSE, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
	}
#endif
	DrawMenuBar(hWnd);
}

#ifdef USE_CART1
void open_cart_dialog(HWND hWnd, int drv)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
#if defined(_GAMEGEAR)
		_T("Supported Files (*.rom;*.bin;*.gg;*.col)\0*.rom;*.bin;*.gg;*.col\0All Files (*.*)\0*.*\0\0"),
		_T("Game Cartridge"),
#elif defined(_MASTERSYSTEM)
		_T("Supported Files (*.rom;*.bin;*.sms)\0*.rom;*.bin;*.sms\0All Files (*.*)\0*.*\0\0"),
		_T("Game Cartridge"),
#elif defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
		_T("Supported Files (*.rom;*.bin;*.60)\0*.rom;*.bin;*.60\0All Files (*.*)\0*.*\0\0"),
		_T("Game Cartridge"),
#elif defined(_PCENGINE) || defined(_X1TWIN)
		_T("Supported Files (*.rom;*.bin;*.pce)\0*.rom;*.bin;*.pce\0All Files (*.*)\0*.*\0\0"),
		_T("HuCARD"),
#else
		_T("Supported Files (*.rom;*.bin)\0*.rom;*.bin\0All Files (*.*)\0*.*\0\0"), 
		_T("Game Cartridge"),
#endif
		config.initial_cart_dir
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_cart_path[drv]);
		_tcscpy(config.initial_cart_dir, get_parent_dir(path));
		emu->open_cart(drv, path);
	}
}
#endif

#ifdef USE_FD1
void open_disk_dialog(HWND hWnd, int drv)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.d88;*.d77;*.td0;*.imd;*.dsk;*.fdi;*.hdm;*.tfd;*.xdf;*.2d;*.sf7)\0*.d88;*.d77;*.td0;*.imd;*.dsk;*.fdi;*.hdm;*.tfd;*.xdf;*.2d;*.sf7\0All Files (*.*)\0*.*\0\0"),
		_T("Floppy Disk"),
		config.initial_disk_dir
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_disk_path[drv]);
		_tcscpy(config.initial_disk_dir, get_parent_dir(path));
		open_disk(drv, path, 0);
	}
}

void open_disk(int drv, _TCHAR* path, int bank)
{
	emu->d88_file[drv].bank_num = 0;
	emu->d88_file[drv].cur_bank = -1;
	emu->d88_file[drv].bank[0].offset = 0;
	
	if(check_file_extension(path, _T(".d88")) || check_file_extension(path, _T(".d77"))) {
		FILE *fp = _tfopen(path, _T("rb"));
		if(fp != NULL) {
			try {
				fseek(fp, 0, SEEK_END);
				int file_size = ftell(fp), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && emu->d88_file[drv].bank_num < MAX_D88_BANKS) {
					emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].offset = file_offset;
					fseek(fp, file_offset, SEEK_SET);
#ifdef _UNICODE
					char tmp[18];
					fread(tmp, 17, 1, fp);
					tmp[17] = 0;
					MultiByteToWideChar(CP_ACP, 0, tmp, -1, emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].name, 18);
#else
					fread(emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].name, 17, 1, fp);
					emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].name[17] = 0;
#endif
					fseek(fp, file_offset + 0x1c, SEEK_SET);
					file_offset += fgetc(fp);
					file_offset += fgetc(fp) << 8;
					file_offset += fgetc(fp) << 16;
					file_offset += fgetc(fp) << 24;
					emu->d88_file[drv].bank_num++;
				}
				_tcscpy(emu->d88_file[drv].path, path);
				emu->d88_file[drv].cur_bank = bank;
			}
			catch(...) {
				emu->d88_file[drv].bank_num = 0;
			}
		}
	}
	emu->open_disk(drv, path, emu->d88_file[drv].bank[bank].offset);
#ifdef USE_FD2
	if((drv & 1) == 0 && drv + 1 < MAX_FD && bank + 1 < emu->d88_file[drv].bank_num) {
		open_disk(drv + 1, path, bank + 1);
	}
#endif
}

void close_disk(int drv)
{
	emu->close_disk(drv);
	emu->d88_file[drv].bank_num = 0;
	emu->d88_file[drv].cur_bank = -1;

}
#endif

#ifdef USE_QD1
void open_quickdisk_dialog(HWND hWnd, int drv)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.mzt;*.q20;*.qdf)\0*.mzt;*.q20;*.qdf\0All Files (*.*)\0*.*\0\0"),
		_T("Quick Disk"),
		config.initial_quickdisk_dir
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_quickdisk_path[drv]);
		_tcscpy(config.initial_quickdisk_dir, get_parent_dir(path));
		emu->open_quickdisk(drv, path);
	}
}
#endif

#ifdef USE_TAPE
void open_tape_dialog(HWND hWnd, bool play)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
#if defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
		_T("Supported Files (*.wav;*.p6;*.cas)\0*.wav;*.p6;*.cas\0All Files (*.*)\0*.*\0\0"),
#elif defined(_PC8001SR) || defined(_PC8801MA) || defined(_PC98DO)
		play ? _T("Supported Files (*.cas;*.cmt;*.n80;*.t88)\0*.cas;*.cmt;*.n80;*.t88\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.cas;*.cmt)\0*.cas;*.cmt\0All Files (*.*)\0*.*\0\0"),
#elif defined(_MZ80A) || defined(_MZ80K) || defined(_MZ1200) || defined(_MZ700) || defined(_MZ800) || defined(_MZ1500)
		play ? _T("Supported Files (*.wav;*.cas;*.mzt;*.m12)\0*.wav;*.cas;*.mzt;*.m12\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.wav;*.cas)\0*.wav;*.cas\0All Files (*.*)\0*.*\0\0"),
#elif defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
		play ? _T("Supported Files (*.wav;*.cas;*.mzt;*.mti;*.mtw;*.dat)\0*.wav;*.cas;*.mzt;*.mti;*.mtw;*.dat\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.wav;*.cas)\0*.wav;*.cas\0All Files (*.*)\0*.*\0\0"),
#elif defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
		play ? _T("Supported Files (*.wav;*.cas;*.tap)\0*.wav;*.cas;*.tap\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.wav;*.cas)\0*.wav;*.cas\0All Files (*.*)\0*.*\0\0"),
#elif defined(TAPE_BINARY_ONLY)
		_T("Supported Files (*.cas;*.cmt)\0*.cas;*.cmt\0All Files (*.*)\0*.*\0\0"),
#else
		_T("Supported Files (*.wav;*.cas)\0*.wav;*.cas\0All Files (*.*)\0*.*\0\0"),
#endif
		play ? _T("Data Recorder Tape [Play]") : _T("Data Recorder Tape [Rec]"),
		config.initial_tape_dir
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_tape_path);
		_tcscpy(config.initial_tape_dir, get_parent_dir(path));
		if(play) {
			emu->play_tape(path);
		} else {
			emu->rec_tape(path);
		}
	}
}
#endif

#ifdef USE_LASER_DISC
void open_laser_disc_dialog(HWND hWnd)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.avi;*.mpg;*.mpeg;*.wmv;*.ogv)\0*.avi;*.mpg;*.mpeg;*.wmv;*.ogv\0All Files (*.*)\0*.*\0\0"),
		_T("Laser Disc"),
		config.initial_laser_disc_dir
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_laser_disc_path);
		_tcscpy(config.initial_laser_disc_dir, get_parent_dir(path));
		emu->open_laser_disc(path);
	}
}
#endif

#ifdef USE_BINARY_FILE1
void open_binary_dialog(HWND hWnd, int drv, bool load)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.ram;*.bin)\0*.ram;*.bin\0All Files (*.*)\0*.*\0\0"),
#if defined(_PASOPIA) || defined(_PASOPIA7)
		_T("RAM Pack Cartridge"),
#else
		_T("Memory Dump"),
#endif
		config.initial_binary_dir
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_binary_path[drv]);
		_tcscpy(config.initial_binary_dir, get_parent_dir(path));
		if(load) {
			emu->load_binary(drv, path);
		} else {
			emu->save_binary(drv, path);
		}
	}
}
#endif

#ifdef SUPPORT_DRAG_DROP
void open_any_file(_TCHAR* path)
{
#if defined(USE_CART1)
	if(check_file_extension(path, _T(".rom")) || 
	   check_file_extension(path, _T(".bin")) || 
	   check_file_extension(path, _T(".gg" )) || 
	   check_file_extension(path, _T(".col")) || 
	   check_file_extension(path, _T(".sms")) || 
	   check_file_extension(path, _T(".60" )) || 
	   check_file_extension(path, _T(".pce"))) {
		UPDATE_HISTORY(path, config.recent_cart_path[0]);
		_tcscpy(config.initial_cart_dir, get_parent_dir(path));
		emu->open_cart(0, path);
		return;
	}
#endif
#if defined(USE_FD1)
	if(check_file_extension(path, _T(".d88")) || 
	   check_file_extension(path, _T(".d77")) || 
	   check_file_extension(path, _T(".td0")) || 
	   check_file_extension(path, _T(".imd")) || 
	   check_file_extension(path, _T(".dsk")) || 
	   check_file_extension(path, _T(".fdi")) || 
	   check_file_extension(path, _T(".hdm")) || 
	   check_file_extension(path, _T(".tfd")) || 
	   check_file_extension(path, _T(".xdf")) || 
	   check_file_extension(path, _T(".2d" )) || 
	   check_file_extension(path, _T(".sf7"))) {
		UPDATE_HISTORY(path, config.recent_disk_path[0]);
		_tcscpy(config.initial_disk_dir, get_parent_dir(path));
		open_disk(0, path, 0);
		return;
	}
#endif
#if defined(USE_TAPE)
	if(check_file_extension(path, _T(".wav")) || 
	   check_file_extension(path, _T(".cas")) || 
	   check_file_extension(path, _T(".p6" )) || 
	   check_file_extension(path, _T(".cmt")) || 
	   check_file_extension(path, _T(".n80")) || 
	   check_file_extension(path, _T(".t88")) || 
	   check_file_extension(path, _T(".mzt")) || 
	   check_file_extension(path, _T(".m12")) || 
	   check_file_extension(path, _T(".mti")) || 
	   check_file_extension(path, _T(".mtw")) || 
	   check_file_extension(path, _T(".tap"))) {
		UPDATE_HISTORY(path, config.recent_tape_path);
		_tcscpy(config.initial_tape_dir, get_parent_dir(path));
		emu->play_tape(path);
		return;
	}
#endif
#if defined(USE_BINARY_FILE1)
	if(check_file_extension(path, _T(".ram")) || 
	   check_file_extension(path, _T(".bin"))) {
		UPDATE_HISTORY(path, config.recent_binary_path[0]);
		_tcscpy(config.initial_binary_dir, get_parent_dir(path));
		emu->load_binary(0, path);
		return;
	}
#endif
}
#endif

void set_window(HWND hWnd, int mode)
{
	static LONG style = WS_VISIBLE;
	WINDOWPLACEMENT place;
	place.length = sizeof(WINDOWPLACEMENT);
	
	if(mode >= 0 && mode < MAX_WINDOW) {
		// window
		int width = emu->get_window_width(mode);
		int height = emu->get_window_height(mode);
		RECT rect = {0, 0, width, height};
		AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, TRUE);
		int dest_x = (int)((desktop_width - (rect.right - rect.left)) / 2);
		int dest_y = (int)((desktop_height - (rect.bottom - rect.top)) / 2);
		//dest_x = (dest_x < 0) ? 0 : dest_x;
		dest_y = (dest_y < 0) ? 0 : dest_y;
		
		if(now_fullscreen) {
			ChangeDisplaySettings(NULL, 0);
			SetWindowLong(hWnd, GWL_STYLE, style);
			SetWindowPos(hWnd, HWND_TOP, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
			now_fullscreen = false;
			
			// show menu
			show_menu_bar(hWnd);
		} else {
			SetWindowPos(hWnd, NULL, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		}
		
		RECT rect_tmp;
		GetClientRect(hWnd, &rect_tmp);
		if(rect_tmp.bottom != height) {
			rect.bottom += height - rect_tmp.bottom;
			dest_y = (int)((desktop_height - (rect.bottom - rect.top)) / 2);
			dest_y = (dest_y < 0) ? 0 : dest_y;
			SetWindowPos(hWnd, NULL, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		}
		config.window_mode = prev_window_mode = mode;
		
		// set screen size to emu class
		emu->set_display_size(width, height, true);
	} else if(!now_fullscreen) {
		// fullscreen
		int width = (mode == -1) ? desktop_width : screen_mode_width[mode - MAX_WINDOW];
		int height = (mode == -1) ? desktop_height : screen_mode_height[mode - MAX_WINDOW];
		
		DEVMODE dev;
		ZeroMemory(&dev, sizeof(dev));
		dev.dmSize = sizeof(dev);
		dev.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		dev.dmBitsPerPel = desktop_bpp;
		dev.dmPelsWidth = width;
		dev.dmPelsHeight = height;
		
		if(ChangeDisplaySettings(&dev, CDS_TEST) == DISP_CHANGE_SUCCESSFUL) {
			GetWindowPlacement(hWnd, &place);
			ChangeDisplaySettings(&dev, CDS_FULLSCREEN);
			style = GetWindowLong(hWnd, GWL_STYLE);
			SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);
			SetCursorPos(width / 2, height / 2);
			now_fullscreen = true;
			if(mode == -1) {
				for(int i = 0; i < screen_mode_count; i++) {
					if(screen_mode_width[i] == desktop_width && screen_mode_height[i] == desktop_height) {
						mode = i + MAX_WINDOW;
						break;
					}
				}
			}
			config.window_mode = mode;
			
			// remove menu
			hide_menu_bar(hWnd);
			
			// set screen size to emu class
			emu->set_display_size(width, height, false);
		}
	}
}

