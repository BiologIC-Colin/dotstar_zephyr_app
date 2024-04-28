/*
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2024 TOKITA Hiroshi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#define LOG_LEVEL 4

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/shell/shell.h>
#include <stdlib.h>
#include <math.h>


#define STRIP_NODE        DT_NODELABEL(apa102)


#define STRIP_NUM_PIXELS    51

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

#define DELAY_TIME 100


#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

static const struct led_rgb colors[] = {
        RGB(0x0f, 0x00, 0x00), /* red */
        RGB(0x00, 0x0f, 0x00), /* green */
        RGB(0x00, 0x00, 0x0f), /* blue */
};

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);

bool fire_rgb_on = false;
bool rainbow = false;
// Struct to represent a color in RGB format
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color;

float hueToRGB(float p, float q, float t) {
    if (t < 0) t += 1;
    if (t > 1) t -= 1;
    if (t < 1.0/6) return p + (q - p) * 6 * t;
    if (t < 1.0/2) return q;
    if (t < 2.0/3) return p + (q - p) * (2.0/3 - t) * 6;
    return p;
}

// Function for converting HSL values to an RGB color
Color HSLtoRGB(float hue, float saturation, float lightness) {
    float r, g, b;

    if (saturation == 0)
    {
        r = g = b = lightness; // Achromatic color (gray scale)
    }
    else
    {
        float q = lightness < 0.5 ? lightness * (1 + saturation) : lightness + saturation - lightness * saturation;
        float p = 2 * lightness - q;
        r = hueToRGB(p, q, hue + 1.0/3);
        g = hueToRGB(p, q, hue);
        b = hueToRGB(p, q, hue - 1.0/3);
    }

    Color color = { r * 255, g * 255, b * 255 };
    return color;
}



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

void led_controller(void *p1, void *p2, void *p3) {
    size_t color = 0;
    int rc;

    while (1) {
        if (fire_rgb_on) {
            for (size_t cursor = 0; cursor < ARRAY_SIZE(pixels); cursor++) {
                memset(&pixels, 0x00, sizeof(pixels));
                memcpy(&pixels[cursor], &colors[color], sizeof(struct led_rgb));

                rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
                if (rc) {
                    LOG_ERR("couldn't update strip: %d", rc);
                }

                k_sleep(K_MSEC(DELAY_TIME));
            }

            color = (color + 1) % ARRAY_SIZE(colors);
        }
        if (rainbow){
            fire_rgb_on = false;

            // clear everything once before we set the colors
            memset(pixels, 0x00, sizeof(pixels));

            for (size_t cursor = 0; cursor < ARRAY_SIZE(pixels); cursor++) {
                float hue = (float) cursor / STRIP_NUM_PIXELS;

                // Calculate the color based on the hue
                Color color = HSLtoRGB(hue, 1, 0.5);

                // Set the color
                pixels[cursor].r = color.r;
                pixels[cursor].g = color.g;
                pixels[cursor].b = color.b;
            }

            // Update the strip once after all colors are set
            int rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
            if (rc) {
                LOG_ERR("Couldn't update strip: %d", rc);
            }

        }
        rainbow = false;
        k_sleep(K_MSEC(100));
    }
}

K_THREAD_DEFINE(my_controller_id, STACKSIZE,
                led_controller, NULL, NULL, NULL,
                PRIORITY, 0, 0);
extern const k_tid_t my_controller_id;

int main(void) {
    if (device_is_ready(strip)) {
        LOG_INF("Found LED strip device %s", strip->name);
    } else {
        LOG_ERR("LED strip device %s is not ready", strip->name);
        return 0;
    }
//    white_dimmer(1);
    rainbow = true;
    return 0;
}


/* Shell commands */
static int cmd_white_dimmer(const struct shell *shell, size_t argc, char **argv) {
    if (argc != 2) {
        shell_print(shell, "Incorrect number of arguments. Usage: %s <percentage>", argv[0]);
        return -1;
    }

    int percentage = atoi(argv[1]);

    white_dimmer(percentage);

    return 0;
}

static int cmd_toggle_fire(){
    fire_rgb_on = !fire_rgb_on;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_white_dimmer,
                               SHELL_CMD_ARG(set_dimmer, NULL, "Control white light intensity", cmd_white_dimmer, 2, 0),
                               SHELL_CMD(fire, NULL, "Toggle rgb firing", cmd_toggle_fire),
                               SHELL_SUBCMD_SET_END // Array terminated.
);
SHELL_CMD_REGISTER(dimmer, &sub_white_dimmer, "Commands to control white light intensity", NULL);
