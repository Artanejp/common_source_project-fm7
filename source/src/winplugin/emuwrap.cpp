// CSCP Unity Plugin Wrapper
// By Takeshi Maruyama
//

#include "dllmain.h"
#include "emuwrap.h"

extern "C" HWND hwndApp;
extern "C" HINSTANCE hInst;

extern EMU* emu;

static bool bInitialized = false;		// プラグイン初期化済みか

extern "C"
{
	IUnityInterfaces* g_unity = nullptr;
	emuwrap*       g_emuwrap = nullptr;
}

#ifdef _UNITY

const char *strClassName = "CSCPWindowClass";
const char *strTitle = "CSCPUnvisibleWindow";

LRESULT CALLBACK UnvisibleWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

#endif

/*
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	//クラス名取得 
	DWORD pid = 0;
	GetWindowThreadProcessId(hWnd, &pid);

	if (pid == 0)
	{
		return TRUE;
	}

	DWORD current = GetCurrentProcessId();

	if (pid == current) {
		hwndApp = hWnd;
		hInst = (HINSTANCE)GetModuleHandle(NULL);
		return FALSE;
	}
	return TRUE;
}

HWND GetWindowHandleByPID(const DWORD targetPID)
{
	HWND hWnd = GetTopWindow(NULL);
	do {
		if (GetWindowLongPtr(hWnd, GWLP_HWNDPARENT) != 0 || !IsWindowVisible(hWnd)) {
			continue;
		}
		DWORD getPID;
		GetWindowThreadProcessId(hWnd, &getPID);
		if (targetPID == getPID) {
			return hWnd;
		}
	} while ((hWnd = GetNextWindow(hWnd, GW_HWNDNEXT)) != NULL);

	return NULL;
}
 */

void emuwrap::init()
{
	if (bInitialized) return;

	// オーディオストリーミング変換テーブル
	m_wavtbl = new float[0x10000];
	for (int i = 0; i < 0x10000; i++)
	{
		int16_t w = (int16_t)i;
		double a = (double)w / 32768.0;
		m_wavtbl[i] = (float)a;
	}

	hInst = (HINSTANCE)GetModuleHandle(NULL);
//	DWORD current = GetCurrentProcessId();
//	hwndApp = GetWindowHandleByPID(current);
//	EnumWindows(EnumWindowsProc, NULL);

	// ウィンドウクラスの登録
	WNDCLASSEX wcex = { };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = WS_EX_NOACTIVATE;
	wcex.lpfnWndProc = UnvisibleWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = nullptr;
	wcex.hCursor = nullptr;
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = strClassName;
	wcex.hIconSm = nullptr;

	if (!RegisterClassEx(&wcex)) return;

	// ウィンドウ作成
	hwndApp = CreateWindowEx(WS_EX_NOACTIVATE, strClassName, strTitle, WS_DISABLED /*WS_OVERLAPPEDWINDOW*/,
		0, 0, 1, 1,
		nullptr, nullptr, hInst, nullptr);
	if (!hwndApp) return;

	ShowWindow(hwndApp, SW_HIDE); // Window非表示

	//
	emu = new EMU(hwndApp, hInst);
	emu->set_host_window_size(m_width, m_height, true);

	m_offscreen = new scrntype_t[m_width * m_height];
	bInitialized = true;
}

void emuwrap::dispose()
{
	if (!bInitialized) return;

	g_emuwrap->stopEmulation();
	delete emu;
	emu = nullptr;

	delete[] m_offscreen;
	m_offscreen = nullptr;

	delete[] m_wavtbl;
	m_wavtbl = nullptr;

	PostQuitMessage(0);

	bInitialized = false;
}

