/****************************************************************************
 * boards/xtensa/esp32s3/esp32s3-xiao/src/esp32s3_bringup.c
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
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <nuttx/debug.h>

#include <errno.h>
#include <nuttx/fs/fs.h>
#include <nuttx/himem/himem.h>
#include <nuttx/power/pm.h>
#include <arch/board/board.h>

#include "espressif/esp_gpio.h"
#include "esp32s3_start.h"

#ifdef CONFIG_ESPRESSIF_HR_TIMER
#  include "espressif/esp_hr_timer.h"
#endif

#ifdef CONFIG_ESP32S3_I2C
#  include "esp32s3_i2c.h"
#endif

#if defined(CONFIG_ESP32S3_SDMMC) || defined(CONFIG_MMCSD_SPI)
#  include "esp32s3_board_sdmmc.h"
#endif

#include "esp32s3-xiao.h"

#ifdef CONFIG_USERLED
#  include <nuttx/leds/userled.h>
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: esp32s3_bringup
 *
 * Description:
 *   Perform architecture-specific initialization
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y :
 *     Called from board_late_initialize().
 *
 ****************************************************************************/

int esp32s3_bringup(void)
{
  int ret;

#ifdef CONFIG_PM
  /* Cap the greedy governor at PM_STANDBY (light sleep) so it never
   * free-falls into PM_SLEEP (real deep sleep, which wipes RAM and loses
   * all running state). Only wake sources armed via
   * CONFIG_PM_EXT0/EXT1_WAKEUP or CONFIG_PM_ULP_WAKEUP apply to PM_SLEEP;
   * CONFIG_PM_GPIO_WAKEUP (what this board uses to let the LSM6DS3TRC's
   * FIFO watermark interrupt wake the MCU) only covers PM_STANDBY.
   *
   * NOTE: pm_stay(domain, state) locks a *floor*, not a ceiling --
   * greedy_governor_checkstate() (drivers/power/pm/greedy_governor.c)
   * walks states from PM_NORMAL upward and stops at the first one whose
   * wakelock queue is non-empty. Staying at PM_SLEEP itself (the deepest
   * state) is a no-op: the walk still passes through PM_STANDBY on its
   * way there. Staying at PM_STANDBY is what actually caps it, since the
   * walk stops there instead of continuing on to PM_SLEEP.
   */

  pm_stay(PM_IDLE_DOMAIN, PM_STANDBY);
#endif

#ifdef CONFIG_ESPRESSIF_HR_TIMER
  ret = esp_hr_timer_init();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: esp_hr_timer_init() failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_FS_PROCFS
  /* Mount the procfs file system */

  ret = nx_mount(NULL, "/proc", "procfs", 0, NULL);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: Failed to mount procfs at /proc: %d\n", ret);
    }
#endif

#ifdef CONFIG_FS_TMPFS
  /* Mount the tmpfs file system */

  ret = nx_mount(NULL, CONFIG_LIBC_TMPDIR, "tmpfs", 0, NULL);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: Failed to mount tmpfs at %s: %d\n",
             CONFIG_LIBC_TMPDIR, ret);
    }
#endif

#ifdef CONFIG_DEV_GPIO
  ret = esp32s3_gpio_init();
  if (ret < 0)
    {
      syslog(LOG_ERR, "Failed to initialize GPIO Driver: %d\n", ret);
    }
#endif

#ifdef CONFIG_USERLED
  /* Register the LED driver */

  ret = userled_lower_initialize("/dev/userleds");
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: userled_lower_initialize() failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_MMCSD_SPI
  ret = board_sdmmc_spi_initialize();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: Failed to initialize SDMMC: %d\n", ret);
    }
#endif

#ifdef CONFIG_I2C_DRIVER
  /* Configure I2C peripheral interfaces */

  ret = board_i2c_init();
  if (ret < 0)
    {
      syslog(LOG_ERR, "Failed to initialize I2C driver: %d\n", ret);
    }
#endif

#ifdef CONFIG_SENSORS_LSM6DS3TRC
  /* Try to register the LSM6DS3TR-C device on I2C0 (D4/D5 = SDA/SCL) */

  ret = board_lsm6ds3trc_initialize(0, ESP32S3_I2C0);
  if (ret < 0)
    {
      syslog(LOG_ERR,
             "Failed to initialize LSM6DS3TR-C driver for I2C0: %d\n",
             ret);
    }
#endif

  /* If we got here then perhaps not all initialization was successful, but
   * at least enough succeeded to bring-up NSH with perhaps reduced
   * capabilities.
   */

  UNUSED(ret);
  return OK;
}
