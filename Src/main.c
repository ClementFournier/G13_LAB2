/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */
#include <math.h>

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

DAC_HandleTypeDef hdac;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

int sampleNB = 100; //CHANGE FOR SAMPLE NUMBER
int sample = 0;

#define MATH_ARRAY_SIZE 5
int displayMode = 0;
float filterMemory [] = {0, 0, 0, 0, 0};
int adc_val;
float filtered_adc;
float mathResults [MATH_ARRAY_SIZE];
extern uint8_t systickFlag;
extern uint8_t buttonFlag;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_DAC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void FIR_C(float input, float *output);
//int buttonPressed();
void displayNum (int num, int pos);
void C_math (float * inputArray, float * outputArray, int length);
void display (int mode, float num);


/* USER CODE END PFP */

/* USER CODE BEGIN 0 */




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	float data [sampleNB];

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_ADC1_Init();
  MX_DAC_Init();
  /* USER CODE BEGIN 2 */
		printf("begin!");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
	
	while (1){
		if(systickFlag == 1){			
			
			systickFlag = 0;

			int dac_val = 0x30;
			HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_8B_R, dac_val);    //DAC is 8 bits resolution
		
			HAL_ADC_Start_IT(&hadc1);
		
			adc_val = HAL_ADC_GetValue(&hadc1);
			//FIR_C(adc_val, &filtered_adc);
			float test = (float)adc_val*3.0/1023.0;
			FIR_C(test, &filtered_adc);             //filter ADC value
			data [sample] = filtered_adc;           //store filtered data in array
		
			sample ++;
			sample = sample % sampleNB;
		}
		if ( buttonFlag == 1){
			buttonFlag = 0;
			displayMode = displayMode + 1;
			displayMode = displayMode % 3;
			printf("displayMode = %d \n!", displayMode);
		}
	
		C_math (&data[0], &mathResults [0], sampleNB);                //perform math operation on data to get RMS, min and max values
	
		
		switch(displayMode) {
			case 0:
				HAL_GPIO_WritePin(GPIOD, LD3_Pin, GPIO_PIN_SET); // switch on LED6
				HAL_GPIO_WritePin(GPIOD, LD4_Pin, GPIO_PIN_RESET); // reset LED4
				HAL_GPIO_WritePin(GPIOD, LD5_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, LD6_Pin, GPIO_PIN_RESET);
				break;
			case 1:
				HAL_GPIO_WritePin(GPIOD, LD4_Pin, GPIO_PIN_SET); // switch on LED5
				HAL_GPIO_WritePin(GPIOD, LD3_Pin, GPIO_PIN_RESET); // reset LED6
				HAL_GPIO_WritePin(GPIOD, LD5_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, LD6_Pin, GPIO_PIN_RESET);
				break;
			case 2:
				HAL_GPIO_WritePin(GPIOD, LD5_Pin, GPIO_PIN_SET); // switch on LED4
				HAL_GPIO_WritePin(GPIOD, LD3_Pin, GPIO_PIN_RESET); // reset LED5
				HAL_GPIO_WritePin(GPIOD, LD4_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, LD6_Pin, GPIO_PIN_RESET);
				break;
			default:
				break;
		}
		
		display (displayMode, mathResults [displayMode]);
		
		
  /* USER CODE END WHILE */

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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_10B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* DAC init function */
static void MX_DAC_Init(void)
{

  DAC_ChannelConfTypeDef sConfig;

    /**DAC Initialization 
    */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**DAC channel OUT1 config 
    */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
     PC3   ------> I2S2_SD
     PA5   ------> SPI1_SCK
     PA6   ------> SPI1_MISO
     PA7   ------> SPI1_MOSI
     PB10   ------> I2S2_CK
     PC7   ------> I2S3_MCK
     PA9   ------> USB_OTG_FS_VBUS
     PA10   ------> USB_OTG_FS_ID
     PA11   ------> USB_OTG_FS_DM
     PA12   ------> USB_OTG_FS_DP
     PC10   ------> I2S3_CK
     PC12   ------> I2S3_SD
     PB6   ------> I2C1_SCL
     PB9   ------> I2C1_SDA
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, CS_I2C_SPI_Pin|Dig_1_Pin|Dig_2_Pin|Dig_L_Pin 
                          |Dig_3_Pin|Dig_4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Seg_DP_Pin|Seg_G_Pin|Seg_F_Pin|Seg_E_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, Seg_D_Pin|Seg_C_Pin|Seg_B_Pin|Seg_A_Pin 
                          |LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin 
                          |Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : CS_I2C_SPI_Pin Dig_1_Pin Dig_2_Pin Dig_L_Pin 
                           Dig_3_Pin Dig_4_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin|Dig_1_Pin|Dig_2_Pin|Dig_L_Pin 
                          |Dig_3_Pin|Dig_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PDM_OUT_Pin */
  GPIO_InitStruct.Pin = PDM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI1_SCK_Pin SPI1_MISO_Pin SPI1_MOSI_Pin */
  GPIO_InitStruct.Pin = SPI1_SCK_Pin|SPI1_MISO_Pin|SPI1_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CLK_IN_Pin */
  GPIO_InitStruct.Pin = CLK_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(CLK_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Seg_DP_Pin Seg_G_Pin Seg_F_Pin Seg_E_Pin */
  GPIO_InitStruct.Pin = Seg_DP_Pin|Seg_G_Pin|Seg_F_Pin|Seg_E_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Seg_D_Pin Seg_C_Pin Seg_B_Pin Seg_A_Pin 
                           LD4_Pin LD3_Pin LD5_Pin LD6_Pin 
                           Audio_RST_Pin */
  GPIO_InitStruct.Pin = Seg_D_Pin|Seg_C_Pin|Seg_B_Pin|Seg_A_Pin 
                          |LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin 
                          |Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : I2S3_MCK_Pin I2S3_SCK_Pin I2S3_SD_Pin */
  GPIO_InitStruct.Pin = I2S3_MCK_Pin|I2S3_SCK_Pin|I2S3_SD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : VBUS_FS_Pin */
  GPIO_InitStruct.Pin = VBUS_FS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VBUS_FS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OTG_FS_ID_Pin OTG_FS_DM_Pin OTG_FS_DP_Pin */
  GPIO_InitStruct.Pin = OTG_FS_ID_Pin|OTG_FS_DM_Pin|OTG_FS_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Audio_SCL_Pin Audio_SDA_Pin */
  GPIO_InitStruct.Pin = Audio_SCL_Pin|Audio_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

/* USER CODE BEGIN 4 */
/**
  * @brief  This function displays a integer on one of the digits of a 7 segiment, 4 digit led display
	* @param  num: integer number to be displayed 
						pos: the location in a 4 digit display,
  * @retval none
  */
void displayNum (int num, int pos) {       //function used to diplay a single digit on LED segments
	switch (num){
		case 0:
			HAL_GPIO_WritePin(GPIOD, Seg_A_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_B_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_C_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_D_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_E_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_F_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_G_Pin, GPIO_PIN_RESET);
			break;
		case 1:
			HAL_GPIO_WritePin(GPIOD, Seg_A_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, Seg_B_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_C_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_D_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_E_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_F_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_G_Pin, GPIO_PIN_RESET);
			break;
		case 2:
			HAL_GPIO_WritePin(GPIOD, Seg_A_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_B_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_C_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, Seg_D_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_E_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_F_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_G_Pin, GPIO_PIN_SET);
			break;
		
		case 3:
			HAL_GPIO_WritePin(GPIOD, Seg_A_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_B_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_C_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_D_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_E_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_F_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_G_Pin, GPIO_PIN_SET);
			break;
		
		case 4:
			HAL_GPIO_WritePin(GPIOD, Seg_A_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, Seg_B_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_C_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_D_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_E_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_F_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_G_Pin, GPIO_PIN_SET);
			break;
		
		case 5:
			HAL_GPIO_WritePin(GPIOD, Seg_A_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_B_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, Seg_C_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_D_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_E_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_F_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_G_Pin, GPIO_PIN_SET);
			break;
		
		case 6:
			HAL_GPIO_WritePin(GPIOD, Seg_A_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_B_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, Seg_C_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_D_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_E_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_F_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_G_Pin, GPIO_PIN_SET);
			break;
		
		case 7:
			HAL_GPIO_WritePin(GPIOD, Seg_A_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_B_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_C_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_D_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_E_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_F_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_G_Pin, GPIO_PIN_RESET);
			break;
		
		case 8:
			HAL_GPIO_WritePin(GPIOD, Seg_A_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_B_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_C_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_D_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_E_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_F_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_G_Pin, GPIO_PIN_SET);
			break;
		case 9:
			HAL_GPIO_WritePin(GPIOD, Seg_A_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_B_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_C_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, Seg_D_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_E_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, Seg_F_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, Seg_G_Pin, GPIO_PIN_SET);
			break;
		default :
			break;
	}
		
		switch (pos){
			case 0:
				HAL_GPIO_WritePin(GPIOE, Dig_1_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOE, Dig_2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_L_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_3_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_4_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, Seg_DP_Pin, GPIO_PIN_RESET);
			break;
			case 1:
				HAL_GPIO_WritePin(GPIOE, Dig_1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_2_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOE, Dig_L_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_3_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_4_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, Seg_DP_Pin, GPIO_PIN_SET);
			break;
			case 2:
				HAL_GPIO_WritePin(GPIOE, Dig_1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_L_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_3_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOE, Dig_4_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, Seg_DP_Pin, GPIO_PIN_RESET);
			break;
			case 3 :
				HAL_GPIO_WritePin(GPIOE, Dig_1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_L_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_3_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, Dig_4_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB, Seg_DP_Pin, GPIO_PIN_RESET);
			break;
			default :
				break;
		}
	
}
/**
  * @brief  This is the FIR function from LAB 1
						Filters data using FIR to eliminate unwanted noise
	* @param  Input: data to be filtered
						Output: filtered data
  * @retval none
  */
void FIR_C(float Input, float *Output) {   
	float coef [] = {0.2, 0.2, 0.2, 0.2, 0.2};          //coefficients -> SHOULD ADD UP TO 1 !
	float out = 0.0;
	for (int i = 4; i>0; i--){
		filterMemory[i] = filterMemory[i-1];
	}
	filterMemory [0] = Input;
	for (int i = 0; i<5; i++){
		out += coef[i]*filterMemory[i];
	}
	*Output = out;
}


/**
  * @brief  This is the C?math function from lab1
						calculates rms, max, min, min index, and max index from the given array
	* @param  inputArray: contains the samples to be analyzed
						outputArray: contains the array where the results are stored
						length: length of the input array
  * @retval none
  */
void C_math (float * inputArray, float * outputArray, int length){      //C_math function from lab 1

	float RMS=0;
	float maxVal = inputArray[0];
	float minVal = inputArray[0];
	int maxInd = 0;
	int minInd = 0;
	
	
	for (int i = 0; i< (length); i++){
		
		RMS += (inputArray[i])*(inputArray[i]);
		
		if ((inputArray[i]) > maxVal){
			maxVal = inputArray[i];
			maxInd = i;
		}
		if ((inputArray[i]) < minVal){
			minVal = (inputArray[i]);
			minInd = i;
		}
	}
	
	RMS = RMS/length;
	RMS = sqrt(RMS);
	
	outputArray [0]=RMS;
	outputArray [1]= maxVal;
	outputArray [2]= minVal;
	outputArray [3]= maxInd;
	outputArray [4]= minInd;
	
}

/**
  * @brief  detects a blue button press
  * @param  none
  * @retval returns 1 when button is pressed
						0 otherwise
  */
/*
int buttonPressed(){    
	if ( HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0 ) == SET) {
		HAL_Delay(200);
		printf("button (pulled)!");
		return 1;
	}
	else 
	{
		return 0;
	}
}
*/
/**
  * @brief  function used to display a float value on the LED segments
  * @param  num: the floating number to be displayed at digits 1, 2, and 3
*         	mode: the int thats displayed at the firist digit to indicate rms/min/max mode 
  * @retval None
  */
void display (int mode, float num){     //
	
	
	displayNum (mode, 0); //display mode at location 0 (first LED display)
	HAL_Delay(1);
	
	float temp = num;
	int digit = (int) num;
	
	
	digit = digit % 10;
	displayNum (digit, 1); //display unit at location 1
	HAL_Delay(1);
	
	temp = num*10;
	digit = (int) temp;
	digit = digit % 10;
	displayNum (digit, 2); //display digit at location 2
	HAL_Delay(1);
	
	temp = num*100;
	digit = (int) temp;
	digit = digit % 10;
	displayNum (digit, 3); //display digit at location 3
	HAL_Delay(1);
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
