#include "../unity/unity_fixture.h"
#include "unity_test_functions.h"
#include "main.h"

UART_HandleTypeDef unity_huart2;
RTC_HandleTypeDef unity_hrtc;

TEST_GROUP(GPIO_clk_test_group);
TEST_GROUP(GPIO_init_test_group);
TEST_GROUP(USART_init_test_group);
TEST_GROUP(RTC_init_test_group);

TEST_SETUP(GPIO_clk_test_group){
	RCC->AHB1ENR = 0x00100000;
}

TEST_TEAR_DOWN(GPIO_clk_test_group){
	RCC->AHB1ENR = 0x00100000;
}

TEST_SETUP(GPIO_init_test_group){

}

TEST_TEAR_DOWN(GPIO_init_test_group){
	SYSCFG->EXTICR[3] = 0;
	GPIOC->MODER = 0;
	GPIOC->PUPDR = 0;
	EXTI->IMR = 0;
	EXTI->RTSR = 0;
	RCC->AHB1ENR = 0x00100000;
}

TEST_SETUP(USART_init_test_group){

}

TEST_TEAR_DOWN(USART_init_test_group){
	unity_huart2.Instance = USART2;
	unity_huart2.Instance->CR1 = 0;
	unity_huart2.Instance->BRR = 0;
	GPIOA->MODER = 0xA8000000; // Reset value for PA
	GPIOA->AFR[0] = 0;
	RCC->AHB1ENR = 0x00100000;
}

TEST_SETUP(RTC_init_test_group){

}

TEST_TEAR_DOWN(RTC_init_test_group){
	HAL_RCC_DeInit();
}

//-------- TESTS START HERE --------//

TEST(GPIO_clk_test_group, GPIOA_clk_en){

	/*!
		  * @param Given :	RCC_AHB1ENR has reset value
		  * @param When  :	GPIOA clk enable macro is called
		  * @param Then  :	RCC_AHB1ENR has the enable value for GPIOA
	*/

	/* Arrange */
	//---

	/* Act */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Assert*/
	LONGS_EQUAL(0x00100001,RCC->AHB1ENR);
}

TEST(GPIO_clk_test_group, GPIOB_clk_en){
	/*!
		  * @param Given :	RCC_AHB1ENR has reset value
		  * @param When  :	GPIOB clk enable macro is called
		  * @param Then  :	RCC_AHB1ENR has the enable value for GPIOB
	*/

	/* Arrange */
	//---

	/* Act */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Assert*/
	LONGS_EQUAL(0x00100002,RCC->AHB1ENR);
}

TEST(GPIO_clk_test_group, GPIOC_clk_en){
	/*!
		  * @param Given :	RCC_AHB1ENR has reset value
		  * @param When  :	GPIOC clk enable macro is called
		  * @param Then  :	RCC_AHB1ENR has the enable value for GPIOC
	*/

	/* Arrange */
	//---

	/* Act */
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/*Assert*/
	LONGS_EQUAL(0x00100004,RCC->AHB1ENR);
}

TEST(GPIO_clk_test_group, GPIOD_clk_en){
	/*!
		  * @param Given :	RCC_AHB1ENR has reset value
		  * @param When  :	GPIOD clk enable macro is called
		  * @param Then  :	RCC_AHB1ENR has the enable value for GPIOD
	*/

	/* Arrange */
	//---

	/* Act */
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Assert*/
	LONGS_EQUAL(0x00100008,RCC->AHB1ENR);
}

TEST(GPIO_clk_test_group, GPIOG_clk_en){
	/*!
		  * @param Given :	RCC_AHB1ENR has reset value
		  * @param When  :	GPIOG clk enable macro is called
		  * @param Then  :	RCC_AHB1ENR has the enable value for GPIOG
	*/

	/* Arrange */
	//---

	/* Act */
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Assert*/
	LONGS_EQUAL(0x00100040,RCC->AHB1ENR);
}

TEST(GPIO_clk_test_group, GPIOH_clk_en){
	/*!
		  * @param Given :	RCC_AHB1ENR has reset value
		  * @param When  :	GPIOH clk enable macro is called
		  * @param Then  :	RCC_AHB1ENR has the enable value for GPIOH
	*/

	/* Arrange */
	//---

	/* Act */
	__HAL_RCC_GPIOH_CLK_ENABLE();

	/*Assert*/
	LONGS_EQUAL(0x00100080,RCC->AHB1ENR);
}

