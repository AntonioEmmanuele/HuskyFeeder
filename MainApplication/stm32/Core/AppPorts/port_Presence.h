#ifndef	PORT_PRESENCE
#define	PORT_PRESENCE
/*********************
 * FILE DESCR:
 *
 *	This file is a portability layer that abstracts the presence sensor functions for the Husky Feeder App.
 *	It basically consists in a set of definitions and macros
 *********************/
#define	PRESENCE_STM32F446RE	1
#define HFEED_PRESENCE_GET_CURR_MILLIS 		HAL_GetTick()
//Soglia = 1m
#define SOGLIA 100

#if PRESENCE_STM32F446RE
#include "main.h"
#include "../AppDrivers/hcsr04/hcsr04_driver.h"
#endif

typedef struct {
	hcsr04_driver* Driver;
	uint8_t soglia;
	//Edit Emmo: last measurement time
	uint32_t last_time_measure;
	void start_measurement ();
	bool is_dog_present ();
} HFeed_PresenceManager;


#endif
