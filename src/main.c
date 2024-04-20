/*
 * Main - Using the APA102 RGB strings to make something cool for the kids.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>




#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_DOTSTAR_LOG_LEVEL);
#define STRIP_NUM_LEDS 180

#define PULSE_LEDS 12 /* Width of the pulse */
#define COLOR_INCREMENT 20 /* Controls the color change rate */

const struct device *dev;
struct led_rgb strip_colors[STRIP_NUM_LEDS];

void setup() {
    dev = DEVICE_DT_GET(DT_NODELABEL(spistrip));
    if (!device_is_ready(dev)) {
        printk("LED strip device is not ready\n");
        return;
    }
}


void loop(int *pulse_position) {
    int i;
    int pulse_start = *pulse_position;
    int pulse_end = (*pulse_position + PULSE_LEDS) % STRIP_NUM_LEDS;

    if (pulse_end < pulse_start)
        pulse_start -= STRIP_NUM_LEDS;

    for (i = 0; i < STRIP_NUM_LEDS; i++) {
        if (i >= pulse_start && i < pulse_end) {
            strip_colors[i].r = 0;
            strip_colors[i].g = 0;
            strip_colors[i].b = 255;
        } else {
            strip_colors[i].r = 0;
            strip_colors[i].g = 0;
            strip_colors[i].b = 0;
        }
    }

    led_strip_update_rgb(dev, strip_colors, STRIP_NUM_LEDS);

    (*pulse_position)++;
    if (*pulse_position >= STRIP_NUM_LEDS)
        *pulse_position = 0;
}

void main(void)
{
    int pulse_position = 0;

    setup();

    while (1) {
        loop(&pulse_position);
        k_sleep(K_MSEC(500));
    }
}