/*
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2024 TOKITA Hiroshi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <zephyr/shell/shell.h>

#define LOG_LEVEL 4
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <stdlib.h>

#define STRIP_NODE        DT_NODELABEL(apa102)


#define STRIP_NUM_PIXELS	51

#define DELAY_TIME K_MSEC(100)

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

static const struct led_rgb colors[] = {
        RGB(0x0f, 0x00, 0x00), /* red */
        RGB(0x00, 0x0f, 0x00), /* green */
        RGB(0x00, 0x00, 0x0f), /* blue */
};

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);

bool fire_rgb_on = false;

void white_dimmer(int percentage) {
    if (percentage <= 0 || percentage > 100) {
        LOG_ERR("Invalid percentage. It must be between 1 and 100.");
        return;
    }
    int intensity = 255 * percentage / 100;

    struct led_rgb new_rgb = {
            .r = intensity,
            .g = intensity,
            .b = intensity
    };

    // Assign the new color to every pixel
    for (int i = 0; i < STRIP_NUM_PIXELS; i++) {
        memcpy(&pixels[i], &new_rgb, sizeof(struct led_rgb));
    }

    //Update the strip to display the new color
    int rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
    if (rc) {
        LOG_ERR("couldn't update strip: %d", rc);
    }

}


int main(void)
{
    if (device_is_ready(strip)) {
        LOG_INF("Found LED strip device %s", strip->name);
    } else {
        LOG_ERR("LED strip device %s is not ready", strip->name);
        return 0;
    }
    white_dimmer(50);
    size_t color = 0;
    int rc;
    while (1){
    // Main program loop
        if (fire_rgb_on) {
            for (size_t cursor = 0; cursor < ARRAY_SIZE(pixels); cursor++) {
                memset(&pixels, 0x00, sizeof(pixels));
                memcpy(&pixels[cursor], &colors[color], sizeof(struct led_rgb));

                rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
                if (rc) {
                    LOG_ERR("couldn't update strip: %d", rc);
                }

                k_sleep(DELAY_TIME);
            }

            color = (color + 1) % ARRAY_SIZE(colors);
        }
    }
    return 0;
}

void cmd_firergb_toggle(){

    fire_rgb_on = !fire_rgb_on;
}

static int cmd_white_dimmer(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 2) {
        shell_print(shell, "Invalid number of arguments");
        return -EINVAL;
    }

    int percentage = atoi(argv[1]);

    if (percentage <= 0 || percentage > 100) {
        shell_print(shell, "Invalid percentage. It must be between 1 and 100.");
        return -EINVAL;
    }

    white_dimmer(percentage);

    return 0;
}

//SHELL_STATIC_SUBCMD_SET_CREATE(sub_white_dimmer,
//                               SHELL_CMD_ARG(set_dimmer, NULL, "<percentage>", cmd_white_dimmer, 2, 0),
////                               SHELL_CMD(rgb_toggle, NULL, "<Toggle RGB>", cmd_firergb_toggle),
//                               SHELL_SUBCMD_SET_END
//);
//
//SHELL_CMD_REGISTER(white_dimmer, &sub_white_dimmer, "Set white dimmer", NULL);