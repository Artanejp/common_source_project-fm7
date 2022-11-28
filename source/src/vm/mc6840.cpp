/*
	Skelton for retropc emulator

	Origin : MAME 0.168 Motorola 6840 (PTM)
	Author : Takeda.Toshiya
	Date   : 2016.02.24-

	[ MC6840 ]
*/

// license:BSD-3-Clause
// copyright-holders:James Wallace
/***************************************************************************

    Motorola 6840 (PTM)

    Programmable Timer Module

    Written By J.Wallace based on previous work by Aaron Giles,
   'Re-Animator' and Mathis Rosenhauer.

    Todo:
         Confirm handling for 'Single Shot' operation.
         (Datasheet suggests that output starts high, going low
         on timeout, opposite of continuous case)
         Establish whether ptm6840_set_c? routines can replace
         hard coding of external clock frequencies.


    Operation:
    The interface is arranged as follows:

    Internal Clock frequency,
    Clock 1 frequency, Clock 2 frequency, Clock 3 frequency,
    Clock 1 output, Clock 2 output, Clock 3 output,
    IRQ function

    If the external clock frequencies are not fixed, they should be
    entered as '0', and the ptm6840_set_c?(which, state) functions
    should be used instead if necessary (This should allow the VBLANK
    clock on the MCR units to operate).


    2009-06 Converted to be a device

***************************************************************************/

#include "mc6840.h"

#define from_hz(v)	(1000000.0 / (double)(v))
typedef double attotime;

#define m_out0_cb(o,v)	write_signals(&outputs_ch0, (v) ? 0xffffffff : 0)
#define m_out1_cb(o,v)	write_signals(&outputs_ch1, (v) ? 0xffffffff : 0)
#define m_out2_cb(o,v)	write_signals(&outputs_ch2, (v) ? 0xffffffff : 0)
#define m_irq_cb(v)	write_signals(&outputs_irq, (v) ? 0xffffffff : 0)

