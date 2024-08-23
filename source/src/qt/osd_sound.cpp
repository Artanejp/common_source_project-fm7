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
#include "gui/menu_flags.h"

#include <QDateTime>
#include <QByteArray>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QApplication>
#endif

#include <cstdint>
#include <cmath>
#include "./sound-drivers/sound_buffer_qt.h"
#include "./sound-drivers/osd_sound_mod_template.h"

#include "emu_thread_tmpl.h"


/* Note: Below are new sound driver. */
#include "./sound-drivers/qt_multimedia/osd_sound_mod_qtmultimedia.h"

void OSD_BASE::do_sink_empty()
{
	if((m_now_mute.load()) && (m_sound_initialized)) {
		// Continue to mute.
		//if(sound_tick_timer.isValid()) {
			//m_elapsed_us_before_rendered = 0;
			//sound_tick_timer.restart();
		//}
		put_null_sound();
	}
	m_sink_empty = true;
}

void OSD_BASE::do_sink_started()
{
	m_sink_started = true;
	m_sound_first_half = true;
}

void OSD_BASE::do_sink_stopped()
{
	m_sink_started = false;
}

void OSD_BASE::sound_debug_log(const char *fmt, ...)
{
	if(m_sound_debug.load()) {
		char strbuf[4096];
		va_list ap;
		
		va_start(ap, fmt);
		vsnprintf(strbuf, 4095, fmt, ap);
		va_end(ap);
		QString tmps = QString::fromLocal8Bit(strbuf);
		emit sig_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_SOUND, tmps);
	}
}

void OSD_BASE::update_sound(int* extra_frames)
{
	__LIKELY_IF(extra_frames != nullptr) {
		*extra_frames = 0;
	}
	if(m_sound_exit.load()) {
		return;
	}

	std::shared_ptr<SOUND_MODULE::M_BASE>sound_drv = m_sound_driver;
	if(m_sound_initialized.load()) {
		// Get sound driver
		bool prev_mute = m_now_mute.load();
		unmute_sound();
		if(!(sound_tick_timer.isValid())) {
			sound_tick_timer.start();
		}
		bool first_half = m_sound_first_half.load();

		// Check enough to render accumlated
		// source (= by VM) rendering data.
		__UNLIKELY_IF((m_sound_rate <= 0) || (m_sound_samples <= 0)) {
			return; 
		}
		int64_t elapsed_us_before_rendered = m_elapsed_us_before_rendered.load();
		const int64_t period_usecs = (((int64_t)m_sound_samples) * 1000 * 1000) / ((int64_t)m_sound_rate);
		
		__UNLIKELY_IF(elapsed_us_before_rendered < 0) {
			elapsed_us_before_rendered = 0;
			m_elapsed_us_before_rendered = 0;
		} else if(elapsed_us_before_rendered >= period_usecs) {
			elapsed_us_before_rendered = period_usecs - 1 // Margin?
			m_elapsed_us_before_rendered = elapsed_us_before_rendered;
		}

		const int64_t elapsed_usec = ((int64_t)sound_tick_timer.nsecsElapsed() / 1000) + elapsed_us_before_rendered; 
		int64_t margin_usecs = m_sound_margin_usecs.load();
		int64_t tmp_frame_us = llrint(1.0e6 / vm_frame_rate());
		
		__UNLIKELY_IF(margin_usecs < (tmp_frame_us / 4)) {
			margin_usecs = (tmp_frame_us / 4); // OK?
		}
		__UNLIKELY_IF(margin_usecs >= (period_usecs / 2)) {
			margin_usecs = (period_usecs / 2); // OK?
		}
//		__LIKELY_IF(!((m_sink_started.load()) && (prev_mute)) || (sound_drv.get() == nullptr)) {
			__LIKELY_IF(elapsed_usec < (period_usecs - margin_usecs)) {
				return;
			}
//		}
		// Restart Timer
		m_elapsed_us_before_rendered = 0;
		sound_tick_timer.restart();
		int __extra_frames = 0;
		int16_t* sound_buffer = (int16_t*)create_sound(&__extra_frames);
		__LIKELY_IF(extra_frames != NULL) {
			*extra_frames = __extra_frames;
		}
		// Go to output sound.
		margin_usecs = tmp_frame_us / 4;
//		margin_usecs = period_usecs / 16;
//		if(__extra_frames > 1) {
//			margin_usecs += ((int64_t)(__extra_frames - 1)) * tmp_frame_us;
//		}
		m_sound_margin_usecs = margin_usecs;

		if(sound_buffer == nullptr) {
			return;
		}
		if(now_record_sound || now_record_video) {
			if(m_sound_samples > rec_sound_buffer_ptr) {
				int samples = m_sound_samples - rec_sound_buffer_ptr;
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
				if(rec_sound_buffer_ptr >= m_sound_samples) rec_sound_buffer_ptr = 0;
			}
		}
		// ToDo: Convert sound format.
		if(sound_drv.get() != nullptr) {
			int64_t _result = 0;
			int _samples = m_sound_samples;
			_result = sound_drv->update_sound((void*)sound_buffer, _samples);
			if(_result > 0) {
				m_sink_empty = false;
			}
//			debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_SOUND,
//					  _T("Push sound %d samples -> %ld\n"), m_sound_samples, _result);
			//if(_result > 0) {
				//printf("%d %ld\n", m_sound_period, _result);
			//}
		}
		m_sound_first_half = !(first_half);
	}
}

