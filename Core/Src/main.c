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
#include "rfal_rf.h"
#include "rfal_nfca.h"
#include "rfal_t2t.h"
#include "st_errno.h"

#include <string.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
  APP_STATE_DISCOVER = 0,
  APP_STATE_ACTIVE_READ
} AppState_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define DEBUG_LOG              0
U

#define APP_T2T_CHUNK_LEN      RFAL_T2T_READ_DATA_LEN   /* 16 Byte */
#define APP_T2T_START_PAGE     4U
#define APP_RAW_READ_LEN       112U                     /* 7x 16 Byte */
#define APP_PAYLOAD_LEN        100U
#define APP_STREAM_INTERVAL_MS 10U

#define APP_FRAME_SYNC1        0xAA
#define APP_FRAME_SYNC2        0x55

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile uint32_t exti_cb_cnt = 0;
volatile uint16_t last_exti_pin = 0;
volatile uint32_t st25r_irq_cnt = 0;

static AppState_t appState = APP_STATE_DISCOVER;
static rfalNfcDiscoverParam discParam;

static uint8_t  last_payload[APP_PAYLOAD_LEN];
static uint8_t  have_last_payload = 0;
static uint16_t frame_seq = 0;

static uint32_t last_status_ms = 0;
static uint32_t last_read_ms   = 0;
static uint32_t last_irq       = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

static ReturnCode appReadCurrentTag(rfalNfcDevice *dev, uint8_t *buf, uint16_t bufSize, uint16_t *outLen);
static void appProcessCurrentTagData(rfalNfcDevice *dev, const uint8_t *buf, uint16_t len);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



static void uart2_tx(const uint8_t *buf, uint16_t len)
{
  HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, 1000);
}

static void dbg_print(const char *s)
{
#if DEBUG_LOG
  HAL_UART_Transmit(&huart2, (uint8_t*)s, (uint16_t)strlen(s), 1000);
#else
  (void)s;
#endif
}

static void dbg_hexln(const uint8_t *b, uint16_t len)
{
#if DEBUG_LOG
  char t[5];
  for (uint16_t i = 0; i < len; i++) {
    snprintf(t, sizeof(t), "%02X ", b[i]);
    dbg_print(t);
  }
  dbg_print("\r\n");
#else
  (void)b;
  (void)len;
#endif
}

static uint16_t app_crc16_ccitt(const uint8_t *data, uint16_t len)
{
  uint16_t crc = 0xFFFF;

  for (uint16_t i = 0; i < len; i++)
  {
    crc ^= ((uint16_t)data[i] << 8);
    for (uint8_t j = 0; j < 8; j++)
    {
      if ((crc & 0x8000U) != 0U)
      {
        crc = (uint16_t)((crc << 1) ^ 0x1021U);
      }
      else
      {
        crc <<= 1;
      }
    }
  }
  return crc;
}

static void pcSendFrame(const uint8_t *payload, uint16_t len)
{
  uint8_t frame[2 + 2 + 2 + APP_PAYLOAD_LEN + 2];
  uint16_t idx = 0;
  uint16_t crc;

  frame[idx++] = APP_FRAME_SYNC1;
  frame[idx++] = APP_FRAME_SYNC2;

  frame[idx++] = (uint8_t)(frame_seq & 0xFFU);
  frame[idx++] = (uint8_t)((frame_seq >> 8) & 0xFFU);

  frame[idx++] = (uint8_t)(len & 0xFFU);
  frame[idx++] = (uint8_t)((len >> 8) & 0xFFU);

  memcpy(&frame[idx], payload, len);
  idx += len;

  crc = app_crc16_ccitt(frame, idx);
  frame[idx++] = (uint8_t)(crc & 0xFFU);
  frame[idx++] = (uint8_t)((crc >> 8) & 0xFFU);

  uart2_tx(frame, idx);
  frame_seq++;
}