//#define PTMVERBOSE 0
//#define PLOG(x) do { if (PTMVERBOSE) logerror x; } while (0)

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void MC6840::initialize()
{
	for (int i = 0; i < 3; i++)
	{
//		if ( m_external_clock[i] == 0 )
//		{
//			m_external_clock[i] = 1;
//		}
		m_gate[i] = m_clk[i] = 0;
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void MC6840::reset()
{
	m_control_reg[2]         = 0;
	m_control_reg[1]         = 0;
	m_control_reg[0]         = 1;
	m_status_reg             = 0;
	m_t3_divisor             = 1;
	m_status_read_since_int = 0;
	m_IRQ                   = 0;
	m_t3_scaler             = 0;
	for (int i = 0; i < 3; i++)
	{
		m_counter[i] = 0xffff;
		m_latch[i]   = 0xffff;
		m_output[i]  = 0;
		m_fired[i]   = 0;
		m_enabled[i] = 0;
		m_mode[i] = 0;
		m_timer[i] = -1;
	}
}

//-------------------------------------------------
//  subtract_from_counter - Subtract from Counter
//-------------------------------------------------

void MC6840::subtract_from_counter(int counter, int count)
{
	double clock;

	// Determine the clock frequency for this timer
	if (m_control_reg[counter] & 0x02)
	{
		clock = m_internal_clock;
	}
	else
	{
		clock = m_external_clock[counter];
	}

	// Dual-byte mode
	if (m_control_reg[counter] & 0x04)
	{
		int lsb = m_counter[counter] & 0xff;
		int msb = m_counter[counter] >> 8;

		// Count the clocks
		lsb -= count;

		// Loop while we're less than zero
		while (lsb < 0)
		{
			// Borrow from the MSB
			lsb += (m_latch[counter] & 0xff) + 1;
			msb--;

			// If MSB goes less than zero, we've expired
			if (msb < 0)
			{
				timeout(counter);
				msb = (m_latch[counter] >> 8) + 1;
			}
		}

		// Store the result
		m_counter[counter] = (msb << 8) | lsb;
	}

	// Word mode
	else
	{
		int word = m_counter[counter];

		// Count the clocks
		word -= count;

		// loop while we're less than zero
		while (word < 0)
		{
			// Borrow from the MSB
			word += m_latch[counter] + 1;

			// We've expired
			timeout(counter);
		}

		// Store the result
		m_counter[counter] = word;
	}

	if (clock && m_enabled[counter])
	{
		attotime duration = from_hz(clock) * m_counter[counter];

		if (counter == 2)
		{
			duration *= m_t3_divisor;
		}

		if (m_timer[counter] != -1)
		{
			cancel_event(this, m_timer[counter]);
		}
		register_event(this, counter, duration, false, &m_timer[counter]);
	}
}

//-------------------------------------------------
//  tick
//-------------------------------------------------

void MC6840::tick(int counter, int count)
{
	if (counter == 2)
	{
		m_t3_scaler += count;

		if ( m_t3_scaler > m_t3_divisor - 1)
		{
			subtract_from_counter(counter, 1);
			m_t3_scaler = 0;
		}
	}
	else
	{
		subtract_from_counter(counter, count);
	}
}

//-------------------------------------------------
//  update_interrupts - Update Internal Interrupts
//-------------------------------------------------

void MC6840::update_interrupts()
{
	int new_state = ((m_status_reg & 0x01) && (m_control_reg[0] & 0x40)) ||
					((m_status_reg & 0x02) && (m_control_reg[1] & 0x40)) ||
					((m_status_reg & 0x04) && (m_control_reg[2] & 0x40));

//	if (new_state != m_IRQ)
	{
		m_IRQ = new_state;

		if (m_IRQ)
		{
			m_status_reg |= 0x80;
		}
		else
		{
			m_status_reg &= ~0x80;
		}

		m_irq_cb(m_IRQ);
	}
}

//-------------------------------------------------
//  compute_counter - Compute Counter
//-------------------------------------------------

UINT16 MC6840::compute_counter( int counter )
{
	double clock;

	// determine the clock frequency for this timer
	if (m_control_reg[counter] & 0x02)
	{
		clock = m_internal_clock;
//		PLOG(("MC6840 #%s: %d internal clock freq %f \n", tag(), counter, clock));
	}
	else
	{
		clock = m_external_clock[counter];
//		PLOG(("MC6840 #%s: %d external clock freq %f \n", tag(), counter, clock));
	}

	// If there's no timer, return the count
	if (!(clock && m_enabled[counter]))
	{
//		PLOG(("MC6840 #%s: read counter(%d): %d\n", tag(), counter, m_counter[counter]));
		return m_counter[counter];
	}

	// See how many are left
	int remaining = (int)(get_event_remaining_usec(m_timer[counter]) / 1000000.0 * clock);

	// Adjust the count for dual byte mode
	if (m_control_reg[counter] & 0x04)
	{
		int divisor = (m_counter[counter] & 0xff) + 1;
		int msb = remaining / divisor;
		int lsb = remaining % divisor;
		remaining = (msb << 8) | lsb;
	}
//	PLOG(("MC6840 #%s: read counter(%d): %d\n", tag(), counter, remaining));
	return remaining;
}

//-------------------------------------------------
//  reload_count - Reload Counter
//-------------------------------------------------

void MC6840::reload_count(int idx)
{
	double clock;

	// Copy the latched value in
	m_counter[idx] = m_latch[idx];

	// Determine the clock frequency for this timer
	if (m_control_reg[idx] & 0x02)
	{
		clock = m_internal_clock;
//		PLOG(("MC6840 #%s: %d internal clock freq %f \n", tag(), idx, clock));
	}
	else
	{
		clock = m_external_clock[idx];
//		PLOG(("MC6840 #%s: %d external clock freq %f \n", tag(), idx, clock));
	}

	// Determine the number of clock periods before we expire
	int count = m_counter[idx];
	if (m_control_reg[idx] & 0x04)
	{
		count = ((count >> 8) + 1) * ((count & 0xff) + 1);
	}
	else
	{
		count = count + 1;
	}

	m_fired[idx] = 0;

	if ((m_mode[idx] == 4) || (m_mode[idx] == 6))
	{
		m_output[idx] = 1;
		switch (idx)
		{
			case 0:
				m_out0_cb((offs_t)0, m_output[0]);
				break;
			case 1:
				m_out1_cb((offs_t)0, m_output[1]);
				break;
			case 2:
				m_out2_cb((offs_t)0, m_output[2]);
				break;
		}
	}

	if(clock) {
		// Set the timer
//		PLOG(("MC6840 #%s: reload_count(%d): clock = %f  count = %d\n", tag(), idx, clock, count));

		attotime duration = from_hz(clock) * count;
		if (idx == 2)
		{
			duration *= m_t3_divisor;
		}

//		PLOG(("MC6840 #%s: reload_count(%d): output = %f\n", tag(), idx, duration.as_double()));

		if (m_timer[idx] != -1)
		{
			cancel_event(this, m_timer[idx]);
		}
		register_event(this, idx, duration, false, &m_timer[idx]);
	}

	m_enabled[idx] = 1;
}

//-------------------------------------------------
//  read - Read Timer
//-------------------------------------------------

uint32_t MC6840::read_io8(uint32_t offset)
{
	int val;

	offset &= 7;

	switch ( offset )
	{
		case PTM_6840_CTRL1:
		{
			val = 0;
			break;
		}

		case PTM_6840_STATUS:
		{
//			PLOG(("%s: MC6840 #%s: Status read = %04X\n", machine().describe_context(), tag(), m_status_reg));
			m_status_read_since_int |= m_status_reg & 0x07;
			val = m_status_reg;
			break;
		}

		case PTM_6840_MSBBUF1:
		case PTM_6840_MSBBUF2:
		case PTM_6840_MSBBUF3:
		{
			int idx = (offset - 2) / 2;
			int result = compute_counter(idx);

			// Clear the interrupt if the status has been read
			if (m_status_read_since_int & (1 << idx))
			{
				m_status_reg &= ~(1 << idx);
				update_interrupts();
			}

			m_lsb_buffer = result & 0xff;

//			PLOG(("%s: MC6840 #%s: Counter %d read = %04X\n", machine().describe_context(), tag(), idx, result >> 8));
			val = result >> 8;
			break;
		}

		case PTM_6840_LSB1:
		case PTM_6840_LSB2:
		case PTM_6840_LSB3:
		{
			val = m_lsb_buffer;
			break;
		}

		default:
		{
			val = 0;
			break;
		}

	}
	return val;
}

//-------------------------------------------------
//  write - Write Timer
//-------------------------------------------------

void MC6840::write_io8(uint32_t offset, uint32_t data)
{
	offset &= 7;

	switch ( offset )
	{
		case PTM_6840_CTRL1:
		case PTM_6840_CTRL2:
		{
			int idx = (offset == 1) ? 1 : (m_control_reg[1] & 0x01) ? 0 : 2;
			UINT8 diffs = data ^ m_control_reg[idx];
			m_t3_divisor = (m_control_reg[2] & 0x01) ? 8 : 1;
			m_mode[idx] = (data >> 3) & 0x07;
			m_control_reg[idx] = data;

//			PLOG(("MC6840 #%s : Control register %d selected\n", tag(), idx));
//			PLOG(("operation mode   = %s\n", opmode[ m_mode[idx] ]));
//			PLOG(("value            = %04X\n", m_control_reg[idx]));
//			PLOG(("t3divisor        = %d\n", m_t3_divisor));

			if (!(m_control_reg[idx] & 0x80 ))
			{
				// Output cleared
				switch (idx)
				{
					case 0:
						m_out0_cb((offs_t)0, 0);
						break;
					case 1:
						m_out1_cb((offs_t)0, 0);
						break;
					case 2:
						m_out2_cb((offs_t)0, 0);
						break;
				}
			}
			// Reset?
			if (idx == 0 && (diffs & 0x01))
			{
				// Holding reset down
				if (data & 0x01)
				{
//					PLOG(("MC6840 #%s : Timer reset\n", tag()));
					for (int i = 0; i < 3; i++)
					{
						if (m_timer[i] != -1)
						{
							cancel_event(this, m_timer[i]);
							m_timer[i] = -1;
						}
						m_enabled[i] = 0;
					}
				}
				// Releasing reset
				else
				{
					for (int i = 0; i < 3; i++)
					{
						reload_count(i);
					}
				}

				m_status_reg = 0;
				update_interrupts();

				// Changing the clock source? (e.g. Zwackery)
				if (diffs & 0x02)
				{
					reload_count(idx);
				}
			}
			break;
		}

		case PTM_6840_MSBBUF1:
		case PTM_6840_MSBBUF2:
		case PTM_6840_MSBBUF3:
		{
//			PLOG(("MC6840 #%s msbbuf%d = %02X\n", tag(), offset / 2, data));
			m_msb_buffer = data;
			break;
		}

		case PTM_6840_LSB1:
		case PTM_6840_LSB2:
		case PTM_6840_LSB3:
		{
			int idx = (offset - 3) / 2;
			m_latch[idx] = (m_msb_buffer << 8) | (data & 0xff);

			// Clear the interrupt
			m_status_reg &= ~(1 << idx);
			update_interrupts();

			// Reload the count if in an appropriate mode
			if (!(m_control_reg[idx] & 0x10))
			{
				reload_count(idx);
			}

//			PLOG(("%s:MC6840 #%s: Counter %d latch = %04X\n", machine().describe_context(), tag(), idx, m_latch[idx]));
			break;
		}
	}
}

//-------------------------------------------------
//  timeout - Called if timer is mature
//-------------------------------------------------

void MC6840::timeout(int idx)
{
//	PLOG(("**ptm6840 %s t%d timeout**\n", tag(), idx));

	// Set the interrupt flag
	m_status_reg |= (1 << idx);
	m_status_read_since_int &= ~(1 << idx);
	update_interrupts();

	if ( m_control_reg[idx] & 0x80 )
	{
		if ((m_mode[idx] == 0)||(m_mode[idx] == 2))
		{
			m_output[idx] = m_output[idx] ? 0 : 1;
//			PLOG(("**ptm6840 %s t%d output %d **\n", tag(), idx, m_output[idx]));

			switch (idx)
			{
				case 0:
					m_out0_cb((offs_t)0, m_output[0]);
					break;
				case 1:
					m_out1_cb((offs_t)0, m_output[1]);
					break;
				case 2:
					m_out2_cb((offs_t)0, m_output[2]);
					break;
			}
		}
		if ((m_mode[idx] == 4)||(m_mode[idx] == 6))
		{
			if (!m_fired[idx])
			{
				m_output[idx] = 1;
//				PLOG(("**ptm6840 %s t%d output %d **\n", tag(), idx, m_output[idx]));

				switch (idx)
				{
					case 0:
						m_out0_cb((offs_t)0, m_output[0]);
						break;
					case 1:
						m_out1_cb((offs_t)0, m_output[1]);
						break;
					case 2:
						m_out2_cb((offs_t)0, m_output[2]);
						break;
				}

				// No changes in output until reinit
				m_fired[idx] = 1;

				m_status_reg |= (1 << idx);
				m_status_read_since_int &= ~(1 << idx);
				update_interrupts();
			}
		}
	}
	m_enabled[idx]= 0;
	reload_count(idx);
}

//-------------------------------------------------
//  set_gate - set gate status (0 or 1)
//-------------------------------------------------

void MC6840::set_gate(int idx, int state)
{
	if ((m_mode[idx] & 1) == 0)
	{
		if (state == 0 && m_gate[idx])
		{
			reload_count(idx);
		}
	}
	m_gate[idx] = state;
}

//-------------------------------------------------
//  set_clock - set clock status (0 or 1)
//-------------------------------------------------

void MC6840::set_clock(int idx, int state)
{

	if (!(m_control_reg[idx] & 0x02))
	{
		if (state && m_clk[idx] == 0)
		{
			tick(idx, 1);
		}
	}
	m_clk[idx] = state;
}

void MC6840::event_callback(int id, int err)
{
	m_timer[id] = -1;
	timeout(id);
}

void MC6840::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id)
	{
		case SIG_MC6840_CLOCK_0:
			set_clock(0, (data & mask) ? 1 : 0);
			break;
		case SIG_MC6840_CLOCK_1:
			set_clock(1, (data & mask) ? 1 : 0);
			break;
		case SIG_MC6840_CLOCK_2:
			set_clock(2, (data & mask) ? 1 : 0);
			break;
		case SIG_MC6840_GATE_0:
			set_gate(0, (data & mask) ? 1 : 0);
			break;
		case SIG_MC6840_GATE_1:
			set_gate(1, (data & mask) ? 1 : 0);
			break;
		case SIG_MC6840_GATE_2:
			set_gate(2, (data & mask) ? 1 : 0);
			break;
	}
}

#define STATE_VERSION	1

bool MC6840::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(m_control_reg, sizeof(m_control_reg), 1);
	state_fio->StateArray(m_output, sizeof(m_output), 1);
	state_fio->StateArray(m_gate, sizeof(m_gate), 1);
	state_fio->StateArray(m_clk, sizeof(m_clk), 1);
	state_fio->StateArray(m_enabled, sizeof(m_enabled), 1);
	state_fio->StateArray(m_mode, sizeof(m_mode), 1);
	state_fio->StateArray(m_fired, sizeof(m_fired), 1);
	state_fio->StateValue(m_t3_divisor);
	state_fio->StateValue(m_t3_scaler);
	state_fio->StateValue(m_IRQ);
	state_fio->StateValue(m_status_reg);
	state_fio->StateValue(m_status_read_since_int);
	state_fio->StateValue(m_lsb_buffer);
	state_fio->StateValue(m_msb_buffer);
	state_fio->StateArray(m_timer, sizeof(m_timer), 1);
	state_fio->StateArray(m_latch, sizeof(m_latch), 1);
	state_fio->StateArray(m_counter, sizeof(m_counter), 1);
	return true;
}

