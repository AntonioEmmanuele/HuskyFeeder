#ifndef	HX711_DRIVER
#define	HX711_DRIVER

/************************************/
//MCU STUFF
#include "../AppDrivers/mcu_defs.h"
#define HX711_DBG	0
#define UINT_RAPPR	1

#if HX711_DBG
#include "string.h"
#include <stdio.h>
#endif

/************************************/

/***********************************/
//	IC DRIVER

//	HX711 TICKS DEFINITION
//Absolute ticks values for pdsck
#define	ABS_TICKS_IN_A_GAIN_128		25
#define	ABS_TICKS_IN_A_GAIN_64		27
#define	ABS_TICKS_IN_B_GAIN_32		26
#define ABS_TICKS_FOR_DATA			24

//	Gain and channel ticks started counting from 24, after a signal read
#define	REL_TICKS_IN_A_GAIN_128		1
#define	REL_TICKS_IN_A_GAIN_64		3
#define	REL_TICKS_IN_B_GAIN_32		2

//	User choices
#define CFG_IN_A_GAIN_128		0
#define	CFG_IN_A_GAIN_64		1
#define	CFG_IN_B_GAIN_32		2

//	Type of the key for indexing the configuration tables
typedef uint8_t	cfg_tick_key;

//	PDSCK Ticks cfg tables
static const uint8_t abs_cfg_ticks[]={ABS_TICKS_IN_A_GAIN_128,ABS_TICKS_IN_A_GAIN_64,ABS_TICKS_IN_B_GAIN_32};
static const uint8_t rel_cfg_ticks[]={REL_TICKS_IN_A_GAIN_128,REL_TICKS_IN_A_GAIN_64,REL_TICKS_IN_B_GAIN_32};

class HX711_Driver{

public:
	pin_port  		pdsck_port;
	pin_number		pdsck_num;
	//	DOUT should be an input pulled up
	pin_port		dout_port;
	pin_number		dout_num;
	cfg_tick_key	current_cfg;
	/*
	 * Out Voltage=gradient*Weight+ intercept
	 * Weight=(Out Voltage-intercept)/gradient
	 * To find Intercept measure the out voltage with Weight 0.
	 * To find gradient, after finding Intercept, weigh an object of known weight and do :
	 * gradient=(Out Voltage-Intercept)/Weight
	 */
	double			gradient=1;
	double			intercept=0;

#if HX711_DBG
#if STM32_HAL
	UART_HandleTypeDef* dbg;
#endif
#endif

#if STM32_HAL
	//	Timer dedicate to delay 1 microsecond
	TIM_HandleTypeDef* hx711_tim_handler;
#endif

#if HX711_DBG
#if STM32_HAL
	__ALWAYSINLINE  HAL_StatusTypeDef hx711_hal_print_string(const uint8_t* to_snd,size_t len ){return HAL_UART_Transmit(dbg,(uint8_t*)to_snd,len,100);}
	__ALWAYSINLINE void hx711_hal_stm42_set_debugger(UART_HandleTypeDef* d){ dbg=d;}
#endif
#endif
#if STM32_HAL
	__ALWAYSINLINE void hx711_hal_stm42_set_timer(TIM_HandleTypeDef* handler){ hx711_tim_handler=handler;}
#endif
	__ALWAYSINLINE void delay_Microseconds(uint32_t microseconds){
		if(microseconds==0)
			return;
	#if	STM32_HAL
		//	THE TIMER SHOULD TICK EVERY MICROSECOND, FREQ 1 MHZ !
		__HAL_TIM_SET_COUNTER(hx711_tim_handler,0);
		while(__HAL_TIM_GET_COUNTER(hx711_tim_handler)<microseconds);
	#endif
	}
	__ALWAYSINLINE void power_on(){
		//For powering the device on we need to place the pd_sck in its normal state, low
		PIN_WRITE(pdsck_port,pdsck_num,PIN_LOW);
	}

	__ALWAYSINLINE void set_intercept(double it ){ this->intercept=it;}

	__ALWAYSINLINE void set_gradient(double gd ){ this->gradient=gd;}
	//Powers off the device and performs a reset
	__ALWAYSINLINE void power_off(){
		//To power of we need to place the pin low and wait 60 microseconds.
		PIN_WRITE(pdsck_port,pdsck_num,PIN_HIGH);
		//delay_Microseconds(60);
	}
	__ALWAYSINLINE void reset(){
		power_off();
		power_on();
	}
	__ALWAYSINLINE bool is_ready(){	return PIN_READ(dout_port,dout_num)==PIN_LOW; }

	uint8_t			wait_ready(uint32_t);
#if UINT_RAPPR
	uint32_t		read(uint32_t,bool&);
#else
	int32_t			read(uint32_t,bool&);
#endif
	double		read_avg(uint32_t,bool&,uint16_t);
	double		weigh_Object(uint32_t,bool&,uint16_t);

	HX711_Driver( pin_port pdsck_port, const  pin_number pdsck_num, pin_port dout_port, const pin_number dout_num,const cfg_tick_key ic_configuration)
	:pdsck_port(pdsck_port),pdsck_num(pdsck_num), dout_port(dout_port),dout_num(dout_num),current_cfg(ic_configuration)
	{
		/*
		//Performing a reset
		power_off();
		power_on();*/
	};
	~HX711_Driver(){}
};
/***********************************/
#endif
