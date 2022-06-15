#include "huskyFeed.h"

/*These two function are a programmer's shame, but I'm too lazy*/

const char* state_to_cstr(const HFeed_State& state) {
    switch(state){
        case WAIT_FOR_MODE:
            return ("WAIT_FOR_MODE");
        case WAIT_FOR_LOPPIDEH:
            return "WAITING_FOR_LOPPIDEH";
        case WAIT_FOR_DEADLINE:
            return "WAIT_FOR_DEADLINE";
        case SERVING:
            return "SERVING";   
        default:
            return "Invalid State";        
    }
}
const char* mode_to_cstr(const uint8_t& mode){
    switch(mode){
        case HFEED_MODE_MANUAL:
            return "MANUAL";
        case HFEED_MODE_AUTOMATIC:
            return "AUTOMATIC";
        case HFEED_MODE_TIME:
            return "TIME";
        case HFEED_MODE_BLANK:
            return "NO MODE SET";   
        default:
            return "Invalid mode";        
    }
}

struct HuskyFeed_CFG&  HuskyFeed_CFG ::operator=(const struct HuskyFeed_CFG& a)
{
    if(IS_VALID_MODE(a.mode)){
    	//	If the mode if time then copy the configuration only if there are validd deadlines
        if(a.mode==HFEED_MODE_TIME )
        {
            if (IS_DEADLINES_NUM_VALID(a.deadlines_num))
                memcpy(this,&a,sizeof(HuskyFeed_CFG));
            else
                memcpy(this,&default_cfg,sizeof(HuskyFeed_CFG));
        }
        //	If the mode is not time then copy the configuration directly
        else
                memcpy(this,&a,sizeof(HuskyFeed_CFG));
    }
    //	If the mode is not valid then copy the default cfg
    else
        memcpy(this,&default_cfg,sizeof(HuskyFeed_CFG));
    return *this;
}

void HuskyFeed_CFG::to_cstring(char* const to_ret){
    char temp[32];
    uint16_t idx;
#if HFEED_UART_FMT
    sprintf(to_ret," Mode: %s \r \n Quantity %u \r \n NumDeadlines  :%u \r \n Deadlines: \r \n ",
        mode_to_cstr(this->mode),this->food_quantity,this->deadlines_num);
    for(idx=0;idx<this->deadlines_num;idx++){
        sprintf(temp,"idx: %u h: %u m: %u s: %u \r \n ",idx,
            this->deadlines_hours[idx],this->deadlines_minutes[idx],this->deadlines_seconds[idx]);
        strcat(to_ret,temp);
    }
    if(this->periodic==HUSKY_FEED_NOTPERIODIC_TIME)
        strcat(to_ret,"Not Periodic \r \n");
    else
        strcat(to_ret,"Periodic \r \n");
#else
    sprintf(to_ret," Mode: %s \n Quantity %u \n NumDeadlines  :%u \n Deadlines: \n ",
        mode_to_cstr(this->mode),this->food_quantity,this->deadlines_num);
    for(idx=0;idx<this->deadlines_num;idx++){
        sprintf(temp,"idx: %u h: %u m: %u s: %u \n ",idx,
            this->deadlines_hours[idx],this->deadlines_minutes[idx],this->deadlines_seconds[idx]);
        strcat(to_ret,temp);
    }
    if(this->periodic==HUSKY_FEED_NOTPERIODIC_TIME)
        strcat(to_ret,"Not Periodic \n");
    else
        strcat(to_ret,"Periodic \n");
#endif
}
void HuskyFeeder::changeCFG(const struct HuskyFeed_CFG& to_set){
	//	Sets the new configuration
    current_configuration=to_set;
    //	Change state
    HUSKY_FEED_MOD_TO_STATE(current_configuration.mode,current_state);
    //	Resets the deadline counter
    deadline_ctr=0;
    //	Resets the time manager
    if(this->current_state==HFeed_State::WAIT_FOR_DEADLINE){
    	// If the time manager is not valid then resets the configuration
    	if(this->time_manager==0)
    		this->hf_reset();
    	//	Else arms the manager
    	else{
    		// SHOULD DO A CHECK HERE FOR VALID TIME..
    		TimeObj start={
					.seconds=this->current_configuration.starting_seconds,
					.minutes=this->current_configuration.starting_minutes,
					.hours=this->current_configuration.starting_hours,
    		};
    		this->time_manager->start(start);
    	}

    }
    else if(this->current_state==HFeed_State::WAIT_FOR_LOPPIDEH){
    	if(this->presence_manager==0)
    		this->hf_reset();
    	else{
    		this->presence_manager->start_measurement();
    	}
    }
}

