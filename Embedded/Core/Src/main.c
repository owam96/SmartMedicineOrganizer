/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "../../Drivers/ili9341-tft-screen/Inc/ili9341.h"

#define UNIT_TEST // Comment this line out to disable testing

#ifdef UNIT_TEST
#include "../../unity/unity_fixture.h"
#include "unity_test_functions.h"
#define MAKE_UNITY_VERBOSE	argc = 2; argv[1] = "-v"
#endif

/*
 * Custom Commands for updating the medicine database
 */
#define ADD_MED_SCHEDULE	'1'
#define MOD_MED_SCHEDULE	'2'
#define DEL_MED_SCHEDULE	'3'

UART_HandleTypeDef huart2;
RTC_HandleTypeDef hrtc;
SPI_HandleTypeDef hspi4;

void SPI4_Init(void);
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);
void MX_RTC_Init(void);

void RTC_Calender_Configuration(void);
static void vSoftwareTimerCallback(xTimerHandle pxTimer);
static void peripheral_task(void * params);

#ifdef UNIT_TEST
void RunAllTests(){
    RUN_TEST_GROUP(GPIO_clk_test_group);
    RUN_TEST_GROUP(GPIO_init_test_group);
    RUN_TEST_GROUP(RTC_init_test_group);
    RUN_TEST_GROUP(USART_init_test_group);
}
#endif

struct Medicine_Database
{
	char date[11];	//Format   DD-MM-YYYY
	char time[6];	//Format   HH:MM
	char medicine_name[21];	//Fill the rest with NULL if len is less than 20
	char comp_num;	//Compartment Number
};
struct Medicine_Database medicine_arr[4];

char rx_char;
uint8_t rx_complete = 0;

char uart_buf[256];
char uart_rx_msg[256];
int uart_buf_index = 0;


int main(int argc, char * argv[])
{
  #ifdef UNIT_TEST
  MAKE_UNITY_VERBOSE;
  UnityMain(argc, (const char **) argv, RunAllTests);
  #endif

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  MX_RTC_Init();
  RTC_Calender_Configuration();

  /* Create Software Timer of 1 minute */
  TimerHandle_t timer_handle = xTimerCreate("timer 1_min", pdMS_TO_TICKS(60000), pdTRUE, (void*)0, vSoftwareTimerCallback);
  if (timer_handle == NULL)
  {
  printf("Fatal : Error in creating Software Timer \r\n");
  for(;;);
  } if (xTimerStart(timer_handle, 0)!=pdPASS)
  {
  printf("Fatal : Error in starting software timer \r\n");
  for(;;);
  }

  MX_USART2_UART_Init();

  /* Code for TFT-lcd screen */

  SPI4_Init();
  ILI9341_DisplayWelcomeScreen();

  HAL_Delay(2000);

  ILI9341_FillScreen(ILI9341_BLACK);

  char inventory_info[100];

  ILI9341_WriteString(10, 10, "Current inventory:", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);

  ILI9341_WriteString(10, 35, "Compartment 0 :", Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  strcpy(inventory_info, "   Medicine: ");
  strcat(inventory_info, medicine_arr[0].medicine_name);
  ILI9341_WriteString(10, 50, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));
  strcpy(inventory_info, "   Date: ");
  strcat(inventory_info, medicine_arr[0].date);
  ILI9341_WriteString(10, 65, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));
  strcpy(inventory_info, "   Time: ");
  strcat(inventory_info, medicine_arr[0].time);
  ILI9341_WriteString(10, 80, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));

  ILI9341_WriteString(10, 95, "Compartment 1 :", Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  strcpy(inventory_info, "   Medicine: ");
  strcat(inventory_info, medicine_arr[1].medicine_name);
  ILI9341_WriteString(10, 110, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));
  strcpy(inventory_info, "   Date: ");
  strcat(inventory_info, medicine_arr[1].date);
  ILI9341_WriteString(10, 125, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));
  strcpy(inventory_info, "   Time: ");
  strcat(inventory_info, medicine_arr[1].time);
  ILI9341_WriteString(10, 140, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));

  ILI9341_WriteString(10, 155, "Compartment 2 :", Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  strcpy(inventory_info, "   Medicine: ");
  strcat(inventory_info, medicine_arr[2].medicine_name);
  ILI9341_WriteString(10, 170, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));
  strcpy(inventory_info, "   Date: ");
  strcat(inventory_info, medicine_arr[2].date);
  ILI9341_WriteString(10, 185, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));
  strcpy(inventory_info, "   Time: ");
  strcat(inventory_info, medicine_arr[2].time);
  ILI9341_WriteString(10, 200, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));

  ILI9341_WriteString(10, 215, "Compartment 3 :", Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  strcpy(inventory_info, "   Medicine: ");
  strcat(inventory_info, medicine_arr[0].medicine_name);
  ILI9341_WriteString(10, 230, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));
  strcpy(inventory_info, "   Date: ");
  strcat(inventory_info, medicine_arr[0].date);
  ILI9341_WriteString(10, 245, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));
  strcpy(inventory_info, "   Time: ");
  strcat(inventory_info, medicine_arr[0].time);
  ILI9341_WriteString(10, 260, (const char*) inventory_info, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  memset(inventory_info, 0, strlen(inventory_info));

  HAL_Delay(5000);

  char dispensing_medicine[100];

  ILI9341_FillScreen(ILI9341_GREEN);
  ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);

  ILI9341_WriteString(10, 35, "Please open:", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
  ILI9341_WriteString(10, 60, "Compartment 0", Font_11x18, ILI9341_RED, ILI9341_GREEN);
  ILI9341_WriteString(10, 85, "To take 1 pill of:", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
  strcpy(dispensing_medicine, medicine_arr[0].medicine_name);
  ILI9341_WriteString(10, 110, "Medicine XYZ", Font_11x18, ILI9341_RED, ILI9341_GREEN);
  memset(dispensing_medicine, 0, strlen(dispensing_medicine));

  for(int i = 0; i < 20; i++){
	  HAL_Delay(800);
	  ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_RED, ILI9341_GREEN);
	  HAL_Delay(800);
	  ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
  }

  /*
   * Create 2 threads.
   * Thread 1 to process the I/O Peripherals
   * Thread 2 to process the Ethernet TCP/IP
   */
   TaskHandle_t task1_handler;
   BaseType_t ret_val = xTaskCreate(peripheral_task, "Task Processing I/O", 400, NULL, 2, &task1_handler); //1600 words - Stack depth
   configASSERT(ret_val == pdPASS);

   //Run the Scheduler
   vTaskStartScheduler();


  while (1)
  {

  }

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

}


