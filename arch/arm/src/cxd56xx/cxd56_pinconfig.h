/****************************************************************************
 * arch/arm/src/cxd56xx/cxd56_pinconfig.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_CXD56XX_CXD56_PINCONFIG_H
#define __ARCH_ARM_SRC_CXD56XX_CXD56_PINCONFIG_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <sys/param.h>
#include <stdint.h>
#include <stdbool.h>

#include <arch/chip/pin.h>
#include "hardware/cxd5602_pinconfig.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* 32-bit encoded pinconf value
 *
 * 3322 2222 2222 1111 1111 1100 0000 0000
 * 1098 7654 3210 9876 5432 1098 7654 3210
 * ---- ---- ---- ---- ---- ---- ---- ----
 * PPPP PPP. .... .... .... .... .... .... Pin number
 * .... ...D .... .... .... .... .... .... Drive strength
 * .... .... .... ...U .... ...U .... .... Pull-up/down/off
 * .... .... .... .... .... .... .... ...I Input enable
 * .... .... .... .... .... .... .... .MM. Alternate mode number
 */

/* Pin number Definitions */

#define PINCONF_PIN_SHIFT       (25)
#define PINCONF_PIN_MASK        (0x7Fu << PINCONF_PIN_SHIFT)
#define PINCONF_GET_PIN(p)      (((p) & PINCONF_PIN_MASK) >> PINCONF_PIN_SHIFT)
#define PINCONF_SET_PIN(p)      (((p) << PINCONF_PIN_SHIFT) & PINCONF_PIN_MASK)

/* Drive strength Definitions */

#define PINCONF_DRIVE_SHIFT     (24)
#define PINCONF_DRIVE_MASK      (1u << PINCONF_DRIVE_SHIFT)

#define PINCONF_DRIVE_NORMAL    (1u << PINCONF_DRIVE_SHIFT) /* 2mA */
#define PINCONF_DRIVE_HIGH      (0u << PINCONF_DRIVE_SHIFT) /* 4mA */

#define PINCONF_IS_DRIVE_NORM(p) (((p) & PINCONF_DRIVE_MASK) == PINCONF_DRIVE_NORMAL)
#define PINCONF_IS_DRIVE_HIGH(p) (((p) & PINCONF_DRIVE_MASK) == PINCONF_DRIVE_HIGH)

/* Pull-up/down/off Definitions */

#define PINCONF_PULL_MASK       ((1u << 16) | (1u << 8))

#define PINCONF_FLOAT           ((1u << 16) | (1u << 8))
#define PINCONF_PULLUP          ((1u << 16) | (0u << 8))
#define PINCONF_PULLDOWN        ((0u << 16) | (1u << 8))
#define PINCONF_BUSKEEPER       ((0u << 16) | (0u << 8))

#define PINCONF_IS_FLOAT(p)     (((p) & PINCONF_PULL_MASK) == PINCONF_FLOAT)
#define PINCONF_IS_PULLUP(p)    (((p) & PINCONF_PULL_MASK) == PINCONF_PULLUP)
#define PINCONF_IS_PULLDOWN(p)  (((p) & PINCONF_PULL_MASK) == PINCONF_PULLDOWN)
#define PINCONF_IS_BUSKEEPER(p) (((p) & PINCONF_PULL_MASK) == PINCONF_BUSKEEPER)

/* Input enable Definitions */

#define PINCONF_IN_EN_SHIFT     (0)
#define PINCONF_IN_EN_MASK      (1u << PINCONF_IN_EN_SHIFT)

#define PINCONF_INPUT_ENABLE    (1u << PINCONF_IN_EN_SHIFT)
#define PINCONF_INPUT_DISABLE   (0u << PINCONF_IN_EN_SHIFT)

#define PINCONF_INPUT_ENABLED(p) (((p) & PINCONF_IN_EN_MASK) == PINCONF_INPUT_ENABLE)

/* Alternate mode number Definitions */

#define PINCONF_MODE_SHIFT      (1)
#define PINCONF_MODE_MASK       (3u << PINCONF_MODE_SHIFT)

#define PINCONF_GET_MODE(p)     (((p) & PINCONF_MODE_MASK) >> PINCONF_MODE_SHIFT)
#define PINCONF_SET_MODE(p)     (((p) << PINCONF_MODE_SHIFT) & PINCONF_MODE_MASK)

#define PINCONF_MODE0           (0) /* GPIO */
#define PINCONF_MODE1           (1) /* Function */
#define PINCONF_MODE2           (2) /* Function */
#define PINCONF_MODE3           (3) /* Function */

/* Set pinconf macro Definitions */

#define PINCONF_SET(pin, mode, input, drive, pull) \
  ( \
    (PINCONF_SET_PIN(pin)) | \
    (PINCONF_SET_MODE(mode)) | \
    (input) | (drive) | (pull) \
    )

#define PINCONF_SET_GPIO(pin, input) \
  PINCONF_SET((pin), PINCONF_MODE0, (input), PINCONF_DRIVE_NORMAL, PINCONF_FLOAT)

#define CXD56_PIN_CONFIGS(pin) do { \
  uint32_t p[] = pin; \
  cxd56_pin_configs((p), nitems(p)); \
} while (0)

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct cxd56_pin_status_s
{
  uint32_t mode;     /* alternate pin function mode */
  uint32_t input_en; /* input enable or disable */
  uint32_t drive;    /* strength of drive current */
  uint32_t pull;     /* internal pull-up, pull-down, floating or buskeeper */
};

typedef struct cxd56_pin_status_s cxd56_pin_status_t;

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__
#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Functions Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: cxd56_pin_config
 *
 * Description:
 *   Configure a pin based on bit-encoded description of the pin.
 *
 * Input Value:
 *   32-bit encoded value describing the pin.
 *
 * Returned Value:
 *   OK on success; A negated errno value on failure.
 *
 ****************************************************************************/

int cxd56_pin_config(uint32_t pinconf);

/****************************************************************************
 * Name: cxd56_pin_configs
 *
 * Description:
 *   Configure multiple pins based on bit-encoded description of the pin.
 *
 * Input Value:
 *   Array of 32-bit encoded value describing the pin.
 *   Number of elements in the array.
 *
 * Returned Value:
 *   OK on success; A negated errno value on failure.
 *
 ****************************************************************************/

int cxd56_pin_configs(uint32_t pinconfs[], size_t n);

/****************************************************************************
 * Name: cxd56_pin_status
 *
 * Description:
 *   Get a pin status.
 *
 * Returned Value:
 *   OK on success; A negated errno value on failure.
 *
 ****************************************************************************/

int cxd56_pin_status(uint32_t pin, cxd56_pin_status_t *stat);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_CXD56XX_CXD56_PINCONFIG_H */
