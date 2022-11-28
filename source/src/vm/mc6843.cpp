/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2020.12.12-

	[ MC6843 / HD46503 ]
*/

// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2007

  Motorola 6843 Floppy Disk Controller emulation.

**********************************************************************/

/*
  Main MC 6843 features are:
   - single density floppies
   - IBM 3740 compatible
   - DMA-able
   - high-level commands (including multi-sector read/write)

   CLONES: HD 46503S seems to be a clone of MC 6843

   BUGS
   The driver was designed with Thomson computer emulation in mind
   (CD 90-015 5"1/4 floppy controller) and works in this context.
   It might work in other contexts but has currently shortcomings:
   - DMA is not emulated
   - Free-Format Read is not emulated
   - Free-Format Write only supports track formatting, in a specific
   format (FWF=1, Thomson-like sector formats)
   - very rough timing: basically, there is a fixed delay between
   a command request (CMR write) and its response (first byte
   available, seek complete, etc.); there is no delay between
   read / write
 */


#include "mc6843.h"
#include "disk.h"
#include "noise.h"

#define DRIVE_MASK	(MAX_DRIVE - 1)

#define EVENT_SEARCH	0
#define EVENT_SEEK	1
#define EVENT_DRQ	2
#define EVENT_INDEX	3

#define m_timer_cont_adjust(d) { \
	if (m_timer_id != -1) { \
		cancel_event(this, m_timer_id); \
		m_timer_id = -1; \
	} \
	register_event(this, EVENT_SEARCH, d, false, &m_timer_id); \
	fdc[m_drive].searching = true; \
	update_head_flag(m_drive, true); \
}

// /lib/formats/flopimg.h

/* sector has a deleted data address mark */
#define ID_FLAG_DELETED_DATA    0x0001
/* CRC error in id field */
#define ID_FLAG_CRC_ERROR_IN_ID_FIELD 0x0002
/* CRC error in data field */
#define ID_FLAG_CRC_ERROR_IN_DATA_FIELD 0x0004

/******************* parameters ******************/

/* macro-command numbers */
#define CMD_STZ 0x2 /* seek track zero */
#define CMD_SEK 0x3 /* seek */
#define CMD_SSR 0x4 /* single sector read */
#define CMD_SSW 0x5 /* single sector write */
#define CMD_RCR 0x6 /* read CRC */
#define CMD_SWD 0x7 /* single sector write with delete data mark */
#define CMD_MSW 0xd /* multiple sector write */
#define CMD_MSR 0xc /* multiple sector read */
#define CMD_FFW 0xb /* free format write */
#define CMD_FFR 0xa /* free format read */

/* coarse delays */
#define DELAY_SEEK   100 //attotime::from_usec( 100 )  /* track seek time */
#define DELAY_ADDR   100 //attotime::from_usec( 100 )  /* search-address time */



static const char *const mc6843_cmd[16] =
{
	"---", "---", "STZ", "SEK", "SSR", "SSW", "RCR", "SWD",
	"---", "---", "FFR", "FFW", "MSR", "MSW", "---", "---",
};




//DEFINE_DEVICE_TYPE(MC6843, mc6843_device, "mc5843", "Motorola MC6843 FDC")

