/****************************************************************************
 * arch/arm/src/ht32f491x3/hardware/ht32f491x3_crm.h
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

#ifndef __ARCH_ARM_SRC_HT32F491X3_HARDWARE_HT32F491X3_CRM_H
#define __ARCH_ARM_SRC_HT32F491X3_HARDWARE_HT32F491X3_CRM_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "chip.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Register Offsets *********************************************************/

#define HT32_CRM_CTRL_OFFSET            0x000
#define HT32_CRM_PLLCFG_OFFSET          0x004
#define HT32_CRM_CFG_OFFSET             0x008
#define HT32_CRM_CLKINT_OFFSET          0x00c
#define HT32_CRM_AHBRST1_OFFSET         0x010
#define HT32_CRM_AHBRST2_OFFSET         0x014
#define HT32_CRM_AHBRST3_OFFSET         0x018
#define HT32_CRM_APB1RST_OFFSET         0x020
#define HT32_CRM_APB2RST_OFFSET         0x024
#define HT32_CRM_AHBEN1_OFFSET          0x030
#define HT32_CRM_AHBEN2_OFFSET          0x034
#define HT32_CRM_AHBEN3_OFFSET          0x038
#define HT32_CRM_APB1EN_OFFSET          0x040
#define HT32_CRM_APB2EN_OFFSET          0x044

/* Register Addresses *******************************************************/

#define HT32_CRM_CTRL                   (HT32_CRM_BASE + HT32_CRM_CTRL_OFFSET)
#define HT32_CRM_PLLCFG                 (HT32_CRM_BASE + HT32_CRM_PLLCFG_OFFSET)
#define HT32_CRM_CFG                    (HT32_CRM_BASE + HT32_CRM_CFG_OFFSET)
#define HT32_CRM_CLKINT                 (HT32_CRM_BASE + HT32_CRM_CLKINT_OFFSET)
#define HT32_CRM_AHBRST1                (HT32_CRM_BASE + HT32_CRM_AHBRST1_OFFSET)
#define HT32_CRM_AHBRST2                (HT32_CRM_BASE + HT32_CRM_AHBRST2_OFFSET)
#define HT32_CRM_AHBRST3                (HT32_CRM_BASE + HT32_CRM_AHBRST3_OFFSET)
#define HT32_CRM_APB1RST                (HT32_CRM_BASE + HT32_CRM_APB1RST_OFFSET)
#define HT32_CRM_APB2RST                (HT32_CRM_BASE + HT32_CRM_APB2RST_OFFSET)
#define HT32_CRM_AHBEN1                 (HT32_CRM_BASE + HT32_CRM_AHBEN1_OFFSET)
#define HT32_CRM_AHBEN2                 (HT32_CRM_BASE + HT32_CRM_AHBEN2_OFFSET)
#define HT32_CRM_AHBEN3                 (HT32_CRM_BASE + HT32_CRM_AHBEN3_OFFSET)
#define HT32_CRM_APB1EN                 (HT32_CRM_BASE + HT32_CRM_APB1EN_OFFSET)
#define HT32_CRM_APB2EN                 (HT32_CRM_BASE + HT32_CRM_APB2EN_OFFSET)

/* Clock control register ***************************************************/

#define HT32_CRM_CTRL_HICKEN            (1 << 0)
#define HT32_CRM_CTRL_HICKSTBL          (1 << 1)
#define HT32_CRM_CTRL_HEXTEN            (1 << 16)
#define HT32_CRM_CTRL_HEXTSTBL          (1 << 17)
#define HT32_CRM_CTRL_HEXTBYPS          (1 << 18)
#define HT32_CRM_CTRL_PLLEN             (1 << 24)
#define HT32_CRM_CTRL_PLLSTBL           (1 << 25)

/* Clock configuration register *********************************************/

#define HT32_CRM_CFG_SCLKSEL_SHIFT      (0)
#define HT32_CRM_CFG_SCLKSEL_MASK       (3 << HT32_CRM_CFG_SCLKSEL_SHIFT)
#  define HT32_CRM_CFG_SEL_HICK         (0 << HT32_CRM_CFG_SCLKSEL_SHIFT)
#  define HT32_CRM_CFG_SEL_HEXT         (1 << HT32_CRM_CFG_SCLKSEL_SHIFT)
#  define HT32_CRM_CFG_SEL_PLL          (2 << HT32_CRM_CFG_SCLKSEL_SHIFT)

