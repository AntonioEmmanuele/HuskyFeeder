/*
 * hcsr04_driver.cpp
 *
 *  Created on: 2 giu 2022
 *      Author: giuseppe
 */

#include "hcsr04_driver.h"

/*
* @brief: Constructor
* @input: echo_pin, trigger_pin
*/
hcsr04_driver::hcsr04_driver(pin_port trig_pin_port, pin_number trig_pin_num) {
	/*
	 * Setting fatto su cubemx dei pin
	 */

	this->trigger_pin_num = trig_pin_num;
	this->trigger_pin_port = trig_pin_port;

	this->last_distance_cm=-1;
	this->val1 = -1;
	this->val2 = -1;
#if _HCSR04DEBUG_
	this->avg = -1;
#endif
	this->avg_array[NUM_AVG] = {-1};
	this->counter_avg = 0;
	this->valid = false;
	//	Edit Emmo
	this->is_measuring=false;
}

/*
* @brief: Prints the last distance measured from an obstacule and the time where the echo_pin was high (read_time)
* @in : huart -> uart object to prins serial string
* @pre: The Serial Object must be previously initialized (in the setup())
*/

void hcsr04_driver::hcsr04Print(UART_HandleTypeDef* huart) {

#if _HCSR04DEBUG_

	char u_buff[256];

	sprintf(u_buff, "[LAST AVG]: %d\r\n", this->avg);
	HAL_UART_Transmit(huart, (uint8_t*)u_buff, strlen(u_buff),100);

#endif
}

/*
* @brief:This function sets trigger pin high for 10 microseconds starting the IC Operations.
*/
void hcsr04_driver::hcsr04Trigger(void){
	PIN_WRITE(this->trigger_pin_port,this->trigger_pin_num,PIN_HIGH);
	delay_Microseconds(10);
	PIN_WRITE(this->trigger_pin_port,this->trigger_pin_num,PIN_LOW);
}
#if STM32_HAL
/*
* @brief: polls echo pin. When echo pin becomes high it calculates the value of time of his high value and then performs conversions.
* @brief: If echo doesn't became high then return error ECHO_TIME_ELAPSE error
* @return : false if there is a problem.
*/
void hcsr04_driver::hcsr04EchoReadAvg(TIM_HandleTypeDef *htim, pin_port port, pin_number num, UART_HandleTypeDef* huart = nullptr){
#if _HCSR04DEBUG_
	char u_buff [256];
#endif

	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)  // if the interrupt source is channel1
		{
			if (HAL_GPIO_ReadPin(port, num) == PIN_HIGH) // if the first value is not captured
			{
#if _HCSR04DEBUG_
				//sprintf(u_buff, "************ RISING EDGE *********** \n\r");
				//HAL_UART_Transmit(huart, (uint8_t*)u_buff, strlen(u_buff),10);
#endif
				this->val1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); // read the first value
				//Edit EMMO: RISING FLAG SET
				this->is_measuring=true;
			}

			else if (HAL_GPIO_ReadPin(port, num) == PIN_LOW)  // if the first is already captured
			{
#if _HCSR04DEBUG_
				//sprintf(u_buff, "************ FALLING EDGE *********** \n\r");
				//HAL_UART_Transmit(huart, (uint8_t*)u_buff, strlen(u_buff),10);
#endif
				this->val2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);  // read second value
				this->last_distance_cm = (this->val2 - this->val1)/TIME_TO_CM;
				this->avg_array[counter_avg]=this->last_distance_cm;
				counter_avg++;

				if (this->counter_avg == NUM_AVG/2)
					valid = false;

				if (this->counter_avg == NUM_AVG) {
					this->counter_avg = 0;
					this->valid = true;
				}
				//Edit EMMO: RISING FLAG RESET
				this->is_measuring=false;
				__HAL_TIM_SET_COUNTER(htim, 0);  // reset the counter
			}
		}
}
#endif

bool hcsr04_driver::hcsr04GetAvg (uint16_t& average) {
	if (this->valid) {
		for (int i=0; i<NUM_AVG; i++)
			average += this->avg_array[i];
		average = average/NUM_AVG;
#if _HCSR04DEBUG_
		this->avg = average;
#endif
		return true;
	}
	else
		return false;
}

/*
* @brief:delays code execution for the next measurement
*/

void hcsr04_driver::hcsr04DelayNextMeasurement(void){
	DELAY_MS(NEXT_MEASUREMENT_DELAY);
}





