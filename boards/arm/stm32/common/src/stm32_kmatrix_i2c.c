/****************************************************************************
 * boards/arm/stm32/common/src/stm32_kmatrix_i2c.c
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

#include <stdio.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/input/kmatrix.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

typedef uint32_t kmatrix_pin_t;

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Callbacks will be filled in from the I2C driver */

static void (*km_config_row_cb)(kmatrix_pin_t pin) = NULL;
static void (*km_config_col_cb)(kmatrix_pin_t pin) = NULL;
static void (*km_row_set_cb)(kmatrix_pin_t pin, bool active) = NULL;
static bool (*km_col_get_cb)(kmatrix_pin_t pin) = NULL;

/* Wrapper functions */

static void km_i2c_config_row(kmatrix_pin_t pin)
{
  if (km_config_row_cb != NULL)
    {
      km_config_row_cb(pin);
    }
}

static void km_i2c_config_col(kmatrix_pin_t pin)
{
  if (km_config_col_cb != NULL)
    {
      km_config_col_cb(pin);
    }
}

static void km_i2c_row_set(kmatrix_pin_t pin, bool active)
{
  if (km_row_set_cb != NULL)
    {
      km_row_set_cb(pin, active);
    }
}

static bool km_i2c_col_get(kmatrix_pin_t pin)
{
  if (km_col_get_cb != NULL)
    {
      return km_col_get_cb(pin);
    }
  return false;
}

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Row and column pin definitions for 4x3 keypad matrix via I2C expander
 *
 * For PCF8574/MCP23017 I2C expanders, pins are numbered 0-7 (PCF8574)
 * or 0-15 (MCP23017) in the expander's address space.
 *
 * Example mapping for PCF8574:
 *   Rows (outputs):    Pins 0-3
 *   Columns (inputs):  Pins 4-6 (with pull-ups)
 *
 * Example mapping for MCP23017:
 *   Rows (outputs):    Port A Pins 0-3
 *   Columns (inputs):  Port B Pins 0-2 (with pull-ups)
 */

static const kmatrix_pin_t g_km_rows[] =
{
  0, 1, 2, 3,  /* Row 0-3: Output pins on expander */
};

static const kmatrix_pin_t g_km_cols[] =
{
  4, 5, 6,  /* Col 0-2: Input pins on expander (with pull-up) */
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

/* Keyboard matrix configuration structure */

static struct kmatrix_config_s g_km_i2c_config =
{
  .nrows              = 4,
  .ncols              = 3,
  .rows               = g_km_rows,
  .cols               = g_km_cols,
  .keymap             = g_km_keymap,
  .poll_interval_ms   = CONFIG_INPUT_KMATRIX_POLL_MS,
  .config_row         = km_i2c_config_row,
  .config_col         = km_i2c_config_col,
  .row_set            = km_i2c_row_set,
  .col_get            = km_i2c_col_get,
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * Name: board_kmatrix_i2c_initialize
 *
 * Description:
 *   Initialize keyboard matrix driver using I2C GPIO expander.
 *   This function is called by stm32_bringup.c during initialization.
 *
 * Input Parameters:
 *   devpath - Device path (e.g., "/dev/kbd0")
 *
 * Returned Value:
 *   Zero on success; negated errno on failure.
 */

extern int kmatrix_i2c_register(FAR const struct kmatrix_config_s *config,
                                FAR const char *devpath,
                                int i2c_bus, uint8_t i2c_addr);

int board_kmatrix_i2c_initialize(const char *devpath)
{
  /* NOTE: Callbacks are set directly in g_km_i2c_config structure above.
   * The actual I2C communication is handled by the kmatrix_i2c driver,
   * which stores the I2C device handle in a global variable accessible
   * to the callback functions through the wrapper layer.
   */

  iinfo("Initializing keyboard matrix via I2C expander\n");

  /* Call the generic I2C driver with board-specific configuration */

  return kmatrix_i2c_register(&g_km_i2c_config,
                              devpath,
                              CONFIG_STM32_KMATRIX_I2C_BUS,
                              CONFIG_STM32_KMATRIX_I2C_ADDR);
}
