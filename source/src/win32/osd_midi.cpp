/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2022.07.03-

	[ win32 midi ]
*/

#include "osd.h"
#include "../fifo.h"

CRITICAL_SECTION send_cs;
CRITICAL_SECTION recv_cs;

unsigned __stdcall midi_thread(void *lpx)
{
	volatile midi_thread_params_t *p = (midi_thread_params_t *)lpx;
	int midi_out_initialized = -1;
	HMIDIOUT hMidi = NULL;
	
	while(!p->terminate) {
		while(true) {
			uint8_t buffer[128];
			int length = 0;
			
			EnterCriticalSection(&send_cs);
			if(!p->send_buffer->empty()) {
				uint8_t msg = p->send_buffer->read_not_remove(0);
				
				switch(msg & 0xf0) {
				case 0x80: case 0x90: case 0xa0: case 0xb0: case 0xe0:
					length = 3;
					break;
				case 0xc0:
				case 0xd0:
					length = 2;
					break;
				case 0xf0:
					switch(msg) {
					case 0xf0:
						// system exclusive
						for(int i = 1; i < p->send_buffer->count(); i++) {
							if(p->send_buffer->read_not_remove(i) == 0xf7) {
								length = i + 1;
								break;
							}
						}
						break;
					case 0xf1: case 0xf3:
						length = 2;
						break;
					case 0xf2:
						length = 3;
						break;
					case 0xf6: case 0xf8: case 0xfa: case 0xfb: case 0xfc: case 0xfe: case 0xff:
						length = 1;
						break;
					default:
						// undefined msg
						p->send_buffer->read();
						break;
					}
					break;
				default:
					// invalid msg
					p->send_buffer->read();
					break;
				}
				if(p->send_buffer->count() >= length) {
					for(int i = 0; i < length; i++) {
						buffer[i] = p->send_buffer->read();
					}
				} else {
					length = 0;
				}
			}
			LeaveCriticalSection(&send_cs);
			
			if(length > 0) {
				if(midi_out_initialized == -1) {
					if(midiOutOpen(&hMidi, MIDI_MAPPER, NULL, NULL, CALLBACK_NULL) == MMSYSERR_NOERROR) {
						midi_out_initialized = 1;
					} else {
						midi_out_initialized = 0;
					}
				}
				if(midi_out_initialized == 1) {
					if(buffer[0] == 0xf0) {
						// system exclusive
						MIDIHDR mhMidi;
						
						ZeroMemory(&mhMidi, sizeof(mhMidi));
						mhMidi.lpData = (LPSTR)buffer;
						mhMidi.dwBufferLength = length;
						mhMidi.dwBytesRecorded = length;
						midiOutPrepareHeader(hMidi, &mhMidi, sizeof(mhMidi));
						midiOutLongMsg(hMidi, &mhMidi, sizeof(mhMidi));
						while(!(mhMidi.dwFlags & MHDR_DONE)) {
							Sleep(10);
						}
						midiOutUnprepareHeader(hMidi, &mhMidi, sizeof(mhMidi));
					} else {
						union UNION_MIDI_DATA {
							DWORD msg;
							BYTE data[4];
						};
						UNION_MIDI_DATA out;
						
						for(int i = 0; i < 4; i++) {
							out.data[i] = (i < length) ? buffer[i] : 0;
						}
						midiOutShortMsg(hMidi,out.msg);
					}
				}
			} else {
				break;
			}
		}
		Sleep(0);
	}
	if(midi_out_initialized == 1) {
		midiOutClose(hMidi);
	}
	_endthreadex(0);
	return 0;
}

void OSD::initialize_midi()
{
	midi_thread_params.send_buffer = new FIFO(1024);
	midi_thread_params.recv_buffer = new FIFO(1024);
	midi_thread_params.terminate = false;
	
	InitializeCriticalSection(&send_cs);
	InitializeCriticalSection(&recv_cs);
	hMidiThread = (HANDLE)_beginthreadex(NULL, 0, midi_thread, &midi_thread_params, 0, NULL);
}

void OSD::release_midi()
{
	if(hMidiThread) {
		midi_thread_params.terminate = true;
		WaitForSingleObject(hMidiThread, INFINITE);
		hMidiThread = NULL;
	}
	DeleteCriticalSection(&send_cs);
	DeleteCriticalSection(&recv_cs);
	
	midi_thread_params.send_buffer->release();
	delete midi_thread_params.send_buffer;
	midi_thread_params.send_buffer = NULL;
	midi_thread_params.recv_buffer->release();
	delete midi_thread_params.recv_buffer;
	midi_thread_params.recv_buffer = NULL;
}

void OSD::send_to_midi(uint8_t data)
{
	EnterCriticalSection(&send_cs);
	midi_thread_params.send_buffer->write(data);
	LeaveCriticalSection(&send_cs);
}

bool OSD::recv_from_midi(uint8_t *data)
{
	bool result = false;
	EnterCriticalSection(&recv_cs);
	if(!midi_thread_params.recv_buffer->empty()) {
		*data = (uint8_t)midi_thread_params.recv_buffer->read();
		result = true;
	}
	LeaveCriticalSection(&recv_cs);
	return result;
}
