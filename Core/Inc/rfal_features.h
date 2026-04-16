/*
 * rfal_features.h
 *
 *  Created on: 26.02.2026
 *      Author: samsc
 */

#pragma once

#include <stdint.h>

/* RF buffer size required by rfal_nfc.h */
#ifndef RFAL_FEATURE_NFC_RF_BUF_LEN
#define RFAL_FEATURE_NFC_RF_BUF_LEN  512U
#endif

/* --- Enable the polling technologies we want (Reader mode) --- */
#ifndef RFAL_FEATURE_NFCA
#define RFAL_FEATURE_NFCA  1
#endif

#ifndef RFAL_FEATURE_T1T
#define RFAL_FEATURE_T1T   1
#endif

#ifndef RFAL_FEATURE_T2T
#define RFAL_FEATURE_T2T   1
#endif

#ifndef RFAL_FEATURE_T4T
#define RFAL_FEATURE_T4T   1
#endif

#ifndef RFAL_FEATURE_NFCB
#define RFAL_FEATURE_NFCB  1
#endif

#ifndef RFAL_FEATURE_NFCF
#define RFAL_FEATURE_NFCF  1
#endif

#ifndef RFAL_FEATURE_NFCV
#define RFAL_FEATURE_NFCV  1
#endif

/* --- Optional features (keep off for now) --- */
#ifndef RFAL_FEATURE_WAKEUP_MODE
#define RFAL_FEATURE_WAKEUP_MODE  0
#endif
#ifndef RFAL_FEATURE_WUM
#define RFAL_FEATURE_WUM          0
#endif
#ifndef RFAL_FEATURE_WLC
#define RFAL_FEATURE_WLC          0
#endif

/* --- Support mode macros used by rfal_cd.c / rfal_nfc.c --- */
#ifndef RFAL_SUPPORT_MODE_POLL_NFCA
#define RFAL_SUPPORT_MODE_POLL_NFCA  RFAL_FEATURE_NFCA
#endif
#ifndef RFAL_SUPPORT_MODE_POLL_NFCB
#define RFAL_SUPPORT_MODE_POLL_NFCB  RFAL_FEATURE_NFCB
#endif
#ifndef RFAL_SUPPORT_MODE_POLL_NFCF
#define RFAL_SUPPORT_MODE_POLL_NFCF  RFAL_FEATURE_NFCF
#endif
#ifndef RFAL_SUPPORT_MODE_POLL_NFCV
#define RFAL_SUPPORT_MODE_POLL_NFCV  RFAL_FEATURE_NFCV
#endif

#ifndef RFAL_FEATURE_ST25TB
#define RFAL_FEATURE_ST25TB  0
#endif

#ifndef RFAL_FEATURE_NFC_DEP
#define RFAL_FEATURE_NFC_DEP  0
#endif

#ifndef RFAL_SUPPORT_MODE_POLL_ACTIVE_P2P
#define RFAL_SUPPORT_MODE_POLL_ACTIVE_P2P  0
#endif

#ifndef RFAL_SUPPORT_MODE_LISTEN_NFCA
#define RFAL_SUPPORT_MODE_LISTEN_NFCA  0
#endif
#ifndef RFAL_SUPPORT_MODE_LISTEN_NFCB
#define RFAL_SUPPORT_MODE_LISTEN_NFCB  0
#endif
#ifndef RFAL_SUPPORT_MODE_LISTEN_NFCF
#define RFAL_SUPPORT_MODE_LISTEN_NFCF  0
#endif
#ifndef RFAL_SUPPORT_MODE_LISTEN_NFCV
#define RFAL_SUPPORT_MODE_LISTEN_NFCV  0
#endif

#ifndef RFAL_SUPPORT_MODE_LISTEN_ACTIVE_P2P
#define RFAL_SUPPORT_MODE_LISTEN_ACTIVE_P2P  0
#endif
