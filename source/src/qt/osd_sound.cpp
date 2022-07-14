/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[Qt/SDL sound ]
*/
/*
  Note: 20220715 K.Ohta : This will be based on QtMultimedia's audio driver,
		But this still has a lot of delay.Will fix or discard.
  Quote from "Audio Overview" of Qt6.3.1 's documentation;
	https://doc.qt.io/qt-6/audiooverview.html 
	Push and Pull
		The low level audio classes can operate in two modes - push and pull. 
		In pull mode, the audio device is started by giving it a QIODevice. 
		For an output device, the QAudioSink class will pull data from the 
		QIODevice (using QIODevice::read()) when more audio data is required. 
		Conversely, for pull mode with QAudioSource, when audio data is available
		then the data will be written directly to the QIODevice.
		
		In push mode, the audio device provides a QIODevice instance that can be
		written or read to as needed. 
		Typically, this results in simpler code but more buffering, 
		which may affect latency.
*/

#include "../emu.h"
#include "../fileio.h"
#include "../fifo.h"
#include "../types/util_sound.h"
#include "../types/util_endians.h"

#include <SDL.h>

#include "qt_main.h"
//#include "csp_logger.h"
#include "gui/menu_flags.h"

#include <QString>
#include <QDateTime>
#include <QThread>
#include <QByteArray>
#include <QBuffer>
#include <cstdint>

void OSD_BASE::audio_capture_callback(void *udata, Uint8 *stream, int len)
{
	if(len <= 0) return;
	if(udata == NULL) return;
	if(stream == NULL) return;
	int len2, len3;
	osd_snddata_capture_t *pData = (osd_snddata_capture_t *)udata;

	if(pData->read_buffer_ptr == NULL) return;
	if((pData->writepos + len) >= pData->buffer_size) {
		// Need Wrap
		len2 = pData->buffer_size - pData->writepos;
		if(len2 > len) len2 = len;
		memset(&(pData->read_buffer_ptr[pData->writepos]), 0x00, len2);
		memcpy(&(pData->read_buffer_ptr[pData->writepos]), stream, len2);
		len3 = len - len2;
		if(len3 > 0) {
			memset(&(pData->read_buffer_ptr[0]), 0x00, len3);
			memcpy(&(pData->read_buffer_ptr[0]), &(stream[len2]), len3);
		}
	} else {
		// Not need to wrap
		memset(&(pData->read_buffer_ptr[pData->writepos]), 0x00, len);
		memcpy(&(pData->read_buffer_ptr[pData->writepos]), stream, len);
	}
	pData->writepos += len;
	if(pData->writepos >= pData->buffer_size) {
		pData->writepos -= pData->buffer_size;
	}
	pData->writelen += len;
	if(pData->writelen >= pData->buffer_size) {
		pData->writelen = pData->buffer_size;
	}
}


void OSD_BASE::audio_callback(void *udata, Uint8 *stream, int len)
{
	if(len <= 0) return;
	if(udata == NULL) return;
	if(stream == NULL) return;
	int len2, len3;
	sdl_snddata_t *pData = (sdl_snddata_t *)udata;
	int sndlen, sndpos, bufsize;
	int spos = 0;
	int format_len = 1;
	
	len3 = len;
	memset(stream, 0x00, len);
	switch(pData->sound_format) {
	case AUDIO_S16LSB:
	case AUDIO_S16MSB:
	case AUDIO_U16LSB:
	case AUDIO_U16MSB:
		format_len = sizeof(int16_t);
		break;
	case AUDIO_S32LSB:
	case AUDIO_S32MSB:
		format_len = sizeof(int32_t);
		break;
	case AUDIO_F32LSB:
	case AUDIO_F32MSB:
		format_len = sizeof(float);
		break;
	}
	
	
	if(pData->p_config->general_sound_level < INT16_MIN) pData->p_config->general_sound_level = INT16_MIN;
	if(pData->p_config->general_sound_level > INT16_MAX)  pData->p_config->general_sound_level = INT16_MAX;
	*pData->snd_total_volume = (uint8_t)(((uint32_t)(pData->p_config->general_sound_level + (-INT16_MIN))) >> 9);
		
	do {
		sndlen = *(pData->sound_data_len);
		bufsize = *(pData->sound_buffer_size);
		sndpos = *(pData->sound_write_pos);
		if(*pData->sound_exit) {
			return;
		}
		
		sndpos = sndpos * format_len;
		sndlen = sndlen * format_len; // ToDo: Multiple format
		bufsize = bufsize * format_len; // ToDo: Multiple format
		
		if(sndlen >= len) sndlen = len;
		if((sndpos + sndlen) >= bufsize) { // Need to wrap
			int len2 = bufsize - sndpos;
			uint8_t* p = (uint8_t *)(*pData->sound_buf_ptr);
			uint8_t* s = &stream[spos];
			p = &p[sndpos];
#if defined(USE_SDL2)
			SDL_MixAudioFormat(s, p, pData->sound_format, len2, *(pData->snd_total_volume));
#else
			SDL_MixAudio(s, p, len2, *(pData->snd_total_volume));
#endif
			spos += len2;
			len2 = sndlen - len2;
			s = &stream[spos];
			*(pData->sound_write_pos) = 0;
			if(len2 > 0) {
				p = (uint8_t *)(*pData->sound_buf_ptr);
				p = &p[0];
#if defined(USE_SDL2)
				SDL_MixAudioFormat(s, p, pData->sound_format, len2, *(pData->snd_total_volume));
#else
				SDL_MixAudio(s, p, len2, *(pData->snd_total_volume));
#endif
				*(pData->sound_write_pos) = (len2 / format_len);
				if(*(pData->sound_buffer_size) <= *(pData->sound_write_pos)) {
					*(pData->sound_write_pos) = 0;
				}
				sndpos = len2;
				spos += len2;
				len2 = 0;
			}
			len -= sndlen;
		} else { // No Need to wrap
			int len2 = sndlen;
			uint8_t* p = (uint8_t *)(*pData->sound_buf_ptr);
			uint8_t* s = &stream[spos];
			p = &p[sndpos];
#if defined(USE_SDL2)
			SDL_MixAudioFormat(s, p, pData->sound_format, len2, *(pData->snd_total_volume));
#else
			SDL_MixAudio(s, p, len2, *(pData->snd_total_volume));
#endif
			*(pData->sound_write_pos) += (len2 / format_len); 
			if(*(pData->sound_buffer_size) <= *(pData->sound_write_pos)) {
				*(pData->sound_write_pos) = 0;
			}
			spos += len2;
			len -= sndlen;
		}
		*(pData->sound_data_len)  -= (sndlen / format_len);
		if(*(pData->sound_data_len) <= 0) return;
		if(spos >= len3) return;
		// WIP: Do place wait (1ms)?
	} while(len > 0);
}

const _TCHAR *OSD_BASE::get_vm_device_name()
{
	if(using_flags != NULL) {
		QString s = using_flags->get_device_name();
		static QByteArray __n = s.toUtf8();
		return (const _TCHAR*)(__n.constData());
	}
	return (const _TCHAR*)"";
}

