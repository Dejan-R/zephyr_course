#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 2048

#define WORKER_PRIORITY 7
#define OBSERVER_PRIORITY 5

#define WORK_TIME_US 600000
#define WORKER_SLEEP_MS 2000
#define OBSERVER_INTERVAL_MS 250

/* Thread 1 - simulated work followed by sleep */
static void worker_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (true) {
        printk("[worker] RUNNING - doing work\n");

        /* Busy wait is used only to demonstrate scheduling. */
        k_busy_wait(WORK_TIME_US);

        printk("[worker] going to sleep\n");
        k_msleep(WORKER_SLEEP_MS);
    }
}

K_THREAD_DEFINE(worker_id, STACK_SIZE, worker_thread,
                NULL, NULL, NULL, WORKER_PRIORITY, 0, 0);

/* Thread 2 - observe worker state */
static void observer_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    char state[32];

    while (true) {
        printk("[observer] worker state: %s\n",
               k_thread_state_str(worker_id, state, sizeof(state)));

        k_msleep(OBSERVER_INTERVAL_MS);
    }
}

K_THREAD_DEFINE(observer_id, STACK_SIZE, observer_thread,
                NULL, NULL, NULL, OBSERVER_PRIORITY, 0, 0);

int main(void)
{
    printk("Thread States\n");
    return 0;
}
