#pragma once

#ifndef __emuwrap_H__
#define __emuwrap_H__

#include <d3d11.h>
#include <thread>
#include <mutex>
#include <cstdint>

#include "..\emu.h"

#include "Unity/IUnityInterface.h"
#include "Unity/IUnityGraphics.h"
#include "Unity/IUnityGraphicsD3D11.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif



class emuwrap
{
public:
	emuwrap(IUnityInterfaces* unity)
		: m_width(WINDOW_WIDTH)
		, m_height(WINDOW_HEIGHT)
		, m_unity(unity)
	{

	}

	~emuwrap()
	{

	}

public:
	void init();
	void TextureUpdate();
	void dispose();

	void startEmulation();
	void stopEmulation();

	int GetWidth() const
	{
		return m_width;
	}

	int GetHeight() const
	{
		return m_height;
	}

	void setPause(bool fPause);

	void SetTexturePtr(void* ptr)
	{
		m_texture = static_cast<ID3D11Texture2D*>(ptr);
	}

	void emuSendAudio(float data[], int sz, int ch);

private:
	int m_width, m_height;

	scrntype_t* m_offscreen = nullptr;
	IUnityInterfaces* m_unity;
	ID3D11Texture2D* m_texture = nullptr;

	std::thread m_thread;
	std::mutex m_mutex;
	bool m_isRunning = false;
	bool m_isSleep = false;

	float *m_wavtbl = nullptr;

};

extern "C"
{
	extern IUnityInterfaces* g_unity;
	extern emuwrap*       g_emuwrap;
}

extern "C"
{
	// Low-Level Native Plugin Interface で Unity 側から呼ばれる
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces);
	// Low-Level Native Plugin Interface で Unity 側から呼ばれる
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginUnload();
	// Unity 側で GL.IssuePlugin(この関数のポインタ, eventId) を呼ぶとレンダリングスレッドから呼ばれる
	void UNITY_INTERFACE_API OnRenderEvent(int eventId);
	// GL.IssuePlugin で登録するコールバック関数のポインタを返す
	UNITY_INTERFACE_EXPORT UnityRenderingEvent UNITY_INTERFACE_API GetRenderEventFunc();
//	UNITY_INTERFACE_EXPORT void* UNITY_INTERFACE_API Init();
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Exec();
//	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Dispose();
	UNITY_INTERFACE_EXPORT void* UNITY_INTERFACE_API CheckResident();
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API StopEmulation();
	//
	UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetEmulWidth(void* ptr);
	UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetEmulHeight(void* ptr);
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetEmulTexturePtr(void* ptr, void* texture);
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetEmulPause(void* ptr, bool fPause);

	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API EmulKeyUp(int keyCode);
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API EmulKeyDown(int keyCode);
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API EmulSetQDFile(int fno);
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API EmulReset(void);

	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API EmuSendAudio(float data[], int sz, int channels);

};

#endif // ! __emuwrap_H__
