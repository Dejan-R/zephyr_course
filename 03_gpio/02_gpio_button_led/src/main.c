#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED_NODE DT_ALIAS(led1)
#define BUTTON_NODE DT_ALIAS(button1)
#define POLLING_INTERVAL_MS 10

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "LED node is missing or disabled in Devicetree"
#endif

#if !DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "Button node is missing or disabled in Devicetree"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

int main(void)
{
    int ret;
    int previous_state = -1;

    printk("GPIO Button LED (Polling)\n");

    if (!gpio_is_ready_dt(&led)) {
        printk("LED GPIO controller is not ready\n");
        return -1;
    }

    if (!gpio_is_ready_dt(&button)) {
        printk("Button GPIO controller is not ready\n");
        return -1;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Failed to configure LED GPIO (error %d)\n", ret);
        return ret;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret < 0) {
        printk("Failed to configure button GPIO (error %d)\n", ret);
        return ret;
    }

    printk("LED GPIO pin: %u\n", (unsigned int)led.pin);
    printk("Button GPIO pin: %u\n", (unsigned int)button.pin);
    printk("Polling interval: %d ms\n", POLLING_INTERVAL_MS);

    while (true) {
        int state = gpio_pin_get_dt(&button);

        if (state < 0) {
            printk("Failed to read button GPIO (error %d)\n", state);
            return state;
        }

        ret = gpio_pin_set_dt(&led, state);
        if (ret < 0) {
            printk("Failed to set LED GPIO (error %d)\n", ret);
            return ret;
        }

        if (state != previous_state) {
            printk("Button: %s | LED: %s\n",
                   state ? "PRESSED" : "RELEASED",
                   state ? "ON" : "OFF");
            previous_state = state;
        }

        k_sleep(K_MSEC(POLLING_INTERVAL_MS));
    }
}
