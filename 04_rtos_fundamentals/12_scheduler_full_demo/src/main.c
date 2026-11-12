#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 2048
#define HIGH_PRIORITY 1
#define LOW_PRIORITY 10

#define TIMER_PERIOD_MS 2000
#define LOW_WORK_US 300000

/* Semaphore: timer event -> HIGH thread. */
K_SEM_DEFINE(timer_sem, 0, 1);

/* Timer callback runs in interrupt context. */
static void timer_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    k_sem_give(&timer_sem);
}

K_TIMER_DEFINE(my_timer, timer_handler, NULL);

/* HIGH: blocks until the timer generates an event. */
static void high_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (true) {
        printk("[HIGH] waiting for timer\n");
        k_sem_take(&timer_sem, K_FOREVER);
        printk("[HIGH] timer event - preempts LOW\n");
    }
}

/* LOW: keeps working while HIGH is blocked. */
static void low_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (true) {
        printk("[LOW] working...\n");
        k_busy_wait(LOW_WORK_US);
    }
}

K_THREAD_DEFINE(high_id, STACK_SIZE, high_thread,
                NULL, NULL, NULL, HIGH_PRIORITY, 0, 0);

K_THREAD_DEFINE(low_id, STACK_SIZE, low_thread,
                NULL, NULL, NULL, LOW_PRIORITY, 0, 0);

int main(void)
{
    printk("Scheduler Full Demo\n");
    k_timer_start(&my_timer,
                  K_MSEC(TIMER_PERIOD_MS),
                  K_MSEC(TIMER_PERIOD_MS));
    return 0;
}
