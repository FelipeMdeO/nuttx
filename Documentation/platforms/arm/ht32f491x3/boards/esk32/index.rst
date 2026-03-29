==================
ESK32 (HT32F49163)
==================

.. tags:: arch:arm, chip:ht32f491x3, chip:ht32f49163, vendor:holtek

The ESK32 is a development board based on the Holtek HT32F49163 MCU.
The current NuttX port targets the HT32F49163 device used on the
HT32F49163 development kit with board bring-up.

For additional hardware details, refer to Holtek's
`HT32F491x3 Starter Kit User Guide <https://www.holtek.com/webapi/106680/HT32F491x3_StarterKitUserManualv100.pdf>`_.

.. figure:: ht32f491x3-starter-kit.jpg
   :align: center
   :alt: HT32F491x3 Starter Kit

   HT32F491x3 Starter Kit board photo

Features
========

The current port provides:

* Holtek HT32F49163 MCU from the HT32F491x3 family
* ARM Cortex-M4 core with FPU support
* Boot and clock initialization for the ESK32 8 MHz external crystal
* System clock configured to 150 MHz
* USART1 serial console at 115200 8N1
* TMR3 PWM output exposed through ``/dev/pwm0``
* TMR2 pulse counter input exposed through ``/dev/pcnt0``
* ``/bin`` mounted through ``binfs``
* ``/proc`` mounted through ``procfs``
* User LED registration through ``/dev/userleds``
* Basic internal GPIO helpers used by the console and LED support

The default ``esk32:nsh`` configuration also enables these built-in
applications:

* ``hello``
* ``ostest``
* ``dumpstack``
* ``leds``

The ``esk32:pwm`` configuration keeps the same board bring-up and adds the
``pwm`` example application together with a board-level PWM device at
``/dev/pwm0``.

The ``esk32:pulsecount`` configuration keeps the same board bring-up and adds
the ``cap`` and ``pwm`` example applications together with board-level
devices at ``/dev/pcnt0`` and ``/dev/pwm0`` for pulse counter validation.

Buttons and LEDs
================

Board LEDs
----------

Three user LEDs from the development kit are currently mapped by the board
port. They are active-low and are exposed through the standard NuttX
``USERLED`` interface and the ``/dev/userleds`` device.

===== =========== ==========
LED   Port/Pin    Notes
===== =========== ==========
LED2  PD13        Active-low
LED3  PD14        Active-low
LED4  PD15        Active-low
===== =========== ==========

The generic ``leds`` example from ``nuttx-apps`` can be used to validate the
LED interface.

Board Buttons
-------------

No button is currently exposed by the board port.

Pin Mapping
===========

The current port uses the following MCU pins:

===== ========== ==========
Pin   Signal     Notes
===== ========== ==========
PA1   TMR2_CH2   ``/dev/pcnt0`` pulse counter input in ``esk32:pulsecount``
PA6   TMR3_CH1   Default ``/dev/pwm0`` output in ``esk32:pwm``
PA7   TMR3_CH2   Alternate ``/dev/pwm0`` output when channel 2 is selected
PA9   USART1_TX  Default serial console TX
PA10  USART1_RX  Default serial console RX
PB0   TMR3_CH3   Alternate ``/dev/pwm0`` output when channel 3 is selected
PB1   TMR3_CH4   Alternate ``/dev/pwm0`` output when channel 4 is selected
PD13  LED2       User LED, active-low
PD14  LED3       User LED, active-low
PD15  LED4       User LED, active-low
===== ========== ==========

Flashing
========

The board directory includes a helper script for flashing through Holtek's
Windows OpenOCD package from a WSL-based development environment:

.. code-block:: console

   $ ./boards/arm/ht32f491x3/esk32/tools/flash.sh

The script expects:

* ``nuttx.bin`` already generated in the ``nuttx`` directory
* Holtek xPack OpenOCD installed under
  ``C:\Program Files (x86)\Holtek HT32 Series\HT32-IDE\xPack\xpack-openocd-0.11.0-4``
