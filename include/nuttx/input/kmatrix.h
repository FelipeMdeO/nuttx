/****************************************************************************
 * include/nuttx/input/kmatrix.h
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

#ifndef __INCLUDE_NUTTX_INPUT_KMATRIX_H
#define __INCLUDE_NUTTX_INPUT_KMATRIX_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/input/keyboard.h>
#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef uint32_t kmatrix_pin_t;

/* Keyboard matrix configuration structure passed to kmatrix_register() */

struct kmatrix_config_s
{
  uint8_t nrows;                          /* Number of rows */
  uint8_t ncols;                          /* Number of columns */
  FAR const kmatrix_pin_t *rows;          /* Array of row GPIO pins */
  FAR const kmatrix_pin_t *cols;          /* Array of column GPIO pins */
  FAR const uint32_t *keymap;             /* Keymap: keycode[row * cols + col] */
  uint16_t poll_interval_ms;              /* Polling interval in milliseconds */

  /* GPIO callback functions specific to the SoC/board */

  void (*config_row)(kmatrix_pin_t pin);
  void (*config_col)(kmatrix_pin_t pin);
  void (*row_set)(kmatrix_pin_t pin, bool active);
  bool (*col_get)(kmatrix_pin_t pin);
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 * Name: kmatrix_register
 *
 * Description:
 *   Configure and register a keyboard matrix device.  This will create the
 *   /dev/kbdN device node and enable keyboard scanning.
 *
 * Input Parameters:
 *   config - The keyboard matrix configuration.  This structure is not copied;
 *            it must persist for the lifetime of the driver.
 *   devno  - The keyboard device number for the /dev/kbdN path.
 *
 * Returned Value:
 *   Zero is returned on success.  Otherwise, a negated errno value is
 *   returned to indicate the nature of the failure.
 *
 ****************************************************************************/

int kmatrix_register(FAR const struct kmatrix_config_s *config, int devno);

#ifdef __cplusplus
}
#endif

#endif /* __INCLUDE_NUTTX_INPUT_KMATRIX_H */
