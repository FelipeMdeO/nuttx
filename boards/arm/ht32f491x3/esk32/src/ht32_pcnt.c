/****************************************************************************
 * boards/arm/ht32f491x3/esk32/src/ht32_pcnt.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with this
 * work for additional information regarding copyright ownership.  The ASF
 * licenses this file to you under the Apache License, Version 2.0 (the
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
#include <errno.h>
#include <debug.h>

#include <arch/board/board.h>
#include <nuttx/timers/capture.h>

#include "chip.h"
#include "arm_internal.h"
#include "ht32f491x3_pcnt.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define HAVE_PCNT 1

#ifndef CONFIG_CAPTURE
#  undef HAVE_PCNT
#endif

#ifndef CONFIG_HT32F491X3_TMR2_PULSECOUNT
#  undef HAVE_PCNT
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int ht32_pcnt_setup(void)
{
#ifdef HAVE_PCNT
  static bool initialized;
  FAR struct cap_lowerhalf_s *pcnt;
  int ret;

  if (!initialized)
    {
      pcnt = ht32f491x3_pcntinitialize(BOARD_PCNT0_TIMER);
      if (pcnt == NULL)
        {
          snerr("ERROR: Failed to get the HT32 pulse counter lower half\n");
          return -ENODEV;
        }

      ret = cap_register("/dev/pcnt0", pcnt);
      if (ret < 0)
        {
          snerr("ERROR: cap_register failed: %d\n", ret);
          return ret;
        }

      initialized = true;
    }

  return OK;
#else
  return -ENODEV;
#endif
}