const _TCHAR *OSD_BASE::get_sound_device_name(int num)
{
	if(num < 0) return NULL;
	if(num >= sound_device_list.count()) return NULL;
	QString sdev = sound_device_list.at(num);
	sdev.truncate(64);
	static QByteArray _n;
	_n.clear();
	_n = sdev.toUtf8().constData();

	return (const _TCHAR*)(_n.constData());
}

int OSD_BASE::get_sound_device_num()
{
	return sound_device_list.count();
}
#if 0
void OSD_BASE::get_sound_device_list()
{
	sound_device_list.clear();
#if defined(USE_SDL2)
	const _TCHAR* drvname = SDL_GetCurrentAudioDriver();
	if(drvname == nullptr) return;
	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, "Using Sound Driver: %s\n", drvname);
	for(int i = 0; i < SDL_GetNumAudioDevices(0); i++) {
		QString tmps = QString::fromUtf8(SDL_GetAudioDeviceName(i, 0));
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,
				  "Audio Device #%d: %s", i, tmps.toLocal8Bit().constData());
		sound_device_list.append(tmps);
	}
#endif
}

void OSD_BASE::initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples)
{
	std::string devname;
	int i;

	bool pre_initialized = sound_initialized;
	if(sound_initialized) {
		release_sound();
	}

	sound_capture_device_list.clear();
#if 0
	for(int ch = 0 ; ch < MAX_CAPTURE_SOUND; ch++) {
		// ToDo: Allocation.
		// ToDo: Close physical device.
		if(!pre_initialized) {
			sound_capturing_emu[ch] = false;
			
			sound_capture_desc[ch].physical_dev = 0;
			sound_capture_desc[ch].read_format = AUDIO_S16SYS;
			sound_capture_desc[ch].read_rate = 0;
			sound_capture_desc[ch].read_channels = 0;
			sound_capture_desc[ch].read_samples = 0;
			sound_capture_desc[ch].read_silence = 0;
			sound_capture_desc[ch].read_size = 0;
			sound_capture_desc[ch].read_userdata = NULL;
			sound_capture_desc[ch].sample_type = 0; // ToDo : ENUM
			sound_capture_desc[ch].rate = 0;
			sound_capture_desc[ch].channels = 0;
			sound_capture_desc[ch].samples = 0;
			sound_capture_desc[ch].write_size = 0;
			sound_capture_desc[ch].write_pos = 0;
			sound_capture_desc[ch].read_data_len = 0;
			sound_capture_desc[ch].read_buffer_len = 0;
			sound_capture_desc[ch].read_buffer_ptr = NULL;
			sound_capture_desc[ch].out_buffer = NULL;
		}
	}
	for(int num = 0 ; num < MAX_SOUND_CAPTURE_DEVICES; num++) {
		sound_capture_dev_desc[num].format = AUDIO_S16SYS;
		sound_capture_dev_desc[num].sample_rate = 0;
		sound_capture_dev_desc[num].channels = 0;
		sound_capture_dev_desc[num].buffer_samples = 0;
		sound_capture_dev_desc[num].silence = 0;
		sound_capture_dev_desc[num].size = 0;
		sound_capture_dev_desc[num].callback = audio_capture_callback;
		sound_capture_dev_desc[num].userdata.format = AUDIO_S16SYS;
		sound_capture_dev_desc[num].userdata.buffer_size = 0;
		sound_capture_dev_desc[num].userdata.readlen = 0;
		sound_capture_dev_desc[num].userdata.writelen = 0;
		sound_capture_dev_desc[num].userdata.readpos = 0;
		sound_capture_dev_desc[num].userdata.read_buffer_ptr = NULL;
		capturing_sound[num] = false;
	}
#endif
	
	sound_rate = rate;
	sound_samples = samples;
	rec_sound_buffer_ptr = 0;
	sound_ok = sound_started = now_mute = now_record_sound = false;
	sound_write_pos = 0;
	sound_data_len = 0;
	sound_buffer_size = 0;
	sound_data_pos = 0;
	sound_exit = false;
	sound_debug = false;
	//sound_debug = true;
	sound_buf_ptr = NULL;
	sound_initialized = false;
	// initialize direct sound

	snd_total_volume = 127;
   
	snddata.sound_buf_ptr = (uint8_t**)(&sound_buf_ptr);
	snddata.sound_buffer_size = &sound_buffer_size;
	snddata.sound_write_pos = &sound_write_pos;
	snddata.sound_data_len = &sound_data_len;
	snddata.snd_total_volume = &snd_total_volume;
	snddata.sound_exit = &sound_exit;
	snddata.sound_debug = &sound_debug;
	snddata.p_config = p_config;
	
	snd_spec_req.format = AUDIO_S16SYS;
	snd_spec_req.channels = 2;
	snd_spec_req.freq = sound_rate;
	//snd_spec_req.samples = ((sound_rate * 100) / 1000);
	snd_spec_req.samples = samples;
	snd_spec_req.callback = &(this->audio_callback);
	snd_spec_req.userdata = (void *)&snddata;
#if defined(USE_SDL2)
	audio_dev_id = 0;
	if(!(sound_device_list.isEmpty())) {
		QString sdev;
		if(p_config->sound_device_num >= sound_device_list.count()) {
			p_config->sound_device_num = sound_device_list.count() - 1;
		}
		if(p_config->sound_device_num <= 0) {
			p_config->sound_device_num = 0;
		}
		sdev = sound_device_list.at(p_config->sound_device_num);
		audio_dev_id = SDL_OpenAudioDevice(sdev.toUtf8().constData(), 0,
										   &snd_spec_req, &snd_spec_presented,
										   0);
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, "Try to open DEVICE #%d: %s -> %s: DEVID=%d\n",
				  p_config->sound_device_num, sdev.toUtf8().constData(), (audio_dev_id <= 0) ? "FAIL" : "SUCCESS", audio_dev_id);
	} else {
		audio_dev_id = SDL_OpenAudioDevice("", 0,
										   &snd_spec_req, &snd_spec_presented,
										   0);
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, "Try to open DEVICE #%d:  %s: DEVID=%d\n",
				  p_config->sound_device_num, (audio_dev_id <= 0) ? "FAIL" : "SUCCESS", audio_dev_id);

	}
#else
	audio_dev_id = 1;
	SDL_OpenAudio(&snd_spec_req, &snd_spec_presented);
#endif

#if defined(USE_SDL2)
	if(audio_dev_id <= 0) {
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,"Failed to initialize sound\n");
		if(presented_rate != NULL) {
			*presented_rate = sound_rate;
		}
		if(presented_samples != NULL) {
			*presented_samples = sound_samples;
		}
		sound_initialized = false;
		sound_ok = sound_first_half = false;
		return;
	}
