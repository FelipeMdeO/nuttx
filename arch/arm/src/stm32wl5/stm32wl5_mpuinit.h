/****************************************************************************
 * arch/arm/src/stm32wl5/stm32wl5_mpuinit.h
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

#ifndef __ARCH_ARM_SRC_STM32WL5_STM32WL5_MPUINIT_H
#define __ARCH_ARM_SRC_STM32WL5_STM32WL5_MPUINIT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Public Functions Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: stm32wl5_mpuinitialize
 *
 * Description:
 *   Configure the MPU to permit user-space access to only unrestricted MCU
 *   resources.
 *
 ****************************************************************************/

#ifdef CONFIG_BUILD_PROTECTED
void stm32wl5_mpuinitialize(void);
#else
#  define stm32wl5_mpuinitialize()
#endif

/****************************************************************************
 * Name: stm32wl5_mpu_uheap
 *
 * Description:
 *  Map the user heap region.
 *
 ****************************************************************************/

#ifdef CONFIG_BUILD_PROTECTED
void stm32wl5_mpu_uheap(uintptr_t start, size_t size);
#else
#  define stm32wl5_mpu_uheap(start,size)
#endif

#endif /* __ARCH_ARM_SRC_STM32WL5_STM32WL5_MPUINIT_H */
