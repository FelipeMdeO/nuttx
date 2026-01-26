/****************************************************************************
 * drivers/input/kmatrix_i2c.c
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

#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/i2c/i2c_master.h>
#include <nuttx/input/kmatrix.h>
#include <nuttx/kmalloc.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_INPUT_KMATRIX_I2C_PCF8574
#  define IOEXP_TYPE_PCF8574
#elif defined(CONFIG_INPUT_KMATRIX_I2C_MCP23017)
#  define IOEXP_TYPE_MCP23017
#else
#  error "CONFIG_INPUT_KMATRIX_I2C_PCF8574 or CONFIG_INPUT_KMATRIX_I2C_MCP23017 must be selected"
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef uint32_t kmatrix_pin_t;

struct kmatrix_i2c_dev_s
{
  FAR struct i2c_master_s *i2c;   /* I2C interface */
  uint8_t addr;                   /* I2C slave address */
  uint8_t reg_cache;              /* Cache of last I/O register value */

  /* Function pointers for I2C read/write operations */

  int (*read_register)(FAR struct kmatrix_i2c_dev_s *dev,
                      uint8_t regaddr, FAR uint8_t *regval);
  int (*write_register)(FAR struct kmatrix_i2c_dev_s *dev,
                       uint8_t regaddr, uint8_t regval);
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void km_i2c_config_row(kmatrix_pin_t pin);
static void km_i2c_config_col(kmatrix_pin_t pin);
static void km_i2c_row_set(kmatrix_pin_t pin, bool active);
static bool km_i2c_col_get(kmatrix_pin_t pin);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Global I2C device instance (simplified - one per board) */

static struct kmatrix_i2c_dev_s g_km_i2c_dev;

/****************************************************************************
 * PCF8574 Operations
 ****************************************************************************/

#ifdef IOEXP_TYPE_PCF8574

static int km_pcf8574_read_register(FAR struct kmatrix_i2c_dev_s *dev,
                                     uint8_t regaddr, FAR uint8_t *regval)
{
  struct i2c_msg_s msgs[1];

  msgs[0].frequency = CONFIG_I2C_MAXBUSFREQ;
  msgs[0].addr      = dev->addr;
  msgs[0].flags     = I2C_M_READ;
  msgs[0].buffer    = regval;
  msgs[0].length    = 1;

  return I2C_TRANSFER(dev->i2c, msgs, 1);
}

static int km_pcf8574_write_register(FAR struct kmatrix_i2c_dev_s *dev,
                                      uint8_t regaddr, uint8_t regval)
{
  struct i2c_msg_s msgs[1];
  uint8_t txbuffer[1];

  txbuffer[0] = regval;

  msgs[0].frequency = CONFIG_I2C_MAXBUSFREQ;
  msgs[0].addr      = dev->addr;
  msgs[0].flags     = 0;
  msgs[0].buffer    = txbuffer;
  msgs[0].length    = 1;

  return I2C_TRANSFER(dev->i2c, msgs, 1);
}

#endif

/****************************************************************************
 * MCP23017 Operations
 ****************************************************************************/

#ifdef IOEXP_TYPE_MCP23017

/* MCP23017 Register Addresses (IOCON.BANK = 0) */

#define MCP23017_IODIRA       0x00
#define MCP23017_IODIRB       0x01
#define MCP23017_GPPUA        0x0c
#define MCP23017_GPPUB        0x0d
#define MCP23017_GPIOA        0x12
#define MCP23017_GPIOB        0x13

static int km_mcp23017_read_register(FAR struct kmatrix_i2c_dev_s *dev,
                                      uint8_t regaddr, FAR uint8_t *regval)
{
  struct i2c_msg_s msgs[2];
  uint8_t txbuffer[1];

  txbuffer[0] = regaddr;

  msgs[0].frequency = CONFIG_I2C_MAXBUSFREQ;
  msgs[0].addr      = dev->addr;
  msgs[0].flags     = 0;
  msgs[0].buffer    = txbuffer;
  msgs[0].length    = 1;

  msgs[1].frequency = CONFIG_I2C_MAXBUSFREQ;
  msgs[1].addr      = dev->addr;
  msgs[1].flags     = I2C_M_READ;
  msgs[1].buffer    = regval;
  msgs[1].length    = 1;

  return I2C_TRANSFER(dev->i2c, msgs, 2);
}

