/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.04.09 -
	History: 09 Apr, 2015 : Initial from Takeda.Toshiya's w32_debugger.cpp.
	[ debugger console ]
*/

#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <QObject>

//#include "res/resource.h"
#include "../../emu.h"
#include "../../vm/device.h"
#include "../../vm/debugger.h"
#include "../../vm/vm.h"
#include "../../fileio.h"
#include "qt_debugger.h"
//#include <QThread>
//#include <QMainWindow>


#ifdef USE_DEBUGGER

void CSP_Debugger::put_string(QString s)
{
	//text->appendPlainText(s);
	text->insertPlainText(s);
}

void CSP_Debugger::my_printf(const _TCHAR *format, ...)
{
	DWORD dwWritten;
	_TCHAR buffer[4096];
	va_list ap;

	memset(buffer, 0x00, sizeof(buffer));
	va_start(ap, format);
	_vstprintf_s(buffer, 4096, format, ap);
	va_end(ap);
	QString str(buffer);
//	str.fromUtf8(buffer);
	//fputs(buffer, hStdOut);
	//text->append(str);
	emit sig_put_string(str);
}

void CSP_Debugger::my_putch(_TCHAR c)
{
	char buffer[2];
	buffer[0] = (char)c;
	buffer[1] = 0x00;
	QString str(buffer);
	emit sig_put_string(str);
	//text->append(str);
}

uint32 CSP_Debugger::my_hexatoi(_TCHAR *str)
{
	_TCHAR *s;
	
	if(str == NULL || strlen(str) == 0) {
		return 0;
	} else if(strlen(str) == 3 && str[0] == _T('\'') && str[2] == _T('\'')) {
		// ank
		return str[1] & 0xff;
	} else if((s = strstr(str, _T(":"))) != NULL) {
		// 0000:0000
		s[0] = _T('\0');
		return (my_hexatoi(str) << 4) + my_hexatoi(s + 1);
	} else if(str[0] == _T('%')) {
		// decimal
		return atoi(str + 1);
	}
	return strtol(str, NULL, 16);
}

break_point_t *CSP_Debugger::get_break_point(DEBUGGER *debugger, _TCHAR *command)
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


void CSP_Debugger::Sleep(uint32_t tick) 
{
	QThread::msleep(tick);
}

void CSP_Debugger::call_debugger(void)
{
	QString str;
	str = text_command->text();
	this->debugger_main(str);
}

void CSP_Debugger::check_trap(void)
{
	volatile debugger_thread_t *p = &debugger_thread_param;
	DEVICE *cpu = p->vm->get_cpu(p->cpu_index);
	DEBUGGER *debugger = (DEBUGGER *)cpu->get_debugger();
	char buffer[1024];

	if(trace_steps < 0) {
		if(!debugger->now_going) return;
		if(!p->request_terminate && !debugger->now_suspended) {
			return;
		}
		debugger->now_going = false;
		debugger->now_suspended = true;
	}

	memset(buffer, 0x00, sizeof(buffer));
	// break cpu
	dasm_addr = cpu->get_next_pc();
					
	cpu->debug_regs_info(buffer, 1024);
	my_printf( _T("%s\n"), buffer);
	cpu->debug_dasm(cpu->get_next_pc(), buffer, 1024);
	my_printf( _T("next\t%08X  %s\n"), cpu->get_next_pc(), buffer);
	
	if(debugger->hit()) {
		if(debugger->bp.hit) {
			my_printf( _T("breaked at %08X\n"), cpu->get_next_pc());
		} else if(debugger->rbp.hit) {
			my_printf( _T("breaked at %08X: memory %08X was read at %08X\n"), cpu->get_next_pc(), debugger->rbp.hit_addr, cpu->get_pc());
		} else if(debugger->wbp.hit) {
			my_printf( _T("breaked at %08X: memory %08X was written at %08X\n"), cpu->get_next_pc(), debugger->wbp.hit_addr, cpu->get_pc());
		} else if(debugger->ibp.hit) {
			my_printf( _T("breaked at %08X: port %08X was read at %08X\n"), cpu->get_next_pc(), debugger->ibp.hit_addr, cpu->get_pc());
		} else if(debugger->obp.hit) {
			my_printf( _T("breaked at %08X: port %08X was written at %08X\n"), cpu->get_next_pc(), debugger->obp.hit_addr, cpu->get_pc());
		}
		debugger->bp.hit = debugger->rbp.hit = debugger->wbp.hit = debugger->ibp.hit = debugger->obp.hit = false;
	} else if(trace_steps < 0) {
		 my_printf( _T("breaked at %08X: Any key was pressed\n"), cpu->get_next_pc());
	} else if(trace_steps == 0) {
		my_printf("Trace OK\n");
		debugger->now_going = false;
		debugger->now_suspended = true;
	}
	text->moveCursor(QTextCursor::End);
	if(trace_steps > 0) {
		trace_steps--;
		debugger->restore_break_points();
		return;
	}
	//if(num >= 2) {
	debugger->restore_break_points();
	//}
	emit sig_end_trap();
}

