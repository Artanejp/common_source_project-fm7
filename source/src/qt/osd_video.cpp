/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 DirectShow ]
*/

#include "../emu.h"

//#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
void OSD_BASE::initialize_video()
{
}

#define SAFE_RELEASE(x) { \
	if(x != NULL) { \
		(x)->Release(); \
		(x) = NULL; \
	} \
}

void OSD_BASE::release_video()
{
}

void OSD_BASE::get_video_buffer()
{
   
}

void OSD_BASE::mute_video_dev(bool l, bool r)
{
}
//#endif // #if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)


bool OSD_BASE::open_movie_file(const _TCHAR* file_path)
{
	return false;
}

void OSD_BASE::close_movie_file()
{
	now_movie_play = false;
	now_movie_pause = false;
}

void OSD_BASE::play_movie()
{
	now_movie_play = true;
	now_movie_pause = false;
	emit sig_movie_play();
}

void OSD_BASE::stop_movie()
{
	now_movie_play = false;
	now_movie_pause = false;
	emit sig_movie_stop();
}

void OSD_BASE::pause_movie()
{
	now_movie_pause = true;
	emit sig_movie_pause(now_movie_pause);
}

void OSD_BASE::set_cur_movie_frame(int frame, bool relative)
{
	emit sig_movie_seek_frame(relative, frame);
}

uint32_t OSD_BASE::get_cur_movie_frame()
{
	return 0;
}

//#ifdef USE_VIDEO_CAPTURE
void OSD_BASE::enum_capture_devs()
{
#if 0	
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	
	num_capture_devs = 0;
	
	// create the system device enum
	if(SUCCEEDED(CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&pDevEnum))) {
		// create the video input device enu,
		if(SUCCEEDED(pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0)) && pClassEnum != NULL) {
			ULONG cFetched;
			IMoniker *pMoniker = NULL;
			
			while(num_capture_devs < MAX_CAPTURE_DEVS && SUCCEEDED(pClassEnum->Next(1, &pMoniker, &cFetched)) && pMoniker != NULL) {
				IPropertyBag *pBag = NULL;
				if(SUCCEEDED(pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag))) {
					VARIANT var;
					var.vt = VT_BSTR;
					if(pBag->Read(L"FriendlyName", &var, NULL) == NOERROR) {
						LPCWSTR _lpw = NULL;
						int _convert = 0;
						_tcscpy_s(capture_dev_name[num_capture_devs++], 256, MyW2T(var.bstrVal));
						SysFreeString(var.bstrVal);
						pMoniker->AddRef();
					}
					SAFE_RELEASE(pBag);
				}
				SAFE_RELEASE(pMoniker);
			}
		}
	}
	SAFE_RELEASE(pClassEnum);
	SAFE_RELEASE(pDevEnum);
#endif	
}

bool OSD_BASE::connect_capture_dev(int index, bool pin)
{
	if(cur_capture_dev_index == index && !pin) {
		return true;
	}
	if(cur_capture_dev_index != -1) {
		//release_video();
		cur_capture_dev_index = -1;
	}
	cur_capture_dev_index = index;
	return true;
}

void OSD_BASE::open_capture_dev(int index, bool pin)
{
	if(!connect_capture_dev(index, pin)) {
		//release_video();
	}
}

void OSD_BASE::close_capture_dev()
{
	//release_video();
	cur_capture_dev_index = -1;
}

void OSD_BASE::show_capture_dev_filter()
{
}

void OSD_BASE::show_capture_dev_pin()
{
	std::lock_guard<std::recursive_timed_mutex> Locker_S(screen_mutex);
#if 0	
	if(cur_capture_dev_index != -1) {
		if(!connect_capture_dev(cur_capture_dev_index, true)) {
			//release_video();
		}
	}
#endif
}

void OSD_BASE::show_capture_dev_source()
{
	std::lock_guard<std::recursive_timed_mutex> Locker_S(screen_mutex);
#if 0
	if(pCaptureGraphBuilder2 != NULL) {
		IAMCrossbar *pCrs = NULL;
		ISpecifyPropertyPages *pSpec = NULL;
		CAUUID cauuid;
		
		if(FAILED(pCaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, pCaptureBaseFilter, IID_IAMCrossbar, (void **)&pCrs))) {
			if(FAILED(pCaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pCaptureBaseFilter, IID_IAMCrossbar, (void **)&pCrs))) {
				return;
			}
		}
		if(SUCCEEDED(pCrs->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec))) {
			pSpec->GetPages(&cauuid);
			OleCreatePropertyFrame(NULL, 30, 30, NULL, 1, (IUnknown **)&pCrs, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL);
			CoTaskMemFree(cauuid.pElems);
			SAFE_RELEASE(pSpec);
			
			AM_MEDIA_TYPE mt;
			pVideoSampleGrabber->GetConnectedMediaType(&mt);
			VIDEOINFOHEADER *pVideoHeader = (VIDEOINFOHEADER*)mt.pbFormat;
			direct_show_width = pVideoHeader->bmiHeader.biWidth;
			direct_show_height = pVideoHeader->bmiHeader.biHeight;
			
			if(dshow_screen_buffer.width != direct_show_width || dshow_screen_buffer.height != direct_show_height) {
				initialize_screen_buffer(&dshow_screen_buffer, direct_show_width, direct_show_height, COLORONCOLOR);
			}
		}
		SAFE_RELEASE(pCrs);
	}
#endif	
}

void OSD_BASE::set_capture_dev_channel(int ch)
{
#if 0	
	IAMTVTuner *pTuner = NULL;
	
	if(pCaptureGraphBuilder2 != NULL) {
		if(SUCCEEDED(pCaptureGraphBuilder2->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pCaptureBaseFilter, IID_IAMTVTuner, (void **)&pTuner))) {
			pTuner->put_Channel(ch, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);
			SAFE_RELEASE(pTuner);
		}
	}
#endif	
}

double OSD_BASE::get_movie_frame_rate()
{
	return movie_frame_rate;
}

int OSD_BASE::get_movie_sound_rate()
{
	return movie_sound_rate;
}

_TCHAR* OSD_BASE::get_capture_dev_name(int index)
{
	return capture_dev_name[index];
}

void OSD_BASE::do_decode_movie(int frames)
{
}

void OSD_BASE::do_video_movie_end(bool flag)
{
	
}

void OSD_BASE::do_video_decoding_error(int num)
{
	
}

void OSD_BASE::do_run_movie_audio_callback(uint8_t *data, long len)
{
}
