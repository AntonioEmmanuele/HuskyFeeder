/*
 * hcsr04_driver.h
 *
 *  Created on: 2 giu 2022
 *      Author: giuseppe
 */

#ifndef INC_HCSR04_DRIVER_H_
#define INC_HCSR04_DRIVER_H_

#include "../AppDrivers/mcu_defs.h"
#define _HCSR04DEBUG_ 1

#if _HCSR04DEBUG_
#include "string.h"
#include <stdio.h>
#endif

#define TIME_TO_CM 58
/* From one measurement to another we need to wait 60 ms*/
#define NEXT_MEASUREMENT_DELAY 60
/* Number of values to be used to calculate the average */
#define NUM_AVG 10


class hcsr04_driver {
  private:
  /* The pin used to trigger ic measurement*/
    pin_port trigger_pin_port;
    pin_number trigger_pin_num;
  /*last distance calculated in cm*/
    float last_distance_cm;
  /* Measurement values */
    uint32_t val1;
	uint32_t val2;
#if _HCSR04DEBUG_
	int avg;
#endif
	int avg_array[NUM_AVG];
	uint8_t counter_avg;

  public:
	//	Edit Emmo: This flag becomes true when the echo rising edge occurs and false when f.e. occurs
	bool is_measuring;
	bool valid;

    /*
     * Valori usati dalla callback del Timer in ICM per
     * calcolare la durata dell'onda quadra fornita
     * dal sensore
     */

	#if STM32_HAL
		TIM_HandleTypeDef* hcsr04_tim_handler;
		__ALWAYSINLINE void hcsr04_hal_stm42_set_timer(TIM_HandleTypeDef* handler){ hcsr04_tim_handler=handler;}
		 void hcsr04EchoReadAvg(TIM_HandleTypeDef *htim, pin_port, pin_number, UART_HandleTypeDef*);
	#endif
		 __ALWAYSINLINE void delay_Microseconds(uint32_t microseconds){
		if(microseconds==0)
			return;
	#if	STM32_HAL
		//	THE TIMER SHOULD TICK EVERY MICROSECOND, FREQ 1 MHZ !
		__HAL_TIM_SET_COUNTER(hcsr04_tim_handler,0);
		while(__HAL_TIM_GET_COUNTER(hcsr04_tim_handler)<microseconds);
	#endif
	}

    hcsr04_driver(pin_port, pin_number);
    void hcsr04Print(UART_HandleTypeDef*);
    void hcsr04Trigger(void);
    __ALWAYSINLINE float hcsr04GetDistance(void) {	return this->last_distance_cm;}
    bool hcsr04GetAvg (uint16_t& average);
    unsigned long hcsr04GetMicros(void);
    bool hcsr04CheckOOR(void);
    void hcsr04DelayNextMeasurement(void);

};


#endif /* INC_HCSR04_DRIVER_H_ */
