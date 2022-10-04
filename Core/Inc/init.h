/*
 * init.h
 *
 *  Created on: Sep 14, 2022
 *      Author: jp112sdl
 */

#pragma once

#include "settings.h"
#include "led.h"
#include <stdio.h>

#define ETH_RESET_PORT GPIOB
#define ETH_RESET_PIN  GPIO_PIN_14

TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart1;

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV5;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_PLL2;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL2_ON;
  RCC_OscInitStruct.PLL2.PLL2MUL = RCC_PLL2_MUL8;
  RCC_OscInitStruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitTypeDef RCC_ClkInitStruct = {
		  .ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2,
		  .SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
		  .AHBCLKDivider = RCC_SYSCLK_DIV1,
		  .APB1CLKDivider = RCC_HCLK_DIV2,
		  .APB2CLKDivider = RCC_HCLK_DIV1
  };
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
  HAL_RCC_MCOConfig(RCC_MCO, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);

  __HAL_RCC_PLLI2S_ENABLE();
}

static void MX_TIM3_Init(void) {
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {  Error_Handler(); }

  TIM_ClockConfigTypeDef sClockSourceConfig = {
		  .ClockSource = TIM_CLOCKSOURCE_INTERNAL
  };
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) { Error_Handler(); }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) { Error_Handler(); }

  TIM_MasterConfigTypeDef sMasterConfig = {
		  .MasterOutputTrigger = TIM_TRGO_RESET,
		  .MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE
  };
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) { Error_Handler(); }

  TIM_OC_InitTypeDef sConfigOC = {
		  .OCMode = TIM_OCMODE_PWM1,
		  .Pulse = 0,
		  .OCPolarity = TIM_OCPOLARITY_LOW,
		  .OCFastMode = TIM_OCFAST_DISABLE
  };


  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) { Error_Handler(); }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) { Error_Handler(); }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) { Error_Handler(); }

  HAL_TIM_MspPostInit(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
}

static void MX_USART1_UART_Init(void) {
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK) { Error_Handler(); }
}

static void MX_GPIO_Init(void) {
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOC, ST7_2_Pin|ST7_5_Pin|GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, ST7_2B2_Pin|ETH_PWRDWN_Pin|GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, ETH_RESET_Pin|EFM32_RESET_Pin, GPIO_PIN_SET);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = ST7_2_Pin |ST7_5_Pin|GPIO_PIN_4|GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = TA1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(TA1_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = ST7_2B2_Pin|ETH_RESET_Pin|EFM32_RESET_Pin|ETH_PWRDWN_Pin|GPIO_PIN_0|GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void ethernetif_notify_conn_changed(struct netif *netif) {
	printf("ETH LINK is %s\n", netif_is_link_up(&gnetif) ? "UP":"DOWN");
}

void strobeETHReset() {
	  HAL_GPIO_WritePin(ETH_RESET_PORT, ETH_RESET_PIN,GPIO_PIN_RESET);
	  HAL_Delay(50);
	  HAL_GPIO_WritePin(ETH_RESET_PORT, ETH_RESET_PIN,GPIO_PIN_SET);
	  HAL_Delay(250);
}

void setPHY() {
	  /*Configure GPIO pin : PA3, MODE: MII */
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  GPIO_InitStruct.Pin = GPIO_PIN_3;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

	  strobeETHReset();
}

void factoryReset(Settings *settings, LED *blueLED, LED *redLED, LED *greenLED) {
	  if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_RESET) {
		  blueLED->setState(LED_STATE_BLINK);
		  uint8_t cnt = 0;
		  uint32_t ms = HAL_GetTick();
		  uint16_t timeout = 1000;
		  while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_RESET) {
			    uint32_t new_ms = HAL_GetTick();

			    if (new_ms - ms > timeout) {
			      cnt++;
			      ms = new_ms;
			      if (cnt > 3) blueLED->setState(LED_STATE_BLINK_FAST);
			      if (cnt > 6) {
					  blueLED->setState(LED_STATE_OFF);
					  greenLED->setState(LED_STATE_ON);
					  redLED->setState(LED_STATE_ON);
			    	  settings->clear();
			          HAL_Delay(500);
			    	  NVIC_SystemReset();
			      }
			    }
		  }
		  blueLED->setState(LED_STATE_OFF);
	  }
}
