#include "hx711_driver.h"

/*
 * @Descr:	Returns 0 when the device is ready, after waiting at most  milliseconds_timeout
 */
uint8_t	HX711_Driver::wait_ready(uint32_t milliseconds_timeout){
	uint32_t time_diff,start_time,end_time;
	/*
	 * 	Insert this if the while condition is < and not <=
	 *		if(milliseconds_timeout==0)
	 *			return this->is_ready;
	 */
#if HX711_DBG
		//char to_snd[256];
#endif
	start_time=GET_MS();
	time_diff=0;
	while(time_diff<=milliseconds_timeout){
		if(this->is_ready())
			return 0;
		end_time=GET_MS();
		time_diff=end_time-start_time;
#if  HX711_DBG
			//sprintf(to_snd,"Waiting ready, FOR: %lu \r\n",time_diff);
			//hx711_hal_print_string((uint8_t*)to_snd,strlen(to_snd)+1);
#endif
	}

	return 0xFF;
}

#if UINT_RAPPR
uint32_t	HX711_Driver::read(uint32_t wait_ready_ms_timeout,bool &valid){
#else
int32_t	HX711_Driver::read(uint32_t wait_ready_ms_timeout,bool &valid){
#endif
	uint8_t	 shift_ctr;
	uint32_t read_value=0;
#if HX711_DBG
		char to_snd[128];
#endif

	if(this->wait_ready(wait_ready_ms_timeout)!=0){
		valid=false;
		return 0;
	}
	valid=true;
    //	Disable interrupts
	//	Now shift 24 bits out
	for(shift_ctr=0;shift_ctr<ABS_TICKS_FOR_DATA;shift_ctr++)
	{
#if  HX711_DBG
			sprintf(to_snd,"Shitf ctr val , FOR: %u \r\n",shift_ctr);
			hx711_hal_print_string((uint8_t*)to_snd,strlen(to_snd)+1);
#endif
			PIN_WRITE(this->pdsck_port,this->pdsck_num,PIN_HIGH);
			//	Wait one microsecond for the data to be shifted out
			delay_Microseconds(1);
			//	Reads the data
			read_value = read_value << 1;
			if(PIN_READ(this->dout_port,this->dout_num) == PIN_HIGH)
				read_value ++;
			PIN_WRITE(this->pdsck_port,this->pdsck_num,PIN_LOW);
			delay_Microseconds(1);
	}
    //	Enable Interrupts
	//	Now set the gain for the next reading
	while(shift_ctr < abs_cfg_ticks[this->current_cfg])
	{
#if HX711_DBG
			sprintf(to_snd,"Shitf ctr val , WHILE: %u \r\n",shift_ctr);
			hx711_hal_print_string((uint8_t*)to_snd,strlen(to_snd)+1);
#endif

		PIN_WRITE(this->pdsck_port,this->pdsck_num,PIN_HIGH);
		delay_Microseconds(1);
		PIN_WRITE(this->pdsck_port,this->pdsck_num,PIN_LOW);
		delay_Microseconds(1);
		shift_ctr++;
	}
#if UINT_RAPPR
	read_value = read_value ^ 0x800000;
	return (uint32_t)(read_value);
#else
	// In case it is negative append FF
	if( read_value & 0x800000)
		read_value|=(0xFF000000);
	return (int32_t)(read_value);
#endif

}

double HX711_Driver::read_avg(uint32_t wait_ready_ms_timeout,bool &valid,uint16_t samples){
	if(samples==0){
		valid=false;
		return 0;
	}
	uint16_t idx;
	float sum=0;
	for(idx=0;idx<samples;idx++){
		sum+=(double)read(wait_ready_ms_timeout,valid);
		if(!valid)
			return 0;
	}
	return sum/samples;
}

double HX711_Driver::weigh_Object(uint32_t ms_timeout,bool &valid,uint16_t samples){
	double out_voltage=read_avg(ms_timeout,valid,samples);
	if(valid)
		return (out_voltage-this->intercept)/this->gradient;
	return 0;
}
