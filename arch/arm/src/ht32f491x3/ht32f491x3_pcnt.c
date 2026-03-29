/****************************************************************************
 * arch/arm/src/ht32f491x3/ht32f491x3_pcnt.c
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
#include <stdint.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/clock.h>
#include <nuttx/irq.h>
#include <nuttx/spinlock.h>
#include <nuttx/timers/capture.h>

#include <arch/board/board.h>
#include <arch/irq.h>

#include "arm_internal.h"
#include "chip.h"
#include "ht32f491x3_gpio.h"
#include "ht32f491x3_pcnt.h"

#include "hardware/ht32f491x3_crm.h"
#include "hardware/ht32f491x3_tmr.h"

#ifdef CONFIG_HT32F491X3_TMR2_PULSECOUNT

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define HT32_PCNT_COUNTER_MAX         0xffffu
#define HT32_PCNT_COUNTER_BITS        16u

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct ht32f491x3_pcnt_s
{
  const struct cap_ops_s *ops;
  uint8_t                 timid;
  int                     irq;
  bool                    started;
  bool                    irqattached;
  uintptr_t               base;
  uint32_t                gpio_clken;
  uintptr_t               gpio_base;
  uint8_t                 pin;
  uint8_t                 af;
  uint32_t                overflows;
  uint32_t                last_count;
  clock_t                 last_ticks;
  bool                    freq_valid;
  spinlock_t              lock;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int pcnt_start(FAR struct cap_lowerhalf_s *lower);
static int pcnt_stop(FAR struct cap_lowerhalf_s *lower);
static int pcnt_getduty(FAR struct cap_lowerhalf_s *lower,
                        FAR uint8_t *duty);
static int pcnt_getfreq(FAR struct cap_lowerhalf_s *lower,
                        FAR uint32_t *freq);
static int pcnt_getedges(FAR struct cap_lowerhalf_s *lower,
                         FAR uint32_t *edges);
static int pcnt_ioctl(FAR struct cap_lowerhalf_s *lower,
                      int cmd, unsigned long arg);
static int pcnt_interrupt(int irq, FAR void *context, FAR void *arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct cap_ops_s g_pcntops =
{
  .start    = pcnt_start,
  .stop     = pcnt_stop,
  .getduty  = pcnt_getduty,
  .getfreq  = pcnt_getfreq,
  .getedges = pcnt_getedges,
  .ioctl    = pcnt_ioctl,
};

static struct ht32f491x3_pcnt_s g_tmr2pcnt =
{
  .ops        = &g_pcntops,
  .timid      = 2,
  .irq        = HT32_IRQ_TMR2,
  .started    = false,
  .irqattached = false,
  .base       = HT32_TMR2_BASE,
  .gpio_clken = BOARD_PCNT0_GPIO_CLKEN,
  .gpio_base  = BOARD_PCNT0_GPIO_BASE,
  .pin        = BOARD_PCNT0_GPIO_PIN,
  .af         = BOARD_PCNT0_GPIO_AF,
  .lock       = SP_UNLOCKED,
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline uint32_t pcnt_getreg(FAR struct ht32f491x3_pcnt_s *priv,
                                   unsigned int offset)
{
  return getreg32(priv->base + offset);
}

static inline void pcnt_putreg(FAR struct ht32f491x3_pcnt_s *priv,
                               unsigned int offset, uint32_t value)
{
  putreg32(value, priv->base + offset);
}

static void ht32f491x3_pcnt_enableclk(FAR struct ht32f491x3_pcnt_s *priv,
                                      bool enable)
{
  switch (priv->timid)
    {
      case 2:
        modifyreg32(HT32_CRM_APB1EN, HT32_CRM_APB1EN_TMR2EN,
                    enable ? HT32_CRM_APB1EN_TMR2EN : 0);
        break;

      default:
        break;
    }
}

static void ht32f491x3_pcnt_reset(FAR struct ht32f491x3_pcnt_s *priv)
{
  switch (priv->timid)
    {
      case 2:
        modifyreg32(HT32_CRM_APB1RST, 0, HT32_CRM_APB1RST_TMR2RST);
        modifyreg32(HT32_CRM_APB1RST, HT32_CRM_APB1RST_TMR2RST, 0);
        break;

      default:
        break;
    }
}

static void ht32f491x3_pcnt_gpioconfig(FAR struct ht32f491x3_pcnt_s *priv,
                                       bool enable)
{
  modifyreg32(HT32_CRM_AHBEN1, 0, priv->gpio_clken);

  ht32f491x3_gpioconfig(priv->gpio_base, priv->pin,
                        enable ? HT32_GPIO_MODE_ALTFN :
                                 HT32_GPIO_MODE_INPUT,
                        false,
                        enable ? HT32_GPIO_DRIVE_HIGH :
                                 HT32_GPIO_DRIVE_LOW,
                        HT32_GPIO_PULL_NONE,
                        enable ? priv->af : 0);
}

static void ht32f491x3_pcnt_resetfreq(FAR struct ht32f491x3_pcnt_s *priv)
{
  priv->last_count = 0;
  priv->last_ticks = 0;
  priv->freq_valid = false;
}

static uint32_t ht32f491x3_pcnt_count_locked(FAR struct ht32f491x3_pcnt_s *priv)
{
  uint32_t count;
  uint32_t overflows;

  count = pcnt_getreg(priv, HT32_TMR_CVAL_OFFSET) & HT32_PCNT_COUNTER_MAX;
  overflows = priv->overflows;

  /* Account for a pending overflow that has not been folded by the ISR yet. */

  if ((pcnt_getreg(priv, HT32_TMR_ISTS_OFFSET) & HT32_TMR_INT_OVF) != 0)
    {
      overflows++;
      count = pcnt_getreg(priv, HT32_TMR_CVAL_OFFSET) & HT32_PCNT_COUNTER_MAX;
    }

  return (overflows << HT32_PCNT_COUNTER_BITS) | count;
}

