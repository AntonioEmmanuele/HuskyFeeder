#define USE_TESTS	0
#if	USE_TESTS
/* USER CODE BEGIN Header */
/**
  **************************
  * @file           : main.cpp
  * @brief          : Main program body
  **************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  **************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

//#include "port_Presence.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define TIME_TEST 	0
#define WEIGHT_TEST 0
#define BALANCE_OFFSET	8223540 //Measuring done in Antonio's room desk ,know obj of 230 grams
#define BALANCE_RATIO	474		//(8332500-8223540)/230
#define RUN_APP			1

#include "huskyFeed.h"
#include "hx711_driver.h"
#include "hcsr04_driver.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim11;
TIM_HandleTypeDef htim12;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#if RUN_APP

//Dbg flags and utils
#define APP_DBG	1
#if APP_DBG
char dbg_buff[1024];
#endif
#define HCSR04_TEST			1
#define APP_TEST_1			0
#define APP_TEST_2			0
#define APP_TEST_DEADLINES 	0

//Calibration flag
#define CALIBRATION 0
#if CALIBRATION
double avg=0;
bool valid=false;
#endif

#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM11_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM12_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM11_Init();
  MX_USART2_UART_Init();
  MX_TIM7_Init();
  MX_TIM12_Init();
  /* Initialize interrupts */
  //MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);
  HAL_TIM_Base_Start(&htim7);
  HAL_TIM_IC_Start(&htim12, TIM_CHANNEL_1);
/***************** CONFIGURING APP ******************/
#if RUN_APP

  	  // First of all, init the drivers

  	  //Presence  sensor
  	  hcsr04_driver hcsr04_driver(HCSR04_TRIGGER_GPIO_Port,HCSR04_TRIGGER_Pin);

  	  //Weight sensor D7-D8 8 (PA8 PA9) DOUT SCK
  	  //Load Cell Connections: White A- Green A+ Black E- Red E+ (I've actually inverted white and green)
  	  HX711_Driver hx711_driver(HX711_PD_SCK_GPIO_Port,HX711_PD_SCK_Pin,HX_711_DOUT_GPIO_Port,HX_711_DOUT_Pin,CFG_IN_A_GAIN_128);
  	  hx711_driver.hx711_hal_stm42_set_timer(&htim7);
  	  hx711_driver.reset();
  	  hx711_driver.set_intercept(BALANCE_OFFSET);
  	  hx711_driver.set_gradient(BALANCE_RATIO);

  	  //Motor, BROWN GND RED 5v Orange PWM
  	  //HFEED_SERVO_MCU_SET_PWM_CFG(&htim11);
  	  //HFEED_SERVO_MCU_SET_TIMCH(TIM_CHANNEL_1);

  	  // Defining Managers
  	  HFeed_WeightManager weight_manager;
  	  HFeed_PresenceManager presence_manager;
  	  HFeed_TimeManager	time_manager;

  	  //Setting managers

  	  //Presence Manager
  	  presence_manager.Driver=&hcsr04_driver;
  	  presence_manager.soglia=SOGLIA;
  	  presence_manager.last_time_measure=0;

  	  //Time Manager
  	  time_manager.reset();

  	  //Weight Manager
  	  weight_manager.set_ptr(&hx711_driver);

  	  //The Application
  	  HuskyFeeder app=HuskyFeeder::getFeeder();
  	  if(!app.setTimeManager(&time_manager))
  		  Error_Handler();
  	  if(!app.setWeightManager(&weight_manager))
  		  Error_Handler();
  	  if(!app.setPresenceManager(&presence_manager))
  		  Error_Handler();
#if APP_DBG
	  sprintf(dbg_buff," Configuration done \r\n");
	  HAL_UART_Transmit(&huart2, (uint8_t*)dbg_buff, strlen(dbg_buff), 50);
#endif
#if APP_TEST_2
	 struct HuskyFeed_CFG base_cfg={
			 .mode=HFEED_MODE_MANUAL,
			 .food_quantity=70,
			 .deadlines_num=0,
			 .deadlines_hours={0},
			 .deadlines_minutes={0},
			 .deadlines_seconds={0},
			 .starting_hours=11,
			 .starting_minutes=25,
			 .starting_seconds=0,
			 .periodic=0,
	 };
#endif
#if APP_TEST_DEADLINES
	 struct HuskyFeed_CFG deadlines_cfg={
			 .mode=HFEED_MODE_TIME,
			 .food_quantity=70,
			 .deadlines_num=3,
			 .deadlines_hours={15,15,15},
			 .deadlines_minutes={2,3,4},
			 .deadlines_seconds={0},
			 .starting_hours=15,
			 .starting_minutes=1,
			 .starting_seconds=0,
			 .periodic=0,

	 };
	 app.changeCFG(deadlines_cfg);
