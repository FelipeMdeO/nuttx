/****************************************************************************
 * arch/risc-v/src/common/espressif/esp_idle.c
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

#include <arch/board/board.h>
#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/spinlock.h>

#ifdef CONFIG_PM
#include "esp_sleep.h"
#include "esp_pm.h"
#include "esp_idle.h"
#endif

#ifdef CONFIG_RTC_DRIVER
#include "esp_hr_timer.h"
#endif

#ifdef CONFIG_SCHED_TICKLESS
#include "esp_tickless.h"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_idle
 *
 * Description:
 *   up_idle() is the logic that will be executed when their is no other
 *   ready-to-run task. This is processor idle time and will continue until
 *   some interrupt occurs to cause a context switch from the idle task.
 *
 *   Processing in this state may be processor-specific. e.g., this is where
 *   power management operations might be performed.
 *
 * Input Parameters:
 *   None.
 *
 * Returned Value:
 *   None.
 *
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_idlepm
 *
 * Description:
 *   Perform IDLE state power management.
 *
 ****************************************************************************/
#define MIN_SLEEP_TIME_US 800
#define LIGHT_SLEEP_EARLY_WAKEUP_US 100

#if defined(CONFIG_PM)

static void up_idlepm(void)
{ 
  uint64_t sleep_us = up_get_idletime();
  if (sleep_us < MIN_SLEEP_TIME_US)
    {
      return;
    }
  esp_sleep_enable_timer_wakeup(sleep_us - LIGHT_SLEEP_EARLY_WAKEUP_US);
  esp_light_sleep_start();
}

#else
#  define up_idlepm() 
#endif


void up_idle(void)
{
#if defined(CONFIG_SUPPRESS_INTERRUPTS) || defined(CONFIG_SUPPRESS_TIMER_INTS)
  /* If the system is idle and there are no timer interrupts, then process
   * "fake" timer interrupts. Hopefully, something will wake up.
   */

  nxsched_process_timer();
#else

  /* This would be an appropriate place to put some MCU-specific logic to
   * sleep in a reduced power mode until an interrupt occurs to save power
   */

  asm("WFI");
  
  /* Perform IDLE mode power management */
  up_idlepm();

#endif
}
