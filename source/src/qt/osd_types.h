/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

#ifndef _QT_OSD_TYPES_H_
#define _QT_OSD_TYPES_H_


#include <QWidget>
#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include <QPainter>
#include <QElapsedTimer>
#include <QString>

#include <SDL.h>
#include "simd_types.h"

#include <ctime>

//#include "../vm/vm.h"
//#include "../emu.h"
#include "../config.h"
#include "../fileio.h"
#include "../fifo.h"

#if !defined(Q_OS_WIN32)
#include "qt_input.h"
#endif
typedef struct {
	Sint16 **sound_buf_ptr;
	int *sound_buffer_size;
	int *sound_write_pos;
	int *sound_data_len;
	SDL_sem **snd_apply_sem;
	Uint8 *snd_total_volume;
	bool *sound_exit;
	bool *sound_debug;
	config_t *p_config;
} sdl_snddata_t;


#if 0 // TODO
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
ISampleGrabberCB : public IUnknown {
public:
	virtual HRESULT STDMETHODCALLTYPE SampleCB( double SampleTime,IMediaSample *pSample) = 0;
	virtual HRESULT STDMETHODCALLTYPE BufferCB( double SampleTime,BYTE *pBuffer,long BufferLen) = 0;
};
EXTERN_C const IID IID_ISampleGrabber;
MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber : public IUnknown {
public:
	virtual HRESULT STDMETHODCALLTYPE SetOneShot( BOOL OneShot) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMediaType( const AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples( BOOL BufferThem) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer( /* [out][in] */ long *pBufferSize,/* [out] */ long *pBuffer) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample( /* [retval][out] */ IMediaSample **ppSample) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetCallback( ISampleGrabberCB *pCallback,long WhichMethodToCallback) = 0;
};
#endif
#ifdef USE_MOVIE_PLAYER
class CMySampleGrabberCB : public ISampleGrabberCB {
private:
	VM *vm;
public:
	CMySampleGrabberCB(VM *vm_ptr)
	{
		vm = vm_ptr;
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		return 2;
	}
	STDMETHODIMP_(ULONG) Release()
	{
		return 1;
	}
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if(riid == IID_ISampleGrabberCB || riid == IID_IUnknown) {
			*ppv = (void *) static_cast<ISampleGrabberCB*>(this);
			return NOERROR;
		}
		return E_NOINTERFACE;
	}
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample)
	{
		return S_OK;
	}
	STDMETHODIMP BufferCB(double dblSampleTime, BYTE *pBuffer, long lBufferSize)
	{
		vm->movie_sound_callback(pBuffer, lBufferSize);
		return S_OK;
	}
};
#endif
#endif

typedef struct bitmap_s {
	int width, height;
	QImage pImage;
	scrntype_t *get_buffer(int y) {
		return (scrntype_t *)pImage.scanLine(y);
	};
	scrntype_t* lpBuf;
	QPainter hPainter;
} bitmap_t;

typedef struct font_s {
	// common
	_TCHAR family[64];
	int width, height;
	int rotate;
	bool bold, italic;
	bool init_flag;
	bool initialized(void) {
		return init_flag;
	};
	// win32 dependent
	QFont hFont;
} font_t;

typedef struct pen_s {
	// common
	int width;
	uint8_t r, g, b;
	// win32 dependent
	QPen hPen;
} pen_t;


typedef struct {
	//PAVISTREAM pAVICompressed;
	scrntype_t* lpBmp;
	//LPBITMAPINFOHEADER pbmInfoHeader;
	DWORD dwAVIFileSize;
	UINT64 lAVIFrames;
	int frames;
	int result;
} rec_video_thread_param_t;

#endif