bool OSD_BASE::calcurate_sample_factor(int rate, int samples, const bool force)
{
	__UNLIKELY_IF(vm == nullptr) {
		return false;
	}
	double fps = vm_frame_rate();
	if((rate <= 0) || (samples <= 0) || (fps <= 0.0)) {
		return false;
	}
	if((samples != m_sound_samples) || (rate != m_sound_rate) || (m_fps != fps) || (force) || (m_sound_samples_factor.load() == 0)) {
		if(samples > 0) {
			m_sound_samples = samples;
		}
		if(rate > 0) {
			m_sound_rate = rate;
		}
		if(fps > 0.0) {
			m_fps = fps;
		}
		if((m_fps > 0.0) && (m_sound_rate > 0) && (m_sound_samples > 0)) {
			m_sound_samples_factor = llrint(1.0e6 / m_fps) / 2;
			sound_debug_log(_T("set samples factor = %lld"), m_sound_samples_factor.load());
			return true;
		}
		return false;
	}
	return true;
}


void OSD_BASE::initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples)
{
	// If sound driver hasn't initialized, initialize.
	m_sound_exit = false;
	m_elapsed_us_before_rendered = 0; // OK?
	sound_tick_timer.invalidate(); // OK?
	
	if((m_sound_driver.get() == nullptr)  ||
	   (m_sound_rate != rate) ||
	   (m_sound_samples != samples)) {
		//m_sound_ok = false;
		if(m_sound_driver.get() != nullptr) {
			m_sound_driver->deleteLater();
			while(m_sound_driver.use_count() > 0) {
				QThread::msleep(10);
			}
		}
		//m_sound_driver.reset(); // 20240511 K.O
		/*
		if(m_sound_thread != nullptr) {
			if(m_sound_thread->isRunning()) {
				m_sound_thread->quit();
				m_sound_thread->wait();
			}
			delete m_sound_thread;
			m_sound_thread = nullptr;
		}
		*/
		m_sound_initialized = false;
		m_sink_started = false;
		m_sink_empty = false; // OK?
		m_source_started = false;
		m_source_empty = false; // OK?
		m_sound_first_half = true;
		m_sound_margin_usecs = 0;
		
		m_sound_driver.reset(
			new SOUND_MODULE::M_QT_MULTIMEDIA(this ,
											  nullptr,
											  nullptr,
											  rate,
											  (samples * 1000) / rate,
											  2,
											  nullptr,
											  0));
		if(m_sound_driver.get() != nullptr) {
			m_sound_driver->initialize_driver(this);
			connect(this, SIGNAL(sig_set_sound_output_device(QString)), m_sound_driver.get(), SLOT(do_set_output_by_name(QString)), Qt::QueuedConnection);
			m_sound_margin_usecs = llrint(1.0e6 / vm_frame_rate()); // Margine is 1 frame.
		
		}
		#if 1
		if(m_sound_thread == nullptr) {
			m_sound_thread = new QThread();
		}
		if((m_sound_thread != nullptr) && (m_sound_driver.get() != nullptr)) {
			m_sound_thread->setObjectName(QString::fromUtf8("SoundThread"));
			m_sound_driver->moveToThread(m_sound_thread);
			//connect(m_sound_driver.get(), SIGNAL(destroyed()), m_sound_thread, SLOT(quit()));
			connect(m_sound_thread, SIGNAL(finished()), m_sound_driver.get(), SLOT(deleteLater()));
			//connect(m_sound_thread, SIGNAL(destroyed()), this, SLOT(do_reset_sound_thread()));
			if(!(m_sound_thread->isRunning())) {
				m_sound_thread->start(QThread::LowPriority);
			}
		}
		#else
		connect(this->thread(), SIGNAL(finished()), m_sound_driver.get(), SLOT(deleteLater()));
		m_sound_driver->moveToThread(this->thread());
		#endif		
	}
	std::shared_ptr<SOUND_MODULE::M_BASE>sound_drv = m_sound_driver;
	
	init_sound_device_list();
	if(p_config != nullptr) {
		QString outdev = QString::fromUtf8("Default");
		std::string tmpout = QString::fromLocal8Bit(p_config->sound_device_name, (sizeof(p_config->sound_device_name) / sizeof(_TCHAR)) - 1).toStdString();
		if(!(tmpout.empty()) && (tmpout.compare("Default") != 0)) {
			if(sound_drv.get() != nullptr) {
				std::list<std::string> _l = sound_drv->get_sound_sink_devices_list();
				for(auto s = _l.begin(); s != _l.end(); ++s) {
					if((*s) == tmpout) {
						outdev = QString::fromStdString(tmpout);
						break;
					}
				}
			}
		}
		emit sig_set_sound_output_device(outdev);
	}

	sound_debug_log(_T("OSD::%s rate=%d samples=%d m_sound_driver=%llx"), __func__, rate, samples, (uintptr_t)(sound_drv.get()));
	
	m_sound_rate = rate;
	m_sound_samples = samples;
	m_sound_initialized = true;
	m_elapsed_us_before_rendered = 0;
	
	if(sound_drv.get() != nullptr) {
		if(presented_samples != nullptr) {
			*presented_samples = samples;
		}
		if(presented_rate != nullptr) {
			*presented_rate = rate;
		}
		m_now_mute = false;
		sound_dvr->start_sink();
		mute_sound(); // Fill blank data a sample period.
	}
}

