.. _smf:

=================================
``smf`` State Machine Framework
=================================

Overview
========

The State Machine Framework (SMF) is a lightweight, application-agnostic
framework for implementing finite and hierarchical state machines in NuttX.

SMF provides:
- Deterministic state transition semantics
- Optional hierarchical state machine (HSM) support
- Explicit entry, run, and exit actions
- No dynamic memory allocation
- Full control over the event loop by the application

The framework is suitable for deeply embedded systems where predictability,
low overhead, and explicit control flow are required.

Conceptually, this implementation is a direct port of the SMF originally
introduced in Zephyr RTOS, adapted to NuttX coding standards, build system,
and documentation conventions.

Architecture
============

SMF separates responsibilities clearly:

- **Framework responsibilities**
  - State transitions
  - Entry/exit sequencing
  - Hierarchy resolution (LCA)
  - Termination handling

- **Application responsibilities**
  - Event acquisition
  - Event dispatching
  - State machine scheduling
  - Data model ownership

This separation ensures that SMF remains fully reusable across applications
and execution models.

State Model
===========

Each state is defined by up to three optional callbacks:

- **Entry action**
- **Run action**
- **Exit action**

All actions operate on a user-defined object whose first member is
``struct smf_ctx``.

.. code-block:: c

   struct app_object
   {
     struct smf_ctx ctx;
     /* Application-specific data */
     int counter;
     bool error;
   };

This layout enables zero-cost casting using the ``SMF_CTX()`` macro.

State Creation
==============

States are declared statically using ``SMF_CREATE_STATE``.

Flat State Machine Example
--------------------------
This example turns the following state diagram into code using the SMF, where the initial state is STATE_IDLE.

.. figure:: images/Flat-state-machine-diagram.png
   :alt: Flat State Machine Diagram
   :align: center
   :width: 30%

   Flat state machine example implemented using SMF.

.. code-block:: c

  #include <system/smf.h>

  /* Forward declaration of state table */
  static const struct smf_state demo_states[];

  /* List of demo states */
     enum demo_state
   {
     STATE_IDLE,
     STATE_ACTIVE,
     STATE_ERROR,
   };

  /* User defined object */
  struct s_object {
          /* This must be first */
          struct smf_ctx ctx;

          /* Other state specific data add here */
  } s_obj;

  /* State idle */
  static void idle_entry(void *o)
  {
    /* Do something */
  }

  static enum smf_state_result idle_run(void *o)
  {
    smf_set_state(SMF_CTX(&s_obj), &demo_states[STATE_ACTIVE]);
    return SMF_EVENT_HANDLED;
  }

  static void idle_exit(void *o)
  {
    /* Do something */
  }

  /* State active */
  static void active_entry(void *o)
    {
      /* Do something */
    }

  static enum smf_state_result active_run(void *o)
  {
    smf_set_state(SMF_CTX(&s_obj), &demo_states[STATE_ERROR]);
    return SMF_EVENT_HANDLED;
  }

  static void active_exit(void *o)
  {
    /* Do something */
  }

  /* State error */
  static void error_entry(void *o)
  {
    /* Do something */
  }

  static enum smf_state_result error_run(void *o)
  {
    smf_set_state(SMF_CTX(&s_obj), &demo_states[STATE_IDLE]);
    return SMF_EVENT_HANDLED;
  }

  static void error_exit(void *o)
  {
    /* Do something */
  }

  /* Populate state table */
  static const struct smf_state demo_states[] = {
    [STATE_IDLE]   = SMF_CREATE_STATE(idle_entry, idle_run, idle_exit, NULL, NULL),
    [STATE_ACTIVE] = SMF_CREATE_STATE(active_entry, active_run, active_exit, NULL, NULL),
    [STATE_ERROR]  = SMF_CREATE_STATE(error_entry, error_run, error_exit, NULL, NULL),
  };

  int main(void)
  {
    int32_t ret;

    /* Set initial state */
    smf_set_initial(SMF_CTX(&s_obj), &demo_states[STATE_IDLE]);

    /* Run the state machine */
    while(1) {
      /* State machine terminates if a non-zero value is returned */
      ret = smf_run_state(SMF_CTX(&s_obj));
      if (ret) {
        /* handle return code and terminate state machine */
        break;
      }
      sleep(1);
    }
  }

Hierarchical State Machine (HSM)
--------------------------------

