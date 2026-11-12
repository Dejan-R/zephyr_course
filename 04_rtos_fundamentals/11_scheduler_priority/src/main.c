#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 2048

#define HIGH_PRIORITY 1
#define MID_PRIORITY 5
#define LOW_PRIORITY 10

#define BUSY_WAIT_US 200000
#define MID_START_DELAY_MS 2000
#define HIGH_START_DELAY_MS 4000

/* Lowest priority: starts immediately. */
static void low_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (true) {
        printk("[LOW] running\n");
        k_busy_wait(BUSY_WAIT_US);
    }
}

/* Medium priority: becomes ready after 2 seconds. */
static void mid_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (true) {
        printk("[MID] running\n");
        k_busy_wait(BUSY_WAIT_US);
    }
}

/* Highest priority: becomes ready after 4 seconds. */
static void high_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (true) {
        printk("[HIGH] running\n");
        k_busy_wait(BUSY_WAIT_US);
    }
}

/* Staggered starts make priority preemption visible. */
K_THREAD_DEFINE(low_id, STACK_SIZE, low_thread,
                NULL, NULL, NULL, LOW_PRIORITY, 0, 0);

K_THREAD_DEFINE(mid_id, STACK_SIZE, mid_thread,
                NULL, NULL, NULL, MID_PRIORITY, 0, MID_START_DELAY_MS);

K_THREAD_DEFINE(high_id, STACK_SIZE, high_thread,
                NULL, NULL, NULL, HIGH_PRIORITY, 0, HIGH_START_DELAY_MS);

int main(void)
{
    printk("Scheduler Priority Example\n");
    printk("LOW starts now, MID after 2 s, HIGH after 4 s\n");
    return 0;
}
