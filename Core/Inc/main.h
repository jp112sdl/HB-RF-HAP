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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define ST7_2_Pin GPIO_PIN_13
#define ST7_2_GPIO_Port GPIOC
#define TA1_Pin GPIO_PIN_0
#define TA1_GPIO_Port GPIOC
#define ST7_2B2_Pin GPIO_PIN_2
#define ST7_2B2_GPIO_Port GPIOB
#define ETH_RESET_Pin GPIO_PIN_14
#define ETH_RESET_GPIO_Port GPIOB
#define ST7_5_Pin GPIO_PIN_9
#define ST7_5_GPIO_Port GPIOC
#define zum_EFM32_Pin GPIO_PIN_12
#define zum_EFM32_GPIO_Port GPIOC
#define vom_EFM32_Pin GPIO_PIN_2
#define vom_EFM32_GPIO_Port GPIOD
#define EFM32_RESET_Pin GPIO_PIN_6
#define EFM32_RESET_GPIO_Port GPIOB
#define ETH_PWRDWN_Pin GPIO_PIN_9
#define ETH_PWRDWN_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
extern struct netif gnetif;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