void emuwrap::startEmulation()
{
	if (!bInitialized) return;

	// initialize emulation core
	initialize_config();
	config.use_d2d1 = true;

	// メインループ

	// メインスレッド
	m_thread = std::thread([this] {
		m_isRunning = true;

//		SetThreadPriority(m_thread.native_handle(), THREAD_PRIORITY_HIGHEST);
		SetThreadPriority(m_thread.native_handle(), THREAD_PRIORITY_TIME_CRITICAL);

		// main loop
		int total_frames = 0, draw_frames = 0, skip_frames = 0;
		DWORD next_time = 0;
		bool prev_skip = false;
		DWORD update_fps_time = 0;
		DWORD update_status_bar_time = 0;
		DWORD disable_screen_saver_time = 0;

		MSG msg = { };

		while (m_isRunning && WM_QUIT != msg.message) {
			auto start = std::chrono::system_clock::now();      // 計測スタート時刻を保存
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				if (m_isSleep) {									// 実行をスリープしてる？
					std::this_thread::yield();
					continue;
				}

				if (emu != nullptr) {
					// drive machine
					int run_frames = emu->run();
					total_frames += run_frames;

					// timing controls
					int sleep_period = 0;
					bool now_skip = (config.full_speed || emu->is_frame_skippable()) && !emu->is_video_recording() && !emu->is_sound_recording();

					if ((prev_skip && !now_skip) || next_time == 0) {
						next_time = timeGetTime();
					}
					if (!now_skip) {
						static int accum = 0;
						accum += emu->get_frame_interval();
						int interval = accum >> 10;
						accum -= interval << 10;
						next_time += interval;
					}
					prev_skip = now_skip;

					if (next_time > timeGetTime()) {
						// update window if enough time
						draw_frames += emu->draw_screen();
						skip_frames = 0;

						// sleep 1 frame priod if need
						DWORD current_time = timeGetTime();
						if ((int)(next_time - current_time) >= 10) {
							sleep_period = next_time - current_time;
						}
					}
					else if (++skip_frames > (int)emu->get_frame_rate()) {
						// update window at least once per 1 sec in virtual machine time
						draw_frames += emu->draw_screen();
						skip_frames = 0;
						next_time = timeGetTime();
					}


					//				Sleep(sleep_period);
					if (sleep_period > 0) {
						std::chrono::milliseconds dura(sleep_period);
						std::this_thread::sleep_for(dura);
					}
					else {
						std::this_thread::yield();
					}


					// calc frame rate
					DWORD current_time = timeGetTime();
					if (update_fps_time <= current_time) {
						if (update_fps_time != 0) {
							if (emu->message_count > 0) {
								//									SetWindowText(hWnd, create_string(_T("%s - %s"), _T(DEVICE_NAME), emu->message));
								emu->message_count--;
							}
							else if (now_skip) {
								int ratio = (int)(100.0 * (double)total_frames / emu->get_frame_rate() + 0.5);
								//								SetWindowText(hWnd, create_string(_T("%s - Skip Frames (%d %%)"), _T(DEVICE_NAME), ratio));
							}
							else {
								int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);
								//								SetWindowText(hWnd, create_string(_T("%s - %d fps (%d %%)"), _T(DEVICE_NAME), draw_frames, ratio));
							}
							update_fps_time += 1000;
							total_frames = draw_frames = 0;
						}
						update_fps_time = current_time + 1000;
					}

					{
						std::lock_guard<std::mutex> lock(m_mutex);
						// 画面書き換え
						scrntype_t* dest = m_offscreen;
						uint32_t data = 0;
						for (int i = 0; i < m_height; i++) {
							scrntype_t* src = emu->get_osd()->get_vm_screen_buffer(m_height - i - 1);
							for (int j = 0; j < m_width; j++) {
								// in common.h
								// #define RGB_COLOR(r, g, b)	(((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 0))

								data = *(src++);
								uint32_t tmpr = data & 0x00FF0000;
								uint32_t tmpg = data & 0x000000FF;

								*(dest++) = scrntype_t((data & 0x00FF00) | (tmpr >> 16) | (tmpg << 16));	// AABBGGRR
							}
						}
					} // !lock_guard
				}
			} 
		} // winmain
	}); // lambda
}