#define HT32_CRM_CFG_SCLKSTS_SHIFT      (2)
#define HT32_CRM_CFG_SCLKSTS_MASK       (3 << HT32_CRM_CFG_SCLKSTS_SHIFT)
#  define HT32_CRM_CFG_STS_HICK         (0 << HT32_CRM_CFG_SCLKSTS_SHIFT)
#  define HT32_CRM_CFG_STS_HEXT         (1 << HT32_CRM_CFG_SCLKSTS_SHIFT)
#  define HT32_CRM_CFG_STS_PLL          (2 << HT32_CRM_CFG_SCLKSTS_SHIFT)

#define HT32_CRM_CFG_AHBDIV_SHIFT       (4)
#define HT32_CRM_CFG_AHBDIV_MASK        (0x0f << HT32_CRM_CFG_AHBDIV_SHIFT)
#  define HT32_CRM_CFG_AHBDIV_NONE      (0 << HT32_CRM_CFG_AHBDIV_SHIFT)
#  define HT32_CRM_CFG_AHBDIV_2         (8 << HT32_CRM_CFG_AHBDIV_SHIFT)
#  define HT32_CRM_CFG_AHBDIV_4         (9 << HT32_CRM_CFG_AHBDIV_SHIFT)
#  define HT32_CRM_CFG_AHBDIV_8         (10 << HT32_CRM_CFG_AHBDIV_SHIFT)
#  define HT32_CRM_CFG_AHBDIV_16        (11 << HT32_CRM_CFG_AHBDIV_SHIFT)
#  define HT32_CRM_CFG_AHBDIV_64        (12 << HT32_CRM_CFG_AHBDIV_SHIFT)
#  define HT32_CRM_CFG_AHBDIV_128       (13 << HT32_CRM_CFG_AHBDIV_SHIFT)
#  define HT32_CRM_CFG_AHBDIV_256       (14 << HT32_CRM_CFG_AHBDIV_SHIFT)
#  define HT32_CRM_CFG_AHBDIV_512       (15 << HT32_CRM_CFG_AHBDIV_SHIFT)

#define HT32_CRM_CFG_APB1DIV_SHIFT      (10)
#define HT32_CRM_CFG_APB1DIV_MASK       (7 << HT32_CRM_CFG_APB1DIV_SHIFT)
#  define HT32_CRM_CFG_APB1DIV_NONE     (3 << HT32_CRM_CFG_APB1DIV_SHIFT)
#  define HT32_CRM_CFG_APB1DIV_2        (4 << HT32_CRM_CFG_APB1DIV_SHIFT)
#  define HT32_CRM_CFG_APB1DIV_4        (5 << HT32_CRM_CFG_APB1DIV_SHIFT)
#  define HT32_CRM_CFG_APB1DIV_8        (6 << HT32_CRM_CFG_APB1DIV_SHIFT)
#  define HT32_CRM_CFG_APB1DIV_16       (7 << HT32_CRM_CFG_APB1DIV_SHIFT)

#define HT32_CRM_CFG_APB2DIV_SHIFT      (13)
#define HT32_CRM_CFG_APB2DIV_MASK       (7 << HT32_CRM_CFG_APB2DIV_SHIFT)
#  define HT32_CRM_CFG_APB2DIV_NONE     (3 << HT32_CRM_CFG_APB2DIV_SHIFT)
#  define HT32_CRM_CFG_APB2DIV_2        (4 << HT32_CRM_CFG_APB2DIV_SHIFT)
#  define HT32_CRM_CFG_APB2DIV_4        (5 << HT32_CRM_CFG_APB2DIV_SHIFT)
#  define HT32_CRM_CFG_APB2DIV_8        (6 << HT32_CRM_CFG_APB2DIV_SHIFT)
#  define HT32_CRM_CFG_APB2DIV_16       (7 << HT32_CRM_CFG_APB2DIV_SHIFT)

/* Clock enable registers ***************************************************/

#define HT32_CRM_AHBEN1_GPIOAEN         (1 << 0)
#define HT32_CRM_AHBEN1_GPIOBEN         (1 << 1)
#define HT32_CRM_AHBEN1_GPIOCEN         (1 << 2)
#define HT32_CRM_AHBEN1_GPIODEN         (1 << 3)
#define HT32_CRM_AHBEN1_GPIOEEN         (1 << 4)
#define HT32_CRM_AHBEN1_GPIOFEN         (1 << 5)
#define HT32_CRM_AHBEN1_GPIOGEN         (1 << 6)
#define HT32_CRM_AHBEN1_GPIOHEN         (1 << 7)

#define HT32_CRM_APB1EN_USART2EN        (1 << 17)
#define HT32_CRM_APB1EN_USART3EN        (1 << 18)

#define HT32_CRM_APB2EN_USART1EN        (1 << 4)

#endif /* __ARCH_ARM_SRC_HT32F491X3_HARDWARE_HT32F491X3_CRM_H */
