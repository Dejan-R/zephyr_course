#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED_NODE DT_ALIAS(led1)
#define BLINK_INTERVAL_MS 1000

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "LED node is missing or disabled in Devicetree"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

int main(void)
{
    int ret;
    bool led_on = false;

    printk("GPIO Toggle LED\n");

    if (!gpio_is_ready_dt(&led)) {
        printk("GPIO controller is not ready\n");
        return -1;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Failed to configure GPIO pin (error %d)\n", ret);
        return ret;
    }

    printk("LED GPIO pin: %u\n", (unsigned int)led.pin);

    while (true) {
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0) {
            printk("Failed to toggle GPIO pin (error %d)\n", ret);
            return ret;
        }

        led_on = !led_on;
        printk("LED: %s\n", led_on ? "ON" : "OFF");

        k_sleep(K_MSEC(BLINK_INTERVAL_MS));
    }
}
