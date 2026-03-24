/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rfal_utils.h"
#include "rfal_nfc.h"
#include <string.h>
#include <stdio.h>
#include "rfal_rf.h"
#include "st_errno.h"

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



static void uart2_print(const char *s)
{
  HAL_UART_Transmit(&huart2, (uint8_t*)s, (uint16_t)strlen(s), 1000);
}

static void print_hex(const uint8_t *b, uint8_t len)
{
  char t[4];
  for(uint8_t i=0; i<len; i++){
    snprintf(t, sizeof(t), "%02X", b[i]);
    uart2_print(t);
  }
}
static uint8_t st25r300_read_reg(uint8_t reg) {
	uint8_t tx[2] = { (uint8_t) (0x80u | (reg & 0x7Fu)), 0x00 };
	uint8_t rx[2] = { 0x00, 0x00 };

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);      // CS low
	HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
	return rx[1];
}

static void st25r300_write_reg(uint8_t reg, uint8_t val) {
	uint8_t tx[2] = { (uint8_t) (reg & 0x7Fu), val }; // MSB=0 => write
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, tx, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
}

static void uart_print(const char *s)
{
  HAL_UART_Transmit(&huart2, (uint8_t*)s, (uint16_t)strlen(s), 1000);
}

static void uart_hex(const uint8_t *b, uint8_t len)
{
  char t[4];
  for(uint8_t i=0; i<len; i++){
    snprintf(t, sizeof(t), "%02X", b[i]);
    uart_print(t);
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
  MX_USART2_UART_Init();
  MX_SPI1_Init();

  /* USER CODE BEGIN 2 */

  uart2_print("UART OK\r\n");

  platformResetST25R();
  uart2_print("Reset done\r\n");

  ReturnCode err;

  err = rfalInitialize();
  char b[48];
  snprintf(b, sizeof(b), "rfalInitialize err=%d\r\n", (int)err);
  uart2_print(b);
  if(err != ERR_NONE) Error_Handler();

  err = rfalNfcInitialize();
  snprintf(b, sizeof(b), "rfalNfcInitialize err=%d\r\n", (int)err);
  uart2_print(b);
  if(err != ERR_NONE) Error_Handler();


  /* Discover param: NFC-V (zum Gegencheck) */
  static rfalNfcDiscoverParam discParam;
  memset(&discParam, 0, sizeof(discParam));
  discParam.compMode      = RFAL_COMPLIANCE_MODE_NFC;
  discParam.devLimit      = 1;
  discParam.techs2Find    = RFAL_NFC_POLL_TECH_V;
  discParam.totalDuration = 3000;

  err = rfalNfcDeactivate(false);
  err = rfalNfcDiscover(&discParam);
  uart2_print("rfalNfcDiscover err=");
  char buf[64];
  snprintf(buf, sizeof(buf), "%d\r\n", (int)err);
  uart2_print(buf);

  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  static uint32_t last = 0;

  while (1)
  {
	  rfalNfcWorker();

	  rfalNfcDevice *devList = NULL;
	  uint8_t devCnt = 0;
	  rfalNfcGetDevicesFound(&devList, &devCnt);

	  static uint8_t lastCnt = 0xFF;
	  if (devCnt != lastCnt) {
	    lastCnt = devCnt;
	    char s[32];
	    snprintf(s, sizeof(s), "devCnt=%u\r\n", devCnt);
	    uart2_print(s);
	  }
  }
  /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == ST25R_INT_PIN)
  {
    platformIrqST25RCallCallback();
  }
}
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
		while (1) {
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