bool HuskyFeeder::setWeightManager(HFeed_WeightManager* manager){
	if (manager==0)
		return false;
	weight_manager=manager;
	return true;
}

bool HuskyFeeder::setTimeManager(HFeed_TimeManager*manager){
	if (manager==0)
		return false;
	time_manager=manager;
	return true;
}

bool HuskyFeeder::setPresenceManager(HFeed_PresenceManager* manager){
	if (manager==0)
		return false;
	presence_manager=manager;
	return true;
}

//	Serve food to the dog.
void HuskyFeeder::exec_serving(){
	//	This should never happen but better check than not
	if(this->current_state!=HFeed_State::SERVING)
		return;
	//	Checking if the weight has been set
	if(this->weight_manager==0)
		return;

#if HFEED_DEBUGGING && HFEED_GLOBAL_DEBUGGER && HFEED_SERVING_DBG
	char dbg[128];
	sprintf(dbg,"[SERVING] Entering \r \n");
	HAL_UART_Transmit(&huart2,(uint8_t *) dbg, strlen(dbg), 100);
#endif

	//	Maybe 32 bits are excessive but as Latin says: " melius abundare quam deficere "
	//	UPDATE 10 June 2022: Added uncertainity (- weight_uncertainity)
	uint32_t actual_weight,final_weight=(uint32_t)this->current_configuration.food_quantity-(uint32_t)this->weight_uncertainity;
	// problem in reading, should set an ERROR
	if(!this->weight_manager->get_measure(weight_ms,actual_weight, this->weight_samples))
		return ;

	// 	Final= desired_food+ actual value
	// 	This should be removed from here,  in case there are already the specific quantity of grams.
	// 	Whas these instructions done is basically a tare that should be handled somewhere else
	//	final_weight+=actual_weight;
	// 	For the same reason I've added the check before the motor turning and the while

	uint32_t start_time=HFEED_TIME_GET_CURR_MILLIS;
	uint32_t end_time=start_time+this->serving_timeout_ms;

	if (actual_weight>=final_weight)
		goto serving_update;

	// Resets the servo and turn 90 degrees
	HFEED_SERVO_SET_ORIG();
	HFEED_SERVO_TURN_90();

	// Waits until the time
	while (actual_weight<final_weight && start_time<end_time){
#if HFEED_DEBUGGING && HFEED_GLOBAL_DEBUGGER && HFEED_SERVING_DBG
		sprintf(dbg,"[SERVING] Waiting at %lu \r \n",start_time);
		HAL_UART_Transmit(&huart2,(uint8_t *) dbg, strlen(dbg), 100);
#endif
		// Perform a weighting, in case it fails returns
		if(!this->weight_manager->get_measure(weight_ms, actual_weight, this->weight_samples)){
			goto serving_end;
		}
		start_time=HFEED_TIME_GET_CURR_MILLIS;
	}
	serving_end: HFEED_SERVO_SET_ORIG();
	//	ERROR, should do a check here

	//	Now updating state
	serving_update: if (this->current_configuration.mode!=HFEED_MODE_TIME)
		this->hf_reset();
	else{
		this->current_state=HFeed_State::WAIT_FOR_DEADLINE;
	}
#if HFEED_DEBUGGING && HFEED_GLOBAL_DEBUGGER && HFEED_SERVING_DBG
	sprintf(dbg,"[SERVING] Exiting \r \n");
	HAL_UART_Transmit(&huart2, (uint8_t*)dbg, strlen(dbg), 100);
#endif
}

