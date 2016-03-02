/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ debugger console ]
*/
#if !defined(_MSC_VER)
#include <SDL.h>
#endif
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include "vm/device.h"
#include "vm/debugger.h"
#include "vm/vm.h"
#include "fileio.h"
#if defined(OSD_QT)
#include "emu_thread.h"
#endif

#ifdef USE_DEBUGGER

#ifndef FOREGROUND_BLUE
#define FOREGROUND_BLUE      0x0001 // text color contains blue.
#endif
#ifndef FOREGROUND_GREEN
#define FOREGROUND_GREEN     0x0002 // text color contains green.
#endif
#ifndef FOREGROUND_RED
#define FOREGROUND_RED       0x0004 // text color contains red.
#endif
#ifndef FOREGROUND_INTENSITY
#define FOREGROUND_INTENSITY 0x0008 // text color is intensified.
#endif

static FILEIO* logfile = NULL;

void my_printf(OSD *osd, const _TCHAR *format, ...)
{
	_TCHAR buffer[1024];
	va_list ap;
	
	va_start(ap, format);
	my_vstprintf_s(buffer, 1024, format, ap);
	va_end(ap);
#if defined(_MSC_VER)	
	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fwrite(buffer, lstrlen(buffer) * sizeof(_TCHAR), 1);
	}
#else	
	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fwrite(buffer, strlen(buffer) * sizeof(_TCHAR), 1);
	}
#endif
	osd->write_console(buffer, _tcslen(buffer));
}

void my_putch(OSD *osd, _TCHAR c)
{
	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fwrite(&c, sizeof(_TCHAR), 1);
	}
	osd->write_console(&c, 1);
}

uint32_t my_hexatoi(const _TCHAR *str)
{
	_TCHAR tmp[1024], *s;
	
	if(str == NULL || _tcslen(str) == 0) {
		return 0;
	}
	my_tcscpy_s(tmp, 1024, str);
	
	if(_tcslen(tmp) == 3 && tmp[0] == _T('\'') && tmp[2] == _T('\'')) {
		// ank
		return tmp[1] & 0xff;
	} else if((s = _tcsstr(tmp, _T(":"))) != NULL) {
		// 0000:0000
		s[0] = _T('\0');
		return (my_hexatoi(tmp) << 4) + my_hexatoi(s + 1);
	} else if(tmp[0] == _T('%')) {
		// decimal
#if defined(__MINGW32__)
		return atoi(tmp + 1);
#else
		return _tstoi(tmp + 1);
#endif
	}
	return _tcstoul(tmp, NULL, 16);
}

uint8_t my_hexatob(char *value)
{
	char tmp[3];
	tmp[0] = value[0];
	tmp[1] = value[1];
	tmp[2] = '\0';
	return (uint8_t)strtoul(tmp, NULL, 16);
}

uint16_t my_hexatow(char *value)
{
	char tmp[5];
	tmp[0] = value[0];
	tmp[1] = value[1];
	tmp[2] = value[2];
	tmp[3] = value[3];
	tmp[4] = '\0';
	return (uint16_t)strtoul(tmp, NULL, 16);
}

break_point_t *get_break_point(DEBUGGER *debugger, const _TCHAR *command)
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

static uint32_t dump_addr;
static uint32_t dasm_addr;

