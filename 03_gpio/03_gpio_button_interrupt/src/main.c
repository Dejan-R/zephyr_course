#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#define LED_NODE DT_ALIAS(led1)
#define BUTTON_NODE DT_ALIAS(button1)

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "LED node is missing or disabled in Devicetree"
#endif

#if !DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "Button node is missing or disabled in Devicetree"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

/* Callback data must remain valid for the lifetime of the interrupt. */
static struct gpio_callback button_cb_data;

/* GPIO callback: on ESP32 it runs in interrupt context. */
static void button_pressed(const struct device *port,
                           struct gpio_callback *cb,
                           gpio_port_pins_t pins)
{
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    /* Keep the callback short. No sleep, logging, or debounce here. */
    (void)gpio_pin_toggle_dt(&led);
}

int main(void)
{
    int ret;

    printk("GPIO Button Interrupt (Direct ISR)\n");

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

    /* Register the callback before enabling the GPIO interrupt. */
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));

    ret = gpio_add_callback(button.port, &button_cb_data);
    if (ret < 0) {
        printk("Failed to add button callback (error %d)\n", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        printk("Failed to configure button interrupt (error %d)\n", ret);
        return ret;
    }

    printk("LED GPIO pin: %u\n", (unsigned int)led.pin);
    printk("Button GPIO pin: %u\n", (unsigned int)button.pin);
    printk("Interrupt: edge to ACTIVE (button press)\n");
    printk("Press the button to toggle the LED\n");

    /* The main thread does not poll the button. */
    while (true) {
        k_sleep(K_FOREVER);
    }
}
