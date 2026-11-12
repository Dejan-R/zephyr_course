#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 2048
#define THREAD_PRIORITY 7

#define FAST_INTERVAL_MS 200
#define SLOW_INTERVAL_MS 1000

/* Devicetree LED definitions */
static const struct gpio_dt_spec led1 =
    GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

static const struct gpio_dt_spec led2 =
    GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

/* Thread 1 - fast blinking */
static void blink_fast(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    if (!gpio_is_ready_dt(&led1) ||
        gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE) < 0) {
        printk("LED1 initialization failed\n");
        return;
    }

    while (true) {
        gpio_pin_toggle_dt(&led1);
        k_msleep(FAST_INTERVAL_MS);
    }
}

/* Thread 2 - slow blinking */
static void blink_slow(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    if (!gpio_is_ready_dt(&led2) ||
        gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE) < 0) {
        printk("LED2 initialization failed\n");
        return;
    }

    while (true) {
        gpio_pin_toggle_dt(&led2);
        k_msleep(SLOW_INTERVAL_MS);
    }
}

/* Thread creation */
K_THREAD_DEFINE(thread1_id, STACK_SIZE, blink_fast,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

K_THREAD_DEFINE(thread2_id, STACK_SIZE, blink_slow,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

int main(void)
{
    printk("Two Threads / Two LEDs\n");

    return 0;
}