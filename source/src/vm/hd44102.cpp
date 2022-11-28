/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2017.03.07-

	[ HD44102 ]
*/

#include "hd44102.h"

#define CONTROL_DISPLAY_OFF         0x38
#define CONTROL_DISPLAY_ON          0x39
#define CONTROL_COUNT_DOWN_MODE     0x3a
#define CONTROL_COUNT_UP_MODE       0x3b
#define CONTROL_Y_ADDRESS_MASK      0x3f
#define CONTROL_X_ADDRESS_MASK      0xc0
#define CONTROL_DISPLAY_START_PAGE  0x3e

#define STATUS_BUSY                 0x80    /* not supported */
#define STATUS_COUNT_UP             0x40
#define STATUS_DISPLAY_OFF          0x20
#define STATUS_RESET                0x10    /* not supported */

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  count_up_or_down -
//-------------------------------------------------

inline void HD44102::count_up_or_down()
{
	m_output = m_ram[m_x][m_y];
	
	if (m_status & STATUS_COUNT_UP)
	{
		if (++m_y > 49) m_y = 0;
	}
	else
	{
		if (--m_y < 0) m_y = 49;
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hd44102_device - constructor
//-------------------------------------------------

void HD44102::initialize()
{
//	m_cs2 = 0;
	m_page = 0;
	m_x = 0;
	m_y = 0;
	memset(m_ram, 0, sizeof(m_ram));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void HD44102::reset()
{
	m_status = STATUS_DISPLAY_OFF | STATUS_COUNT_UP;
}

//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint8_t HD44102::read(uint32_t offset)
{
	uint8_t data = 0;

//	if (m_cs2)
	{
		data = (offset & 0x01) ? data_r() : status_r();
	}

	return data;
}

//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void HD44102::write(uint32_t offset, uint8_t data)
{
//	if (m_cs2)
	{
		(offset & 0x01) ? data_w(data) : control_w(data);
	}
}

//-------------------------------------------------
//  status_r - status read
//-------------------------------------------------

uint8_t HD44102::status_r()
{
	return m_status;
}

//-------------------------------------------------
//  control_w - control write
//-------------------------------------------------

void HD44102::control_w(uint8_t data)
{
	if (m_status & STATUS_BUSY) return;

	switch (data)
	{
	case CONTROL_DISPLAY_OFF:
//		if (LOG) logerror("HD44102 '%s' Display Off\n", tag());

		m_status |= STATUS_DISPLAY_OFF;
		break;

	case CONTROL_DISPLAY_ON:
//		if (LOG) logerror("HD44102 '%s' Display On\n", tag());

		m_status &= ~STATUS_DISPLAY_OFF;
		break;

	case CONTROL_COUNT_DOWN_MODE:
//		if (LOG) logerror("HD44102 '%s' Count Down Mode\n", tag());

		m_status &= ~STATUS_COUNT_UP;
		break;

	case CONTROL_COUNT_UP_MODE:
//		if (LOG) logerror("HD44102 '%s' Count Up Mode\n", tag());

		m_status |= STATUS_COUNT_UP;
		break;

	default:
		{
		int x = (data & CONTROL_X_ADDRESS_MASK) >> 6;
		int y = data & CONTROL_Y_ADDRESS_MASK;

		if ((data & CONTROL_Y_ADDRESS_MASK) == CONTROL_DISPLAY_START_PAGE)
		{
//			if (LOG) logerror("HD44102 '%s' Display Start Page %u\n", tag(), x);

			m_page = x;
		}
		else if (y > 49)
		{
//			logerror("HD44102 '%s' Invalid Address X %u Y %u (%02x)!\n", tag(), data, x, y);
		}
		else
		{
//			if (LOG) logerror("HD44102 '%s' Address X %u Y %u (%02x)\n", tag(), data, x, y);

			m_x = x;
			m_y = y;
		}
		}
	}
}

//-------------------------------------------------
//  data_r - data read
//-------------------------------------------------

uint8_t HD44102::data_r()
{
	uint8_t data = m_output;

//	m_output = m_ram[m_x][m_y];

	count_up_or_down();

	return data;
}

//-------------------------------------------------
//  data_w - data write
//-------------------------------------------------

void HD44102::data_w(uint8_t data)
{
	m_ram[m_x][m_y] = data;

	count_up_or_down();
}

//-------------------------------------------------
//  cs2_w - chip select 2 write
//-------------------------------------------------

//void HD44102::write_signal(int id, uint32_t data, uint32_t mask)
//{
//	if(id == SIG_HD44102_CS2) {
//		m_cs2 = data & mask;
//	}
//}

//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

void HD44102::screen_update(int m_sx, int m_sy, bool reverse)
{
	scrntype_t color_on   = RGB_COLOR( 48,  56,  16);	// dot on
//	scrntype_t color_off  = RGB_COLOR(144, 150, 144);	// dot off
	scrntype_t color_back = RGB_COLOR(160, 168, 160);	// back
	
	for (int x = 0; x < 50; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			int sy = (m_page + y) % 4;
			int sx = reverse ? (49 - x) : x;
			
			uint8_t data = m_ram[sy][x];
			
			for (int b = 0; b < 8; b++)
			{
				int dy = m_sy + 8 * sy + b;
				int dx = m_sx + sx;
				
				if(dx >= 0 && dx < SCREEN_WIDTH && dy >= 0 && dy < SCREEN_HEIGHT) {
					int color = (m_status & STATUS_DISPLAY_OFF) ? 0 : ((data >> b) & 0x01);
					scrntype_t *dest = emu->get_screen_buffer(m_sy + sy * 8 + b) + (m_sx + sx);
					*dest = color ? color_on : color_back;
				}
			}
		}
	}
}

#define STATE_VERSION	1

bool HD44102::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(&m_ram[0][0], sizeof(m_ram), 1);
	state_fio->StateValue(m_status);
	state_fio->StateValue(m_output);
//	state_fio->StateValue(m_cs2);
	state_fio->StateValue(m_page);
	state_fio->StateValue(m_x);
	state_fio->StateValue(m_y);
	return true;
}