#endif
	snddata.sound_format = snd_spec_presented.format;
	if((snd_spec_presented.freq != sound_rate) ||
	   (snd_spec_presented.samples != sound_samples)) { // DEINI
		sound_rate = snd_spec_presented.freq;
		sound_samples = snd_spec_presented.samples;
	}
	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,"Sample rate=%d samples=%d\n", sound_rate, sound_samples);
	if(presented_rate != NULL) {
		*presented_rate = sound_rate;
	}
	if(presented_samples != NULL) {
		*presented_samples = sound_samples;
	}
	// secondary buffer
	int format_len = 1;
	switch(snddata.sound_format) {
	case AUDIO_S16LSB:
	case AUDIO_S16MSB:
	case AUDIO_U16LSB:
	case AUDIO_U16MSB:
		format_len = sizeof(int16_t);
		break;
	case AUDIO_S32LSB:
	case AUDIO_S32MSB:
		format_len = sizeof(int32_t);
		break;
	case AUDIO_F32LSB:
	case AUDIO_F32MSB:
		format_len = sizeof(float);
		break;
	}
	sound_buffer_size = sound_samples * snd_spec_presented.channels * 2;
	sound_buf_ptr = (uint8_t *)malloc(sound_buffer_size * format_len); 
	if(sound_buf_ptr == NULL) {
#if defined(USE_SDL2)   	   
		SDL_CloseAudioDevice(audio_dev_id);
#else	   
		SDL_CloseAudio();
#endif	   
		return;
	}

	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,
						  "Sound OK: BufSize = %d", sound_buffer_size);
	memset(sound_buf_ptr, 0x00, sound_buffer_size * format_len);
	sound_initialized = true;
	sound_ok = sound_first_half = true;
}

void OSD_BASE::release_sound()
{
	// release SDL sound
	sound_exit = true;
	sound_initialized = false;
#if 0
	for(int num = 0; num < MAX_SOUND_CAPTURE_DEVICES; num++) {
		if(capturing_sound[num]) {
			close_sound_capture_device(num, true);
		}
	}
	// ToDo: Reopen
	for(int ch = 0; ch < MAX_CAPTURE_SOUNDS; ch++) {
		if(sound_capturing_emu[ch]) {
			close_capture_sound_emu(ch);
		}
	}
#endif
#if defined(USE_SDL2)   
	//SDL_PauseAudioDevice(audio_dev_id, 1);
	SDL_CloseAudioDevice(audio_dev_id);
#else   
	SDL_CloseAudio();
#endif   
	stop_record_sound();
	if(sound_buf_ptr != NULL) free(sound_buf_ptr);
	sound_buf_ptr = NULL;
	// stop recording
}
#else
// QT_Multimedia
#include <QtMultimedia>

void OSD_BASE::initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples)
{
	// ToDo: Sound Input
	QAudioFormat desired;

	if(m_audioOutputSink != nullptr) {
		m_audioOutputSink->stop();
		delete m_audioOutputSink;
		m_audioOutputSink = nullptr;
	}
	desired.setChannelCount(2);
	desired.setSampleRate(rate);
	desired.setSampleFormat(QAudioFormat::Int16);
	desired.setChannelConfig(QAudioFormat::ChannelConfigStereo);

	bool output_available = false;
	if(m_audioOutputDevice.isFormatSupported(desired)) {
		    desired = m_audioOutputDevice.preferredFormat();
	}
	m_audioOutputFormat = desired;
	m_audioOutputSink = new QAudioSink(m_audioOutputDevice, m_audioOutputFormat, this);
//	if(m_audioOutput != nullptr) {
//		if(m_audioOutput->isOpen()) {
//			m_audioOutput->close();
//		}
//		delete m_audioOutput;
//		m_audioOutput = nullptr;
//	}
	if(m_audioOutputBuffer != nullptr) {
		delete m_audioOutputBuffer;
	}
	rate = m_audioOutputFormat.sampleRate();
	if(rate <= 0) rate = 8000;
	int wordsize = sizeof(int16_t);
	switch(m_audioOutputFormat.sampleFormat()) {
	case QAudioFormat::UInt8:
		wordsize = sizeof(uint8_t);
		break;
	case QAudioFormat::Int16:
		wordsize = sizeof(int16_t);
		break;
	case QAudioFormat::Int32:
		wordsize = sizeof(int32_t);
		break;
	case QAudioFormat::Float:
		wordsize = sizeof(float);
		break;
	}
	int outbuffer_length = 2 * 2 * samples * 4;
	m_audioOutputBuffer = new FIFO(outbuffer_length);
	if(m_audioOutputBuffer != nullptr) {
		sound_samples = samples;
		sound_rate = rate;
//		m_audioOutput = new QBuffer(m_audioOutputBuffer, this);
//		if(m_audioOutput != nullptr) {
//			m_audioOutput->open(QIODeviceBase::ReadWrite);
//		}
	} else {
		sound_samples = samples;
		sound_rate = rate;
	}
//	sound_samples = samples;
//	sound_rate = rate;
	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,"Sample rate=%d samples=%d\n", sound_rate, sound_samples);
	
	rec_sound_buffer_ptr = 0;
	sound_ok = sound_started = now_mute = now_record_sound = false;
	sound_write_pos = 0;
	sound_data_len = 0;
	sound_buffer_size = 0;
	sound_data_pos = 0;
	sound_exit = false;
	sound_debug = false;
	//sound_debug = true;
	sound_buf_ptr = NULL;

	if(presented_rate != nullptr) {
		*presented_rate = sound_rate;
	}
	if(presented_samples != nullptr) {
		*presented_samples = sound_samples;
	}

	
//	if(m_audioOutput != nullptr) {
		if(m_audioOutputSink != nullptr) {
			sound_ok = true;
			sound_initialized = true;
			//connect(m_audioOutputSink, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
			m_audioOutputSink->setBufferSize(sound_samples * 2 * sizeof(int16_t) * 4);
			m_audioOutput = m_audioOutputSink->start();
			//m_audioOutputSink->suspend();
		} else {
			sound_ok = false;
			sound_initialized = false;
			m_audioOutput = nullptr;
		}
//	}
	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,
						  "Sound OK: BufSize = %d", outbuffer_length);

}

void OSD_BASE::release_sound()
{
	// ToDo: Sound Input
	// release Qt Multimedia sound
	sound_exit = true;
	sound_initialized = false;
	if(m_audioOutputSink != nullptr) {
		m_audioOutputSink->stop();
		int timeout = 1000;
		while((m_audioOutputSink->state() != QAudio::StoppedState) && (timeout >= 0)) {
			QThread::usleep(1000); // Wait 1mS
			timeout--;
		}
		delete m_audioOutputSink;
		m_audioOutputSink = nullptr;
	}
//	if(m_audioOutput != nullptr) {
//		if(m_audioOutput->isOpen()) {
//			m_audioOutput->close();
//		}
//		delete m_audioOutput;
//		m_audioOutput = nullptr;
//	}
	if(m_audioOutputBuffer != nullptr) {
		delete m_audioOutputBuffer;
		m_audioOutputBuffer = nullptr;
	}
	sound_ok = false;
	sound_initialized = false;
}

