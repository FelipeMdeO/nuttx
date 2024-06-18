/****************************************************************************
 * sched/sched/sched.h
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

#ifndef __SCHED_SCHED_SCHED_H
#define __SCHED_SCHED_SCHED_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdbool.h>
#include <sched.h>

#include <nuttx/arch.h>
#include <nuttx/queue.h>
#include <nuttx/kmalloc.h>
#include <nuttx/spinlock.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define PIDHASH(pid)             ((pid) & (g_npidhash - 1))

/* The state of a task is indicated both by the task_state field of the TCB
 * and by a series of task lists.  All of these tasks lists are declared
 * below. Although it is not always necessary, most of these lists are
 * prioritized so that common list handling logic can be used (only the
 * g_readytorun, the g_pendingtasks, and the g_waitingforsemaphore lists
 * need to be prioritized).
 */

#define list_readytorun()        (&g_readytorun)
#define list_pendingtasks()      (&g_pendingtasks)
#define list_waitingforsignal()  (&g_waitingforsignal)
#define list_waitingforfill()    (&g_waitingforfill)
#define list_stoppedtasks()      (&g_stoppedtasks)
#define list_inactivetasks()     (&g_inactivetasks)
#define list_assignedtasks(cpu)  (&g_assignedtasks[cpu])

/* These are macros to access the current CPU and the current task on a CPU.
 * These macros are intended to support a future SMP implementation.
 * NOTE: this_task() for SMP is implemented in sched_thistask.c
 */

#ifdef CONFIG_SMP
#  define current_task(cpu)      ((FAR struct tcb_s *)list_assignedtasks(cpu)->head)
#else
#  define current_task(cpu)      ((FAR struct tcb_s *)list_readytorun()->head)
#  define this_task()            (current_task(this_cpu()))
#endif

#define is_idle_task(t)          ((t)->pid < CONFIG_SMP_NCPUS)

/* This macro returns the running task which may different from this_task()
 * during interrupt level context switches.
 */

#define running_task() \
  (up_interrupt_context() ? g_running_tasks[this_cpu()] : this_task())

/* List attribute flags */

#define TLIST_ATTR_PRIORITIZED   (1 << 0) /* Bit 0: List is prioritized */
#define TLIST_ATTR_INDEXED       (1 << 1) /* Bit 1: List is indexed by CPU */
#define TLIST_ATTR_RUNNABLE      (1 << 2) /* Bit 2: List includes running tasks */
#define TLIST_ATTR_OFFSET        (1 << 3) /* Bit 3: Pointer of task list is offset */

#define __TLIST_ATTR(s)          g_tasklisttable[s].attr
#define TLIST_ISPRIORITIZED(s)   ((__TLIST_ATTR(s) & TLIST_ATTR_PRIORITIZED) != 0)
#define TLIST_ISINDEXED(s)       ((__TLIST_ATTR(s) & TLIST_ATTR_INDEXED) != 0)
#define TLIST_ISRUNNABLE(s)      ((__TLIST_ATTR(s) & TLIST_ATTR_RUNNABLE) != 0)
#define TLIST_ISOFFSET(s)        ((__TLIST_ATTR(s) & TLIST_ATTR_OFFSET) != 0)

#define __TLIST_HEAD(t) \
  (TLIST_ISOFFSET((t)->task_state) ? (FAR dq_queue_t *)((FAR uint8_t *)((t)->waitobj) + \
  (uintptr_t)g_tasklisttable[(t)->task_state].list) : g_tasklisttable[(t)->task_state].list)

#ifdef CONFIG_SMP
#  define TLIST_HEAD(t,c) \
    ((TLIST_ISINDEXED((t)->task_state)) ? (&(__TLIST_HEAD(t))[c]) : __TLIST_HEAD(t))
#  define TLIST_BLOCKED(t)       __TLIST_HEAD(t)
#else
#  define TLIST_HEAD(t)          __TLIST_HEAD(t)
#  define TLIST_BLOCKED(t)       __TLIST_HEAD(t)
#endif