//mc6843_device::mc6843_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
void MC6843::mc6843_device()
{
	m_CTAR = 0;
	m_CMR = 0;
	m_ISR = 0;
	m_SUR = 0;
	m_STRA = 0;
	m_STRB = 0;
	m_SAR = 0;
	m_GCR = 0;
	m_CCR = 0;
	m_LTAR = 0;
	m_drive = 0;
	m_side = 0;
	m_data_size = 0;
	m_data_idx = 0;
//	m_data_id = 0;
//	m_index_pulse = 0;
	m_index_clock = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void MC6843::device_start()
{
//	m_write_irq.resolve_safe();

//	m_timer_cont = timer_alloc(TIMER_CONT);
	m_timer_id = m_seek_id = -1;

//	save_item(NAME(m_CTAR));
//	save_item(NAME(m_CMR));
//	save_item(NAME(m_ISR));
//	save_item(NAME(m_SUR));
//	save_item(NAME(m_STRA));
//	save_item(NAME(m_STRB));
//	save_item(NAME(m_SAR));
//	save_item(NAME(m_GCR));
//	save_item(NAME(m_CCR));
//	save_item(NAME(m_LTAR));
//	save_item(NAME(m_drive));
//	save_item(NAME(m_side));
//	save_item(NAME(m_data));
//	save_item(NAME(m_data_size));
//	save_item(NAME(m_data_idx));
//	save_item(NAME(m_data_id));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void MC6843::device_reset()
{
//	LOG (( "mc6843 reset\n" ));

	/* setup/reset floppy drive */
//	for (auto &img : m_floppy)
//	{
//		if (img.found())
//		{
//			img->floppy_mon_w(CLEAR_LINE);
//			img->floppy_drive_set_ready_state(FLOPPY_DRIVE_READY, 0 );
//			img->floppy_drive_set_rpm( 300. );
//		}
//	}

	/* reset registers */
	m_CMR &= 0xf0; /* zero only command */
	m_ISR = 0;
	m_STRA &= 0x5c;
	m_SAR = 0;
	m_STRB &= 0x20;
	status_update( );

	m_data_size = 0;
//	m_data_idx = 0;
//	m_timer_cont->adjust( attotime::never );
	m_timer_id = -1;
	m_seek_id = -1;
}

/************************** floppy interface ****************************/



//legacy_floppy_image_device* mc6843_device::floppy_image( )
//{
//	assert(m_floppy[m_drive].found());
//	return m_floppy[m_drive].target();
//}


void MC6843::set_drive( int drive )
{
	m_drive = drive;
}



void MC6843::set_side( int side )
{
	m_side = side;
}



/* called after ISR or STRB has changed */
void MC6843::status_update( )
{
	int irq = 0;

	/* ISR3 */
	if ( (m_CMR & 0x40) || ! m_STRB )
		m_ISR &= ~8;
	else
		m_ISR |=  8;

	/* interrupts */
	if ( m_ISR & 4 )
		irq = 1; /* unmaskable */
	if ( ! (m_CMR & 0x80) )
	{
		/* maskable */
		if ( m_ISR & ~4 )
			irq = 1;
	}

//	m_write_irq( irq );
//	LOG( "status_update: irq=%i (CMR=%02X, ISR=%02X)\n", irq, m_CMR, m_ISR );
	write_signals(&outputs_irq, irq ? 0xffffffff : 0);
}


//void MC6843::set_index_pulse( int index_pulse )
//{
//	m_index_pulse = index_pulse;
//}


/* called at end of command */
void MC6843::cmd_end( )
{
	int cmd = m_CMR & 0x0f;
	if ( ( cmd == CMD_STZ ) || ( cmd == CMD_SEK ) )
	{
		m_ISR |= 0x02; /* set Settling Time Complete */
	}
	else
	{
		m_ISR |= 0x01;  /* set Macro Command Complete */
	}
	m_STRA &= ~0x80; /* clear Busy */
	m_CMR  &=  0xf0; /* clear command */
	status_update( );
	update_head_flag(m_drive, false);
}



/* Seek Track Zero bottom half */
void MC6843::finish_STZ( )
{
//	legacy_floppy_image_device* img = floppy_image( );
//	int i;

	/* seek to track zero */
//	for ( i=0; i<83; i++ )
//	{
//		if (img->floppy_tk00_r() == CLEAR_LINE)
//			break;
//		img->floppy_drive_seek( -1 );
//	}

//	LOG( "%f mc6843_finish_STZ: actual=%i\n", machine().time().as_double(), img->floppy_drive_get_current_track() );

	/* update state */
	m_CTAR = 0;
	m_GCR = 0;
	m_SAR = 0;
//	m_STRB |= img->floppy_tk00_r() << 4;
	m_STRB |= (fdc[m_drive].track != 0) << 4;

	cmd_end( );
}



/* Seek bottom half */
void MC6843::finish_SEK( )
{
//	legacy_floppy_image_device* img = floppy_image( );

	/* seek to track */
	// TODO: not sure how CTAR bit 7 is handled here, but this is the safest approach for now
//	img->floppy_drive_seek( m_GCR - (m_CTAR & 0x7F) );

//	LOG( "%f mc6843_finish_SEK: from %i to %i (actual=%i)\n", machine().time().as_double(), (m_CTAR & 0x7F), m_GCR, img->floppy_drive_get_current_track() );

	/* update state */
	m_CTAR = m_GCR;
	m_SAR = 0;
	cmd_end( );
}



/* preamble to all sector read / write commands, returns 1 if found */
int MC6843::address_search( chrn_id* id )
{
//	legacy_floppy_image_device* img = floppy_image( );
//	int r = 0;

	fdc[m_drive].searching = false;

#if 0
	if ( !(fdc[m_drive].track >= 0 && fdc[m_drive].track < 70 && disk[m_drive]->get_track(fdc[m_drive].track >> 1, m_side)) )
#else
	if ( !disk[m_drive]->get_track(m_LTAR, m_side) )
#endif
	{
		m_STRB |= 0x08; /* set Sector Address Undetected */
		cmd_end( );
		return 0;
	}

	for (int i = 0; i < disk[m_drive]->sector_num.sd; i++)
	{
		// fixme: need to get current head position to determin next sector
		int sector = (fdc[m_drive].sector++) % disk[m_drive]->sector_num.sd;

#if 0
		if ( !disk[m_drive]->get_sector(fdc[m_drive].track >> 1, m_side, sector) )
#else
		if ( !disk[m_drive]->get_sector(m_LTAR, m_side, sector) )
#endif
		{
			/* read address error */
//			LOG( "%f mc6843_address_search: get_next_id failed\n", machine().time().as_double() );
			m_STRB |= 0x0a; /* set CRC error & Sector Address Undetected */
			cmd_end( );
			return 0;
		}
		id->C = disk[m_drive]->id[0];
		id->H = disk[m_drive]->id[1];
		id->R = disk[m_drive]->id[2];
		id->N = disk[m_drive]->id[3];
		id->flags = 0;

		if ( disk[m_drive]->deleted )
			id->flags |= ID_FLAG_DELETED_DATA;
		if ( disk[m_drive]->addr_crc_error )
			id->flags |= ID_FLAG_CRC_ERROR_IN_ID_FIELD;
		if ( disk[m_drive]->data_crc_error )
			id->flags |= ID_FLAG_CRC_ERROR_IN_DATA_FIELD;

		if ( ( id->flags & ID_FLAG_CRC_ERROR_IN_ID_FIELD ) || ( id->N != 0 ) )
		{
			/* read address error */
//			LOG( "%f mc6843_address_search: get_next_id failed\n", machine().time().as_double() );
			m_STRB |= 0x0a; /* set CRC error & Sector Address Undetected */
			cmd_end( );
			return 0;
		}

		if ( id->C != m_LTAR )
		{
			/* track mismatch */
//			LOG( "%f mc6843_address_search: track mismatch: logical=%i real=%i\n", machine().time().as_double(), m_LTAR, id->C );
			m_data[0] = id->C; /* make the track number available to the CPU */
			m_STRA |= 0x20;    /* set Track Not Equal */
			cmd_end( );
			return 0;
		}

		if ( id->R == m_SAR )
		{
			/* found! */
//			LOG( "%f mc6843_address_search: sector %i found on track %i\n", machine().time().as_double(), id->R, id->C );
			if ( ! (m_CMR & 0x20) )
			{
				m_ISR |= 0x04; /* if no DMA, set Status Sense */
			}
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC: FOUND C=%02X H=%02X R=%02X N=%02X\n"), id->C, id->H, id->R, id->N);
#endif
			return 1;
		}
	}

//		if ( img->floppy_drive_get_flag_state( FLOPPY_DRIVE_INDEX ) )
//		{
//			r++;
//			if ( r >= 4 )
//			{
				/* time-out after 3 full revolutions */
//				LOG( "%f mc6843_address_search: no sector %i found after 3 revolutions\n", machine().time().as_double(), m_SAR );
				m_STRB |= 0x08; /* set Sector Address Undetected */
				cmd_end( );
				return 0;
//			}
//		}

	//return 0; /* unreachable */
}



/* preamble specific to read commands (adds extra checks) */
int MC6843::address_search_read( chrn_id* id )
{
	if ( ! address_search( id ) )
		return 0;

	if ( id->flags & ID_FLAG_CRC_ERROR_IN_DATA_FIELD )
	{
//		LOG( "%f mc6843_address_search_read: data CRC error\n", machine().time().as_double() );
		m_STRB |= 0x06; /* set CRC error & Data Mark Undetected */
		cmd_end( );
		return 0;
	}

	if ( id->flags & ID_FLAG_DELETED_DATA )
	{
//		LOG( "%f mc6843_address_search_read: deleted data\n", machine().time().as_double() );
		m_STRA |= 0x02; /* set Delete Data Mark Detected */
	}

	return 1;
}




/* Read CRC bottom half */
void MC6843::finish_RCR( )
{
	chrn_id id;
	if ( ! address_search_read( &id ) )
		return;
	cmd_end( );
}



/* Single / Multiple Sector Read bottom half */
void MC6843::cont_SR( )
{
	chrn_id id;
//	legacy_floppy_image_device* img = floppy_image( );

	/* sector seek */
	if ( ! address_search_read( &id ) )
		return;

	/* sector read */
//	img->floppy_drive_read_sector_data( m_side, id.data_id, m_data, 128 );
	memcpy(m_data, disk[m_drive]->sector, 128);
	m_data_idx = 0;
	m_data_size = 128;
	m_STRA |= 0x01;     /* set Data Transfer Request */
	status_update( );
}



/* Single / Multiple Sector Write bottom half */
void MC6843::cont_SW( )
{
	chrn_id id;

	/* sector seek */
	if ( ! address_search( &id ) )
		return;

	/* setup sector write buffer */
	m_data_idx = 0;
	m_data_size = 128;
	m_STRA |= 0x01;         /* set Data Transfer Request */
//	m_data_id = id.data_id; /* for subsequent write sector command */
	status_update( );
}



/* bottom halves, called to continue / finish a command after some delay */
//void MC6843::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
void MC6843::device_timer()
{
//	switch (id)
//	{
//		case TIMER_CONT:
//			{
				int cmd = m_CMR & 0x0f;

//				LOG( "%f mc6843_cont: timer called for cmd=%s(%i)\n", machine().time().as_double(), mc6843_cmd[cmd], cmd );

//				m_timer_cont->adjust( attotime::never );

				switch ( cmd )
				{
//					case CMD_STZ: finish_STZ( ); break;
//					case CMD_SEK: finish_SEK( ); break;
					case CMD_SSR: cont_SR( );    break;
					case CMD_SSW: cont_SW( );    break;
					case CMD_RCR: finish_RCR( ); break;
					case CMD_SWD: cont_SW( );    break;
					case CMD_MSW: cont_SW( );    break;
					case CMD_MSR: cont_SR( );    break;
				}
//			}
//			break;

//		default:
//			break;
//	}
}



/************************** CPU interface ****************************/



uint8_t MC6843::read(offs_t offset)
{
	uint8_t data = 0;

	switch ( offset ) {
	case 0: /* Data Input Register (DIR) */
	{
		int cmd = m_CMR & 0x0f;
		int data_index = m_data_idx;

//		LOG( "%f %s mc6843_r: data input cmd=%s(%i), pos=%i/%i, GCR=%i, ",
//				machine().time().as_double(), machine().describe_context(),
//				mc6843_cmd[cmd], cmd, m_data_idx,
//				m_data_size, m_GCR );

		if ( cmd == CMD_SSR || cmd == CMD_MSR )
		{
			/* sector read */
			assert( m_data_size > 0 );
			assert( m_data_idx < m_data_size );
			assert( m_data_idx < sizeof(m_data) );
			data = m_data[ m_data_idx ];
			m_data_idx++;
			fdc[m_drive].access = true;

			if ( m_data_idx >= m_data_size )
			{
				/* end of sector read */

				m_STRA &= ~0x01; /* clear Data Transfer Request */
				write_signals(&outputs_drq, 0);

				if ( cmd == CMD_MSR )
				{
					/* schedule next sector in multiple sector read */
					m_GCR--;
					m_SAR++;
					if ( m_GCR == 0xff )
					{
						cmd_end( );
					}
					else if ( m_SAR > 26 )

					{
						m_STRB |= 0x08; /* set Sector Address Undetected */
						cmd_end( );
					}
					else
					{
//						m_timer_cont->adjust( DELAY_ADDR );
						m_timer_cont_adjust( DELAY_ADDR );
					}
				}
				else
				{
					cmd_end( );
				}
			}
		}
		else if ( cmd == 0 )
		{
			data = m_data[0];
		}
		else
		{
			/* XXX TODO: other read modes */
			data = m_data[0];
//			logerror( "%s mc6843 read in unsupported command mode %i\n", machine().describe_context(), cmd );
		}

//		LOG( "data=%02X\n", data );

#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: IN DIR=%02X IDX=%02X\n"), data, data_index);
#endif
		break;
	}

	case 1: /* Current-Track Address Register (CTAR) */
		data = m_CTAR;
//		LOG( "%f %s mc6843_r: read CTAR %i (actual=%i)\n",
//				machine().time().as_double(), machine().describe_context(), data,
//				floppy_image()->floppy_drive_get_current_track());
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: IN CTAR=%02X\n"), data);
#endif
		break;

	case 2: /* Interrupt Status Register (ISR) */
		data = m_ISR;
//		LOG( "%f %s mc6843_r: read ISR %02X: cmd=%scomplete settle=%scomplete sense-rq=%i STRB=%i\n",
//				machine().time().as_double(), machine().describe_context(), data,
//				(data & 1) ? "" : "not-" , (data & 2) ? "" : "not-",
//				(data >> 2) & 1, (data >> 3) & 1 );

		/* reset */
		m_ISR &= 8; /* keep STRB */
		status_update( );
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: IN ISR=%02X\n"), data);
#endif
		break;

	case 3: /* Status Register A (STRA) */
	{
		/* update */
//		legacy_floppy_image_device* img = floppy_image( );
//		int flag = img->floppy_drive_get_flag_state( FLOPPY_DRIVE_READY);
		m_STRA &= 0xa3;
//		if ( flag & FLOPPY_DRIVE_READY )
		if ( disk[m_drive]->inserted )
			m_STRA |= 0x04;

//		m_STRA |= !img->floppy_tk00_r() << 3;
		m_STRA |= (disk[m_drive]->inserted && fdc[m_drive].track == 0) << 3;
//		m_STRA |= !img->floppy_wpt_r() << 4;
		m_STRA |= (disk[m_drive]->inserted && disk[m_drive]->write_protected) << 4;

		// index hole signal width is 5msec (thanks Mr.Sato)
//		if ( m_index_pulse )
		if ( disk[m_drive]->inserted && get_passed_usec(m_index_clock) < 5000 )
			m_STRA |= 0x40;

		data = m_STRA;
//		LOG( "%f %s mc6843_r: read STRA %02X: data-rq=%i del-dta=%i ready=%i t0=%i wp=%i trk-dif=%i idx=%i busy=%i\n",
//				machine().time().as_double(), machine().describe_context(), data,
//				data & 1, (data >> 1) & 1, (data >> 2) & 1, (data >> 3) & 1,
//				(data >> 4) & 1, (data >> 5) & 1, (data >> 6) & 1, (data >> 7) & 1 );
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: IN STRA=%02X\n"), data);
#endif
		break;
	}

	case 4: /* Status Register B (STRB) */
		data = m_STRB;
//		LOG( "%f %s mc6843_r: read STRB %02X: data-err=%i CRC-err=%i dta--mrk-err=%i sect-mrk-err=%i seek-err=%i fi=%i wr-err=%i hard-err=%i\n",
//				machine().time().as_double(), machine().describe_context(), data,
//				data & 1, (data >> 1) & 1, (data >> 2) & 1, (data >> 3) & 1,
//				(data >> 4) & 1, (data >> 5) & 1, (data >> 6) & 1, (data >> 7) & 1 );

		/* (partial) reset */
		m_STRB &= ~0xfb;
		status_update( );
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: IN STRB=%02X\n"), data);
#endif
		break;

	case 7: /* Logical-Track Address Register (LTAR) */
		data = m_LTAR;
//		LOG( "%f %s mc6843_r: read LTAR %i (actual=%i)\n",
//				machine().time().as_double(), machine().describe_context(), data,
//				floppy_image()->floppy_drive_get_current_track());
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: IN LTAR=%02X\n"), data);
#endif
		break;

//	default:
//		logerror( "%s mc6843 invalid read offset %i\n", machine().describe_context(), offset );
	}

	return data;
}

void MC6843::write(offs_t offset, uint8_t data)
{
	switch ( offset ) {
	case 0: /* Data Output Register (DOR) */
	{
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: OUT DOR=%02X IDX=%02X\n"), data, m_data_idx);
#endif
		int cmd = m_CMR & 0x0f;
		int FWF = (m_CMR >> 4) & 1;

//		LOG( "%f %s mc6843_w: data output cmd=%s(%i), pos=%i/%i, GCR=%i, data=%02X\n",
//				machine().time().as_double(), machine().describe_context(),
//				mc6843_cmd[cmd], cmd, m_data_idx,
//				m_data_size, m_GCR, data );

		if ( cmd == CMD_SSW || cmd == CMD_MSW || cmd == CMD_SWD )
		{
			/* sector write */
			assert( m_data_size > 0 );
			assert( m_data_idx < m_data_size );
			assert( m_data_idx < sizeof(m_data) );
			m_data[ m_data_idx ] = data;
			m_data_idx++;
			fdc[m_drive].access = true;

			if ( m_data_idx >= m_data_size )
			{
				/* end of sector write */
//				legacy_floppy_image_device* img = floppy_image( );

//				LOG( "%f %s mc6843_w: write sector %i\n", machine().time().as_double(), machine().describe_context(), m_data_id );

//				img->floppy_drive_write_sector_data(
//					m_side, m_data_id,
//					m_data, m_data_size,
//					(cmd == CMD_SWD) ? ID_FLAG_DELETED_DATA : 0 );
				if (!disk[m_drive]->write_protected) {
					memcpy(disk[m_drive]->sector, m_data, m_data_size);
					disk[m_drive]->set_deleted(cmd == CMD_SWD);
				}

				m_STRA &= ~0x01; /* clear Data Transfer Request */
				write_signals(&outputs_drq, 0);

				if (disk[m_drive]->write_protected)
				{
					m_STRB |= 0x40; /* set Write Error */
					cmd_end( );
				}
				else if ( cmd == CMD_MSW )
				{
					m_GCR--;
					m_SAR++;
					if ( m_GCR == 0xff )
					{
						cmd_end( );
					}
					else if ( m_SAR > 26 )

					{
						m_STRB |= 0x08; /* set Sector Address Undetected */
						cmd_end( );
					}
					else
					{
//						m_timer_cont->adjust( DELAY_ADDR );
						m_timer_cont_adjust( DELAY_ADDR );
					}
				}
				else
				{
					cmd_end( );
				}
			}
		}
		else if ( (cmd == CMD_FFW) && FWF )
		{
			/* assume we are formatting */
			uint8_t nibble;
			nibble =
				(data & 0x01) |
				((data & 0x04) >> 1 )|
				((data & 0x10) >> 2 )|
				((data & 0x40) >> 3 );

			assert( m_data_idx < sizeof(m_data) );

			m_data[m_data_idx / 2] =
				(m_data[m_data_idx / 2] << 4) | nibble;

			if ( (m_data_idx == 0) && (m_data[0] == 0xfe ) )
			{
				/* address mark detected */
				m_data_idx = 2;
			}
			else if ( m_data_idx == 9 )
			{
				/* address id field complete */
				if ( (m_data[2] == 0) && (m_data[4] == 0) )
				{
					/* valid address id field */
//					legacy_floppy_image_device* img = floppy_image( );
					uint8_t track  = m_data[1];
					uint8_t sector = m_data[3];
					uint8_t filler = 0xe5; /* standard Thomson filler */
//					LOG( "%f %s mc6843_w: address id detected track=%i sector=%i\n", machine().time().as_double(), machine().describe_context(), track, sector);
//					img->floppy_drive_format_sector( m_side, sector, track, 0, sector, 0, filler );
				}
				else
				{
					/* abort */
					m_data_idx = 0;
				}
			}
			else if ( m_data_idx > 0 )
			{
				/* accumulate address id field */
				m_data_idx++;
			}
		}
		else if ( cmd == 0 )
		{
			/* nothing */
		}
		else
		{
			/* XXX TODO: other write modes */
//			logerror( "%s mc6843 write %02X in unsupported command mode %i (FWF=%i)\n", machine().describe_context(), data, cmd, FWF );
		}
		break;
	}

	case 1: /* Current-Track Address Register (CTAR) */
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: OUT CTAR=%02X\n"), data);
#endif
		m_CTAR = data;
//		LOG( "%f %s mc6843_w: set CTAR to %i %02X (actual=%i) \n",
//				machine().time().as_double(), machine().describe_context(), m_CTAR, data,
//				floppy_image()->floppy_drive_get_current_track());
		break;

	case 2: /* Command Register (CMR) */
	{
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: OUT CMR=%02X %s\n"), data, mc6843_cmd[data & 15]);
#endif
		int cmd = data & 15;

//		LOG( "%f %s mc6843_w: set CMR to $%02X: cmd=%s(%i) FWF=%i DMA=%i ISR3-intr=%i fun-intr=%i\n",
//				machine().time().as_double(), machine().describe_context(),
//				data, mc6843_cmd[cmd], cmd, (data >> 4) & 1, (data >> 5) & 1,
//				(data >> 6) & 1, (data >> 7) & 1 );

		/* sanitize state */
		m_STRA &= ~0x81; /* clear Busy & Data Transfer Request */
		m_data_idx = 0;
		m_data_size = 0;

		/* commands are initiated by updating some flags and scheduling
		   a bottom-half (mc6843_cont) after some delay */

		switch (cmd)
		{
		case CMD_SSW:
		case CMD_SSR:
		case CMD_SWD:
		case CMD_RCR:
		case CMD_MSR:
		case CMD_MSW:
			m_STRA |=  0x80; /* set Busy */
			m_STRA &= ~0x22; /* clear Track Not Equal & Delete Data Mark Detected */
			m_STRB &= ~0x04; /* clear Data Mark Undetected */
//			m_timer_cont->adjust( DELAY_ADDR );
			m_timer_cont_adjust( DELAY_ADDR );
			break;
		case CMD_STZ:
		case CMD_SEK:
			m_STRA |= 0x80; /* set Busy */
//			m_timer_cont->adjust( DELAY_SEEK );
			if (m_seek_id != -1) {
				cancel_event(this, m_seek_id);
			}
			register_event(this, EVENT_SEEK, 64 * ((m_SUR >> 4) + 1), false, &m_seek_id);
			// set target track number
			if(cmd == CMD_STZ) {
				fdc[m_drive].target_track = 0;
			} else {
				fdc[m_drive].target_track = fdc[m_drive].track + m_GCR - (m_CTAR & 0x7F);
			}
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC: SEEK DRIVE=%d TARGET=%d\n"), m_drive, fdc[m_drive].target_track);
#endif
			break;
		case CMD_FFW:
		case CMD_FFR:
			m_data_idx = 0;
			m_STRA |= 0x01; /* set Data Transfer Request */
			break;
		}

		m_CMR = data;
		status_update( );
		break;
	}

	case 3: /* Set-Up Register (SUR) */
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: OUT SUR=%02X\n"), data);
#endif
		m_SUR = data;

		/* assume CLK freq = 1MHz (IBM 3740 compatibility) */
//		LOG( "%f %s mc6843_w: set SUR to $%02X: head settling time=%fms, track-to-track seek time=%f\n",
//				machine().time().as_double(), machine().describe_context(),
//				data, 4.096 * (data & 15), 1.024 * ((data >> 4) & 15) );
		break;

	case 4: /* Sector Address Register (SAR) */
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: OUT SAR=%02X\n"), data);
#endif
		m_SAR = data & 0x1f;
//		LOG( "%f %s mc6843_w: set SAR to %i (%02X)\n", machine().time().as_double(), machine().describe_context(), m_SAR, data );
		break;

	case 5: /* General Count Register (GCR) */
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: OUT GCR=%02X\n"), data);
#endif
		m_GCR = data & 0x7f;
//		LOG( "%f %s mc6843_w: set GCR to %i (%02X)\n", machine().time().as_double(), machine().describe_context(), m_GCR, data );
		break;

	case 6: /* CRC Control Register (CCR) */
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: OUT CCR=%02X\n"), data);
#endif
		m_CCR = data & 3;
//		LOG( "%f %s mc6843_w: set CCR to %02X: CRC=%s shift=%i\n",
//				machine().time().as_double(), machine().describe_context(), data,
//				(data & 1) ? "enabled" : "disabled", (data >> 1) & 1 );
		break;

	case 7: /* Logical-Track Address Register (LTAR) */
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: OUT LTAR=%02X\n"), data);
#endif
		m_LTAR = data & 0x7f;
//		LOG( "%f %s mc6843_w: set LTAR to %i %02X (actual=%i)\n",
//				machine().time().as_double(), machine().describe_context(), m_LTAR, data,
//				floppy_image()->floppy_drive_get_current_track());
		break;

//	default:
//		logerror( "%s mc6843 invalid write offset %i (data=$%02X)\n", machine().describe_context(), offset, data );
	}
}

// ----------------------------------------------------------------------------
// common source code project functions
// ----------------------------------------------------------------------------

void MC6843::initialize()
{
	// initialize d88 handler
	for(int i = 0; i < MAX_DRIVE; i++) {
		disk[i] = new DISK(emu);
		disk[i]->drive_type = DRIVE_TYPE_2D;
		disk[i]->drive_rpm = 300;
		disk[i]->drive_mfm = false;
		disk[i]->set_device_name(_T("%s/Disk #%d"), this_device_name, i + 1);
	}
	memset(fdc, 0, sizeof(fdc));
	
	// initialize noise
	if(d_noise_seek != NULL) {
		d_noise_seek->set_device_name(_T("Noise Player (FDD Seek)"));
		if(!d_noise_seek->load_wav_file(_T("FDDSEEK.WAV"))) {
			if(!d_noise_seek->load_wav_file(_T("FDDSEEK1.WAV"))) {
				d_noise_seek->load_wav_file(_T("SEEK.WAV"));
			}
		}
		d_noise_seek->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_down != NULL) {
		d_noise_head_down->set_device_name(_T("Noise Player (FDD Head Load)"));
		d_noise_head_down->load_wav_file(_T("HEADDOWN.WAV"));
		d_noise_head_down->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_up != NULL) {
		d_noise_head_up->set_device_name(_T("Noise Player (FDD Head Unload)"));
		d_noise_head_up->load_wav_file(_T("HEADUP.WAV"));
		d_noise_head_up->set_mute(!config.sound_noise_fdd);
	}
	
	mc6843_device();
	device_start();
	
	// register event for raise drq and index hole signals
	register_event(this, EVENT_DRQ, disk[0]->get_usec_per_bytes(1), true, NULL);
	register_event(this, EVENT_INDEX, disk[0]->get_usec_per_track(), true, NULL);
}

void MC6843::release()
{
	// release d88 handler
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}

void MC6843::reset()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc[i].searching = false;
		fdc[i].access = false;
//		fdc[i].head_load = false;
		update_head_flag(i, false);
	}
	device_reset();
}

