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
#include "tim.h"
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

#include "tim.h"

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

#define DEBUG_LOG               1U

#define APP_MODE_LPC_TEST       1U

#define APP_T2T_CHUNK_LEN       RFAL_T2T_READ_DATA_LEN
#define APP_T2T_START_PAGE      4U
#define APP_T2T_CMD_FAST_READ   0x3AU
#define APP_T2T_PAGE_LEN        4U
#define APP_T2T_FAST_FWT        rfalConvMsTo1fc(6U)
#define APP_USE_T2T_FAST_READ   1U

#define APP_STREAM_INTERVAL_US   ((uint32_t)APP_STREAM_INTERVAL_MS * 1000U)
#define APP_DEADLINE_SLACK_US    500U
#define APP_STAT_READ_WINDOW	 20U

#if APP_MODE_LPC_TEST
  #define APP_RAW_READ_LEN        100U
  #define APP_PAYLOAD_LEN         100U
  #define APP_STREAM_INTERVAL_MS  10U
#else
  #define APP_RAW_READ_LEN        100U
  #define APP_PAYLOAD_LEN         100U
  #define APP_STREAM_INTERVAL_MS  10U
#endif

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

static uint32_t stat_last_ms = 0;
static uint32_t read_ok_cnt = 0;
static uint32_t read_err_cnt = 0;
static uint32_t read_overrun_cnt = 0;
static uint32_t payload_change_cnt = 0;
static uint32_t valid_frame_cnt = 0;
static uint32_t invalid_frame_cnt = 0;
static uint8_t stats_armed = 0;

static uint32_t read_us_last = 0;
static uint16_t last_crc16 = 0;

static uint32_t read_time_us_last = 0;
static uint32_t read_time_us_min = 0xFFFFFFFFU;

static uint32_t read_time_us_max = 0U;

static uint32_t cycle_time_us_last = 0;
static uint32_t cycle_time_us_min = 0xFFFFFFFFU;
static uint32_t cycle_time_us_max = 0U;

static uint16_t last_read_start_us = 0;
static uint32_t missed_deadline_cnt = 0;
static uint16_t last_cycle_mark_us = 0;

static uint32_t stat_read_count = 0;
static uint32_t stat_read_sum_us = 0;
static uint32_t stat_cycle_sum_us = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

static ReturnCode appReadCurrentTag(rfalNfcDevice *dev, uint8_t *buf, uint16_t bufSize, uint16_t *outLen);
static void appProcessCurrentTagData(rfalNfcDevice *dev, const uint8_t *buf, uint16_t len);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static inline uint16_t timer_now_us(void)
{
  return (uint16_t)__HAL_TIM_GET_COUNTER(&htim2);
}

static inline uint32_t timer_diff_us(uint16_t newer, uint16_t older)
{
  return (uint16_t)(newer - older);
}