/**
* @brief USART2 Initialization Function
* @param None
* @retval None
*/
void MX_USART2_UART_Init(void)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
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

}


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */

void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();


  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Congifuring pins for TFT-display*/
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC,&GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC,&GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC,&GPIO_InitStruct);

}



void RTC_Calender_Configuration(void)
{
	RTC_DateTypeDef date_struct;
	RTC_TimeTypeDef time_struct;

	time_struct.Hours = 1;
	time_struct.Minutes = 12;
	time_struct.Seconds = 00;
	time_struct.TimeFormat = RTC_HOURFORMAT12_PM;
	HAL_StatusTypeDef ret_val = HAL_RTC_SetTime(&hrtc, &time_struct, RTC_FORMAT_BIN);
	if(ret_val != HAL_OK)
	{
		while(1);
	}

	date_struct.Date = 27;
	date_struct.Month = RTC_MONTH_FEBRUARY;
	date_struct.Year = 22;
	date_struct.WeekDay = RTC_WEEKDAY_SUNDAY;

	ret_val = HAL_RTC_SetDate(&hrtc, &date_struct, RTC_FORMAT_BIN);
	if(ret_val != HAL_OK)
	{
		while(1);
	}

	HAL_RTC_GetTime(&hrtc, &time_struct, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &date_struct, RTC_FORMAT_BIN);
}



static void peripheral_task(void * params)
{
	char * name = "Welcome - Smart Medicine Organizer Device";
	printf("%s\n", (char *)name);

	/*
	* Packet Description
	* Command - 1 byte
	* Length -> 1(command) + 11(date) + 6(time) + 1(comp_num) + 20 (medicine name) (39 bytes fixed)
	*/
	char * uart_name_msg = "Welcome - Smart Medicine Organizer Device \r\nPlease enter the command \r\n";
	HAL_UART_Transmit_IT(&huart2, (uint8_t *)uart_name_msg, strlen(uart_name_msg));

	HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_char, 1);

	while(1)	//Task cannot return unless explictly destroyed
	{

	}
}



/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }

}