static ReturnCode t2t_read_bytes(uint8_t startPage, uint8_t *dst, uint16_t wantLen, uint16_t *outLen)
{
  ReturnCode rc;
  uint16_t total = 0;
  uint8_t page = startPage;

  while (total < wantLen)
  {
    uint8_t rx[APP_T2T_CHUNK_LEN];
    uint16_t rcvLen = 0;

    rc = rfalT2TPollerRead(page, rx, sizeof(rx), &rcvLen);
    if (rc != ERR_NONE)
    {
      *outLen = total;
      return rc;
    }

    if (rcvLen != APP_T2T_CHUNK_LEN)
    {
      *outLen = total;
      return ERR_REQUEST;
    }

    {
      uint16_t copyLen = ((wantLen - total) > APP_T2T_CHUNK_LEN) ? APP_T2T_CHUNK_LEN : (wantLen - total);
      memcpy(&dst[total], rx, copyLen);
      total += copyLen;
    }

    page += 4U;   /* READ(page) liest 4 Pages = 16 Byte */
  }

  *outLen = total;
  return ERR_NONE;
}

static void appLogHeartbeat(uint8_t devCnt, rfalNfcState state)
{
#if DEBUG_LOG
  if ((HAL_GetTick() - last_status_ms) >= 1000U)
  {
    char s[140];
    GPIO_PinState p0 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    GPIO_PinState p1 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);

    last_status_ms = HAL_GetTick();

    snprintf(s, sizeof(s),
      "APP=%u cb=%lu last=0x%04X irq/s=%lu PA0=%d PA1=%d devCnt=%u state=%d\r\n",
      (unsigned)appState,
      (unsigned long)exti_cb_cnt,
      (unsigned)last_exti_pin,
      (unsigned long)(st25r_irq_cnt - last_irq),
      (int)p0, (int)p1,
      (unsigned)devCnt,
      (int)state);

    last_irq = st25r_irq_cnt;
    dbg_print(s);
  }
#else
  (void)devCnt;
  (void)state;
#endif
}

static void appReportTag(rfalNfcDevice *dev)
{
#if DEBUG_LOG
  char s[160];

  snprintf(s, sizeof(s),
    "TAG FOUND: type=%u rfIf=%u uidLen=%u\r\n",
    (unsigned)dev->type,
    (unsigned)dev->rfInterface,
    (unsigned)dev->nfcidLen);
  dbg_print(s);

  dbg_print("UID: ");
  dbg_hexln(dev->nfcid, dev->nfcidLen);

  if (dev->type == RFAL_NFC_LISTEN_TYPE_NFCA)
  {
    snprintf(s, sizeof(s),
      "NFCA subtype=%u SAK=0x%02X\r\n",
      (unsigned)dev->dev.nfca.type,
      (unsigned)dev->dev.nfca.selRes.sak);
    dbg_print(s);
  }
#else
  (void)dev;
#endif
}

static ReturnCode appReadCurrentTag(rfalNfcDevice *dev, uint8_t *buf, uint16_t bufSize, uint16_t *outLen)
{
  if (bufSize < APP_RAW_READ_LEN)
  {
    *outLen = 0;
    return ERR_PARAM;
  }

  if ((dev->type == RFAL_NFC_LISTEN_TYPE_NFCA) &&
      (dev->dev.nfca.type == RFAL_NFCA_T2T))
  {
    return t2t_read_bytes(APP_T2T_START_PAGE, buf, APP_RAW_READ_LEN, outLen);
  }

  *outLen = 0;
  return ERR_REQUEST;
}

static void appProcessCurrentTagData(rfalNfcDevice *dev, const uint8_t *buf, uint16_t len)
{
  uint8_t payload[APP_PAYLOAD_LEN];

  (void)dev;

  if (len < APP_PAYLOAD_LEN)
  {
#if DEBUG_LOG
    dbg_print("RAW buffer shorter than 100 bytes\r\n");
#endif
    return;
  }

  memcpy(payload, buf, APP_PAYLOAD_LEN);
  pcSendFrame(payload, APP_PAYLOAD_LEN);

#if DEBUG_LOG
  if ((!have_last_payload) || (memcmp(last_payload, payload, APP_PAYLOAD_LEN) != 0))
  {
    dbg_print("PAYLOAD CHANGED\r\n");
    dbg_hexln(payload, APP_PAYLOAD_LEN);
    memcpy(last_payload, payload, APP_PAYLOAD_LEN);
    have_last_payload = 1;
  }
#endif
}

