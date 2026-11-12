#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#define LED_NODE DT_ALIAS(led1)
#define BUTTON_NODE DT_ALIAS(sw0)

#if !DT_NODE_HAS_STATUS(LED_NODE, okay) || !DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "Define led1 and sw0 aliases in Devicetree"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback button_cb;

/* Work handler: runs in the system workqueue thread, not in the ISR. */
static void button_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    if (gpio_pin_toggle_dt(&led) < 0) {
        printk("LED toggle failed\n");
        return;
    }

    printk("[workqueue] LED toggled\n");
}

/* Statically define and initialize one work item. */
K_WORK_DEFINE(button_work, button_work_handler);

/* GPIO interrupt: only submit work. */
static void button_pressed(const struct device *port,
                           struct gpio_callback *cb,
                           gpio_port_pins_t pins)
{
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    /* A work item already queued is not queued a second time. */
    (void)k_work_submit(&button_work);
}

int main(void)
{
    int ret;

    if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button)) {
        printk("GPIO controller is not ready\n");
        return -1;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("LED configuration failed: %d\n", ret);
        return ret;
    }

    /* Pull-up and active-low polarity come from Devicetree. */
    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret < 0) {
        printk("Button configuration failed: %d\n", ret);
        return ret;
    }

    gpio_init_callback(&button_cb, button_pressed, BIT(button.pin));

    ret = gpio_add_callback(button.port, &button_cb);
    if (ret < 0) {
        printk("Button callback failed: %d\n", ret);
        return ret;
    }

    /* Enable the interrupt only after the callback is registered. */
    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        printk("Button interrupt setup failed: %d\n", ret);
        return ret;
    }

    printk("Button -> ISR -> Workqueue -> LED\n");
    printk("Press the button on GPIO27\n");
    return 0;
}
