/*
 * port_Presence.cpp
 *
 *  Created on: Jun 3, 2022
 *      Author: giuseppe
 */

#include "port_Presence.h"

void HFeed_PresenceManager::start_measurement () {
	if (HFEED_PRESENCE_GET_CURR_MILLIS - last_time_measure >= 60) {
		Driver->hcsr04Trigger();
		last_time_measure=HFEED_PRESENCE_GET_CURR_MILLIS;
	}
	//	busy waiting is blasphemy, why don't we work with polling ad this point?
	//Driver->hcsr04DelayNextMeasurement();
}

bool HFeed_PresenceManager::is_dog_present () {
	uint16_t media = -1;
	if (Driver->hcsr04GetAvg(media)) {
		if (media < soglia && media > 0) {
			return true;
		}
	}
	else{
		// Edit Emmo: If it is not measuring and 60 ms has passed since the last measurement
		if(Driver->is_measuring==false && HFEED_PRESENCE_GET_CURR_MILLIS - last_time_measure >=60)
			start_measurement();
	}
	return false;
}


