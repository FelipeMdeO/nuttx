/****************************************************************************
 * boards/xtensa/esp32s3/common/src/esp32s3_board_mpu6050.c
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

#include <nuttx/i2c/i2c_master.h>
#include <nuttx/sensors/mpu6050.h>

#include "esp32s3_i2c.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_mpu6050_initialize
 *
 * Description:
 *   Initialize and register the MPU6050 6-axis MotionTracker driver,
 *   exposing it through uORB as /dev/uorb/sensor_accelN and
 *   /dev/uorb/sensor_gyroN.
 *
 * Input Parameters:
 *   devno - The device number, used to build the uORB device paths
 *   busno - The I2C bus number
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int board_mpu6050_initialize(int devno, int busno)
{
  struct i2c_master_s *i2c;

  i2c = esp32s3_i2cbus_initialize(busno);
  if (i2c == NULL)
    {
      return -ENODEV;
    }

  /* AD0 tied low.  Use MPU6050_ADDR_HIGH when AD0 is pulled to 3V3. */

  return mpu6050_register(devno, i2c, MPU6050_ADDR_LOW);
}