static int km_mcp23017_write_register(FAR struct kmatrix_i2c_dev_s *dev,
                                       uint8_t regaddr, uint8_t regval)
{
  struct i2c_msg_s msgs[1];
  uint8_t txbuffer[2];

  txbuffer[0] = regaddr;
  txbuffer[1] = regval;

  msgs[0].frequency = CONFIG_I2C_MAXBUSFREQ;
  msgs[0].addr      = dev->addr;
  msgs[0].flags     = 0;
  msgs[0].buffer    = txbuffer;
  msgs[0].length    = 2;

  return I2C_TRANSFER(dev->i2c, msgs, 1);
}

#endif

/****************************************************************************
 * I2C Keyboard Matrix Callbacks
 ****************************************************************************/

/**
 * Name: km_i2c_config_row
 *
 * Description:
 *   Configure row pins as outputs. For I2C expanders, this means setting
 *   the IODIR register bits to 0 (output).
 */

static void km_i2c_config_row(kmatrix_pin_t pin)
{
  iinfo("I2C: Configuring pin %d as output (row)\n", pin);

#ifdef IOEXP_TYPE_MCP23017
  /* MCP23017: pin 0-3 -> GPIOA, pin 4-7 -> GPIOB */
  /* Clear IODIR bit to make it an output */
  if (pin < 4)
    {
      uint8_t regval;
      g_km_i2c_dev.read_register(&g_km_i2c_dev, MCP23017_IODIRA, &regval);
      regval &= ~(1 << pin);
      g_km_i2c_dev.write_register(&g_km_i2c_dev, MCP23017_IODIRA, regval);
    }
#endif

  /* PCF8574: All pins default to quasi-bidirectional (output capable) */
}

/**
 * Name: km_i2c_config_col
 *
 * Description:
 *   Configure column pins as inputs with pull-up.
 */

static void km_i2c_config_col(kmatrix_pin_t pin)
{
  iinfo("I2C: Configuring pin %d as input (column)\n", pin);

#ifdef IOEXP_TYPE_MCP23017
  /* MCP23017: Set IODIR bit to 1 to make it an input */
  if (pin >= 4 && pin < 7)
    {
      uint8_t regval;
      int port = (pin >= 4) ? 1 : 0;
      uint8_t pin_bit = pin % 4;
      g_km_i2c_dev.read_register(&g_km_i2c_dev, MCP23017_IODIRB, &regval);
      regval |= (1 << pin_bit);
      g_km_i2c_dev.write_register(&g_km_i2c_dev, MCP23017_IODIRB, regval);

      /* Enable pull-up */
      g_km_i2c_dev.read_register(&g_km_i2c_dev, MCP23017_GPPUB, &regval);
      regval |= (1 << pin_bit);
      g_km_i2c_dev.write_register(&g_km_i2c_dev, MCP23017_GPPUB, regval);
    }
#endif

  /* PCF8574: Quasi-bidirectional pins with internal pull-up */
}

/**
 * Name: km_i2c_row_set
 *
 * Description:
 *   Control row output (active-low for matrix with diodes).
 */

static void km_i2c_row_set(kmatrix_pin_t pin, bool active)
{
  if (active)
    {
      g_km_i2c_dev.reg_cache &= ~(1 << pin);  /* Clear bit = low = active */
    }
  else
    {
      g_km_i2c_dev.reg_cache |= (1 << pin);   /* Set bit = high = inactive */
    }

  iinfo("I2C: Row set pin %d to %d (cache=0x%02x)\n", pin, active ? 0 : 1,
        g_km_i2c_dev.reg_cache);

  g_km_i2c_dev.write_register(&g_km_i2c_dev, 0, g_km_i2c_dev.reg_cache);
}

/**
 * Name: km_i2c_col_get
 *
 * Description:
 *   Read column input (active-low with pull-up).
 */