* an HT32-Link compatible debug connection
* Holtek xPack OpenOCD can be installed together with the HT32 IDE, available
  from Holtek's website: `Holtek Downloads <https://www.holtek.com/page/index>`_

Useful options:

.. code-block:: console

   $ ./boards/arm/ht32f491x3/esk32/tools/flash.sh --dry-run
   $ ./boards/arm/ht32f491x3/esk32/tools/flash.sh --device HT32F49163_100LQFP
   $ ./boards/arm/ht32f491x3/esk32/tools/flash.sh --openocd-root /mnt/c/path/to/openocd

Testing Notes
=============

The following commands are useful for validating the current port:

.. code-block:: console

   nsh> hello
   nsh> ostest
   nsh> dumpstack
   nsh> leds

When ``leds`` is executed, the example opens ``/dev/userleds`` and cycles
through the LED bitmasks supported by the board.

The ``esk32:pwm`` configuration also exposes ``/dev/pwm0`` through TMR3. The
default defconfig selects ``CONFIG_HT32F491X3_TMR3_CHANNEL=1``, so the PWM
signal is routed to ``PA6``. Probe ``PA6`` against board ``GND`` and run:

.. code-block:: console

   nsh> pwm -f 1000 -d 50 -t 5

This command starts a 1 kHz PWM waveform with a 50% duty cycle for 5 seconds.
On the default channel that corresponds to a 1 ms period with a 500 us high
pulse on ``PA6``. A typical console log is:

.. code-block:: console

   nsh> pwm -f 1000 -d 50 -t 5
   pwm_main: starting output with frequency: 1000 duty: 00007fff
   pwm_main: stopping output

The ``esk32:pulsecount`` configuration exposes ``/dev/pcnt0`` through TMR2
using ``PA1`` as the pulse counter input. A convenient board-level loopback
test connects ``PA6`` to ``PA1`` with a jumper, then starts PWM in the
background and samples the pulse counter:

.. code-block:: console

   nsh> pwm -f 1000 -d 50 -t 3600 &
   nsh> cap -p /dev/pcnt0 -n 10 -t 1000

The expected result is a 1 kHz, 50% duty-cycle waveform on ``PA6`` and a
pulse counter report close to 1000 Hz with the edge count increasing by about
1000 counts per second. The ``cap`` example always prints a duty-cycle field,
but for ``/dev/pcnt0`` that value is reported as ``0`` because the driver only
counts pulses.

Peripheral Support
==================

+---------------------+---------+-------------------------------------+
| Peripheral          | Support | Notes                               |
+=====================+=========+=====================================+
| Boot / Clock / IRQ  | Yes     | Board start-up, clock tree and tick |
+---------------------+---------+-------------------------------------+
| UART                | Yes     | USART1 console                      |
+---------------------+---------+-------------------------------------+
| GPIO                | Partial | Internal helpers only               |
+---------------------+---------+-------------------------------------+
| LEDs                | Yes     | ``USERLED`` and ``/dev/userleds``   |
+---------------------+---------+-------------------------------------+
| Buttons             | No      |                                     |
+---------------------+---------+-------------------------------------+
| PWM                 | Yes     | TMR3 exposed as ``/dev/pwm0``       |
+---------------------+---------+-------------------------------------+
| Pulse Counter       | Yes     | TMR2 exposed as ``/dev/pcnt0``      |
+---------------------+---------+-------------------------------------+
| Timers              | Partial | OS tick, TMR3 PWM and TMR2 counter  |
+---------------------+---------+-------------------------------------+
| SPI                 | No      |                                     |
+---------------------+---------+-------------------------------------+
| I2C                 | No      |                                     |
+---------------------+---------+-------------------------------------+
| ADC                 | No      |                                     |
+---------------------+---------+-------------------------------------+
| DAC                 | No      |                                     |
+---------------------+---------+-------------------------------------+
| CAN                 | No      |                                     |
+---------------------+---------+-------------------------------------+
| DMA                 | No      |                                     |
+---------------------+---------+-------------------------------------+
| RTC / ERTC          | No      |                                     |
+---------------------+---------+-------------------------------------+
| I2S                 | No      |                                     |
+---------------------+---------+-------------------------------------+
| Watchdog            | No      |                                     |
+---------------------+---------+-------------------------------------+
| USB Device          | No      |                                     |
+---------------------+---------+-------------------------------------+
| USB Host            | No      |                                     |
+---------------------+---------+-------------------------------------+
| External Interrupts | No      |                                     |
+---------------------+---------+-------------------------------------+
| Power Control       | No      |                                     |
+---------------------+---------+-------------------------------------+