When ``CONFIG_SMF_ANCESTOR_SUPPORT`` is enabled, states may define a parent.

.. code-block:: c

   enum demo_state
   {
     STATE_PARENT,
     STATE_CHILD_A,
     STATE_CHILD_B,
   };

   static const struct smf_state demo_states[] =
   {
     [STATE_PARENT]  = SMF_CREATE_STATE(parent_entry, parent_run, parent_exit, NULL, NULL),
     [STATE_CHILD_A] = SMF_CREATE_STATE(NULL, child_a_run, NULL, &demo_states[STATE_PARENT], NULL),
     [STATE_CHILD_B] = SMF_CREATE_STATE(NULL, child_b_run, NULL, &demo_states[STATE_PARENT], NULL),
   };

Initial Transitions
===================

If ``CONFIG_SMF_INITIAL_TRANSITION`` is enabled, a parent state may define an
initial child state.

.. code-block:: c

   static const struct smf_state demo_states[] =
   {
     [STATE_PARENT]  = SMF_CREATE_STATE(parent_entry, NULL, parent_exit, NULL, &demo_states[STATE_CHILD_A]),
     [STATE_CHILD_A] = SMF_CREATE_STATE(NULL, child_a_run, NULL, &demo_states[STATE_PARENT], NULL),
   };

When entering ``STATE_PARENT``, the framework automatically transitions to
``STATE_CHILD_A``.

.. note::

   Without initial transition support enabled, applications must always
   transition directly to a leaf state.

State Execution Model
=====================

The application controls execution explicitly.

1. Set the initial state using ``smf_set_initial()``
2. Call ``smf_run_state()`` from an event loop
3. Stop execution when a non-zero value is returned

.. code-block:: c

   smf_set_initial(SMF_CTX(&app), &demo_states[STATE_IDLE]);

   while (1)
     {
       int32_t rc = smf_run_state(SMF_CTX(&app));
       if (rc != 0)
         {
           break;
         }

       /* Block, poll, or wait for an event */
     }

State Run Semantics
-------------------

The run action returns:

- ``SMF_EVENT_HANDLED`` – event consumed
- ``SMF_EVENT_PROPAGATE`` – propagate to parent (HSM only)

If ``smf_set_state()`` is called, propagation stops immediately.

State Transitions
=================

Transitions are requested explicitly by calling ``smf_set_state()`` from
entry or run actions.

.. code-block:: c

   static enum smf_state_result active_run(void *obj)
   {
     struct s_object *s = (struct s_object *)obj;

     if (s->error)
       {
         smf_set_state(SMF_CTX(s), &demo_states[STATE_ERROR]);
         return SMF_EVENT_HANDLED;
       }

     return SMF_EVENT_HANDLED;
   }

Calling ``smf_set_state()`` from exit actions is rejected by design.

Termination
===========

To terminate a state machine, call ``smf_set_terminate()``.

.. code-block:: c

   smf_set_terminate(SMF_CTX(&app), -ECANCELED);

The value passed is returned by ``smf_run_state()`` and can be used to signal
the termination reason.

State Introspection
===================

SMF exposes two helper APIs:

- ``smf_get_current_leaf_state()``
- ``smf_get_current_executing_state()``

These functions are primarily intended for diagnostics, logging, and testing.

UML Compliance
==============

SMF follows UML hierarchical state machine semantics:

- Entry/exit actions execute according to the least common ancestor (LCA)
- Self-transitions execute full exit/entry sequences
- Local transitions are supported implicitly

Differences from UML:

1. Transition actions execute in the source state context
2. Only external self-transitions are supported
3. No explicit terminate pseudostate

Visual Model
============

The following diagram illustrates a typical hierarchical SMF configuration.

``STATE_PARENT`` owns two substates, with an initial transition to
``STATE_CHILD_A``.

Notes and Constraints
=====================

- SMF performs no dynamic allocation
- State tables are typically ``static const``
- Thread safety is the responsibility of the application
- One SMF instance represents one execution region
- Orthogonal regions require multiple SMF instances

Configuration Options
=====================

- ``CONFIG_APPS_SMF``
- ``CONFIG_SMF_ANCESTOR_SUPPORT``
- ``CONFIG_SMF_INITIAL_TRANSITION``

Code Location
=============

- ``apps/system/smf`` – Framework implementation
- ``apps/include/system/smf.h`` – Public API