void MC6843::write_io8(uint32_t addr, uint32_t data)
{
	this->write(addr & 7, data);
}

uint32_t MC6843::read_io8(uint32_t addr)
{
	return this->read(addr & 7);
}

void MC6843::write_dma_io8(uint32_t addr, uint32_t data)
{
	this->write(0, data);
}

uint32_t MC6843::read_dma_io8(uint32_t addr)
{
	return this->read(0);
}

void MC6843::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MC6843_DRIVEREG) {
		this->set_drive(data & DRIVE_MASK);
	} else if(id == SIG_MC6843_SIDEREG) {
		this->set_side((data & mask) ? 1 : 0);
	}
}

uint32_t MC6843::read_signal(int ch)
{
	if(ch == SIG_MC6843_DRIVEREG) {
		return m_drive & DRIVE_MASK;
	} else if(ch == SIG_MC6843_SIDEREG) {
		return m_side & 1;
	}
	
	// get access status
	uint32_t stat = 0;
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(fdc[i].searching || fdc[i].access) {
			stat |= 1 << i;
		}
		fdc[i].access = false;
	}
	return stat;
}

void MC6843::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_SEARCH:
		m_timer_id = -1;
		device_timer();
		break;
	case EVENT_SEEK:
		if(fdc[m_drive].track > fdc[m_drive].target_track) {
			fdc[m_drive].track--;
			if(d_noise_seek != NULL) d_noise_seek->play();
		} else if(fdc[m_drive].track < fdc[m_drive].target_track) {
			fdc[m_drive].track++;
			if(d_noise_seek != NULL) d_noise_seek->play();
		}
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: SEEK DRIVE=%d TARGET=%d TRACK=%d\n"), m_drive, fdc[m_drive].target_track, fdc[m_drive].track);
#endif
		if(fdc[m_drive].track == fdc[m_drive].target_track) {
			switch ( m_CMR & 0x0f )
			{
				case CMD_STZ: finish_STZ( ); break;
				case CMD_SEK: finish_SEK( ); break;
			}
			m_seek_id = -1;
		} else {
			register_event(this, EVENT_SEEK, 64 * ((m_SUR >> 4) + 1), false, &m_seek_id);
		}
		break;
	case EVENT_DRQ:
		if((m_CMR & 0x20) && (m_STRA & 1)) {
			write_signals(&outputs_drq, 0xffffffff);
		}
		break;
	case EVENT_INDEX:
		m_index_clock = get_current_clock();
		break;
	}
}

