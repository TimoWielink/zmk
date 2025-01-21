#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>

#include <zmk/behavior.h>
#include <zmk/behaviors.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/logging.h>

struct led_layer_config {
    const struct gpio_dt_spec *led_specs;
    size_t num_leds;
};

// LED configuration - can be customized through devicetree
static const struct gpio_dt_spec led_specs[] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(layer0_led), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(layer1_led), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(layer2_led), gpios),
};

static struct led_layer_config config = {
    .led_specs = led_specs,
    .num_leds = ARRAY_SIZE(led_specs),
};

// Handler for layer change events
static int led_layer_listener(const zmk_event_t *eh) {
    const struct zmk_layer_state_changed *event = as_zmk_layer_state_changed(eh);
    if (event == NULL) {
        return -ENOTSUP;
    }

    // Turn off all LEDs first
    for (int i = 0; i < config.num_leds; i++) {
        gpio_pin_set_dt(&config.led_specs[i], 0);
    }

    // Turn on LED for active layer
    if (event->layer < config.num_leds) {
        gpio_pin_set_dt(&config.led_specs[event->layer], 1);
    }

    return 0;
}

ZMK_LISTENER(led_layer_behavior, led_layer_listener);
ZMK_SUBSCRIPTION(led_layer_behavior, zmk_layer_state_changed);

// Initialize the behavior
static int led_layer_init(const struct device *dev) {
    for (int i = 0; i < config.num_leds; i++) {
        if (!device_is_ready(config.led_specs[i].port)) {
            return -ENODEV;
        }
        int ret = gpio_pin_configure_dt(&config.led_specs[i], GPIO_OUTPUT_INACTIVE);
        if (ret < 0) {
            return ret;
        }
    }
    return 0;
}

DEVICE_DT_DEFINE(DT_INST(0, zmk_behavior_led_layer), led_layer_init, NULL, NULL, &config,
                 APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, NULL);