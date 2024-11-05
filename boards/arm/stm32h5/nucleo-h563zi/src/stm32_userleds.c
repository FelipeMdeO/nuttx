/****************************************************************************
 * boards/arm/stm32h5/nucleo-h563zi/src/stm32_userleds.c
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

#include <stdbool.h>
#include <debug.h>

#include <sys/param.h>

#include <nuttx/board.h>
#include <arch/board/board.h>

#include "stm32_gpio.h"
#include "nucleo-h563zi.h"

#ifndef CONFIG_ARCH_LEDS

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* This array maps an LED number to GPIO pin configuration and is indexed by
 * BOARD_LED_<color>
 */

static const uint32_t g_ledcfg[BOARD_NLEDS] =
{
  GPIO_LD1,
  GPIO_LD2,
  GPIO_LD3,
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_userled_initialize
 *
 * Description:
 *   If CONFIG_ARCH_LEDS is defined, then NuttX will control the on-board
 *   LEDs.  If CONFIG_ARCH_LEDS is not defined, then the
 *   board_userled_initialize() is available to initialize the LEDs from user
 *   application logic.
 *
 ****************************************************************************/

uint32_t board_userled_initialize(void)
{
  int i;

  /* Configure LED1-2 GPIOs for output */

  for (i = 0; i < nitems(g_ledcfg); i++)
    {
      stm32_configgpio(g_ledcfg[i]);
    }

  return BOARD_NLEDS;
}

/****************************************************************************
 * Name: board_userled
 *
 * Description:
 *   If CONFIG_ARCH_LEDS is defined, then NuttX will control the on-board
 *   LEDs.  If CONFIG_ARCH_LEDS is not defined, then the board_userled() is
 *   available to control the LEDs from user application logic.
 *
 ****************************************************************************/

void board_userled(int led, bool ledon)
{
  if ((unsigned)led < nitems(g_ledcfg))
    {
      stm32_gpiowrite(g_ledcfg[led], ledon);
    }
}

/****************************************************************************
 * Name: board_userled_all
 *
 * Description:
 *   If CONFIG_ARCH_LEDS is defined, then NuttX will control the on-board
 *   LEDs.  If CONFIG_ARCH_LEDS is not defined, then the board_userled_all()
 *   is available to control the LED from user application logic.
 *
 ****************************************************************************/

void board_userled_all(uint32_t ledset)
{
  int i;

  /* Configure LED1-2 GPIOs for output */

  for (i = 0; i < nitems(g_ledcfg); i++)
    {
      stm32_gpiowrite(g_ledcfg[i], (ledset & (1 << i)));
    }
}

#endif /* !CONFIG_ARCH_LEDS */
