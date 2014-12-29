/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ debugger console ]
*/

#include <conio.h>
#include <io.h>
#include <fcntl.h>
#include "res/resource.h"
#include "emu.h"
#include "vm/device.h"
#include "vm/debugger.h"
#include "vm/vm.h"
#include "fileio.h"

#ifdef USE_DEBUGGER

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

unsigned __stdcall debugger_thread(void *lpx)
{
	volatile debugger_thread_t *p = (debugger_thread_t *)lpx;
	p->running = true;
	
	DEVICE *cpu = p->vm->get_cpu(p->cpu_index);
	DEBUGGER *debugger = (DEBUGGER *)cpu->get_debugger();
	
	debugger->now_going = false;
	debugger->now_debugging = true;
	while(!debugger->now_suspended) {
		Sleep(10);
	}
	
	uint32 prog_addr_mask = cpu->debug_prog_addr_mask();
	uint32 data_addr_mask = cpu->debug_data_addr_mask();
	uint32 dump_addr = 0;
	uint32 dasm_addr = cpu->get_next_pc();
	
	// initialize console
	_TCHAR buffer[1024];
	_stprintf(buffer, _T("Debugger - %s"), _T(DEVICE_NAME));
	
	AllocConsole();
	SetConsoleTitle(buffer);
	
	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	bool cp932 = (GetConsoleCP() == 932);
	
	COORD coord;
	coord.X = 80;
	coord.Y = 4000;
	
	SetConsoleScreenBufferSize(hStdOut, coord);
	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	RemoveMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);
	
	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	cpu->debug_regs_info(buffer);
	my_printf(hStdOut, _T("%s\n"), buffer);
	
	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
	my_printf(hStdOut, _T("breaked at %08X\n"), cpu->get_next_pc());
	
	SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	cpu->debug_dasm(cpu->get_next_pc(), buffer);
	my_printf(hStdOut, _T("next\t%08X  %s\n"), cpu->get_next_pc(), buffer);
	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	
	#define MAX_COMMAND_LEN	64
	
	_TCHAR command[MAX_COMMAND_LEN + 1];
	_TCHAR prev_command[MAX_COMMAND_LEN + 1];
	
	memset(prev_command, 0, sizeof(prev_command));
	
	while(!p->request_terminate) {
		my_printf(hStdOut, _T("- "));
		
		// get command
		int ptr = 0;
		bool enter_done = false;
		
		while(!p->request_terminate && !enter_done) {
			DWORD dwRead;
			INPUT_RECORD ir[16];
			
			if(ReadConsoleInput(hStdIn, ir, 16, &dwRead)) {
				for(unsigned int i = 0; i < dwRead; i++) {
#ifdef _UNICODE
					if((ir[i].EventType & KEY_EVENT) && ir[i].Event.KeyEvent.bKeyDown && ir[i].Event.KeyEvent.uChar.UnicodeChar) {
						_TCHAR chr = ir[i].Event.KeyEvent.uChar.UnicodeChar;
#else
					if((ir[i].EventType & KEY_EVENT) && ir[i].Event.KeyEvent.bKeyDown && ir[i].Event.KeyEvent.uChar.AsciiChar) {
						_TCHAR chr = ir[i].Event.KeyEvent.uChar.AsciiChar;
#endif
						if(chr == 0x0d || chr == 0x0a) {
							if(ptr == 0 && prev_command[0] != _T('\0')) {
								memcpy(command, prev_command, sizeof(command));
								my_printf(hStdOut, _T("%s\n"), command);
								enter_done = true;
								break;
							} else if(ptr > 0) {
								command[ptr] = _T('\0');
								memcpy(prev_command, command, sizeof(command));
								my_printf(hStdOut, _T("\n"));
								enter_done = true;
								break;
							}
						} else if(chr == 0x08) {
							if(ptr > 0) {
								ptr--;
								my_putch(hStdOut, chr);
								my_putch(hStdOut, _T(' '));
								my_putch(hStdOut, chr);
							}
						} else if(chr >= 0x20 && chr <= 0x7e && ptr < MAX_COMMAND_LEN && !(chr == 0x20 && ptr == 0)) {
							command[ptr++] = chr;
							my_putch(hStdOut, chr);
						}
					}
				}
			}
			Sleep(10);
		}
		
		// process command
		if(!p->request_terminate && enter_done) {
			_TCHAR *params[32], *token = NULL;
			int num = 0;
			
			if((token = _tcstok(command, _T(" "))) != NULL) {
				params[num++] = token;
				while(num < 32 && (token = _tcstok(NULL, _T(" "))) != NULL) {
					params[num++] = token;
				}
			}
			if(_tcsicmp(params[0], _T("D")) == 0) {
				if(num <= 3) {
					uint32 start_addr = dump_addr;
					if(num >= 2) {
						start_addr = my_hexatoi(params[1]);
					}
					start_addr &= data_addr_mask;
					
					uint32 end_addr = start_addr + 8 * 16 - 1;
					if(num == 3) {
						end_addr = my_hexatoi(params[2]);
					}
					end_addr &= data_addr_mask;
					
					if(start_addr > end_addr) {
						end_addr = data_addr_mask;
					}
					for(uint64 addr = start_addr & ~0x0f; addr <= end_addr; addr++) {
						if(addr > data_addr_mask) {
							end_addr = data_addr_mask;
							break;
						}
						if((addr & 0x0f) == 0) {
							my_printf(hStdOut, _T("%08X "), addr & data_addr_mask);
							memset(buffer, 0, sizeof(buffer));
						}
						if(addr < start_addr) {
							my_printf(hStdOut, _T("   "));
							buffer[addr & 0x0f] = _T(' ');
						} else {
							uint32 data = cpu->debug_read_data8(addr & data_addr_mask);
							my_printf(hStdOut, _T(" %02X"), data);
							buffer[addr & 0x0f] = ((data >= 0x20 && data <= 0x7e) || (cp932 && data >= 0xa1 && data <= 0xdf)) ? data : _T('.');
						}
						if((addr & 0x0f) == 0x0f) {
							my_printf(hStdOut, _T("  %s\n"), buffer);
						}
					}
					if((end_addr & 0x0f) != 0x0f) {
						for(uint32 addr = (end_addr & 0x0f) + 1; addr <= 0x0f; addr++) {
							my_printf(hStdOut, _T("   "));
						}
						my_printf(hStdOut, _T("  %s\n"), buffer);
					}
					dump_addr = (end_addr + 1) & data_addr_mask;
					prev_command[1] = _T('\0'); // remove parameters to dump continuously
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("E")) == 0 || _tcsicmp(params[0], _T("EB")) == 0) {
				if(num >= 3) {
					uint32 addr = my_hexatoi(params[1]) & data_addr_mask;
					for(int i = 2; i < num; i++) {
						cpu->debug_write_data8(addr, my_hexatoi(params[i]) & 0xff);
						addr = (addr + 1) & data_addr_mask;
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("EW")) == 0) {
				if(num >= 3) {
					uint32 addr = my_hexatoi(params[1]) & data_addr_mask;
					for(int i = 2; i < num; i++) {
						cpu->debug_write_data16(addr, my_hexatoi(params[i]) & 0xffff);
						addr = (addr + 2) & data_addr_mask;
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("ED")) == 0) {
				if(num >= 3) {
					uint32 addr = my_hexatoi(params[1]) & data_addr_mask;
					for(int i = 2; i < num; i++) {
						cpu->debug_write_data32(addr, my_hexatoi(params[i]));
						addr = (addr + 4) & data_addr_mask;
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("EA")) == 0) {
				if(num >= 3) {
					uint32 addr = my_hexatoi(params[1]) & data_addr_mask;
					_tcscpy(buffer, prev_command);
					if((token = _tcstok(buffer, _T("\""))) != NULL && (token = _tcstok(NULL, _T("\""))) != NULL) {
						int len = _tcslen(token);
						for(int i = 0; i < len; i++) {
							cpu->debug_write_data8(addr, token[i] & 0xff);
							addr = (addr + 1) & data_addr_mask;
						}
					} else {
						my_printf(hStdOut, _T("invalid parameter\n"));
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("I")) == 0 || _tcsicmp(params[0], _T("IB")) == 0) {
				if(num == 2) {
					my_printf(hStdOut, _T("%02X\n"), cpu->debug_read_io8(my_hexatoi(params[1])) & 0xff);
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("IW")) == 0) {
				if(num == 2) {
					my_printf(hStdOut, _T("%02X\n"), cpu->debug_read_io16(my_hexatoi(params[1])) & 0xffff);
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("ID")) == 0) {
				if(num == 2) {
					my_printf(hStdOut, _T("%02X\n"), cpu->debug_read_io32(my_hexatoi(params[1])));
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("O")) == 0 || _tcsicmp(params[0], _T("OB")) == 0) {
				if(num == 3) {
					cpu->debug_write_io8(my_hexatoi(params[1]), my_hexatoi(params[2]) & 0xff);
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("OW")) == 0) {
				if(num == 3) {
					cpu->debug_write_io16(my_hexatoi(params[1]), my_hexatoi(params[2]) & 0xffff);
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("OD")) == 0) {
				if(num == 3) {
					cpu->debug_write_io32(my_hexatoi(params[1]), my_hexatoi(params[2]));
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("R")) == 0) {
				if(num == 1) {
					cpu->debug_regs_info(buffer);
					my_printf(hStdOut, _T("%s\n"), buffer);
				} else if(num == 3) {
					if(!cpu->debug_write_reg(params[1], my_hexatoi(params[2]))) {
						my_printf(hStdOut, _T("unknown register %s\n"), params[1]);
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("S")) == 0) {
				if(num >= 4) {
					uint32 start_addr = my_hexatoi(params[1]) & data_addr_mask;
					uint32 end_addr = my_hexatoi(params[2]) & data_addr_mask;
					uint8 list[32];
					for(int i = 3, j = 0; i < num; i++, j++) {
						list[j] = my_hexatoi(params[i]);
					}
					for(uint64 addr = start_addr; addr <= end_addr; addr++) {
						bool found = true;
						for(int i = 3, j = 0; i < num; i++, j++) {
							if(cpu->debug_read_data8((addr + j) & data_addr_mask) != list[j]) {
								found = false;
								break;
							}
						}
						if(found) {
							my_printf(hStdOut, _T("%08X\n"), addr);
						}
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("U")) == 0) {
				if(num <= 3) {
					if(num >= 2) {
						dasm_addr = my_hexatoi(params[1]) & prog_addr_mask;
					}
					if(num == 3) {
						uint32 end_addr = my_hexatoi(params[2]) & prog_addr_mask;
						while(dasm_addr <= end_addr) {
							int len = cpu->debug_dasm(dasm_addr, buffer);
							my_printf(hStdOut, _T("%08X  %s\n"), dasm_addr, buffer);
							dasm_addr = (dasm_addr + len) & prog_addr_mask;
						}
					} else {
						for(int i = 0; i < 16; i++) {
							int len = cpu->debug_dasm(dasm_addr, buffer);
							my_printf(hStdOut, _T("%08X  %s\n"), dasm_addr, buffer);
							dasm_addr = (dasm_addr + len) & prog_addr_mask;
						}
					}
					prev_command[1] = _T('\0'); // remove parameters to disassemble continuously
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("H")) == 0) {
				if(num == 3) {
					uint32 l = my_hexatoi(params[1]);
					uint32 r = my_hexatoi(params[2]);
					my_printf(hStdOut, _T("%08X  %08X\n"), l + r, l - r);
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("N")) == 0) {
				if(num >= 2 && params[1][0] == _T('\"')) {
					_tcscpy(buffer, prev_command);
					if((token = _tcstok(buffer, _T("\""))) != NULL && (token = _tcstok(NULL, _T("\""))) != NULL) {
						_tcscpy(debugger->file_path, token);
					} else {
						my_printf(hStdOut, _T("invalid parameter\n"));
					}
				} else if(num == 2) {
					_tcscpy(debugger->file_path, params[1]);
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("L")) == 0) {
				if(num == 3) {
					uint32 start_addr = my_hexatoi(params[1]) & data_addr_mask, end_addr = my_hexatoi(params[2]) & data_addr_mask;
					FILEIO* fio = new FILEIO();
					if(fio->Fopen(debugger->file_path, FILEIO_READ_BINARY)) {
						for(uint32 addr = start_addr; addr <= end_addr; addr++) {
							int data = fio->Fgetc();
							if(data == EOF) {
								break;
							}
							cpu->debug_write_data8(addr & data_addr_mask, data);
						}
						fio->Fclose();
					} else {
						my_printf(hStdOut, _T("can't open %s\n"), debugger->file_path);
					}
					delete fio;
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("W")) == 0) {
				if(num == 3) {
					uint32 start_addr = my_hexatoi(params[1]) & data_addr_mask, end_addr = my_hexatoi(params[2]) & data_addr_mask;
					FILEIO* fio = new FILEIO();
					if(fio->Fopen(debugger->file_path, FILEIO_WRITE_BINARY)) {
						for(uint32 addr = start_addr; addr <= end_addr; addr++) {
							fio->Fputc(cpu->debug_read_data8(addr & data_addr_mask));
						}
						fio->Fclose();
					} else {
						my_printf(hStdOut, _T("can't open %s\n"), debugger->file_path);
					}
					delete fio;
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T( "BP")) == 0 || _tcsicmp(params[0], _T("RBP")) == 0 || _tcsicmp(params[0], _T("WBP")) == 0) {
				break_point_t *bp = get_break_point(debugger, params[0]);
				if(num == 2) {
					uint32 addr = my_hexatoi(params[1]);
					bool found = false;
					for(int i = 0; i < MAX_BREAK_POINTS && !found; i++) {
						if(bp->table[i].status == 0 || (bp->table[i].addr == addr && bp->table[i].mask == prog_addr_mask)) {
							bp->table[i].addr = addr;
							bp->table[i].mask = prog_addr_mask;
							bp->table[i].status = 1;
							found = true;
						}
					}
					if(!found) {
						my_printf(hStdOut, _T("too many break points\n"));
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("IBP")) == 0 || _tcsicmp(params[0], _T("OBP")) == 0) {
				break_point_t *bp = get_break_point(debugger, params[0]);
				if(num == 2 || num == 3) {
					uint32 addr = my_hexatoi(params[1]), mask = 0xff;
					if(num == 3) {
						mask = my_hexatoi(params[2]);
					}
					bool found = false;
					for(int i = 0; i < MAX_BREAK_POINTS && !found; i++) {
						if(bp->table[i].status == 0 || (bp->table[i].addr == addr && bp->table[i].mask == mask)) {
							bp->table[i].addr = addr;
							bp->table[i].mask = mask;
							bp->table[i].status = 1;
							found = true;
						}
					}
					if(!found) {
						my_printf(hStdOut, _T("too many break points\n"));
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("BC")) == 0 || _tcsicmp(params[0], _T("RBC")) == 0 || _tcsicmp(params[0], _T("WBC")) == 0 || _tcsicmp(params[0], _T("IBC")) == 0 || _tcsicmp(params[0], _T("OBC")) == 0) {
				break_point_t *bp = get_break_point(debugger, params[0]);
				if(num == 2 && _tcsicmp(params[1], _T("ALL")) == 0) {
					memset(bp->table, 0, sizeof(bp->table));
				} else if(num >= 2) {
					for(int i = 1; i < num; i++) {
						int index = my_hexatoi(params[i]);
						if(!(index >= 1 && index <= MAX_BREAK_POINTS)) {
							my_printf(hStdOut, _T("invalid index %x\n"), index);
						} else {
							bp->table[index - 1].addr = bp->table[index - 1].mask = 0;
							bp->table[index - 1].status = 0;
						}
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("BD")) == 0 || _tcsicmp(params[0], _T("RBD")) == 0 || _tcsicmp(params[0], _T("WBD")) == 0 || _tcsicmp(params[0], _T("IBD")) == 0 || _tcsicmp(params[0], _T("OBD")) == 0 ||
			          _tcsicmp(params[0], _T("BE")) == 0 || _tcsicmp(params[0], _T("RBE")) == 0 || _tcsicmp(params[0], _T("WBE")) == 0 || _tcsicmp(params[0], _T("IBE")) == 0 || _tcsicmp(params[0], _T("OBE")) == 0) {
				break_point_t *bp = get_break_point(debugger, params[0]);
				bool enabled = (params[0][1] == _T('E') || params[0][1] == _T('e') || params[0][2] == _T('E') || params[0][2] == _T('e'));
				if(num == 2 && _tcsicmp(params[1], _T("ALL")) == 0) {
					for(int i = 0; i < MAX_BREAK_POINTS; i++) {
						if(bp->table[i].status != 0) {
							bp->table[i].status = enabled ? 1 : -1;
						}
					}
				} else if(num >= 2) {
					for(int i = 1; i < num; i++) {
						int index = my_hexatoi(params[i]);
						if(!(index >= 1 && index <= MAX_BREAK_POINTS)) {
							my_printf(hStdOut, _T("invalid index %x\n"), index);
						} else if(bp->table[index - 1].status == 0) {
							my_printf(hStdOut, _T("break point %x is null\n"), index);
						} else {
							bp->table[index - 1].status = enabled ? 1 : -1;
						}
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("BL")) == 0 || _tcsicmp(params[0], _T("RBL")) == 0 || _tcsicmp(params[0], _T("WBL")) == 0) {
				if(num == 1) {
					break_point_t *bp = get_break_point(debugger, params[0]);
					for(int i = 0; i < MAX_BREAK_POINTS; i++) {
						if(bp->table[i].status) {
							my_printf(hStdOut, _T("%d %c %08X\n"), i + 1, bp->table[i].status == 1 ? _T('e') : _T('d'), bp->table[i].addr);
						}
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("IBL")) == 0 || _tcsicmp(params[0], _T("OBL")) == 0) {
				if(num == 1) {
					break_point_t *bp = get_break_point(debugger, params[0]);
					for(int i = 0; i < MAX_BREAK_POINTS; i++) {
						if(bp->table[i].status) {
							my_printf(hStdOut, _T("%d %c %08X %08X\n"), i + 1, bp->table[i].status == 1 ? _T('e') : _T('d'), bp->table[i].addr, bp->table[i].mask);
						}
					}
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("G")) == 0) {
				if(num == 1 || num == 2) {
					if(num >= 2) {
						debugger->store_break_points();
						debugger->bp.table[0].addr = my_hexatoi(params[1]) & prog_addr_mask;
						debugger->bp.table[0].mask = prog_addr_mask;
						debugger->bp.table[0].status = 1;
					}
					debugger->now_going = true;
					debugger->now_suspended = false;
					while(!p->request_terminate && !debugger->now_suspended) {
						if((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
							break;
						}
						Sleep(10);
					}
					// break cpu
					debugger->now_going = false;
					while(!p->request_terminate && !debugger->now_suspended) {
						Sleep(10);
					}
					dasm_addr = cpu->get_next_pc();
					
					SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					cpu->debug_dasm(cpu->get_pc(), buffer);
					my_printf(hStdOut, _T("done\t%08X  %s\n"), cpu->get_pc(), buffer);
					
					SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					cpu->debug_regs_info(buffer);
					my_printf(hStdOut, _T("%s\n"), buffer);
					
					if(debugger->hit()) {
						SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
						if(debugger->bp.hit) {
							my_printf(hStdOut, _T("breaked at %08X\n"), cpu->get_next_pc());
						} else if(debugger->rbp.hit) {
							my_printf(hStdOut, _T("breaked at %08X: memory %08X was read at %08X\n"), cpu->get_next_pc(), debugger->rbp.hit_addr, cpu->get_pc());
						} else if(debugger->wbp.hit) {
							my_printf(hStdOut, _T("breaked at %08X: memory %08X was written at %08X\n"), cpu->get_next_pc(), debugger->wbp.hit_addr, cpu->get_pc());
						} else if(debugger->ibp.hit) {
							my_printf(hStdOut, _T("breaked at %08X: port %08X was read at %08X\n"), cpu->get_next_pc(), debugger->ibp.hit_addr, cpu->get_pc());
						} else if(debugger->obp.hit) {
							my_printf(hStdOut, _T("breaked at %08X: port %08X was written at %08X\n"), cpu->get_next_pc(), debugger->obp.hit_addr, cpu->get_pc());
						}
						debugger->bp.hit = debugger->rbp.hit = debugger->wbp.hit = debugger->ibp.hit = debugger->obp.hit = false;
					} else {
						SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
						my_printf(hStdOut, _T("breaked at %08X: esc key was pressed\n"), cpu->get_next_pc());
					}
					if(num >= 2) {
						debugger->restore_break_points();
					}
					SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					cpu->debug_dasm(cpu->get_next_pc(), buffer);
					my_printf(hStdOut, _T("next\t%08X  %s\n"), cpu->get_next_pc(), buffer);
					SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("T")) == 0) {
				if(num == 1 || num == 2) {
					int steps = 1;
					if(num >= 2) {
						steps = my_hexatoi(params[1]);
					}
					for(int i = 0; i < steps; i++) {
						debugger->now_going = false;
						debugger->now_suspended = false;
						while(!p->request_terminate && !debugger->now_suspended) {
							Sleep(10);
						}
						dasm_addr = cpu->get_next_pc();
						
						SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
						cpu->debug_dasm(cpu->get_pc(), buffer);
						my_printf(hStdOut, _T("done\t%08X  %s\n"), cpu->get_pc(), buffer);
						
						SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
						cpu->debug_regs_info(buffer);
						my_printf(hStdOut, _T("%s\n"), buffer);
						
						if(debugger->hit() || (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
							break;
						}
					}
					if(debugger->hit()) {
						SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
						if(debugger->bp.hit) {
							my_printf(hStdOut, _T("breaked at %08X\n"), cpu->get_next_pc());
						} else if(debugger->rbp.hit) {
							my_printf(hStdOut, _T("breaked at %08X: memory %08X was read at %08X\n"), cpu->get_next_pc(), debugger->rbp.hit_addr, cpu->get_pc());
						} else if(debugger->wbp.hit) {
							my_printf(hStdOut, _T("breaked at %08X: memory %08X was written at %08X\n"), cpu->get_next_pc(), debugger->wbp.hit_addr, cpu->get_pc());
						} else if(debugger->ibp.hit) {
							my_printf(hStdOut, _T("breaked at %08X: port %08X was read at %08X\n"), cpu->get_next_pc(), debugger->ibp.hit_addr, cpu->get_pc());
						} else if(debugger->obp.hit) {
							my_printf(hStdOut, _T("breaked at %08X: port %08X was written at %08X\n"), cpu->get_next_pc(), debugger->obp.hit_addr, cpu->get_pc());
						}
						debugger->bp.hit = debugger->rbp.hit = debugger->wbp.hit = debugger->ibp.hit = debugger->obp.hit = false;
					}
					SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					cpu->debug_dasm(cpu->get_next_pc(), buffer);
					my_printf(hStdOut, _T("next\t%08X  %s\n"), cpu->get_next_pc(), buffer);
					SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
				} else {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("Q")) == 0) {
				PostMessage(p->emu->main_window_handle, WM_COMMAND, ID_CLOSE_DEBUGGER, 0L);
				break;
			} else if(_tcsicmp(params[0], _T("!")) == 0) {
				if(num == 1) {
					my_printf(hStdOut, _T("invalid parameter number\n"));
				} else if(_tcsicmp(params[1], _T("RESET")) == 0) {
					if(num == 2) {
						p->vm->reset();
					} else if(num == 3) {
						if(_tcsicmp(params[2], _T("ALL")) == 0) {
							p->vm->reset();
						} if(_tcsicmp(params[2], _T("CPU")) == 0) {
							cpu->reset();
						} else {
							my_printf(hStdOut, _T("unknown device %s\n"), params[2]);
						}
					} else {
						my_printf(hStdOut, _T("invalid parameter number\n"));
					}
				} else if(_tcsicmp(params[1], _T("KEY")) == 0) {
					if(num == 3 || num == 4) {
						int code =  my_hexatoi(params[2]) & 0xff, msec = 100;
						if(num == 4) {
							msec = my_hexatoi(params[3]);
						}
#ifdef SUPPORT_VARIABLE_TIMING
						p->emu->key_buffer()[code] = max((int)(p->vm->frame_rate() * (double)msec / 1000.0 + 0.5), 1);
#else
						p->emu->key_buffer()[code] = max((int)(FRAMES_PER_SEC * (double)msec / 1000.0 + 0.5), 1);
#endif
#ifdef NOTIFY_KEY_DOWN
						p->vm->key_down(code, false);
#endif
					} else {
						my_printf(hStdOut, _T("invalid parameter number\n"));
					}
				} else {
					my_printf(hStdOut, _T("unknown command ! %s\n"), params[1]);
				}
			} else if(_tcsicmp(params[0], _T("?")) == 0) {
				my_printf(hStdOut, _T("D [<range>] - dump memory\n"));
				my_printf(hStdOut, _T("E[{B,W,D}] <address> <list> - edit memory (byte,word,dword)\n"));
				my_printf(hStdOut, _T("EA <address> \"<value>\" - edit memory (ascii)\n"));
				my_printf(hStdOut, _T("I[{B,W,D}] <port> - input port (byte,word,dword)\n"));
				my_printf(hStdOut, _T("O[{B,W,D}] <port> <value> - output port (byte,word,dword)\n"));
				my_printf(hStdOut, _T("R - show register(s)\n"));
				my_printf(hStdOut, _T("R <reg> <value> - edit register\n"));
				my_printf(hStdOut, _T("S <range> <list> - search\n"));
				my_printf(hStdOut, _T("U [<range>] - unassemble\n"));
				
				my_printf(hStdOut, _T("H <value> <value> - hexadd\n"));
				my_printf(hStdOut, _T("N <filename> - name\n"));
				my_printf(hStdOut, _T("L <range> - load file\n"));
				my_printf(hStdOut, _T("W <range> - write file\n"));
				
				my_printf(hStdOut, _T("BP <address> - set breakpoint\n"));
				my_printf(hStdOut, _T("{R,W}BP <address> - set breakpoint (break at memory access)\n"));
				my_printf(hStdOut, _T("{I,O}BP <port> [<mask>] - set breakpoint (break at i/o access)\n"));
				my_printf(hStdOut, _T("[{R,W,I,O}]B{C,D,E} {all,<list>} - clear/disable/enable breakpoint(s)\n"));
				my_printf(hStdOut, _T("[{R,W,I,O}]BL - list breakpoint(s)\n"));
				
				my_printf(hStdOut, _T("G - go (press esc key to break)\n"));
				my_printf(hStdOut, _T("G <address> - go and break at address\n"));
				my_printf(hStdOut, _T("T [<count>] - trace\n"));
				my_printf(hStdOut, _T("Q - quit\n"));
				
				my_printf(hStdOut, _T("! reset [cpu] - reset\n"));
				my_printf(hStdOut, _T("! key <code> [<msec>] - press key\n"));
				
				my_printf(hStdOut, _T("<value> - hexa, decimal(%%d), ascii('a')\n"));
			} else {
				my_printf(hStdOut, _T("unknown command %s\n"), params[0]);
			}
		}
	}
	
	// stop debugger
	try {
		debugger->now_debugging = debugger->now_going = debugger->now_suspended = false;
	} catch(...) {
	}
	
	// release console
	FreeConsole();
	
	p->running = false;
	_endthreadex(0);
	return 0;
}

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
}

void EMU::close_debugger()
{
	if(now_debugging) {
		if(debugger_thread_param.running) {
			debugger_thread_param.request_terminate = true;
			WaitForSingleObject(hDebuggerThread, INFINITE);
		}
		CloseHandle(hDebuggerThread);
		now_debugging = false;
	}
}

bool EMU::debugger_enabled(int cpu_index)
{
	return (vm->get_cpu(cpu_index) != NULL && vm->get_cpu(cpu_index)->get_debugger() != NULL);
}

#endif

