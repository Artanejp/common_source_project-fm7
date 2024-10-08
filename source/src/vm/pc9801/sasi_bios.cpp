/*
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'

	Author : Kyuma Ohta <whaitshis.sowhat _at_ gmail.com>
	Date   : 2018.11.25-

	[ sasi pseudo bios ]
*/

#include <memory>

#include "../../common.h"
#include "./sasi_bios.h"
#include "./membus.h"
#include "./sasi.h"
#include "./cpureg.h"

#include "../harddisk.h"
#include "../scsi_host.h"
#include "../scsi_hdd.h"

#include "../i8259.h"

#if defined(UPPER_I386)
#include "../i386_np21.h"
#elif defined(HAS_I86) || defined(HAS_I186) || defined(HAS_V30)
#include "../i86.h"
#else
#include "../i286.h"
#endif
#if defined(HAS_SUB_V30)
#include "../i86.h"
#endif

#define EVENT_HALT_HOST 97
#define EVENT_IRQ_HOST  98
#define EVENT_IRQ_OFF   99

//#define _PSEUDO_BIOS_DEBUG

namespace PC9801 {

// regs
#define EAX	 regs[0]
#define ECX	 regs[1]
#define EDX	 regs[2]
#define EBX	 regs[3]
#define ESP	 regs[4]
#define EBP	 regs[5]
#define ESI	 regs[6]
#define EDI	 regs[7]
#define IP_L regs[8]
#define IP_H regs[9]

#define AX regpair[0].w.l
#define CX regpair[1].w.l
#define DX regpair[2].w.l
#define BX regpair[3].w.l
#define SP regpair[4].w.l
#define BP regpair[5].w.l
#define SI regpair[6].w.l
#define DI regpair[7].w.l

#define LOAD_AX(val) { regpair[0].w.h = 0; regpair[0].w.l = val; }
#define LOAD_CX(val) { regpair[1].w.h = 0; regpair[1].w.l = val; }
#define LOAD_DX(val) { regpair[2].w.h = 0; regpair[2].w.l = val; }
#define LOAD_BX(val) { regpair[3].w.h = 0; regpair[3].w.l = val; }
#define LOAD_SP(val) { regpair[4].w.h = 0; regpair[4].w.l = val; }
#define LOAD_BP(val) { regpair[5].w.h = 0; regpair[5].w.l = val; }
#define LOAD_SI(val) { regpair[6].w.h = 0; regpair[6].w.l = val; }
#define LOAD_DI(val) { regpair[7].w.h = 0; regpair[7].w.l = val; }

#define AL	regpair[0].b.l
#define AH	regpair[0].b.h
#define CL	regpair[1].b.l
#define CH	regpair[1].b.h
#define DL	regpair[2].b.l
#define DH	regpair[2].b.h
#define BL	regpair[3].b.l
#define BH	regpair[3].b.h
#define SPL	regpair[4].b.l
#define SPH	regpair[4].b.h
#define BPL	regpair[5].b.l
#define BPH	regpair[5].b.h
#define SIL	regpair[6].b.l
#define SIH	regpair[6].b.h
#define DIL	regpair[7].b.l
#define DIH	regpair[7].b.l

// sregs
#define ES	sregs[0]
#define CS	sregs[1]
#define SS	sregs[2]
#define DS	sregs[3]

// error
#define ERR_FDD_NOTREADY	1
#define ERR_FDD_PROTECTED	2
#define ERR_FDD_DELETED		4
#define ERR_FDD_NOTFOUND	8
#define ERR_FDD_CRCERROR	0x10
#define ERR_SCSI_NOTREADY	1
#define ERR_SCSI_PARAMERROR	2
#define ERR_SCSI_NOTCONNECTED	4

void BIOS::initialize()
{
	event_halt = -1;
	event_irq = -1;
}

void BIOS::reset()
{
	if(event_halt >= 0) cancel_event(this, event_halt);
	event_halt = -1;
	if(event_irq >= 0) cancel_event(this, event_irq);
	event_irq = -1;
}

bool BIOS::bios_int_i86(int intnum, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
{
	// SASI
	switch(intnum) {
	case 0x1b: // SASI BIOS (INT3)
		if(d_mem->is_sasi_bios_load()) return false;
		//out_debug_log("INT 1Bh\n");
		return bios_call_far_i86(0xfffc4, regs, sregs, ZeroFlag, CarryFlag, cycles, total_cycles);
		break;
	default:
		break;
	}
	return false;
}

bool BIOS::bios_int_ia32(int intnum, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
{
	// SASI
	//printf("INT %02x\n", intnum);
	switch(intnum) {
	case 0x1b: // SASI BIOS (INT3)
		if(d_mem->is_sasi_bios_load()) return false;
		//out_debug_log("INT 1Bh\n");
		return bios_call_far_ia32(0xfffc4, regs, sregs, ZeroFlag, CarryFlag, cycles, total_cycles);
		break;
	default:
		break;
	}
	return false;
}

bool BIOS::bios_call_far_i86(uint32_t PC, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
{
	uint32_t reg32[10];
	for(int i = 0; i < 10; i++) {
		reg32[i] = regs[i];
	}
	if(bios_call_far_ia32(PC, reg32, sregs, ZeroFlag, CarryFlag, cycles, total_cycles)) {
		for(int i = 0; i < 10; i++) {
			regs[i] = reg32[i];
		}
		return true;
	}
	return false;
}
bool BIOS::bios_call_far_ia32(uint32_t PC, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
{
	pair32_t *regpair = (pair32_t *)regs;
	bool need_retcall = false;
#ifdef _PSEUDO_BIOS_DEBUG
	this->out_debug_log(_T("%6x\tDISK BIOS: AH=%2x,AL=%2x,CX=%4x,DX=%4x,BX=%4x,DS=%2x,DI=%2x\n"), get_cpu_pc(0), AH,AL,CX,DX,BX,DS,DI);
#endif
	// ToDo: Check ITF BANK for EPSON :
	// Check ADDRESS: This pseudo-bios acts only $fffc4 ($1B) or $00ffffc4:
//#if defined(HAS_I386)
//	if(((PC != 0xfffc4) && (PC != 0x00ffffc4)) &&
//	   ((PC < 0xd7000) && (PC > 0xd7fff))) return false; // INT 1Bh
//#else
	if((PC != 0xfffc4) && (PC != 0x00ffffc4)) return false; // INT 1Bh
//#endif
	static const int elapsed_cycle = 200; // From NP2 0.86+trunk/ OK?
#if 1

	/*if((((AL & 0xf0) != 0x00) && ((AL & 0xf0) != 0x80))) */	{
		uint8_t seg = d_mem->read_data8(0x004b0 + (AL >> 4));
		uint32_t sp, ss;
		if ((seg != 0)) {
#if !defined(_PC9801) && !defined(_PC9801E) && !defined(_PC9801F) && !defined(_PC9801M)
			if(!(d_mem->is_sasi_bios_load())) {
				if(seg == 0xd7) goto __next; // To Pseudo SASI BIOS.
			}
#endif
			return false;
#ifdef _PSEUDO_BIOS_DEBUG
			this->out_debug_log(_T("%6x\tDISK BIOS: AH=%2x,AL=%2x,CX=%4x,DX=%4x,BX=%4x,DS=%2x,DI=%2x\n"), get_cpu_pc(0), AH,AL,CX,DX,BX,DS,DI);
#endif
			// ToDo: 32bit protected mode.
			sp = SP;
			ss = (uint32_t)SS;
			ss = ss << 4;
			ss = ss & 0xfffff0;
#ifdef _PSEUDO_BIOS_DEBUG
			out_debug_log("call by %.4x:%.4x",
						  d_mem->read_data16(ss + sp + 2),
						  d_mem->read_data16(ss + sp + 0));
			out_debug_log("bypass to %.4x:0018", seg << 8);
			out_debug_log("From AX=%04x BX=%04x %02x:%02x:%02x:%02x ES=%04x BP=%04x",
						  AX, BX, CL, DH, DL, CH,
						  ES, BP);
#endif
			d_mem->write_data16(ss + sp - 2, DS);
			d_mem->write_data16(ss + sp - 4, SI);
			d_mem->write_data16(ss + sp - 6, DI);
			d_mem->write_data16(ss + sp - 8, ES);
			d_mem->write_data16(ss + sp - 10, BP);
			d_mem->write_data16(ss + sp - 12, DX);
			d_mem->write_data16(ss + sp - 14, CX);
			d_mem->write_data16(ss + sp - 16, BX);
			d_mem->write_data16(ss + sp - 18, AX);

			sp = sp - 18;
			LOAD_SP(sp);
			LOAD_BP(sp);
//			DS = 0x0000;
			LOAD_BX(0x04b0);
			LOAD_AX(((uint16_t)seg) << 8);
//			CS = ((uint16_t)seg) << 8;
			IP_L = 0x0018;
			IP_H = 0x0000;
			if(cycles != NULL) {
				*cycles -= elapsed_cycle;
			}
			if(total_cycles != NULL) {
				*total_cycles += (uint64_t)elapsed_cycle;
			}
#ifdef _PSEUDO_BIOS_DEBUG
			out_debug_log("To AX=%04x BX=%04x %02x:%02x:%02x:%02x ES=%04x BP=%04x",
						  AX, BX, CL, DH, DL, CH,
						  ES, BP);
#endif
			return true;
		}
	}
#endif
__next:
	// FUNC $1B: If FLOPPY, return (MAY USE STANDARD IPL).
	uint16_t backup_ax = AX;
	uint16_t backup_bx = BX;
	uint16_t backup_cx = CX;
	uint16_t backup_dx = DX;

	switch(AL & 0xf0) {
		case 0xc0:
			// ToDo: SCSI BIOS
//			return false;
			break;
		case 0x00:
		case 0x80:
			if(!(d_mem->is_sasi_bios_load())) {
				if(sasi_bios(PC, regs, (uint16_t*)sregs, ZeroFlag, CarryFlag)) {
#ifdef _PSEUDO_BIOS_DEBUG
					out_debug_log(_T("SASI BIOS CALL SUCCESS:\n From AX=%04x BX=%04x CX=%04x DX=%04x\n To AX=%04x BX=%04x CX=%04x DX=%04x\n"), backup_ax, backup_bx, backup_cx, backup_dx, AX, BX, CX, DX);
#endif
				} else {
#ifdef _PSEUDO_BIOS_DEBUG
					out_debug_log(_T("SASI BIOS CALL FAILED:\n From AX=%04x BX=%04x CX=%04x DX=%04x\n To AX=%04x BX=%04x CX=%04x DX=%04x\n"), backup_ax, backup_bx, backup_cx, backup_dx, AX, BX, CX, DX);
#endif
				}
				need_retcall = true;
			}
			break;
		default:
//			return false;
			break;
	}
	if(need_retcall) {
		uint8_t flag = d_mem->read_data8((SS << 4) | ((SP + 4) & 0xffff)) & 0xfe;

		if(AH >= 0x20) {
			flag++;
		}
		d_mem->write_data8((SS << 4) | ((SP + 4) & 0xffff), flag);
		if(cycles != NULL) {
			*cycles -= elapsed_cycle;
		}
		if(total_cycles != NULL) {
			*total_cycles += (uint64_t)elapsed_cycle;
		}
		return true;
	}
	return false;
}

bool BIOS::sasi_bios(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	pair32_t *regpair = (pair32_t *)regs;
#if defined(SUPPORT_SASI_IF)
	int drv = 0;
	if(d_sasi != NULL) {
		// ToDo: Multi SASI
		drv = sxsi_get_drive(AL);
		if(d_sasi->get_hdd(drv) != NULL) {
			if(drv >= 0) {
				//out_debug_log("SASI BIOS CALL AH=%02X\n", AH);
				switch(AH & 0x0f) {
				case 0x01:
					// Verify
					sasi_command_verify(PC, regs, sregs, ZeroFlag, CarryFlag);
					break;
				case 0x03:
					// INITIALIZE
					sasi_command_initialize(PC, regs, sregs, ZeroFlag, CarryFlag);
					break;
				case 0x04:
					// SENS
					sasi_command_sense(PC, regs, sregs, ZeroFlag, CarryFlag);
					break;
				case 0x05:
					// WRITE DATA
					sasi_command_write(PC, regs, sregs, ZeroFlag, CarryFlag);
					break;
				case 0x06:
					// READ DATA
					//out_debug_log(_T("READ DATA ES:BP=%04X:%04X SIZE=%d\n"), ES, BP, (BX == 0) ? 0x10000 : BX);
					sasi_command_read(PC, regs, sregs, ZeroFlag, CarryFlag);
					break;
				case 0x07:
					// RETRACT
					sasi_command_retract(PC, regs, sregs, ZeroFlag, CarryFlag);
					break;
				case 0x0d:
					// FORMAT
					sasi_command_format(PC, regs, sregs, ZeroFlag, CarryFlag);
					break;
				case 0x0f:
					// RETRACT
					sasi_command_retract(PC, regs, sregs, ZeroFlag, CarryFlag);
					break;
				default:
					sasi_command_illegal(PC, regs, sregs, ZeroFlag, CarryFlag);
					break;
				}
				d_mem->write_io8(0x043f, 0xc2); // Enable to write ram
				if(*CarryFlag != 0) {
					d_mem->write_data8(0x0585, ((drv & 7) << 5) | 0x1c | 0x02);
				} else {
					d_mem->write_data8(0x0585, ((drv & 7) << 5) | 0x1c | 0x00);
				}
				d_mem->write_io8(0x043f, 0xc0); // Disable to write ram
				return true;
			}
		}
	}
	AH = 0x60; // SASI NOT SET
	*CarryFlag = 1;
	d_mem->write_io8(0x043f, 0xc2); // Enable to write ram
	d_mem->write_data8(0x0585, ((drv & 7) << 5) | 0x1c | 0x02);
	d_mem->write_io8(0x043f, 0xc0); // Disable to write ram
	return true;
#else

// FALLBACK
	return false;
#endif
}

int BIOS::sxsi_get_drive(uint8_t al)
{

	uint8_t num = al & 0x0f;
	if((al & 0x20) == 0) {
		// ToDo: Multiple SASI
		if(num < USE_HARD_DISK) {
			return num;
		}
	}
	return -1;
}

// Command $x1
void BIOS::sasi_command_verify(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	pair32_t *regpair = (pair32_t *)regs;
	regpair[0].w.h = 0; // HIGH WORD OF EAX
	// Assume success
	AH = 0x00;
	*CarryFlag = 0;
	return;
}

// Command $x7, $xf
void BIOS::sasi_command_retract(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	pair32_t *regpair = (pair32_t *)regs;
	regpair[0].w.h = 0; // HIGH WORD OF EAX
	// Assume success
	int drive = sxsi_get_drive(AL);
	//halt_host_cpu(20.0 * 1000.0); // Seek time
	if (drive < 0) {
		AH = 0x60;
		*CarryFlag = 1;
		return;
	}
	interrupt_to_host(15.0 * 1000.0);
	AH = 0x00 | (drive & 0x07);
	*CarryFlag = 0;
	return;
}

// Command (ILLEGAL)
void BIOS::sasi_command_illegal(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	pair32_t *regpair = (pair32_t *)regs;
	int drive = sxsi_get_drive(AL);
	regpair[0].w.h = 0; // HIGH WORD OF EAX
	if (drive < 0) {
		AH = 0x60;
		*CarryFlag = 1;
		return;
	}
	AH = 0x40;
	*CarryFlag = 1;
	return;
}

long BIOS::sasi_get_position(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	pair32_t *regpair = (pair32_t *)regs;
	int drive = sxsi_get_drive(AL);
	regpair[0].w.h = 0; // HIGH WORD OF EAX
	SASI_HDD*  d_hdd = d_sasi->get_hdd(drive);
	if(d_hdd == NULL) {
		return -1;
	} else {
		if(d_hdd->mounted(drive)) {
			HARDDISK *hdd = d_hdd->get_disk_handler(drive);
			if(hdd == NULL) {
				return -1;
			} else {
				if((AL & 0x80) != 0) {
					int sectsize = hdd->get_sector_size();
					int cylinders = hdd->get_cylinders();
					int head = hdd->get_headers();
					int sectors = hdd->get_sectors_per_cylinder();
					long npos;
					if((DL >= (uint8_t)sectors) ||
					   (DH >= (uint8_t)head) ||
					   (CX >= (uint16_t)cylinders)) {
						return -1;
					}
					npos = ((head * CX) + DH) * sectors + DL;
					return npos;
				} else {
					long npos;
					long apos;

					npos = (DL << 16) | CX;
					//apos = hdd->get_cur_position();
					//if(npos >= 0x100000) {
					//	npos = -npos;
					//}
					//npos = npos + apos;
					npos = npos & 0x1fffff;
					if(npos < 0) npos = 0;
					if(npos >= (long)hdd->get_sector_num()) {
						return -1;
					}
					return npos;
				}
			}
		}
	}
	return -1;
}

void BIOS::sasi_command_initialize(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	pair32_t *regpair = (pair32_t *)regs;
	regpair[0].w.h = 0; // HIGH WORD OF EAX

	uint16_t disk_equip;
	uint32_t _bit = 0x0100;
	pair16_t _d;

	d_mem->write_io8(0x043f, 0xc2); // Enable to read ram
	_d.b.l = d_mem->read_data8(0x055c + 0);
	_d.b.h = d_mem->read_data8(0x055c + 1);
	d_mem->write_io8(0x043f, 0xc0); // Disable to read ram

	disk_equip = _d.u16;
	disk_equip = disk_equip & 0xf0ff;
	for(uint32_t i = 0; i < 4; i++) {
		// ToDo: Multi SASI
		if(d_sasi != NULL) {
			SASI_HDD*  d_hdd = d_sasi->get_hdd(i);
			if(d_hdd != NULL) {
				if(d_hdd->mounted(i)) disk_equip = disk_equip | _bit;
			}
		}
		_bit <<= 1;
	}
	_d.u16 = disk_equip;
	d_mem->write_io8(0x043f, 0xc2); // Enable to write ram
	d_mem->write_dma_data8(0x055c + 0, _d.b.l);
	d_mem->write_dma_data8(0x055c + 1, _d.b.h);
	d_mem->write_io8(0x043f, 0xc0); // Disable to write ram
#ifdef _PSEUDO_BIOS_DEBUG
	out_debug_log(_T("SASI CMD: INITIALIZE STAT=%04x"), disk_equip);
#endif
	AH = 0x00;
	*CarryFlag = 0;
}

void BIOS::sasi_command_sense(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	pair32_t *regpair = (pair32_t *)regs;
	regpair[0].w.h = 0; // HIGH WORD OF EAX

	uint16_t disk_equip;
	uint32_t _bit = 0x0100;
	pair16_t _d;
	// ToDo: IDE OR SASI
	//if(is_ide) {
	//	if(AH == 0x84) {
	//		// SIZE, CHS
	//	}
	//	AH = 0x0f;
	//} else
	{
		// ToDo: Multi HDD.
		uint8_t d = AL & 0x0f;
		uint8_t cmd = AH;
//		if(d == 0x0f) {
//			// HOST
//			AH = 0x07;
//			*CarryFlag = 0;
#ifdef _PSEUDO_BIOS_DEBUG
			out_debug_log(_T("SASI CMD: SENSE DL=%02x DH=%02x CX=%04x BX=%04x\n"), DL, DH, CX, BX);
#endif
//			return;
//		} else
		{
			SASI_HDD*  d_hdd = d_sasi->get_hdd(d);
#if 0 // Still not support wait state
			if(d_hdd != NULL) {
				d_hdd->command[0] = SCSI_CMD_REQ_SENSE;
				d_hdd->command[1] = 0;
				d_hdd->command[2] = 0;
				d_hdd->command[3] = 0;
				d_hdd->start_command();
				// Poll command
			} else {
				AH = 0x60; // Not Ready
				*CarryFlag = 1;
			}
#else
			// We still read from disk directly. 20181125 K.O
			if(d_hdd != NULL) {
				int drive = sxsi_get_drive(AL);
				if(drive >= 0) {
					if(d_hdd->mounted(drive)) {
						AH = drive & 7; // OK?
						*CarryFlag = 0;
						HARDDISK *hdd = d_hdd->get_disk_handler(drive);
						if((cmd == 0x84) || (cmd == 0x04)) {
							int sectsize = hdd->get_sector_size();
							int cylinders = hdd->get_cylinders();
							int head = hdd->get_headers();
							int sectors = hdd->get_sectors_per_cylinder();

							regpair[1].w.h = 0; // HIGH WORD OF ECX
							regpair[2].w.h = 0; // HIGH WORD OF EDX
							regpair[3].w.h = 0; // HIGH WORD OF EBX
							if(cmd == 0x84) {
								DL = (uint8_t)sectors;                // Sectors
								DH = (uint8_t)head;         // Heads
								CX = (uint16_t)cylinders; // Cylinders
								BX = (uint16_t)(sectsize); // logical block
							}
							int total_size = (sectors * head * cylinders * sectsize);
							uint8_t sense_size = 0x04;
							if(total_size <= 0) {
								AH = (sense_size | 0x40);
								*CarryFlag = 1;
								return;
							}
							d_mem->write_io8(0x043f, 0xc2); // Enable to write ram
							d_mem->write_io8(0x043f, 0xc2); // Disable to write ram


							// ToDo: Support Larger drive.
							if(total_size < (10 * 1024 * 1024)) {
								sense_size = 0x00;
							} else if(total_size < (20 * 1024 * 1024)) {
								sense_size = 0x01;
							} else if(total_size < (40 * 1024 * 1024)) {
								sense_size = 0x03;
							} else {
								sense_size = 0x04;
							}
							AH = (sense_size | 0x00);
							//AH = 0x00;
							*CarryFlag = 0;
						}
#ifdef _PSEUDO_BIOS_DEBUG
						out_debug_log(_T("SASI CMD: CMD#=%02x SENSE DL=%02x DH=%02x CX=%04x BX=%04x\n"), cmd, DL, DH, CX, BX);
#endif
						return;
					}
				}
			}
#ifdef _PSEUDO_BIOS_DEBUG
			out_debug_log(_T("SASI CMD: SENSE DL=%02x DH=%02x CX=%04x BX=%04x\n"), DL, DH, CX, BX);
#endif
			AH = 0x60; // Not Ready
			*CarryFlag = 1;
#endif
		}
	}
}

void BIOS::sasi_command_read(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	pair32_t *regpair = (pair32_t *)regs;
	regpair[0].w.h = 0; // HIGH WORD OF EAX

	int size = (int)(BX & 0xffff);
	if(size == 0) size = 0x10000;
	uint32_t addr;
	addr = (((uint32_t)ES) << 4) + BP;
	//try {
	//	addr = d_cpu->translate_address(0, BP); // ES:BP
	//} catch(...) {
	//	out_debug_log("Access vioration ES:%04x\n", BP);
	//}
	//out_debug_log(_T("SASI CMD: READ ADDR=%08x\n"), addr);

	int drive = sxsi_get_drive(AL);
	if(drive < 0) { // ToDo: Multi SASI
		AH = 0x60;
		*CarryFlag = 1;
#ifdef _PSEUDO_BIOS_DEBUG
		out_debug_log(_T("SASI CMD: READ AH=%02x\n"), AH);
#endif
		return;
	} else {
		SASI_HDD*  d_hdd = d_sasi->get_hdd(drive); // OK?
		if(d_hdd == NULL) {
			// Not Connected
			AH = 0x60;
			*CarryFlag = 1;
#ifdef _PSEUDO_BIOS_DEBUG
			out_debug_log(_T("SASI CMD: READ AH=%02x\n"), AH);
#endif
			return;
		} else {
			long npos = sasi_get_position(PC, regs, sregs, ZeroFlag, CarryFlag);
			HARDDISK* harddisk = d_hdd->get_disk_handler(drive);
			const uint32_t sectors = (uint32_t)harddisk->get_sector_num();
			const int block_size = (int)harddisk->get_sector_size();
#ifdef _PSEUDO_BIOS_DEBUG
			out_debug_log(_T("SASI CMD: READ: DRIVE=%d POS=%d DL=%02x DH=%02x CX=%04x BX=%04x\n"), drive, npos, DL, DH, CX, BX);
#endif
			if((npos < 0) || (npos >= (int)sectors)) {
				AH = 0x40;
				*CarryFlag = 1;
				return;
			}
			if(harddisk == NULL) {
				AH = 0x60;
				*CarryFlag = 1;
				return;
			} else if(!(harddisk->mounted())) {
			// Not Connected
				AH = 0x60;
				*CarryFlag = 1;
				return;
			}

			int64_t position = ((int64_t)npos) * ((int64_t)block_size);
			//size = size * block_size;

			std::unique_ptr<uint8_t[]> buffer(new uint8_t[block_size]);
			int block = (int)npos;
			while(size > 0) {
				if(!(block++ < sectors)) {
					// SEEK ERROR
					AH = 0xd0;
					*CarryFlag = 1;
					return;
				}
				// data transfer
				if(harddisk->read_buffer((long)position, block_size, buffer.get())) {
					position += block_size;
					for(int i = 0; i < block_size; i++) {
						d_mem->write_dma_data8(addr++, buffer[i]);
					}
					size -= block_size;
				} else {
					// READ ERROR
					AH = 0xd0;
					*CarryFlag = 1;
					return;
				}
			}
		}
	}
	// SUCCESS
	AH = 0x00;
	*CarryFlag = 0;
	return;
}

void BIOS::sasi_command_write(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	pair32_t *regpair = (pair32_t *)regs;
	int size = (int)(BX & 0xffff);
	if(size == 0) size = 0x10000;

	regpair[0].w.h = 0; // HIGH WORD OF EAX

	uint32_t addr;
	addr = (((uint32_t)ES) << 4) + BP;
	//try {
	//	addr = d_cpu->translate_address(0, BP); // ES:BP
	//} catch(...) {
	//	out_debug_log("Access vioration ES:%04x\n", BP);
	//}
	int drive = sxsi_get_drive(AL);
	if(drive < 0) { // ToDo: Multi SASI
		AH = 0x60;
		*CarryFlag = 1;
		return;
	} else {
		SASI_HDD*  d_hdd = d_sasi->get_hdd(drive); // OK?
		if(d_hdd == NULL) {
			// Not Connected
			AH = 0x60;
			*CarryFlag = 1;
			return;
		} else {
			long npos = sasi_get_position(PC, regs, sregs, ZeroFlag, CarryFlag);
			HARDDISK* harddisk = d_hdd->get_disk_handler(drive);
			const uint32_t sectors = (uint32_t)harddisk->get_sector_num();
			const int block_size = (int)harddisk->get_sector_size();
#ifdef _PSEUDO_BIOS_DEBUG
			out_debug_log(_T("SASI CMD: WRITE DL=%02x DH=%02x CX=%04x BX=%04x\n"), DL, DH, CX, BX);
#endif
			if((npos < 0) || (npos >= (int)sectors)) {
				AH = 0x40;
				*CarryFlag = 1;
				return;
			}
			if(harddisk == NULL) {
				AH = 0x60;
				*CarryFlag = 1;
				return;
			} else if(!(harddisk->mounted())) {
			// Not Connected
				AH = 0x60;
				*CarryFlag = 1;
				return;
			}
			//size = size * block_size;
			int64_t position = ((int64_t)npos) * ((int64_t)block_size);

			std::unique_ptr<uint8_t[]> buffer(new uint8_t[block_size]);
			int block = (int)npos;
			while(size > 0) {
				if(!(block++ < sectors)) {
					// SEEK ERROR
					AH = 0xd0;
					*CarryFlag = 1;
					return;
				}
				// data transfer
				for(int i = 0; i < block_size; i++) {
					buffer[i] = d_mem->read_dma_data8(addr++);
				}
				if(harddisk->write_buffer((long)position, block_size, buffer.get())) {
					position += block_size;
					size -= block_size;
				} else {
					// READ ERROR
					AH = 0xd0;
					*CarryFlag = 1;
					return;
				}
			}
		}
	}
	// SUCCESS
	AH = 0x00;
	*CarryFlag = 0;
	return;
}

void BIOS::sasi_command_format(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	pair32_t *regpair = (pair32_t *)regs;
	regpair[0].w.h = 0; // HIGH WORD OF EAX


	int drive = sxsi_get_drive(AL);
	SASI_HDD*  d_hdd = d_sasi->get_hdd(drive); // OK?

	if(d_hdd == NULL) {
		// Not Connected
		AH = 0x60;
		*CarryFlag = 1;
		return;
	}
	if((AH & 0x80) != 0) {
		// SCSI
		//WIP. Not SASI
		AH = 0xd0;
		*CarryFlag = 1;
		return;
	} else {
		if(DL != 0) {
			AH = 0x30;
			*CarryFlag = 1;
			return;
		}
		HARDDISK* harddisk = d_hdd->get_disk_handler(drive);
#ifdef _PSEUDO_BIOS_DEBUG
		out_debug_log(_T("SASI CMD: FORMAT DL=%02x DH=%02x CX=%04x BX=%04x\n"), DL, DH, CX, BX);
#endif
		if(harddisk == NULL) {
			AH = 0x60;
			*CarryFlag = 1;
			return;
		} else if(!(harddisk->mounted())) {
			// Not Connected
			AH = 0x60;
			*CarryFlag = 1;
			return;
		} else {
			long npos = sasi_get_position(PC, regs, sregs, ZeroFlag, CarryFlag);
			//const uint32_t sectors = (uint32_t)harddisk->get_sector_num();
			const int sectors = (uint32_t)harddisk->get_sectors_per_cylinder();
			const int block_size = (int)harddisk->get_sector_size();

			if(block_size > 1024) {
				AH = 0xd0;
				*CarryFlag = 1;
				return;
			}
			if((npos < 0) || (npos >= sectors)) {
				AH = 0x40;
				*CarryFlag = 1;
				return;
			}
			int64_t position = ((int64_t)npos) * ((int64_t)block_size);

			std::unique_ptr<uint8_t[]> work(new uint8_t[block_size]);
			for(int i = 0; i < sectors; i++) {
				memset(work.get(), 0xe5, block_size);
				if(harddisk->write_buffer((long)position, block_size, work.get())) {
					position += block_size;
					// TBD: CLOCKS
				} else {
					// FORMAT ERROR
					AH = 0x70;
					*CarryFlag = 1;
					return;
				}
			}
			*CarryFlag = 0;
			AH = 0x00; // SUCCESS
			return;
		}
	}
	// WIP
	AH = 0xd0;
	*CarryFlag = 1;
	return;
}

void BIOS::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_HALT_HOST:
	// ToDo: Before V30 model.
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
		d_cpureg->write_signal(SIG_CPUREG_HALT, 0x00000000, 0xffffffff);
#endif
		event_halt = -1;
		break;
	case EVENT_IRQ_HOST:
		d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR1, 0xffffffff, 0xffffffff);
		//register_event(this, EVENT_IRQ_HOST, 1.0, false, &event_irq);
		event_irq = -1;
		break;
	case EVENT_IRQ_OFF:
		d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR1, 0x00000000, 0xffffffff);
		event_irq = -1;
		break;
	default:
		break;
	}
}

void BIOS::halt_host_cpu(double usec)
{
	if(event_halt >= 0) {
		cancel_event(this, event_halt);
	}
	// ToDo: Before V30 model.
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	d_cpureg->write_signal(SIG_CPUREG_HALT, 0xffffffff, 0xffffffff);
#endif
	register_event(this, EVENT_HALT_HOST, usec, false, &event_halt);
}

void BIOS::interrupt_to_host(double usec)
{
	if(event_irq >= 0) {
		cancel_event(this, event_irq);
	}
	register_event(this, EVENT_IRQ_HOST, usec, false, &event_irq);
}

#define STATE_VERSION	1

bool BIOS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}

	state_fio->StateValue(event_halt);
	state_fio->StateValue(event_irq);

	return true;
}

}
