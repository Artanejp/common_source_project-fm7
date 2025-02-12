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
#include <memory>

class EVENT_TEMPLATE : public DEVICE
{
protected:
	std::atomic<bool>    event_half;	//! Display second half of frame.
	std::atomic<bool>    driven_by_half;
	std::atomic<int64_t> frame_clocks;
public:
	EVENT_TEMPLATE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		event_half = false;
		driven_by_half = false;
		frame_clocks = 1000 * 1000; // 1MHz?
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
		if(driven_by_half.load()) {
			event_half = !(event_half.load());
		} else {
			event_half = false;
		}
		return event_half.load();
	}
	/*!
	 * @brief Checf whether a half of frame.
	 * @return true  : half of frame
	 *		   false : End (or begin) of frame. 
	 */
	bool is_half_event()
	{
		if(!(driven_by_half.load())) {
			return false;
		} else {
			return event_half.load();
		}
	}
	/*!
	 * @brief Check driven by a frame or harf of frame.
	 * @return true  : driven by half of a (0.5) frame.
	 *		   false : driven by a (1) frane.
	 */
	bool is_driven_by_half()
	{
		return driven_by_half.load();
	}
	
	/*!
	 * @brief Get next period by clocks.
	 * @return next period.
	 */
	inline int64_t get_next_period_clocks()
	{
		int64_t _fclocks;
		int64_t _clk = frame_clocks.load();
		if(driven_by_half.load()) {
			if(event_half.load()) {
				_fclocks = _clk - (_clk / 2);
			} else {
				_fclocks = _clk / 2;
			}
		} else {
			_fclocks = _clk;
		}
		__UNLIKELY_IF(_fclocks < 0) {
			_fclocks = 0;
		}
		__UNLIKELY_IF(_fclocks >= (INT32_MAX >> 5)) {
			_fclocks = (INT32_MAX >> 5) - 1;
		}
		return _fclocks;
	}
	/*!
	 * @brief Get next period by nanosecond..
	 * @return next period.
	 */
	inline int64_t get_next_period_nsec()
	{
		double _fps = get_frame_rate();
		__UNLIKELY_IF(_fps < 1.0) {
			_fps = 1.0;
		}
		double _nsec = (1.0e9 / _fps) ;
		if(driven_by_half.load()) {
			if(event_half.load()) {
				_nsec = _nsec - (_nsec / 2.0);
			} else {
				_nsec = _nsec / 2.0;
			}
		}
		
		__UNLIKELY_IF(_nsec < 1.0) {
			_nsec = 1.0;
		}
		__UNLIKELY_IF(_nsec >= 2.0e9) {
			_nsec = 2.0e9 - 1;
		}
		return llrint(_nsec);
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
	virtual int set_context_cpu(DEVICE* device, uint32_t clocks)
	{
		return 0;
	}
	/*!
	  @brief Register CPU to event manager (set DEFAULT value; CPU_CLOCKS defined by vm/foo/foo.h .)
	  @param device Pointer of CPU DEVICE to register.
	  @return index number of regitered. -1 if failed to register.
	*/
	virtual int set_context_cpu(DEVICE* device)
	{
		return set_context_cpu(device, 1000 * 1000);
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