static inline void stat_update_minmax(uint32_t v, uint32_t *minv, uint32_t *maxv)
{
  if (v < *minv) *minv = v;
  if (v > *maxv) *maxv = v;
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
  dbg_print("BUILD: timing_v3_16B_10ms\r\n");
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

static ReturnCode t2t_fast_read_bytes(uint8_t startPage, uint8_t *dst, uint16_t wantLen, uint16_t *outLen)
{
  ReturnCode rc;
  uint8_t req[3];
  uint8_t endPage;
  uint16_t rcvLen = 0;
  uint16_t pagesToRead;

  if ((dst == NULL) || (outLen == NULL))
  {
    return ERR_PARAM;
  }

  if ((wantLen == 0U) || ((wantLen % APP_T2T_PAGE_LEN) != 0U))
  {
    *outLen = 0;
    return ERR_PARAM;
  }

  pagesToRead = (uint16_t)(wantLen / APP_T2T_PAGE_LEN);
  endPage = (uint8_t)(startPage + pagesToRead - 1U);

  req[0] = APP_T2T_CMD_FAST_READ;
  req[1] = startPage;
  req[2] = endPage;

  rc = rfalTransceiveBlockingTxRx(req,
                                  sizeof(req),
                                  dst,
                                  wantLen,
                                  &rcvLen,
                                  RFAL_TXRX_FLAGS_DEFAULT,
                                  APP_T2T_FAST_FWT);

  if (rc != ERR_NONE)
  {
    *outLen = rcvLen;
    return rc;
  }

  if (rcvLen != wantLen)
  {
    *outLen = rcvLen;
    return ERR_REQUEST;
  }

  *outLen = rcvLen;
  return ERR_NONE;
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
  (void)devCnt;
  (void)state;
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
  ReturnCode rc;

  if (bufSize < APP_RAW_READ_LEN)
  {
    *outLen = 0;
    return ERR_PARAM;
  }

  if ((dev->type != RFAL_NFC_LISTEN_TYPE_NFCA) ||
      (dev->dev.nfca.type != RFAL_NFCA_T2T))
  {
    *outLen = 0;
    return ERR_REQUEST;
  }

#if APP_USE_T2T_FAST_READ
  rc = t2t_fast_read_bytes(APP_T2T_START_PAGE, buf, APP_RAW_READ_LEN, outLen);
  if (rc == ERR_NONE)
  {
    return ERR_NONE;
  }

#if DEBUG_LOG
  {
    char s[80];
    snprintf(s, sizeof(s), "FAST_READ rc=%d -> fallback READ\r\n", (int)rc);
    dbg_print(s);
  }
#endif
#endif

  return t2t_read_bytes(APP_T2T_START_PAGE, buf, APP_RAW_READ_LEN, outLen);
}

static void appProcessCurrentTagData(rfalNfcDevice *dev, const uint8_t *buf, uint16_t len)
{
  (void)dev;
  (void)buf;

  if (len < APP_PAYLOAD_LEN)
  {
    invalid_frame_cnt++;
    return;
  }

  valid_frame_cnt++;
}

static void appStartDiscover(void)
{
  ReturnCode err;
#if DEBUG_LOG
  char s[64];
#endif

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
  uint16_t now_us;

  rfalNfcGetDevicesFound(&devList, &devCnt);
  appLogHeartbeat(devCnt, state);

  if ((devCnt > 0U) && (state == RFAL_NFC_STATE_ACTIVATED))
  {
    appReportTag(&devList[0]);

    now_us = timer_now_us();

    stat_last_ms = HAL_GetTick();
    read_ok_cnt = 0;
    read_err_cnt = 0;
    read_overrun_cnt = 0;
    payload_change_cnt = 0;
    valid_frame_cnt = 0;
    invalid_frame_cnt = 0;
    missed_deadline_cnt = 0;

    stat_read_count = 0;
    stat_read_sum_us = 0;
    stat_cycle_sum_us = 0;


    read_time_us_last = 0;
    read_time_us_min = 0xFFFFFFFFU;
    read_time_us_max = 0U;

    cycle_time_us_last = 0;
    cycle_time_us_min = 0xFFFFFFFFU;
    cycle_time_us_max = 0U;

    last_read_start_us = 0;
    last_cycle_mark_us = now_us;
    stats_armed = 0;

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
  uint16_t now_us;
  uint16_t t0_us;
  uint16_t t1_us;
  uint32_t cycle_us;

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

  now_us = timer_now_us();
  cycle_us = timer_diff_us(now_us, last_cycle_mark_us);

  if (cycle_us < APP_STREAM_INTERVAL_US)
  {
    return;
  }

  cycle_time_us_last = cycle_us;
  stat_update_minmax(cycle_us, &cycle_time_us_min, &cycle_time_us_max);

  if (cycle_us > (APP_STREAM_INTERVAL_US + APP_DEADLINE_SLACK_US))
  {
    read_overrun_cnt++;
    missed_deadline_cnt++;
  }

  /* Deadline-basiert weiterschieben, damit kein Drift entsteht */
  do
  {
    last_cycle_mark_us = (uint16_t)(last_cycle_mark_us + APP_STREAM_INTERVAL_US);
  }
  while (timer_diff_us(now_us, last_cycle_mark_us) >= APP_STREAM_INTERVAL_US);

  dev = &devList[0];

  t0_us = timer_now_us();
  last_read_start_us = t0_us;

  rc = appReadCurrentTag(dev, rx, sizeof(rx), &rcvLen);

  t1_us = timer_now_us();
  read_time_us_last = timer_diff_us(t1_us, t0_us);
  stat_update_minmax(read_time_us_last, &read_time_us_min, &read_time_us_max);

  if (rc != ERR_NONE)
  {
    read_err_cnt++;
#if DEBUG_LOG
    {
      char s[80];
      snprintf(s, sizeof(s), "READ ERROR rc=%d -> rediscover\r\n", (int)rc);
      dbg_print(s);
    }
#endif
    appStartDiscover();
    return;
  }

  read_ok_cnt++;
  appProcessCurrentTagData(dev, rx, rcvLen);

  stat_read_count++;
  stat_read_sum_us += read_time_us_last;
  stat_cycle_sum_us += cycle_time_us_last;

  if (stat_read_count >= APP_STAT_READ_WINDOW)
  {
  #if DEBUG_LOG
    char s[260];
    uint32_t avg_read_us = stat_read_sum_us / stat_read_count;
    uint32_t avg_cycle_us = stat_cycle_sum_us / stat_read_count;
    uint32_t rate_hz = (avg_cycle_us > 0U) ? (1000000U / avg_cycle_us) : 0U;

    snprintf(s, sizeof(s),
      "N=%lu rate~=%luHz read_us=%lu [%lu..%lu] cycle_us=%lu [%lu..%lu] miss=%lu err=%lu valid=%lu invalid=%lu\r\n",
      (unsigned long)stat_read_count,
      (unsigned long)rate_hz,
      (unsigned long)avg_read_us,
      (unsigned long)read_time_us_min,
      (unsigned long)read_time_us_max,
      (unsigned long)avg_cycle_us,
      (unsigned long)cycle_time_us_min,
      (unsigned long)cycle_time_us_max,
      (unsigned long)missed_deadline_cnt,
      (unsigned long)read_err_cnt,
      (unsigned long)valid_frame_cnt,
      (unsigned long)invalid_frame_cnt);
    dbg_print(s);
  #endif

    stat_read_count = 0;
    stat_read_sum_us = 0;
    stat_cycle_sum_us = 0;

    read_ok_cnt = 0;
    read_err_cnt = 0;
    read_overrun_cnt = 0;
    missed_deadline_cnt = 0;
    valid_frame_cnt = 0;
    invalid_frame_cnt = 0;

    read_time_us_min = 0xFFFFFFFFU;
    read_time_us_max = 0U;
    cycle_time_us_min = 0xFFFFFFFFU;
    cycle_time_us_max = 0U;
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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Base_Start(&htim2);
  {
    char s[120];
    snprintf(s, sizeof(s),
             "SYSCLK=%lu HCLK=%lu PCLK1=%lu\r\n",
             HAL_RCC_GetSysClockFreq(),
             HAL_RCC_GetHCLKFreq(),
             HAL_RCC_GetPCLK1Freq());
    dbg_print(s);
  }
  {
    uint16_t t0 = timer_now_us();
    HAL_Delay(20);
    uint16_t t1 = timer_now_us();

    char s[80];
    snprintf(s, sizeof(s),
             "TIM2 delta for 20ms delay = %u\r\n",
             (unsigned)timer_diff_us(t1, t0));
    dbg_print(s);
  }

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
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