void emuwrap::stopEmulation()
{
	// スレッド停止
	m_isSleep = true;
	m_isRunning = false;
	if (m_thread.joinable()) {
		m_thread.join();
	}
	SetThreadPriority(m_thread.native_handle(), THREAD_PRIORITY_NORMAL);

}

void emuwrap::setPause(bool fPause)
{
	m_isSleep = fPause;
}

void emuwrap::TextureUpdate()
{
	if (m_unity == nullptr || m_isRunning == false || m_texture == nullptr || m_offscreen == nullptr) return;

	std::lock_guard<std::mutex> lock(m_mutex);
	auto device = m_unity->Get<IUnityGraphicsD3D11>()->GetDevice();
	ID3D11DeviceContext* context;
	device->GetImmediateContext(&context);
	context->UpdateSubresource(m_texture, 0, nullptr, m_offscreen, m_width*4, 0);
}

void emuwrap::emuSendAudio(float data[], int sz, int ch)
{
/*
	int16_t *buf = emu->get_osd()->get_sound_buffer();
	if (buf == NULL) return;

	std::lock_guard<std::mutex> lock(m_mutex);
	for (int i = 0; i < sz; i++)
	{
		data[i] = m_wavtbl[buf[i]];

	}
 */
}

extern "C"
{
	// Low-Level Native Plugin Interface で Unity 側から呼ばれる
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
	{
		g_unity = unityInterfaces;
		g_emuwrap = new emuwrap(g_unity);
		g_emuwrap->init();
	}

	// Low-Level Native Plugin Interface で Unity 側から呼ばれる
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginUnload()
	{
		if (!bInitialized) return;

		g_emuwrap->dispose();

		delete g_emuwrap;
		g_emuwrap = nullptr;
	}

	// Unity 側で GL.IssuePlugin(この関数のポインタ, eventId) を呼ぶとレンダリングスレッドから呼ばれる
	void UNITY_INTERFACE_API OnRenderEvent(int eventId)
	{
		if (g_emuwrap) g_emuwrap->TextureUpdate();
	}

	// GL.IssuePlugin で登録するコールバック関数のポインタを返す
	UNITY_INTERFACE_EXPORT UnityRenderingEvent UNITY_INTERFACE_API GetRenderEventFunc()
	{
		return OnRenderEvent;
	}

	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Exec()
	{
		if (!bInitialized) return;
		g_emuwrap->startEmulation();
	}

	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API StopEmulation()
	{
		if (!bInitialized) return;
		g_emuwrap->stopEmulation();
	}

	UNITY_INTERFACE_EXPORT void* UNITY_INTERFACE_API CheckResident()
	{
		return g_emuwrap;
	}


	UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetEmulWidth(void* ptr)
	{
		auto emucore = reinterpret_cast<emuwrap*>(ptr);
		return emucore->GetWidth();
	}

	UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetEmulHeight(void* ptr)
	{
		auto emucore = reinterpret_cast<emuwrap*>(ptr);
		return emucore->GetHeight();
	}

	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetEmulTexturePtr(void* ptr, void* texture)
	{
		auto emucore = reinterpret_cast<emuwrap*>(ptr);
		emucore->SetTexturePtr(texture);
	}

	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetEmulPause(void* ptr, bool fPause)
	{
		auto emucore = reinterpret_cast<emuwrap*>(ptr);
		emucore->setPause(fPause);
	}

	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API EmulKeyUp(int keyCode)
	{
		//		keyup(keyCode);
	}

	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API EmulKeyDown(int keyCode)
	{
		//		keydown(keyCode);
	}

	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API EmulSetQDFile(int fno)
	{
		//		set_file_no(fno);
	}

	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API EmulReset(void)
	{
		//		reset();
	}

	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API EmuSendAudio(float data[], int sz, int ch)
	{
		if (!bInitialized) return;
		g_emuwrap->emuSendAudio(data, sz, ch);
	}

};
