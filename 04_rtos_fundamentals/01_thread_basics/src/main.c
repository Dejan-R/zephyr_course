#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 2048
#define WORKER_PRIORITY 5

#define WORKER_INTERVAL_MS 1000
#define MAIN_INTERVAL_MS 1500

static void worker_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (true) {
        printk("Worker thread running\n");
        k_msleep(WORKER_INTERVAL_MS);
    }
}

K_THREAD_DEFINE(worker_id, STACK_SIZE, worker_thread,
                NULL, NULL, NULL, WORKER_PRIORITY, 0, 0);

int main(void)
{
    printk("Thread Basics\n");

    while (true) {
        printk("Main thread running\n");
        k_msleep(MAIN_INTERVAL_MS);
    }
}