static int ht32f491x3_pcnt_attachirq(FAR struct ht32f491x3_pcnt_s *priv)
{
  int ret = OK;

  if (!priv->irqattached)
    {
      ret = irq_attach(priv->irq, pcnt_interrupt, priv);
      if (ret < 0)
        {
          return ret;
        }

      priv->irqattached = true;
    }

  up_enable_irq(priv->irq);
  return OK;
}

static void ht32f491x3_pcnt_detachirq(FAR struct ht32f491x3_pcnt_s *priv)
{
  if (priv->irqattached)
    {
      up_disable_irq(priv->irq);
      irq_detach(priv->irq);
      priv->irqattached = false;
    }
}

static int pcnt_interrupt(int irq, FAR void *context, FAR void *arg)
{
  FAR struct ht32f491x3_pcnt_s *priv = arg;
  irqstate_t flags;

  UNUSED(irq);
  UNUSED(context);

  flags = spin_lock_irqsave(&priv->lock);

  if ((pcnt_getreg(priv, HT32_TMR_ISTS_OFFSET) & HT32_TMR_INT_OVF) != 0)
    {
      priv->overflows++;
      pcnt_putreg(priv, HT32_TMR_ISTS_OFFSET, ~HT32_TMR_INT_OVF);
    }

  spin_unlock_irqrestore(&priv->lock, flags);
  return OK;
}