void MC6843::update_head_flag(int drv, bool head_load)
{
	if(fdc[drv].head_load != head_load) {
		if(head_load) {
			if(d_noise_head_down != NULL) d_noise_head_down->play();
		} else {
			if(d_noise_head_up != NULL) d_noise_head_up->play();
		}
		fdc[drv].head_load = head_load;
	}
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void MC6843::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->open(file_path, bank);
	}
}

void MC6843::close_disk(int drv)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->close();
		update_head_flag(drv, false);
	}
}

bool MC6843::is_disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

void MC6843::is_disk_protected(int drv, bool value)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->write_protected = value;
	}
}

bool MC6843::is_disk_protected(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->write_protected;
	}
	return false;
}

uint8_t MC6843::get_media_type(int drv)
{
	if(drv < MAX_DRIVE) {
		if(disk[drv]->inserted) {
			return disk[drv]->media_type;
		}
	}
	return MEDIA_TYPE_UNK;
}

void MC6843::set_drive_type(int drv, uint8_t type)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_type = type;
	}
}

uint8_t MC6843::get_drive_type(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->drive_type;
	}
	return DRIVE_TYPE_UNK;
}

void MC6843::set_drive_rpm(int drv, int rpm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_rpm = rpm;
	}
}

void MC6843::set_drive_mfm(int drv, bool mfm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_mfm = mfm;
	}
}