int CSP_Debugger::debugger_main(QString command)
{
	
	volatile debugger_thread_t *p = &debugger_thread_param;
	p->running = true;
	
	DEVICE *cpu = p->vm->get_cpu(p->cpu_index);
	DEBUGGER *debugger = (DEBUGGER *)cpu->get_debugger();
	if(debugger->now_going) {
		debugger->now_going = false;
		debugger->now_suspended = true;
		return;
	}
	
	//debugger->now_going = false;
	debugger->now_debugging = true;
	
	uint32 prog_addr_mask = cpu->debug_prog_addr_mask();
	uint32 data_addr_mask = cpu->debug_data_addr_mask();
	
	// initialize console
	_TCHAR buffer[1024];
	snprintf(buffer, 1024, _T("Debugger - %s"), _T(DEVICE_NAME));
	//AllocConsole();
	//SetConsoleTitle(buffer);
	
	bool cp932 = false; //(GetConsoleCP() == 932);

	//while(!p->request_terminate) {
	//my_printf( _T("- "));
	text->append("");
	text->moveCursor(QTextCursor::End);
	// get command
	int ptr = 0;
	running = true;
	
	// process command
	if(!p->request_terminate) {
		QStringList params;
		int num;
		QString cmd;
		params = command.split(" ");
		num = params.count();
		cmd = params.first().toUpper();
		
		if(cmd == QString::fromUtf8("D")) {
			debugger->now_going = false;
			if(num <= 3) {
				uint32 start_addr = dump_addr;
				if(num >= 2) {
					char *arg_1 = params.value(1).toUtf8().data();
					start_addr = my_hexatoi(arg_1);
				}
				start_addr &= data_addr_mask;
				
				uint32 end_addr = start_addr + 8 * 16 - 1;
				if(num == 3) {
					char *arg_2 = params.value(2).toUtf8().data();
					end_addr = my_hexatoi(arg_2);
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
						my_printf( _T("%08X "), addr & data_addr_mask);
						memset(buffer, 0, sizeof(buffer));
					}
					if(addr < start_addr) {
						my_printf( _T("   "));
						buffer[addr & 0x0f] = _T(' ');
					} else {
						uint32 data = cpu->debug_read_data8(addr & data_addr_mask);
						my_printf( _T(" %02X"), data);
						buffer[addr & 0x0f] = ((data >= 0x20 && data <= 0x7e) || (cp932 && data >= 0xa1 && data <= 0xdf)) ? data : _T('.');
					}
					if((addr & 0x0f) == 0x0f) {
						my_printf( _T("  %s\n"), buffer);
					}
				}
				if((end_addr & 0x0f) != 0x0f) {
					for(uint32 addr = (end_addr & 0x0f) + 1; addr <= 0x0f; addr++) {
						my_printf( _T("   "));
					}
					my_printf( _T("  %s\n"), buffer);
				}
				dump_addr = (end_addr + 1) & data_addr_mask;
				prev_command.clear(); // remove parameters to dump continuously
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("E") || cmd == QString::fromUtf8("EB")) {
			debugger->now_going = false;
			if(num >= 3) {
				char *arg_1 = params.value(1).toUtf8().data();
				uint32 addr = my_hexatoi(arg_1) & data_addr_mask;
				for(int i = 2; i < num; i++) {
					char *arg_n = params.value(i).toUtf8().data();
					cpu->debug_write_data8(addr, my_hexatoi(arg_n) & 0xff);
					addr = (addr + 1) & data_addr_mask;
				}
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("EW")) {
			debugger->now_going = false;
			if(num >= 3) {
				char *arg_1 = params.value(1).toUtf8().data();
				uint32 addr = my_hexatoi(arg_1) & data_addr_mask;
				for(int i = 2; i < num; i++) {
					char *arg_n = params.value(i).toUtf8().data();
					cpu->debug_write_data16(addr, my_hexatoi(arg_n) & 0xffff);
					addr = (addr + 2) & data_addr_mask;
				}
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("ED")) {
			debugger->now_going = false;
			if(num >= 3) {
				char *arg_1 = params.value(1).toUtf8().data();
				uint32 addr = my_hexatoi(arg_1) & data_addr_mask;
				for(int i = 2; i < num; i++) {
					char *arg_n = params.value(i).toUtf8().data();
					cpu->debug_write_data32(addr, my_hexatoi(arg_n));
					addr = (addr + 4) & data_addr_mask;
				}
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("EA")) {
			debugger->now_going = false;
			if(num >= 3) {
				char *arg_1 = params.value(1).toUtf8().data();
				uint32 addr = my_hexatoi(arg_1) & data_addr_mask;
				QString sbuffer;
				QStringList slist;
				sbuffer = prev_command;
				slist = sbuffer.split("\"");
				if(!slist.isEmpty()) {
					char *token = slist.first().toUtf8().data();
					if(token == NULL) {
						running = false;
						text_command->clear();
						text->moveCursor(QTextCursor::End);
						return 0;
					}
					int len = strlen(token);
					for(int i = 0; i < len; i++) {
						cpu->debug_write_data8(addr, token[i] & 0xff);
						addr = (addr + 1) & data_addr_mask;
					}
				} else {
					my_printf( _T("invalid parameter\n"));
				}
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("I") || cmd == QString::fromUtf8("IB")) {
			debugger->now_going = false;
			if(num == 2) {
				char *arg_1 = params.value(1).toUtf8().data();
				my_printf( _T("%02X\n"), cpu->debug_read_io8(my_hexatoi(arg_1) & 0xff));
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("IW")) {
			debugger->now_going = false;
			if(num >= 2) {
				char *arg_1 = params.value(1).toUtf8().data();
				my_printf( _T("%02X\n"), cpu->debug_read_io16(my_hexatoi(arg_1)) & 0xffff);
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("ID")) {
			debugger->now_going = false;
			if(num >= 2) {
				char *arg_1 = params.value(1).toUtf8().data();
				my_printf( _T("%02X\n"), cpu->debug_read_io32(my_hexatoi(arg_1)));
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("O") || cmd == QString::fromUtf8("OB")) {
			debugger->now_going = false;
			if(num == 3) {
				char *arg_1 = params.value(1).toUtf8().data();
				char *arg_2 = params.value(2).toUtf8().data();
				cpu->debug_write_io8(my_hexatoi(arg_1), my_hexatoi(arg_2) & 0xff);
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("OW")) {
			debugger->now_going = false;
			if(num == 3) {
				char *arg_1 = params.value(1).toUtf8().data();
				char *arg_2 = params.value(2).toUtf8().data();
				cpu->debug_write_io16(my_hexatoi(arg_1), my_hexatoi(arg_2) & 0xffff);
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("OD")) {
			debugger->now_going = false;
			if(num == 3) {
				char *arg_1 = params.value(1).toUtf8().data();
				char *arg_2 = params.value(2).toUtf8().data();
				cpu->debug_write_io32(my_hexatoi(arg_1), my_hexatoi(arg_2) & 0xffff);
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("R")) {
			debugger->now_going = false;
			my_printf( _T("CPU DOMAIN=%d\n"), debugger_thread_param.cpu_index);
			if(num == 1) {
				cpu->debug_regs_info(buffer, 1024);
				my_printf( _T("%s\n"), buffer);
			} else if(num == 3) {
				char *arg_1 = params.value(1).toUtf8().data();
				char *arg_2 = params.value(2).toUtf8().data();
				if(!cpu->debug_write_reg(arg_1, my_hexatoi(arg_2))) {
					my_printf( _T("unknown register %s\n"), arg_1);
				}
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("S")) {
			debugger->now_going = false;
			if(num >= 4) {
				char *arg_1 = params.value(1).toUtf8().data();
				char *arg_2 = params.value(2).toUtf8().data();
				uint32 start_addr = my_hexatoi(arg_1) & data_addr_mask;
				uint32 end_addr = my_hexatoi(arg_2) & data_addr_mask;
				uint8 list[32];
				char *arg_n = NULL;
				for(int i = 3, j = 0; i < num; i++, j++) {
					arg_n = params.value(i).toUtf8().data();
					if(arg_n == NULL) break;
					list[j] = my_hexatoi(arg_n);
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
						my_printf( _T("%08X\n"), addr);
					}
				}
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("UW")) {
			debugger->now_going = false;
			if((num <= 4) && (num >= 2)) {
				uint32 start_a, end_a;
				int filename_num = 1;
				start_a = dasm_addr;
				if(num >= 3) {
					char *arg_1 = params.value(1).toUtf8().data();
					start_a = my_hexatoi(arg_1) & prog_addr_mask;
					filename_num = 2;
				}
				
				if(num == 4) {
						char *arg_2 = params.value(2).toUtf8().data();
						end_a = my_hexatoi(arg_2) & prog_addr_mask;
						filename_num = 3;
					} else {
						end_a = start_a + 0x100;
					}
					char *arg_name = params.value(filename_num).toUtf8().data();

					if(arg_name[0] == _T('\"')) {
						QString sbuffer;
						QStringList slist;

						sbuffer = prev_command;
						slist = sbuffer.split("\"");
						if(!slist.isEmpty()) {
							strncpy(debugger->text_path, slist.first().toUtf8().data(), _MAX_PATH);
						} else {
							my_printf( _T("invalid parameter\n"));
							filename_num = -1;
						}
					}
					if(end_a < start_a) {
						uint32 tmp_a;
						tmp_a = start_a;
						start_a = end_a;
						end_a = tmp_a;
					}
				   	if(filename_num >= 1) {
						FILEIO* fio = new FILEIO();
						_TCHAR dasm_str_buffer[1024];
						_TCHAR stream_buffer[1024];
						int addrcount = (int)(end_a - start_a);
						if(fio->Fopen(debugger->text_path, FILEIO_WRITE_ASCII)) {
							for(dasm_addr = start_a; addrcount > 0;) {
								memset(dasm_str_buffer, 0x00, sizeof(dasm_str_buffer));
								memset(stream_buffer, 0x00, sizeof(stream_buffer));
								int len = cpu->debug_dasm(dasm_addr, dasm_str_buffer, 1024);
								if(len > 0) {
									snprintf(stream_buffer, 1024, _T("%08X  %s\n"), dasm_addr, dasm_str_buffer);
									fio->Fwrite(stream_buffer, strlen(stream_buffer), 1); 
									dasm_addr = (dasm_addr + len) & prog_addr_mask;
									addrcount -= len;
								} else {
									break;
								}
							}
							fio->Fclose();
							delete fio;
						}
					}
				   
					prev_command.clear(); // remove parameters to disassemble continuously
			} else {
				my_printf( _T("invalid parameter number\n"));
			}
		} else if(cmd == QString::fromUtf8("U")) {
			debugger->now_going = false;
				if(num <= 3) {
					if(num >= 2) {
						char *arg_1 = params.value(1).toUtf8().data();
						dasm_addr = my_hexatoi(arg_1) & prog_addr_mask;
					}
					if(num == 3) {
						char *arg_2 = params.value(2).toUtf8().data();
						uint32 end_addr = my_hexatoi(arg_2) & prog_addr_mask;
						while(dasm_addr <= end_addr) {
							int len = cpu->debug_dasm(dasm_addr, buffer, 1024);
							my_printf( _T("%08X  %s\n"), dasm_addr, buffer);
							dasm_addr = (dasm_addr + len) & prog_addr_mask;
						}
					} else {
						for(int i = 0; i < 16; i++) {
							int len = cpu->debug_dasm(dasm_addr, buffer, 1024);
							my_printf( _T("%08X  %s\n"), dasm_addr, buffer);
							dasm_addr = (dasm_addr + len) & prog_addr_mask;
						}
					}
					prev_command.clear(); // remove parameters to disassemble continuously
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
		} else if(cmd == QString::fromUtf8("H")) {
			debugger->now_going = false;
				if(num == 3) {
					char *arg_1 = params.value(1).toUtf8().data();
					char *arg_2 = params.value(2).toUtf8().data();
					uint32 l = my_hexatoi(arg_1);
					uint32 r = my_hexatoi(arg_2);
					my_printf( _T("%08X  %08X\n"), l + r, l - r);
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
		} else if(cmd == QString::fromUtf8("N")) {
			debugger->now_going = false;
			char *arg_1 = params.value(1).toUtf8().data();
			if(num >= 2 && arg_1[0] == _T('\"')) {
					QString sbuffer;
					QStringList slist;
					sbuffer = prev_command;
					slist = sbuffer.split("\"");
					if(!slist.isEmpty()) {
						strncpy(debugger->file_path, slist.first().toUtf8().data(), _MAX_PATH);
					} else {
						my_printf( _T("invalid parameter\n"));
					}
				} else if(num == 2) {
					strncpy(debugger->file_path, arg_1, _MAX_PATH);
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
		} else if(cmd == QString::fromUtf8("L")) {
			debugger->now_going = false;
			if(num == 3) {
					char *arg_1 = params.value(1).toUtf8().data();
					char *arg_2 = params.value(2).toUtf8().data();
					uint32 start_addr = my_hexatoi(arg_1) & data_addr_mask, end_addr = my_hexatoi(arg_2) & data_addr_mask;
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
						my_printf( _T("can't open %s\n"), debugger->file_path);
					}
					delete fio;
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
		} else if(cmd == QString::fromUtf8("W")) {
			debugger->now_going = false;
				if(num == 3) {
					char *arg_1 = params.value(1).toUtf8().data();
					char *arg_2 = params.value(2).toUtf8().data();
					uint32 start_addr = my_hexatoi(arg_1) & data_addr_mask, end_addr = my_hexatoi(arg_2) & data_addr_mask;
					FILEIO* fio = new FILEIO();
					if(fio->Fopen(debugger->file_path, FILEIO_WRITE_BINARY)) {
						for(uint32 addr = start_addr; addr <= end_addr; addr++) {
							fio->Fputc(cpu->debug_read_data8(addr & data_addr_mask));
						}
						fio->Fclose();
					} else {
						my_printf( _T("can't open %s\n"), debugger->file_path);
					}
					delete fio;
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
		} else if(cmd == QString::fromUtf8( "BP") || cmd == QString::fromUtf8("RBP") || cmd == QString::fromUtf8("WBP")) {
			debugger->now_going = false;
				break_point_t *bp = get_break_point(debugger, cmd.toUtf8().data());
				if(num == 2) {
					char *arg_1 = params.value(1).toUtf8().data();
					uint32 addr = my_hexatoi(arg_1);
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
						my_printf( _T("too many break points\n"));
					}
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
		} else if(cmd == QString::fromUtf8("IBP") || cmd == QString::fromUtf8("OBP")) {
			debugger->now_going = false;
				break_point_t *bp = get_break_point(debugger, cmd.toUtf8().data());
				if(num == 2 || num == 3) {
					char *arg_1 = params.value(1).toUtf8().data();
					uint32 addr = my_hexatoi(arg_1), mask = 0xff;
					if(num == 3) {
						char *arg_2 = params.value(2).toUtf8().data();
						mask = my_hexatoi(arg_2);
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
						my_printf( _T("too many break points\n"));
					}
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
		} else if(cmd == QString::fromUtf8("BC") || cmd == QString::fromUtf8("RBC") || cmd == QString::fromUtf8("WBC") || cmd == QString::fromUtf8("IBC") || cmd == QString::fromUtf8("OBC")) {
			debugger->now_going = false;
				char *arg_1 = params.value(1).toUtf8().data();
				break_point_t *bp = get_break_point(debugger, cmd.toUtf8().data());
				if(num == 2 && strcasecmp(arg_1, _T("ALL"))) {
					memset(bp->table, 0, sizeof(bp->table));
				} else if(num >= 2) {
					char *arg_n;
					for(int i = 1; i < num; i++) {
						arg_n = params.value(i).toUtf8().data();
						if(arg_n == NULL) break;
						int index = my_hexatoi(arg_n);
						if(!(index >= 1 && index <= MAX_BREAK_POINTS)) {
							my_printf( _T("invalid index %x\n"), index);
						} else {
							bp->table[index - 1].addr = bp->table[index - 1].mask = 0;
							bp->table[index - 1].status = 0;
						}
					}
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
		} else if(cmd == QString::fromUtf8("BD") || cmd == QString::fromUtf8("RBD") || cmd == QString::fromUtf8("WBD") || cmd == QString::fromUtf8("IBD") || cmd == QString::fromUtf8("OBD") ||
			          cmd == QString::fromUtf8("BE") || cmd == QString::fromUtf8("RBE") || cmd == QString::fromUtf8("WBE") || cmd == QString::fromUtf8("IBE") || cmd == QString::fromUtf8("OBE")) {
			debugger->now_going = false;
						break_point_t *bp = get_break_point(debugger, cmd.toUtf8().data());
						QString m1 = cmd.mid(1, 1);
						QString m2 = cmd.mid(2, 1);
						
						bool enabled = (m1 == QString::fromUtf8("E") || m1 == QString::fromUtf8("e") ||
										m2 == QString::fromUtf8("E") || m2 == QString::fromUtf8("e"));
						char *arg_1 = params.value(1).toUtf8().data();
						if(num == 2 && strcasecmp(arg_1, _T("ALL"))) {
					for(int i = 0; i < MAX_BREAK_POINTS; i++) {
						if(bp->table[i].status != 0) {
							bp->table[i].status = enabled ? 1 : -1;
						}
					}
						} else if(num >= 2) {
							for(int i = 1; i < num; i++) {
								char *arg_n = params.value(i).toUtf8().data();
								if(arg_n == NULL) break;
								int index = my_hexatoi(arg_n);
								if(!(index >= 1 && index <= MAX_BREAK_POINTS)) {
									my_printf( _T("invalid index %x\n"), index);
								} else if(bp->table[index - 1].status == 0) {
									my_printf( _T("break point %x is null\n"), index);
								} else {
									bp->table[index - 1].status = enabled ? 1 : -1;
								}
							}
						} else {
							my_printf( _T("invalid parameter number\n"));
						}
		} else if(cmd == QString::fromUtf8("BL") || cmd == QString::fromUtf8("RBL") || cmd == QString::fromUtf8("WBL")) {
			debugger->now_going = false;
			if(num == 1) {
					break_point_t *bp = get_break_point(debugger, cmd.toUtf8().data());
					for(int i = 0; i < MAX_BREAK_POINTS; i++) {
						if(bp->table[i].status) {
							my_printf( _T("%d %c %08X\n"), i + 1, bp->table[i].status == 1 ? _T('e') : _T('d'), bp->table[i].addr);
						}
					}
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
			} else if(cmd == QString::fromUtf8("IBL") || cmd == QString::fromUtf8("OBL")) {
				if(num == 1) {
					break_point_t *bp = get_break_point(debugger, cmd.toUtf8().data());
					for(int i = 0; i < MAX_BREAK_POINTS; i++) {
						if(bp->table[i].status) {
							my_printf( _T("%d %c %08X %08X\n"), i + 1, bp->table[i].status == 1 ? _T('e') : _T('d'), bp->table[i].addr, bp->table[i].mask);
						}
					}
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
			} else if(cmd == QString::fromUtf8("G")) {
				struct termios console_saved;
				if(num == 1 || num == 2) {
					if(num >= 2) {
						char *arg_1 = params.value(1).toUtf8().data();
						debugger->store_break_points();
						debugger->bp.table[0].addr = my_hexatoi(arg_1) & prog_addr_mask;
						debugger->bp.table[0].mask = prog_addr_mask;
						debugger->bp.table[0].status = 1;
					}
					debugger->now_going = true;
					debugger->now_suspended = false;
					trace_steps = -1;
					emit sig_start_trap();
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
		} else if(cmd == QString::fromUtf8("T")) {
			if(num == 1 || num == 2) {
				int steps = 1;
				if(num >= 2) {
					char *arg_1 = params.value(1).toUtf8().data();
					steps = my_hexatoi(arg_1);
				}
				if(steps >= 1) trace_steps = steps;
					debugger->now_going = false;
					debugger->now_suspended = false;
					emit sig_start_trap();
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
		} else if(cmd == QString::fromUtf8("Q")) {
			debugger->now_going = false;
			running = false;
			p->request_terminate = true;
			text_command->clear();
			text->moveCursor(QTextCursor::End);
			emit quit_debugger_thread();
			return -1;
		} else if(cmd == QString::fromUtf8("!")) {
			char *arg_1 = params.value(1).toUtf8().data();
			char *arg_2 = params.value(2).toUtf8().data();
			debugger->now_going = false;
			if(num == 1) {
				my_printf( _T("invalid parameter number\n"));
			} else if(strcasecmp(arg_1, _T("RESET"))) {
				if(num == 2) {
					p->vm->reset();
				} else if(num == 3) {
					if(strcasecmp(arg_2, _T("ALL"))) {
						p->vm->reset();
					} if(strcasecmp(arg_2, _T("CPU"))) {
						cpu->reset();
					} else {
						my_printf( _T("unknown device %s\n"), arg_2);
					}
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
			} else if(strcasecmp(arg_1, _T("KEY"))) {
				if(num == 3 || num == 4) {
					int code =  my_hexatoi(arg_1) & 0xff, msec = 100;
					if(num == 4) {
						char *arg_3 = params.value(3).toUtf8().data();
						msec = my_hexatoi(arg_3);
					}
#ifdef SUPPORT_VARIABLE_TIMING
					p->emu->key_buffer()[code] = (int)fmax(p->vm->frame_rate() * (double)msec / 1000.0 + 0.5, 1.0);
#else
					p->emu->key_buffer()[code] = (int)fmax(FRAMES_PER_SEC * (double)msec / 1000.0 + 0.5, 1.0);
#endif
#ifdef NOTIFY_KEY_DOWN
					p->vm->key_down(code, false);
#endif
				} else {
					my_printf( _T("invalid parameter number\n"));
				}
			} else {
				my_printf( _T("unknown command ! %s\n"), arg_1);
			}
		} else if(cmd == QString::fromUtf8("?")) {
			debugger->now_going = false;
			my_printf( _T("D [<range>] - dump memory\n"));
			my_printf( _T("E[{B,W,D}] <address> <list> - edit memory (byte,word,dword)\n"));
			my_printf( _T("EA <address> \"<value>\" - edit memory (ascii)\n"));
			my_printf( _T("I[{B,W,D}] <port> - input port (byte,word,dword)\n"));
			my_printf( _T("O[{B,W,D}] <port> <value> - output port (byte,word,dword)\n"));
			my_printf( _T("R - show register(s)\n"));
			my_printf( _T("R <reg> <value> - edit register\n"));
			my_printf( _T("S <range> <list> - search\n"));
			my_printf( _T("U [<range>] - unassemble\n"));
			my_printf( _T("UW [<start>] [<end>] filename - unassemble to file\n"));
				
			my_printf( _T("H <value> <value> - hexadd\n"));
			my_printf( _T("N <filename> - name\n"));
			my_printf( _T("L <range> - load file\n"));
			my_printf( _T("W <range> - write file\n"));
				
			my_printf( _T("BP <address> - set breakpoint\n"));
			my_printf( _T("{R,W}BP <address> - set breakpoint (break at memory access)\n"));
			my_printf( _T("{I,O}BP <port> [<mask>] - set breakpoint (break at i/o access)\n"));
			my_printf( _T("[{R,W,I,O}]B{C,D,E} {all,<list>} - clear/disable/enable breakpoint(s)\n"));
			my_printf( _T("[{R,W,I,O}]BL - list breakpoint(s)\n"));
				
			my_printf( _T("G - go (press esc key to break)\n"));
			my_printf( _T("G <address> - go and break at address\n"));
			my_printf( _T("T [<count>] - trace\n"));
			my_printf( _T("Q - quit\n"));
				
			my_printf( _T("! reset [cpu] - reset\n"));
			my_printf( _T("! key <code> [<msec>] - press key\n"));
			
			my_printf( _T("<value> - hexa, decimal(%%d), ascii('a')\n"));
		} else {
			my_printf( _T("unknown command %s\n"), cmd.toUtf8().data());
		}
	}
	running = false;
	text_command->clear();
	text->moveCursor(QTextCursor::End);
				
	return 0;
}


void CSP_Debugger::doWork(const QString &params)
{
	DEVICE *cpu = debugger_thread_param.vm->get_cpu(debugger_thread_param.cpu_index);
	DEBUGGER *debugger = (DEBUGGER *)cpu->get_debugger();
	_TCHAR buffer[1024];
	
	my_printf(_T("CPU DOMAIN=%d\n"), debugger_thread_param.cpu_index);
	
	cpu->debug_regs_info(buffer, 1024);
	my_printf(_T("%s\n"), buffer);
	
	my_printf(_T("breaked at %08X\n"), cpu->get_next_pc());
	cpu->debug_dasm(cpu->get_next_pc(), buffer, 1024);
	my_printf(_T("next\t%08X  %s\n"), cpu->get_next_pc(), buffer);

	debugger_thread_param.running = true;
	debugger_thread_param.request_terminate == false;
	dump_addr = 0;
	dasm_addr = cpu->get_next_pc();
	
	//running = true;
}

void CSP_Debugger::doExit(void)
{
	DEVICE *cpu = debugger_thread_param.vm->get_cpu(debugger_thread_param.cpu_index);
	DEBUGGER *debugger = (DEBUGGER *)cpu->get_debugger();
	debugger_thread_param.request_terminate = true;
	
	try {
		debugger->now_debugging = debugger->now_going = debugger->now_suspended = false;
	} catch(...) {
	}
	// release console
	debugger_thread_param.running = false;
	emit sig_finished();
}

void CSP_Debugger::stop_polling()
{
	//poll_stop = true;
}

CSP_Debugger::CSP_Debugger(QWidget *parent) : QWidget(parent, Qt::Window)
{
	widget = this;
	parent_object = parent;
	text = new QTextEdit(this);
	text->setReadOnly(true);
	text->setLineWrapMode(QTextEdit::WidgetWidth);
	//text->setCenterOnScroll(true);

	text_command = new QLineEdit(this);
	text_command->setEchoMode(QLineEdit::Normal);
	text_command->setMaxLength(1024);
	text_command->setReadOnly(false);
	text_command->setEnabled(true);
	text_command->clear();
	connect(text_command, SIGNAL(editingFinished()), this, SLOT(call_debugger()));
	
	VBoxWindow = new QVBoxLayout;

	VBoxWindow->addWidget(text);
	VBoxWindow->addWidget(text_command);
	this->setLayout(VBoxWindow);
	this->resize(640, 500);

	trap_timer = new QTimer(this);
	trap_timer->setInterval(3);
	trap_timer->setSingleShot(false);
	connect(this, SIGNAL(sig_start_trap()), trap_timer, SLOT(start()));
	connect(this, SIGNAL(sig_end_trap()),   trap_timer, SLOT(stop()));
	connect(trap_timer, SIGNAL(timeout()),  this, SLOT(check_trap()));

	trace_steps = 0;
}


CSP_Debugger::~CSP_Debugger()
{
	
}
   

void EMU::initialize_debugger()
{
	now_debugging = false;
	hDebugger = NULL;
}

void EMU::release_debugger()
{
	close_debugger();
}

void EMU::open_debugger(int cpu_index)
{
}

void EMU::close_debugger()
{
}

bool EMU::debugger_enabled(int cpu_index)
{
	return (vm->get_cpu(cpu_index) != NULL && vm->get_cpu(cpu_index)->get_debugger() != NULL);
}

#endif