void OSD_BASE::get_sound_device_list()
{
	sound_device_list.clear();
	QList<QAudioDevice> tmplist = QMediaDevices::audioOutputs();
	int i = 0;
	for(auto p = tmplist.begin(); p != tmplist.end(); ++p) {
		QString tmps = (*p).description();
		sound_device_list.append(tmps);
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,
				  "Audio Device #%d: %s", i, tmps.toLocal8Bit().constData());
		i++;
	}
}
		
#endif
void OSD_BASE::convert_sound_format(uint8_t* dst1, uint8_t* dst2, int16_t* src1, int16_t* src2, int samples1, int samples2)
{
	if(dst1 == NULL) return;
	if(snddata.sound_format == AUDIO_S16SYS) { // Not Convert
		if((src1 != NULL) && (samples1 > 0)) {
			my_memcpy(dst1, src1, samples1 * sizeof(int16_t));
		}
		if((src2 != NULL) && (samples2 > 0) && (dst2 != NULL)) {
			my_memcpy(dst2, src2, samples2 * sizeof(int16_t));
		}
		return;
	}

	union {
#if defined(__LITTLE_ENDIAN__)
		uint8_t l, h, h1, h2;
#else		
		uint8_t h2, h1, h, l;
#endif
		float f;
		uint32_t d;
	} float_data;
	
	switch(snddata.sound_format) {
		// S16SYS
	case AUDIO_S8:
		if(src1 != NULL) {
			int16_t *q = (int16_t*)src1;
			int8_t* p = (int8_t*)dst1;
			int dat;
			for(int i = 0; i < samples1; i++) {
				dat = q[i];
				dat >>= 8;
				p[i] = dat;
			}
		}
		if((src2 != NULL) && (dst2 != NULL)) {
			int16_t *q = (int16_t*)src2;
			int8_t* p = (int8_t*)dst2;
			int dat;
			for(int i = 0; i < samples2; i++) {
				dat = q[i];
				dat >>= 8;
				p[i] = dat;
			}
		}
		break;
	case AUDIO_U8:
		if(src1 != NULL) {
			int16_t *q = (int16_t*)src1;
			uint8_t* p = (uint8_t*)dst1;
			int dat;
			for(int i = 0; i < samples1; i++) {
				dat = q[i];
				dat += 32768;
				dat >>= 8;
				p[i] = dat;
			}
		}
		if((src2 != NULL) && (dst2 != NULL)) {
			int16_t *q = (int16_t*)src2;
			uint8_t* p = (uint8_t*)dst2;
			int dat;
			for(int i = 0; i < samples2; i++) {
				dat = q[i];
				dat += 32768;
				dat >>= 8;
				p[i] = dat;
			}
		}
		break;
	case AUDIO_S16LSB:
		if(src1 != NULL) {
			int16_t *q = (int16_t*)src1;
			int16_t* p = (int16_t*)dst1;
			pair16_t dat;
			for(int i = 0; i < samples1; i++) {
				dat.sw = q[i];
#if defined(__LITTLE_ENDIAN__)
				p[i] = dat.sw;
#else
				p[i] = dat.get_2bytes_le_to();
#endif
			}
		}
		if((src2 != NULL) && (dst2 != NULL)) {
			int16_t* q = (int16_t*)src2;
			int16_t* p = (int16_t*)dst2;
			pair16_t dat;
			for(int i = 0; i < samples2; i++) {
				dat.sw = q[i];
#if defined(__LITTLE_ENDIAN__)
				p[i] = dat.sw;
#else
				p[i] = dat.get_2bytes_le_to();
#endif
			}
		}
		break;
	case AUDIO_U16LSB:
		if(src1 != NULL) {
			int16_t* q = (int16_t*)src1;
			uint16_t* p = (uint16_t*)dst1;
			pair16_t dat;
			int32_t d2;
			for(int i = 0; i < samples1; i++) {
				d2 = q[i];
				d2 = d2 + 32768;
#if defined(__LITTLE_ENDIAN__)
				p[i] = d2 & 0xffff;
#else
				dat.w = d2 & 0xffff;
				p[i] = dat.get_2bytes_le_to();
#endif
			}
		}
		if((src2 != NULL) && (dst2 != NULL)) {
			int16_t* q = (int16_t*)src2;
			uint16_t* p = (uint16_t*)dst2;
			pair16_t dat;
			int d2;
			for(int i = 0; i < samples2; i++) {
				d2 = q[i];
				d2 = d2 + 32768;
#if defined(__LITTLE_ENDIAN__)
				p[i] = d2 & 0xffff;
#else
				dat.w = d2 & 0xffff;
				p[i] = dat.get_2bytes_le_to();
#endif
			}
		}
		break;
	case AUDIO_S16MSB:
		if(src1 != NULL) {
			int16_t *q = (int16_t*)src1;
			int16_t* p = (int16_t*)dst1;
			pair16_t dat;
			for(int i = 0; i < samples1; i++) {
				dat.sw = q[i];
#if defined(__LITTLE_ENDIAN__)
				p[i] = dat.get_2bytes_be_to();
#else
				p[i] = dat.w;
#endif
			}
		}
		if((src2 != NULL) && (dst2 != NULL)) {
			int16_t* q = (int16_t*)src2;
			int16_t* p = (int16_t*)dst2;
			pair16_t dat;
			for(int i = 0; i < samples2; i++) {
				dat.sw = q[i];
#if defined(__LITTLE_ENDIAN__)
				p[i] = dat.get_2bytes_be_to();
#else
				p[i] = dat.w;
#endif
			}
		}
		break;
	case AUDIO_U16MSB:
		if(src1 != NULL) {
			int16_t* q = (int16_t*)src1;
			uint16_t* p = (uint16_t*)dst1;
			pair16_t dat;
			int d2;
			for(int i = 0; i < samples1; i++) {
				d2 = q[i];
				d2 = d2 + 32768;
#if defined(__LITTLE_ENDIAN__)
				dat.w = d2 & 0xffff;
				p[i] = dat.get_2bytes_be_to();
#else
				p[i] = d2 & 0xffff;
#endif
			}
		}
		if((src2 != NULL) && (dst2 != NULL)) {
			int16_t* q = (int16_t*)src2;
			uint16_t* p = (uint16_t*)dst2;
			pair16_t dat;
			int d2;
			for(int i = 0; i < samples2; i++) {
				d2 = q[i];
				d2 = d2 + 32768;
#if defined(__LITTLE_ENDIAN__)
				dat.w = d2 & 0xffff;
				p[i] = dat.get_2bytes_be_to();
#else
				p[i] = d2 & 0xffff;
#endif
			}
		}
		break;
	case AUDIO_S32LSB:
		if(src1 != NULL) {
			uint32_t data;
			uint32_t* p = (uint32_t*)dst1;
			int32_t* q = (int32_t*)src1;
			for(int i = 0; i < samples1; i++) {
				data = q[i];
				data <<= 16;
				p[i] = EndianToLittle_DWORD(data);
			}
		}
		if((src2 != NULL) && (dst2 != NULL)) {
			uint32_t data;
			uint32_t* p = (uint32_t*)dst2;
			int32_t* q = (int32_t*)src2;
			for(int i = 0; i < samples2; i++) {
				data = q[i];
				data <<= 16;
				p[i] = EndianToLittle_DWORD(data);
			}
		}
		break;
	case AUDIO_S32MSB:
		if(src1 != NULL) {
			uint32_t data;
			uint32_t* p = (uint32_t*)dst1;
			int32_t* q = (int32_t*)src1;
			for(int i = 0; i < samples1; i++) {
				data = q[i];
				data <<= 16;
				p[i] = EndianToBig_DWORD(data);
			}
		}
		if((src2 != NULL) && (dst2 != NULL)) {
			uint32_t* p = (uint32_t*)dst2;
			int32_t* q = (int32_t*)src2;
			uint32_t data;
			for(int i = 0; i < samples2; i++) {
				data = q[i];
				data <<= 16;
				p[i] = EndianToBig_DWORD(data);
			}
		}
		break;
	case AUDIO_F32LSB:
		if(src1 != NULL) {
			uint32_t* p = (uint32_t*)dst1;
			int32_t* q = (int32_t*)src1;
			for(int i = 0; i < samples1; i++) {
				float_data.f = q[i];
				float_data.f /= 65536;
				p[i] = EndianToLittle_DWORD(float_data.d);
			}
		}
		if((src2 != NULL) && (dst2 != NULL)) {
			uint32_t* p = (uint32_t*)dst2;
			int32_t* q = (int32_t*)src2;
			for(int i = 0; i < samples2; i++) {
				float_data.f = q[i];
				float_data.f /= 65536;
				p[i] = EndianToLittle_DWORD(float_data.d);
			}
		}
		break;
	case AUDIO_F32MSB:
		if(src1 != NULL) {
			uint32_t* p = (uint32_t*)dst1;
			int32_t* q = (int32_t*)src1;
			for(int i = 0; i < samples1; i++) {
				float_data.f = q[i];
				float_data.f /= 65536;
				p[i] = EndianToBig_DWORD(float_data.d);
			}
		}
		if((src2 != NULL) && (dst2 != NULL)) {
			uint32_t* p = (uint32_t*)dst2;
			int32_t* q = (int32_t*)src2;
			for(int i = 0; i < samples2; i++) {
				float_data.f = q[i];
				float_data.f /= 65536;
				p[i] = EndianToBig_DWORD(float_data.d);
			}
		}
		break;
	}
}
#if 0
void OSD_BASE::update_sound(int* extra_frames)
{
	*extra_frames = 0;
	
	now_mute = false;
	if(sound_ok) {
		uint32_t play_c, size1, size2;
		//uint32_t offset;
		uint8_t *ptr1, *ptr2;
		
		// start play
		// check current position
		play_c = sound_write_pos;
		if(sound_debug) debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_SOUND,
											  "Called time=%d sound_write_pos=%d\n", osd_timer.elapsed(), play_c);
		if(!sound_first_half) {
			if((int)play_c < (sound_buffer_size / 2)) {
				return;
			}
			//offset = 0;
		} else {
			if((int)play_c >= (sound_buffer_size / 2)) {
				return;
			}
			//offset = sound_buffer_size / 2;
		}
		//SDL_UnlockAudio();
		// sound buffer must be updated
		Sint16* sound_buffer = (Sint16 *)this->create_sound(extra_frames);
		if(now_record_sound || now_record_video) {
			if(sound_samples > rec_sound_buffer_ptr) {
				int samples = sound_samples - rec_sound_buffer_ptr;
				int length = samples * sizeof(int16_t) * 2; // stereo
				rec_sound_bytes += length;
				if(now_record_video) {
					//AGAR_DebugLog(AGAR_LOG_DEBUG, "Push Sound %d bytes\n", length);
					emit sig_enqueue_audio((int16_t *)(&(sound_buffer[rec_sound_buffer_ptr * 2])), length);
				}
				// record sound
				if(now_record_sound) {
					rec_sound_fio->Fwrite(sound_buffer + rec_sound_buffer_ptr * 2, length, 1);
				}
				//if(now_record_video) {
				//	// sync video recording
				//	static double frames = 0;
				//	static int prev_samples = -1;
				//	static double prev_fps = -1;
				//	double fps = this->vm_frame_rate();
				//	frames = fps * (double)samples / (double)sound_rate;
				//}
				//printf("Wrote %d samples ptr=%d\n", samples, rec_sound_buffer_ptr);
				rec_sound_buffer_ptr += samples;
				if(rec_sound_buffer_ptr >= sound_samples) rec_sound_buffer_ptr = 0;
			}
		}
		if(sound_buffer) {
		        int ssize;
		        int pos;
		        int pos2;
				int format_len = 1;
		        if(sound_initialized) {
					switch(snddata.sound_format) {
					case AUDIO_S16LSB:
					case AUDIO_S16MSB:
					case AUDIO_U16LSB:
					case AUDIO_U16MSB:
						format_len = sizeof(int16_t);
						break;
					case AUDIO_S32LSB:
					case AUDIO_S32MSB:
						format_len = sizeof(int32_t);
						break;
					case AUDIO_F32LSB:
					case AUDIO_F32MSB:
						format_len = sizeof(float);
						break;
					}
					ssize = sound_samples * snd_spec_presented.channels;
			        pos = sound_data_pos;
			        pos2 = pos + ssize;
		        	ptr1 = (uint8_t*)(&sound_buf_ptr[pos * format_len]);
			        if(pos2 >= sound_buffer_size) {
						size1 = sound_buffer_size  - pos;
						size2 = pos2 - sound_buffer_size;
						ptr2 = &sound_buf_ptr[0];
					} else {
						size1 = ssize;
						size2 = 0;
						ptr2 = NULL;
					}
#if defined(USE_SDL2)   
					SDL_LockAudioDevice(audio_dev_id);
#else
					SDL_LockAudio();
#endif
#if 0
					if(ptr1) {
						my_memcpy(ptr1, sound_buffer, size1 * format_len);
					}
					if((ptr2) && (size2 > 0)) {
						my_memcpy(ptr2, &sound_buffer[size1], size2 * format_len);
					}
#else
					convert_sound_format(ptr1, ptr2, sound_buffer, &sound_buffer[size1], size1, size2);
#endif

					sound_data_len = sound_data_len + ssize;
					if(sound_data_len >= sound_buffer_size) sound_data_len = sound_buffer_size;
					sound_data_pos = sound_data_pos + ssize;
					if(sound_data_pos >= sound_buffer_size) sound_data_pos = sound_data_pos - sound_buffer_size;
					if(!sound_started) sound_started = true;
#if defined(USE_SDL2)   
					SDL_UnlockAudioDevice(audio_dev_id);
#else
					SDL_UnlockAudio();
#endif
					//SDL_UnlockAudio();
					SDL_PauseAudioDevice(audio_dev_id, 0);
			}
		}
	   
//	        SDL_PauseAudioDevice(audio_dev_id, 0);
		sound_first_half = !sound_first_half;
	}
}

