/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "port_time.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define WEB_SERV_TX_Pin GPIO_PIN_0
#define WEB_SERV_TX_GPIO_Port GPIOA
#define WEB_SERV_RX_Pin GPIO_PIN_1
#define WEB_SERV_RX_GPIO_Port GPIOA
#define BOARD_LED_Pin GPIO_PIN_5
#define BOARD_LED_GPIO_Port GPIOA
#define HCSR04_ECHO_Pin GPIO_PIN_14
#define HCSR04_ECHO_GPIO_Port GPIOB
#define HCSR04_Trigger_Pin GPIO_PIN_6
#define HCSR04_Trigger_GPIO_Port GPIOC
#define HX_711_DOUT_Pin GPIO_PIN_8
#define HX_711_DOUT_GPIO_Port GPIOA
#define HX711_PD_SCK_Pin GPIO_PIN_9
#define HX711_PD_SCK_GPIO_Port GPIOA
#define Servo_PWM_Pin GPIO_PIN_9
#define Servo_PWM_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
