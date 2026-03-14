/****************************************************************************
 * arch/arm/src/ht32f491x3/hardware/ht32f491x3_gpio.h
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

#ifndef __ARCH_ARM_SRC_HT32F491X3_HARDWARE_HT32F491X3_GPIO_H
#define __ARCH_ARM_SRC_HT32F491X3_HARDWARE_HT32F491X3_GPIO_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* GPIO register offsets ****************************************************/

#define HT32_GPIO_DIRCR_OFFSET         0x0000
#define HT32_GPIO_INER_OFFSET          0x0004
#define HT32_GPIO_PUR_OFFSET           0x0008
#define HT32_GPIO_PDR_OFFSET           0x000c
#define HT32_GPIO_ODR_OFFSET           0x0010
#define HT32_GPIO_DRVR_OFFSET          0x0014
#define HT32_GPIO_LOCKR_OFFSET         0x0018
#define HT32_GPIO_DINR_OFFSET          0x001c
#define HT32_GPIO_DOUTR_OFFSET         0x0020
#define HT32_GPIO_SRR_OFFSET           0x0024
#define HT32_GPIO_RR_OFFSET            0x0028
#define HT32_GPIO_CFGLR_OFFSET         0x002c
#define HT32_GPIO_CFGHR_OFFSET         0x0030

/* GPIO helpers *************************************************************/

#define HT32_GPIO_PIN(n)               (1u << (n))
#define HT32_GPIO_CFG_SHIFT(n)         (((n) & 7u) << 2)
#define HT32_GPIO_CFG_MASK(n)          (0x0fu << HT32_GPIO_CFG_SHIFT(n))
#define HT32_GPIO_CFG_VALUE(n, af)     ((uint32_t)(af) << HT32_GPIO_CFG_SHIFT(n))
#define HT32_GPIO_CFG_OFFSET(n)        (((n) < 8u) ? HT32_GPIO_CFGLR_OFFSET : \
                                        HT32_GPIO_CFGHR_OFFSET)

#endif /* __ARCH_ARM_SRC_HT32F491X3_HARDWARE_HT32F491X3_GPIO_H */