Configurations
==============

nsh
---

This is the currently maintained configuration for the board. It provides a
serial console with the NuttShell and mounts ``/bin`` and ``/proc`` during
board bring-up.

Configure and build it from the ``nuttx`` directory:

.. code-block:: console

   $ ./tools/configure.sh -l esk32:nsh
   $ make -j

After boot, a typical prompt looks like:

.. code-block:: console

   NuttShell (NSH) NuttX-12.x.x
   nsh> ls /
   /:
    bin/
    dev/
    proc/

And the built-in applications can be listed with:

.. code-block:: console

   nsh> ls /bin
   dd
   dumpstack
   hello
   leds
   nsh
   ostest
   sh

pwm
---

This configuration enables the generic PWM framework, registers the TMR3
lower-half driver as ``/dev/pwm0``, and includes the ``pwm`` example
application for board-level validation.

Configure and build it from the ``nuttx`` directory:

.. code-block:: console

   $ ./tools/configure.sh -l esk32:pwm
   $ make olddefconfig
   $ make -j

The generated defconfig selects ``CONFIG_HT32F491X3_TMR3_CHANNEL=1`` by
default, so ``/dev/pwm0`` is routed to ``PA6``. After flashing the image,
the following command can be used from NSH to validate PWM generation:

.. code-block:: console

   nsh> pwm -f 1000 -d 50 -t 5

The expected result is a 1 kHz, 50% duty-cycle waveform on ``PA6`` for 5
seconds, followed by the application stopping the output. A typical console
log is:

.. code-block:: console

   nsh> pwm -f 1000 -d 50 -t 5
   pwm_main: starting output with frequency: 1000 duty: 00007fff
   pwm_main: stopping output

Other TMR3 output pins can be selected at configuration time through
``CONFIG_HT32F491X3_TMR3_CHANNEL``:

* channel 1: ``PA6``
* channel 2: ``PA7``
* channel 3: ``PB0``
* channel 4: ``PB1``

pulsecount
----------

This configuration enables the generic capture framework, registers the TMR2
pulse counter lower-half driver as ``/dev/pcnt0``, keeps the TMR3 PWM
lower-half driver as ``/dev/pwm0``, and includes the ``cap`` and ``pwm``
example applications for board-level validation.

Configure and build it from the ``nuttx`` directory:

.. code-block:: console

   $ ./tools/configure.sh -l esk32:pulsecount
   $ make olddefconfig
   $ make -j

The generated defconfig routes ``/dev/pwm0`` to ``PA6`` and ``/dev/pcnt0`` to
``PA1``. After flashing the image, install a jumper from ``PA6`` to ``PA1``
and run:

.. code-block:: console

   nsh> pwm -f 1000 -d 50 -t 3600 &
   nsh> cap -p /dev/pcnt0 -n 10 -t 1000

The expected result is a 1 kHz PWM waveform generated on ``PA6`` and a pulse
counter report near 1000 Hz with the accumulated edge count increasing by
about 1000 every second. The duty-cycle field printed by ``cap`` remains ``0``
because ``/dev/pcnt0`` reports pulse count and frequency rather than duty.
