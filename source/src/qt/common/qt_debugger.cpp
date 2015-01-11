#include "emu.h"
#include "vm/device.h"
#include "vm/debugger.h"
#include "vm/vm.h"
#include "fileio.h"

#ifdef USE_DEBUGGER
#if 0 //!
void my_printf(HANDLE hStdOut, const _TCHAR *format, ...)
{
	DWORD dwWritten;
	_TCHAR buffer[1024];
	va_list ap;
	
	va_start(ap, format);
	_vstprintf(buffer, format, ap);
	va_end(ap);
	
	WriteConsole(hStdOut, buffer, _tcslen(buffer), &dwWritten, NULL);
}

void my_putch(HANDLE hStdOut, _TCHAR c)
{
	DWORD dwWritten;
	
	WriteConsole(hStdOut, &c, 1, &dwWritten, NULL);
}

uint32 my_hexatoi(_TCHAR *str)
{
	_TCHAR *s;
	
	if(str == NULL || _tcslen(str) == 0) {
		return 0;
	} else if(_tcslen(str) == 3 && str[0] == _T('\'') && str[2] == _T('\'')) {
		// ank
		return str[1] & 0xff;
	} else if((s = _tcsstr(str, _T(":"))) != NULL) {
		// 0000:0000
		s[0] = _T('\0');
		return (my_hexatoi(str) << 4) + my_hexatoi(s + 1);
	} else if(str[0] == _T('%')) {
		// decimal
		return atoi(str + 1);
	}
	return _tcstol(str, NULL, 16);
}

break_point_t *get_break_point(DEBUGGER *debugger, _TCHAR *command)
{
	if(command[0] == _T('B') || command[0] == _T('b')) {
		return &debugger->bp;
	} else if(command[0] == _T('R') || command[0] == _T('r')) {
		return &debugger->rbp;
	} else if(command[0] == _T('W') || command[0] == _T('w')) {
		return &debugger->wbp;
	} else if(command[0] == _T('I') || command[0] == _T('i')) {
		return &debugger->ibp;
	} else if(command[0] == _T('O') || command[0] == _T('o')) {
		return &debugger->obp;
	}
	return NULL;
}

//unsigned __stdcall debugger_thread(void *lpx)
#endif //!

void EMU::initialize_debugger()
{
	now_debugging = false;
}

void EMU::release_debugger()
{
	close_debugger();
}

void EMU::open_debugger(int cpu_index)
{
#if 0
	if(!(now_debugging && debugger_thread_param.cpu_index == cpu_index)) {
		close_debugger();
		if(vm->get_cpu(cpu_index) != NULL && vm->get_cpu(cpu_index)->get_debugger() != NULL) {
			debugger_thread_param.emu = this;
			debugger_thread_param.vm = vm;
			debugger_thread_param.cpu_index = cpu_index;
			debugger_thread_param.request_terminate = false;
			if((hDebuggerThread = (HANDLE)_beginthreadex(NULL, 0, debugger_thread, &debugger_thread_param, 0, NULL)) != (HANDLE)0) {
				stop_rec_sound();
				stop_rec_video();
				now_debugging = true;
			}
		}
	}
#endif
}

void EMU::close_debugger()
{
	if(now_debugging) {
//		if(debugger_thread_param.running) {
//			debugger_thread_param.request_terminate = true;
//			WaitForSingleObject(hDebuggerThread, INFINITE);
//		}
//		CloseHandle(hDebuggerThread);
		now_debugging = false;
	}
}

bool EMU::debugger_enabled(int cpu_index)
{
   return false;
//	return (vm->get_cpu(cpu_index) != NULL && vm->get_cpu(cpu_index)->get_debugger() != NULL);
}

#endif