void OSD_BASE::release_sound()
{
	// ToDo: Sound Input
	// release Qt Multimedia sound
	m_sound_exit = true;
	m_sound_initialized = false;
	m_sink_started = false;
	m_sink_empty = false; // OK?
	m_source_started = false;
	m_source_empty = false; // OK?

	m_sound_period = 0;

	if(m_sound_thread != nullptr) {
		if(m_sound_thread->isRunning()) {
			m_sound_thread->quit();
			m_sound_thread->wait();
		}
		delete m_sound_thread;
		m_sound_thread = nullptr;
	}
	
	while(m_sound_driver.use_count() > 0) {
		QThread::msleep(10);
	}
	m_sound_driver.reset();
}


void OSD_BASE::do_update_master_volume(int level)
{
	if(p_config != nullptr) {
		p_config->general_sound_level = level;;
	}
	std::shared_ptr<SOUND_MODULE::M_BASE>sound_drv = m_sound_driver;
	if(sound_drv.get() != nullptr) {
		sound_drv->set_sink_volume((int)(p_config->general_sound_level));
	}
}

void OSD_BASE::do_set_host_sound_output_device(QString device_name)
{
	if(device_name.isEmpty()) return;
	emit sig_set_sound_output_device(device_name);
}

const _TCHAR *OSD_BASE::get_sound_device_name(int num)
{
	std::shared_ptr<SOUND_MODULE::M_BASE>sound_drv = m_sound_driver;
	if(sound_drv.get() != nullptr) {
		return sound_drv->get_sound_device_name(num);
	}
	return (const _TCHAR *)nullptr;
}