#ifdef CONFIG_SCHED_CRITMONITOR_MAXTIME_PANIC
#  define CRITMONITOR_PANIC(fmt, ...) \
          do \
            { \
              _alert(fmt, ##__VA_ARGS__); \
              PANIC(); \
            } \
          while(0)
#else
#  define CRITMONITOR_PANIC(fmt, ...) _alert(fmt, ##__VA_ARGS__)
#endif

/****************************************************************************
 * Public Type Definitions
 ****************************************************************************/

/* This structure defines an element of the g_tasklisttable[].  This table
 * is used to map a task_state enumeration to the corresponding task list.
 */

struct tasklist_s
{
  DSEG dq_queue_t *list; /* Pointer to the task list */
  uint8_t attr;          /* List attribute flags */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* Declared in nx_start.c ***************************************************/

/* The state of a task is indicated both by the task_state field of the TCB
 * and by a series of task lists.  All of these tasks lists are declared
 * below. Although it is not always necessary, most of these lists are
 * prioritized so that common list handling logic can be used (only the
 * g_readytorun, the g_pendingtasks, and the g_waitingforsemaphore lists
 * need to be prioritized).
 */

/* This is the list of all tasks that are ready to run.  This is a
 * prioritized list with head of the list holding the highest priority
 * (unassigned) task.  In the non-SMP case, the head of this list is the
 * currently active task and the tail of this list, the lowest priority
 * task, is always the IDLE task.
 */

extern dq_queue_t g_readytorun;

#ifdef CONFIG_SMP
/* In order to support SMP, the function of the g_readytorun list changes,
 * The g_readytorun is still used but in the SMP case it will contain only:
 *
 *  - Only tasks/threads that are eligible to run, but not currently running,
 *    and
 *  - Tasks/threads that have not been assigned to a CPU.
 *
 * Otherwise, the TCB will be retained in an assigned task list,
 * g_assignedtasks.  As its name suggests, on 'g_assignedtasks queue for CPU
 * 'n' would contain only tasks/threads that are assigned to CPU 'n'.  Tasks/
 * threads would be assigned a particular CPU by one of two mechanisms:
 *
 *  - (Semi-)permanently through an RTOS interfaces such as
 *    pthread_attr_setaffinity(), or
 *  - Temporarily through scheduling logic when a previously unassigned task
 *    is made to run.
 *
 * Tasks/threads that are assigned to a CPU via an interface like
 * pthread_attr_setaffinity() would never go into the g_readytorun list, but
 * would only go into the g_assignedtasks[n] list for the CPU 'n' to which
 * the thread has been assigned.  Hence, the g_readytorun list would hold
 * only unassigned tasks/threads.
 *
 * Like the g_readytorun list in in non-SMP case, each g_assignedtask[] list
 * is prioritized:  The head of the list is the currently active task on this
 * CPU.  Tasks after the active task are ready-to-run and assigned to this
 * CPU. The tail of this assigned task list, the lowest priority task, is
 * always the CPU's IDLE task.
 */

extern dq_queue_t g_assignedtasks[CONFIG_SMP_NCPUS];
#endif

/* g_running_tasks[] holds a references to the running task for each cpu.
 * It is valid only when up_interrupt_context() returns true.
 */

extern FAR struct tcb_s *g_running_tasks[CONFIG_SMP_NCPUS];

/* This is an array of task control block (TCB) for the IDLE thread of each
 * CPU.  For the non-SMP case, this is a a single TCB; For the SMP case,
 * there is one TCB per CPU.  NOTE: The system boots on CPU0 into the IDLE
 * task.  The IDLE task later starts the other CPUs and spawns the user
 * initialization task.  That user initialization task is responsible for
 * bringing up the rest of the system.
 */

extern struct tcb_s g_idletcb[CONFIG_SMP_NCPUS];

/* This is the list of all tasks that are ready-to-run, but cannot be placed
 * in the g_readytorun list because:  (1) They are higher priority than the
 * currently active task at the head of the g_readytorun list, and (2) the
 * currently active task has disabled pre-emption.
 */

extern dq_queue_t g_pendingtasks;

/* This is the list of all tasks that are blocked waiting for a signal */

extern dq_queue_t g_waitingforsignal;

/* This is the list of all tasks that are blocking waiting for a page fill */

#ifdef CONFIG_LEGACY_PAGING
extern dq_queue_t g_waitingforfill;
#endif

/* This is the list of all tasks that have been stopped
 * via SIGSTOP or SIGTSTP
 */

#ifdef CONFIG_SIG_SIGSTOP_ACTION
extern dq_queue_t g_stoppedtasks;
#endif

/* This the list of all tasks that have been initialized, but not yet
 * activated. NOTE:  This is the only list that is not prioritized.
 */

extern dq_queue_t g_inactivetasks;

/* This is the value of the last process ID assigned to a task */

extern volatile pid_t g_lastpid;

/* The following hash table is used for two things:
 *
 * 1. This hash table greatly speeds the determination of a new unique
 *    process ID for a task, and
 * 2. Is used to quickly map a process ID into a TCB.
 */

extern FAR struct tcb_s **g_pidhash;
extern volatile int g_npidhash;

/* This is a table of task lists.  This table is indexed by the task stat
 * enumeration type (tstate_t) and provides a pointer to the associated
 * static task list (if there is one) as well as a a set of attribute flags
 * indicating properties of the list, for example, if the list is an
 * ordered list or not.
 */

extern struct tasklist_s g_tasklisttable[NUM_TASK_STATES];

#ifndef CONFIG_SCHED_CPULOAD_NONE
/* This is the total number of clock tick counts.  Essentially the
 * 'denominator' for all CPU load calculations.
 */

extern volatile clock_t g_cpuload_total;
#endif

/* Declared in sched_lock.c *************************************************/

/* Pre-emption is disabled via the interface sched_lock(). sched_lock()
 * works by preventing context switches from the currently executing tasks.
 * This prevents other tasks from running (without disabling interrupts) and
 * gives the currently executing task exclusive access to the (single) CPU
 * resources. Thus, sched_lock() and its companion, sched_unlock(), are
 * used to implement some critical sections.
 *
 * In the single CPU case, Pre-emption is disabled using a simple lockcount
 * in the TCB. When the scheduling is locked, the lockcount is incremented;
 * when the scheduler is unlocked, the lockcount is decremented. If the
 * lockcount for the task at the head of the g_readytorun list has a
 * lockcount > 0, then pre-emption is disabled.
 *
 * No special protection is required since only the executing task can
 * modify its lockcount.
 */

#ifdef CONFIG_SMP
/* Used to keep track of which CPU(s) hold the IRQ lock. */

extern volatile cpu_set_t g_cpu_lockset;

/* This is the spinlock that enforces critical sections when interrupts are
 * disabled.
 */

extern volatile spinlock_t g_cpu_irqlock;

/* Used to keep track of which CPU(s) hold the IRQ lock. */

extern volatile cpu_set_t g_cpu_irqset;

/* Used to lock tasklist to prevent from concurrent access */

extern volatile spinlock_t g_cpu_tasklistlock;

#endif /* CONFIG_SMP */

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int nxthread_create(FAR const char *name, uint8_t ttype, int priority,
                    FAR void *stack_addr, int stack_size, main_t entry,
                    FAR char * const argv[], FAR char * const envp[]);

/* Task list manipulation functions */

bool nxsched_add_readytorun(FAR struct tcb_s *rtrtcb);
bool nxsched_remove_readytorun(FAR struct tcb_s *rtrtcb, bool merge);
bool nxsched_add_prioritized(FAR struct tcb_s *tcb, DSEG dq_queue_t *list);
void nxsched_merge_prioritized(FAR dq_queue_t *list1, FAR dq_queue_t *list2,
                               uint8_t task_state);
bool nxsched_merge_pending(void);
void nxsched_add_blocked(FAR struct tcb_s *btcb, tstate_t task_state);
void nxsched_remove_blocked(FAR struct tcb_s *btcb);
int  nxsched_set_priority(FAR struct tcb_s *tcb, int sched_priority);
bool nxsched_reprioritize_rtr(FAR struct tcb_s *tcb, int priority);

/* Priority inheritance support */

#ifdef CONFIG_PRIORITY_INHERITANCE
int  nxsched_reprioritize(FAR struct tcb_s *tcb, int sched_priority);
#else
#  define nxsched_reprioritize(tcb,sched_priority) \
     nxsched_set_priority(tcb,sched_priority)
#endif

/* Support for tickless operation */

#ifdef CONFIG_SCHED_TICKLESS
clock_t nxsched_cancel_timer(void);
void nxsched_resume_timer(void);
void nxsched_reassess_timer(void);
#else
#  define nxsched_cancel_timer() (0)
#  define nxsched_resume_timer()
#  define nxsched_reassess_timer()
#endif

/* Scheduler policy support */

#if CONFIG_RR_INTERVAL > 0
uint32_t nxsched_process_roundrobin(FAR struct tcb_s *tcb, uint32_t ticks,
                                    bool noswitches);
#endif

#ifdef CONFIG_SCHED_SPORADIC
int  nxsched_initialize_sporadic(FAR struct tcb_s *tcb);
int  nxsched_start_sporadic(FAR struct tcb_s *tcb);
int  nxsched_stop_sporadic(FAR struct tcb_s *tcb);
int  nxsched_reset_sporadic(FAR struct tcb_s *tcb);
int  nxsched_resume_sporadic(FAR struct tcb_s *tcb);
int  nxsched_suspend_sporadic(FAR struct tcb_s *tcb);
uint32_t nxsched_process_sporadic(FAR struct tcb_s *tcb, uint32_t ticks,
                                  bool noswitches);
void nxsched_sporadic_lowpriority(FAR struct tcb_s *tcb);
#endif

#ifdef CONFIG_SIG_SIGSTOP_ACTION
void nxsched_suspend(FAR struct tcb_s *tcb);
#endif

#ifdef CONFIG_SMP
noinstrument_function
static inline_function FAR struct tcb_s *this_task(void)
{
  FAR struct tcb_s *tcb;
  irqstate_t flags;

  /* If the CPU supports suppression of interprocessor interrupts, then
   * simple disabling interrupts will provide sufficient protection for
   * the following operations.
   */

  flags = up_irq_save();

  /* Obtain the TCB which is currently running on this CPU */

  tcb = current_task(this_cpu());

  /* Enable local interrupts */

  up_irq_restore(flags);
  return tcb;
}

int  nxsched_select_cpu(cpu_set_t affinity);
int  nxsched_pause_cpu(FAR struct tcb_s *tcb);

#  define nxsched_islocked_global() (g_cpu_lockset != 0)
#  define nxsched_islocked_tcb(tcb) nxsched_islocked_global()

#else
#  define nxsched_select_cpu(a)     (0)
#  define nxsched_pause_cpu(t)      (-38)  /* -ENOSYS */
#  define nxsched_islocked_tcb(tcb) ((tcb)->lockcount > 0)
#endif

/* CPU load measurement support */

#if defined(CONFIG_SCHED_CPULOAD_SYSCLK) || \
    defined (CONFIG_SCHED_CPULOAD_CRITMONITOR)
void nxsched_process_taskload_ticks(FAR struct tcb_s *tcb, clock_t ticks);
void nxsched_process_cpuload_ticks(clock_t ticks);
#define nxsched_process_cpuload() nxsched_process_cpuload_ticks(1)
#endif

/* Critical section monitor */

#ifdef CONFIG_SCHED_CRITMONITOR
void nxsched_critmon_preemption(FAR struct tcb_s *tcb, bool state);
void nxsched_critmon_csection(FAR struct tcb_s *tcb, bool state);
void nxsched_resume_critmon(FAR struct tcb_s *tcb);
void nxsched_suspend_critmon(FAR struct tcb_s *tcb);
#endif

/* TCB operations */

bool nxsched_verify_tcb(FAR struct tcb_s *tcb);

/* Obtain TLS from kernel */

struct tls_info_s; /* Forward declare */
FAR struct tls_info_s *nxsched_get_tls(FAR struct tcb_s *tcb);
FAR char **nxsched_get_stackargs(FAR struct tcb_s *tcb);

#endif /* __SCHED_SCHED_SCHED_H */
