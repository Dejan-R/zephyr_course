#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define GPIO_NODE DT_NODELABEL(gpio0)

#if !DT_NODE_HAS_STATUS(GPIO_NODE, okay)
#error "GPIO controller is disabled in Devicetree"
#endif

static const struct device *const gpio_dev = DEVICE_DT_GET(GPIO_NODE);

int main(void)
{
    printk("Driver API Pattern\n");
    printk("Device: %s\n", gpio_dev->name);

    if (!device_is_ready(gpio_dev)) {
        printk("Device ready: no\n");
        return 0;
    }

    printk("Device ready: yes\n");

    /* Inspect the driver's dispatch table; do not call backend functions directly. */
    const struct gpio_driver_api *api =
        (const struct gpio_driver_api *)gpio_dev->api;

    printk("Driver API available: %s\n", api != NULL ? "yes" : "no");

    if (api != NULL) {
        printk("Pin configure operation: %s\n",
               api->pin_configure != NULL ? "available" : "unavailable");
        printk("Port read operation: %s\n",
               api->port_get_raw != NULL ? "available" : "unavailable");
    }

    return 0;
}