TEST(GPIO_init_test_group, GPIO_init){
	/*!
		  * @param Given :	user button (GPIOC, pin 13) has reset values
		  * @param When  :	user button initialized to accept interrupt at rising edge, with no pull up
		  * @param Then  :	The registers for PC and EXTI have appropriate values
	*/

	/* Arrange */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	SYSCFG->EXTICR[3] = 0;
	GPIOC->MODER = 0;
	GPIOC->PUPDR = 0;
	EXTI->IMR = 0;
	EXTI->RTSR = 0;

	/* Act */

	MX_GPIO_Init();

	/*Assert*/
	LONGS_EQUAL(0x00000020, SYSCFG->EXTICR[3]);
	LONGS_EQUAL(0x01500000, GPIOC->MODER);
	LONGS_EQUAL(0x01500000, GPIOC->PUPDR);
	LONGS_EQUAL(0x00002000,EXTI->IMR);
	LONGS_EQUAL(0x00002000,EXTI->RTSR);
}

TEST(USART_init_test_group, USART2_init){
	/*!
		  * @param Given :	registers that control USART2 have reset values
		  * @param When  :	USART2 is initialized with: 115200 baud rate, 8-bit data, 1 stop-bit, no parity
		  * 				TX and RX enabled, no HW control, oversampling 16
		  * @param Then  :	control registers for USART2 have appropriate values
	*/

	/* Arrange */
	unity_huart2.Instance = USART2;
	unity_huart2.Instance->CR1 = 0;
	unity_huart2.Instance->BRR = 0;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIOA->MODER = 0xA8000000; // Reset value for PA
	GPIOA->AFR[0] = 0;

	/* Act */
	MX_USART2_UART_Init();


	/*Assert*/
	LONGS_EQUAL(0x0000200C, unity_huart2.Instance->CR1);
	LONGS_EQUAL(0x0000008B, unity_huart2.Instance->BRR);
	LONGS_EQUAL(0xA80000A0, GPIOA->MODER); // Pins PA2 and PA3 in alternate fn mode
	LONGS_EQUAL(0x00007700, GPIOA->AFR[0]); // Pins PA2 and PA3 in alternate fn mode no. 7
}

TEST(RTC_init_test_group, RTC_init){

	/*!
		  * @param Given :	rtc module has reset value
		  * @param When  :	rtc is initialized to: 24 hour format, 127 asynchornous prediv, 255 synchronous prediv, no output, high polarity, open drain
		  * 				no daylight savings, store operation reset, sunday, february, date 27, year 22
		  * @param Then  :	The necessary registers have appropriate values
	*/

	/* Arrange */
	//-------------

	/* Act */
	SystemClock_Config();
	MX_RTC_Init();
	RTC_Calender_Configuration();

	/* Assert */
	LONGS_EQUAL(0x0022E227, unity_hrtc.Instance->DR); // Value for Monday, 27th of February, 2022 (0x0016221B for BCD) - (0x00222227 for BIN)
	LONGS_EQUAL(0x00011200, unity_hrtc.Instance->TR); // (0x00010c00 for BCD) - (0x00011200 for BIN) (Why is the 22nd bit not set? --> because the FMT bit in CR register must be set first)
}


TEST_GROUP_RUNNER(GPIO_clk_test_group){

	RUN_TEST_CASE(GPIO_clk_test_group, GPIOA_clk_en);
	RUN_TEST_CASE(GPIO_clk_test_group, GPIOB_clk_en);
	RUN_TEST_CASE(GPIO_clk_test_group, GPIOC_clk_en);
	RUN_TEST_CASE(GPIO_clk_test_group, GPIOD_clk_en);
	RUN_TEST_CASE(GPIO_clk_test_group, GPIOG_clk_en);
	RUN_TEST_CASE(GPIO_clk_test_group, GPIOH_clk_en);

}

TEST_GROUP_RUNNER(GPIO_init_test_group){

	RUN_TEST_CASE(GPIO_init_test_group, GPIO_init);

}

TEST_GROUP_RUNNER(USART_init_test_group){

	RUN_TEST_CASE(USART_init_test_group, USART2_init);

}

TEST_GROUP_RUNNER(RTC_init_test_group){

	RUN_TEST_CASE(RTC_init_test_group, RTC_init);

}