int debugger_command(debugger_thread_t *p, _TCHAR *command, _TCHAR *prev_command, bool cp932)
{
	DEVICE *cpu = p->vm->get_cpu(p->cpu_index);
	DEBUGGER *debugger = (DEBUGGER *)cpu->get_debugger();
	uint32_t prog_addr_mask = cpu->debug_prog_addr_mask();
	uint32_t data_addr_mask = cpu->debug_data_addr_mask();
	//while(!debugger->now_suspended) {
	//		p->osd->sleep(10);
	//}
	
	// initialize console
	_TCHAR buffer[1024];
	if(strlen(command) > 0) {
			_TCHAR *params[32], *token = NULL, *context = NULL;
			int num = 0;
			
			if((token = my_tcstok_s(command, _T(" "), &context)) != NULL) {
				params[num++] = token;
				while(num < 32 && (token = my_tcstok_s(NULL, _T(" "), &context)) != NULL) {
					params[num++] = token;
				}
			}
			if(_tcsicmp(params[0], _T("D")) == 0) {
				if(num <= 3) {
					uint32_t start_addr = dump_addr;
					if(num >= 2) {
						start_addr = my_hexatoi(params[1]);
					}
					start_addr &= data_addr_mask;
					
					uint32_t end_addr = start_addr + 8 * 16 - 1;
					if(num == 3) {
						end_addr = my_hexatoi(params[2]);
					}
					end_addr &= data_addr_mask;
					
					if(start_addr > end_addr) {
						end_addr = data_addr_mask;
					}
					for(uint64_t addr = start_addr & ~0x0f; addr <= end_addr; addr++) {
						if(addr > data_addr_mask) {
							end_addr = data_addr_mask;
							break;
						}
						if((addr & 0x0f) == 0) {
							my_printf(p->osd, _T("%08X "), addr & data_addr_mask);
							memset(buffer, 0, sizeof(buffer));
						}
						if(addr < start_addr) {
							my_printf(p->osd, _T("   "));
							buffer[addr & 0x0f] = _T(' ');
						} else {
							uint32_t data = cpu->read_debug_data8(addr & data_addr_mask);
							my_printf(p->osd, _T(" %02X"), data);
							buffer[addr & 0x0f] = ((data >= 0x20 && data <= 0x7e) || (cp932 && data >= 0xa1 && data <= 0xdf)) ? data : _T('.');
						}
						if((addr & 0x0f) == 0x0f) {
							my_printf(p->osd, _T("  %s\n"), buffer);
						}
					}
					if((end_addr & 0x0f) != 0x0f) {
						for(uint32_t addr = (end_addr & 0x0f) + 1; addr <= 0x0f; addr++) {
							my_printf(p->osd, _T("   "));
						}
						my_printf(p->osd, _T("  %s\n"), buffer);
					}
					dump_addr = (end_addr + 1) & data_addr_mask;
					prev_command[1] = _T('\0'); // remove parameters to dump continuously
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("E")) == 0 || _tcsicmp(params[0], _T("EB")) == 0) {
				if(num >= 3) {
					uint32_t addr = my_hexatoi(params[1]) & data_addr_mask;
					for(int i = 2; i < num; i++) {
						cpu->write_debug_data8(addr, my_hexatoi(params[i]) & 0xff);
						addr = (addr + 1) & data_addr_mask;
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("EW")) == 0) {
				if(num >= 3) {
					uint32_t addr = my_hexatoi(params[1]) & data_addr_mask;
					for(int i = 2; i < num; i++) {
						cpu->write_debug_data16(addr, my_hexatoi(params[i]) & 0xffff);
						addr = (addr + 2) & data_addr_mask;
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("ED")) == 0) {
				if(num >= 3) {
					uint32_t addr = my_hexatoi(params[1]) & data_addr_mask;
					for(int i = 2; i < num; i++) {
						cpu->write_debug_data32(addr, my_hexatoi(params[i]));
						addr = (addr + 4) & data_addr_mask;
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("EA")) == 0) {
				if(num >= 3) {
					uint32_t addr = my_hexatoi(params[1]) & data_addr_mask;
					my_tcscpy_s(buffer, 1024, prev_command);
					if((token = my_tcstok_s(buffer, _T("\""), &context)) != NULL && (token = my_tcstok_s(NULL, _T("\""), &context)) != NULL) {
						int len = _tcslen(token);
						for(int i = 0; i < len; i++) {
							cpu->write_debug_data8(addr, token[i] & 0xff);
							addr = (addr + 1) & data_addr_mask;
						}
					} else {
						my_printf(p->osd, _T("invalid parameter\n"));
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("I")) == 0 || _tcsicmp(params[0], _T("IB")) == 0) {
				if(num == 2) {
					my_printf(p->osd, _T("%02X\n"), cpu->read_debug_io8(my_hexatoi(params[1])) & 0xff);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("IW")) == 0) {
				if(num == 2) {
					my_printf(p->osd, _T("%02X\n"), cpu->read_debug_io16(my_hexatoi(params[1])) & 0xffff);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("ID")) == 0) {
				if(num == 2) {
					my_printf(p->osd, _T("%02X\n"), cpu->read_debug_io32(my_hexatoi(params[1])));
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("O")) == 0 || _tcsicmp(params[0], _T("OB")) == 0) {
				if(num == 3) {
					cpu->write_debug_io8(my_hexatoi(params[1]), my_hexatoi(params[2]) & 0xff);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("OW")) == 0) {
				if(num == 3) {
					cpu->write_debug_io16(my_hexatoi(params[1]), my_hexatoi(params[2]) & 0xffff);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("OD")) == 0) {
				if(num == 3) {
					cpu->write_debug_io32(my_hexatoi(params[1]), my_hexatoi(params[2]));
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("R")) == 0) {
				if(num == 1) {
					cpu->debug_regs_info(buffer, 1024);
					my_printf(p->osd, _T("%s\n"), buffer);
				} else if(num == 3) {
					if(!cpu->write_debug_reg(params[1], my_hexatoi(params[2]))) {
						my_printf(p->osd, _T("unknown register %s\n"), params[1]);
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("S")) == 0) {
				if(num >= 4) {
					uint32_t start_addr = my_hexatoi(params[1]) & data_addr_mask;
					uint32_t end_addr = my_hexatoi(params[2]) & data_addr_mask;
					uint8_t list[32];
					for(int i = 3, j = 0; i < num; i++, j++) {
						list[j] = my_hexatoi(params[i]);
					}
					for(uint64_t addr = start_addr; addr <= end_addr; addr++) {
						bool found = true;
						for(int i = 3, j = 0; i < num; i++, j++) {
							if(cpu->read_debug_data8((addr + j) & data_addr_mask) != list[j]) {
								found = false;
								break;
							}
						}
						if(found) {
							my_printf(p->osd, _T("%08X\n"), addr);
						}
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("U")) == 0) {
				if(num <= 3) {
					if(num >= 2) {
						dasm_addr = my_hexatoi(params[1]) & prog_addr_mask;
					}
					if(num == 3) {
						uint32_t end_addr = my_hexatoi(params[2]) & prog_addr_mask;
						while(dasm_addr <= end_addr) {
							int len = cpu->debug_dasm(dasm_addr, buffer, 1024);
							my_printf(p->osd, _T("%08X  "), dasm_addr);
							for(int i = 0; i < len; i++) {
								my_printf(p->osd, _T("%02X"), cpu->read_debug_data8((dasm_addr + i) & data_addr_mask));
							}
							for(int i = len; i < 8; i++) {
								my_printf(p->osd, _T("  "));
							}
							my_printf(p->osd, _T("  %s\n"), buffer);
							dasm_addr = (dasm_addr + len) & prog_addr_mask;
						}
					} else {
						for(int i = 0; i < 16; i++) {
							int len = cpu->debug_dasm(dasm_addr, buffer, 1024);
							my_printf(p->osd, _T("%08X  "), dasm_addr);
							for(int i = 0; i < len; i++) {
								my_printf(p->osd, _T("%02X"), cpu->read_debug_data8((dasm_addr + i) & data_addr_mask));
							}
							for(int i = len; i < 8; i++) {
								my_printf(p->osd, _T("  "));
							}
							my_printf(p->osd, _T("  %s\n"), buffer);
							dasm_addr = (dasm_addr + len) & prog_addr_mask;
						}
					}
					prev_command[1] = _T('\0'); // remove parameters to disassemble continuously
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("H")) == 0) {
				if(num == 3) {
					uint32_t l = my_hexatoi(params[1]);
					uint32_t r = my_hexatoi(params[2]);
					my_printf(p->osd, _T("%08X  %08X\n"), l + r, l - r);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("N")) == 0) {
				if(num >= 2 && params[1][0] == _T('\"')) {
					my_tcscpy_s(buffer, 1024, prev_command);
					if((token = my_tcstok_s(buffer, _T("\""), &context)) != NULL && (token = my_tcstok_s(NULL, _T("\""), &context)) != NULL) {
						my_tcscpy_s(debugger->file_path, _MAX_PATH, token);
					} else {
						my_printf(p->osd, _T("invalid parameter\n"));
					}
				} else if(num == 2) {
					my_tcscpy_s(debugger->file_path, _MAX_PATH, params[1]);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("L")) == 0) {
				FILEIO* fio = new FILEIO();
				if(check_file_extension(debugger->file_path, _T(".hex"))) {
					if(fio->Fopen(debugger->file_path, FILEIO_READ_ASCII)) {
						uint32_t start_addr = 0, linear = 0, segment = 0;
						if(num >= 2) {
							start_addr = my_hexatoi(params[1]);
						}
						char line[1024];
						while(fio->Fgets(line, sizeof(line)) != NULL) {
							if(line[0] != ':') continue;
							int type = my_hexatob(line + 7);
							if(type == 0x00) {
								uint32_t bytes = my_hexatob(line + 1);
								uint32_t addr = my_hexatow(line + 3) + start_addr + linear + segment;
								for(uint32_t i = 0; i < bytes; i++) {
									cpu->write_debug_data8((addr + i) & data_addr_mask, my_hexatob(line + 9 + 2 * i));
								}
							} else if(type == 0x01) {
								break;
							} else if(type == 0x02) {
								segment = my_hexatow(line + 9) << 4;
								start_addr = 0;
							} else if(type == 0x04) {
								linear = my_hexatow(line + 9) << 16;
								start_addr = 0;
							}
						}
						fio->Fclose();
					} else {
						my_printf(p->osd, _T("can't open %s\n"), debugger->file_path);
					}
				} else {
					if(fio->Fopen(debugger->file_path, FILEIO_READ_BINARY)) {
						uint32_t start_addr = 0x100, end_addr = data_addr_mask;
						if(num >= 2) {
							start_addr = my_hexatoi(params[1]) & data_addr_mask;
						}
						if(num >= 3) {
							end_addr = my_hexatoi(params[2]) & data_addr_mask;
						}
						for(uint32_t addr = start_addr; addr <= end_addr; addr++) {
							int data = fio->Fgetc();
							if(data == EOF) {
								break;
							}
							cpu->write_debug_data8(addr & data_addr_mask, data);
						}
						fio->Fclose();
					} else {
						my_printf(p->osd, _T("can't open %s\n"), debugger->file_path);
					}
				}
				delete fio;
			} else if(_tcsicmp(params[0], _T("W")) == 0) {
				if(num == 3) {
					uint32_t start_addr = my_hexatoi(params[1]) & data_addr_mask, end_addr = my_hexatoi(params[2]) & data_addr_mask;
					FILEIO* fio = new FILEIO();
					if(check_file_extension(debugger->file_path, _T(".hex"))) {
						// write intel hex format file
						if(fio->Fopen(debugger->file_path, FILEIO_WRITE_ASCII)) {
							uint32_t addr = start_addr;
							while(addr <= end_addr) {
								uint32_t len = min(end_addr - addr + 1, (uint32_t)16);
								uint32_t sum = len + ((addr >> 8) & 0xff) + (addr & 0xff) + 0x00;
								fio->Fprintf(":%02X%04X%02X", len, addr & 0xffff, 0x00);
								for(uint32_t i = 0; i < len; i++) {
									uint8_t data = cpu->read_debug_data8((addr++) & data_addr_mask);
									sum += data;
									fio->Fprintf("%02X", data);
								}
								fio->Fprintf("%02X\n", (0x100 - (sum & 0xff)) & 0xff);
							}
							fio->Fprintf(":00000001FF\n");
							fio->Fclose();
						} else {
							my_printf(p->osd, _T("can't open %s\n"), debugger->file_path);
						}
					} else {
						if(fio->Fopen(debugger->file_path, FILEIO_WRITE_BINARY)) {
							for(uint32_t addr = start_addr; addr <= end_addr; addr++) {
								fio->Fputc(cpu->read_debug_data8(addr & data_addr_mask));
							}
							fio->Fclose();
						} else {
							my_printf(p->osd, _T("can't open %s\n"), debugger->file_path);
						}
					}
					delete fio;
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T( "BP")) == 0 || _tcsicmp(params[0], _T("RBP")) == 0 || _tcsicmp(params[0], _T("WBP")) == 0) {
				break_point_t *bp = get_break_point(debugger, params[0]);
				if(num == 2) {
					uint32_t addr = my_hexatoi(params[1]);
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
						my_printf(p->osd, _T("too many break points\n"));
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("IBP")) == 0 || _tcsicmp(params[0], _T("OBP")) == 0) {
				break_point_t *bp = get_break_point(debugger, params[0]);
				if(num == 2 || num == 3) {
					uint32_t addr = my_hexatoi(params[1]), mask = 0xff;
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
						my_printf(p->osd, _T("too many break points\n"));
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("BC")) == 0 || _tcsicmp(params[0], _T("RBC")) == 0 || _tcsicmp(params[0], _T("WBC")) == 0 || _tcsicmp(params[0], _T("IBC")) == 0 || _tcsicmp(params[0], _T("OBC")) == 0) {
				break_point_t *bp = get_break_point(debugger, params[0]);
				if(num == 2 && (_tcsicmp(params[1], _T("*")) == 0 || _tcsicmp(params[1], _T("ALL")) == 0)) {
					memset(bp->table, 0, sizeof(bp->table));
				} else if(num >= 2) {
					for(int i = 1; i < num; i++) {
						int index = my_hexatoi(params[i]);
						if(!(index >= 1 && index <= MAX_BREAK_POINTS)) {
							my_printf(p->osd, _T("invalid index %x\n"), index);
						} else {
							bp->table[index - 1].addr = bp->table[index - 1].mask = 0;
							bp->table[index - 1].status = 0;
						}
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("BD")) == 0 || _tcsicmp(params[0], _T("RBD")) == 0 || _tcsicmp(params[0], _T("WBD")) == 0 || _tcsicmp(params[0], _T("IBD")) == 0 || _tcsicmp(params[0], _T("OBD")) == 0 ||
			          _tcsicmp(params[0], _T("BE")) == 0 || _tcsicmp(params[0], _T("RBE")) == 0 || _tcsicmp(params[0], _T("WBE")) == 0 || _tcsicmp(params[0], _T("IBE")) == 0 || _tcsicmp(params[0], _T("OBE")) == 0) {
				break_point_t *bp = get_break_point(debugger, params[0]);
				bool enabled = (params[0][1] == _T('E') || params[0][1] == _T('e') || params[0][2] == _T('E') || params[0][2] == _T('e'));
				if(num == 2 && (_tcsicmp(params[1], _T("*")) == 0 || _tcsicmp(params[1], _T("ALL")) == 0)) {
					for(int i = 0; i < MAX_BREAK_POINTS; i++) {
						if(bp->table[i].status != 0) {
							bp->table[i].status = enabled ? 1 : -1;
						}
					}
				} else if(num >= 2) {
					for(int i = 1; i < num; i++) {
						int index = my_hexatoi(params[i]);
						if(!(index >= 1 && index <= MAX_BREAK_POINTS)) {
							my_printf(p->osd, _T("invalid index %x\n"), index);
						} else if(bp->table[index - 1].status == 0) {
							my_printf(p->osd, _T("break point %x is null\n"), index);
						} else {
							bp->table[index - 1].status = enabled ? 1 : -1;
						}
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("BL")) == 0 || _tcsicmp(params[0], _T("RBL")) == 0 || _tcsicmp(params[0], _T("WBL")) == 0) {
				if(num == 1) {
					break_point_t *bp = get_break_point(debugger, params[0]);
					for(int i = 0; i < MAX_BREAK_POINTS; i++) {
						if(bp->table[i].status) {
							my_printf(p->osd, _T("%d %c %08X\n"), i + 1, bp->table[i].status == 1 ? _T('e') : _T('d'), bp->table[i].addr);
						}
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("IBL")) == 0 || _tcsicmp(params[0], _T("OBL")) == 0) {
				if(num == 1) {
					break_point_t *bp = get_break_point(debugger, params[0]);
					for(int i = 0; i < MAX_BREAK_POINTS; i++) {
						if(bp->table[i].status) {
							my_printf(p->osd, _T("%d %c %08X %08X\n"), i + 1, bp->table[i].status == 1 ? _T('e') : _T('d'), bp->table[i].addr, bp->table[i].mask);
						}
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
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
#if defined(_MSC_VER)					   
					while(!p->request_terminate && !debugger->now_suspended) {
						if(p->osd->is_console_key_pressed(VK_ESCAPE) && p->osd->is_console_active()) {
							break;
						}
						p->osd->sleep(10);
					}
#elif defined(OSD_QT)
					while(!p->request_terminate && !debugger->now_suspended) {
						if(p->osd->console_input_string() != NULL && p->osd->is_console_active()) {
							p->osd->clear_console_input_string();
							break;
						}
						p->osd->sleep(10);
					}
#endif					   
					// break cpu
					debugger->now_going = false;
					while(!p->request_terminate && !debugger->now_suspended) {
						p->osd->sleep(10);
					}
					dasm_addr = cpu->get_next_pc();
					
					p->osd->set_console_text_attribute(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					cpu->debug_dasm(cpu->get_pc(), buffer, 1024);
					my_printf(p->osd, _T("done\t%08X  %s\n"), cpu->get_pc(), buffer);
					
					p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					cpu->debug_regs_info(buffer, 1024);
					my_printf(p->osd, _T("%s\n"), buffer);
					
					if(debugger->hit()) {
						p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_INTENSITY);
						if(debugger->bp.hit) {
							my_printf(p->osd, _T("breaked at %08X\n"), cpu->get_next_pc());
						} else if(debugger->rbp.hit) {
							my_printf(p->osd, _T("breaked at %08X: memory %08X was read at %08X\n"), cpu->get_next_pc(), debugger->rbp.hit_addr, cpu->get_pc());
						} else if(debugger->wbp.hit) {
							my_printf(p->osd, _T("breaked at %08X: memory %08X was written at %08X\n"), cpu->get_next_pc(), debugger->wbp.hit_addr, cpu->get_pc());
						} else if(debugger->ibp.hit) {
							my_printf(p->osd, _T("breaked at %08X: port %08X was read at %08X\n"), cpu->get_next_pc(), debugger->ibp.hit_addr, cpu->get_pc());
						} else if(debugger->obp.hit) {
							my_printf(p->osd, _T("breaked at %08X: port %08X was written at %08X\n"), cpu->get_next_pc(), debugger->obp.hit_addr, cpu->get_pc());
						}
						debugger->bp.hit = debugger->rbp.hit = debugger->wbp.hit = debugger->ibp.hit = debugger->obp.hit = false;
					} else {
						p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_INTENSITY);
						my_printf(p->osd, _T("breaked at %08X: esc key was pressed\n"), cpu->get_next_pc());
					}
					if(num >= 2) {
						debugger->restore_break_points();
					}
					p->osd->set_console_text_attribute(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					cpu->debug_dasm(cpu->get_next_pc(), buffer, 1024);
					my_printf(p->osd, _T("next\t%08X  %s\n"), cpu->get_next_pc(), buffer);
					p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
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
							p->osd->sleep(10);
						}
						dasm_addr = cpu->get_next_pc();
						
						p->osd->set_console_text_attribute(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
						cpu->debug_dasm(cpu->get_pc(), buffer, 1024);
						my_printf(p->osd, _T("done\t%08X  %s\n"), cpu->get_pc(), buffer);
						
						p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
						cpu->debug_regs_info(buffer, 1024);
						my_printf(p->osd, _T("%s\n"), buffer);
						if(debugger->hit() || (p->osd->is_console_key_pressed(VK_ESCAPE) && p->osd->is_console_active())) {
							//p->osd->clear_console_input_string();
							break;
						}
					}
					if(debugger->hit()) {
						p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_INTENSITY);
						if(debugger->bp.hit) {
							my_printf(p->osd, _T("breaked at %08X\n"), cpu->get_next_pc());
						} else if(debugger->rbp.hit) {
							my_printf(p->osd, _T("breaked at %08X: memory %08X was read at %08X\n"), cpu->get_next_pc(), debugger->rbp.hit_addr, cpu->get_pc());
						} else if(debugger->wbp.hit) {
							my_printf(p->osd, _T("breaked at %08X: memory %08X was written at %08X\n"), cpu->get_next_pc(), debugger->wbp.hit_addr, cpu->get_pc());
						} else if(debugger->ibp.hit) {
							my_printf(p->osd, _T("breaked at %08X: port %08X was read at %08X\n"), cpu->get_next_pc(), debugger->ibp.hit_addr, cpu->get_pc());
						} else if(debugger->obp.hit) {
							my_printf(p->osd, _T("breaked at %08X: port %08X was written at %08X\n"), cpu->get_next_pc(), debugger->obp.hit_addr, cpu->get_pc());
						}
						debugger->bp.hit = debugger->rbp.hit = debugger->wbp.hit = debugger->ibp.hit = debugger->obp.hit = false;
					}
					p->osd->set_console_text_attribute(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					cpu->debug_dasm(cpu->get_next_pc(), buffer, 1024);
					my_printf(p->osd, _T("next\t%08X  %s\n"), cpu->get_next_pc(), buffer);
					p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("Q")) == 0) {
				p->osd->close_debugger_console();
				return -1;
			} else if(_tcsicmp(params[0], _T(">")) == 0) {
				if(num == 2) {
					if(logfile != NULL) {
						if(logfile->IsOpened()) {
							logfile->Fclose();
						}
						delete logfile;
						logfile = NULL;
					}
					logfile = new FILEIO();
					logfile->Fopen(params[1], FILEIO_WRITE_ASCII);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("!")) == 0) {
				if(num == 1) {
					my_printf(p->osd, _T("invalid parameter number\n"));
				} else if(_tcsicmp(params[1], _T("RESET")) == 0) {
					if(num == 2) {
						p->vm->reset();
					} else if(num == 3) {
						if(_tcsicmp(params[2], _T("ALL")) == 0) {
							p->vm->reset();
						} if(_tcsicmp(params[2], _T("CPU")) == 0) {
							cpu->reset();
						} else {
							my_printf(p->osd, _T("unknown device %s\n"), params[2]);
						}
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				} else if(_tcsicmp(params[1], _T("KEY")) == 0) {
					if(num == 3 || num == 4) {
						int code =  my_hexatoi(params[2]) & 0xff, msec = 100;
						if(num == 4) {
							msec = my_hexatoi(params[3]);
						}
#ifdef SUPPORT_VARIABLE_TIMING
						p->osd->modify_key_buffer(code, max((int)(p->vm->get_frame_rate() * (double)msec / 1000.0 + 0.5), 1));
#else
						p->osd->modify_key_buffer(code, max((int)(FRAMES_PER_SEC * (double)msec / 1000.0 + 0.5), 1));
#endif
#ifdef NOTIFY_KEY_DOWN
						p->vm->key_down(code, false);
#endif
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				} else {
					my_printf(p->osd, _T("unknown command ! %s\n"), params[1]);
				}
			} else if(_tcsicmp(params[0], _T("?")) == 0) {
				my_printf(p->osd, _T("D [<range>] - dump memory\n"));
				my_printf(p->osd, _T("E[{B,W,D}] <address> <list> - edit memory (byte,word,dword)\n"));
				my_printf(p->osd, _T("EA <address> \"<value>\" - edit memory (ascii)\n"));
				my_printf(p->osd, _T("I[{B,W,D}] <port> - input port (byte,word,dword)\n"));
				my_printf(p->osd, _T("O[{B,W,D}] <port> <value> - output port (byte,word,dword)\n"));
				my_printf(p->osd, _T("R - show register(s)\n"));
				my_printf(p->osd, _T("R <reg> <value> - edit register\n"));
				my_printf(p->osd, _T("S <range> <list> - search\n"));
				my_printf(p->osd, _T("U [<range>] - unassemble\n"));
				
				my_printf(p->osd, _T("H <value> <value> - hexadd\n"));
				my_printf(p->osd, _T("N <filename> - name\n"));
				my_printf(p->osd, _T("L [<range>] - load file\n"));
				my_printf(p->osd, _T("W <range> - write file\n"));
				
				my_printf(p->osd, _T("BP <address> - set breakpoint\n"));
				my_printf(p->osd, _T("{R,W}BP <address> - set breakpoint (break at memory access)\n"));
				my_printf(p->osd, _T("{I,O}BP <port> [<mask>] - set breakpoint (break at i/o access)\n"));
				my_printf(p->osd, _T("[{R,W,I,O}]B{C,D,E} {*,<list>} - clear/disable/enable breakpoint(s)\n"));
				my_printf(p->osd, _T("[{R,W,I,O}]BL - list breakpoint(s)\n"));
				
				my_printf(p->osd, _T("G - go (press esc key to break)\n"));
				my_printf(p->osd, _T("G <address> - go and break at address\n"));
				my_printf(p->osd, _T("T [<count>] - trace\n"));
				my_printf(p->osd, _T("Q - quit\n"));
				
				my_printf(p->osd, _T("> <filename> - output logfile\n"));
				
				my_printf(p->osd, _T("! reset [cpu] - reset\n"));
				my_printf(p->osd, _T("! key <code> [<msec>] - press key\n"));
				
				my_printf(p->osd, _T("<value> - hexa, decimal(%%d), ascii('a')\n"));
			} else {
				my_printf(p->osd, _T("unknown command %s\n"), params[0]);
			}
		}
	return 0;
}
   
#ifdef _MSC_VER
unsigned __stdcall debugger_thread(void *lpx)
#else
int debugger_thread(void *lpx)
#endif
{
	volatile debugger_thread_t *p = (debugger_thread_t *)lpx;
	p->running = true;
	
	DEVICE *cpu = p->vm->get_cpu(p->cpu_index);
	DEBUGGER *debugger = (DEBUGGER *)cpu->get_debugger();
	
	debugger->now_going = false;
	debugger->now_debugging = true;
	while(!debugger->now_suspended) {
		p->osd->sleep(10);
	}
	
	uint32_t prog_addr_mask = cpu->debug_prog_addr_mask();
	uint32_t data_addr_mask = cpu->debug_data_addr_mask();
	dump_addr = 0;
	dasm_addr = cpu->get_next_pc();
	
	// initialize console
	_TCHAR buffer[1024];
	bool cp932 = (p->osd->get_console_code_page() == 932);
	
	p->osd->open_console(create_string(_T("Debugger - %s"), _T(DEVICE_NAME)));
	p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	cpu->debug_regs_info(buffer, 1024);
	my_printf(p->osd, _T("%s\n"), buffer);
	
	p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_INTENSITY);
	my_printf(p->osd, _T("breaked at %08X\n"), cpu->get_next_pc());
	
	p->osd->set_console_text_attribute(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	cpu->debug_dasm(cpu->get_next_pc(), buffer, 1024);
	my_printf(p->osd, _T("next\t%08X  %s\n"), cpu->get_next_pc(), buffer);
	p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	
	// initialize logfile
	logfile = NULL;
	
	#define MAX_COMMAND_LEN	64
	
	_TCHAR command[MAX_COMMAND_LEN + 1];
	_TCHAR prev_command[MAX_COMMAND_LEN + 1];
	
	memset(prev_command, 0, sizeof(prev_command));
	while(!p->request_terminate) {
		my_printf(p->osd, _T("- "));
		
		// get command
		int ptr = 0;
		bool enter_done = false;
		
		while(!p->request_terminate && !enter_done) {
			_TCHAR ir[16];
			int count = p->osd->read_console_input(ir);

			for(int i = 0; i < count; i++) {
				_TCHAR chr = ir[i];
				
				if(chr == '\n' || chr == '\r') {
					if(ptr == 0 && prev_command[0] != _T('\0')) {
						memcpy(command, prev_command, sizeof(command));
						my_printf(p->osd, _T("%s\n"), command);
						enter_done = true;
						break;
					} else if(ptr > 0) {
						command[ptr] = _T('\0');
						memcpy(prev_command, command, sizeof(command));
						my_printf(p->osd, _T("\n"));
						enter_done = true;
						break;
					}
				} else if(chr == 0x08) {
					if(ptr > 0) {
						ptr--;
						my_putch(p->osd, chr);
						my_putch(p->osd, _T(' '));
						my_putch(p->osd, chr);
					}
				} else if(chr >= 0x20 && chr <= 0x7e && ptr < MAX_COMMAND_LEN && !(chr == 0x20 && ptr == 0)) {
					command[ptr++] = chr;
					my_putch(p->osd, chr);
				}
			}
			p->osd->sleep(10);
		}
		// process command
		if(!p->request_terminate && enter_done) {
			if(debugger_command(p, command, prev_command, cp932) < 0) break;
		}
	}
   
	// stop debugger
	try {
		debugger->now_debugging = debugger->now_going = debugger->now_suspended = false;
	} catch(...) {
	}
	
	// release logfile
	if(logfile != NULL) {
		if(logfile->IsOpened()) {
			logfile->Fclose();
		}
		delete logfile;
		logfile = NULL;
	}
	
	// release console
	p->osd->close_console();
	
	p->running = false;
#ifdef _MSC_VER
	_endthreadex(0);
	return 0;
#else
	return 0;
#endif
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
//			debugger_thread_param.emu = this;
			debugger_thread_param.osd = osd;
			debugger_thread_param.vm = vm;
			debugger_thread_param.cpu_index = cpu_index;
			debugger_thread_param.request_terminate = false;
#ifdef _MSC_VER
			if((hDebuggerThread = (HANDLE)_beginthreadex(NULL, 0, debugger_thread, &debugger_thread_param, 0, NULL)) != (HANDLE)0) {
#elif !defined(_USE_QT)
			if((debugger_thread_id = SDL_CreateThread(debugger_thread, "DebuggerThread", (void *)&debugger_thread_param)) != 0) {
#else // USE_QT
				{
					volatile debugger_thread_t *p = (debugger_thread_t *)(&debugger_thread_param);
					p->running = true;
					
					DEVICE *cpu = p->vm->get_cpu(p->cpu_index);
					DEBUGGER *debugger = (DEBUGGER *)cpu->get_debugger();
					
					debugger->now_going = false;
					debugger->now_debugging = true;
					//while(!debugger->now_suspended) {
					//	p->osd->sleep(10);
					//}
					
					uint32_t prog_addr_mask = cpu->debug_prog_addr_mask();
					uint32_t data_addr_mask = cpu->debug_data_addr_mask();
					uint32_t dump_addr = 0;
					uint32_t dasm_addr = cpu->get_next_pc();
	
					// initialize console
					_TCHAR buffer[1024];
					my_stprintf_s(buffer, 1024, _T("Debugger - %s"), _T(DEVICE_NAME));
					
					p->osd->open_console(buffer);
					
					bool cp932 = (p->osd->get_console_code_page() == 932);
					
					p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					cpu->debug_regs_info(buffer, 1024);
					my_printf(p->osd, _T("%s\n"), buffer);
					
					p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_INTENSITY);
					my_printf(p->osd, _T("breaked at %08X\n"), cpu->get_next_pc());
					
					p->osd->set_console_text_attribute(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					cpu->debug_dasm(cpu->get_next_pc(), buffer, 1024);
					my_printf(p->osd, _T("next\t%08X  %s\n"), cpu->get_next_pc(), buffer);
					p->osd->set_console_text_attribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	
					// initialize logfile
					if(logfile != NULL && logfile->IsOpened()) {
						logfile->Fclose();
					}
					logfile = NULL;
#endif
					stop_record_sound();
					stop_record_video();
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
		}
#ifdef _MSC_VER
		WaitForSingleObject(hDebuggerThread, INFINITE);
		CloseHandle(hDebuggerThread);
#elif !defined(_USE_QT)
		//pthread_join(debugger_thread_id, NULL);
		SDL_DetachThread(debugger_thread_id);
#else
		volatile debugger_thread_t *p = (debugger_thread_t *)(&debugger_thread_param);
		p->running = false;
		
		if(logfile != NULL && logfile->IsOpened()) {
			logfile->Fclose();
		}
		// initialize logfile
		logfile = NULL;
		
#endif
		now_debugging = false;
	}
}

bool EMU::is_debugger_enabled(int cpu_index)
{
	return (vm->get_cpu(cpu_index) != NULL && vm->get_cpu(cpu_index)->get_debugger() != NULL);
}

#endif

