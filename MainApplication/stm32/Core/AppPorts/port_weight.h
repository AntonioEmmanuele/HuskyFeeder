#ifndef PORT_WEIGHT
#define PORT_WEIGHT
#define HFEED_USE_HX711	1
#if HFEED_USE_HX711
#include "../AppDrivers/hx711/hx711_driver.h"
#endif
#define __AINL  __attribute__((always_inline)) inline
#define HFEED_WM_MEASURE_DOUBLE	0
struct HFeed_WeightManager{
#if HFEED_USE_HX711
	HX711_Driver *driver_ptr;
#endif
	// Function that should be called from main
	__AINL bool set_ptr(HX711_Driver *ptr){
#if HFEED_USE_HX711
		if (ptr!=0){
			driver_ptr=ptr;
			return true;
		}
#endif
		return false;
	}
#if HFEED_WM_MEASURE_DOUBLE
	// Performs a measurement
	__AINL bool get_measure(uint32_t ms_timeout,double &data,uint16_t samples){
		bool to_ret=false;
#if HFEED_USE_HX711
		data=(*driver_ptr).weigh_Object(ms_timeout,to_ret,samples);
#endif
		return to_ret;
	}
#else
	// Performs a measurement
	__AINL bool get_measure(uint32_t ms_timeout,uint32_t &data,uint16_t samples){
		bool to_ret=false;
#if HFEED_USE_HX711
		data=(uint32_t)(*driver_ptr).weigh_Object(ms_timeout,to_ret,samples);
#endif
		return to_ret;
	}
#endif
};
#endif