// Waits until the next dealine and then serve the dog
void HuskyFeeder::exec_wait_for_deadline(){

	if(this->current_state!=HFeed_State::WAIT_FOR_DEADLINE)
		return ;
	if(this->time_manager==0)
		return ;

#if HFEED_DEBUGGING && HFEED_GLOBAL_DEBUGGER && HFEED_DEADLINE_DBG
	char dbg[128];
	sprintf(dbg,"[WAIT_FOR DEADLINE] Entering \r \n");
	HAL_UART_Transmit(&huart2,(uint8_t *) dbg, strlen(dbg), 100);
#endif

	if(this-> time_manager->is_greater(current_configuration.deadlines_hours[deadline_ctr],current_configuration.deadlines_minutes[deadline_ctr],current_configuration.deadlines_seconds[deadline_ctr])){
		// update the deadline and the state
		this->deadline_ctr++;

#if HFEED_DEBUGGING && HFEED_GLOBAL_DEBUGGER && HFEED_DEADLINE_DBG
		sprintf(dbg,"[WAIT_FOR DEADLINE] Going to serve deadline %u at %lu \r \n",this->deadline_ctr-1,HFEED_TIME_GET_CURR_MILLIS);
		HAL_UART_Transmit(&huart2,(uint8_t *) dbg, strlen(dbg), 200);
#endif
		//	In case we ended the deadline number
		if(this->deadline_ctr==this->current_configuration.deadlines_num){

			//	reset the counter if we must restart the next day, the mode should not be changed
			if(this->current_configuration.periodic>0){
#if HFEED_DEBUGGING && HFEED_GLOBAL_DEBUGGER && HFEED_DEADLINE_DBG
				sprintf(dbg,"[WAIT_FOR DEADLINE] DEADLINE ENDED  PERIODIC \r \n ");
				HAL_UART_Transmit(&huart2,(uint8_t *) dbg, strlen(dbg), 200);
#endif
				deadline_ctr=0;
				// HERE WE SHOULD PUT A NEW CONFIGURATION FOR TIME MANAGER FOR MIDNIGHT
			}
			else{
#if HFEED_DEBUGGING && HFEED_GLOBAL_DEBUGGER && HFEED_DEADLINE_DBG
				sprintf(dbg,"[WAIT_FOR DEADLINE] DEADLINE ENDED NO PERIODIC \r \n");
				HAL_UART_Transmit(&huart2,(uint8_t *) dbg, strlen(dbg), 200);
#endif
				//	If it is not periodic then set blank mode in order to reset the Configuration from the serving mode
				this->current_configuration.mode=HFEED_MODE_BLANK;
			}
		}
		this->current_state=HFeed_State::SERVING;
		this->exec_serving();
	}

#if HFEED_DEBUGGING && HFEED_GLOBAL_DEBUGGER && HFEED_DEADLINE_DBG
	else{
		sprintf(dbg,"[WAIT_FOR DEADLINE] LEAVING WITHOUT EXECUTING %lu \r \n",HFEED_TIME_GET_CURR_MILLIS);
		HAL_UART_Transmit(&huart2,(uint8_t *) dbg, strlen(dbg), 200);
	}
#endif

}

// Waits until the dog comes close and then serve the food
void HuskyFeeder::exec_wait_for_BAUBAU(){
	if(this->current_state!=HFeed_State::WAIT_FOR_LOPPIDEH)
		return ;
	if (this->presence_manager==0)
		return ;
#if HFEED_AUTOMATIC_DBG
	char dbg[128];
	sprintf(dbg,"[WAIT_FOR BAUBAU] Waiting for quadrupede \r \n");
	HAL_UART_Transmit(&huart2,(uint8_t *) dbg, strlen(dbg), 200);
#endif
	if(this->presence_manager->is_dog_present()){
#if HFEED_AUTOMATIC_DBG
	sprintf(dbg,"[WAIT_FOR BAUBAU] quadrupede FOUND \r \n");
	HAL_UART_Transmit(&huart2,(uint8_t *) dbg, strlen(dbg), 200);
#endif
		this->current_state=HFeed_State::SERVING;
		this->exec_serving();
	}
}

HuskyFeeder::HuskyFeeder(){
    hf_reset();
    //	Set the state execution table
    //exec_table=&HuskyFeeder::exec_wait_for_mode;
    exec_table[STATE_TO_NUM(WAIT_FOR_MODE)]        =&HuskyFeeder::exec_wait_for_mode;
    exec_table[STATE_TO_NUM(SERVING)]              =&HuskyFeeder::exec_serving;
    exec_table[STATE_TO_NUM(WAIT_FOR_DEADLINE)]    =&HuskyFeeder::exec_wait_for_deadline;
    exec_table[STATE_TO_NUM(WAIT_FOR_LOPPIDEH)]    =&HuskyFeeder::exec_wait_for_BAUBAU;
}

