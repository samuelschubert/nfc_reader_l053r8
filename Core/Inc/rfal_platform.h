/*
 * rfal_platform.h
 *
 *  Created on: 26.02.2026
 *      Author: samsc
 */

#ifndef ST25R500
#define ST25R500  1
#endif

#pragma once

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* ---- Wake-Up Mode types (RFAL headers reference these) ----
 * We are not using Wake-Up Mode right now, but the RFAL headers still
 * declare APIs/struct members with these types.
 * This minimal definition is enough to compile.
 */
#ifndef RFAL_WAKEUP_TYPES_DEFINED
#define RFAL_WAKEUP_TYPES_DEFINED

typedef struct
{
  uint8_t dummy;   /* placeholder */
} rfalWakeUpConfig;

typedef struct
{
  uint8_t dummy;   /* placeholder */
} rfalWakeUpInfo;

#endif

/* ====== Pin-Aliase (passe NUR diese 3 Zeilen an, wenn CubeMX andere Namen hat) ====== */
#define ST25R_CS_PORT     (GPIOB)
#define ST25R_CS_PIN      (GPIO_PIN_6)

#define ST25R_RST_PORT    (GPIOA)
#define ST25R_RST_PIN     (GPIO_PIN_9)

#define ST25R_INT_PORT    (GPIOA)
#define ST25R_INT_PIN     (GPIO_PIN_0)

/* ====== Critical section / Schutz ====== */
extern uint32_t g_platform_primask;

#define platformProtectST25RComm()            do { g_platform_primask = __get_PRIMASK(); __disable_irq(); } while(0)
#define platformUnprotectST25RComm()          do { if(g_platform_primask == 0U) { __enable_irq(); } } while(0)

/* Diese 4 sind in deinem Log als fehlend aufgefallen -> sonst "implicit declaration" */
#define platformProtectWorker()               platformProtectST25RComm()
#define platformUnprotectWorker()             platformUnprotectST25RComm()
#define platformProtectST25RIrqStatus()       platformProtectST25RComm()
#define platformUnprotectST25RIrqStatus()     platformUnprotectST25RComm()

/* ====== GPIO helpers ====== */
#define platformGpioIsHigh(port, pin)         (HAL_GPIO_ReadPin((port),(pin)) == GPIO_PIN_SET)
#define platformGpioIsLow(port, pin)          (HAL_GPIO_ReadPin((port),(pin)) == GPIO_PIN_RESET)
#define platformGpioSet(port, pin)            HAL_GPIO_WritePin((port),(pin), GPIO_PIN_SET)
#define platformGpioClear(port, pin)          HAL_GPIO_WritePin((port),(pin), GPIO_PIN_RESET)

/* ====== Timer (minimal, tick-basiert) ====== */
typedef uint32_t platformTimer;

platformTimer platformTimerCreate(uint16_t time_ms);
void          platformTimerDestroy(platformTimer t);
bool          platformTimerIsExpired(platformTimer t);

/* ====== SPI hooks ====== */
void platformSpiSelect(void);
void platformSpiDeselect(void);
void platformSpiTxRx(const uint8_t* tx, uint8_t* rx, uint16_t len);

/* ====== IRQ glue ====== */
void platformIrqST25RPinInitialize(void);
void platformIrqST25RSetCallback(void (*cb)(void));
void platformIrqST25RCallCallback(void);

/* ====== Optional (werden teils vom BSP aufgerufen) ====== */
void platformLedsInitialize(void);
void platformErrorHandle(void);
void platformResetST25R(void);

uint32_t platformTimerGetRemaining(platformTimer t);

void platformDelay(uint32_t ms);