void OSD_BASE::mute_sound()
{
	if(!now_mute && sound_ok) {
		// check current position
		uint32_t size1, size2;
	    
		uint8_t *ptr1, *ptr2;
		// WIP
		int ssize;
		int pos;
		int pos2;
#if defined(USE_SDL2)   
		SDL_LockAudioDevice(audio_dev_id);
#else
		SDL_LockAudio();
#endif
		int format_len = 1;
		switch(snddata.sound_format) {
		case AUDIO_S16LSB:
		case AUDIO_S16MSB:
		case AUDIO_U16LSB:
		case AUDIO_U16MSB:
			format_len = sizeof(int16_t);
			break;
		case AUDIO_S32LSB:
		case AUDIO_S32MSB:
			format_len = sizeof(int32_t);
			break;
		case AUDIO_F32LSB:
		case AUDIO_F32MSB:
			format_len = sizeof(float);
			break;
		}
		ssize = sound_buffer_size / 2;
		pos = sound_data_pos;
		pos2 = pos + ssize;
		ptr1 = &sound_buf_ptr[pos * format_len];
		if(pos2 >= sound_buffer_size) {
			size1 = sound_buffer_size - pos;
			size2 = pos2 - sound_buffer_size;
			ptr2 = &sound_buf_ptr[0];
		} else {
			size1 = ssize;
			size2 = 0;
			ptr2 = NULL;
		}
		
		if(ptr1) {
			memset(ptr1, 0x00, size1 * format_len);
		}
		if((ptr2) && (size2 > 0)){
			memset(ptr2, 0x00, size2 * format_len);
		}
		sound_data_pos = (sound_data_pos + ssize) % sound_buffer_size;
#if defined(USE_SDL2)   
		SDL_UnlockAudioDevice(audio_dev_id);
#else
		SDL_UnlockAudio();
#endif
	}
	now_mute = true;
}