#endif
#endif
  /*************** END APP CONFIG********************/
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /* USER CODE END WHILE */

	  // In case we must run the app
#if RUN_APP

	  /***** CALIBRATION CODE ******/
#if CALIBRATION
	  avg=hx711_driver.read_avg(100, valid,50);
	  if (valid){
#if APP_DBG
		  sprintf(dbg_buff," Value %.4f %.4f Converted  \r\n",avg, (avg-BALANCE_OFFSET)/BALANCE_RATIO);
		  HAL_UART_Transmit(&huart2, (uint8_t*)dbg_buff, strlen(dbg_buff), 100);
#endif
	  }
	  else{
#if APP_DBG
		  sprintf(dbg_buff," Data Invalid \r\n");
		  HAL_UART_Transmit(&huart2, (uint8_t*)dbg_buff, strlen(dbg_buff), 100);
#endif
	  }
#endif

	  /**** BASIC TESTS ****/
#if APP_TEST_1 && APP_DBG
	  sprintf(dbg_buff, "The app current state  is %u \r \n" ,app.current_state);
	  HAL_UART_Transmit(&huart2, (uint8_t*)dbg_buff, strlen(dbg_buff), 100);
#endif
	  /***** SERVIG TESTS***/
#if APP_TEST_2
	  	  app.serving_timeout_ms=10000;
#if APP_DBG
	  	  app.to_cstring(dbg_buff);
	  	  HAL_UART_Transmit(&huart2, (uint8_t*)dbg_buff, strlen(dbg_buff), 150);
	  	  HAL_Delay(1000);
#endif
	  app.changeCFG(base_cfg);
#if APP_DBG
	  	  app.to_cstring(dbg_buff);
	  	  HAL_UART_Transmit(&huart2, (uint8_t*)dbg_buff, strlen(dbg_buff), 150);
	  	  HAL_Delay(1000);
#endif
	  app.exec_state();
#endif
	  /***** DEADLINE TESTS***/
#if APP_TEST_DEADLINES
	  app.serving_timeout_ms=10000;

#if APP_DBG
	  	  app.to_cstring(dbg_buff);
	  	  HAL_UART_Transmit(&huart2, (uint8_t*)dbg_buff, strlen(dbg_buff), 300);
	  	  HAL_Delay(1000);
#endif

	  //app.changeCFG(deadlines_cfg);

#if APP_DBG
	  	  app.to_cstring(dbg_buff);
	  	  HAL_UART_Transmit(&huart2, (uint8_t*)dbg_buff, strlen(dbg_buff), 300);
	  	  HAL_Delay(1000);
#endif
	  	app.exec_state();
#endif
#if HCSR04_TEST

	  	presence_manager.start_measurement();
	  	/*
#if APP_DBG
	  	if (presence_manager.is_dog_present()) {
	  	  		sprintf(dbg_buff, "**** Detected ***** \n\r");
	  			HAL_UART_Transmit(&huart2, (uint8_t*)dbg_buff, strlen(dbg_buff),50);
	  			hcsr04_driver.hcsr04Print(&huart2);
	  	  }
#endif*/
#endif
	  	HAL_GPIO_TogglePin(BOARD_LED_GPIO_Port, BOARD_LED_Pin);
	  	HAL_Delay(100);
#endif
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 83;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 1680-1;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 1000-1;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim11, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */
  HAL_TIM_MspPostInit(&htim11);

}

/**
  * @brief TIM12 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM12_Init(void)
{

  /* USER CODE BEGIN TIM12_Init 0 */

  /* USER CODE END TIM12_Init 0 */

  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM12_Init 1 */

  /* USER CODE END TIM12_Init 1 */
  htim12.Instance = TIM12;
  htim12.Init.Prescaler = 41;
  htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim12.Init.Period = 65535;
  htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim12.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim12) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim12, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM12_Init 2 */

  /* USER CODE END TIM12_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, BOARD_LED_Pin|HX711_PD_SCK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BOARD_LED_Pin HX711_PD_SCK_Pin */
  GPIO_InitStruct.Pin = BOARD_LED_Pin|HX711_PD_SCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : HCSR04_TRIGGER_Pin */
  GPIO_InitStruct.Pin = HCSR04_TRIGGER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(HCSR04_TRIGGER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : HX_711_DOUT_Pin */
  GPIO_InitStruct.Pin = HX_711_DOUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(HX_711_DOUT_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
#endif