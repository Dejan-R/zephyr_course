#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 2048
#define THREAD_PRIORITY 5

#define TIMER_DELAY_MS 1000
#define TIMER_PERIOD_MS 1000

/* Binary semaphore: timer signals the worker thread. */
K_SEM_DEFINE(timer_sem, 0, 1);

/* Timer callback runs in interrupt context. */
static void timer_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    k_sem_give(&timer_sem);
}

K_TIMER_DEFINE(my_timer, timer_handler, NULL);

/* The actual work runs in thread context. */
static void worker_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    unsigned int count = 0;

    while (true) {
        k_sem_take(&timer_sem, K_FOREVER);
        printk("[worker] timer tick: %u\n", ++count);
    }
}

K_THREAD_DEFINE(worker_id, STACK_SIZE, worker_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

int main(void)
{
    printk("Kernel Timer Example\n");

    /* First event after 1 s, then every 1 s. */
    k_timer_start(&my_timer, K_MSEC(TIMER_DELAY_MS),
                 K_MSEC(TIMER_PERIOD_MS));

    return 0;
}
