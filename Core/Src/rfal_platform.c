

/*
 * rfal_platform.c
 *
 *  Created on: 26.02.2026
 *      Author: samsc
 */

#include "rfal_platform.h"
#include "spi.h"
#include "gpio.h"

volatile uint32_t g_platform_saved_primask = 0U;
volatile uint32_t g_platform_irq_nesting   = 0U;

static void (*s_st25rIrqCb)(void) = NULL;

platformTimer platformTimerCreate(uint16_t time_ms)
{
  return HAL_GetTick() + (uint32_t)time_ms;
}

void platformTimerDestroy(platformTimer t)
{
  (void)t;
}

bool platformTimerIsExpired(platformTimer t)
{
  return ((int32_t)(HAL_GetTick() - t) >= 0);
}

/* CS (NSS) */
void platformSpiSelect(void)
{
  platformGpioClear(ST25R_CS_PORT, ST25R_CS_PIN);
}

void platformSpiDeselect(void)
{
  platformGpioSet(ST25R_CS_PORT, ST25R_CS_PIN);
}

void platformSpiTxRx(const uint8_t* tx, uint8_t* rx, uint16_t len)
{
  if ((tx != NULL) && (rx != NULL)) {
    (void)HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)tx, rx, len, HAL_MAX_DELAY);
  } else if ((tx != NULL) && (rx == NULL)) {
    (void)HAL_SPI_Transmit(&hspi1, (uint8_t*)tx, len, HAL_MAX_DELAY);
  } else if ((tx == NULL) && (rx != NULL)) {
    (void)HAL_SPI_Receive(&hspi1, rx, len, HAL_MAX_DELAY);
  }
}

void platformResetST25R(void)
{
  /* CS inaktiv lassen */
  HAL_GPIO_WritePin(ST25R_CS_PORT, ST25R_CS_PIN, GPIO_PIN_SET);

  /* Reset ist active-high:
     kurz HIGH = Reset aktiv
     danach LOW = normaler Betrieb */
  HAL_GPIO_WritePin(ST25R_RST_PORT, ST25R_RST_PIN, GPIO_PIN_SET);
  HAL_Delay(5);
  HAL_GPIO_WritePin(ST25R_RST_PORT, ST25R_RST_PIN, GPIO_PIN_RESET);
  HAL_Delay(5);
}






/* IRQ */
void platformIrqST25RPinInitialize(void)
{
  /* EXTI ist normalerweise schon via MX_GPIO_Init() konfiguriert */
}

void platformIrqST25RSetCallback(void (*cb)(void))
{
  s_st25rIrqCb = cb;
}

void platformIrqST25RCallCallback(void)
{
  if (s_st25rIrqCb) { s_st25rIrqCb(); }
}


/* Stubs, damit BSP/RFAL nicht meckert */
void platformLedsInitialize(void) {}
void platformErrorHandle(void) { Error_Handler(); }

uint32_t platformTimerGetRemaining(platformTimer t)
{
  uint32_t now = HAL_GetTick();
  if ((int32_t)(t - now) <= 0) return 0;
  return (t - now);
}

void platformDelay(uint32_t ms)
{
  HAL_Delay(ms);
}