void OSD_BASE::stop_sound()
{
	if(sound_ok && sound_started) {
		//sound_exit = true;
#if defined(USE_SDL2)   
		SDL_PauseAudioDevice(audio_dev_id, 1);
#else   
		SDL_PauseAudio(1);
#endif   
		sound_started = false;
		//sound_exit = false;
	}
}
void OSD_BASE::handleStateChanged(QAudio::State newState)
{
}
#else

void OSD_BASE::handleStateChanged(QAudio::State newState)
{
	switch(newState) {
	case QAudio::ActiveState:
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, _T("AUDIO:ACTIVE"));
		break;
	case QAudio::IdleState:
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, _T("AUDIO:IDLE"));
#if 0		
		if((m_audioOutput != nullptr) && (m_audioOutputBuffer != nullptr)) {
			if(!(m_audioOutputBuffer->empty())) {
				int _count = m_audioOutputBuffer->count();
				int remain = m_audioOutputSink->bytesFree() / sizeof(int16_t);
				if(_count > remain) _count = remain;
				if(_count <= 0) return;
				uint8_t *buf = new uint8_t[_count * sizeof(int16_t)];
				for(int i = 0; i < _count; i++) {
					int d =  m_audioOutputBuffer->read();
					buf[(i << 1) + 0] = d & 0xff;
					buf[(i << 1) + 1] = (d & 0xff00) >> 8;
				}
				m_audioOutput->write((const char *)buf, _count * sizeof(int16_t));
			}
		} else {
			//m_audioOutputSink->suspend();
		}
#endif
		break;
	case QAudio::StoppedState:
		m_audioOutput = nullptr;
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, _T("AUDIO:STOP"));
		break;
	case QAudio::SuspendedState:
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, _T("AUDIO:SUSPEND"));
		break;
	}
}
void OSD_BASE::update_sound(int* extra_frames)
{
	*extra_frames = 0;
	
	now_mute = false;
	if(sound_ok) {
		if(m_audioOutputBuffer == nullptr) return;
		if((m_audioOutputSink != nullptr) /*&& (written > 0)*/) {
			if(m_audioOutputBuffer->count() >= ((sound_samples) * 2)) {
				if((m_audioOutput != nullptr) && (m_audioOutputBuffer != nullptr)) {
					if(!(m_audioOutputBuffer->empty())) {
						int _count = m_audioOutputBuffer->count();
						int remain = m_audioOutputSink->bytesFree() / sizeof(int16_t);
						if(_count > remain) _count = remain;
						if(_count > 0) {
							uint8_t *buf = new uint8_t[_count * sizeof(int16_t)];
							for(int i = 0; i < _count; i++) {
								int d =  m_audioOutputBuffer->read();
								buf[(i << 1) + 0] = d & 0xff;
								buf[(i << 1) + 1] = (d & 0xff00) >> 8;
							}
							m_audioOutput->write((const char *)buf, _count * sizeof(int16_t));
						}
					}
				}
			}
			if(m_audioOutputSink->bytesFree() < ((sound_samples * 2 * sizeof(int16_t)))) {
				if(((m_audioOutputSink->state() == QAudio::ActiveState))) {
					return;
				}
			}
		}
		
//m_audioOutputSink->suspend();
				
//				if(((m_audioOutputSink->state() == QAudio::ActiveState))) {
//					return;
//				}
//			}
#if 0		
			if((m_audioOutput != nullptr) && (m_audioOutputBuffer != nullptr)) {
				if(!(m_audioOutputBuffer->empty())) {
					int _count = m_audioOutputBuffer->count();
					int remain = m_audioOutputSink->bytesFree() / sizeof(int16_t);
					if(_count > remain) _count = remain;
					if(_count > 0) {
						uint8_t *buf = new uint8_t[_count * sizeof(int16_t)];
						for(int i = 0; i < _count; i++) {
							int d =  m_audioOutputBuffer->read();
							buf[(i << 1) + 0] = d & 0xff;
							buf[(i << 1) + 1] = (d & 0xff00) >> 8;
						}
						m_audioOutput->write((const char *)buf, _count * sizeof(int16_t));
					}
				}
			} else {
				//m_audioOutputSink->suspend();
			}
#endif			
				//return;
		// Input
		int16_t* sound_buffer = (int16_t*)create_sound(extra_frames);
		if(now_record_sound || now_record_video) {
			if(sound_samples > rec_sound_buffer_ptr) {
				int samples = sound_samples - rec_sound_buffer_ptr;
				int length = samples * sizeof(int16_t) * 2; // stereo
				rec_sound_bytes += length;
				if(now_record_video) {
					//AGAR_DebugLog(AGAR_LOG_DEBUG, "Push Sound %d bytes\n", length);
					emit sig_enqueue_audio((int16_t *)(&(sound_buffer[rec_sound_buffer_ptr * 2])), length);
				}
				// record sound
				if(now_record_sound) {
					rec_sound_fio->Fwrite(sound_buffer + rec_sound_buffer_ptr * 2, length, 1);
				}
				//if(now_record_video) {
				//	// sync video recording
				//	static double frames = 0;
				//	static int prev_samples = -1;
				//	static double prev_fps = -1;
				//	double fps = this->vm_frame_rate();
				//	frames = fps * (double)samples / (double)sound_rate;
				//}
				//printf("Wrote %d samples ptr=%d\n", samples, rec_sound_buffer_ptr);
				rec_sound_buffer_ptr += samples;
				if(rec_sound_buffer_ptr >= sound_samples) rec_sound_buffer_ptr = 0;
			}
		}
		//if(sound_initialized) return;
		if(sound_buffer != nullptr) {
			if(m_audioOutput != nullptr) {
				int wordsize = sizeof(int32_t);
				switch(m_audioOutputFormat.sampleFormat()) {
				case QAudioFormat::UInt8:
					wordsize = sizeof(uint8_t);
					break;
				case QAudioFormat::Int16:
					wordsize = sizeof(int16_t);
					break;
				case QAudioFormat::Int32:
					wordsize = sizeof(int32_t);
					break;
				case QAudioFormat::Float:
					wordsize = sizeof(float);
					break;
				}
				// ToDo: Not Int16.
				//qint64 sound_len = sound_samples * sound_rate * 2 * wordsize;
				qint64 sound_len = sound_samples * 2;
				qint64 written = 0;
#if 1
				if(m_audioOutputBuffer != nullptr) {
					int ptr = 0;
					while(!(m_audioOutputBuffer->full()) && (sound_len > 0)) {
						m_audioOutputBuffer->write(sound_buffer[ptr]);
						ptr++;
						sound_len--;
					}
					written = m_audioOutputBuffer->count();
				}
			
#endif
#if 0
			int _count = sound_len * sizeof(int16_t);
			int remain = m_audioOutputSink->bytesFree();
			if(_count > remain) _count = remain;
			//debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, "COUNT=%d", _count);
			if(_count > 0) {
				m_audioOutput->write((const char *)sound_buffer, _count);
			}
#endif
			}
		}
	}
}

