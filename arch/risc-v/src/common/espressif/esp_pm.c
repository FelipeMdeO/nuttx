/****************************************************************************
 * arch/risc-v/src/common/espressif/esp_pm.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/arch.h>
#include <nuttx/irq.h>
#include <nuttx/power/pm.h>
#include <arch/board/board.h>

#ifdef CONFIG_PM

#if defined(CONFIG_ESPRESSIF_SPI) && defined(CONFIG_SPI_SLAVE)
#include "esp_gpio.h"
#endif

#include "esp_sleep.h"
#include "esp_pm.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static _Atomic uint32_t pm_wakelock = 0;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: esp_pm_lockacquire
 *
 * Description:
 *   Take a power management lock
 *
 ****************************************************************************/

void esp_pm_lockacquire(void) {
  ++pm_wakelock;
}

/****************************************************************************
 * Name: esp_pm_lockrelease
 *
 * Description:
 *   Release the lock taken using esp_pm_lockacquire.
 *
 ****************************************************************************/

void esp_pm_lockrelease(void) {
  --pm_wakelock;
}

/****************************************************************************
 * Name: esp_pm_lockstatus
 *
 * Description:
 *   Return power management lock status.
 *
 ****************************************************************************/

uint32_t esp_pm_lockstatus(void) {
  return pm_wakelock;
}

#if defined(CONFIG_ESPRESSIF_SPI) && defined(CONFIG_SPI_SLAVE)
/**
 * @brief Initialize power management
 *
 * This function initializes the power management system.
 *
 * @note This function should be called during system initialization.
 * @warning This function is not thread-safe and should be called only once.
 */

void esp_pm_init(void) {
  gpio_wakeup_enable(CONFIG_ESPRESSIF_SPI2_CSPIN, ONLOW);
  esp_sleep_enable_gpio_wakeup();
}

bool esp_pm_cs_asserted(void)
{
  return esp_gpioread(CONFIG_ESPRESSIF_SPI2_CSPIN);
}

#endif // CONFIG_ESPRESSIF_SPI && CONFIG_SPI_SLAVE
#endif // CONFIG_PM