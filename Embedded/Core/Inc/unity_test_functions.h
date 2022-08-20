/*
 * unity_test_functions.h
 *
 *  Created on: Mar. 9, 2022
 *      Author: omarmostafa
 */

#ifndef INC_UNITY_TEST_FUNCTIONS_H_
#define INC_UNITY_TEST_FUNCTIONS_H_

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_rtc.h"

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);
void MX_RTC_Init(void);
void RTC_Calender_Configuration(void);

#endif /* INC_UNITY_TEST_FUNCTIONS_H_ */