void OSD_BASE::mute_sound()
{
	if(!now_mute && sound_ok) {
		if(m_audioOutputSink != nullptr) {
			switch(m_audioOutputSink->state()) {
			case QAudio::ActiveState:
			case QAudio::IdleState:
				m_audioOutputSink->suspend();
				break;
			}
		}
		if(m_audioOutput != nullptr) {
			int wordsize = sizeof(int32_t);
			switch(m_audioOutputFormat.sampleFormat()) {
			case QAudioFormat::UInt8:
				wordsize = sizeof(uint8_t);
				break;
			case QAudioFormat::Int16:
				wordsize = sizeof(int16_t);
				break;
			case QAudioFormat::Int32:
				wordsize = sizeof(int32_t);
				break;
			case QAudioFormat::Float:
				wordsize = sizeof(float);
				break;
			}
			// ToDo: Not Int16.
			//qint64 sound_len = sound_samples * sound_rate * 2 * wordsize;
			qint64 sound_len = sound_samples * 2 * sizeof(int16_t);
			qint64 written = 0;
			bool stat = false;
			//stat = m_audioOutput->open(QIODeviceBase::WriteOnly);
			stat = m_audioOutput->isOpen();
			if(stat) {
				m_audioOutput->reset();
				while(!(m_audioOutputBuffer->full()) && (sound_len > 0)) {
					m_audioOutputBuffer->write(0x00);
					sound_len--;
				}
				//m_audioOutput->close();
			}
		}
	}
	now_mute = true;
}
void OSD_BASE::stop_sound()
{
	if(sound_ok) {
		if(m_audioOutputSink != nullptr) {
			switch(m_audioOutputSink->state()) {
			case QAudio::ActiveState:
			case QAudio::IdleState:
			case QAudio::SuspendedState:
				m_audioOutputSink->stop();
				break;
			}
		}
	}
}

#endif
void OSD_BASE::start_record_sound()
{
   
	if(!now_record_sound) {
		//LockVM();
		QDateTime nowTime = QDateTime::currentDateTime();
		QString tmps = QString::fromUtf8("Sound_Save_emu");
		tmps = tmps + get_vm_config_name();
		tmps = tmps + QString::fromUtf8("_");
		tmps = tmps + nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz"));
		tmps = tmps + QString::fromUtf8(".wav");
		strncpy((char *)sound_file_name, tmps.toLocal8Bit().constData(), sizeof(sound_file_name) - 1);
		// create wave file
		rec_sound_fio = new FILEIO();
		if(rec_sound_fio->Fopen(bios_path(sound_file_name), FILEIO_WRITE_BINARY)) {
			// write dummy wave header
			write_dummy_wav_header((void *)rec_sound_fio);
			
			rec_sound_bytes = 0;
			rec_sound_buffer_ptr = 0;
			now_record_sound = true;
		} else {
			// failed to open the wave file
			delete rec_sound_fio;
		}
		//UnlockVM();
	}
}