static void appStartDiscover(void)
{
  ReturnCode err;
#if DEBUG_LOG
  char s[64];
#endif

  have_last_payload = 0;

  err = rfalNfcDeactivate(false);
#if DEBUG_LOG
  if ((int)err != 33)   /* wrong state on first startup is harmless in this project */
  {
    snprintf(s, sizeof(s), "rfalNfcDeactivate err=%d\r\n", (int)err);
    dbg_print(s);
  }
#endif

  err = rfalNfcDiscover(&discParam);
#if DEBUG_LOG
  snprintf(s, sizeof(s), "rfalNfcDiscover err=%d\r\n", (int)err);
  dbg_print(s);
#endif

  appState = APP_STATE_DISCOVER;
}

static void appHandleDiscover(void)
{
  rfalNfcDevice *devList = NULL;
  uint8_t devCnt = 0;
  rfalNfcState state = rfalNfcGetState();

  rfalNfcGetDevicesFound(&devList, &devCnt);
  appLogHeartbeat(devCnt, state);

  if ((devCnt > 0U) && (state == RFAL_NFC_STATE_ACTIVATED))
  {
    appReportTag(&devList[0]);
    have_last_payload = 0;
    last_read_ms = HAL_GetTick();
    appState = APP_STATE_ACTIVE_READ;
  }
}

static void appHandleActiveRead(void)
{
  rfalNfcDevice *devList = NULL;
  uint8_t devCnt = 0;
  rfalNfcState state = rfalNfcGetState();
  uint8_t rx[APP_RAW_READ_LEN];
  uint16_t rcvLen = 0;
  ReturnCode rc;
  rfalNfcDevice *dev;

  rfalNfcGetDevicesFound(&devList, &devCnt);
  appLogHeartbeat(devCnt, state);

  if ((devCnt == 0U) || (state != RFAL_NFC_STATE_ACTIVATED))
  {
#if DEBUG_LOG
    dbg_print("TAG LOST -> rediscover\r\n");
#endif
    appStartDiscover();
    return;
  }

  if ((HAL_GetTick() - last_read_ms) < APP_STREAM_INTERVAL_MS)
  {
    return;
  }
  last_read_ms = HAL_GetTick();

  dev = &devList[0];
  rc = appReadCurrentTag(dev, rx, sizeof(rx), &rcvLen);

  if (rc != ERR_NONE)
  {
#if DEBUG_LOG
    char s[80];
    snprintf(s, sizeof(s), "READ ERROR rc=%d -> rediscover\r\n", (int)rc);
    dbg_print(s);
#endif
    appStartDiscover();
    return;
  }

  appProcessCurrentTagData(dev, rx, rcvLen);
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

  dbg_print("UART OK\r\n");
  platformResetST25R();
  dbg_print("Reset done\r\n");

  ReturnCode err;
  char b[48];

  err = rfalInitialize();
  snprintf(b, sizeof(b), "rfalInitialize err=%d\r\n", (int)err);
  dbg_print(b);
  if (err != ERR_NONE) Error_Handler();

  err = rfalNfcInitialize();
  snprintf(b, sizeof(b), "rfalNfcInitialize err=%d\r\n", (int)err);
  dbg_print(b);
  if (err != ERR_NONE) Error_Handler();

  memset(&discParam, 0, sizeof(discParam));
  discParam.compMode      = RFAL_COMPLIANCE_MODE_NFC;
  discParam.devLimit      = 1;
  discParam.techs2Find    = RFAL_NFC_POLL_TECH_A;
  discParam.totalDuration = 3000;

  appStartDiscover();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    rfalNfcWorker();

    switch (appState)
    {
      case APP_STATE_DISCOVER:
        appHandleDiscover();
        break;

      case APP_STATE_ACTIVE_READ:
        appHandleActiveRead();
        break;

      default:
        appStartDiscover();
        break;
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
  exti_cb_cnt++;
  last_exti_pin = GPIO_Pin;

  if (GPIO_Pin == GPIO_PIN_0) {        // PA0 = IRQ_MCU
    st25r_irq_cnt++;
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
