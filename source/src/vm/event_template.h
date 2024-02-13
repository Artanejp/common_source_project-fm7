/*
	Skelton for retropc emulator

	Author : Kyuma.Ohta
	Date   : 2024.02.14-

	[ event manager template]
	This is skelton of event manager.
	This MUST inherit to EVENT:: class.
*/

#pragma once
#include "./vm_template.h"
#include "./device.h"

class EVENT_TEMPLATE : public DEVICE
{
protected:
	bool event_half;	//! Display second half of frame.
	
public:
	EVENT_TEMPLATE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		event_half = false;
	}
	~EVENT_TEMPLATE() {}
	// unique functions
	/*!
	 * @brief Drive (run) a half frame.
	 * @return true  : half of frame
	 *		   false : End (or begin) of frame. 
	 * @note This is replacement of EVENT::drive().
	 */
	virtual bool drive()
	{
		event_half = !(event_half);
		return event_half;
	}
	/*!
	 * @brief Checf whether a half of frame.
	 * @return true  : half of frame
	 *		   false : End (or begin) of frame. 
	 */
	virtual bool is_half_event()
	{
		return event_half;
	}
	/*!
	  @brief Get frame rate of next frame period.
	  @return Frame rate by Hz.
	  @note This doesn't return FPS of this period.
	*/
	virtual double get_frame_rate()
	{
		return 59.94;
	}
	/*!
	  @brief Get time from start by micro-seconds.
	  @return current time by usec.
	*/
	virtual double get_current_usec()
	{
		return 0.0;
	}
	
	/*!
	  @brief Register CPU to event manager.
	  @param device Pointer of CPU DEVICE to register.
	  @param clocks Basic clock Hz of this device.
	  @return index number of regitered. -1 if failed to register.
	*/
	virtual int set_context_cpu(DEVICE* device, uint32_t clocks = 1000*1000)
	{
		return 0;
	}
	/*!
	  @brief Remove CPU from EVENT MANAGER.
	  @param device Device pointer requesting to remove.
	  @param num  CPU number requesting to remove.
	  @return true if success.
	  @note You can't remove CPU #0, because this is base of scheduling.
	  @note You should notify both num and device as same device.
	*/
	virtual bool remove_context_cpu(DEVICE* device, int num)
	{
		return false;
	}
	/*!
	  @brief Set CPU clocks for not primary CPU.
	  @param device Device pointer expect to set.
	  @param clocks expect CPU clock by Hz.
	  @note For CPU #0, this don't effect to.
	*/
	virtual void set_secondary_cpu_clock(DEVICE* device, uint32_t clocks)
	{
	}
	/*!
	  @brief Add device to sound source (to mix).
	  @param device Device pointer expect to add.
	  @note You can register devices less than MAX_SOUND.
	*/
	virtual void set_context_sound(DEVICE* device)
	{
	}
	/*!
	  @brief Check frame skippable.
	  @return true if avalable to skip.
	*/
	virtual bool is_frame_skippable()
	{
		return false;
	}

	virtual void initialize_sound(int rate, int samples)
	{
		
	}
	virtual uint16_t* __FASTCALL create_sound(int* extra_frames)
	{
		__LIKELY_IF(extra_frames != NULL) {
			*extra_frames = 0;
		}
		return NULL;
	}
	virtual int get_sound_buffer_ptr()
	{
		return 0;
	}

	
	virtual int rechannel_sound_in_data(int32_t*dst, int16_t* src, int dst_channels, int src_channels, int samples)
	{
		return 0;
	}


};
