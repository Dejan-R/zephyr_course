#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#define BUTTON_NODE DT_ALIAS(sw0)
#define STACK_SIZE 2048
#define THREAD_PRIORITY 7

#if !DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "Button alias sw0 must be defined in Devicetree"
#endif

static const struct gpio_dt_spec button =
    GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

static struct gpio_callback button_cb;

/* Initially empty binary semaphore (count = 0, max = 1). */
K_SEM_DEFINE(button_sem, 0, 1);

/* GPIO interrupt: only signal the waiting thread. */
static void button_pressed(const struct device *port,
                           struct gpio_callback *cb,
                           gpio_port_pins_t pins)
{
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    k_sem_give(&button_sem);
}

/* Thread: sleep until the button interrupt signals the semaphore. */
static void button_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (true) {
        k_sem_take(&button_sem, K_FOREVER);
        printk("[thread] Button pressed!\n");
    }
}

K_THREAD_DEFINE(button_thread_id, STACK_SIZE, button_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

int main(void)
{
    int ret;

    printk("Button -> ISR -> Semaphore -> Thread\n");

    if (!gpio_is_ready_dt(&button)) {
        printk("Button GPIO is not ready\n");
        return -1;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret < 0) {
        printk("Button configuration failed: %d\n", ret);
        return ret;
    }

    gpio_init_callback(&button_cb, button_pressed, BIT(button.pin));

    ret = gpio_add_callback(button.port, &button_cb);
    if (ret < 0) {
        printk("GPIO callback registration failed: %d\n", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        printk("Button interrupt configuration failed: %d\n", ret);
        return ret;
    }

    printk("Press the button on GPIO27\n");
    return 0;
}