static int pcnt_start(FAR struct cap_lowerhalf_s *lower)
{
  FAR struct ht32f491x3_pcnt_s *priv = (FAR struct ht32f491x3_pcnt_s *)lower;
  irqstate_t flags;
  uint32_t regval;
  int ret;

  DEBUGASSERT(priv != NULL);

  flags = spin_lock_irqsave(&priv->lock);
  if (priv->started)
    {
      spin_unlock_irqrestore(&priv->lock, flags);
      return OK;
    }

  priv->overflows = 0;
  ht32f491x3_pcnt_resetfreq(priv);
  spin_unlock_irqrestore(&priv->lock, flags);

  ht32f491x3_pcnt_enableclk(priv, true);
  ht32f491x3_pcnt_reset(priv);
  ht32f491x3_pcnt_gpioconfig(priv, true);

  pcnt_putreg(priv, HT32_TMR_CTRL1_OFFSET, HT32_TMR_CTRL1_COUNTUP);
  pcnt_putreg(priv, HT32_TMR_CTRL2_OFFSET, 0);
  pcnt_putreg(priv, HT32_TMR_CCTRL_OFFSET, 0);
  pcnt_putreg(priv, HT32_TMR_CVAL_OFFSET, 0);
  pcnt_putreg(priv, HT32_TMR_DIV_OFFSET, 0);
  pcnt_putreg(priv, HT32_TMR_PR_OFFSET, HT32_PCNT_COUNTER_MAX);
  pcnt_putreg(priv, HT32_TMR_SWEVT_OFFSET, HT32_TMR_SWEVT_OVFSWTR);

  regval = pcnt_getreg(priv, HT32_TMR_CM1_OFFSET);
  regval &= ~(HT32_TMR_CM_CAPTURE_SEL_MASK(1) |
              HT32_TMR_CM_INPUT_DIV_MASK(1) |
              HT32_TMR_CM_INPUT_FILTER_MASK(1));
  regval |= HT32_TMR_CM_CAPTURE_SEL(1, HT32_TMR_CAPTURE_DIRECT);
  pcnt_putreg(priv, HT32_TMR_CM1_OFFSET, regval);

  regval = pcnt_getreg(priv, HT32_TMR_CCTRL_OFFSET);
  regval &= ~(HT32_TMR_CCTRL_EN(2) |
              HT32_TMR_CCTRL_POL(2) |
              HT32_TMR_CCTRL_COMPOL(2));
  regval |= HT32_TMR_CCTRL_EN(2);
  pcnt_putreg(priv, HT32_TMR_CCTRL_OFFSET, regval);

  pcnt_putreg(priv, HT32_TMR_STCTRL_OFFSET,
              HT32_TMR_STCTRL_STIS(HT32_TMR_TRIGGER_C2DF2) |
              HT32_TMR_STCTRL_SMSEL(HT32_TMR_EXTERNAL_CLOCK_MODE_A));

  pcnt_putreg(priv, HT32_TMR_ISTS_OFFSET, ~HT32_TMR_INT_OVF);
  pcnt_putreg(priv, HT32_TMR_IDEN_OFFSET, HT32_TMR_INT_OVF);

  ret = ht32f491x3_pcnt_attachirq(priv);
  if (ret < 0)
    {
      pcnt_putreg(priv, HT32_TMR_IDEN_OFFSET, 0);
      ht32f491x3_pcnt_gpioconfig(priv, false);
      ht32f491x3_pcnt_enableclk(priv, false);
      return ret;
    }

  modifyreg32(priv->base + HT32_TMR_CTRL1_OFFSET, 0,
              HT32_TMR_CTRL1_PRBEN | HT32_TMR_CTRL1_TMREN);

  flags = spin_lock_irqsave(&priv->lock);
  priv->started = true;
  spin_unlock_irqrestore(&priv->lock, flags);
  return OK;
}

static int pcnt_stop(FAR struct cap_lowerhalf_s *lower)
{
  FAR struct ht32f491x3_pcnt_s *priv = (FAR struct ht32f491x3_pcnt_s *)lower;
  irqstate_t flags;
  uint32_t regval;

  DEBUGASSERT(priv != NULL);

  flags = spin_lock_irqsave(&priv->lock);
  if (!priv->started)
    {
      spin_unlock_irqrestore(&priv->lock, flags);
      return OK;
    }

  priv->started = false;
  spin_unlock_irqrestore(&priv->lock, flags);

  regval = pcnt_getreg(priv, HT32_TMR_CTRL1_OFFSET);
  regval &= ~HT32_TMR_CTRL1_TMREN;
  pcnt_putreg(priv, HT32_TMR_CTRL1_OFFSET, regval);
  pcnt_putreg(priv, HT32_TMR_IDEN_OFFSET, 0);
  pcnt_putreg(priv, HT32_TMR_ISTS_OFFSET, ~HT32_TMR_INT_OVF);

  ht32f491x3_pcnt_detachirq(priv);
  ht32f491x3_pcnt_gpioconfig(priv, false);
  ht32f491x3_pcnt_enableclk(priv, false);
  return OK;
}

