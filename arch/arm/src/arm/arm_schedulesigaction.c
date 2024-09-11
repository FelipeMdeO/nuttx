/****************************************************************************
 * arch/arm/src/arm/arm_schedulesigaction.c
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

#include <stdint.h>
#include <sched.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>

#include "arm.h"
#include "sched/sched.h"
#include "arm_internal.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_schedule_sigaction
 *
 * Description:
 *   This function is called by the OS when one or more
 *   signal handling actions have been queued for execution.
 *   The architecture specific code must configure things so
 *   that the 'sigdeliver' callback is executed on the thread
 *   specified by 'tcb' as soon as possible.
 *
 *   This function may be called from interrupt handling logic.
 *
 *   This operation should not cause the task to be unblocked
 *   nor should it cause any immediate execution of sigdeliver.
 *   Typically, a few cases need to be considered:
 *
 *   (1) This function may be called from an interrupt handler
 *       During interrupt processing, all xcptcontext structures
 *       should be valid for all tasks.  That structure should
 *       be modified to invoke sigdeliver() either on return
 *       from (this) interrupt or on some subsequent context
 *       switch to the recipient task.
 *   (2) If not in an interrupt handler and the tcb is NOT
 *       the currently executing task, then again just modify
 *       the saved xcptcontext structure for the recipient
 *       task so it will invoke sigdeliver when that task is
 *       later resumed.
 *   (3) If not in an interrupt handler and the tcb IS the
 *       currently executing task -- just call the signal
 *       handler now.
 *
 * Assumptions:
 *   Called from critical section
 *
 ****************************************************************************/

void up_schedule_sigaction(struct tcb_s *tcb, sig_deliver_t sigdeliver)
{
  sinfo("tcb=%p sigdeliver=%p\n", tcb, sigdeliver);

  /* Refuse to handle nested signal actions */

  if (!tcb->sigdeliver)
    {
      tcb->sigdeliver = sigdeliver;

      /* First, handle some special cases when the signal is
       * being delivered to the currently executing task.
       */

      sinfo("rtcb=%p current_regs=%p\n", this_task(),
            this_task()->xcp.regs);

      if (tcb == this_task() && !up_interrupt_context())
        {
          /* In this case just deliver the signal now. */

          sigdeliver(tcb);
          tcb->sigdeliver = NULL;
        }

      /* Otherwise, we are (1) signaling a task is not running
       * from an interrupt handler or (2) we are not in an
       * interrupt handler and the running task is signalling
       * some non-running task.
       */

      else
        {
          /* Save the return lr and cpsr and one scratch register
           * These will be restored by the signal trampoline after
           * the signals have been delivered.
           */

          /* Save the current register context location */

          tcb->xcp.saved_regs      = tcb->xcp.regs;

          /* Duplicate the register context.  These will be
           * restored by the signal trampoline after the signal has been
           * delivered.
           */

          tcb->xcp.regs            = (void *)
                                     ((uint32_t)tcb->xcp.regs -
                                                XCPTCONTEXT_SIZE);
          memcpy(tcb->xcp.regs, tcb->xcp.saved_regs, XCPTCONTEXT_SIZE);

          tcb->xcp.regs[REG_SP]    = (uint32_t)tcb->xcp.regs +
                                               XCPTCONTEXT_SIZE;

          /* Then set up to vector to the trampoline with interrupts
           * disabled
           */

          tcb->xcp.regs[REG_PC]    = (uint32_t)arm_sigdeliver;
          tcb->xcp.regs[REG_CPSR]  = PSR_MODE_SYS | PSR_I_BIT | PSR_F_BIT;
#ifdef CONFIG_ARM_THUMB
          tcb->xcp.regs[REG_CPSR] |= PSR_T_BIT;
#endif
        }
    }
}
