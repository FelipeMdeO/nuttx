/****************************************************************************
 * boards/arm/stm32/common/src/stm32_kmatrix.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/arch.h>

#include <stdio.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/board.h>
#include <arch/board/board.h>
#include <nuttx/input/kmatrix.h>

#include "stm32.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef uint32_t kmatrix_pin_t;

struct stm32_kmatrixconfig_s
{
  /* Configuration structure as seen by the kmatrix driver */

  struct kmatrix_config_s config;

  /* Additional private definitions only known to this driver */

  void *arg;    /* Argument to pass if needed */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void km_stm32_config_row(kmatrix_pin_t pin);
static void km_stm32_config_col(kmatrix_pin_t pin);
static void km_stm32_row_set(kmatrix_pin_t pin, bool active);
static bool km_stm32_col_get(kmatrix_pin_t pin);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Row and column GPIO pin definitions for 4x3 keypad matrix on STM32F4Discovery
 * Rows: PB0-PB3 (outputs)
 * Columns: PC0-PC2 (inputs with pull-up)
 */

static const kmatrix_pin_t g_km_rows[] =
{
  BOARD_KMATRIX_ROW0,
  BOARD_KMATRIX_ROW1,
  BOARD_KMATRIX_ROW2,
  BOARD_KMATRIX_ROW3,
};

static const kmatrix_pin_t g_km_cols[] =
{
  BOARD_KMATRIX_COL0,
  BOARD_KMATRIX_COL1,
  BOARD_KMATRIX_COL2,
};

/* Keymap for 4x3 matrix - Standard phone keypad layout
 * Rows: 0-3, Columns: 0-2
 */

static const uint32_t g_km_keymap[] =
{
  '1', '2', '3',  /* Row 0 */
  '4', '5', '6',  /* Row 1 */
  '7', '8', '9',  /* Row 2 */
  '*', '0', '#',  /* Row 3 */
};

/* A reference to a structure of this type must be passed to the kmatrix
 * driver.  This structure provides information about the configuration
 * of the keypad matrix and provides GPIO callbacks.
 *
 * Memory for this structure is provided by the caller.  It is not copied
 * by the driver and is presumed to persist while the driver is active.
 */

static struct stm32_kmatrixconfig_s g_km_config =
{
  .config =
  {
    .nrows              = 4,
    .ncols              = 3,
    .rows               = g_km_rows,
    .cols               = g_km_cols,
    .keymap             = g_km_keymap,
    .poll_interval_ms   = CONFIG_INPUT_KMATRIX_POLL_MS,
    .config_row         = km_stm32_config_row,
    .config_col         = km_stm32_config_col,
    .row_set            = km_stm32_row_set,
    .col_get            = km_stm32_col_get,
  },
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: km_stm32_config_row
 *
 * Description:
 *   Configure a row GPIO pin as an output
 *
 ****************************************************************************/

static void km_stm32_config_row(kmatrix_pin_t pin)
{
  iinfo("Configuring row pin as output\n");
  stm32_configgpio(pin);
  stm32_gpiowrite(pin, true);  /* Initialize to inactive (high) */
}

/****************************************************************************
 * Name: km_stm32_config_col
 *
 * Description:
 *   Configure a column GPIO pin as an input with pull-up
 *
 ****************************************************************************/

static void km_stm32_config_col(kmatrix_pin_t pin)
{
  iinfo("Configuring column pin as input\n");
  stm32_configgpio(pin);
}

/****************************************************************************
 * Name: km_stm32_row_set
 *
 * Description:
 *   Activate or deactivate a row (logic: active=true sets to 0/low to
 *   activate the row, active=false sets to 1/high to deactivate)
 *
 ****************************************************************************/

static void km_stm32_row_set(kmatrix_pin_t pin, bool active)
{
  /* With diodes, we drive rows low to activate.
   * active=true  -> write 0 (low)
   * active=false -> write 1 (high)
   */

  stm32_gpiowrite(pin, active ? 0 : 1);
}

/****************************************************************************
 * Name: km_stm32_col_get
 *
 * Description:
 *   Read the state of a column GPIO pin
 *
 ****************************************************************************/

static bool km_stm32_col_get(kmatrix_pin_t pin)
{
  /* With pull-up resistors and diodes:
   * Key pressed   -> column goes low (0)
   * Key released  -> column stays high (1)
   * Return inverted logic: true when pressed (low), false when released (high)
   */

  return !stm32_gpioread(pin);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_kmatrix_initialize
 *
 * Description:
 *   This function is called by application-specific setup logic to
 *   configure the keyboard matrix device.
 *
 * Input Parameters:
 *   devpath - The device path, typically "/dev/kbd0"
 *
 * Returned Value:
 *   Zero is returned on success.  Otherwise, a negated errno value is
 *   returned to indicate the nature of the failure.
 *
 ****************************************************************************/

int board_kmatrix_initialize(const char *devpath)
{
  iinfo("Initializing keyboard matrix at %s\n", devpath);

  /* Register the keyboard matrix with the generic driver */

  return kmatrix_register(&g_km_config.config, devpath);
}