void MC6843::set_track_size(int drv, int size)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->track_size = size;
	}
}

void MC6843::update_config()
{
	if(d_noise_seek != NULL) {
		d_noise_seek->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_down != NULL) {
		d_noise_head_down->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_up != NULL) {
		d_noise_head_up->set_mute(!config.sound_noise_fdd);
	}
}

#define STATE_VERSION	1

bool MC6843::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < array_length(fdc); i++) {
		state_fio->StateValue(fdc[i].target_track);
		state_fio->StateValue(fdc[i].track);
		state_fio->StateValue(fdc[i].sector);
		state_fio->StateValue(fdc[i].head_load);
	}
	for(int i = 0; i < array_length(disk); i++) {
		if(!disk[i]->process_state(state_fio, loading)) {
			return false;
		}
	}
	state_fio->StateValue(m_CTAR);
	state_fio->StateValue(m_CMR);
	state_fio->StateValue(m_ISR);
	state_fio->StateValue(m_SUR);
	state_fio->StateValue(m_STRA);
	state_fio->StateValue(m_STRB);
	state_fio->StateValue(m_SAR);
	state_fio->StateValue(m_GCR);
	state_fio->StateValue(m_CCR);
	state_fio->StateValue(m_LTAR);
	state_fio->StateValue(m_drive);
	state_fio->StateValue(m_side);
	state_fio->StateArray(m_data, sizeof(m_data), 1);
	state_fio->StateValue(m_data_size);
	state_fio->StateValue(m_data_idx);
//	state_fio->StateValue(m_data_id);
	state_fio->StateValue(m_index_clock);
	state_fio->StateValue(m_timer_id);
	state_fio->StateValue(m_seek_id);
	return true;
}
