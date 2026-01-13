/****************************************************************************
 * boards/sim/sim/sim/src/sim_ioexpander.c
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

#include <errno.h>
#include <debug.h>

#include <nuttx/ioexpander/gpio.h>
#include <nuttx/ioexpander/ioe_dummy.h>

#include "sim.h"

#if defined(CONFIG_EXAMPLES_GPIO) && defined(CONFIG_GPIO_LOWER_HALF)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sim_gpio_initialize
 *
 * Description:
 *   Initialize simulated GPIO expander for use with /apps/examples/gpio
 *
 ****************************************************************************/

int sim_gpio_initialize(void)
{
  /* Get an instance of the simulated I/O expander */

  struct ioexpander_dev_s *ioe = ioe_dummy_initialize();
  if (ioe == NULL)
    {
      gpioerr("ERROR: ioe_dummy_initialize failed\n");
      return -ENOMEM;
    }

  int ret;
  int pin;
  int act_count = CONFIG_IOEXPANDER_NPINS / 2;

  /* Register pins: first half outputs (actuators), second half inputs
   * (feedback).
   */

  for (pin = 0; pin < CONFIG_IOEXPANDER_NPINS; pin++)
    {
      int direction = (pin < act_count) ? IOEXPANDER_DIRECTION_OUT
                                        : IOEXPANDER_DIRECTION_IN;
      enum gpio_pintype_e pintype = (pin < act_count) ? GPIO_OUTPUT_PIN
                                                      : GPIO_INPUT_PIN;

      ret = IOEXP_SETDIRECTION(ioe, pin, direction);
      if (ret < 0)
        {
          gpioerr("ERROR: IOEXP_SETDIRECTION pin %d failed: %d\n",
                  pin, ret);
          return ret;
        }

      ret = IOEXP_SETOPTION(ioe, pin, IOEXPANDER_OPTION_INVERT,
                            (void *)IOEXPANDER_VAL_NORMAL);
      if (ret < 0 && ret != -ENOSYS)
        {
          gpioerr("ERROR: IOEXP_SETOPTION pin %d invert failed: %d\n",
                  pin, ret);
          return ret;
        }

      ret = IOEXP_SETOPTION(ioe, pin, IOEXPANDER_OPTION_INTCFG,
                            (void *)IOEXPANDER_VAL_DISABLE);
      if (ret < 0 && ret != -ENOSYS)
        {
          gpioerr("ERROR: IOEXP_SETOPTION pin %d intcfg failed: %d\n",
                  pin, ret);
          return ret;
        }

      ret = gpio_lower_half(ioe, pin, pintype, pin);
      if (ret < 0)
        {
          gpioerr("ERROR: gpio_lower_half pin %d failed: %d\n",
                  pin, ret);
          return ret;
        }
    }

  return 0;
}
#endif /* CONFIG_EXAMPLES_GPIO && CONFIG_GPIO_LOWER_HALF */
