/*
 * nfc_conf.h
 *
 *  Created on: 26.02.2026
 *      Author: samsc
 */

#ifndef NFC_CONF_H
#define NFC_CONF_H

#include "stm32l0xx_hal.h"
#include "main.h"
#include "spi.h"
#include "gpio.h"

/* Interface */
#define NFC_USE_SPI  1

extern SPI_HandleTypeDef hspi1;
#define NFC_SPI_HANDLE   hspi1
#define NFC_SPI_TIMEOUT  HAL_MAX_DELAY

/* CS / RST / IRQ (falls du keine CubeMX Labels nutzt) */
#ifndef NFC_CS_GPIO_Port
#define NFC_CS_GPIO_Port GPIOB
#define NFC_CS_Pin       GPIO_PIN_6
#endif

#ifndef NFC_RST_GPIO_Port
#define NFC_RST_GPIO_Port GPIOA
#define NFC_RST_Pin       GPIO_PIN_9   /* RESET active-high (kurz high, dann low) */
#endif

#ifndef NFC_IRQ_GPIO_Port
#define NFC_IRQ_GPIO_Port GPIOA
#define NFC_IRQ_Pin       GPIO_PIN_0
#endif
/* ST25R IRQ/INT pin used by driver */
#define ST25R_INT_PORT   NFC_IRQ_GPIO_Port
#define ST25R_INT_PIN    NFC_IRQ_Pin

/* ---- NFC12A1 BSP LED mapping (Dummy pins, nur für Compile) ---- */
#define NFC12A1_ALLLED_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()

#define NFC12A1_LED1_PIN        GPIO_PIN_0
#define NFC12A1_LED1_PIN_PORT   GPIOC

#define NFC12A1_LED2_PIN        GPIO_PIN_1
#define NFC12A1_LED2_PIN_PORT   GPIOC

#define NFC12A1_LED3_PIN        GPIO_PIN_2
#define NFC12A1_LED3_PIN_PORT   GPIOC

#define NFC12A1_LED4_PIN        GPIO_PIN_3
#define NFC12A1_LED4_PIN_PORT   GPIOC

#define NFC12A1_LED5_PIN        GPIO_PIN_4
#define NFC12A1_LED5_PIN_PORT   GPIOC

#define NFC12A1_LED6_PIN        GPIO_PIN_5
#define NFC12A1_LED6_PIN_PORT   GPIOC

/* ---- Aliases to match ST25R driver / user code naming ---- */
#ifndef ST25R_RST_GPIO_Port
#define ST25R_RST_GPIO_Port  NFC_RST_GPIO_Port
#endif
#ifndef ST25R_RST_Pin
#define ST25R_RST_Pin        NFC_RST_Pin
#endif

#ifndef ST25R_IRQ_Pin
#define ST25R_IRQ_Pin        NFC_IRQ_Pin
#endif

#endif /* NFC_CONF_H */