void OSD_BASE::init_sound_device_list()
{
	std::shared_ptr<SOUND_MODULE::OUTPUT::M_BASE>sound_drv = m_sound_driver;
	
	sound_output_device_list.clear();
	sound_capture_device_list.clear();
	
	if(sound_drv.get() != nullptr) {
		std::list<std::string> _l = sound_drv->get_sound_sink_devices_list();
		int _xi = 1;
		for(auto s = _l.begin(); s != _l.end(); ++s) {
			sound_output_device_list.append(QString::fromStdString(*s));
			sound_debug_log("SOUND OUTPUT DEVICE#%03d %s", _xi, (*s).c_str());
			_xi++;
		}
	}
	// ToDo: Capturing (SOURCE)
}

void OSD_BASE::put_null_sound()
{
	std::shared_ptr<SOUND_MODULE::OUTPUT::M_BASE>sound_drv = m_sound_driver;
	if(sound_drv.get() != nullptr) {
		//sound_drv->mute_sink();
		size_t chunk_bytes = sound_drv->get_sink_chunk_bytes();
		size_t samples = sound_drv->get_sink_sample_count();
		uint8_t* buf = new uint8_t[chunk_bytes];
		if(buf != nullptr) {
			memset(buf, 0x00, chunk_bytes);
			sound_drv->update_sound((void*)buf, _samples);
			delete [] buf;
			m_sink_empty = false;
		}
	}
}

void OSD_BASE::unmute_sound()
{
	if((m_now_mute.load()) && (m_sound_initialized.load())) {
		std::shared_ptr<SOUND_MODULE::OUTPUT::M_BASE>sound_drv = m_sound_driver;
		if(sound_drv.get() != nullptr) {
			//sound_drv->unmute_sink();
			sound_drv->discard_sink(); // Resume.
		}
	}
	m_now_mute = false;
}

void OSD_BASE::mute_sound()
{
	if(!(m_now_mute.load()) && (m_sound_initialized.load())) {
		if(sound_tick_timer.isValid()) {
			m_elapsed_us_before_rendered = (int64_t)sound_tick_timer.nsecsElapsed() / 1000;
			sound_tick_timer.invalidate();
		}
		put_null_sound();
	}
	m_now_mute = true;
	
}

void OSD_BASE::stop_sound()
{
	//m_sound_ok = false;
	std::shared_ptr<SOUND_MODULE::OUTPUT::M_BASE>sound_drv = m_sound_driver;
	if(sound_drv.get() != nullptr) {
		sound_drv->stop_sink();
	}
	m_elapsed_us_before_rendered = 0;
	m_sound_period = 0;
	sound_tick_timer.invalidate(); // Don't use timer.
	m_sound_initialized = false;
}

//void OSD_BASE::do_acknowledge_sound_started()
//{
	//m_sound_ok = true;
//	elapsed_us_before_rendered = sound_drv->driver_elapsed_usec();
//}

int OSD_BASE::get_sound_rate()
{
	#if 1
	__LIKELY_IF(m_sound_rate > 0) {
		return m_sound_rate;
	}
	return 0;
	#else
	std::shared_ptr<SOUND_MODULE::M_BASE>sound_drv = m_sound_driver;
	if(sound_drv.get() != nullptr) {
		return sound_drv->get_sample_rate();
	}
	return 0;
	#endif
}
/* End Note: */


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
	#if 0 /* Temporally Disable  Caoptuing Sound 20220921 K.O */
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
	#endif /* Temporally Disable  Caoptuing Sound 20220921 K.O */

}

void OSD_BASE::restart_record_sound()
{
	bool tmp = now_record_sound;
	stop_record_sound();
	if(tmp) {
		start_record_sound();
	}
}


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
	if(ch < 0) return nullptr;
	if(ch >= MAX_CAPTURE_SOUNDS) return nullptr;

	void *p = nullptr;
#if 0
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
#endif
	return p;
}

bool OSD_BASE::open_sound_capture_device(int num, int req_rate, int req_channels)
{
	if(num < 0) return false;
	if(num >= MAX_SOUND_CAPTURE_DEVICES) return false;
	if(sound_capture_device_list.count() <= num) return false;
#if 0
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
#endif
	return true;
}

bool OSD_BASE::close_sound_capture_device(int num, bool force)
{
	// ToDo: Check capturing entries
#if 0
	if((capturing_sound[num]) && (sound_capture_desc[num].physical_dev > 0)) {
		SDL_CloseAudioDevice(sound_capture_desc[num].physical_dev);
	}
#endif
	return true;
}
