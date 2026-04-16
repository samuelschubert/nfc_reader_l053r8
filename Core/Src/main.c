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
#include "rfal_nfca.h"
#include "rfal_t2t.h"

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

volatile uint32_t exti_cb_cnt = 0;
volatile uint16_t last_exti_pin = 0;
volatile uint32_t st25r_irq_cnt = 0;

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

#define APP_T2T_CHUNK_LEN   RFAL_T2T_READ_DATA_LEN   /* 16 Byte */
#define APP_T2T_MAX_NDEF    128U

static void uart_hexln(const uint8_t *b, uint16_t len)
{
  char t[5];
  for (uint16_t i = 0; i < len; i++) {
    snprintf(t, sizeof(t), "%02X ", b[i]);
    uart2_print(t);
  }
  uart2_print("\r\n");
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

    uint16_t copyLen = (wantLen - total > APP_T2T_CHUNK_LEN) ? APP_T2T_CHUNK_LEN : (wantLen - total);
    memcpy(&dst[total], rx, copyLen);
    total += copyLen;

    page += 4;   /* READ(page) liest 4 Pages = 16 Byte */
  }

  *outLen = total;
  return ERR_NONE;
}

static void t2t_print_ndef_text(const uint8_t *buf, uint16_t len)
{
  char s[96];
  uint16_t i = 0;

  /* TLV suchen */
  while (i < len)
  {
    uint8_t tlv = buf[i];

    if (tlv == 0x00)   /* NULL TLV */
    {
      i++;
      continue;
    }

    if (tlv == 0xFE)   /* Terminator */
    {
      uart2_print("NDEF: terminator reached, no record\r\n");
      return;
    }

    if (tlv != 0x03)   /* nicht NDEF Message TLV */
    {
      snprintf(s, sizeof(s), "NDEF: unsupported TLV 0x%02X\r\n", tlv);
      uart2_print(s);
      return;
    }

    break;
  }

  if (i + 1 >= len)
  {
    uart2_print("NDEF: TLV too short\r\n");
    return;
  }

  uint16_t ndefLen = 0;
  uint16_t ndefStart = 0;

  if (buf[i + 1] == 0xFF)
  {
    if (i + 3 >= len)
    {
      uart2_print("NDEF: extended length header incomplete\r\n");
      return;
    }
    ndefLen = ((uint16_t)buf[i + 2] << 8) | buf[i + 3];
    ndefStart = i + 4;
  }
  else
  {
    ndefLen = buf[i + 1];
    ndefStart = i + 2;
  }

  snprintf(s, sizeof(s), "NDEF length=%u\r\n", (unsigned)ndefLen);
  uart2_print(s);

  if ((ndefStart + ndefLen) > len)
  {
    uart2_print("NDEF: buffer too small for full message\r\n");
    return;
  }

  if (ndefLen < 5)
  {
    uart2_print("NDEF: record too short\r\n");
    return;
  }

  const uint8_t *rec = &buf[ndefStart];

  uint8_t hdr = rec[0];
  uint8_t typeLen = rec[1];
  uint32_t payloadLen = 0;
  uint16_t pos = 2;

  if (hdr & 0x10)   /* SR = short record */
  {
    payloadLen = rec[pos];
    pos += 1;
  }
  else
  {
    payloadLen = ((uint32_t)rec[pos] << 24) |
                 ((uint32_t)rec[pos + 1] << 16) |
                 ((uint32_t)rec[pos + 2] << 8) |
                 ((uint32_t)rec[pos + 3]);
    pos += 4;
  }

  uint8_t idLen = 0;
  if (hdr & 0x08)   /* IL */
  {
    idLen = rec[pos];
    pos += 1;
  }

  if ((pos + typeLen + idLen + payloadLen) > ndefLen)
  {
    uart2_print("NDEF: malformed record\r\n");
    return;
  }

  const uint8_t *typeField = &rec[pos];
  pos += typeLen;

  pos += idLen;

  const uint8_t *payload = &rec[pos];

  snprintf(s, sizeof(s),
           "NDEF rec hdr=0x%02X typeLen=%u payloadLen=%lu\r\n",
           hdr, typeLen, (unsigned long)payloadLen);
  uart2_print(s);

  if ((typeLen == 1) && (typeField[0] == 'T'))
  {
    if (payloadLen < 1)
    {
      uart2_print("TEXT: payload too short\r\n");
      return;
    }

    uint8_t status = payload[0];
    uint8_t langLen = status & 0x3F;
    uint8_t utf16 = (status & 0x80) ? 1U : 0U;

    if (payloadLen < (uint32_t)(1U + langLen))
    {
      uart2_print("TEXT: invalid language length\r\n");
      return;
    }

    snprintf(s, sizeof(s), "TEXT encoding=%s lang=", utf16 ? "UTF16" : "UTF8");
    uart2_print(s);

    for (uint8_t k = 0; k < langLen; k++)
    {
      char c[2] = { (char)payload[1 + k], 0 };
      uart2_print(c);
    }
    uart2_print("\r\n");

    uint32_t textLen = payloadLen - 1U - langLen;
    uart2_print("TEXT: ");

    for (uint32_t k = 0; k < textLen; k++)
    {
      char c[2] = { (char)payload[1U + langLen + k], 0 };
      uart2_print(c);
    }
    uart2_print("\r\n");
  }
  else
  {
    uart2_print("NDEF: first record is not a Text record\r\n");
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
  char b[48];
  char buf[64];

  err = rfalInitialize();
  snprintf(b, sizeof(b), "rfalInitialize err=%d\r\n", (int)err);
  uart2_print(b);
  if (err != ERR_NONE) Error_Handler();

  err = rfalNfcInitialize();
  snprintf(b, sizeof(b), "rfalNfcInitialize err=%d\r\n", (int)err);
  uart2_print(b);
  if (err != ERR_NONE) Error_Handler();

  /* Discover param: NFC-V */
  static rfalNfcDiscoverParam discParam;
  memset(&discParam, 0, sizeof(discParam));
  discParam.compMode      = RFAL_COMPLIANCE_MODE_NFC;
  discParam.devLimit      = 1;
  discParam.techs2Find    = RFAL_NFC_POLL_TECH_A | RFAL_NFC_POLL_TECH_V;
  discParam.totalDuration = 3000;

  err = rfalNfcDeactivate(false);
  err = rfalNfcDiscover(&discParam);
  snprintf(buf, sizeof(buf), "rfalNfcDiscover err=%d\r\n", (int)err);
  uart2_print(buf);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    rfalNfcWorker();

    rfalNfcDevice *devList = NULL;
    uint8_t devCnt = 0;
    rfalNfcGetDevicesFound(&devList, &devCnt);

    static uint32_t last_status_ms = 0;
    static uint32_t last_read_ms   = 0;
    static uint32_t last_irq       = 0;

    static uint8_t  tag_reported   = 0;
    static uint8_t  have_last_data = 0;
    static uint8_t  last_data[APP_T2T_MAX_NDEF];

    rfalNfcState state = rfalNfcGetState();

    /* Heartbeat nur 1x pro Sekunde */
    if ((HAL_GetTick() - last_status_ms) >= 1000U)
    {
      last_status_ms = HAL_GetTick();

      GPIO_PinState p0 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
      GPIO_PinState p1 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);

      char s[140];
      snprintf(s, sizeof(s),
        "cb=%lu last=0x%04X irq/s=%lu PA0=%d PA1=%d devCnt=%u state=%d\r\n",
        (unsigned long)exti_cb_cnt,
        (unsigned)last_exti_pin,
        (unsigned long)(st25r_irq_cnt - last_irq),
        (int)p0, (int)p1,
        (unsigned)devCnt,
        (int)state);
      last_irq = st25r_irq_cnt;
      uart2_print(s);
    }

    /* Tag aktiv */
    if ((devCnt > 0U) && (state == RFAL_NFC_STATE_ACTIVATED))
    {
      rfalNfcDevice *dev = &devList[0];

      if (!tag_reported)
      {
        char s[160];

        snprintf(s, sizeof(s),
          "TAG FOUND: type=%u rfIf=%u uidLen=%u\r\n",
          (unsigned)dev->type,
          (unsigned)dev->rfInterface,
          (unsigned)dev->nfcidLen);
        uart2_print(s);

        uart2_print("UID: ");
        uart_hexln(dev->nfcid, dev->nfcidLen);

        if (dev->type == RFAL_NFC_LISTEN_TYPE_NFCA)
        {
          snprintf(s, sizeof(s),
            "NFCA subtype=%u SAK=0x%02X\r\n",
            (unsigned)dev->dev.nfca.type,
            (unsigned)dev->dev.nfca.selRes.sak);
          uart2_print(s);
        }

        tag_reported   = 1;
        have_last_data = 0;
        last_read_ms   = HAL_GetTick();
      }

      /* Alle 10 ms lesen */
      if ((HAL_GetTick() - last_read_ms) >= 10U)
      {
        last_read_ms = HAL_GetTick();

        if ((dev->type == RFAL_NFC_LISTEN_TYPE_NFCA) &&
            (dev->dev.nfca.type == RFAL_NFCA_T2T))
        {
            uint8_t rx[APP_T2T_MAX_NDEF];
            uint16_t rcvLen = 0;
            ReturnCode rc;

            rc = t2t_read_bytes(4, rx, sizeof(rx), &rcvLen);

            if (rc == ERR_NONE)
            {
              if ((!have_last_data) ||
                  (rcvLen != APP_T2T_MAX_NDEF) ||
                  (memcmp(last_data, rx, APP_T2T_MAX_NDEF) != 0))
              {
                char s[64];
                snprintf(s, sizeof(s), "DATA CHANGED len=%u\r\n", (unsigned)rcvLen);
                uart2_print(s);

                uart2_print("RAW: ");
                uart_hexln(rx, rcvLen);

                t2t_print_ndef_text(rx, rcvLen);

                memcpy(last_data, rx, APP_T2T_MAX_NDEF);
                have_last_data = 1;
              }
            }
          else
          {
            char s[80];
            snprintf(s, sizeof(s), "READ ERROR rc=%d -> rediscover\r\n", (int)rc);
            uart2_print(s);

            tag_reported   = 0;
            have_last_data = 0;

            rfalNfcDeactivate(false);
            rfalNfcDiscover(&discParam);
          }
        }
      }
    }
    else
    {
      /* Tag weg oder noch nicht aktiviert */
      if (tag_reported)
      {
        uart2_print("TAG LOST -> rediscover\r\n");
        tag_reported   = 0;
        have_last_data = 0;

        rfalNfcDeactivate(false);
        rfalNfcDiscover(&discParam);
      }
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
