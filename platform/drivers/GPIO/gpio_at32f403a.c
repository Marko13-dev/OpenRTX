/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "drivers/GPIO/gpio_at32f403a.h"

#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/sys_io.h>

#define GPIO_CFGLR 0x00
#define GPIO_CFGHR 0x04
#define GPIO_SCR 0x10
#define GPIO_CLR 0x14

#define PIN_CFG_IN_FLOATING 0x4
#define PIN_CFG_IN_PULL 0x8

struct gpio_at32_port {
    const struct device *device;
    uintptr_t base;
};

#define GPIO_AT32_PORT(label)                               \
    static const struct gpio_at32_port gpio##label = {      \
        .device = DEVICE_DT_GET(DT_NODELABEL(gpio##label)), \
        .base = DT_REG_ADDR(DT_NODELABEL(gpio##label)),     \
    }

GPIO_AT32_PORT(a);
GPIO_AT32_PORT(b);
GPIO_AT32_PORT(c);
GPIO_AT32_PORT(d);
GPIO_AT32_PORT(e);

static int gpio_at32_mode(const struct gpioDev *dev, const uint8_t pin,
                          const uint16_t mode)
{
    const struct gpio_at32_port *port = dev->priv;
    gpio_flags_t flags;
    uint32_t pin_cfg = 0;
    bool configure_input = false;
    bool pull_up = false;
    bool pull_down = false;

    if (pin > 15)
        return -EINVAL;

    if (!device_is_ready(port->device))
        return -ENODEV;

    switch (mode & 0x00FF) {
        case INPUT:
            flags = GPIO_INPUT;
            configure_input = true;
            pin_cfg = PIN_CFG_IN_FLOATING;
            break;
        case INPUT_PULL_UP:
            flags = GPIO_INPUT;
            configure_input = true;
            pull_up = true;
            pin_cfg = PIN_CFG_IN_PULL;
            break;
        case INPUT_PULL_DOWN:
            flags = GPIO_INPUT;
            configure_input = true;
            pull_down = true;
            pin_cfg = PIN_CFG_IN_PULL;
            break;
        case ANALOG:
            flags = 0;
            break;
        case OUTPUT:
            flags = GPIO_OUTPUT;
            break;
        case OPEN_DRAIN:
            flags = GPIO_OUTPUT | GPIO_OPEN_DRAIN;
            break;
        case OPEN_DRAIN_PU:
            flags = GPIO_OUTPUT | GPIO_OPEN_DRAIN | GPIO_PULL_UP;
            break;
        default:
            return -ENOTSUP;
    }

    int ret = gpio_pin_configure(port->device, pin, flags);
    if (ret < 0)
        return ret;

    if (configure_input) {
        uintptr_t cfg_reg = port->base + ((pin < 8) ? GPIO_CFGLR : GPIO_CFGHR);
        uint32_t shift = (pin % 8) * 4;
        unsigned int key = irq_lock();

        /* The F403A HAL uses different encodings for input pull settings. */
        if (pull_up)
            sys_write32(BIT(pin), port->base + GPIO_SCR);
        else if (pull_down)
            sys_write32(BIT(pin), port->base + GPIO_CLR);

        uint32_t value = sys_read32(cfg_reg);
        value = (value & ~(0xFUL << shift)) | (pin_cfg << shift);
        sys_write32(value, cfg_reg);
        irq_unlock(key);
    }

    return 0;
}

static void gpio_at32_set(const struct gpioDev *dev, const uint8_t pin)
{
    const struct gpio_at32_port *port = dev->priv;
    (void)gpio_pin_set(port->device, pin, 1);
}

static void gpio_at32_clear(const struct gpioDev *dev, const uint8_t pin)
{
    const struct gpio_at32_port *port = dev->priv;
    (void)gpio_pin_set(port->device, pin, 0);
}

static bool gpio_at32_read(const struct gpioDev *dev, const uint8_t pin)
{
    const struct gpio_at32_port *port = dev->priv;
    return gpio_pin_get(port->device, pin) > 0;
}

static const struct gpioApi gpio_at32_api = {
    .mode = gpio_at32_mode,
    .set = gpio_at32_set,
    .clear = gpio_at32_clear,
    .read = gpio_at32_read,
};

#define GPIO_AT32_DEVICE(letter, port)                           \
    const struct gpioDev Gpio##letter = { .api = &gpio_at32_api, \
                                          .priv = &gpio##port }

GPIO_AT32_DEVICE(A, a);
GPIO_AT32_DEVICE(B, b);
GPIO_AT32_DEVICE(C, c);
GPIO_AT32_DEVICE(D, d);
GPIO_AT32_DEVICE(E, e);