/* UART Callback for RX */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(rx_char == '\r')
	{
		memcpy(uart_rx_msg, uart_buf, 41);

		/*
		 * Parsing UART Messages
		 */
		if(uart_rx_msg[0] == ADD_MED_SCHEDULE)
		{
			uint8_t compartment_num;
			memcpy(&compartment_num, &uart_rx_msg[19], 1);
			compartment_num -= '0';

			if(compartment_num < 4)
			{
				memcpy(medicine_arr[compartment_num].date, &uart_rx_msg[2], 10);
				medicine_arr[compartment_num].date[10] = '\0';

				memcpy(medicine_arr[compartment_num].time, &uart_rx_msg[13], 5);
				medicine_arr[compartment_num].time[5] = '\0';

				memcpy(&medicine_arr[compartment_num].comp_num, &uart_rx_msg[19], 1);
				medicine_arr[compartment_num].comp_num -= '0';

				memcpy(medicine_arr[compartment_num].medicine_name, &uart_rx_msg[21], 20);
				medicine_arr[compartment_num].medicine_name[20] = '\0';
			}
		}
		else if(uart_rx_msg[0] == MOD_MED_SCHEDULE)
		{
			/*
			 * Search the medicine in the database
			 */
			uint8_t found = 0;
			uint8_t index = 0;

			char med_name[21];
			memcpy(med_name, &uart_rx_msg[21], 20);
			med_name[20] = '\0';

			if(strcmp(medicine_arr[0].medicine_name, med_name) == 0)
			{
				index = 0;
				found = 1;
			}
			else if(strcmp(medicine_arr[1].medicine_name, med_name) == 0)
			{
				index = 1;
				found = 1;
			}
			else if(strcmp(medicine_arr[2].medicine_name, med_name) == 0)
			{
				index = 2;
				found = 1;
			}
			else if(strcmp(medicine_arr[3].medicine_name, med_name) == 0)
			{
				index = 3;
				found = 1;
			}


			if(found)
			{
				memcpy(medicine_arr[index].date, &uart_rx_msg[2], 10);
				medicine_arr[index].date[10] = '\0';

				memcpy(medicine_arr[index].time, &uart_rx_msg[13], 5);
				medicine_arr[index].time[5] = '\0';

			}
			else
			{
				printf("\r\nError cannot modify medicine entry! Schedule of the specified medicine not found in the device\r\n");
			}
		}
		else if(uart_rx_msg[0] == DEL_MED_SCHEDULE)
		{
			/*
			 * Search the medicine in the database
			 */
			uint8_t found = 0;
			uint8_t index = 0;

			char med_name[21];
			memcpy(med_name, &uart_rx_msg[21], 20);
			med_name[20] = '\0';

			if(strcmp(medicine_arr[0].medicine_name, med_name) == 0)
			{
				index = 0;
				found = 1;
			}
			else if(strcmp(medicine_arr[1].medicine_name, med_name) == 0)
			{
				index = 1;
				found = 1;
			}
			else if(strcmp(medicine_arr[2].medicine_name, med_name) == 0)
			{
				index = 2;
				found = 1;
			}
			else if(strcmp(medicine_arr[3].medicine_name, med_name) == 0)
			{
				index = 3;
				found = 1;
			}


			if(found)
			{
				medicine_arr[index].date[0] = '\0';
				medicine_arr[index].time[0] = '\0';
				medicine_arr[index].medicine_name[0] = '\0';
				medicine_arr[index].comp_num = '\0';

			}
			else
			{
				printf("\r\nError Cannot delete medicine entry! Schedule of the specified medicine not found in the device\r\n");
			}
		}

		/*Print to virtual terminal about the current inventory of the device*/
		//printf("\r\n\r\nCurrent Inventory\r\n");
		//printf("Comp 0, Med - %s, Date - %s, Time - %s\r\n", medicine_arr[0].medicine_name, medicine_arr[0].date, medicine_arr[0].time);
		//printf("Comp 1, Med - %s, Date - %s, Time - %s\r\n", medicine_arr[1].medicine_name, medicine_arr[1].date, medicine_arr[1].time);
		//printf("Comp 2, Med - %s, Date - %s, Time - %s\r\n", medicine_arr[2].medicine_name, medicine_arr[2].date, medicine_arr[2].time);
		//printf("Comp 3, Med - %s, Date - %s, Time - %s\r\n", medicine_arr[3].medicine_name, medicine_arr[3].date, medicine_arr[3].time);

		uart_buf_index = 0;
		memset(uart_buf, 0x00, sizeof(uart_buf));

	}
	else
	{
		uart_buf[uart_buf_index++] = rx_char;
	}

	HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_char, 1);
}

