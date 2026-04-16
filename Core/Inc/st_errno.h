/*
 * st_errno.h
 *
 *  Created on: 26.02.2026
 *      Author: samsc
 */

#pragma once
#include <stdint.h>

/* Minimal ST-style error definitions */
#ifndef ERR_NONE
#define ERR_NONE      ((uint16_t)0U)
#endif

#ifndef ERR_PARAM
#define ERR_PARAM     ((uint16_t)1U)
#endif

#ifndef ERR_REQUEST
#define ERR_REQUEST   ((uint16_t)2U)
#endif

#ifndef ERR_IO
#define ERR_IO        ((uint16_t)3U)
#endif