static int pcnt_getduty(FAR struct cap_lowerhalf_s *lower, FAR uint8_t *duty)
{
  UNUSED(lower);

  DEBUGASSERT(duty != NULL);
  *duty = 0;
  return OK;
}

static int pcnt_getfreq(FAR struct cap_lowerhalf_s *lower, FAR uint32_t *freq)
{
  FAR struct ht32f491x3_pcnt_s *priv = (FAR struct ht32f491x3_pcnt_s *)lower;
  irqstate_t flags;
  uint32_t count;
  clock_t ticks;

  DEBUGASSERT(priv != NULL && freq != NULL);

  flags = spin_lock_irqsave(&priv->lock);
  count = ht32f491x3_pcnt_count_locked(priv);
  ticks = clock_systime_ticks();

  if (!priv->freq_valid || ticks == priv->last_ticks)
    {
      *freq = 0;
      priv->last_count = count;
      priv->last_ticks = ticks;
      priv->freq_valid = true;
    }
  else
    {
      uint32_t delta_count = count - priv->last_count;
      clock_t delta_ticks = ticks - priv->last_ticks;
      uint64_t scaled = (uint64_t)delta_count * (uint64_t)TICK_PER_SEC;

      *freq = (uint32_t)((scaled + (delta_ticks / 2)) / delta_ticks);
      priv->last_count = count;
      priv->last_ticks = ticks;
    }

  spin_unlock_irqrestore(&priv->lock, flags);
  return OK;
}

static int pcnt_getedges(FAR struct cap_lowerhalf_s *lower, FAR uint32_t *edges)
{
  FAR struct ht32f491x3_pcnt_s *priv = (FAR struct ht32f491x3_pcnt_s *)lower;
  irqstate_t flags;

  DEBUGASSERT(priv != NULL && edges != NULL);

  flags = spin_lock_irqsave(&priv->lock);
  *edges = ht32f491x3_pcnt_count_locked(priv);
  spin_unlock_irqrestore(&priv->lock, flags);
  return OK;
}

static int pcnt_ioctl(FAR struct cap_lowerhalf_s *lower,
                      int cmd, unsigned long arg)
{
  FAR struct ht32f491x3_pcnt_s *priv = (FAR struct ht32f491x3_pcnt_s *)lower;
  irqstate_t flags;
  int ret = OK;

  DEBUGASSERT(priv != NULL);

  switch (cmd)
    {
      case CAPIOC_PULSES:
        {
          FAR int *count = (FAR int *)((uintptr_t)arg);

          if (count == NULL)
            {
              ret = -EINVAL;
              break;
            }

          flags = spin_lock_irqsave(&priv->lock);
          *count = (int)ht32f491x3_pcnt_count_locked(priv);
          spin_unlock_irqrestore(&priv->lock, flags);
        }
        break;

      case CAPIOC_CLR_CNT:
        {
          flags = spin_lock_irqsave(&priv->lock);
          priv->overflows = 0;
          pcnt_putreg(priv, HT32_TMR_CVAL_OFFSET, 0);
          pcnt_putreg(priv, HT32_TMR_ISTS_OFFSET, ~HT32_TMR_INT_OVF);
          ht32f491x3_pcnt_resetfreq(priv);
          spin_unlock_irqrestore(&priv->lock, flags);
        }
        break;

      default:
        ret = -ENOTTY;
        break;
    }

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

FAR struct cap_lowerhalf_s *ht32f491x3_pcntinitialize(int timer)
{
  if (timer != 2)
    {
      snerr("ERROR: TMR%d pulse counter is not supported\n", timer);
      return NULL;
    }

  return (FAR struct cap_lowerhalf_s *)&g_tmr2pcnt;
}

#endif /* CONFIG_HT32F491X3_TMR2_PULSECOUNT */
