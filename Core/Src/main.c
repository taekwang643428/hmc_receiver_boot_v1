/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "xmodem.h"
#include "uart.h"
#include "flash_if.h"

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
IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

typedef  void (*pFunction)(void);

pFunction JumpToApplication;
uint32_t JumpAddress;
uint32_t FlashProtection = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_IWDG_Init(void);
static void MX_UART5_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void phy_reset(void)
{
    HAL_GPIO_WritePin(PHY_RESET_GPIO_Port, PHY_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(PHY_RESET_GPIO_Port, PHY_RESET_Pin, GPIO_PIN_SET);
}
void pmdll_reset(void)
{
    HAL_GPIO_WritePin(PMDLL_RESET_GPIO_Port, PMDLL_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(PMDLL_RESET_GPIO_Port, PMDLL_RESET_Pin, GPIO_PIN_SET);
}


void flash_jump_to_app(void)
{
    /* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
    if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
    {
      /* Jump to user application */
      JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
      JumpToApplication = (pFunction) JumpAddress;
      /* Initialize user application's Stack Pointer */
      __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
      JumpToApplication();
   }

}

void flash_protect(void)
{
	if (FlashProtection != FLASHIF_PROTECTION_NONE)
	{
	/* Disable the write protection */
		if (FLASH_If_WriteProtectionConfig(OB_WRPSTATE_DISABLE) == HAL_OK)
		{
			//Serial_PutString((uint8_t *)"Write Protection disabled...\r\n");
			//Serial_PutString((uint8_t *)"System will now restart...\r\n");
			/* Launch the option byte loading */
			HAL_FLASH_OB_Launch();
			/* Ulock the flash */
			HAL_FLASH_Unlock();
		}
		else
		{
			//Serial_PutString((uint8_t *)"Error: Flash write un-protection failed...\r\n");
		}
	}
	else
	{
		if (FLASH_If_WriteProtectionConfig(OB_WRPSTATE_ENABLE) == HAL_OK)
		{
			//Serial_PutString((uint8_t *)"Write Protection enabled...\r\n");
			//Serial_PutString((uint8_t *)"System will now restart...\r\n");
			/* Launch the option byte loading */
			HAL_FLASH_OB_Launch();
		}
		else
		{
			//Serial_PutString((uint8_t *)"Error: Flash write protection failed...\r\n");
		}
	}
}




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

	int i;
	uint8_t rdata;
	int setup = 0;
	uint8_t temp_data;
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
  MX_UART4_Init();
  MX_USART1_UART_Init();
  MX_IWDG_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */


  uart_transmit_str((uint8_t*)"\n\r================================\n\r");
  uart_transmit_str((uint8_t*)"UART Bootloader Ver:1.1\n\r");
  /* Test if any sector of Flash memory where user application will be loaded is write protected */
  FlashProtection = FLASH_If_GetWriteProtectionStatus();

  if(FlashProtection != FLASHIF_PROTECTION_NONE)
  {
  uart_transmit_str((uint8_t *)"Protection  enabled\n\r");
  }
  uart_transmit_str((uint8_t *)"J : Jump to Main\n\r");
  uart_transmit_str((uint8_t *)"~ : Firmware Update(xmodem)\n\r");
  uart_transmit_str((uint8_t *)"S : DataLink Setup\n\r");
  uart_transmit_str((uint8_t *)"P : DataLink Serial\n\r");
  uart_transmit_str((uint8_t *)"After 5 seconds,  Auto Jump to Main\n\r");

  for(i = 0; i < 50; i++)
  {
      //HAL_Delay(100);
      if(setup == 0)
      {
          if(i%10 == 0)
          {
              uart_transmit_ch((uint8_t)('1'+(i/10)));
              //                  uart_transmit_str((uint8_t*)"\n\r");
              //uart_transmit_str((uint8_t *)"\033[F"); // Move cursor to the previous line
          }
      }
      if(HAL_UART_Receive(&huart4, (uint8_t*)&rdata, 1, 100) == HAL_OK)
      {
          if(rdata == 'J')
          {
              setup = 0;
              i = 0;
              HAL_GPIO_WritePin(PHY_RESET_GPIO_Port, PHY_RESET_Pin, GPIO_PIN_SET);
              flash_jump_to_app();

          }
          else if(rdata == '~')
          {
              setup = 1;
              HAL_GPIO_WritePin(PHY_RESET_GPIO_Port, PHY_RESET_Pin, GPIO_PIN_SET);
              i = 0;
              break;
          }
          else if(rdata == 'S')
          {
              uart_transmit_str((uint8_t *)"DataLink Setup start\n\r");
              setup = 2;
              i = 0;
              HAL_GPIO_WritePin(PHY_RESET_GPIO_Port, PHY_RESET_Pin, GPIO_PIN_RESET);
              break;
          }
          else if(rdata == 'P')
          {
              uart_transmit_str((uint8_t *)"DataLink Serial start\n\r");
              uart_transmit_str((uint8_t *)"waiting......\n\r");
              setup = 3;
              i = 0;
              HAL_GPIO_WritePin(PHY_RESET_GPIO_Port, PHY_RESET_Pin, GPIO_PIN_RESET);
              uart4_interupt_set(1);
              uart5_interupt_set(1);
              break;
          }
      }

  }

  if(setup == 0)
  {
      flash_jump_to_app();
  }


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


	  if(setup == 1)
	          {
	          /* Turn on the green LED to indicate, that we are in bootloader mode.*/
	          //	HAL_GPIO_WritePin(GPIOC, LD3_Pin, GPIO_PIN_SET);
	              /* Ask for new data and start the Xmodem protocol. */
	              uart_transmit_str((uint8_t*)"\n\rPlease send a new binary file with Xmodem protocol to update the firmware.\n\r");
	              xmodem_receive();
	              /* We only exit the xmodem protocol, if there are any errors.
	              * In that case, notify the user and start over. */
	              uart_transmit_str((uint8_t*)"\n\rFailed... Please try again.\n\r");
	          }
	          else if(setup == 3)
	          {
	              if(ringbuf_get(&uart4_buffer, &temp_data))
	              {
	                  uart5_transmit_ch(temp_data);
	              }
	              if(ringbuf_get(&uart5_buffer, &temp_data))
	              {
	                  uart_transmit_ch(temp_data);
	              }
	              else
	              {
	                  HAL_Delay(10);
	              }
	          }
	          else
	          {
	              HAL_Delay(10);
	          }
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PHY_RESET_GPIO_Port, PHY_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PMDLL_CONFIG_GPIO_Port, PMDLL_CONFIG_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PMDLL_RESET_GPIO_Port, PMDLL_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PHY_RESET_Pin */
  GPIO_InitStruct.Pin = PHY_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PHY_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LAN_TX_LED_Pin LAN_LINK_LED_Pin */
  GPIO_InitStruct.Pin = LAN_TX_LED_Pin|LAN_LINK_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : LAN_RX_LED_Pin PMDLL_STATUS_Pin */
  GPIO_InitStruct.Pin = LAN_RX_LED_Pin|PMDLL_STATUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PMDLL_CONFIG_Pin */
  GPIO_InitStruct.Pin = PMDLL_CONFIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PMDLL_CONFIG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PMDLL_RESET_Pin */
  GPIO_InitStruct.Pin = PMDLL_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PMDLL_RESET_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
#ifdef USE_FULL_ASSERT
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
