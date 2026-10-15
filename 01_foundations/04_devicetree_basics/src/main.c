#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>
#include <zephyr/devicetree/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define LED_NODE DT_ALIAS(course_led)

#if !DT_NODE_EXISTS(LED_NODE)
#error "Devicetree alias 'course-led' is missing"
#endif

#define GPIO_NODE DT_GPIO_CTLR(LED_NODE, gpios)

#if !DT_NODE_HAS_STATUS(GPIO_NODE, okay)
#error "GPIO controller is disabled in Devicetree"
#endif

static const struct gpio_dt_spec led =
    GPIO_DT_SPEC_GET(LED_NODE, gpios);

static const struct device *const gpio_dev =
    DEVICE_DT_GET(GPIO_NODE);

int main(void)
{
    const struct device *runtime_dev = device_get_binding(gpio_dev->name);

    printk("Devicetree Basics\n");
    printk("LED label: %s\n", DT_PROP(LED_NODE, label));
    printk("GPIO controller: %s\n", gpio_dev->name);
    printk("LED GPIO pin: %u\n", (unsigned int)DT_GPIO_PIN(LED_NODE, gpios));
    printk("LED flags: %u\n", (unsigned int)DT_GPIO_FLAGS(LED_NODE, gpios));
    printk("GPIO controller ready: %s\n",
           device_is_ready(gpio_dev) ? "yes" : "no");
    printk("Same device instance: %s\n",
           led.port == gpio_dev ? "yes" : "no");
    printk("Runtime lookup: %s\n",
           runtime_dev != NULL ? "found" : "not found");
    printk("Runtime same instance: %s\n",
           runtime_dev == gpio_dev ? "yes" : "no");

    return 0;
}