void OSD_BASE::stop_record_sound()
{
		if(now_record_sound) {
			//LockVM();
		if(rec_sound_bytes == 0) {
			rec_sound_fio->Fclose();
			rec_sound_fio->RemoveFile(sound_file_name);
		} else {
			// update wave header
			wav_header_t wav_header;
			wav_chunk_t wav_chunk;
	#if 0		
			if(!set_wav_header(&wav_header, &wav_chunk, 2, snd_spec_presented.freq, 16,
							 (size_t)(rec_sound_bytes + sizeof(wav_header) + sizeof(wav_chunk)))) {
	#else
			if(!set_wav_header(&wav_header, &wav_chunk, 2, (uint32_t)(m_audioOutputFormat.sampleRate()), 16,
							 (size_t)(rec_sound_bytes + sizeof(wav_header) + sizeof(wav_chunk)))) {
	#endif
				delete rec_sound_fio;
				now_record_sound = false;
				return;
			}
			rec_sound_fio->Fseek(0, FILEIO_SEEK_SET);
			rec_sound_fio->Fwrite(&wav_header, sizeof(wav_header_t), 1);
			rec_sound_fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
			rec_sound_fio->Fclose();
		}
		delete rec_sound_fio;
		now_record_sound = false;
		//UnlockVM();
	}
}

void OSD_BASE::restart_record_sound()
{
	bool tmp = now_record_sound;
	stop_record_sound();
	if(tmp) {
		start_record_sound();
	}
}

#if 0
int OSD_BASE::get_sound_rate()
{
	return snd_spec_presented.freq;
}
#else
int OSD_BASE::get_sound_rate()
{
	return (int)(m_audioOutputFormat.sampleRate());
}
#endif

void OSD_BASE::close_capture_sound_emu(int ch)
{
	if(ch < 0) return;
	if(ch >= MAX_CAPTURE_SOUNDS) return;
	if(sound_capture_desc[ch].out_buffer != NULL) {
		free(sound_capture_desc[ch].out_buffer);
	}
	sound_capture_desc[ch].out_buffer = NULL;
	sound_capturing_emu[ch] = false;
}

void *OSD_BASE::get_capture_sound_buffer(int ch)
{
	if(ch < 0) return NULL;
	if(ch >= MAX_CAPTURE_SOUNDS) return NULL;
	return sound_capture_desc[ch].out_buffer;
}

bool OSD_BASE::is_capture_sound_buffer(int ch)
{
	if(ch < 0) return false;
	if(ch >= MAX_CAPTURE_SOUNDS) return false;
	if(sound_capture_desc[ch].out_buffer == NULL) return false;
	return sound_capturing_emu[ch];
}
void *OSD_BASE::open_capture_sound_emu(int ch, int rate, int channels, int sample_type, int samples, int physical_device_num)
{
	if(ch < 0) return NULL;
	if(ch >= MAX_CAPTURE_SOUNDS) return NULL;
	
	close_capture_sound_emu(ch);
	sound_capture_desc[ch].rate = rate;
	sound_capture_desc[ch].channels = channels;
	sound_capture_desc[ch].samples = samples;
	sound_capture_desc[ch].sample_type = sample_type;
	sound_capture_desc[ch].physical_dev = physical_device_num;
	bool stat = false;
	if((physical_device_num >= 0) && (physical_device_num < sound_capture_device_list.count()) && (physical_device_num < MAX_SOUND_CAPTURE_DEVICES)) {
		if(!(capturing_sound[physical_device_num])) {
			stat = open_sound_capture_device(physical_device_num, (rate < 44100) ? 44100 : rate, (channels > 2) ? channels : 2);
		}
	}
	
	void *p = NULL;
	if(stat) {
		switch(sample_type) {
		case SAMPLE_TYPE_UINT8:
		case SAMPLE_TYPE_SINT8:
			p = malloc(sizeof(uint8_t) * channels * (samples + 100));
			break;
		case SAMPLE_TYPE_UINT16_BE:
		case SAMPLE_TYPE_SINT16_BE:
		case SAMPLE_TYPE_UINT16_LE:
		case SAMPLE_TYPE_SINT16_LE:
			p = malloc(sizeof(uint16_t) * channels * (samples + 100));
			break;
		case SAMPLE_TYPE_UINT32_BE:
		case SAMPLE_TYPE_SINT32_BE:
		case SAMPLE_TYPE_UINT32_LE:
		case SAMPLE_TYPE_SINT32_LE:
		p = malloc(sizeof(uint32_t) * channels * (samples + 100));
		break;
		case SAMPLE_TYPE_FLOAT_BE:
		case SAMPLE_TYPE_FLOAT_LE:
			p = malloc(sizeof(float) * channels * (samples + 100));
			break;
		}
	}
	sound_capture_desc[ch].out_buffer = (uint8_t *)p;
	sound_capturing_emu[ch] = true;
	return p;
}
	
bool OSD_BASE::open_sound_capture_device(int num, int req_rate, int req_channels)
{
	if(num < 0) return false;
	if(num >= MAX_SOUND_CAPTURE_DEVICES) return false;
	if(sound_capture_device_list.count() <= num) return false;
	SDL_AudioSpec req;
	SDL_AudioSpec desired;
	req.freq = req_rate;
	req.channels = req_channels;
	req.silence = 0;
	req.format = AUDIO_S16SYS;
	req.samples = (sizeof(sound_capture_buffer[num]) / sizeof(int16_t)) / req_channels;
	req.callback = &(this->audio_capture_callback);
	req.userdata = (void *)(&(sound_capture_dev_desc[num].userdata));
	
	if(!(capturing_sound[num])) {
		sound_capture_desc[num].physical_dev = SDL_OpenAudioDevice((const char *)sound_capture_device_list.value(num).toUtf8().constData(), 1, &req, &desired, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
		if(sound_capture_desc[num].physical_dev <= 0) {
			debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,"Failed to initialize sound capture device \"%s\"\n", (const char *)sound_capture_device_list.value(num).toUtf8().constData());
			sound_capture_desc[num].physical_dev = -1;
			return false;
		}
		// Device OK
		capturing_sound[num] = true;
		sound_capture_dev_desc[num].format = desired.format;
		sound_capture_dev_desc[num].sample_rate = desired.freq;
		sound_capture_dev_desc[num].channels = desired.channels;
		sound_capture_dev_desc[num].buffer_samples = desired.samples;
		sound_capture_dev_desc[num].size = desired.size;
		sound_capture_dev_desc[num].callback = desired.callback;
		sound_capture_dev_desc[num].silence = desired.silence;
		int buflen = desired.samples * desired.channels;
		switch(desired.format) {
		case AUDIO_S8:
		case AUDIO_U8:
			buflen = buflen * sizeof(int8_t);
			break;
		case AUDIO_S16LSB:
		case AUDIO_S16MSB:
		case AUDIO_U16LSB:
		case AUDIO_U16MSB:
			buflen = buflen * sizeof(int16_t);
			break;
		case AUDIO_S32LSB:
		case AUDIO_S32MSB:
			buflen = buflen * sizeof(int32_t);
			break;
		case AUDIO_F32LSB:
		case AUDIO_F32MSB:
			buflen = buflen * sizeof(float);
			break;
		default:
			break;
		}
		
		sound_capture_dev_desc[num].userdata.buffer_size = buflen;
		sound_capture_dev_desc[num].userdata.format = desired.format;
		sound_capture_dev_desc[num].userdata.readlen = 0;
		sound_capture_dev_desc[num].userdata.writelen = 0;
		sound_capture_dev_desc[num].userdata.readpos = 0;
		sound_capture_dev_desc[num].userdata.writepos = 0;
		sound_capture_dev_desc[num].userdata.read_buffer_ptr = &(sound_capture_buffer[num][0]);
		memset(&(sound_capture_buffer[num][0]), 0x00, buflen);
		
		for(int ch = 0; ch < MAX_SOUND_CAPTURE_DEVICES; ch++) {
			if(sound_capture_desc[ch].physical_dev == num) {
				sound_capture_desc[ch].read_format = desired.format;
				sound_capture_desc[ch].read_rate = desired.freq;
				sound_capture_desc[ch].read_silence = desired.silence;
				sound_capture_desc[ch].read_size = desired.size;
				sound_capture_desc[ch].read_channels = desired.channels;
				sound_capture_desc[ch].read_samples = desired.samples;
				sound_capture_desc[ch].read_callback = desired.callback;
				sound_capture_desc[ch].read_userdata = desired.userdata;
				
				sound_capture_desc[ch].read_pos = 0;
				sound_capture_desc[ch].read_data_len = 0;
				sound_capture_desc[ch].read_buffer_len = buflen;
				sound_capture_desc[ch].read_buffer_ptr = (uint8_t *)(&(sound_capture_buffer[num][0]));

				
			}				
		}
	}
	return true;
}

bool OSD_BASE::close_sound_capture_device(int num, bool force)
{
	// ToDo: Check capturing entries
	if((capturing_sound[num]) && (sound_capture_desc[num].physical_dev > 0)) {
		SDL_CloseAudioDevice(sound_capture_desc[num].physical_dev);
	}
	return true;
}