static void vSoftwareTimerCallback(xTimerHandle pxTimer)
{

	char dispensing_medicine[100];

	//Read the current date and time from RTC
	RTC_DateTypeDef date_struct;
	RTC_TimeTypeDef time_struct; HAL_RTC_GetTime(&hrtc, &time_struct, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &date_struct, RTC_FORMAT_BIN);
	printf("\r\nCurrent Date and Time - %d:%d:%d, %d:%d", date_struct.Date, date_struct.Month, date_struct.Year, time_struct.Hours, time_struct.Minutes); //Check if the medicine is due to consume
	if(medicine_arr[0].date[0] != '\0')
	{
	char str[10] = {'\0'};
	memcpy(str, &(medicine_arr[0].date[0]), 2);
	str[2] = '\0';
	int stored_day = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[0].date[3]), 2);
	str[2] = '\0';
	int stored_mon = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[0].date[8]), 2);
	str[2] = '\0';
	int stored_year = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[0].time[0]), 2);
	str[2] = '\0';
	int stored_hours = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[0].time[3]), 2);
	str[2] = '\0';
	int stored_min = atoi(str);
	memset(str, '\0', sizeof(char)*10); if((stored_day == date_struct.Date) && (stored_mon == date_struct.Month) && \
	(stored_year == date_struct.Year) && (stored_hours == time_struct.Hours) \
	&& (stored_min == time_struct.Minutes))
	{
	//printf("\r\n\r\nMedicine Name - %s\r\n", medicine_arr[0].medicine_name);
	//printf("\r\nOpening Compartment 0 and Dispersing 1 Tablet\r\n");


	ILI9341_FillScreen(ILI9341_GREEN);
	ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	ILI9341_WriteString(10, 35, "Please open:", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	ILI9341_WriteString(10, 60, "Compartment 0", Font_11x18, ILI9341_RED, ILI9341_GREEN);
	ILI9341_WriteString(10, 85, "To take 1 pill of:", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	strcpy(dispensing_medicine, medicine_arr[0].medicine_name);
	ILI9341_WriteString(10, 110, (const char*) dispensing_medicine, Font_11x18, ILI9341_YELLOW, ILI9341_GREEN);
	memset(dispensing_medicine, 0, strlen(dispensing_medicine));

	for(int i = 0; i < 20; i++){
	  HAL_Delay(800);
	  ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_RED, ILI9341_GREEN);
	  HAL_Delay(800);
	  ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	}

	} } if(medicine_arr[1].date[0] != '\0')
	{
	char str[10] = {'\0'};
	memcpy(str, &(medicine_arr[1].date[0]), 2);
	str[2] = '\0';
	int stored_day = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[1].date[3]), 2);
	str[2] = '\0';
	int stored_mon = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[1].date[8]), 2);
	str[2] = '\0';
	int stored_year = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[1].time[0]), 2);
	str[2] = '\0';
	int stored_hours = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[1].time[3]), 2);
	str[2] = '\0';
	int stored_min = atoi(str);
	memset(str, '\0', sizeof(char)*10); if((stored_day == date_struct.Date) && (stored_mon == date_struct.Month) && \
	(stored_year == date_struct.Year) && (stored_hours == time_struct.Hours) \
	&& (stored_min == time_struct.Minutes))
	{
	//printf("\r\n\r\nMedicine Name - %s\r\n", medicine_arr[1].medicine_name);
	//printf("\r\nOpening Compartment 1 and Dispersing 1 Tablet\r\n");

	ILI9341_FillScreen(ILI9341_GREEN);
	ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	ILI9341_WriteString(10, 35, "Please open:", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	ILI9341_WriteString(10, 60, "Compartment 1", Font_11x18, ILI9341_RED, ILI9341_GREEN);
	ILI9341_WriteString(10, 85, "To take 1 pill of:", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	strcpy(dispensing_medicine, medicine_arr[1].medicine_name);
	ILI9341_WriteString(10, 110, (const char*) dispensing_medicine, Font_11x18, ILI9341_YELLOW, ILI9341_GREEN);
	memset(dispensing_medicine, 0, strlen(dispensing_medicine));

	for(int i = 0; i < 20; i++){
	  HAL_Delay(800);
	  ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_RED, ILI9341_GREEN);
	  HAL_Delay(800);
	  ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	}

	}
	} if(medicine_arr[2].date[0] != '\0')
	{
	char str[10] = {'\0'};
	memcpy(str, &(medicine_arr[2].date[0]), 2);
	str[2] = '\0';
	int stored_day = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[2].date[3]), 2);
	str[2] = '\0';
	int stored_mon = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[2].date[8]), 2);
	str[2] = '\0';
	int stored_year = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[2].time[0]), 2);
	str[2] = '\0';
	int stored_hours = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[2].time[3]), 2);
	str[2] = '\0';
	int stored_min = atoi(str);
	memset(str, '\0', sizeof(char)*10); if((stored_day == date_struct.Date) && (stored_mon == date_struct.Month) && \
	(stored_year == date_struct.Year) && (stored_hours == time_struct.Hours) \
	&& (stored_min == time_struct.Minutes))
	{
	//printf("\r\n\r\nMedicine Name - %s\r\n", medicine_arr[2].medicine_name);
	//printf("\r\nOpening Compartment 2 and Dispersing 1 Tablet\r\n");

	ILI9341_FillScreen(ILI9341_GREEN);
	ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	ILI9341_WriteString(10, 35, "Please open:", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	ILI9341_WriteString(10, 60, "Compartment 2", Font_11x18, ILI9341_RED, ILI9341_GREEN);
	ILI9341_WriteString(10, 85, "To take 1 pill of:", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	strcpy(dispensing_medicine, medicine_arr[2].medicine_name);
	ILI9341_WriteString(10, 110, (const char*) dispensing_medicine, Font_11x18, ILI9341_YELLOW, ILI9341_GREEN);
	memset(dispensing_medicine, 0, strlen(dispensing_medicine));

	for(int i = 0; i < 20; i++){
	  HAL_Delay(800);
	  ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_RED, ILI9341_GREEN);
	  HAL_Delay(800);
	  ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	}

	}
	} if(medicine_arr[3].date[0] != '\0')
	{
	char str[10] = {'\0'};
	memcpy(str, &(medicine_arr[3].date[0]), 2);
	str[2] = '\0';
	int stored_day = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[3].date[3]), 2);
	str[2] = '\0';
	int stored_mon = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[3].date[8]), 2);
	str[2] = '\0';
	int stored_year = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[3].time[0]), 2);
	str[2] = '\0';
	int stored_hours = atoi(str);
	memset(str, '\0', sizeof(char)*10); memcpy(str, &(medicine_arr[3].time[3]), 2);
	str[2] = '\0';
	int stored_min = atoi(str);
	memset(str, '\0', sizeof(char)*10); if((stored_day == date_struct.Date) && (stored_mon == date_struct.Month) && \
	(stored_year == date_struct.Year) && (stored_hours == time_struct.Hours) \
	&& (stored_min == time_struct.Minutes))
	{
	//printf("\r\n\r\nMedicine Name - %s\r\n", medicine_arr[3].medicine_name);
	//printf("\r\nOpening Compartment 3 and Dispersing 1 Tablet\r\n");

	ILI9341_FillScreen(ILI9341_GREEN);
	ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	ILI9341_WriteString(10, 35, "Please open:", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	ILI9341_WriteString(10, 60, "Compartment 3", Font_11x18, ILI9341_RED, ILI9341_GREEN);
	ILI9341_WriteString(10, 85, "To take 1 pill of:", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	strcpy(dispensing_medicine, medicine_arr[3].medicine_name);
	ILI9341_WriteString(10, 110, (const char*) dispensing_medicine, Font_11x18, ILI9341_YELLOW, ILI9341_GREEN);
	memset(dispensing_medicine, 0, strlen(dispensing_medicine));

	for(int i = 0; i < 20; i++){
	  HAL_Delay(800);
	  ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_RED, ILI9341_GREEN);
	  HAL_Delay(800);
	  ILI9341_WriteString(10, 10, "Time for medicine!", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
	}

	}
	}
}

void SPI4_Init(){

	hspi4.Instance = SPI4;
	hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi4.Init.Direction = SPI_DIRECTION_2LINES;
	hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi4.Init.Mode = SPI_MODE_MASTER;
	hspi4.Init.NSS = SPI_NSS_SOFT;
	hspi4.Init.TIMode = SPI_TIMODE_DISABLE;

	if (HAL_SPI_Init(&hspi4) != HAL_OK)
		{
			Error_Handler();
	}

}



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



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