static bool km_i2c_col_get(kmatrix_pin_t pin)
{
  uint8_t value;

#ifdef IOEXP_TYPE_MCP23017
  uint8_t regaddr = (pin >= 4) ? MCP23017_GPIOB : MCP23017_GPIOA;
  uint8_t pin_bit = pin % 4;
#else
  uint8_t regaddr = 0;  /* PCF8574 has single register */
  uint8_t pin_bit = pin;
#endif

  g_km_i2c_dev.read_register(&g_km_i2c_dev, regaddr, &value);

  /* Return inverted: true = active (low), false = inactive (high) */
  bool result = (value & (1 << pin_bit)) == 0;

  iinfo("I2C: Col get pin %d = %d (reg=0x%02x)\n", pin, result, value);

  return result;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * Name: kmatrix_i2c_get_callbacks
 *
 * Description:
 *   Get the I2C callback functions to use in keyboard matrix config.
 *   This is called by board adapters to populate the callbacks.
 *
 * Returned Value:
 *   Structure with the callback function pointers.
 */

struct kmatrix_callbacks_s
{
  void (*config_row)(kmatrix_pin_t pin);
  void (*config_col)(kmatrix_pin_t pin);
  void (*row_set)(kmatrix_pin_t pin, bool active);
  bool (*col_get)(kmatrix_pin_t pin);
};

static struct kmatrix_callbacks_s g_km_i2c_callbacks =
{
  .config_row = km_i2c_config_row,
  .config_col = km_i2c_config_col,
  .row_set    = km_i2c_row_set,
  .col_get    = km_i2c_col_get,
};

FAR struct kmatrix_callbacks_s *kmatrix_i2c_get_callbacks(void)
{
  return &g_km_i2c_callbacks;
}

/**
 * Name: kmatrix_i2c_register
 *
 * Description:
 *   Register keyboard matrix driver using I2C GPIO expander.
 *
 * Input Parameters:
 *   config  - Keyboard matrix configuration (with callbacks set)
 *   devpath - Device path (e.g., "/dev/kbd0")
 *   i2c_bus - I2C bus number
 *   i2c_addr - I2C slave address of the expander
 *
 * Returned Value:
 *   Zero on success; negated errno on failure.
 */

int kmatrix_i2c_register(FAR const struct kmatrix_config_s *config,
                         FAR const char *devpath,
                         int i2c_bus, uint8_t i2c_addr)
{
  FAR struct i2c_master_s *i2c;
  int ret;

  iinfo("Initializing keyboard matrix via I2C at 0x%02x (bus %d)\n",
        i2c_addr, i2c_bus);

  /* Initialize I2C interface */

  i2c = i2c_bus_initialize(i2c_bus);
  if (i2c == NULL)
    {
      ierr("ERROR: Failed to initialize I2C bus %d\n", i2c_bus);
      return -ENODEV;
    }

  /* Store I2C interface and address in global device for callbacks */

  g_km_i2c_dev.i2c = i2c;
  g_km_i2c_dev.addr = i2c_addr;
  g_km_i2c_dev.reg_cache = 0xff;  /* All pins inactive initially */

#ifdef IOEXP_TYPE_PCF8574
  g_km_i2c_dev.read_register = km_pcf8574_read_register;
  g_km_i2c_dev.write_register = km_pcf8574_write_register;
  iinfo("Using PCF8574 I2C expander\n");
#endif

#ifdef IOEXP_TYPE_MCP23017
  g_km_i2c_dev.read_register = km_mcp23017_read_register;
  g_km_i2c_dev.write_register = km_mcp23017_write_register;
  iinfo("Using MCP23017 I2C expander\n");
#endif

  /* Register the keyboard matrix driver with provided config
   * (which must have callbacks already set by the board adapter)
   */

  ret = kmatrix_register(config, devpath);
  if (ret < 0)
    {
      ierr("ERROR: kmatrix_register failed: %d\n", ret);
      return ret;
    }

  iinfo("Keyboard matrix I2C driver registered successfully\n");
  return OK;
}
